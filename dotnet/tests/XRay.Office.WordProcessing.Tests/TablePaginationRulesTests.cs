using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Three rules a table obeys that a paragraph does not, each found on a corpus document whose page
/// count was one out.
/// </summary>
/// <remarks>
/// <para>
/// They are kept together because they share a cause: the reader and the paginator both grew up around
/// paragraphs, and a table reaches each of these decisions by a different path or by no path at all.
/// </para>
/// <list type="number">
/// <item>
/// A <c>w:trHeight</c> floor sits <em>under</em> the row's borders rather than over them, so a row
/// resting on its floor is one border taller than the floor says. Measured by sweeping the border width
/// against a fixed floor — <c>dotnet/probes/words-pagination-01/row-min-height-border.py</c> reads
/// 24.00 / 24.50 / 25.00 / 26.00 / 27.00 pt out of LibreOffice 26.2.4.2 for <c>w:sz</c> 0 / 4 / 8 / 16 /
/// 24 under a 24 pt floor. That the answer tracks the border exactly is what rules out the reading the
/// two corpus documents cannot distinguish it from: both the FAA Holdover Tables and
/// <c>ESPN-R - MCF - Manual</c> draw a <c>w:sz="4"</c> grid, so a flat half point fits them just as well.
/// </item>
/// <item>
/// <c>w:cantSplit</c> is overridden for a row taller than a whole page, because such a row has nowhere
/// to go and honouring the flag hides its content. <c>SwTabFrame::Split</c>,
/// <c>sw/source/core/layout/tabfrm.cxx</c>:1161.
/// </item>
/// <item>
/// A <c>&lt;w:br w:type="page"/&gt;</c> in the paragraph before a table starts the table on a new page.
/// DOCX has no break-before on a table, so the break has to be carried forward; it used to be eaten by
/// the first paragraph inside the table's first cell, where it does nothing.
/// </item>
/// </list>
/// </remarks>
public sealed class TablePaginationRulesTests
{
    /// <summary>A row resting on its <c>w:trHeight</c> floor is that floor plus one border tall.</summary>
    /// <remarks>
    /// The <c>0</c> row is the control that keeps this from being read as a constant: with no border at
    /// all the row is exactly its floor. The rest are the measured sweep.
    /// </remarks>
    [Theory]
    [InlineData(0, 24.00)]
    [InlineData(4, 24.50)]
    [InlineData(8, 25.00)]
    [InlineData(16, 26.00)]
    [InlineData(24, 27.00)]
    public void ARowRestingOnItsFloorIsOneBorderTallerThanTheFloor(int borderEighths, double expected)
    {
        RowPitch(borderEighths, "atLeast").ShouldBe(expected, 0.01);
    }

    /// <summary>
    /// An <c>exact</c> height is the whole of the row, borders included, and gains nothing.
    /// </summary>
    /// <remarks>
    /// The other branch, and measured the other way round: at <c>w:sz="16"</c> — a 2 pt border, four
    /// times the gap this whole rule is about — LibreOffice still draws the rows 24.00 pt apart.
    /// Applying the border here too would be the obvious symmetry and is wrong.
    /// </remarks>
    [Fact]
    public void AnExactHeightCarriesNoBorder()
    {
        RowPitch(16, "exact").ShouldBe(24.00, 0.01);
    }

    /// <summary>
    /// <c>w:hRule="auto"</c> is a floor like any other, not an absence of one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer honours exactly one of <c>w:hRule</c>'s three words. <c>MeasureHandler</c> opens at
    /// <c>SizeType::MIN</c> and its <c>LN_CT_Height_hRule</c> case tests only for <c>exact</c>
    /// (<c>sw/source/writerfilter/dmapper/MeasureHandler.cxx</c>:35, 70-76), so <c>auto</c> never
    /// reaches the layout and the stated <c>w:val</c> stands exactly as <c>atLeast</c>'s does.
    /// </para>
    /// <para>
    /// Reading it as "no height at all" was this reader's own invention, and both reference versions
    /// refute it at once — six rows at <c>w:val="480" w:hRule="auto"</c> come out 480 twips apart under
    /// 24.2.7.2 and 489.6 to 740.4 under 26.2.4.2, matching each binary's own <c>atLeast</c> figures to
    /// the twip, while we drew them 241.2 apart. See <c>probes/words-row-height/pitch.py</c>.
    /// </para>
    /// <para>
    /// It has <b>no corpus reach</b>: 11 230 <c>w:trHeight</c> elements across every DOCX in
    /// <c>sample-files</c> and not one of them states <c>auto</c>. Asserted as an identity with
    /// <c>atLeast</c> rather than against a figure, so that the two can never drift apart.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(4)]
    [InlineData(8)]
    [InlineData(24)]
    public void AnAutoHeightIsTheSameFloorAsAtLeast(int borderEighths)
    {
        RowPitch(borderEighths, "auto").ShouldBe(RowPitch(borderEighths, "atLeast"), 0.001);
    }

