using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table takes the proportional line spacing of the paragraph above it, exactly as a paragraph does.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwFlowFrame::CalcUpperSpace</c> adds <c>nPrevLineSpacing</c> to <c>nUpper</c> in all four of its
/// branches and consults <c>pOwn-&gt;IsTextFrame()</c> only for the frame's <em>own</em> leading
/// (<c>sw/source/core/layout/flowfrm.cxx</c>:1655-1739). What follows is never asked, so a
/// <c>SwTabFrame</c> is handed the previous text frame's leading like anything else. We handed it only
/// between paragraphs, and a table below a proportionally-spaced paragraph therefore started a point
/// or so too high — every time.
/// </para>
/// <para>
/// <c>table-paragraph-leading.docx</c> is three paragraph-then-table pairs on A4 with 72 pt margins,
/// 11 pt Cambria (Caladea here, natural line 12.65 pt), no paragraph spacing at all so nothing but the
/// line spacing can move anything. Measured on LibreOffice 26.2.4.2, as offsets from the body's top
/// edge:
/// </para>
/// <list type="table">
///   <item><term>P at 150 %</term><description>baseline 9.90 — a paragraph's own first line takes no
///     interline spacing, which is <c>if( !IsParaLine() )</c> at <c>itrform2.cxx</c>:2425</description></item>
///   <item><term>table 1 top</term><description>19.00 = 12.65 + <b>6.30</b>, the leading P hands
///     down</description></item>
///   <item><term>Q at 100 %</term><description>baseline 42.50, i.e. hard against the table above —
///     a table hands nothing down, <c>GetSpacingValuesOfFrame</c> reporting a line spacing only for a
///     text frame</description></item>
///   <item><term>table 2 top</term><description>45.25 = table 1 bottom + 12.65 and no more, the control
///     that separates "a table takes the leading" from "a table is placed a point lower"</description></item>
/// </list>
/// <para>
/// Worth 1.00 pt at each of four boundaries on
/// <c>097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx</c> — the whole of that document's
/// 3.36 pt deficit against the reference, and the reason its trailing empty paragraph fitted on page 1
/// here and takes a second page there. 275 such boundaries in 85 of the corpus's 271 <c>.docx</c>.
/// See <c>probes/words-r61/</c>.
/// </para>
/// </remarks>
public sealed class TableParagraphLeadingTests
{
    private const string Fixture = "table-paragraph-leading.docx";

    /// <summary>The reference's own figures, as offsets from the body's top edge, in points.</summary>
    private const double FirstTableTop = 19.00;
    private const double SecondTableTop = 45.25;

    /// <summary>Half a twip and a little: the reference quantises these onto the twip grid.</summary>
    private const double Tolerance = 0.06;

    /// <summary>A 150 % paragraph pushes the table below it down by its leading.</summary>
    [Fact]
    public void ATableTakesTheLeadingOfTheParagraphAboveIt()
    {
        Tables()[0].Points.ShouldBe(FirstTableTop, tolerance: Tolerance);
    }

    /// <summary>
    /// A 100 % paragraph hands down nothing, so the table below it sits hard against the one above.
    /// </summary>
    /// <remarks>
    /// The control. Without it the first assertion is equally satisfied by adding a point to every
    /// table, which is a different rule and the wrong one.
    /// </remarks>
    [Fact]
    public void AParagraphAtAHundredPerCentHandsNothingToTheTableBelowIt()
    {
        Tables()[1].Points.ShouldBe(SecondTableTop, tolerance: Tolerance);
    }

    /// <summary>
    /// The gap between the two is the leading and nothing else, which no absolute figure can say.
    /// </summary>
    /// <remarks>
    /// Table 2's top less table 1's is the first table's own height plus the second paragraph's line,
    /// with no leading in it; table 1's top is the first paragraph's line plus its leading. Their
    /// difference, 6.30 pt, is the quantity under test, and it survives any constant this layout may be
    /// out by — a page origin, a border width, the cell's own padding.
    /// </remarks>
    [Fact]
    public void TheDifferenceBetweenThemIsTheLeadingItself()
    {
        (Tables()[0].Points - 12.65).ShouldBe(6.30, tolerance: Tolerance);
    }

    /// <summary>
    /// An <c>atLeast</c> line keeps its raise even as a paragraph's first line, and hands none of it on.
    /// </summary>
    /// <remarks>
    /// The fixture's third paragraph states <c>w:line="400" w:lineRule="atLeast"</c> over an 11 pt line
    /// whose natural height is 12.65 pt, so the raise is 7.35 pt. It is the paragraph's <em>only</em>
    /// line and it follows a table, which is both places this engine used to strip the raise: a
    /// paragraph's first line and the first line after something that hands nothing down. The
    /// reference draws its baseline 76.15 pt below the body's top edge — table 2's bottom at 58.90,
    /// plus the 7.35 pt raise, plus the 9.90 pt ascent — and puts table 3's top at 78.90, i.e. exactly
    /// 20.00 pt of paragraph and not a twip of leading beyond it, because
    /// <c>SwTextFrame::GetLineSpace</c> answers for <c>Prop</c> and <c>Fix</c> and not for <c>Min</c>.
    /// <para>
    /// Two claims in one number, and they pull opposite ways: strip the raise and the table comes out
    /// 7.35 pt high, hand the raise on as leading and it comes out 7.35 pt low.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnAtLeastParagraphKeepsItsRaiseAndHandsNoneOfItOn()
    {
        Tables()[2].Points.ShouldBe(78.90, tolerance: Tolerance);
    }

    /// <summary>The <c>atLeast</c> raise sits above the text, so the baseline moves with it.</summary>
    /// <remarks>
    /// <para>
    /// <b>The table position alone cannot see this, and finding that out is what the test is for.</b>
    /// Stripping the raise from the paragraph's first line shortens the paragraph by 7.35 pt, and
    /// handing that same raise on as leading lengthens the gap below it by 7.35 — so the table lands in
    /// exactly the same place either way and all three assertions above pass over a paragraph drawn
    /// 7.35 pt too high. Running the mutation through <c>verify-test.sh</c> is what exposed the
    /// cancellation; the baseline is the quantity that separates them.
    /// </para>
    /// <para>
    /// The reference draws it 76.15 pt below the body's top edge: table 2's bottom edge at 58.90, the
    /// 7.35 pt raise, and Caladea's 9.90 pt ascent at 11 pt.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheAtLeastRaiseSitsAboveTheTextAndNotBelowIt()
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(Fixture)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        IReadOnlyList<PlacedLine> lines = pages.Pages[0].Lines;

        lines.Count.ShouldBe(3, $"{Fixture}'s three body paragraphs are one line each");
        lines[2].Baseline.Points.ShouldBe(76.15, tolerance: Tolerance);
    }

    /// <summary>Where each table's top edge sits, relative to the body's top edge.</summary>
    private static List<Length> Tables()
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(Fixture)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(1, $"{Fixture} is three short pairs and fits one page");
        pages.Pages[0].Tables.Count.ShouldBe(3, $"{Fixture} holds three tables");

        Length top = pages.Pages[0].BodyArea.Y;
        return pages.Pages[0].Tables.Select(t => t.Area.Y - top).ToList();
    }
}
