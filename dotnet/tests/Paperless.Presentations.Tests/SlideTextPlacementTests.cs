using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks where a shape's text is drawn, against the pens in LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The deck is <c>shape-geometry.pptx</c> slide 3, four text boxes at round positions in
/// Liberation Sans at 18 pt — a face that is installed, so nothing is substituted and every
/// advance is the one the reference measured. Each box isolates one property: zero insets,
/// DrawingML's default insets, centred alignment, and middle anchoring.
/// </para>
/// <para>
/// The vertical numbers are the interesting ones, because they are not the font's. LibreOffice
/// draws the first box's baseline 18.000 pt below the top of the shape — one em exactly, where
/// Liberation Sans's own ascent is 0.905 em and would put it at 16.30. That is EditEngine's
/// font-independent line spacing, which the PPTX importer turns on for every text body
/// (<c>oox/source/ppt/pptshapecontext.cxx:186</c>), and it makes the ascent the font height and
/// the line 1.2 times it (<c>editeng/source/editeng/impedit3.cxx:501,3138</c>).
/// </para>
/// </remarks>
public class SlideTextPlacementTests
{
    /// <summary>
    /// How far a pen may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A tenth of a point, the bound the rest of the project holds itself to. It covers both the
    /// 0.0283 pt the PDF export shifts everything by and the hundredth of a point the
    /// middle-anchor arithmetic rounds by.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    private static LaidOutSlide TextSlide()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("shape-geometry.pptx")));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[2];
    }

    private static GlyphRun Only(LaidOutSlide slide, string name)
    {
        PlacedShape shape = slide.Shapes.First(candidate => candidate.Name == name);

        shape.Text.ShouldNotBeNull();
        shape.Text.IsUpright.ShouldBeTrue();
        return shape.Text.Runs.Single().Run;
    }

    [Fact]
    public void AFirstBaselineSitsOneEmBelowTheTextTop()
    {
        GlyphRun run = Only(TextSlide(), "TopLeftText");

        // The shape is at 72, 72 with every inset zero. LibreOffice draws the line at
        // 71.972 450.028 Td on a 540 pt page, which is 71.972 across and 89.972 down.
        run.Origin.X.Points.ShouldBe(72, TolerancePoints);
        run.Origin.Y.Points.ShouldBe(90, TolerancePoints);
        run.Text.ShouldBe("Anchored top left");
    }

    [Fact]
    public void TheStatedInsetsMoveThePenBothWays()
    {
        GlyphRun run = Only(TextSlide(), "InsetText");

        // lIns="91440" is 7.2 pt and tIns="45720" is 3.6 pt, on a shape at 72, 216. LibreOffice
        // draws 79.172 302.428 Td: 79.172 across and 237.572 down, which is 72 + 7.2 and
        // 216 + 3.6 + 18.
        run.Origin.X.Points.ShouldBe(79.2, TolerancePoints);
        run.Origin.Y.Points.ShouldBe(237.6, TolerancePoints);
    }

    [Fact]
    public void ACentredParagraphIsCentredInTheTextRectangle()
    {
        GlyphRun run = Only(TextSlide(), "CentredText");

        // The shape runs 396 to 540 with no insets, so a centred line of width w starts at
        // 468 - w/2. LibreOffice draws it at 435.969, which makes its "Centred" 64.06 pt wide —
        // and the assertion is really that our shaping agrees with that to a tenth of a point.
        run.Origin.X.Points.ShouldBe(435.97, TolerancePoints);
        run.Origin.Y.Points.ShouldBe(90, TolerancePoints);
    }

    [Fact]
    public void MiddleAnchoringCentresTheBlockOnTheLineHeightAndNotOnTheFont()
    {
        GlyphRun run = Only(TextSlide(), "MiddleAnchored");

        // anchor="ctr" on a shape at 216 that is 72 pt tall, holding one 21.6 pt line: the block
        // starts at 216 + (72 - 21.6) / 2 = 241.2 and its baseline is an em below that, at 259.2.
        // LibreOffice draws 395.972 280.828 Td, which is 259.172 down — 0.014 pt from the
        // arithmetic, and 1.4 pt from what the font's own metrics would have given.
        run.Origin.X.Points.ShouldBe(396, TolerancePoints);
        run.Origin.Y.Points.ShouldBe(259.2, TolerancePoints);
    }

    [Fact]
    public void ABulletsHangingIndentIsTheRoomTheMarkerOccupies()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("deck-features.pptx")));

        LaidOutSlide slide = ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];

        // The outline placeholder, whose paragraphs state marL="216000" indent="-216000" — a
        // 17.01 pt hanging indent — with an a:buChar at 45% of the run size.
        PlacedShape outline = slide.Shapes.First(
            shape => shape.Text is { Runs.Count: > 2 } && shape.Name == "PlaceHolder 2");

        List<GlyphRun> runs = [.. outline.Text!.Runs.Select(run => run.Run)];

        // LibreOffice's PDF draws the first level's bullet at 56.693 and its text at 73.701, and
        // the second level's at 73.701 and 90.709. Applying the hanging indent to the text as
        // well as to the marker puts every line 17 pt to the left of that.
        runs[0].FontSize.Points.ShouldBe(12.6, 0.05);
        runs[0].Origin.X.Points.ShouldBe(56.69, TolerancePoints);
        runs[0].Glyphs.Count.ShouldBe(1);

        runs[1].FontSize.Points.ShouldBe(28, 0.05);
        runs[1].Origin.X.Points.ShouldBe(73.70, TolerancePoints);

        // The nested item, one outline level deeper, which the same rule moves by the same amount.
        runs[^1].Origin.X.Points.ShouldBe(90.71, TolerancePoints);
    }

    [Fact]
    public void ARotatedShapesTextTravelsWithItsOwnTransform()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("deck-features.pptx")));

        SlidePages pages = (SlidePages)((IPaginatedDocument)document).Layout();

        // Nothing in the corpus rotates a shape that also holds text, so what is asserted here is
        // the invariant rather than a measurement: an upright shape's runs are in slide
        // coordinates and carry no matrix, which is what keeps a backend's pens directly
        // comparable with a reference renderer's.
        foreach (LaidOutSlide slide in pages.Slides)
        {
            foreach (PlacedShape shape in slide.Shapes)
            {
                if (shape.Text is not { } text) continue;
                text.IsUpright.ShouldBeTrue();
            }
        }
    }
}