    /// <summary>
    /// A <c>w:cantSplit</c> row that would not fit on a page of its own is split anyway.
    /// </summary>
    /// <remarks>
    /// Found on <c>ESPN-R - MCF - RA - Ed1.docx</c>, whose "Engine - Flight" row is about 440 pt tall
    /// under a 424 pt landscape body: the reference splits it across two pages, and we moved the whole
    /// row on and then drew it past the bottom of the paper — as far as y = 597.0 on a 595.30 pt page.
    /// </remarks>
    [Fact]
    public void ARowTallerThanThePageSplitsDespiteCantSplit()
    {
        // 90 lines is around 1030 pt against a 648 pt body, so the row cannot fit whichever page it
        // starts on.
        List<LaidOutPage> pages = Paginate(TallRowDocument(lines: 90, cantSplit: true));

        // Two pages is not the assertion, because an unsplit row overflows onto a second page too and
        // would pass it. What says the row *split* is that its one row is drawn on both of them: an
        // overflowing row is one rectangle hanging off the foot of page one, and page two then holds
        // only the paragraph after the table.
        pages.Count.ShouldBe(2);
        pages[0].Tables.Count.ShouldBe(1);
        pages[1].Tables.Count.ShouldBe(1);

        // And nothing hangs off the bottom of the first page, which is the visible half of the defect.
        pages[0].Tables[0].Cells.Max(cell => cell.Area.Bottom)
            .ShouldBeLessThanOrEqualTo(pages[0].BodyArea.Bottom);
    }

    /// <summary>
    /// A <c>w:cantSplit</c> row that <em>would</em> fit on a page of its own still moves whole.
    /// </summary>
    /// <remarks>
    /// The control that stops the override being read as "cantSplit is ignored". Twenty lines is about
    /// 230 pt against a 648 pt body, so the row fits on a page of its own comfortably; the 45 filler
    /// paragraphs in front of it are what stop it fitting where it is. It must arrive on page two whole
    /// rather than being cut at the foot of page one, which is what `pages[0].Tables` being empty says.
    /// </remarks>
    [Fact]
    public void ARowThatFitsOnAPageOfItsOwnStillHonoursCantSplit()
    {
        List<LaidOutPage> pages = Paginate(TallRowDocument(lines: 20, cantSplit: true, filler: 45));

        pages.Count.ShouldBe(2);
        pages[0].Tables.ShouldBeEmpty();
    }

    /// <summary>A page break in the paragraph before a table starts the table on a new page.</summary>
    [Fact]
    public void APageBreakBeforeATableStartsANewPage()
    {
        List<LaidOutPage> pages = Paginate(BreakBeforeTableDocument(breaks: true));

        pages.Count.ShouldBe(2);
        pages[0].Tables.ShouldBeEmpty();
        pages[1].Tables.Count.ShouldBe(1);
    }

    /// <summary>Without the break the same table stays on page one.</summary>
    /// <remarks>
    /// The control. Everything else about the two fixtures is identical, so a failure here is the
    /// paginator having gained a break rather than kept one.
    /// </remarks>
    [Fact]
    public void WithoutTheBreakTheTableStaysWhereItWas()
    {
        List<LaidOutPage> pages = Paginate(BreakBeforeTableDocument(breaks: false));

        pages.Count.ShouldBe(1);
        pages[0].Tables.Count.ShouldBe(1);
    }

