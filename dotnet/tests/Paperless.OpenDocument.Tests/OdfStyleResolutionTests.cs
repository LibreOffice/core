using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// Tests for the three-way style split and the parent-chain resolution over it.
/// </summary>
/// <remarks>
/// These use hand-built XML rather than corpus files on purpose: the point is to pin down
/// resolution <em>precedence</em>, and constructing a chain where each rule matters in
/// isolation is not something a real document conveniently does.
/// </remarks>
public class OdfStyleResolutionTests
{
    private const string Fo = OdfNamespaces.FoCompatible;
    private const string St = OdfNamespaces.Style;

    /// <summary>
    /// A document with a three-deep paragraph chain, a family default, an automatic style
    /// standing in for direct formatting, a master page, and a list style.
    /// </summary>
    private static OdfStyles BuildStyles(List<Diagnostic>? diagnostics = null)
    {
        XElement root = XElement.Parse($$"""
            <office:document-styles
                xmlns:office="{{OdfNamespaces.Office}}"
                xmlns:style="{{St}}"
                xmlns:fo="{{Fo}}"
                xmlns:text="{{OdfNamespaces.Text}}"
                xmlns:table="{{OdfNamespaces.Table}}">
              <office:styles>
                <style:default-style style:family="paragraph">
                  <style:text-properties fo:font-size="12pt" fo:language="en" fo:country="GB"
                                         fo:color="#101010"/>
                  <style:paragraph-properties fo:text-align="start"/>
                </style:default-style>
                <style:default-style style:family="table">
                  <style:table-properties table:display="true"/>
                </style:default-style>

                <style:style style:name="Standard" style:family="paragraph">
                  <style:text-properties fo:font-size="11pt"/>
                </style:style>
                <style:style style:name="Heading" style:family="paragraph"
                             style:parent-style-name="Standard">
                  <style:text-properties fo:font-weight="bold"/>
                </style:style>
                <style:style style:name="Heading_20_1" style:display-name="Heading 1"
                             style:family="paragraph" style:parent-style-name="Heading"
                             style:default-outline-level="1">
                  <style:text-properties fo:font-size="20pt"/>
                </style:style>
                <style:style style:name="Emphasis" style:family="text">
                  <style:text-properties fo:font-style="italic"/>
                </style:style>
                <style:style style:name="Cycle_20_A" style:family="paragraph"
                             style:parent-style-name="Cycle_20_B"/>
                <style:style style:name="Cycle_20_B" style:family="paragraph"
                             style:parent-style-name="Cycle_20_A"/>

                <text:list-style style:name="Numbers">
                  <text:list-level-style-number text:level="1" style:num-format="1"
                                                style:num-suffix="."/>
                  <text:list-level-style-number text:level="2" style:num-format="a"
                                                style:num-suffix=")" text:start-value="3"/>
                  <text:list-level-style-number text:level="3" style:num-format="i"
                                                text:display-levels="2"/>
                </text:list-style>
                <text:list-style style:name="Bullets">
                  <text:list-level-style-bullet text:level="1" text:bullet-char="&#xF0B7;"/>
                </text:list-style>
              </office:styles>

              <office:automatic-styles>
                <style:style style:name="T1" style:family="text"
                             style:parent-style-name="Emphasis">
                  <style:text-properties fo:font-weight="bold"/>
                </style:style>
                <style:page-layout style:name="pm1">
                  <style:page-layout-properties fo:page-width="21.001cm" fo:margin-left="2cm"/>
                </style:page-layout>
              </office:automatic-styles>

              <office:master-styles>
                <style:master-page style:name="Standard" style:page-layout-name="pm1">
                  <style:header><text:p>Header text</text:p></style:header>
                </style:master-page>
              </office:master-styles>
            </office:document-styles>
            """);

        OdfStyles styles = new();
        styles.AddDocument(root, diagnostics);
        return styles;
    }

    [Fact]
    public void SetOnTheStyleItselfIsReportedAsSetHere()
    {
        OdfProperty size = BuildStyles().ResolveProperty(
            "Heading_20_1", OdfStyleFamily.Paragraph, OdfPropertyKind.Text, Fo, "font-size");

        size.Value.ShouldBe("20pt");
        size.Origin.ShouldBe(OdfPropertyOrigin.SetHere);
        size.IsSetHere.ShouldBeTrue();
        size.SourceStyleName.ShouldBe("Heading_20_1");
        size.AsLength()!.Value.Points.ShouldBe(20);
    }

    [Fact]
    public void ValueFromTheParentChainIsReportedAsInherited()
    {
        OdfProperty weight = BuildStyles().ResolveProperty(
            "Heading_20_1", OdfStyleFamily.Paragraph, OdfPropertyKind.Text, Fo, "font-weight");

        weight.Value.ShouldBe("bold");
        weight.Origin.ShouldBe(OdfPropertyOrigin.Inherited);
        weight.IsSetHere.ShouldBeFalse();
        // Knowing *which* ancestor supplied it is the point of tracking the source.
        weight.SourceStyleName.ShouldBe("Heading");
    }

