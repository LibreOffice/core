using XRay.Office.Core.Graphics;

namespace XRay.Office.Spreadsheets.MsBinary;

/// <summary>
/// The one colour a BIFF fill pattern paints.
/// </summary>
/// <remarks>
/// <para>
/// The BIFF twin of <c>XlsxPatternFill</c>, and the same rule seen through a different notation:
/// Calc has no hatched cell background, so it mixes the pattern colour into the background at a
/// weight the pattern <em>number</em> decides and stores the single result.
/// <c>XclTools::GetPatternColor</c> (<c>sc/source/filter/excel/xltools.cxx</c>:347-358) indexes a
/// nineteen-entry table by the pattern and hands it to <c>ScfTools::GetMixedColor</c>
/// (<c>sc/source/filter/ftools/ftools.cxx</c>:118-130), which is
/// <c>(background − pattern) × ratio / 0x80 + pattern</c> per component in integer arithmetic.
/// </para>
/// <para>
/// <strong>The ratio runs the other way from SpreadsheetML's.</strong> There <c>alpha</c> is how
/// much of the pattern colour shows and <c>solid</c> is <c>0x80</c>; here <c>0x00</c> is the full
/// pattern colour and <c>0x80</c> the full background, which is why <c>solid</c> is index 1 with
/// ratio <c>0x00</c>. The two tables are not each other's complement either — BIFF's 12.5 %
/// grey is <c>0x70</c> where SpreadsheetML's <c>gray125</c> is <c>0x10</c>, which is the same
/// thing said twice — so neither can be derived from the other and both are transcribed.
/// </para>
/// <para>
/// Both BIFF readers used to take one of the two colours whole: <c>XlsDecorationTable.FormatOf</c>
/// the background for anything but <c>solid</c>, and <c>XlsConditionalFormats.ReadAreaBlock</c> the
/// foreground. Measured on <c>PC1000.xls</c>, whose 82 cells state pattern <c>0x11</c> — 12.5 %
/// grey, black over white — 26.2.4.2's own <c>--convert-to fods</c> gives their cell style
/// <c>fo:background-color="#dfdfdf"</c>, which is <c>(255 − 0) × 0x70 / 0x80</c> exactly.
/// </para>
/// </remarks>
internal static class XlsPatternFill
{
    /// <summary>A pattern that paints nothing, <c>EXC_PATT_NONE</c>.</summary>
    public const int None = 0;

    /// <summary>A pattern whose colour is its foreground whole, <c>EXC_PATT_SOLID</c>.</summary>
    public const int Solid = 1;

    /// <summary>
    /// The colour a pattern paints over its two colours.
    /// </summary>
    /// <remarks>
    /// Neither colour may be absent by the time this is asked: an unstated foreground is the
    /// system window <em>text</em> colour and an unstated background the window background —
    /// <c>EXC_COLOR_WINDOWTEXT</c> and <c>EXC_COLOR_WINDOWBACK</c>, black and white — which is
    /// what <c>XclImpCellArea::FillToItemSet</c> (<c>xistyle.cxx</c>:1105-1111) substitutes
    /// before it mixes. A caller that has neither passes those.
    /// </remarks>
    /// <param name="pattern">The six-bit pattern field, 0 to 63.</param>
    /// <param name="foreground">The pattern colour.</param>
    /// <param name="background">The colour behind it.</param>
    public static Colour Resolve(int pattern, Colour foreground, Colour background)
        => Mix(foreground, background, RatioOf(pattern));

    /// <summary>
    /// How much of the <em>background</em> shows, out of <c>0x80</c>.
    /// </summary>
    /// <remarks>
    /// <c>pnRatioTable</c>, <c>xltools.cxx</c>:351-356. A pattern past the table's nineteen
    /// entries returns the pattern colour whole, which <c>0x00</c> gives.
    /// </remarks>
    private static int RatioOf(int pattern)
        => pattern >= 0 && pattern < Ratios.Length ? Ratios[pattern] : 0x00;

    /// <summary>
    /// <c>ScfTools::GetMixedColor</c>: integer, per component, and truncating towards zero.
    /// </summary>
    /// <remarks>
    /// The arithmetic is written the way the source writes it rather than through a double,
    /// because C++ integer division truncates towards zero and the sign of
    /// <c>background − pattern</c> therefore decides which way a component rounds.
    /// </remarks>
    private static Colour Mix(Colour pattern, Colour background, int ratio)
        => new(
            (byte)Component(pattern.R, background.R, ratio),
            (byte)Component(pattern.G, background.G, ratio),
            (byte)Component(pattern.B, background.B, ratio));

    private static int Component(int pattern, int background, int ratio)
        => (((background - pattern) * ratio) / 0x80) + pattern;

    /// <summary>
    /// <c>pnRatioTable</c>, patterns 0 to 18 in order.
    /// </summary>
    /// <remarks>
    /// Entry 0 is never reached: a caller must test <see cref="None"/> first, because a pattern
    /// of none paints nothing at all rather than painting the background.
    /// </remarks>
    private static readonly int[] Ratios =
    [
        0x80, 0x00, 0x40, 0x20, 0x60, 0x40, 0x40, 0x40,
        0x40, 0x40, 0x20, 0x60, 0x60, 0x60, 0x60, 0x48,
        0x50, 0x70, 0x78,
    ];
}
