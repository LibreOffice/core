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

    /// <summary>XLink, which carries every href in an ODF document.</summary>
    public const string XLink = "http://www.w3.org/1999/xlink";

    /// <summary>The <c>config</c> namespace, used by <c>settings.xml</c>.</summary>
    public const string Config = "urn:oasis:names:tc:opendocument:xmlns:config:1.0";

    /// <summary>The <c>dr3d</c> namespace, holding 3-D scene objects.</summary>
    public const string Dr3d = "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0";

    /// <summary>The <c>chart</c> namespace.</summary>
    public const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";

    /// <summary>The <c>form</c> namespace, holding control definitions.</summary>
    public const string Form = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";

    /// <summary>The <c>script</c> namespace. Paperless reads macros as data and never runs them.</summary>
    public const string Script = "urn:oasis:names:tc:opendocument:xmlns:script:1.0";

    /// <summary>
    /// The <c>of</c> namespace, which prefixes OpenFormula formulas
    /// (<c>table:formula="of:=[.A1]"</c>).
    /// </summary>
    public const string OpenFormula = "urn:oasis:names:tc:opendocument:xmlns:of:1.2";

    /// <summary>
    /// LibreOffice's extension namespace. Real ODF files written by LibreOffice put a
    /// good deal of formatting here, so ignoring it loses information that is present.
    /// </summary>
    public const string LoExt = "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";

    /// <summary>LibreOffice's Calc extension namespace, mostly duplicating value types.</summary>
    public const string CalcExt = "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0";

    /// <summary>The <c>manifest</c> namespace, used by <c>META-INF/manifest.xml</c>.</summary>
    public const string Manifest = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
}
