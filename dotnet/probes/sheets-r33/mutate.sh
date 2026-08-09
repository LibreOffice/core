#!/usr/bin/env bash
# The defects the round's tests were verified against, each through
# .claude/skills/corpus-batches/scripts/verify-test.sh Paperless.Spreadsheets '<cmd>' \
#     'FullyQualifiedName~SheetPinnedBandTests'
#
#   m1  the floor applied to every band, dynamic or not — the defect this round removed
#   m2  a band the margins leave no room for treated as dynamic
#   m3  a band whose text overflows it still reported dynamic
#
# m1 detected by 2 of 4 cases, m2 by 1, m3 by 3.
#
# A fourth mutation was tried and is worth recording as a non-detection: writing m3 as
# `isDynamic = nominal > statedBand + Length.FromPoints(1000)` changes nothing, because inside
# that branch the expression is false either way. That is an equivalent formulation rather than
# a defect, and verify-test.sh reporting it undetected is the right answer.
case "${1:?m1|m2|m3}" in
m1) sed -i 's|return isDynamic ? Length.Max(printed, DefaultBandHeight) : printed;|return Length.Max(printed, DefaultBandHeight);|' \
      dotnet/src/Paperless.Spreadsheets/MsBinary/XlsPrintSetup.cs ;;
m2) sed -i 's|            isDynamic = statedBand > Length.Zero;|            isDynamic = true;|' \
      dotnet/src/Paperless.Spreadsheets/Layout/SheetBandHeight.cs ;;
m3) perl -0pi -e 's/            isDynamic = false;\n            return statedBand;/            isDynamic = nominal >= statedBand;\n            return statedBand;/' \
      dotnet/src/Paperless.Spreadsheets/Layout/SheetBandHeight.cs ;;
esac
# Added with the second half of the round:
#   m4  a band of no height suppressed rather than drawn
#   m5  a pinned band keeping the shared 142-twip gap
# m4 detected by 1 of 5 cases, m5 by 1.
case "${1:-}" in
m4) sed -i 's|        if (right <= left || height < Length.Zero) return;|        if (right <= left || height <= Length.Zero) return;|' \
      dotnet/src/Paperless.Spreadsheets/Layout/SheetPageDecoration.cs ;;
m5) perl -0pi -e 's/        return isDynamic \? fallback : Length.Zero;/        return fallback;/' \
      dotnet/src/Paperless.Spreadsheets/MsBinary/XlsPrintSetup.cs ;;
esac
