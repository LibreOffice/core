using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// Which piece of a chart's furniture an automatic <em>line</em> is being asked about.
/// </summary>
/// <remarks>
/// <c>ObjectFormatter</c> keeps one automatic-format table per object kind
/// (<c>oox/source/drawingml/chart/objectformatter.cxx:215-234</c>) and the three that matter to a
/// plot's furniture differ only in the tint they put on <c>tx1</c>. They are named rather than
/// folded into one because the minor grid's table has a different style split — 1…40 and 41…48
/// against the other two's 1…32 and 33…48 — so a single table would be right for two of them and
/// silently wrong for the third above style 32.
/// </remarks>
public enum ChartAutoLine
{
    /// <summary>The axis line itself — <c>spAxisLines</c>.</summary>
    Axis,

    /// <summary>An axis' major gridlines — <c>spMajorGridLines</c>.</summary>
    MajorGrid,

    /// <summary>An axis' minor gridlines — <c>spMinorGridLines</c>.</summary>
    MinorGrid,
}

/// <summary>
/// Which object a chart's automatic formatting is being asked about.
/// </summary>
public enum ChartAutoObject
{
    /// <summary>A series drawn as a line — line, scatter, radar. Its colour is its stroke.</summary>
    LinearSeries,

    /// <summary>A series drawn as an area — bar, pie, area. Its colour is its fill.</summary>
    FilledSeries,
}

/// <summary>
/// Which of a chart's two framed backgrounds an automatic fill is being asked about.
/// </summary>
public enum ChartAutoFrame
{
    /// <summary>The whole chart object's background — <c>spChartSpaceFill</c>.</summary>
    ChartSpace,

    /// <summary>The two-dimensional plot area's wall — <c>spPlotArea2dFills</c>.</summary>
    PlotArea,
}

/// <summary>
/// The colours a chart gives a series that states none — <c>ObjectFormatter</c>'s automatic
/// formatting, ported.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A <c>c:ser</c> with no <c>c:spPr</c> is not a series with no colour.</strong> It is a
/// series whose colour comes from the chart's style index and the document theme's accent cycle,
/// and reading it as "no colour" draws every line in a chart black and leaves every legend key
/// blank. Measured on <c>Demick_JetBlue.pptx</c>, whose three line series state nothing at all
/// and whose reference draws them in the Aspect theme's <c>F07F09</c>, <c>9F2936</c> and
/// <c>1B587C</c> — accent 1, 2 and 3 exactly.
/// </para>
/// <para>
/// <strong>The style index is <c>c:style/@val</c> and its default is 2, not 1.</strong>
/// <c>ChartSpaceModel</c>'s constructor is <c>mnStyle( 2 )</c>
/// (<c>oox/source/drawingml/chart/chartspacemodel.cxx:27</c>), and style 2 is the plain accent
/// cycle — accent 1 to accent 6 with nothing done to them. That is why so many charts come out
/// right with no <c>c:style</c> in the file at all.
/// </para>
/// <para>
/// <strong>The cycle is shaded across cycles rather than across series.</strong>
/// <c>DetailFormatterBase::getPhColor</c> (<c>objectformatter.cxx:766-816</c>) takes
/// <c>pattern[idx % size]</c> and then shades or tints it by which *cycle* the series is in,
/// stepping through <c>[-70%, +70%]</c> in <c>1.4 / (maxCycle + 2)</c> steps. With six accents
/// and six or fewer series that step lands exactly on zero, so the common case is the accents
/// untouched; a single-colour style (3 to 8, one accent for every series) has a cycle size of
/// one and therefore shades every series differently, which is the whole point of those styles.
/// </para>
/// <para>
/// The series index is <c>c:ser/c:idx</c> and the maximum is taken over every type group in the
/// plot area (<c>plotareaconverter.cxx:452-457</c>) — not the series' position in the file, and
/// not a count within one group. A combination chart whose line group states <c>c:idx val="2"</c>
/// takes accent 3 however few series that group holds.
/// </para>
/// </remarks>
public static class DrawingChartAutoFormat
{
    /// <summary>The style index a chart space with no <c>c:style</c> has.</summary>
    /// <remarks><c>ChartSpaceModel::ChartSpaceModel</c>, <c>mnStyle( 2 )</c>.</remarks>
    public const int DefaultStyle = 2;

