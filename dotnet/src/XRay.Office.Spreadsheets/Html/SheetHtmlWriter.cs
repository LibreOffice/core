using XRay.Office.Core.Threading;
using System.Globalization;
using System.Text;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Html;

/// <summary>
/// Writes a workbook as HTML: one table per sheet, the way Calc's own HTML export does.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A second rendering target rather than a second renderer.</strong> The raster, PDF and
/// SVG backends all draw the same paginated display list; HTML is not paginated and has no pen, so
/// it is written from the sheet model — cells, formats, the column and row axes — exactly as
/// <c>ScHTMLExport::WriteTables</c> walks <c>ScDocument</c> rather than a print layout
/// (<c>sc/source/filter/html/htmlexp.cxx</c>:704-903). A page break is not a thing an HTML table
/// has, which is why nothing here consults <see cref="SpreadsheetPages.Pages"/>.
/// </para>
/// <para>
/// <strong>Two dialects exist and this is LibreOffice's.</strong> Aspose.Cells writes the
/// Excel "Save as Web Page" markup — an XHTML doctype with <c>xmlns:v/o/x</c>, a class per cell
/// style carrying <c>mso-*</c> properties, <c>&lt;col&gt;</c> elements and <c>x:num</c>
/// attributes. LibreOffice writes plain HTML with the styling on the cells, <c>&lt;colgroup&gt;</c>
/// runs, and two families of round-trip attributes: <c>sdval</c>/<c>sdnum</c>, which its own HTML
/// <em>import</em> reads back (<c>sc/source/filter/html/htmlpars.cxx</c>), and the
/// <c>data-sheets-*</c> attributes Google Sheets understands. Following the reference
/// implementation is the whole premise of this library, so this is the one implemented.
/// </para>
/// <para>
/// <strong>What is deliberately not written.</strong> Pictures and charts: Calc writes each as a
/// file beside the HTML and an <c>&lt;img&gt;</c> pointing at it, which needs an image encoder and
/// a place to put the files — its own <c>SkipImages</c> filter option produces exactly what this
/// writes today. Hyperlinks: Calc writes an anchor for a cell whose <em>formula</em> is a
/// <c>HYPERLINK()</c> call, and the model here records which ranges hold one but not where they
/// point. <c>data-sheets-formula</c>: Calc states it in R1C1 grammar and the formula travels
/// through this library in the grammar its file used, so writing the stored text under an
/// attribute defined as R1C1 would be a quiet lie. All three are in the module's TODO.
/// </para>
/// </remarks>
public static class SheetHtmlWriter
{
    /// <summary>Pixels per inch, which is what a twip is converted through.</summary>
    /// <remarks>
    /// <c>BorderToStyle</c> converts a border's width with <c>o3tl::convert(twip, px)</c>
    /// (<c>htmlexp.cxx</c>:556-558), which is this constant; <c>ScHTMLExport::ToPixel</c> converts
    /// the column widths and row heights through the application window's own map mode instead, so
    /// a headless export follows whatever device that window has. 96 dpi is the value o3tl states
    /// and the one every desktop reports; the widths a running LibreOffice writes are a few per
    /// cent wider, which the tests record rather than chase.
    /// </remarks>
    private const double PixelsPerInch = 96;

    private const int TwipsPerInch = 1440;

    /// <summary>The seven font sizes HTML's <c>size=</c> attribute names, in twips.</summary>
    /// <remarks>
    /// <c>HTMLFONTSZ1_DFLT</c> … <c>HTMLFONTSZ7_DFLT</c> (<c>include/svtools/parhtml.hxx</c>:40-46)
    /// are 7, 10, 12, 14, 18, 24 and 36 points, and <c>ScHTMLExport</c>'s constructor keeps them in
    /// twips (<c>htmlexp.cxx</c>:237-245).
    /// </remarks>
    private static readonly int[] FontSizeTwips = [140, 200, 240, 280, 360, 480, 720];

    /// <summary>What each of those sizes is called in CSS.</summary>
    private static readonly string[] FontSizeCss =
        ["xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large"];

    /// <summary>Writes a workbook's sheets as HTML.</summary>
    /// <param name="pages">The laid-out workbook, whose <see cref="SpreadsheetPages.Sheets"/> is read.</param>
    /// <param name="output">Where to write. Left open.</param>
    /// <param name="options">What to include, or null for <see cref="SheetHtmlOptions.Default"/>.</param>
    public static void Write(SpreadsheetPages pages, Stream output, SheetHtmlOptions? options = null)
        => Write(pages, output, options, CancellationToken.None);

    /// <summary>Writes a workbook's sheets as HTML, checking <paramref name="cancellationToken"/> row by row.</summary>
    /// <remarks>
    /// The HTML is written to <paramref name="output"/> as each row is produced, so memory does not grow with
    /// the sheet. A cancelled write throws <see cref="OperationCanceledException"/> and leaves the partial
    /// document in the stream.
    /// </remarks>
    /// <param name="pages">The laid-out workbook, whose <see cref="SpreadsheetPages.Sheets"/> is read.</param>
    /// <param name="output">Where to write. Left open.</param>
    /// <param name="options">What to include, or null for <see cref="SheetHtmlOptions.Default"/>.</param>
    /// <param name="cancellationToken">Cancels the write.</param>
    public static void Write(
        SpreadsheetPages pages, Stream output, SheetHtmlOptions? options, CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(pages);
        ArgumentNullException.ThrowIfNull(output);

        Write(pages.Sheets, output, options, cancellationToken);
    }

