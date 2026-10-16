using System.Xml.Linq;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// The chart title LibreOffice draws when the file states one and leaves it empty, or states none
/// at all and has not deleted it.
/// </summary>
/// <remarks>
/// <para>
/// <c>ChartSpaceConverter::convertFromModel</c>
/// (<c>oox/source/drawingml/chart/chartspaceconverter.cxx:177-208</c>) gives a chart a title the
/// part never spells out, and there are two different substitutes with different rules:
/// </para>
/// <list type="number">
/// <item><description>
/// the **single series' name**, when the plot resolves to exactly one series that carries a
/// <c>c:tx</c> — <c>PlotAreaConverter</c> and <c>AxesSetConverter</c>
/// (<c>plotareaconverter.cxx:170-176</c> and <c>:483-491</c>) plus
/// <c>TypeGroupConverter::getSingleSeriesTitle</c> (<c>typegroupconverter.cxx:272-281</c>);
/// </description></item>
/// <item><description>
/// failing that, the localized literal <c>Chart Title</c> — <c>STR_DIAGRAM_TITLE</c> — but only
/// when the part states a <c>c:title</c> element at all, and only when neither of
/// tdf#146487's two escapes applies.
/// </description></item>
/// </list>
/// <para>
/// <strong>Both halves are measured on 26.2.4.2 against corpus documents, not authored ones.</strong>
/// The reference draws <c>Sales</c> thirteen times on
/// <c>005_Contextures_chart_sample</c> where we drew six, <c>East</c> seven times on
/// <c>013_Contextures_chart_sample</c> where we drew six, <c>Production in 2017</c> once on each
/// of <c>pie-chart-result.docx</c> and <c>pie-chart-template.docx</c> where we drew none, and
/// <c>Chart Title</c> twice on <c>035_Chemistry_Column_PowerPoint_Chart.pptx</c> — once per chart
/// part — where we drew none. Censused over all 946 corpus documents and all 307
/// <c>c:chartSpace</c> parts in them, those five documents are the whole reach:
/// <c>dotnet/probes/sheets-r54/census-autotitle.py</c>.
/// </para>
/// <para>
/// <strong>The suppressing branch has its own controls, because it is the larger population.</strong>
/// 157 of the 307 parts draw no title only because <c>autoTitleDeleted</c> holds — and for 82 of
/// them the census can name the string that *would* appear. Three sheets documents where it can
/// —<c>052_Manufacturing_output_chart</c> (<c>COMPONENTS COMPLETED</c>),
/// <c>058_Social_media_engagement_data</c> (<c>DAILY IMPRESSIONS</c>) and
/// <c>001_Contextures_chart_sample</c> (<c>Amt</c>) — render with ours = reference = one
/// occurrence, the worksheet cell. And <c>005</c>'s own sixth chart states
/// <c>autoTitleDeleted val="0"</c> with no <c>c:title</c> and a series with no <c>c:tx</c>: its
/// page has ours = reference = one <c>Sales</c>. Both negatives hold.
/// </para>
/// <para>
/// <strong>The default when <c>c:autoTitleDeleted</c> is absent is "deleted".</strong>
/// <c>ChartSpaceModel</c>'s constructor sets <c>mbAutoTitleDel( !bMSO2007Doc )</c>
/// (<c>chartspacemodel.cxx:29</c>) and the attribute reader takes the same default
/// (<c>chartspacefragment.cxx:113</c>) — so an Excel 2007 package gets a title where a modern one
/// does not. <see cref="OoxmlMetadata.IsOffice2007(XElement?)"/> is exactly
/// <c>XmlFilterBase::checkDocumentProperties</c>'s test and the caller passes it in. It changes
/// nothing on this corpus in either direction: twenty documents satisfy it and all fourteen of
/// their chart parts carry their own title text, and every one of the ten parts this class
/// actually answers for states <c>autoTitleDeleted val="0"</c> explicitly.
/// </para>
/// </remarks>
internal static class DrawingChartTitle
{
    /// <summary>
    /// LibreOffice's <c>STR_DIAGRAM_TITLE</c>, in the en-US resource this project's reference
    /// binary runs under.
    /// </summary>
    /// <remarks>
    /// A localized string, so a reference under another UI language draws another one. There is
    /// nothing in the file to read it from — that is the point of the branch — so the language is
    /// the renderer's and not the document's.
    /// </remarks>
    internal const string DiagramTitle = "Chart Title";

    /// <summary>
    /// LibreOffice's <c>STR_DIAGRAM_AXISTITLE</c>, in the same en-US resource.
    /// </summary>
    /// <remarks>
    /// The axis counterpart of <see cref="DiagramTitle"/> and localized in the same way; see
    /// <c>DrawingChartPlot.AxisTitleText</c> for the condition, which is not this one's.
    /// </remarks>
    internal const string AxisTitle = "Axis Title";

