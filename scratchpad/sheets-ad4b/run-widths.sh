#!/usr/bin/env bash
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
NAME="$1"; SIZE="$2"; FONT="$3"; STRINGS="$4"; WIDTHS=$(cat "$5")
python3 "$D/make-widths.py" "$D/$NAME.fods" "$SIZE" "$FONT" "$STRINGS" "$WIDTHS" || exit 1
rm -f "$D/rt2/$NAME.fods"
timeout 1800 soffice -env:UserInstallation="file://$D/prof-rt" \
  --headless --convert-to fods --outdir "$D/rt2" "$D/$NAME.fods" > /dev/null 2>&1
[ -f "$D/rt2/$NAME.fods" ] || { echo "convert failed"; exit 1; }
python3 "$D/rowheights.py" "$D/rt2/$NAME.fods" > "$D/$NAME-rows.tsv"
python3 "$D/lines.py" "$D/$NAME-rows.tsv"
