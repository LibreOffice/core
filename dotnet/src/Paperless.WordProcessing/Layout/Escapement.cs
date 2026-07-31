using Paperless.Core.Units;
using Paperless.Text.Fonts;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// A superscript or subscript: how far the text moves off the baseline, and how much it shrinks.
/// </summary>
/// <remarks>
/// <para>
/// Two numbers rather than one because the formats state two and a document can set either alone — text
/// raised at full size is legal, and so is text shrunk without being moved. Every format spells it
/// differently (<c>style:text-position</c>, <c>w:vertAlign</c>, <c>\super</c>, <c>sprmCIss</c>) but they all
/// reduce to this pair, so the arithmetic lives here once.
/// </para>
/// <para>
/// The arithmetic is the part worth writing down, because the obvious reading of "raised 33%" is wrong. The
/// percentage is of the font's <em>height</em> — its ascent plus descent plus line gap — not of its em size,
/// which for a typical Latin face is about 22% more. Reading it as the em size puts a superscript 0.7 pt low
/// at eleven point, which is seven times the tolerance a rendering comparison runs at. LibreOffice's own
/// arithmetic is <c>nOfst = m_nOrgHeight * nEsc / 100</c> in <c>swfont.cxx</c>, where <c>m_nOrgHeight</c> is
/// the unshrunk font's height in the reference device's units.
/// </para>
/// <para>
/// Both results snap to whole twips, because Writer measures in them and the snapping is visible: 58% of
/// eleven point is 127.6 twips, and LibreOffice draws the citation at 128 — 6.4 pt, not 6.38.
/// </para>
/// </remarks>
/// <param name="Percent">
/// How far the text moves, as a percentage of the font's height; positive raises it, negative lowers it,
/// zero leaves it on the baseline.
/// </param>
/// <param name="Proportion">
/// The size the text is set at, as a percentage of the size it would otherwise take. Zero and 100 both mean
/// no change, so that a default-constructed value is "no escapement at all".
/// </param>
public readonly record struct Escapement(int Percent, int Proportion)
{
    /// <summary>
    /// The rise LibreOffice applies to an <em>automatic</em> superscript, as a percentage of the height.
    /// </summary>
    /// <remarks>
    /// <c>DFLT_ESC_SUPER</c> in <c>editeng/escapementitem.hxx</c>, whose comment does the conversion the
    /// other way: 33% of the total font height is about 42% of the ascent, which is what shrinking to 58%
    /// leaves above the smaller text. Measured against LibreOffice's own output at two sizes — a footnote
    /// citation at eleven point is raised exactly 4.40 pt and at ten point exactly 4.00 pt, which is this
    /// number applied to Carlito's height and truncated to twips in both cases.
    /// </remarks>
    public const int AutomaticPercent = 33;

    /// <summary>The size an automatic superscript or subscript is set at.</summary>
    /// <remarks><c>DFLT_ESC_PROP</c>, the companion to <see cref="AutomaticPercent"/>.</remarks>
    public const int AutomaticProportion = 58;

    /// <summary>Text on the baseline at its own size.</summary>
    public static Escapement None => default;

    /// <summary>LibreOffice's automatic superscript: raised a third of the height, at 58% of the size.</summary>
    public static Escapement Superscript { get; } = new(AutomaticPercent, AutomaticProportion);

    /// <summary>Its automatic subscript, which is the same pair lowered.</summary>
    public static Escapement Subscript { get; } = new(-AutomaticPercent, AutomaticProportion);

    /// <summary>True when the text is neither moved nor resized.</summary>
    public bool IsNone => Percent == 0 && Proportion is 0 or 100;

    /// <summary>The size the text is actually set at, given the size it would otherwise take.</summary>
    public Length SizeOf(Length emSize)
        => Proportion is 0 or 100 ? emSize : Twips(emSize.Twips * Proportion / 100.0);

    /// <summary>
    /// How far the text sits above the baseline, negative for a subscript.
    /// </summary>
    /// <param name="face">The face the text is set in, whose height the percentage is of.</param>
    /// <param name="emSize">The size the text would take were it not escaped.</param>
    /// <remarks>
    /// Truncated rather than rounded, which is what the measurements say: at eleven point Carlito's height is
    /// 268.6 twips and 33% of that is 88.6, and LibreOffice draws the citation 88 twips up rather than 89.
    /// </remarks>
    public Length RiseOf(OpenTypeFace face, Length emSize)
    {
        if (Percent == 0) return Length.Zero;

        double height = Math.Round((double)LineSpacing.Resolve(face).ScaledLineHeight(emSize).Twips);
        return Length.FromTwips((long)(height * Percent / 100.0));
    }

    private static Length Twips(double value) => Length.FromTwips((long)Math.Round(value));
}