    [Fact]
    public void ValueFromTheFamilyDefaultIsReportedAsDefaulted()
    {
        OdfProperty colour = BuildStyles().ResolveProperty(
            "Heading_20_1", OdfStyleFamily.Paragraph, OdfPropertyKind.Text, Fo, "color");

        colour.Origin.ShouldBe(OdfPropertyOrigin.Defaulted);
        colour.AsColour().ShouldBe(new Core.Graphics.Colour(0x10, 0x10, 0x10));
    }

    [Fact]
    public void AnUnsetPropertyIsDistinguishableFromOneSetToAnything()
    {
        OdfProperty absent = BuildStyles().ResolveProperty(
            "Heading_20_1", OdfStyleFamily.Paragraph, OdfPropertyKind.Text, Fo, "font-variant");

        absent.HasValue.ShouldBeFalse();
        absent.Origin.ShouldBe(OdfPropertyOrigin.Unset);
        absent.Value.ShouldBeNull();
    }

    [Fact]
    public void TheNearestStyleInTheChainWins()
    {
        // Standard sets 11pt, Heading 1 sets 20pt, the default says 12pt.
        BuildStyles().ResolveProperty("Standard", OdfStyleFamily.Paragraph,
                                      OdfPropertyKind.Text, Fo, "font-size")
                     .Value.ShouldBe("11pt");
    }

    [Fact]
    public void PropertyKindSeparatesAttributesThatShareAName()
    {
        OdfStyles styles = BuildStyles();

        // fo:text-align exists only in paragraph properties, so asking for it as a text
        // property must not find the paragraph one.
        styles.ResolveProperty("Standard", OdfStyleFamily.Paragraph,
                               OdfPropertyKind.Paragraph, Fo, "text-align")
              .Value.ShouldBe("start");
        styles.ResolveProperty("Standard", OdfStyleFamily.Paragraph,
                               OdfPropertyKind.Text, Fo, "text-align")
              .HasValue.ShouldBeFalse();
    }

    [Fact]
    public void AutomaticStylesAreFoundAndKnowTheyAreAutomatic()
    {
        OdfStyles styles = BuildStyles();

        OdfStyle? automatic = styles.Find("T1", OdfStyleFamily.Text);
        automatic.ShouldNotBeNull();
        automatic.Source.ShouldBe(OdfStyleSource.Automatic);

        // ODF has no inline style attribute, so "set on the automatic style" is the only
        // evidence that a user applied direct formatting.
        styles.ResolveProperty("T1", OdfStyleFamily.Text, OdfPropertyKind.Text, Fo, "font-weight")
              .IsSetHere.ShouldBeTrue();
        styles.ResolveProperty("T1", OdfStyleFamily.Text, OdfPropertyKind.Text, Fo, "font-style")
              .Origin.ShouldBe(OdfPropertyOrigin.Inherited);
    }

    [Fact]
    public void CharacterDefaultsFallBackToTheParagraphFamilyDefault()
    {
        // ODF declares no style:default-style for the text family: default character
        // formatting lives in the paragraph default's text properties.
        BuildStyles().ResolveProperty("Emphasis", OdfStyleFamily.Text,
                                      OdfPropertyKind.Text, Fo, "font-size")
                     .Origin.ShouldBe(OdfPropertyOrigin.Defaulted);
    }

    [Fact]
    public void ACircularParentChainTerminates()
    {
        // Illegal but present in files written by buggy converters, and an infinite loop
        // over untrusted input is not an acceptable failure mode.
        BuildStyles().ResolveProperty("Cycle_20_A", OdfStyleFamily.Paragraph,
                                      OdfPropertyKind.Text, Fo, "font-size")
                     .Origin.ShouldBe(OdfPropertyOrigin.Defaulted);
    }

    [Fact]
    public void ANullStyleNameResolvesToTheFamilyDefaults()
    {
        BuildStyles().ResolveProperty(null, OdfStyleFamily.Paragraph,
                                      OdfPropertyKind.Text, Fo, "font-size")
                     .Value.ShouldBe("12pt");
    }

    [Fact]
    public void ACascadeLetsTheInnerStyleWinButNotItsDefaults()
    {
        OdfStyles styles = BuildStyles();
        OdfStyleReference[] cascade =
        [
            new("Heading_20_1", OdfStyleFamily.Paragraph),
            new("Emphasis", OdfStyleFamily.Text),
        ];

        // Emphasis sets italic, so it wins outright.
        styles.ResolveProperty(cascade, OdfPropertyKind.Text, Fo, "font-style")
              .Value.ShouldBe("italic");

        // Emphasis sets no size. Its *defaulted* 12pt must not beat the paragraph style's
        // explicit 20pt — the two-pass precedence this asserts is easy to get wrong.
        OdfProperty size = styles.ResolveProperty(cascade, OdfPropertyKind.Text, Fo, "font-size");
        size.Value.ShouldBe("20pt");
        size.SourceStyleName.ShouldBe("Heading_20_1");
    }

