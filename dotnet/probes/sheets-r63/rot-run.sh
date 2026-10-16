#!/usr/bin/env bash
# Render every rotation deck with the *reference* only: once to .odp so its own
# chart:coordinate-region can be read, once to .pdf so the drawn label width and the
# text-layer presence of the labels can be read.  Our renderer never runs.
#
# One soffice profile per worker slot is wrong for a corpus sweep (batch-check.sh's own
# comment says so) but right here: the decks are authored, tiny and known-good, and a slot
# converts them one after another.
set -uo pipefail
D="${1:?deck dir}"
P="${2:?scratch dir}"
W="${3:-4}"
mkdir -p "$P/ref" "$P/odp"
run_slot() {
  local slot="$1"; local i=0
  mkdir -p "$P/prof$slot"
  for f in "$D"/*.pptx; do
    i=$((i+1)); [ $(( (i-1) % W )) -eq "$slot" ] || continue
    b=$(basename "$f" .pptx)
    [ -f "$P/odp/$b.odp" ] && [ -f "$P/ref/$b.pdf" ] && continue
    SOURCE_DATE_EPOCH=1700000000 TZ=UTC timeout 300 soffice --headless \
      -env:UserInstallation=file://$P/prof$slot --convert-to pdf --outdir "$P/ref" "$f" >/dev/null 2>&1
    SOURCE_DATE_EPOCH=1700000000 TZ=UTC timeout 300 soffice --headless \
      -env:UserInstallation=file://$P/prof$slot --convert-to odp --outdir "$P/odp" "$f" >/dev/null 2>&1
  done
}
export -f run_slot 2>/dev/null
for s in $(seq 0 $((W-1))); do run_slot "$s" & done
wait
echo "pdf $(ls "$P/ref" | wc -l)  odp $(ls "$P/odp" | wc -l)"
