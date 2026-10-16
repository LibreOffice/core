#!/bin/bash
# Render one extension column with two binaries and report which renderings differ.
#
# `SOURCE_DATE_EPOCH` is fixed on both legs so a printed date cannot make two runs of the
# same tree differ (dotnet/CLAUDE.md, "Set SOURCE_DATE_EPOCH when comparing two renderings
# byte for byte"). Each document gets its own working directory keyed on its own path --
# never on a worker slot -- because a thread pool does not walk consecutive indices, and two
# live renders in one directory silently lose each other's output.
#
# usage: reach.sh <glob> <outdir> <jobs>
set -u
GLOB="$1"; OUT="$2"; JOBS="${3:-2}"
BASE="${BASE_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}"
HEAD="${HEAD_CLI:-/home/user/wt-pptxfield/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}"
export SOURCE_DATE_EPOCH=1700000000
mkdir -p "$OUT/movers"
: > "$OUT/rows.tsv"

render() {
    doc="$1"
    key=$(printf '%s' "$doc" | md5sum | cut -c1-16)
    d="$OUT/w/$key"
    rm -rf "$d"; mkdir -p "$d/a" "$d/b"
    timeout -k 30 300 "$BASE" render "$doc" --outdir "$d/a" >/dev/null 2>&1 || { echo -e "$doc\tbase-failed" >> "$OUT/rows.tsv"; rm -rf "$d"; return; }
    timeout -k 30 300 "$HEAD" render "$doc" --outdir "$d/b" >/dev/null 2>&1 || { echo -e "$doc\thead-failed" >> "$OUT/rows.tsv"; rm -rf "$d"; return; }
    ha=$(md5sum "$d"/a/*.pdf 2>/dev/null | awk '{print $1}')
    hb=$(md5sum "$d"/b/*.pdf 2>/dev/null | awk '{print $1}')
    if [ -z "$ha" ] || [ -z "$hb" ]; then
        echo -e "$doc\tno-output" >> "$OUT/rows.tsv"
    elif [ "$ha" = "$hb" ]; then
        echo -e "$doc\tsame" >> "$OUT/rows.tsv"
    else
        echo -e "$doc\tDIFFER" >> "$OUT/rows.tsv"
        mkdir -p "$OUT/movers/$key"
        cp "$d"/a/*.pdf "$OUT/movers/$key/base.pdf"
        cp "$d"/b/*.pdf "$OUT/movers/$key/head.pdf"
        echo "$doc" > "$OUT/movers/$key/doc"
    fi
    rm -rf "$d"
}
export -f render
export OUT BASE HEAD

# A list file (one path per line) or a glob, so a second run can name exactly the population
# the change under test can reach rather than re-rendering the whole column.
if [ -f "$GLOB" ]; then cat "$GLOB"; else ls -d $GLOB 2>/dev/null; fi \
    | xargs -d '\n' -I{} -P "$JOBS" bash -c 'render "$@"' _ {}
echo "--- $(wc -l < "$OUT/rows.tsv") rows"
awk -F'\t' '{c[$2]++} END {for (k in c) print k, c[k]}' "$OUT/rows.tsv"
