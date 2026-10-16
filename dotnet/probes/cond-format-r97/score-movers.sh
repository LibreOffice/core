#!/bin/bash
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
for b in "$@"; do
  r="/home/user/gate-orig-r83/ref/${b}__xlsx.pdf"
  printf '%s\n' "$b"
  for leg in base after; do
    o="/home/user/cfdraw-r97/sweep-$leg/ours/${b}__xlsx.pdf"
    w=$(mktemp -d)
    printf '  %-6s ' "$leg"
    python3 "$DIFF" "$o" "$r" --outdir "$w" 2>&1 \
      | awk -F'\t' 'NF>=6 && $1+0==$1 {s+=$4; n++; if($6 ~ /MAJOR/) m++}
                    END{printf "pages %4d  sum|ink|%% %8.2f  MAJOR %d\n", n, s, m+0}'
    rm -rf "$w"
  done
done
