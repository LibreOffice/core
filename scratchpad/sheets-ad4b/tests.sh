#!/usr/bin/env bash
set -u
W=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61
OUT="$W/scratchpad/sheets-ad4b/tests"
mkdir -p "$OUT"
cd "$W/dotnet" || exit 1
for p in "$@"; do
  timeout 3600 dotnet test "tests/Paperless.$p.Tests/Paperless.$p.Tests.csproj" \
    > "$OUT/$p.txt" 2>&1
  printf '%-18s %s\n' "$p" "$(grep -oP 'Failed:\s+\d+, Passed:\s+\d+' "$OUT/$p.txt" | tail -1)"
done
