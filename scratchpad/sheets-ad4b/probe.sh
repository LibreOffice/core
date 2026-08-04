#!/usr/bin/env bash
set -u
W=/home/user/libreoffice-core/.claude/worktrees/agent-ad4ba149aefc94b61
export PROBE_WORKBOOK="$1"
export PROBE_OUT="$2"
export PROBE_ROWS="${3:-}"
export PROBE_SCALES="${4:-}"
cd "$W/dotnet" || exit 1
timeout 3600 dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
  --no-build --filter "FullyQualifiedName~RowHeightProbe" 2>&1 | tail -4
