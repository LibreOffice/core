using System.Globalization;
using System.Xml.Linq;
using Paperless.Ooxml;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Shorthand for reaching into SpreadsheetML.
/// </summary>
/// <remarks>
/// Unlike WordprocessingML, SpreadsheetML puts its elements in a default-declared namespace and
/// its attributes in <em>none</em> — <c>&lt;c r="A1" t="s"&gt;</c> — so element lookups are
/// namespace-qualified and attribute lookups must not be. Qualifying an attribute here is the
/// mistake that silently reads every cell as untyped and unaddressed.
/// </remarks>
internal static class Xlsx
{
    /// <summary>The SpreadsheetML name for a local name.</summary>
    public static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.SpreadsheetML);

    /// <summary>An unqualified attribute's value, or null.</summary>
    public static string? Attribute(XElement? element, string localName)
        => element?.Attribute(localName)?.Value;

    /// <summary>The first SpreadsheetML child with this local name, or null.</summary>
    public static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    /// <summary>Every SpreadsheetML child with this local name, in document order.</summary>
    public static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    /// <summary>True when an element is the named SpreadsheetML element.</summary>
    public static bool Is(XElement element, string localName)
    {
        ArgumentNullException.ThrowIfNull(element);
        return element.Name.NamespaceName == OoxmlNamespaces.SpreadsheetML
               && element.Name.LocalName == localName;
    }

    /// <summary>An <c>r:id</c> relationship reference.</summary>
    public static string? RelationshipId(XElement? element)
        => element?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

    /// <summary>Parses an unqualified integer attribute.</summary>
    public static int? Integer(XElement? element, string localName)
        => int.TryParse(Attribute(element, localName), NumberStyles.Integer,
                        CultureInfo.InvariantCulture, out int value)
            ? value
            : null;

    /// <summary>
    /// Parses a double the way SpreadsheetML writes one: invariant, no grouping.
    /// </summary>
    public static double? Double(string? text)
        => double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : null;

    /// <summary>
    /// True when an unqualified boolean attribute means true.
    /// </summary>
    /// <remarks>
    /// SpreadsheetML writes <c>1</c>/<c>0</c>, but producers write <c>true</c>/<c>false</c>
    /// freely and both appear in files LibreOffice itself has written.
    /// </remarks>
    public static bool Flag(XElement? element, string localName, bool fallback = false)
        => Attribute(element, localName) switch
        {
            null => fallback,
            "1" or "true" or "on" or "True" or "TRUE" => true,
            "0" or "false" or "off" or "False" or "FALSE" => false,
            _ => fallback,
        };

    /// <summary>
    /// Splits an A1-style cell reference into zero-based column and row.
    /// </summary>
    /// <returns>False when the reference is not one, which real files do contain.</returns>
    public static bool TryParseCellReference(string? reference, out int column, out int row)
    {
        column = 0;
        row = 0;
        if (string.IsNullOrEmpty(reference)) return false;

        int i = 0;
        // A reference may carry absolute markers; they do not change which cell it names.
        if (reference[i] == '$') i++;

        int columnValue = 0;
        int letters = 0;
        while (i < reference.Length && char.IsAsciiLetter(reference[i]))
        {
            columnValue = (columnValue * 26) + (char.ToUpperInvariant(reference[i]) - 'A' + 1);
            letters++;
            i++;
            if (letters > 3) return false;
        }
        if (letters == 0) return false;

        if (i < reference.Length && reference[i] == '$') i++;

        int rowValue = 0;
        int digits = 0;
        while (i < reference.Length && char.IsAsciiDigit(reference[i]))
        {
            rowValue = (rowValue * 10) + (reference[i] - '0');
            digits++;
            i++;
            if (digits > 7) return false;
        }
        if (digits == 0 || i != reference.Length || rowValue == 0) return false;

        column = columnValue - 1;
        row = rowValue - 1;
        return true;
    }

    /// <summary>Splits an A1:B2 range into its two corners.</summary>
    public static bool TryParseRange(
        string? reference, out int firstColumn, out int firstRow, out int lastColumn, out int lastRow)
    {
        firstColumn = firstRow = lastColumn = lastRow = 0;
        if (string.IsNullOrEmpty(reference)) return false;

        int colon = reference.IndexOf(':', StringComparison.Ordinal);
        if (colon < 0)
        {
            if (!TryParseCellReference(reference, out firstColumn, out firstRow)) return false;
            lastColumn = firstColumn;
            lastRow = firstRow;
            return true;
        }

        return TryParseCellReference(reference[..colon], out firstColumn, out firstRow)
               && TryParseCellReference(reference[(colon + 1)..], out lastColumn, out lastRow);
    }
}
