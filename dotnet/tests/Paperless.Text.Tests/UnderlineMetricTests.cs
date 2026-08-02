using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Where a rule goes under a face whose own <c>post</c> table LibreOffice refuses to read.
/// </summary>
/// <remarks>
/// <para>
/// Liberation Serif, Liberation Sans and Liberation Mono are named in LibreOffice's shipped
/// configuration — <c>Office::Common::Misc::FontsDontUseUnderlineMetrics</c>, tdf#152267 and
/// tdf#154235 — and <c>FontMetricData::ShouldNotUseUnderlineMetrics</c> checks that list before it
/// will read a face's own numbers. Three names sound like a curiosity; they are in fact the
/// metric-compatible substitutes for Arial, Times New Roman and Courier New, which is what most of
/// a real corpus is set in.
/// </para>
/// <para>
/// Their <c>post</c> tables are wrong in a way that shows. Liberation Serif Bold declares an
/// underline 195 units thick sitting 28 units below the baseline; at 28 pt that is a 2.67 pt rule
/// 0.38 pt under the text — nearly touching it and almost twice as thick as the 1.53 pt rule
/// 2.30 pt down that LibreOffice's own PDF draws. Measured on page 13 of
/// <c>slides/batch-001/ppt/wells08_basic.ppt</c>, rasterised at 300 dpi: eleven pixels of rule
/// starting three below the baseline against the reference's seven starting nine below.
/// </para>
/// <para>
/// The expectations here are stated design units rather than a read of the installed file, so the
/// arithmetic is tested whether or not the font is present — but the two faces named do have to be
/// spelled the way the list spells them, because the name is the whole discriminator.
/// </para>
/// </remarks>
public class UnderlineMetricTests
{
    private static LineMetrics LiberationSerif()
        => new(1825, 443, 87, LineMetricSource.HorizontalHeader, 2048, null);

    [Theory]
    [InlineData("Liberation Serif")]
    [InlineData("Liberation Sans")]
    [InlineData("Liberation Mono")]
    public void ABlacklistedFaceTakesItsRuleFromTheDescentInstead(string family)
    {
        // post says 195/-28; the descent rule says a quarter of the descent thick, half a descent
        // down, less half the thickness so the offset is to the rule's top.
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(
            family, new PostTable(-28, 195, 0, false), os2: null, LiberationSerif());

        metrics.UnderlineThickness.ShouldBe(111);   // round(443 / 4)
        metrics.UnderlinePosition.ShouldBe(-166);   // -(443/2 - 111/2)

        // Which at 28 pt is within a tenth of a pixel at 300 dpi of what LibreOffice draws.
        (28.0 * metrics.UnderlineThickness / 2048).ShouldBe(1.531, 0.03);
        (28.0 * -metrics.UnderlinePosition / 2048).ShouldBe(2.296, 0.03);
    }

    [Fact]
    public void AFaceNotOnTheListKeepsItsOwnDeclaredMetrics()
    {
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(
            "Carlito", new PostTable(-150, 90, 0, false), os2: null, LiberationSerif());

        metrics.UnderlineThickness.ShouldBe(90);
        metrics.UnderlinePosition.ShouldBe(-150);
    }

    [Fact]
    public void AFaceDeclaringNothingStillDrawsARule()
    {
        // A zero thickness draws no line at all, so an absent post table falls back rather than
        // being taken at its word.
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(
            "Something", new PostTable(0, 0, 0, false), os2: null, LiberationSerif());

        metrics.UnderlineThickness.ShouldBeGreaterThan(0);
        metrics.UnderlinePosition.ShouldBeLessThan(0);
    }

    [Fact]
    public void ABlacklistedFacesStrikethroughSitsAboveItsBaseline()
    {
        FontVerticalMetrics metrics = LineSpacing.ResolveDecorations(
            "Liberation Serif", new PostTable(-28, 195, 0, false), os2: null, LiberationSerif());

        // A third of the way up the ascent, less the internal leading — the ascent and descent of
        // Liberation Serif exceed its em by 220 units.
        metrics.StrikeoutPosition.ShouldBe(590);
        metrics.StrikeoutThickness.ShouldBe(111);
    }
}
