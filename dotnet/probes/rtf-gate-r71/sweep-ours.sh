#!/usr/bin/env bash
# Render OUR half of the .rtf column of /home/user/corpus-odf and measure the gate columns.
#
#   XRAY_OFFICE_CLI=... sweep-ours.sh <corpus-root> <outdir> [workers] [glob]
#
# Sound because the diff under test is confined to `dotnet/src` and cannot touch `soffice`,
# so the reference columns banked in gate-odf-rows.tsv still describe the same bytes.
# Writes `ours.tsv`; score.py joins the banked reference columns and applies the gate rule.
set -uo pipefail
ROOT="${1:?corpus root}"; OUT="${2:?outdir}"; WORKERS="${3:-4}"; GLOB="${4:-}"
mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI to the binary you mean to measure}"
[ -x "$CLI" ] || { echo "no CLI at $CLI" >&2; exit 1; }
mkdir -p "$OUT/ours"
: > "$OUT/ours.tsv"

mapfile -t FILES < <(find "$ROOT" -name '*.rtf' -type f | sort)
if [ -n "$GLOB" ]; then
  mapfile -t FILES < <(printf '%s\n' "${FILES[@]}" | grep -F "$GLOB")
fi
echo "${#FILES[@]} documents" >&2

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

one() {
  local idx="$1" i=-1 f base stem id o op ow oraw og of un
  # keyed on the worker slot but each render is moved out immediately, and the directory is
  # wiped before every render, so two live renders can never share it.
  mkdir -p "$OUT/t$idx"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    base="$(basename "$f")"; stem="${base%.*}"; id="${stem}__rtf"
    o="$OUT/ours/$id.pdf"
    rm -rf "${OUT:?}/t$idx"; mkdir -p "$OUT/t$idx"
    timeout 300 "$CLI" render "$f" --format pdf --outdir "$OUT/t$idx" >/dev/null 2>&1
    [ -f "$OUT/t$idx/$stem.pdf" ] && mv -f "$OUT/t$idx/$stem.pdf" "$o"
    op="-"; ow="-"; oraw="-"; og="-"; of="-"; un="-"
    if [ -f "$o" ]; then
      op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r ow oraw og < <(words_of "$o")
      of=$(pdffonts "$o" 2>/dev/null | tail -n +3 | grep -c .)
      un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
    fi
    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
      "${f#"$ROOT"/}" "$op" "$ow" "$oraw" "$og" "$of" "$un" >> "$OUT/ours.tsv"
  done
  rm -rf "${OUT:?}/t$idx"
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
sort -o "$OUT/ours.tsv" "$OUT/ours.tsv"
wc -l < "$OUT/ours.tsv"
