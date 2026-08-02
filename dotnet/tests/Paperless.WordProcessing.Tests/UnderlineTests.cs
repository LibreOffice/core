using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The rules an underlined or struck-through run draws under and through itself.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:u</c> and <c>w:strike</c> in a DOCX, <c>sprmCKul</c> and <c>sprmCFStrike</c> in a DOC,
/// <c>\ul</c> and <c>\strike</c> in an RTF, <c>style:text-underline-style</c> in an ODT. Every reader
/// read past them into extraction — which emits <c>&lt;u&gt;</c> correctly — and
/// <see cref="PageRun"/> had no field to put them in, so the word-processing rendering model could not
/// express an underline at all and no document in the corpus had one drawn.
/// </para>
/// <para>
/// Checked at the drawing pass, like the highlight band beside it, because that is where the two halves
/// meet: the flag comes from the run and the rectangle from the face's own metrics and the pen position
/// the tab stops left behind.
/// </para>
/// </remarks>
public sealed class UnderlineTests
{
    private static readonly Length Size = Length.FromPoints(12);

    [Fact]
    public void AnUnderlinedRunDrawsARuleUnderItsOwnGlyphs()
    {
        (List<(GlyphRun Run, Colour Colour)> runs, List<(DocRect Area, Colour Colour)> rules) =
            Draw(Decorated("plain and lined", from: 10, underline: true, strike: false));

        rules.Count.ShouldBe(1);

        // Under the decorated run and only it: the rule starts where that run's glyphs start, and it sits
        // below the baseline rather than on it.
        GlyphRun lined = runs[^1].Run;
        rules[0].Area.X.ShouldBe(lined.Origin.X);
        rules[0].Area.Width.ShouldBeGreaterThan(Length.Zero);
        rules[0].Area.Y.ShouldBeGreaterThan(lined.Origin.Y);
        rules[0].Area.Height.ShouldBeGreaterThan(Length.Zero);
    }

    [Fact]
    public void AStruckRunDrawsARuleThroughItRatherThanUnderIt()
    {
        (List<(GlyphRun Run, Colour Colour)> runs, List<(DocRect Area, Colour Colour)> rules) =
            Draw(Decorated("plain and struck", from: 10, underline: false, strike: true));

        rules.Count.ShouldBe(1);

        // A strikethrough sits *above* the baseline, which is the sign that distinguishes it from an
        // underline and the one an OS/2 table records positively.
        rules[0].Area.Y.ShouldBeLessThan(runs[^1].Run.Origin.Y);
    }

    [Fact]
    public void ARunCarryingBothDrawsBoth()
    {
        (_, List<(DocRect Area, Colour Colour)> rules) =
            Draw(Decorated("plain and both", from: 10, underline: true, strike: true));

        rules.Count.ShouldBe(2);
    }

    [Fact]
    public void APlainParagraphDrawsNoRule()
    {
        (_, List<(DocRect Area, Colour Colour)> rules) =
            Draw(Decorated("plain and plain", from: 10, underline: false, strike: false));

        rules.ShouldBeEmpty();
    }

