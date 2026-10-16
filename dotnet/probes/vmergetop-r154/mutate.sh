#!/bin/sh
# Mutation pin for TableCoveredCellRuleTests: take each of the five places the covered cell's rules
# are carried, put it back the way it was, and check the suite goes red. The harness greps the build
# output for compiler errors, because an arm that does not compile prints no failing test and reads
# exactly like an arm that passed.
set -u
cd /home/user/libreoffice-core/dotnet || exit 1
LAY=src/XRay.Office.WordProcessing/Layout/TableLayouter.cs
DOCX=src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs
cp "$LAY" /tmp/claude-0/TableLayouter.cs.before || exit 1
cp "$DOCX" /tmp/claude-0/DocxLayoutSource.Tables.cs.before || exit 1

restore() {
    cp /tmp/claude-0/TableLayouter.cs.before "$LAY" && touch "$LAY"
    cp /tmp/claude-0/DocxLayoutSource.Tables.cs.before "$DOCX" && touch "$DOCX"
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
            --filter "FullyQualifiedName~TableCoveredCellRuleTests" 2>&1)
    if printf '%s' "$out" | grep -qE 'error [A-Z]+[0-9]+'; then
        echo "$name: DID NOT COMPILE -- arm void"
        printf '%s' "$out" | grep -E 'error [A-Z]+[0-9]+' | head -2
    else
        printf '%-34s %s\n' "$name:" "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
    fi
    restore
}

run 'the row-own term of TopBand' "$LAY" \
    '        Length band = row.CoveredTopRule;' \
    '        Length band = Length.Zero;'
run 'the above term of TopBand' "$LAY" \
    '            band = Length.Max(band, above.CoveredBottomRule);' \
    '            band = Length.Max(band, Length.Zero);'
run 'OwnTopRule' "$LAY" \
    '        Length top = row.CoveredTopRule;' \
    '        Length top = Length.Zero;'
run 'BottomBand' "$LAY" \
    '        Length band = row.CoveredBottomRule;' \
    '        Length band = Length.Zero;'
run 'the DOCX reader filling either' "$DOCX" \
    '                    coveredTop = Length.Max(coveredTop, cell.Definition.Borders.Top.Width);' \
    '                    coveredTop = Length.Max(coveredTop, Length.Zero);'

restore
out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter "FullyQualifiedName~TableCoveredCellRuleTests" 2>&1)
printf '%-34s %s\n' 'base (all five in place):' "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
