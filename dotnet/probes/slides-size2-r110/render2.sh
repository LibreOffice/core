#!/usr/bin/env bash
# render2.sh <fodp> <tag>  -> prints "<tag> ref=<size>/<alnum> ours=<size>/<alnum>" for page 1 (or page N with $3)
set -uo pipefail
SO=/opt/libreoffice26.2/program/soffice
CLI=/home/user/wt-slidesize2/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli
SZ=/home/user/wt-slidesize2/dotnet/probes/slides-r107/sizes.py
f="$1"; tag="$2"; pg="${3:-1}"
d="$(dirname "$f")/out-$tag"; rm -rf "$d"; mkdir -p "$d/u" "$d/ref" "$d/ours"
export HOME="$d/u" SOURCE_DATE_EPOCH=1700000000
timeout -k 20 300 "$SO" -env:UserInstallation="file://$d/u/x" --headless --convert-to pdf --outdir "$d/ref" "$f" >/dev/null 2>&1
timeout -k 20 300 "$CLI" render "$f" --format pdf --outdir "$d/ours" >/dev/null 2>&1
b="$(basename "$f")"; b="${b%.fodp}"
r=$(python3 "$SZ" "$d/ref/$b.pdf" r 2>/dev/null | awk -v p="$pg" '$2==p{print $3"/"$4}')
o=$(python3 "$SZ" "$d/ours/$b.pdf" o 2>/dev/null | awk -v p="$pg" '$2==p{print $3"/"$4}')
echo -e "$tag\tref=${r:-FAIL}\tours=${o:-FAIL}"
