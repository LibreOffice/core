using System.Xml.Linq;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A layout that amends the master's colour map, so one background reference resolves two ways.
/// </summary>
/// <remarks>
/// <para>
/// The master states <c>p:clrMap</c> once and a layout amends it with
/// <c>p:clrMapOvr/a:overrideClrMapping</c>. Both parts were parsed and only the first was read,
/// which is invisible to every text comparison and to the page and word counts the corpus gate
/// runs — a slide whose whole background is the wrong colour still extracts perfectly.
/// </para>
/// <para>
/// The document that found it is
/// <c>slides/batch-004/pptx/redac-fullComm-201705-EE-FRs-briefing.pptx</c>, whose title layout
/// sends <c>bg2</c> to <c>dk2</c> where its master sends it to <c>lt2</c>. Its title page is
/// built from the theme's second background fill over that colour: a deep teal radial in the
/// reference, and a near-white wash in ours. 44.78 units of unaccounted ink to 12.69, and that
/// page from 32.15% to 0.06%.
/// </para>
/// <para>
/// <c>slide-colour-map-override.pptx</c> puts the three cases side by side. All three slides
/// inherit the <em>same</em> <c>&lt;p:bgRef idx="1001"&gt;&lt;a:schemeClr val="bg2"/&gt;</c> from
/// the master, so nothing but the map can make them differ. The expectations are LibreOffice
/// 24.2.7.2's own flat-ODF export of the deck, which gives two drawing-page styles —
/// <c>draw:fill-color="#dbf5f9"</c> and <c>draw:fill-color="#04617b"</c> — and puts the third
/// slide back on the first of them.
/// </para>
/// </remarks>
public class SlideColourMapOverrideTests
{
    private const string Deck = "slide-colour-map-override.pptx";

    [Fact]
    public void ALayoutInheritingTheMastersMapResolvesThroughIt()
    {
        // bg2 -> lt2, the master's own map. #DBF5F9.
        Slides()[0].Background.ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(Colour.FromRgb(0xDBF5F9));
    }

    [Fact]
    public void ALayoutOverridingTheMastersMapResolvesThroughTheOverride()
    {
        // The same bgRef, the same phClr token, and a layout sending bg2 to dk2. #04617B.
        Slides()[1].Background.ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(Colour.FromRgb(0x04617B));
    }

    /// <summary>
    /// A slide's own override does not reach the background it inherits.
    /// </summary>
    /// <remarks>
    /// Slide 3 sits on the inheriting layout and states
    /// <c>&lt;a:overrideClrMapping bg2="accent1" …/&gt;</c> for itself. LibreOffice puts it back
    /// on the same drawing-page style as slide 1, because Impress resolves a master page's fill
    /// once when it imports the layout and a slide only shows it. Asserted so that adding the
    /// slide level later has to face this measurement rather than look like an improvement.
    /// </remarks>
    [Fact]
    public void ASlidesOwnOverrideDoesNotRepaintTheInheritedBackground()
    {
        Slides()[2].Background.ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(Colour.FromRgb(0xDBF5F9));
    }

    // ------------------------------------------------------------------- the map itself

    [Fact]
    public void AnOverridePatchesTheMapRatherThanReplacingIt()
    {
        // SlideFragmentHandler copies the map in force for an overrideClrMapping and starts
        // from an empty one for a clrMap (oox/source/ppt/slidefragmenthandler.cxx:194-203), so
        // a key the override leaves out keeps what the master said.
        DrawingColourMap map = DrawingColourMap.ReadLayered(
            Element("clrMap", ("bg1", "dk1"), ("bg2", "lt2")),
            Element("overrideClrMapping", ("bg2", "dk2")));

        map.Resolve("bg1").ShouldBe(ThemeColourSlot.Dark1);
        map.Resolve("bg2").ShouldBe(ThemeColourSlot.Dark2);
    }

    [Fact]
    public void AnEmptyOverrideKeepsTheInheritedMap()
    {
        // a:masterClrMapping is the "inherit" alternative and states no attributes at all,
        // which is the same thing as an override element with none.
        DrawingColourMap map = DrawingColourMap.ReadLayered(
            Element("clrMap", ("bg2", "dk2")),
            Element("masterClrMapping"));

        map.Resolve("bg2").ShouldBe(ThemeColourSlot.Dark2);
    }

    private static XElement Element(string name, params (string Name, string Value)[] attributes)
        => new(
            XName.Get(name, "http://schemas.openxmlformats.org/drawingml/2006/main"),
            attributes.Select(a => new XAttribute(a.Name, a.Value)));

    private static IReadOnlyList<LaidOutSlide> Slides()
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(Deck)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides;
    }
}
