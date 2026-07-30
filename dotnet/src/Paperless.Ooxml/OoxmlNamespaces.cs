namespace Paperless.Ooxml;

/// <summary>
/// The XML namespace URIs used across OOXML.
/// </summary>
/// <remarks>
/// Two generations of these exist. ECMA-376 1st edition (what Office 2007 shipped)
/// and the later ISO/IEC 29500 "strict" variant use different URIs for the same
/// elements, and real files in the wild use both — sometimes mixed within one package.
/// Readers must therefore accept either, which is why both are listed here rather than
/// just the transitional set.
/// </remarks>
public static class OoxmlNamespaces
{
    /// <summary>WordprocessingML, transitional.</summary>
    public const string WordprocessingML = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>SpreadsheetML, transitional.</summary>
    public const string SpreadsheetML = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>PresentationML, transitional.</summary>
    public const string PresentationML = "http://schemas.openxmlformats.org/presentationml/2006/main";

    /// <summary>DrawingML main.</summary>
    public const string DrawingML = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>DrawingML spreadsheet anchoring.</summary>
    public const string DrawingMLSpreadsheet = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";

    /// <summary>DrawingML word-processing anchoring.</summary>
    public const string DrawingMLWordprocessing = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";

    /// <summary>OPC relationships.</summary>
    public const string Relationships = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>The VML fallback namespace, still needed for older files and for comments.</summary>
    public const string Vml = "urn:schemas-microsoft-com:vml";

    /// <summary>Markup Compatibility and Extensibility: <c>mc:AlternateContent</c> and friends.</summary>
    public const string MarkupCompatibility = "http://schemas.openxmlformats.org/markup-compatibility/2006";
}
