#!/usr/bin/env bash
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
mkdir -p "$D/rt2" "$D/prof-rt"
timeout 1800 soffice -env:UserInstallation="file://$D/prof-rt" \
  --headless --convert-to fods --outdir "$D/rt2" "$1" 2>&1 | tail -2
ls -la "$D/rt2"
