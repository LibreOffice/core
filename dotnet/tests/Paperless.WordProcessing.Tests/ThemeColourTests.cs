using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.TestKit;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests DOCX theme-colour resolution, from the theme part through to a run's colour.
/// </summary>
/// <remarks>
/// The transform chain itself is checked against LibreOffice's rendering in
/// <c>ThemeColourComparisonTests</c>; what is checked here is the WordprocessingML half — which
/// slot a name reaches, what <c>w:themeTint</c> and <c>w:themeShade</c> mean, and the precedence
/// between the cached <c>w:val</c> and the reference beside it.
/// </remarks>
public class ThemeColourTests
{
    private static DocxFile Open(string name)
        => DocxFile.Open(File.OpenRead(Corpus.Require(name)));

    [Fact]
    public void TheThemePartIsFoundByRelationshipAndItsSchemeRead()
    {
        using DocxFile file = Open("theme-colours.docx");

        DrawingTheme theme = file.Theme.ShouldNotBeNull();

        theme.Lookup(ThemeColourSlot.Accent1).ShouldBe(Colour.FromRgb(0x4F81BD));
        theme.Lookup(ThemeColourSlot.Dark2).ShouldBe(Colour.FromRgb(0x1F497D));

        // dk1 and lt1 are a:sysClr with a lastClr, not a:srgbClr — which is what every theme
        // Word ships writes, so reading only a:srgbClr loses exactly the two slots that
        // "text 1" and "background 1" refer to.
        theme.Lookup(ThemeColourSlot.Dark1).ShouldBe(Colour.Black);
        theme.Lookup(ThemeColourSlot.Light1).ShouldBe(Colour.White);
    }

    [Fact]
    public void WordsLongSlotNamesReachTheSameColoursAsDrawingMlsShortOnes()
    {
        using DocxFile file = Open("theme-colours.docx");
        DrawingTheme theme = file.Theme.ShouldNotBeNull();

        // The identity colour map, which is what Word writes into every settings.xml, so
        // "text 2" is the second dark colour and "background 2" the second light one.
        theme.Lookup("text2").ShouldBe(theme.Lookup(ThemeColourSlot.Dark2));
        theme.Lookup("tx2").ShouldBe(theme.Lookup(ThemeColourSlot.Dark2));
        theme.Lookup("dk2").ShouldBe(theme.Lookup(ThemeColourSlot.Dark2));
        theme.Lookup("background2").ShouldBe(theme.Lookup(ThemeColourSlot.Light2));
        theme.Lookup("followedHyperlink").ShouldBe(theme.Lookup(ThemeColourSlot.FollowedHyperlink));
    }

    /// <summary>
    /// A colour map is what makes "background 1" something other than the first light colour.
    /// </summary>
    /// <remarks>
    /// The swapped map is the dark-master case: <c>bg1</c> follows the swap and lands on the
    /// theme's dark colour while <c>lt1</c>, which names the theme's storage rather than the
    /// document's idea of a background, does not move. Skipping the map gives a white background
    /// with white text on precisely the slides that use one.
    /// </remarks>
    [Fact]
    public void AColourMapMovesTheDocumentFacingNamesAndNotTheThemesOwn()
    {
        using DocxFile file = Open("theme-colours.docx");
        DrawingTheme theme = file.Theme.ShouldNotBeNull();

        DrawingTheme swapped = theme.WithMap(DrawingColourMap.Read(
            new XElement(
                Drawing.Name("clrMap"),
                new XAttribute("bg1", "dk1"),
                new XAttribute("tx1", "lt1"),
                new XAttribute("bg2", "lt2"),
                new XAttribute("tx2", "dk2"))));

        swapped.Lookup("bg1").ShouldBe(Colour.Black);
        swapped.Lookup("tx1").ShouldBe(Colour.White);
        swapped.Lookup("lt1").ShouldBe(Colour.White);
        swapped.Lookup("dk1").ShouldBe(Colour.Black);
    }

