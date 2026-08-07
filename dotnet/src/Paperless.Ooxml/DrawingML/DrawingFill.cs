using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Reads the three DrawingML fills that are not a colour: <c>a:gradFill</c>,
/// <c>a:blipFill</c> and <c>a:pattFill</c>.
/// </summary>
/// <remarks>
/// <para>
/// Parsing only. Neither fill can be turned into a <c>Paint</c> here: a gradient's geometry
/// needs the box it fills, and a picture needs the package relationship its <c>r:embed</c>
/// names — and both of those belong to whichever family's reader is asking. So this produces
/// the file's own numbers and leaves the resolution to the caller, exactly as
/// <see cref="DrawingColour"/> does for a colour that needs a theme.
/// </para>
/// <para>
/// Ported from <c>oox/source/drawingml/fillproperties.cxx</c>
/// (<c>FillProperties::pushToPropMap</c>) and the context that fills it,
/// <c>oox/source/drawingml/fillpropertiesgroupcontext.cxx</c>.
/// </para>
/// </remarks>
public static class DrawingFill
{
    /// <summary>
    /// DrawingML's whole, in the unit its percentages are written in.
    /// </summary>
    /// <remarks>
    /// A thousandth of a percent: <c>ST_Percentage</c> writes 100% as <c>100000</c>. The
    /// literal is <c>MAX_PERCENT</c> in <c>oox/inc/drawingml/fillproperties.hxx</c>.
    /// </remarks>
    public const int MaxPercent = 100000;

    /// <summary>Reads an <c>a:gradFill</c>, or null when the element is not one.</summary>
    /// <param name="element">The candidate <c>a:gradFill</c>.</param>
    public static DrawingGradientFill? ReadGradient(XElement? element)
    {
        if (!Drawing.Is(element, "gradFill")) return null;

        List<DrawingGradientStop> stops = [];
        foreach (XElement stop in Drawing.Children(Drawing.Child(element, "gsLst"), "gs"))
        {
            foreach (XElement child in stop.Elements())
            {
                if (DrawingColour.Read(child) is not { } colour) continue;

                stops.Add(new DrawingGradientStop(
                    Percentage(Drawing.Attribute(stop, "pos")) ?? 0, colour));
                break;
            }
        }

        XElement? path = Drawing.Child(element, "path");
        XElement? linear = Drawing.Child(element, "lin");

        return new DrawingGradientFill(stops)
        {
            Path = Drawing.Attribute(path, "path"),
            FillToRect = RelativeRect(Drawing.Child(path, "fillToRect"), whenAbsent: 0),
            Angle = linear is null
                ? null
                : (Drawing.Number(linear, "ang") ?? 0) / 60000.0,
            Scaled = Drawing.Flag(linear, "scaled") ?? false,
            RotateWithShape = Drawing.Flag(element, "rotWithShape") ?? true,
        };
    }

    /// <summary>
    /// Reads a <c>a:blipFill</c> or a <c>p:blipFill</c>, or null when the element is neither.
    /// </summary>
    /// <remarks>
    /// The element's own namespace is not checked, only its local name, because the same
    /// content appears under three of them: <c>a:blipFill</c> as a shape's fill,
    /// <c>p:blipFill</c> inside a <c>p:pic</c>, and <c>xdr:blipFill</c> inside a spreadsheet
    /// drawing. Everything below the wrapper is DrawingML in all three.
    /// </remarks>
    /// <param name="element">The candidate blip fill.</param>
    public static DrawingBlipFill? ReadBlip(XElement? element)
    {
        if (element is null || element.Name.LocalName != "blipFill") return null;

        XElement? blip = Drawing.Child(element, "blip");
        XElement? tile = Drawing.Child(element, "tile");
        XElement? stretch = Drawing.Child(element, "stretch");

        return new DrawingBlipFill
        {
            EmbedId = blip?.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value,
            LinkId = blip?.Attribute(XName.Get("link", OoxmlNamespaces.Relationships))?.Value,
            SourceRect = RelativeRect(Drawing.Child(element, "srcRect"), whenAbsent: 0),
            Tile = tile is not null,
            TileOffsetX = Offset(tile, "tx"),
            TileOffsetY = Offset(tile, "ty"),
            TileScaleX = Percentage(Drawing.Attribute(tile, "sx")) ?? 1,
            TileScaleY = Percentage(Drawing.Attribute(tile, "sy")) ?? 1,
            TileAlign = Drawing.Attribute(tile, "algn") ?? "tl",
            Stretch = stretch is not null,
            FillRect = RelativeRect(Drawing.Child(stretch, "fillRect"), whenAbsent: 0),
            Opacity = Percentage(Drawing.Attribute(Drawing.Child(blip, "alphaModFix"), "amt")) ?? 1,
            Duotone = Duotone(Drawing.Child(blip, "duotone")),
        };
    }

