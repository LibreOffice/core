using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.Vector;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// ODF's named fill definitions — <c>draw:gradient</c> and <c>draw:fill-image</c> — indexed by
/// the name a graphic style refers to them by.
/// </summary>
/// <remarks>
/// <para>
/// A gradient in ODF is not on the shape and not in its style either: the style says
/// <c>draw:fill-gradient-name="LinearRedBlue"</c> and a sibling of <c>office:styles</c> called
/// <c>draw:gradient</c> carries the colours. That indirection is why this is a separate index
/// rather than another property lookup — <c>OdfStyles</c> resolves <c>style:style</c>
/// elements through their parent chain, and these are not styles at all.
/// </para>
/// <para>
/// <b>Indexed under both names.</b> A definition may carry a <c>draw:display-name</c> as well as
/// a <c>draw:name</c>, and when it does LibreOffice registers the display name as the one a
/// style refers to (<c>xmloff/source/style/GradientStyle.cxx:147-152</c>). Real files reference
/// either, so both are keys and the internal name never shadows a display name that another
/// definition already claimed.
/// </para>
/// </remarks>
internal sealed class OdpFills
{
    private readonly Dictionary<string, XElement> _gradients = new(StringComparer.Ordinal);
    private readonly Dictionary<string, XElement> _images = new(StringComparer.Ordinal);
    private readonly OdfFile _file;

    /// <summary>Indexes every fill definition the document declares.</summary>
    /// <param name="file">The open document.</param>
    public OdpFills(OdfFile file)
    {
        _file = file;

        foreach (XElement? root in (XElement?[])[file.StylesRoot, file.ContentRoot])
        {
            if (root is null) continue;

            foreach (XElement container in root.Elements(XName.Get("styles", OdfNamespaces.Office)))
            {
                foreach (XElement definition in container.Elements())
                {
                    if (definition.Name.NamespaceName != OdfNamespaces.Draw) continue;

                    Dictionary<string, XElement>? index = definition.Name.LocalName switch
                    {
                        "gradient" => _gradients,
                        "fill-image" => _images,
                        _ => null,
                    };

                    if (index is null) continue;

                    Register(index, Attribute(definition, OdfNamespaces.Draw, "name"), definition);
                    Register(index, Attribute(definition, OdfNamespaces.Draw, "display-name"), definition);
                }
            }
        }
    }

    /// <summary>The <c>draw:gradient</c> a style names, read, or null when there is none.</summary>
    /// <param name="name">The value of <c>draw:fill-gradient-name</c>.</param>
    public OdpGradient? Gradient(string? name)
    {
        if (name is null || !_gradients.TryGetValue(name, out XElement? element)) return null;

        Colour start = Colour(element, "start-color", Core.Graphics.Colour.Black);
        Colour end = Colour(element, "end-color", Core.Graphics.Colour.White);

        return new OdpGradient(
            Attribute(element, OdfNamespaces.Draw, "style") ?? "linear",
            Intense(start, Percentage(Attribute(element, OdfNamespaces.Draw, "start-intensity"))),
            Intense(end, Percentage(Attribute(element, OdfNamespaces.Draw, "end-intensity"))),
            Angle(Attribute(element, OdfNamespaces.Draw, "angle")),
            Math.Clamp(Percentage(Attribute(element, OdfNamespaces.Draw, "border")) ?? 0, 0, 1),
            Percentage(Attribute(element, OdfNamespaces.Draw, "cx")) ?? 0.5,
            Percentage(Attribute(element, OdfNamespaces.Draw, "cy")) ?? 0.5);
    }

    /// <summary>
    /// The picture a <c>draw:fill-image</c> holds, still encoded, or null when it resolves to
    /// nothing.
    /// </summary>
    /// <param name="name">The value of <c>draw:fill-image-name</c>.</param>
    public RasterImage? Image(string? name)
        => name is not null && _images.TryGetValue(name, out XElement? element)
            ? Picture(element)
            : null;

