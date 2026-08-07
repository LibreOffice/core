#!/usr/bin/env bash
# pagewords.sh <ours.pdf> <ref.pdf>
O="$1"; R="$2"
no=$(pdfinfo "$O" | awk '/^Pages/{print $2}')
nr=$(pdfinfo "$R" | awk '/^Pages/{print $2}')
echo "pages ours=$no ref=$nr"
printf "%4s %8s %8s %-40s %-40s\n" pg ourW refW "ours-tail" "ref-tail"
m=$no; [ "$nr" -gt "$m" ] && m=$nr
for p in $(seq 1 "$m"); do
  ow=$(pdftotext -f "$p" -l "$p" "$O" - 2>/dev/null | wc -w)
  rw=$(pdftotext -f "$p" -l "$p" "$R" - 2>/dev/null | wc -w)
  ot=$(pdftotext -f "$p" -l "$p" "$O" - 2>/dev/null | tr -s ' \n' '  ' | tail -c 40)
  rt=$(pdftotext -f "$p" -l "$p" "$R" - 2>/dev/null | tr -s ' \n' '  ' | tail -c 40)
  printf "%4s %8s %8s %-40s %-40s\n" "$p" "$ow" "$rw" "$ot" "$rt"
done
