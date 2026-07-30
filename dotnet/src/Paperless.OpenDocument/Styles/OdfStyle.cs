using System.Xml.Linq;

namespace Paperless.OpenDocument.Styles;

/// <summary>Which of ODF's three style containers a style was declared in.</summary>
/// <remarks>
/// ODF splits styles three ways and the split is meaningful, not organisational — see
/// <see cref="OdfStyles"/> for why each container exists.
/// </remarks>
public enum OdfStyleSource
{
    /// <summary>Declared in <c>office:styles</c>: a named, user-visible style.</summary>
    Named = 0,

    /// <summary>
    /// Declared in <c>office:automatic-styles</c>: a generated single-use style standing
    /// in for direct formatting.
    /// </summary>
    Automatic,

    /// <summary>Declared as <c>style:default-style</c>: the family's defaults.</summary>
    Default,
}

/// <summary>
/// One parsed ODF style: its identity, its place in the parent chain, and the property
/// sets it carries.
/// </summary>
/// <remarks>
/// A style holds only what it itself declares. Resolution across the parent chain and the
/// family defaults is <see cref="OdfStyles"/>'s job, which is what keeps
/// "set here" distinguishable from "inherited".
/// </remarks>
public sealed class OdfStyle
{
    private readonly Dictionary<OdfPropertyKind, OdfPropertySet> _propertySets = [];

    internal OdfStyle(string name, OdfStyleFamily family, OdfStyleSource source, XElement element)
    {
        Name = name;
        Family = family;
        Source = source;

        DisplayName = Attribute(element, OdfNamespaces.Style, "display-name");
        ParentStyleName = Attribute(element, OdfNamespaces.Style, "parent-style-name");
        NextStyleName = Attribute(element, OdfNamespaces.Style, "next-style-name");
        ListStyleName = Attribute(element, OdfNamespaces.Style, "list-style-name");
        MasterPageName = Attribute(element, OdfNamespaces.Style, "master-page-name");
        DataStyleName = Attribute(element, OdfNamespaces.Style, "data-style-name");
        StyleClass = Attribute(element, OdfNamespaces.Style, "class");
        DefaultOutlineLevel = OdfValue.ParseInt(Attribute(element, OdfNamespaces.Style, "default-outline-level"));

        foreach (XElement child in element.Elements())
        {
            OdfPropertyKind kind = OdfPropertyKinds.FromElementName(child.Name.LocalName);
            if (kind == OdfPropertyKind.Unknown) continue;

            // A style may carry the same kind twice when LibreOffice writes both a
            // style:-namespaced and a loext:-namespaced set (it does this for graphic
            // properties on paragraph styles). Merge rather than replace: the loext set
            // holds extra properties, not a correction of the standard one.
            if (_propertySets.TryGetValue(kind, out OdfPropertySet? existing))
                _propertySets[kind] = Merge(existing, new OdfPropertySet(kind, child));
            else
                _propertySets[kind] = new OdfPropertySet(kind, child);
        }
    }

    /// <summary>The style's name, as referenced by <c>*:style-name</c> attributes.</summary>
    /// <remarks>
    /// This is the encoded name — LibreOffice escapes spaces and other characters, so the
    /// style a user sees as "Text body" is named <c>Text_20_body</c> here.
    /// <see cref="DisplayName"/> holds the unescaped form when the file records one.
    /// </remarks>
    public string Name { get; }

    /// <summary>The style's family, which is part of its identity.</summary>
    public OdfStyleFamily Family { get; }

    /// <summary>Which container declared the style.</summary>
    public OdfStyleSource Source { get; }

    /// <summary>The user-visible name, when the file records one separately.</summary>
    public string? DisplayName { get; }

    /// <summary>The parent style's name, or null at the top of the chain.</summary>
    public string? ParentStyleName { get; }

    /// <summary>The style to apply to the next paragraph, for paragraph styles.</summary>
    public string? NextStyleName { get; }

    /// <summary>The list style governing numbering for paragraphs in this style.</summary>
    public string? ListStyleName { get; }

    /// <summary>The master page this style forces a break to, when it does.</summary>
    public string? MasterPageName { get; }

    /// <summary>The number format applied to cell values, for cell styles.</summary>
    public string? DataStyleName { get; }

    /// <summary>
    /// The style's <c>style:class</c> — a coarse grouping ("text", "list", "index") the UI
    /// uses to bucket styles. Not part of resolution.
    /// </summary>
    public string? StyleClass { get; }

    /// <summary>
    /// The outline level a paragraph in this style occupies, when the style makes its
    /// paragraphs headings without them being written as <c>text:h</c>.
    /// </summary>
    public int? DefaultOutlineLevel { get; }

    /// <summary>The property sets this style declares, keyed by kind.</summary>
    public IReadOnlyDictionary<OdfPropertyKind, OdfPropertySet> PropertySets => _propertySets;

    /// <summary>The property set of a given kind, or null when this style declares none.</summary>
    public OdfPropertySet? Properties(OdfPropertyKind kind)
        => _propertySets.TryGetValue(kind, out OdfPropertySet? set) ? set : null;

    /// <summary>
    /// Looks up a property declared by this style alone, without consulting its parents.
    /// </summary>
    public string? GetOwnProperty(OdfPropertyKind kind, string propertyNamespace, string localName)
        => Properties(kind)?.Get(propertyNamespace, localName);

    private static string? Attribute(XElement element, string ns, string localName)
        => element.Attribute(XName.Get(localName, ns))?.Value;

    private static OdfPropertySet Merge(OdfPropertySet first, OdfPropertySet second)
    {
        // Rebuilt through a synthetic element so OdfPropertySet stays the single place
        // that knows how a property set is shaped.
        XElement merged = new(XName.Get(first.Kind.ToString(), OdfNamespaces.Style));
        foreach ((OdfPropertyName name, string value) in first.Properties)
            merged.SetAttributeValue(XName.Get(name.LocalName, name.Namespace), value);
        foreach ((OdfPropertyName name, string value) in second.Properties)
            merged.SetAttributeValue(XName.Get(name.LocalName, name.Namespace), value);
        foreach (XElement child in first.Children.Concat(second.Children))
            merged.Add(new XElement(child));
        return new OdfPropertySet(first.Kind, merged);
    }
}
