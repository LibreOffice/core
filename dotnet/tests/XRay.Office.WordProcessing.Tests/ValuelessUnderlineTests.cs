using System.Xml.Linq;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>w:u</c> stating no <c>w:val</c> is not an underline, and does not turn one off either.
/// </summary>
/// <remarks>
/// <para>
/// <c>CT_Underline</c>'s <c>val</c> is optional with no default
/// (<c>sw/source/writerfilter/ooxml/model.xml</c>:17021-17027), so an element carrying only a
/// <c>w:color</c> emits no <c>LN_CT_Underline_val</c> sprm at all: <c>DomainMapper::lcl_sprm</c>'s
/// case for it (<c>dmapper/DomainMapper.cxx</c>:364) never fires and the sibling at <c>:368</c>
/// takes the colour and nothing else. The run therefore keeps whatever an outer layer stated — the
/// element is <em>transparent</em>, which is a third answer beside "underline" and "no underline"
/// and is why the resolution has to see every layer rather than only the innermost.
/// </para>
/// <para>
/// <strong>Reading the absent <c>val</c> as <c>single</c> cost a whole document's ink.</strong>
/// <c>f445896eb008d14c1746fc37d412dc22.docx</c> states <c>&lt;w:u w:color="000000"/&gt;</c> on four of
/// its styles and <c>&lt;w:u w:color="666666"/&gt;</c> on a run, with no <c>val</c> anywhere, and this
/// tree underlined every word: 108 filled rules per page against 26.2.4.2's none, on fourteen of its
/// fifteen pages, at identical text, identical faces, identical sizes and identical line breaks. Its
/// summed unsigned ink goes <strong>29.26 → 0.24</strong>, and a blind reading of one page supplied
/// the control that makes it unambiguous: the table's <em>left</em> column, whose style states
/// <c>w:val="single"</c>, is underlined on both sides.
/// </para>
/// <para>
/// <strong>Reach: 8 of the 337 words-track renderings move</strong>, six better and two worse by
/// 0.02, summed ink 113.21 → 75.52, 329 byte-identical, and no gate verdict either way — an
/// underline adds no character and no page. <c>probes/underline-r176</c>.
/// </para>
/// </remarks>
public sealed class ValuelessUnderlineTests
{
    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static WordStyles Styles(params XElement[] styles)
    {
        WordStyles read = new();
        read.Add(new XElement(W + "styles", styles));
        return read;
    }

    private static XElement Style(string id, params XElement[] runProperties)
        => new(
            W + "style",
            new XAttribute(W + "type", "paragraph"),
            new XAttribute(W + "styleId", id),
            new XElement(W + "name", new XAttribute(W + "val", id)),
            new XElement(W + "rPr", runProperties));

    private static XElement Underline(params XAttribute[] attributes)
        => new(W + "u", attributes);

    private static XAttribute Value(string value) => new(W + "val", value);

    private static XAttribute Colour(string value) => new(W + "color", value);

    private static TextUnderline Resolved(XElement? direct, params XElement[] styles)
        => WordCharacterFormat.Resolve(Styles(styles), direct, styles.Length > 0 ? "S" : null)
            .Underline;

    /// <summary>A <c>w:u</c> with a colour and no value underlines nothing.</summary>
    [Fact]
    public void AColourWithoutAValueIsNotAnUnderline()
        => Resolved(new XElement(W + "rPr", Underline(Colour("666666"))))
            .ShouldBe(TextUnderline.None);

    /// <summary>Nor does a bare <c>&lt;w:u/&gt;</c>, which states no value either.</summary>
    [Fact]
    public void ABareElementIsNotAnUnderlineEither()
        => Resolved(new XElement(W + "rPr", Underline())).ShouldBe(TextUnderline.None);

    /// <summary>The control: the same element with a value underlines.</summary>
    [Fact]
    public void AStatedValueUnderlines()
        => Resolved(new XElement(W + "rPr", Underline(Value("single"), Colour("666666"))))
            .ShouldBe(TextUnderline.SingleLine);

    /// <summary>
    /// A valueless element over a style that states one keeps the style's, rather than voiding it.
    /// </summary>
    /// <remarks>
    /// This is the half that separates "transparent" from "off", and no stated value can express it:
    /// <c>w:val="none"</c> would turn the style's underline off and the absent attribute does not.
    /// </remarks>
    [Fact]
    public void AValuelessElementDoesNotVoidAnInheritedUnderline()
        => Resolved(
                new XElement(W + "rPr", new XElement(W + "pStyle", Value("S")),
                    Underline(Colour("000000"))),
                Style("S", Underline(Value("double"))))
            .ShouldBe(TextUnderline.DoubleLine);

    /// <summary>And a stated <c>none</c> over the same style does void it.</summary>
    [Fact]
    public void AStatedNoneStillVoidsIt()
        => Resolved(
                new XElement(W + "rPr", new XElement(W + "pStyle", Value("S")),
                    Underline(Value("none"))),
                Style("S", Underline(Value("double"))))
            .ShouldBe(TextUnderline.None);

    /// <summary>A style whose only <c>w:u</c> is valueless underlines nothing.</summary>
    /// <remarks>
    /// The corpus shape: four of <c>f445896e</c>'s styles carry exactly this and no layer under them
    /// states a value, so the answer is none rather than the <c>single</c> an absent attribute used
    /// to be read as.
    /// </remarks>
    [Fact]
    public void AStylesValuelessUnderlineIsNoUnderline()
        => Resolved(
                new XElement(W + "rPr", new XElement(W + "pStyle", Value("S"))),
                Style("S", Underline(Colour("000000"))))
            .ShouldBe(TextUnderline.None);

    /// <summary>Every heavy form still comes back as one thick line.</summary>
    [Theory]
    [InlineData("double", TextUnderline.DoubleLine)]
    [InlineData("wavyDouble", TextUnderline.DoubleLine)]
    [InlineData("thick", TextUnderline.BoldLine)]
    [InlineData("dashDotDotHeavy", TextUnderline.BoldLine)]
    [InlineData("dotted", TextUnderline.SingleLine)]
    [InlineData("words", TextUnderline.SingleLine)]
    public void TheStatedFormsAreUnchanged(string stated, TextUnderline expected)
        => Resolved(new XElement(W + "rPr", Underline(Value(stated)))).ShouldBe(expected);
}
