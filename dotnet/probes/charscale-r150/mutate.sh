#!/bin/sh
# Mutation pin for O101's WW8 and RTF thirds. Six arms: the value, the adjacent-run MERGE and the
# uniform-paragraph varies-predicate, for each reader. The merge arm is the one a one-line probe
# cannot see -- WW8 builds its runs a character at a time, so a scaled stretch missing from the
# comparison is absorbed into the unscaled run beside it before the shortcut is ever asked.
#
# Restores with `cp` + `touch`, never `mv` and never `git checkout` alone: either leaves the source
# older than the assembly and MSBuild then SKIPS the project while reporting `0 Error(s)`. Round 150
# lost two "base" builds to exactly that.
cd "$(dirname "$0")/../.."
W=src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Layout.cs
D=src/XRay.Office.WordProcessing/Ww8/DocReader.cs
R=src/XRay.Office.WordProcessing/Rtf/RtfDocumentReader.cs
L=src/XRay.Office.WordProcessing/Rtf/RtfLayoutParagraph.cs
T=src/XRay.Office.WordProcessing/Rtf/RtfReader.cs

for f in "$W" "$D" "$R" "$L" "$T"; do cp "$f" "/tmp/r150m.$(basename $f)"; done
restore() {
    for f in "$W" "$D" "$R" "$L" "$T"; do cp "/tmp/r150m.$(basename $f)" "$f"; touch "$f"; done
}
trap restore EXIT

run() {
    printf '\n=== %s ===\n' "$1"
    dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter 'FullyQualifiedName~CharacterScaleFormats' 2>&1 \
        | grep -E '^(Passed!|Failed!|  Failed |.*error [A-Z]+[0-9]+)' || echo 'NO OUTPUT'
    restore
}

sed -i 's/^                    format = format with { CharacterScale = sprm.Word };/                    format = format with { CharacterScale = null };/' "$W"
run "M1 WW8 never reads sprmCCharScale"

sed -i 's/^           \&\& a.WidthPerCent == b.WidthPerCent;/           ;/' "$W"
run "M2 WW8's adjacent-run merge does not compare the width"

sed -i 's/^                || run.WidthPerCent != paragraph.WidthPerCent/                || false/' "$D"
run "M3 WW8's varies-predicate does not list the width"

# An empty range rather than deleting the statement: one line, still valid C#, and every value
# then falls through to Natural. The first cut produced code that did not compile, and a build
# failure in a mutation arm prints NOTHING and reads exactly like an arm that is not pinned.
# `? scale` -> `? Natural`, not an empty range: `and >= 601 and <= 600` is an impossible
# relational pattern, the compiler warns, and TreatWarningsAsErrors turns that into a build
# failure -- which printed NOTHING and read exactly like an arm that is not pinned. `run` now
# greps for compiler errors as well, so a broken arm can never look like a green one again.
sed -i 's|^                    ? scale$|                    ? TextWidthScale.Natural|' "$R"
run "M4 RTF never reads charscalex"

sed -i 's/^           \&\& WidthPerCent == other.WidthPerCent;/           ;/' "$L"
run "M5 RTF's adjacent-run merge does not compare the width"

sed -i 's/^                || run.WidthPerCent != paragraph.WidthPerCent/                || false/' "$T"
run "M6 RTF's varies-predicate does not list the width"
