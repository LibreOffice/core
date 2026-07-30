namespace Paperless.OpenDocument;

/// <summary>The XML namespace URIs used across OpenDocument Format.</summary>
public static class OdfNamespaces
{
    /// <summary>The <c>office</c> namespace.</summary>
    public const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";

    /// <summary>The <c>style</c> namespace.</summary>
    public const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";

    /// <summary>The <c>text</c> namespace.</summary>
    public const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

    /// <summary>The <c>table</c> namespace.</summary>
    public const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";

    /// <summary>The <c>draw</c> namespace.</summary>
    public const string Draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";

    /// <summary>The <c>presentation</c> namespace.</summary>
    public const string Presentation = "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0";

    /// <summary>The <c>fo</c> namespace, holding most formatting properties.</summary>
    public const string FoCompatible = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";

    /// <summary>The <c>svg</c> namespace, holding positions and sizes.</summary>
    public const string SvgCompatible = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";

    /// <summary>The <c>meta</c> namespace.</summary>
    public const string Meta = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";

    /// <summary>Dublin Core, used for most metadata fields.</summary>
    public const string DublinCore = "http://purl.org/dc/elements/1.1/";

    /// <summary>The <c>number</c> namespace, holding number format definitions.</summary>
    public const string Number = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
}
