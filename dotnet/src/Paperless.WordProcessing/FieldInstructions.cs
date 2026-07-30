namespace Paperless.WordProcessing;

/// <summary>
/// Reads what a word-processing field's instruction says, for the fields whose meaning is not
/// recoverable from their cached result.
/// </summary>
/// <remarks>
/// <para>
/// A field is a small program: an instruction, and the result the writing application last computed
/// for it. Paperless keeps the result, because that is what a reader saw — but a hyperlink's
/// <em>target</em> appears only in the instruction, so that one field has to be understood rather
/// than skipped.
/// </para>
/// <para>
/// Shared because three of the four word-processing formats spell a hyperlink the same way. Neither
/// RTF nor DOC has hyperlink markup at all, and a DOCX written by a converter often has a
/// <c>HYPERLINK</c> field where a native one would have a relationship — so the same parsing serves
/// all three, over three completely different containers.
/// </para>
/// </remarks>
public static class FieldInstructions
{
    /// <summary>The field name that introduces a hyperlink.</summary>
    private const string Hyperlink = "HYPERLINK";

    /// <summary>
    /// The target of a <c>HYPERLINK</c> field, or null when the instruction is not one.
    /// </summary>
    /// <remarks>
    /// The syntax is <c>HYPERLINK "target"</c> with optional switches, and the quoted argument is the
    /// target. An unquoted argument is accepted too: producers omit the quotes when the target has no
    /// spaces, and rejecting those loses ordinary links. A <c>\l</c> switch introduces a bookmark
    /// within the document, which is a location rather than a target and is left to the caller.
    /// </remarks>
    public static string? HyperlinkTarget(string? instruction)
    {
        if (instruction is null) return null;

        string text = instruction.Trim();
        if (!text.StartsWith(Hyperlink, StringComparison.OrdinalIgnoreCase)) return null;

        string arguments = text[Hyperlink.Length..].TrimStart();

        int firstQuote = arguments.IndexOf('"', StringComparison.Ordinal);
        if (firstQuote >= 0)
        {
            int secondQuote = arguments.IndexOf('"', firstQuote + 1);
            if (secondQuote < 0) return null;

            string quoted = arguments[(firstQuote + 1)..secondQuote];
            return quoted.Length == 0 ? null : quoted;
        }

        // Unquoted: the target runs to the first whitespace, since anything after it is a switch.
        int end = arguments.AsSpan().IndexOfAny(' ', '\t', '\n');
        string bare = end < 0 ? arguments : arguments[..end];
        return bare.Length == 0 || bare.StartsWith('\\') ? null : bare;
    }
}
