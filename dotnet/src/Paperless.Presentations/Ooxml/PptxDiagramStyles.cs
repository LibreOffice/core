using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// The quick style and colour transform, resolved into the fill, line and text colour a node draws.
/// </summary>
/// <remarks>
/// <para>
/// A diagram's appearance is stated in two parts and neither is enough alone. The quick style
/// (<c>quickStyle1.xml</c>) says <em>which</em> of the theme's three fill styles and three line
/// styles a labelled node uses — <c>a:fillRef idx="1"</c> — and the colour transform
/// (<c>colors1.xml</c>) says what colour to substitute for the <c>phClr</c> those theme styles
/// are written in terms of. Read one without the other and every node comes out the theme's
/// first accent, whatever colour scheme the author picked.
/// </para>
/// <para>
/// <strong>The substitution is done in XML rather than in a colour model</strong>, by cloning
/// the theme's style element and replacing each <c>a:schemeClr val="phClr"</c> with the resolved
/// colour. That keeps whatever the theme wrapped around the placeholder — a
/// <c>lumMod</c>/<c>lumOff</c> pair, a gradient's stop list, a line's width and dash — and
/// hands the result to the same fill and line readers the rest of a slide uses, so a diagram's
/// gradient is read by the code that reads a slide's gradient rather than by a second copy of
/// it.
/// </para>
/// <para>
/// Measured over the 66 diagram-bearing decks in <c>sd/qa/unit/data/pptx</c>: of 3 919
/// <c>a:fillRef</c> in their quick styles 3 098 are index 1, 487 are index 0 (no fill) and the
/// remaining 334 are 2 or 3; and the first entry of <c>a:fillStyleLst</c> is
/// <c>solidFill(phClr)</c> in every single theme. So the common case is a plain solid fill in
/// the colour list's colour, and the general path exists for the other 334.
/// </para>
/// </remarks>
internal sealed class PptxDiagramStyles
{
    /// <summary>One <c>dgm:styleLbl</c> of the quick style: four indices into the theme.</summary>
    private readonly record struct StyleReference(int Line, int Fill, string FontIndex);

    /// <summary>One <c>dgm:styleLbl</c> of the colour transform: the lists it cycles over.</summary>
    private sealed record ColourLists
    {
        public List<XElement> Fill { get; } = [];

        public List<XElement> Line { get; } = [];

        public List<XElement> TextFill { get; } = [];
    }

    private readonly Dictionary<string, StyleReference> _styles = new(StringComparer.Ordinal);
    private readonly Dictionary<string, ColourLists> _colours = new(StringComparer.Ordinal);
    private readonly List<XElement> _fillStyles = [];
    private readonly List<XElement> _lineStyles = [];
    private readonly DrawingTheme? _theme;

    private PptxDiagramStyles(DrawingTheme? theme) => _theme = theme;

    /// <summary>Reads the two diagram parts and the theme's format scheme beside them.</summary>
    /// <param name="quickStyle">The <c>dgm:styleDef</c> root, or null.</param>
    /// <param name="colours">The <c>dgm:colorsDef</c> root, or null.</param>
    /// <param name="theme">The <c>a:theme</c> root, whose <c>a:fmtScheme</c> the indices name.</param>
    /// <param name="resolved">The theme as a colour model, for resolving the colour lists.</param>
    public static PptxDiagramStyles Read(
        XElement? quickStyle, XElement? colours, XElement? theme, DrawingTheme? resolved)
    {
        PptxDiagramStyles styles = new(resolved);

        foreach (XElement label in Labels(quickStyle))
        {
            if (label.Attribute("name")?.Value is not { } name) continue;

            XElement? style = label.Element(XName.Get("style", PptxDiagram.Uri));
            if (style is null) continue;

            styles._styles[name] = new StyleReference(
                Index(Drawing.Child(style, "lnRef")),
                Index(Drawing.Child(style, "fillRef")),
                Drawing.Attribute(Drawing.Child(style, "fontRef"), "idx") ?? "minor");
        }

        foreach (XElement label in Labels(colours))
        {
            if (label.Attribute("name")?.Value is not { } name) continue;

            ColourLists lists = new();
            Collect(label, "fillClrLst", lists.Fill);
            Collect(label, "linClrLst", lists.Line);
            Collect(label, "txFillClrLst", lists.TextFill);
            styles._colours[name] = lists;
        }

        XElement? format = Drawing.Child(Drawing.Child(theme, "themeElements"), "fmtScheme");
        styles._fillStyles.AddRange(Drawing.Child(format, "fillStyleLst")?.Elements() ?? []);
        styles._lineStyles.AddRange(Drawing.Child(format, "lnStyleLst")?.Elements() ?? []);

        return styles;
    }

