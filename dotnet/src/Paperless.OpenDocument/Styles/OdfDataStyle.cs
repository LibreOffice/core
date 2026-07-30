using System.Xml.Linq;

namespace Paperless.OpenDocument.Styles;

/// <summary>What kind of value a <c>number:</c> data style formats.</summary>
public enum OdfDataStyleKind
{
    /// <summary>Not a data style Paperless recognises.</summary>
    Unknown = 0,

    /// <summary><c>number:number-style</c>.</summary>
    Number,

    /// <summary><c>number:percentage-style</c>.</summary>
    Percentage,

    /// <summary><c>number:currency-style</c>.</summary>
    Currency,

    /// <summary><c>number:date-style</c>.</summary>
    Date,

    /// <summary><c>number:time-style</c>.</summary>
    Time,

    /// <summary><c>number:boolean-style</c>.</summary>
    Boolean,

    /// <summary><c>number:text-style</c>.</summary>
    Text,
}

/// <summary>
/// A <c>number:*-style</c> data style: the number format a cell's value is displayed
/// through.
/// </summary>
/// <remarks>
/// <para>
/// Extraction does not need to apply these. An ODF cell carries its formatted appearance
/// as <c>text:p</c> children alongside the raw <c>office:value</c>, so the displayed text
/// is already in the file and re-deriving it would risk disagreeing with what the
/// authoring application actually showed.
/// </para>
/// <para>
/// They are parsed and kept because rendering will need them for cells whose cached text
/// is absent or stale, and because a caller may want to know the format a value carries.
/// The format's pieces are left as XML for now rather than compiled into a formatter —
/// that belongs with the spreadsheet work, not here.
/// </para>
/// </remarks>
public sealed class OdfDataStyle
{
    internal OdfDataStyle(XElement element)
    {
        Name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value ?? string.Empty;
        Kind = element.Name.LocalName switch
        {
            "number-style" => OdfDataStyleKind.Number,
            "percentage-style" => OdfDataStyleKind.Percentage,
            "currency-style" => OdfDataStyleKind.Currency,
            "date-style" => OdfDataStyleKind.Date,
            "time-style" => OdfDataStyleKind.Time,
            "boolean-style" => OdfDataStyleKind.Boolean,
            "text-style" => OdfDataStyleKind.Text,
            _ => OdfDataStyleKind.Unknown,
        };
        LanguageCode = element.Attribute(XName.Get("language", OdfNamespaces.Number))?.Value;
        CountryCode = element.Attribute(XName.Get("country", OdfNamespaces.Number))?.Value;
        Element = element;
    }

    /// <summary>The style's name, as referenced by <c>style:data-style-name</c>.</summary>
    public string Name { get; }

    /// <summary>What kind of value the style formats.</summary>
    public OdfDataStyleKind Kind { get; }

    /// <summary>The format's language, which decides month and day names.</summary>
    public string? LanguageCode { get; }

    /// <summary>The format's country, which decides separators and currency placement.</summary>
    public string? CountryCode { get; }

    /// <summary>The format's definition, kept verbatim until a formatter exists.</summary>
    public XElement Element { get; }
}

/// <summary>
/// A <c>style:font-face</c> declaration: the mapping from the font name styles reference
/// to the family name a font system can resolve.
/// </summary>
/// <remarks>
/// Worth keeping even during extraction. <c>style:font-name</c> refers to one of these by
/// name, and the declaration is the only place the actual family, generic class and pitch
/// are recorded — all three of which font substitution will need, and a missing
/// substitution is the usual explanation for a mysterious reflow.
/// </remarks>
public sealed class OdfFontFace
{
    internal OdfFontFace(XElement element)
    {
        Name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value ?? string.Empty;
        FontFamily = element.Attribute(XName.Get("font-family", OdfNamespaces.SvgCompatible))?.Value;
        GenericFamily = element.Attribute(XName.Get("font-family-generic", OdfNamespaces.Style))?.Value;
        Pitch = element.Attribute(XName.Get("font-pitch", OdfNamespaces.Style))?.Value;
    }

    /// <summary>The name <c>style:font-name</c> refers to.</summary>
    public string Name { get; }

    /// <summary>
    /// The CSS-style family list, quotes included as written
    /// (<c>'Liberation Serif'</c>).
    /// </summary>
    public string? FontFamily { get; }

    /// <summary>The generic class: <c>roman</c>, <c>swiss</c>, <c>modern</c>, <c>system</c>.</summary>
    public string? GenericFamily { get; }

    /// <summary><c>fixed</c> or <c>variable</c>.</summary>
    public string? Pitch { get; }
}
