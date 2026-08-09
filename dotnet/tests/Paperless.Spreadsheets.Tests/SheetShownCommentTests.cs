using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A cell comment the file marks visible is drawn on the sheet, where its client anchor puts it.
/// </summary>
/// <remarks>
/// <para>
/// Calc keeps a shown comment's caption on the internal drawing layer and prints it with the rest
/// of that layer (<c>PrintDrawingLayer(SC_LAYER_INTERN)</c>,
/// <c>sc/source/ui/view/printfun.cxx:1713</c>); a hidden one goes to <c>SC_LAYER_HIDDEN</c>, which
/// is neither printed nor counted when the print area is widened
/// (<c>ScCaptionUtil::SetCaptionLayer</c>, <c>sc/source/core/data/postit.cxx:82-87</c>). We drew
/// neither, which on <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c> cost 490 words —
/// four captions on a sheet that repeats its first row, so each of them prints six times.
/// </para>
/// <para>
/// <c>sheet-cell-comment-shown.xlsx</c> is authored to separate the three decisions rather than
/// copied from that workbook. It has one sheet repeating row 1 as a print title, three comments,
/// and a deliberately misleading VML:
/// </para>
/// <list type="bullet">
/// <item><c>B1</c> is <c>visibility:visible</c> and its shape's CSS puts it at
/// <c>margin-left:540pt;margin-top:504pt</c> — nowhere near where its <c>x:Anchor</c> puts it, so
/// a reader taking the CSS draws it in a different place or off the sheet entirely;</item>
/// <item><c>B3</c> carries <c>&lt;x:Visible/&gt;</c> and <c>visibility:hidden</c>, which is the
/// combination Excel writes constantly and which decides whether the element or the style is
/// read;</item>
/// <item><c>E1</c> is visible and reaches column J, four columns past the last cell, so the
/// printed block has to grow with it.</item>
/// </list>
/// <para>
/// Every expectation is LibreOffice 24.2.7.2's own answer: its flat-ODF export of this file
/// carries exactly two <c>office:annotation</c> elements, both <c>office:display="true"</c>, at
/// <c>svg:x/y/width/height</c> of 1.0295/0.6075/1.9539/0.6768 in and
/// 4.6252/0.5555/2.987/0.4161 in; and its PDF is three pages with both captions drawn on pages 1
/// and 2 at the same place on each.
/// </para>
/// <para>
/// <strong>One difference is measured and deliberately not reproduced.</strong> LibreOffice
/// drops the character formatting of a comment's <em>first</em> run when it builds the caption —
/// the reference draws "Shown note " at the caption's own 10 pt Liberation Sans and "beta", the
/// second run, at the stated 9 pt Calibri. It is visible in both this fixture and the corpus
/// workbook, it looks like the property write on the replaced text range rather than a rule, and
/// it moves no measurement: the corpus document lands 38 words from the reference with the stated
/// font applied to every run. So the assertions below are on positions and on which text is drawn
/// at all, never on the face.
/// </para>
/// <para>
/// Verified by reintroducing each defect; see <c>dotnet/probes/sheets-r32/mutate.sh</c>.
/// </para>
/// </remarks>
public sealed class SheetShownCommentTests
{
    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-cell-comment-shown.xlsx"));

