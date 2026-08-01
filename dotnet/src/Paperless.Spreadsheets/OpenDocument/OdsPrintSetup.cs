using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.OpenDocument;

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

        Length? width = Measure(page, OdfNamespaces.FoCompatible, "page-width");
        Length? height = Measure(page, OdfNamespaces.FoCompatible, "page-height");
        if (width is { } w && height is { } h && w > Length.Zero && h > Length.Zero)
            setup = setup with { PageSize = new DocSize(w, h) };

        // "style:print" lists what is printed as space-separated words, so an absent attribute
        // is not an absent feature: Calc omits it on a sheet printing its defaults, which
        // include neither the grid nor the row and column headings.
        string[] prints = (Get(page, OdfNamespaces.Style, "print") ?? string.Empty)
            .Split(' ', StringSplitOptions.RemoveEmptyEntries);

        string? centring = Get(page, OdfNamespaces.Style, "table-centring");
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
            HeaderHeight = BandHeight(layout?.HeaderProperties, master?.Header),
            FooterHeight = BandHeight(layout?.FooterProperties, master?.Footer),
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
            Header = OdsCellDecoration.ReadBand(Displayed(master?.Header)),
            Footer = OdsCellDecoration.ReadBand(Displayed(master?.Footer)),
            PrintsGrid = prints.Contains("grid"),
            PrintsHeadings = prints.Contains("headers"),
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
    /// The declared height <em>alone</em>, because the gap to the body is already inside it.
    /// Calc's <c>aHdr.nHeight</c> comes straight from <c>ATTR_PAGE_SIZE</c> — the declared
    /// height — and <c>aHdr.nDistance</c> is subtracted from it to get the rectangle the text is
    /// laid out in (<c>lcl_FillHFParam</c>, <c>printfun.cxx:664</c>, and <c>PrintHF</c>,
    /// <c>printfun.cxx:1808</c>). Adding the gap on top double-counts it.
    /// </para>
    /// <para>
    /// Measured, that is a quarter of a centimetre — 7.09 pt — on every page of every file
    /// LibreOffice writes, since it writes <c>fo:min-height="0.75cm"</c> with
    /// <c>fo:margin-bottom="0.25cm"</c>. On <c>sheet-decor-ods.ods</c> LibreOffice puts the top
    /// of the first printed row 21.11 pt below the top margin, not 28.35 pt.
    /// </para>
    /// <para>
    /// The dynamic case is the one this still under-measures. <c>UpdateHFHeight</c> recomputes
    /// the band from the header's own text and takes the larger of that and the declared height
    /// (<c>printfun.cxx:846-856</c>), so a header whose single line is taller than the declared
    /// band grows it — measured at 18.13 pt against a declared 17.1 pt on
    /// <c>sheet-decor-xlsx.xlsx</c>, a difference of one point. Reproducing it needs the header
    /// font's metrics here, which the readers do not have; it is recorded in the module's TODO.
    /// </para>
    /// </remarks>
    private static Length BandHeight(OdfPropertySet? properties, XElement? content)
    {
        if (properties is null || !IsDisplayed(content)) return Length.Zero;

        return Measure(properties, OdfNamespaces.SvgCompatible, "height")
               ?? Measure(properties, OdfNamespaces.FoCompatible, "min-height")
               ?? Length.Zero;
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
    /// In the OOO reference syntax, so each range reads <c>Sheet.$A$1:$D$7</c> with the sheet
    /// name and the dollars both optional. Only the cell part is used: a print range naming
    /// another sheet is not something Calc produces, and taking the addresses without checking
    /// the name is what makes a range written without one work.
    /// </remarks>
    private static List<SheetRange> ReadPrintAreas(XElement table)
    {
        List<SheetRange> areas = [];
        string? ranges = Attribute(table, "print-ranges");
        if (string.IsNullOrWhiteSpace(ranges)) return areas;

        foreach (string range in ranges.Split(' ', StringSplitOptions.RemoveEmptyEntries))
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
