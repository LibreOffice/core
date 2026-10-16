#!/bin/sh
# Mutation pin for OdtTableRowHeightTests: revert each of the three rules this round reads and check
# the suite goes red. The harness greps the build output for compiler errors, because an arm that does
# not compile prints no failing test and reads exactly like an arm that passed.
set -u
cd /home/user/libreoffice-core/dotnet || exit 1
SRC=src/XRay.Office.WordProcessing/OpenDocument/OdtLayoutSource.cs
TBL=src/XRay.Office.WordProcessing/OpenDocument/OdtLayoutSource.Tables.cs
cp "$SRC" /tmp/claude-0/OdtLayoutSource.cs.before || exit 1
cp "$TBL" /tmp/claude-0/OdtLayoutSource.Tables.cs.before || exit 1

restore() {
    cp /tmp/claude-0/OdtLayoutSource.cs.before "$SRC" && touch "$SRC"
    cp /tmp/claude-0/OdtLayoutSource.Tables.cs.before "$TBL" && touch "$TBL"
}

run() {
    name=$1; file=$2; old=$3; new=$4
    python3 - "$file" "$old" "$new" <<'PY'
import sys
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p).read()
assert old in s, 'base text not found: ' + old[:60]
open(p, 'w').write(s.replace(old, new, 1))
PY
    touch "$file"
    out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
            --filter "FullyQualifiedName~OdtTableRowHeightTests" 2>&1)
    if printf '%s' "$out" | grep -qE 'error [A-Z]+[0-9]+'; then
        echo "$name: DID NOT COMPILE -- arm void"
        printf '%s' "$out" | grep -E 'error [A-Z]+[0-9]+' | head -2
    else
        printf '%-40s %s\n' "$name:" "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
    fi
    restore
}

run 'the MinRowHeightInclBorder setting' "$SRC" \
    '        => Setting(settings, "MinRowHeightInclBorder") == "true";' \
    '        => Setting(settings, "MinRowHeightInclBorder") == "this is not a boolean";'
run 'the covered cell''s top rule' "$TBL" \
    '                coveredTop = Length.Max(coveredTop, coveredBorders.Top.Width);' \
    '                coveredTop = Length.Max(coveredTop, Length.Zero);'
run 'the covered cell''s bottom rule' "$TBL" \
    '                coveredBottom = Length.Max(coveredBottom, coveredBorders.Bottom.Width);' \
    '                coveredBottom = Length.Max(coveredBottom, Length.Zero);'
run 'fo:break-before on a table style' "$TBL" \
    '            StartsNewPage = StartsNewPage(styleName),' \
    '            StartsNewPage = false,'

restore
out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter "FullyQualifiedName~OdtTableRowHeightTests" 2>&1)
printf '%-40s %s\n' 'base (all four in place):' "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