    /// <summary>
    /// <c>THEMED_STYLE_SUBTLE</c> — the first entry of <c>a:lnStyleLst</c>.
    /// </summary>
    /// <remarks>
    /// <c>include/oox/drawingml/theme.hxx:48</c>. Every automatic series entry names it; the
    /// intense variant appears only in the fill tables for styles 17 and up.
    /// </remarks>
    public const int SubtleStyleIndex = 1;

    /// <summary>Accent 1 to accent 6, untouched — <c>spAutoFormatPattern2</c>.</summary>
    private static readonly (string Colour, string? Modification, int Value)[] AccentPattern =
    [
        ("accent1", null, 0), ("accent2", null, 0), ("accent3", null, 0),
        ("accent4", null, 0), ("accent5", null, 0), ("accent6", null, 0),
    ];

    /// <summary>Six tints of <c>dk1</c> — <c>spAutoFormatPattern1</c>, the greyscale styles.</summary>
    private static readonly (string Colour, string? Modification, int Value)[] GreyPattern =
    [
        ("dk1", "tint", 88500), ("dk1", "tint", 55000), ("dk1", "tint", 78000),
        ("dk1", "tint", 92500), ("dk1", "tint", 70000), ("dk1", "tint", 30000),
    ];

    /// <summary>The same six with a darker first entry — <c>spAutoFormatPattern4</c>.</summary>
    private static readonly (string Colour, string? Modification, int Value)[] DarkGreyPattern =
    [
        ("dk1", "tint", 5000), ("dk1", "tint", 55000), ("dk1", "tint", 78000),
        ("dk1", "tint", 15000), ("dk1", "tint", 70000), ("dk1", "tint", 30000),
    ];

    /// <summary>Six half-shaded accents — <c>spAutoFormatPattern3</c>.</summary>
    private static readonly (string Colour, string? Modification, int Value)[] ShadedAccentPattern =
    [
        ("accent1", "shade", 50000), ("accent2", "shade", 50000), ("accent3", "shade", 50000),
        ("accent4", "shade", 50000), ("accent5", "shade", 50000), ("accent6", "shade", 50000),
    ];

    /// <summary><c>spLinearSeriesLines</c>: a line series' stroke, by chart style.</summary>
    private static readonly AutoFormatEntry[] LinearSeriesLines =
    [
        Pattern(1, 1, 300, GreyPattern),
        Pattern(2, 2, 300, AccentPattern),
        .. FadedAccents(3, 300),
        Pattern(9, 9, 500, GreyPattern),
        Pattern(10, 10, 500, AccentPattern),
        .. FadedAccents(11, 500),
        Pattern(17, 17, 500, GreyPattern),
        Pattern(18, 18, 500, AccentPattern),
        .. FadedAccents(19, 500),
        Pattern(25, 25, 700, GreyPattern),
        Pattern(26, 26, 700, AccentPattern),
        .. FadedAccents(27, 700),
        Pattern(33, 33, 500, GreyPattern),
        Pattern(34, 34, 500, AccentPattern),
        .. FadedAccents(35, 500),
        Pattern(41, 42, 500, DarkGreyPattern),
        Pattern(42, 42, 500, AccentPattern),
        .. FadedAccents(43, 500),
    ];

    /// <summary><c>spFilledSeries2dFills</c>: a bar, pie or area series' fill, by chart style.</summary>
    /// <remarks>
    /// The themed index differs between the subtle and intense halves of the real table and is
    /// not read here, because a fill entry's <c>mnThemedIdx</c> reaches only
    /// <c>Theme::getFillStyle</c>'s gradient — the colour, which is what a solid bar shows, comes
    /// from the pattern alone.
    /// </remarks>
    private static readonly AutoFormatEntry[] FilledSeriesFills =
    [
        Pattern(1, 1, 100, GreyPattern),
        Pattern(2, 2, 100, AccentPattern),
        .. FadedAccents(3, 100),
        Pattern(9, 9, 100, GreyPattern),
        Pattern(10, 10, 100, AccentPattern),
        .. FadedAccents(11, 100),
        Pattern(17, 17, 100, GreyPattern),
        Pattern(18, 18, 100, AccentPattern),
        .. FadedAccents(19, 100),
        Pattern(25, 25, 100, GreyPattern),
        Pattern(26, 26, 100, AccentPattern),
        .. FadedAccents(27, 100),
        Pattern(33, 33, 100, GreyPattern),
        Pattern(34, 34, 100, AccentPattern),
        .. FadedAccents(35, 100),
        Pattern(41, 42, 100, DarkGreyPattern),
        Pattern(42, 42, 100, AccentPattern),
        .. FadedAccents(43, 100),
    ];

