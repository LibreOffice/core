using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A watermark picture on a sheet: <c>xdr:blipFill/a:blip/a:alphaModFix</c>, and the opacity it
/// has to reach the sink at.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the defect no gate column can see.</b> A picture anchored over eighteen rows of
/// text and painted opaque hides the text and leaves every word of it in the PDF's text layer, so
/// page count, extractable words and font embedding all read correct while the page is unreadable.
/// Measured on <c>SIL_TDB648.xlsx</c>, whose <c>General Info</c> sheet states
/// <c>a:alphaModFix amt="20000"</c> on a full-width photograph: its word column read 7499 against
/// 7497 while about 85% of the body copy was buried. The arithmetic confirms the number rather
/// than only the fact — the reference's faded pixels sample at RGB 217/223/233 against our opaque
/// 74/97/157, and <c>0.2·74 + 0.8·255 = 219</c>.
/// </para>
/// <para>
/// <b>It is not a paint-order defect</b>, which is what it looks like and what was suspected
/// first. Calc prints <c>SC_LAYER_BACK</c> before the cell text and <c>SC_LAYER_FRONT</c> after
/// (<c>sc/source/ui/view/printfun.cxx:1651</c> and <c>:1699</c>); a sheet picture is on the front
/// layer, so drawing it after the strings — which <see cref="SheetPageGraphics"/> already did — is
/// correct. Moving it behind the text would have hidden this on one document and been wrong on
/// every other.
/// </para>
/// <para>
/// The fixture states the attribute on one picture and omits it on a second, and both are asserted.
/// Half of this test is the control: a reader that faded every picture would pass a fixture holding
/// only the faded one.
/// </para>
/// </remarks>
public sealed class SheetPictureOpacityTests
{
    private const string Fixture = "picture-watermark.xlsx";

    [Fact]
    public void AlphaModFixReachesTheModel()
    {
        IReadOnlyList<SheetDrawing> drawings = Drawings();

        drawings.Count.ShouldBe(2);
        drawings[0].Opacity.ShouldBe(0.20, 0.0005);
    }

    /// <summary>
    /// A picture stating no <c>alphaModFix</c> is fully opaque, which is the half that keeps the
    /// reach of this honest.
    /// </summary>
    [Fact]
    public void APictureWithoutOneIsOpaque() => Drawings()[1].Opacity.ShouldBe(1.0);

    [Fact]
    public void TheFadedPictureIsPaintedAtItsOwnOpacity()
    {
        List<DrawnPicture> pictures = Painted();

        pictures.Count.ShouldBe(2);
        pictures[0].Opacity.ShouldBe(0.20, 0.0005);
    }

    /// <summary>
    /// And the opaque one is still painted at one, so the change cannot have been a constant.
    /// </summary>
    [Fact]
    public void TheOpaquePictureIsStillPaintedAtFullStrength()
        => Painted()[1].Opacity.ShouldBe(1.0);

    /// <summary>
    /// The text the watermark sits over is still drawn, and drawn <em>after</em> it — the order
    /// Calc prints the front drawing layer in is what makes an opaque picture a lid, and it is the
    /// opacity rather than the order that this round changed.
    /// </summary>
    [Fact]
    public void TheCellsUnderItAreStillDrawn()
    {
        RecordingDrawingSink sink = Render();
        string text = string.Concat(sink.Pages[0].Runs.Select(run => run.Run.Text));

        text.ShouldContain("under the watermark");
        text.ShouldContain("under the opaque one");
    }

    // ------------------------------------------------------------------ helpers

    private static IReadOnlyList<SheetDrawing> Drawings()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings.Items;
    }

    private static List<DrawnPicture> Painted() => Render().Pages[0].Pictures;

    private static RecordingDrawingSink Render()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink;
    }
}
