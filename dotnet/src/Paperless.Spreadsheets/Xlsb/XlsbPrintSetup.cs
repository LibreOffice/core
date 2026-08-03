using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// Reads a BIFF12 worksheet part's page geometry and grid.
/// </summary>
/// <remarks>
/// <para>
/// The same fields <c>XlsxPrintSetup</c> reads and the same rules applied to them, because they
/// are the same settings — LibreOffice puts both formats through one <c>PageSettings</c> and one
/// <c>WorksheetGlobals</c>, so an XLSB read differently from an XLSX would be a bug rather than
/// a dialect. What differs is only the encoding, and in two places it differs in a way worth
/// stating:
/// </para>
/// <para>
/// <strong>Widths are 256ths of a character, not characters.</strong> <c>COL</c> and
/// <c>SHEETFORMATPR</c> both state a width as <c>1/256</c>th of a digit and the XML states it as
/// a fraction of one (<c>worksheetfragment.cxx:800, :827</c>), so an XLSB read with the XML's
/// scale gives columns 256 times too wide and a sheet of one column per page.
/// </para>
/// <para>
/// <strong>Row and default heights are twips, not points.</strong> The same two records state a
/// height in twentieths of a point where the XML states points.
/// </para>
/// <para>
/// The margins are the one place BIFF12 is <em>easier</em>: <c>PAGEMARGINS</c> is six doubles in
/// inches, in the same order and meaning as the XML attributes, so the header-band arithmetic
/// carries across unchanged.
/// </para>
/// </remarks>
internal static class XlsbPrintSetup
{
    /// <summary>Half a twip, which turns <see cref="SheetDigitWidth"/>'s truncation into rounding.</summary>
    /// <remarks>See <c>XlsxPrintSetup</c>, whose conversion this is byte for byte.</remarks>
    private const double RoundingBiasTwips = 0.5;

    /// <summary>The five screen pixels <c>baseColWidth</c> carries and <c>defaultColWidth</c> does not.</summary>
    private const double BasePaddingTwips = 75;

    private const int DefaultBaseColumnWidth = 8;

    private const ushort PageSetupInRows = 0x0001;
    private const ushort PageSetupLandscape = 0x0002;
    private const ushort PageSetupInvalid = 0x0004;
    private const ushort PageSetupDefaultOrientation = 0x0040;
    private const ushort PageSetupUseFirstPage = 0x0080;

    private const ushort PrintOptionHorizontalCentre = 0x0001;
    private const ushort PrintOptionVerticalCentre = 0x0002;
    private const ushort PrintOptionHeadings = 0x0004;
    private const ushort PrintOptionGrid = 0x0008;

    private const ushort SheetPrFitToPages = 0x0100;

    private const ushort ColumnHidden = 0x0001;
    private const ushort RowHidden = 0x1000;
    private const ushort RowCustomHeight = 0x2000;

