using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Core.Extraction;
using Paperless.Presentations.MsBinary;
using Paperless.TestKit;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The rules a slide's underlined and struck-through runs draw.
/// </summary>
/// <remarks>
/// <para>
/// Both formats read the flag — DrawingML's <c>a:rPr/@u</c> and bit 2 of a PPT character-property
/// mask, the latter already resolved into <c>RunEmphasis.Underline</c> — and nothing carried it
/// past the reader, so no deck in any format has ever drawn one. It is the "read but never used"
/// shape: invisible to every text comparison, and to every geometric one that does not look at
/// what is filled.
/// </para>
/// <para>
/// Measured on <c>wells08_basic.ppt</c>, whose column headings are underlined on eight of its
/// twenty-seven pages.
/// </para>
/// </remarks>
public class SlideUnderlineTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static XElement Body(string runProperties) => XElement.Parse(
        $"""
         <a:txBody xmlns:a="{A}">
           <a:bodyPr/>
           <a:p><a:r><a:rPr lang="en-US" sz="2400" {runProperties}/><a:t>Advantages</a:t></a:r></a:p>
         </a:txBody>
         """);

    [Theory]
    [InlineData("", false, false)]
    [InlineData("u=\"sng\"", true, false)]
    [InlineData("u=\"dbl\"", true, false)]
    [InlineData("u=\"none\"", false, false)]
    [InlineData("strike=\"sngStrike\"", false, true)]
    [InlineData("strike=\"noStrike\"", false, false)]
    [InlineData("u=\"sng\" strike=\"dblStrike\"", true, true)]
    public void ARunCarriesTheDecorationsItsPropertiesState(
        string properties, bool underlined, bool struckThrough)
    {
        SlideTextRun run = PptxTextBody.Read(Body(properties)).Paragraphs[0].Runs[0];

        run.IsUnderlined.ShouldBe(underlined);
        run.IsStruckThrough.ShouldBe(struckThrough);
    }

    /// <summary>
    /// Every value but <c>none</c> underlines, because the nineteen of them differ in how the
    /// rule is drawn rather than in whether there is one.
    /// </summary>
    [Fact]
    public void AnUnrecognisedUnderlineStyleStillUnderlines()
        => PptxTextBody.Read(Body("u=\"wavyHeavy\"")).Paragraphs[0].Runs[0]
            .IsUnderlined.ShouldBeTrue();

    [Fact]
    public void AnUnderlinedRunPlacesARuleUnderItsOwnGlyphs()
    {
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(100));

        List<PlacedGlyphRun> plain = SlideTextLayout.Place(
            PptxTextBody.Read(Body(string.Empty)), area, new SlideFonts());
        List<PlacedGlyphRun> underlined = SlideTextLayout.Place(
            PptxTextBody.Read(Body("u=\"sng\"")), area, new SlideFonts());

        // The plain run draws none at all, which is the assertion that matters most: a layout that
        // ruled every run would pass everything else here.
        plain.ShouldAllBe(run => run.Rules == null || run.Rules.Count == 0);

        underlined.Count.ShouldBe(1);
        IReadOnlyList<DocRect> rules = underlined[0].Rules.ShouldNotBeNull();
        rules.Count.ShouldBe(1);

        GlyphRun glyphs = underlined[0].Run;
        Length width = Length.Zero;
        foreach (PositionedGlyph glyph in glyphs.Glyphs) width += glyph.Advance;

        rules[0].X.ShouldBe(glyphs.Origin.X);
        rules[0].Width.Points.ShouldBe(width.Points, 0.01);

        // Below the baseline, and by less than a fifth of the em — a rule further down than that
        // has taken the descent for the offset rather than half of it.
        rules[0].Y.ShouldBeGreaterThan(glyphs.Origin.Y);
        (rules[0].Y - glyphs.Origin.Y).Points.ShouldBeLessThan(24.0 / 5);
        rules[0].Height.Points.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void AStruckThroughRunPlacesItsRuleAboveTheBaseline()
    {
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(100));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            PptxTextBody.Read(Body("strike=\"sngStrike\"")), area, new SlideFonts());

        IReadOnlyList<DocRect> rules = placed[0].Rules.ShouldNotBeNull();
        rules.Count.ShouldBe(1);
        rules[0].Y.ShouldBeLessThan(placed[0].Run.Origin.Y);
    }

    [Fact]
    public void TheRulesReachThePageAsFilledPaths()
    {
        // Asserted through the drawing commands because the run carrying the rectangle and the
        // painter ignoring it is exactly the state this replaced.
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(100));

        PlacedShape shape = new()
        {
            Outline = new GraphicsPath(),
            Text = new PlacedText(
                SlideTextLayout.Place(
                    PptxTextBody.Read(Body("u=\"sng\"")), area, new SlideFonts()),
                AffineTransform.Identity),
        };

        RecordingDrawingSink sink = new();
        SlideDrawing.Draw(
            new LaidOutSlide
            {
                Index = 0,
                Size = new DocSize(Length.FromPoints(720), Length.FromPoints(540)),
                Background = null,
                Shapes = [shape],
            },
            sink);

        sink.Pages[0].FilledPaths.Count.ShouldBe(1);
        sink.Pages[0].Runs.Count.ShouldBe(1);
    }

    /// <summary>
    /// The PPT reader's emphasis bits reach the run, which is the half of this that had a reader
    /// already and no consumer.
    /// </summary>
    [Fact]
    public void ThePptEmphasisBitsAreTheOnesTheRunNowCarries()
    {
        PptCharacterStyle.ToEmphasis(0x0004, 0).HasFlag(RunEmphasis.Underline).ShouldBeTrue();
        PptCharacterStyle.ToEmphasis(0x0100, 0).HasFlag(RunEmphasis.Strikethrough).ShouldBeTrue();
        PptCharacterStyle.ToEmphasis(0x0001, 0).HasFlag(RunEmphasis.Underline).ShouldBeFalse();
    }
}
