using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A shape's text that names no typeface is set in the drawing layer's default face, which is not
/// the sheet's.
/// </summary>
/// <remarks>
/// <para>
/// A shape on a sheet is an <c>SdrObject</c> and its text lives in the drawing layer's item pool.
/// <c>SdrModel::SetTextDefaults</c> seeds that pool from <c>DefaultFontType::LATIN_TEXT</c>
/// (<c>svx/source/svdraw/svdmodel.cxx</c>:668-669), which <c>VCL.xcu</c> heads with Liberation
/// Serif; a cell takes <c>DefaultFontType::LATIN_SPREADSHEET</c>
/// (<c>sc/source/core/data/docpool.cxx</c>:201-202), which is Liberation Sans. Both defaults are
/// live in the same workbook, so the shape path cannot borrow the cell face the way a header band
/// correctly does — and it did, which is what this pins.
/// </para>
/// <para>
/// Measured on 26.2.4.2 over the five corpus workbooks carrying Excel's slicer-fallback shape,
/// whose runs name no typeface: <c>Part_129_Operators.xlsx</c>, <c>Part_375_Operators.xlsx</c>,
/// <c>TDA_Smoke-Detectors.xlsx</c>, <c>DynamicBubbleChart.xlsx</c> and
/// <c>049_Expenses_calculator…xlsx</c> together draw 77 spans the reference sets in
/// <c>LiberationSerif</c> and we set in <c>LiberationSans</c>.
/// </para>
/// <para>
/// Built in memory rather than from a fixture, because the question is what the painter does with
/// a run whose <c>Family</c> is null and no producer writes a text box that plainly — Excel's own
/// slicer fallback is the shape that does, and it is not a file this repository ships.
/// </para>
/// </remarks>
public sealed class SheetShapeDefaultFontTests
{
    private static readonly DocRect Box = new(
        Length.FromPoints(0), Length.FromPoints(0), Length.FromPoints(400), Length.FromPoints(80));

    private static List<DrawnGlyphRun> Draw(string? family, bool bold = false)
    {
        SheetShapeText text = new()
        {
            Paragraphs =
            [
                new SheetShapeParagraph
                {
                    Runs = [new SheetShapeRun("Handgloves", Length.FromPoints(11), family, bold)],
                },
            ],
        };

        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(400), Length.FromPoints(80)));
        SheetShapePainter.Draw(sink, text, Box, scale: 1.0);
        sink.EndPage();

        return sink.Pages[0].Runs;
    }

    [Fact]
    public void ARunNamingNoTypefaceIsSetInTheDrawingLayersDefault()
    {
        List<DrawnGlyphRun> runs = Draw(null);

        runs.ShouldNotBeEmpty();
        runs.ShouldAllBe(run => run.Run.Font.FamilyName == SheetShapeText.DefaultFamily);
    }

    [Fact]
    public void ThatDefaultIsNotTheOneACellTakes()
        // Stated as an inequality as well as a value, because the defect was the two being the
        // same thing and a test that only named one face would still pass if they were merged.
        => SheetShapeText.DefaultFamily.ShouldNotBe(SheetFonts.DefaultFamily);

    [Fact]
    public void ARunThatNamesOneIsStillSetInIt()
    {
        List<DrawnGlyphRun> runs = Draw(SheetFonts.DefaultFamily);

        runs.ShouldNotBeEmpty();
        runs.ShouldAllBe(run => run.Run.Font.FamilyName == SheetFonts.DefaultFamily);
    }

    [Fact]
    public void ABoldRunIsDrawnInTheBoldFace()
    {
        // A shape run's `b="1"` was read by nothing: the model did not carry a weight, so a bold
        // note box was measured and drawn in the regular face. A bold face is a different file with
        // different advances, so this is a wrap as well as an ink difference — measured on
        // `Air_Boss_Master_List.xlsx`, whose one bold paragraph 26.2.4.2 draws in Carlito-Bold.
        List<DrawnGlyphRun> upright = Draw("Liberation Sans");
        List<DrawnGlyphRun> heavy = Draw("Liberation Sans", bold: true);

        upright.ShouldNotBeEmpty();
        heavy.ShouldNotBeEmpty();
        upright.ShouldAllBe(run => run.Run.Font.Weight < 700);
        heavy.ShouldAllBe(run => run.Run.Font.Weight >= 700);
    }
}
