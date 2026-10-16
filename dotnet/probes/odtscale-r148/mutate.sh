#!/bin/sh
# Mutation pin for round 148. Each arm reverts one line of the round's diff, rebuilds, runs the
# ODT scale tests, and restores the source with `cp` + `touch` -- never `mv`, which keeps the old
# mtime and lets MSBuild skip the project (dotnet/CLAUDE.md records that trap).
set -e
cd "$(dirname "$0")/../.."

FMT=src/XRay.Office.WordProcessing/OpenDocument/OdfParagraphFormats.cs
SRC=src/XRay.Office.WordProcessing/OpenDocument/OdtLayoutSource.cs
WS=src/XRay.Office.Text/Layout/TextWidthScale.cs

cp "$FMT" /tmp/r148.fmt; cp "$SRC" /tmp/r148.src; cp "$WS" /tmp/r148.ws

restore() {
    cp /tmp/r148.fmt "$FMT"; touch "$FMT"
    cp /tmp/r148.src "$SRC"; touch "$SRC"
    cp /tmp/r148.ws  "$WS";  touch "$WS"
}
trap restore EXIT

run() {
    printf '\n=== %s ===\n' "$1"
    dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter 'FullyQualifiedName~OdtCharacterScale' 2>&1 | grep -E '^(Passed!|Failed!|  Failed )' || true
    restore
}

sed -i 's/^        string? stated = Cascaded(styles, cascade, OdfNamespaces.Style, "text-scale").Value;/        string? stated = null;/' "$FMT"
run "M1 ScaleIn never reads style:text-scale"

# The guard below it already returns the bare percentage for a gridless em, so this is the
# "no twip grid" arm with no unreachable code for TreatWarningsAsErrors to reject.
sed -i 's#^        long height = emSize.Twips;#        long height = 0;#' "$WS"
run "M2 the scale is the stated percentage, not the twip grid"

sed -i 's#^                || style.WidthPerCent != paragraph.WidthPerCent#                || false#' "$SRC"
run "M3 the varies-predicate does not list WidthPerCent"

sed -i 's#^                WidthPerCent: style.WidthPerCent));#                WidthPerCent: 100));#' "$SRC"
run "M4 PageRun is built without the run's own WidthPerCent"
