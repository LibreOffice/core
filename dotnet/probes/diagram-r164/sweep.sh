#!/bin/bash
# Renders both sides of the eight diagram/chart templates.
# Reference is 26.2.4.2 at /opt/libreoffice26.2 put FIRST on PATH; /usr/bin/soffice (24.2.7.2)
# is NOT the target and must never be picked up.
set -u
export PATH=/opt/libreoffice26.2/program:$PATH
HERE=$(cd "$(dirname "$0")" && pwd)
CORPUS=/home/user/sample-files
CLI=/home/user/cli-frozen-r164/XRay.Office.Cli
mkdir -p "$HERE/ref" "$HERE/ours" "$HERE/profile"
echo "soffice resolved: $(command -v soffice)  ->  $(soffice --version 2>&1 | head -1)"
while IFS=$'\t' read -r stem rel; do
  src="$CORPUS/$rel"
  # own outdir per conversion: --convert-to names output by stem only and silently overwrites
  mkdir -p "$HERE/ref/$stem"
  if [ ! -f "$HERE/ref/$stem/$stem.pdf" ]; then
    soffice --headless --norestore \
      -env:UserInstallation="file://$HERE/profile/$stem" \
      --convert-to pdf --outdir "$HERE/ref/$stem" "$src" >/dev/null 2>&1
  fi
  mkdir -p "$HERE/ours/$stem"
  if [ ! -f "$HERE/ours/$stem/$stem.pdf" ]; then
    "$CLI" render --format pdf --outdir "$HERE/ours/$stem" "$src" >/dev/null 2>&1
  fi
  printf '%s\tref:%s\tours:%s\n' "$stem" \
    "$([ -f "$HERE/ref/$stem/$stem.pdf" ] && echo yes || echo NO)" \
    "$([ -f "$HERE/ours/$stem/$stem.pdf" ] && echo yes || echo NO)"
done < "$HERE/docs.tsv"
