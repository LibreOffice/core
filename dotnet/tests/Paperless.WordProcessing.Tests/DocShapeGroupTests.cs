using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A DOC shape group: one <c>FSPA</c>, many shapes, and the coordinate space between them.
/// </summary>
/// <remarks>
/// <para>
/// Word anchors a group with a single <c>FSPA</c> naming the group's identifier. The shapes
/// underneath it have no entry in the anchor table at all and state their rectangles in the group's
/// own <c>msofbtSpgr</c> coordinate space with an <c>msofbtChildAnchor</c>, so a reader that stops at
/// the anchor's shape draws the group's outline and nothing inside it. Measured on
/// <c>omrIMInterpretiveGuideLine.doc</c>, whose masthead is a group of five text boxes, a seal and a
/// rectangle: we drew the rectangle, and the user reported it as a missing header.
/// </para>
/// <para>
/// The transform is asserted rather than only the member count, because a group whose members all
/// land at its origin extracts exactly as well as one laid out correctly and looks like one black
/// box. The child space here is scaled unequally on the two axes on purpose — Word's
/// <c>msofbtSpgr</c> is arbitrary and a square-to-square test cannot tell a real division from an
/// accidental identity.
/// </para>
/// </remarks>
public sealed class DocShapeGroupTests
{
    [Fact]
    public void AChildAnchorIsDividedOutOfTheGroupsCoordinateSpace()
    {
        EscherShape group = Group(new EscherRectangle(0, 0, 2000, 1000));

        // 4000 twips across 2000 units of child space, 500 down 1000: two to one, and one to two.
        Ww8GroupTransform transform = Ww8GroupTransform.Of(group, width: 4000, height: 500);

        transform.Map(new EscherRectangle(500, 200, 1500, 600))
            .ShouldBe((X: 1000, Y: 100, Width: 2000, Height: 200));
    }

    /// <summary>
    /// A child space whose origin is not zero is shifted as well as scaled.
    /// </summary>
    /// <remarks>
    /// Word writes a non-zero <c>msofbtSpgr</c> origin routinely, and a reader that only divides puts
    /// every member of such a group off the page by the origin's own magnitude.
    /// </remarks>
    [Fact]
    public void ANonZeroChildOriginIsSubtracted()
    {
        EscherShape group = Group(new EscherRectangle(1000, 4000, 2000, 5000));

        Ww8GroupTransform transform = Ww8GroupTransform.Of(group, width: 1000, height: 1000);

        transform.Map(new EscherRectangle(1000, 4000, 1500, 4500))
            .ShouldBe((X: 0, Y: 0, Width: 500, Height: 500));
    }

    /// <summary>
    /// A group stating no coordinate space, or a degenerate one, is read one to one.
    /// </summary>
    /// <remarks>
    /// A zero-width <c>msofbtSpgr</c> is something a file may state and no scale can divide by. The
    /// answer that keeps the member's own numbers is the one that draws something; inventing a factor
    /// or dropping the member both lose the shape.
    /// </remarks>
    [Fact]
    public void ADegenerateChildSpaceIsReadOneToOne()
    {
        EscherShape group = Group(new EscherRectangle(0, 0, 0, 0));

        Ww8GroupTransform.Of(group, width: 4000, height: 500)
            .Map(new EscherRectangle(10, 20, 30, 50))
            .ShouldBe((X: 10, Y: 20, Width: 20, Height: 30));
    }

    /// <summary>
    /// A nested group composes: the inner space is divided into the rectangle the outer one gave it.
    /// </summary>
    [Fact]
    public void ANestedGroupComposesBothCoordinateSpaces()
    {
        EscherShape outer = Group(new EscherRectangle(0, 0, 2000, 1000));
        EscherShape inner = Group(new EscherRectangle(0, 0, 100, 100));

        Ww8GroupTransform first = Ww8GroupTransform.Of(outer, width: 4000, height: 500);
        (int x, int y, int width, int height) = first.Map(new EscherRectangle(500, 200, 1500, 600));

        Ww8GroupTransform second = Ww8GroupTransform.Of(inner, width, height, x, y);

        // The inner group occupies (1000, 100) 2000x200, and its own space is 100 square.
        second.Map(new EscherRectangle(0, 0, 50, 50))
            .ShouldBe((X: 1000, Y: 100, Width: 1000, Height: 100));
    }

