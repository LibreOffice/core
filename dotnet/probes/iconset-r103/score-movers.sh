#!/bin/bash
# Summed unsigned ink per document against the banked 26.2.4.2 reference, both legs.
# Usage: score-movers.sh <identity-without-__xlsx> ...
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
for b in "$@"; do
  r="/home/user/gate-orig-r83/ref/${b}__xlsx.pdf"
  printf '%s\n' "$b"
  for leg in base after; do
    o="/home/user/r103-work/out-$leg/${b}/${b}.pdf"
    w=$(mktemp -d /home/user/r103-work/tmp.XXXXXX)
    printf '  %-6s ' "$leg"
    python3 "$DIFF" "$o" "$r" --outdir "$w" 2>&1 \
      | awk -F'\t' 'NF>=6 && $1+0==$1 {s+=$4; n++; if($6 ~ /MAJOR/) m++}
                    END{printf "pages %4d  sum|ink|%% %8.3f  MAJOR %d\n", n, s, m+0}'
    rm -rf "$w"
  done
done
