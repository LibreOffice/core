using System.Xml.Linq;
using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What an <c>a:hlinkClick</c> on a run does to a slide's line breaking and to how its lines
/// are stacked.
/// </summary>
/// <remarks>
/// <para>
/// A DrawingML hyperlink is not a character property: it is an EditEngine <em>field</em>.
/// <c>TextRun::insertAt</c> branches on whether the run's own hyperlink property map is empty
/// (<c>oox/source/drawingml/textrun.cxx</c>:88) and builds a
/// <c>com.sun.star.text.TextField.URL</c> whose <c>Representation</c> is the run's own
/// <c>a:t</c> when it is not (<c>:149-157</c>) — one field per <c>a:r</c>. It is the same rule
/// <see cref="OdpHyperlinkFieldTests"/> establishes for a <c>text:a</c>, reached from the other
/// importer, and the two share every line of layout below <see cref="SlideTextRun.IsField"/>.
/// </para>
/// <para>
/// Two consequences, neither visible in the formatting. A field is one portion, so a field wider
/// than the room left on its line is filled to the <em>cell</em> — every character, on Latin text
/// — rather than moved down or broken at a separator
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1101-1206). And the lines it spills onto belong to
/// the line that holds it, so each is drawn one <em>ascent</em> below the last (<c>:3778-3795</c>)
/// while the formatter never sees them at all — the block height that anchors the box counts the
/// field's line once.
/// </para>
/// <para>
/// Every figure asserted here is 26.2.4.2's own, read out of its PDF of this same fixture by
/// <c>probes/pptx-field-r82/baselines.py</c>. The deck's six slides are a 14 × 5 cm box at
/// (1 cm, 1 cm) with zero insets, no autofit and 16 pt Liberation Sans, so the ordinary pitch is
/// 1.2 em = 19.190 pt and a spill is 1.0 em = 15.987.
/// </para>
/// <para>
/// Reach: <strong>608 text-run <c>a:hlinkClick</c> in 138 of the corpus's 251 decks</strong>, of
/// which 416 are on a slide proper. The other 1922 <c>a:hlinkClick</c> those decks carry sit on a
/// <c>p:cNvPr</c> and are a click action on the shape, which is not text and not a field.
/// </para>
/// </remarks>
public class PptxHyperlinkFieldTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("slide-hyperlink-field.pptx")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>The drawn baselines of a slide, in points from the page top, in order.</summary>
    private static List<double> Baselines(LaidOutSlide slide) =>
    [
        .. slide.Shapes.Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Origin.Y.Points)
            .Distinct()
            .Order(),
    ];

    /// <summary>The distances between consecutive baselines.</summary>
    private static List<double> Pitches(LaidOutSlide slide)
    {
        List<double> baselines = Baselines(slide);
        return [.. baselines.Zip(baselines.Skip(1), (a, b) => b - a)];
    }

    /// <summary>The text drawn on each line, in order.</summary>
    private static List<string> Lines(LaidOutSlide slide)
    {
        Dictionary<double, string> byBaseline = [];

        foreach (PlacedGlyphRun run in
                 slide.Shapes.Where(shape => shape.Text is not null)
                             .SelectMany(shape => shape.Text!.Runs))
        {
            double y = Math.Round(run.Run.Origin.Y.Points, 3);
            byBaseline[y] = byBaseline.TryGetValue(y, out string? already)
                ? already + run.Run.Text
                : run.Run.Text;
        }

        return [.. byBaseline.OrderBy(pair => pair.Key).Select(pair => pair.Value)];
    }

    /// <summary>The colour every run of a slide is drawn in.</summary>
    private static List<Core.Graphics.Colour> Colours(LaidOutSlide slide) =>
    [
        .. slide.Shapes.Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Colour),
    ];

    /// <remarks>
    /// 26.2.4.2 draws slide 1's three baselines at 44.306, 60.293 and 76.280 — the two spills
    /// 15.987 pt below their predecessor where the ordinary pitch on the same slide's control is
    /// 19.190. That is the line's <em>ascent</em> against its height, and under this deck's fixed
    /// cell height it is 1.0 em against 1.2.
    /// </remarks>
    [Fact]
    public void AFieldsSpillLinesAreOneAscentApartRatherThanOneLineHeight()
    {
        List<double> pitches = Pitches(Layout().Slides[0]);

        pitches.Count.ShouldBe(2);
        pitches[0].ShouldBe(15.987, 0.03);
        pitches[1].ShouldBe(15.987, 0.03);
    }

    /// <remarks>
    /// The break positions themselves, which are what a character-level fill produces and a
    /// word-level one cannot: 26.2.4.2 ends slide 1's first line inside <c>development</c> and
    /// its second at the solidus that happens to fall there.
    /// </remarks>
    [Fact]
    public void AFieldIsBrokenAtAnyCharacterRatherThanAtASeparator()
    {
        List<string> lines = Lines(Layout().Slides[0]);

        lines.Count.ShouldBe(3);
        lines[0].ShouldBe("https://www.example.org/hr-connect/organisational-dev");
        lines[1].ShouldBe("elopment/career-and-development-planning-framework/");
        lines[2].ShouldBe("leadership/");
    }

    /// <remarks>
    /// The control, and the one that says both halves belong to the field rather than to the URL:
    /// the same characters with no <c>a:hlinkClick</c> draw every pitch at 19.190 and break where
    /// UAX #14 and the solidus glue put them, which is after <c>organisational-</c>.
    /// </remarks>
    [Fact]
    public void TheSameCharactersAsPlainTextTakeTheOrdinaryPitchAndBreakAtASeparator()
    {
        LaidOutSlide slide = Layout().Slides[1];

        List<double> pitches = Pitches(slide);
        pitches.Count.ShouldBe(2);
        pitches.ShouldAllBe(pitch => Math.Abs(pitch - 19.190) < 0.03);

        Lines(slide)[0].ShouldBe("https://www.example.org/hr-connect/organisational-");
    }

    /// <remarks>
    /// <para>
    /// The spill is invisible to the formatter, so a middle-anchored box holding an over-long
    /// field is centred as though the paragraph were one line and the spill hangs below the box.
    /// The box runs 28.35 to 170.08 pt; one line of 19.190 centred in it starts at 89.62 and its
    /// baseline is 105.59, which is what 26.2.4.2 draws on slide 3.
    /// </para>
    /// <para>
    /// Counting the two spills would put the first baseline 15.99 pt higher, so this is what
    /// separates <em>“the painter advances by the ascent”</em> from <em>“the spill lines are
    /// lines”</em>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheBlockHeightThatAnchorsAMiddleAnchoredBoxCountsTheFieldsLineOnce()
        => Baselines(Layout().Slides[2])[0].ShouldBe(105.591, 0.05);

    /// <remarks>
    /// The control for the whole rule: a link short enough to fit its line is placed exactly as
    /// ordinary text would be. 375 of the corpus's 416 slide hyperlink runs are under 40
    /// characters, so this is the case that must not move.
    /// </remarks>
    [Fact]
    public void AFieldThatFitsIsOneLineAndMovesNothing()
    {
        List<double> baselines = Baselines(Layout().Slides[3]);

        baselines.Count.ShouldBe(1);
        baselines[0].ShouldBe(44.306, 0.05);
    }

    /// <remarks>
    /// <para>
    /// <c>&lt;a:hlinkClick r:id=""/&gt;</c> with nothing else on it leaves the run's hyperlink
    /// property map empty, so <c>textrun.cxx</c>:88 takes the <em>text</em> branch: no field, no
    /// <c>hlink</c> scheme colour and no automatic underline. 26.2.4.2 draws slide 5 in black at
    /// the word-broken positions of slide 2, and this tree drew it blue, underlined and
    /// cell-broken until <see cref="DrawingHyperlink.MakesField"/> was the test.
    /// </para>
    /// <para>
    /// Corpus reach is one occurrence, in an <c>.xlsx</c> drawing — so this is a correctness
    /// assertion and not a measured mover.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnHlinkClickThatStatesNothingIsNotAFieldAndIsNotDecorated()
    {
        LaidOutSlide slide = Layout().Slides[4];

        Lines(slide)[0].ShouldBe("https://www.example.org/hr-connect/organisational-");
        Pitches(slide).ShouldAllBe(pitch => Math.Abs(pitch - 19.190) < 0.03);
        Colours(slide).ShouldAllBe(colour => colour == Core.Graphics.Colour.Black);
    }

    /// <remarks>
    /// The painter's pen <em>does</em> carry the spill — <c>MoveToNextLine</c> takes its start
    /// position by reference (<c>impedit3.cxx</c>:3273-3279) — so the paragraph after a field
    /// begins one full line height below the last spill line rather than where the unspilled
    /// block would have ended. 26.2.4.2 draws slide 6's <c>AFTER</c> at 95.471, which is
    /// 76.280 + 19.190.
    /// </remarks>
    [Fact]
    public void TheParagraphAfterAFieldStartsBelowTheSpillRatherThanBelowItsOwnLine()
        => Baselines(Layout().Slides[5])[3].ShouldBe(95.471, 0.05);
}

