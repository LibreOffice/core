using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Ooxml;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;

namespace XRay.Office.Presentations.Ooxml;

/// <summary>
/// The PPTX layout's chart half: a <c>p:graphicFrame</c> holding a <c>c:chart</c>.
/// </summary>
/// <remarks>
/// A separate file rather than another method in the layout, because a chart is reached exactly
/// as a table is — the frame's <c>p:xfrm</c> and its <c>a:graphicData/@uri</c> — and everything
/// after that is a different subsystem. Keeping the two apart is what lets the whole chart path
/// be read, and reverted, without touching the shape walk.
/// </remarks>
internal sealed partial class PptxSlideLayout
{
    /// <summary>
    /// The shapes a graphic frame holding a chart draws, or nothing when it holds something else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The frame carries only <c>c:chart/@r:id</c>; the chart is a part of its own, resolved
    /// against the <em>slide's</em> relationships. Reading it through
    /// <see cref="DrawingChartPlot"/> rather than <see cref="DrawingChart"/> is deliberate: this
    /// path wants the fills, the gap width and the axis scaling, and that one wants the cached
    /// strings — see the remarks on <see cref="DrawingChartPlot"/> for why the two readers are
    /// not one.
    /// </para>
    /// <para>
    /// A chart part that will not resolve, or one whose plot area holds no series, draws nothing
    /// at all — which leaves the slide exactly as it was before charts were drawn rather than
    /// leaving an empty rectangle where the reference draws a picture.
    /// </para>
    /// </remarks>
    private List<PlacedShape> Chart(
        XElement frame, PptxSlide slide, SlideTheme theme, AffineTransform space)
    {
        XElement? graphic = Drawing.Child(Drawing.Child(frame, "graphic"), "graphicData");
        if (Drawing.Attribute(graphic, "uri") != DrawingChart.ChartUri) return [];

        XName chart = XName.Get("chart", OoxmlNamespaces.DrawingMLChart);
        string? relationshipId = graphic!.Element(chart)
            ?.Attribute(XName.Get("id", OoxmlNamespaces.Relationships))?.Value;

        if (_file.Relationship(slide.PartName, relationshipId) is not { IsExternal: false } link)
            return [];
        if (_file.Load(link.Target) is not { } chartSpace) return [];

        // A chart part may bring a theme of its own, and where it does that theme wins outright.
        SlideTheme charted = ChartTheme(link.Target, theme);

        // The theme's format matrix goes with the colour scheme, because a chart's automatic
        // series formatting needs both: the accent cycle for the colour and `a:lnStyleLst`'s
        // first entry for the width. This is the route that was missing — both halves have been
        // read for rounds and nothing joined them, so every series stating no `c:spPr` was drawn
        // black at a hairline.
        if (DrawingChartPlot.Read(
                chartSpace, charted.Colours, _file.IsOffice2007, charted.Styles)
            is not { } plot)
        {
            return [];
        }

        // A frame's transform is p:xfrm — PresentationML's own element with DrawingML's a:off and
        // a:ext inside it — rather than the a:xfrm a shape carries. Reading it with the drawing
        // namespace finds nothing and puts every chart at the slide's top-left corner at no size.
        XElement? transform = Ppt.Child(frame, "xfrm");
        DocRect local = Bounds(transform);
        if (local.Width <= Core.Units.Length.Zero || local.Height <= Core.Units.Length.Zero)
            return [];

        AffineTransform placement = ShapeTransform.Place(
            local,
            ShapeTransform.Radians(Rotation(transform)),
            Drawing.Flag(transform, "flipH") ?? false,
            Drawing.Flag(transform, "flipV") ?? false,
            space);

        List<PlacedShape> drawn =
            SlideChart.Place(plot, local.Size, placement, _fonts, Name(frame));

        drawn.AddRange(UserShapes(link.Target, slide, charted, local.Size, placement));
        return drawn;
    }

