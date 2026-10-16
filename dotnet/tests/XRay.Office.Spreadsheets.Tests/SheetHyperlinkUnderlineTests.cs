using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A hyperlink cell is underlined by the field, not by the file.
/// </summary>
/// <remarks>
/// <para>
/// The fourth consequence of a hyperlink cell being one <c>SvxURLField</c>, and it comes out of
/// the same function call as the third. <c>ScEditUtil::GetCellFieldValue</c>
/// (<c>sc/source/core/tool/editutil.cxx:209-244</c>) answers a URL field by writing
/// <em>both</em> <c>*ppTextColor = …GetColorValue(LINKS)</c> and <c>*ppFldLineStyle =
/// FontLineStyle::LINESTYLE_SINGLE</c>; <c>EditCharAttribField::SetFont</c>
/// (<c>editeng/source/editeng/editattr.cxx:349-350</c>) then puts both on the run's font. This
/// tree modelled the colour half — see <see cref="SheetHyperlinkColourTests"/> — and not this one,
/// which is the whole of O62.
/// </para>
/// <para>
/// <strong>The reference's own resolved view says the opposite, and that is not a contradiction.</strong>
/// <c>ScXMLExport</c> resolves a field with both out-parameters null
/// (<c>sc/source/filter/xml/xmlexprt.cxx:3062</c>), so <c>--convert-to fods</c> prints
/// <c>style:text-underline-style="none"</c> on the very cells 26.2.4.2's PDF strokes a rule under.
/// Measured on <c>084_Service_invoice_Use_this_template</c> page 1: the flat export gives the
/// <c>jordan@example.com</c> cell <c>underline=none</c> and <c>colour=#000000</c>; the PDF strokes
/// <c>#000080</c> 0.51 pt thick from x 83.8 to 193.3 at y 243.8 and paints the text <c>#000080</c>.
/// </para>
/// <para>
/// <strong>The fixture states no underline on the linked cell on purpose.</strong> Most workbooks'
/// hyperlink cells carry Excel's own underlined <c>Hyperlink</c> font as well, and those were
/// already drawn correctly here — measured over the sheets track, 1771 of 1983 hyperlink-covered
/// cells in the 243 zip spreadsheets resolve to a font that states <c>&lt;u/&gt;</c> in the file.
/// It is the other 212, in 27 documents, that this rule is the only thing that reaches.
/// </para>
/// </remarks>
public sealed class SheetHyperlinkUnderlineTests
{
    private static readonly Colour Link = Colour.FromRgb(0x000080);

