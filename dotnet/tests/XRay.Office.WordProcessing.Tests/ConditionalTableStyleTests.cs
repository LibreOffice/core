using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table style's conditional formatting — <c>w:tblStylePr</c> — and the <c>w:tblLook</c> that
/// switches it on.
/// </summary>
/// <remarks>
/// <para>
/// The layer §17.7.2 puts between the document defaults and the paragraph style, and one this reader
/// had no notion of: <c>grep -rn "tblStylePr\|cnfStyle\|tblLook" dotnet/src</c> returned nothing at
/// all, so a style that makes its heading row bold produced a heading row that was not.
/// </para>
/// <para>
/// It is not a cosmetic loss. Bold is wider, so the header wraps onto more lines, so the row is
/// taller, so fewer body rows fit on the page. On
/// <c>airbus-pdf-information-package_v1-4.docx</c> the repeated header came out three lines against
/// the reference's four, which moved a row onto every page from the sixth.
/// </para>
/// <para>
/// The fixture is its own control. <c>table-style-first-row.docx</c> names a style whose
/// <c>firstRow</c> layer turns bold on and whose <c>lastRow</c> layer turns italic on, with a
/// <c>w:tblLook</c> asking for the first row and <em>not</em> the last — and its heading row's second
/// cell says <c>w:b w:val="0"</c> outright. LibreOffice 26.2.4.2's own PDF of it embeds
/// <c>LiberationSerif-Bold</c> and <c>LiberationSerif</c> and no italic face at any weight: the
/// heading's first cell is bold, its second is not, and nothing in the last row is italic.
/// </para>
/// </remarks>
public sealed class ConditionalTableStyleTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    /// <summary>The named attributes are read.</summary>
    [Fact]
    public void ANamedTableLookIsRead()
    {
        WordTableLook look = WordTableLook.Read(XElement.Parse(
            $"""<w:tblPr xmlns:w="{W}"><w:tblLook w:firstRow="1" w:lastRow="0" w:firstColumn="1" w:lastColumn="0"/></w:tblPr>"""));

        look.FirstRow.ShouldBeTrue();
        look.LastRow.ShouldBeFalse();
        look.FirstColumn.ShouldBeTrue();
        look.LastColumn.ShouldBeFalse();
    }

    /// <summary>
    /// And so is the 2007 spelling, which is a hexadecimal bitmask in <c>w:val</c> and nothing else.
    /// </summary>
    /// <remarks>
    /// <c>04A0</c> is what Word wrote for "first row and first column", and a reader that only knows
    /// the named attributes applies none of a style's conditional formatting to any document of that
    /// vintage while looking entirely correct on a newer one.
    /// </remarks>
    [Fact]
    public void TheHexadecimalTableLookIsReadToo()
    {
        WordTableLook look = WordTableLook.Read(XElement.Parse(
            $"""<w:tblPr xmlns:w="{W}"><w:tblLook w:val="04A0"/></w:tblPr>"""));

        look.FirstRow.ShouldBeTrue("0x0020");
        look.FirstColumn.ShouldBeTrue("0x0080");
        look.LastRow.ShouldBeFalse();
        look.LastColumn.ShouldBeFalse();
    }

    /// <summary>Where a file states both, the named attribute wins.</summary>
    /// <remarks>
    /// Word 2010 and later write both, and they can disagree — the real
    /// <c>airbus-pdf-information-package_v1-4.docx</c> writes <c>w:val="0420"</c> beside
    /// <c>w:firstRow="1"</c>, which happen to agree, and nothing guarantees that. The newer and
    /// unambiguous form is the one to believe.
    /// </remarks>
    [Fact]
    public void ANamedAttributeBeatsTheBitmask()
    {
        WordTableLook look = WordTableLook.Read(XElement.Parse(
            $"""<w:tblPr xmlns:w="{W}"><w:tblLook w:val="04A0" w:firstRow="0"/></w:tblPr>"""));

        look.FirstRow.ShouldBeFalse("the attribute contradicts the mask and is the newer spelling");
        look.FirstColumn.ShouldBeTrue("and the mask still supplies what the attributes do not");
    }

    /// <summary>A table stating no look at all asks for none of the conditional layers.</summary>
    [Fact]
    public void ATableWithNoLookAsksForNothing()
    {
        WordTableLook look = WordTableLook.Read(XElement.Parse($"""<w:tblPr xmlns:w="{W}"/>"""));

        look.ShouldBe(WordTableLook.None);
    }

    /// <summary>
    /// The layers a cell is in, most specific first: corner cells, then rows, then columns, then the
    /// unconditional formatting.
    /// </summary>
    /// <remarks>
    /// §17.7.6's application order reversed. It matters where two layers set the same property — a
    /// style whose <c>firstRow</c> is bold and whose <c>firstCol</c> is not must leave the corner cell
    /// bold, which is only true if <c>firstRow</c> is consulted first.
    /// </remarks>
    [Fact]
    public void TheLayersOfACellAreOrderedMostSpecificFirst()
    {
        WordTableStyleConditions corner = new(
            new WordTableLook(true, true, true, true),
            IsFirstRow: true, IsLastRow: false, IsFirstColumn: true, IsLastColumn: false);

        corner.Names.ShouldBe(["nwCell", "firstRow", "firstCol", "wholeTable"]);
    }

    /// <summary>A layer the table did not ask for is not offered, wherever the cell sits.</summary>
    [Fact]
    public void ALayerTheLookDoesNotAskForIsNotOffered()
    {
        WordTableStyleConditions cell = new(
            new WordTableLook(FirstRow: true, LastRow: false, FirstColumn: false, LastColumn: false),
            IsFirstRow: false, IsLastRow: true, IsFirstColumn: true, IsLastColumn: true);

        cell.Names.ShouldBe(["wholeTable"], "the look asks only for a first row and this is not one");
    }

    /// <summary>The style's conditional run properties are found for the cell that is in the layer.</summary>
    [Fact]
    public void TheFirstRowsRunPropertiesAreResolvedForAFirstRowCell()
    {
        WordStyles styles = Read(Style());

        styles.TableStyleRunProperties("Banded", InFirstRow).Count
            .ShouldBe(1, "the firstRow layer applies");
        styles.TableStyleRunProperties("Banded", InBody).ShouldBeEmpty("a body cell is in no layer");
    }

    /// <summary>
    /// And a layer whose bit the table cleared is not resolved even for the cell it names.
    /// </summary>
    [Fact]
    public void ALayerTheTableSwitchedOffIsNotResolved()
    {
        WordStyles styles = Read(Style());

        WordTableStyleConditions last = new(
            new WordTableLook(FirstRow: true, LastRow: false, FirstColumn: false, LastColumn: false),
            IsFirstRow: false, IsLastRow: true, IsFirstColumn: false, IsLastColumn: false);

        styles.TableStyleRunProperties("Banded", last).ShouldBeEmpty();
    }

    /// <summary>
    /// A style's own <c>w:rPr</c> is the unconditional layer and applies to every cell.
    /// </summary>
    [Fact]
    public void TheStylesOwnRunPropertiesApplyToEveryCell()
    {
        WordStyles styles = Read(Style(unconditional: "<w:rPr><w:i/></w:rPr>"));

        styles.TableStyleRunProperties("Banded", InBody).Count.ShouldBe(1);
        styles.TableStyleRunProperties("Banded", InFirstRow).Count
            .ShouldBe(2, "the conditional layer and the unconditional one, in that order");
    }

    /// <summary>
    /// The whole path, on the fixture LibreOffice's own rendering was read from.
    /// </summary>
    /// <remarks>
    /// Four assertions and each is a different rule: the heading row takes the style's bold; a run
    /// saying <c>w:b w:val="0"</c> beats it, because direct formatting is absolute; a body row takes
    /// nothing; and the last row takes nothing either, because the <c>w:tblLook</c> cleared that bit
    /// even though the style declares the layer.
    /// </remarks>
    [Fact]
    public void TheFixtureResolvesEveryLayerTheWayLibreOfficeDraws()
    {
        Dictionary<string, PageParagraph> cells = FixtureCells();

        FaceOf(cells["HEADA"]).Weight.ShouldBeGreaterThanOrEqualTo(700, "the firstRow layer is bold");
        FaceOf(cells["HEADB"]).Weight.ShouldBeLessThan(700, "its own w:b w:val=\"0\" wins");
        FaceOf(cells["BODYA"]).Weight.ShouldBeLessThan(700, "a body row is in no layer");
        FaceOf(cells["TAILA"]).IsItalic.ShouldBeFalse("the look cleared lastRow");
        FaceOf(cells["TAILA"]).Weight.ShouldBeLessThan(700);
    }

    /// <summary>
    /// The face the cell's text is actually drawn in.
    /// </summary>
    /// <remarks>
    /// A run whose formatting differs from the paragraph's is emitted as its own <see cref="PageRun"/>
    /// and the paragraph keeps the face an unstyled run would inherit — so reading
    /// <see cref="PageParagraph.Face"/> alone would miss exactly the cell this fixture uses as its
    /// control.
    /// </remarks>
    private static OpenTypeFace FaceOf(PageParagraph paragraph)
        => paragraph.Runs.Count > 0 ? paragraph.Runs[^1].Face : paragraph.Face;

    private static Dictionary<string, PageParagraph> FixtureCells()
    {
        using IDocument document = new WordProcessingReader().Read(
            DocumentSource.FromFile(Corpus.Require("table-style-first-row.docx")));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        Dictionary<string, PageParagraph> found = new(StringComparer.Ordinal);
        foreach (PlacedTableCell cell in pages.Pages[0].Tables.SelectMany(table => table.Cells))
        {
            foreach (PageBlock block in cell.Content?.Blocks ?? [])
            {
                if (block is PageParagraph paragraph && paragraph.Text.Length > 0)
                    found[paragraph.Text] = paragraph;
            }
        }

        found.Count.ShouldBe(6, "three rows of two cells");
        return found;
    }

    private static WordTableStyleConditions InFirstRow => new(
        new WordTableLook(FirstRow: true, LastRow: false, FirstColumn: false, LastColumn: false),
        IsFirstRow: true, IsLastRow: false, IsFirstColumn: false, IsLastColumn: false);

    private static WordTableStyleConditions InBody => new(
        new WordTableLook(FirstRow: true, LastRow: true, FirstColumn: true, LastColumn: true),
        IsFirstRow: false, IsLastRow: false, IsFirstColumn: false, IsLastColumn: false);

    private static string Style(string unconditional = "") => $"""
        <w:styles xmlns:w="{W}">
          <w:style w:type="table" w:styleId="Banded">
            <w:name w:val="Banded"/>
            {unconditional}
            <w:tblStylePr w:type="firstRow"><w:rPr><w:b/></w:rPr></w:tblStylePr>
            <w:tblStylePr w:type="lastRow"><w:rPr><w:i/></w:rPr></w:tblStylePr>
          </w:style>
        </w:styles>
        """;

    private static WordStyles Read(string stylesXml)
    {
        WordStyles styles = new();
        styles.Add(XElement.Parse(stylesXml));
        return styles;
    }
}