    /// <summary>
    /// Word's tint and shade are luminance modulation, not DrawingML's tint and shade.
    /// </summary>
    /// <remarks>
    /// <c>w:themeTint="99"</c> is the Word UI's "lighter 40%": 0x99 is 153, and 153/255 is 60%,
    /// so it becomes <c>lumMod</c> 60% with a <c>lumOff</c> of 40% making up the difference.
    /// <c>w:themeShade="BF"</c> is "darker 25%": 0xBF is 191, 191/255 is 74.902%, and there is
    /// no offset. Reading either as the identically named DrawingML transform gives a colour
    /// that is wrong in the right direction, which is the hardest kind to notice.
    /// </remarks>
    [Fact]
    public void ThemeTintAndThemeShadeBecomeLuminanceModulation()
    {
        WordThemeColour.Modifiers(tint: "99", shade: null).ShouldBe(
        [
            new ColourTransform(ColourTransformKind.LuminanceModulation, 60000),
            new ColourTransform(ColourTransformKind.LuminanceOffset, 40000),
        ]);

        WordThemeColour.Modifiers(tint: null, shade: "BF").ShouldBe(
            [new ColourTransform(ColourTransformKind.LuminanceModulation, 74902)]);

        WordThemeColour.Modifiers(tint: null, shade: null).ShouldBeEmpty();
    }

    [Fact]
    public void AThemedRunResolvesToTheColourWordCached()
    {
        using DocxFile file = Open("theme-colours.docx");

        List<Colour?> colours = [.. RunColours(file)];

        // 1. w:val beside w:themeColor: the cache wins, exactly.
        colours[0].ShouldBe(Colour.FromRgb(0x4F81BD));

        // 2. w:val="auto" is not a colour, so the theme reference beside it decides. Accent 1
        //    lightened 40% is 95B3D7, which is what Word's own colour picker calls it.
        colours[1].ShouldBe(Colour.FromRgb(0x95B3D7));

        // 3. No w:val at all. Accent 1 darkened 25%; Word caches 365F91 for this pair and the
        //    chain lands one unit away per channel, which is LibreOffice's rounding rather than
        //    a different answer.
        colours[2].ShouldBe(Colour.FromRgb(0x376092));

        // 4. A bare slot reference with no modifier resolves to the slot itself.
        colours[3].ShouldBe(Colour.FromRgb(0x1F497D));

        // 5. Background 2 darkened 50%, reached through the identity colour map: 948A54, which
        //    is the value Word's own picker gives for that swatch of the Office theme.
        colours[4].ShouldBe(Colour.FromRgb(0x948A54));
    }

    /// <summary>
    /// A document with no theme part leaves a themed colour unresolved rather than guessing.
    /// </summary>
    /// <remarks>
    /// Null rather than black, because null means "nothing set a colour" and lets the document
    /// default apply, while black is a claim. The distinction matters for a run whose paragraph
    /// style sets a colour: guessing black would override it.
    /// </remarks>
    [Fact]
    public void WithoutAThemeAThemedColourIsUnresolvedRatherThanBlack()
    {
        XElement colour = new(
            W("color"),
            new XAttribute(W("themeColor"), "accent1"),
            new XAttribute(W("themeShade"), "BF"));

        WordThemeColour.Read(colour, theme: null).ShouldBeNull();
    }

    private static XName W(string localName)
        => XName.Get(localName, OoxmlNamespaces.WordprocessingML);

    private static IEnumerable<Colour?> RunColours(DocxFile file)
        => file.Body!.Elements(W("p"))
               .Select(paragraph => paragraph.Element(W("r")))
               .Where(run => run?.Element(W("t")) is not null)
               .Select(run => WordCharacterFormat
                   .Resolve(file.Styles, run!.Element(W("rPr")), paragraphStyleId: null, file.Theme)
                   .Colour);
}
