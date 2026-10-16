using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// <c>style:font-name</c> and <c>fo:font-family</c> are two spellings of one item, so the level
/// decides before the spelling.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice imports <c>style:font-name</c> through
/// <c>XMLTextImportPropertyMapper::handleSpecialItem</c>'s <c>CTF_FONTNAME</c> branch, which hands
/// the <c>office:font-face-decls</c> entry to <c>XMLFontStylesContext::FillProperties</c> and
/// fills the <c>CTF_FONTFAMILYNAME</c> slot beside it
/// (<c>xmloff/source/text/txtimppr.cxx</c>:58-101); <c>fo:font-family</c> writes that same slot
/// directly. One slot, one item — so a child style stating either spelling shadows whatever its
/// parent stated, and resolving the two spellings independently through the whole parent chain
/// lets an outer <c>fo:font-family</c> beat an inner <c>style:font-name</c>.
/// </para>
/// <para>
/// That is not a corner: it is the shape LibreOffice writes for every workbook converted to ODF.
/// The named <c>Default</c> cell style carries both spellings and the automatic cell styles carry
/// <c>style:font-name</c> alone, so every cell in such a file resolved to the document default's
/// face. Censused over the converted corpus, <strong>290 of 307 <c>.ods</c> and 109 of 338
/// <c>.odt</c></strong> hold at least one style where the two spellings sit at different levels —
/// 16 687 and 2419 styles between them.
/// </para>
/// <para>
/// The fixture's cell asks for Arial and inherits Calibri. 26.2.4.2's own PDF of it embeds
/// <c>LiberationSans</c>; taking the inherited spelling gives Carlito, which is a different face
/// with different advances and therefore different line breaks and row heights.
/// </para>
/// </remarks>
public sealed class OdfFontNamePrecedenceTests
{
    /// <summary>The cell's own declaration beats the family its parent style states.</summary>
    [Fact]
    public void AnInnerFontNameBeatsAnOuterFontFamily()
    {
        using IPaginatedDocument document = (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require("odf-font-name-precedence.fods")));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        pages.Count.ShouldBe(1);

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        DrawnWord word = DrawnWords.On(sink.Pages[0]).Single(w => w.Text == "Hamburgefonstiv");

        // Arial is installed nowhere here and resolves to its metric-compatible substitute, which
        // is what 26.2.4.2 embeds too. Calibri would resolve to Carlito.
        word.Family.ShouldBe("Liberation Sans");
    }
}
