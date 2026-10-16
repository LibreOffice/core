using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// What a worksheet <c>xdr:sp</c> or <c>xdr:cxnSp</c> declares for its own fill and outline.
/// </summary>
/// <remarks>
/// <para>
/// <strong>None of it was read, in any format.</strong> Every shape on every worksheet was drawn
/// as bare text over whatever was under it. Censused whole-corpus over the <c>xlsx</c> family
/// (<c>probes/sheet-fill-r84/census.py</c>): <b>644 <c>xdr:sp</c> in 49 documents</b>, of which
/// 421 state an <c>a:solidFill</c> of their own, 205 an <c>a:ln/a:solidFill</c> and 583 an
/// <c>xdr:style</c>.
/// </para>
/// <para>
/// <strong>The style reference is far less of the work than a count of it suggests, and that is
/// worth stating because the shape of the feature depends on it.</strong> Of those 583 styled
/// shapes <b>497 also state a fill of their own</b>, and the shape's own statement wins —
/// <c>Shape::getActualFillProperties</c> takes the theme as the base and <c>assignUsed</c>s the
/// shape's over it (<c>oox/source/drawingml/shape.cxx</c>). Resolved per shape, the effective
/// source of a fill across the corpus is: <b>421 the shape's own <c>a:solidFill</c>, 54 a theme
/// solid fill, 32 a theme gradient, 98 <c>a:grpFill</c>, 29 <c>a:noFill</c> and 10
/// <c>a:blipFill</c></b>. So the format matrix decides 86 shapes in 3 documents; what really is
/// needed everywhere is theme <em>colour</em> resolution, because 332 of the 626 explicit colour
/// references are an <c>a:schemeClr</c> sitting on the shape's own fill.
/// </para>
/// <para>
/// The reader is <c>DocxFrames.Appearance</c>'s shape, deliberately: a shape's own fill wins over
/// the matrix, an <c>a:ln</c> is laid <em>over</em> the theme's rather than replacing it — so a
/// shape stating a colour and no width takes the theme's width — and <c>a:noFill</c> inside the
/// line beats the matrix outright. A pattern or a picture fill is a real fill this cannot yet
/// draw, and painting its first colour flat would be a confident wrong answer rather than an
/// absent one.
/// </para>
/// </remarks>
internal static class XlsxShapeInk
{
    private const string DrawingNamespace =
        "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";

    private const string MainNamespace =
        "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>A shape's resolved interior and outline.</summary>
    /// <param name="Fill">The flat interior colour, or null.</param>
    /// <param name="Gradient">The interior ramp, or null.</param>
    /// <param name="Stroke">The outline colour, or null.</param>
    /// <param name="StrokeWidth">How wide the outline is; zero for a hairline.</param>
    /// <param name="Preset">The preset the ink is painted through, or null for the box.</param>
    /// <param name="Adjustments">The <c>a:avLst</c> the shape states, or null.</param>
    /// <param name="FlipHorizontal">The <c>a:xfrm/@flipH</c> the geometry is mirrored by.</param>
    /// <param name="FlipVertical">Its <c>@flipV</c>.</param>
    public readonly record struct Ink(
        Colour? Fill,
        GradientDescription? Gradient,
        Colour? Stroke,
        Length StrokeWidth,
        string? Preset,
        IReadOnlyDictionary<string, double>? Adjustments,
        bool FlipHorizontal = false,
        bool FlipVertical = false)
    {
        /// <summary>True when there is something to paint.</summary>
        public bool HasInk => Fill is not null || Gradient is not null || Stroke is not null;
    }

