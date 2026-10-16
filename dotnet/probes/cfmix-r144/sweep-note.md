The sweep is `par-sweep.sh`, run twice with nothing but the binary changing, under
`SOURCE_DATE_EPOCH=0` so a workbook printing `&D` in its header does not move on its own.

Three habits this file exists to record:

* **One output directory per DOCUMENT, never per worker slot.** A thread pool does not work
  consecutive indices, so a slot-keyed directory gets two live renders in it and one `rm -rf`s
  the other's output — silently, as fewer rows rather than as an error.
* **The document list comes from `MANIFEST.tsv`, not from `find`.** This mount materialises a
  case-variant alias whenever a tool resolves a document by a second spelling, so a filesystem
  walk counts the same inode twice and a sweep `TOTAL` drifts upwards with no commit to the
  corpus.
* **A sweep and a rebuild must never overlap.** Both legs here were run to completion with no
  build of any kind in between; the check afterwards is that the newest render in each leg is
  older than the binary that produced the next one.
