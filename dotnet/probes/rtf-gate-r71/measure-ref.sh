#!/usr/bin/env bash
# Measure the gate columns of a directory of reference PDFs named <stem>__rtf.pdf.
#   measure-ref.sh <dir-with-ref/> <corpus-root>  > ref.tsv
set -uo pipefail
D="${1:?dir}"; ROOT="${2:-/home/user/corpus-odf}"
for f in "$D"/ref/*.pdf; do
  id="$(basename "$f" .pdf)"; stem="${id%__rtf}"
  rel="$(cd "$ROOT" && find . -name "$stem.rtf" -type f | head -1)"; rel="${rel#./}"
  p=$(pdfinfo "$f" 2>/dev/null | awk '/^Pages/{print $2}')
  read -r w raw g < <(pdftotext "$f" - 2>/dev/null | python3 -c '
import sys
b = sys.stdin.buffer.read().decode("utf-8", "replace")
t = b.split()
print(sum(1 for x in t if any(c.isalnum() for c in x)), len(t),
      sum(1 for c in b if c.isalnum()))')
  fo=$(pdffonts "$f" 2>/dev/null | tail -n +3 | grep -c .)
  printf '%s\trtf\t-/%s\t-/%s\t-/%s\t0\t-\t-/%s\t-/%s\n' "$rel" "$p" "$w" "$fo" "$raw" "$g"
done
