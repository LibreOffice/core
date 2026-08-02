using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;

namespace Paperless.Presentations.Ooxml;

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
        if (DrawingChartPlot.Read(chartSpace, theme.Colours, _file.IsOffice2007) is not { } plot)
            return [];

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

        return SlideChart.Place(plot, local.Size, placement, _fonts, Name(frame));
    }
}
