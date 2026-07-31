using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Rtf;

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
public readonly record struct RtfLayoutParagraph(
    string Text,
    ParagraphFormat Format,
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language);