    /// <summary>The distance between two consecutive rows' tops, in points.</summary>
    private static double RowPitch(int borderEighths, string rule)
    {
        using IDocument document = Open(RowHeightDocument(borderEighths, rule));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PlacedTable table = pages.Pages[0].Tables.Single();

        // Row tops read off the placed cells rather than off the row heights, so this measures what is
        // drawn — the same quantity the probe reads out of the two PDFs.
        List<double> tops = [.. table.Cells
            .GroupBy(cell => cell.Row)
            .OrderBy(group => group.Key)
            .Select(group => group.Min(cell => cell.Area.Top.Points))];

        return tops[2] - tops[1];
    }

    private static List<LaidOutPage> Paginate(string body)
    {
        using IDocument document = Open(body);
        return [.. ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages];
    }

    private const int BorderEighths = 4;

    private static string RowHeightDocument(int borderEighths, string rule)
    {
        string borders = borderEighths == 0
            ? Borders("none", 0)
            : Borders("single", borderEighths);

        string rows = string.Concat(Enumerable.Range(0, 5).Select(i => $"""
              <w:tr>
                <w:trPr><w:trHeight w:val="480" w:hRule="{rule}"/></w:trPr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="5000" w:type="dxa"/>
                    <w:tcMar><w:top w:w="0" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/></w:tcMar>
                  </w:tcPr>
                  <w:p>
                    <w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr>
                    <w:r><w:rPr><w:sz w:val="20"/></w:rPr><w:t>R{i}</w:t></w:r>
                  </w:p>
                </w:tc>
              </w:tr>
            """));

        return $"""
              <w:tbl>
                <w:tblPr><w:tblW w:w="5000" w:type="dxa"/><w:tblLayout w:type="fixed"/>{borders}
                  <w:tblCellMar><w:top w:w="0" w:type="dxa"/><w:bottom w:w="0" w:type="dxa"/></w:tblCellMar>
                </w:tblPr>
                <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                {rows}
              </w:tbl>
            """;
    }

    private static string TallRowDocument(int lines, bool cantSplit, int filler = 0)
    {
        string before = string.Concat(Enumerable.Repeat(
            "<w:p><w:r><w:t>filler</w:t></w:r></w:p>", filler));
        string content = string.Concat(Enumerable.Range(0, lines).Select(
            i => $"<w:p><w:r><w:t>line {i}</w:t></w:r></w:p>"));

        return $"""
            {before}
              <w:tbl>
                <w:tblPr><w:tblW w:w="5000" w:type="dxa"/><w:tblLayout w:type="fixed"/>
                  {Borders("single", BorderEighths)}
                </w:tblPr>
                <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                <w:tr>
                  <w:trPr>{(cantSplit ? "<w:cantSplit/>" : string.Empty)}</w:trPr>
                  <w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>{content}</w:tc>
                </w:tr>
              </w:tbl>
            """;
    }

    private static string BreakBeforeTableDocument(bool breaks)
    {
        string mark = breaks ? """<w:r><w:br w:type="page"/></w:r>""" : string.Empty;

        return $"""
              <w:p><w:r><w:t>before</w:t></w:r></w:p>
              <w:p>{mark}</w:p>
              <w:tbl>
                <w:tblPr><w:tblW w:w="5000" w:type="dxa"/><w:tblLayout w:type="fixed"/>
                  {Borders("single", BorderEighths)}
                </w:tblPr>
                <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                <w:tr><w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                  <w:p><w:r><w:t>in the table</w:t></w:r></w:p>
                </w:tc></w:tr>
              </w:tbl>
            """;
    }

    private static string Borders(string style, int eighths)
        => $"""
            <w:tblBorders>
              <w:top w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
              <w:left w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
              <w:bottom w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
              <w:right w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
              <w:insideH w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
              <w:insideV w:val="{style}" w:sz="{eighths}" w:space="0" w:color="000000"/>
            </w:tblBorders>
            """;

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "table-pagination.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string body)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
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
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        // An American-letter page with one-inch margins and no running head, so the body is exactly
        // nine inches and a row's fitting or not fitting is arithmetic rather than a measurement.
        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {body}
                <w:p><w:r><w:t>after</w:t></w:r></w:p>
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="0" w:footer="0" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
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
