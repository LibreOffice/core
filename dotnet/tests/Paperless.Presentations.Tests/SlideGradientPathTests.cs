using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// <c>a:gradFill/a:path/a:fillToRect</c>: where the focus lands, and when a circle path stops
/// being a circle.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is LibreOffice 24.2.7.2's own answer for the same four slides, read
/// out of its flat-ODF export of this deck rather than inferred from
/// <c>oox/source/drawingml/fillproperties.cxx</c> in the surrounding checkout — which is a
/// development branch and states the *opposite* of the second rule. The measurement and the
/// probe that produced it are in <c>probes/slides-r39/gradient-path.md</c>.
/// </para>
/// <para>
/// The four slides exist to separate the three readings that all fit the common case:
/// </para>
/// <list type="bullet">
/// <item><description><b>No clamp.</b> The stock Office theme gradient states
/// <c>t="-80000" b="180000"</c>, a focus 80% of the box above its own top edge. Unclamped every
/// point of the box is past the last stop and the fill comes out flat — which is what we drew,
/// on a <c>fillToRect</c> carried by 79 of the corpus's 114 zip-container decks. The reference
/// puts it on the top edge.</description></item>
/// <item><description><b>Always radial.</b> A circle path focused on a corner is imported as a
/// 45-degree linear ramp instead, stop 0 at the corner.</description></item>
/// <item><description><b>No truncation.</b> The focus is kept as a whole number of per cent, so
/// a stated 0.5% is 0 and crosses the corner test while 1% does not. The last two slides differ
/// only in that half of one per cent.</description></item>
/// </list>
/// </remarks>
public class SlideGradientPathTests
{
    private const string Deck = "slide-gradient-path.pptx";

    // The deck's own p:sldSz.
    private const long SlideWidth = 9144000;
    private const long SlideHeight = 6858000;

    [Fact]
    public void AFocusAboveTheBoxIsClampedToItsTopEdge()
    {
        GradientPaint background = Background(0);

        background.Kind.ShouldBe(GradientKind.Radial);
        background.Start.X.Emu.ShouldBe(SlideWidth / 2);

        // The file says -80%; unclamped this is -5486400 and the whole slide comes out flat.
        background.Start.Y.Emu.ShouldBe(0);
    }

    [Fact]
    public void AFocusOnACornerBecomesADiagonalLinearRamp()
    {
        GradientPaint background = Background(1);

        background.Kind.ShouldBe(GradientKind.Linear);

        // Stop 0 sits at the bottom-right corner the fillToRect names, and the ramp runs at 45
        // degrees to the opposite one: equal and opposite displacement in both axes.
        (background.Start.X.Emu - background.End.X.Emu)
            .ShouldBe(background.Start.Y.Emu - background.End.Y.Emu);
        background.Start.X.Emu.ShouldBeGreaterThan(background.End.X.Emu);

        // At 45 degrees the ramp spans w + h projected, so each corner is (w + h) / 4 from the
        // centre in each axis.
        background.Start.X.Emu.ShouldBe((SlideWidth / 2) + ((SlideWidth + SlideHeight) / 4));
        background.Start.Y.Emu.ShouldBe((SlideHeight / 2) + ((SlideWidth + SlideHeight) / 4));
    }

    [Fact]
    public void HalfAPerCentTruncatesOntoTheCornerAndAWholeOneDoesNot()
    {
        // Both slides state a focus in the top-left corner region and differ by half of one per
        // cent. The reference imports the first as linear at 225 degrees and the second as a
        // radial gradient centred at 1%, 1%.
        Background(2).Kind.ShouldBe(GradientKind.Linear);

        GradientPaint kept = Background(3);
        kept.Kind.ShouldBe(GradientKind.Radial);
        kept.Start.X.Emu.ShouldBe(SlideWidth / 100);
        kept.Start.Y.Emu.ShouldBe(SlideHeight / 100);
    }

    private static GradientPaint Background(int slide)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        IReadOnlyList<LaidOutSlide> slides = ((SlidePages)((IPaginatedDocument)document).Layout()).Slides;

        return slides[slide].Background.ShouldBeOfType<GradientPaint>();
    }
}
