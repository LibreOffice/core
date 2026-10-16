#!/usr/bin/env bash
# Render every probe both ways. One `soffice` profile per document, keyed on an md5 of its
# own path, because `-env:UserInstallation=file://…` truncates at the first space.
#
#   XRAY_OFFICE_CLI=… render.sh <probe-dir> <outdir>
set -uo pipefail
IN="${1:?probe dir}"; OUT="${2:?outdir}"
CLI="${XRAY_OFFICE_CLI:?set XRAY_OFFICE_CLI}"
REF="${REF_SOFFICE:-/opt/libreoffice26.2/program/soffice}"
mkdir -p "$OUT/ref" "$OUT/ours"
for f in "$IN"/*.rtf; do
  d=$(printf '%s' "$f" | md5sum | cut -c1-16)
  "$REF" --headless -env:UserInstallation="file://${TMPDIR:-/tmp}/lo-$d" \
      --convert-to pdf --outdir "$OUT/ref" "$f" >/dev/null 2>&1
  "$CLI" render "$f" --outdir "$OUT/ours" >/dev/null 2>&1
done
# Assert the instrument produced output before anything compares it.
n=$(ls "$IN"/*.rtf | wc -l); r=$(ls "$OUT/ref"/*.pdf 2>/dev/null | wc -l); o=$(ls "$OUT/ours"/*.pdf 2>/dev/null | wc -l)
echo "probes=$n ref=$r ours=$o"
[ "$r" -eq "$n" ] && [ "$o" -eq "$n" ] || { echo "MISSING OUTPUT — do not compare" >&2; exit 1; }
