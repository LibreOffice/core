using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A chart label's width includes its trailing blanks, which is what an accounting number format
/// puts at the end of every value-axis tick.
/// </summary>
/// <remarks>
/// <para>
/// A line's trailing blanks are not part of its width — they hang past its right edge rather than
/// pushing a word onto the next line — so a slide's text layout stops the drawn run at the line's
/// visible end. That is right for a wrapped body and wrong for a chart label, which is an
/// EditEngine text shape autogrown around the paragraph whole. The consequence is one-sided: the
/// value axis reserves its widest label's width, so a lost blank makes the band narrow and the
/// plot area that much too wide, with every gridline and every bar in it displaced.
/// </para>
/// <para>
/// Excel's accounting formats end their positive and zero subformats in <c>_)</c> — a blank the
/// width of a closing parenthesis — so this fires on every currency axis rather than on a
/// curiosity. <c>Demick_JetBlue.pptx</c> page 5 is the corpus witness, and the rule was measured
/// at 26.2.4.2 by deleting that one token from its format and changing nothing else
/// (<c>probes/chartblank-r191/</c>): the reference's plot area starts at x = 165.71 pt as
/// authored and at 162.65 without it, the gap from the widest label's last glyph to the plot's
/// edge going 5.91 pt to 2.85 — and this tree drew 161.99 and 2.66, which is the without-it
/// answer. Deleting the zero row's two <c>?</c> placeholders instead moves nothing, because that
/// row is not the widest label; deleting the leading <c>_(</c> moves the edge by the same blank
/// again, so both ends count and only the trailing one was lost.
/// </para>
/// <para>
/// <strong>The fixture is what makes this a test of the rule rather than of that deck.</strong>
/// <c>slide-chart-accounting-axis.pptx</c> is <c>chart-bar-deck.pptx</c> with the accounting code
/// on its value axis and nothing else changed, so the <em>only</em> difference from a deck this
/// suite already asserts exactly is the format. 26.2.4.2 draws its grey wall from x = 131.41 pt;
/// this tree drew 128.70 and now draws 131.41.
/// </para>
/// </remarks>
public class SlideChartTrailingBlankTests
{
    /// <summary>
    /// A twentieth of a point. The quantity under test is 2.71, so this is far finer than it
    /// needs to be — it is loose only against the last bit of an EMU division.
    /// </summary>
    private const double Tolerance = 0.05;

    private static LaidOutSlide Slide(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];
    }

    /// <summary>The chart's grey wall, which is its plot rectangle.</summary>
    private static DocRect Wall(LaidOutSlide slide)
    {
        double left = double.MaxValue, top = double.MaxValue;
        double right = double.MinValue, bottom = double.MinValue;

        foreach (PlacedShape shape in slide.Shapes)
        {
            if (shape.Fill != Paint.Solid(Colour.FromRgb(0xD9D9D9))) continue;

            foreach (PathCommand command in shape.Outline.Commands)
            {
                if (command.Verb == PathVerb.Close) continue;
                left = Math.Min(left, command.Point.X.Points);
                top = Math.Min(top, command.Point.Y.Points);
                right = Math.Max(right, command.Point.X.Points);
                bottom = Math.Max(bottom, command.Point.Y.Points);
            }
        }

        return new DocRect(
            Core.Units.Length.FromPoints(left),
            Core.Units.Length.FromPoints(top),
            Core.Units.Length.FromPoints(right - left),
            Core.Units.Length.FromPoints(bottom - top));
    }

    /// <summary>
    /// An accounting axis' band is wide enough for its widest label's trailing blank.
    /// </summary>
    /// <remarks>
    /// The left edge alone is the assertion, because it is the only edge the value labels decide:
    /// the wall's right edge is 606.76 in the reference and 606.79 here before and after, which is
    /// the residual this suite's own <c>ComputedTolerance</c> records for the same deck.
    /// </remarks>
    [Fact]
    public void AnAccountingValueAxisReservesItsLabelsTrailingBlank()
    {
        DocRect wall = Wall(Slide("slide-chart-accounting-axis.pptx"));

        wall.X.Points.ShouldBe(131.41, Tolerance);
    }

    /// <summary>
    /// The blank is worth a whole space, not a rounding: the same deck without the format keeps
    /// its own left edge.
    /// </summary>
    /// <remarks>
    /// The control that makes the number above mean something. <c>chart-bar-deck.pptx</c> states
    /// <c>General</c> on the same axis, so its labels carry no blank at either end and its wall
    /// starts where <see cref="SlideChartDrawingTests"/> already asserts — 24.4 pt left of the
    /// accounting one, which is the two formats' whole width difference and not the blank alone.
    /// A change that widened every label by a space would move this one too, and it does not:
    /// 107.00 before and after, against 26.2.4.2's 106.53.
    /// </remarks>
    [Fact]
    public void TheSameDeckWithoutTheFormatIsUnmoved()
    {
        DocRect plain = Wall(Slide("chart-bar-deck.pptx"));
        DocRect accounting = Wall(Slide("slide-chart-accounting-axis.pptx"));

        plain.X.Points.ShouldBeLessThan(accounting.X.Points - 20.0);
        plain.X.Points.ShouldBe(106.53, 2.0);
    }
}
