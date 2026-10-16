using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// A VML WordArt shape — a <c>v:shape</c> of a <c>#_x0000_t136</c>-family type carrying a
/// <c>v:textpath</c> — drawn as warped outlines.
/// </summary>
/// <remarks>
/// <para>
/// This is the other half of <see cref="DocxFontwork"/> and it arrives by a different route.
/// A <c>v:shape</c> naming a WordArt shape type becomes a Fontwork custom shape without any
/// conversion step at all: <c>oox/source/vml/vmlshape.cxx:1329</c> hands the shape type number
/// straight to <c>SdrObjCustomShape::MergeDefaultAttributes</c>, which resolves it through
/// <c>EnhancedCustomShapeTypeNames</c> into exactly the LibreOffice type names
/// <see cref="FontworkPresets"/> is keyed by, and <c>TextpathModel::pushToPropMap</c>
/// (<c>oox/source/vml/vmlformatting.cxx:962-1057</c>) then puts it into text-path mode.
/// </para>
/// <para>
/// <strong>Three things the VML path does that the DrawingML one does not, and all three are
/// measurable.</strong>
/// </para>
/// <para>
/// <strong>The text is an attribute, not a body.</strong> <c>v:textpath/@string</c> carries it, and
/// its <c>@style</c> carries the face and size as CSS — <c>font-family:"Arial";font-size:1pt</c>.
/// There is no <c>w:txbxContent</c>, so nothing was ever in the text layer and nothing has to leave
/// it; a warp this cannot draw simply draws nothing, which is what
/// <see cref="DocxVmlFrames"/> already did for all fifteen of them.
/// </para>
/// <para>
/// <strong>The shape's height is thrown away and recomputed from the text.</strong>
/// <c>vmlformatting.cxx:1041-1056</c>: unless the element says <c>trim="t"</c>, LibreOffice measures
/// the string in the stated family at 96 units on a <c>VirtualDevice</c> and replaces the height with
/// <c>textHeight / textWidth × shapeWidth</c>. On the corpus's watermarks that is not a rounding —
/// <c>DOA_Template</c> states <c>height:53pt</c> and the reference imports 57.5 pt, and
/// <c>technical-architecture</c> states 247.45 pt for the five letters of <c>DRAFT</c> and the
/// reference imports 138.
/// </para>
/// <para>
/// <strong><c>ScaleX</c> and <c>SameLetterHeights</c> are hard-coded false</strong>
/// (<c>vmlformatting.cxx:966-975</c>), whatever the shape type says. So the arch family does
/// <em>not</em> keep its font size here, where the DrawingML path gives it
/// <c>fontworkhelpers.cxx:173-179</c>; and <c>gtextFSameHeights</c> — which the ODRAW property set
/// carries and <c>EnhancedCustomShapeFontWork.cxx:488</c> honours — is unreachable from OOXML VML.
/// It is reachable only from binary Escher, at <c>msdffimp.cxx:2516-2600</c>.
/// </para>
/// </remarks>
internal static class DocxVmlFontwork
{
    /// <summary>How LibreOffice measures the string when it recomputes the height.</summary>
    /// <remarks>
    /// <c>vmlformatting.cxx:1044-1047</c> sets a <c>VirtualDevice</c>'s font to <c>Size(0, 96)</c> and
    /// asks for the text's width and height. Only their ratio survives, so the 96 cancels and this
    /// works in font units.
    /// </remarks>
    private const string MeasureNote = "vmlformatting.cxx:1041-1056";