    [Fact]
    public void ResolvedTextFormattingCombinesTheWholeCascade()
    {
        OdfTextFormat format = OdfTextFormat.Resolve(BuildStyles(),
        [
            new OdfStyleReference("Heading_20_1", OdfStyleFamily.Paragraph),
            new OdfStyleReference("T1", OdfStyleFamily.Text),
        ]);

        format.IsBold.ShouldBeTrue();
        format.IsItalic.ShouldBeTrue();
        format.FontSize!.Value.Points.ShouldBe(20);
        format.Language.ShouldBe("en-GB");
        format.Emphasis.ShouldBe(Core.Extraction.RunEmphasis.Bold | Core.Extraction.RunEmphasis.Italic);
    }

    [Fact]
    public void MasterPagesAndPageLayoutsAreKeptSeparatelyFromStyles()
    {
        OdfStyles styles = BuildStyles();

        OdfMasterPage? master = styles.FindMasterPage("Standard");
        master.ShouldNotBeNull();
        master.PageLayoutName.ShouldBe("pm1");
        master.Header.ShouldNotBeNull();
        master.Footer.ShouldBeNull();

        // A master page and a paragraph style may share a name; the families keep them apart.
        styles.Find("Standard", OdfStyleFamily.Paragraph).ShouldNotBeNull();
        styles.ResolveProperty("pm1", OdfStyleFamily.PageLayout, OdfPropertyKind.PageLayout,
                               Fo, "page-width")
              .AsLength()!.Value.Mm100.ShouldBe(21001);
    }

    [Fact]
    public void ADuplicateStyleDeclarationIsReportedButNotFatal()
    {
        List<Diagnostic> diagnostics = [];
        OdfStyles styles = BuildStyles(diagnostics);

        XElement second = XElement.Parse($$"""
            <office:styles xmlns:office="{{OdfNamespaces.Office}}" xmlns:style="{{St}}"
                           xmlns:fo="{{Fo}}">
              <style:style style:name="Standard" style:family="paragraph">
                <style:text-properties fo:font-size="9pt"/>
              </style:style>
            </office:styles>
            """);
        styles.AddContainer(second, diagnostics);

        diagnostics.ShouldContain(d => d.Code == "PL2002");
        styles.ResolveProperty("Standard", OdfStyleFamily.Paragraph,
                               OdfPropertyKind.Text, Fo, "font-size")
              .Value.ShouldBe("9pt");
    }

    [Fact]
    public void ListLabelsAreGeneratedFromTheCountersAndTheLevelFormat()
    {
        OdfListStyle numbers = BuildStyles().FindListStyle("Numbers")!;

        numbers.FormatLabel(1, [3]).ShouldBe("3.");
        numbers.FormatLabel(2, [3, 1]).ShouldBe("a)");
        numbers.FormatLabel(2, [3, 4]).ShouldBe("d)");
        // display-levels="2" shows the parent level too, each in its own format.
        numbers.FormatLabel(3, [3, 2, 4]).ShouldBe("b.iv");
        // A level the style does not define falls back to the deepest one above it — level 3
        // here, so level 5 inherits its roman format and its display-levels of 2.
        numbers.FormatLabel(5, [1, 1, 1, 2, 7]).ShouldBe("ii.vii");
    }

    [Fact]
    public void APrivateUseAreaBulletBecomesARealBullet()
    {
        // Impress writes its default bullet as a symbol-font code point, which means nothing
        // outside that font. LibreOffice's own HTML export substitutes a bullet here too.
        BuildStyles().FindListStyle("Bullets")!.FormatLabel(1, [1]).ShouldBe("•");
    }

    [Theory]
    [InlineData(1, "1", "1")]
    [InlineData(26, "a", "z")]
    [InlineData(27, "a", "aa")]
    [InlineData(28, "a", "ab")]
    [InlineData(4, "A", "D")]
    [InlineData(4, "i", "iv")]
    [InlineData(1990, "I", "MCMXC")]
    [InlineData(0, "i", "0")]
    public void NumberFormatsMatchOdfsFiveFormats(int value, string format, string expected)
        => OdfListStyle.FormatNumber(value, format).ShouldBe(expected);

    [Fact]
    public void SynchronisedLetteringRepeatsTheLetterRatherThanCounting()
    {
        OdfListStyle.FormatNumber(27, "a", letterSynchronised: true).ShouldBe("aa");
        OdfListStyle.FormatNumber(28, "a", letterSynchronised: true).ShouldBe("bb");
    }
}
