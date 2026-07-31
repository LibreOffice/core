using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Rtf;

/// <summary>
/// One stretch of an RTF paragraph's text and the character formatting in force over it.
/// </summary>
/// <remarks>
/// Only what a page needs: what decides a glyph's width, and the colour it is drawn in. RTF states these
/// as loose control words rather than as a property set, so the state in force is snapshotted as the text
/// is appended — there is no group to point back at afterwards.
/// </remarks>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="FamilyName">The family the font table names, or null when it names none.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the text is drawn in, or null for the automatic colour.</param>
public readonly record struct RtfLayoutRun(
    int Start,
    int Length,
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>
    /// True when another run's formatting is identical, whatever its range.
    /// </summary>
    /// <remarks>
    /// What decides whether two adjacent stretches merge. RTF restates properties freely — a producer
    /// writes <c>\f0\fs22</c> before every run whether or not anything changed — and an unmerged
    /// restatement would break the shaping context for no reason, losing a kern pair mid-word.
    /// </remarks>
    public bool MatchesFormatting(RtfLayoutRun other)
        => string.Equals(FamilyName, other.FamilyName, StringComparison.Ordinal)
           && Size == other.Size
           && Weight == other.Weight
           && IsItalic == other.IsItalic
           && string.Equals(Language, other.Language, StringComparison.Ordinal)
           && Colour == other.Colour;
}

/// <summary>
/// One of an RTF body's paragraphs, with the formatting layout needs.
/// </summary>
/// <remarks>
/// <para>
/// Collected during the content walk rather than by a second pass, which is the one structural
/// difference from the XML formats' layout sources. RTF is a token stream with nothing to revisit —
/// re-reading it would mean running the whole state machine again, including its encoding and
/// destination handling, and two runs could then disagree about what the document says. So the state in
/// force is recorded as each paragraph closes.
/// </para>
/// <para>
/// The properties themselves are translated through <see cref="Ww8.Ww8LayoutFormat"/>, because RTF and
/// the binary format state them the same way: twips throughout, a font size in half-points, and a line
/// spacing whose sign and companion flag together choose between a multiple, a minimum and a fixed
/// height. Writing that arithmetic twice would be two chances to get the sign wrong.
/// </para>
/// </remarks>
/// <param name="Text">The paragraph's text.</param>
/// <param name="Format">Its resolved layout properties.</param>
/// <param name="FamilyName">The family the font table names, or null when it names none.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the paragraph's mark carries, or null for the automatic colour.</param>
/// <param name="Runs">
/// The stretches its character formatting divides it into, in order. Always populated, even where the
/// whole paragraph is uniform — the layout source decides whether they are worth carrying, since it is
/// the only party that can compare two <em>resolved</em> faces rather than two requested families.
/// </param>
public readonly record struct RtfLayoutParagraph(
    string Text,
    ParagraphFormat Format,
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour = null,
    IReadOnlyList<RtfLayoutRun>? Runs = null);
