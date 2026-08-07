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
/// <para>
/// That multiplication does not happen here. Measuring the face is layout's job and reading is
/// the extraction path, so what this reader produces is the digits and the font's <em>name</em>,
/// both free, and <see cref="SheetLayout.Grid"/> converts them. See
/// <see cref="SheetColumnDigits"/>.
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
    /// Half a twip, which turns <see cref="SheetDigitWidth"/>'s truncation into rounding.
    /// </summary>
    /// <remarks>
    /// SpreadsheetML's conversion is a plain multiplication by the digit width and LibreOffice
    /// rounds it (<c>std::round</c>, <c>WorksheetGlobals::convertColumns</c>,
    /// <c>sc/source/filter/oox/worksheethelper.cxx:1211</c>), where BIFF's subtracts half a twip
    /// and truncates. One truncation serves both once this is carried as the bias.
    /// </remarks>
    private const double RoundingBiasTwips = 0.5;

    /// <summary>
    /// The padding <c>baseColWidth</c> carries that <c>defaultColWidth</c> does not.
    /// </summary>
    /// <remarks>
    /// Five screen pixels, which <c>WorksheetGlobals::setBaseColumnWidth</c> adds with the comment
    /// <c>#i3006# add 5 pixels padding to the width</c>
    /// (<c>sc/source/filter/oox/worksheethelper.cxx:745-752</c>). It is added in
    /// <em>digits</em> there — <c>scaleValue(5, Unit::ScreenX, Unit::Digit)</c> — and multiplied
    /// back by the digit width afterwards, so in twips it is just the five pixels: a screen pixel
    /// is a ninety-sixth of an inch and therefore fifteen twips exactly. It does not scale with
    /// the font, which is why it is a bias rather than a count of digits.
    /// </remarks>
    private const double BasePaddingTwips = 75;

    /// <summary>The <c>baseColWidth</c> a sheet that states none is read as having.</summary>
    /// <remarks><c>rAttribs.getInteger(XML_baseColWidth, 8)</c>, <c>worksheetfragment.cxx:672</c>.</remarks>
    private const int DefaultBaseColumnWidth = 8;

    /// <summary>Builds a sheet's layout input from its <c>worksheet</c> element.</summary>
    /// <param name="worksheet">The worksheet part's root, or null when it did not load.</param>
    /// <param name="printAreas">The print areas the workbook's defined names gave this sheet.</param>
    /// <param name="repeatColumns">The repeated columns, from <c>_xlnm.Print_Titles</c>.</param>
    /// <param name="repeatRows">The repeated rows, from the same name.</param>
    /// <param name="defaultFont">
    /// The workbook's default font, which a column width is stated in digits of. Null falls back
    /// to Calc's own — see <see cref="SheetColumnDigits"/>.
    /// </param>
    public static (SheetPrintSetup Setup, SheetGrid Grid) Read(
        XElement? worksheet,
        IReadOnlyList<SheetRange> printAreas,
        SheetRange? repeatColumns,
        SheetRange? repeatRows,
        SheetDefaultFont? defaultFont = null)
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

            // `asDisplayed`, not `atEnd`, and that is not a slip. Calc has one mode — the notes
            // are listed after the sheet — so its OOXML filter has to pick which of the two
            // SpreadsheetML values turns it on, and it picks the other one:
            // `PROP_PrintAnnotations` is set from `mnCellComments == XML_asDisplayed`
            // (`sc/source/filter/oox/pagesettings.cxx:968`), where the BIFF filter sets the same
            // property from `EXC_SETUP_PRINTNOTES` and the BIFF12 path maps *both* non-`none`
            // values onto it (`:270`). Reading `atEnd` here instead would print pages the
            // reference does not. Neither value appears in the corpus, so this follows the
            // binary rather than a measurement.
            PrintsNotes = string.Equals(
                Xlsx.Attribute(setupElement, "cellComments"),
                "asDisplayed",
                StringComparison.Ordinal),
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

        return (setup, ReadGrid(worksheet, defaultFont));
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
    /// <remarks>
    /// <para>
    /// <strong>A sheet stating no <c>pageSetup</c> at all keeps the application's own paper, and
    /// that is not what the element's defaults say.</strong> <c>PageSettingsModel</c> initialises
    /// <c>mbValidSettings</c> to <em>true</em> (<c>pagesettings.cxx:117</c>) and only
    /// <c>importPageSetup</c> overwrites it, from <c>usePrinterDefaults</c>, which defaults to
    /// false (<c>:180</c>); and the paper size is written onto the page style only when
    /// <c>mbValidSettings</c> is false (<c>:934</c>). So an absent <c>pageSetup</c> leaves Calc's
    /// locale default standing and a present one applies the index — the opposite way round from
    /// reading <c>paperSize</c>'s own default of 1 whenever the attribute is missing, which puts
    /// every Excel workbook that states no page setup on Letter. Measured on
    /// <c>chart2/qa/extras/data/xlsx/</c>: LibreOffice renders all seven of its chart workbooks on
    /// A4 and this reader put them on Letter.
    /// </para>
    /// </remarks>
    private static DocSize PaperSize(XElement? setup, bool landscape)
    {
        if (setup is null || Xlsx.Flag(setup, "usePrinterDefaults"))
        {
            (Length defaultWidth, Length defaultHeight) = ExcelPaperSizes.A4;
            return landscape
                ? new DocSize(defaultHeight, defaultWidth)
                : new DocSize(defaultWidth, defaultHeight);
        }

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
    private static SheetGrid ReadGrid(XElement worksheet, SheetDefaultFont? defaultFont)
    {
        XElement? format = Xlsx.Child(worksheet, "sheetFormatPr");

        // **A sheet that states no defaultColWidth does not take Calc's own default.** Excel
        // writes `baseColWidth` instead — or nothing at all, which means 8 — and LibreOffice reads
        // it as that many digits plus five screen pixels of padding
        // (`setBaseColumnWidth`, `worksheethelper.cxx:745`), which is 963 twips against Calc's own
        // 1280. Every workbook LibreOffice writes states `defaultColWidth`, so this is invisible on
        // anything round-tripped through it and decides the page count of anything Excel wrote:
        // `chart2/qa/extras/data/xlsx/bubble_chart_simple.xlsx` fits ten columns to a Letter page
        // at 963 and seven at 1280, which is two pages against three.
        SheetDigitWidth defaultWidth = Digits(Xlsx.Attribute(format, "defaultColWidth"))
                                       ?? BaseWidth(Xlsx.Integer(format, "baseColWidth"));
        Length? statedHeight = Points(Xlsx.Attribute(format, "defaultRowHeight"));
        Length defaultHeight = statedHeight ?? SheetGrid.StandardRowHeight;

        List<SheetDigitRun> columns = [];
        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            int min = Xlsx.Integer(column, "min") ?? 1;
            int max = Xlsx.Integer(column, "max") ?? min;
            if (max < min) continue;

            // A column that states no width takes the sheet default; one that is only hidden
            // still needs a run, so that the hidden flag survives.
            SheetDigitWidth width = Digits(Xlsx.Attribute(column, "width")) ?? defaultWidth;
            columns.Add(new SheetDigitRun(min - 1, max - 1, width, Xlsx.Flag(column, "hidden")));
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

        SheetColumnDigits digits = new(defaultFont ?? SheetDefaultFont.Calc, defaultWidth, columns);

        // Materialised at the fallback so that the grid is complete the moment it is built, and
        // remeasured by `SheetLayout.Grid` once a face can be resolved.
        return new SheetGrid(
            digits.Resolve(SheetColumnDigits.FallbackDigitWidthTwips),
            new SheetAxis(defaultHeight, rows))
        {
            ColumnDigits = digits,

            // Only the OOXML filter tells the sheet what its recomputed rows may not go below,
            // and it tells it the sheet's own default row height —
            // `pTable->SetOptimalMinRowHeight(maDefRowModel.mfHeight * 20)`,
            // `sc/source/filter/oox/worksheethelper.cxx:965`. A sheet stating none leaves
            // `mfHeight` at 0, which `ScTable::GetOptimalMinRowHeight` reads as "not set" and
            // answers with Calc's own 256 twips.
            OptimalMinimumRowHeight = statedHeight ?? SheetGrid.StandardRowHeight,
        };
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
    private static SheetDigitWidth? Digits(string? value)
    {
        double? digits = Xlsx.Double(value);
        return digits is { } count && count > 0
            ? new SheetDigitWidth(count, RoundingBiasTwips)
            : null;
    }

    /// <summary>A column width stated as a <c>baseColWidth</c>, which carries padding.</summary>
    private static SheetDigitWidth BaseWidth(int? baseColumnWidth)
    {
        int digits = baseColumnWidth is { } stated && stated > 0 ? stated : DefaultBaseColumnWidth;
        return new SheetDigitWidth(digits, BasePaddingTwips + RoundingBiasTwips);
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
