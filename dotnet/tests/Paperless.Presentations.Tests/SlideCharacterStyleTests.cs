using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.TestKit;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the character properties a slide's text resolves to that its content tree does not
/// carry: colour, size and typeface.
/// </summary>
/// <remarks>
/// The ordering of the chain, and the arithmetic inside it, are measured against LibreOffice's
/// own render in <c>SlideTextStyleComparisonTests</c>. What is tested here is everything that
/// measurement cannot separate: the indirections, the empty-string conventions, and what an
/// absent theme means — each of which has a wrong answer that looks like a right one.
/// </remarks>
public class SlideCharacterStyleTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string P = "http://schemas.openxmlformats.org/presentationml/2006/main";

    /// <summary>
    /// Parses a fragment, declaring both prefixes around it.
    /// </summary>
    /// <remarks>
    /// Both, because a shape style is the one element here that is <em>not</em> DrawingML: it is
    /// <c>p:style</c> holding <c>a:fontRef</c>, and reading it has to reach across the boundary.
    /// </remarks>
    private static XElement Drawing(string markup)
        => XElement.Parse($"<wrapper xmlns:a=\"{A}\" xmlns:p=\"{P}\">{markup}</wrapper>")
                   .Elements().First();

    private static readonly DrawingFontScheme Fonts = new(
        MajorLatin: "Cambria", MajorAsian: null, MajorComplex: null,
        MinorLatin: "Calibri", MinorAsian: "Meiryo", MinorComplex: null);

    private static DrawingTheme Theme(DrawingColourMap? map = null)
        => new(
            DrawingColourScheme.Read(Drawing(
                $"<a:clrScheme name=\"t\">"
                + "<a:dk1><a:srgbClr val=\"111111\"/></a:dk1>"
                + "<a:lt1><a:srgbClr val=\"EEEEEE\"/></a:lt1>"
                + "<a:dk2><a:srgbClr val=\"1F497D\"/></a:dk2>"
                + "<a:lt2><a:srgbClr val=\"EEECE1\"/></a:lt2>"
                + "<a:accent1><a:srgbClr val=\"4F81BD\"/></a:accent1>"
                + "</a:clrScheme>")),
            map ?? DrawingColourMap.Identity)
        {
            Fonts = Fonts,
        };

    [Fact]
    public void AnIndirectTypefaceIsResolvedThroughTheThemesFontScheme()
    {
        DrawingCharacterStyle style = DrawingCharacterStyle.Read(
            Drawing("<a:defRPr><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mj-ea\"/></a:defRPr>"),
            Theme());

        style.LatinTypeface.ShouldBe("Calibri");

        // The theme's major East Asian face is unset, so the indirection resolves to nothing
        // rather than to the placeholder name.
        style.AsianTypeface.ShouldBeNull();
    }

    /// <summary>
    /// A direct name is passed through, and a <c>+</c> name without a theme is dropped.
    /// </summary>
    /// <remarks>
    /// Reporting "+mn-lt" as a family name is the failure this guards: it is not a font, so it
    /// would end the search on a face that cannot be found, where null falls through to the next
    /// source in the chain — which is what a reader with no theme part has to do.
    /// </remarks>
    [Fact]
    public void WithoutAThemeAnIndirectTypefaceIsNothingAndADirectOneSurvives()
    {
        DrawingCharacterStyle indirect = DrawingCharacterStyle.Read(
            Drawing("<a:defRPr><a:latin typeface=\"+mn-lt\"/></a:defRPr>"), theme: null);
        indirect.LatinTypeface.ShouldBeNull();

        DrawingCharacterStyle direct = DrawingCharacterStyle.Read(
            Drawing("<a:defRPr><a:latin typeface=\"Gill Sans\"/></a:defRPr>"), theme: null);
        direct.LatinTypeface.ShouldBe("Gill Sans");
    }

    /// <summary>
    /// <c>sz</c> is hundredths of a point, not the half-points <c>w:sz</c> uses.
    /// </summary>
    /// <remarks>
    /// The two units sit in the same package — a DOCX shape holds DrawingML text inside
    /// WordprocessingML — and there is a factor of fifty between them, so a run stated at 18 pt
    /// comes out at 900 pt if the wrong one is applied.
    /// </remarks>
    [Fact]
    public void ASizeIsInHundredthsOfAPoint()
    {
        DrawingCharacterStyle.Read(Drawing("<a:defRPr sz=\"1800\"/>"), theme: null)
            .Size.ShouldBe(Length.FromPoints(18));

        DrawingCharacterStyle.Read(Drawing("<a:defRPr sz=\"0\"/>"), theme: null).Size.ShouldBeNull();
    }

    /// <summary>
    /// A shape style's colour goes through the colour map, as every scheme reference does.
    /// </summary>
    /// <remarks>
    /// A dark master states <c>&lt;p:clrMap bg1="dk1" tx1="lt1" …/&gt;</c>, and a shape on it
    /// whose <c>a:fontRef</c> asks for <c>tx1</c> means the theme's <em>light</em> colour. Read
    /// without the map it gets the dark one, which is invisible against the master's dark
    /// background — the one case where skipping the map is not a rounding error but unreadable
    /// text.
    /// </remarks>
    [Fact]
    public void AShapeStylesColourFollowsTheMastersColourMap()
    {
        XElement style = Drawing(
            "<p:style><a:fontRef idx=\"minor\"><a:schemeClr val=\"tx1\"/></a:fontRef></p:style>");

        DrawingCharacterStyle plain = DrawingCharacterStyle.FromShapeStyle(style, Theme());
        plain.Colour.ShouldBe(Colour.FromRgb(0x111111));

        DrawingColourMap swapped = DrawingColourMap.Read(
            Drawing("<a:clrMap bg1=\"dk1\" tx1=\"lt1\" bg2=\"lt2\" tx2=\"dk2\"/>"));

        DrawingCharacterStyle dark = DrawingCharacterStyle.FromShapeStyle(style, Theme(swapped));
        dark.Colour.ShouldBe(Colour.FromRgb(0xEEEEEE));
    }

    /// <summary>
    /// A font reference of <c>none</c> contributes a colour but no typeface.
    /// </summary>
    /// <remarks>
    /// The two halves of an <c>a:fontRef</c> are independent — the index picks the theme's face
    /// and the child element states the colour — so a reference that opts out of the face still
    /// paints the text. LibreOffice reads them separately for the same reason
    /// (<c>oox/source/drawingml/shape.cxx</c>:2242, where the themed index is tested but the
    /// placeholder colour is applied outside that test).
    /// </remarks>
    [Fact]
    public void AFontReferenceWithNoThemedIndexStillCarriesItsColour()
    {
        DrawingCharacterStyle style = DrawingCharacterStyle.FromShapeStyle(
            Drawing("<p:style><a:fontRef idx=\"none\"><a:srgbClr val=\"C0504D\"/></a:fontRef></p:style>"),
            Theme());

        style.Colour.ShouldBe(Colour.FromRgb(0xC0504D));
        style.LatinTypeface.ShouldBeNull();
    }

    /// <summary>
    /// The theme's font scheme is read off a real deck, including the empty-string convention.
    /// </summary>
    /// <remarks>
    /// Every theme Word ships writes <c>&lt;a:ea typeface=""/&gt;</c> rather than omitting the
    /// element, so an attribute-presence test yields a typeface named "" and hands it to font
    /// resolution, which then substitutes for it.
    /// </remarks>
    [Fact]
    public void ADecksFontSchemeIsReadFromItsMastersTheme()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("deck-text-style.pptx"));
        using PptxFile file = PptxFile.Open(stream);

        DrawingFontScheme fonts = file.Slides[0].Theme.ShouldNotBeNull().Fonts.ShouldNotBeNull();

        fonts.MinorLatin.ShouldBe("Calibri");
        fonts.MajorLatin.ShouldBe("Cambria");
        fonts.MinorAsian.ShouldBeNull();
        fonts.MajorComplex.ShouldBeNull();
    }
}