    /// <summary>Writes a set of sheets as HTML.</summary>
    /// <param name="sheets">The sheets, in workbook order.</param>
    /// <param name="output">Where to write. Left open.</param>
    /// <param name="options">What to include, or null for <see cref="SheetHtmlOptions.Default"/>.</param>
    public static void Write(
        IReadOnlyList<SheetLayout> sheets, Stream output, SheetHtmlOptions? options = null)
        => Write(sheets, output, options, CancellationToken.None);

    /// <summary>Writes a set of sheets as HTML, checking <paramref name="cancellationToken"/> row by row.</summary>
    /// <param name="sheets">The sheets, in workbook order.</param>
    /// <param name="output">Where to write. Left open.</param>
    /// <param name="options">What to include, or null for <see cref="SheetHtmlOptions.Default"/>.</param>
    /// <param name="cancellationToken">Cancels the write.</param>
    public static void Write(
        IReadOnlyList<SheetLayout> sheets,
        Stream output,
        SheetHtmlOptions? options,
        CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(sheets);
        ArgumentNullException.ThrowIfNull(output);

        OfficeOperation.Run(() =>
        {
            WriteCore(sheets, output, options);
            return true;
        }, cancellationToken);
    }

    private static void WriteCore(
        IReadOnlyList<SheetLayout> sheets, Stream output, SheetHtmlOptions? options)
    {

        SheetHtmlOptions settings = options ?? SheetHtmlOptions.Default;

        // No byte-order mark: Calc writes the encoding in a meta element and the bytes plain.
        using StreamWriter writer = new(output, new UTF8Encoding(false), leaveOpen: true)
        {
            NewLine = "\n",
        };

        // A hidden sheet is not written at all — `WriteTables` skips a table `!pDoc->IsVisible`
        // (`htmlexp.cxx`:738-739) — and neither is one with nothing in it, which is what decides
        // whether the per-sheet headings appear at all.
        List<SheetLayout> written = [.. sheets.Where(sheet => !sheet.IsHidden && sheet.UsedRange.IsValid)];

        // Tabs are navigation, and one sheet has nothing to navigate — so a single-sheet workbook
        // is the same document either way, as it already is for the overview and the headings.
        bool tabbed = settings.Navigation == SheetHtmlNavigation.Tabs && written.Count > 1;
        string prefix = Identifier(settings.IdPrefix);

        // The head's default font comes from a sheet, so it is the written set that decides it: a
        // workbook whose first sheet is hidden takes the first one a reader will actually see.
        if (!settings.SkipHeaderFooter)
        {
            writer.WriteLine("<!DOCTYPE html>");
            writer.WriteLine();
            writer.WriteLine("<html>");
            WriteHead(writer, written, settings, tabbed ? (prefix, written.Count) : null);
            writer.WriteLine();
            writer.WriteLine("<body>");
        }

        if (tabbed) WriteTabStrip(writer, written, prefix, settings.SkipHeaderFooter);
        else WriteOverview(writer, written);

        for (int at = 0; at < written.Count; at++)
        {
            if (tabbed) writer.WriteLine($"<div class=\"sheet-panel\" id=\"{prefix}-panel-{Number(at + 1)}\">");

            WriteTable(writer, written[at], at, written.Count, settings, tabbed);

            if (tabbed) writer.WriteLine("</div>");
        }

        if (tabbed) writer.WriteLine("</div>");

        if (!settings.SkipHeaderFooter)
        {
            writer.WriteLine("</body>");
            writer.WriteLine();
            writer.WriteLine("</html>");
        }
    }

    /// <summary>
    /// Folds a caller's prefix into something usable as an element identifier and inside a CSS
    /// selector.
    /// </summary>
    /// <remarks>
    /// A caller passing a file name is the expected case, and a file name holds spaces, dots and
    /// worse. Anything outside the safe set becomes a hyphen, and a value that would not start
    /// with a letter gains one, because a CSS identifier may not begin with a digit.
    /// </remarks>
    private static string Identifier(string? prefix)
    {
        if (string.IsNullOrWhiteSpace(prefix)) return "sheets";

        StringBuilder folded = new(prefix.Length + 1);
        foreach (char character in prefix)
        {
            folded.Append(char.IsAsciiLetterOrDigit(character) || character is '-' or '_'
                ? character
                : '-');
        }

        if (!char.IsAsciiLetter(folded[0])) folded.Insert(0, 's');

        return folded.ToString();
    }

    // ----------------------------------------------------------------------------- the head

    private static void WriteHead(
        TextWriter writer,
        List<SheetLayout>      sheets,
        SheetHtmlOptions       options,
        (string Prefix, int Count)? tabs)
    {
        SheetCellFormat defaults = sheets.Count > 0
            ? sheets[0].Formats.SheetDefault
            : SheetCellFormat.Default;

        writer.WriteLine("<head>");
        writer.WriteLine("\t<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>");
        writer.WriteLine($"\t<title>{Escape(options.Title ?? string.Empty)}</title>");
        writer.WriteLine($"\t<meta name=\"generator\" content=\"{Escape(options.Generator)}\"/>");
        writer.WriteLine("\t<style type=\"text/css\">");

        // The document's own default font, at the nearest of HTML's seven sizes. Calc writes the
        // rule over the same eleven selectors (`htmlexp.cxx`:352-393).
        string family = DefaultFamily(defaults);
        writer.WriteLine(
            "\t\tbody,div,table,thead,tbody,tfoot,tr,th,td,p { font-family:\""
            + Escape(family) + "\"; font-size:" + FontSize(defaults.FontSize) + " }");

        // A note is shown by hovering its indicator, which is the whole of Calc's comment support
        // in HTML (`htmlexp.cxx`:396-460). Written whether or not this workbook has one, as there.
        writer.WriteLine(
            "\t\ta.comment-indicator:hover + comment { background:#ffd; position:absolute; "
            + "display:block; border:1px solid black; padding:0.5em;  } ");
        writer.WriteLine(
            "\t\ta.comment-indicator { background:red; display:inline-block; border:1px solid black; "
            + "width:0.5em; height:0.5em;  } ");
        writer.WriteLine("\t\tcomment { display:none;  } ");

        WriteThemeStyle(writer, "\t\t");

        if (tabs is { } strip) WriteTabStyle(writer, strip.Prefix, strip.Count, "\t\t");

        writer.WriteLine("\t</style>");
        writer.WriteLine("</head>");
    }