    /// <summary>
    /// The warp a VML shape states, or nothing when it states none this can draw.
    /// </summary>
    /// <param name="shape">The <c>v:shape</c>.</param>
    /// <param name="shapeType">
    /// Its <c>v:shapetype</c>, which is where <c>o:spt</c> usually lives — the shape refers to it by
    /// <c>type="#_x0000_t136"</c> and states no <c>o:spt</c> of its own.
    /// </param>
    /// <param name="size">The rectangle the style declares, before the height is recomputed.</param>
    /// <param name="face">How to resolve a font family into a face.</param>
    public static VmlFontwork Read(
        XElement shape,
        XElement? shapeType,
        DocSize size,
        Func<string?, OpenTypeFace?>? face)
    {
        if (face is null) return default;
        if (ShapeTypeNumber(shape, shapeType) is not { } number) return default;
        if (Fontwork.FontworkTypeOfShapeType(number) is not { } type) return default;

        XElement? path = shape.Element(XName.Get("textpath", OoxmlNamespaces.Vml))
            ?? shapeType?.Element(XName.Get("textpath", OoxmlNamespaces.Vml));

        if (path is null) return default;
        if (!On(path.Attribute("on")?.Value)) return default;

        string text = path.Attribute("string")?.Value ?? string.Empty;
        if (text.Length == 0) return default;

        Dictionary<string, string> style = Css(path.Attribute("style")?.Value);
        OpenTypeFace? resolved = face(Family(style.GetValueOrDefault("font-family")));
        if (resolved is null) return default;

        DocSize box = Fitted(size, text, resolved, path);

        // A `v:textpath` states its lines with a carriage return in the attribute value, which XML
        // has already normalised to a line feed by the time it is read here.
        List<string> lines = [];
        foreach (string line in text.Split('\n')) lines.Add(line.TrimEnd('\r'));

        GraphicsPath? outline = Fontwork.Outline(new FontworkRequest
        {
            FontworkType = type,
            AdjustmentValues = Adjustments(shape, shapeType),

            // Not derived: `vmlformatting.cxx:966-975` writes ScaleX false for every VML text path.
            KeepsFontSize = false,
            Box = box,
            Lines = lines,
            Face = resolved,

            // Only the four presets that keep their size read it, and none of them does here.
            FontSize = Length.FromPoints(1),
        });

        return new VmlFontwork(outline, box);
    }

    /// <summary>
    /// The rectangle the reference gives the shape, which is its stated width and a height measured
    /// off the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>vmlformatting.cxx:1041-1056</c>, and the ratio is all that survives it — the reference
    /// measures at 96 units and divides one measurement by the other. It measures with VCL, which
    /// grid-fits the outline at that size, and this measures the face's own design metrics:
    /// <c>hhea</c>'s ascender less its descender over the sum of the advances. Probed against the
    /// reference on five (family, string) pairs isolated in an otherwise empty document — Arial and
    /// Calibri and Times New Roman, over <c>EASA example document</c>, <c>EASA Example Documents</c>
    /// and <c>DRAFT</c> — this predicts the height it imports to within <b>0.9%</b>, worst case
    /// −0.88% on Times New Roman and +0.17% best. That is 0.5 pt on a 57 pt band, and the probe's own
    /// measurement error at 300 dpi is a third of it.
    /// </para>
    /// <para>
    /// <c>trim="t"</c> asks for the stated height to be kept. No corpus shape states it.
    /// </para>
    /// </remarks>
    private static DocSize Fitted(DocSize size, string text, OpenTypeFace face, XElement path)
    {
        if (Stated(path.Attribute("trim")?.Value)) return size;
        if (size.Width <= Length.Zero) return size;

        double advance = 0;
        foreach (char character in text)
        {
            if (character is '\n' or '\r') continue;
            advance += face.AdvanceForCharacter(character);
        }

        if (advance <= 0) return size;

        double height = face.Horizontal.Ascender - face.Horizontal.Descender;
        if (height <= 0) return size;

        return new DocSize(
            size.Width,
            Length.FromEmu((long)Math.Round(size.Width.Emu * height / advance)));
    }

    /// <summary>The MS-ODRAW shape type the shape or its shape type states, or null.</summary>
    /// <remarks>
    /// <c>oox/source/vml/vmlshape.cxx:320-331</c> reads it from <c>o:spt</c> and falls back to the
    /// number in a <c>type="#_x0000_t136"</c> reference, which is how Word writes it: the number is
    /// in the shape type's <c>id</c> and the shape names it.
    /// </remarks>
    private static int? ShapeTypeNumber(XElement shape, XElement? shapeType)
    {
        XNamespace office = OoxmlNamespaces.VmlOffice;

        foreach (XElement? element in new[] { shape, shapeType })
        {
            if (element?.Attribute(office + "spt")?.Value is not { } stated) continue;
            if (double.TryParse(stated, NumberStyles.Float, CultureInfo.InvariantCulture, out double spt))
            {
                return (int)spt;
            }
        }

        string? reference = shape.Attribute("type")?.Value;
        const string Prefix = "#_x0000_t";
        return reference is not null && reference.StartsWith(Prefix, StringComparison.Ordinal)
               && int.TryParse(
                   reference.AsSpan(Prefix.Length), NumberStyles.Integer,
                   CultureInfo.InvariantCulture, out int number)
            ? number
            : null;
    }

