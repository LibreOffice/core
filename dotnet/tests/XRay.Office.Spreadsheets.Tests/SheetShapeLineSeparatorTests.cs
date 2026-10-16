using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A line separator inside one <c>a:t</c> ends the paragraph, and two in a row leave a blank line.
/// </summary>
/// <remarks>
/// <para>
/// <strong>LibreOffice never sees the separator as a character.</strong> Every importer hands its
/// string to the EditEngine — <c>TextRun::insertAt</c> calls <c>xText-&gt;insertString</c>
/// (<c>oox/source/drawingml/textrun.cxx</c>:124) — and
/// <c>ImpEditEngine::ImpInsertText</c> normalises the line ends with
/// <c>convertLineEnd(rStr, LINEEND_LF)</c>, then walks the string from separator to separator and
/// calls <c>ImpInsertParaBreak</c> whenever the separator was not the end of it
/// (<c>editeng/source/editeng/impedit2.cxx</c>:2864-2983). Its own comment names the case two in a
/// row make: <c>// Start == End =&gt; empty line</c>. So <c>CCC\n\nDDD</c> is three paragraphs, the
/// middle one empty.
/// </para>
/// <para>
/// <strong>Shaping the separator instead loses a whole line of height.</strong> That is what this
/// painter did: <c>U+000A</c> went through the shaper as an ordinary character, came out
/// zero-width, and joined the two sentences into one run — which made the body one line
/// <em>shorter</em> than the reference's and so, on a box stating <c>vertOverflow="clip"</c>,
/// stopped the clip firing at all. On
/// <c>070_Equipment_inventory_list_Use_this_template_fd524c8a.xlsx</c>'s three slicer
/// placeholders the notice is six line slots with the breaks and five without, the boxes hold
/// five, and 26.2.4.2 drops the last line of each while we drew all of it: 1128 alphanumerics
/// against the reference's 1028, and 1004 now. A lost paragraph and an unfired clip are one cause
/// there, not two.
/// </para>
/// <para>
/// <strong>The fixture varies the number of separators and nothing else.</strong> Two text boxes
/// on one sheet, one 12 pt Liberation Sans run each, wide enough not to wrap and tall enough not
/// to clip: <c>AAA\nBBB</c> and <c>CCC\n\nDDD</c>. The second gap must be exactly twice the first,
/// which is the empty paragraph stated as arithmetic rather than as a line count.
/// </para>
/// <para>
/// The expectation is the reference's own. LibreOffice <b>26.2.4.2</b> renders the committed
/// workbook with baselines at <b>579.60 / 566.19</b> and <b>579.60 / 552.78</b> — gaps of 13.41
/// and 26.82 — and ours are 13.41 and 26.82.
/// </para>
/// </remarks>
public sealed class SheetShapeLineSeparatorTests
{
    private const string Fixture = "sheet-shape-line-separator.xlsx";

    private static DrawnPage Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(Fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages.Single();
    }

    private static double BaselineOf(DrawnPage page, string text)
        => page.Runs.Single(run => run.Text == text).Origin.Y.Points;

    /// <summary>
    /// The two sentences are two runs on two lines, not one run with a swallowed character.
    /// </summary>
    /// <remarks>
    /// Both halves matter. That <c>AAA</c> and <c>BBB</c> are separate runs is the paragraph break;
    /// that no run carries the separator itself is what stops it being drawn as a glyph — the
    /// failure this closes rendered <c>…of Excel.</c> and <c>If the shape…</c> as
    /// <c>Excel.If the shape…</c>, with no space and no break.
    /// </remarks>
    [Fact]
    public void ASeparatorInsideOneRunEndsTheParagraph()
    {
        DrawnPage page = Draw();

        page.Runs.Select(run => run.Text).ShouldContain("AAA");
        page.Runs.Select(run => run.Text).ShouldContain("BBB");
        page.Runs.ShouldAllBe(run => !run.Text.Contains('\n') && !run.Text.Contains('\r'));

        BaselineOf(page, "BBB").ShouldBeGreaterThan(BaselineOf(page, "AAA"));
    }

    /// <summary>
    /// Two separators in a row leave an empty paragraph, so the gap is exactly twice.
    /// </summary>
    /// <remarks>
    /// Stated as a ratio rather than as a line height, so the assertion says <em>an empty paragraph
    /// occupies one line</em> and stays true whatever the face's metrics are later measured to be.
    /// A painter that collapsed the run of separators into one break would put both gaps at 13.41.
    /// </remarks>
    [Fact]
    public void TwoSeparatorsLeaveABlankLineBetweenThem()
    {
        DrawnPage page = Draw();

        double single = BaselineOf(page, "BBB") - BaselineOf(page, "AAA");
        double doubled = BaselineOf(page, "DDD") - BaselineOf(page, "CCC");

        single.ShouldBeGreaterThan(0.0);
        doubled.ShouldBe(2 * single, 0.05);
    }
}
