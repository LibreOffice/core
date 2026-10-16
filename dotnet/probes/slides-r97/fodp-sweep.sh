#!/usr/bin/env bash
# Convert every .ppt in the corpus to flat ODP with 26.2.4.2 — the reference's own resolved view.
set -uo pipefail
OUT=/home/user/r97-slides/fodp-all
mkdir -p "$OUT"
mapfile -t FILES < <(git -C /home/user/sample-files ls-files slides | grep -Ei '\.ppt$' | sed 's|^|/home/user/sample-files/|')
W=3
one() {
  local idx=$1 i=-1 f
  for f in "${FILES[@]}"; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    b="$(basename "$f" .ppt)"
    [ -f "$OUT/$b.fodp" ] && continue
    timeout -k 20 180 /opt/libreoffice26.2/program/soffice --headless --norestore \
      -env:UserInstallation=file:///home/user/r97-slides/lu$idx \
      --convert-to fodp --outdir "$OUT" "$f" >/dev/null 2>&1
  done
}
for w in $(seq 0 $((W-1))); do one $w & done
wait
ls "$OUT" | wc -l
