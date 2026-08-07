using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A shape's text is set in the face its runs name, with the theme's indirection followed.
/// </summary>
/// <remarks>
/// <para>
/// The face reaches three measurements and not only the ink: it sets the line height, it sets the
/// advance widths the wrap is decided by, and it sets the ascent the baseline sits at. So a body
/// drawn in one face and measured in another breaks every line in a place its own metrics did not
/// pick, and the error compounds down the box.
/// </para>
/// <para>
/// <c>&lt;a:latin typeface="+mn-lt"/&gt;</c> is what most producers write and it is not a family
/// name — it means "the theme's minor Latin face" and is resolved through <c>a:fontScheme</c>
/// (<c>Theme::resolveFont</c>, <c>oox/source/drawingml/theme.cxx:71</c>). Handing the literal
/// string to a font resolver asks for a family that exists nowhere and gets whatever fontconfig
/// offers, which is how a Calibri text box came to be measured in Liberation Sans.
/// </para>
/// <para>
/// The fixture is LibreOffice's own <c>sheet-shape-text.xlsx</c> with its theme's minor Latin face
/// changed to Calibri and its major to Times New Roman, and two text boxes in place of one: the
/// first naming its face indirectly, the second naming Times New Roman outright. Checked against
/// LibreOffice 24.2.7.2's own PDF for it, which embeds <c>Carlito-Regular</c>,
/// <c>LiberationSerif</c> and <c>LiberationSans</c> and wraps each box after "the" and "run"
/// respectively — the same three faces and the same two breaks Paperless now produces.
/// </para>
/// <para>
/// Measured on <c>SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA.xlsx</c>, whose methodology
/// note is one text box stating <c>+mn-lt</c> against a Calibri theme: its drawn line pitch was
/// 12.65 pt against the reference's 13.43 and is 13.43 now, and its extractable words went 479 to
/// 519 of 550. Seven of the track's 109 package spreadsheets state a typeface on shape text; all
/// seven were rendered before and after and none changed verdict.
/// </para>
/// </remarks>
public sealed class SheetShapeFontTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-shape-theme-font.xlsx"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    private static SheetShapeParagraph Paragraph(SheetLayout sheet, string opening)
        => sheet.Drawings.Items
            .Select(drawing => drawing.Text)
            .Where(text => text is not null)
            .SelectMany(text => text!.Paragraphs)
            .First(paragraph => paragraph.Text.StartsWith(opening, StringComparison.Ordinal));

    [Fact]
    public void AnIndirectTypefaceIsResolvedThroughTheThemesFontScheme()
    {
        Paragraph(Sheet(), "Themed box").Runs[0].Family.ShouldBe(
            "Calibri", "+mn-lt is the theme's minor Latin face, not a family called \"+mn-lt\"");
    }

    [Fact]
    public void AStatedTypefaceIsKeptAsItStands()
    {
        Paragraph(Sheet(), "Stated box").Runs[0].Family.ShouldBe("Times New Roman");
    }

    /// <summary>
    /// The face reaches the drawing, and reaches it as the resolver's own answer.
    /// </summary>
    /// <remarks>
    /// Carlito for the themed box and Liberation Serif for the stated one, which is what
    /// LibreOffice's own PDF for this fixture embeds. Asserting on the drawn run rather than on the
    /// model is what makes this a rendering test: a family read into the model and dropped on the
    /// way to the sink would satisfy the two above and change no pixel.
    /// </remarks>
    [Fact]
    public void TheResolvedFaceReachesTheDrawnRun()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-shape-theme-font.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        string Face(string opening) => sink.Pages
            .SelectMany(page => page.Runs)
            .First(run => run.Text.StartsWith(opening, StringComparison.Ordinal))
            .Run.Font.FamilyName;

        Face("Themed box").ShouldContain("Carlito");
        Face("Stated box").ShouldContain("Liberation Serif");
    }

    /// <summary>
    /// The two boxes are measured apart, which is the whole point of carrying the face.
    /// </summary>
    /// <remarks>
    /// Carlito's line box is wider than Liberation Sans's at the same size, so a body measured in
    /// the wrong one breaks in the wrong place. The two boxes hold different words, so the test is
    /// on the baselines: the themed box's second line sits further below its first than the stated
    /// box's does, and it can only do so if two different faces supplied the two line heights.
    /// </remarks>
    [Fact]
    public void TheLineHeightComesFromEachBoxsOwnFace()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-shape-theme-font.xlsx"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        List<DrawnGlyphRun> runs = [.. sink.Pages.SelectMany(page => page.Runs)];

        double Pitch(string opening)
        {
            int at = runs.FindIndex(run => run.Text.StartsWith(opening, StringComparison.Ordinal));
            at.ShouldBeGreaterThanOrEqualTo(0, opening);
            return (runs[at + 1].Origin.Y - runs[at].Origin.Y).Points;
        }

        Pitch("Themed box").ShouldBeGreaterThan(Pitch("Stated box"));
    }
}
