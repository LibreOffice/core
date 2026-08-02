using Paperless.MsBinary.Escher;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A DOC line shape is a diagonal, not a box, so it has neither a fill nor a rectangular border.
/// </summary>
/// <remarks>
/// <para>
/// <c>SvxMSDffManager::ImportShape</c> builds <c>mso_sptLine</c> as a two-point path
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:4403) and a straight connector as an <c>SdrEdgeObj</c>
/// whose track runs corner to corner (:4820). Neither has an area, so neither reads <c>fFilled</c>.
/// </para>
/// <para>
/// The default the box branch takes is <em>opaque white</em>, which is why this is not a cosmetic
/// difference: on <c>words/batch-001/doc/foca_form_1.doc</c> the two connectors that cross out the unused
/// half of the certificate were drawn as two white rectangles over it, hiding six table cells of text that
/// extracted perfectly and could not be seen. The page's ink imbalance against the reference falls from
/// 0.90% to 0.08% with them drawn as lines.
/// </para>
/// </remarks>
public sealed class DocLineShapeTests
{
    [Theory]
    [InlineData(EscherShapeTypes.Line)]
    [InlineData(EscherShapeTypes.StraightConnector)]
    public void ALineShapeHasNoFill(ushort type)
    {
        PageFrame frame = Build(type, EscherShapeAttributes.None).ShouldNotBeNull();

        frame.IsLine.ShouldBeTrue();
        frame.Fill.ShouldBeNull("a shape with no area cannot have a background");
    }

    [Fact]
    public void AnOrdinaryShapeStillFillsItsRectangle()
    {
        PageFrame frame = Build(EscherShapeTypes.Rectangle, EscherShapeAttributes.None).ShouldNotBeNull();

        frame.IsLine.ShouldBeFalse();
        frame.Fill.ShouldNotBeNull("fFilled defaults to on, and a rectangle has an area to fill");
    }

    /// <summary>
    /// One mirror flag turns the diagonal round and two turn it back.
    /// </summary>
    /// <remarks>
    /// A cross is two of these shapes over one rectangle, distinguished by nothing but the flags — so
    /// reading either flag on its own draws the same diagonal twice and leaves the page with half a cross.
    /// </remarks>
    [Theory]
    [InlineData(EscherShapeAttributes.None, false)]
    [InlineData(EscherShapeAttributes.FlipVertical, true)]
    [InlineData(EscherShapeAttributes.FlipHorizontal, true)]
    [InlineData(EscherShapeAttributes.FlipHorizontal | EscherShapeAttributes.FlipVertical, false)]
    public void MirroringChoosesWhichDiagonal(EscherShapeAttributes flags, bool mirrored)
        => Build(EscherShapeTypes.StraightConnector, flags)
            .ShouldNotBeNull().IsLineMirrored.ShouldBe(mirrored);

    private static PageFrame? Build(ushort type, EscherShapeAttributes flags)
        => Ww8Frames.Build(
            new Ww8ShapeAnchor(
                Position: 0,
                ShapeId: 1,
                Left: 7112,
                Top: 587,
                Right: 14612,
                Bottom: 3876,
                IsHeaderAnchor: false,
                HorizontalOrigin: Ww8ShapeOrigin.Text,
                VerticalOrigin: Ww8ShapeOrigin.Text,
                Wrap: 3,
                WrapSide: 0,
                IsPageRelative: false,
                IsBelowText: false),
            new EscherShape { ShapeId = 1, ShapeType = type, Flags = flags },
            offset: 0,
            blocks: []);
}
