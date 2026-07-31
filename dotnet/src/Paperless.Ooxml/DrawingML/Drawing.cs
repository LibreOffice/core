using System.Xml.Linq;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Shorthand for reaching into DrawingML, where nearly every name is <c>a:</c>-namespaced.
/// </summary>
/// <remarks>
/// Matching is on namespace and local name, never on prefix: a package may bind the DrawingML
/// namespace to any prefix it likes, and the anchoring vocabularies around it
/// (<c>xdr:</c>, <c>wp:</c>, <c>p:</c>) differ per family while the drawing inside them does
/// not. Parts are normalised to the transitional namespace when loaded (see
/// <see cref="OoxmlXml.Normalise"/>), so one namespace is enough here.
/// </remarks>
public static class Drawing
{
    /// <summary>The DrawingML name for a local name.</summary>
    public static XName Name(string localName) => XName.Get(localName, OoxmlNamespaces.DrawingML);

    /// <summary>An unprefixed attribute's value, or null.</summary>
    /// <remarks>
    /// Unprefixed because DrawingML attributes are unqualified — <c>&lt;a:pPr lvl="1"/&gt;</c>,
    /// not <c>a:lvl</c>. Only <c>r:id</c> and <c>xml:*</c> carry a namespace.
    /// </remarks>
    public static string? Attribute(XElement? element, string name)
        => element?.Attribute(name)?.Value;

    /// <summary>The first DrawingML child with this local name, or null.</summary>
    public static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    /// <summary>Every DrawingML child with this local name, in document order.</summary>
    public static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    /// <summary>True when an element is the named DrawingML element.</summary>
    public static bool Is(XElement? element, string localName)
        => element is not null
           && element.Name.NamespaceName == OoxmlNamespaces.DrawingML
           && element.Name.LocalName == localName;

    /// <summary>
    /// An <c>r:id</c> relationship reference, which is how a hyperlink or an image names its
    /// target.
    /// </summary>
    public static string? RelationshipId(XElement? element)
        => element?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

    /// <summary>
    /// Reads an integer attribute, or null when it is absent or unparseable.
    /// </summary>
    /// <remarks>
    /// Lenient rather than throwing: a producer writing <c>lvl="x"</c> should cost the reader a
    /// default level, not the whole slide.
    /// </remarks>
    public static int? Number(XElement? element, string name)
        => int.TryParse(
            Attribute(element, name),
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int parsed)
            ? parsed
            : null;

    /// <summary>
    /// Reads an ST_Boolean attribute, which DrawingML spells <c>1</c>/<c>0</c> or
    /// <c>true</c>/<c>false</c>.
    /// </summary>
    /// <returns>Null when the attribute is absent, so "not stated" stays distinguishable
    /// from "stated false" — which matters because a property that is merely unset inherits
    /// while one set false does not.</returns>
    public static bool? Flag(XElement? element, string name) => Attribute(element, name) switch
    {
        "1" or "true" or "on" => true,
        "0" or "false" or "off" => false,
        _ => null,
    };
}
