#!/usr/bin/env bash
# The reintroduced bugs the cell-margin tests were verified against.
set -uo pipefail
V=/home/user/libreoffice-core/.claude/skills/corpus-batches/scripts/verify-test.sh
run() {
  echo "=================================================================="
  echo "MUTATION: $1"
  "$V" Paperless.Spreadsheets "$2" 2>&1 | tail -12
}
run "the BIFF filter's 40 twips reduced to the pool's 20" \
  "sed -i 's/public static readonly Length CellMargin = Length.FromTwips(40);/public static readonly Length CellMargin = Length.FromTwips(20);/' dotnet/src/Paperless.Spreadsheets/MsBinary/XlsCellFormats.cs"
run "the placement reads the constant instead of the cell's margin" \
  "sed -i 's/Length margin = SheetDeviceUnits.Snap(format.Margin) \* scale;/Length margin = SheetDeviceUnits.Snap(CellMargin) * scale;/' dotnet/src/Paperless.Spreadsheets/Layout/SheetTextLayout.cs"
run "the BIFF reader never states a margin on its formats" \
  "sed -i '/^            Margin = CellMargin,$/d' dotnet/src/Paperless.Spreadsheets/MsBinary/XlsCellFormats.cs"
run "the sheet's fallback format keeps the pool's margin" \
  "sed -i 's/formats.Intern(SheetCellFormat.Default with { Margin = XlsCellFormats.CellMargin })/formats.Intern(SheetCellFormat.Default with { Margin = SheetTextLayout.CellMargin })/' dotnet/src/Paperless.Spreadsheets/MsBinary/XlsWorkbookReader.cs"
run "the row-height arithmetic pins the margin at Excel's 40" \
  "sed -i 's/=> 2 \* (int)format.Margin.Twips;/=> 80;/' dotnet/src/Paperless.Spreadsheets/Layout/SheetOptimalRowHeights.cs"
run "the wrapped-cell margin pixels pinned at Excel's two" \
  "sed -i 's/=> (int)(format.Margin.Twips \* PixelsPerTwip);/=> 2;/' dotnet/src/Paperless.Spreadsheets/Layout/SheetOptimalRowHeights.cs"
run "the wrapping paper's margin pinned at the pool default" \
  "sed -i 's/- (2 \* (long)(format.Margin.Twips \* horizontal)) - 1;/- (2 * (long)(20 * horizontal)) - 1;/' dotnet/src/Paperless.Spreadsheets/Layout/SheetOptimalRowHeights.cs"
run "the print-area extension reads the pool default" \
  "sed -i 's/private static Length CellMarginsOf(SheetCellFormat format) => format.Margin \* 2;/private static Length CellMarginsOf(SheetCellFormat format) => SheetTextLayout.CellMargin * 2;/' dotnet/src/Paperless.Spreadsheets/Layout/SheetTextOverflow.cs"
