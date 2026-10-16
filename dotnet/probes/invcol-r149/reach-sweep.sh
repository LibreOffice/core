#!/usr/bin/env bash
# Render one list of .ods both ways. Two workers only: the parent session is
# building in this tree and a reference render starved of CPU fails on the
# REFERENCE side, which reads exactly like our regression (dotnet/CLAUDE.md).
set -u
LIST="$1"; mkdir -p "$2"; OUT="$(cd "$2" && pwd)"; W=2
# Absolute, always. soffice takes its profile as file://$OUT/profN, and a RELATIVE
# one does NOT fail -- it HANGS. Measured here: two workers sat 167 s on their first
# document with nothing written and no error. (batch-check.sh:56 says the same.)
CLI=/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
REF=/opt/libreoffice26.2/program/soffice
mkdir -p "$OUT/ours" "$OUT/ref"
stat -c "CLI %y %s" "$CLI" > "$OUT/cli-mtime.txt"
"$REF" --version > "$OUT/ref-version.txt" 2>&1
run() {
  local idx=$1 i=0
  while IFS= read -r f; do
    i=$((i+1)); [ $((i % W)) -eq "$idx" ] || continue
    local base stem; base="$(basename "$f")"; stem="${base%.*}"
    [ -f "$OUT/ours/$stem.pdf" ] || {
      rm -rf "$OUT/to$idx"; mkdir -p "$OUT/to$idx"
      SOURCE_DATE_EPOCH=0 timeout -k 30 600 "$CLI" render "$f" --format pdf --outdir "$OUT/to$idx" >/dev/null 2>&1
      [ -f "$OUT/to$idx/$stem.pdf" ] && mv -f "$OUT/to$idx/$stem.pdf" "$OUT/ours/$stem.pdf"; }
    [ -f "$OUT/ref/$stem.pdf" ] || {
      rm -rf "$OUT/tr$idx"; mkdir -p "$OUT/tr$idx"
      timeout -k 30 600 "$REF" -env:UserInstallation="file://$OUT/prof$idx" \
        --headless --convert-to pdf --outdir "$OUT/tr$idx" "$f" >/dev/null 2>&1
      [ -f "$OUT/tr$idx/$stem.pdf" ] && mv -f "$OUT/tr$idx/$stem.pdf" "$OUT/ref/$stem.pdf"; }
  done < "$LIST"
}
for k in $(seq 0 $((W-1))); do run "$k" & done
wait
stat -c "CLI %y %s" "$CLI" >> "$OUT/cli-mtime.txt"
echo done
