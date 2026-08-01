using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Reads a <c>c:chartSpace</c> into the model a renderer draws — <see cref="ChartPlot"/>.
/// </summary>
/// <remarks>
/// <para>
/// The drawing counterpart to <see cref="DrawingChart"/>, which reads the same part into the
/// content tree. Two readers over one part rather than one reader with two outputs, because they
/// want disjoint halves of it: this one needs the fills, the gap width, the axis scaling and the
/// legend position and never looks at a formula; that one needs the cached strings and never
/// looks at an <c>a:solidFill</c>. Extraction is the common case and must not pay for geometry.
/// </para>
/// <para>
/// <strong>Only a bar or column chart, and everything else reads as null.</strong> The layout
/// engine draws rectangles against a category axis and a value axis; a pie chart has neither,
/// and a chart part is not obliged to say so in any way a suffix match can see. Matching
/// <c>c:barChart</c> and <c>c:bar3DChart</c> by name rather than taking the first
/// <c>…Chart</c> group is what stops a pie being drawn as eight bars under two axes that do not
/// exist. Measured over LibreOffice's own <c>chart2/qa/extras/data/pptx/</c>: the loose match
/// drew <em>82 words</em> of axis labels onto
/// <c>PieChartWithAutomaticLayout_SizeAndPosition.pptx</c>, against a reference that draws one.
/// </para>
/// <para>
/// A part holding several groups — a column chart with a line series over it writes a
/// <c>c:barChart</c> and a <c>c:lineChart</c> sharing an axis — draws its bars and drops its
/// line. That is visibly incomplete rather than subtly wrong, which is the failure mode to
/// prefer. <see cref="DrawingChart"/> still reads every group of every type, so the content tree
/// holds all the numbers whatever gets drawn: a chart type that is not drawn loses its picture
/// and not its data.
/// </para>
/// </remarks>
public static class DrawingChartPlot
{
    /// <summary>How many <c>c:pt</c> a cache is trusted to declare.</summary>
    /// <remarks>The same ceiling <see cref="DrawingChart"/> applies, for the same reason.</remarks>
    private const int MaxPointCount = 65536;

