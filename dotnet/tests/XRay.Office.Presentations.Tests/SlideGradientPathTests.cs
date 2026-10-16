using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// <c>a:gradFill/a:path/a:fillToRect</c>: where the focus lands, and when a circle path stops
/// being a circle.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is the reference binary's own answer for the same four slides, read
/// out of its flat-ODF export of this deck rather than inferred from
/// <c>oox/source/drawingml/fillproperties.cxx</c> in the surrounding checkout. The probe that
/// produced it is <c>probes/slides-r39/make-gradient-path-fixture.py</c>, re-run against
/// <b>26.2.4.2</b> in round 59.
/// </para>
/// <para>
/// <b>One of the three rules did not survive the version bump and this file used to assert
/// it.</b> Round 39 measured a corner focus as <c>draw:style="linear"</c> at 45°; 26.2.4.2
/// exports <c>radial</c> for all four arms, corners included. What did survive is the clamp and
/// the truncation, and the truncation is still visible here — slides 3 and 4 differ by half of
/// one per cent and land on <c>0%</c> and <c>1%</c>.
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
/// <item><description><b>A corner is special.</b> It is not, on this binary: a circle path
/// focused on a corner is a radial gradient centred on that corner, exactly as any other focus
/// is.</description></item>
/// <item><description><b>No truncation.</b> The focus is kept as a whole number of per cent, so
/// a stated 0.5% lands on 0 and 1% does not. The last two slides differ only in that half of
/// one per cent, and it is what separates the two readings.</description></item>
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
    public void AFocusOnACornerIsRadialAndCentredOnThatCorner()
    {
        // `l="100000" t="100000"` — the bottom-right corner. 26.2.4.2's own export of this slide
        // is `draw:style="radial" draw:cx="100%" draw:cy="100%"`.
        GradientPaint background = Background(1);

        background.Kind.ShouldBe(GradientKind.Radial);
        background.Start.X.Emu.ShouldBe(SlideWidth);
        background.Start.Y.Emu.ShouldBe(SlideHeight);
    }

    [Fact]
    public void HalfAPerCentTruncatesToZeroAndAWholeOneDoesNot()
    {
        // The two slides state a focus in the top-left corner region and differ by half of one
        // per cent. The reference exports the first at `draw:cx="0%"` and the second at
        // `draw:cx="1%"`, both radial — which is the whole of what says the per cent is
        // truncated rather than kept.
        GradientPaint truncated = Background(2);
        truncated.Kind.ShouldBe(GradientKind.Radial);
        truncated.Start.X.Emu.ShouldBe(0);
        truncated.Start.Y.Emu.ShouldBe(0);

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
