#!/usr/bin/env bash
set -u
W=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61
export PROBE_STRINGS="$1"
export PROBE_OUT="$2"
export PROBE_COLS="${3:-6200}"
export PROBE_SIZE="${4:-11}"
export PROBE_FONT="${5:-Calibri}"
cd "$W/dotnet" || exit 1
timeout 1800 dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
  --no-build --filter "FullyQualifiedName~RowHeightProbe.Strings" 2>&1 | tail -3
cat "$PROBE_OUT"
