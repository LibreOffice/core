using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Charts;
using Paperless.Core.Diagnostics;
using Paperless.Core.Graphics;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Resolves the parts a <c>w:drawing</c> points at: an <c>a:blip</c>'s <c>r:embed</c> into picture
/// bytes, and a <c>c:chart</c>'s <c>r:id</c> into a chart.
/// </summary>
/// <remarks>
/// <para>
/// Two indirections and both matter. A DrawingML picture never names a file: it names a
/// <em>relationship</em>, and the relationship names a part. LibreOffice's importer does the same walk —
/// <c>GraphicImport::lcl_attribute</c> reads <c>NS_ooxml::LN_blip</c> and hands the id to
/// <c>m_xComponentContext</c>'s relation lookup (<c>writerfilter/source/dmapper/GraphicImport.cxx</c>) —
/// and the reason it cannot be short-circuited is that the part name is a producer's free choice.
/// <c>/word/media/image1.png</c> is a convention that Word happens to follow and nothing enforces.
/// </para>
/// <para>
/// <strong>Relationship ids are scoped to the part that uses them.</strong> This is the trap, and it is
/// silent: <c>rId1</c> in <c>document.xml</c> and <c>rId1</c> in <c>header1.xml</c> are different
/// relationships in different <c>.rels</c> files, and Word numbers both from one. So a resolver that
/// looked only at the main document's relationships would answer a header's logo with whatever
/// <c>document.xml</c> happens to call <c>rId1</c> — usually <c>styles.xml</c>, which sniffs as no
/// picture at all, so the frame silently stays empty and nothing says why. <see cref="Scope"/> is what
/// the reader sets as it moves between parts.
/// </para>
/// <para>
/// One <see cref="FramePicture"/> per part, cached: a logo in a running head is one picture drawn on
/// every page, and the PDF writer deduplicates its image XObjects by object identity — so returning a
/// fresh instance per use would write the same bytes into the file once per page. The same cache is what
/// makes a vector logo decode once however many pages carry it, since the deferred decode it holds
/// caches its own answer.
/// </para>
/// </remarks>
public sealed class DocxPictures
{
    private readonly DocxFile _file;
    private readonly List<Diagnostic>? _diagnostics;
    private readonly Dictionary<string, FramePicture> _byPart = new(StringComparer.Ordinal);
    private readonly Dictionary<string, DocxChart> _chartsByPart = new(StringComparer.Ordinal);

    /// <summary>Creates a resolver over an open package.</summary>
    /// <param name="file">The package, for its parts and their relationships.</param>
    /// <param name="diagnostics">Where to record a picture that will not draw, or null to say nothing.</param>
    public DocxPictures(DocxFile file, List<Diagnostic>? diagnostics)
    {
        ArgumentNullException.ThrowIfNull(file);
        _file = file;
        _diagnostics = diagnostics;
        Scope = file.MainPartName;
    }

    /// <summary>
    /// The part whose relationships an <c>r:embed</c> is looked up in.
    /// </summary>
    /// <remarks>
    /// The main document to begin with, which is where the body's pictures are. A reader walking a
    /// header, a footer or a note part sets this to that part's name for the duration and puts it back
    /// afterwards; see the remarks on the class for what happens when it does not.
    /// </remarks>
    public string Scope { get; set; }

