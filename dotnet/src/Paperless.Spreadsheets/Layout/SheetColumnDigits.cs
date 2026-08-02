using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// The workbook's default font: the face a column width's <em>digits</em> are digits of.
/// </summary>
/// <remarks>
/// <para>
/// Only the name and the size, because that is all the readers can supply without measuring
/// anything. The weight and the posture travel with them because LibreOffice sets all four on
/// the device it measures with (<c>XclRoot::SetCharWidth</c>,
/// <c>sc/source/filter/excel/xlroot.cxx:210</c>) and a bold face's digits are wider.
/// </para>
/// <para>
/// Which font this is differs a little between the formats and matters less than it looks.
/// SpreadsheetML's is the font of the default cell style's <c>styleXf</c>, falling back to
/// <c>fonts[0]</c> (<c>StylesBuffer::getDefaultFont</c>,
/// <c>sc/source/filter/oox/stylesbuffer.cxx:3167</c>); BIFF's is the "app font", which is
/// <c>FONT</c> record zero (<c>XclImpFontBuffer::UpdateAppFont</c>,
/// <c>sc/source/filter/excel/xistyle.cxx:632</c>). Every producer writes the two the same, so
/// both readers use the first font and say so here rather than modelling the difference.
/// </para>
/// </remarks>
/// <param name="Family">The family name, or null for the application's own default.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The CSS-style weight; 400 unless the font record says otherwise.</param>
/// <param name="IsItalic">Whether the face is italic.</param>
public sealed record SheetDefaultFont(
    string? Family, Length Size, int Weight = 400, bool IsItalic = false)
{
    /// <summary>What a workbook naming no font is measured in: ten-point Liberation Sans.</summary>
    /// <remarks>
    /// <c>DefaultFontType::LATIN_SPREADSHEET</c> on Linux, and the face LibreOffice puts in a new
    /// spreadsheet — so it is what its rendering of a workbook stating nothing measures.
    /// </remarks>
    public static SheetDefaultFont Calc { get; } = new(null, Length.FromPoints(10));
}

/// <summary>
/// A column width as a spreadsheet format states it: a count of digits and a fixed part.
/// </summary>
/// <remarks>
/// <para>
/// Neither Excel format states a column width as a length. SpreadsheetML states a number of
/// digits of the workbook's default font and BIFF a number of 256ths of one, so a width only
/// becomes a measurement once that font has been measured — which is the whole reason this type
/// exists rather than a <see cref="Length"/>.
/// </para>
/// <para>
/// The fixed part is not decoration. Two of the four conversions carry one and neither scales
/// with the font: <c>baseColWidth</c> adds five screen pixels of padding
/// (<c>WorksheetGlobals::setBaseColumnWidth</c>, <c>sc/source/filter/oox/worksheethelper.cxx:745</c>,
/// <c>#i3006#</c>), which is 75 twips whatever the digit is worth, and BIFF's conversion
/// subtracts half a twip before truncating as a deliberate bias
/// (<c>XclTools::GetScColumnWidth</c>, <c>sc/source/filter/excel/xltools.cxx:304</c>). A width
/// that is simply a fixed length — Calc's own 64-point standard column, which a BIFF file with no
/// <c>DEFCOLWIDTH</c> falls back to — is stated as no digits and a fixed part.
/// </para>
/// <para>
/// Truncation rather than rounding, in one place for all four conversions, because that is what
/// BIFF's bias needs: <c>(long)(w / 256.0 * digit - 0.5)</c> is what gives an eight-character
/// column the width Excel shows rather than one twip more. The SpreadsheetML conversions carry a
/// bias of half a twip instead, which makes the same truncation round to nearest.
/// </para>
/// </remarks>
/// <param name="Digits">How many digits of the default font the column is wide.</param>
/// <param name="BiasTwips">The part of the width that does not scale with the font.</param>
public readonly record struct SheetDigitWidth(double Digits, double BiasTwips = 0)
{
    /// <summary>A width that is a fixed length rather than a count of digits.</summary>
    /// <param name="length">The length.</param>
    public static SheetDigitWidth Fixed(Length length) => new(0, length.Twips);

    /// <summary>The width, once one digit is known to be worth so many twips.</summary>
    /// <param name="digitWidthTwips">The advance of the default font's widest digit.</param>
    public Length At(double digitWidthTwips)
    {
        double twips = (Digits * digitWidthTwips) + BiasTwips;
        return twips <= 0 ? Length.Zero : Length.FromTwips((long)twips);
    }
}

/// <summary>One run of consecutive columns whose width is stated in digits.</summary>
/// <param name="First">The first column index the run covers.</param>
/// <param name="Last">The last column index it covers, inclusive.</param>
/// <param name="Width">The width each of them is stated as.</param>
/// <param name="IsHidden">True when every column in the run is hidden.</param>
public readonly record struct SheetDigitRun(
    int First, int Last, SheetDigitWidth Width, bool IsHidden);

/// <summary>
/// A sheet's column widths, still stated in digits of the workbook's default font.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This exists because measuring a face belongs to layout and reading a workbook does
/// not.</strong> The three Excel readers build a <see cref="SheetGrid"/> while they read, which is
/// the extraction path, and <c>dotnet/CLAUDE.md</c> is explicit that extraction must not pay for
/// fonts. LibreOffice does resolve early — <c>UnitConverter::finalizeImport</c>
/// (<c>sc/source/filter/oox/unitconverter.cxx:113</c>) asks the document's reference device for
/// the widest of <c>'0'</c>–<c>'9'</c> and every column width is a length from then on — but it
/// has a document with a device attached and Paperless has a reader with a stream. So the readers
/// carry the digits and the font's name, both free, and <see cref="SheetLayout.Grid"/> resolves
/// them the first time anything asks for the geometry.
/// </para>
/// <para>
/// The grid is still materialised eagerly at <see cref="FallbackDigitWidthTwips"/>, so
/// <see cref="SheetGrid.Columns"/> is never empty and a caller that never resolves gets exactly
/// what it got before this type existed.
/// </para>
/// </remarks>
/// <param name="Font">The font the digits are digits of.</param>
/// <param name="Default">The width of a column no run covers.</param>
/// <param name="Runs">The runs, in the order the file states them; overlaps are resolved later.</param>
public sealed record SheetColumnDigits(
    SheetDefaultFont Font, SheetDigitWidth Default, IReadOnlyList<SheetDigitRun> Runs)
{
    /// <summary>
    /// What one digit is worth when no face can be measured: 111 twips.
    /// </summary>
    /// <remarks>
    /// Ten-point Liberation Sans, whose digits are 1139 units of a 2048-unit em — 111.23 twips,
    /// which LibreOffice's own device reports as 111. It is the face a new Calc spreadsheet uses,
    /// so a workbook that names no font is measured in it either way; it was the hardcoded
    /// constant in all three readers before they carried the font, and it is the right answer for
    /// a third of the corpus by accident and for the rest by measurement.
    /// </remarks>
    public const double FallbackDigitWidthTwips = 111;

    /// <summary>The column axis, once one digit is known to be worth so many twips.</summary>
    /// <param name="digitWidthTwips">The advance of the default font's widest digit.</param>
    public SheetAxis Resolve(double digitWidthTwips) => new(
        Default.At(digitWidthTwips),
        Runs.Select(run => new SheetSizeRun(
            run.First, run.Last, run.Width.At(digitWidthTwips), run.IsHidden)));
}
