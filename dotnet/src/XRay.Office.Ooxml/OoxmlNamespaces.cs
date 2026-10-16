namespace XRay.Office.Ooxml;

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

    /// <summary>
    /// Office MathML — the namespace a formula's own elements live in.
    /// </summary>
    /// <remarks>
    /// Worth a constant of its own because an OMML subtree sits <em>inside</em> a
    /// <see cref="WordprocessingML"/> paragraph and shares local names with it: <c>m:r</c> and
    /// <c>m:t</c> against <c>w:r</c> and <c>w:t</c>. A walk that switches on the local name alone
    /// therefore reads a formula as ordinary text, draws the concatenation of its leaves on the body
    /// baseline, and silently loses every mark a construct generates rather than states — a fraction's
    /// rule, a radical sign, an n-ary operator, a delimiter. Measured on
    /// <c>ABCD-FE-01-00 Flight Envelope</c>, which holds 54 <c>m:oMath</c> and 22 <c>m:oMathPara</c>.
    /// </remarks>
    public const string OfficeMath = "http://schemas.openxmlformats.org/officeDocument/2006/math";

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

    /// <summary>
    /// DrawingML charts: the vocabulary of <c>/ppt/charts/chartN.xml</c> and
    /// <c>/xl/charts/chartN.xml</c>.
    /// </summary>
    /// <remarks>
    /// The same string is also the <c>a:graphicData/@uri</c> that identifies a chart inside a
    /// graphic frame, which is why <see cref="DrawingML.DrawingChart.ChartUri"/> is defined as
    /// this constant rather than repeated.
    /// </remarks>
    public const string DrawingMLChart = "http://schemas.openxmlformats.org/drawingml/2006/chart";

    /// <summary>DrawingML for the shapes drawn <em>on</em> a chart — <c>cdr:</c>.</summary>
    /// <remarks>
    /// The <c>chartUserShapes</c> part's vocabulary. It is a third drawing dialect beside
    /// <see cref="DrawingMLSpreadsheet"/> and <see cref="DrawingMLWordprocessing"/>, with the same
    /// shape: three anchor and wrapper elements of its own around ordinary
    /// <see cref="DrawingML"/> content.
    /// </remarks>
    public const string ChartDrawing =
        "http://schemas.openxmlformats.org/drawingml/2006/chartDrawing";

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

    /// <summary>
    /// Word 2010 drawing canvases — a group of shapes with its own background, written as
    /// <c>wpc:wpc</c> inside a <c>a:graphicData</c>.
    /// </summary>
    public const string WordCanvas = "http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas";

    /// <summary>Word 2010 drawing extensions.</summary>
    public const string WordDrawing2010 = "http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing";

    /// <summary>Word 2010 markup extensions.</summary>
    public const string WordMl2010 = "http://schemas.microsoft.com/office/word/2010/wordml";

    /// <summary>Word 2012 markup extensions.</summary>
    public const string WordMl2012 = "http://schemas.microsoft.com/office/word/2012/wordml";

    /// <summary>
    /// DrawingML 2010 extensions — <c>a14</c>, and deliberately <strong>not</strong> in
    /// <see cref="UnderstoodExtensions"/>.
    /// </summary>
    /// <remarks>
    /// See the note on <see cref="UnderstoodExtensions"/>. The constant is still needed, because
    /// the tests that pin the rule have to bind the prefix to something.
    /// </remarks>
    public const string DrawingML2010 = "http://schemas.microsoft.com/office/drawing/2010/main";

    /// <summary>
    /// The <c>a:graphicData/@uri</c> of an extended ("chartex") chart — the family Excel 2016
    /// added for Pareto, histogram, waterfall, treemap, sunburst, box-and-whisker and funnel.
    /// </summary>
    /// <remarks>
    /// It is named here because it decides an <c>mc:AlternateContent</c> branch rather than
    /// because anything reads the part yet. See
    /// <see cref="OoxmlXml"/>'s note on preferring a choice whose content we cannot draw.
    /// </remarks>
    public const string ExtendedChart = "http://schemas.microsoft.com/office/drawing/2014/chartex";

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
            ["http://purl.oclc.org/ooxml/drawingml/chart"] = DrawingMLChart,
            ["http://purl.oclc.org/ooxml/drawingml/spreadsheetDrawing"] = DrawingMLSpreadsheet,
            ["http://purl.oclc.org/ooxml/drawingml/wordprocessingDrawing"] = DrawingMLWordprocessing,
            ["http://purl.oclc.org/ooxml/officeDocument/relationships"] = Relationships,
        };

    /// <summary>
    /// The extension namespaces X-Ray.Office understands well enough to prefer an
    /// <c>mc:Choice</c> that requires them over the <c>mc:Fallback</c> beside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Preferring a choice is only right when its content can actually be read: the fallback
    /// exists precisely because the choice may be unreadable. For the shape namespaces the
    /// choice is the higher-fidelity branch and its text body is plain WordprocessingML, so it
    /// is preferred; anything not listed here loses to the fallback.
    /// </para>
    /// <para>
    /// <c>wpc</c> is listed for a reason worth stating: the word-processing frame reader has read a
    /// drawing canvas since it was written, and could never be reached, because the canvas is always
    /// offered as a <c>Requires="wpc"</c> choice beside a VML fallback. Leaving it off dropped every
    /// shape in the canvas — measured on an EASA manual whose two organisation diagrams lost their
    /// text and, with it, 2.4 inches of declared height apiece.
    /// </para>
    /// <para>
    /// <strong><c>a14</c> is not listed, and that is the whole of the rule the slicer special case
    /// used to approximate.</strong> LibreOffice keeps this same list twice —
    /// <c>ContextHandler2Helper::prepareMceContext</c> for the <c>oox</c> filters (xlsx, pptx) and
    /// <c>OOXMLFastContextHandler::prepareMceContext</c> for writerfilter — and
    /// <see cref="DrawingML2010"/> is on neither. In <c>oox</c> it is present and commented out,
    /// with the reason attached: <c>// We do not currently support inline formulas and other a14
    /// stuff</c>. So all three filters take the fallback beside an <c>a14</c> choice, and this used
    /// to take the choice.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 rather than read off the source. Unwrapping the
    /// <c>mc:AlternateContent</c> around <c>013_Contextures_chart_sample</c>'s camera picture — one
    /// edit, nothing else changed — makes the reference draw that picture **twice**, at 129.5 from
    /// the DrawingML anchor it can now see and at 133.8 from the legacy VML shape it was already
    /// drawing, 41 extractable words against 23. With the wrapper in place it draws the VML one
    /// only. See <c>probes/sheets-r55/probe-vml-camera.py</c>.
    /// </para>
    /// <para>
    /// Censused over all 946 corpus documents (<c>probes/sheets-r55/census-a14.py</c>, keyed on the
    /// resolved URI of each <c>Requires</c> prefix rather than on the prefix text): 2324 choices, of
    /// which <strong>34 resolve to <c>a14</c>, in 10 documents</strong> — seven slicer choices in
    /// three spreadsheets that already lost to the fallback by the removed special case, five
    /// spreadsheet anchors beside an <em>empty</em> fallback, and 22 <c>a14:m</c> inline-formula
    /// choices in three decks, which is the case <c>oox</c>'s comment names. No word-processing
    /// document in the corpus states one.
    /// </para>
    /// </remarks>
    public static readonly IReadOnlySet<string> UnderstoodExtensions =
        new HashSet<string>(StringComparer.Ordinal)
        {
            WordShape, WordShapeGroup, WordCanvas, WordDrawing2010, WordMl2010, WordMl2012,
        };
}
