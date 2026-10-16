#!/bin/sh
# Per-page dominant text size for every document in $1, both ways, into $2.
# Each rendering is measured and then deleted: this container has under half a gigabyte free.
set -e
LIST="$1"; OUT="$2"
: > "$OUT"
while IFS= read -r doc; do
    [ -n "$doc" ] || continue
    key=$(printf '%s' "$doc" | md5sum | cut -c1-16)
    dir="/tmp/r84-size-$key"
    rm -rf "$dir"; mkdir -p "$dir/ref" "$dir/ours"
    ./ref-render.sh "$doc" "$dir/ref" >/dev/null 2>&1 || true
    SOURCE_DATE_EPOCH=1700000000 "$XRAY_OFFICE_CLI" render --outdir "$dir/ours" "$doc" >/dev/null 2>&1 || true
    base=$(basename "${doc%.*}")
    if [ -f "$dir/ref/$base.pdf" ] && [ -f "$dir/ours/$base.pdf" ]; then
        python3 - "$dir/ref/$base.pdf" "$dir/ours/$base.pdf" "$base" >> "$OUT" <<'PY'
import sys
sys.path.insert(0, '.')
from sizes import dominant
ref, ours, name = sys.argv[1], sys.argv[2], sys.argv[3]
r, o = dominant(ref), dominant(ours)
for i in range(min(len(r), len(o))):
    print(f'{name}\t{i+1}\t{o[i][0]}\t{r[i][0]}\t{o[i][1]}\t{r[i][1]}')
PY
    else
        printf '%s\t0\tFAILED\tFAILED\t0\t0\n' "$base" >> "$OUT"
    fi
    rm -rf "$dir"
done < "$LIST"