    /// <summary><c>spAxisLines</c>: an axis line's colour, by chart style.</summary>
    /// <remarks>
    /// <c>objectformatter.cxx:215-220</c>. The comment beside both rows in the real table is
    /// "tint not documented!?" — it is what LibreOffice does, and it is what 26.2.4.2 draws.
    /// </remarks>
    private static readonly AutoFormatEntry[] AxisLines =
    [
        Single(1, 32, "tx1", "tint", 75000),
        Single(33, 48, "dk1", "tint", 75000),
    ];

    /// <summary><c>spMajorGridLines</c>: a major gridline's colour, by chart style.</summary>
    /// <remarks><c>objectformatter.cxx:222-227</c>; the same two rows as the axis line.</remarks>
    private static readonly AutoFormatEntry[] MajorGridLines =
    [
        Single(1, 32, "tx1", "tint", 75000),
        Single(33, 48, "dk1", "tint", 75000),
    ];

    /// <summary><c>spMinorGridLines</c>: a minor gridline's colour, by chart style.</summary>
    /// <remarks>
    /// <c>objectformatter.cxx:229-234</c>. <strong>Its style split is not the other two's</strong>
    /// — 1…40 and 41…48, and both rows name <c>tx1</c> rather than falling back to <c>dk1</c>.
    /// </remarks>
    private static readonly AutoFormatEntry[] MinorGridLines =
    [
        Single(1, 40, "tx1", "tint", 50000),
        Single(41, 48, "tx1", "tint", 90000),
    ];

    /// <summary><c>spFilledSeriesLines</c>: the outline of a filled series, by chart style.</summary>
    /// <remarks>
    /// Invisible for every style below 33 except 9 to 16, which outline in <c>lt1</c>. That is why
    /// an ordinary bar chart has no bar outline at all and drawing one is visible immediately.
    /// </remarks>
    private static readonly AutoFormatEntry[] FilledSeriesLines =
    [
        Invisible(1, 8),
        Single(9, 16, "lt1", null, 0),
        Invisible(17, 32),
        Single(33, 33, "dk1", "shade", 50000),
        Pattern(34, 34, 100, ShadedAccentPattern),
        .. AccentsModified(35, "shade", 50000),
        Invisible(41, 48),
    ];

    /// <summary><c>spChartSpaceFill</c>: the chart object's own background, by chart style.</summary>
    /// <remarks><c>objectformatter.cxx:174-180</c>.</remarks>
    private static readonly AutoFormatEntry[] ChartSpaceFills =
    [
        Single(1, 32, "bg1", null, 0),
        Single(33, 40, "lt1", null, 0),
        Single(41, 48, "dk1", null, 0),
    ];

    /// <summary><c>spPlotArea2dFills</c>: the plot area's wall, by chart style.</summary>
    /// <remarks>
    /// <c>objectformatter.cxx:190-197</c>. The <c>tint</c> on the two dark rows carries the
    /// source's own comment, "tint not documented!?" — it is what LibreOffice does.
    /// </remarks>
    private static readonly AutoFormatEntry[] PlotAreaFills =
    [
        Single(1, 32, "bg1", null, 0),
        Single(33, 34, "dk1", "tint", 20000),
        .. AccentsModified(35, "tint", 20000),
        Single(41, 48, "dk1", "tint", 95000),
    ];

