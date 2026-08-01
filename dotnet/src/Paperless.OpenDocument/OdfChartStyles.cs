using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.OpenDocument;

/// <summary>
/// The automatic styles a chart sub-document declares, indexed by name.
/// </summary>
/// <remarks>
/// <para>
/// A chart sub-document is a whole <c>office:document</c> of its own, so its styles are its own
/// too: <c>ch1</c>, <c>ch2</c>, <c>ch3</c> … declared in its <c>office:automatic-styles</c> and
/// referred to by <c>chart:style-name</c> on the chart, the plot area, each axis and each
/// series. They are not the containing deck's styles and they do not resolve through its style
/// tree, which is why this is a small reader of its own rather than a use of
/// <c>OdfStyles</c>.
/// </para>
/// <para>
/// <strong>Only automatic styles, and no inheritance chain.</strong> Every chart LibreOffice
/// writes states each property on a flat automatic style with no <c>style:parent-style-name</c>
/// — measured on <c>chart-bar-deck.odp</c>, all eleven of whose <c>style:style</c> elements are
/// parentless. A hand-written chart could use a named parent; this would then read the child's
/// properties and miss the parent's, which loses a colour rather than a position.
/// </para>
/// </remarks>
public sealed class OdfChartStyles
{
    private readonly Dictionary<string, XElement> _styles = new(StringComparer.Ordinal);

    /// <summary>Indexes every <c>style:style</c> in a chart sub-document.</summary>
    /// <param name="document">The chart's <c>office:document</c> or <c>office:document-content</c>.</param>
    public OdfChartStyles(XElement? document)
    {
        if (document is null) return;

        foreach (XElement style in document.Descendants(XName.Get("style", OdfNamespaces.Style)))
        {
            if (style.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value is not { Length: > 0 } name)
                continue;

            _styles.TryAdd(name, style);
        }
    }

    /// <summary>The fill colour a style states, or null for none.</summary>
    /// <remarks>
    /// <c>draw:fill="solid"</c> with a <c>draw:fill-color</c>. A style whose <c>draw:fill</c> is
    /// <c>none</c> yields null rather than white: a plot area with no fill shows the chart's
    /// background through it, and painting white over that hides a coloured chart area.
    /// </remarks>
    public Colour? Fill(string? name)
    {
        XElement? graphic = Properties(name, "graphic-properties");
        if (graphic is null) return null;

        string? fill = Attribute(graphic, OdfNamespaces.Draw, "fill");
        if (fill == "none") return null;

        return OdfValue.ParseColour(Attribute(graphic, OdfNamespaces.Draw, "fill-color"));
    }

    /// <summary>The outline colour a style states, or null when it states <c>draw:stroke="none"</c>.</summary>
    public Colour? Line(string? name)
    {
        XElement? graphic = Properties(name, "graphic-properties");
        if (graphic is null) return null;
        if (Attribute(graphic, OdfNamespaces.Draw, "stroke") == "none") return null;

        return OdfValue.ParseColour(Attribute(graphic, OdfNamespaces.SvgCompatible, "stroke-color"))
            ?? Colour.Black;
    }

    /// <summary>
    /// Whether a style asks for a stroke at all.
    /// </summary>
    /// <remarks>
    /// <see cref="Line"/> cannot answer this: it returns null both for "no stroke" and for
    /// "no graphic properties whatever", and those mean opposite things to a gridline — the first
    /// turns it off and the second leaves it at chart2's default grey.
    /// </remarks>
    /// <param name="name">The style's name.</param>
    public bool HasStroke(string? name)
        => Attribute(Properties(name, "graphic-properties"), OdfNamespaces.Draw, "stroke") != "none";

    /// <summary>The outline width, zero for a hairline.</summary>
    public Length LineWidth(string? name)
    {
        XElement? graphic = Properties(name, "graphic-properties");
        return OdfValue.ParseLength(Attribute(graphic, OdfNamespaces.SvgCompatible, "stroke-width"))
            ?? Length.Zero;
    }

    /// <summary>A numeric chart property, such as <c>chart:gap-width</c>.</summary>
    public double? Number(string? name, string property)
        => OdfValue.ParseDouble(
            Attribute(Properties(name, "chart-properties"), OdfNamespaces.Chart, property));

    /// <summary>A boolean chart property, such as <c>chart:stacked</c>.</summary>
    public bool? Flag(string? name, string property)
        => OdfValue.ParseBoolean(
            Attribute(Properties(name, "chart-properties"), OdfNamespaces.Chart, property));

    /// <summary>
    /// Whether the plot area's bars run horizontally.
    /// </summary>
    /// <remarks>
    /// <c>chart:vertical="true"</c> means the <em>value</em> axis is horizontal — that is, the
    /// bars lie on their side — and it defaults to false, which is the ordinary column chart.
    /// The naming is ODF's and it reads backwards; getting it the wrong way round transposes
    /// every chart in the corpus at once, which is at least obvious.
    /// </remarks>
    public bool IsVertical(string? name) => Flag(name, "vertical") ?? false;

    /// <summary>The font size a style states, or null when it states none.</summary>
    public Length? FontSize(string? name)
        => OdfValue.ParseLength(
            Attribute(Properties(name, "text-properties"), OdfNamespaces.FoCompatible, "font-size"));

    private XElement? Properties(string? name, string kind)
    {
        if (name is null || !_styles.TryGetValue(name, out XElement? style)) return null;
        return style.Element(XName.Get(kind, OdfNamespaces.Style));
    }

    private static string? Attribute(XElement? element, string ns, string name)
        => element?.Attribute(XName.Get(name, ns))?.Value;
}
