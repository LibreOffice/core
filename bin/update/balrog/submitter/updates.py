import site
import os

site.addsitedir(os.path.join(os.path.dirname(__file__), "..", ".."))
import jsonmerge


def merge_partial_updates(base_obj, new_obj):
    """Merges 2 update objects, merging partials and replacing completes"""
    schema = {
        "properties": {
            # Merge partials using "from" as an identifier field
            "partials": {
                "mergeStrategy": "arrayMergeById",
                "mergeOptions": {
                    "idRef": "from"
                }
            },
            # Replace completes - we don't usually have more than one
            "completes": {
                "mergeStrategy": "overwrite"
            }
        }
    }
    merger = jsonmerge.Merger(schema=schema)
    return merger.merge(base_obj, new_obj)
