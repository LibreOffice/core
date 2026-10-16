#!/bin/bash
# Run every test project individually and total them by hand.
#
# `dotnet test XRay.Office.slnx` is the most likely to truncate and the least
# likely to say so -- CLAUDE.md records it printing `Passed! - Failed: 0` after
# silently dropping the tests it never reached, and being OOM-killed outright.
# So: one project at a time, and compare what PASSED against what was
# DISCOVERED. A drop with zero failures is a truncated run, not a green one.
set -u
cd /home/user/libreoffice-core/dotnet || exit 1
LABEL="${1:-run}"
MODE="${2:-full}"   # "fast" skips XRay.Office.Fidelity.Tests (the only project that shells out to soffice)
OUT="/data/bench/testruns/$LABEL"
mkdir -p "$OUT"
total_pass=0; total_fail=0; total_skip=0; total_disc=0; bad=""
for p in tests/*/*.csproj; do
  n=$(basename "$p" .csproj)
  [ "$MODE" = fast ] && [ "$n" = XRay.Office.Fidelity.Tests ] && continue
  disc=$(dotnet test "$p" --no-build --list-tests 2>/dev/null | grep -cE '^\s{4}\S')
  dotnet test "$p" --no-build --nologo -v q > "$OUT/$n.log" 2>&1
  line=$(grep -oE 'Passed!?[^0-9]*-?\s*Failed:\s*[0-9]+,\s*Passed:\s*[0-9]+,\s*Skipped:\s*[0-9]+' "$OUT/$n.log" | tail -1)
  f=$(grep -oE 'Failed:\s*[0-9]+' "$OUT/$n.log" | tail -1 | grep -oE '[0-9]+')
  s=$(grep -oE 'Skipped:\s*[0-9]+' "$OUT/$n.log" | tail -1 | grep -oE '[0-9]+')
  q=$(grep -oE 'Passed:\s*[0-9]+' "$OUT/$n.log" | tail -1 | grep -oE '[0-9]+')
  f=${f:-0}; s=${s:-0}; q=${q:-0}
  flag=""
  [ "$f" != "0" ] && { flag="  <-- FAILURES"; bad="$bad $n"; }
  [ -n "$disc" ] && [ "$disc" != "0" ] && [ $((q+f+s)) -lt "$disc" ] && flag="$flag  <-- TRUNCATED ($((q+f+s)) of $disc)"
  printf "%-38s pass %4s  fail %3s  skip %3s  discovered %4s%s\n" "$n" "$q" "$f" "$s" "${disc:-?}" "$flag"
  total_pass=$((total_pass+q)); total_fail=$((total_fail+f)); total_skip=$((total_skip+s))
  total_disc=$((total_disc+${disc:-0}))
done
echo "-------------------------------------------------------------------"
printf "TOTAL%-33s pass %4s  fail %3s  skip %3s  discovered %4s\n" "" "$total_pass" "$total_fail" "$total_skip" "$total_disc"
[ -n "$bad" ] && echo "projects with failures:$bad"
echo "logs in $OUT"