    /// <summary>
    /// Reads one shape's ink.
    /// </summary>
    /// <param name="shape">The <c>xdr:sp</c>, <c>xdr:cxnSp</c> or <c>xdr:grpSp</c> element.</param>
    /// <param name="theme">The workbook's colour scheme, for an <c>a:schemeClr</c>.</param>
    /// <param name="styles">The theme's format matrix, for an <c>a:fillRef</c>/<c>a:lnRef</c>.</param>
    /// <param name="inherited">
    /// What the enclosing group offers a shape saying <c>a:grpFill</c> — the group's own fill,
    /// resolved on the way down. Default for a shape that is in no group.
    /// </param>
    public static Ink Read(
        XElement? shape, DrawingTheme? theme, DrawingStyleMatrix? styles, Ink inherited = default)
    {
        XElement? properties = Child(shape, DrawingNamespace, "spPr")
                               ?? Child(shape, DrawingNamespace, "grpSpPr");

        if (properties is null) return default;

        XElement? style = Child(shape, DrawingNamespace, "style");

        Colour? fill = Solid(Child(properties, MainNamespace, "solidFill"), theme);
        GradientDescription? gradient =
            DrawingGradient.Read(Child(properties, MainNamespace, "gradFill"), theme);

        // `a:grpFill` is not a fill but a reference to the enclosing group's, so it takes what the
        // group offered and ends the search: a shape asking for one whose group has none is
        // unfilled rather than falling through to its style's. 98 corpus shapes in 2 documents.
        if (Child(properties, MainNamespace, "grpFill") is not null)
        {
            fill = inherited.Fill;
            gradient = inherited.Gradient;
        }
        else if (fill is null && gradient is null && !StatesFill(properties)
                 && styles?.Fill(style, theme) is { } themed)
        {
            fill = Solid(Child(themed, MainNamespace, "solidFill"), theme);
            gradient = DrawingGradient.Read(Child(themed, MainNamespace, "gradFill"), theme);
        }

        XElement? themedLine = styles?.Line(style, theme);
        XElement? line = Child(properties, MainNamespace, "ln");

        if (line is null) line = themedLine;
        else if (themedLine is not null && Child(line, MainNamespace, "noFill") is null)
            line = DrawingStyleMatrix.Overlay(themedLine, line);

        Colour? stroke = Solid(Child(line, MainNamespace, "solidFill"), theme);
        Length width = stroke is null ? Length.Zero : Emu(line?.Attribute("w")?.Value);

        (string? preset, IReadOnlyDictionary<string, double>? adjustments) = Geometry(properties);

        // The two flips, which are what a connector states its direction with. They mirror the
        // shape's own space *before* the rotation, so all three compose: an elbow connector
        // written `rot="10800000" flipH="1" flipV="1"` is the identity, and honouring only the
        // rotation draws its elbow in the opposite corner. 26 of the corpus's worksheet shapes
        // state one, every one of them in an organisation chart's connectors.
        XElement? transform = Child(properties, MainNamespace, "xfrm");
        bool flipH = transform?.Attribute("flipH")?.Value is "1" or "true";
        bool flipV = transform?.Attribute("flipV")?.Value is "1" or "true";

        return new Ink(fill, gradient, stroke, width, preset, adjustments, flipH, flipV);
    }

    /// <summary>
    /// The preset the shape's outline follows, and the adjustment handles it states.
    /// </summary>
    /// <remarks>
    /// <c>rect</c> is answered as null on purpose: it evaluates to the bounding box the painter's
    /// own fallback already draws, so naming it would build a four-point path to arrive exactly
    /// where not naming it arrives. It is the commonest preset in the corpus — 206 of the 644 —
    /// so this is most of the shapes rather than a corner. <c>a:custGeom</c> is not resolved here;
    /// such a shape falls back to its box, which is what a preset the catalogue does not know
    /// gets.
    /// </remarks>
    private static (string? Preset, IReadOnlyDictionary<string, double>? Adjustments) Geometry(
        XElement properties)
    {
        XElement? geometry = Child(properties, MainNamespace, "prstGeom");
        if (geometry?.Attribute("prst")?.Value is not { Length: > 0 } preset) return (null, null);
        if (preset == "rect") return (null, null);

        Dictionary<string, double>? adjustments = null;
        foreach (XElement guide in
                 Child(geometry, MainNamespace, "avLst")?.Elements() ?? [])
        {
            if (guide.Name.LocalName != "gd") continue;
            if (guide.Attribute("name")?.Value is not { Length: > 0 } name) continue;
            if (guide.Attribute("fmla")?.Value is not { Length: > 0 } formula) continue;
            if (!formula.StartsWith("val ", StringComparison.Ordinal)) continue;
            if (!double.TryParse(
                    formula.AsSpan(4), NumberStyles.Float, CultureInfo.InvariantCulture,
                    out double value))
            {
                continue;
            }

            (adjustments ??= [])[name] = value;
        }

        return (preset, adjustments);
    }

    /// <summary>Any of the six fill elements, which each mean the shape has said its own answer.</summary>
    private static bool StatesFill(XElement properties)
        => properties.Elements().Any(child => child.Name.LocalName
            is "noFill" or "solidFill" or "gradFill" or "blipFill" or "pattFill" or "grpFill");

    private static Colour? Solid(XElement? solidFill, DrawingTheme? theme)
        => solidFill is null
            ? null
            : DrawingColour.Read(solidFill.Elements().FirstOrDefault())?.Resolve(theme);

    private static Length Emu(string? value)
        => value is not null
           && long.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out long emu)
           && emu > 0
            ? Length.FromEmu(emu)
            : Length.Zero;

    private static XElement? Child(XElement? parent, string ns, string name)
        => parent?.Element(XName.Get(name, ns));
}
