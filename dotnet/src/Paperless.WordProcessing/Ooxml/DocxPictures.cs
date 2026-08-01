using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Graphics;
using Paperless.Ooxml;

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
/// One <see cref="RasterImage"/> per part, cached: a logo in a running head is one picture drawn on
/// every page, and the PDF writer deduplicates its image XObjects by object identity — so returning a
/// fresh instance per use would write the same bytes into the file once per page.
/// </para>
/// </remarks>
public sealed class DocxPictures
{
    private readonly DocxFile _file;
    private readonly List<Diagnostic>? _diagnostics;
    private readonly Dictionary<string, RasterImage?> _byPart = new(StringComparer.Ordinal);

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
    /// The picture a <c>w:drawing</c> or a legacy <c>w:pict</c> holds, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// The first <c>a:blip</c> anywhere beneath the element, because the element it hangs from differs by
    /// what the picture is for: <c>pic:blipFill</c> for an ordinary picture, <c>a:blipFill</c> for a
    /// shape filled with one, and <c>wps:spPr</c> for either inside a text box. All three mean the same
    /// thing to a reader that wants the bytes.
    /// </remarks>
    public RasterImage? Read(XElement drawing)
    {
        ArgumentNullException.ThrowIfNull(drawing);

        XElement? blip = drawing
            .DescendantsAndSelf()
            .FirstOrDefault(element => element.Name.LocalName == "blip");

        if (blip is null) return null;

        string? embed = blip.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value;
        if (embed is not null) return Embedded(embed);

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

        return null;
    }

    /// <summary>The picture a relationship id names, or null when it names none that can be drawn.</summary>
    private RasterImage? Embedded(string relationshipId)
    {
        if (_file.Package is not OpcPackage package) return null;

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
            return null;
        }

        if (_byPart.TryGetValue(target.Target, out RasterImage? cached)) return cached;

        RasterImage? image = Load(package, target.Target);
        _byPart[target.Target] = image;
        return image;
    }

    private RasterImage? Load(OpcPackage package, string partName)
    {
        IPackagePart? part = package.GetPart(partName);
        if (part is null)
        {
            _diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2373",
                $"A picture names the package part '{partName}', which the package does not hold, "
                + "so the frame stays empty.",
                new DiagnosticLocation(partName)));
            return null;
        }

        using Stream content = part.Open();
        using MemoryStream buffer = new();
        content.CopyTo(buffer);

        // The part's declared content type is passed on as a hint and nothing more. OPC requires one
        // and producers still get it wrong; the bytes decide.
        return EmbeddedPicture.Read(buffer.ToArray(), part.MediaType, partName, _diagnostics);
    }
}
