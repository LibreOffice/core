#!/usr/bin/env bash
# Render every band probe with ours, and export LibreOffice's own odp so its computed
# plot rectangle can be read out of chart:coordinate-region rather than inferred.
set -uo pipefail
SP=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/slides-r30
D="${1:-$SP/band}"
mkdir -p "$D/ours" "$D/ref" "$D/odp"
for f in "$D"/*.pptx; do
  b=$(basename "$f" .pptx)
  SOURCE_DATE_EPOCH=1700000000 "$SP/cli-base/Paperless.Cli" render "$f" --format pdf --outdir "$D/ours" >/dev/null 2>&1
  timeout 300 soffice --headless -env:UserInstallation=file://$SP/prof-band --convert-to pdf --outdir "$D/ref" "$f" >/dev/null 2>&1
  timeout 300 soffice --headless -env:UserInstallation=file://$SP/prof-band --convert-to odp --outdir "$D/odp" "$f" >/dev/null 2>&1
  echo "$b done"
done
