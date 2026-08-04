#!/usr/bin/env bash
# Open a probe .fods with Paperless, dump its row heights, and score them against
# LibreOffice's own round-trip of the same file.
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
NAME="$1"
bash "$D/probe.sh" "$D/$NAME.fods" "$D/$NAME-ours.tsv" > /dev/null 2>&1
python3 "$D/rowheights.py" "$D/rt2/$NAME.fods" > "$D/$NAME-lo.tsv"
echo "== $NAME"
python3 "$D/compare-rows.py" "$D/$NAME-lo.tsv" "$D/$NAME-ours.tsv" > "$D/$NAME.score"
head -3 "$D/$NAME.score"
grep -A 8 'worst 40' "$D/$NAME.score" | head -9