    /// <summary>
    /// An <c>a:duotone</c>'s pair of colours, or null when it does not carry two.
    /// </summary>
    private static (DrawingColour Dark, DrawingColour Light)? Duotone(XElement? element)
    {
        if (element is null) return null;

        List<DrawingColour> colours = [];
        foreach (XElement child in element.Elements())
        {
            if (DrawingColour.Read(child) is { } colour) colours.Add(colour);
        }

        return colours.Count >= 2 ? (colours[0], colours[1]) : null;
    }

    /// <summary>
    /// An <c>ST_Percentage</c> as a fraction of one, or null when it is absent.
    /// </summary>
    /// <remarks>
    /// Both spellings are accepted. The integer form — a thousandth of a percent, so
    /// <c>100000</c> is one — is what every producer writes; the <c>"12.5%"</c> form is what
    /// the strict schema of ECMA-376 second edition allows, and a reader that takes only the
    /// first reads <c>50%</c> as fifty thousand and puts a gradient stop five hundred times
    /// past the end of its ramp.
    /// </remarks>
    private static double? Percentage(string? value)
    {
        if (string.IsNullOrEmpty(value)) return null;

        if (value[^1] == '%')
        {
            return double.TryParse(
                value.AsSpan(0, value.Length - 1), NumberStyles.Float,
                CultureInfo.InvariantCulture, out double percent)
                ? percent / 100.0
                : null;
        }

        return double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double raw)
            ? raw / MaxPercent
            : null;
    }

    private static long Offset(XElement? element, string name)
        => long.TryParse(
            Drawing.Attribute(element, name), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : 0;

    private static DrawingRelativeRect RelativeRect(XElement? element, double whenAbsent)
    {
        if (element is null) return new DrawingRelativeRect(whenAbsent, whenAbsent, whenAbsent, whenAbsent);

        return new DrawingRelativeRect(
            Percentage(Drawing.Attribute(element, "l")) ?? 0,
            Percentage(Drawing.Attribute(element, "t")) ?? 0,
            Percentage(Drawing.Attribute(element, "r")) ?? 0,
            Percentage(Drawing.Attribute(element, "b")) ?? 0);
    }

    /// <summary>Reads an <c>a:pattFill</c>, or null when the element is not one.</summary>
    /// <remarks>
    /// The two colours are read but not resolved, for the same reason a gradient's stops are:
    /// either may be a scheme colour or a <c>phClr</c> placeholder, and only the family's own
    /// reader holds the theme that settles it.
    /// </remarks>
    /// <param name="element">The candidate <c>a:pattFill</c>.</param>
    public static DrawingPatternFill? ReadPattern(XElement? element)
    {
        if (!Drawing.Is(element, "pattFill")) return null;

        return new DrawingPatternFill
        {
            Preset = Drawing.Attribute(element, "prst"),
            Foreground = Colour(Drawing.Child(element, "fgClr")),
            Background = Colour(Drawing.Child(element, "bgClr")),
        };

        static DrawingColour? Colour(XElement? parent)
        {
            if (parent is null) return null;

            foreach (XElement child in parent.Elements())
            {
                if (DrawingColour.Read(child) is { } colour) return colour;
            }

            return null;
        }
    }
}