    /// <summary>
    /// The family the head's style rule names, which is what a cell is compared against.
    /// </summary>
    /// <remarks>
    /// Resolved in one place because the two uses have to agree: a cell writes a
    /// <c>&lt;font face&gt;</c> exactly when it differs from what the head already says, so a head
    /// that falls back to Arial while the comparison tests against nothing puts a redundant
    /// <c>face="Arial"</c> on every cell of an XLS. Measured: 2400 of them on one document.
    /// </remarks>
    private static string DefaultFamily(SheetCellFormat defaults) =>
        string.IsNullOrEmpty(defaults.FontFamily) ? "Arial" : defaults.FontFamily;

    /// <summary>The nearest of HTML's seven font sizes, as its CSS name.</summary>
    /// <remarks>
    /// <c>ScHTMLExport::GetFontSizeNumber</c> (<c>htmlexp.cxx</c>:260-272) walks the table from the
    /// top and takes the first size whose midpoint with its predecessor the height clears, so the
    /// boundaries sit halfway between neighbours rather than at the sizes themselves.
    /// </remarks>
    private static string FontSize(Length height) => FontSizeCss[FontSizeNumber(height) - 1];

    /// <summary>The 1-to-7 size number, which is what the <c>&lt;font&gt;</c> element states.</summary>
    private static int FontSizeNumber(Length height)
    {
        long twips = height.Twips;
        for (int at = FontSizeTwips.Length - 1; at > 0; at--)
        {
            if (twips > (FontSizeTwips[at] + FontSizeTwips[at - 1]) / 2) return at + 1;
        }

        return 1;
    }

    /// <summary>
    /// What the document is drawn on, in both of the reader's colour schemes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// An addition to the reference, and the fix for a real failure rather than a preference:
    /// LibreOffice's output paints no ground at all, so a document dropped into a page with a dark
    /// one — an <c>iframe</c> in a dark application shell, which is the expected embedding — shows
    /// that ground through every cell that states no fill of its own, under text that is still
    /// black. Measured on a three-sheet workbook: the banded rows stayed legible and the rows
    /// between them, the totals row and every heading did not.
    /// </para>
    /// <para>
    /// <strong>The sheet keeps white paper in both schemes, and that is the decision worth
    /// stating.</strong> A workbook's fills, borders and font colours are one set chosen against
    /// white — the demo's banded rows alternate a stated <c>#EDF2F9</c> with a cell that states
    /// nothing — so darkening the unstated half inverts every other stripe and leaves a header
    /// fill floating over it. Worse, a cell whose font colour is stated dark and whose fill is not
    /// would become unreadable, and nothing in the file says which way round it was meant. So the
    /// table paints its own paper unconditionally, which both fixes the embedding and keeps every
    /// authored colour on the ground it was authored for; what follows the reader's scheme is the
    /// document furniture around it — the page, the headings, the rules, the links, the note
    /// tooltip and the tab strip.
    /// </para>
    /// <para>
    /// The tokens are declared here and consumed below, so the dark scheme redefines values rather
    /// than repeating selectors — and <c>--paper</c> is deliberately absent from that block.
    /// </para>
    /// <para>
    /// A fragment gets none of this, as it gets no default font: <see cref="
    /// SheetHtmlOptions.SkipHeaderFooter"/> says the embedding page states the styling, and a
    /// caller embedding into a dark page is exactly the caller who has already chosen one.
    /// </para>
    /// </remarks>
    private static void WriteThemeStyle(TextWriter writer, string indent)
    {
        writer.WriteLine($"{indent}:root {{ color-scheme:light dark; --page:#ffffff; --ink:#000000; "
            + "--rule:#808080; --link:#0000ee; --paper:#ffffff; --paper-ink:#000000; "
            + "--tab-face:#ededed; --tab-line:#808080; }");
        writer.WriteLine($"{indent}@media (prefers-color-scheme:dark) {{ :root {{ --page:#1c1d1f; "
            + "--ink:#e4e4e6; --rule:#5a5d63; --link:#8ab4f8; --tab-face:#2a2c30; "
            + "--tab-line:#5a5d63; } }");
        writer.WriteLine($"{indent}body {{ background:var(--page); color:var(--ink); }}");
        writer.WriteLine($"{indent}table {{ background:var(--paper); color:var(--paper-ink); }}");
        writer.WriteLine($"{indent}hr {{ border-color:var(--rule); }}");
        writer.WriteLine($"{indent}a {{ color:var(--link); }}");

        // The note tooltip states its own pale ground and no text colour, so on a dark page it
        // would be light on light. It is paper too.
        writer.WriteLine($"{indent}comment {{ color:var(--paper-ink); }}");
    }

    // ------------------------------------------------------------------------- the overview

    /// <summary>The table of contents, which a one-sheet workbook does not get.</summary>
    /// <remarks><c>ScHTMLExport::WriteOverview</c> (<c>htmlexp.cxx</c>:462-493).</remarks>
    private static void WriteOverview(TextWriter writer, List<SheetLayout> sheets)
    {
        if (sheets.Count <= 1) return;

        writer.WriteLine("<hr>");
        writer.WriteLine("\t<p><center>");
        writer.WriteLine("\t\t<h1>Overview</h1>");

        foreach (SheetLayout sheet in sheets)
        {
            writer.WriteLine($"\t\t<A HREF=\"#table{Number(sheet.Index)}\">{Escape(sheet.Name)}</A><br>");
        }

        writer.WriteLine("\t</center></p>");
    }

