#!/usr/bin/env bash
# pagehead.sh <ours.pdf> <ref.pdf> <npages>
O="$1"; R="$2"; N="${3:-10}"
for p in $(seq 1 "$N"); do
  o=$(pdftotext -f "$p" -l "$p" "$O" - 2>/dev/null | tr -s ' \n' '  ' | head -c 70)
  r=$(pdftotext -f "$p" -l "$p" "$R" - 2>/dev/null | tr -s ' \n' '  ' | head -c 70)
  echo "p$p O: $o"
  echo "p$p R: $r"
  echo
done
