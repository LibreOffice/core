#!/usr/bin/env bash
# Render one corpus track with a given CLI into per-format-identity PDFs.
#
#   sweep.sh <cli> <track> <outdir> [jobs]
#
# <track> is words|slides|sheets. Output is <outdir>/<stem>__<ext>.pdf, the same
# identity the canonical reference renderings use, so a rendering can be compared
# across two builds without an extension collision hiding half the corpus.
#
# WHY THE COMPLETENESS CHECK. A first run of this reported "rendered 163 of 163"
# and left 158 files behind: the container was killed between the write and the
# flush, and the shortfall was invisible until the files were counted again in a
# later session. A sweep whose count is taken from its own in-flight bookkeeping
# cannot see that. So the count is re-taken from the directory at the end, the
# missing inputs are named, and a second pass re-renders them; a sweep that still
# does not match its input count exits non-zero rather than reporting a number.
set -uo pipefail

CLI=$1
TRACK=$2
OUT=$3
JOBS=${4:-6}
CORPUS=${CORPUS:-/c/sandbox/workdir/sample-files}

export SOURCE_DATE_EPOCH=1700000000
export TZ=UTC

mkdir -p "$OUT"

# The output name for an input: stem and extension, the reference corpus's identity.
identity() {
    local base ext stem
    base=$(basename "$1")
    ext=$(printf '%s' "${base##*.}" | tr '[:upper:]' '[:lower:]')
    stem=${base%.*}
    printf '%s__%s.pdf' "$stem" "$ext"
}
export -f identity

render_one() {
    local file=$1 tmp stem out
    out="$OUT/$(identity "$file")"
    [ -s "$out" ] && return 0

    stem=$(basename "$file"); stem=${stem%.*}
    tmp=$(mktemp -d)
    if "$CLI" render --quiet --outdir "$tmp" "$file" >/dev/null 2>&1 \
        && [ -f "$tmp/$stem.pdf" ]; then
        mv "$tmp/$stem.pdf" "$out"
    else
        printf 'FAIL\t%s\n' "$file" >> "$OUT/_failures.tsv"
    fi
    rm -rf "$tmp"
}
export -f render_one
export CLI OUT

inputs=$(mktemp)
find "$CORPUS/$TRACK" -type f -not -name '.*' > "$inputs"
total=$(wc -l < "$inputs")

for pass in 1 2; do
    missing=$(mktemp)
    while IFS= read -r file; do
        [ -s "$OUT/$(identity "$file")" ] || printf '%s\n' "$file"
    done < "$inputs" > "$missing"

    count=$(wc -l < "$missing")
    [ "$count" -eq 0 ] && { rm -f "$missing"; break; }
    echo "pass $pass: rendering $count of $total"
    xargs -a "$missing" -d '\n' -P "$JOBS" -I{} bash -c 'render_one "$@"' _ {}
    rm -f "$missing"
done

# Re-counted from the directory, never from the loop's own bookkeeping.
have=$(find "$OUT" -maxdepth 1 -name '*.pdf' | wc -l)
echo "rendered $have of $total into $OUT"
rm -f "$inputs"
[ "$have" -eq "$total" ] || { echo "INCOMPLETE: $((total - have)) missing" >&2; exit 1; }