    // ------------------------------------------------------------------------------ the tabs

    /// <summary>
    /// The tab strip, and the container everything after it lives in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A radio group, one input per sheet, with the labels drawn as tabs and the panels shown by
    /// two generated rules. The inputs come first because the rules reach the strip and the panels
    /// with the sibling combinator, which only looks forward.
    /// </para>
    /// <para>
    /// The container is closed by the caller, after the last panel.
    /// </para>
    /// </remarks>
    private static void WriteTabStrip(
        TextWriter writer, List<SheetLayout> sheets, string prefix, bool fragment)
    {
        writer.WriteLine($"<div class=\"sheet-tabs\" id=\"{prefix}\">");

        // A fragment has no head to put the rules in, and the embedding page cannot be asked to
        // supply them — the tabs do not work without them. The theme tokens come along because the
        // rules below name them, and a fragment has no `:root` block of its own; they are scoped to
        // the container so an embedding page's own tokens are untouched.
        if (fragment)
        {
            writer.WriteLine("<style type=\"text/css\">");
            writer.WriteLine($"\t#{prefix} {{ --paper:#ffffff; --paper-ink:#000000; --ink:#000000; "
                + "--link:#0000ee; --tab-face:#ededed; --tab-line:#808080; }");
            writer.WriteLine($"\t@media (prefers-color-scheme:dark) {{ #{prefix} {{ --ink:#e4e4e6; "
                + "--link:#8ab4f8; --tab-face:#2a2c30; --tab-line:#5a5d63; } }");
            WriteTabStyle(writer, prefix, sheets.Count, "\t");
            writer.WriteLine("</style>");
        }

        for (int at = 0; at < sheets.Count; at++)
        {
            writer.WriteLine(
                $"\t<input type=\"radio\" class=\"sheet-tab-state\" name=\"{prefix}-choice\" "
                + $"id=\"{prefix}-tab-{Number(at + 1)}\"{(at == 0 ? " checked" : string.Empty)}>");
        }

        writer.WriteLine("\t<div class=\"sheet-tab-strip\">");

        for (int at = 0; at < sheets.Count; at++)
        {
            writer.WriteLine(
                $"\t\t<label for=\"{prefix}-tab-{Number(at + 1)}\">{Escape(sheets[at].Name)}</label>");
        }

        writer.WriteLine("\t</div>");
    }

    /// <summary>The rules the tab strip needs, keyed on the container so two exports coexist.</summary>
    /// <remarks>
    /// <para>
    /// Two of them are generated per sheet and joined into one selector list: the checked input
    /// shows its panel, and marks its label. Everything else is fixed.
    /// </para>
    /// <para>
    /// The inputs are moved out of sight rather than hidden, because <c>display:none</c> takes
    /// them out of the focus order and the keyboard is the only way a reader without a mouse
    /// changes sheet. The <c>:focus-visible</c> rule is what then shows where the focus is, since
    /// the outline would otherwise land on the invisible input.
    /// </para>
    /// <para>
    /// <strong>Print shows every sheet.</strong> A tabbed document printed as it appears would be
    /// one sheet of a workbook, silently — so the panels all open, the strip goes away, and the
    /// per-sheet headings that are hidden on screen come back. The printed document is the
    /// <see cref="SheetHtmlNavigation.Overview"/> one.
    /// </para>
    /// </remarks>
    private static void WriteTabStyle(TextWriter writer, string prefix, int count, string indent)
    {
        string panels = Selectors(count, indent, at =>
            $"#{prefix}-tab-{Number(at)}:checked ~ #{prefix}-panel-{Number(at)}");
        string labels = Selectors(count, indent, at =>
            $"#{prefix}-tab-{Number(at)}:checked ~ .sheet-tab-strip label[for=\"{prefix}-tab-{Number(at)}\"]");
        string focus = Selectors(count, indent, at =>
            $"#{prefix}-tab-{Number(at)}:focus-visible ~ .sheet-tab-strip label[for=\"{prefix}-tab-{Number(at)}\"]");

        writer.WriteLine($"{indent}#{prefix} .sheet-tab-state {{ position:absolute; width:1px; height:1px; "
            + "margin:0; opacity:0; }");
        writer.WriteLine($"{indent}#{prefix} .sheet-tab-strip {{ display:flex; flex-wrap:wrap; gap:2px; "
            + "border-bottom:1px solid var(--tab-line); margin:0 0 12px; }");
        writer.WriteLine($"{indent}#{prefix} .sheet-tab-strip label {{ padding:4px 14px; cursor:pointer; "
            + "margin-bottom:-1px; background:var(--tab-face); color:var(--ink); "
            + "border:1px solid var(--tab-line); border-bottom:none; border-radius:4px 4px 0 0; }");
        writer.WriteLine($"{indent}#{prefix} .sheet-panel {{ display:none; }}");
        writer.WriteLine($"{indent}#{prefix} .sheet-panel h1 {{ display:none; }}");
        writer.WriteLine($"{indent}{panels} {{ display:block; }}");

        // The live tab takes the paper's colours, not the page's, because it is the edge of the
        // sheet below it — the border it drops is what joins the two.
        writer.WriteLine($"{indent}{labels} {{ background:var(--paper); color:var(--paper-ink); "
            + "border-bottom:1px solid var(--paper); font-weight:bold; }");
        writer.WriteLine($"{indent}{focus} {{ outline:2px solid var(--link); outline-offset:-2px; }}");
        writer.WriteLine($"{indent}@media print {{ #{prefix} .sheet-tab-strip {{ display:none; }} "
            + $"#{prefix} .sheet-panel, #{prefix} .sheet-panel h1 {{ display:block; }} }}");
    }

