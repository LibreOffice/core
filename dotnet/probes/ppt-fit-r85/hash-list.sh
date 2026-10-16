#!/usr/bin/env bash
# Render a list of corpus-relative paths and print `path<TAB>md5`, deleting each PDF as it goes.
#   XRAY_OFFICE_CLI=... hash-list.sh <root> <list-file> <out.tsv> [workers]
set -uo pipefail
ROOT="${1:?root}"; LIST="${2:?list}"; OUT="${3:?out}"; W="${4:-2}"
export SOURCE_DATE_EPOCH=1700000000
: > "$OUT"
mapfile -t FILES < "$LIST"
TMP="$(mktemp -d)"
one() {
  local idx="$1" i=-1 rel f stem
  for rel in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % W)) -eq "$idx" ] || continue
    f="$ROOT/$rel"; stem="$(basename "${rel%.*}")"
    rm -rf "$TMP/t$idx"; mkdir -p "$TMP/t$idx"
    timeout -k 30 240 "$XRAY_OFFICE_CLI" render "$f" --format pdf --outdir "$TMP/t$idx" >/dev/null 2>&1
    if [ -f "$TMP/t$idx/$stem.pdf" ]; then
      printf '%s\t%s\n' "$rel" "$(md5sum "$TMP/t$idx/$stem.pdf" | cut -d' ' -f1)" >> "$OUT"
    else
      printf '%s\tFAILED\n' "$rel" >> "$OUT"
    fi
    rm -rf "$TMP/t$idx"
  done
}
for w in $(seq 0 $((W - 1))); do one "$w" & done
wait
rm -rf "$TMP"
echo "rows $(wc -l < "$OUT")"
