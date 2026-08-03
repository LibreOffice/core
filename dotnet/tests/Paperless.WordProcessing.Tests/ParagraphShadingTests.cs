using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A paragraph's own background: <c>w:pPr/w:shd</c>, and the rectangle it fills.
/// </summary>
/// <remarks>
/// <para>
/// A background nobody draws is the worst kind of missing feature, because the page it ruins looks
/// <em>empty</em> rather than wrong: a shaded heading is nearly always set in white, so losing the
/// fill loses the heading too. That is why the assertions below are on the fills' geometry rather
/// than on a "shading was read" flag — the property was reachable from the reader long before
/// anything painted it.
/// </para>
/// <para>
/// Every number here was measured from LibreOffice 24.2.7.2's own rendering of the same corpus
/// document at 150 dpi, and each pins a claim that does not follow from the others:
/// </para>
/// <list type="bullet">
/// <item>the fill spans the paragraph's indents, not the column, and ignores the first-line
/// indent — Writer fills <c>getFramePrintArea()</c>, <c>paintfrm.cxx:1265</c>;</item>
/// <item>it stops at the first and last line, so the space before and after the paragraph stays
/// unfilled;</item>
/// <item>except between two paragraphs filled the <em>same</em> colour, where the gap is filled —
/// <c>aRect.Top( getFrameArea().Top() )</c>, <c>paintfrm.cxx:7033</c>;</item>
/// <item>a paragraph style's <c>w:shd</c> counts, which is where a real document states it;</item>
/// <item>and <c>w:fill="auto"</c> in a derived style cancels the parent's fill.</item>
/// </list>
/// </remarks>
public sealed class ParagraphShadingTests
{
    /// <summary>The document's two fills, as its markup states them.</summary>
    private static readonly Colour Navy = Colour.FromRgb(0x1E0E53);
    private static readonly Colour Blue = Colour.FromRgb(0x487DCC);

    /// <summary>Half a point, which is finer than any of these measurements needs to be.</summary>
    private const double Tolerance = 0.5;

    [Fact]
    public void EveryShadedParagraphIsFilledAndNothingElseIs()
    {
        List<(DocRect Bounds, Paint Paint)> fills = FillsOnFirstPage();

        // Six shaded paragraphs in the document; five rectangles, because the two joined ones are one.
        fills.Count.ShouldBe(5, "shaded paragraphs, after the joined pair merges");

        fills.Count(fill => Solid(fill.Paint) == Navy).ShouldBe(3);
        fills.Count(fill => Solid(fill.Paint) == Blue).ShouldBe(2);
    }

    [Fact]
    public void TheIndentsNarrowTheFillAndTheFirstLineIndentDoesNot()
    {
        // "Direct shading, indented, spaced": w:ind left=720 right=1440 firstLine=360, in a 21 cm
        // page with 2 cm margins. LibreOffice fills x 193..971 px at 150 dpi — 36 pt in from the
        // left margin and 72 pt in from the right — and starts the *text* 18 pt further in again.
        DocRect fill = FillsOnFirstPage()[0].Bounds;

        fill.X.Points.ShouldBe(56.7 + 36, Tolerance, "left margin plus the left indent");
        fill.Right.Points.ShouldBe(595.3 - 56.7 - 72, Tolerance, "right margin less the right indent");
    }

    [Fact]
    public void TheSpaceBeforeAndAfterAParagraphIsNotFilled()
    {
        // The same paragraph: 400 twips — 20 pt — of spacing on each side, and a single line of
        // 10 pt text between them. A fill covering the spacings would be some 40 pt taller.
        DocRect fill = FillsOnFirstPage()[0].Bounds;

        fill.Height.Points.ShouldBeLessThan(20, "one line's box, with neither spacing in it");
    }

    [Fact]
    public void TwoParagraphsFilledTheSameColourAreOneRectangleAndTwoAreNot()
    {
        List<(DocRect Bounds, Paint Paint)> fills = FillsOnFirstPage();

        // "Joined above" has 400 twips of spacing after it and "Joined below" the same fill, so
        // LibreOffice paints one band 90 px tall across both and the 20 pt between them.
        DocRect joined = fills[1].Bounds;
        joined.Height.Points.ShouldBeGreaterThan(40, "two lines and the 20 pt gap between them");

        // "Disjoint above" has the same spacing and "Disjoint below" a different fill, so the gap
        // stays white and the two are separate rectangles of the two colours.
        Solid(fills[2].Paint).ShouldBe(Navy);
        Solid(fills[3].Paint).ShouldBe(Blue);
        fills[2].Bounds.Height.Points.ShouldBeLessThan(20, "one line, the spacing after it unfilled");
        fills[3].Bounds.Y.ShouldBeGreaterThan(
            fills[2].Bounds.Bottom, "the gap between two different fills is left unpainted");
    }

    [Fact]
    public void AStylesShadingCountsAndADerivedStyleCanCancelIt()
    {
        List<(DocRect Bounds, Paint Paint)> fills = FillsOnFirstPage();

        // "Styled heading" states no w:shd of its own: the fill comes from the ShadedHeading style.
        Solid(fills[4].Paint).ShouldBe(Blue);

        // "Cleared heading" is based on ShadedHeading and overrides it with w:fill="auto", which
        // means "let what is behind show" and so is no fill at all. If it were painted there would
        // be a sixth rectangle below the fifth.
        fills[4].Bounds.Y.Points.ShouldBeLessThan(
            270, "the last fill is the styled heading, not the cleared one below it");
    }

    /// <summary>The paragraph shading rectangles on the document's first page, in drawing order.</summary>
    /// <remarks>
    /// Filtered to full-width-ish rectangles is deliberately <em>not</em> done: the corpus document
    /// contains nothing else that fills, so anything extra here is a bug worth failing on.
    /// </remarks>
    private static List<(DocRect Bounds, Paint Paint)> FillsOnFirstPage()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("paragraph-shading.docx"));

        WordProcessingPages pages = (WordProcessingPages)document.Layout();
        pages.Count.ShouldBe(1, "the corpus document is one page");

        PlacedDrawingSink sink = new();
        PageDrawing.Draw(pages.Pages[0], pages.Blocks, sink);

        return sink.Fills;
    }

    private static Colour Solid(Paint paint) => ((SolidPaint)paint).Colour;
}