    /// <summary>
    /// The background a chart space or a plot area takes when it states no <c>c:spPr</c>, or null
    /// when it takes none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Below style 33 a pptx chart has no automatic background at all, and that is a
    /// quirk rather than a table row.</strong> <c>ObjectTypeFormatter</c>'s constructor overrides
    /// the fill *style* for exactly these two object types when the chart style is 32 or less —
    /// <c>maFillFormatter.setAutoFill(rData.mrFilter.getGraphicHelper().getDefaultChartAreaFillStyle())</c>,
    /// <c>objectformatter.cxx:956-959</c> — and <c>PptGraphicHelper</c> answers <c>XML_noFill</c>
    /// (<c>oox/source/ppt/pptimport.cxx:309-312</c>) where the base helper answers
    /// <c>XML_solidFill</c> (<c>oox/source/helper/graphichelper.cxx:139-142</c>). The source's own
    /// comment on it is "this seems to be an undocumented quirk in the OOXML spec".
    /// </para>
    /// <para>
    /// So the <c>bg1</c> row is reachable only from a workbook or a document, where it paints the
    /// page's own white over a white page. This returns null for it rather than a colour: it is
    /// today's behaviour on all three tracks, it is right for the format that holds 160 of the
    /// corpus' 163 slides charts, and the two rows that <em>are</em> implemented — 33 and up —
    /// are the ones that put ink on a page which has none today. The corpus states a style above
    /// 32 on exactly three chart parts, all of them 42:
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> and <c>DynamicBubbleChart.xlsx</c>.
    /// </para>
    /// <para>
    /// Measured on Pavese's page 8: the reference paints <c>#000000</c> over the whole chart
    /// object, 720 x 391 pt, and <c>#454545</c> over the plot wall, 640 x 292 pt, and this reader
    /// painted neither — 72% of the page.
    /// </para>
    /// </remarks>
    /// <param name="style">The chart's <c>c:style/@val</c>.</param>
    /// <param name="what">Which of the two backgrounds.</param>
    /// <param name="theme">The theme the scheme names resolve against.</param>
    public static Colour? FrameFillOf(int style, ChartAutoFrame what, DrawingTheme? theme)
    {
        if (style <= 32) return null;

        AutoFormatEntry[] table =
            what == ChartAutoFrame.ChartSpace ? ChartSpaceFills : PlotAreaFills;

        return Entry(table, style) is { } entry ? Resolve(entry, 0, 0, theme) : null;
    }

