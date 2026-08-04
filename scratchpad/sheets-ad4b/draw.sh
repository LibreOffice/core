#!/usr/bin/env bash
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
NAME="$1"; SIZE="$2"; FONT="$3"; STRINGS="$4"
python3 "$D/make-draw.py" "$D/$NAME.fods" "$SIZE" "$FONT" "$STRINGS" || exit 1
rm -f "$D/rt2/$NAME.pdf"
timeout 900 soffice -env:UserInstallation="file://$D/prof-rt" \
  --headless --convert-to pdf --outdir "$D/rt2" "$D/$NAME.fods" > /dev/null 2>&1
[ -f "$D/rt2/$NAME.pdf" ] || { echo "convert failed"; exit 1; }
pdftotext -bbox "$D/rt2/$NAME.pdf" "$D/$NAME.html"
grep -o '<word xMin="[0-9.]*" yMin="[0-9.]*" xMax="[0-9.]*"[^>]*>[^<]*' "$D/$NAME.html" | head -20
pdffonts "$D/rt2/$NAME.pdf"
