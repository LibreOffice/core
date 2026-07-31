using System.Xml.Linq;
using Paperless.Ooxml;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Shorthand for reaching into WordprocessingML, where nearly every name is
/// <c>w:</c>-namespaced.
/// </summary>
/// <remarks>
/// Matching is on namespace and local name, never on prefix: a document may bind the
/// WordprocessingML namespace to any prefix it likes, and files that use something other than
/// <c>w</c> do exist. Parts are normalised to the transitional namespace when loaded (see
/// <c>OoxmlXml.Normalise</c>), so one namespace is enough here.
/// </remarks>
internal static class Word
{
    /// <summary>The WordprocessingML name for a local name.</summary>
    public static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.WordprocessingML);

    /// <summary>A <c>w:</c>-namespaced attribute's value, or null.</summary>
    public static string? Attribute(XElement? element, string localName)
        => element?.Attribute(Name(localName))?.Value;

    /// <summary>The first <c>w:</c>-namespaced child with this local name, or null.</summary>
    public static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    /// <summary>Every <c>w:</c>-namespaced child with this local name, in document order.</summary>
    public static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    /// <summary>
    /// The <c>w:val</c> of a child element: the shape almost every WordprocessingML property
    /// takes, as in <c>&lt;w:pStyle w:val="Heading1"/&gt;</c>.
    /// </summary>
    public static string? Value(XElement? element, string localName)
        => Attribute(Child(element, localName), "val");

    /// <summary>True when an element is in the WordprocessingML namespace.</summary>
    public static bool Is(XElement element, string localName)
    {
        ArgumentNullException.ThrowIfNull(element);
        return element.Name.NamespaceName == OoxmlNamespaces.WordprocessingML
               && element.Name.LocalName == localName;
    }

    /// <summary>
    /// An <c>r:id</c> relationship reference, which is how a hyperlink, header, footer or
    /// image names its target.
    /// </summary>
    public static string? RelationshipId(XElement? element)
        => element?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

    /// <summary>
    /// True when a <c>w:val</c>-style on/off attribute means on.
    /// </summary>
    /// <remarks>
    /// An absent attribute means on, because presence of the element is the signal. All three
    /// spellings of off appear in real files.
    /// </remarks>
    public static bool IsOn(XElement? element)
        => element is not null && Attribute(element, "val") switch
        {
            null or "" => true,
            "0" or "false" or "off" => false,
            _ => true,
        };
}