/// <summary>An <c>a:pattFill</c> as the file states it.</summary>
/// <remarks>
/// A preset name and two colours, which is all the element carries.
/// <see cref="DrawingHatchPresets.Hatch"/> turns the name into the geometry LibreOffice draws
/// for it.
/// </remarks>
public sealed record DrawingPatternFill
{
    /// <summary>
    /// <c>@prst</c>, one of the fifty-four <c>ST_PresetPatternVal</c> tokens, or null.
    /// </summary>
    /// <remarks>
    /// The attribute is required by the schema and absent in the wild. Its absence is not the
    /// same as an unrecognised value: <c>fillproperties.cxx:758-760</c> tests
    /// <c>moPattPreset.has_value()</c> before taking the hatch branch at all, so a pattern with
    /// no preset falls through to being painted in its background colour alone, while one
    /// naming a preset <c>createHatch</c> does not know becomes a hatch of distance nought —
    /// visually the same thing, by a different route.
    /// </remarks>
    public string? Preset { get; init; }

    /// <summary>
    /// <c>a:fgClr</c>, the colour the pattern's lines are drawn in, or null when absent.
    /// </summary>
    public DrawingColour? Foreground { get; init; }

    /// <summary>
    /// <c>a:bgClr</c>, the colour behind them, or null when absent.
    /// </summary>
    public DrawingColour? Background { get; init; }
}

/// <summary>
/// An <c>a:gradFill</c> as the file states it, before any box is known.
/// </summary>
/// <param name="Stops">
/// The <c>a:gs</c> stops in document order, positions as fractions of one.
/// </param>
public sealed record DrawingGradientFill(IReadOnlyList<DrawingGradientStop> Stops)
{
    /// <summary>
    /// <c>a:path/@path</c> — <c>circle</c>, <c>rect</c> or <c>shape</c> — or null for a
    /// linear ramp.
    /// </summary>
    /// <remarks>
    /// The discriminator between the two families of gradient DrawingML has, and the reason
    /// <see cref="Angle"/> and this are both nullable rather than one enum: a file states
    /// either an <c>a:lin</c> or an <c>a:path</c>, and one that states neither is a linear
    /// ramp at zero degrees.
    /// </remarks>
    public string? Path { get; init; }

    /// <summary>
    /// <c>a:path/a:fillToRect</c> — the inner rectangle a path gradient converges on, as
    /// fractions inset from each edge.
    /// </summary>
    public DrawingRelativeRect FillToRect { get; init; }

    /// <summary>
    /// <c>a:lin/@ang</c> in degrees, clockwise from the positive x axis, or null when the file
    /// states no <c>a:lin</c>.
    /// </summary>
    /// <remarks>
    /// Clockwise because a document's y axis points down, so DrawingML's own reading of its
    /// angle and a screen's reading of it agree. LibreOffice converts through its own
    /// anticlockwise convention — <c>(8100 - ang/6000) % 3600</c> in tenths of a degree,
    /// <c>fillproperties.cxx:563</c> — which composes back to exactly this.
    /// </remarks>
    public double? Angle { get; init; }

    /// <summary>
    /// <c>a:lin/@scaled</c>: whether the angle is measured in the shape's own squashed space.
    /// </summary>
    /// <remarks>
    /// Read and carried but not acted on, which is also what LibreOffice does — it assigns
    /// <c>moShadeScaled</c> and never consults it. Honouring it would skew the gradient's
    /// axis by the shape's aspect ratio.
    /// </remarks>
    public bool Scaled { get; init; }

    /// <summary>
    /// <c>a:gradFill/@rotWithShape</c>: whether the gradient turns with the shape's rotation.
    /// </summary>
    public bool RotateWithShape { get; init; } = true;
}

