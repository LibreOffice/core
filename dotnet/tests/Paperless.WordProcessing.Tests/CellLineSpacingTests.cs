using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A table cell grows by the proportional line spacing on its last paragraph.
/// </summary>
/// <remarks>
/// <para>
/// Writer's <c>AddParaLineSpacingToTableCells</c>, switched on for every document the DOCX, DOC and RTF
/// importers open (<c>sw/source/writerfilter/filter/WriterFilter.cxx</c>:314) and off for a native ODF
/// one. <c>SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell</c>
/// (<c>sw/source/core/layout/flowfrm.cxx</c>:1946) adds it to the cell alongside the space-after this
/// engine already charged, and the amount is <c>SwBorderAttrs::CalcLineSpacing_</c>
/// (<c>sw/source/core/layout/frmtool.cxx</c>:2681): the paragraph's <em>font size</em> in twips times
/// <c>(prop − 100) × 1.15 / 100</c>, truncated.
/// </para>
/// <para>
/// The numbers below are LibreOffice's own, measured on a one-cell fixture rendered by
/// LibreOffice 24.2.7.2 at 110, 115, 125, 150, 200 and 250 per cent — 27, 41, 68, 137, 275 and 414 twips
/// for a twelve-point paragraph. Three of those are one twip below the decimal arithmetic because
/// <c>1.15</c> is a shade under 1.15 in binary and the product is truncated, which is why the expected
/// values are stated rather than computed.
/// </para>
/// <para>
/// Without it every row of a 115%-spaced table is 2.05 pt short, which on the corpus's
/// <c>slcc-architecture-uu-architecture.docx</c> was a page.
/// </para>
/// </remarks>
public sealed class CellLineSpacingTests
{
    [Theory]
    [InlineData(240, 0)]
    [InlineData(264, 27)]
    [InlineData(276, 41)]
    [InlineData(300, 68)]
    [InlineData(360, 137)]
    [InlineData(480, 275)]
    [InlineData(600, 414)]
    public void ARowGrowsByLibreOfficesOwnLineSpacingAllowance(int line, int twips)
    {
        PageTable table = TableOf(line);

        List<Length> without = TableLayouter.LayOut(
            table, DocPoint.Origin, addsCellLineSpacing: false).RowHeights;

        List<Length> with = TableLayouter.LayOut(
            table, DocPoint.Origin, addsCellLineSpacing: true).RowHeights;

        (with[0] - without[0]).ShouldBe(Length.FromTwips(twips));
    }

    /// <summary>
    /// The allowance is charged once per cell rather than once per paragraph in it.
    /// </summary>
    /// <remarks>
    /// <c>CalcAddLowerSpaceAsLastInTableCell</c> is reached only from <c>CalcLowerSpace</c>'s
    /// <c>!m_rThis.GetIndNext()</c> branch, so it is the cell's last flow frame that carries it. Charging
    /// every paragraph would make a three-paragraph cell three times too generous.
    /// </remarks>
    [Fact]
    public void OnlyTheCellsLastParagraphCarriesIt()
    {
        PageTable table = TableOf(276, paragraphs: 3);

        Length without = TableLayouter.LayOut(
            table, DocPoint.Origin, addsCellLineSpacing: false).RowHeights[0];

        Length with = TableLayouter.LayOut(
            table, DocPoint.Origin, addsCellLineSpacing: true).RowHeights[0];

        (with - without).ShouldBe(Length.FromTwips(41));
    }

    /// <summary>The two presets differ on it, which is the whole of how a document chooses.</summary>
    [Fact]
    public void OnlyTheWordPresetAsksForIt()
    {
        PaginationOptions.Word.AddsCellLineSpacing.ShouldBeTrue();
        PaginationOptions.Default.AddsCellLineSpacing.ShouldBeFalse();
    }

    private static PageTable TableOf(int line, int paragraphs = 1)
    {
        using IDocument document = Open(line, paragraphs);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().Single();
    }

    private static IDocument Open(int line, int paragraphs)
    {
        MemoryStream package = BuildPackage(line, paragraphs);
        using DocumentSource source = DocumentSource.FromStream(package, "cell-line-spacing.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(int line, int paragraphs)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        // Twelve points, stated in the document defaults, because `SwBorderAttrs::CalcLineSpacing_`
        // reads `RES_CHRATR_FONTSIZE` off the *paragraph's* attribute set — not off a run and not off
        // the paragraph mark, neither of which reaches that set. The fixture used to state it on the
        // run and on the mark and carry no styles part at all, which made it a twelve-point paragraph
        // only by accident: LibreOffice's own default for a styles-less document is twelve points and
        // this reader's is ten, so the two sides agreed on the number for different reasons and the
        // expectations below would have been measuring the default rather than the rule.
        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault>
                  <w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="24"/></w:rPr>
                </w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
            </w:styles>
            """;

        string cell = string.Concat(Enumerable.Range(1, paragraphs).Select(index => $"""
            <w:p>
              <w:pPr>
                <w:spacing w:after="0" w:line="{line}" w:lineRule="auto"/>
              </w:pPr>
              <w:r>
                <w:t>Row {index}</w:t>
              </w:r>
            </w:p>
            """));

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>
                  <w:tr><w:tc>{cell}</w:tc></w:tr>
                </w:tbl>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
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
