namespace XRay.Office.WordProcessing;

/// <summary>
/// The two rules Writer applies to the text a <c>REF</c> field takes from its bookmark, whichever
/// format the bookmark came out of.
/// </summary>
/// <remarks>
/// Shared because three of the four readers meet the same field: the arithmetic is
/// <c>SwGetRefField</c>'s and has nothing to do with how a bookmark was spelled, so a second copy of
/// it would be a second chance to get the control characters wrong.
/// </remarks>
internal static class ReferenceFieldText
{
    /// <summary>What Writer draws for a <c>REF</c> whose bookmark it cannot find.</summary>
    /// <remarks><c>STR_GETREFFLD_REFITEMNOTFOUND</c>, <c>sw/inc/strings.hrc</c>:787.</remarks>
    public const string NotFound = "Error: Reference source not found";

    /// <summary>
    /// The two names Writer gives its own cross-reference bookmarks.
    /// </summary>
    /// <remarks>
    /// <c>IDocumentMarkAccess::GetType</c> and <c>CrossRefBookmark</c>'s two subclasses
    /// (<c>sw/inc/crossrefbookmark.hxx</c>); the prefixes are what its ODF and Word filters write. A
    /// collapsed bookmark of either name stands for its whole node (#i81002#), where any other
    /// collapsed one stands for nothing.
    /// </remarks>
    public static bool IsCrossReference(string name)
        => name.StartsWith("__RefHeading__", StringComparison.Ordinal)
           || name.StartsWith("__RefNumPara__", StringComparison.Ordinal);

    /// <summary>
    /// <c>FilterText</c> (<c>sw/source/core/fields/reffld.cxx</c>:461-489): no soft hyphens, no
    /// control characters and no non-breaking hyphen in a reference's text.
    /// </summary>
    /// <remarks>
    /// The control-character rule is what makes a multi-line caption come back as one line: a line
    /// break inside the bookmark is U+000A in the text node and reaches the field as a space, so a
    /// reader that keeps the break draws the reference over two lines where the reference draws one.
    /// </remarks>
    public static string Filter(string text)
    {
        if (text.Length == 0) return text;

        char[]? buffer = null;
        int length = 0;
        for (int i = 0; i < text.Length; i++)
        {
            char character = text[i];
            char replaced = character switch
            {
                '­' => '\0',   // a soft hyphen is removed outright, so nought stands for "drop"
                '‑' => '-',
                < ' ' => ' ',
                _ => character,
            };

            if (buffer is null)
            {
                if (replaced == character) { length++; continue; }

                buffer = new char[text.Length];
                text.AsSpan(0, length).CopyTo(buffer);
            }

            if (replaced != '\0') buffer[length++] = replaced;
        }

        return buffer is null ? text : new string(buffer, 0, length);
    }
}
