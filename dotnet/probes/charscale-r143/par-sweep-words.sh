#!/bin/sh
# Render every words-track corpus document, three at a time, one output
# directory per DOCUMENT.
OUT="$1"
JOBS="${2:-3}"
mkdir -p "$OUT/pdf"
: > "$OUT/fp.txt"
export OUT
find /home/user/sample-files/words -type f \( -iname '*.docx' -o -iname '*.doc' -o -iname '*.docm' \) \
  | sort | xargs -P "$JOBS" -I{} sh -c '
    f="$1"
    b=`basename "$f"`
    b=`echo "$b" | sed "s/\.[^.]*$//"`
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