    [Fact]
    public void ARuleTakesTheColourOfTheTextItDecorates()
    {
        PageParagraph paragraph = new()
        {
            Text = "red and underlined",
            Face = Face,
            EmSize = Size,
            Runs =
            [
                new PageRun(
                    0, 18, Face, Size, Colour: Colour.FromRgb(0xFF0000), IsUnderlined: true),
            ],
        };

        Draw(paragraph).Rules[0].Colour.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    /// <summary>
    /// The rule is placed from the face's descent, not from its <c>post</c> table.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Liberation Serif is on LibreOffice's shipped <c>FontsDontUseUnderlineMetrics</c> list because its
    /// <c>post</c> table is not to be believed, so the rule has to come from the descent instead. The two
    /// answers are close, which is exactly why this asserts narrowly: at 12 pt this face's
    /// <c>post</c> gives 0.721 pt below the baseline and 0.586 pt thick, and the descent gives 0.973 pt
    /// and 0.651 pt. A band wide enough to hold both would pass either way and prove nothing — a first
    /// draft of this test did precisely that.
    /// </para>
    /// <para>
    /// Design units against LibreOffice's device-rounded arithmetic differ by about 0.03 pt, which is
    /// what the tolerance here allows and is a tenth of the gap between the two candidate answers.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheRuleIgnoresAPostTableLibreOfficeRefusesToBelieve()
    {
        (List<(GlyphRun Run, Colour Colour)> runs, List<(DocRect Area, Colour Colour)> rules) =
            Draw(Decorated("underlined", from: 0, underline: true, strike: false));

        Length below = rules[0].Area.Y - runs[^1].Run.Origin.Y;

        below.ShouldBeGreaterThan(Length.FromPoints(0.94));
        below.ShouldBeLessThan(Length.FromPoints(1.01));

        rules[0].Area.Height.ShouldBeGreaterThan(Length.FromPoints(0.62));
        rules[0].Area.Height.ShouldBeLessThan(Length.FromPoints(0.68));
    }

    /// <summary>
    /// A decoration must not move a line break.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The invariant the whole change rests on. A reader has to keep a paragraph's runs when any of them
    /// carries a decoration — a property dropped by the uniform-paragraph shortcut is a property never
    /// drawn — and keeping them puts the paragraph on the multi-run measuring path, where a shaper called
    /// once per run loses the kern pair straddling each boundary. So underlining a sentence would make it
    /// fractionally wider, and a paragraph near a line's end would break differently.
    /// </para>
    /// <para>
    /// <see cref="PageParagraph.Measure"/> joins adjacent runs whose measurement halves are equal, which
    /// is what makes this hold. Reintroducing the bug — dropping that join — makes this test fail on a
    /// face with kerning, which Liberation Serif is.
    /// </para>
    /// </remarks>
    [Fact]
    public void UnderliningAParagraphChangesNoWidth()
    {
        // "AV" and "To" are kern pairs in this face, and they straddle the run boundary below.
        const string Text = "AVAVAVA TToToTo AWAY";

        Length plain = Width(Split(Text, at: 7, underline: false));
        Length lined = Width(Split(Text, at: 7, underline: true));

        lined.ShouldBe(plain);

        // And the split itself must be what a single run measures, or the invariant is vacuous.
        Length whole = Width(new PageParagraph
        {
            Text = Text,
            Face = Face,
            EmSize = Size,
            Runs = [new PageRun(0, Text.Length, Face, Size, Colour: Colour.Black)],
        });

        lined.ShouldBe(whole);
    }

    private static Length Width(PageParagraph paragraph)
        => paragraph.Measure().WidthBetween(0, paragraph.Text.Length);

    /// <summary>The same text as two runs, differing only in a decoration.</summary>
    private static PageParagraph Split(string text, int at, bool underline)
        => new()
        {
            Text = text,
            Face = Face,
            EmSize = Size,
            Runs =
            [
                new PageRun(0, at, Face, Size, Colour: Colour.Black, IsUnderlined: underline),
                new PageRun(
                    at, text.Length - at, Face, Size,
                    Colour: Colour.Black, IsUnderlined: underline),
            ],
        };

    /// <summary>
    /// <c>w:u</c> carries a line style rather than a switch, so <c>none</c> is off and the rest are on.
    /// </summary>
    /// <remarks>
    /// The trap is that <c>w:u w:val="none"</c> is how a run turns off an underline its style set, and
    /// reading the element as an ordinary on/off toggle — which is what every other decoration in
    /// <c>w:rPr</c> is — underlines it instead.
    /// </remarks>
    [Theory]
    [InlineData("single", true)]
    [InlineData("double", true)]
    [InlineData("wavyHeavy", true)]
    [InlineData("none", false)]
    public void AWordUnderlineIsReadAsAStyleAndNotAsASwitch(string value, bool expected)
        => Resolved("u", value).IsUnderlined.ShouldBe(expected);

    [Theory]
    [InlineData("strike", null, true)]
    [InlineData("dstrike", null, true)]
    [InlineData("strike", "false", false)]
    public void BothOfWordsStrikeElementsFoldOntoOneRule(string name, string? value, bool expected)
        => Resolved(name, value).IsStruckThrough.ShouldBe(expected);

    /// <summary>
    /// <c>sprmCKul</c> is a <c>kul</c> naming a line style, and three of its values name no line.
    /// </summary>
    /// <remarks>
    /// The WW8 spelling of the same trap: nought is "none", 255 cancels what the style set, and 5
    /// ("hidden") and 8 (a dot style Word never writes) have no case in
    /// <c>SwWW8ImplReader::Read_Underline</c>'s switch and so fall through to
    /// <c>LINESTYLE_NONE</c>. Reading the byte as non-zero underlines all three.
    /// </remarks>
    [Theory]
    [InlineData(0, false)]
    [InlineData(1, true)]
    [InlineData(2, true)]
    [InlineData(3, true)]
    [InlineData(5, false)]
    [InlineData(8, false)]
    [InlineData(11, true)]
    [InlineData(255, false)]
    public void AWordBinaryUnderlineIsAStyleAndThreeStylesAreNoLine(int kul, bool expected)
        => Ww8.Ww8DocumentReader.IsUnderlineStyle(kul).ShouldBe(expected);

    /// <summary>The character formatting of a run whose <c>w:rPr</c> names one element.</summary>
    private static Ooxml.WordTextStyle Resolved(string name, string? value)
    {
        XNamespace w = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

        XElement element = new(w + name);
        if (value is not null) element.Add(new XAttribute(w + "val", value));

        return Ooxml.WordParagraphFormats.ResolveRun(
            new Ooxml.WordStyles(), null, new XElement(w + "rPr", element));
    }

    private static (List<(GlyphRun Run, Colour Colour)> Runs, List<(DocRect Area, Colour Colour)> Rules)
        Draw(PageParagraph paragraph)
    {
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400));
        List<(DocRect Area, Colour Colour)> rules = [];

        List<(GlyphRun Run, Colour Colour)> runs =
            PageDrawing.RunsIn(area, Line(paragraph), paragraph, highlights: null, rules: rules);

        return (runs, rules);
    }

    /// <summary>A paragraph whose text from <paramref name="from"/> onwards carries a decoration.</summary>
    private static PageParagraph Decorated(string text, int from, bool underline, bool strike)
        => new()
        {
            Text = text,
            Face = Face,
            EmSize = Size,
            Runs = from > 0
                ?
                [
                    new PageRun(0, from, Face, Size, Colour: Colour.Black),
                    new PageRun(
                        from, text.Length - from, Face, Size, Colour: Colour.Black,
                        IsUnderlined: underline, IsStruckThrough: strike),
                ]
                :
                [
                    new PageRun(
                        0, text.Length, Face, Size, Colour: Colour.Black,
                        IsUnderlined: underline, IsStruckThrough: strike),
                ],
        };

    private static PlacedLine Line(PageParagraph paragraph)
        => new(
            ParagraphIndex: 0,
            LineIndex: 0,
            Box: new LineBox(
                new TextLine(
                    0, paragraph.Text.Length, paragraph.Text.Length, Length.Zero, EndsParagraph: true),
                Length.Zero,
                Length.Zero,
                Length.FromPoints(14),
                Length.FromPoints(11),
                Length.Zero),
            Top: Length.Zero);

    /// <summary>A real face, since a rule's offset and thickness are measurements rather than constants.</summary>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