    /// <summary>
    /// The <c>chartUserShapes</c> part's shapes, placed over the chart, or nothing when the chart
    /// declares none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart's annotations are a separate part and are not in the chart at all.</strong>
    /// <c>c:userShapes</c> names a <c>chartDrawing</c> part holding <c>cdr:relSizeAnchor</c> and
    /// <c>cdr:absSizeAnchor</c> elements, each wrapping an ordinary DrawingML shape. On
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c>'s page 8 that part's entire content is the
    /// four labels <c>88%</c>, <c>(548/621)</c>, <c>72%</c> and <c>(317/439)</c> — every token the
    /// reference draws there and we did not, and none of them anywhere else in the package.
    /// </para>
    /// <para>
    /// <strong>The anchor places the shape and its own <c>a:xfrm</c> does not.</strong> The
    /// <c>a:off</c>/<c>a:ext</c> inside <c>cdr:spPr</c> is in the coordinate space of whatever
    /// document the chart came from — 2071670 EMU on a chart frame 5.6 million EMU wide here —
    /// while the anchor's <c>cdr:from</c>/<c>cdr:to</c> are fractions of the frame. LibreOffice
    /// uses the anchor (<c>oox/source/drawingml/chartspaceconverter.cxx</c> converts the user
    /// shapes against the chart rectangle), so the synthesised <c>a:xfrm</c> below is built from
    /// the fractions and the frame's size.
    /// </para>
    /// <para>
    /// Reading them by rewriting each <c>cdr:sp</c> into the <c>p:sp</c> the shape walk already
    /// understands, rather than by writing a second shape reader: the children of
    /// <c>cdr:spPr</c> and <c>cdr:txBody</c> are DrawingML already and only the three wrappers
    /// carry the <c>cdr</c> namespace, so the rewrite is three elements deep and everything a
    /// slide shape can express keeps working — fills, outlines, autofit, the lot.
    /// </para>
    /// </remarks>
    /// <param name="chartPartName">The chart part, whose relationships name the drawing.</param>
    /// <param name="slide">The slide, for the placeholder and fill context the shape walk wants.</param>
    /// <param name="theme">The theme the chart is read against, override included.</param>
    /// <param name="size">The chart frame's size, which the anchors' fractions are of.</param>
    /// <param name="placement">The transform from frame coordinates onto the page.</param>
    private List<PlacedShape> UserShapes(
        string chartPartName,
        PptxSlide slide,
        SlideTheme theme,
        DocSize size,
        AffineTransform placement)
    {
        List<PlacedShape> shapes = [];

        if (_file.TargetOfType(chartPartName, "chartUserShapes") is not { } part) return shapes;
        if (_file.Load(part) is not { } root) return shapes;

        foreach (XElement anchor in root.Elements())
        {
            if (anchor.Name.NamespaceName != OoxmlNamespaces.ChartDrawing) continue;

            bool relative = anchor.Name.LocalName == "relSizeAnchor";
            if (!relative && anchor.Name.LocalName != "absSizeAnchor") continue;

            if (Anchored(anchor, relative, size) is not { } bounds) continue;

            foreach (XElement shape in anchor.Elements(
                         XName.Get("sp", OoxmlNamespaces.ChartDrawing)))
            {
                if (Rewritten(shape, bounds) is not { } rewritten) continue;
                if (Shape(rewritten, slide, theme, placement) is { } placed)
                    Add(placed, shapes);
            }
        }

        return shapes;
    }

    /// <summary>
    /// The rectangle an anchor names inside the chart frame, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Both anchors state <c>cdr:from</c> as fractions of the frame; a <c>relSizeAnchor</c> states
    /// <c>cdr:to</c> as fractions too and an <c>absSizeAnchor</c> states a <c>cdr:ext</c> in EMU.
    /// A reversed pair is normalised rather than dropped, because a negative extent is what a
    /// zero-size shape would come out as.
    /// </remarks>
    private static DocRect? Anchored(XElement anchor, bool relative, DocSize size)
    {
        XElement? from = anchor.Element(XName.Get("from", OoxmlNamespaces.ChartDrawing));
        if (Fraction(from, "x") is not { } left || Fraction(from, "y") is not { } top) return null;

        double right;
        double bottom;

        if (relative)
        {
            XElement? to = anchor.Element(XName.Get("to", OoxmlNamespaces.ChartDrawing));
            if (Fraction(to, "x") is not { } x || Fraction(to, "y") is not { } y) return null;
            right = x;
            bottom = y;
        }
        else
        {
            XElement? extent = anchor.Element(XName.Get("ext", OoxmlNamespaces.ChartDrawing));
            if (extent is null || size.Width <= Length.Zero || size.Height <= Length.Zero)
                return null;

            right = left + Emu(extent, "cx") / (double)size.Width.Emu;
            bottom = top + Emu(extent, "cy") / (double)size.Height.Emu;
        }

        if (right < left) (left, right) = (right, left);
        if (bottom < top) (top, bottom) = (bottom, top);

        return new DocRect(
            Length.FromEmu((long)Math.Round(left * size.Width.Emu)),
            Length.FromEmu((long)Math.Round(top * size.Height.Emu)),
            Length.FromEmu((long)Math.Round((right - left) * size.Width.Emu)),
            Length.FromEmu((long)Math.Round((bottom - top) * size.Height.Emu)));
    }

    /// <summary>A <c>cdr:x</c>-style child read as a fraction, or null when it is absent.</summary>
    private static double? Fraction(XElement? parent, string localName)
        => parent?.Element(XName.Get(localName, OoxmlNamespaces.ChartDrawing))?.Value
           is { Length: > 0 } text
           && double.TryParse(
               text, System.Globalization.NumberStyles.Float,
               System.Globalization.CultureInfo.InvariantCulture, out double value)
            ? value
            : null;

