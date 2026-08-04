#!/usr/bin/env bash
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
LO="$1"; shift
for f in "$@"; do
  echo "== $f"
  python3 "$D/compare-rows.py" "$LO" "$f" > "$f.score"
  head -3 "$f.score"
done