    private static DrawnPage Page()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-hyperlink-underline.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return sink.Pages[0];
    }

    /// <summary>The rules drawn under one run, nearest first.</summary>
    /// <remarks>
    /// Attributed by geometry rather than by order: a rule starts at its run's origin and hangs
    /// within a couple of points below its baseline, and the fixture puts every string in a row of
    /// its own so no two runs' bands can overlap. Matching on order instead would pass for a
    /// painter that drew the right number of rules in the wrong places.
    /// </remarks>
    private static List<DrawnFill> Under(DrawnPage page, string text)
    {
        DrawnGlyphRun run = page.Runs.First(r => r.Text.Contains(text, StringComparison.Ordinal));

        return [.. page.FilledPaths
            .Where(rule => Math.Abs((rule.Bounds.X - run.Origin.X).Points) < 0.01
                           && rule.Bounds.Y > run.Origin.Y
                           && rule.Bounds.Y < run.Origin.Y + Length.FromPoints(4))
            .OrderBy(rule => rule.Bounds.Y.Emu)];
    }

    /// <summary>A linked cell whose own font states no line is still underlined; its twin is not.</summary>
    /// <remarks>
    /// Both halves are the assertion. Underlining every cell would satisfy the first alone, and
    /// that is the shape a fix applied to the cell format rather than to the field would take — the
    /// same trap <see cref="SheetHyperlinkColourTests"/> guards on the colour.
    /// </remarks>
    [Fact]
    public void ALinkedCellIsUnderlinedWhereItsOwnFontStatesNoLine()
    {
        DrawnPage page = Page();

        List<DrawnFill> linked = Under(page, "linkplain");
        linked.Count.ShouldBe(1);
        linked[0].Paint.ShouldBe(Paint.Solid(Link));

        Under(page, "bareplain").ShouldBeEmpty();
    }

    /// <summary>The rule spans the run and hangs just under its baseline.</summary>
    /// <remarks>
    /// Measured against 26.2.4.2 on this fixture: it strokes the linked plain cell from x 51.392
    /// to 94.705, and this tree fills x 51.392 to 94.717 — the same left edge and the same width
    /// to a hundredth of a point. Two residuals, neither of them this rule and both stated rather
    /// than hidden. (1) The whole cell block sits 0.85 pt lower here than there, which is a
    /// placement difference the fixture inherits and which moves the text as well as the rule; the
    /// rule's offset from its own run is 1.269 pt against 1.226, so it is 0.043 pt out *relative to
    /// its text*. (2) The thickness is 0.638 pt here against 0.595, because the reference quantises
    /// it to a whole hundredth of a millimetre (21) and this tree does not — see
    /// <c>probes/underline-r118/results.md</c> §6.
    /// </remarks>
    [Fact]
    public void TheRuleSpansTheRunAndSitsUnderItsBaseline()
    {
        DrawnPage page = Page();
        DrawnGlyphRun run = page.Runs.First(
            r => r.Text.Contains("linkplain", StringComparison.Ordinal));
        DrawnFill rule = Under(page, "linkplain")[0];

        rule.Bounds.X.ShouldBe(run.Origin.X);
        rule.Bounds.Width.Points.ShouldBe(run.Width.Points, 0.01);
        (rule.Bounds.Y - run.Origin.Y).Points.ShouldBeGreaterThan(0.0);
        (rule.Bounds.Y - run.Origin.Y).Points.ShouldBeLessThan(2.0);
    }

    /// <summary>A link over a doubly-underlined font draws one line, and its twin draws two.</summary>
    /// <remarks>
    /// <c>SvxFont::SetUnderline</c> <em>replaces</em> the run's line style rather than adding to
    /// it, so the field's single line wins over the file's double. Modelling it as "underline if
    /// either says so" would draw two here and would be indistinguishable from the right answer on
    /// every cell that states no underline at all — which is every cell the seat was opened for.
    /// </remarks>
    [Fact]
    public void ALinkedDoubleUnderlineIsDrawnAsOneLineAndAnUnlinkedOneAsTwo()
    {
        DrawnPage page = Page();

        Under(page, "linkdouble").Count.ShouldBe(1);
        Under(page, "baredouble").Count.ShouldBe(2);
    }

    /// <summary>A hyperlink on a numeric cell is not a field, so it takes neither the line nor the colour.</summary>
    /// <remarks>
    /// <c>WorksheetGlobals::insertHyperlink</c> converts a <c>CELLTYPE_STRING</c> or
    /// <c>CELLTYPE_EDIT</c> cell and leaves everything else carrying a plain
    /// <c>ATTR_HYPERLINK</c>, which changes nothing about how the cell is drawn — see
    /// <see cref="SheetLayout.HoldsField"/>. The fixture's A5 is covered by a
    /// <c>&lt;hyperlink&gt;</c> and holds the number 1205.
    /// </remarks>
    [Fact]
    public void AHyperlinkOnANumericCellDrawsNoRuleAndNoLinkColour()
    {
        DrawnPage page = Page();

        Under(page, "1205").ShouldBeEmpty();
        page.Runs.First(r => r.Text.Contains("1205", StringComparison.Ordinal))
            .Paint.ShouldNotBe(Paint.Solid(Link));
    }
}
