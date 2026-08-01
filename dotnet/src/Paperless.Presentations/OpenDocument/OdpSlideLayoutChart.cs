using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.OpenDocument;
using Paperless.Presentations.Layout;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// The ODP layout's chart half: a <c>draw:frame</c> holding a <c>draw:object</c> that is a chart.
/// </summary>
/// <remarks>
/// A separate file, as the PPTX side is, so that the whole chart path can be read and reverted
/// without touching the shape walk.
/// </remarks>
internal sealed partial class OdpSlideLayout
{
    /// <summary>
    /// The shapes a frame holding a chart draws, or nothing when it holds something else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>OdfChart.Locate</c> resolves both shapes an embedded chart takes — a packaged file's
    /// <c>Object 1/content.xml</c> reached by <c>xlink:href</c>, and a flat file's inlined
    /// <c>office:document</c> — so this reader sees a <c>chart:chart</c> either way and does not
    /// know which it was.
    /// </para>
    /// <para>
    /// <strong>Drawing the chart is also what stops a flat file drawing its markup.</strong>
    /// Before this, <c>chart-bar-deck.fodp</c> rendered the chart's title, its axis titles and
    /// all fifteen cells of its local table as the frame's own text — eighteen words stacked as
    /// paragraphs where LibreOffice draws a bar chart — because a descendant search for
    /// <c>text:p</c> reaches into the inlined sub-document. <c>Paragraphs</c> was taught to stop
    /// at an <c>office:document</c> boundary when charts were read; now the frame draws the
    /// chart instead of nothing.
    /// </para>
    /// </remarks>
    private List<PlacedShape> Chart(XElement frame, AffineTransform space)
    {
        if (frame.Name.LocalName != "frame") return [];

        XElement? drawObject = frame.Element(XName.Get("object", OdfNamespaces.Draw));
        if (drawObject is null) return [];

        if (OdfChart.Locate(drawObject, _file) is not { } chart) return [];

        // The styles are the chart sub-document's own, not the deck's, so they are read from
        // whichever root the chart was found under rather than from _file.Styles.
        OdfChartStyles styles = new(chart.AncestorsAndSelf().Last());
        if (OdfChartPlot.Read(chart, styles) is not { } plot) return [];

        DocSize size = new(
            Measure(frame, OdfNamespaces.SvgCompatible, "width"),
            Measure(frame, OdfNamespaces.SvgCompatible, "height"));

        if (size.Width <= Length.Zero || size.Height <= Length.Zero) return [];

        AffineTransform placement = AffineTransform.Concat(Placement(frame), space);

        return SlideChart.Place(
            plot, size, placement, _fonts,
            frame.Attribute(XName.Get("name", OdfNamespaces.Draw))?.Value);
    }
}