    /// <summary>
    /// The text to put in a chart title the part leaves empty, or null when the reference draws
    /// no title.
    /// </summary>
    /// <param name="chart">The <c>c:chart</c> element of a chart part.</param>
    /// <param name="office2007">
    /// Whether Office 2007 wrote the package, which decides the <c>c:autoTitleDeleted</c> default.
    /// </param>
    /// <remarks>
    /// Call this only when the title model has no text of its own: LibreOffice hands this string
    /// to <c>TextConverter::createStringSequence</c> as its *default*, and that function reaches
    /// it only after the rich body, the <c>c:txPr</c> paragraphs and the <c>c:tx</c> cache have
    /// all come back empty (<c>titleconverter.cxx:87-160</c>).
    /// </remarks>
    internal static string? Automatic(XElement? chart, bool office2007)
    {
        if (chart is null) return null;

        XElement? title = Child(chart, "title");

        // `if( !mrModel.mbAutoTitleDel || mrModel.mxTitle.is() )`. A stated element with no @val
        // is true, which is what CT_Boolean's `val` defaulting to 1 means.
        bool deleted = Flag(Child(chart, "autoTitleDeleted")) ?? !office2007;
        if (deleted && title is null) return null;

        (string? automatic, bool single) = SingleSeriesTitle(Child(chart, "plotArea"));

        // `if( mrModel.mxTitle.is() || !aAutoTitle.isEmpty() )` — with neither, no title object is
        // created at all.
        if (automatic is { Length: > 0 }) return automatic;
        if (title is null) return null;

        // tdf#146487's two escapes, both of which mean "the author asked for an empty title and
        // meant it". Either one suppresses the literal; neither can fire without a title element,
        // which the test above has already established.
        if (!deleted && single
            && Child(title, "spPr") is not null
            && Child(title, "txPr") is { } properties && IsEmptyBody(properties))
        {
            return null;
        }

        if (Child(Child(title, "tx"), "rich") is { } rich && IsEmptyBody(rich)) return null;

        return DiagramTitle;
    }

    /// <summary>
    /// The name of the plot's one series, and whether that one series states a <c>c:tx</c> at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two answers because LibreOffice asks two questions of the same shape:
    /// <c>getSingleSeriesTitle</c> wants the cached string and <c>isSingleSeriesTitle</c> only
    /// wants to know that a <c>c:tx</c> is there, and a series with an empty cache separates them.
    /// </para>
    /// <para>
    /// <strong>Axes sets, and why more than one means no automatic title.</strong>
    /// <c>PlotAreaConverter::convertFromModel</c> buckets the type groups by their
    /// <c>c:axId</c> list — order-sensitive equality, and a group with no series is skipped
    /// entirely (<c>plotareaconverter.cxx:415-440</c>). It then converts each bucket and keeps the
    /// automatic title only from the one it started on, <em>clearing</em> it on every other
    /// (<c>:483-491</c>). With two buckets the loop always reaches the clearing arm, so a chart
    /// with a secondary axes set never has one. <c>mbSingleSeriesTitle</c> is not cleared there,
    /// so it keeps the first bucket's answer, which is what this returns.
    /// </para>
    /// <para>
    /// Within one bucket the title is taken only when it holds exactly one type group
    /// (<c>:170-176</c>) — a column chart with a line over it, sharing one axis pair, has two.
    /// </para>
    /// </remarks>
    private static (string? Title, bool Single) SingleSeriesTitle(XElement? plotArea)
    {
        if (plotArea is null) return (null, false);

        List<(string[] Axes, List<XElement> Groups)> sets = [];

        foreach (XElement group in plotArea.Elements())
        {
            if (group.Name.NamespaceName != OoxmlNamespaces.DrawingMLChart) continue;
            if (!group.Name.LocalName.EndsWith("Chart", StringComparison.Ordinal)) continue;
            if (!Children(group, "ser").Any()) continue;

            string[] axes = [.. Children(group, "axId").Select(id => id.Attribute("val")?.Value ?? "")];

            int at = sets.FindIndex(set => set.Axes.SequenceEqual(axes));
            if (at < 0) sets.Add((axes, [group]));
            else sets[at].Groups.Add(group);
        }

        if (sets.Count == 0) return (null, false);

        List<XElement> first = sets[0].Groups;
        if (first.Count != 1) return (null, false);

        XElement only = first[0];
        List<XElement> series = [.. Children(only, "ser")];

        // `mbSingleSeriesVis || maSeries.size() == 1`. The type table's "1stvis" column
        // (typegroupconverter.cxx:95-118) is set for TYPEID_PIE and TYPEID_OFPIE and for nothing
        // else — not doughnut, not surface, both of which a first draft of this read wrongly
        // included. c:pieChart and c:pie3DChart are both TYPEID_PIE (:191-192).
        bool singleVisible = only.Name.LocalName
            is "pieChart" or "pie3DChart" or "ofPieChart";

        if (series.Count == 0 || !(singleVisible || series.Count == 1)) return (null, false);

        XElement? text = Child(series[0], "tx");
        if (text is null) return (null, false);

        // Two axes sets clear the title but leave `mbSingleSeriesTitle` standing.
        return (sets.Count > 1 ? null : CachedText(text), true);
    }