    /// <summary>
    /// The colour a chart's text takes when its own element states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>spChartTitleTexts</c>, <c>spAxisTitleTexts</c> and <c>spOtherTexts</c>
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx:415-434</c>) are three tables of two
    /// rows each, and all three carry the same two colours: <c>tx1</c> for styles 1 to 40 and
    /// <c>lt1</c> for 41 to 48. What separates the tables is the size and the weight, which
    /// <see cref="DrawingChartPlot"/> already reads; the colour is one answer for every piece of
    /// text a chart holds, so it is one function.
    /// </para>
    /// <para>
    /// <strong>It is not "black", and the difference is a whole chart.</strong>
    /// <c>TextFormatter</c>'s constructor pushes this colour in as the placeholder the element's
    /// own <c>a:defRPr</c> then overrides (<c>:906-928</c>), so an element that states nothing
    /// takes it outright. Style 42 is one of the dark styles — <see cref="FrameFillOf"/> paints
    /// its chart space black — and its text is <c>lt1</c>, white. Falling back to black instead
    /// draws every title, tick label and legend entry of a dark chart in black on black:
    /// measured on <c>DynamicBubbleChart.xlsx</c>, whose title, both axis titles and all four
    /// tick labels are in our PDF's text layer and none of them visible.
    /// </para>
    /// <para>
    /// The 1-to-40 row matters too, and less dramatically. <c>tx1</c> resolves to the theme's
    /// <c>dk1</c>, which is black on 162 of the corpus's 169 chart-bearing OOXML files and
    /// <c>3C3D36</c> or <c>003F42</c> on the other seven — so those seven were drawn a shade too
    /// dark rather than invisibly.
    /// </para>
    /// </remarks>
    /// <param name="style">The chart's <c>c:style/@val</c>.</param>
    /// <param name="theme">The theme the scheme name resolves against.</param>
    public static Colour? TextColourOf(int style, DrawingTheme? theme)
    {
        XElement colour = new(
            Drawing.Name("schemeClr"), new XAttribute("val", style >= 41 ? "lt1" : "tx1"));

        return DrawingColour.Read(colour)?.Resolve(theme);
    }

    /// <summary>
    /// The colour a series takes when it states none, as a <c>a:solidFill</c>, or null for none.
    /// </summary>
    /// <param name="style">The chart's <c>c:style/@val</c>.</param>
    /// <param name="what">Whether the series is drawn as a line or as an area.</param>
    /// <param name="stroke">True for the outline rather than the interior.</param>
    /// <param name="index">The series' <c>c:ser/c:idx</c>, or its point index when varying.</param>
    /// <param name="maximum">The largest such index in the whole plot area.</param>
    /// <param name="theme">The theme the accent names resolve against.</param>
    /// <param name="styles">
    /// The theme's format matrix. A stroke's accent is a <em>placeholder</em> pushed into the
    /// theme's subtle line style rather than the drawn colour itself — see
    /// <see cref="ThroughSubtleLineStyle"/>. Null leaves the accent raw, which is what a caller
    /// with no theme to ask has to do.
    /// </param>
    public static Colour? ColourOf(
        int style,
        ChartAutoObject what,
        bool stroke,
        int index,
        int maximum,
        DrawingTheme? theme,
        DrawingStyleMatrix? styles = null)
    {
        AutoFormatEntry[] table = (what, stroke) switch
        {
            (ChartAutoObject.LinearSeries, true) => LinearSeriesLines,
            (ChartAutoObject.LinearSeries, false) => [],
            (ChartAutoObject.FilledSeries, true) => FilledSeriesLines,
            _ => FilledSeriesFills,
        };

        if (Entry(table, style) is not { } entry) return null;
        if (Resolve(entry, index, maximum, theme) is not { } placeholder) return null;

        return stroke ? ThroughSubtleLineStyle(placeholder, styles, theme) : placeholder;
    }

    /// <summary>
    /// The accent a stroke's automatic entry names, put through the theme's subtle line style.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The accent is not the colour drawn; it is what the theme's line style is drawn
    /// <em>in terms of</em>.</strong> <c>LineFormatter</c> copies
    /// <c>Theme::getLineStyle(THEMED_STYLE_SUBTLE)</c> whole and then resolves it with
    /// <c>getPhColor(nSeriesIdx)</c> as the placeholder —
    /// <c>aLineProps.pushToPropMap(rPropMap, …, getPhColor(nSeriesIdx))</c>,
    /// <c>oox/source/drawingml/chart/objectformatter.cxx:857-864</c>. So whatever the theme wrapped
    /// around its <c>phClr</c> acts on the accent.
    /// </para>
    /// <para>
    /// Measured on <c>Demick_JetBlue.pptx</c>, whose theme's first <c>a:lnStyleLst</c> entry is a
    /// <c>phClr</c> under <c>shade 50000</c> and <c>satMod 103000</c>: the reference draws its three
    /// automatic series in <c>B45D03</c>, <c>761D26</c> and <c>12415C</c>, not in the accents
    /// <c>F07F09</c>, <c>9F2936</c> and <c>1B587C</c> they are derived from. Taking the accent raw
    /// draws every automatic chart line noticeably too bright on any theme that states a transform
    /// there, and every theme Office ships states one.
    /// </para>
    /// <para>
    /// A theme entry with no <c>a:solidFill</c> — a bare width, or an <c>a:noFill</c> — leaves the
    /// accent alone rather than making the series invisible. LibreOffice would draw no line at all
    /// in that case, but no corpus theme states it and inventing an absence is the more expensive
    /// way to be wrong.
    /// </para>
    /// </remarks>
    /// <param name="placeholder">The accent the automatic table names, cycle shading applied.</param>
    /// <param name="styles">The theme's format matrix, or null.</param>
    /// <param name="theme">The theme the substituted colour resolves against.</param>
    public static Colour ThroughSubtleLineStyle(
        Colour placeholder, DrawingStyleMatrix? styles, DrawingTheme? theme)
    {
        if (styles?.LineStyle(SubtleStyleIndex) is not { } line) return placeholder;
        if (Drawing.Child(DrawingStyleMatrix.Substitute(line, placeholder), "solidFill")
            is not { } fill)
        {
            return placeholder;
        }

        foreach (XElement child in fill.Elements())
            if (DrawingColour.Read(child)?.Resolve(theme) is { } resolved) return resolved;

        return placeholder;
    }

    /// <summary>
    /// The colour a chart's axis line or gridline takes when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It is a tint of the theme's <c>tx1</c>, put through the theme's subtle line
    /// style — and the second half is what makes the numbers come out.</strong> A tint of 75000
    /// on a black <c>tx1</c> is <c>0x8B8B8B</c> on its own; the reference draws a major gridline
    /// <c>0x666666</c>, because <c>LineFormatter</c> substitutes that tint for the <c>phClr</c>
    /// inside <c>Theme::getLineStyle(THEMED_STYLE_SUBTLE)</c> and every theme Office ships wraps
    /// a <c>shade 50000</c> around it. See <see cref="ThroughSubtleLineStyle"/>.
    /// </para>
    /// <para>
    /// Five arms on one deck, patching one thing at a time and rendering each on 26.2.4.2 —
    /// theme <c>dk1</c> black, then <c>2050C0</c>, then <c>FFFFFF</c>:
    /// </para>
    /// <code>
    ///   tx1        major     minor
    ///   000000    #666666   #8B8B8B
    ///   2050C0    #676E9C   #8B8FA7
    ///   FFFFFF    #BCBCBC   #BCBCBC
    /// </code>
    /// <para>
    /// The white arm is the one that decides it: a tint of white is white whatever the tint, so
    /// two different tints can only collapse onto one value if something after them is doing the
    /// darkening — and <c>shade 50000</c> of white is <c>0xBCBCBC</c> exactly. A constant grey,
    /// which is what this reader drew before, cannot produce the middle row at all.
    /// </para>
    /// </remarks>
    /// <param name="what">Which piece of furniture is being asked about.</param>
    /// <param name="style">The chart's <c>c:style/@val</c>.</param>
    /// <param name="theme">The theme <c>tx1</c> and <c>dk1</c> resolve against.</param>
    /// <param name="styles">The theme's format matrix, or null to leave the tint raw.</param>
    public static Colour? LineColourOf(
        ChartAutoLine what, int style, DrawingTheme? theme, DrawingStyleMatrix? styles)
    {
        AutoFormatEntry[] table = what switch
        {
            ChartAutoLine.Axis => AxisLines,
            ChartAutoLine.MajorGrid => MajorGridLines,
            _ => MinorGridLines,
        };

        if (Entry(table, style) is not { } entry) return null;
        if (Resolve(entry, 0, 0, theme) is not { } placeholder) return null;

        return ThroughSubtleLineStyle(placeholder, styles, theme);
    }

    /// <summary>
    /// How wide an automatic line is: the theme's subtle line style, at the stated percentage.
    /// </summary>
    /// <remarks>
    /// <c>LineFormatter</c>'s constructor multiplies the themed line's width by
    /// <c>mnRelLineWidth / 100</c> (<c>objectformatter.cxx:850-853</c>), and every furniture entry
    /// states 100. Measured by patching the theme's own <c>a:lnStyleLst</c> first entry and
    /// re-rendering: 9525 EMU draws every gridline and axis line at 0.73 pt, 38100 draws them at
    /// 3.00 and 4763 at 0.37 — <c>round(EMU / 360)</c> hundredths of a millimetre, to the
    /// hundredth of a point in all three.
    /// </remarks>
    /// <param name="styles">The theme's format matrix, or null.</param>
    /// <param name="relative">The entry's relative width, in percent.</param>
    public static Length AutomaticLineWidth(DrawingStyleMatrix? styles, int relative = 100)
    {
        if (styles?.LineStyle(SubtleStyleIndex) is not { } line) return Length.Zero;
        if (Drawing.Number(line, "w") is not { } emu || emu <= 0) return Length.Zero;

        return LineWidth(emu * relative / 100);
    }

    /// <summary>
    /// A DrawingML line width, in the unit the reference actually keeps it in: a whole hundredth
    /// of a millimetre.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>LineProperties::getLineWidth</c> is <c>convertEmuToHmm(moLineWidth)</c>
    /// (<c>oox/source/drawingml/lineproperties.cxx</c>:560-563), and <c>convertEmuToHmm</c> is
    /// <c>o3tl::convertNarrowing&lt;sal_Int32, emu, mm100&gt;</c> whose <c>MulDiv</c> is
    /// <c>(n + 180) / 360</c> for a positive value — an integer, rounded half up
    /// (<c>include/oox/drawingml/drawingmltypes.hxx</c>:186-190,
    /// <c>include/o3tl/unit_conversion.hxx</c>:72-78). So the reference never holds a width of
    /// 0.75 pt: 9525 EMU is 26.458 hundredths of a millimetre and it keeps 26, which is
    /// 0.73701 pt.
    /// </para>
    /// <para>
    /// Confirmed against 26.2.4.2's own output rather than only in source. Its
    /// <c>--convert-to ods</c> of the corpus writes every chart's <c>svg:stroke-width</c> as a
    /// whole hundredth of a millimetre — <c>0.009cm</c>, <c>0.026cm</c>, <c>0.035cm</c>,
    /// <c>0.044cm</c>, <c>0.071cm</c>, <c>0.079cm</c>, <c>0.088cm</c>, <c>0.106cm</c> on
    /// <c>008_Contextures_chart_sample</c>, <c>018_Weight_Loss_Chart</c> and
    /// <c>022_Pareto_Chart_Template</c> — and its PDF of the same three strokes them at
    /// 0.2551, 0.9921, 1.2472, 2.0124, 2.4943 and 3.0045 pt, which are those integers times
    /// 0.0283465 pt to four decimal places. The corresponding stated widths are 0.25, 1, 1.25, 2,
    /// 2.5 and 3 pt, none of which the reference draws.
    /// </para>
    /// </remarks>
    /// <param name="emu">The stated width in EMU.</param>
    public static Length LineWidth(long emu)
        => emu <= 0 ? Length.Zero : Length.FromMm100(Length.FromEmu(emu).Mm100);

    /// <summary>
    /// How wide a line series' automatic stroke is, as a percentage of the theme's subtle line.
    /// </summary>
    /// <remarks>
    /// <c>LineFormatter</c>'s constructor multiplies the themed line's width by
    /// <c>mnRelLineWidth</c> (<c>objectformatter.cxx:851-853</c>), so a chart at the default style
    /// draws its lines at three times the theme's own — 2.25 pt against 0.75 pt on every theme
    /// Office ships. Returns 0 when the style draws no line at all.
    /// </remarks>
    /// <param name="style">The chart's <c>c:style/@val</c>.</param>
    /// <param name="what">Whether the series is drawn as a line or as an area.</param>
    public static int RelativeLineWidth(int style, ChartAutoObject what)
    {
        AutoFormatEntry[] table = what == ChartAutoObject.LinearSeries
            ? LinearSeriesLines
            : FilledSeriesLines;

        return Entry(table, style) is { } entry && !entry.Invisible ? entry.RelativeLineWidth : 0;
    }

    /// <summary>The style index a chart space states, or <see cref="DefaultStyle"/>.</summary>
    /// <remarks>
    /// A file written by Office 2010 or later states it twice: a <c>c14:style</c> of 100 plus the
    /// value inside an <c>mc:Choice</c>, and the plain <c>c:style</c> inside the
    /// <c>mc:Fallback</c> beside it. LibreOffice's markup-compatibility handling declares no
    /// support for the <c>c14</c> namespace, so the fallback is what it reads and the choice is
    /// discarded — which is why a chart carrying <c>c14:style val="102"</c> formats as style 2.
    /// </remarks>
    /// <param name="chartSpace">The <c>c:chartSpace</c> root.</param>
    public static int StyleOf(XElement? chartSpace)
    {
        if (chartSpace is null) return DefaultStyle;

        foreach (XElement candidate in chartSpace.Descendants(
                     XName.Get("style", OoxmlNamespaces.DrawingMLChart)))
        {
            if (Drawing.Attribute(candidate, "val") is not { } text) continue;
            if (!int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value))
                continue;
            if (value is >= 1 and <= 48) return value;
        }

        return DefaultStyle;
    }

    private static Colour? Resolve(
        AutoFormatEntry entry, int index, int maximum, DrawingTheme? theme)
    {
        if (entry.Invisible) return null;
        if (entry.Pattern.Length == 0) return null;

        (string name, string? modification, int value) =
            entry.Pattern[Math.Max(index, 0) % entry.Pattern.Length];

        XElement colour = new(Drawing.Name("schemeClr"), new XAttribute("val", name));
        if (modification is not null)
        {
            colour.Add(new XElement(
                Drawing.Name(modification),
                new XAttribute("val", value.ToString(CultureInfo.InvariantCulture))));
        }

        if (DrawingColour.Read(colour)?.Resolve(theme) is not { } resolved) return null;
        if (!entry.Cycled) return resolved;

        // The shade or tint that separates one cycle of the pattern from the next. Applied on the
        // resolved colour rather than as another a:shade, because that is what
        // Color::addChartTintTransformation does — it appends a transform to an already-resolved
        // sRGB colour rather than to the scheme reference.
        int cycle = Math.Max(index, 0) / entry.Pattern.Length;
        int cycles = Math.Max(maximum, 0) / entry.Pattern.Length;
        double amount = (((cycle + 1.0) / (cycles + 2.0)) * 1.4) - 0.7;

        return ChartTint(resolved, amount);
    }

    /// <summary>
    /// A colour shaded or tinted the way <c>Color::addChartTintTransformation</c> does.
    /// </summary>
    /// <remarks>
    /// Negative is a <c>shade</c> of <c>1 + amount</c> and positive a <c>tint</c> of
    /// <c>1 - amount</c> — note that both are stated as the *remaining* fraction, so a
    /// <c>fTint</c> of −0.42 becomes <c>shade 58%</c> and one of +0.42 becomes <c>tint 58%</c>
    /// (<c>oox/source/drawingml/color.cxx:488-495</c>). Routed back through
    /// <see cref="DrawingColour"/> so that the gamma-corrected forms of both transforms are the
    /// ones the rest of the reader already uses.
    /// </remarks>
    /// <param name="colour">The resolved colour.</param>
    /// <param name="amount">The signed fraction, in <c>[-1, 1]</c>.</param>
    public static Colour ChartTint(Colour colour, double amount)
    {
        int scaled = (int)Math.Clamp((amount * 100000.0) + 0.5, -100000.0, 100000.0);
        if (scaled == 0) return colour;

        string kind = scaled < 0 ? "shade" : "tint";
        int value = scaled < 0 ? scaled + 100000 : 100000 - scaled;

        XElement element = new(
            Drawing.Name("srgbClr"),
            new XAttribute("val", $"{colour.R:X2}{colour.G:X2}{colour.B:X2}"),
            new XElement(Drawing.Name(kind), new XAttribute("val", value.ToString(CultureInfo.InvariantCulture))));

        return DrawingColour.Read(element)?.Resolve(null) ?? colour;
    }

    private static AutoFormatEntry? Entry(AutoFormatEntry[] table, int style)
    {
        foreach (AutoFormatEntry entry in table)
        {
            if (entry.First <= style && style <= entry.Last) return entry;
        }

        return null;
    }

    private static AutoFormatEntry Pattern(
        int first, int last, int width, (string Colour, string? Modification, int Value)[] pattern)
        => new(first, last, width, pattern, Cycled: true, Invisible: false);

    private static AutoFormatEntry Single(
        int first, int last, string colour, string? modification, int value)
        => new(first, last, 100, [(colour, modification, value)], Cycled: false, Invisible: false);

    private static AutoFormatEntry Invisible(int first, int last)
        => new(first, last, 0, [], Cycled: false, Invisible: true);

    /// <summary>
    /// Six consecutive styles, one accent each, faded across the series.
    /// </summary>
    /// <remarks>
    /// <c>AUTOFORMAT_FADEDACCENTS</c>. The pattern has one entry, so every series lands in a
    /// cycle of its own and the shade/tint step above separates them — which is what makes styles
    /// 3 to 8 a single-colour chart rather than six copies of one line.
    /// </remarks>
    private static AutoFormatEntry[] FadedAccents(int first, int width)
        => [.. Enumerable.Range(0, 6).Select(i => new AutoFormatEntry(
            first + i,
            first + i,
            width,
            [($"accent{i + 1}", null, 0)],
            Cycled: true,
            Invisible: false))];

    /// <summary>Six consecutive styles, one modified accent each, not faded.</summary>
    /// <remarks><c>AUTOFORMAT_ACCENTSMOD</c>, which sets a single colour and no pattern.</remarks>
    private static AutoFormatEntry[] AccentsModified(int first, string modification, int value)
        => [.. Enumerable.Range(0, 6).Select(i => new AutoFormatEntry(
            first + i,
            first + i,
            100,
            [($"accent{i + 1}", modification, value)],
            Cycled: false,
            Invisible: false))];

    /// <summary>One row of one of <c>ObjectFormatter</c>'s automatic format tables.</summary>
    private readonly record struct AutoFormatEntry(
        int First,
        int Last,
        int RelativeLineWidth,
        (string Colour, string? Modification, int Value)[] Pattern,
        bool Cycled,
        bool Invisible);
}
