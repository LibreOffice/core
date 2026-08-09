#!/usr/bin/env bash
# The defects `SheetShownCommentTests` was verified against.
#
# Each is one line, meant to be handed to
# `.claude/skills/corpus-batches/scripts/verify-test.sh Paperless.Spreadsheets '<line>' SheetShownComment`
# from the repository root with a clean `dotnet/`.
set -euo pipefail

N=dotnet/src/Paperless.Spreadsheets/Ooxml/XlsxNoteCaptions.cs
G=dotnet/src/Paperless.Spreadsheets/Layout/SheetPageGraphics.cs

case "${1:?usage: mutate.sh m1|m2|m3|m4|m5}" in

# The shown comments are never read at all — the tree as it stood before this round.
m1) sed -i 's|List<SheetDrawing> captions = \[\];|List<SheetDrawing> captions = []; return captions;|' "$N" ;;

# Visibility keyed on the `x:Visible` element rather than on the shape's CSS, which draws
# every note the file marks visible in either sense.
m2) sed -i 's|IsVisible(shape.Attribute("style")?.Value)|true|' "$N" ;;

# The anchor's pixel offsets read as points rather than as screen pixels, which is the
# obvious wrong unit and moves the caption by a third.
m3) sed -i 's|private const double PixelsPerInch = 96;|private const double PixelsPerInch = 72;|' "$N" ;;

# The caption placed against the cell its anchor names rather than against the commented
# cell, which is where it sat before the re-basing.
m4) sed -i 's|drawing.NoteCell?.Column ?? drawing.From.Column|drawing.From.Column|; s|drawing.NoteCell?.Row ?? drawing.From.Row|drawing.From.Row|' "$G" ;;

# The caption's text inset left at an ordinary drawing object's, not Calc's Note style's.
m5) sed -i 's|Length.FromMm100(100)|Length.FromInches(0.1)|' "$N" ;;

*) echo "unknown mutation" >&2; exit 2 ;;
esac
