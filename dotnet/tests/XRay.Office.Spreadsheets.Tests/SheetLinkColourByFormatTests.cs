using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which hyperlink cells keep the file's own colour, which is a property of the importer.
/// </summary>
/// <remarks>
/// <para>
/// The companion of <see cref="SheetHyperlinkColourTests"/>, which pins the SpreadsheetML answer:
/// there a hyperlink cell is navy whatever the file states, and that stays true. What this adds is
/// that the same content read from BIFF or from ODF is <em>not</em>.
/// </para>
/// <para>
/// <c>ImpEditEngine::SeekCursor</c> applies the field's LINKS colour and then re-applies any hard
/// <c>EE_CHAR_COLOR</c> covering the field, under the comment <c>#i1550# hard color attrib should
/// win over text color from field</c> (<c>editeng/source/editeng/impedit3.cxx</c>:2947-2957). What
/// is hard differs by importer: <c>WorksheetGlobals::insertHyperlink</c> calls <c>rEE.Clear()</c>
/// and inserts a bare field, so SpreadsheetML leaves nothing; <c>lclInsertUrl</c> keeps the edit
/// object and, for a plain cell, applies the pattern through <c>ScPatternAttr::FillEditItemSet</c>,
/// which is hard unless the colour is <c>COL_AUTO</c>; and an ODF <c>text:span</c>'s own
/// <c>fo:color</c> is hard while a cell style's is an engine default.
/// </para>
/// <para>
/// <strong>Every expectation here is 26.2.4.2's own drawn colour</strong>, read out of its PDF of
/// these three files and reproduced on a second render (C11). The <c>.xls</c> is the reference's
/// own <c>--convert-to xls</c> of the <c>.xlsx</c>, which is why its cells state colours the
/// <c>.xlsx</c> does not: the OOXML import had already thrown the rich runs away and the BIFF
/// export baked the drawn colour into each cell's font. That is the finding restated, not a flaw
/// in the fixture. <c>probes/quantise-r120/</c> §5.
/// </para>
/// </remarks>
public sealed class SheetLinkColourByFormatTests
{
    private static readonly Colour Link = Colour.FromRgb(0x000080);

    /// <summary>
    /// The colour each labelled cell is drawn in, keyed on the label its text begins with.
    /// </summary>
    /// <remarks>
    /// Keyed on a prefix rather than on the whole string, because a rich cell is drawn as one run
    /// per portion here while the reference draws the whole hyperlink cell as one field — C16's
    /// count trap in miniature. Every run of a cell has to agree, which is what
    /// <see cref="Single"/> below asserts.
    /// </remarks>
    private static List<(string Text, Colour Colour)> Drawn(string fixture)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(fixture));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        return [.. sink.Pages[0].Runs
                    .Where(run => run.Paint is SolidPaint)
                    .Select(run => (run.Text, ((SolidPaint)run.Paint).Colour))];
    }

    /// <summary>The one colour every run of the labelled cell is drawn in.</summary>
    /// <remarks>
    /// A run belongs to the cell whose label contains its text. The fixture is authored so that no
    /// run's text is a substring of any other cell's label, which is what makes that sound.
    /// </remarks>
    private static Colour Single(List<(string Text, Colour Colour)> drawn, string label)
    {
        List<Colour> found =
            [.. drawn.Where(run => run.Text.Length > 0
                                   && label.Contains(run.Text, StringComparison.Ordinal))
                     .Select(run => run.Colour)];

        found.ShouldNotBeEmpty($"{label} should be drawn");
        found.Distinct().Count().ShouldBe(1, $"{label} should be drawn in one colour");
        return found[0];
    }

    /// <summary>SpreadsheetML: navy on every hyperlink cell, whatever the file states.</summary>
    /// <remarks>
    /// The two unlinked controls in the same colours are what stop "paint everything navy" from
    /// satisfying this.
    /// </remarks>
    [Fact]
    public void ASpreadsheetMLHyperlinkCellIsNavyWhateverColourItStates()
    {
        List<(string Text, Colour Colour)> drawn = Drawn("sheet-link-colour-ooxml.xlsx");

        Single(drawn, "plainauto").ShouldBe(Link);
        Single(drawn, "plainred").ShouldBe(Link);       // states #FF0000 and does not get it
        Single(drawn, "richgreen").ShouldBe(Link);      // a run states #00B050 and does not get it
        Single(drawn, "splitAsplitB").ShouldBe(Link);

        Single(drawn, "barered").ShouldBe(Colour.FromRgb(0xFF0000));
        Single(drawn, "bargreen").ShouldBe(Colour.FromRgb(0x00B050));
    }

    /// <summary>BIFF: the colour the cell states wins, because the importer makes it hard.</summary>
    [Fact]
    public void ABiffHyperlinkCellKeepsTheColourItStates()
    {
        List<(string Text, Colour Colour)> drawn = Drawn("sheet-link-colour-biff.xls");

        Single(drawn, "plainauto").ShouldBe(Colour.FromRgb(0x0000FF));
        Single(drawn, "plainred").ShouldBe(Colour.FromRgb(0xFF0000));
        Single(drawn, "richgreen").ShouldBe(Colour.FromRgb(0x0000FF));
        Single(drawn, "splitAsplitB").ShouldBe(Colour.FromRgb(0x0000FF));

        Single(drawn, "barered").ShouldBe(Colour.FromRgb(0xFF0000));
        Single(drawn, "bargreen").ShouldBe(Colour.FromRgb(0x00B050));

        drawn.Select(run => run.Colour)
             .ShouldNotContain(Link, "no cell of this file is drawn navy");
    }

    /// <summary>
    /// ODF: a <c>text:span</c>'s colour wins and a cell style's does not, which is the mechanism
    /// on its own.
    /// </summary>
    /// <remarks>
    /// The sharpest of the three, because the two hyperlink cells differ only in <em>where</em> the
    /// colour is stated and the reference draws them differently — so the rule cannot be "the file
    /// states a colour", it has to be "the colour is a character attribute".
    /// </remarks>
    [Fact]
    public void AnOdfHyperlinkCellKeepsASpansColourAndNotACellStylesColour()
    {
        List<(string Text, Colour Colour)> drawn = Drawn("sheet-link-colour-odf.fods");

        Single(drawn, "odsplainauto").ShouldBe(Link);
        Single(drawn, "odsplainred").ShouldBe(Link);                        // the CELL style states it
        Single(drawn, "odsrichgreen").ShouldBe(Colour.FromRgb(0x00B050));   // a SPAN states it

        // A span that states only a weight leaves the colour soft, so the red the CELL style
        // states still loses to the field. Without this the rule "the portion has a colour" would
        // pass, and every ODF portion has one.
        Single(drawn, "odsspannocolour").ShouldBe(Link);

        Single(drawn, "odsbarered").ShouldBe(Colour.FromRgb(0xFF0000));
        Single(drawn, "odsbargreen").ShouldBe(Colour.FromRgb(0x00B050));
    }
}
