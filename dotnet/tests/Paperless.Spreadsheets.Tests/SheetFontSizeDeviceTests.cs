using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The em size a printed sheet draws at is quantised through the reference device, and the print
/// scale sits inside that quantisation rather than after it.
/// </summary>
/// <remarks>
/// <para>
/// A font height crosses three roundings between the file and the page. The first is the one
/// <see cref="SheetDeviceUnits.SnapFontSize(Length)"/> already made: twips to a whole hundredth of
/// a millimetre. The other two belong to the device — VCL instantiates a face at a whole number of
/// device pixels and the PDF writer maps that pixel height back to a whole logical unit — and both
/// happen through a map mode that already carries the print scale. Scaling a snapped size instead
/// gives the right answer at 100% and the wrong one at almost every zoom.
/// </para>
/// <para>
/// Every expectation below is LibreOffice 24.2.7.2's own <c>Tf</c> operand, read out of its PDF
/// export of the probe sheets in <c>dotnet/probes/sheets-r23</c>. The whole set is 178 observations
/// over sixteen sizes and seven scales; these are the ones that separate the rules.
/// </para>
/// <para>
/// Two of the four cases here do not fail when the fix is reverted, and they are kept deliberately
/// as drift guards rather than as tests of it: <see cref="Unscaled"/> and
/// <see cref="EvenPointSizesAreUnmovedByTheRoundTrip"/> both pass under the old
/// <c>Snap(size) * scale</c> rule. They are what says the change did not move the common case.
/// </para>
/// </remarks>
public sealed class SheetFontSizeDeviceTests
{
    /// <summary>The reference's emitted size for a stated point size at a print scale.</summary>
    private static double Drawn(double points, double scale)
        => SheetDeviceUnits.SnapFontSize(Length.FromPoints(points), scale).Points;

    /// <summary>
    /// At 100% every whole point size from 6 to 48 comes back exactly where the single
    /// twips-to-hundredths rounding puts it, which is why the class hid until a zoomed sheet.
    /// </summary>
    [Theory]
    [InlineData(6, 6.009)]
    [InlineData(7, 7.002)]
    [InlineData(9, 9.014)]
    [InlineData(10, 10.006)]
    [InlineData(11, 10.998)]
    [InlineData(18, 18.0)]
    [InlineData(48, 47.991)]
    public void Unscaled(double points, double expected)
        => Drawn(points, 1.0).ShouldBe(expected, 0.0006);

    /// <summary>
    /// Under a zoom an odd whole point size does not land on the scaled hundredth. Nine point at
    /// 75% is the case that named this class: 6.803 pt is 240 hundredths, and neither 238 nor
    /// 238.5 — the two answers available from rounding 9 pt to the hundredth before or after the
    /// scale — is what the reference writes.
    /// </summary>
    [Theory]
    [InlineData(9, 0.75, 6.803)]
    [InlineData(7, 0.75, 5.294)]
    [InlineData(11, 0.75, 8.206)]
    [InlineData(13, 0.75, 9.801)]
    [InlineData(9, 0.25, 2.303)]
    [InlineData(11, 0.25, 2.7)]
    [InlineData(9, 0.33, 3.003)]
    public void OddPointSizesUnderAZoom(double points, double scale, double expected)
        => Drawn(points, scale).ShouldBe(expected, 0.0006);

    /// <summary>
    /// The round trip is transparent for even whole point sizes at 75%, so a sheet set entirely in
    /// 8 or 10 point looks identical either way. A drift guard: this passes under the old rule too.
    /// </summary>
    [Theory]
    [InlineData(6, 4.507)]
    [InlineData(8, 5.995)]
    [InlineData(10, 7.505)]
    [InlineData(12, 8.993)]
    [InlineData(14, 10.502)]
    [InlineData(18, 13.5)]
    public void EvenPointSizesAreUnmovedByTheRoundTrip(double points, double expected)
        => Drawn(points, 0.75).ShouldBe(expected, 0.0006);

    /// <summary>
    /// A fractional size is quantised at 100% as well — 8.25 pt is 291 hundredths and comes back
    /// 289 — so the device round trip is not something a zoom switches on.
    /// </summary>
    [Theory]
    [InlineData(8.05, 8.107)]
    [InlineData(8.25, 8.192)]
    [InlineData(8.45, 8.391)]
    [InlineData(9.95, 9.893)]
    public void FractionalSizesAreQuantisedWithoutAZoom(double points, double expected)
        => Drawn(points, 1.0).ShouldBe(expected, 0.0006);
}
