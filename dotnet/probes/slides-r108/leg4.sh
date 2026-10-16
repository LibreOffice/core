#!/usr/bin/env bash
# Render one document's flat ODP through 26.2.4.2 and through our CLI, and the .ppt through ours.
set -uo pipefail
SO=/opt/libreoffice26.2/program/soffice
CLI=/home/user/wt-slidesize/dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli
W=/home/user/r108-work/leg4
stem="$1"; src="$2"
d="$W/$stem"; mkdir -p "$d/u" "$d/rt" "$d/ours-fodp" "$d/ours-ppt"
cp "/home/user/r108-work/fodp/$stem.fodp" "$d/in.fodp"
cp "/home/user/sample-files/$src" "$d/in.ppt"
export HOME="$d/u" SOURCE_DATE_EPOCH=1700000000
timeout -k 30 600 "$SO" -env:UserInstallation="file://$d/u/x" --headless --convert-to pdf --outdir "$d/rt" "$d/in.fodp" >/dev/null 2>&1
timeout -k 30 600 "$CLI" render "$d/in.fodp" --format pdf --outdir "$d/ours-fodp" >/dev/null 2>&1
timeout -k 30 600 "$CLI" render "$d/in.ppt"  --format pdf --outdir "$d/ours-ppt"  >/dev/null 2>&1
echo "$stem rt=$([ -f $d/rt/in.pdf ] && echo y || echo N) of=$([ -f $d/ours-fodp/in.pdf ] && echo y || echo N) op=$([ -f $d/ours-ppt/in.pdf ] && echo y || echo N)"
