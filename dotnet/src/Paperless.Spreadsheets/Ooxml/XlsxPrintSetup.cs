using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Reads a worksheet's print setup and geometry out of a SpreadsheetML package.
/// </summary>
/// <remarks>
/// <para>
/// The conversion that matters is the header band. SpreadsheetML states a <c>top</c> margin
/// measured to the first row and a <c>header</c> margin measured to the header, so the band
/// between them is <c>top - header</c>; Calc stores the distance to the <em>header</em> as its
/// top margin and the band separately, so the two swap round. LibreOffice's own conversion is
/// at <c>sc/source/filter/oox/pagesettings.cxx:1001-1040</c>, and the consequence is worth
/// stating plainly because it is what makes this cheap: whether or not a sheet has a header,
/// the first row still starts at the file's own <c>top</c> margin.
/// </para>
/// <para>
/// The other conversion is the column width, which SpreadsheetML states in <em>digits</em> of
/// the workbook's default font rather than in any unit of length. LibreOffice resolves it by
/// asking the reference device for the widest digit's advance in whole twips and multiplying
/// (<c>WorksheetGlobals::convertColumns</c>, <c>sc/source/filter/oox/worksheethelper.cxx:1212</c>).
/// Measured against LibreOffice's own rendering of <c>sheet-ooxml-features.xlsx</c>, whose
/// columns are <c>width="20.76"</c>: the columns come out 115.2 points apart, which is 2304
/// twips, which is 20.76 × 111 rounded — 111 twips being the advance of a digit of
/// 10-point Liberation Sans.
/// </para>
/// </remarks>
internal static class XlsxPrintSetup
{
    /// <summary>SpreadsheetML's own default margins, in inches.</summary>
    /// <remarks>
    /// <c>OOX_MARGIN_DEFAULT_*</c>, <c>sc/source/filter/oox/pagesettings.cxx:63-65</c>. They are
    /// not round numbers because they are centimetres rounded to three decimal places of an
    /// inch: 1.9 cm, 2.5 cm and 1.3 cm.
    /// </remarks>
    private const double DefaultSideMarginInches = 0.748;

    /// <summary>The default top and bottom margin, in inches.</summary>
    private const double DefaultEndMarginInches = 0.984;

    /// <summary>The default header and footer margin, in inches.</summary>
    private const double DefaultBandMarginInches = 0.512;

    /// <summary>
    /// The advance of the widest digit in the default font, in twips.
    /// </summary>
    /// <remarks>
    /// 10-point Liberation Sans, which is what LibreOffice puts in a new spreadsheet and
    /// therefore what its rendering of a workbook naming no font measures in. A workbook whose
    /// default font is something else has proportionally wrong column widths, which is a real
    /// limitation and is recorded in the module's TODO: resolving it properly means reading
    /// <c>styles.xml</c>'s font table and measuring the face, which the reader does not do yet.
    /// </remarks>
    private const double DigitWidthTwips = 111;

