using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table whose width is a percentage of the area it sits in — <c>w:tblW w:type="pct"</c>.
/// </summary>
/// <remarks>
/// <para>
/// The unit is fiftieths of a percent, so <c>5000</c> is 100%, and <c>DomainMapperTableManager::sprm</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapperTableManager.cxx</c>:191) turns it into
/// <c>SizeType::VARIABLE</c> with the percentage clamped to 100. Writer then restates the declared grid
/// in the same proportions at that width, so a grid summing to less than the text width is *stretched*
/// rather than taken literally.
/// </para>
/// <para>
/// Reading the percentage as no width at all — which is what a reader that only understands <c>dxa</c>
/// does — leaves the table as wide as its grid adds up to. On
/// <c>ESPN-R - MCF - RA - Ed1.docx</c> that drew the running header 481.65 pt wide where Writer draws it
/// 714.35 pt, breaking <c>Page 26/58</c> across two lines in a cell that holds it on one; and it left
/// the severity table on page 25 about 1.9% too wide, so <c>PERSONNEL</c> fitted on one line where the
/// reference wraps it after <c>PERSONNE</c>. Both agree with the reference to a tenth of a point once
/// the percentage is honoured.
/// </para>
/// </remarks>
public sealed class TableRelativeWidthTests
{
    /// <summary>The text width of the page these fixtures declare: 12240 twips less two 1440 margins.</summary>
    private const int TextWidth = 9360;

    /// <summary>A grid of 1000 + 2000 + 1000, which is well under the text width.</summary>
    private const int GridSum = 4000;

    /// <summary>A table at 100% fills the text width, its columns keeping the grid's proportions.</summary>
    [Fact]
    public void AFullWidthPercentageStretchesTheGridToTheTextWidth()
    {
        PageTable table = Table("5000", "pct");

        table.RelativeWidth.ShouldBe(100);

        IReadOnlyList<Length> widths = table.WidthsWithin(Length.FromTwips(TextWidth));

        widths.Sum(width => width.Twips).ShouldBe(TextWidth, "the table is the whole text width");
        widths[0].Twips.ShouldBe(1000L * TextWidth / GridSum);
        widths[1].Twips.ShouldBe(2000L * TextWidth / GridSum);
        widths[2].Twips.ShouldBe(TextWidth - widths[0].Twips - widths[1].Twips);
    }

    /// <summary>Half of the area is half of the area, and the proportions are unchanged.</summary>
    [Fact]
    public void HalfIsHalfOfTheAreaRatherThanHalfOfTheGrid()
    {
        PageTable table = Table("2500", "pct");

        table.RelativeWidth.ShouldBe(50);
        table.WidthWithin(Length.FromTwips(TextWidth)).Twips.ShouldBe(TextWidth / 2);
    }

    /// <summary>
    /// A percentage above 100 is laid out at 100, which is a clamp rather than an overflow.
    /// </summary>
    /// <remarks>
    /// <c>if (nPercent &gt; 100) nPercent = 100;</c>. The corpus document that found this states
    /// <c>w:w="5112"</c> — 102.24% — and the reference draws it at exactly the text width, not 2% past it.
    /// </remarks>
    [Fact]
    public void APercentageAboveOneHundredIsClampedToIt()
    {
        Table("5112", "pct").RelativeWidth.ShouldBe(100);
        Table("7500", "pct").RelativeWidth.ShouldBe(100);

        Table("5112", "pct").WidthWithin(Length.FromTwips(TextWidth)).Twips.ShouldBe(TextWidth);
    }

    /// <summary>A width written as a literal percentage is read as the percentage it says.</summary>
    /// <remarks>
    /// <c>ST_MeasurementOrPercent</c> allows it, and a file using it means 50% rather than 1%.
    /// </remarks>
    [Fact]
    public void ALiteralPercentSignIsReadAsThePercentage()
        => Table("50%", "pct").RelativeWidth.ShouldBe(50);

    /// <summary>A zero percentage is no width at all, not a table of no width.</summary>
    [Fact]
    public void AZeroPercentageIsNoWidth()
    {
        PageTable table = Table("0", "pct");

        table.RelativeWidth.ShouldBeNull();
        table.WidthWithin(Length.FromTwips(TextWidth)).Twips.ShouldBe(GridSum);
    }

    /// <summary>An absolute width is untouched: the grid is the table and the area decides nothing.</summary>
    [Fact]
    public void AnAbsoluteWidthIsUnchangedByTheAreaAroundIt()
    {
        PageTable table = Table("4000", "dxa");

        table.RelativeWidth.ShouldBeNull();
        table.WidthWithin(Length.FromTwips(TextWidth)).Twips.ShouldBe(GridSum);
        table.WidthsWithin(Length.FromTwips(TextWidth))
            .Select(width => width.Twips)
            .ShouldBe([1000L, 2000L, 1000L]);
    }

    /// <summary>
    /// The cells are laid out at the stretched widths, which is the half that reflows the text.
    /// </summary>
    /// <remarks>
    /// The column list could be right and the layout still measure the cells against the declared grid,
    /// and it is the cell rectangle that decides where a line breaks.
    /// </remarks>
    [Fact]
    public void TheCellsAreLaidOutAtTheStretchedWidths()
    {
        PageTable table = Table("5000", "pct");

        (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(
            table,
            new Core.Geometry.DocPoint(Length.Zero, Length.Zero),
            available: Length.FromTwips(TextWidth));

        cells.Sum(cell => cell.Area.Width.Twips).ShouldBe(TextWidth);
        cells[1].Area.Width.Twips.ShouldBe(2000L * TextWidth / GridSum);
    }

    private static PageTable Table(string width, string type)
    {
        using IDocument document = Open(width, type);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().Single();
    }

    private static IDocument Open(string width, string type)
    {
        MemoryStream package = BuildPackage(width, type);
        using DocumentSource source = DocumentSource.FromStream(package, "relative-width.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string width, string type)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblPr><w:tblW w:w="{width}" w:type="{type}"/></w:tblPr>
                  <w:tblGrid>
                    <w:gridCol w:w="1000"/><w:gridCol w:w="2000"/><w:gridCol w:w="1000"/>
                  </w:tblGrid>
                  <w:tr>
                    <w:tc><w:p><w:r><w:t>a</w:t></w:r></w:p></w:tc>
                    <w:tc><w:p><w:r><w:t>b</w:t></w:r></w:p></w:tc>
                    <w:tc><w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc>
                  </w:tr>
                </w:tbl>
                <w:p/>
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="720" w:footer="720" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
