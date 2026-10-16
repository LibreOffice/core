#!/usr/bin/env bash
# Verdict for a list of documents under two CLIs against ONE reference conversion.
#
# batch-check.sh re-runs soffice per sweep, which doubles the only expensive part when the
# question is "did my change move a verdict". Here the reference is converted once and both
# renderings are scored against it, so the answer costs one soffice run per document.
set -uo pipefail
LIST="${1:?usage: verdict-pair.sh <list> <cli-a> <cli-b> <outdir> [workers]}"
A="${2:?}"; B="${3:?}"; OUT="${4:?}"; WORKERS="${5:-2}"
export SOURCE_DATE_EPOCH=1700000000
export TZ=UTC
mkdir -p "$OUT"; OUT="$(cd "$OUT" && pwd)"
: > "$OUT/rows.tsv"
mapfile -t FILES < <(sort "$LIST")

verdict() {  # verdict <ourPdf> <refPdf>
  local o="$1" r="$2" op rp ow rw un v
  [ -f "$o" ] || { echo "ours-failed"; return; }
  [ -f "$r" ] || { echo "ref-failed"; return; }
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
  echo "${op}/${rp} ${ow}/${rw} ${v:-match}"
}

one() {
  local idx="$1" i=-1 f key
  local prof="$OUT/prof$idx"
  mkdir -p "$prof"
  for f in "${FILES[@]}"; do
    i=$((i + 1)); [ $((i % WORKERS)) -eq "$idx" ] || continue
    key=$(echo "$f" | tr '/ ' '__')
    rm -rf "$OUT/w$idx"; mkdir -p "$OUT/w$idx/a" "$OUT/w$idx/b" "$OUT/w$idx/r"
    timeout 300 "$A" render "/workspace/sample-files/$f" --format pdf --outdir "$OUT/w$idx/a" >/dev/null 2>&1
    timeout 300 "$B" render "/workspace/sample-files/$f" --format pdf --outdir "$OUT/w$idx/b" >/dev/null 2>&1
    timeout 300 soffice -env:UserInstallation="file://$prof" --headless --convert-to pdf \
      --outdir "$OUT/w$idx/r" "/workspace/sample-files/$f" >/dev/null 2>&1
    local pa pb pr
    pa=$(find "$OUT/w$idx/a" -name '*.pdf' | head -1)
    pb=$(find "$OUT/w$idx/b" -name '*.pdf' | head -1)
    pr=$(find "$OUT/w$idx/r" -name '*.pdf' | head -1)
    printf '%s\tA\t%s\tB\t%s\n' "$f" "$(verdict "${pa:-/nonexistent}" "${pr:-/nonexistent}")" \
      "$(verdict "${pb:-/nonexistent}" "${pr:-/nonexistent}")" >> "$OUT/rows.tsv"
  done
  rm -rf "$OUT/w$idx"
}
for w in $(seq 0 $((WORKERS - 1))); do one "$w" & done
wait
echo "rows $(wc -l < "$OUT/rows.tsv")"
