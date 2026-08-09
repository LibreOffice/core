#!/usr/bin/env bash
# The reintroduced defects round thirty-one's tests were verified against.
#
#   .claude/skills/corpus-batches/scripts/verify-test.sh Paperless.Spreadsheets "<one line below>"
#
# Exit 0 means the mutation was detected, 1 means nothing caught it, 2 means it refused to run
# or would not build. Run from the worktree root, on a clean `dotnet/`.

L=dotnet/src/Paperless.Spreadsheets/Layout
O=dotnet/src/Paperless.Spreadsheets/Ooxml

# 1. The collapsed outline group is never applied — the code as it stood before this round.
#    Expect: every SheetOutlineCollapseTests case that reads the grid fails.
m1() { perl -0pi -e 's/        rows = SheetOutlineCollapse\.Apply\(\n.*?defaultHeight\);\n//s' "$O/XlsxPrintSetup.cs"; }

# 2. Only the first line of each part prints.
#    Expect: BothHeaderLinesAreDrawn, TheFooterIsAnchoredToTheFooterMargin.
m2() { perl -0pi -e 's/(IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> lines = part\.Lines\(context\);)/$1 lines = lines.Count > 0 ? [lines[0]] : lines;/' "$L/SheetPageDecoration.cs"; }

# 3. The stated size is discarded, as the parser used to discard it.
#    Expect: TheStatedSizeIsTheSizeDrawn, AShorterPartIsCentredAgainstTheTallestOne,
#            TheHeaderIsAnchoredToTheHeaderMargin, BothHeaderLinesAreDrawn.
m3() { perl -0pi -e 's/\(piece\.Size \?\? SheetBandText\.DefaultSize\)/(SheetBandText.DefaultSize)/' "$L/SheetPageDecoration.cs"; }

# 4. The band is centred rather than anchored, which is what it did before this round.
#    Expect: TheHeaderIsAnchoredToTheHeaderMargin, TheFooterIsAnchoredToTheFooterMargin,
#            AShorterPartIsCentredAgainstTheTallestOne.
m4() { perl -0pi -e 's/Length drawn = dynamic \? bandText : height;/Length drawn = height;/;
                     s/Length bandTop = dynamic && fromBottom \? top \+ height - bandText : top;/Length bandTop = top;/' "$L/SheetPageDecoration.cs"; }

# 5. Each part is anchored on its own rather than centred against the tallest.
#    Expect: AShorterPartIsCentredAgainstTheTallestOne.
m5() { perl -0pi -e 's/Length spare = drawn - text;/Length spare = dynamic ? Length.Zero : drawn - text;/;
                     s/Length pen = bandTop \+ \(spare > Length\.Zero \? spare \/ 2 : Length\.Zero\);/Length pen = (dynamic \&\& fromBottom ? top + height - text : bandTop) + (spare > Length.Zero ? spare \/ 2 : Length.Zero);/' "$L/SheetPageDecoration.cs"; }

# 6. The size carries across a section switch.
#    Expect: ASectionSwitchResetsTheSize.
m6() { perl -0pi -e "s/case 'L': Flush\(\); current = left; size = null; break;/case 'L': Flush(); current = left; break;/" "$L/SheetHeaderFooter.cs"; }

"${1:?usage: mutate.sh m1|m2|m3|m4|m5|m6}"
