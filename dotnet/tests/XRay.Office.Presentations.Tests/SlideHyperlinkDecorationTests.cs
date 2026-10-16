using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.Ooxml;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What a slide run carrying an <c>a:hlinkClick</c> is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// The target was read and reported to extraction, and nothing decorated the run — so a linked
/// run drew in the body's colour with no rule under it, on every deck in the corpus. It is the
/// "read but never used" shape again: the word gate cannot see a colour and cannot see a rule,
/// so a defect this loud left the scoreboard untouched. Three independent blind readings of
/// pages 3, 5 and 15 of <c>slides/batch-004/pptx/solog_orientation_august_2019.pptx</c> each
/// ranked it the most obvious difference on the page, above everything the round was briefed on.
/// </para>
/// <para>
/// The rule is <c>oox/source/drawingml/textrun.cxx:145-170</c>: a run whose own <c>a:rPr</c>
/// holds an <c>a:hlinkClick</c> has its fill colour's <em>scheme slot</em> reassigned to
/// <c>hlink</c> and is underlined, each unless the run states otherwise itself.
/// </para>
/// <para>
/// Measured against the banked 26.2.4.2 reference for that deck: the reference emits the
/// theme's <c>hlink</c> at <c>0 0 1 rg</c> on pages 4, 5, 8, 9, 13 and 15 and at
/// <c>0.545 0.545 1 rg</c> on page 3, and we emitted no link colour on any of them. With this
/// rule the set of text fill colours agrees with the reference's on all fifteen pages.
/// </para>
/// </remarks>
public class SlideHyperlinkDecorationTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>The theme's link colour, chosen so it cannot be confused with a default.</summary>
    private static readonly Colour Link = new(0x00, 0x00, 0xFF);

    private static DrawingTheme Theme() => DrawingTheme.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements>
           <a:clrScheme name="probe">
             <a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
             <a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
             <a:dk2><a:srgbClr val="1F1F1F"/></a:dk2>
             <a:lt2><a:srgbClr val="EEEEEE"/></a:lt2>
             <a:accent1><a:srgbClr val="850F89"/></a:accent1>
             <a:accent2><a:srgbClr val="2A6E3F"/></a:accent2>
             <a:accent3><a:srgbClr val="1B587C"/></a:accent3>
             <a:accent4><a:srgbClr val="4E8542"/></a:accent4>
             <a:accent5><a:srgbClr val="604878"/></a:accent5>
             <a:accent6><a:srgbClr val="C19859"/></a:accent6>
             <a:hlink><a:srgbClr val="0000FF"/></a:hlink>
             <a:folHlink><a:srgbClr val="800080"/></a:folHlink>
           </a:clrScheme>
         </a:themeElements></a:theme>
         """))!;

    /// <summary>
    /// A one-run body: <paramref name="attributes"/> go on the <c>a:rPr</c> and
    /// <paramref name="children"/> inside it, so a case can vary either without the other.
    /// </summary>
    private static XElement Body(
        string attributes = "", string children = "", string levelDefault = "") => XElement.Parse(
        $"""
         <a:txBody xmlns:a="{A}" xmlns:r="{R}">
           <a:bodyPr/>
           <a:lstStyle>{levelDefault}</a:lstStyle>
           <a:p><a:r><a:rPr lang="en-US" sz="2000" {attributes}>{children}</a:rPr>
             <a:t>dtpoole@miami.edu</a:t></a:r></a:p>
         </a:txBody>
         """);

    private static SlideTextRun Run(
        string attributes = "", string children = "", string levelDefault = "")
        => PptxTextBody.Read(Body(attributes, children, levelDefault), Theme())
            .Paragraphs[0].Runs[0];

    /// <summary>The <c>a:hlinkClick</c> a linked run carries.</summary>
    private const string Linked = "<a:hlinkClick r:id=\"rId3\"/>";

    [Fact]
    public void APlainRunIsNeitherLinkColouredNorUnderlined()
    {
        // The control, and the one that matters most: a rule that decorated every run would pass
        // everything else in this file.
        SlideTextRun run = Run();

        run.Colour.ShouldBe(Colour.Black);
        run.IsUnderlined.ShouldBeFalse();
    }

    [Fact]
    public void ALinkedRunTakesTheThemesHyperlinkColour()
        => Run(children: Linked).Colour.ShouldBe(Link);

    [Fact]
    public void ALinkedRunIsUnderlined()
        => Run(children: Linked).IsUnderlined.ShouldBeTrue();

    /// <summary>
    /// The slot is swapped and the transform chain over it is kept.
    /// </summary>
    /// <remarks>
    /// <c>Color::setSchemeClr</c> assigns only <c>meMode</c> and <c>mnC1</c>
    /// (<c>oox/source/drawingml/color.cxx:405-413</c>), so the <c>tint</c>, <c>lumMod</c> or
    /// <c>alpha</c> already on the inherited fill survives. Measured: the deck's
    /// <c>slideLayout1.xml</c> tints its subtitle placeholder's <c>tx1</c> to 75%, drawing the
    /// body at <c>#8B8B8B</c>, and the reference draws that page's three <c>mailto:</c> runs at
    /// <c>#8B8BFF</c> rather than at the theme's flat <c>#0000FF</c> — the same tint over the
    /// link colour, which leaves a saturated blue channel at FF and lifts the other two to 8B.
    /// Resolving <c>hlink</c> on its own is right on the deck's other five linked pages and
    /// wrong on this one, which is exactly the kind of near-miss that survives a round.
    /// </remarks>
    [Fact]
    public void TheTintOnTheInheritedColourSurvivesTheSwapToTheLinkColour()
    {
        const string Tinted =
            "<a:lvl1pPr><a:defRPr><a:solidFill>"
            + "<a:schemeClr val=\"tx1\"><a:tint val=\"75000\"/></a:schemeClr>"
            + "</a:solidFill></a:defRPr></a:lvl1pPr>";

        Run(levelDefault: Tinted).Colour.ShouldBe(new Colour(0x8B, 0x8B, 0x8B));

        Run(children: Linked, levelDefault: Tinted)
            .Colour.ShouldBe(new Colour(0x8B, 0x8B, 0xFF));
    }

    /// <summary>
    /// A run's own literal colour loses to the link colour.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This was predicted the other way round and the prediction was refuted by the binary, so
    /// the reasoning is worth keeping. <c>Color::setSchemeClr</c> writes <c>meMode</c> and
    /// <c>mnC1</c>, and <c>mnC1</c> is exactly where <c>setSrgbClr</c> put the literal — so
    /// reassigning the slot <em>destroys</em> a literal rather than being unable to reach it.
    /// The run's own <c>a:solidFill</c> therefore does not survive.
    /// </para>
    /// <para>
    /// Measured rather than argued, because a citation against a 27.2-alpha tree is not the
    /// 26.2.4.2 binary that made the references.
    /// <c>slides/batch-003/pptx/ROK-PI Climate Bulletin - Edition 2017-06.pptx</c> states
    /// <c>&lt;a:srgbClr val="C00000"/&gt;</c> — a dark red — on the linked run reading
    /// <c>clikp.sprep.org</c>, and its theme's <c>hlink</c> is <c>0563C1</c>. Sampling the
    /// reference PDF's own page 1 at 150 dpi over that word's <c>pdftotext -bbox</c> rectangle
    /// gives 239 pixels of exactly <c>#0563C1</c> and not one pixel of <c>#C00000</c>.
    /// </para>
    /// <para>
    /// It is also the only reading under which the <c>a:extLst</c> escape hatch below means
    /// anything: a format would not need an extension to say "use the text's own colour" if
    /// stating that colour already worked.
    /// </para>
    /// </remarks>
    [Fact]
    public void ALinkedRunsOwnLiteralColourLosesToTheLinkColour()
        => Run(children: "<a:solidFill><a:srgbClr val=\"C00000\"/></a:solidFill>" + Linked)
            .Colour.ShouldBe(Link);

    /// <summary>
    /// A run stating its own underline keeps it, including <c>none</c>.
    /// </summary>
    /// <remarks>
    /// The guard is <c>if (!maTextCharacterProperties.moUnderline.has_value())</c> — the run's
    /// own properties, before the defaults are merged in — so it is only the run's own
    /// <c>@u</c> that can refuse the rule.
    /// </remarks>
    [Theory]
    [InlineData("u=\"none\"", false)]
    [InlineData("u=\"dbl\"", true)]
    public void ALinkedRunStatingItsOwnUnderlineKeepsIt(string underline, bool underlined)
        => Run(attributes: underline, children: Linked).IsUnderlined.ShouldBe(underlined);

    /// <summary>
    /// A <c>u="none"</c> inherited from a level default does <em>not</em> refuse the rule.
    /// </summary>
    /// <remarks>
    /// Asserted separately from the case above because the two look like one rule and are not:
    /// the guard reads the run's own <c>a:rPr</c>, so an inherited <c>u="none"</c> is invisible
    /// to it and the link still underlines. Getting this wrong is silent — every deck whose
    /// master writes <c>u="none"</c> on its body style, which is every deck LibreOffice itself
    /// exports, would lose every link rule.
    /// </remarks>
    [Fact]
    public void AnInheritedUnderlineOfNoneDoesNotRefuseTheLinksRule()
        => Run(children: Linked, levelDefault: "<a:lvl1pPr><a:defRPr u=\"none\"/></a:lvl1pPr>")
            .IsUnderlined.ShouldBeTrue();

    /// <summary>
    /// PowerPoint's "use the text's own colour" link extension is honoured.
    /// </summary>
    /// <remarks>
    /// An <c>a:extLst</c> under the <c>a:hlinkClick</c> is what
    /// <c>HyperLinkContext::onCreateContext</c> turns into a <c>PROP_CharColor</c> on the
    /// hyperlink's own property map (<c>oox/source/drawingml/hyperlinkcontext.cxx:166-169</c>),
    /// and that property is precisely the guard <c>textrun.cxx:162</c> tests before swapping the
    /// slot. The rule is still drawn: only the colour is refused.
    /// </remarks>
    [Fact]
    public void TheUseTextColourLinkExtensionRefusesTheColourAndKeepsTheRule()
    {
        SlideTextRun run = Run(children:
            "<a:hlinkClick r:id=\"rId3\"><a:extLst><a:ext uri=\"{A12FA001}\"/></a:extLst>"
            + "</a:hlinkClick>");

        run.Colour.ShouldBe(Colour.Black);
        run.IsUnderlined.ShouldBeTrue();
    }

    /// <summary>
    /// An internal jump — <c>action</c> with no <c>r:id</c> — decorates like any other link.
    /// </summary>
    /// <remarks>
    /// LibreOffice branches on the hyperlink property map being non-empty, which
    /// <c>HyperLinkContext</c> fills for a <c>ppaction://</c> just as it does for a relationship,
    /// so "go to the next slide" is a link on the page as much as a <c>mailto:</c> is.
    /// </remarks>
    [Fact]
    public void AnInternalJumpIsDecoratedToo()
    {
        SlideTextRun run = Run(children:
            "<a:hlinkClick action=\"ppaction://hlinkshowjump?jump=nextslide\"/>");

        run.Colour.ShouldBe(Link);
        run.IsUnderlined.ShouldBeTrue();
    }

    /// <summary>
    /// With no theme to look the slot up in, the run keeps the colour the chain gave it.
    /// </summary>
    /// <remarks>
    /// <c>DrawingColour.Resolve</c> returns null for a scheme reference it cannot follow, and
    /// null here has to mean "keep what the chain gave you". Blackening instead would repaint
    /// every linked run on every deck whose theme failed to parse.
    /// </remarks>
    [Fact]
    public void ARunWithNoThemeKeepsTheColourTheChainGaveIt()
    {
        SlideTextRun run = PptxTextBody
            .Read(Body(children: "<a:solidFill><a:srgbClr val=\"008000\"/></a:solidFill>" + Linked))
            .Paragraphs[0].Runs[0];

        run.Colour.ShouldBe(new Colour(0x00, 0x80, 0x00));
        run.IsUnderlined.ShouldBeTrue();
    }
}
