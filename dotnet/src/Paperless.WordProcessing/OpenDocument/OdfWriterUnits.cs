using Paperless.Core.Units;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Converts an ODF measure into Writer's own unit.
/// </summary>
/// <remarks>
/// <para>
/// ODF states its lengths with units and to arbitrary precision — <c>2.5cm</c>, <c>0.79in</c> — and
/// Writer stores them as whole twips. The conversion is not lossless and the loss is not academic:
/// 2.5 cm is 1417.32 twips, LibreOffice keeps 1417, and a margin a third of a twip wider narrows the text
/// area by the same amount on both sides. That is invisible on one line and decides a break on the line
/// where a word just fits.
/// </para>
/// <para>
/// This is what LibreOffice's own import does, not an approximation of it: <c>SvXMLUnitConverter</c> is
/// constructed with the target application's <c>MapUnit</c>, which for Writer is <c>MapTwip</c>, so
/// <c>convertMeasureToCore</c> rounds every measure to a twip on the way in. The same file read into Calc
/// or Draw keeps 1/100 mm instead — which is why this lives beside the Writer readers rather than in
/// <c>Paperless.OpenDocument</c>, where a spreadsheet would wrongly inherit it.
/// </para>
/// </remarks>
internal static class OdfWriterUnits
{
    /// <summary>Rounds a measure to Writer's whole-twip grid.</summary>
    /// <remarks>
    /// Half away from zero, which is what <see cref="Length.Twips"/> does and what the office formats
    /// themselves do; banker's rounding would disagree on exactly the half-twip values that round numbers
    /// of centimetres produce.
    /// </remarks>
    internal static Length ToCore(Length value) => Length.FromTwips(value.Twips);

    /// <summary>Rounds a measure to Writer's grid, passing a missing value through.</summary>
    internal static Length? ToCore(Length? value) => value is { } length ? ToCore(length) : null;
}