    /// <summary>Builds a sheet's layout input from its worksheet part.</summary>
    /// <param name="part">The part's bytes, or null when it did not load.</param>
    /// <param name="defaultFont">
    /// The workbook's default font, which a column width is stated in digits of. Null falls back
    /// to Calc's own — see <see cref="SheetColumnDigits"/>.
    /// </param>
    public static (SheetPrintSetup Setup, SheetGrid Grid) Read(
        byte[]? part, SheetDefaultFont? defaultFont = null)
    {
        if (part is null) return (SheetPrintSetup.Default, SheetGrid.Standard);

        double left = 0.7, right = 0.7, top = 0.75, bottom = 0.75, header = 0.3, footer = 0.3;
        string? headerText = null;
        string? footerText = null;
        bool landscape = false;
        bool fitToPages = false;
        bool statedPaper = false;
        int paperSize = 1;
        int scale = 100;
        int fitToWidth = 1;
        int fitToHeight = 1;
        int firstPage = 0;
        bool acrossThenDown = false;
        bool printsGrid = false;
        bool printsHeadings = false;
        bool centresHorizontally = false;
        bool centresVertically = false;

        SheetDigitWidth defaultWidth = BaseWidth(null);
        Length defaultHeight = SheetGrid.StandardRowHeight;
        Length? statedHeight = null;
        List<SheetDigitRun> columns = [];
        List<SheetSizeRun> rows = [];
        List<int> columnBreaks = [];
        List<int> rowBreaks = [];
        int breakAxis = 0;

        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            Biff12Cursor cursor = new(record.Data.Span);
            switch (record.Id)
            {
                case Biff12.RowBreaks or Biff12.ColBreaks:
                    breakAxis = record.Id;
                    break;

                case Biff12.RowBreaks + 1 or Biff12.ColBreaks + 1:
                    breakAxis = 0;
                    break;

                case Biff12.Brk:
                {
                    int at = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    bool manual = cursor.ReadInt32() != 0;

                    // Only the author's breaks are honoured; the automatic ones Excel records
                    // beside them are its own pagination, which is the thing being recomputed.
                    if (!manual || at <= 0) break;
                    if (breakAxis == Biff12.RowBreaks) rowBreaks.Add(at);
                    else if (breakAxis == Biff12.ColBreaks) columnBreaks.Add(at);
                    break;
                }

                case Biff12.SheetPr:
                {
                    ushort flags = cursor.ReadUInt16();
                    fitToPages = (flags & SheetPrFitToPages) != 0;
                    break;
                }

                case Biff12.SheetFormatPr:
                {
                    int statedWidth = cursor.ReadInt32();
                    int baseWidth = cursor.ReadUInt16();
                    int height = cursor.ReadUInt16();

                    // A stated defaultColWidth overrides the base width, which is the order
                    // setDefaultColumnWidth documents; -1 means the sheet states none.
                    defaultWidth = statedWidth > 0
                        ? Digits(statedWidth / 256.0)
                        : BaseWidth(baseWidth > 0 ? baseWidth : null);
                    if (height > 0) statedHeight = defaultHeight = Length.FromTwips(height);
                    break;
                }

                case Biff12.Col:
                {
                    int first = cursor.ReadInt32();
                    int last = cursor.ReadInt32();
                    int width = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    ushort flags = cursor.ReadUInt16();

                    if (last < first || first < 0) break;
                    columns.Add(new SheetDigitRun(
                        first,
                        Math.Min(last, SheetAddress.MaxColumn),
                        width > 0 ? Digits(width / 256.0) : defaultWidth,
                        (flags & ColumnHidden) != 0));
                    break;
                }

                case Biff12.Row:
                {
                    int index = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    int height = cursor.ReadUInt16();
                    ushort flags = cursor.ReadUInt16();

                    bool hidden = (flags & RowHidden) != 0;
                    if (index < 0 || (height <= 0 && !hidden)) break;

                    rows.Add(new SheetSizeRun(
                        index, index,
                        height > 0 ? Length.FromTwips(height) : defaultHeight,
                        hidden,
                        (flags & RowCustomHeight) == 0));
                    break;
                }

                case Biff12.PageMargins:
                    left = cursor.ReadDouble();
                    right = cursor.ReadDouble();
                    top = cursor.ReadDouble();
                    bottom = cursor.ReadDouble();
                    header = cursor.ReadDouble();
                    footer = cursor.ReadDouble();
                    break;

                case Biff12.PrintOptions:
                {
                    ushort flags = cursor.ReadUInt16();
                    centresHorizontally = (flags & PrintOptionHorizontalCentre) != 0;
                    centresVertically = (flags & PrintOptionVerticalCentre) != 0;
                    printsHeadings = (flags & PrintOptionHeadings) != 0;
                    printsGrid = (flags & PrintOptionGrid) != 0;
                    break;
                }

                case Biff12.PageSetup:
                {
                    paperSize = cursor.ReadInt32();
                    scale = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    int statedFirstPage = cursor.ReadInt32();
                    fitToWidth = cursor.ReadInt32();
                    fitToHeight = cursor.ReadInt32();
                    ushort flags = cursor.ReadUInt16();

                    landscape = (flags & PageSetupDefaultOrientation) == 0
                                && (flags & PageSetupLandscape) != 0;
                    acrossThenDown = (flags & PageSetupInRows) != 0;
                    firstPage = (flags & PageSetupUseFirstPage) != 0 ? statedFirstPage : 0;

                    // The same rule the XML path records, stated by a flag rather than by an
                    // attribute's absence: the paper size is applied only when the settings are
                    // marked invalid, and otherwise the application's own paper stands.
                    statedPaper = (flags & PageSetupInvalid) != 0;
                    break;
                }

                case Biff12.HeaderFooter:
                {
                    _ = cursor.ReadUInt16();
                    string odd = cursor.ReadString();
                    string oddFooter = cursor.ReadString();
                    headerText = odd.Length > 0 ? odd : null;
                    footerText = oddFooter.Length > 0 ? oddFooter : null;
                    break;
                }
            }
        }

