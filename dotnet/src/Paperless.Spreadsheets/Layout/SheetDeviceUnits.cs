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
}
