using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// The unit Calc's output actually lands on, and the two roundings that reach it.
/// </summary>
/// <remarks>
/// <para>
/// Calc stores a sheet's geometry in twips and draws it through a device whose logical unit is a
/// hundredth of a millimetre, so every length crosses one conversion on its way to the page. The
/// conversion is lossy — 256 twips is 451.6 hundredths — and which way it loses is measurable:
/// <strong>positions and sizes truncate, font heights round</strong>. A default 12.8 pt row comes
/// out 12.7843 pt tall in LibreOffice's own PDF (451, not 452) and ten-point text comes out at
/// 10.0063 pt (353, not 352).
/// </para>
/// <para>
/// Worth reproducing rather than tolerating, because only one of the two accumulates. The font
/// size is a fixed six-thousandths of a point and would never matter; the row height is a
/// sixty-fourth of a point <em>per row</em>, so by the sixty-seventh row of a page the baseline is
/// 0.86 pt out — eight times the tenth of a point this project holds itself to, and it looks like
/// a wrong row height rather than like rounding.
/// </para>
/// <para>
/// This is the reference renderer's own artefact, not the document's, which is why it lives in one
/// small type that names it rather than being spread through the geometry. It is also used by
/// whatever else draws on a sheet — gridlines and borders have to land on the same edges the text
/// is aligned to, or a right-aligned figure sits a hair outside its own cell.
/// </para>
/// </remarks>
internal static class SheetDeviceUnits
{
    /// <summary>EMUs in one hundredth of a millimetre.</summary>
    private const long EmuPerMm100 = 360;

    /// <summary>EMUs in one twip.</summary>
    private const long EmuPerTwip = 635;

    /// <summary>
    /// A position or a size as the drawing device holds it: through whole twips, then truncated
    /// into whole hundredths of a millimetre.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both steps are needed and each is visible in a reference rendering. Calc's own storage is
    /// twips — a row height is a <c>sal_uInt16</c> of them — so a file stating a length in any
    /// other unit is quantised on the way in: an ODF row of <c>0.178in</c> is 452.1 hundredths
    /// exactly, 256 twips once Calc has it, and 451 hundredths by the time it is drawn. Snapping
    /// only to the device unit gives 452 and the sheet's rows drift a hundredth of a millimetre
    /// apiece — 1.5 pt by the bottom of an eighty-row page.
    /// </para>
    /// <para>
    /// A column of <c>0.889in</c> is the same story across: 2258 hundredths direct, 2257 through
    /// twips, which is the 0.028 pt every column of <c>sheet-print-ods.ods</c> was out by.
    /// </para>
    /// </remarks>
    /// <param name="value">The exact length.</param>
    public static Length Snap(Length value)
        => Length.FromEmu(value.Twips * EmuPerTwip / EmuPerMm100 * EmuPerMm100);

    /// <summary>A font's em size as the device holds it: rounded to nearest, not truncated.</summary>
    /// <param name="value">The exact size.</param>
    public static Length SnapFontSize(Length value)
    {
        long hundredths = (long)Math.Round(
            (double)(value.Twips * EmuPerTwip) / EmuPerMm100, MidpointRounding.AwayFromZero);
        return Length.FromEmu(hundredths * EmuPerMm100);
    }

    /// <summary>
    /// Dots per inch of the reference device a PDF export draws through. Measured, not assumed —
    /// see <see cref="SnapFontSize(Length, double)"/>.
    /// </summary>
    private const long ReferenceDpi = 720;

    /// <summary>Hundredths of a millimetre in one inch.</summary>
    private const long Mm100PerInch = 2540;

    /// <summary>
    /// A font's em size as it reaches a page printed at <paramref name="scale"/>: through the
    /// device unit, through whole device pixels, and back.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Scaling the snapped size is not what the reference renderer does, and the difference is
    /// visible on every sheet printed at a zoom. A font height crosses <em>two</em> more roundings
    /// on its way to the page: VCL selects a face at a whole number of device pixels, and the PDF
    /// writer then maps that pixel height back to a whole logical unit — both through a map mode
    /// that already carries the print scale, so the scale sits inside the quantisation rather than
    /// after it.
    /// </para>
    /// <para>
    /// Measured against LibreOffice 24.2.7.2 on eight probe sheets — sixteen sizes at each of seven
    /// print scales, plus two sweeps in 0.05 pt steps — this reproduces all 178 emitted sizes
    /// exactly, and nothing simpler does. Nine point at 75% is the case that names the class: the
    /// reference draws 6.803 pt, which is 240 hundredths, and 240 is not reachable by rounding
    /// 9 pt to the hundredth in either order (238 or 238.5). It is
    /// <c>round(round(238.5 × 720/2540) × 2540/540) × 0.75</c>, or 320 unscaled hundredths.
    /// </para>
    /// <para>
    /// At 100% the round trip is the identity for every whole-point size from 6 to 48, which is why
    /// the class only ever surfaced on zoomed sheets. It is not the identity in general — 8.25 pt
    /// is 291 hundredths and comes back 289 — and under a zoom it is the identity for even
    /// whole-point sizes and not for odd ones, 9 pt and 11 pt being the two a spreadsheet uses
    /// most.
    /// </para>
    /// </remarks>
    /// <param name="value">The exact stated size.</param>
    /// <param name="scale">The print scale, 1.0 for an unscaled sheet.</param>
    public static Length SnapFontSize(Length value, double scale)
    {
        long hundredths = SnapFontSize(value).Emu / EmuPerMm100;

        // A face is selected at whole device pixels. One pixel is the floor: a zero-height font
        // selects the device default rather than drawing nothing, so collapsing to it would be a
        // different defect rather than a smaller one.
        double pixels = Math.Round(
            hundredths * scale * ReferenceDpi / Mm100PerInch, MidpointRounding.AwayFromZero);
        if (pixels < 1) pixels = 1;

        // ...and mapped back through the same scaled map mode, which is why the scale divides here
        // and multiplies again below rather than cancelling out.
        long logical = (long)Math.Round(
            pixels * Mm100PerInch / (ReferenceDpi * scale), MidpointRounding.AwayFromZero);

        return Length.FromEmu(logical * EmuPerMm100) * scale;
    }
}
