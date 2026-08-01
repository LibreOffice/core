using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Graphics;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Resolves an <c>a:blip</c>'s <c>r:embed</c> into the bytes of a package part.
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
