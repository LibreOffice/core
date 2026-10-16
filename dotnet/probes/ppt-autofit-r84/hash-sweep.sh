#!/bin/sh
# Render every document named in $1 with $XRAY_OFFICE_CLI and record the PDF's md5 in $2,
# deleting each rendering as it goes -- this container has under half a gigabyte free and
# the comparison only needs the digest.
#
# One directory per DOCUMENT, never per worker slot: two live renders in one directory is
# the failure `dotnet/CLAUDE.md` records twice, and it reads as documents that would not
# render rather than as an error.
set -e
LIST="$1"; OUT="$2"; JOBS="${3:-2}"
: > "$OUT"
xargs -a "$LIST" -d '\n' -P "$JOBS" -I{} sh -c '
    doc="$1"; out="$2"
    key=$(printf "%s" "$doc" | md5sum | cut -c1-16)
    dir="/tmp/r84-render-$key"
    rm -rf "$dir"; mkdir -p "$dir"
    if SOURCE_DATE_EPOCH=1700000000 "$XRAY_OFFICE_CLI" render --outdir "$dir" "$doc" >/dev/null 2>&1 \
       && ls "$dir"/*.pdf >/dev/null 2>&1; then
        sum=$(md5sum "$dir"/*.pdf | cut -d" " -f1)
        pages=$(pdfinfo "$dir"/*.pdf 2>/dev/null | awk "/^Pages/{print \$2}")
    else
        sum="FAILED"; pages=""
    fi
    printf "%s\t%s\t%s\n" "$doc" "$sum" "$pages" >> "$out"
    rm -rf "$dir"
' _ {} "$OUT"
