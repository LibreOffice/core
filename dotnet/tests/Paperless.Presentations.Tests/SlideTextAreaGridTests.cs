using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A shape's text rectangle is an integer number of hundredths of a millimetre.
/// </summary>
/// <remarks>
/// <para>
/// The companion of <c>SlideAutofitTests</c>'s em-size grid, and the other half of the same
/// defect. oox builds a shape's matrix in EMUs and scales it into hundredths of a millimetre at
/// the end (<c>oox/source/drawingml/shape.cxx</c>:1226-1230, 24.2.7.2); <c>SvxShape</c> hands the
/// result to <c>SdrObject::SetSnapRect</c>, whose <c>tools::Rectangle</c> holds four
/// <c>sal_Int32</c>; and a <c>SdrTextObj</c>'s text rectangle is that rectangle less four
/// <c>SdrMetricItem</c> distances of the same unit. So the reference never lays text out against
/// a fraction of a unit, and we did.
/// </para>
/// <para>
/// <strong>Both edges round, not the extent.</strong> That is what these two cases are for: the
/// same 6000000 EMU extent at two different origins, one whose left edge rounds down and one
/// whose rounds up. An implementation that rounded the width and height instead would put both
/// pens half a unit from where the reference puts them and would pass any test that only checked
/// one of them.
/// </para>
/// <para>
/// The reference's own numbers for this deck are in
/// <c>SlideTextAreaGridComparisonTests</c>: 72.000 and 144.028 pt, against the
/// 72.0141 and 144.0143 an unquantised rectangle gives.
/// </para>
/// </remarks>
public sealed class SlideTextAreaGridTests
{
    /// <summary>914579 EMU is 2540.497 units, so its left edge rounds down to 2540 — 72 pt.</summary>
    [Theory]
    [InlineData(914579L, 2540L)]
    [InlineData(1828981L, 5081L)]
    public void ATextRectangleIsAWholeNumberOfHundredthsOfAMillimetre(long emu, long expectedMm100)
    {
        DocRect area = new(
            Length.FromEmu(emu), Length.Zero,
            Length.FromEmu(6000000), Length.FromEmu(900000));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(Body(), area, new SlideFonts());

        placed.ShouldNotBeEmpty();
        placed[0].Run.Origin.X.ShouldBe(Length.FromMm100(expectedMm100));
    }

    /// <summary>
    /// Two boxes of one extent whose bottom edges round to the same unit anchor alike.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the half that decides the shrink-to-fit search, stated as something a placement
    /// can show. <c>round(bottom) − round(top)</c> is not <c>round(bottom − top)</c>, so the
    /// height the search compares against depends on where on the slide the box sits and the
    /// extent alone does not determine it.
    /// </para>
    /// <para>
    /// Both boxes are 971928 EMU tall. The first starts at 914400 — 2540 units exactly — and ends
    /// at 5239.8, which rounds to 5240. The second starts 216 EMU lower, at 2540.6, and ends at
    /// 5240.4, which rounds to 5240 as well. So the reference gives them heights of 2700 and 2699
    /// and one shared bottom edge, and a bottom-anchored body hangs from the same place in both.
    /// Against unquantised edges the two bottoms are 216 EMU apart and so are the baselines,
    /// which is what this assertion fails by when the quantisation is taken out.
    /// </para>
    /// </remarks>
    [Fact]
    public void TwoBoxesOfOneExtentShareABottomEdgeWhenTheirEdgesRoundAlike()
    {
        Length onGrid = Baseline(Length.FromEmu(914400), Length.FromEmu(971928));
        Length below = Baseline(Length.FromEmu(914616), Length.FromEmu(971928));

        below.Emu.ShouldBe(onGrid.Emu);
        (onGrid.Emu % Length.EmuPerMm100).ShouldBe(0);
    }

    /// <summary>Where the only glyph run's baseline lands, for a bottom-anchored body.</summary>
    private static Length Baseline(Length top, Length height)
    {
        DocRect area = new(Length.Zero, top, Length.FromEmu(6000000), height);

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            Body() with { Anchor = TextAnchor.Bottom }, area, new SlideFonts());

        placed.ShouldNotBeEmpty();
        return placed[0].Run.Origin.Y;
    }

    /// <summary>One 18 pt word, no insets, so only the rectangle decides where the pen goes.</summary>
    private static SlideTextBody Body() => new()
    {
        FontIndependentLineSpacing = true,
        Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
        Paragraphs =
        [
            new SlideParagraph(
                "Down",
                [
                    new SlideTextRun(
                        0, 4, "Liberation Sans", Length.FromPoints(18), 400, false, Colour.Black),
                ]),
        ],
    };
}
