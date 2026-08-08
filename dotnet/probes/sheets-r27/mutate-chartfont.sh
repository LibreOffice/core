#!/usr/bin/env bash
# Reintroduce each bug XlsChartFontTests is meant to catch, and report which cases fail.
#
# Every mutation is applied to a clean tree, measured, and reverted. A mutation that fails
# no case means the test does not test it.
set -uo pipefail
W=/home/user/libreoffice-core/.claude/worktrees/sheets-r27/dotnet
R=$W/src/Paperless.Spreadsheets/MsBinary/XlsChartReader.cs
C=$W/src/Paperless.Spreadsheets/MsBinary/XlsCellFormats.cs
K=$W/src/Paperless.Spreadsheets/MsBinary/XlsWorkbookReader.cs
FILTER="${FILTER:-XlsChartFontTests}"

run() {
  local name="$1"
  local out
  out=$(cd "$W" && dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
        --filter "FullyQualifiedName~$FILTER" 2>&1)
  local line
  line=$(echo "$out" | grep -oE 'Failed:[[:space:]]+[0-9]+, Passed:[[:space:]]+[0-9]+' | tail -1)
  if [ -z "$line" ]; then
    line="BUILD FAILED: $(echo "$out" | grep -E 'error CS' | head -1)"
  fi
  printf '%-58s %s\n' "$name" "$line"
}

mutate() {  # mutate <file> <name> <python-expression-file>
  local file="$1" name="$2"
  cp "$file" "$file.orig"
  python3 -c "$3" "$file" || { echo "patch failed: $name"; cp "$file.orig" "$file"; return; }
  if cmp -s "$file" "$file.orig"; then echo "NO-OP patch: $name"; rm "$file.orig"; return; fi
  run "$name"
  mv "$file.orig" "$file"
}

echo "=== baseline (clean tree)"
run "clean"

mutate "$R" "1. CHFONT never read" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""            case BiffChartRecords.Font:
                ReadFont(stream.ReadUInt16());
                break;
""","")
open(p,"w").write(s)'

mutate "$K" "2. the FONT buffer not handed to Build" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace(", index, _cellFormats)",", index)").replace(", _sheetIndex, _cellFormats)",", _sheetIndex)")
open(p,"w").write(s)'

mutate "$R" "3. the first CHFONT wins, no default-text order" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""        int index = _globalFont != NoFont ? _globalFont
            : _axesSetFont != NoFont ? _axesSetFont
            : _firstFont;""","""        int index = _firstFont;""")
open(p,"w").write(s)'

mutate "$R" "4. the axes-set default outranks the global one" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""        int index = _globalFont != NoFont ? _globalFont
            : _axesSetFont != NoFont ? _axesSetFont
            : _firstFont;""","""        int index = _axesSetFont != NoFont ? _axesSetFont
            : _globalFont != NoFont ? _globalFont
            : _firstFont;""")
open(p,"w").write(s)'

mutate "$R" "5. any CHFONT taken as the open default text" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if (!InnermostIs(BiffChartRecords.Text)) return;\n","")
open(p,"w").write(s)'

mutate "$R" "6. a CHDEFAULTTEXT id kept until a CHTEXT arrives" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""        int defaultText = _pendingDefaultText;
        _pendingDefaultText = NoDefaultText;""","""        int defaultText = _pendingDefaultText;""")
open(p,"w").write(s)'

mutate "$C" "7. the FONT buffer indexed by record ordinal" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if (_fonts.Count == SkippedFontIndex) _fonts.Add(font);\n","")
open(p,"w").write(s)'

echo "=== rebuilding clean"
(cd "$W" && dotnet build Paperless.slnx 2>&1 | grep -E "error|Build succeeded" | head -3)
