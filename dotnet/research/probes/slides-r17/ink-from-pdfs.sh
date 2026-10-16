#!/usr/bin/env bash
# Compute the ink columns from an existing batch-check.sh output directory, without
# re-rendering anything. Takes <outdir> [workers]; reads <outdir>/ours and <outdir>/ref
# and the paths in <outdir>/rows.tsv, and writes <outdir>/cmp/<id>.txt and <outdir>/ink.tsv
# in exactly the shape track-ink-sweep.sh writes them, so ink-columns.py can read either.
set -uo pipefail
OUT="$(cd "${1:?usage: ink-from-pdfs.sh <outdir> [workers]}" && pwd)"
WORKERS="${2:-2}"
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
[ -f "$DIFF" ] || { echo "no pdf-image-diff.py at $DIFF" >&2; exit 1; }
mkdir -p "$OUT/cmp"
: > "$OUT/ink.tsv"

mapfile -t ROWS < <(sort "$OUT/rows.tsv")

one() {
  local idx="$1" i=-1 row path ext base stem id o r op ink major pages v
  for row in "${ROWS[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    path="$(cut -f1 <<<"$row")"; v="$(cut -f7 <<<"$row")"
    op="$(cut -f3 <<<"$row" | cut -d/ -f1)"; rp="$(cut -f3 <<<"$row" | cut -d/ -f2)"
    base="$(basename "$path")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    o="$OUT/ours/$id.pdf"; r="$OUT/ref/$id.pdf"
    ink="-"; major="-"; pages="-"
    if [ -f "$o" ] && [ -f "$r" ] && [ "$op" = "$rp" ]; then
      rm -rf "$OUT/c$idx"
      timeout 900 python3 "$DIFF" "$o" "$r" --outdir "$OUT/c$idx" > "$OUT/cmp/$id.txt" 2>&1
      rm -rf "$OUT/c$idx"
      ink=$(awk -F'\t' '$1 ~ /^[0-9]+$/ && $3 ~ /^-?[0-9.]+$/ {s+=$3} END{printf "%.2f", s}' "$OUT/cmp/$id.txt")
      major=$(awk '/pages, .* with major differences/{print $3}' "$OUT/cmp/$id.txt")
      pages="$op"
      [ -n "$ink" ] || ink="?"
      [ -n "$major" ] || major="?"
    fi
    printf "%s\t%s\t%s\t%s\t%s\n" "$path" "$pages" "$ink" "$major" "$v" >> "$OUT/ink.tsv"
  done
}
for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT/ink.tsv" "$OUT/ink.tsv"
awk -F'\t' '$3!="-" && $3!="?" {i+=$3; m+=$4; n++}
            END{printf "INK %.2f  MAJOR PAGES %d  over %d documents\n", i, m, n}' "$OUT/ink.tsv"
