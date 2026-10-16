using XRay.Office.Spreadsheets.Layout;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A page placement's scale, and what an unset one means.
/// </summary>
/// <remarks>
/// <para>
/// This arithmetic stood in four places as <c>Math.Max(1, ZoomPercentage) / 100.0</c>, and that
/// guard clamps to one <em>per cent</em>. A placement that never had a zoom set — <c>default</c>,
/// which the note-page drawing constructs its decoration with — therefore came out at 0.01 and drew
/// its header and footer at a hundredth of the size the file states.
/// </para>
/// <para>
/// It did not present as a small band. It presented as a missing one, and as a drawing failure
/// rather than an arithmetic one: <c>Hazard Analysis Template.xls</c> states
/// <c>&amp;C&amp;"Arial,Bold"&amp;12</c> and we drew its two bands at <b>0.120 pt</b> against the
/// reference's <b>7.887</b> — the same face, the same colour, correctly centred, and invisible on
/// the paper. Every other span on that page is 10 pt, which is what localised it to the bands.
/// With the zero handled it draws at 7.920 pt, the remaining 0.4% being the advance divergence
/// recorded in <c>dotnet/CLAUDE.md</c> and not this.
/// </para>
/// </remarks>
public sealed class SheetPlacementScaleTests
{
    /// <summary>A stated percentage is that percentage.</summary>
    [Theory]
    [InlineData(100, 1.0)]
    [InlineData(65, 0.65)]
    [InlineData(66, 0.66)]
    [InlineData(10, 0.10)]
    [InlineData(400, 4.0)]
    public void AStatedZoomIsItsOwnFraction(int percentage, double expected) =>
        Placement(percentage).Scale.ShouldBe(expected, 1e-9);

    /// <summary>
    /// An unset zoom is full size, which is the whole point of this type carrying the conversion.
    /// </summary>
    /// <remarks>
    /// Zero is not "one per cent" and it is not "the smallest scale a fit-to-pages search will
    /// settle on" either. It is a placement that was never given a zoom, and the only reading of
    /// that which cannot silently shrink a page is 100%.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(-1)]
    [InlineData(int.MinValue)]
    public void AnUnsetOrNegativeZoomIsFullSize(int percentage) =>
        Placement(percentage).Scale.ShouldBe(1.0);

    /// <summary>A default placement — what a note page's decoration is built with — is full size.</summary>
    [Fact]
    public void ADefaultPlacementIsFullSize() =>
        default(SheetPagePlacement).Scale.ShouldBe(1.0);

    private static SheetPagePlacement Placement(int percentage) =>
        new(new SheetRange(0, 0, 1, 1), null, null, percentage, 0, 0, 0);
}
