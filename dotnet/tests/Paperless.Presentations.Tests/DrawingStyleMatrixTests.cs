using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A shape's <c>p:style</c> resolved against the theme's <c>a:fmtScheme</c>.
/// </summary>
/// <remarks>
/// <para>
/// The case this exists for is a shape that states no fill and no outline at all and is still
/// painted, which is most of what PowerPoint draws: a flowchart box outlined in accent 1, an
/// arrow filled with it, a master's rule under the title. Nothing read the reference, so all
/// three came out invisible while every text comparison passed.
/// </para>
/// <para>
/// Measured on two corpus decks. <c>Statement of Work presentation.pptx</c> drew three rounded
/// boxes, two arrows and a banner as blank space — its second page's ink imbalance against
/// LibreOffice went from 0.95% to under the image comparison's threshold — and
/// <c>111006 COMSTAC STOWG Aero spaceports IFG.pptx</c> lost the rule its master draws under
/// every title, which is five of its eight pages.
/// </para>
/// </remarks>
public class DrawingStyleMatrixTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string P = "http://schemas.openxmlformats.org/presentationml/2006/main";

    /// <summary>A theme whose first line style is thin and second is thick, both in phClr.</summary>
    private static XElement Theme() => XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}">
           <a:themeElements>
             <a:clrScheme name="t">
               <a:dk1><a:srgbClr val="000000"/></a:dk1>
               <a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
               <a:dk2><a:srgbClr val="1F497D"/></a:dk2>
               <a:lt2><a:srgbClr val="EEECE1"/></a:lt2>
               <a:accent1><a:srgbClr val="4F81BD"/></a:accent1>
               <a:accent2><a:srgbClr val="C0504D"/></a:accent2>
             </a:clrScheme>
             <a:fmtScheme name="Office">
               <a:fillStyleLst>
                 <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                 <a:solidFill><a:schemeClr val="phClr"><a:tint val="50000"/></a:schemeClr></a:solidFill>
               </a:fillStyleLst>
               <a:lnStyleLst>
                 <a:ln w="9525" cap="flat"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                   <a:prstDash val="solid"/></a:ln>
                 <a:ln w="25400" cap="flat"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                   <a:prstDash val="dash"/></a:ln>
               </a:lnStyleLst>
               <a:bgFillStyleLst>
                 <a:solidFill><a:schemeClr val="phClr"/></a:solidFill>
                 <a:gradFill><a:gsLst>
                   <a:gs pos="0"><a:schemeClr val="phClr"/></a:gs>
                   <a:gs pos="100000"><a:schemeClr val="phClr"><a:shade val="30000"/></a:schemeClr></a:gs>
                 </a:gsLst></a:gradFill>
                 <a:blipFill><a:blip r:embed="rId9" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>
                   <a:stretch/></a:blipFill>
               </a:bgFillStyleLst>
             </a:fmtScheme>
           </a:themeElements>
         </a:theme>
         """);

    private static DrawingTheme? Colours() => DrawingTheme.Read(Theme());

    private static DrawingStyleMatrix Matrix()
        => DrawingStyleMatrix.Read(Theme()).ShouldNotBeNull();

    private static XElement Style(string inner)
        => XElement.Parse($"<p:style xmlns:p=\"{P}\" xmlns:a=\"{A}\">{inner}</p:style>");

    [Fact]
    public void AFillReferenceTakesTheThemeStyleInTheReferencesOwnColour()
    {
        XElement? fill = Matrix().Fill(
            Style("<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"), Colours());

        fill.ShouldNotBeNull();

        XElement solid = Drawing.Child(fill, "solidFill").ShouldNotBeNull();
        Drawing.Attribute(Drawing.Child(solid, "srgbClr"), "val").ShouldBe("4F81BD");
    }

    [Fact]
    public void TheReferencesOwnTransformsSurviveIntoTheSubstitutedColour()
    {
        // An a:lnRef naming accent1 under a shade of 50% is a *darker* accent 1, and the darkening
        // belongs to the reference rather than to the theme's entry. Dropping it is the difference
        // between a flowchart's outline and its fill being the same colour.
        XElement? line = Matrix().Line(
            Style("""
                  <a:lnRef idx="1"><a:schemeClr val="accent1"><a:shade val="50000"/></a:schemeClr></a:lnRef>
                  """),
            Colours());

        line.ShouldNotBeNull();

        string? value = Drawing.Attribute(
            Drawing.Child(Drawing.Child(line, "solidFill"), "srgbClr"), "val");

        // 0x4F81BD shaded to half. Not accent1 itself, which is the assertion that matters.
        value.ShouldNotBeNull();
        value.ShouldNotBe("4F81BD");
        Convert.ToInt32(value, 16).ShouldBeLessThan(0x4F81BD);
    }

    [Fact]
    public void TheThemeStyleKeepsItsOwnWidthAndDash()
    {
        XElement? line = Matrix().Line(
            Style("<a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef>"), Colours());

        line.ShouldNotBeNull();
        Drawing.Attribute(line, "w").ShouldBe("25400");
        Drawing.Attribute(Drawing.Child(line, "prstDash"), "val").ShouldBe("dash");
    }

    [Fact]
    public void IndexZeroNamesNothingAtAll()
    {
        // a:fillRef idx="0" is how a shape says it takes no fill from the theme. Treating the
        // index as one-based-with-a-fallback fills every such shape with the theme's first style.
        Matrix().Fill(
            Style("<a:fillRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:fillRef>"), Colours())
            .ShouldBeNull();

        Matrix().Line(Style("<a:lnRef idx=\"4\"/>"), Colours()).ShouldBeNull();
        Matrix().Fill(style: null, Colours()).ShouldBeNull();
    }

    [Fact]
    public void AShapeStatingHalfALineKeepsTheThemesOtherHalf()
    {
        // The master connector of the COMSTAC deck: <a:ln w="57150"/> under an a:lnRef, meaning
        // "the theme's first line style, four and a half points wide". Taking the shape's element
        // alone loses the colour and draws nothing; taking the theme's alone draws the rule at
        // three quarters of a point.
        XElement themed = Matrix().Line(
            Style("<a:lnRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:lnRef>"), Colours())
            .ShouldNotBeNull();

        XElement own = XElement.Parse($"<a:ln xmlns:a=\"{A}\" w=\"57150\"/>");
        XElement merged = DrawingStyleMatrix.Overlay(themed, own);

        Drawing.Attribute(merged, "w").ShouldBe("57150");
        Drawing.Attribute(merged, "cap").ShouldBe("flat");
        Drawing.Attribute(
            Drawing.Child(Drawing.Child(merged, "solidFill"), "srgbClr"), "val").ShouldBe("4F81BD");
    }

    [Fact]
    public void AStatedFillReplacesTheThemesRatherThanJoiningIt()
    {
        XElement themed = Matrix().Line(
            Style("<a:lnRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:lnRef>"), Colours())
            .ShouldNotBeNull();

        XElement own = XElement.Parse($"<a:ln xmlns:a=\"{A}\"><a:noFill/></a:ln>");
        XElement merged = DrawingStyleMatrix.Overlay(themed, own);

        Drawing.Child(merged, "noFill").ShouldNotBeNull();
        Drawing.Child(merged, "solidFill").ShouldBeNull();
    }

    private static XElement BackgroundRef(string inner)
        => XElement.Parse($"<p:bgRef xmlns:p=\"{P}\" xmlns:a=\"{A}\" {inner}</p:bgRef>");

    [Fact]
    public void ABackgroundReferenceIndexesTheBackgroundListWithAThousandSubtracted()
    {
        // idx="1003" is the *third* background style, not the thousand-and-third of anything:
        // Theme::getFillStyle sends 1000 and up to a:bgFillStyleLst with 1000 taken off, and
        // everything below it to a:fillStyleLst (oox/source/drawingml/theme.cxx:49-54). Reading
        // the number without the split takes the wrong list or none, and the slide comes out white.
        XElement first = Matrix()
            .Background(BackgroundRef("idx=\"1001\"><a:schemeClr val=\"accent2\"/>"), Colours())
            .ShouldNotBeNull();

        Drawing.Attribute(
            Drawing.Child(Drawing.Child(first, "solidFill"), "srgbClr"), "val").ShouldBe("C0504D");

        XElement second = Matrix()
            .Background(BackgroundRef("idx=\"1002\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldNotBeNull();

        Drawing.Child(second, "gradFill").ShouldNotBeNull();

        XElement third = Matrix()
            .Background(BackgroundRef("idx=\"1003\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldNotBeNull();

        Drawing.Child(third, "blipFill").ShouldNotBeNull();
    }

    [Fact]
    public void ABackgroundReferenceBelowAThousandIndexesTheOrdinaryFillList()
    {
        XElement fill = Matrix()
            .Background(BackgroundRef("idx=\"1\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldNotBeNull();

        Drawing.Attribute(
            Drawing.Child(Drawing.Child(fill, "solidFill"), "srgbClr"), "val").ShouldBe("4F81BD");
    }

    [Fact]
    public void ABackgroundIndexPastTheEndTakesTheLastEntryRatherThanNothing()
    {
        // lclGetStyleElement clamps rather than rejects, and themes from other producers do state
        // an index past the end. Rejecting it leaves the slide white, which is the failure this
        // whole path exists to stop.
        XElement fill = Matrix()
            .Background(BackgroundRef("idx=\"1009\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldNotBeNull();

        Drawing.Child(fill, "blipFill").ShouldNotBeNull();
    }

    [Fact]
    public void ABackgroundReferenceNamingNothingResolvesNothing()
    {
        Matrix().Background(reference: null, Colours()).ShouldBeNull();
        Matrix().Background(BackgroundRef("idx=\"1000\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldBeNull();
        Matrix().Background(BackgroundRef("idx=\"0\"><a:schemeClr val=\"accent1\"/>"), Colours())
            .ShouldBeNull();
    }

    [Fact]
    public void AThemeWithNoFormatSchemeResolvesNothing()
        => DrawingStyleMatrix.Read(XElement.Parse($"<a:theme xmlns:a=\"{A}\"/>")).ShouldBeNull();

    [Fact]
    public void TheSubstitutedColourKeepsItsAlpha()
    {
        // Stated on the colour rather than on the fill, so it has to survive the substitution —
        // a diagram's overlapping circles are the same accent at half opacity.
        XElement fill = DrawingStyleMatrix.Substitute(
            XElement.Parse($"<a:solidFill xmlns:a=\"{A}\"><a:schemeClr val=\"phClr\"/></a:solidFill>"),
            new Colour(0x40, 0x50, 0x60, 0x80));

        XElement colour = Drawing.Child(fill, "srgbClr").ShouldNotBeNull();
        Drawing.Attribute(colour, "val").ShouldBe("405060");
        Drawing.Attribute(Drawing.Child(colour, "alpha"), "val").ShouldBe("50196");
    }
}
