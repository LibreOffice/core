using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A shape's text is measured run by run, and a run that states no size takes twelve point.
/// </summary>
/// <remarks>
/// <para>
/// Two rules, and the fixture separates them because getting either wrong looks the same from a
/// distance — every line breaks early.
/// </para>
/// <para>
/// <strong>An unsized run is 12 pt, not the shape's 18.</strong> A DrawingML shape carries a
/// default character height of 18 pt (<c>Shape::setDefaults</c>,
/// <c>oox/source/drawingml/shape.cxx:334</c>), and that is what LibreOffice's flat-ODS export
/// writes as each of these three shapes' default paragraph style. It is not what a run inherits:
/// <c>TextBody::insertAt</c> reads the text cursor's <c>CharHeight</c> before anything is inserted
/// (<c>oox/source/drawingml/textbody.cxx:62</c>) and <c>TextRun::insertAt</c> puts that on any run
/// whose own <c>moHeight</c> is unset (<c>textrun.cxx:82-85</c>) — the EditEngine pool's own 240
/// twips on a fresh Calc drawing object. The export of this fixture states 12 pt for both of its
/// unsized runs and 18 pt for none of them.
/// </para>
/// <para>
/// <strong>A paragraph is not one size.</strong> The EditEngine breaks a portion at every run
/// boundary, so a body at <c>sz="1100"</c> followed by an unsized space wraps at eleven point and
/// not at twelve, and a following <c>sz="1800"</c> run leaves the body's breaks untouched.
/// Collapsing a paragraph to the largest size any run states measures a long body at the size of
/// one stray character.
/// </para>
/// <para>
/// Checked against LibreOffice 24.2.7.2's own PDF for this fixture, whose three boxes break after
/// "decides", "every" and "every" respectively — which is what Paperless now produces, with the
/// whole layout-preserved text layer identical.
/// </para>
/// </remarks>
public sealed class SheetShapeRunSizeTests
{
    private const string Fixture = "sheet-shape-run-size.xlsx";

    /// <summary>The words the given box's lines begin with, in order.</summary>
    private static List<string> Lines(string box)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetLayout sheet = pages.Sheets[0];

        // The boxes are stacked down the sheet in the order they are anchored, so the drawing's
        // index picks one out; its text is then whatever the painter drew for it.
        int index = sheet.Drawings.Items
            .Select((drawing, at) => (drawing, at))
            .First(pair => pair.drawing.Name == box)
            .at;

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        // A line is every piece sharing a baseline, joined in the order it was drawn — the mixed
        // box draws its body and its large trailing run as two pieces of one line.
        List<(Length Y, string Text)> drawn = [.. sink.Pages
            .SelectMany(page => page.Runs)
            .Where(run => !run.Text.StartsWith("cell ", StringComparison.Ordinal)
                          && !run.Text.Contains("Sheet1", StringComparison.Ordinal)
                          && !run.Text.StartsWith("Page ", StringComparison.Ordinal))
            .GroupBy(run => run.Origin.Y)
            .OrderBy(group => group.Key.Emu)
            .Select(group => (group.Key, string.Concat(group.OrderBy(r => r.Origin.X.Emu)
                                                            .Select(r => r.Text))))];

        // Every box holds the same body, so the lines cannot be told apart by their text; the
        // boxes are separated by a vertical gap far larger than a line.
        List<List<string>> boxes = [];
        Length previous = Length.Zero;
        foreach ((Length y, string text) in drawn)
        {
            if (boxes.Count == 0 || (y - previous).Points > 30) boxes.Add([]);
            boxes[^1].Add(text);
            previous = y;
        }

        return boxes[index];
    }

    /// <summary>
    /// A box whose only run states no size wraps at twelve point.
    /// </summary>
    /// <remarks>
    /// At eighteen this body takes four lines rather than two and breaks after "the"; at twelve it
    /// breaks after "decides", which is where LibreOffice's own PDF breaks it.
    /// </remarks>
    [Fact]
    public void AnUnsizedRunIsTwelvePoint()
    {
        List<string> lines = Lines("NoSizeAtAll");

        lines.Count.ShouldBe(2);
        lines[0].ShouldEndWith("decides");
        lines[1].ShouldStartWith("where every one");
    }

    /// <summary>
    /// A trailing unsized run does not re-measure the body in front of it.
    /// </summary>
    /// <remarks>
    /// This is the shape <c>SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx</c> has, and it
    /// is what a paragraph-wide size gets wrong: the body is 11 pt and the trailing space is 12,
    /// so the largest-run rule wraps 440 characters of body text one point too wide and breaks
    /// every line of it early. The break here is after "every", one word later than the box above.
    /// </remarks>
    [Fact]
    public void ATrailingUnsizedRunLeavesTheBodysBreaksAlone()
    {
        List<string> lines = Lines("TrailingUnsized");

        lines.Count.ShouldBe(2);
        lines[0].ShouldEndWith("every");
        lines[1].ShouldStartWith("one of its lines");
    }

    /// <summary>
    /// An eighteen-point run after the body leaves the body's breaks alone as well.
    /// </summary>
    /// <remarks>
    /// The clean statement of the same rule: the large run is real, stated and drawn, and it still
    /// does not reach the words in front of it. The largest-run rule would set the whole body at
    /// eighteen point.
    /// </remarks>
    [Fact]
    public void AStatedLargeRunDoesNotReMeasureTheBody()
    {
        List<string> lines = Lines("MixedSizes");

        lines.Count.ShouldBe(2);
        lines[0].ShouldEndWith("every");
        lines[^1].ShouldEndWith(" bigger");
    }
}