    /// <summary>The <c>adj</c> values the shape states, or its shape type's, or none.</summary>
    /// <remarks>
    /// Already in the 21600 viewbox the preset tables are written in, so nothing is converted. An
    /// empty field keeps the preset's own default, which is what a bare comma in <c>adj=",5400"</c>
    /// means; the reference reaches the same answer by leaving the adjustment
    /// <c>PropertyState_DEFAULT_VALUE</c> for <c>MergeDefaultAttributes</c> to fill in
    /// (<c>svx/source/svdraw/svdoashp.cxx</c>).
    /// </remarks>
    private static List<double>? Adjustments(XElement shape, XElement? shapeType)
    {
        string? stated = shape.Attribute("adj")?.Value ?? shapeType?.Attribute("adj")?.Value;
        if (stated is not { Length: > 0 }) return null;

        List<double> values = [];
        foreach (string part in stated.Split(','))
        {
            values.Add(
                double.TryParse(part.Trim(), NumberStyles.Float, CultureInfo.InvariantCulture,
                                out double value)
                    ? value
                    : 0);
        }

        return values.Count > 0 ? values : null;
    }

    /// <summary>The family a <c>font-family</c> declaration names, unquoted.</summary>
    /// <remarks><c>vmlformatting.cxx:1021-1029</c> strips the first and last character when there is
    /// more than one, which is a quote-stripping that also eats a bare unquoted single letter.</remarks>
    private static string? Family(string? declaration)
    {
        if (declaration is not { Length: > 0 }) return null;

        string value = declaration.Trim();
        return value.Length > 2 ? value[1..^1] : value;
    }

    /// <summary>A CSS declaration list, lower-cased on its property names.</summary>
    private static Dictionary<string, string> Css(string? text)
    {
        Dictionary<string, string> declarations = new(StringComparer.OrdinalIgnoreCase);
        if (text is not { Length: > 0 }) return declarations;

        foreach (string declaration in text.Split(';', StringSplitOptions.RemoveEmptyEntries))
        {
            int colon = declaration.IndexOf(':', StringComparison.Ordinal);
            if (colon <= 0) continue;

            declarations[declaration[..colon].Trim()] = declaration[(colon + 1)..].Trim();
        }

        return declarations;
    }

    /// <summary>A VML boolean whose absence means on, which is what <c>v:textpath/@on</c> is.</summary>
    private static bool On(string? value) => value is null || Stated(value);

    /// <summary>A VML boolean that has to be written to be true, which is what <c>trim</c> is.</summary>
    /// <remarks>
    /// <c>lclDecodeBool</c> returns nothing at all for an absent attribute and
    /// <c>vmlformatting.cxx:1041</c> then tests <c>moTrim.has_value() &amp;&amp; moTrim.value()</c>,
    /// so an unstated <c>trim</c> resizes the shape. Reading it the other way round leaves every
    /// watermark at its declared height, which is the shape of the first cut of this and is
    /// invisible on a shape whose declared height happens to be close.
    /// </remarks>
    private static bool Stated(string? value)
        => value is "t" or "true" or "1" or "T" or "True" or "TRUE";
}

/// <summary>What a VML WordArt shape draws, and the rectangle it draws it in.</summary>
/// <param name="Outline">
/// The warped curves in the shape's own coordinates, or null when the preset is one
/// <see cref="FontworkPresets"/> does not carry, the face has no <c>glyf</c> outlines, or the shape
/// states no text.
/// </param>
/// <param name="Box">
/// The rectangle the reference gives the shape. Its width is the stated one and its height is
/// measured off the text; see <see cref="DocxVmlFontwork"/>.
/// </param>
internal readonly record struct VmlFontwork(GraphicsPath? Outline, DocSize Box)
{
    /// <summary>Whether the shape is a WordArt one at all, warped or not.</summary>
    public bool IsFontwork => Box.Width > Length.Zero || Box.Height > Length.Zero;
}