    /// <summary>The fill a labelled node draws, or null when the style states none.</summary>
    public XElement? Fill(string label, int index)
    {
        if (!_styles.TryGetValue(label, out StyleReference reference)) return null;
        if (reference.Fill <= 0 || reference.Fill > _fillStyles.Count) return null;

        return Substitute(_fillStyles[reference.Fill - 1], Placeholder(label, index, l => l.Fill));
    }

    /// <summary>The <c>a:ln</c> a labelled node draws, or null when the style states none.</summary>
    public XElement? Line(string label, int index)
    {
        if (!_styles.TryGetValue(label, out StyleReference reference)) return null;
        if (reference.Line <= 0 || reference.Line > _lineStyles.Count) return null;

        return Substitute(_lineStyles[reference.Line - 1], Placeholder(label, index, l => l.Line));
    }

    /// <summary>
    /// The <c>p:style</c> that carries a labelled node's text colour and typeface.
    /// </summary>
    /// <remarks>
    /// Emitted as an <c>a:fontRef</c> rather than as a run colour because that is the rung of
    /// the inheritance chain it belongs to: a run that states its own colour still wins, and one
    /// that does not takes this. LibreOffice does the same thing from the other end at
    /// <c>oox/source/drawingml/shape.cxx:2248</c>, where <c>maPhClr</c> becomes the character
    /// fill.
    /// </remarks>
    public XElement? FontReference(string label, int index)
    {
        if (!_styles.TryGetValue(label, out StyleReference reference)) return null;

        XElement font = new(
            Drawing.Name("fontRef"), new XAttribute("idx", reference.FontIndex));

        if (Placeholder(label, index, l => l.TextFill) is { } colour) font.Add(Literal(colour));

        return new XElement(Ppt.Name("style"), font);
    }

    // ------------------------------------------------------------------ parts

    private static IEnumerable<XElement> Labels(XElement? root)
        => root?.Elements(XName.Get("styleLbl", PptxDiagram.Uri)) ?? [];

    private static void Collect(XElement label, string localName, List<XElement> into)
    {
        XElement? list = label.Element(XName.Get(localName, PptxDiagram.Uri));
        if (list is null) return;

        foreach (XElement colour in list.Elements())
        {
            if (colour.Name.NamespaceName == OoxmlNamespaces.DrawingML) into.Add(colour);
        }
    }

    private static int Index(XElement? reference)
        => Drawing.Attribute(reference, "idx") is { } text
           && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : 0;

    /// <summary>
    /// The colour a label's list supplies for the n-th shape carrying that label.
    /// </summary>
    /// <remarks>
    /// The index wraps — <c>DiagramColor::getColorByIndex</c>, <c>diagram.cxx:725</c> — so a
    /// four-colour list across seven nodes repeats from the fifth. It is the presentation
    /// point's own <c>presStyleIdx</c>, not the child's position, which is why a diagram whose
    /// nodes are created out of order still colours them in the author's order.
    /// </remarks>
    private Colour? Placeholder(string label, int index, Func<ColourLists, List<XElement>> which)
    {
        if (!_colours.TryGetValue(label, out ColourLists? lists)) return null;

        List<XElement> list = which(lists);
        if (list.Count == 0) return null;

        return DrawingColour.Read(list[index % list.Count])?.Resolve(_theme);
    }

    /// <summary>Clones a theme style element with its <c>phClr</c> replaced by a literal colour.</summary>
    /// <remarks>
    /// The substitution itself is <see cref="DrawingStyleMatrix"/>'s, because a diagram's quick
    /// style and a shape's <c>p:style</c> index into the same <c>a:fmtScheme</c> and mean the
    /// same thing by it. What stays here is only the diagram's way of choosing the placeholder
    /// colour — a cycling list per label rather than a colour on the reference itself.
    /// </remarks>
    private static XElement? Substitute(XElement element, Colour? placeholder)
        => placeholder is null ? null : DrawingStyleMatrix.Substitute(element, placeholder.Value);

    private static XElement Literal(Colour colour)
        => new(
            Drawing.Name("srgbClr"),
            new XAttribute("val", $"{colour.R:X2}{colour.G:X2}{colour.B:X2}"),
            DrawingStyleMatrix.Alpha(colour));
}
