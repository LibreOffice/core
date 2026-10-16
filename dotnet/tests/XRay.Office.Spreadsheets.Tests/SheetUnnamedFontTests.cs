using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A <c>&lt;font&gt;</c> in <c>styles.xml</c> that names no face is Cambria at 11 pt.
/// </summary>
/// <remarks>
/// <para>
/// Not the workbook's own <c>fonts[0]</c>, which is the reading the schema invites, and not a
/// generic sans. Every <c>&lt;font&gt;</c> the OOXML filter builds begins as a copy of the theme
/// buffer's default model, and that model is a hard-coded <c>Cambria</c> at 11.0 —
/// <c>ThemeBuffer::ThemeBuffer</c> (<c>sc/source/filter/oox/themebuffer.cxx:31-33</c>), where it
/// carries a "TODO: locale dependent font name" that has never been done. Nothing in the theme
/// part overrides it: <c>getDefaultFontModel</c> hands back that same object however the
/// workbook's major and minor fonts are declared.
/// </para>
/// <para>
/// The fixture is <c>dotnet/probes/sheets-rest-01/mkfontprobe.py</c>, whose <c>fonts[0]</c> is
/// deliberately <em>Arial 10</em> so that the two candidate answers are distinguishable — a
/// fixture whose default font were Calibri 11 would pass either way. Measured against the
/// installed LibreOffice 26.2.4.2, reading the face out of its own PDF: <c>&lt;font/&gt;</c>
/// draws in Caladea-Regular at 11.00 pt, <c>&lt;font&gt;&lt;b/&gt;&lt;/font&gt;</c> in
/// Caladea-Bold at 11.00, <c>&lt;font&gt;&lt;sz val="20"/&gt;&lt;/font&gt;</c> in Caladea-Regular
/// at 20.01, and <c>&lt;font&gt;&lt;name val="Arial"/&gt;&lt;/font&gt;</c> in LiberationSans at
/// <em>11.00</em>. Caladea is Cambria's metric-compatible substitute, which is what makes the
/// answer visible in a rendering at all.
/// </para>
/// <para>
/// <c>ans_mappings_of_eccairs_terms.xlsx</c> is the corpus case: five of its seventeen fonts name
/// no face, the reference embeds Caladea-Regular and Caladea-Bold for them, and setting those runs
/// in a sans face left a few extra characters on most of its pages and one extra page over 190.
/// </para>
/// <para>
/// The size half matters on its own and is the wider reach: the rule was previously 10 pt, so
/// every <c>&lt;font&gt;</c> naming a face and stating no <c>sz</c> was a point small.
/// </para>
/// </remarks>
public sealed class SheetUnnamedFontTests
{
    [Theory]
    [InlineData(1, "Caladea", 11.0)]   // <font/>
    [InlineData(2, "Caladea", 11.0)]   // <font><b/></font>
    [InlineData(3, "Caladea", 20.0)]   // a size but no name
    [InlineData(4, "Liberation Sans", 11.0)]   // a name but no size
    [InlineData(5, "Caladea", 11.0)]   // a colour and nothing else
    [InlineData(6, "Caladea", 11.0)]   // an underline and a colour
    public void AFontThatStatesNoFaceTakesTheThemeDefault(int row, string family, double points)
    {
        DrawnGlyphRun run = Drawn()[row];

        run.Run.Font.FamilyName.ShouldBe(family);
        run.Run.FontSize.Points.ShouldBe(points, 0.05);
    }

    [Fact]
    public void AFontThatStatesBothIsUnaffected()
    {
        // The control. `fonts[0]` is Arial 10 and stays Arial 10, so the rule cannot be "make
        // everything Cambria".
        DrawnGlyphRun first = Drawn()[0];

        first.Run.Font.FamilyName.ShouldBe("Liberation Sans");
        first.Run.FontSize.Points.ShouldBe(10.0, 0.05);
    }

    [Fact]
    public void TheBoldOneResolvesToTheBoldFace()
    {
        // Caladea-Bold in the reference, and the reason the weight is checked separately: a rule
        // that set the family and dropped the weight would still satisfy the family assertion.
        Drawn()[2].Run.Font.Weight.ShouldBeGreaterThanOrEqualTo(700);
    }

    /// <summary>One run per row of the fixture, in row order.</summary>
    private static List<DrawnGlyphRun> Drawn()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-font-unnamed.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        // One cell per row and nothing else on the page, so the runs come out in row order.
        return sink.Pages[0].Runs;
    }
}
