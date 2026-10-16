#!/bin/sh
# Render the whole words track, three at a time, one output directory per DOCUMENT --
# never per worker slot, which is what cost an earlier round 124 renders when two live
# renders landed in one directory.
#
# The track is taken from MANIFEST.tsv rather than from a `find`, because this mount
# materialises a case-variant alias whenever a tool resolves a document by a second
# spelling and a filesystem walk then counts the same inode twice.
OUT="$1"
JOBS="${2:-3}"
CORPUS=/home/user/sample-files
mkdir -p "$OUT/pdf"
: > "$OUT/fp.txt"
export OUT CORPUS
awk -F'\t' 'NR>1 && $1=="words" {print $3}' "$CORPUS/MANIFEST.tsv" \
  | xargs -P "$JOBS" -I{} sh -c '
    rel="$1"
    f="$CORPUS/$rel"
    b=`echo "$rel" | tr / _`
    w="$OUT/pdf/$b"
    mkdir -p "$w"
    SOURCE_DATE_EPOCH=0 "$XRAY_OFFICE_CLI" render "$f" --outdir "$w" >/dev/null 2>&1
    p=`ls "$w"/*.pdf 2>/dev/null | head -1`
    if [ -n "$p" ]; then
        printf "%s\t%s\n" "`md5sum < "$p" | cut -d" " -f1`" "$rel" >> "$OUT/fp.txt"
    else
        printf "OURS-FAILED\t%s\n" "$rel" >> "$OUT/fp.txt"
    fi' _ {}
echo "rows `wc -l < "$OUT/fp.txt"`"
