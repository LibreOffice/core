using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A paragraph with no text draws no bullet.
/// </summary>
/// <remarks>
/// <para>
/// Both of LibreOffice's presentation readers say so in their own comments and on the same
/// condition — the paragraph's own character count, not anything its level states:
/// <c>oox/source/drawingml/textparagraph.cxx:193-197</c>, "empty paragraphs do not have bullets
/// in ppt", setting <c>NumberingLevel</c> to −1; and
/// <c>filter/source/msfilter/svdfppt.cxx:2363-2366</c>, "in PPT empty paragraphs never gets a
/// bullet", putting <c>EE_PARA_BULLETSTATE</c> false. So an author's blank line between two
/// bullets is a blank line rather than a bare bullet.
/// </para>
/// <para>
/// <b>Asserted on the layout rather than on a document, because no document could be generated
/// that shows it.</b> Our PPTX reader already declines to build a marker for a paragraph with
/// no text (<c>PptxTextBody</c>'s <c>if (!hasText) return null</c>), so the defect is reachable
/// only through the binary reader and through ODF — and LibreOffice's own PPT export writes the
/// empty paragraphs of a probe deck with no bullet at all, so a fixture converted through it
/// renders identically with the rule and without it. That was measured, not assumed: a
/// four-paragraph probe was authored as <c>.pptx</c>, converted to <c>.ppt</c>, and both forms
/// came back with two bullets on both binaries.
/// </para>
/// <para>
/// The corpus measurement is what the claim rests on instead. Counting extracted lines holding
/// nothing but a bullet glyph across the whole slides track, <b>75 of its 163 documents drew
/// more than the reference, 2405 lines in all</b> — 293 on
/// <c>2015-Civil-Rights-Website-training.ppt</c>, 185 on <c>71393_pp7.ppt</c>, 129 on
/// <c>Employment-Based_I-485.ppt</c>. <b>The word gate cannot see any of it</b>: <c>wc -w</c> in
/// the POSIX locale ignores a token made of non-ASCII bytes alone, so every one of those decks
/// passes on words while drawing bullets the reference does not.
/// </para>
/// </remarks>
public class SlideEmptyParagraphBulletTests
{
    private static readonly DocRect Area =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200));

    [Fact]
    public void OnlyTheParagraphsWithTextAreBulleted()
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(Body(), Area, new SlideFonts());

        // Three paragraphs, all carrying the same marker and the middle one empty. Each marker
        // is its own one-glyph run, so a bulleted blank line shows up as a fourth run.
        placed.Count.ShouldBe(4);
        placed.Count(run => run.Run.Glyphs.Count == 1).ShouldBe(2);

        // Both markers sit in the hanging indent, left of every line of text — which is what
        // says the two one-glyph runs are the markers and not two stray letters.
        double textLeft = placed
            .Where(run => run.Run.Glyphs.Count > 1)
            .Min(run => run.Run.Origin.X.Points);

        foreach (PlacedGlyphRun marker in placed.Where(run => run.Run.Glyphs.Count == 1))
        {
            marker.Run.Origin.X.Points.ShouldBeLessThan(textLeft);
        }
    }

    /// <summary>Three bulleted paragraphs, the middle one empty but still carrying a run.</summary>
    /// <remarks>
    /// The empty paragraph keeps a run of length nought, which is the shape every reader here
    /// produces and the reason the existing <c>Runs.Count == 0</c> guard never fired: a blank
    /// line carries one run so that its height is known.
    /// </remarks>
    private static SlideTextBody Body() => new()
    {
        Paragraphs =
        [
            Paragraph("First bullet"),
            Paragraph(""),
            Paragraph("Second bullet"),
        ],
    };

    private static SlideParagraph Paragraph(string text) => new(
        text,
        [new SlideTextRun(0, text.Length, "Liberation Sans", Length.FromPoints(18), 400, false, Colour.Black)],
        StartIndent: Length.FromPoints(18),
        FirstLineIndent: Length.FromPoints(-18),
        Marker: new SlideMarker("•", "Liberation Sans", IsSymbol: false));
}
