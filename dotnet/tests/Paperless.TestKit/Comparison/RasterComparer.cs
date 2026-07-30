namespace Paperless.TestKit.Comparison;

/// <summary>
/// Compares a Paperless rendering against a LibreOffice reference rendering.
/// </summary>
/// <remarks>
/// <para>
/// Exact pixel equality is the wrong target. Two correct rasterisers disagree on
/// antialiasing and on sub-pixel rounding, so a strict comparison fails on output that is
/// visually identical. Equally, a permissive average-difference score passes output where
/// a whole paragraph has shifted, because most of the page is still white.
/// </para>
/// <para>
/// So the harness reports several independent measures and lets each test assert on the
/// one that matches what it is actually testing. In practice
/// <see cref="RasterComparison.ShiftedRegionCount"/> is the most diagnostic: it catches
/// the reflow-cascade failure, where one wrong line break moves everything after it and
/// every subsequent page diverges.
/// </para>
/// </remarks>
public static class RasterComparer
{
    /// <summary>
    /// Compares two PNG images of the same page.
    /// </summary>
    /// <param name="actualPng">The Paperless rendering.</param>
    /// <param name="expectedPng">The LibreOffice reference rendering.</param>
    public static RasterComparison Compare(ReadOnlySpan<byte> actualPng, ReadOnlySpan<byte> expectedPng)
        => throw new NotImplementedException();

    /// <summary>
    /// Writes a side-by-side image with differences highlighted, for a human to look at.
    /// </summary>
    /// <remarks>
    /// The most valuable output of the whole harness. A number tells you a page is wrong;
    /// this tells you which part of it, which is what actually leads to a fix.
    /// </remarks>
    public static void WriteDiffImage(
        ReadOnlySpan<byte> actualPng,
        ReadOnlySpan<byte> expectedPng,
        string destinationPath) => throw new NotImplementedException();
}

/// <summary>The result of comparing two renderings of a page.</summary>
/// <param name="DimensionsMatch">
/// Whether the two images are the same size. When false, every other measure is
/// meaningless — a page-size mismatch means the page geometry itself was read wrong.
/// </param>
/// <param name="DifferingPixelFraction">
/// The fraction of pixels differing by more than a small tolerance. Sensitive to
/// antialiasing, so useful as a trend rather than a pass/fail gate.
/// </param>
/// <param name="MeanAbsoluteError">Mean per-channel absolute difference, 0 to 1.</param>
/// <param name="MaxRegionalError">
/// The worst mean error found in any single tile of the page. Catches a badly wrong
/// small area that a whole-page average would dilute away.
/// </param>
/// <param name="ShiftedRegionCount">
/// How many regions match the reference well but at a different offset. Non-zero means
/// content moved rather than changed, which points at layout — a wrong line break, a
/// wrong margin, wrong font metrics — rather than at drawing.
/// </param>
/// <param name="InkCoverageDelta">
/// The difference in non-white pixel fraction between the two images. Strongly negative
/// means Paperless drew less than it should have, which usually means something was
/// skipped outright — an unsupported shape type, or a vector image that failed to decode.
/// </param>
public readonly record struct RasterComparison(
    bool DimensionsMatch,
    double DifferingPixelFraction,
    double MeanAbsoluteError,
    double MaxRegionalError,
    int ShiftedRegionCount,
    double InkCoverageDelta);