    /// <summary>One selector per sheet, joined as a list and kept under the rule's own indent.</summary>
    private static string Selectors(int count, string indent, Func<int, string> selector)
        => string.Join(",\n" + indent, Enumerable.Range(1, count).Select(selector));

    // --------------------------------------------------------------------------- the tables

    private static void WriteTable(
        TextWriter       writer,
        SheetLayout      sheet,
        int              position,
        int              count,
        SheetHtmlOptions options,
        bool             tabbed)
    {
        SheetRange used = sheet.UsedRange;

        if (count > 1)
        {
            // Under tabs the rule goes: the strip already separates the sheets, and the heading is
            // hidden by the stylesheet rather than dropped, so printing brings it back.
            if (!tabbed) writer.WriteLine("<hr>");

            writer.WriteLine(
                $"<A NAME=\"table{Number(sheet.Index)}\"><h1>Sheet {Number(position + 1)}: "
                + $"<em>{Escape(sheet.Name)}</em></h1></A>");
        }

        writer.WriteLine("<table cellspacing=\"0\" border=\"0\">");
        WriteColumnGroups(writer, sheet, used);

        for (int row = used.FirstRow; row <= used.LastRow; row++)
        {
            OfficeOperation.Poll();
            if (sheet.Grid.Rows.IsHidden(row)) continue;

            writer.WriteLine("\t<tr>");
            bool first = true;

            for (int column = used.FirstColumn; column <= used.LastColumn; column++)
            {
                if (sheet.Grid.Columns.IsHidden(column)) continue;

                if (WriteCell(writer, sheet, row, column, first, options)) first = false;
            }

            writer.WriteLine("\t</tr>");
        }

        writer.WriteLine("</table>");
        writer.WriteLine("<!-- ************************************************************************** -->");
    }

    /// <summary>
    /// One <c>&lt;colgroup&gt;</c> per run of equally wide columns.
    /// </summary>
    /// <remarks>
    /// Calc coalesces the runs as it walks (<c>htmlexp.cxx</c>:804-835) and gives the group a
    /// <c>span</c> only when it covers more than one column. A hidden column is skipped rather
    /// than given a zero width, so the run it falls in continues across it.
    /// </remarks>
    private static void WriteColumnGroups(TextWriter writer, SheetLayout sheet, SheetRange used)
    {
        int width = 0;
        int span = 0;

        for (int column = used.FirstColumn; column <= used.LastColumn; column++)
        {
            if (sheet.Grid.Columns.IsHidden(column)) continue;

            int at = ToPixel(sheet.Grid.Columns.SizeAt(column));
            if (at != width)
            {
                if (span != 0) WriteColumnGroup(writer, span, width);
                width = at;
                span = 1;
            }
            else
            {
                span++;
            }
        }

        if (span != 0) WriteColumnGroup(writer, span, width);
    }

    private static void WriteColumnGroup(TextWriter writer, int span, int width)
    {
        string attributes = span > 1
            ? $"span=\"{Number(span)}\" width=\"{Number(width)}\""
            : $"width=\"{Number(width)}\"";

        writer.WriteLine($"\t<colgroup {attributes}></colgroup>");
    }

    // ---------------------------------------------------------------------------- the cells

    /// <summary>Writes one cell, and answers whether it was written.</summary>
    /// <remarks>
    /// A position covered by a merge that starts elsewhere writes nothing at all — the merge's own
    /// cell already carries the <c>colspan</c> and <c>rowspan</c> covering it — which is
    /// <c>WriteCell</c>'s first act (<c>htmlexp.cxx</c>:929-931).
    /// </remarks>
    private static bool WriteCell(
        TextWriter writer, SheetLayout sheet, int row, int column, bool first, SheetHtmlOptions options)
    {
        SheetRange? merge = sheet.Merges.Covering(row, column);
        if (merge is { } covering && (covering.FirstRow != row || covering.FirstColumn != column))
            return false;

        ContentTableCell? cell = sheet.CellAt(row, column);
        SheetCellFormat format = sheet.Formats.At(row, column);
        SheetCellDecoration decoration = sheet.Formatting.At(row, column);
        string text = cell?.GetOwnText() ?? string.Empty;

        // The same predicate the drawn cell aligns by — `SheetTextLayout.Place`'s
        // `cell.Value is not null and not string` — so the two agree about every cell. It takes in
        // an error, which Calc also treats as a value: `=1/0` is right-aligned and carries
        // `sdval="0"`, because `hasNumeric` is true for a formula cell whatever its result is.
        bool isValue = cell?.Value is not null and not string;
        double? value = isValue ? NumericValue(cell, sheet.DateSystem) ?? 0 : null;

        StringBuilder attributes = new();

        if (!decoration.Borders.IsNone) attributes.Append(Borders(decoration.Borders));

        if (merge is { } span)
        {
            int columns = span.LastColumn - span.FirstColumn + 1;
            int rows = span.LastRow - span.FirstRow + 1;
            if (columns > 1) attributes.Append(" colspan=").Append(Number(columns));
            if (rows > 1) attributes.Append(" rowspan=").Append(Number(rows));
        }

        // The height goes on the row's first cell only, which is where Calc puts it —
        // `bTableDataHeight` is set once per row and cleared by the first cell written
        // (`htmlexp.cxx`:854-866).
        if (first)
        {
            attributes.Append(" height=\"")
                .Append(Number(ToPixel(RowHeight(sheet, row, merge))))
                .Append('"');
        }

        attributes.Append(" align=\"").Append(Alignment(format.Horizontal, value is not null)).Append('"');

        if (VerticalAlignment(format.Vertical) is { } valign) attributes.Append(" valign=").Append(valign);

        if (decoration.Background is { } background)
            attributes.Append(" bgcolor=\"").Append(Hex(background)).Append('"');

        // sdval carries the value as it is, sdnum the locale and the format code: the pair Calc's
        // own HTML import reads back (`HTMLOutFuncs::CreateTableDataOptionsValNum`,
        // `svtools/source/svhtml/htmlout.cxx`:914-955). A cell with neither a value nor a stated
        // format gets neither attribute.
        //
        // A boolean carries Calc's own BOOLEAN format, whose format string is what its HTML export
        // keys the Google Sheets type off (`htmlexp.cxx`:1156). No file states such a format — a
        // boolean is a cell type in OOXML and ODF, not a format — so it is supplied here, which is
        // where the two models meet.
        bool boolean = cell?.Value is bool;
        string? code = boolean
            ? "BOOLEAN"
            : format.HasGeneralFormat ? null : format.NumberFormat?.Code;

        if (value is { } number)
            attributes.Append(" sdval=\"").Append(Invariant(number)).Append('"');

        if (value is not null || code is not null)
        {
            attributes.Append(" sdnum=\"").Append(Number(options.LanguageId)).Append(';');

            // The format's own language, then its code. Zero is what LibreOffice writes for a
            // format that states none, which is every format this library reads.
            if (code is not null) attributes.Append("0;").Append(Escape(code));

            attributes.Append('"');
        }

        AppendDataSheets(attributes, text, value, code, boolean);

        writer.Write("\t\t<td");
        writer.Write(attributes.ToString());
        writer.Write('>');

        WriteNote(writer, sheet, row, column);
        WriteContent(writer, format, sheet.Formats.SheetDefault, text);

        writer.WriteLine("</td>");
        return true;
    }

