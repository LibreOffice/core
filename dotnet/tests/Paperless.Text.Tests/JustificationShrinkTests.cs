using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Whether a justified line may squeeze its blanks below their natural width to hold another word.
/// </summary>
/// <remarks>
/// <para>
/// Word 2013's justification, which LibreOffice turns on for every file declaring
/// <c>compatibilityMode</c> 15 or more and warns "typesets the same text with less lines and pages"
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10172</c>). Without it a line that overflows
/// by a hair sends its last word down, and the whole document paginates longer.
/// </para>
/// <para>
/// The column widths here are derived from the face rather than stated, so each case says what it means:
/// a line overrunning by a tenth of what its blanks are worth is inside the 75% floor and must fit, and
/// one overrunning by two fifths is outside it and must not.
/// </para>
/// </remarks>
public sealed class JustificationShrinkTests
{
    /// <summary>Eight short words, so a break lands between two of them rather than inside one.</summary>
    private const string Text = "aa bb cc dd ee ff gg hh";

    private static readonly Length Size = Length.FromPoints(11);

    [Fact]
    public void ALineOverrunningWithinTheFloorFitsWhenShrinkingIsOn()
    {
        Fixture fixture = new();

        // A tenth of what the blanks are worth: well inside the quarter squeezing them recovers.
        List<LineBox> lines = fixture.LayOut(fixture.Column(0.10), shrinks: true);

        lines.Count.ShouldBe(1);
    }

    [Fact]
    public void TheSameLineBreaksInTwoWhenShrinkingIsOff()
    {
        Fixture fixture = new();

        List<LineBox> lines = fixture.LayOut(fixture.Column(0.10), shrinks: false);

        lines.Count.ShouldBe(2);
    }

    [Fact]
    public void AnOverrunBeyondTheFloorStillBreaks()
    {
        Fixture fixture = new();

        // Two fifths of the blanks' width, which squeezing every one of them to 75% cannot recover.
        List<LineBox> lines = fixture.LayOut(fixture.Column(0.40), shrinks: true);

        lines.Count.ShouldBe(2);
    }

    /// <summary>
    /// The squeezed line is brought back inside its column rather than merely admitted to it.
    /// </summary>
    /// <remarks>
    /// The two halves of the feature are separable and only one of them is visible in a line count: a
    /// filler that admits the line and a justifier that leaves its blanks alone produces the right
    /// pagination and draws the last word past the margin.
    /// </remarks>
    [Fact]
    public void TheSqueezedLineIsPulledBackInsideItsColumn()
    {
        Fixture fixture = new();
        Length column = fixture.Column(0.10);

        LineBox line = fixture.LayOut(column, shrinks: true).ShouldHaveSingleItem();

        line.SpaceAdd.ShouldBeLessThan(Length.Zero);

        // Seven blanks in eight words, each narrowed by the space-add.
        Length drawn = line.Width + (line.SpaceAdd * 7);
        drawn.ShouldBeLessThanOrEqualTo(column);

        // And not by so much that the line falls short of the margin it was stretched to reach: within a
        // twip per blank, which is the truncation the space-add is computed with.
        drawn.ShouldBeGreaterThan(column - Length.FromTwips(7));
    }

    /// <summary>A blank is never squeezed below the floor, whatever the line asks for.</summary>
    [Fact]
    public void ABlankIsNeverSqueezedBelowThreeQuarters()
    {
        Fixture fixture = new();

        // The widest overrun the floor still admits: a hair under a quarter of the blanks' width.
        LineBox line = fixture.LayOut(fixture.Column(0.24), shrinks: true).ShouldHaveSingleItem();

        Length blank = fixture.BlankWidth;
        (blank + line.SpaceAdd).ShouldBeGreaterThanOrEqualTo(
            Length.FromEmu((long)(blank.Emu * JustificationShrink.MinimumBlankProportion)));
    }

    /// <summary>An unjustified paragraph is untouched by the flag.</summary>
    /// <remarks>
    /// The flag is a document-wide setting carried on every paragraph, so a left-aligned one has to be
    /// proved unaffected — otherwise it would silently gain an overflowing line wherever a break was tight.
    /// </remarks>
    [Fact]
    public void ALeftAlignedParagraphIsNotShrunk()
    {
        Fixture fixture = new();

        List<LineBox> lines =
            fixture.LayOut(fixture.Column(0.10), shrinks: true, TextAlignment.Start);

        lines.Count.ShouldBe(2);
    }

    /// <summary>
    /// The paragraph, its measurement and the column widths derived from it.
    /// </summary>
    private sealed class Fixture
    {
        private readonly MeasuredParagraph _measured;
        private readonly OpenTypeFace _face;

        public Fixture()
        {
            _face = Carlito();
            _measured = MeasuredParagraph.Measure(
                Text, [new FormattedRun(0, Text.Length, _face, Size)]);

            Natural = _measured.WidthBetween(0, Text.Length);

            long blanks = 0;
            for (int at = 0; at < Text.Length; at++)
            {
                if (Text[at] == ' ') blanks += _measured.WidthBetween(at, at + 1).Emu;
            }

            Blanks = Length.FromEmu(blanks);
            BlankWidth = Length.FromEmu(blanks / 7);
        }

        /// <summary>The whole paragraph's width, set on one line at natural widths.</summary>
        public Length Natural { get; }

        /// <summary>What all seven of its blanks are worth together.</summary>
        public Length Blanks { get; }

        /// <summary>What one of them is worth.</summary>
        public Length BlankWidth { get; }

        /// <summary>A column narrower than the paragraph by a share of what its blanks are worth.</summary>
        public Length Column(double share)
            => Natural - Length.FromEmu((long)(Blanks.Emu * share));

        public List<LineBox> LayOut(
            Length column, bool shrinks, TextAlignment alignment = TextAlignment.Justify)
        {
            ParagraphLayouter layouter = new(_face);

            return [.. layouter.Layout(
                _measured,
                new ParagraphFormat
                {
                    Alignment = alignment,
                    ShrinksJustifiedBlanks = shrinks,
                },
                column).Lines];
        }

        private static OpenTypeFace Carlito()
        {
            foreach (string directory in new[]
                     {
                         "/usr/share/fonts/truetype/crosextra",
                         "/usr/share/fonts/truetype/dejavu",
                         "/usr/share/fonts",
                     })
            {
                string path = Path.Combine(directory, "Carlito-Regular.ttf");
                if (File.Exists(path)) return OpenTypeFace.ReadFile(path).ShouldNotBeNull();
            }

            string? found = Directory.Exists("/usr/share/fonts")
                ? Directory.EnumerateFiles("/usr/share/fonts", "Carlito-Regular.ttf",
                    SearchOption.AllDirectories).FirstOrDefault()
                : null;

            Assert.SkipWhen(found is null, "Carlito is not installed; see check-env.sh");
            return OpenTypeFace.ReadFile(found!).ShouldNotBeNull();
        }
    }
}
