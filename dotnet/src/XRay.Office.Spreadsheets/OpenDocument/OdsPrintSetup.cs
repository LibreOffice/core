using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// Reads a sheet's print setup and geometry out of an ODF spreadsheet.
/// </summary>
/// <remarks>
/// <para>
/// ODF is the one of the three formats that states Calc's model directly, because Calc wrote
/// it: <c>fo:margin-top</c> on the page layout is the distance to the top of the header, the
/// <c>style:header-style</c> carries the header's height and the gap below it, and both go into
/// <see cref="SheetPrintSetup"/> unchanged. The other two formats state a single top margin and
/// have to be converted.
/// </para>
/// <para>
/// What a page layout leaves out matters as much as what it puts in, and the corpus file is the
/// example: <c>sheet-features.ods</c>'s <c>style:page-layout</c> declares nothing but a writing
/// mode and its two bands. There is no page size and no margin anywhere in the file, and
/// LibreOffice still renders it on A4 with two-centimetre margins — because those are what
/// <c>ScStyleSheet::GetItemSet</c> puts in a page style before any file is read
/// (<c>sc/source/core/data/stlsheet.cxx:170-200</c>). A reader that treats an absent
/// <c>fo:margin-left</c> as zero widens the page by four centimetres and loses a page break.
/// </para>
/// </remarks>
internal static class OdsPrintSetup
{
    private static readonly XName TableColumn = XName.Get("table-column", OdfNamespaces.Table);
    private static readonly XName TableRow = XName.Get("table-row", OdfNamespaces.Table);
    private static readonly XName HeaderColumns = XName.Get("table-header-columns", OdfNamespaces.Table);
    private static readonly XName HeaderRows = XName.Get("table-header-rows", OdfNamespaces.Table);
    private static readonly XNamespace TableNamespace = OdfNamespaces.Table;

    /// <summary>Builds a sheet's layout input from its <c>table:table</c> element.</summary>
    /// <param name="file">The open document, for its styles.</param>
    /// <param name="table">The sheet's element.</param>
    public static (SheetPrintSetup Setup, SheetGrid Grid) Read(OdfFile file, XElement table)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(table);

        SheetPrintSetup geometry = ReadGeometry(file, table);
        Axes axes = ReadAxes(file.Styles, table);

        SheetPrintSetup setup = geometry with
        {
            PrintAreas = ReadPrintAreas(table),
            PrintsEntireSheet =
                !string.Equals(Attribute(table, "print"), "false", StringComparison.Ordinal),
            RepeatColumns = axes.RepeatColumns,
            RepeatRows = axes.RepeatRows,
            ManualColumnBreaks = axes.ColumnBreaks,
            ManualRowBreaks = axes.RowBreaks,
        };

