using Paperless.Core.Units;

namespace Paperless.Text.Shaping;

/// <summary>
/// One positioned glyph.
/// </summary>
/// <param name="GlyphId">The glyph's index in the face.</param>
/// <param name="Cluster">
/// The index of the first character the glyph came from. Several glyphs may share a cluster (a mark
/// following its base) and one glyph may cover several characters (a ligature), so this is a mapping
/// back to the text rather than a one-to-one correspondence.
/// </param>
/// <param name="Advance">How far the pen moves after the glyph, in the face's design units.</param>
/// <param name="OffsetX">A horizontal displacement of the glyph that does not move the pen.</param>
/// <param name="OffsetY">A vertical displacement of the glyph that does not move the pen.</param>
public readonly record struct ShapedGlyph(
    ushort GlyphId,
    int Cluster,
    int Advance,
    int OffsetX,
    int OffsetY);

/// <summary>
/// What a shaper made of a run of text: the glyphs, and the width of every prefix of the text.
/// </summary>
/// <remarks>
/// <para>
/// The prefix widths are the point. Filling lines asks for the width of a great many prefixes of the
/// same paragraph — every break opportunity, on every line — and shaping each one separately would be
/// both quadratic and wrong. Wrong because shaping is contextual: the glyphs a shaper picks for a
/// prefix are not always the glyphs it picks for that prefix inside the whole text, so a paragraph
/// measured prefix by prefix does not add up to itself.
/// </para>
/// <para>
/// A glyph's advance is credited to the character its cluster names, so a ligature's whole width lands
/// on its first character and a position inside the ligature measures as its end. That is what
/// LibreOffice does: <c>GenericSalLayout::GetPartialTextWidth</c> adds a glyph's whole width whenever
/// its <c>charPos()</c> falls in the range asked about
/// (<c>vcl/source/gdi/sallayout.cxx</c>). Its <em>caret</em> positions are worked out differently —
/// <c>GetCharWidths</c> spreads a ligature across its grapheme clusters using
/// <c>hb_ot_layout_get_ligature_carets</c> — but that is for placing a cursor, not for deciding a line
/// break, and nothing here asks for a width inside a ligature because there is no break opportunity
/// there.
/// </para>
/// </remarks>
public sealed class ShapedText
{
    private readonly ShapedGlyph[] _glyphs;
    private readonly long[] _prefix;

    internal ShapedText(ShapedGlyph[] glyphs, long[] prefix, int unitsPerEm)
    {
        _glyphs = glyphs;
        _prefix = prefix;
        UnitsPerEm = unitsPerEm;
    }

    /// <summary>The grid an empty run reports, so scaling it divides by something.</summary>
    private const int FallbackUnitsPerEm = 1000;

    /// <summary>An empty run, which every measurement of reads as zero.</summary>
    public static ShapedText Empty { get; } = new([], [0], FallbackUnitsPerEm);

    /// <summary>The glyphs, in visual order for the run's direction.</summary>
    public IReadOnlyList<ShapedGlyph> Glyphs => _glyphs;

    /// <summary>The design grid the advances are in.</summary>
    public int UnitsPerEm { get; }

    /// <summary>How many characters were shaped.</summary>
    public int TextLength => _prefix.Length - 1;

    /// <summary>The whole run's advance width, in design units.</summary>
    public long AdvanceInDesignUnits => _prefix[^1];

    /// <summary>
    /// The advance width of the run's first <paramref name="characterCount"/> characters, in design
    /// units.
    /// </summary>
    public long AdvanceUpTo(int characterCount)
        => _prefix[Math.Clamp(characterCount, 0, TextLength)];

    /// <summary>The advance width of the characters between two indices, in design units.</summary>
    public long AdvanceBetween(int start, int end)
        => AdvanceUpTo(end) - AdvanceUpTo(start);

    /// <summary>The whole run's width at an em size.</summary>
    public Length Width(Length emSize) => Scale(AdvanceInDesignUnits, emSize);

    /// <summary>The width of the run's first <paramref name="characterCount"/> characters.</summary>
    public Length WidthUpTo(int characterCount, Length emSize)
        => Scale(AdvanceUpTo(characterCount), emSize);

    /// <summary>The width of the characters between two indices.</summary>
    public Length WidthBetween(int start, int end, Length emSize)
        => Scale(AdvanceBetween(start, end), emSize);

    /// <summary>
    /// Scales a measurement in design units to an em size.
    /// </summary>
    /// <remarks>
    /// Once, at the end. Scaling and rounding each glyph and then adding them accumulates an error
    /// proportional to the line's length, which is exactly where it does the most damage — at the
    /// margin, deciding a break.
    /// </remarks>
    public Length Scale(long designUnits, Length emSize)
        => UnitsPerEm <= 0
            ? Length.Zero
            : Length.FromEmu((long)Math.Round((double)designUnits * emSize.Emu / UnitsPerEm));

    /// <summary>
    /// Builds the prefix table from glyphs whose clusters index into a text of a known length.
    /// </summary>
    internal static long[] PrefixWidths(ReadOnlySpan<ShapedGlyph> glyphs, int textLength)
    {
        // One entry per position, plus the end: prefix[i] is the width of the first i characters.
        long[] perCharacter = new long[textLength + 1];
        foreach (ShapedGlyph glyph in glyphs)
        {
            // A cluster out of range would be a shaper bug, but clamping keeps a damaged font from
            // throwing out of a measurement.
            int at = Math.Clamp(glyph.Cluster, 0, textLength - 1);
            perCharacter[at] += glyph.Advance;
        }

        long running = 0;
        for (int i = 0; i < textLength; i++)
        {
            long own = perCharacter[i];
            perCharacter[i] = running;
            running += own;
        }
        perCharacter[textLength] = running;
        return perCharacter;
    }
}