    /// <summary>
    /// Reads a chart part's geometry, or null when there is nothing to draw.
    /// </summary>
    /// <param name="chartSpace">The <c>c:chartSpace</c> root, or the <c>c:chart</c> inside it.</param>
    /// <param name="theme">The theme, for resolving a <c>a:schemeClr</c> fill.</param>
    public static ChartPlot? Read(XElement chartSpace, DrawingTheme? theme = null)
    {
        ArgumentNullException.ThrowIfNull(chartSpace);

        XElement? chart = Is(chartSpace, "chart") ? chartSpace : Child(chartSpace, "chart");
        if (chart is null) return null;

        XElement? plotArea = Child(chart, "plotArea");
        if (plotArea is null) return null;

        XElement? group = null;
        foreach (XElement candidate in plotArea.Elements())
        {
            if (candidate.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (candidate.Name.LocalName is not ("barChart" or "bar3DChart")) continue;
            group = candidate;
            break;
        }

        if (group is null) return null;

        (List<ChartSeries> series, string?[] categories) = ReadSeries(group, theme);
        if (series.Count == 0) return null;

        string? grouping = Value(Child(group, "grouping"));
        DocRect? manual = ManualLayout(Child(plotArea, "layout"));

        return new ChartPlot
        {
            Title = TitleText(Child(chart, "title")),
            CategoryAxisTitle = AxisTitle(plotArea, "catAx") ?? AxisTitle(plotArea, "dateAx"),
            ValueAxisTitle = AxisTitle(plotArea, "valAx"),
            Categories = categories,
            Series = series,
            Direction = Value(Child(group, "barDir")) == "bar"
                ? ChartBarDirection.Bar
                : ChartBarDirection.Column,

            // c:gapWidth and c:overlap default to 150 and 0 in the schema, but LibreOffice's
            // importer defaults them to 100 and 0 (oox/source/drawingml/chart/typegroupmodel.cxx)
            // and every file the corpus holds states them. 100 is used here so that a part that
            // omits them agrees with the reference rather than with the specification.
            GapWidth = Number(Child(group, "gapWidth")) ?? 100.0,
            Overlap = Number(Child(group, "overlap")) ?? 0.0,
            IsStacked = grouping is "stacked" or "percentStacked",
            ValueScale = ScaleOf(plotArea),
            Legend = LegendOf(Child(chart, "legend")),
            Background = FillOf(Child(chartSpace, "spPr"), theme),
            PlotBackground = FillOf(Child(plotArea, "spPr"), theme),
            TitleSize = SizeOf(Child(chart, "title")) ?? Length.FromPoints(13),
            AxisTitleSize = AxisTitleSizeOf(plotArea) ?? Length.FromPoints(9),
            LabelSize = AxisLabelSizeOf(plotArea) ?? Length.FromPoints(10),
            PlotArea = manual,

            // Only set beside a manual layout, because it is the space that layout's fractions
            // were expressed in. An OOXML chart otherwise has no coordinate space of its own —
            // the frame is the space — so leaving it null is what makes the computed path use
            // the frame directly.
            Space = manual is null ? null : UnitSquare,
        };
    }

    /// <summary>
    /// The plot rectangle a <c>c:manualLayout</c> states, as a fraction of the frame.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Written only when the author dragged the plot area; an automatically laid-out chart has no
    /// <c>c:layout</c> content at all, which is the case for every OOXML chart in the corpus and
    /// for 0 of the 192 chart documents in LibreOffice's own <c>chart2/qa/extras/data/</c> that
    /// were checked. So this is the rare path and the computed layout is the common one — the
    /// reverse of ODF, where <c>chart:coordinate-region</c> is always written.
    /// </para>
    /// <para>
    /// <c>c:layoutTarget val="inner"</c> means the rectangle is the plot area proper; the default,
    /// <c>outer</c>, means it includes the axis labels. Only <c>inner</c> is honoured, because an
    /// outer rectangle needs the label sizes subtracted from it and that is the computation this
    /// was meant to avoid — an outer layout falls back to the computed one, which is at worst as
    /// wrong as it would have been.
    /// </para>
    /// </remarks>
    private static DocRect? ManualLayout(XElement? layout)
    {
        XElement? manual = Child(layout, "manualLayout");
        if (manual is null) return null;
        if (Value(Child(manual, "layoutTarget")) != "inner") return null;

        if (Number(Child(manual, "x")) is not { } x) return null;
        if (Number(Child(manual, "y")) is not { } y) return null;
        if (Number(Child(manual, "w")) is not { } width) return null;
        if (Number(Child(manual, "h")) is not { } height) return null;

        // The fractions are of the chart *frame*, which the layout maps through Space. Expressing
        // them against a notional unit square keeps this reader from needing to know the frame.
        return new DocRect(
            Length.FromEmu((long)(x * UnitSpace)),
            Length.FromEmu((long)(y * UnitSpace)),
            Length.FromEmu((long)(width * UnitSpace)),
            Length.FromEmu((long)(height * UnitSpace)));
    }

    /// <summary>The notional space a fractional manual layout is expressed in.</summary>
    /// <remarks>
    /// One inch, chosen only because it is exactly representable in EMUs and large enough that a
    /// fraction keeps five decimal places of resolution. <see cref="ChartPlot.Space"/> is set to
    /// the same square, so the layout scales it onto the real frame.
    /// </remarks>
    private const long UnitSpace = 914400;

    /// <summary>The square a fractional manual layout is expressed in.</summary>
    public static DocSize UnitSquare { get; } =
        new(Length.FromEmu(UnitSpace), Length.FromEmu(UnitSpace));

    /// <summary>What the value axis states about its scale.</summary>
    /// <remarks>
    /// From the first <c>c:valAx</c>, which is the primary one. A chart with a secondary value
    /// axis states a second, and its series are not drawn against this scale — but nothing here
    /// draws a secondary axis yet, so reading the first is what matches what is drawn.
    /// </remarks>
    private static ChartScaleRequest ScaleOf(XElement plotArea)
    {
        XElement? axis = null;
        foreach (XElement candidate in Children(plotArea, "valAx")) { axis = candidate; break; }
        if (axis is null) return default;

        XElement? scaling = Child(axis, "scaling");

        return new ChartScaleRequest(
            Number(Child(scaling, "min")),
            Number(Child(scaling, "max")),
            Number(Child(axis, "majorUnit")),
            Value(Child(scaling, "orientation")) == "maxMin");
    }

    private static ChartLegendPosition LegendOf(XElement? legend)
        => legend is null
            ? ChartLegendPosition.None
            : Value(Child(legend, "legendPos")) switch
            {
                "l" => ChartLegendPosition.Left,
                "t" => ChartLegendPosition.Top,
                "b" => ChartLegendPosition.Bottom,
                "tr" => ChartLegendPosition.Right,

                // c:legendPos is optional and its default is "r", so a c:legend with nothing in
                // it still draws a legend on the right.
                _ => ChartLegendPosition.Right,
            };

    private static (List<ChartSeries> Series, string?[] Categories) ReadSeries(
        XElement group, DrawingTheme? theme)
    {
        List<ChartSeries> series = [];
        string?[] categories = [];

        foreach (XElement element in Children(group, "ser"))
        {
            (string?[] labels, _) = ReadSequence(Child(element, "cat") ?? Child(element, "xVal"));
            if (categories.Length == 0 && labels.Length > 0) categories = labels;

            (_, double?[] numbers) = ReadSequence(Child(element, "val") ?? Child(element, "yVal"));

            XElement? properties = Child(element, "spPr");

            series.Add(new ChartSeries(
                DrawingChartText.Label(Child(element, "tx")),
                numbers,
                FillOf(properties, theme),
                LineOf(properties, theme),
                LineWidthOf(properties)));
        }

        return (series, categories);
    }

    /// <summary>A shape property bag's solid fill, or null when it has none.</summary>
    /// <remarks>
    /// Only <c>a:solidFill</c>. A gradient or picture fill on a bar is legal and rare; drawing
    /// nothing for one leaves the bar's outline, which reads as "something is here and it is not
    /// coloured in" rather than as a missing bar.
    /// </remarks>
    private static Colour? FillOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? fill = Drawing.Child(properties, "solidFill");
        if (fill is null) return null;

        foreach (XElement child in fill.Elements())
            if (DrawingColour.Read(child) is { } colour) return colour.Resolve(theme);

        return null;
    }

