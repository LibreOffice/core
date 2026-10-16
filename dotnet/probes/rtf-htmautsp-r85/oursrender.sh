#!/usr/bin/env bash
# Render every probe through XRay.Office into <outdir>/ours, beside the reference
# half `refrender.sh` wrote, so the two can be compared without re-rendering the
# reference (nothing in `dotnet/src` can reach `soffice`).
#
#   XRAY_OFFICE_CLI=… oursrender.sh <probe-dir> <outdir>
set -uo pipefail
IN="${1:?probe dir}"; OUT="${2:?outdir}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
mkdir -p "$OUT/ours"
for f in "$IN"/*.rtf; do
  "$CLI" render "$f" --format pdf --outdir "$OUT/ours" >/dev/null 2>&1
done
n=$(ls "$IN"/*.rtf | wc -l); o=$(ls "$OUT/ours"/*.pdf 2>/dev/null | wc -l)
echo "probes=$n ours=$o"
[ "$o" -eq "$n" ] || { echo "MISSING OUTPUT — do not compare" >&2; exit 1; }
