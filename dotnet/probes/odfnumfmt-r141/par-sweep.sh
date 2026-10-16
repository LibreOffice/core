#!/bin/sh
# Render every converted .ods, three at a time, one output directory per
# DOCUMENT -- never per worker slot, which is what cost an earlier round 124
# renders when two live renders landed in one directory.
OUT="$1"
JOBS="${2:-3}"
mkdir -p "$OUT/pdf"
: > "$OUT/fp.txt"
export OUT
ls /home/user/corpus-odf/ods/*.ods | xargs -P "$JOBS" -I{} sh -c '
    f="$1"
    b=`basename "$f" .ods`
    w="$OUT/pdf/$b"
    mkdir -p "$w"
    SOURCE_DATE_EPOCH=0 "$XRAY_OFFICE_CLI" render "$f" --outdir "$w" >/dev/null 2>&1
    p="$w/$b.pdf"
    if [ -f "$p" ]; then
        printf "%s\t%s\n" "`md5sum < "$p" | cut -d" " -f1`" "$b" >> "$OUT/fp.txt"
    else
        printf "OURS-FAILED\t%s\n" "$b" >> "$OUT/fp.txt"
    fi' _ {}
echo "rows `wc -l < "$OUT/fp.txt"`"
