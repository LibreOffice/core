namespace Paperless.OpenDocument.Styles;

/// <summary>
/// Resolves the effective formatting of an ODF object by walking its style chain.
/// </summary>
/// <remarks>
/// <para>
/// ODF splits styles three ways and all three participate in resolution:
/// <c>office:styles</c> holds named styles the user can see, <c>office:automatic-styles</c>
/// holds generated one-off styles standing in for direct formatting, and
/// <c>office:master-styles</c> holds page and slide masters.
/// </para>
/// <para>
/// Resolution walks <c>style:parent-style-name</c> upwards, then falls back to the
/// family's defaults from <c>style:default-style</c>. This is the same
/// resolve-through-a-parent-chain semantics LibreOffice implements with
/// <c>SfxItemSet</c>, described in <c>dotnet/research/05-infrastructure.md</c>
/// section E — a property is either set here, inherited, or defaulted, and
/// <see cref="OdfProperty.Origin"/> keeps the three cases distinguishable rather than
/// collapsing them into "has a value".
/// </para>
/// <para>
/// The implementation is <see cref="OdfStyles"/>. The interface exists so that
/// code needing only resolution — most of it — does not have to be handed the whole
/// collection, and so tests can substitute a fixed set of properties.
/// </para>
/// </remarks>
public interface IOdfStyleResolver
{
    /// <summary>
    /// Resolves one property for a named style, following the parent chain and then the
    /// family defaults.
    /// </summary>
    /// <param name="styleName">
    /// The style to start from. Null resolves straight to the family defaults, which is
    /// what an object with no style attribute at all should get.
    /// </param>
    /// <param name="family">The style's family; part of its identity, not a hint.</param>
    /// <param name="kind">
    /// Which <c>style:*-properties</c> element to look in. Required because the same
    /// attribute name means different things in different property sets.
    /// </param>
    /// <param name="propertyNamespace">The property attribute's namespace URI.</param>
    /// <param name="propertyName">The property attribute's local name.</param>
    /// <returns>
    /// The value with its origin, or <see cref="OdfProperty.Unset"/> when nothing in the
    /// chain sets it.
    /// </returns>
    OdfProperty ResolveProperty(
        string? styleName,
        OdfStyleFamily family,
        OdfPropertyKind kind,
        string propertyNamespace,
        string propertyName);
}
