using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A shape's drop shadow: where the second copy goes, what it is drawn in, and what it carries.
/// </summary>
/// <remarks>
/// <para>
/// A shadow is the shape drawn again — offset, with every colour replaced by one, behind itself
/// (<c>svx/source/sdr/primitive2d/sdrdecompositiontools.cxx:860</c>). Nothing in Paperless drew
/// one at all until this, on either the OOXML or the binary side, and the gap is easy to
/// under-rate from a word count: on <c>slides/batch-016/ppt/pres_ioc_phuket.ppt</c> the reference
/// draws "National" fourteen times where we drew it seven, seven <em>pairs</em> 6.01 pt apart in
/// both axes, and the deck's coins and boxes carry a black offset behind every one of them.
/// </para>
/// <para>
/// <c>slide-drop-shadow.pptx</c> puts five one-inch rectangles side by side, each isolating one
/// decision: a shadow stated inline at 45°, one taken from the theme's first effect style, one
/// whose shape states an empty effect list, one whose shape states a glow and no shadow, and one
/// behind a fill that is 30% opaque. Every expectation below was read out of LibreOffice
/// 24.2.7.2's own flat-ODF export of that deck rather than derived from the specification, and
/// two of the five disagree with what the source alone says.
/// </para>
/// </remarks>
public class SlideDropShadowTests
{
    private const string Deck = "slide-drop-shadow.pptx";

    /// <summary>
    /// The same deck as LibreOffice writes it in ODF, which is the ODP reader's expectations.
    /// </summary>
    /// <remarks>
    /// Produced by <c>soffice --convert-to odp</c> from the <c>pptx</c>, so it is not a second
    /// hand-written fixture that could drift from the first: it is the reference renderer's own
    /// statement of what that deck's shadows are, in the format whose attributes hold exactly the
    /// quantities the model does. A reader disagreeing with it disagrees with the binary that
    /// makes every reference PDF this corpus is measured against.
    /// </remarks>
    private const string OdfDeck = "slide-drop-shadow.odp";

    /// <summary>A twentieth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.05;

    /// <summary>
    /// A stated distance and direction become an offset in both axes.
    /// </summary>
    /// <remarks>
    /// <c>dist="76200" dir="2700000"</c> is a twelfth of an inch at 45°, and the offset is
    /// <c>cos(dir) × dist</c> across and <c>sin(dir) × dist</c> down — down, because the drawing
    /// space has y growing downwards. LibreOffice's export of this shape reads
    /// <c>draw:shadow-offset-x="0.149cm"</c> and the same in y, which is 53881 EMU, or 4.243 pt.
    /// </remarks>
    [Theory]
    [InlineData(Deck)]
    [InlineData(OdfDeck)]
    public void AnAngledShadowOffsetsInBothAxes(string deck)
    {
        SlideShadow shadow = Shadow(deck, "Angled");

        shadow.OffsetX.Points.ShouldBe(4.243, TolerancePoints);
        shadow.OffsetY.Points.ShouldBe(4.243, TolerancePoints);
        shadow.Colour.ShouldBe(Colour.Black);
        shadow.Opacity.ShouldBe(1.0, 1e-6);
    }

    /// <summary>
    /// A shape that states no effects of its own takes the theme's, through <c>a:effectRef</c>.
    /// </summary>
    /// <remarks>
    /// This is where most shadows in real decks come from and it is not close: across the 112
    /// <c>pptx</c> decks of the slides corpus, 1120 slide shapes reach a shadow this way against
    /// 352 that state one on their own <c>p:spPr</c>. A reader that only looks at <c>spPr</c>
    /// finds under a quarter of them.
    /// </remarks>
    [Theory]
    [InlineData(Deck)]
    [InlineData(OdfDeck)]
    public void AShapeTakesItsShadowFromTheThemesEffectStyle(string deck)
    {
        SlideShadow shadow = Shadow(deck, "Themed");

        // dist="20000" dir="5400000" — ninety degrees, so straight down and nothing across.
        shadow.OffsetX.Emu.ShouldBe(0);
        shadow.OffsetY.Points.ShouldBe(1.575, TolerancePoints);

        // a:alpha val="38000" is the shadow's transparency and not part of its colour.
        shadow.Colour.ShouldBe(Colour.Black);
        shadow.Opacity.ShouldBe(0.38, 0.005);
    }

