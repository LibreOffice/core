#!/usr/bin/env bash
# Per-page dominant drawn text size for a track, one document at a time, deleting as it goes.
#
#   XRAY_OFFICE_CLI=... size-sweep.sh <root> <prefix> <ext-regex> <out.tsv> [workers]
#
# With XRAY_OFFICE_CLI unset and BANK set, censuses the banked REFERENCE PDFs instead of rendering.
set -uo pipefail
ROOT="${1:?root}"; PREFIX="${2:?prefix}"; EXTS="${3:?ext regex}"; OUT="${4:?out.tsv}"; W="${5:-2}"
export SOURCE_DATE_EPOCH=1700000000
HERE="$(cd "$(dirname "$0")" && pwd)"
: > "$OUT"
mapfile -t FILES < <(git -C "$ROOT" -c core.quotePath=false ls-files "$PREFIX" \
  | grep -Ei "$EXTS" | sed "s|^|$ROOT/|")
echo "${#FILES[@]} documents" >&2
TMP="$(mktemp -d)"
one() {
  local idx="$1" i=-1 f base ext stem id
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % W)) -eq "$idx" ] || continue
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"; id="${stem}__${ext,,}"
    if [ -n "${BANK:-}" ]; then
      [ -f "$BANK/ref/$id.pdf" ] || continue
      python3 "$HERE/sizes.py" "$BANK/ref/$id.pdf" 2>/dev/null \
        | awk -F'\t' -v id="$id" '{print id"\t"$2"\t"$3"\t"$4}' >> "$OUT"
    else
      rm -rf "$TMP/t$idx"; mkdir -p "$TMP/t$idx"
      timeout -k 30 240 "$XRAY_OFFICE_CLI" render "$f" --format pdf --outdir "$TMP/t$idx" >/dev/null 2>&1
      [ -f "$TMP/t$idx/$stem.pdf" ] && python3 "$HERE/sizes.py" "$TMP/t$idx/$stem.pdf" 2>/dev/null \
        | awk -F'\t' -v id="$id" '{print id"\t"$2"\t"$3"\t"$4}' >> "$OUT"
      rm -rf "$TMP/t$idx"
    fi
  done
}
for w in $(seq 0 $((W - 1))); do one "$w" & done
wait
rm -rf "$TMP"
echo "rows $(wc -l < "$OUT")"