    /// <summary>Builds a sheet's layout input from its <c>worksheet</c> element.</summary>
    /// <param name="worksheet">The worksheet part's root, or null when it did not load.</param>
    /// <param name="printAreas">The print areas the workbook's defined names gave this sheet.</param>
    /// <param name="repeatColumns">The repeated columns, from <c>_xlnm.Print_Titles</c>.</param>
    /// <param name="repeatRows">The repeated rows, from the same name.</param>
    public static (SheetPrintSetup Setup, SheetGrid Grid) Read(
        XElement? worksheet,
        IReadOnlyList<SheetRange> printAreas,
        SheetRange? repeatColumns,
        SheetRange? repeatRows)
    {
        if (worksheet is null)
            return (SheetPrintSetup.Default with { PrintAreas = printAreas }, SheetGrid.Standard);

        XElement? margins = Xlsx.Child(worksheet, "pageMargins");
        XElement? setupElement = Xlsx.Child(worksheet, "pageSetup");
        XElement? options = Xlsx.Child(worksheet, "printOptions");
        XElement? headerFooter = Xlsx.Child(worksheet, "headerFooter");

        double left = Inches(margins, "left", DefaultSideMarginInches);
        double right = Inches(margins, "right", DefaultSideMarginInches);
        double top = Inches(margins, "top", DefaultEndMarginInches);
        double bottom = Inches(margins, "bottom", DefaultEndMarginInches);
        double header = Inches(margins, "header", DefaultBandMarginInches);
        double footer = Inches(margins, "footer", DefaultBandMarginInches);

        string? headerText = Xlsx.Child(headerFooter, "oddHeader")?.Value;
        string? footerText = Xlsx.Child(headerFooter, "oddFooter")?.Value;
        bool hasHeader = !string.IsNullOrEmpty(headerText);
        bool hasFooter = !string.IsNullOrEmpty(footerText);

        // Only a header with content occupies a band. Calc's "header is on" flag is set from
        // whether any of the three header strings is non-empty, not from the margin being
        // written (pagesettings.cxx:1003).
        Length headerBand = hasHeader
            ? Length.FromInches(Math.Max(0, top - header))
            : Length.Zero;
        Length footerBand = hasFooter
            ? Length.FromInches(Math.Max(0, bottom - footer))
            : Length.Zero;

        bool landscape = string.Equals(
            Xlsx.Attribute(setupElement, "orientation"), "landscape", StringComparison.Ordinal);

        (PrintScaleMode mode, int percentage, int wide, int tall) = ReadScale(worksheet, setupElement);

        SheetPrintSetup setup = new()
        {
            PageSize = PaperSize(setupElement, landscape),
            IsLandscape = landscape,
            LeftMargin = Length.FromInches(left),
            RightMargin = Length.FromInches(right),
            TopMargin = Length.FromInches(hasHeader ? header : top),
            BottomMargin = Length.FromInches(hasFooter ? footer : bottom),
            HeaderHeight = headerBand,
            FooterHeight = footerBand,
            HeaderText = headerText,
            FooterText = footerText,

            // The band's own margins are zero here and not inherited, unlike ODF's: SpreadsheetML
            // states no header margin of its own, so the header runs the full width between the
            // page margins — measured at 56.7 pt to 538.55 pt on sheet-decor-xlsx.xlsx, exactly
            // the page's own margins, where the ODS twin indents by a further two centimetres.
            Header = headerText is null ? null : SheetHeaderFooter.ParseCodes(headerText),
            Footer = footerText is null ? null : SheetHeaderFooter.ParseCodes(footerText),
            ScaleMode = mode,
            ScalePercentage = percentage,
            FitToPagesWide = wide,
            FitToPagesTall = tall,
            PageOrder = string.Equals(
                Xlsx.Attribute(setupElement, "pageOrder"), "overThenDown", StringComparison.Ordinal)
                ? PagePrintOrder.AcrossThenDown
                : PagePrintOrder.DownThenAcross,
            PrintAreas = printAreas,
            RepeatColumns = repeatColumns,
            RepeatRows = repeatRows,
            PrintsGrid = Xlsx.Flag(options, "gridLines"),
            PrintsHeadings = Xlsx.Flag(options, "headings"),
            CentresHorizontally = Xlsx.Flag(options, "horizontalCentered"),
            CentresVertically = Xlsx.Flag(options, "verticalCentered"),

            // firstPageNumber only counts when useFirstPageNumber says so, which is exactly how
            // Calc reads it (pagesettings.cxx:968); otherwise numbering continues.
            FirstPageNumber = Xlsx.Flag(setupElement, "useFirstPageNumber")
                ? Xlsx.Integer(setupElement, "firstPageNumber") ?? 1
                : 0,
            ManualColumnBreaks = Breaks(Xlsx.Child(worksheet, "colBreaks")),
            ManualRowBreaks = Breaks(Xlsx.Child(worksheet, "rowBreaks")),
        };

        return (setup, ReadGrid(worksheet));
    }

    /// <summary>
    /// Which scaling mode the sheet uses, and the numbers that go with it.
    /// </summary>
    /// <remarks>
    /// <c>fitToWidth</c> and <c>fitToHeight</c> sit on <c>pageSetup</c> and mean nothing on
    /// their own: they take effect only when <c>sheetPr/pageSetUpPr/@fitToPage</c> is set.
    /// LibreOffice calls that out — "for whatever reason, this flag is still stored separated
    /// from the page settings" (<c>sc/source/filter/oox/worksheetfragment.cxx:650</c>) — and it
    /// is a trap, because every workbook LibreOffice writes carries
    /// <c>fitToWidth="1" fitToHeight="1"</c> whether or not it is fitting to anything. Reading
    /// those without the flag turns every ordinary sheet into a one-page sheet.
    /// </remarks>
    private static (PrintScaleMode Mode, int Percentage, int Wide, int Tall) ReadScale(
        XElement worksheet, XElement? setup)
    {
        XElement? sheetProperties = Xlsx.Child(worksheet, "sheetPr");
        bool fits = Xlsx.Flag(Xlsx.Child(sheetProperties, "pageSetUpPr"), "fitToPage");

        if (!fits)
        {
            int scale = Xlsx.Integer(setup, "scale") ?? 100;
            return (PrintScaleMode.Percentage, scale > 0 ? scale : 100, 0, 0);
        }

        return (PrintScaleMode.FitToPages,
                100,
                Math.Max(0, Xlsx.Integer(setup, "fitToWidth") ?? 1),
                Math.Max(0, Xlsx.Integer(setup, "fitToHeight") ?? 1));
    }

