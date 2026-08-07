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

    /// <summary>
    /// An integer attribute, read the way the reference reads one: leading sign, digits, and
    /// everything from the first character that is not a digit ignored.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The schema says these are integers and real files write <c>w:w="8730.0"</c> anyway — a
    /// whole document of them, produced by something that is not Word. LibreOffice takes them:
    /// its attribute list hands the string to <c>rtl_ustr_toInt32</c>, which parses as far as it
    /// can and stops, so <c>8730.0</c> is 8730. A reader using <c>int.TryParse</c> instead gets
    /// nothing at all, and the property falls back to its default — which for a cell margin means
    /// no margin, for a column no width, and for a table indent no indent.
    /// </para>
    /// <para>
    /// It truncates rather than rounds, which is worth stating because rounding is the plausible
    /// alternative and it is wrong. Measured on three otherwise identical documents whose
    /// paragraph states <c>w:before</c> of <c>240</c>, <c>240.9</c> and <c>241</c>: LibreOffice
    /// puts the second paragraph's first word at 96.996, 96.996 and 97.046 pt — the decimal one
    /// landing exactly on 240 and not on 241.
    /// </para>
    /// </remarks>
    /// <param name="text">The attribute's value, or null.</param>
    /// <param name="value">The number read, or zero.</param>
    /// <returns>True when at least one digit was read.</returns>
    public static bool Integer(string? text, out int value)
    {
        value = 0;
        if (string.IsNullOrEmpty(text)) return false;

        int index = 0;
        bool negative = false;

        if (text[0] is '-' or '+')
        {
            negative = text[0] == '-';
            index = 1;
        }

        long magnitude = 0;
        int digits = 0;

        for (; index < text.Length && char.IsAsciiDigit(text[index]); index++, digits++)
        {
            // Saturating, so a number too long to hold does not wrap into a small one. The
            // reference's own conversion saturates too.
            if (magnitude <= int.MaxValue) magnitude = (magnitude * 10) + (text[index] - '0');
        }

        if (digits == 0) return false;

        magnitude = Math.Min(magnitude, negative ? -(long)int.MinValue : int.MaxValue);
        value = (int)(negative ? -magnitude : magnitude);
        return true;
    }

    /// <summary>
    /// The same, widened — for the measures that are counted in twips and can legitimately be
    /// larger than a page.
    /// </summary>
    /// <param name="text">The attribute's value, or null.</param>
    /// <param name="value">The number read, or zero.</param>
    /// <returns>True when at least one digit was read.</returns>
    public static bool Long(string? text, out long value)
    {
        bool read = Integer(text, out int narrow);
        value = narrow;
        return read;
    }

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
