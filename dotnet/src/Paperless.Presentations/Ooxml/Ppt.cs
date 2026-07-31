using System.Xml.Linq;
using Paperless.Ooxml;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Shorthand for reaching into PresentationML, where the deck's own vocabulary is
/// <c>p:</c>-namespaced and everything it contains is DrawingML.
/// </summary>
/// <remarks>
/// The split is worth keeping straight: <c>p:sp</c>, <c>p:spTree</c>, <c>p:ph</c> and
/// <c>p:txBody</c> are PresentationML, while <c>a:p</c>, <c>a:r</c>, <c>a:tbl</c> and every
/// property inside them are DrawingML. Reading one with the other's namespace finds nothing and
/// produces a silently empty slide.
/// </remarks>
internal static class Ppt
{
    /// <summary>The PresentationML name for a local name.</summary>
    public static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.PresentationML);

    /// <summary>An unprefixed attribute's value, or null.</summary>
    public static string? Attribute(XElement? element, string name)
        => element?.Attribute(name)?.Value;

    /// <summary>The first PresentationML child with this local name, or null.</summary>
    public static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    /// <summary>Every PresentationML child with this local name, in document order.</summary>
    public static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    /// <summary>True when an element is the named PresentationML element.</summary>
    public static bool Is(XElement? element, string localName)
        => element is not null
           && element.Name.NamespaceName == OoxmlNamespaces.PresentationML
           && element.Name.LocalName == localName;

    /// <summary>An <c>r:id</c> relationship reference.</summary>
    public static string? RelationshipId(XElement? element)
        => element?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

    /// <summary>
    /// Reads an ST_Boolean attribute, defaulting when it is absent.
    /// </summary>
    /// <remarks>
    /// The default matters: <c>&lt;p:sld show="0"&gt;</c> hides a slide, and the attribute's
    /// absence means shown. Treating absence as false would hide every slide in every deck.
    /// </remarks>
    public static bool Flag(XElement? element, string name, bool whenAbsent)
        => Attribute(element, name) switch
        {
            "1" or "true" or "on" => true,
            "0" or "false" or "off" => false,
            _ => whenAbsent,
        };
}
