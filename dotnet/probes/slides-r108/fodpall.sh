#!/usr/bin/env bash
set -uo pipefail
SO=/opt/libreoffice26.2/program/soffice
W=/home/user/r108-work/fodp
mkdir -p "$W/u"
export HOME="$W/u"
while IFS= read -r f; do
  [ -n "$f" ] || continue
  base="$(basename "$f")"; stem="${base%.ppt}"
  [ -f "$W/$stem.fodp" ] && continue
  mkdir -p "$W/t"; cp "/home/user/sample-files/$f" "$W/t/in.ppt"
  timeout -k 30 300 "$SO" -env:UserInstallation="file://$W/u/x" --headless --convert-to fodp --outdir "$W/t" "$W/t/in.ppt" >/dev/null 2>&1
  [ -f "$W/t/in.fodp" ] && mv "$W/t/in.fodp" "$W/$stem.fodp"
  rm -rf "$W/t"
done < /home/user/wt-slidesize/dotnet/probes/slides-r107/ppt.list
echo DONE; ls "$W"/*.fodp | wc -l
