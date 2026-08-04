#!/usr/bin/env bash
F=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61/scratchpad/sheets-ad4b/$1/rows.tsv
until [ "$(wc -l < "$F")" -ge 171 ]; do sleep 15; done
echo "done: $(wc -l < "$F") rows"
