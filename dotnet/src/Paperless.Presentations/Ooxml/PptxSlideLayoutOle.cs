using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// The PPTX layout's embedded-object half: a <c>p:graphicFrame</c> holding a <c>p:oleObj</c>.
/// </summary>
/// <remarks>
/// A separate file for the same reason the chart half is one — a frame is reached identically
/// whatever it holds, and what happens after that is a different subsystem.
/// </remarks>
internal sealed partial class PptxSlideLayout
{
    /// <summary>The <c>a:graphicData/@uri</c> of an embedded object.</summary>
    private const string OleUri = "http://schemas.openxmlformats.org/presentationml/2006/ole";

    /// <summary>
    /// The shapes a graphic frame holding an embedded object draws — its replacement picture.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An OLE object is drawn as the picture beside it, never by running the object.</strong>
    /// Every <c>p:oleObj</c> PowerPoint writes carries a <c>p:pic</c> holding a rendering of the
    /// object as it looked when the deck was saved — usually an EMF, so its text survives as real
    /// glyph runs. LibreOffice reads the same element into the OLE shape's replacement graphic
    /// (<c>OleObjectGraphicDataContext::onCreateContext</c>,
    /// <c>oox/source/drawingml/graphicshapecontext.cxx:255-258</c>) and draws that whenever the
    /// object is not activated, which in a headless conversion is always.
    /// </para>
    /// <para>
    /// The picture is normally inside an <c>mc:Fallback</c>, because the <c>mc:Choice</c> beside
    /// it needs VML to place the object and states its position as a <c>@spid</c> into the
    /// slide's VML drawing instead of carrying a picture at all. Paperless does not read VML in
    /// a deck, so <see cref="OoxmlXml"/> has already chosen the fallback by the time this runs —
    /// which is exactly the branch that has the picture. A frame whose chosen branch has no
    /// picture draws nothing, as it did before.
    /// </para>
    /// <para>
    /// Measured across the slides corpus: 45 embedded objects over 10 decks, of which
    /// <c>NAS-Infrastructure-Roadmaps-Weather.pptx</c> loses 232 of a page's 247 words to one of
    /// them and <c>NAS-Infrastructure-Roadmaps-v16.0.pptx</c> holds thirty.
    /// </para>
    /// </remarks>
    private List<PlacedShape> Ole(XElement frame, PptxSlide slide, SlideTheme theme, AffineTransform space)
    {
        XElement? graphic = Drawing.Child(Drawing.Child(frame, "graphic"), "graphicData");
        if (Drawing.Attribute(graphic, "uri") != OleUri) return [];

        XElement? picture = null;
        foreach (XElement ole in Ppt.Children(graphic!, "oleObj"))
        {
            if (Ppt.Child(ole, "pic") is { } found) { picture = found; break; }
        }

        if (picture is null) return [];

        // A replacement picture usually repeats the frame's own rectangle, but it is not obliged
        // to state one at all — and a picture with no transform is dropped for having no size.
        // The frame's p:xfrm is the answer in that case, and it is the rectangle the object
        // occupies whatever the picture says.
        if (Drawing.Child(Ppt.Child(picture, "spPr"), "xfrm") is null
            && Ppt.Child(frame, "xfrm") is { } stated)
        {
            DocRect local = Bounds(stated);
            if (local.Width <= Length.Zero || local.Height <= Length.Zero) return [];

            picture = new XElement(picture);
            XElement properties = Ppt.Child(picture, "spPr")
                                  ?? new XElement(Ppt.Name("spPr"));
            if (properties.Parent is null) picture.Add(properties);

            // p:xfrm and a:xfrm differ in the wrapper's namespace and in nothing else: the
            // a:off and a:ext inside a frame's transform are already DrawingML.
            properties.AddFirst(
                new XElement(Drawing.Name("xfrm"), stated.Attributes(), stated.Elements()));
        }

        return Shape(picture, slide, theme, space) is { } placed ? [placed] : [];
    }
}
