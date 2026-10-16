#!/usr/bin/env bash
# score.sh <ours-dir> <ref-dir> <out.tsv>   — batch-check's three checks over PDFs already on disk
set -uo pipefail
O="$1"; R="$2"; T="$3"; : > "$T"
for o in "$O"/*.pdf; do
  id="$(basename "$o" .pdf)"; r="$R/$id.pdf"
  [ -f "$r" ] || { echo -e "$id\t-\t-\tno-ref" >> "$T"; continue; }
  op=$(pdfinfo "$o" 2>/dev/null | awk '/^Pages/{print $2}')
  rp=$(pdfinfo "$r" 2>/dev/null | awk '/^Pages/{print $2}')
  ow=$(pdftotext "$o" - 2>/dev/null | wc -w)
  rw=$(pdftotext "$r" - 2>/dev/null | wc -w)
  un=$(pdffonts "$o" 2>/dev/null | tail -n +3 | awk 'NF>=8 && $(NF-4)=="no"' | wc -l)
  v=""
  [ "$op" = "$rp" ] || v="pages"
  if [ "$rw" -gt 0 ] 2>/dev/null; then
    awk -v a="$ow" -v b="$rw" 'BEGIN{d=(a>b?a-b:b-a); exit !(d > b*0.02 && d > 3)}' && v="${v:+$v,}words"
  elif [ "${ow:-0}" -gt 3 ]; then v="${v:+$v,}words"; fi
  [ "${un:-0}" = "0" ] || v="${v:+$v,}unembedded"
  [ -n "$v" ] || v="match"
  echo -e "$id\t$op/$rp\t$ow/$rw\t$v" >> "$T"
done
awk -F'\t' '{split($2,p,"/"); split($3,w,"/"); pe+=(p[1]>p[2]?p[1]-p[2]:p[2]-p[1]); if(p[1]==p[2])ex++; we+=(w[1]>w[2]?w[1]-w[2]:w[2]-w[1]); if($4=="match")m++} END{print "rows",NR,"match",m,"pageerr",pe,"exactpages",ex,"worderr",we}' "$T"