/// <summary>One <c>a:gs</c> stop.</summary>
/// <param name="Position">Where it sits on the ramp, from 0 to 1.</param>
/// <param name="Colour">Its colour, still unresolved against a theme.</param>
public readonly record struct DrawingGradientStop(double Position, DrawingColour Colour);

/// <summary>
/// A <c>a:blipFill</c> or <c>p:blipFill</c> as the file states it.
/// </summary>
public sealed record DrawingBlipFill
{
    /// <summary>
    /// <c>a:blip/@r:embed</c>: the relationship naming the image part inside the package.
    /// </summary>
    public string? EmbedId { get; init; }

    /// <summary>
    /// <c>a:blip/@r:link</c>: a relationship naming an image <em>outside</em> the package.
    /// </summary>
    /// <remarks>
    /// Carried so a caller can report it, never followed. Fetching it would make reading a
    /// document a network request against an address the document chose.
    /// </remarks>
    public string? LinkId { get; init; }

    /// <summary>
    /// <c>a:srcRect</c>: how much of each edge of the <em>source</em> image to throw away.
    /// </summary>
    public DrawingRelativeRect SourceRect { get; init; }

    /// <summary>True when the fill states <c>a:tile</c>.</summary>
    public bool Tile { get; init; }

    /// <summary><c>a:tile/@tx</c> in EMUs.</summary>
    public long TileOffsetX { get; init; }

    /// <summary><c>a:tile/@ty</c> in EMUs.</summary>
    public long TileOffsetY { get; init; }

    /// <summary><c>a:tile/@sx</c> as a fraction of the image's own width.</summary>
    public double TileScaleX { get; init; } = 1;

    /// <summary><c>a:tile/@sy</c> as a fraction of the image's own height.</summary>
    public double TileScaleY { get; init; } = 1;

    /// <summary><c>a:tile/@algn</c>: which point of the shape the tile grid is anchored on.</summary>
    public string TileAlign { get; init; } = "tl";

    /// <summary>True when the fill states <c>a:stretch</c>.</summary>
    public bool Stretch { get; init; }

    /// <summary>
    /// <c>a:stretch/a:fillRect</c>: how far inside the filled area the stretched image sits.
    /// </summary>
    public DrawingRelativeRect FillRect { get; init; }

    /// <summary><c>a:blip/a:alphaModFix/@amt</c> as a fraction of one; 1 when absent.</summary>
    public double Opacity { get; init; } = 1;

    /// <summary>
    /// <c>a:blip/a:duotone</c>'s two colours, still unresolved against a theme, or null when
    /// the blip states none or states one of them alone.
    /// </summary>
    /// <remarks>
    /// Both or neither, because a ramp needs two ends: <c>lclCheckAndApplyDuotoneTransform</c>
    /// applies the transform only when both are used
    /// (<c>oox/source/drawingml/fillproperties.cxx:74</c>). The first is the colour a black
    /// pixel becomes.
    /// </remarks>
    public (DrawingColour Dark, DrawingColour Light)? Duotone { get; init; }
}

/// <summary>
/// A rectangle stated as fractions inset from each edge of something else, which is how
/// DrawingML's <c>a:srcRect</c>, <c>a:fillRect</c> and <c>a:fillToRect</c> all read.
/// </summary>
/// <param name="Left">Inset from the left edge, as a fraction of the width.</param>
/// <param name="Top">Inset from the top edge, as a fraction of the height.</param>
/// <param name="Right">Inset from the right edge, as a fraction of the width.</param>
/// <param name="Bottom">Inset from the bottom edge, as a fraction of the height.</param>
public readonly record struct DrawingRelativeRect(
    double Left, double Top, double Right, double Bottom)
{
    /// <summary>True when every edge is zero, so the rectangle is the whole of its parent.</summary>
    public bool IsWhole => Left == 0 && Top == 0 && Right == 0 && Bottom == 0;
}
