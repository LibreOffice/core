#!/usr/bin/env bash
# Reintroduce each bug XlsChartFormatTests is meant to catch.
set -uo pipefail
W=/home/user/libreoffice-core/.claude/worktrees/sheets-r27/dotnet
R=$W/src/Paperless.Spreadsheets/MsBinary/XlsChartReader.cs
FILTER="${FILTER:-XlsChartFormatTests}"

run() {
  local name="$1" out line
  out=$(cd "$W" && dotnet test tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj \
        --filter "FullyQualifiedName~$FILTER" 2>&1)
  line=$(echo "$out" | grep -oE 'Failed:[[:space:]]+[0-9]+, Passed:[[:space:]]+[0-9]+' | tail -1)
  [ -z "$line" ] && line="BUILD FAILED: $(echo "$out" | grep -E 'error CS' | head -1)"
  printf '%-58s %s\n' "$name" "$line"
}

mutate() {
  local file="$1" name="$2"
  cp "$file" "$file.orig"
  python3 -c "$3" "$file" || { echo "patch failed: $name"; cp "$file.orig" "$file"; return; }
  if cmp -s "$file" "$file.orig"; then echo "NO-OP patch: $name"; rm "$file.orig"; return; fi
  run "$name"
  mv "$file.orig" "$file"
}

echo "=== baseline (clean tree)"
run "clean"

mutate "$R" "1. CHAREAFORMAT never read" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""            case BiffChartRecords.AreaFormat:
                ReadAreaFormat(stream);
                break;
""","")
open(p,"w").write(s)'

mutate "$R" "2. CHLINEFORMAT never read" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""            case BiffChartRecords.LineFormat:
                ReadLineFormat(stream);
                break;
""","")
open(p,"w").write(s)'

mutate "$R" "3. CHESCHERFORMAT never read" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""            case BiffChartRecords.EscherFormat:
                ReadEscherFormat(stream);
                break;
""","")
open(p,"w").write(s)'

mutate "$R" "4. the Escher format does not supersede" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("Fill(new BiffChartColour(value & 0x00FFFFFF, NoPaletteIndex), supersedes: true);",
            "Fill(new BiffChartColour(value & 0x00FFFFFF, NoPaletteIndex), supersedes: false);")
open(p,"w").write(s)'

mutate "$R" "5. the fFilled boolean ignored" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if (!properties.Boolean(EscherPropertyIds.Filled, fallback: true)) return;\n","")
open(p,"w").write(s)'

mutate "$R" "6. an MSO scheme word read as a literal" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if ((value & (IndexedColour | SchemeColour | SystemColour)) != 0) return;\n","")
open(p,"w").write(s)'

mutate "$R" "7. the AUTO flag ignored" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""        if ((flags & AutomaticFormat) != 0) return;

        Fill(pattern""","""        Fill(pattern""")
open(p,"w").write(s)'

mutate "$R" "8. EXC_PATT_NONE filled anyway" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("Fill(pattern == PatternNone ? null : new BiffChartColour(foreground, index), supersedes: false);",
            "Fill(new BiffChartColour(foreground, index), supersedes: false);")
open(p,"w").write(s)'

mutate "$R" "9. any frame taken as the background or wall" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if (Inside(BiffChartRecords.Legend) || Inside(BiffChartRecords.Text)) return;\n","")
open(p,"w").write(s)'

mutate "$R" "10. the axes-set frame taken as the chart background" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("        if (Inside(BiffChartRecords.AxesSet))","        if (!Inside(BiffChartRecords.AxesSet))")
open(p,"w").write(s)'

mutate "$R" "11. one fill for every series" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("            if (_series.Count > 0) _series[^1].Fill = colour;",
            "            foreach (SeriesLinks each in _series) each.Fill = colour;")
open(p,"w").write(s)'

mutate "$R" "12. a series fill filed as the chart background" '
import sys; p=sys.argv[1]; s=open(p).read()
s=s.replace("""        if (InSeriesFormat())
        {
            if (_series.Count > 0) _series[^1].Fill = colour;
            return;
        }

""","")
open(p,"w").write(s)'

echo "=== rebuilding clean"
(cd "$W" && dotnet build Paperless.slnx 2>&1 | grep -E "error|Build succeeded" | head -3)
