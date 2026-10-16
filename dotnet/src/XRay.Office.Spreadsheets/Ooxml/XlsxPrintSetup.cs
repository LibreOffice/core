using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

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

    /// <summary>
    /// The grid a row height written by a Microsoft application is snapped down onto, in points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Excel stores a row height as a count of screen pixels and writes it out in points, so its
    /// numbers are only ever multiples of a pixel; LibreOffice takes the file at less than its
    /// word and rounds every one of them <em>down</em> to a multiple of 0.75 pt —
    /// <c>fHeight -= fmod(fHeight, 0.75)</c>, applied to <c>sheetFormatPr/@defaultRowHeight</c>
    /// (<c>sc/source/filter/oox/worksheetfragment.cxx:681-684</c>) and to every
    /// <c>row/@ht</c> (<c>sc/source/filter/oox/sheetdatacontext.cxx:316-319</c>).
    /// </para>
    /// <para>
    /// <strong>Both places are gated on <c>isMSODocument()</c></strong>, which is the generator
    /// string and nothing else, so the same bytes in the same sheet are read as two different
    /// heights depending on what <c>docProps/app.xml</c> says wrote them. Measured on two
    /// packages differing in that one element and in nothing else: <c>ht="18.6"</c> comes back
    /// 360 twips from the Microsoft copy and 372 from the other, <c>ht="29.4"</c> 585 against
    /// 588, and a sheet default of 14.4 gives 285 against 288.
    /// </para>
    /// <para>
    /// <strong>The rounding is the XML filter's, not the format's.</strong> BIFF12 states a row
    /// height in whole twips and its own <c>importRow</c> and <c>importSheetFormatPr</c> apply
    /// nothing (<c>sheetdatacontext.cxx:412-440</c>, <c>worksheetfragment.cxx:790-808</c>), so
    /// XLSB is left alone — see <c>XlsbPrintSetup</c>, which deliberately does not do this.
    /// </para>
    /// </remarks>
    private const double MicrosoftRowHeightGridPoints = 0.75;

    /// <summary>Builds a sheet's layout input from its <c>worksheet</c> element.</summary>
    /// <param name="worksheet">The worksheet part's root, or null when it did not load.</param>
    /// <param name="printAreas">The print areas the workbook's defined names gave this sheet.</param>
    /// <param name="repeatColumns">The repeated columns, from <c>_xlnm.Print_Titles</c>.</param>
    /// <param name="repeatRows">The repeated rows, from the same name.</param>
    /// <param name="defaultFont">
    /// The workbook's default font, which a column width is stated in digits of. Null falls back
    /// to Calc's own — see <see cref="SheetColumnDigits"/>.
    /// </param>
    /// <param name="isMicrosoftGenerated">
    /// Whether <c>docProps/app.xml</c> names a Microsoft application, which is what decides
    /// whether row heights are snapped onto <see cref="MicrosoftRowHeightGridPoints"/>.
    /// </param>
    public static (SheetPrintSetup Setup, SheetGrid Grid) Read(
        XElement? worksheet,
        IReadOnlyList<SheetRange> printAreas,
        SheetRange? repeatColumns,
        SheetRange? repeatRows,
        SheetDefaultFont? defaultFont = null,
        bool isMicrosoftGenerated = false)
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

        // `differentFirst` gives the first page its own pair. Reading the flag matters even when
        // the file supplies no first-page strings, which is the only shape this corpus holds: all
        // 49 workbooks that set it state no firstHeader or firstFooter, so their first page must
        // print bare. Calc keeps the same distinction in `mbShareFirst = !bUseFirstContent`
        // (sc/source/filter/oox/pagesettings.cxx:1019) rather than deriving it from the strings.
        bool differentFirst = Xlsx.Flag(headerFooter, "differentFirst");
        string? firstHeaderText = differentFirst
            ? Xlsx.Child(headerFooter, "firstHeader")?.Value
            : null;
        string? firstFooterText = differentFirst
            ? Xlsx.Child(headerFooter, "firstFooter")?.Value
            : null;

        // The band is sized from every variant that has content, not from the odd pair alone —
        // `orHFData.mnHeight = max(nOddHeight, nEvenHeight, nFirstHeight)` and `mbHasContent` is
        // the OR of the three (pagesettings.cxx:1017,1026). One height serves every page, which
        // is why giving the first page different ink moves no page boundary.
        bool hasHeader = !string.IsNullOrEmpty(headerText) || !string.IsNullOrEmpty(firstHeaderText);
        bool hasFooter = !string.IsNullOrEmpty(footerText) || !string.IsNullOrEmpty(firstFooterText);

        // Only a header with content occupies a band. Calc's "header is on" flag is set from
        // whether any of the three header strings is non-empty, not from the margin being
        // written (pagesettings.cxx:1003).
        //
        // The band the two margins imply is not the band that prints: Calc keeps the distance
        // between the text and the body and re-measures the text itself when it prints, so the
        // band grows by however much the real line height exceeds the bare point size. See
        // SheetBandHeight, which is the port.
        Length headerBand = hasHeader
            ? Taller(
                headerText, firstHeaderText, Length.FromInches(Math.Max(0, top - header)),
                defaultFont)
            : Length.Zero;
        Length footerBand = hasFooter
            ? Taller(
                footerText, firstFooterText, Length.FromInches(Math.Max(0, bottom - footer)),
                defaultFont)
            : Length.Zero;

        // A chart sheet is a sheet part like any other and is read through this reader, but three
        // of its print settings are decided by what it *is* rather than by what it states. See
        // `IsChartSheet`.
        bool chartSheet = IsChartSheet(worksheet);

        // A chart sheet is landscape unless the file names an orientation of its own, which is a
        // default rather than an override: one that says `portrait` gets portrait.
        string? orientation = Xlsx.Attribute(setupElement, "orientation");
        bool landscape = string.Equals(orientation, "landscape", StringComparison.Ordinal)
            || (chartSheet && !string.Equals(orientation, "portrait", StringComparison.Ordinal));

        (PrintScaleMode mode, int percentage, int wide, int tall) = chartSheet
            ? (PrintScaleMode.FitToPageCount, 100, 0, 0)
            : ReadScale(worksheet, setupElement);

        SheetPrintSetup setup = new()
        {
            PageSize = PaperSize(setupElement, landscape),
            IsLandscape = landscape,
            LeftMargin = Length.FromInches(left),
            RightMargin = Length.FromInches(right),

            // The body starts at the **page** margin whatever the band margin says, and the
            // `Math.Min` is what makes that true when a file states a `header` larger than its
            // `top` — a negative band. With a band of zero or more, `min(header, top) + max(0,
            // top - header)` is `top` either way and the clamp is inert; without it, a negative
            // band pushes the body down to the band margin.
            //
            // Measured on 26.2.4.2 (`probes/sheets-r55/audit_pagedecoration.py`): with
            // `top="0.75" header="1.00"` the reference starts the body at the top margin, exactly
            // where it starts it at every non-negative band, and we started it **18 pt** lower.
            // Two corpus worksheets state a negative band — `023_Waterfall_Chart_Template`'s
            // header at −3.6 pt and `2025_Active_Civil_Airmen_Statistics`' footer at −5.76 pt —
            // and **both render byte-identically with and without this clamp**, because neither
            // sheet's negative band is on a page whose body position the gate can see. So it is a
            // correctness fix with a measured mechanism and no corpus witness, which is worth
            // saying rather than implying.
            TopMargin = Length.FromInches(hasHeader ? Math.Min(header, top) : top),
            BottomMargin = Length.FromInches(hasFooter ? Math.Min(footer, bottom) : bottom),
            HeaderHeight = headerBand,
            FooterHeight = footerBand,
            // Calc's `nDistance`, and **zero when the band is pinned** — see
            // `SheetBandHeight.BodyDistance`. Setting this at all is the fix: leaving it at
            // `SheetPrintSetup`'s ODF default of 142 twips made `FooterHeight - FooterGap`
            // negative for every band under 7.1 pt, and `SheetPageDecoration.DrawBand` returns on
            // a negative text rectangle, so those bands were dropped with no ink and no words.
            // `XlsPrintSetup` has had the rule since it was written; this reader and the XLSB one
            // simply never called it.
            HeaderGap = hasHeader
                ? SheetBandHeight.BodyDistance(
                    headerText, Length.FromInches(Math.Max(0, top - header)), defaultFont,
                    SheetPrintSetup.Default.HeaderGap)
                : SheetPrintSetup.Default.HeaderGap,
            FooterGap = hasFooter
                ? SheetBandHeight.BodyDistance(
                    footerText, Length.FromInches(Math.Max(0, bottom - footer)), defaultFont,
                    SheetPrintSetup.Default.FooterGap)
                : SheetPrintSetup.Default.FooterGap,

            HeaderText = headerText,
            FooterText = footerText,

            // The band's own margins are zero here and not inherited, unlike ODF's: SpreadsheetML
            // states no header margin of its own, so the header runs the full width between the
            // page margins — measured at 56.7 pt to 538.55 pt on sheet-decor-xlsx.xlsx, exactly
            // the page's own margins, where the ODS twin indents by a further two centimetres.
            Header = headerText is null ? null : SheetHeaderFooter.ParseCodes(headerText),
            Footer = footerText is null ? null : SheetHeaderFooter.ParseCodes(footerText),
            FirstHeader = firstHeaderText is null
                ? null
                : SheetHeaderFooter.ParseCodes(firstHeaderText),
            FirstFooter = firstFooterText is null
                ? null
                : SheetHeaderFooter.ParseCodes(firstFooterText),
            DifferentFirstPage = differentFirst,

            // The face the band's own codes fall back to: the workbook's default cell font,
            // family and size. `SheetBandHeight` above is already given the same object to size
            // the band with; until round 56 the *drawing* used a fixed ten-point Liberation Sans
            // instead, so the two halves of the same band disagreed on every workbook whose
            // default is not that. See `SheetPrintSetup.BandFont`.
            BandFont = defaultFont,

            // Every Excel band is dynamic — see `SheetPrintSetup.HeaderIsDynamic`.
            HeaderIsDynamic = true,
            FooterIsDynamic = true,
            ScaleMode = mode,
            ScalePercentage = percentage,
            FitToPagesWide = wide,
            FitToPagesTall = tall,
            FitToPageCount = chartSheet ? 1 : 0,
            PageOrder = string.Equals(
                Xlsx.Attribute(setupElement, "pageOrder"), "overThenDown", StringComparison.Ordinal)
                ? PagePrintOrder.AcrossThenDown
                : PagePrintOrder.DownThenAcross,
            PrintAreas = printAreas,
            RepeatColumns = repeatColumns,
            RepeatRows = repeatRows,
            PrintsGrid = !chartSheet && Xlsx.Flag(options, "gridLines"),
            PrintsHeadings = !chartSheet && Xlsx.Flag(options, "headings"),

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

        return (setup, ReadGrid(worksheet, defaultFont, isMicrosoftGenerated));
    }

    /// <summary>Whether the part read is a chart sheet rather than a worksheet.</summary>
    /// <remarks>
    /// <para>
    /// A chart sheet is a whole sheet whose only content is one chart. Its part is
    /// <c>xl/chartsheets/sheetN.xml</c> with a <c>chartsheet</c> root, and it carries the same
    /// <c>pageMargins</c>, <c>pageSetup</c> and <c>drawing</c> children a worksheet does — which
    /// is why it comes through this reader unremarked and rendered, until now, as a worksheet
    /// with a very large drawing on it.
    /// </para>
    /// <para>
    /// <strong>Three of its print settings are decided by what it is, not by what it states</strong>
    /// (<c>PageSettingsConverter::writePageSettingsProperties</c>,
    /// <c>sc/source/filter/oox/pagesettings.cxx:905-972</c>):
    /// </para>
    /// <list type="bullet">
    ///   <item><description>
    ///     it is <em>always</em> scaled to fit exactly one page — <c>ScaleToPages = 1</c>, with
    ///     the comment "always fit chart sheet to 1 page" — whatever <c>scale</c> or
    ///     <c>fitToPage</c> say;
    ///   </description></item>
    ///   <item><description>
    ///     it defaults to landscape when the file names no orientation, rather than to portrait;
    ///   </description></item>
    ///   <item><description>
    ///     it never prints gridlines or row/column headings — "no gridlines in chart sheets".
    ///   </description></item>
    /// </list>
    /// <para>
    /// The first is the one with teeth. A chart whose <c>xdr:absoluteAnchor</c> extent, plus the
    /// overhang of its rotated category labels, exceeds the printable area spills onto a second
    /// page — and both of the corpus's two chart-sheet workbooks did exactly that, each printing
    /// one page more than the reference with the spill's stray glyphs on it.
    /// </para>
    /// <para>
    /// Detected from the root element rather than from the workbook relationship's type, because
    /// this reader is handed the loaded part and nothing else, and the root name is the same fact:
    /// a <c>chartsheet</c> part has a <c>chartsheet</c> root by schema.
    /// </para>
    /// </remarks>
    private static bool IsChartSheet(XElement worksheet)
        => string.Equals(worksheet.Name.LocalName, "chartsheet", StringComparison.Ordinal);

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
        // No page setup, or one that defers to the printer, leaves the application's own paper
        // standing — and the orientation with it. See ExcelPaperSizes.Default: measured,
        // `usePrinterDefaults="1"` with `orientation="landscape"` renders A4 portrait.
        if (setup is null || Xlsx.Flag(setup, "usePrinterDefaults")) return ExcelPaperSizes.Default;

        Length? statedWidth = Measure(Xlsx.Attribute(setup, "paperWidth"));
        Length? statedHeight = Measure(Xlsx.Attribute(setup, "paperHeight"));

        if (statedWidth is { } explicitWidth && statedHeight is { } explicitHeight)
        {
            // An explicit measure is always honoured, so the orientation applies to it.
            return landscape
                ? new DocSize(explicitHeight, explicitWidth)
                : new DocSize(explicitWidth, explicitHeight);
        }

        // Index 9 is A4 and index 1 is Letter; the default is Letter, which is what Excel
        // writes for an American workbook and what the OOXML importer defaults to
        // (pagesettings.cxx:103, mnPaperSize(1)). An index outside the table takes the
        // application's paper *unrotated* — see ExcelPaperSizes.Page.
        return ExcelPaperSizes.Page(Xlsx.Integer(setup, "paperSize") ?? 1, landscape);
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
    private static SheetGrid ReadGrid(
        XElement worksheet, SheetDefaultFont? defaultFont, bool isMicrosoftGenerated)
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
        Length? statedHeight = RowHeight(
            Xlsx.Attribute(format, "defaultRowHeight"), isMicrosoftGenerated);
        Length defaultHeight = statedHeight ?? SheetGrid.StandardRowHeight;

        List<SheetDigitRun> columns = [];
        List<SheetOutlineRun> columnOutline = [];
        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            int min = Xlsx.Integer(column, "min") ?? 1;
            int max = Xlsx.Integer(column, "max") ?? min;
            if (max < min) continue;

            // A column that states no width takes the sheet default; one that is only hidden
            // still needs a run, so that the hidden flag survives.
            SheetDigitWidth width = Digits(Xlsx.Attribute(column, "width")) ?? defaultWidth;
            columns.Add(new SheetDigitRun(min - 1, max - 1, width, Xlsx.Flag(column, "hidden")));
            SheetOutlineCollapse.Append(
                columnOutline, min - 1, max - 1,
                Xlsx.Integer(column, "outlineLevel") ?? 0, Xlsx.Flag(column, "collapsed"));
        }

        List<SheetSizeRun> rows = [];
        List<SheetOutlineRun> rowOutline = [];
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int index = Xlsx.Integer(row, "r") ?? 0;
            if (index <= 0) continue;

            SheetOutlineCollapse.Append(
                rowOutline, index - 1, index - 1,
                Xlsx.Integer(row, "outlineLevel") ?? 0, Xlsx.Flag(row, "collapsed"));

            bool hidden = Xlsx.Flag(row, "hidden");
            Length? height = RowHeight(Xlsx.Attribute(row, "ht"), isMicrosoftGenerated);
            if (height is null && !hidden) continue;

            // customHeight is the flag that says the height came from a user rather than from
            // the writer's own measurement, and LibreOffice writes it explicitly false on every
            // ordinary row.
            rows.Add(new SheetSizeRun(
                index - 1, index - 1, height ?? defaultHeight, hidden,
                !Xlsx.Flag(row, "customHeight")));
        }

        // A collapsed outline group hides its detail rows whether or not the part says so, which
        // is a derivation rather than a reading — see `SheetOutlineCollapse`.
        rows = SheetOutlineCollapse.Apply(
            rows, SheetOutlineCollapse.Hidden(rowOutline), defaultHeight);
        columns = SheetOutlineCollapse.Apply(
            columns, SheetOutlineCollapse.Hidden(columnOutline), defaultWidth);

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

    /// <summary>
    /// A row height in points, snapped down onto Excel's own pixel grid where Calc snaps it.
    /// </summary>
    /// <remarks>
    /// See <see cref="MicrosoftRowHeightGridPoints"/> for the two citations and the measurement.
    /// The subtraction is written as Calc's own — <c>fHeight -= fmod(fHeight, 0.75)</c> — to keep
    /// the correspondence readable, and <em>not</em> because it differs from a floor-divide.
    /// Checked rather than assumed: the two agree on every one of the 49142 heights Excel can
    /// write, being each hundredth of a point and each twip up to the 409.5 pt ceiling. So do not
    /// read the form as load-bearing, and do not write a test that claims it is.
    /// </remarks>
    private static Length? RowHeight(string? value, bool isMicrosoftGenerated)
    {
        double? points = Xlsx.Double(value);
        if (points is not { } measure || measure < 0) return null;

        // Calc guards the per-row rounding on a positive height and leaves an unstated sheet
        // default at zero, where the remainder is zero anyway; both come out the same here.
        if (isMicrosoftGenerated && measure > 0)
            measure -= measure % MicrosoftRowHeightGridPoints;

        return Length.FromPoints(measure);
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

    /// <summary>The taller of the two bands a variant needs, in Calc's own terms.</summary>
    /// <remarks>
    /// One height serves every page of the sheet, so the band has to fit whichever variant is
    /// tallest — <c>max(nOddHeight, nEvenHeight, nFirstHeight)</c> at
    /// <c>sc/source/filter/oox/pagesettings.cxx:1026</c>. A first-page header of three lines over
    /// an odd one of a single line reserves three lines on every page, blank ones included.
    /// </remarks>
    private static Length Taller(
        string? odd, string? first, Length available, SheetDefaultFont? defaultFont)
    {
        Length forOdd = string.IsNullOrEmpty(odd)
            ? Length.Zero
            : SheetBandHeight.Printed(odd, available, defaultFont);
        Length forFirst = string.IsNullOrEmpty(first)
            ? Length.Zero
            : SheetBandHeight.Printed(first, available, defaultFont);

        return forOdd > forFirst ? forOdd : forFirst;
    }

    private static double Inches(XElement? element, string attribute, double fallback)
        => Xlsx.Double(Xlsx.Attribute(element, attribute)) ?? fallback;
}