    private static Colour? LineOf(XElement? properties, DrawingTheme? theme)
    {
        XElement? line = Drawing.Child(properties, "ln");
        if (line is null) return null;
        if (Drawing.Child(line, "noFill") is not null) return null;
        return FillOf(line, theme);
    }

    /// <summary>
    /// A line's width, or zero when it states none.
    /// </summary>
    /// <remarks>
    /// Zero is not "no line": <c>a:ln w="0"</c> is what LibreOffice's own export writes for a
    /// hairline, and it appears on every bar of the corpus chart. The renderer draws a zero-width
    /// stroke as the thinnest the device can, which is what the reference PDF does with
    /// <c>0 w</c>.
    /// </remarks>
    private static Length LineWidthOf(XElement? properties)
    {
        XElement? line = Drawing.Child(properties, "ln");
        return Drawing.Number(line, "w") is { } emu && emu > 0
            ? Length.FromEmu(emu)
            : Length.Zero;
    }

    /// <summary>
    /// The size a titled element's text is set at, from the first <c>a:defRPr</c> or
    /// <c>a:rPr</c> under it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>@sz</c> is in hundredths of a point — <c>sz="1300"</c> is thirteen points — and it may
    /// sit on either the paragraph's default run properties or an individual run's. Taking the
    /// first of either in document order gets the common case, which is a chart whose title is
    /// one run and states the same size in both places.
    /// </para>
    /// <para>
    /// This is read because it decides layout, not appearance. The main title's height is
    /// subtracted from the top of the chart before the plot area is placed, so assuming ten
    /// points where the file says thirteen puts the plot area — and therefore every bar's base —
    /// several points too high.
    /// </para>
    /// </remarks>
    private static Length? SizeOf(XElement? element)
    {
        if (element is null) return null;

        foreach (XElement properties in element.Descendants())
        {
            if (properties.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;
            if (properties.Name.LocalName is not ("defRPr" or "rPr")) continue;
            if (Drawing.Number(properties, "sz") is not { } hundredths || hundredths <= 0) continue;

            return Length.FromPoints(hundredths / 100.0);
        }

        return null;
    }

    private static Length? AxisTitleSizeOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (SizeOf(Child(axis, "title")) is { } size) return size;
        }

