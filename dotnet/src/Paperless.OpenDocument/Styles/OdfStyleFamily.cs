namespace Paperless.OpenDocument.Styles;

/// <summary>
/// The ODF style families. A style's family determines which properties it may carry
/// and what it may be applied to.
/// </summary>
public enum OdfStyleFamily
{
    /// <summary>Paragraph styles.</summary>
    Paragraph,

    /// <summary>Character styles.</summary>
    Text,

    /// <summary>Section styles.</summary>
    Section,

    /// <summary>Table styles.</summary>
    Table,

    /// <summary>Table column styles.</summary>
    TableColumn,

    /// <summary>Table row styles.</summary>
    TableRow,

    /// <summary>Table cell styles.</summary>
    TableCell,

    /// <summary>Graphic and frame styles.</summary>
    Graphic,

    /// <summary>Presentation styles, used for placeholder content.</summary>
    Presentation,

    /// <summary>Drawing page styles, carrying slide backgrounds and transitions.</summary>
    DrawingPage,

    /// <summary>List styles.</summary>
    List,
}

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
/// section E — a property is either set here, inherited, or defaulted, and the
/// three cases must stay distinguishable rather than collapsing into "has a value".
/// </para>
/// </remarks>
public interface IOdfStyleResolver
{
    /// <summary>
    /// Resolves one property for a named style, following the parent chain and then
    /// the family defaults. Returns null when nothing in the chain sets it.
    /// </summary>
    /// <param name="styleName">The style to start from.</param>
    /// <param name="family">The style's family.</param>
    /// <param name="propertyNamespace">The property attribute's namespace URI.</param>
    /// <param name="propertyName">The property attribute's local name.</param>
    string? ResolveProperty(
        string styleName,
        OdfStyleFamily family,
        string propertyNamespace,
        string propertyName);
}