/// <summary>
/// Which <c>a:hlinkClick</c> elements make a run a field, and which are inert.
/// </summary>
/// <remarks>
/// The test is not the element's presence but whether <c>HyperLinkContext</c>
/// (<c>oox/source/drawingml/hyperlinkcontext.cxx</c>:40-156) left anything on the run's property
/// map, because that is what <c>textrun.cxx</c>:88 asks.
/// </remarks>
public class DrawingHyperlinkTests
{
    private static XElement Link(string attributes, string children = "")
        => XElement.Parse(
            "<a:hlinkClick xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
            + "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
            + attributes + ">" + children + "</a:hlinkClick>");

    [Fact]
    public void NoElementIsNoField() => DrawingHyperlink.MakesField(null).ShouldBeFalse();

    /// <remarks>What PowerPoint writes to clear a link inherited from a placeholder.</remarks>
    [Fact]
    public void AnEmptyRelationshipIdAloneIsNotAField()
        => DrawingHyperlink.MakesField(Link("r:id=\"\"")).ShouldBeFalse();

    [Fact]
    public void AStatedRelationshipIdIsAField()
        => DrawingHyperlink.MakesField(Link("r:id=\"rId2\"")).ShouldBeTrue();

    /// <remarks>
    /// <c>tooltip</c> sets <c>PROP_Representation</c> on its own (<c>hyperlinkcontext.cxx</c>:59-61),
    /// so the map is not empty and the run is a field even with no target at all.
    /// </remarks>
    [Theory]
    [InlineData("r:id=\"\" tooltip=\"go\"")]
    [InlineData("r:id=\"\" tgtFrame=\"_blank\"")]
    [InlineData("action=\"ppaction://hlinkshowjump?jump=nextslide\"")]
    [InlineData("r:id=\"\" invalidUrl=\"x\"")]
    [InlineData("r:id=\"\" history=\"0\"")]
    [InlineData("r:id=\"\" highlightClick=\"1\"")]
    [InlineData("r:id=\"\" endSnd=\"1\"")]
    public void AnyStatedPropertyMakesItAField(string attributes)
        => DrawingHyperlink.MakesField(Link(attributes)).ShouldBeTrue();

