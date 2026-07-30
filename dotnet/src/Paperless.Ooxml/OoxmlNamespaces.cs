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

    /// <summary>The <c>o:</c> namespace, VML's Office extensions.</summary>
    public const string VmlOffice = "urn:schemas-microsoft-com:office:office";

    /// <summary>The <c>w10:</c> namespace, VML's Word extensions.</summary>
    public const string VmlWord = "urn:schemas-microsoft-com:office:word";

    /// <summary>Word 2010 shapes: what a text box is written as in a current file.</summary>
    public const string WordShape = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";

    /// <summary>Word 2010 shape groups.</summary>
    public const string WordShapeGroup = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";

    /// <summary>Word 2010 drawing extensions.</summary>
    public const string WordDrawing2010 = "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing";

    /// <summary>Word 2010 markup extensions.</summary>
    public const string WordMl2010 = "http://schemas.microsoft.com/office/word/2010/wordml";

    /// <summary>Word 2012 markup extensions.</summary>
    public const string WordMl2012 = "http://schemas.microsoft.com/office/word/2012/wordml";

    /// <summary>DrawingML 2010 extensions.</summary>
    public const string DrawingML2010 = "http://schemas.microsoft.com/office/drawing/2010/main";

    /// <summary>
    /// The ISO/IEC 29500 strict URIs, paired with the transitional URI each replaces.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Strict and transitional name the same elements with different namespaces, and real
    /// packages use both — occasionally mixed within one package, since a producer may write a
    /// strict main part and a transitional theme. Every reader therefore has to accept either.
    /// </para>
    /// <para>
    /// Rather than checking two URIs at every comparison — which is the version of this that
    /// gets forgotten in one place and produces a silently empty document — strict names are
    /// rewritten to their transitional equivalents once, when the part is loaded. See
    /// <see cref="OoxmlXml.Normalise"/>.
    /// </para>
    /// </remarks>
    public static readonly IReadOnlyDictionary<string, string> StrictToTransitional =
        new Dictionary<string, string>(StringComparer.Ordinal)
        {
            ["http://purl.oclc.org/ooxml/wordprocessingml/main"] = WordprocessingML,
            ["http://purl.oclc.org/ooxml/spreadsheetml/main"] = SpreadsheetML,
            ["http://purl.oclc.org/ooxml/presentationml/main"] = PresentationML,
            ["http://purl.oclc.org/ooxml/drawingml/main"] = DrawingML,
            ["http://purl.oclc.org/ooxml/drawingml/spreadsheetDrawing"] = DrawingMLSpreadsheet,
            ["http://purl.oclc.org/ooxml/drawingml/wordprocessingDrawing"] = DrawingMLWordprocessing,
            ["http://purl.oclc.org/ooxml/officeDocument/relationships"] = Relationships,
        };

    /// <summary>
    /// The extension namespaces Paperless understands well enough to prefer an
    /// <c>mc:Choice</c> that requires them over the <c>mc:Fallback</c> beside it.
    /// </summary>
    /// <remarks>
    /// Preferring a choice is only right when its content can actually be read: the fallback
    /// exists precisely because the choice may be unreadable. For the shape namespaces the
    /// choice is the higher-fidelity branch and its text body is plain WordprocessingML, so it
    /// is preferred; anything not listed here loses to the fallback.
    /// </remarks>
    public static readonly IReadOnlySet<string> UnderstoodExtensions =
        new HashSet<string>(StringComparer.Ordinal)
        {
            WordShape, WordShapeGroup, WordDrawing2010, WordMl2010, WordMl2012, DrawingML2010,
        };
}