        bool hasHeader = headerText is { Length: > 0 };
        bool hasFooter = footerText is { Length: > 0 };

        (Length paperWidth, Length paperHeight) = statedPaper
            ? ExcelPaperSizes.Portrait(paperSize)
            : ExcelPaperSizes.A4;

        SheetPrintSetup setup = new()
        {
            PageSize = landscape
                ? new DocSize(paperHeight, paperWidth)
                : new DocSize(paperWidth, paperHeight),
            IsLandscape = landscape,
            LeftMargin = Length.FromInches(left),
            RightMargin = Length.FromInches(right),
            TopMargin = Length.FromInches(hasHeader ? header : top),
            BottomMargin = Length.FromInches(hasFooter ? footer : bottom),
            HeaderHeight = hasHeader ? Length.FromInches(Math.Max(0, top - header)) : Length.Zero,
            FooterHeight = hasFooter ? Length.FromInches(Math.Max(0, bottom - footer)) : Length.Zero,
            HeaderText = headerText,
            FooterText = footerText,
            Header = headerText is null ? null : SheetHeaderFooter.ParseCodes(headerText),
            Footer = footerText is null ? null : SheetHeaderFooter.ParseCodes(footerText),
            ScaleMode = fitToPages ? PrintScaleMode.FitToPages : PrintScaleMode.Percentage,
            ScalePercentage = scale > 0 ? scale : 100,
            FitToPagesWide = fitToPages ? Math.Max(0, fitToWidth) : 0,
            FitToPagesTall = fitToPages ? Math.Max(0, fitToHeight) : 0,
            PageOrder = acrossThenDown ? PagePrintOrder.AcrossThenDown : PagePrintOrder.DownThenAcross,
            PrintsGrid = printsGrid,
            PrintsHeadings = printsHeadings,
            CentresHorizontally = centresHorizontally,
            CentresVertically = centresVertically,
            FirstPageNumber = firstPage,
            ManualColumnBreaks = columnBreaks,
            ManualRowBreaks = rowBreaks,
        };

        SheetColumnDigits digits = new(defaultFont ?? SheetDefaultFont.Calc, defaultWidth, columns);

        // Materialised at the fallback so the grid is complete the moment it is built, and
        // remeasured by `SheetLayout.Grid` once a face can be resolved.
        return (setup, new SheetGrid(
            digits.Resolve(SheetColumnDigits.FallbackDigitWidthTwips),
            new SheetAxis(defaultHeight, rows))
        {
            ColumnDigits = digits,

            // BIFF12 goes through the same OOXML filter as SpreadsheetML, so it gets the same
            // floor from the same place — `worksheethelper.cxx:965`, reached for both.
            OptimalMinimumRowHeight = statedHeight ?? SheetGrid.StandardRowHeight,
        });
    }

    private static SheetDigitWidth Digits(double count)
        => count > 0 ? new SheetDigitWidth(count, RoundingBiasTwips) : default;

    private static SheetDigitWidth BaseWidth(int? baseColumnWidth)
    {
        int digits = baseColumnWidth is { } stated && stated > 0 ? stated : DefaultBaseColumnWidth;
        return new SheetDigitWidth(digits, BasePaddingTwips + RoundingBiasTwips);
    }
}
