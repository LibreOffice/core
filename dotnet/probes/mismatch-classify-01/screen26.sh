#!/usr/bin/env bash
# Render a list of corpus documents with LibreOffice 26.2.4.2 and record pages + glyphs.
#
#   screen26.sh <corpus-root> <list-file> <outdir> [workers]
#
# The list file holds corpus-relative paths, one per line. Output: <outdir>/ref26/<id>.pdf
# and <outdir>/rows26.tsv with "id<TAB>pages<TAB>words<TAB>rawwords<TAB>glyphs".
#
# `id` is `<stem>__<lowercased ext>`, the same identity batch-check.sh uses, so the rows
# join to a stored parity.tsv without any further mangling.
set -uo pipefail

ROOT_DIR="${1:?corpus root}"
LIST="${2:?list file}"
OUT="${3:?outdir}"
WORKERS="${4:-4}"
SOFFICE="${SOFFICE:-/opt/libreoffice26.2/program/soffice}"

mkdir -p "$OUT" && OUT="$(cd "$OUT" && pwd)"
mkdir -p "$OUT/ref26"
: > "$OUT/rows26.tsv"

mapfile -t FILES < "$LIST"

words_of() {
  pdftotext "$1" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for w in t if any(c.isalnum() for c in w)), len(t),
      sum(1 for c in b if c.isalnum()))'
}

one() {
  local idx="$1" i=-1 rel f base ext stem id r rp rw rwraw rg
  local prof="$OUT/p26_$idx"
  mkdir -p "$prof" "$OUT/w26_$idx"
  for rel in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    f="$ROOT_DIR/$rel"
    base="$(basename "$f")"; ext="${base##*.}"; stem="${base%.*}"
    id="${stem}__${ext,,}"
    r="$OUT/ref26/$id.pdf"

    if [ ! -f "$r" ]; then
      rm -rf "${OUT:?}/w26_$idx"; mkdir -p "$OUT/w26_$idx"
      timeout 600 "$SOFFICE" -env:UserInstallation="file://$prof" \
        --headless --convert-to pdf --outdir "$OUT/w26_$idx" "$f" >/dev/null 2>&1
      [ -f "$OUT/w26_$idx/$stem.pdf" ] && mv -f "$OUT/w26_$idx/$stem.pdf" "$r"
    fi

    rp="-"; rw="-"; rwraw="-"; rg="-"
    if [ -f "$r" ]; then
      rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
      read -r rw rwraw rg < <(words_of "$r")
    fi
    printf "%s\t%s\t%s\t%s\t%s\t%s\n" "$id" "${rp:--}" "$rw" "$rwraw" "$rg" "$rel" \
      >> "$OUT/rows26.tsv"
  done
}

for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait

echo "ROWS $(wc -l < "$OUT/rows26.tsv") of ${#FILES[@]}"