    /// <summary>
    /// The Google Sheets attributes, which travel beside the LibreOffice ones rather than instead
    /// of them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>WriteCell</c> (<c>htmlexp.cxx</c>:1146-1206) writes <c>data-sheets-value</c> as a small
    /// JSON object whose <c>"1"</c> is the type — 2 text, 3 number, 4 boolean — and
    /// <c>data-sheets-numberformat</c> beside it when the cell states a format. A number with no
    /// stated format gets neither: the <c>oJson</c> block is entered only when <c>nFormat</c> is
    /// non-zero.
    /// </para>
    /// <para>
    /// <strong>The number is truncated to an integer, and that is faithful.</strong>
    /// <c>oJson->put("3", static_cast&lt;sal_Int32&gt;(fVal))</c> writes 4 for 4.5, and the corpus
    /// output confirms the binary does it. It is a defect upstream and not one to fix here: the
    /// exact value is already in <c>sdval</c>, and a reader that took this attribute instead would
    /// disagree with LibreOffice about the file. Kept, with the citation, so that nobody
    /// "corrects" it into a divergence.
    /// </para>
    /// </remarks>
    private static void AppendDataSheets(
        StringBuilder attributes, string text, double? value, string? code, bool isBoolean)
    {
        if (value is { } number)
        {
            if (code is null) return;

            // Type 4 is a boolean and 3 a number, decided by the format string being the literal
            // "BOOLEAN" (`htmlexp.cxx`:1155-1160).
            attributes.Append(" data-sheets-value=\"")
                .Append(Escape(isBoolean
                    ? $"{{ \"1\": 4, \"4\": {Number((int)number)}}}"
                    : $"{{ \"1\": 3, \"3\": {Number((int)number)}}}"))
                .Append('"');

            // Only a number states its format: Calc leaves `pNumberFormat` null on the boolean arm
            // (`htmlexp.cxx`:1157-1170), so a boolean carries its type and nothing else.
            if (!isBoolean)
            {
                attributes.Append(" data-sheets-numberformat=\"")
                    .Append(Escape($"{{ \"1\": 2, \"2\": \"{Json(code)}\", \"3\": 1}}"))
                    .Append('"');
            }

            return;
        }

        attributes.Append(" data-sheets-value=\"")
            .Append(Escape($"{{ \"1\": 2, \"2\": \"{Json(text)}\"}}"))
            .Append('"');
    }

    /// <summary>The note on a cell, as an indicator and the hidden text beside it.</summary>
    private static void WriteNote(TextWriter writer, SheetLayout sheet, int row, int column)
    {
        if (sheet.Notes.IsEmpty) return;

        foreach (SheetNote note in sheet.Notes.Items)
        {
            if (note.Row != row || note.Column != column) continue;

            writer.Write("<a class=\"comment-indicator\"></a>");
            writer.Write($"<comment>{Escape(note.Text)}</comment>");
            return;
        }
    }

