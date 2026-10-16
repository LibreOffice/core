using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A Writer document's drawing shapes carry their text directly, and a group is one anchor.
/// </summary>
/// <remarks>
/// <para>
/// A <c>draw:frame</c> keeps its text in a <c>draw:text-box</c> child and a
/// <c>draw:custom-shape</c> does not: ODF 1.3 §10.6.2 lets every shape element take
/// <c>&lt;text:p&gt;*</c> directly, which is what <c>SdrTextObj</c> hangs its outliner text on.
/// A reader that looks only for the box therefore draws every shape in the document empty, and
/// the failure is silent — the shape is still placed, filled and stroked, so nothing on the page
/// says a paragraph is missing.
/// </para>
/// <para>
/// A <c>draw:g</c> is worse than empty: it states no <c>svg:width</c> and no <c>svg:height</c>,
/// so a reader that needs a size to place something drops the whole group. ODF gives a group no
/// coordinate system of its own — no <c>svg:viewBox</c>, no transform on the element — and
/// <c>xmloff</c> reads its children straight into the page's coordinates
/// (<c>xmloff/source/draw/ximpgrp.cxx</c>), so flattening the group to its shapes is the whole of
/// it. What the children do not restate is the placement: the group is the object anchored in the
/// text, and a child's graphic style is derived from the named <c>Frame</c> style, which says
/// <c>horizontal-pos="center"</c> and <c>vertical-pos="top"</c>. Letting that inherited default
/// win over the group's own <c>from-left</c> stacks every shape of the group at the top centre of
/// the page.
/// </para>
/// <para>
/// Reach, censused over the 338 <c>.odt</c> of the LibreOffice-converted corpus: <strong>2839
/// <c>draw:custom-shape</c> in 129 documents carry 42 773 alphanumeric characters</strong>, 7146
/// of them inside a <c>draw:g</c>; 9 <c>draw:rect</c> carry 45 more; no other <c>draw:</c>
/// element carries one character.
/// </para>
/// <para>
/// Ground truth is 26.2.4.2's own PDF of the fixture, read with <c>pdftotext -bbox</c>: ALPHA
/// centred at 240.874 pt, BRAVO at 141.602, CHARLIE at 425.074 and DELTA at 453.406, each within
/// a tenth of a point of its own shape's centre.
/// </para>
/// </remarks>
public sealed class OdfShapeTextTests
{
    /// <summary>The shapes' own paragraphs are drawn, each in its own shape.</summary>
    /// <remarks>
    /// Asserted on the drawn centre rather than on the left edge because that is what the
    /// placement decides: a shape's default <c>SdrTextHorzAdjust</c> is <c>BLOCK</c>, which gives
    /// the outliner the shape's own width as its minimum paper
    /// (<c>svx/source/svdraw/svdoashp.cxx</c>:2673-2676), so the paragraph's own alignment puts
    /// the label inside the box and the box is what this is about.
    /// </remarks>
    [Fact]
    public void EveryShapeDrawsItsTextWhereTheFilePutsIt()
    {
        Dictionary<string, DrawnWord> words = Labels();

        words.Keys.Order(StringComparer.Ordinal)
             .ShouldBe(["ALPHA", "BRAVO", "CHARLIE", "DELTA"]);

        // The page's left margin is 2 cm = 56.7 pt and every shape is 4 cm = 113.4 pt wide except
        // ALPHA, which is 5 cm. Centres, in points from the page's left edge:
        //   ALPHA   at svg:x 4 cm  -> 56.7 + 113.4 + 141.7/2  = 240.95
        //   BRAVO   at svg:x 1 cm  -> 56.7 +  28.3 + 113.4/2  = 141.75
        //   CHARLIE at svg:x 11 cm -> 56.7 + 311.8 + 113.4/2  = 425.20
        //   DELTA   by transform   -> 56.7 + 340.2 + 113.4/2  = 453.55
        Centre(words["ALPHA"]).ShouldBe(240.95, 0.3);
        Centre(words["BRAVO"]).ShouldBe(141.75, 0.3);
        Centre(words["CHARLIE"]).ShouldBe(425.20, 0.3);
        Centre(words["DELTA"]).ShouldBe(453.55, 0.3);
    }

    /// <summary>
    /// A group's two members do not land on top of one another at the page's centre.
    /// </summary>
    /// <remarks>
    /// The failure this pins is the one that survives reading the text at all: with the child's
    /// inherited <c>center</c>/<c>top</c> winning over the group's <c>from-left</c>, both members
    /// are drawn at <c>margin + (content width − frame width) / 2</c> and at the top margin, so
    /// the page holds two copies of one position. It is invisible to a glyph count taken from
    /// <c>pdftotext</c>, which reads two identical strings at one point as one.
    /// </remarks>
    [Fact]
    public void AGroupsMembersKeepTheirOwnPositions()
    {
        Dictionary<string, DrawnWord> words = Labels();

        // 21 cm page, 2 cm margins: the centre of the text column is 297.65 pt.
        const double ColumnCentre = 297.65;

        Math.Abs(Centre(words["BRAVO"]) - ColumnCentre).ShouldBeGreaterThan(100);
        Math.Abs(Centre(words["CHARLIE"]) - ColumnCentre).ShouldBeGreaterThan(100);
        Centre(words["CHARLIE"]).ShouldBeGreaterThan(Centre(words["BRAVO"]) + 200);

        // And they share the group's row rather than the page's top margin, which is 56.7 pt.
        words["BRAVO"].Baseline.ShouldBe(words["CHARLIE"].Baseline, 0.05);
        words["BRAVO"].Baseline.ShouldBeGreaterThan(150);
    }

    private static double Centre(DrawnWord word) => (word.Left + word.Right) / 2;

    private static Dictionary<string, DrawnWord> Labels()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("odt-shape-text.fodt")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return DrawnWords.On(sink.Pages[0])
                         .Where(word => word.Text is "ALPHA" or "BRAVO" or "CHARLIE" or "DELTA")
                         .ToDictionary(word => word.Text, word => word);
    }
}