    /// <summary>
    /// A member takes the envelope's position and its own rectangle, and never its own wrap.
    /// </summary>
    /// <remarks>
    /// The wrap is the half that is easy to get wrong and expensive: a member punching its own hole in
    /// the text would narrow the column once per shape, so a masthead of a dozen boxes would push the
    /// body text a dozen indents across. The envelope keeps the anchor's wrap and the members take
    /// <see cref="TextWrap.Through"/>.
    /// </remarks>
    [Fact]
    public void AMemberKeepsTheEnvelopesPositionAndNotItsWrap()
    {
        PageFrame envelope = new()
        {
            Size = new DocSize(Length.FromTwips(4000), Length.FromTwips(500)),
            Wrap = TextWrap.Both,
            HorizontalOrigin = FrameHorizontalOrigin.Page,
            HorizontalOffset = Length.FromTwips(720),
            Anchor = FrameAnchor.Character,
        };

        PageFrame member = Ww8Frames.Member(
            envelope,
            new EscherShape { ShapeId = 7 },
            (X: 1000, Y: 100, Width: 2000, Height: 200),
            [],
            default)!;

        member.ShouldNotBeNull();
        member.Size.ShouldBe(new DocSize(Length.FromTwips(2000), Length.FromTwips(200)));
        member.GroupSize.ShouldBe(envelope.Size);
        member.GroupOffset.ShouldBe(new DocPoint(Length.FromTwips(1000), Length.FromTwips(100)));
        member.Wrap.ShouldBe(TextWrap.Through);

        member.HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Page);
        member.HorizontalOffset.ShouldBe(Length.FromTwips(720));
    }

    /// <summary>
    /// A group's own envelope paints nothing.
    /// </summary>
    /// <remarks>
    /// An <c>SdrObjGroup</c> has no fill and no line; ours had both, because <c>fFilled</c> and
    /// <c>fLine</c> default to true when a shape states neither and a group states neither. The
    /// opaque white was painting over the body text under the masthead, which is the second reason
    /// that document's header block was blank.
    /// </remarks>
    [Fact]
    public void AGroupsEnvelopePaintsNothing()
    {
        PageFrame envelope = Ww8Frames.Build(
            Anchor(), Group(new EscherRectangle(0, 0, 2000, 1000)), offset: 0, [])!;

        envelope.ShouldNotBeNull();
        envelope.Fill.ShouldBeNull();
        envelope.BorderColour.ShouldBeNull();
    }

    /// <summary>A leaf shape's envelope still paints, which is what the group case must not break.</summary>
    [Fact]
    public void ALeafShapesFrameStillPaints()
    {
        PageFrame frame = Ww8Frames.Build(Anchor(), new EscherShape { ShapeId = 7 }, offset: 0, [])!;

        frame.ShouldNotBeNull();
        frame.Fill.ShouldNotBeNull();
        frame.BorderColour.ShouldNotBeNull();
    }

    private static EscherShape Group(EscherRectangle bounds) => new()
    {
        ShapeId = 1,
        Flags = EscherShapeAttributes.Group,
        GroupBounds = bounds,
    };

    private static Ww8ShapeAnchor Anchor() => new(
        Position: 0,
        ShapeId: 1,
        Left: 0,
        Top: 0,
        Right: 4000,
        Bottom: 500,
        IsHeaderAnchor: false,
        HorizontalOrigin: Ww8ShapeOrigin.Page,
        VerticalOrigin: Ww8ShapeOrigin.Page,
        Wrap: 3,
        WrapSide: 0,
        IsPageRelative: false,
        IsBelowText: false);
}
