using System.Xml.Linq;

namespace Paperless.OpenDocument.Styles;

/// <summary>Identifies a formatting property by namespace and local name.</summary>
/// <param name="Namespace">The attribute's namespace URI, from <see cref="OdfNamespaces"/>.</param>
/// <param name="LocalName">The attribute's local name, without a prefix.</param>
public readonly record struct OdfPropertyName(string Namespace, string LocalName)
{
    /// <inheritdoc/>
    public override string ToString() => $"{{{Namespace}}}{LocalName}";
}

/// <summary>
/// One <c>style:*-properties</c> element: the formatting attributes it carries, plus the
/// child elements that hold structured formatting.
/// </summary>
/// <remarks>
/// Most ODF formatting is attributes, but not all of it: tab stops, list-label alignment,
/// column definitions, background images and border-line details are child elements. Those
/// are kept as-is rather than flattened, because flattening them would need a schema for
/// every one and the consumers that care can read the elements directly.
/// </remarks>
public sealed class OdfPropertySet
{
    private readonly Dictionary<OdfPropertyName, string> _properties;
    private readonly List<XElement> _children;

    internal OdfPropertySet(OdfPropertyKind kind, XElement element)
    {
        Kind = kind;
        _properties = new Dictionary<OdfPropertyName, string>(element.Attributes().Count());
        foreach (XAttribute attribute in element.Attributes())
        {
            if (attribute.IsNamespaceDeclaration) continue;
            _properties[new OdfPropertyName(attribute.Name.NamespaceName, attribute.Name.LocalName)] =
                attribute.Value;
        }
        _children = [.. element.Elements()];
    }

    /// <summary>Which property set this is.</summary>
    public OdfPropertyKind Kind { get; }

    /// <summary>The formatting attributes carried directly by this element.</summary>
    public IReadOnlyDictionary<OdfPropertyName, string> Properties => _properties;

    /// <summary>The structured formatting children, in document order.</summary>
    public IReadOnlyList<XElement> Children => _children;

    /// <summary>Looks up one attribute, returning null when it is absent.</summary>
    public string? Get(string propertyNamespace, string localName)
        => _properties.TryGetValue(new OdfPropertyName(propertyNamespace, localName), out string? value)
            ? value
            : null;

    /// <summary>Finds the first child element with the given name.</summary>
    public XElement? Child(string childNamespace, string localName)
    {
        foreach (XElement child in _children)
        {
            if (child.Name.LocalName == localName && child.Name.NamespaceName == childNamespace)
                return child;
        }
        return null;
    }
}