        return (setup, new SheetGrid(
            new SheetAxis(SheetGrid.StandardColumnWidth, axes.Columns),
            new SheetAxis(SheetGrid.StandardRowHeight, axes.Rows)));
    }

    private static SheetPrintSetup ReadGeometry(OdfFile file, XElement table)
    {
        // A sheet reaches its page layout through its table style's master page, which is the
        // one indirection ODF adds over the other two formats: table:style-name names a
        // style:style whose style:master-page-name names a master, whose page layout holds the
        // geometry. Anything missing falls back to Calc's own page-style defaults.
        OdfStyle? tableStyle = file.Styles.Find(Attribute(table, "style-name"), OdfStyleFamily.Table);
        OdfMasterPage? master = file.Styles.FindMasterPage(tableStyle?.MasterPageName)
                                ?? file.Styles.FindMasterPage("Default")
                                ?? file.Styles.MasterPages.Values.FirstOrDefault();

        OdfStyle? layout = file.Styles.FindPageLayout(master?.PageLayoutName);
        OdfPropertySet? page = layout?.Properties(OdfPropertyKind.PageLayout);

        SheetPrintSetup setup = SheetPrintSetup.Default;

        // The band's own text has to be read before its height can be, because the height is
        // `max(declared, text + gap)` — see `Band` below. Calc maximises over the shared band and
        // the left- and first-page ones, so all three are read where the master distinguishes
        // them (`UpdateHFHeight`, sc/source/ui/view/printfun.cxx:817-836).
        SheetDefaultFont bandFont = OdsCellFormats.DefaultFont(file.Styles);
        SheetHeaderFooter? header = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.Header));
        SheetHeaderFooter? footer = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.Footer));
        SheetHeaderFooter? leftHeader = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.LeftHeader));
        SheetHeaderFooter? leftFooter = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.LeftFooter));
        SheetHeaderFooter? firstHeader = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.FirstHeader));
        SheetHeaderFooter? firstFooter = OdsCellDecoration.ReadBand(file.Styles, Displayed(master?.FirstFooter));

        Length? width = Measure(page, OdfNamespaces.FoCompatible, "page-width");
        Length? height = Measure(page, OdfNamespaces.FoCompatible, "page-height");
        if (width is { } w && height is { } h && w > Length.Zero && h > Length.Zero)
            setup = setup with { PageSize = new DocSize(w, h) };

        // "style:print" lists what is printed as space-separated words, so an absent attribute
        // is not an absent feature: Calc omits it on a sheet printing its defaults, which
        // include neither the grid nor the row and column headings.
        string[] prints = (Get(page, OdfNamespaces.Style, "print") ?? string.Empty)
            .Split(' ', StringSplitOptions.RemoveEmptyEntries);

        // "table-centering", with the American spelling, because that is the attribute's name in
        // ODF and an attribute name is not ours to anglicise. This file's convention is British
        // everywhere it names something of our own -- `centring`, `CentresHorizontally`,
        // `SheetHorizontalAlignment.Centre` -- and the convention leaked one word too far: asking
        // for `style:table-centring` matched nothing in any `.ods` ever read, so
        // `CentresHorizontally` and `CentresVertically` were never once true.
        //
        // [src] `xmloff/source/core/xmltoken.cxx`:1992 interns the token, and
        // `PageMasterStyleMap.cxx`:97-98 maps the one attribute onto BOTH `PROP_CenterHorizontally`
        // and `PROP_CenterVertically` with `MID_FLAG_MERGE_ATTRIBUTE`, its value being
        // `horizontal`, `vertical` or `both`. [bin] 81 of the 307 converted `.ods` state it, on 236
        // rendered tables; the specification's spelling appears in the corpus 175 times and the
        // British one not once. See `probes/invcol-r149/results.md`.
        string? centring = Get(page, OdfNamespaces.Style, "table-centering");
        (PrintScaleMode mode, int percentage, int count, int wide, int tall) = ReadScale(page);

        return setup with
        {
            IsLandscape = string.Equals(
                Get(page, OdfNamespaces.Style, "print-orientation"), "landscape",
                StringComparison.Ordinal),
            LeftMargin = Margin(page, "margin-left") ?? setup.LeftMargin,
            RightMargin = Margin(page, "margin-right") ?? setup.RightMargin,
            TopMargin = Margin(page, "margin-top") ?? setup.TopMargin,
            BottomMargin = Margin(page, "margin-bottom") ?? setup.BottomMargin,
            HeaderHeight = BandHeight(
                layout?.HeaderProperties, master?.Header, "margin-bottom", bandFont,
                header, leftHeader, firstHeader),
            FooterHeight = BandHeight(
                layout?.FooterProperties, master?.Footer, "margin-top", bandFont,
                footer, leftFooter, firstFooter),
            HeaderGap = BandGap(layout?.HeaderProperties, master?.Header, "margin-bottom"),
            FooterGap = BandGap(layout?.FooterProperties, master?.Footer, "margin-top"),
            HeaderLeftMargin = BandMargin(layout?.HeaderProperties, page, "margin-left"),
            HeaderRightMargin = BandMargin(layout?.HeaderProperties, page, "margin-right"),
            FooterLeftMargin = BandMargin(layout?.FooterProperties, page, "margin-left"),
            FooterRightMargin = BandMargin(layout?.FooterProperties, page, "margin-right"),
            PageOrder = string.Equals(
                Get(page, OdfNamespaces.Style, "print-page-order"), "ltr", StringComparison.Ordinal)
                ? PagePrintOrder.AcrossThenDown
                : PagePrintOrder.DownThenAcross,
            HeaderText = Displayed(master?.Header)?.Value,
            FooterText = Displayed(master?.Footer)?.Value,
            Header = header,
            Footer = footer,
            BandFont = bandFont,
            PrintsGrid = prints.Contains("grid"),
            PrintsHeadings = prints.Contains("headers"),

            // "annotations" is Calc's ATTR_PAGE_NOTES — the notes listed on pages of their own
            // after the sheet, not a caption drawn on it. `PROP_PrintAnnotations` is mapped from
            // this very token (`xmloff/source/style/PageMasterStyleMap.cxx`:80 through
            // `XMLPMPropHdl_Print(XML_ANNOTATIONS)`, `PageMasterPropHdlFactory.cxx`:85) and
            // `ScPrintFunc` reads the item into `aTableParam.bNotes` (`printfun.cxx`:944).
            // Two of the 307 converted `.ods` state it, and both are `Hazard Analysis
            // Template.ods` and `RMP 2011-2014 and Inventory.ods`, whose `.xls` originals state
            // the BIFF flag this tree has honoured since round 56.
            PrintsNotes = prints.Contains("annotations"),
            CentresHorizontally = centring is "horizontal" or "both",
            CentresVertically = centring is "vertical" or "both",

            // style:first-page-number is either a number or the word "continue", and the word is
            // the common case — so anything unparseable reads as "continue" rather than as one.
            FirstPageNumber = Integer(Get(page, OdfNamespaces.Style, "first-page-number")) ?? 0,
            ScaleMode = mode,
            ScalePercentage = percentage,
            FitToPageCount = count,
            FitToPagesWide = wide,
            FitToPagesTall = tall,
        };
    }

    /// <summary>
    /// The whole height of a header or footer band, or zero when the master page has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>max(fo:min-height, textHeight + gap)</c>, which is Calc's own
    /// <c>max(nManHeight, nMaxHeight + nDistance)</c> stated in ODF's own attributes —
    /// <c>ScPrintFunc::UpdateHFHeight</c>, <c>sc/source/ui/view/printfun.cxx:838</c> and
    /// <c>:848-849</c>. <see cref="SheetBandHeight.Dynamic"/> carries the rule, the citations for
    /// each of its three terms, and the probe family that establishes the floor on the text.
    /// </para>
    /// <para>
    /// <strong>This read the declared height alone for eight rounds</strong>, on the reasoning
    /// that the gap is already inside it. It is — <c>aHdr.nDistance</c> is subtracted from
    /// <c>aHdr.nHeight</c> to get the rectangle the text is laid out in (<c>PrintHF</c>,
    /// <c>printfun.cxx:1807</c>) — but only after <c>UpdateHFHeight</c> has grown the band to
    /// hold both. An <c>.ods</c> Calc converted from a workbook carries the workbook's own header
    /// margin as the gap, so the pair is routinely <c>fo:min-height="0.2953in"</c> (21.26 pt)
    /// with <c>fo:margin-bottom="0.361in"</c> (25.99 pt) — a gap larger than the whole declared
    /// band. Censused over the 307 converted <c>.ods</c> (<c>probes/ods-band-r75/hfcensus.py</c>):
    /// <strong>13</strong> declare a band whose gap alone exceeds it, which must grow whatever
    /// its text is, and <strong>79</strong> declare one under <c>gap + one 11.5 pt line</c>, which
    /// grows for any ordinary one-line header. On <c>activespecs.ods</c> that is 37.66 pt of band
    /// against the 21.25 read here, which is 16.4 pt of body on every one of 266 pages.
    /// </para>
    /// <para>
    /// <strong><c>svg:height</c> is the fixed case and must not grow.</strong> The two spellings
    /// are one property with two special items: <c>svg:height</c> fills
    /// <c>HeaderIsDynamicHeight</c> with false and <c>fo:min-height</c> with true
    /// (<c>XMLPageMasterPropSetMapper</c>'s <c>finished</c>,
    /// <c>xmloff/source/style/PageMasterImportPropMapper.cxx:324-330</c>), that property is
    /// <c>ATTR_PAGE_DYNAMIC</c> (<c>sc/source/ui/unoobj/styleuno.cxx:341-343</c>), and
    /// <c>UpdateHFHeight</c> returns before it measures anything when the flag is off
    /// (<c>printfun.cxx:793</c>). Measured on three <c>svg:height</c> probes at 0.2, 0.75 and
    /// 1.5 cm with a 0.25 cm gap: the band is the declared height at all three, where the same
    /// heights written as <c>fo:min-height</c> grow two of them.
    /// </para>
    /// <para>
    /// <see cref="SheetPrintSetup.HeaderIsDynamic"/> stays false either way, and that is
    /// deliberate rather than an omission. What that flag decides here is whether the text
    /// rectangle is clamped to the text, which is a shortcut for "a dynamic band has
    /// <c>nDif == 0</c>" — true only while the text term wins. Once the band is
    /// <c>max(declared, text + gap)</c> the rectangle <see cref="SheetPrintSetup.HeaderHeight"/>
    /// less <see cref="SheetPrintSetup.HeaderGap"/> already <em>is</em> <c>PrintHF</c>'s paper
    /// height, and the text is centred in it whichever term won. Seven probes varying only
    /// <c>fo:min-height</c> put the reference's header ink at
    /// <c>top + (declared - gap - text) / 2</c> to 0.01 pt.
    /// </para>
    /// </remarks>
    /// <param name="properties">The <c>style:header-footer-properties</c> for the band.</param>
    /// <param name="content">The master page's band, for whether it is displayed at all.</param>
    /// <param name="gap">Which margin is the gap: the header's bottom, the footer's top.</param>
    /// <param name="bandFont">The workbook's default cell font.</param>
    /// <param name="bands">The band as the shared, left and first page states it.</param>
    private static Length BandHeight(
        OdfPropertySet? properties,
        XElement? content,
        string gap,
        SheetDefaultFont bandFont,
        params SheetHeaderFooter?[] bands)
    {
        if (properties is null || !IsDisplayed(content)) return Length.Zero;

        if (Measure(properties, OdfNamespaces.SvgCompatible, "height") is { } fixedHeight)
            return fixedHeight;

        return SheetBandHeight.Dynamic(
            Measure(properties, OdfNamespaces.FoCompatible, "min-height") ?? Length.Zero,
            Measure(properties, OdfNamespaces.FoCompatible, gap) ?? Length.Zero,
            bandFont,
            bands);
    }

    /// <summary>The gap inside the band between its text and the body.</summary>
    private static Length BandGap(OdfPropertySet? properties, XElement? content, string gap)
        => properties is null || !IsDisplayed(content)
            ? Length.Zero
            : Measure(properties, OdfNamespaces.FoCompatible, gap) ?? Length.Zero;

    /// <summary>
    /// A band's own left or right margin, which falls back to the page's rather than to zero.
    /// </summary>
    /// <remarks>
    /// The fallback is not a guess: a <c>style:header-footer-properties</c> that states no
    /// <c>fo:margin-left</c> leaves Calc's <c>ATTR_LRSPACE</c> inherited from the page style, so
    /// <c>rParam.nLeft</c> comes out as the page's own left margin and the header is indented
    /// twice. Measured on <c>sheet-decor-ods.ods</c>, whose header states no margins: the header
    /// text is clipped to 113.4 pt … 481.85 pt on a page whose margins are 56.7 pt — two
    /// centimetres of page margin and two more the header inherited.
    /// </remarks>
    private static Length BandMargin(OdfPropertySet? properties, OdfPropertySet? page, string name)
    {
        if (properties is null) return Length.Zero;

        return Measure(properties, OdfNamespaces.FoCompatible, name)
               ?? Margin(page, name)
               ?? Length.Zero;
    }

    /// <summary>
    /// True when a master page's band is switched on.
    /// </summary>
    /// <remarks>
    /// <c>style:display="false"</c> is how a master page switches a band off while keeping the
    /// element, so the element's presence is not enough — and a page layout is free to keep
    /// declaring a height for a header that is not printed.
    /// </remarks>
    private static bool IsDisplayed(XElement? content)
        => content is not null
           && !string.Equals(
               content.Attribute(XName.Get("display", OdfNamespaces.Style))?.Value, "false",
               StringComparison.Ordinal);

    private static (PrintScaleMode Mode, int Percentage, int Count, int Wide, int Tall) ReadScale(
        OdfPropertySet? page)
    {
        int count = Integer(Get(page, OdfNamespaces.Style, "scale-to-pages")) ?? 0;
        int wide = Integer(Get(page, OdfNamespaces.Style, "scale-to-X")) ?? 0;
        int tall = Integer(Get(page, OdfNamespaces.Style, "scale-to-Y")) ?? 0;
        int percentage = Integer(Get(page, OdfNamespaces.Style, "scale-to")?.TrimEnd('%')) ?? 100;

        PrintScaleMode mode = count > 0
            ? PrintScaleMode.FitToPageCount
            : wide > 0 || tall > 0
                ? PrintScaleMode.FitToPages
                : PrintScaleMode.Percentage;

        return (mode, percentage, count, wide, tall);
    }

    /// <summary>
    /// The last row of an ODF sheet whose automatic height Calc recomputes on load.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An ODF sheet recalculates its automatic row heights for its first two hundred rows
    /// and keeps the writer's own measurement for the rest.</strong>
    /// <c>ScXMLTableRowContext::endFastElement</c> adds each row block to the recalculation ranges
    /// and then takes it straight back out again when the block ends past row 200 and its style
    /// carries a stored height alongside the optimal flag — <c>maRanges.setFalse(nFirstRow,
    /// nCurrentRow)</c>, <c>sc/source/filter/xml/xmlrowi.cxx</c>:215-244 and the test itself at
    /// <c>:228</c>, under the comment <em>"recalc only the first 200 row in case of optimal
    /// document loading"</em>. The rows are
    /// zero-based there: <c>nFirstRow = nCurrentRow − nRepeatedRows + 1</c> and both are clamped
    /// against <c>pDoc-&gt;MaxRow()</c>.
    /// </para>
    /// <para>
    /// <strong>The test reads as "a style with no height", and it means the opposite.</strong>
    /// <c>ScXMLRowImportPropertyMapper::finished</c> (<c>sc/source/filter/xml/xmlstyli.cxx</c>:245-258)
    /// moves a stored <c>style:row-height</c> <em>into</em> the optimal-height property and clears
    /// the height one whenever <c>style:use-optimal-row-height</c> is true, so
    /// <c>FindProperty(CTF_SC_ROWHEIGHT)</c> is null for exactly the styles that state both — and
    /// the <c>CTF_SC_ROWOPTIMALHEIGHT</c> it then reads is the height, which <c>any2bool</c> takes
    /// as true whenever it is non-zero. A style stating the flag and <em>no</em> height has both
    /// properties cleared, is not found either, and falls through to <c>setTrue</c>: it is
    /// recalculated wherever it sits.
    /// </para>
    /// <para>
    /// <strong>It is worth twenty pages on one corpus document and the boundary is visible in the
    /// reference's own PDF.</strong> Every row of
    /// <c>Laser Report 2024 FOIA __Oct (1).ods</c> states <c>style:row-height="0.2189in"</c> with
    /// the optimal flag, one <c>table:table-row</c> element each. 26.2.4.2 draws its first 201
    /// rows on a 15.0 pt pitch — the recomputed height of their 10 pt text — and every row after
    /// them on the stated 15.75, which is 48 rows to a page and then 46; we recomputed all of
    /// them and printed 486 pages against 506. <c>dotnet/probes/ods-resid-r80/</c>.
    /// </para>
    /// <para>
    /// The rule is the <em>importer's</em> and belongs here rather than in
    /// <see cref="Layout.SheetOptimalRowHeights"/>, which serves all four readers: the two Excel
    /// filters recalculate every automatic row and have no such limit.
    /// </para>
    /// </remarks>
    private const int RecalculatedRowLimit = 200;

    private readonly record struct Axes(
        List<SheetSizeRun> Columns,
        List<SheetSizeRun> Rows,
        List<int> ColumnBreaks,
        List<int> RowBreaks,
        SheetRange? RepeatColumns,
        SheetRange? RepeatRows);

    /// <summary>
    /// Walks the sheet's columns and rows, keeping the repeat counts as runs.
    /// </summary>
    /// <remarks>
    /// As runs rather than as entries, which is the whole point: ODF pads a sheet with a single
    /// <c>table:table-column</c> carrying <c>table:number-columns-repeated="16384"</c> and a row
    /// element repeated a million times, so expanding them per column or per row would
    /// materialise the empty sheet the compression exists to avoid.
    /// </remarks>
    private static Axes ReadAxes(OdfStyles styles, XElement table)
    {
        List<SheetSizeRun> columns = [];
        List<SheetSizeRun> rows = [];
        List<int> columnBreaks = [];
        List<int> rowBreaks = [];
        SheetRange? repeatColumns = null;
        SheetRange? repeatRows = null;

        int column = 0;
        int row = 0;
        Walk(table, false, false);

        return new Axes(columns, rows, columnBreaks, rowBreaks, repeatColumns, repeatRows);

        void Walk(XElement parent, bool inHeaderColumns, bool inHeaderRows)
        {
            foreach (XElement child in parent.Elements())
            {
                if (child.Name == TableColumn)
                {
                    int first = column;
                    column += Math.Max(1, Repeated(child, "number-columns-repeated"));

                    Describe(child, OdfStyleFamily.TableColumn, OdfPropertyKind.TableColumn,
                             "column-width", out Length? size, out bool hidden, out bool breaks,
                             out _);

                    if (size is { } wide)
                        columns.Add(new SheetSizeRun(first, column - 1, wide, hidden));
                    else if (hidden)
                        columns.Add(new SheetSizeRun(
                            first, column - 1, SheetGrid.StandardColumnWidth, true));

                    if (breaks && first > 0) columnBreaks.Add(first);
                    if (inHeaderColumns)
                        repeatColumns = ExtendColumns(repeatColumns, first, column - 1);
                }
                else if (child.Name == TableRow)
                {
                    int first = row;
                    row += Math.Max(1, Repeated(child, "number-rows-repeated"));

                    Describe(child, OdfStyleFamily.TableRow, OdfPropertyKind.TableRow,
                             "row-height", out Length? size, out bool hidden, out bool breaks,
                             out bool optimal);

                    // The 200-row rule: below it the stored height is a cache Calc throws away,
                    // and past it the stored height stands. See `RecalculatedRowLimit`.
                    if (size is not null && row - 1 > RecalculatedRowLimit) optimal = false;

                    if (size is { } tall)
                        rows.Add(new SheetSizeRun(first, row - 1, tall, hidden, optimal));
                    else if (hidden)
                        rows.Add(new SheetSizeRun(
                            first, row - 1, SheetGrid.StandardRowHeight, true, optimal));

                    if (breaks && first > 0) rowBreaks.Add(first);
                    if (inHeaderRows) repeatRows = ExtendRows(repeatRows, first, row - 1);
                }
                else if (child.Name.Namespace == TableNamespace)
                {
                    // table:table-header-rows, table:table-row-group and their column
                    // equivalents wrap rows and columns without interrupting the numbering, so
                    // they are descended into rather than counted.
                    Walk(child,
                         inHeaderColumns || child.Name == HeaderColumns,
                         inHeaderRows || child.Name == HeaderRows);
                }
            }
        }

        void Describe(
            XElement element,
            OdfStyleFamily family,
            OdfPropertyKind kind,
            string sizeProperty,
            out Length? size,
            out bool hidden,
            out bool breaks,
            out bool optimal)
        {
            OdfPropertySet? properties =
                styles.Find(Attribute(element, "style-name"), family)?.Properties(kind);

            size = Measure(properties, OdfNamespaces.Style, sizeProperty);

            // style:use-optimal-row-height says the height in the file is Calc's own measurement,
            // so Calc recomputes it on load and the stored value is a cache. Absent means manual,
            // which is the safe reading: every row LibreOffice writes states it either way.
            optimal = string.Equals(
                Get(properties, OdfNamespaces.Style, $"use-optimal-{sizeProperty}"), "true",
                StringComparison.Ordinal);

            // "collapse" is a row or column the user hid and "filter" one an autofilter hid;
            // neither prints, and Calc's break loop treats them alike.
            hidden = Attribute(element, "visibility") is "collapse" or "filter";

            breaks = string.Equals(
                Get(properties, OdfNamespaces.FoCompatible, "break-before"), "page",
                StringComparison.Ordinal);
        }

        static SheetRange ExtendColumns(SheetRange? current, int first, int last)
            => current is { } range
                ? range with { LastColumn = last }
                : new SheetRange(first, 0, last, 0);

        static SheetRange ExtendRows(SheetRange? current, int first, int last)
            => current is { } range
                ? range with { LastRow = last }
                : new SheetRange(0, first, 0, last);
    }

    /// <summary>Reads <c>table:print-ranges</c>, which is a space-separated list.</summary>
    /// <remarks>
    /// <para>
    /// In the OOO reference syntax, so each range reads <c>Sheet.$A$1:$D$7</c> with the sheet
    /// name and the dollars both optional. Only the cell part is used: a print range naming
    /// another sheet is not something Calc produces, and taking the addresses without checking
    /// the name is what makes a range written without one work.
    /// </para>
    /// <para>
    /// <strong>The separator is a space and a sheet name may hold one</strong>, which is why the
    /// split goes through <see cref="SheetAddress.SplitList"/> rather than
    /// <c>string.Split(' ')</c>. Calc's export quotes such a name — <c>'PROPS (Light)'.AS1</c> —
    /// and a naive split cuts it at the space, leaving fragments of which only the last
    /// <em>parses</em>, as the single cell the range ended at. That is silent: a valid range
    /// comes back, the sheet paginates to one page, and nothing reports a problem.
    /// </para>
    /// </remarks>
    private static List<SheetRange> ReadPrintAreas(XElement table)
    {
        List<SheetRange> areas = [];

        foreach (string range in SheetAddress.SplitList(Attribute(table, "print-ranges"), ' '))
        {
            if (SheetAddress.TryParseRange(range, out SheetRange parsed)) areas.Add(parsed);
        }

        return areas;
    }

    /// <summary>The element, or null when the master page has switched it off.</summary>
    private static XElement? Displayed(XElement? element)
        => element is null
           || string.Equals(
               element.Attribute(XName.Get("display", OdfNamespaces.Style))?.Value, "false",
               StringComparison.Ordinal)
            ? null
            : element;

    private static string? Attribute(XElement element, string localName)
        => element.Attribute(XName.Get(localName, OdfNamespaces.Table))?.Value;

    private static string? Get(OdfPropertySet? set, string ns, string localName)
        => set?.Get(ns, localName);

    private static Length? Measure(OdfPropertySet? set, string ns, string localName)
        => OdfValue.ParseLength(set?.Get(ns, localName));

    /// <summary>A page margin, honouring the <c>fo:margin</c> shorthand.</summary>
    private static Length? Margin(OdfPropertySet? page, string localName)
        => Measure(page, OdfNamespaces.FoCompatible, localName)
           ?? Measure(page, OdfNamespaces.FoCompatible, "margin");

    private static int Repeated(XElement element, string localName)
        => Integer(Attribute(element, localName)) ?? 1;

    private static int? Integer(string? value)
        => value is not null
           && int.TryParse(value.Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture,
                           out int parsed)
            ? parsed
            : null;
}
