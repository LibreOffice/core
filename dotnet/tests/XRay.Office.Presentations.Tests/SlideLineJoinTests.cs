using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// How the corners of a slide shape's pen are drawn when the file says nothing about them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>DrawingML's unstated default is round and this read it as mitre.</strong>
/// <c>LineProperties::pushToPropMap</c> sets <c>ShapeProperty::LineJoint</c> only when the markup
/// states one of <c>a:round</c>, <c>a:bevel</c> or <c>a:miter</c>
/// (<c>oox/source/drawingml/lineproperties.cxx</c>:491-492), so an <c>a:ln</c> carrying none of
/// them leaves the draw layer's pool default, and <c>XLineJointItem</c>'s is
/// <c>LineJoint_ROUND</c> (<c>include/svx/xlinjoit.hxx</c>:35,
/// <c>svx/source/svdraw/svdattr.cxx</c>:182).
/// </para>
/// <para>
/// Measured as well as cited, because a citation is a hypothesis with a line number: on a probe
/// deck of nine presets each stating an <c>a:ln</c> with no join child, 26.2.4.2 writes
/// <c>1 j</c> — round — on all eleven of its stroke setups and we wrote <c>0 j</c> on all nine
/// (<c>probes/slides-subpath-paint/results.md</c>).
/// </para>
/// <para>
/// The Escher side is the other way round and is deliberately left alone:
/// <c>SvxMSDffManager::ApplyLineAttributes</c> defaults <c>DFF_Prop_lineJoinStyle</c> to
/// <c>mso_lineJoinMiter</c> for every shape type but <c>mso_sptMin</c>
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:1052-1061), which is what
/// <c>PptSlideLayout</c> already does.
/// </para>
/// </remarks>
public class SlideLineJoinTests
{
    private const string Deck = "slide-shape-features.pptx";

    [Fact]
    public void ALineStatingNoJoinIsRoundedAtItsCorners()
    {
        // The four pens on the deck's third slide: three connectors and a dashed rectangle. Every
        // one states `a:ln` with a width, a colour and no join child at all.
        List<Stroke> pens = [.. Slides()[2].Shapes
            .Select(shape => shape.Line)
            .OfType<Stroke>()];

        pens.Count.ShouldBe(4);
        pens.ShouldAllBe(pen => pen.Join == LineJoin.Round);

        // The cap is not part of this and must not move with it: DrawingML's `cap` attribute is
        // absent on all four, and both stacks draw a butt end.
        pens.ShouldAllBe(pen => pen.Cap == LineCap.Butt);
    }

    private static IReadOnlyList<LaidOutSlide> Slides()
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        return ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;
    }
}
