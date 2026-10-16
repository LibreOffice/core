namespace XRay.Office.Text.Fonts;

/// <summary>
/// The font item a run is set from, as glyph fallback needs it: a family, a class and a language.
/// </summary>
/// <remarks>
/// <para>
/// <strong>It travels with the run rather than being looked up from the face, because the item is a
/// property of the run and the face is shared.</strong> A resolver that records the generic against
/// the face it chose gets the <em>first</em> request to reach that face, and in a
/// word-processing document that is the paragraph mark — so a <c>w:hint="eastAsia"</c> run whose
/// family resolves to the same face as the body text would silently take the body's western item.
/// Measured: with the item recorded against the face, every cell of
/// <c>probes/fonts-r65/gen-scriptitem.py</c> answered exactly as it did before the item existed.
/// </para>
/// <para>
/// <see cref="IsStated"/> is what tells a caller that supplies one from a caller that does not.
/// Slides, sheets and metafiles have no script items to select between and pass nothing, which
/// leaves them on the face-keyed lookup they have always used.
/// </para>
/// </remarks>
/// <param name="FamilyName">The family the run asked for, which the pattern names first.</param>
/// <param name="DeclaredClass">
/// The class the item carries, which through a DOCX only the western item ever does.
/// </param>
/// <param name="Language">The item's language, as a BCP 47 tag.</param>
public readonly record struct FontItem(
    string? FamilyName, FontFamilyClass DeclaredClass, string? Language)
{
    /// <summary>True when a caller supplied an item at all.</summary>
    public bool IsStated => Language is { Length: > 0 };
}
