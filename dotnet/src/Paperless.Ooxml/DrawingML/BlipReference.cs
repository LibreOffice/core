using System.Xml.Linq;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Which part of an <c>a:blip</c> to draw when it offers more than one.
/// </summary>
/// <remarks>
/// <para>
/// A DrawingML picture is one <c>a:blip</c>, but since Office 2016 it can carry two
/// renderings of the same artwork: the <c>r:embed</c> relationship names a raster, and an
/// <c>asvg:svgBlip</c> inside the <c>{96DAC541-7B7A-43D3-8B79-37D633B846F1}</c> extension
/// names an SVG. The raster exists so that a consumer which cannot read SVG still shows
/// something; a consumer which can read SVG is meant to prefer it, and that preference is
/// the whole point of vector import — the raster fallback is written at one fixed size and
/// looks soft at print resolution, whereas the SVG is exact at any.
/// </para>
/// <para>
/// This is not a Microsoft-only construct. LibreOffice 24.2 writes it too: converting an ODT
/// holding a 1 395-byte <c>Pictures/logo.svg</c> to <c>.docx</c> produced
/// <c>word/media/image2.svg</c> beside a 39 497-byte <c>image1.png</c>, wired up exactly as
/// above — so a document that has been through either application arrives in this shape.
/// </para>
/// </remarks>
public static class BlipReference
{
    /// <summary>
    /// The extension GUID Office assigns to the SVG blip.
    /// </summary>
    /// <remarks>
    /// Matched on rather than on the element name alone because <c>a:extLst</c> is an
    /// open-ended list and the URI is what identifies an extension; a producer is free to
    /// use its own prefix for the namespace.
    /// </remarks>
    public const string SvgExtensionUri = "{96DAC541-7B7A-43D3-8B79-37D633B846F1}";

    /// <summary>The namespace the <c>svgBlip</c> element lives in.</summary>
    public const string SvgBlipNamespace = "http://schemas.microsoft.com/office/drawing/2016/SVG/main";

    /// <summary>What a blip offers to draw.</summary>
    /// <param name="RelationshipId">
    /// The relationship naming the part to draw, or null when the blip names nothing
    /// embedded — a linked picture, which Paperless never fetches.
    /// </param>
    /// <param name="IsVector">
    /// True when <paramref name="RelationshipId"/> is the SVG rather than the raster.
    /// </param>
    /// <param name="FallbackRelationshipId">
    /// The raster, when the vector was preferred. A caller whose vector decode comes back
    /// empty should draw this instead rather than drawing nothing.
    /// </param>
    public readonly record struct Choice(
        string? RelationshipId,
        bool IsVector,
        string? FallbackRelationshipId);

    /// <summary>
    /// Chooses what to draw for a blip, preferring the vector.
    /// </summary>
    /// <param name="blip">
    /// The <c>a:blip</c> element, or null. Null in, nothing out — a picture without a blip is
    /// routine in files with a VML fallback.
    /// </param>
    /// <remarks>
    /// <c>r:link</c> is deliberately not read. It names a file outside the package, and a
    /// document reader that fetches one is an exfiltration channel — the same reason
    /// <c>Paperless.Vector</c>'s SVG asset loader resolves nothing but <c>data:</c> URIs.
    /// </remarks>
    public static Choice Choose(XElement? blip)
    {
        if (blip is null) return default;

        string? raster = blip.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value;
        string? vector = Svg(blip);

        return vector is { Length: > 0 }
            ? new Choice(vector, IsVector: true, raster)
            : new Choice(raster is { Length: > 0 } ? raster : null, IsVector: false, null);
    }

    /// <summary>The relationship id of a blip's SVG rendering, if it has one.</summary>
    private static string? Svg(XElement blip)
    {
        XName extensions = XName.Get("extLst", OoxmlNamespaces.DrawingML);
        XName extension = XName.Get("ext", OoxmlNamespaces.DrawingML);
        XName svgBlip = XName.Get("svgBlip", SvgBlipNamespace);

        foreach (XElement ext in blip.Elements(extensions).Elements(extension))
        {
            if (!string.Equals(ext.Attribute("uri")?.Value, SvgExtensionUri, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            if (ext.Element(svgBlip)?.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value
                is { Length: > 0 } id)
            {
                return id;
            }
        }

        return null;
    }
}
