#!/usr/bin/env bash
# Reintroduce one drawing bug and count the rotated-placement cases that fail.
set -u
D=/home/user/libreoffice-core/.claude/worktrees/sheets-r26/dotnet
F=$D/src/Paperless.Spreadsheets/Layout/SheetTextLayout.cs
cp /tmp/stl.bak "$F"

case "$1" in
  no-ascent)
    python3 - "$F" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("anchor.Y + down + line.Run.Ascent)","anchor.Y + down)")
open(p,'w').write(s)
PY
    ;;
  lines-share-one-origin)
    python3 - "$F" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("                down += line.Run.LineHeight;","                down += Length.Zero;")
open(p,'w').write(s)
PY
    ;;
  one-anchor-for-both-signs)
    python3 - "$F" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""        DocPoint anchor = quarter && cell.Format.RotationDegrees < 0
            ? new DocPoint(cell.Box.X + margin + Stack(placement), cell.Box.Y + margin)
            : new DocPoint(cell.Box.X + margin, cell.Box.Y + cell.Box.Height - margin);""",
"""        DocPoint anchor =
            new DocPoint(cell.Box.X + margin, cell.Box.Y + cell.Box.Height - margin);""")
open(p,'w').write(s)
PY
    ;;
  no-along-offset)
    python3 - "$F" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("                        anchor.X + AlongOffset(cell.Format, inner, line.Run.Width),","                        anchor.X,")
open(p,'w').write(s)
PY
    ;;
  wrap-at-the-column-width)
    python3 - "$F" <<'PY'
import sys
p=sys.argv[1]; s=open(p).read()
s=s.replace("""        Length available = IsQuarterTurned(format) && breaks
            ? cell.Box.Height - (2 * margin)
            : cell.Box.Width - totalMargin;""",
"""        Length available = cell.Box.Width - totalMargin;""")
open(p,'w').write(s)
PY
    ;;
  none) ;;
  *) echo "unknown $1" >&2; exit 2 ;;
esac

out=$(cd "$D" && dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
      --filter "FullyQualifiedName~SheetRotatedTextPlacementTests" 2>&1)
echo "$1: $(echo "$out" | grep -oP 'Failed:\s+\d+, Passed:\s+\d+' | tail -1)${_:-}"
echo "$out" | grep -c "error CS" | sed 's/^/    build errors: /'
cp /tmp/stl.bak "$F"
