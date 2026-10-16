#!/usr/bin/env bash
# flat-ODP round trip at 26.2.4.2: ppt -> fodp -> pdf, and ppt -> pdf directly.
set -uo pipefail
SO=/opt/libreoffice26.2/program/soffice
W=/home/user/r108-work/rt
mkdir -p "$W"
f="$1"
base="$(basename "$f")"; stem="${base%.ppt}"
d="$W/$stem"; rm -rf "$d"; mkdir -p "$d/p"
cp "/home/user/sample-files/$f" "$d/in.ppt"
export HOME="$d/p"
"$SO" -env:UserInstallation="file://$d/p/u" --headless --convert-to fodp --outdir "$d" "$d/in.ppt" >/dev/null 2>&1
"$SO" -env:UserInstallation="file://$d/p/u" --headless --convert-to pdf --outdir "$d/direct" "$d/in.ppt" >/dev/null 2>&1
if [ -f "$d/in.fodp" ]; then
  "$SO" -env:UserInstallation="file://$d/p/u" --headless --convert-to pdf --outdir "$d/rt" "$d/in.fodp" >/dev/null 2>&1
fi
echo "$stem  fodp=$( [ -f "$d/in.fodp" ] && echo yes || echo NO )  direct=$( [ -f "$d/direct/in.pdf" ] && echo yes || echo NO )  rt=$( [ -f "$d/rt/in.pdf" ] && echo yes || echo NO )"
