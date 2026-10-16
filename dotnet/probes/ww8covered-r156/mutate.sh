#!/bin/sh
# Mutation pin for Ww8CoveredCellBordersTests: the one line that gives a covered cell the master's
# borders, and two rival rules for it. The harness greps for compiler errors, because an arm that does
# not compile prints no failing test and reads exactly like one that passed.
set -u
cd /home/user/libreoffice-core/dotnet || exit 1
SRC=src/XRay.Office.WordProcessing/Ww8/Ww8DocumentReader.Tables.cs
cp "$SRC" /tmp/claude-0/Ww8Tables.cs.before || exit 1

run() {
    name=$1; new=$2
    python3 - "$SRC" 'cell.Borders = owner.Borders;' "$new" <<'PY'
import sys
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p).read()
assert old in s, 'base text not found'
open(p, 'w').write(s.replace(old, new, 1))
PY
    touch "$SRC"
    out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
            --filter "FullyQualifiedName~Ww8CoveredCellBordersTests" 2>&1)
    if printf '%s' "$out" | grep -qE 'error [A-Z]+[0-9]+'; then
        echo "$name: DID NOT COMPILE -- arm void"
        printf '%s' "$out" | grep -E 'error [A-Z]+[0-9]+' | head -2
    else
        printf '%-46s %s\n' "$name:" "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
    fi
    cp /tmp/claude-0/Ww8Tables.cs.before "$SRC" && touch "$SRC"
}

run 'the covered cell keeps its own borders (r154)' '_ = owner.Borders;'
run 'the thicker of the two'                        'cell.Borders = (owner.Borders.Top?.EighthPoints ?? 0) > (cell.Borders.Top?.EighthPoints ?? 0) ? owner.Borders : cell.Borders;'
run 'no borders at all (before r154)'               'cell.Borders = default;'

cp /tmp/claude-0/Ww8Tables.cs.before "$SRC" && touch "$SRC"
out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter "FullyQualifiedName~Ww8CoveredCellBordersTests" 2>&1)
printf '%-46s %s\n' 'base (the master, always):' "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
