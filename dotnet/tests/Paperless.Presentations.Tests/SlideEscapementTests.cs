using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// What a slide does with a run raised or lowered off its baseline.
/// </summary>
/// <remarks>
/// <para>
/// Both readers had the value and neither passed it on: DrawingML's <c>a:rPr/@baseline</c> was
/// turned into a <c>RunEmphasis</c> flag for extraction and a binary PowerPoint's
/// <c>PPT_CharAttr_Escapement</c> likewise, while <c>SlideTextRun</c> had nowhere to put either.
/// So every ordinal in every deck — <c>5<sup>th</sup> March</c> is the corpus's commonest use —
/// was drawn on the baseline at full size.
/// </para>
/// <para>
/// The size is the half that matters, because it moves line breaks: 58% of the em is 42% less
/// advance. Measured on <c>slides/batch-003/pptx/NCW-2024-Guide-.pptx</c>, whose left-hand column
/// lists eight dated events in a text box already taller than the slide — one line wrapping that
/// should not pushed the box's last paragraph off the bottom edge, and the deck extracted 216
/// words against LibreOffice's 224.
/// </para>
/// </remarks>
public class SlideEscapementTests
{
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static XElement Body(string ordinalProperties) => XElement.Parse(
        $"""
         <a:txBody xmlns:a="{A}">
           <a:bodyPr/>
           <a:p>
             <a:r><a:rPr lang="en-GB" sz="2400"/><a:t>5</a:t></a:r>
             <a:r><a:rPr lang="en-GB" sz="2400" {ordinalProperties}/><a:t>th</a:t></a:r>
           </a:p>
         </a:txBody>
         """);

    private static DocRect Area =>
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(100));

    [Theory]
    [InlineData("", 0, 0)]
    [InlineData("baseline=\"30000\"", 30, 58)]
    [InlineData("baseline=\"100000\"", 100, 58)]
    [InlineData("baseline=\"-25000\"", -25, 58)]
    [InlineData("baseline=\"0\"", 0, 0)]
    public void ABaselineAttributeBecomesARiseAndAShrink(
        string properties, int percent, int proportion)
    {
        SlideTextRun run = PptxTextBody.Read(Body(properties)).Paragraphs[0].Runs[1];

        run.Escapement.Percent.ShouldBe(percent);
        run.Escapement.Proportion.ShouldBe(proportion);
    }

    /// <summary>
    /// The arithmetic itself: the percentage is of the em size, which is EditEngine's rule and
    /// not the word processor's.
    /// </summary>
    [Fact]
    public void TheRiseAndTheSizeAreFractionsOfTheEm()
    {
        SlideEscapement superscript = new(30, SlideEscapement.AutomaticProportion);

        superscript.SizeOf(Length.FromPoints(12)).Points.ShouldBe(6.96, 0.001);
        superscript.RiseOf(Length.FromPoints(12)).Points.ShouldBe(3.6, 0.001);
        SlideEscapement.None.SizeOf(Length.FromPoints(12)).Points.ShouldBe(12.0, 0.001);
        SlideEscapement.None.RiseOf(Length.FromPoints(12)).Points.ShouldBe(0.0, 0.001);
    }

    /// <summary>
    /// A superscript is drawn above its line's baseline and at a fraction of its size — the two
    /// halves the layout was missing, asserted against the plain run beside it on the same line.
    /// </summary>
    [Fact]
    public void ASuperscriptIsDrawnRaisedAndSmaller()
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            PptxTextBody.Read(Body("baseline=\"30000\"")), Area, new SlideFonts());

        placed.Count.ShouldBe(2);
        GlyphRun digit = placed[0].Run;
        GlyphRun ordinal = placed[1].Run;

        // 30% of 24 pt above the digit's own baseline.
        (digit.Origin.Y - ordinal.Origin.Y).Points.ShouldBe(7.2, 0.05);
        ordinal.FontSize.Points.ShouldBe(24 * 0.58, 0.05);
    }

    /// <summary>A subscript is the same pair with the sign turned round.</summary>
    [Fact]
    public void ASubscriptIsDrawnLoweredAndSmaller()
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            PptxTextBody.Read(Body("baseline=\"-25000\"")), Area, new SlideFonts());

        placed.Count.ShouldBe(2);
        (placed[1].Run.Origin.Y - placed[0].Run.Origin.Y).Points.ShouldBe(6.0, 0.05);
        placed[1].Run.FontSize.Points.ShouldBe(24 * 0.58, 0.05);
    }

    /// <summary>
    /// A run left on the baseline is drawn there, which is the assertion that would still pass
    /// were the rise applied to everything.
    /// </summary>
    [Fact]
    public void APlainRunStaysOnTheBaselineAtItsOwnSize()
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            PptxTextBody.Read(Body(string.Empty)), Area, new SlideFonts());

        placed.Count.ShouldBe(2);
        placed[1].Run.Origin.Y.ShouldBe(placed[0].Run.Origin.Y);
        placed[1].Run.FontSize.Points.ShouldBe(24.0, 0.05);
    }

    /// <summary>
    /// The shrink reaches the line breaker, not only the painter.
    /// </summary>
    /// <remarks>
    /// This is the assertion the corpus failure was: an ordinal measured at full size makes its
    /// line wider, and a line wide enough to wrap does. The width is taken from the pen positions
    /// of the two runs rather than from a line count, because at 24 pt in a 400 pt box nothing
    /// wraps — what changes is where the second run starts.
    /// </remarks>
    [Fact]
    public void AShrunkRunAdvancesTheLineLessThanAFullSizedOne()
    {
        Length Reach(string properties)
        {
            List<PlacedGlyphRun> placed = SlideTextLayout.Place(
                PptxTextBody.Read(Body(properties)), Area, new SlideFonts());
            Length width = Length.Zero;
            foreach (PositionedGlyph glyph in placed[1].Run.Glyphs) width += glyph.Advance;
            return placed[1].Run.Origin.X + width;
        }

        double ordinal(string properties)
        {
            List<PlacedGlyphRun> placed = SlideTextLayout.Place(
                PptxTextBody.Read(Body(properties)), Area, new SlideFonts());
            Length width = Length.Zero;
            foreach (PositionedGlyph glyph in placed[1].Run.Glyphs) width += glyph.Advance;
            return width.Points;
        }

        Reach("baseline=\"30000\"").ShouldBeLessThan(Reach(string.Empty));

        // 58% of the advance, to the accuracy hinting and rounding leave: the shrink is the run's
        // own and nothing else on the line moves.
        ordinal("baseline=\"30000\"").ShouldBe(ordinal(string.Empty) * 0.58, 0.1);
    }

    /// <summary>
    /// An ordinal never makes its own line taller, because 58% of the ascent plus 30% of the em
    /// is less than the ascent it replaced.
    /// </summary>
    [Fact]
    public void ASuperscriptLeavesTheLineWhereThePlainTextPutsIt()
    {
        Length Baseline(string properties) => SlideTextLayout.Place(
            PptxTextBody.Read(Body(properties)), Area, new SlideFonts())[0].Run.Origin.Y;

        Baseline("baseline=\"30000\"").Points.ShouldBe(Baseline(string.Empty).Points, 0.01);
    }
}