    /// <summary>
    /// The first entry of a <c>c:tx</c>'s data sequence — <c>maData.begin()->second</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>maData</c> is keyed by <c>c:pt/@idx</c> and ordered by it, so "first" is the lowest
    /// index rather than the first element in document order. A <c>c:tx</c> cache normally holds
    /// one point at index 0 and the two agree, but the file decides the order and not the reader.
    /// </para>
    /// <para>
    /// A bare <c>&lt;c:tx&gt;&lt;c:v&gt;text&lt;/c:v&gt;&lt;/c:tx&gt;</c> counts too:
    /// <c>TextContext::onCharacters</c> (<c>titlecontext.cxx:74-86</c>) stores it into the same
    /// <c>maData[0]</c>, so a series named by a literal has an automatic title exactly as one
    /// named by a reference does.
    /// </para>
    /// </remarks>
    private static string? CachedText(XElement text)
    {
        if (Child(text, "v") is { } literal && literal.Value.Length > 0) return literal.Value;

        int lowest = int.MaxValue;
        string? found = null;

        foreach (XElement point in text.Descendants(
                     XName.Get("pt", OoxmlNamespaces.DrawingMLChart)))
        {
            if (Child(point, "v")?.Value is not { Length: > 0 } value) continue;
            if (!int.TryParse(point.Attribute("idx")?.Value, out int index)) index = 0;
            if (index >= lowest) continue;

            lowest = index;
            found = value;
        }

        return found;
    }

    /// <summary>
    /// <c>TextBody::isEmpty()</c> — <c>oox/source/drawingml/textbody.cxx</c>.
    /// </summary>
    /// <remarks>
    /// Zero paragraphs, or exactly one paragraph holding zero runs, or exactly one paragraph
    /// holding exactly one run whose text is empty. More than one of either is *not* empty even
    /// when every run is blank, which is the transcription and not a simplification of it. An
    /// <c>a:endParaRPr</c> is not a run, so the <c>&lt;a:p&gt;&lt;a:pPr/&gt;&lt;a:endParaRPr/&gt;&lt;/a:p&gt;</c>
    /// Excel writes for a formatted-but-empty title is empty here.
    /// </remarks>
    private static bool IsEmptyBody(XElement body)
    {
        List<XElement> paragraphs =
            [.. body.Elements(XName.Get("p", OoxmlNamespaces.DrawingML))];

        if (paragraphs.Count == 0) return true;
        if (paragraphs.Count > 1) return false;

        List<XElement> runs = [.. paragraphs[0].Elements().Where(IsRun)];

        if (runs.Count == 0) return true;
        if (runs.Count > 1) return false;

        // `rRuns[0]->getText().isEmpty()`. A line break's TextRun carries the flag and no text
        // (`setLineBreak` touches nothing else, `oox/inc/drawingml/textrun.hxx:42-45`), so a
        // paragraph holding one <a:br> and nothing else is empty here as well.
        return runs[0].Element(XName.Get("t", OoxmlNamespaces.DrawingML))
            is not { Value.Length: > 0 };
    }

    /// <summary>The three <c>EG_TextRun</c> members, which are what <c>getRuns()</c> holds.</summary>
    private static bool IsRun(XElement element)
        => element.Name.NamespaceName == OoxmlNamespaces.DrawingML
           && element.Name.LocalName is "r" or "br" or "fld";

    private static bool? Flag(XElement? element)
    {
        if (element is null) return null;
        if (element.Attribute("val")?.Value is not { } stated) return true;

        return stated is not ("0" or "false");
    }

    private static XElement? Child(XElement? element, string localName)
        => element?.Element(XName.Get(localName, OoxmlNamespaces.DrawingMLChart));

    private static IEnumerable<XElement> Children(XElement? element, string localName)
        => element?.Elements(XName.Get(localName, OoxmlNamespaces.DrawingMLChart)) ?? [];
}
