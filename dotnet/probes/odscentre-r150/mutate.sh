#!/bin/sh
# Mutation pin for the O100 fix. Restores with `cp` + `touch`, never `mv`: `mv` keeps the old
# mtime and MSBuild then SKIPS the project while reporting `0 Error(s)`, so the binary still
# carries the experiment. `dotnet/CLAUDE.md` records three builds surviving that way.
set -e
cd "$(dirname "$0")/../.."
SRC=src/XRay.Office.Spreadsheets/OpenDocument/OdsPrintSetup.cs
cp "$SRC" /tmp/r150.ods
restore() { cp /tmp/r150.ods "$SRC"; touch "$SRC"; }
trap restore EXIT

run() {
    printf '\n=== %s ===\n' "$1"
    dotnet test tests/XRay.Office.Spreadsheets.Tests/XRay.Office.Spreadsheets.Tests.csproj \
        --filter 'FullyQualifiedName~SheetOdfPrintCentring' 2>&1 \
        | grep -E '^(Passed!|Failed!|  Failed )' || true
    restore
}

sed -i 's/"table-centering"/"table-centring"/' "$SRC"
run "M1 the reader asks for the British spelling again"

sed -i 's/centring is "horizontal" or "both"/centring is "horizontal"/' "$SRC"
run "M2 the horizontal arm does not accept the value both"

sed -i 's/centring is "vertical" or "both"/centring is "vertical"/' "$SRC"
run "M3 the vertical arm does not accept the value both"
