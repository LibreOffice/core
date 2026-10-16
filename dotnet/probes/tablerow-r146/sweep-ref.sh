#!/bin/bash
# Render every words-track document through 26.2.4.2 into refpdf/, one profile per worker,
# `timeout -k 30 900` as CLAUDE.md requires.  Each document is checked for a non-empty PDF.
set -u
REF=/opt/libreoffice26.2/program/soffice
ROOT=/home/user/sample-files
HERE=/home/user/libreoffice-core/dotnet/probes/tablerow-r146
OUT=$HERE/refpdf
mkdir -p "$OUT"
worker() {
  local id=$1
  local prof=/tmp/lo-r146-w$id
  mkdir -p "$prof"
  while read -r rel; do
    local base; base=$(basename "$rel"); base="${base%.*}"
    local d="$OUT/$id"
    mkdir -p "$d"
    [ -s "$d/$base.pdf" ] && continue
    timeout -k 30 900 "$REF" --headless --norestore \
      -env:UserInstallation=file://"$prof" \
      --convert-to pdf --outdir "$d" "$ROOT/$rel" >/dev/null 2>&1
    if [ -s "$d/$base.pdf" ]; then echo -e "ok\t$rel\t$d/$base.pdf"
    else echo -e "FAILED\t$rel\t-"; fi
  done
}
N=3
for i in $(seq 0 $((N-1))); do
  awk -v n=$N -v i=$i 'NR % n == i' "$HERE/words-paths.txt" | worker $i > "$HERE/sweep-$i.tsv" &
done
wait
cat "$HERE"/sweep-*.tsv > "$HERE/sweep.tsv"
echo "done: $(grep -c '^ok' "$HERE/sweep.tsv") ok, $(grep -c '^FAILED' "$HERE/sweep.tsv") failed"
