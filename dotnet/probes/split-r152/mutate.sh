#!/bin/sh
# Mutation pin for O83. Three arms, one per error the round removed -- the three are separable and
# each was worth a line or a rule on its own.
#
# `run` greps for compiler errors as well as test results: round 150 lost an arm to a mutation that
# did not compile, which prints NOTHING and reads exactly like an arm that is not pinned.
# Restores with `cp` + `touch`, never `mv` and never `git checkout` alone.
cd "$(dirname "$0")/../.."
T=src/XRay.Office.WordProcessing/Layout/TableLayouter.cs
cp "$T" /tmp/r152.tl
restore() { cp /tmp/r152.tl "$T"; touch "$T"; }
trap restore EXIT

run() {
    printf '\n=== %s ===\n' "$1"
    dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter 'FullyQualifiedName~TableSplitBorder' 2>&1 \
        | grep -E '^(Passed!|Failed!|  Failed |.*error [A-Z]+[0-9]+)' || echo 'NO OUTPUT'
    restore
}

sed -i 's/^        Length border = BottomBand(row);/        Length border = TopBand(null, row);/' "$T"
run "M1 a part is charged its own TOP rule, as it was before this round"

sed -i 's/^            Length needed = bandAbove$/            Length needed = Length.Zero/' "$T"
run "M2 a part does not charge the boundary band above it"

# A runtime condition that is never true, not `if (false)`: the compiler detects the latter as
# unreachable code and TreatWarningsAsErrors turns that into a build failure.
sed -i 's/^        if (complete) height -= border;/        if (complete \&\& room < Length.Zero) height -= border;/' "$T"
run "M3 a part that finishes its row still charges a band at its foot"
