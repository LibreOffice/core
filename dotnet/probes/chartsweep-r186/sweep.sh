#!/usr/bin/env bash
# Render every chart-bearing corpus document both ways and score it.
#
#   ./sweep.sh <work-dir> [chartdocs.tsv]
#
# One output directory per *document*, keyed on the path, never per worker slot -- two live
# renders sharing a directory silently delete each other's output, which cost an earlier
# round 124 renders. It caches the reference half, so a re-run after a rebuild renders only
# our side. It TRUNCATES rows.tsv on entry: point it at a fresh directory rather than an old
# one, or two runs append to one file and each clears the other's scratch between documents.
set -uo pipefail
OUT=${1:?work dir}; mkdir -p "$OUT/ref" "$OUT/ours" "$OUT/cmp"
DOCS=${2:-$(dirname "$0")/chartdocs.tsv}
CLI=${XRAY_OFFICE_CLI:-/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli}
REF=${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}
DIFF=${DIFF:-/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py}
echo "reference $REF -- $("$REF" --version 2>/dev/null | head -1)"
echo "measuring $CLI"
export SOURCE_DATE_EPOCH=0
: > "$OUT/rows.tsv"
n=0
while IFS=$'\t' read -r kinds path; do
  n=$((n+1))
  id=$(printf '%s' "$path" | md5sum | cut -c1-12)
  stem=$(basename "$path"); stem="${stem%.*}"
  [ -f "$OUT/ref/$id.pdf" ] || { timeout 180 "$REF" -env:UserInstallation=file:///tmp/lo-$id \
      --headless --convert-to pdf --outdir "$OUT/ref/$id" "$path" >/dev/null 2>&1
      mv "$OUT/ref/$id/$stem.pdf" "$OUT/ref/$id.pdf" 2>/dev/null; rm -rf "$OUT/ref/$id"; }
  rm -rf "$OUT/ours/$id"
  timeout 180 "$CLI" render --quiet --outdir "$OUT/ours/$id" "$path" >/dev/null 2>&1
  o=$(ls "$OUT/ours/$id"/*.pdf 2>/dev/null | head -1)
  if [ ! -f "$OUT/ref/$id.pdf" ] || [ -z "$o" ]; then
    printf '%s\t%s\t-\t-\t-\tfailed\n' "$kinds" "$path" >> "$OUT/rows.tsv"; continue
  fi
  timeout 300 python3 "$DIFF" "$o" "$OUT/ref/$id.pdf" --outdir "$OUT/c-$id" > "$OUT/cmp/$id.txt" 2>&1
  rm -rf "$OUT/c-$id"
  read -r worst sum pages < <(awk -F'\t' '$1 ~ /^[0-9]+$/ && $4 ~ /^[0-9.]+$/ {s+=$4; if($4+0>m)m=$4+0; n++}
                              END{printf "%.2f %.2f %d", m, s, n}' "$OUT/cmp/$id.txt")
  [ -n "$pages" ] && [ "$pages" != "0" ] || { printf '%s\t%s\t-\t-\t-\tpages\n' "$kinds" "$path" >> "$OUT/rows.tsv"; continue; }
  printf '%s\t%s\t%s\t%s\t%s\tok\n' "$kinds" "$path" "$worst" "$sum" "$pages" >> "$OUT/rows.tsv"
done < "$DOCS"
echo "DONE $n"
