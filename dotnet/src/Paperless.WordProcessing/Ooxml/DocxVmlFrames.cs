using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Reads the frame a legacy <c>w:pict</c> or an embedded <c>w:object</c> holds.
/// </summary>
/// <remarks>
/// <para>
/// Before DrawingML, a picture in a Word document was a VML shape: <c>w:pict</c> wrapping a
/// <c>v:shape</c> whose size is a CSS declaration in its <c>style</c> attribute rather than an
/// <c>wp:extent</c> in EMUs. An embedded OLE object is the same shape with an <c>o:OLEObject</c> beside
/// it and a <c>v:imagedata</c> standing in for what the server would draw. Word 2007 and later still
/// write both — every DOCX in this corpus that was ever a <c>.doc</c> does — and a reader that skips
/// them loses the picture <em>and</em> the height it stood in, which is the half that moves page breaks.
/// </para>
/// <para>
/// <strong>Only an inline shape is taken.</strong> VML says where it goes in the same style declaration:
/// <c>position:absolute</c> with <c>mso-position-horizontal</c> and friends is a floating shape, which
/// occupies no space in the text and is far more often a rule, a watermark or a form field's outline
/// than a picture. Measured across the words track: 20 documents carry an inline VML shape and 33 carry
/// only floating ones, so reading the inline case alone is most of the height and none of the risk of
/// putting a watermark in the middle of a paragraph.
/// </para>
/// <para>
/// LibreOffice reads both through <c>oox::vml::Drawing</c> — <c>oox/source/vml/vmlshape.cxx</c>, whose
/// <c>ShapeTypeModel::maWidth</c> and <c>maHeight</c> come from the same style properties by way of
/// <c>ConversionHelper::decodeMeasureToHmm</c>. The units are CSS's, and the one that catches a reader
/// out is the bare number: VML's default is pixels at 96 dpi, not points.
/// </para>
/// </remarks>
internal static class DocxVmlFrames
{
    /// <summary>The VML elements that can carry a shape's size.</summary>
    private static readonly string[] ShapeNames =
        ["shape", "rect", "roundrect", "oval", "group", "image", "polyline", "line", "curve", "arc"];

    /// <summary>
    /// The frame a <c>w:pict</c> or <c>w:object</c> stands for, or null when it holds nothing placeable.
    /// </summary>
    /// <param name="element">The <c>w:pict</c> or <c>w:object</c>.</param>
    /// <param name="anchorOffset">Where in the paragraph's text it sits.</param>
    /// <param name="pictures">
    /// How to resolve a <c>v:imagedata</c>'s relationship into bytes, or null to reserve the geometry
    /// without them — which is all the line height ever needed.
    /// </param>
    public static PageFrame? Read(XElement element, int anchorOffset, DocxPictures? pictures)
    {
        ArgumentNullException.ThrowIfNull(element);

        XElement? shape = element
            .Descendants()
            .FirstOrDefault(child => child.Name.NamespaceName == OoxmlNamespaces.Vml
                                     && ShapeNames.Contains(child.Name.LocalName));

        if (shape is null) return null;

        Dictionary<string, string> style = Style(shape.Attribute("style")?.Value);

        // A floating shape is positioned rather than set in the text, and reserves no height.
        if (style.TryGetValue("position", out string? position)
            && !string.Equals(position, "static", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        Length width = Measure(style.GetValueOrDefault("width"))
                       ?? Twips(element.Attribute(Word.Name("dxaOrig"))?.Value)
                       ?? Length.Zero;
        Length height = Measure(style.GetValueOrDefault("height"))
                        ?? Twips(element.Attribute(Word.Name("dyaOrig"))?.Value)
                        ?? Length.Zero;

        if (width <= Length.Zero || height <= Length.Zero) return null;

        FramePicture picture = pictures?.ReadVml(shape) ?? FramePicture.None;

        return new PageFrame
        {
            Size = new DocSize(width, height),
            Anchor = FrameAnchor.AsCharacter,
            AnchorOffset = anchorOffset,
            Wrap = TextWrap.Through,
            IsImage = true,
            Image = picture.Raster,
            Vector = picture.Vector,
            Name = shape.Attribute("id")?.Value,
        };
    }

    /// <summary>A VML <c>style</c> attribute split into its declarations, lower-cased by property.</summary>
    private static Dictionary<string, string> Style(string? value)
    {
        Dictionary<string, string> declarations = new(StringComparer.OrdinalIgnoreCase);
        if (string.IsNullOrEmpty(value)) return declarations;

        foreach (string part in value.Split(';', StringSplitOptions.RemoveEmptyEntries))
        {
            int colon = part.IndexOf(':', StringComparison.Ordinal);
            if (colon <= 0) continue;

            declarations[part[..colon].Trim()] = part[(colon + 1)..].Trim();
        }

        return declarations;
    }

    /// <summary>
    /// A CSS length as this project's unit, or null when it is missing or unreadable.
    /// </summary>
    /// <remarks>
    /// The units VML uses, which are CSS's with one trap: a bare number is <em>pixels</em> at 96 dpi and
    /// not points, so a shape written <c>width:96</c> is an inch wide. Everything Word itself writes
    /// carries an explicit <c>pt</c>.
    /// </remarks>
    private static Length? Measure(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return null;

        string text = value.Trim();
        int end = 0;
        while (end < text.Length
               && (char.IsAsciiDigit(text[end]) || text[end] is '.' or '-' or '+'))
        {
            end++;
        }

        if (end == 0) return null;
        if (!double.TryParse(text[..end], NumberStyles.Float, CultureInfo.InvariantCulture,
                             out double number))
        {
            return null;
        }

        string unit = text[end..].Trim().ToLowerInvariant();

        return unit switch
        {
            "pt" => Length.FromPoints(number),
            "in" => Length.FromInches(number),
            "cm" => Length.FromInches(number / 2.54),
            "mm" => Length.FromInches(number / 25.4),
            "pc" => Length.FromPoints(number * 12),
            "" or "px" => Length.FromInches(number / 96.0),
            _ => null,
        };
    }

    /// <summary>A twip count, for the <c>w:dxaOrig</c> fallback an object states.</summary>
    private static Length? Twips(string? value)
        => long.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out long twips)
            ? Length.FromTwips(twips)
            : null;
}