    /// <summary>
    /// Neither an empty effect list nor one holding a different effect drops the theme's shadow.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The source says otherwise, which is why this is a test rather than a comment.
    /// <c>EffectProperties::assignUsed</c> (<c>oox/source/drawingml/effectproperties.cxx:46-58</c>)
    /// clears the inherited effect list whenever the shape states any effect of its own, so a
    /// shape holding only an <c>a:glow</c> should lose the theme's shadow and one holding
    /// <c>&lt;a:effectLst/&gt;</c> should keep it.
    /// </para>
    /// <para>
    /// LibreOffice 24.2.7.2 keeps it in both cases: its flat-ODF export of this deck gives
    /// <c>draw:shadow="visible"</c> with the themed 38% black to the plain shape, the
    /// empty-list shape and the glow shape alike. The binary is what made the reference PDFs, so
    /// the rule here is "the first source that states an outer shadow wins", not "the first
    /// source that states anything".
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(Deck, "EmptyList")]
    [InlineData(Deck, "Glow")]
    [InlineData(OdfDeck, "EmptyList")]
    [InlineData(OdfDeck, "Glow")]
    public void AnEffectListWithoutAShadowDoesNotSuppressTheThemes(string deck, string name)
    {
        SlideShadow shadow = Shadow(deck, name);

        shadow.OffsetX.Emu.ShouldBe(0);
        shadow.OffsetY.Points.ShouldBe(1.575, TolerancePoints);
        shadow.Opacity.ShouldBe(0.38, 0.005);
    }

    /// <summary>
    /// A blurred shadow does not carry the shape's text; an unblurred one does.
    /// </summary>
    /// <remarks>
    /// This is the difference between a fix and a corpus-wide regression. LibreOffice rasterises
    /// a blurred shadow — <c>ShadowPrimitive2D</c> renders its children to a bitmap and softens
    /// that (<c>drawinglayer/source/primitive2d/shadowprimitive2d.cxx:91-140</c>) — so the
    /// reference PDF of a deck with themed shadows holds greyscale images with soft masks and no
    /// duplicated words. Verified on <c>passiv.pptx</c>: every page carries a 918 × 272 gray JPEG
    /// plus smask, and its extractable word count is unchanged at 1256. Drawing the shadow's text
    /// under blur would add words to every such deck.
    /// </remarks>
    [Fact]
    public void OnlyAnUnblurredShadowCarriesTheShapesText()
    {
        Shadow(Deck, "Angled").CarriesText
            .ShouldBeTrue("blurRad=\"0\", so the reference keeps it vector");
        Shadow(Deck, "Themed").CarriesText
            .ShouldBeFalse("blurRad=\"40000\", so the reference rasterises it");
    }

    /// <summary>
    /// The shadow of a translucent fill is as translucent as the fill.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shadow replaces a primitive's <em>colour</em> and not its transparency —
    /// <c>BColorModifier_replace</c>, which is what <c>ShadowPrimitive2D</c> embeds its children
    /// in. Filling the outline with a flat opaque colour instead looks like the same operation
    /// and is not, and the difference is a whole slide rather than a detail: page 34 of
    /// <c>Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx</c> is covered by a shape
    /// whose gradient runs from zero to 30% alpha and which states a shadow with no distance, so
    /// the copy sits exactly underneath it. Cast opaque, it tinted the whole slide and that
    /// page's unaccounted ink went from 0.18% to 13.52%; cast with the fill's own alpha it is
    /// invisible, which is what the reference shows.
    /// </para>
    /// <para>
    /// Asserted on what reaches the sink rather than on the model, because the model has no place
    /// to hold it: the alpha belongs to the shape's fill and is applied when the shadow is drawn.
    /// </para>
    /// <para>
    /// The <c>pptx</c> only, and the reason is a separate defect worth naming rather than hiding
    /// in a tolerance: ODF states a fill's transparency as <c>draw:opacity="30%"</c> on the
    /// graphic style rather than as an alpha on the colour, and the ODP reader does not read it —
    /// so the same shape comes out fully opaque there and its shadow correctly matches. The rule
    /// under test lives in <c>SlideDrawing</c> and is format-independent; what the ODP cannot
    /// supply is the input.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShadowKeepsTheFillsOwnTransparency()
    {
        RecordingSink sink = Draw(Deck);

        // The translucent shape's fill is C00000 at 30%; its shadow is black at the same 30%.
        SolidPaint fill = sink.Fills
            .Select(f => f.Paint)
            .OfType<SolidPaint>()
            .First(p => p.Colour.R == 0xC0 && p.Colour.G == 0 && p.Colour.B == 0);

        fill.Colour.A.ShouldBe((byte)77, "30000 of 100000, rounded");

        sink.Fills
            .Select(f => f.Paint)
            .OfType<SolidPaint>()
            .Select(p => p.Colour)
            .ShouldContain(
                new Colour(0, 0, 0, fill.Colour.A),
                "the shadow of a 30% fill is 30% opaque");
    }

    /// <summary>The shadow is drawn before the shape it belongs to, and offset from it.</summary>
    /// <remarks>
    /// Before, because a shadow drawn afterwards covers the shape; offset on the state stack
    /// rather than baked into the geometry, because a rotated shape's text carries a matrix of
    /// its own that the translation has to compose with rather than replace.
    /// </remarks>
    [Theory]
    [InlineData(Deck)]
    [InlineData(OdfDeck)]
    public void TheShadowIsDrawnBehindTheShapeAtTheStatedOffset(string deck)
    {
        RecordingSink sink = Draw(deck);

        int shadow = sink.Fills.FindIndex(f => f.Paint is SolidPaint { Colour.R: 0, Colour.G: 0, Colour.B: 0 });
        int shape = sink.Fills.FindIndex(f => f.Paint is SolidPaint { Colour.R: 0x9B, Colour.G: 0xBB });

        shadow.ShouldBeGreaterThanOrEqualTo(0);
        shape.ShouldBeGreaterThan(shadow, "the shadow goes down first");

        sink.Fills[shadow].Transform.E.ShouldBe(
            Shadow(deck, "Angled").OffsetX.Emu, 1.0, "translated by the shadow's own offset");
        sink.Fills[shadow].Transform.F.ShouldBe(Shadow(deck, "Angled").OffsetY.Emu, 1.0);
    }

    // ------------------------------------------------------------------------- the machinery

    private static SlideShadow Shadow(string deck, string name)
        => Slide(deck).Shapes.First(s => s.Name == name).Shadow.ShouldNotBeNull();

    private static RecordingSink Draw(string deck)
    {
        RecordingSink sink = new();
        SlideDrawing.Draw(Slide(deck), sink);
        return sink;
    }

    private static LaidOutSlide Slide(string deck)
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(deck)));

        IReadOnlyList<LaidOutSlide> slides =
            ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;

        slides.Count.ShouldBe(1);
        return slides[0];
    }

    /// <summary>
    /// A sink that keeps every fill it is given together with the transform in force.
    /// </summary>
    /// <remarks>
    /// Not the test kit's <c>RecordingDrawingSink</c>, and the difference is the whole reason
    /// this exists: that one counts <c>Transform</c> calls rather than composing them, and a
    /// shadow is the <em>same geometry as its shape</em> differing only by the translation on the
    /// state stack. A recorder that drops the matrix cannot tell the two apart at all.
    /// </remarks>
    private sealed class RecordingSink : IDrawingSink
    {
        private readonly Stack<AffineTransform> _stack = new();
        private AffineTransform _current = AffineTransform.Identity;

        public List<(GraphicsPath Path, Paint Paint, AffineTransform Transform)> Fills { get; } = [];

        public List<(GlyphRun Run, Paint Paint, AffineTransform Transform)> Runs { get; } = [];

        public void BeginPage(DocSize size)
        {
        }

        public void EndPage()
        {
        }

        public void Save() => _stack.Push(_current);

        public void Restore() => _current = _stack.Count > 0 ? _stack.Pop() : AffineTransform.Identity;

        public void Transform(AffineTransform transform)
            => _current = AffineTransform.Concat(transform, _current);

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero)
        {
        }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
            => Fills.Add((path, paint, _current));

        public void StrokePath(GraphicsPath path, Stroke stroke)
        {
        }

        public void DrawGlyphRun(GlyphRun run, Paint paint) => Runs.Add((run, paint, _current));

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
        {
        }

        public void BeginTransparencyGroup(double opacity)
        {
        }

        public void EndTransparencyGroup()
        {
        }
    }
}
