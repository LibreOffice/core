#!/bin/sh
# Mutation pin for EscapementSizeTests: replace the truncation with each rival rule and check the
# suite goes red.  The harness greps the build output for compiler errors, because an arm that does
# not compile prints no failing test and reads exactly like an arm that passed.
set -u
cd /home/user/libreoffice-core/dotnet || exit 1
SRC=src/XRay.Office.WordProcessing/Layout/Escapement.cs
TRUNC='=> Proportion is 0 or 100 ? emSize : Length.FromTwips(emSize.Twips * Proportion / 100);'
cp "$SRC" /tmp/claude-0/Escapement.cs.before || exit 1

run() {
    name=$1; repl=$2
    python3 - "$SRC" "$TRUNC" "$repl" <<'PY'
import sys
p, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p).read()
assert old in s, 'base text not found'
open(p, 'w').write(s.replace(old, new))
PY
    touch "$SRC"
    out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
            --filter "FullyQualifiedName~EscapementSizeTests" 2>&1)
    if printf '%s' "$out" | grep -qE 'error [A-Z]+[0-9]+'; then
        echo "$name: DID NOT COMPILE -- arm void"
        printf '%s' "$out" | grep -E 'error [A-Z]+[0-9]+' | head -3
    else
        printf '%s: %s\n' "$name" "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
    fi
    cp /tmp/claude-0/Escapement.cs.before "$SRC" && touch "$SRC"
}

run 'round-to-a-twip  ' '=> Proportion is 0 or 100 ? emSize : Length.FromTwips((long)System.Math.Round(emSize.Twips * Proportion / 100.0));'
run 'round-to-a-tenth ' '=> Proportion is 0 or 100 ? emSize : Length.FromTwips(2 * (long)System.Math.Floor(emSize.Twips * Proportion / 200.0 + 0.5));'
run 'no quantisation  ' '=> Proportion is 0 or 100 ? emSize : emSize * Proportion / 100;'
run 'truncate to a tenth' '=> Proportion is 0 or 100 ? emSize : Length.FromTwips(2 * (emSize.Twips * Proportion / 200));'

# and the base state again, so the file the round ships is the one that was measured
cp /tmp/claude-0/Escapement.cs.before "$SRC" && touch "$SRC"
out=$(timeout 900 dotnet test tests/XRay.Office.WordProcessing.Tests/XRay.Office.WordProcessing.Tests.csproj \
        --filter "FullyQualifiedName~EscapementSizeTests" 2>&1)
printf 'base (truncate)   : %s\n' "$(printf '%s' "$out" | grep -E '^(Passed|Failed)!' | tail -1)"