    /// <summary>The paper size, from the <c>paperSize</c> index or an explicit measure.</summary>
    private static DocSize PaperSize(XElement? setup, bool landscape)
    {
        Length width;
        Length height;

        Length? statedWidth = Measure(Xlsx.Attribute(setup, "paperWidth"));
        Length? statedHeight = Measure(Xlsx.Attribute(setup, "paperHeight"));

        if (statedWidth is { } explicitWidth && statedHeight is { } explicitHeight)
        {
            width = explicitWidth;
            height = explicitHeight;
        }
        else
        {
            // Index 9 is A4 and index 1 is Letter; the default is Letter, which is what Excel
            // writes for an American workbook and what the OOXML importer defaults to
            // (pagesettings.cxx:103, mnPaperSize(1)).
            (width, height) = ExcelPaperSizes.Portrait(Xlsx.Integer(setup, "paperSize") ?? 1);
        }

        return landscape ? new DocSize(height, width) : new DocSize(width, height);
    }

    /// <summary>
    /// The column widths and row heights, as runs.
    /// </summary>
    /// <remarks>
    /// <c>&lt;col&gt;</c> carries <c>min</c> and <c>max</c> and so is already a run; a
    /// <c>&lt;row&gt;</c> carries one row, but a sheet only writes a <c>&lt;row&gt;</c> element
    /// for a row that holds something, and the rest take <c>defaultRowHeight</c>. So neither
    /// axis needs expanding and the empty remainder of the sheet costs nothing.
    /// </remarks>
    private static SheetGrid ReadGrid(XElement worksheet)
    {
        XElement? format = Xlsx.Child(worksheet, "sheetFormatPr");

        Length defaultWidth = Digits(Xlsx.Attribute(format, "defaultColWidth"))
                              ?? SheetGrid.StandardColumnWidth;
        Length defaultHeight = Points(Xlsx.Attribute(format, "defaultRowHeight"))
                               ?? SheetGrid.StandardRowHeight;

        List<SheetSizeRun> columns = [];
        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            int min = Xlsx.Integer(column, "min") ?? 1;
            int max = Xlsx.Integer(column, "max") ?? min;
            if (max < min) continue;

            // A column that states no width takes the sheet default; one that is only hidden
            // still needs a run, so that the hidden flag survives.
            Length width = Digits(Xlsx.Attribute(column, "width")) ?? defaultWidth;
            columns.Add(new SheetSizeRun(min - 1, max - 1, width, Xlsx.Flag(column, "hidden")));
        }

        List<SheetSizeRun> rows = [];
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int index = Xlsx.Integer(row, "r") ?? 0;
            if (index <= 0) continue;

            bool hidden = Xlsx.Flag(row, "hidden");
            Length? height = Points(Xlsx.Attribute(row, "ht"));
            if (height is null && !hidden) continue;

            // customHeight is the flag that says the height came from a user rather than from
            // the writer's own measurement, and LibreOffice writes it explicitly false on every
            // ordinary row.
            rows.Add(new SheetSizeRun(
                index - 1, index - 1, height ?? defaultHeight, hidden,
                !Xlsx.Flag(row, "customHeight")));
        }

        return new SheetGrid(
            new SheetAxis(defaultWidth, columns), new SheetAxis(defaultHeight, rows));
    }

    /// <summary>A <c>rowBreaks</c> or <c>colBreaks</c> element's manual breaks.</summary>
    /// <remarks>
    /// The <c>man</c> attribute distinguishes the author's own breaks from the automatic ones
    /// Excel records alongside them, and only the author's are honoured — the automatic ones are
    /// Excel's pagination, which is the very thing being recomputed here.
    /// </remarks>
    private static List<int> Breaks(XElement? element)
    {
        List<int> breaks = [];
        foreach (XElement brk in Xlsx.Children(element, "brk"))
        {
            if (!Xlsx.Flag(brk, "man")) continue;

            int at = Xlsx.Integer(brk, "id") ?? -1;
            if (at > 0) breaks.Add(at);
        }
        return breaks;
    }

    /// <summary>A column width stated in digits of the default font.</summary>
    private static Length? Digits(string? value)
    {
        double? digits = Xlsx.Double(value);
        return digits is { } count && count > 0
            ? Length.FromTwips((long)Math.Round(count * DigitWidthTwips))
            : null;
    }

    private static Length? Points(string? value)
    {
        double? points = Xlsx.Double(value);
        return points is { } measure && measure >= 0 ? Length.FromPoints(measure) : null;
    }

    /// <summary>An explicit paper dimension, which carries its unit as a suffix.</summary>
    private static Length? Measure(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;

        string text = value.Trim();
        int digits = text.Length;
        while (digits > 0 && !char.IsAsciiDigit(text[digits - 1]) && text[digits - 1] != '.') digits--;

        if (!double.TryParse(text[..digits], NumberStyles.Float, CultureInfo.InvariantCulture,
                             out double number))
        {
            return null;
        }

        return text[digits..].Trim() switch
        {
            "in" => Length.FromInches(number),
            "cm" => Length.FromMillimetres(number * 10),
            "mm" => Length.FromMillimetres(number),
            "pt" or "" => Length.FromPoints(number),
            "pc" => Length.FromPoints(number * 12),
            _ => null,
        };
    }

    private static double Inches(XElement? element, string attribute, double fallback)
        => Xlsx.Double(Xlsx.Attribute(element, attribute)) ?? fallback;
}
