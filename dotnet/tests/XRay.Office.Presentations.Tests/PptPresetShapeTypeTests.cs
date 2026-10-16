using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Pins the bridge from the binary drawing layer's numbered shape types onto the DrawingML preset
/// names the shared evaluator knows.
/// </summary>
/// <remarks>
/// <para>
/// The table used to hold six entries and everything else drew its bounding rectangle, so a
/// <c>.ppt</c> block diagram joined by arrows came out as a row of plain boxes — a defect no gate
/// on this corpus can see, because both documents it was found on match on page count, word count
/// and font embedding.
/// </para>
/// <para>
/// Two properties are worth holding rather than the two hundred rows. <strong>Every name the table
/// yields must resolve</strong>, or the lookup has quietly reintroduced the bounding-rectangle
/// fallback under a name that reads like support. And <strong>the deliberate absentees must stay
/// absent</strong>: type 0 is what a group and a freeform both carry, the WordArt types describe a
/// path text is bent along rather than an outline, and a picture frame and a text box are their
/// rectangle already.
/// </para>
/// </remarks>
public class PptPresetShapeTypeTests
{
    [Fact]
    public void EveryPresetTheTableNamesIsOneTheEvaluatorKnows()
    {
        List<ushort> unknown = [];

        for (ushort type = 0; type < 4096; type++)
        {
            if (PptShapeGeometry.PresetOf(type) is { } preset && !SlidePresetGeometry.IsKnown(preset))
            {
                unknown.Add(type);
            }
        }

        unknown.ShouldBeEmpty();
    }

    [Fact]
    public void TheTableCoversTheWholeMsoShapeTypeRangeRatherThanAHandful()
    {
        int named = 0;

        for (ushort type = 0; type < 4096; type++)
        {
            if (PptShapeGeometry.PresetOf(type) is not null) named++;
        }

        // 148 of the 203 MSO_SPT values name a preset PresetShapeGeometry holds; the rest are the
        // absentees the next test pins. A regression to the six that were transcribed by hand is
        // what this number exists to catch.
        named.ShouldBe(148);
    }

    [Theory]
    // The arrows and the connectors, which are what a block diagram is made of.
    [InlineData((ushort)13, "rightArrow")]
    [InlineData((ushort)66, "leftArrow")]
    [InlineData((ushort)69, "leftRightArrow")]
    [InlineData((ushort)104, "curvedUpArrow")]
    [InlineData((ushort)32, "straightConnector1")]
    [InlineData((ushort)34, "bentConnector3")]
    // A line is the one type whose outline is a diagonal rather than its rectangle, so drawing it
    // as a box paints over whatever it was drawn across.
    [InlineData((ushort)20, "line")]
    [InlineData((ushort)109, "flowChartProcess")]
    [InlineData((ushort)87, "leftBrace")]
    // The six that were there before, unchanged.
    [InlineData((ushort)2, "roundRect")]
    [InlineData((ushort)3, "ellipse")]
    [InlineData((ushort)4, "diamond")]
    [InlineData((ushort)5, "triangle")]
    [InlineData((ushort)6, "rtTriangle")]
    public void AShapeTypeExpandsAsTheDrawingMlPresetItCorrespondsTo(ushort type, string preset)
        => PptShapeGeometry.PresetOf(type).ShouldBe(preset);

    [Theory]
    [InlineData((ushort)0)]     // mso_sptNotPrimitive: a group, or a freeform with its own vertices
    [InlineData((ushort)1)]     // a rectangle is the fallback already
    [InlineData((ushort)24)]    // WordArt: a path text is bent along, not an outline
    [InlineData((ushort)156)]   // WordArt again, the second block
    [InlineData((ushort)75)]    // a picture frame
    [InlineData((ushort)201)]   // a host control
    [InlineData((ushort)202)]   // a text box
    [InlineData((ushort)4095)]  // mso_sptNil, which LibreOffice's own export writes
    public void ATypeWithNoOutlineOfItsOwnStaysOnTheBoundingRectangle(ushort type)
        => PptShapeGeometry.PresetOf(type).ShouldBeNull();

    [Fact]
    public void AnAdjustmentIsConvertedOnlyWhereTheTwoVocabulariesMeasureTheSameThing()
    {
        DocSize square = new(Length.FromMm100(4000), Length.FromMm100(4000));

        // 21600ths to hundred-thousandths, for the presets whose DrawingML adjustment was measured
        // to mean what the binary one does -- round 127's conversion census.
        PptShapeGeometry.Adjustment(2, 10800, square).ShouldBe(50000);
        PptShapeGeometry.Adjustment(5, 21600, square).ShouldBe(100000);
        PptShapeGeometry.Adjustment(22, 10800, square).ShouldBe(50000);   // can

        // Everything else is drawn at the preset's stated defaults rather than fed a foreign
        // number. Passing one through would put an arrow's head where a rounded rectangle's corner
        // radius belongs.
        PptShapeGeometry.Adjustment(13, 10800, square).ShouldBeNull();
        PptShapeGeometry.Adjustment(109, 10800, square).ShouldBeNull();

        // A connector that has no adjustment in `EnhancedCustomShapeGeometry` at all -- a straight
        // connector is two points -- states one in 28 corpus shapes and the reference ignores it.
        PptShapeGeometry.Adjustment(32, 8826, square).ShouldBeNull();

        // And one that does have an adjustment is still discarded, because the reference replaces
        // every connector with an SdrEdgeObj and routes it: `msdffimp.cxx`:4792-4888.
        PptShapeGeometry.Adjustment(34, 8826, square).ShouldBeNull();
        PptShapeGeometry.Adjustment(38, 8826, square).ShouldBeNull();
    }

    [Fact]
    public void ATrapezoidsAdjustmentCrossesTheWidthAndTheDrawingMlPresetsCrossesTheShortSide()
    {
        // Escher insets the narrow edge by `adj/21600` of the WIDTH; `trapezoid` insets it by
        // `ss x a/100000`. In a square box the two bases coincide and the conversion is the plain
        // rescale...
        DocSize square = new(Length.FromMm100(4000), Length.FromMm100(4000));
        PptShapeGeometry.Adjustment(8, 10800, square)!.Value.ShouldBe(50000, 1e-6);

        // ...and in a box half as tall as it is wide it is twice that, which is the whole of the
        // 90.6 pt this tree used to inset `ws_prod-...-European-Safety-Strategy-Initiative.ppt`
        // slide 8's trapezoid against the reference's 170.8.
        DocSize wide = new(Length.FromMm100(8000), Length.FromMm100(4000));
        PptShapeGeometry.Adjustment(8, 10800, wide)!.Value.ShouldBe(100000, 1e-6);
    }

    [Fact]
    public void OnlyTheTrapezoidIsAReflectionOfTheDrawingMlPresetOfItsName()
    {
        // All 148 name-mapped types were expanded by 26.2.4.2 and here and compared under the
        // identity and all three reflections: `probes/pptgeom-r127/preset-census.tsv`.
        PptShapeGeometry.MirrorsVertically(8).ShouldBeTrue();

        foreach (ushort type in (ushort[])[2, 5, 7, 9, 13, 34, 38, 43, 49, 63, 109])
        {
            PptShapeGeometry.MirrorsVertically(type).ShouldBeFalse();
        }
    }

    [Fact]
    public void TheEvaluatorGivesAnArrowAnOutlineThatIsNotItsRectangle()
    {
        // The property the whole table exists for: the shape a corpus reviewer saw as a plain box.
        PresetShapeGeometry.Find(PptShapeGeometry.PresetOf(13)).ShouldNotBeNull();
    }
}