    /// <summary>
    /// The cell's text, inside whatever the format asks for.
    /// </summary>
    /// <remarks>
    /// An empty cell gets a <c>&lt;br&gt;</c> rather than nothing, "so there is no completely
    /// empty line" (<c>htmlexp.cxx</c>:1319-1322), and a newline inside a cell becomes one too.
    /// Bold, italic, underline and strike-through are elements around the text rather than CSS,
    /// which is the shape Calc writes.
    /// </remarks>
    private static void WriteContent(
        TextWriter writer, SheetCellFormat format, SheetCellFormat defaults, string text)
    {
        bool bold = format.FontWeight >= 700;
        bool italic = format.IsItalic;
        bool underline = format.Underline != SheetUnderline.None;
        bool struck = format.IsStruckThrough;
        string? font = FontElement(format, defaults);

        if (bold) writer.Write("<b>");
        if (italic) writer.Write("<i>");
        if (underline) writer.Write("<u>");
        if (struck) writer.Write("<s>");
        if (font is not null) writer.Write(font);

        if (text.Length == 0)
        {
            writer.Write("<br>");
        }
        else
        {
            string[] lines = text.Split('\n');
            for (int at = 0; at < lines.Length; at++)
            {
                if (at > 0) writer.Write("<br>");
                writer.Write(Escape(lines[at]));
            }
        }

        if (font is not null) writer.Write("</font>");
        if (struck) writer.Write("</s>");
        if (underline) writer.Write("</u>");
        if (italic) writer.Write("</i>");
        if (bold) writer.Write("</b>");
    }

    /// <summary>
    /// The opening <c>&lt;font&gt;</c> a cell needs, or null when it takes the document's own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Calc writes a face, a size or a colour only where the cell differs from the default the
    /// head's style rule already states (<c>htmlexp.cxx</c>:1069-1084, :1240-1279), which is why
    /// the attribute is a comparison rather than a dump of the cell's format. The size is the
    /// 1-to-7 index, not a measurement.
    /// </para>
    /// <para>
    /// <strong>The colour condition here is "differs from the default", where Calc's is "is not
    /// automatic".</strong> Its <c>COL_AUTO</c> is a distinct value from black, and this model
    /// resolves an unstated colour to black as it reads, so the two cannot be told apart. The
    /// visible result is the same — automatic text is black — and the difference is that Calc
    /// writes a redundant <c>&lt;font color="#000000"&gt;</c> around a cell that states black
    /// explicitly, where this writes nothing.
    /// </para>
    /// </remarks>
    private static string? FontElement(SheetCellFormat format, SheetCellFormat defaults)
    {
        bool face = !string.IsNullOrEmpty(format.FontFamily)
                    && !string.Equals(format.FontFamily, DefaultFamily(defaults), StringComparison.Ordinal);
        bool size = FontSizeNumber(format.FontSize) != FontSizeNumber(defaults.FontSize);
        bool colour = format.Colour != defaults.Colour;

        if (!face && !size && !colour) return null;

        StringBuilder element = new("<font");
        if (face) element.Append(" face=\"").Append(Escape(format.FontFamily!)).Append('"');
        if (size) element.Append(" size=").Append(Number(FontSizeNumber(format.FontSize)));
        if (colour) element.Append(" color=\"").Append(Hex(format.Colour)).Append('"');

        return element.Append('>').ToString();
    }

    // --------------------------------------------------------------------------- the pieces

    /// <summary>A merged cell's height is its rows' together.</summary>
    private static Length RowHeight(SheetLayout sheet, int row, SheetRange? merge)
    {
        if (merge is not { } span || span.LastRow <= span.FirstRow) return sheet.Grid.Rows.SizeAt(row);

        Length total = Length.Zero;
        for (int at = span.FirstRow; at <= span.LastRow; at++) total += sheet.Grid.Rows.SizeAt(at);

        return total;
    }

    /// <summary>
    /// The four borders as one inline style, in Calc's own order and spelling.
    /// </summary>
    /// <remarks>
    /// <c>ScHTMLExport::BorderToStyle</c> (<c>htmlexp.cxx</c>:540-608) writes top, bottom, left
    /// then right, separated by "; ", each as a pixel width, a CSS line style and a six-digit
    /// colour. A width under half a pixel still draws, so it is floored at one.
    /// </remarks>
    private static string Borders(SheetCellBorders borders)
    {
        StringBuilder style = new(" style=\"");
        bool written = false;

        Append("top", borders.Top);
        Append("bottom", borders.Bottom);
        Append("left", borders.Left);
        Append("right", borders.Right);

        return style.Append('"').ToString();

        void Append(string side, SheetBorder border)
        {
            if (border.IsNone) return;
            if (written) style.Append("; ");

            style.Append("border-").Append(side).Append(": ")
                .Append(Number(Math.Max(1, ToPixel(border.Width)))).Append("px ")
                .Append(Pattern(border))
                .Append(" #").Append(Hex(border.Colour).ToLowerInvariant()[1..]);

            written = true;
        }
    }

    /// <summary>Which CSS line style a border pattern is.</summary>
    /// <remarks>
    /// A double border is a width plus a gap plus a width in this model, and Calc maps every
    /// two-line style — <c>DOUBLE</c> and the five thick/thin pairs — to <c>double</c>.
    /// </remarks>
    private static string Pattern(SheetBorder border) => border.IsDouble
        ? "double"
        : border.Pattern switch
        {
            SheetBorderPattern.Dotted => "dotted",
            SheetBorderPattern.Dashed or SheetBorderPattern.DashDot
                or SheetBorderPattern.DashDotDot or SheetBorderPattern.FineDashed => "dashed",
            _ => "solid",
        };

    /// <summary>
    /// Which way the cell's text is set, with a general alignment resolved by what is in it.
    /// </summary>
    /// <remarks>
    /// <c>htmlexp.cxx</c>:1090-1104: standard means right for a value and left for anything else,
    /// which is the same rule the drawn cell follows.
    /// </remarks>
    private static string Alignment(SheetHorizontalAlignment alignment, bool isValue) => alignment switch
    {
        SheetHorizontalAlignment.Centre => "center",
        SheetHorizontalAlignment.Right => "right",
        SheetHorizontalAlignment.Justify => "justify",
        SheetHorizontalAlignment.General => isValue ? "right" : "left",
        _ => "left",
    };

