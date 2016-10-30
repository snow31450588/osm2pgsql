/* One implementation of output-layer processing for osm2pgsql.
 * Manages a single table, transforming geometry using a
 * variety of algorithms plus tag transformation for the
 * database columns.
 */

#ifndef OUTPUT_MULTI_HPP
#define OUTPUT_MULTI_HPP

#include "expire-tiles.hpp"
#include "id-tracker.hpp"
#include "osmtypes.hpp"
#include "output.hpp"
#include "geometry-processor.hpp"

#include <cstddef>
#include <string>
#include <memory>

class table_t;
class tagtransform;
struct export_list;
struct middle_query_t;
struct options_t;

class output_multi_t : public output_t {
public:
    output_multi_t(const std::string &name,
                   std::shared_ptr<geometry_processor> processor_,
                   const export_list &export_list_,
                   const middle_query_t* mid_, const options_t &options_);
    output_multi_t(const output_multi_t& other);
    virtual ~output_multi_t();

    virtual std::shared_ptr<output_t> clone(const middle_query_t* cloned_middle) const;

    int start();
    void stop();
    void commit();

    void enqueue_ways(pending_queue_t &job_queue, osmid_t id, size_t output_id, size_t& added);
    int pending_way(osmid_t id, int exists);

    void enqueue_relations(pending_queue_t &job_queue, osmid_t id, size_t output_id, size_t& added);
    int pending_relation(osmid_t id, int exists);

    int node_add(osmium::Node const &node, double lat, double lon, bool extra_tags) override;
    int way_add(osmium::Way const &way, bool extra_tags) override;
    int relation_add(osmium::Relation const &rel, bool extra_tags) override;

    int node_modify(osmium::Node const &node, double lat, double lon, bool extra_tags) override;
    int way_modify(osmium::Way const &way, bool extra_tags) override;
    int relation_modify(osmium::Relation const &rel, bool extra_tags) override;

    int node_delete(osmid_t id);
    int way_delete(osmid_t id);
    int relation_delete(osmid_t id);

    size_t pending_count() const;

    void merge_pending_relations(output_t *other);
    void merge_expire_trees(output_t *other);

protected:

    void delete_from_output(osmid_t id);
    int process_node(osmium::Node const &node, bool extra, double lat, double lon);
    int process_way(osmium::Way const &way, bool extra);
    int reprocess_way(osmium::Way const &way, bool exists);
    int process_relation(osmium::Relation const &rel, bool extra, bool exists, bool pending=false);
    void copy_node_to_table(osmid_t id, const std::string &geom, taglist_t &tags);
    void copy_to_table(const osmid_t id, const geometry_builder::pg_geom_t &geom, taglist_t &tags, int polygon);

    std::unique_ptr<tagtransform> m_tagtransform;
    std::unique_ptr<export_list> m_export_list;
    std::shared_ptr<geometry_processor> m_processor;
    osmium::item_type const m_osm_type;
    std::unique_ptr<table_t> m_table;
    id_tracker ways_pending_tracker, rels_pending_tracker;
    std::shared_ptr<id_tracker> ways_done_tracker;
    expire_tiles m_expire;
    way_helper m_way_helper;
    relation_helper m_relation_helper;
    osmium::memory::Buffer buffer;
};

#endif
