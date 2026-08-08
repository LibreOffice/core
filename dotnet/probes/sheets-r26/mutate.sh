#!/usr/bin/env bash
# Reintroduce one bug, run the two rotated-row test classes, report how many fail.
#   mutate.sh <name>
set -u
D=/home/user/libreoffice-core/.claude/worktrees/sheets-r26/dotnet
ORH=$D/src/Paperless.Spreadsheets/Layout/SheetOptimalRowHeights.cs
ST=$D/src/Paperless.Spreadsheets/Layout/SheetText.cs
ODS=$D/src/Paperless.Spreadsheets/OpenDocument/OdsCellFormats.cs

restore() { cp /tmp/orh.bak "$ORH"; cp /tmp/st.bak "$ST"; cp /tmp/ods.bak "$ODS"; }
restore

case "$1" in
  quarter-turn-is-a-rotation)
    python3 - "$ORH" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("=> format.IsStacked || Math.Abs(format.RotationDegrees) == 90;","=> format.IsStacked;")
open(p,'w').write(s)
PY
    ;;
  no-wrap-cap)
    python3 - "$ORH" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""            if (breaks)
            {
                long em = grid.ToEmSize(size).Twips / TwipsPerPixel;
                pixels = Math.Min(pixels, em * RotatedBreakFactor);
            }""","""            if (breaks)
            {
                long em = grid.ToEmSize(size).Twips / TwipsPerPixel;
                pixels = Math.Min(pixels, em * RotatedBreakFactor * 1000);
            }""")
open(p,'w').write(s)
PY
    ;;
  width-rounds-the-total)
    python3 - "$ST" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""        long pixels = 0;
        foreach (SheetTextSegment segment in run.Segments)
        {
            foreach (PositionedGlyph glyph in segment.Glyphs)
            {
                pixels += (long)Math.Round(
                    glyph.Advance.Twips / (double)twipsPerPixel, MidpointRounding.AwayFromZero);
            }
        }

        return pixels;""","""        return (long)Math.Round(
            run.Width.Twips / (double)twipsPerPixel, MidpointRounding.AwayFromZero);""")
open(p,'w').write(s)
PY
    ;;
  turned-cell-unmeasurable)
    python3 - "$ORH" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""                bool direct = turned
                              && portions is not ({ Count: > 0 })
                              && text.AsSpan().IndexOfAny('\\n', '\\r') < 0;""",
"""                bool direct = false && turned
                              && portions is not ({ Count: > 0 })
                              && text.AsSpan().IndexOfAny('\\n', '\\r') < 0;""")
open(p,'w').write(s)
PY
    ;;
  angle-clamped-not-folded)
    python3 - "$ODS" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""            int folded = ((int)Math.Round(degrees) % 360 + 360) % 360;
            if (folded > 180) folded -= 360;

            return Math.Clamp(folded, -90, 90);""",
"""            return Math.Clamp((int)Math.Round(degrees), -90, 90);""")
open(p,'w').write(s)
PY
    ;;
  floor-applies-to-every-row)
    python3 - "$ORH" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""                    IsQuarterTurned(format) ? 0 : AttributeHeight(format, minimum)),""",
"""                    format.IsStacked ? 0 : AttributeHeight(format, minimum)),""")
s=s.replace("""        if (unmeasurable) height = Math.Max(height, (int)axis.SizeAt(row).Twips);

        return height;""",
"""        if (unmeasurable) height = Math.Max(height, (int)axis.SizeAt(row).Twips);

        return Math.Max(height, 256);""")
open(p,'w').write(s)
PY
    ;;
  none) ;;
  *) echo "unknown mutation $1" >&2; exit 2 ;;
esac

out=$(cd "$D" && dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
      --filter "FullyQualifiedName~SheetRotatedRowHeightTests|FullyQualifiedName~SheetRowHeightDeviceTests" 2>&1)
echo "$1: $(echo "$out" | grep -oP 'Failed:\s+\d+, Passed:\s+\d+' | tail -1)"
echo "$out" | grep -oP '^\s+Failed \S+' | sed 's/^/    /' | sort | uniq -c | head -5
restore