    /// <summary>
    /// The picture an element carries, whether inline or as a package part.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both spellings appear on the same elements — <c>draw:image</c> and
    /// <c>draw:fill-image</c> — and which one a file uses is decided by its physical form
    /// rather than by anything about the picture: a flat <c>.fodp</c> has nowhere to put a
    /// package part and inlines base64, while a zipped <c>.odp</c> writes
    /// <c>xlink:href="Pictures/…"</c>. A reader that knows only one loses every picture in half
    /// the documents it is given.
    /// </para>
    /// <para>
    /// Nothing here decodes. The bytes go on as <see cref="RasterImage.Encoded"/> and whichever
    /// backend wants pixels decodes them, which is what keeps this library free of a dependency
    /// on the rasteriser.
    /// </para>
    /// </remarks>
    /// <param name="element">A <c>draw:image</c> or <c>draw:fill-image</c>.</param>
    public RasterImage? Picture(XElement? element) => Bytes(element) is { } found
        ? RasterImage.Encoded(found.Bytes, found.MediaType)
        : null;

    /// <summary>
    /// The same picture, but told apart into a raster and a vector.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Separate from <see cref="Picture"/> because the two callers want different things. A
    /// <c>draw:fill-image</c> becomes a <c>BitmapPaint</c>, which tiles pixels and has no form
    /// for a display list; a <c>draw:image</c> inside a <c>draw:frame</c> is a picture placed
    /// once, and that one can be a metafile. Giving the fill path a vector it cannot use would
    /// lose the fill altogether.
    /// </para>
    /// <para>
    /// <b>Sniffed rather than believed.</b> <c>draw:mime-type</c> is right far more often in ODF
    /// than a DOCX part name is, and it is still only a claim; the bytes are what
    /// <c>VectorImages.For</c> reads.
    /// </para>
    /// </remarks>
    /// <param name="element">A <c>draw:image</c>.</param>
    public (RasterImage? Raster, Lazy<VectorImage>? Vector, bool IsInline) Drawable(XElement? element)
    {
        if (Bytes(element) is not { } found) return default;

        ReadOnlyMemory<byte> bytes = found.Bytes;

        return VectorImages.For(bytes.Span) is not null
            ? (null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)), found.IsInline)
            : (RasterImage.Encoded(bytes, found.MediaType), null, found.IsInline);
    }

    /// <summary>The bytes an element names, inline or as a package part, with its declared type.</summary>
    private (ReadOnlyMemory<byte> Bytes, string? MediaType, bool IsInline)? Bytes(XElement? element)
    {
        if (element is null) return null;

        XElement? inline = element.Element(XName.Get("binary-data", OdfNamespaces.Office));
        if (inline is not null)
        {
            try
            {
                byte[] bytes = Convert.FromBase64String(inline.Value);
                return bytes.Length == 0
                    ? null
                    : (bytes, Attribute(element, OdfNamespaces.Draw, "mime-type"), true);
            }
            catch (FormatException)
            {
                // A picture whose base64 will not decode is a defect in one shape, not a reason
                // to fail the slide around it.
                return null;
            }
        }

        if (PartName(Attribute(element, OdfNamespaces.XLink, "href")) is not { } part) return null;
        using Stream? content = _file.OpenPart(part);
        if (content is null) return null;

        using MemoryStream buffer = new();
        content.CopyTo(buffer);

        return buffer.Length == 0
            ? null
            : (buffer.ToArray(), _file.Package?.GetPart(part)?.MediaType, false);
    }

    /// <summary>
    /// An ODF percentage as a fraction of one, or null when it is absent or unreadable.
    /// </summary>
    public static double? Percentage(string? value)
    {
        if (string.IsNullOrEmpty(value)) return null;

        ReadOnlySpan<char> text = value.AsSpan().Trim();
        bool percent = text.Length > 0 && text[^1] == '%';
        if (percent) text = text[..^1];

        return double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? (percent ? parsed / 100.0 : parsed)
            : null;
    }

    private static void Register(Dictionary<string, XElement> index, string? name, XElement element)
    {
        if (!string.IsNullOrEmpty(name)) index.TryAdd(name, element);
    }

    private static Colour Colour(XElement element, string name, Colour whenAbsent)
        => OdfValue.ParseColour(Attribute(element, OdfNamespaces.Draw, name)) ?? whenAbsent;

    /// <summary>
    /// A colour scaled towards black by an intensity percentage.
    /// </summary>
    /// <remarks>
    /// <c>draw:start-intensity</c> and <c>draw:end-intensity</c> are a dimmer on the stop, which
    /// LibreOffice folds into the colour before the gradient is drawn
    /// (<c>BGradient::tryToApplyStartEndIntensity</c>). A hundred per cent, which is what every
    /// file LibreOffice writes states, leaves the colour alone.
    /// </remarks>
    private static Colour Intense(Colour colour, double? intensity)
    {
        if (intensity is not { } factor || factor >= 1) return colour;

        double clamped = Math.Clamp(factor, 0, 1);
        return new Colour(
            (byte)Math.Round(colour.R * clamped),
            (byte)Math.Round(colour.G * clamped),
            (byte)Math.Round(colour.B * clamped),
            colour.A);
    }

    /// <summary>
    /// A <c>draw:angle</c> in radians.
    /// </summary>
    /// <remarks>
    /// <b>A bare number means two different things depending on the document's version.</b> ODF
    /// 1.1 left the unit undefined and OpenOffice wrote tenths of a degree; ODF 1.2 says degrees
    /// and LibreOffice now writes the unit outright. <c>Converter::convert10thDegAngle</c>
    /// (<c>sax/source/tools/converter.cxx:878</c>) resolves it exactly this way, and reading a
    /// 1.3 document the old way turns 90 degrees into 9 and lays every gradient down a different
    /// axis.
    /// </remarks>
    private double Angle(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return 0;

        ReadOnlySpan<char> text = value.AsSpan().Trim();
        double scale = 1;

        if (text.EndsWith("deg", StringComparison.Ordinal)) text = text[..^3];
        else if (text.EndsWith("grad", StringComparison.Ordinal)) { text = text[..^4]; scale = 0.9; }
        else if (text.EndsWith("rad", StringComparison.Ordinal)) { text = text[..^3]; scale = 180 / Math.PI; }
        else if (!IsAtLeastOdf12) scale = 0.1;

        if (!double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed))
            return 0;

        return parsed * scale * Math.PI / 180.0;
    }

    private bool IsAtLeastOdf12
        => _file.Version is not { } version
           || !double.TryParse(version, NumberStyles.Float, CultureInfo.InvariantCulture, out double number)
           || number >= 1.2;

    private static string? Attribute(XElement? element, string ns, string name)
        => element?.Attribute(XName.Get(name, ns))?.Value;

    /// <summary>
    /// An <c>xlink:href</c> as a package part name, or null when it points outside the package.
    /// </summary>
    /// <remarks>
    /// Anything with a scheme is external and is never fetched: extraction must make no network
    /// requests, and following one on untrusted input is a server-side request forgery.
    /// </remarks>
    private static string? PartName(string? href)
    {
        if (string.IsNullOrWhiteSpace(href)) return null;

        string trimmed = href.Trim();
        if (trimmed.Contains("://", StringComparison.Ordinal)
            || trimmed.StartsWith("data:", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        if (trimmed.StartsWith("./", StringComparison.Ordinal)) trimmed = trimmed[2..];
        return trimmed.Length == 0 ? null : trimmed;
    }
}

/// <summary>
/// A <c>draw:gradient</c> as the file states it, before any shape is known.
/// </summary>
/// <param name="Style">
/// <c>draw:style</c>: <c>linear</c>, <c>axial</c>, <c>radial</c>, <c>ellipsoid</c>,
/// <c>square</c> or <c>rectangular</c>.
/// </param>
/// <param name="StartColour">
/// <c>draw:start-color</c>, with its intensity applied. <b>For every style but linear and axial
/// this is the colour of the outer edge, not of the centre.</b>
/// </param>
/// <param name="EndColour"><c>draw:end-color</c>, with its intensity applied.</param>
/// <param name="Angle"><c>draw:angle</c> in radians, anticlockwise as ODF measures it.</param>
/// <param name="Border"><c>draw:border</c> as a fraction of the ramp held at its first colour.</param>
/// <param name="CentreX"><c>draw:cx</c> as a fraction of the shape's width.</param>
/// <param name="CentreY"><c>draw:cy</c> as a fraction of its height.</param>
internal readonly record struct OdpGradient(
    string Style,
    Colour StartColour,
    Colour EndColour,
    double Angle,
    double Border,
    double CentreX,
    double CentreY);
