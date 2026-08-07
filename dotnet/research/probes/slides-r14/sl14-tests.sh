#!/usr/bin/env bash
# Per project, each run redirected to its own file so a failure's detail survives.
W=/home/user/libreoffice-core/.claude/worktrees/agent-afd045030560da190
OUT=/tmp/claude-0/-home-user-libreoffice-core/3e093938-a459-5268-987b-74c5270259d6/scratchpad/sl14-tests
mkdir -p "$OUT"
for p in "$@"; do
  dotnet test "$W/dotnet/tests/Paperless.$p.Tests/Paperless.$p.Tests.csproj" --no-build \
    > "$OUT/$p.txt" 2>&1
  printf "%-16s %s\n" "$p" "$(grep -oP 'Failed:\s+\d+, Passed:\s+\d+, Skipped:\s+\d+' "$OUT/$p.txt" | tail -1)"
done