    /// <remarks>
    /// The defaults are the other way round for <c>history</c> than for the two sound flags, and
    /// stating a value equal to the default records nothing (<c>:145-155</c>).
    /// </remarks>
    [Theory]
    [InlineData("r:id=\"\" history=\"1\"")]
    [InlineData("r:id=\"\" highlightClick=\"0\"")]
    [InlineData("r:id=\"\" endSnd=\"false\"")]
    public void AFlagStatedAtItsDefaultRecordsNothing(string attributes)
        => DrawingHyperlink.MakesField(Link(attributes)).ShouldBeFalse();

    /// <remarks>
    /// An <c>a:extLst</c> child sets <c>PROP_CharColor</c> (<c>:167-169</c>), which both makes the
    /// map non-empty and stops <c>textrun.cxx</c>:162 imposing the theme's <c>hlink</c> slot.
    /// </remarks>
    [Fact]
    public void AnExtensionListIsAFieldThatStatesItsOwnColour()
    {
        XElement link = Link("r:id=\"\"", "<a:extLst/>");

        DrawingHyperlink.MakesField(link).ShouldBeTrue();
        DrawingHyperlink.StatesItsOwnColour(link).ShouldBeTrue();
    }

    [Fact]
    public void AnOrdinaryLinkDoesNotStateItsOwnColour()
        => DrawingHyperlink.StatesItsOwnColour(Link("r:id=\"rId2\"")).ShouldBeFalse();
}