    /// <summary>
    /// A <c>cdr:sp</c> rewritten as the <c>p:sp</c> the shape walk reads, at a stated rectangle.
    /// </summary>
    /// <remarks>
    /// Only the three <c>cdr</c> wrappers are renamed and the <c>a:xfrm</c> is replaced; every
    /// other child is reused as it stands, because it is DrawingML already. The new element is a
    /// copy rather than a move, so the loaded part is left intact for the next slide that shares
    /// this chart.
    /// </remarks>
    private static XElement? Rewritten(XElement shape, DocRect bounds)
    {
        XElement? properties = shape.Element(XName.Get("spPr", OoxmlNamespaces.ChartDrawing));
        XElement? body = shape.Element(XName.Get("txBody", OoxmlNamespaces.ChartDrawing));
        XElement? visual = shape.Element(XName.Get("nvSpPr", OoxmlNamespaces.ChartDrawing));

        if (properties is null && body is null) return null;

        XElement transform = new(
            Drawing.Name("xfrm"),
            new XElement(
                Drawing.Name("off"),
                new XAttribute("x", bounds.X.Emu),
                new XAttribute("y", bounds.Y.Emu)),
            new XElement(
                Drawing.Name("ext"),
                new XAttribute("cx", bounds.Width.Emu),
                new XAttribute("cy", bounds.Height.Emu)));

        List<object> shapeProperties = [transform];
        foreach (XElement child in properties?.Elements() ?? [])
        {
            if (child.Name == Drawing.Name("xfrm")) continue;
            shapeProperties.Add(new XElement(child));
        }

        return new XElement(
            Ppt.Name("sp"),
            new XElement(
                Ppt.Name("nvSpPr"),
                new XElement(
                    Ppt.Name("cNvPr"),
                    new XAttribute(
                        "name",
                        Drawing.Attribute(
                            visual?.Element(XName.Get("cNvPr", OoxmlNamespaces.ChartDrawing)),
                            "name")
                        ?? string.Empty))),
            new XElement(Ppt.Name("spPr"), shapeProperties),
            body is null
                ? null
                : new XElement(Ppt.Name("txBody"), body.Elements().Select(one => new XElement(one))));
    }

    /// <summary>
    /// The theme a chart part is read against: its own <c>themeOverride</c> where it has one, and
    /// the slide's otherwise.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart embedded from another document keeps that document's theme.</strong> The
    /// chart part declares a <c>themeOverride</c> relationship to an <c>a:themeOverride</c> part
    /// holding a whole colour, font and format scheme, and
    /// <c>ChartSpaceFragment::openFragmentStream</c> pushes it onto the filter so that every
    /// <c>a:schemeClr</c> and every <c>+mn-lt</c> inside the chart resolves against it instead of
    /// against the deck's (<c>oox/source/drawingml/chart/chartspacefragment.cxx</c>).
    /// </para>
    /// <para>
    /// It was found from the other end. A blind reviewer sent
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c>'s page 8, with no access to the package,
    /// reported the reference's chart text as a <em>serif</em> face against our sans and named "a
    /// theme major/minor latin font not being applied" as its first candidate. The part's
    /// <c>a:minorFont</c> is Palatino Linotype and the deck's is Calibri.
    /// </para>
    /// <para>
    /// The colour <em>map</em> is not overridden with it. The override part carries a scheme and
    /// no map — <c>c:clrMapOvr</c> on the chart space is where a chart states one — so the
    /// slide's map is kept, which is what makes <c>bg1</c> go on meaning the same thing inside
    /// the chart as outside it.
    /// </para>
    /// </remarks>
    /// <param name="chartPartName">The chart part, whose relationships are searched.</param>
    /// <param name="slideTheme">The theme in force on the slide.</param>
    private SlideTheme ChartTheme(string chartPartName, SlideTheme slideTheme)
    {
        if (_file.TargetOfType(chartPartName, "themeOverride") is not { } part) return slideTheme;
        if (_file.Load(part) is not { } root) return slideTheme;
        if (DrawingTheme.Read(root) is not { Colours: not null } overridden) return slideTheme;

        XElement? minor = Drawing.Child(
            Drawing.Child(Drawing.Child(root, "fontScheme"), "minorFont"), "latin");

        return new SlideTheme(
            overridden.WithMap(slideTheme.Colours?.Map),
            Drawing.Attribute(minor, "typeface") ?? slideTheme.MinorLatin,
            DrawingStyleMatrix.Read(root) ?? slideTheme.Styles);
    }
}
