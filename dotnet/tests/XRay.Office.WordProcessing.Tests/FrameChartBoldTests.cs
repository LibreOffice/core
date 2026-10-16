using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A chart label that states a bold weight is drawn in the bold face, on the words track too.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="ChartFace"/> resolved one face and shaped every label through it, so a chart's
/// bold title, bold axis labels and bold data labels were all drawn light on a DOCX, a DOC and
/// an RTF while the model carried the right value — the slides and sheets painters having acted
/// on it since they were written. The class's own remarks named the gap and deferred it.
/// </para>
/// <para>
/// <strong>The witness is <c>021_Unit_Circle_Chart_3D_Pie_Chart</c>.</strong> Its series-level
/// <c>c:dLbls/c:txPr</c> states <c>sz="1400" b="1"</c>; 26.2.4.2 draws its labels in
/// <c>Carlito-Bold</c> at 14.01 pt and this tree drew <c>Carlito-Regular</c> at 14.00, which
/// <c>pdffonts</c> reports as the whole face set of the page. After: <c>Carlito-Bold</c> on both
/// sides. 5 of the corpus's 10 chart-bearing words documents state a bold anywhere in a chart
/// part, and exactly those 5 move.
/// </para>
/// </remarks>
public class FrameChartBoldTests
{
    /// <summary>A family this container has in both weights, and whose two differ in width.</summary>
    private const string Family = "Carlito";

    /// <summary>A family with no bold of its own, to pin the fallback.</summary>
    /// <remarks>
    /// <c>OpenSymbol</c> ships one face. The resolver answers the same file for both weights, so
    /// the pair must collapse rather than embedding a second identical subset.
    /// </remarks>
    private const string SingleWeight = "OpenSymbol";

    private static readonly Length Size = Length.FromPoints(14);

    /// <summary>Bold text is wider than the same text in the regular face.</summary>
    /// <remarks>
    /// The measurement a layout makes: <see cref="ChartFace.Measure"/> is what reserves a
    /// label's room, so a weight the shaper honours and the measurer does not would put the
    /// glyphs outside the box that was kept for them.
    /// </remarks>
    [Fact]
    public void BoldTextMeasuresWiderThanRegular()
    {
        ChartFace face = ChartFace.For(Family);

        Length regular = face.Measure("Handgloves 1st Qtr", Size, null, bold: false).Width;
        Length bold = face.Measure("Handgloves 1st Qtr", Size, null, bold: true).Width;

        regular.ShouldBeGreaterThan(Length.Zero);
        bold.ShouldBeGreaterThan(regular);
    }

    /// <summary>And the shaped run is the one the measurement described.</summary>
    /// <remarks>
    /// Asserted through both entry points because <c>FrameChart</c> reserves room with the first
    /// and draws glyphs from the second; a rule applied to one alone is the defect this closes,
    /// with the sign reversed.
    /// </remarks>
    [Fact]
    public void TheShapedRunAgreesWithTheMeasurement()
    {
        ChartFace face = ChartFace.For(Family);

        foreach (bool bold in new[] { false, true })
        {
            face.Shape("Handgloves", Size, bold)!.Width
                .ShouldBe(face.Measure("Handgloves", Size, null, bold).Width);
        }
    }

    /// <summary>The bold run references the bold file, not the regular one.</summary>
    /// <remarks>
    /// The face key is the path the PDF writer embeds by, so this is what separates "drawn bold"
    /// from "measured bold and drawn light" — which is exactly the state the words track was in.
    /// </remarks>
    [Fact]
    public void TheBoldRunReferencesADifferentFace()
    {
        ChartFace face = ChartFace.For(Family);

        string regular = face.Shape("Handgloves", Size)!.At(default).Font.FaceKey;
        string bold = face.Shape("Handgloves", Size, bold: true)!.At(default).Font.FaceKey;

        regular.ShouldNotBeNullOrEmpty();
        bold.ShouldNotBe(regular);
    }

    /// <summary>A family with one weight answers the same face for both, and embeds one.</summary>
    [Fact]
    public void AFamilyWithNoBoldOfItsOwnKeepsOneFace()
    {
        ChartFace face = ChartFace.For(SingleWeight);

        if (face.Shape("123", Size) is not { } regular) return;

        face.Shape("123", Size, bold: true)!.At(default).Font.FaceKey
            .ShouldBe(regular.At(default).Font.FaceKey);
    }

    /// <summary>
    /// The line height and ascent follow the weight too, because the bold face's metrics are
    /// its own.
    /// </summary>
    /// <remarks>
    /// A label is drawn at <c>blockCentre − blockHeight/2 + ascent</c>, so the two have to move
    /// together or a bold label sits off its own baseline. Carlito's two weights share their
    /// vertical metrics, so this asserts the pair is consistent rather than that it differs —
    /// the failure it catches is one half taking the bold face and the other not.
    /// </remarks>
    [Fact]
    public void TheVerticalMetricsFollowTheWeight()
    {
        ChartFace face = ChartFace.For(Family);

        Length height = face.LineHeightAt(Size, bold: true);
        Length ascent = face.AscentAt(Size, bold: true);

        height.ShouldBeGreaterThan(Length.Zero);
        ascent.ShouldBeGreaterThan(Length.Zero);
        ascent.ShouldBeLessThan(height);
        face.Measure("Handgloves", Size, null, bold: true).Height.ShouldBe(height);
    }
}
