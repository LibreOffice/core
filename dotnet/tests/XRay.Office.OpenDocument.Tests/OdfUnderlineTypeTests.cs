using System.Xml.Linq;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// ODF states an underline as two attributes of one item, and the level decides before the
/// attribute does.
/// </summary>
/// <remarks>
/// <para>
/// <c>style:text-underline-style</c> says solid, dotted or none;
/// <c>style:text-underline-type</c> says single or double. Both are mapped to
/// <c>CharUnderline</c> with <c>MID_FLAG_MERGE_PROPERTY</c>
/// (<c>xmloff/source/text/txtprmap.cxx</c>:177-179) and their handlers merge into whatever the
/// <em>same element</em> has already contributed (<c>xmloff/source/style/undlihdl.cxx</c>:114-160),
/// so one style's two attributes combine and a child style's shadow the parent's outright.
/// </para>
/// <para>
/// This is <c>OdfFontNamePrecedenceTests</c>' rule from the other side — there two spellings say the
/// same thing, here two attributes each say part of one thing — and it is not hypothetical. It is
/// the shape of the corpus's only word-processing double underline: <c>RobertQ_Service.doc</c>'s
/// <c>Subtitle</c> style states both attributes, four of its automatic children restate the style
/// alone, and 26.2.4.2 draws those four with one line and the children restating neither with two.
/// </para>
/// <para>
/// Hand-built XML rather than a corpus file, for <c>OdfStyleResolutionTests</c>' reason: what is
/// being pinned is precedence, and a chain where each rule matters in isolation is not something a
/// real document conveniently offers.
/// </para>
/// </remarks>
public sealed class OdfUnderlineTypeTests
{
    [Theory]
    // The parent states both, so a child that restates nothing inherits two lines.
    [InlineData("Inheritor", TextUnderline.DoubleLine)]
    // A child restating the STYLE alone replaces the whole item and is drawn with one line. This is
    // the case a per-attribute resolution gets wrong, and it gets it wrong in the loud direction.
    [InlineData("RestatesStyle", TextUnderline.SingleLine)]
    // A child restating the TYPE alone is two lines: the type handler writes its own value where
    // nothing has been merged into the item yet.
    [InlineData("RestatesType", TextUnderline.DoubleLine)]
    // And a child turning the line off is off, whatever its parent's type says.
    [InlineData("TurnsOff", TextUnderline.None)]
    // A style stating neither attribute anywhere in its chain has no underline at all.
    [InlineData("Plain", TextUnderline.None)]
    // The width is a third attribute of the same item, and a double line beats a bold one:
    // "A double line style has priority over a bold line style" (undlihdl.cxx:135-136).
    [InlineData("Bold", TextUnderline.BoldLine)]
    [InlineData("BoldAndDouble", TextUnderline.DoubleLine)]
    public void TheLevelDecidesBeforeTheAttribute(string style, TextUnderline expected)
        => OdfTextFormat.UnderlineIn(BuildStyles(), [Reference(style)]).ShouldBe(expected);

    /// <summary>
    /// A span's own style beats the paragraph style it sits in, by the same rule one level out.
    /// </summary>
    [Fact]
    public void ASpanRestatingTheStyleAloneBeatsTheParagraphsType()
        => OdfTextFormat
           .UnderlineIn(BuildStyles(), [Reference("Doubled"), Span("SolidSpan")])
           .ShouldBe(TextUnderline.SingleLine);

    /// <summary>And a span stating nothing leaves the paragraph's two lines alone.</summary>
    [Fact]
    public void ASpanStatingNothingKeepsTheParagraphsTwoLines()
        => OdfTextFormat
           .UnderlineIn(BuildStyles(), [Reference("Doubled"), Span("BoldSpan")])
           .ShouldBe(TextUnderline.DoubleLine);

    private static OdfStyleReference Reference(string name)
        => new(name, OdfStyleFamily.Paragraph);

    private static OdfStyleReference Span(string name)
        => new(name, OdfStyleFamily.Text);

    private static OdfStyles BuildStyles()
    {
        XElement root = XElement.Parse($$"""
            <office:document-styles
                xmlns:office="{{OdfNamespaces.Office}}"
                xmlns:style="{{OdfNamespaces.Style}}"
                xmlns:fo="{{OdfNamespaces.FoCompatible}}">
              <office:styles>
                <style:style style:name="Doubled" style:family="paragraph">
                  <style:text-properties style:text-underline-style="solid"
                                         style:text-underline-type="double"
                                         style:text-underline-width="auto"/>
                </style:style>
                <style:style style:name="Plain" style:family="paragraph"/>
                <style:style style:name="Inheritor" style:family="paragraph"
                             style:parent-style-name="Doubled"/>
                <style:style style:name="RestatesStyle" style:family="paragraph"
                             style:parent-style-name="Doubled">
                  <style:text-properties style:text-underline-style="solid"
                                         style:text-underline-width="auto"/>
                </style:style>
                <style:style style:name="RestatesType" style:family="paragraph"
                             style:parent-style-name="Plain">
                  <style:text-properties style:text-underline-type="double"/>
                </style:style>
                <style:style style:name="TurnsOff" style:family="paragraph"
                             style:parent-style-name="Doubled">
                  <style:text-properties style:text-underline-style="none"/>
                </style:style>
                <style:style style:name="Bold" style:family="paragraph">
                  <style:text-properties style:text-underline-style="solid"
                                         style:text-underline-width="bold"/>
                </style:style>
                <style:style style:name="BoldAndDouble" style:family="paragraph">
                  <style:text-properties style:text-underline-style="solid"
                                         style:text-underline-type="double"
                                         style:text-underline-width="bold"/>
                </style:style>
                <style:style style:name="SolidSpan" style:family="text">
                  <style:text-properties style:text-underline-style="solid"/>
                </style:style>
                <style:style style:name="BoldSpan" style:family="text">
                  <style:text-properties fo:font-weight="bold"/>
                </style:style>
              </office:styles>
            </office:document-styles>
            """);

        OdfStyles styles = new();
        styles.AddDocument(root);
        return styles;
    }
}