    /// <summary>The vertical alignment, or null where the cell states none.</summary>
    private static string? VerticalAlignment(SheetVerticalAlignment alignment) => alignment switch
    {
        SheetVerticalAlignment.Top => "top",
        SheetVerticalAlignment.Centre => "middle",
        SheetVerticalAlignment.Bottom => "bottom",
        _ => null,
    };

    /// <summary>The cell's number, or null when it holds text or nothing.</summary>
    /// <remarks>
    /// A boolean is a number in Calc and in every one of these formats, so it counts here as one —
    /// <c>hasNumeric</c> is what decides both the alignment and the <c>sdval</c>.
    /// </remarks>
    private static double? NumericValue(ContentTableCell? cell, SpreadsheetDateSystem dates)
        => cell?.Value switch
        {
            double number => number,
            float number => number,
            decimal number => (double)number,
            long number => number,
            int number => number,
            bool flag => flag ? 1 : 0,

            // A date arrives as a DateTime and a duration as a TimeSpan, because the readers
            // resolve the serial as they read it. `sdval` wants the serial back, which is what
            // the sheet's own epoch is carried for.
            DateTime moment => (moment - Epoch(dates)).TotalDays,
            TimeSpan elapsed => elapsed.TotalDays,
            _ => null,
        };

    private static DateTime Epoch(SpreadsheetDateSystem dates) => dates == SpreadsheetDateSystem.Date1904
        ? new DateTime(1904, 1, 1)
        : new DateTime(1899, 12, 30);

    private static int ToPixel(Length length)
    {
        long twips = length.Twips;
        if (twips <= 0) return 0;

        return Math.Max(1, (int)Math.Round(twips * PixelsPerInch / TwipsPerInch));
    }

    private static string Hex(Colour colour) =>
        string.Create(CultureInfo.InvariantCulture, $"#{colour.R:X2}{colour.G:X2}{colour.B:X2}");

    private static string Number(int value) => value.ToString(CultureInfo.InvariantCulture);

    /// <summary>The value as <c>sdval</c> states it: fifteen significant digits.</summary>
    /// <remarks>
    /// <c>CreateTableDataOptionsValNum</c> writes <c>rFormatter.GetInputLineString(fVal, 0)</c>
    /// (<c>htmlout.cxx</c>:924) — the string Calc's own input line would show, which carries
    /// fifteen significant digits and no trailing zeros. Round-tripping the double instead writes
    /// seventeen: measured, a cell holding 14:30 comes out <c>0.604166666666667</c> from
    /// LibreOffice and <c>0.6041666666666666</c> from <c>"R"</c>.
    /// </remarks>
    private static string Invariant(double value) =>
        value.ToString("G15", CultureInfo.InvariantCulture);

    /// <summary>
    /// Escapes text for the inside of a JSON string literal, the way the reference's own writer
    /// does.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two <c>data-sheets-*</c> attributes are JSON inside an HTML attribute, so their values
    /// carry two layers of escaping; this is the inner one and <see cref="Escape"/> is the outer.
    /// <c>tools::JsonWriter::writeEscapedOUString</c>
    /// (<c>tools/source/misc/json_writer.cxx</c>:116-141) escapes the eight characters below —
    /// <c>/</c> among them, which JSON permits but does not require — and writes anything at or
    /// below U+001F, plus U+2028 and U+2029, in <c>\uXXXX</c> form.
    /// </para>
    /// <para>
    /// Measured on a probe sheet converted by 24.2.7.2: a <c>\£#,##0.00</c> format is written
    /// <c>\\£#,##0.00</c>, an <c>MM/DD/YYYY</c> one <c>MM\/DD\/YYYY</c>, and a cell holding
    /// <c>he said "hi"</c> comes out <c>he said \"hi\"</c>. Note that <c>sdnum</c> beside them is
    /// <em>not</em> JSON and takes the raw code — the same probe writes
    /// <c>sdnum="1033;0;MM/DD/YYYY"</c> against that escaped format attribute.
    /// </para>
    /// </remarks>
    private static string Json(string text)
    {
        if (!text.Any(NeedsJsonEscape)) return text;

        StringBuilder escaped = new(text.Length + 16);
        foreach (char character in text)
        {
            switch (character)
            {
                case '\b': escaped.Append("\\b"); break;
                case '\t': escaped.Append("\\t"); break;
                case '\n': escaped.Append("\\n"); break;
                case '\f': escaped.Append("\\f"); break;
                case '\r': escaped.Append("\\r"); break;
                case '"':  escaped.Append("\\\""); break;
                case '/':  escaped.Append("\\/"); break;
                case '\\': escaped.Append("\\\\"); break;

                default:
                    if (NeedsJsonEscape(character))
                    {
                        escaped.Append("\\u")
                            .Append(((int)character).ToString("x4", CultureInfo.InvariantCulture));
                    }
                    else
                    {
                        escaped.Append(character);
                    }

                    break;
            }
        }

        return escaped.ToString();
    }

    private static bool NeedsJsonEscape(char character)
        => character is '"' or '/' or '\\' or '\u2028' or '\u2029' || character <= '\u001f';

    /// <summary>
    /// HTML-escapes text, quotes included, because everything written here goes either into an
    /// attribute or into element content and the same escaping serves both.
    /// </summary>
    private static string Escape(string text)
    {
        if (!text.Any(character => character is '&' or '<' or '>' or '"')) return text;

        StringBuilder escaped = new(text.Length + 16);
        foreach (char character in text)
        {
            switch (character)
            {
                case '&': escaped.Append("&amp;"); break;
                case '<': escaped.Append("&lt;"); break;
                case '>': escaped.Append("&gt;"); break;
                case '"': escaped.Append("&quot;"); break;
                default: escaped.Append(character); break;
            }
        }

        return escaped.ToString();
    }
}