        return (SpreadsheetPages)document.Layout();
    }

    private static List<IReadOnlyList<DrawnGlyphRun>> DrawnPages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-cell-comment-shown.xlsx"));

        RecordingDrawingSink sink = new();
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return [.. sink.Pages.Select(page => (IReadOnlyList<DrawnGlyphRun>)page.Runs)];
    }

    private static DrawnGlyphRun? Find(IReadOnlyList<DrawnGlyphRun> runs, string text)
        => runs.FirstOrDefault(run => run.Text.StartsWith(text, StringComparison.Ordinal));

    /// <summary>
    /// A comment whose VML says <c>visibility:visible</c> is drawn.
    /// </summary>
    [Fact]
    public void AShownCommentIsDrawn()
    {
        Find(DrawnPages()[0], "Shown note").ShouldNotBeNull();
    }

    /// <summary>
    /// A caption hangs off the commented cell, so a repeated print title carries it onto every
    /// page — at the same place on each.
    /// </summary>
    /// <remarks>
    /// The fixture's caption is anchored in rows 2 to 5 and belongs to <c>B1</c>. Placing it where
    /// its anchor names draws it on the first page alone, because rows 2 to 5 are printed once;
    /// placing it against <c>B1</c>, which the repeated band prints on both pages, is what
    /// LibreOffice does and what its PDF shows — the same glyph origin on page 1 and page 2.
    /// </remarks>
    [Fact]
    public void AShownCommentIsDrawnOnEveryPageThatRepeatsItsCell()
    {
        List<IReadOnlyList<DrawnGlyphRun>> pages = DrawnPages();
        pages.Count.ShouldBe(3);

        DrawnGlyphRun? first = Find(pages[0], "Shown note");
        DrawnGlyphRun? second = Find(pages[1], "Shown note");

        first.ShouldNotBeNull();
        second.ShouldNotBeNull();
        second.Origin.Y.Points.ShouldBe(first.Origin.Y.Points, 0.01);
        second.Origin.X.Points.ShouldBe(first.Origin.X.Points, 0.01);

        // The third page is the second column band, which the captions do not reach.
        Find(pages[2], "Shown note").ShouldBeNull();
    }

    /// <summary>
    /// Visibility is the shape's CSS, not the <c>x:Visible</c> element beside it.
    /// </summary>
    /// <remarks>
    /// <c>B3</c> states both, and LibreOffice's own export of this file has no annotation for it
    /// at all. Reading the element instead draws a comment the reference does not — three of the
    /// nine note shapes on the corpus workbook's sheets are exactly this shape.
    /// </remarks>
    [Fact]
    public void ACommentHiddenByItsStyleIsNotDrawn()
    {
        foreach (IReadOnlyList<DrawnGlyphRun> page in DrawnPages())
            Find(page, "Hidden note").ShouldBeNull();
    }

    /// <summary>
    /// The caption's rectangle comes from <c>x:Anchor</c>, not from the shape's CSS position.
    /// </summary>
    /// <remarks>
    /// LibreOffice puts the first glyph of the <c>B1</c> caption at 127.36 pt across and 109.64 pt
    /// down, and the <c>E1</c> caption's at 386.25 and 105.90. The fixture's CSS would put the
    /// first of them at 540 pt across and 504 pt down, off the printed block on the page above.
    /// The vertical tolerance is the looser of the two because the reference draws that first run
    /// a point larger than the file states — see the note on the class.
    /// </remarks>
    [Fact]
    public void TheCaptionIsPlacedFromTheAnchorAndNotFromTheCss()
    {
        IReadOnlyList<DrawnGlyphRun> page = DrawnPages()[0];

        DrawnGlyphRun? shown = Find(page, "Shown note");
        shown.ShouldNotBeNull();
        shown.Origin.X.Points.ShouldBe(127.36, 0.3);
        shown.Origin.Y.Points.ShouldBe(109.64, 0.8);

        DrawnGlyphRun? wide = Find(page, "Wide note");
        wide.ShouldNotBeNull();
        wide.Origin.X.Points.ShouldBe(386.25, 0.3);
        wide.Origin.Y.Points.ShouldBe(105.90, 0.8);
    }

    /// <summary>
    /// A shown caption widens the printed block, exactly as any other object on the sheet does.
    /// </summary>
    /// <remarks>
    /// The cells stop at column E. The <c>E1</c> caption reaches into column J, so
    /// <c>ScDocument::GetPrintArea</c>'s maximum of the cells' extent and the drawing layer's
    /// (<c>documen2.cxx:644-664</c>) ends at J — and the sheet then needs a second column band,
    /// which is the third page both renderers produce.
    /// </remarks>
    [Fact]
    public void AShownCaptionWidensThePrintedBlock()
    {
        SheetLayout sheet = Pages().Sheets[0];

        sheet.PrintedRange.LastColumn.ShouldBeGreaterThanOrEqualTo(9);
        Pages().Count.ShouldBe(3);
    }
}
