#!/bin/bash
# Summed unsigned ink for one identity, our two legs against the banked 26.2.4.2 reference.
#
# Usage: score-witnesses.sh <identity-without-__xlsx> ...
# Expects <leg>/<identity>/*.pdf under the working directory for leg in {base, after}.
DIFF=/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py
for b in "$@"; do
  r=/home/user/gate-orig-r83/ref/${b}__xlsx.pdf
  for leg in base after; do
    o=$(ls "$leg/$b"/*.pdf 2>/dev/null | head -1)
    w=$(mktemp -d)
    printf '%-6s %s\n' "$leg" "$b"
    python3 "$DIFF" "$o" "$r" --outdir "$w" 2>&1 \
      | awk -F'\t' 'NF>=6 && $1+0==$1 {s+=$4; n++; if($6 ~ /MAJOR/) m++}
                    END{printf "    pages %d  sum|ink|%% %.2f  MAJOR %d\n", n, s, m}'
    rm -rf "$w"
  done
done