        return null;
    }

    /// <summary>The size the axis <em>labels</em> are set at — <c>c:txPr</c>, not the title's.</summary>
    private static Length? AxisLabelSizeOf(XElement plotArea)
    {
        foreach (XElement axis in plotArea.Elements())
        {
            if (axis.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!axis.Name.LocalName.EndsWith("Ax", StringComparison.Ordinal)) continue;
            if (SizeOf(Child(axis, "txPr")) is { } size) return size;
        }

        return null;
    }

    private static string? AxisTitle(XElement plotArea, string localName)
    {
        foreach (XElement axis in Children(plotArea, localName))
            if (TitleText(Child(axis, "title")) is { Length: > 0 } text) return text;

        return null;
    }

    private static string? TitleText(XElement? title)
    {
        if (title is null) return null;

        XElement? tx = Child(title, "tx");

        if (Child(tx, "rich") is { } rich && DrawingTextBody.Text(rich) is { Length: > 0 } text)
            return text;

        if (Child(title, "txPr") is { } properties
            && DrawingTextBody.Text(properties) is { Length: > 0 } fallback)
        {
            return fallback;
        }

        return DrawingChartText.Label(tx);
    }

    /// <summary>Reads a cached data sequence, sparse indices and all.</summary>
    /// <remarks>
    /// The same rule <see cref="DrawingChart"/> documents at length: the array is sized from
    /// <c>c:ptCount</c> and every point placed at its own <c>@idx</c>, because the indices skip
    /// blanks and reading in document order slides every later value onto the wrong category. A
    /// chart drawn that way has the right bars against the wrong labels and looks entirely
    /// plausible.
    /// </remarks>
    private static (string?[] Text, double?[] Numbers) ReadSequence(XElement? source)
    {
        if (source is null) return ([], []);

        XElement? cache =
            Child(Child(source, "strRef"), "strCache")
            ?? Child(source, "strLit")
            ?? Child(Child(source, "numRef"), "numCache")
            ?? Child(source, "numLit")
            ?? Child(Child(source, "multiLvlStrRef"), "multiLvlStrCache");

        if (cache is null) return ([], []);

        int declared = Drawing.Number(Child(cache, "ptCount"), "val") ?? -1;
        if (declared < 0)
        {
            foreach (XElement point in cache.Descendants(Name("pt")))
                declared = Math.Max(declared, (Drawing.Number(point, "idx") ?? -1) + 1);
        }

        int count = Math.Clamp(declared, 0, MaxPointCount);
        string?[] text = new string?[count];
        double?[] numbers = new double?[count];

        foreach (XElement point in cache.Descendants(Name("pt")))
        {
            int index = Drawing.Number(point, "idx") ?? -1;
            if (index < 0 || index >= count) continue;

            string value = Child(point, "v")?.Value ?? string.Empty;
            text[index] = value;
            numbers[index] = double.TryParse(
                value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
                ? parsed
                : null;
        }

        return (text, numbers);
    }

    private static double? Number(XElement? element)
    {
        string? value = Drawing.Attribute(element, "val");
        return double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;
    }

    private static string? Value(XElement? element) => Drawing.Attribute(element, "val");

    private static XName Name(string localName)
        => XName.Get(localName, OoxmlNamespaces.DrawingMLChart);

    private static XElement? Child(XElement? element, string localName)
        => element?.Element(Name(localName));

    private static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(Name(localName)) ?? [];

    private static bool Is(XElement element, string localName)
        => element.Name.NamespaceName == OoxmlNamespaces.DrawingMLChart
           && element.Name.LocalName == localName;
}

/// <summary>
/// The one piece of chart-part text reading both chart readers need.
/// </summary>
/// <remarks>
/// Factored out rather than duplicated because a series' label is the one thing the content
/// reader and the drawing reader genuinely share, and because getting it wrong in one of them
/// produces a chart whose legend and whose table disagree about what a series is called.
/// </remarks>
internal static class DrawingChartText
{
    /// <summary>
    /// A series' or title's label, from a <c>c:tx</c>-shaped element.
    /// </summary>
    /// <remarks>
    /// <c>CT_SerTx</c> is a choice of <c>c:strRef</c> and a bare <c>c:v</c>, and a reference's
    /// cache may hold several points when the label spans cells. LibreOffice joins those with one
    /// space and keeps a single label — "the internal data table does not support complex labels"
    /// (<c>oox/source/drawingml/chart/datasourceconverter.cxx:50-73</c>).
    /// </remarks>
    internal static string? Label(XElement? source)
    {
        if (source is null) return null;

        XName v = XName.Get("v", OoxmlNamespaces.DrawingMLChart);
        if (source.Element(v) is { } literal && literal.Value.Length > 0) return literal.Value;

        System.Text.StringBuilder joined = new();
        XName pt = XName.Get("pt", OoxmlNamespaces.DrawingMLChart);

        foreach (XElement point in source.Descendants(pt))
        {
            if (point.Element(v)?.Value is not { Length: > 0 } value) continue;
            if (joined.Length > 0) joined.Append(' ');
            joined.Append(value);
        }

        return joined.Length == 0 ? null : joined.ToString();
    }
}
