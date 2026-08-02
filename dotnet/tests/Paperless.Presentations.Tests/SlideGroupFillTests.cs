using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// <c>a:grpFill</c>: a shape whose fill is whatever the group it sits in is filled with.
/// </summary>
/// <remarks>
/// <para>
/// The fifth of DrawingML's six fill kinds, and the only one that is not a fill at all — it is a
/// reference upwards. LibreOffice threads the parent group's fill properties down the shape tree
/// and <c>Shape::getActualFillProperties</c> uses them when a shape's own fill type is
/// <c>XML_grpFill</c>. A group has no geometry, so its <c>p:grpSpPr</c> fill is never painted for
/// its own sake; it exists to be inherited.
/// </para>
/// <para>
/// The fixture is <c>shape-group-fill.pptx</c>, derived from the hand-written
/// <c>shape-geometry.pptx</c> so every offset is a round number of EMU. Its expectations are
/// LibreOffice 24.2.7.2's own rendering of it: a red square, a blue square, and nothing where the
/// two unfilled shapes are.
/// </para>
/// <para>
/// Worth a fixture rather than a corpus note because the element is not rare: eight of the 112
/// PPTX decks in the slides corpus use it on a slide, and on
/// <c>slides/batch-002/pptx/iaeg_work_group_leader_updates.pptx</c> it decides six of the eight
/// boxes of an organisation chart.
/// </para>
/// </remarks>
public class SlideGroupFillTests
{
    private static readonly Colour GroupRed = Colour.FromRgb(0xC00000);
    private static readonly Colour OwnBlue = Colour.FromRgb(0x0000C0);

    [Fact]
    public void AShapeAskingForItsGroupsFillIsPaintedWithIt()
    {
        Fill("InheritsGroupFill").ShouldBeOfType<SolidPaint>().Colour.ShouldBe(GroupRed);
    }

    [Fact]
    public void AShapeStatingItsOwnFillKeepsItInsideAFilledGroup()
    {
        Fill("StatesOwnFill").ShouldBeOfType<SolidPaint>().Colour.ShouldBe(OwnBlue);
    }

    [Fact]
    public void AShapeStatingNoFillStaysEmptyInsideAFilledGroup()
    {
        Fill("StatesNoFill").ShouldBeNull();
    }

    /// <summary>
    /// A group fill with no group above it is nothing, rather than falling through to the theme.
    /// </summary>
    /// <remarks>
    /// The reference draws no square here, which is the case that says <c>a:grpFill</c> ends the
    /// search: treating it as "state nothing" would let the shape's <c>p:style</c> or the theme's
    /// format matrix answer instead, and on a deck whose style matrix fills with accent 1 that
    /// paints a shape the reference leaves blank.
    /// </remarks>
    [Fact]
    public void AGroupFillOutsideAnyGroupIsNoFillAtAll()
    {
        Fill("GroupFillWithNoGroup").ShouldBeNull();
    }

    private static Paint? Fill(string name)
    {
        using IDocument read = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("shape-group-fill.pptx")));

        read.ShouldBeAssignableTo<IPaginatedDocument>();
        IReadOnlyList<LaidOutSlide> slides = ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;

        slides.Count.ShouldBe(1);
        PlacedShape shape = slides[0].Shapes.FirstOrDefault(s => s.Name == name)
            ?? throw new InvalidOperationException(
                $"the fixture has no shape named '{name}'; it has "
                + string.Join(", ", slides[0].Shapes.Select(s => s.Name ?? "<unnamed>")));

        return shape.Fill;
    }
}
