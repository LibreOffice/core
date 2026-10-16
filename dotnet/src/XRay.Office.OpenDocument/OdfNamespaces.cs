namespace XRay.Office.OpenDocument;

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

    /// <summary>
    /// LibreOffice's chart extension namespace, spelled <c>chartooo</c>.
    /// </summary>
    /// <remarks>
    /// <strong>The exact plot rectangle is written in this namespace about twice as often as in
    /// the standard one.</strong> <c>coordinate-region</c> was a LibreOffice extension before it
    /// was standardised, and which spelling a file uses depends on the ODF version the writer was
    /// set to. Counted over the 71 charts in <c>chart2/qa/extras/data/</c>'s <c>.odp</c>,
    /// <c>.ods</c> and <c>.odt</c> documents that state one at all: <strong>24 are
    /// <c>chart:coordinate-region</c> and 47 are <c>chartooo:coordinate-region</c></strong>.
    /// Reading only the first left two ODF charts in three falling back to the OOXML layout
    /// heuristic while the file held the answer to the hundredth of a millimetre — and the corpus
    /// deck, which uses the standard spelling, hid it completely.
    /// </remarks>
    public const string ChartExtension = "http://openoffice.org/2010/chart";

    /// <summary>The <c>drawooo</c> namespace, LibreOffice's drawing extensions.</summary>
    /// <remarks>
    /// <strong><c>draw:display</c> is written here and read from both spellings</strong>, exactly
    /// as <see cref="ChartExtension"/>'s <c>coordinate-region</c> is. The exporter always emits
    /// the extension form — <c>XML_NAMESPACE_DRAW_EXT</c>,
    /// <c>xmloff/source/draw/shapeexport.cxx</c>:816 — and the importer accepts either
    /// (<c>ximpshap.cxx</c>:840-841). Counted over the 302 <c>.odp</c> of the converted corpus:
    /// <strong>887 occurrences in 49 documents, every one of them <c>drawooo:display</c></strong>
    /// and none in the standard namespace, so a reader that looks only at <c>draw:display</c>
    /// finds nothing at all and concludes the attribute is not used.
    /// </remarks>
    public const string DrawExtension = "http://openoffice.org/2010/draw";

    /// <summary>The <c>form</c> namespace, holding control definitions.</summary>
    public const string Form = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";

    /// <summary>The <c>script</c> namespace. X-Ray.Office reads macros as data and never runs them.</summary>
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

    /// <summary>
    /// True for the namespace of a table element, which is <em>either</em> <c>table:</c> or
    /// <c>loext:</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF puts a table in the <c>table:</c> namespace, and ODF 1.3 does not allow one everywhere
    /// LibreOffice can put one — inside a drawing shape's text, most of all. LibreOffice therefore
    /// writes such a table as <c>loext:table</c>, with <c>loext:table-row</c>,
    /// <c>loext:table-cell</c> and the rest beneath it, and reads the two spellings as one thing:
    /// <c>XMLTextImportHelper::CreateTextChildContext</c> falls the two element tokens through to the
    /// same <c>CreateTableChildContext</c> —
    /// <c>case XML_ELEMENT(TABLE, XML_TABLE): case XML_ELEMENT(LO_EXT, XML_TABLE):</c>,
    /// <c>xmloff/source/text/txtimp.cxx</c>:1787-1795.
    /// </para>
    /// <para>
    /// The <em>attributes</em> stay in <c>table:</c> — a <c>loext:table-cell</c> carries
    /// <c>table:style-name</c> and <c>table:number-columns-spanned</c> — so only the element names
    /// move, which is why this asks about a namespace rather than about a name.
    /// </para>
    /// <para>
    /// It is not a curiosity of the specification: <b>seven of the 338 converted <c>.odt</c> hold a
    /// <c>loext:table</c></b> and one of them draws 41 of its 1099 characters without it, because
    /// every table of a roadmap diagram is one.
    /// </para>
    /// </remarks>
    /// <param name="namespaceName">The element's namespace.</param>
    public static bool IsTable(string? namespaceName)
        => namespaceName is Table or LoExt;

    /// <summary>LibreOffice's Calc extension namespace, mostly duplicating value types.</summary>
    public const string CalcExt = "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0";

    /// <summary>
    /// The OpenOffice.org office extension namespace.
    /// </summary>
    /// <remarks>
    /// One element in it matters: a comment on a <em>slide</em>. ODF puts
    /// <c>office:annotation</c> inside a paragraph or a cell, and Impress needs one on a
    /// <c>draw:page</c>, so LibreOffice writes <c>officeooo:annotation</c> instead
    /// (<c>xmloff/source/draw/sdxmlexp.cxx:2647</c>) and accepts either on import
    /// (<c>xmloff/source/draw/ximppage.cxx:278</c>). Reading only the ODF-namespaced one puts
    /// every slide comment's text into the slide's own paragraphs, where it is
    /// indistinguishable from what the slide says.
    /// </remarks>
    public const string OfficeExt = "http://openoffice.org/2009/office";

    /// <summary>The <c>manifest</c> namespace, used by <c>META-INF/manifest.xml</c>.</summary>
    public const string Manifest = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
}
