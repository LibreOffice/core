#!/usr/bin/env bash
set -u
D=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b
mkdir -p "$D/rt" "$D/prof-rt"
timeout 1800 soffice -env:UserInstallation="file://$D/prof-rt" \
  --headless --convert-to fods --outdir "$D/rt" "$1"
ls -la "$D/rt"
