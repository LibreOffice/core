#!/usr/bin/env bash
F=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b/ink-dev/summary.tsv
until [ "$(wc -l < "$F")" -ge "$1" ]; do sleep 30; done
echo "done: $(wc -l < "$F") rows"
