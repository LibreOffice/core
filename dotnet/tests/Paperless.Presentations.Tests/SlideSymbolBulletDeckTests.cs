using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The recode over a whole committed deck, laid out the way the renderer lays it out.
/// </summary>
/// <remarks>
/// <para>
/// <c>SlideSymbolBulletGlyphTests</c> builds its bodies in the test, which pins the rule but not
/// the path a real file takes through the reader. This reads
/// <c>tests/corpus/features/slide-symbol-bullet.pptx</c>, whose nine paragraphs were chosen to
/// separate the behaviours rather than to repeat one — see
/// <c>research/probes/slides-r14/make-symbol-bullet-deck.py</c> for why each is there.
/// </para>
/// <para>
/// Every expectation was read out of LibreOffice's own rendering of this same file, and the
/// recoded glyphs' advances agree with it to a hundredth of a point: <c>F0D8</c> 15.94 against
/// 15.93, <c>F0FC</c> 16.56 against 16.57, <c>F06E</c> 15.25 against 15.25.
/// </para>
/// <para>
/// Reintroducing the bug: make <c>SymbolFontRecode.TryRecode</c> return false and every
/// expectation below that names an <c>E</c>-range code point becomes U+2022.
/// </para>
/// </remarks>
public class SlideSymbolBulletDeckTests
{
    private const string Deck = "slide-symbol-bullet.pptx";

    /// <summary>Every marker on the slide, in the order the shapes were authored.</summary>
    private static List<GlyphRun> Markers()
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        read.ShouldBeAssignableTo<IPaginatedDocument>();
        IReadOnlyList<LaidOutSlide> slides =
            ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;
        slides.Count.ShouldBe(1);

        List<GlyphRun> markers = [];
        foreach (PlacedShape shape in slides[0].Shapes)
        {
            if (shape.Text is not { } text || text.Runs.Count == 0) continue;

            // Each box holds exactly one bulleted line, so its marker is the run drawn
            // furthest left — the label follows it on the same baseline.
            GlyphRun leftmost = text.Runs[0].Run;
            foreach (PlacedGlyphRun run in text.Runs)
            {
                if (run.Run.Origin.X < leftmost.Origin.X) leftmost = run.Run;
            }

            markers.Add(leftmost);
        }

        return markers;
    }

    [Fact]
    public void EverySymbolBulletInTheDeckIsDrawnAsItsOpenSymbolGlyph()
    {
        List<string> drawn = [.. Markers().Select(run => run.Text)];

        drawn.ShouldBe(
        [
            "\uE49E",   // Wingdings, stated as &#xF0D8;
            "\uE49E",   // Wingdings, the same slot stated as &#xD8; — must agree with the above
            "\uE4C2",   // Wingdings 0xFC, the corpus's commonest symbol bullet at 266 uses
            "\uE439",   // Wingdings 0x6E, what `Framing Europe.ppt` bullets with
            "\uE12C",   // Symbol 0xB7
            "\uE25D",   // Monotype Sorts 0xD8 — a different picture from Wingdings' same slot
            "\uE12C",   // Wingdings 0x7F is a hole in the table, substituted rather than .notdef
            "\u2022",   // a symbol face with no table keeps the old answer
            "\u2022",   // a non-symbol face is not a symbol position at all
        ]);
    }

    /// <summary>
    /// The recoded bullets are drawn from OpenSymbol, and the two that are not recoded are not.
    /// </summary>
    /// <remarks>
    /// Worth asserting separately from the code points because the two halves can fail
    /// independently: a recode that kept the requested face would draw the right code point out
    /// of a font that has no such glyph, which is the tofu this rule exists to avoid.
    /// </remarks>
    [Fact]
    public void TheRecodedBulletsComeFromOpenSymbolAndTheOthersDoNot()
    {
        List<GlyphRun> markers = Markers();
        markers.Count.ShouldBe(9);

        foreach (GlyphRun marker in markers.Take(7))
        {
            marker.Font.FamilyName.ShouldBe("OpenSymbol");
            marker.Glyphs.Count.ShouldBe(1);
        }

        markers[8].Font.FamilyName.ShouldNotBe("OpenSymbol");
    }

    /// <summary>
    /// A <em>pictorial</em> symbol bullet claims more width than the U+2022 that used to stand in
    /// for it, which is what the first line has to clear — so this is a layout change and not
    /// only a glyph change.
    /// </summary>
    /// <remarks>
    /// <strong>Only the pictorial ones.</strong> The first version of this asserted it of all
    /// seven recoded markers and failed, correctly: <c>Symbol</c>'s 0xB7 and the table hole both
    /// recode to U+E12C, which is OpenSymbol's own bullet and is <em>narrower</em> than Calibri's
    /// at the same size — 8.23 pt against 9.96 pt. The claim "a symbol bullet is wider than
    /// U+2022" is false for the bullets that really are bullets, so it is asserted where it
    /// means something instead of being weakened until it passes.
    /// </remarks>
    [Fact]
    public void APictorialSymbolBulletIsWiderThanTheFallbackItReplaces()
    {
        List<GlyphRun> markers = Markers();

        long fallback = markers[8].Glyphs[0].Advance.Emu;
        foreach (int index in (int[])[0, 1, 2, 3, 5])
        {
            markers[index].Glyphs[0].Advance.Emu.ShouldBeGreaterThan(fallback);
        }

        // The two that recode to OpenSymbol's bullet are the exception, and pinned as such.
        markers[4].Text.ShouldBe("\uE12C");
        markers[6].Text.ShouldBe("\uE12C");
        markers[4].Glyphs[0].Advance.Emu.ShouldBeLessThan(fallback);
    }
}
