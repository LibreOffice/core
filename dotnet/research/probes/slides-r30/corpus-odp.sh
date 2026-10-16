#!/usr/bin/env bash
# Export every corpus deck that carries a chart part to odp, so LibreOffice's own computed
# plot rectangle can be read out of chart:coordinate-region.
set -uo pipefail
SP=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/slides-r30
mkdir -p "$SP/corpus-odp"
while read -r rel; do
  f="/workspace/sample-files/slides/$rel"
  timeout 600 soffice --headless -env:UserInstallation=file://$SP/prof-corpus \
      --convert-to odp --outdir "$SP/corpus-odp" "$f" >/dev/null 2>&1
  echo "$(basename "$f")"
done < "$SP/chartdocs.txt"