    /// <summary>
    /// The picture a <c>w:drawing</c> or a legacy <c>w:pict</c> holds, or nothing when it holds none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The first <c>a:blip</c> anywhere beneath the element, because the element it hangs from differs by
    /// what the picture is for: <c>pic:blipFill</c> for an ordinary picture, <c>a:blipFill</c> for a
    /// shape filled with one, and <c>wps:spPr</c> for either inside a text box. All three mean the same
    /// thing to a reader that wants the bytes.
    /// </para>
    /// <para>
    /// <strong>Which of a blip's renderings to take is <c>BlipReference.Choose</c>'s</strong> rather
    /// than <c>r:embed</c> read directly, because since Office 2016 one <c>a:blip</c> can carry two: an
    /// <c>asvg:svgBlip</c> in an extension beside the raster in <c>r:embed</c>. The vector is the one to
    /// draw — it is exact at any size where the raster is written once at one — and the raster is
    /// carried alongside so that a decode which comes back empty still draws something. Measured on
    /// <c>svg-picture.docx</c>: 769 bytes of SVG beside a 3 803-byte PNG.
    /// </para>
    /// </remarks>
    public FramePicture Read(XElement drawing)
    {
        ArgumentNullException.ThrowIfNull(drawing);

        XElement? blip = drawing
            .DescendantsAndSelf()
            .FirstOrDefault(element => element.Name.LocalName == "blip");

        if (blip is null) return FramePicture.None;

        BlipReference.Choice choice = BlipReference.Choose(blip);

        if (choice.RelationshipId is { } chosen)
        {
            FramePicture picture = Embedded(chosen);

            if (!choice.IsVector) return picture;
            if (choice.FallbackRelationshipId is not { } fallback) return picture;

            // The vector was preferred. If nothing here can decode it, the raster is what the file put
            // beside it for exactly this; if something can, keep the raster anyway so an empty decode
            // still leaves a picture on the page.
            return picture.Vector is null
                ? Embedded(fallback)
                : picture with { Raster = Embedded(fallback).Raster };
        }

        // r:link is a picture stored outside the package. Not fetched: reading a document must not
        // reach the file system beside it or the network, and LibreOffice's own answer for a link it
        // cannot resolve is the same empty frame.
        if (blip.Attribute(XName.Get("link", OoxmlNamespaces.Relationships)) is not null)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2374",
                "A picture is linked rather than embedded, so its bytes are not in the document and "
                + "the frame stays empty."));
        }

        return FramePicture.None;
    }

    /// <summary>
    /// The chart a <c>w:drawing</c> holds, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Resolved here rather than in a reader of its own because the two indirections are the same ones a
    /// picture takes and the second of them is the trap: <c>c:chart/@r:id</c> is scoped to the part the
    /// drawing sits in, so a chart in a header resolves against <c>header1.xml</c>'s relationships and
    /// not <c>document.xml</c>'s. That is exactly what <see cref="Scope"/> already tracks. It is also
    /// why a spreadsheet's version of this resolves against the <em>drawing</em> part: a Writer drawing
    /// is inline in the story, and a Calc one is not.
    /// </para>
    /// <para>
    /// <strong>The theme comes from the document and a themed chart draws nothing without it.</strong> A
    /// chart part may state <c>a:schemeClr val="accent1"</c> rather than an <c>a:srgbClr</c>, and
    /// resolving one needs <c>word/theme/theme1.xml</c> — which <see cref="DocxFile.Theme"/> has already
    /// read for the run colours. Passing null instead leaves every series with no fill and draws a plot
    /// area with its axes and not one mark on it, and every chart LibreOffice's own export writes states
    /// <c>a:srgbClr</c>, which is how a corpus of round-tripped files hides it.
    /// </para>
    /// <para>
    /// Cached per part beside the pictures, because <c>testMultiplechartembeddings.docx</c> is the shape
    /// this exists for: several frames, and nothing stops two of them naming one chart part.
    /// </para>
    /// </remarks>
    public DocxChart Chart(XElement drawing)
    {
        ArgumentNullException.ThrowIfNull(drawing);

        XElement? data = drawing
            .Descendants(XName.Get("graphicData", OoxmlNamespaces.DrawingML))
            .FirstOrDefault();

        if (data is null) return default;
        if (data.Attribute("uri")?.Value != DrawingChart.ChartUri) return default;

        string? relationshipId = data
            .Element(XName.Get("chart", OoxmlNamespaces.DrawingMLChart))
            ?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

        if (relationshipId is null) return default;
        if (_file.Package is not OpcPackage package) return default;

        OpcXml.Relationship? found = null;
        foreach (OpcXml.Relationship relationship in package.GetRelationships(Scope))
        {
            if (!string.Equals(relationship.Id, relationshipId, StringComparison.Ordinal)) continue;

            found = relationship;
            break;
        }

        if (found is not { IsExternal: false } target) return default;
        if (_chartsByPart.TryGetValue(target.Target, out DocxChart cached)) return cached;

        DocxChart chart = LoadChart(package, target.Target);
        _chartsByPart[target.Target] = chart;
        return chart;
    }

    /// <summary>The chart a package part holds, or nothing when the part is missing or unreadable.</summary>
    private DocxChart LoadChart(OpcPackage package, string partName)
    {
        if (package.GetPart(partName) is not { } part)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2376",
                $"A chart names the package part '{partName}', which the package does not hold, "
                + "so the frame stays empty.",
                new DiagnosticLocation(partName)));
            return default;
        }

        XElement? chartSpace;
        using (Stream content = part.Open()) chartSpace = OoxmlXml.TryLoad(content, out _);
        if (chartSpace is null) return default;

        return new DocxChart(
            DrawingChartPlot.Read(chartSpace, _file.Theme, _file.IsOffice2007),
            LabelFamily(chartSpace));
    }

    /// <summary>
    /// The family a chart part's text is set in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The first literal <c>a:latin/@typeface</c> anywhere in the part, then the theme's minor latin
    /// face, then Calibri. Anything beginning with a plus — <c>+mn-lt</c>, <c>+mj-lt</c> — is a
    /// <em>reference</em> to the theme rather than a name, so taking it as one asks the resolver for a
    /// family no system has and every label is measured in a fallback.
    /// </para>
    /// <para>
    /// Counted over <c>chart2/qa/extras/data/docx/</c>'s 69 chart parts: 36 name no face at all, 22
    /// name <c>+mn-lt</c>, and 11 state a real one — six Arial, two Calibri, two Times New Roman. So
    /// both halves of this are exercised by the corpus, and neither alone covers a sixth of it.
    /// </para>
    /// </remarks>
    private string? LabelFamily(XElement chartSpace)
    {
        foreach (XElement latin in chartSpace.Descendants(XName.Get("latin", OoxmlNamespaces.DrawingML)))
        {
            string? typeface = latin.Attribute("typeface")?.Value;
            if (string.IsNullOrWhiteSpace(typeface)) continue;
            if (typeface[0] == '+') continue;

            return typeface;
        }

        return _file.Theme?.Fonts?.MinorLatin ?? "Calibri";
    }

    /// <summary>The picture a relationship id names, or nothing when it names none that can be drawn.</summary>
    private FramePicture Embedded(string relationshipId)
    {
        if (_file.Package is not OpcPackage package) return FramePicture.None;

        OpcXml.Relationship? found = null;

        foreach (OpcXml.Relationship relationship in package.GetRelationships(Scope))
        {
            if (!string.Equals(relationship.Id, relationshipId, StringComparison.Ordinal)) continue;

            found = relationship;
            break;
        }

        if (found is not { IsExternal: false } target)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2375",
                $"A picture names the relationship '{relationshipId}', which '{Scope}' does not "
                + "declare, so the frame stays empty.",
                new DiagnosticLocation(Scope)));
            return FramePicture.None;
        }

        if (_byPart.TryGetValue(target.Target, out FramePicture cached)) return cached;

        FramePicture image = Load(package, target.Target);
        _byPart[target.Target] = image;
        return image;
    }

    private FramePicture Load(OpcPackage package, string partName)
    {
        IPackagePart? part = package.GetPart(partName);
        if (part is null)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2373",
                $"A picture names the package part '{partName}', which the package does not hold, "
                + "so the frame stays empty.",
                new DiagnosticLocation(partName)));
            return FramePicture.None;
        }

        using Stream content = part.Open();
        using MemoryStream buffer = new();
        content.CopyTo(buffer);

        // The part's declared content type is passed on as a hint and nothing more. OPC requires one
        // and producers still get it wrong; the bytes decide.
        return EmbeddedPicture.Read(buffer.ToArray(), part.MediaType, partName, _diagnostics);
    }
}

/// <summary>What a chart part yielded: the chart itself and the face its text is set in.</summary>
/// <remarks>
/// The two together because they come out of one part and are read in one pass, and because the face
/// cannot be recovered from the chart afterwards — <c>ChartPlot</c> carries type sizes and no family.
/// </remarks>
/// <param name="Plot">The chart, or null when the part holds none that can be drawn.</param>
/// <param name="Family">The family its labels are set in, or null when the part named none.</param>
public readonly record struct DocxChart(ChartPlot? Plot, string? Family);
