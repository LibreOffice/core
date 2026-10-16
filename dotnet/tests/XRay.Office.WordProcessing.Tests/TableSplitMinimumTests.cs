using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table is split across a page only when a minimum number of its rows fits; otherwise it moves whole,
/// and a keep-with-next paragraph in front of it moves with it.
/// </summary>
/// <remarks>
/// <para>
/// Writer counts that minimum in <c>SwTabFrame::MakeAll</c>
/// (<c>sw/source/core/layout/tabfrm.cxx</c>:3061-3092): it starts at <c>GetRowsToRepeat()</c> — the
/// <c>w:tblHeader</c> count — and, when <c>TABLE_ROW_KEEP</c> is set, grows over the run of rows whose
/// first cell begins with a keep-with-next paragraph. Only if that many rows fit is
/// <c>SwTabFrame::Split</c> called at all.
/// </para>
/// <para>
/// <strong>Measured on <c>AC-150-5370-10G-updated-201604.docx</c>, which renders 693 pages against 696
/// without this and 696 with it.</strong> Its `Requirements for Gradation of Mixture` table has two
/// <c>w:tblHeader</c> rows and a direct <c>w:keepNext</c> on the first-cell paragraph of every row but
/// the last, so the minimum is 2 + 7 = 9 of 10 rows; those cannot fit the ~94 pt left, so LibreOffice
/// moves the table whole and ends the page at y=583.0 where we split it after two data rows.
/// <c>probes/caption-keepnext-table/reproduce.py</c> holds the thirteen-block reproducer, which now
/// agrees with the reference on all eleven of its cases to 0.1 pt.
/// </para>
/// <para>
/// The compatibility split is the same one the other three flags on <see cref="PaginationOptions"/>
/// follow: the DOCX filter sets <c>TABLE_ROW_KEEP</c>
/// (<c>sw/source/writerfilter/dmapper/SettingsTable.cxx</c>:677), the DOC filter sets it
/// (<c>ww8par.cxx</c>:2039), and a native ODF document leaves it false.
/// </para>
/// </remarks>
public sealed class TableSplitMinimumTests
{
    /// <summary>Enough filler to leave about 95 pt — room for some rows, not for nine.</summary>
    private const int Filler = 40;

    /// <summary>
    /// A table whose rows each keep with the next is moved whole rather than split.
    /// </summary>
    [Fact]
    public void ATableWhoseRowsKeepWithTheNextIsMovedWholeRatherThanSplit()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(Document(rows: 10, keeps: true)).Pages;

        pages.Count.ShouldBeGreaterThanOrEqualTo(2);
        RowsOn(pages[0]).ShouldBe(0, "no part of the table may start on the page it cannot finish");
        RowsOn(pages[1]).ShouldBe(10);
    }

    /// <summary>
    /// The control, and the reason this is a minimum rather than a refusal to split at all: the same
    /// table without those keeps is split exactly as before.
    /// </summary>
    [Fact]
    public void TheSameTableWithoutThoseKeepsIsStillSplit()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(Document(rows: 10, keeps: false)).Pages;

        pages.Count.ShouldBeGreaterThanOrEqualTo(2);
        RowsOn(pages[0]).ShouldBeGreaterThan(0);
        pages.Sum(RowsOn).ShouldBe(10);
    }

    /// <summary>
    /// Repeated headings set the minimum on their own, with no keep-with-next anywhere.
    /// </summary>
    /// <remarks>
    /// The <c>nRepeat</c> term, isolated from the keep chain — eight headings do not fit the room left,
    /// so the table moves whole where <see cref="TheSameTableWithoutThoseKeepsIsStillSplit"/>, the same
    /// ten rows with no headings and no keeps, is split.
    /// </remarks>
    [Fact]
    public void RepeatedHeadingsSetTheMinimumOnTheirOwn()
    {
        IReadOnlyList<LaidOutPage> pages =
            Paginate(Document(rows: 10, keeps: false, headings: 8)).Pages;

        pages.Count.ShouldBeGreaterThanOrEqualTo(2);
        RowsOn(pages[0]).ShouldBe(0);
    }

    /// <summary>
    /// A keep-with-next paragraph in front of a table that places no row here goes with it.
    /// </summary>
    /// <remarks>
    /// Writer's keep asks only that the successor share the page, and a table that starts no row here has
    /// not started. This is the half worth 18.7 pt on the reproducer — the caption's own line.
    /// </remarks>
    [Fact]
    public void AKeepWithNextParagraphInFrontOfATableMovesWithIt()
    {
        WordProcessingPages pages = Paginate(Document(rows: 10, keeps: true, caption: true));

        TextOn(pages, 0).ShouldNotContain("CAPTION");
        TextOn(pages, 1).ShouldContain("CAPTION");
    }

    /// <summary>
    /// And the control: without the keep it stays behind, at the foot of the page it fits on.
    /// </summary>
    /// <remarks>
    /// Without this the test above would pass against a paginator that moved every paragraph in front of
    /// a table, which is a different and much broader rule.
    /// </remarks>
    [Fact]
    public void AParagraphWithoutTheKeepStaysWhereItFits()
    {
        WordProcessingPages pages = Paginate(
            Document(rows: 10, keeps: true, caption: true, captionKeeps: false));

        TextOn(pages, 0).ShouldContain("CAPTION");
    }

    /// <summary>
    /// A table whose <em>last</em> row keeps with the next moves to join the paragraph below it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer states this on the following frame rather than on the table: <c>SwFrameNotify</c>
    /// invalidates the previous frame's position when "pPre is a table and the last row wants to keep
    /// with me" (<c>sw/source/core/layout/frmtool.cxx</c>:167-176), and <c>SwTabFrame::MakeAll</c>'s rule
    /// 7 — "The last table row wants to keep with its next" — says it from the table's side
    /// (<c>tabfrm.cxx</c>:2831-2845). Both are gated on the same <c>TABLE_ROW_KEEP</c>.
    /// </para>
    /// <para>
    /// Measured on <c>150-5370-10H.docx</c>, which renders 725 pages against 727 with only the minimum
    /// rule and 726 with this one. Its `Coarse Aggregate Material Requirements` table carries
    /// <c>w:keepNext</c> on the first cell of all eight rows and is followed by three note paragraphs
    /// that are themselves a keep chain: we moved the notes and left the table, and every row of that
    /// table lands at the same height in both renderings, so it is placement and not measurement.
    /// </para>
    /// </remarks>
    /// <remarks>
    /// Swept over the filler rather than pinned at one length, because the exact row height of a
    /// synthetic decides which side of the page boundary the pair falls on, and a single length either
    /// tests the rule or tests nothing depending on that. The invariant holds at every length: the table
    /// and the paragraph it keeps with are never on different pages.
    /// </remarks>
    [Theory]
    [InlineData(36)]
    [InlineData(38)]
    [InlineData(40)]
    [InlineData(42)]
    [InlineData(44)]
    public void ATableWhoseLastRowKeepsIsNeverSeparatedFromTheParagraphBelowIt(int filler)
    {
        WordProcessingPages pages = Paginate(
            Document(rows: 5, keeps: true, keepsLast: true, after: 3, filler: filler));

        PageOfTable(pages).ShouldBe(PageOfText(pages, "after 0"));
    }

    /// <summary>
    /// And the sweep separates them when the last row does not keep, so the rule above is not vacuous.
    /// </summary>
    /// <remarks>
    /// Without this, a paginator that never split the two at any filler length — because they always
    /// fitted together — would pass the theory above at every one of its five lengths.
    /// </remarks>
    [Fact]
    public void WithoutThatKeepTheSweepDoesSeparateThem()
    {
        int[] fillers = [36, 38, 40, 42, 44];

        bool anySeparated = fillers.Any(filler =>
        {
            WordProcessingPages pages = Paginate(
                Document(rows: 5, keeps: true, keepsLast: false, after: 3, filler: filler));
            return PageOfTable(pages) != PageOfText(pages, "after 0");
        });

        anySeparated.ShouldBeTrue(
            "the sweep must cross the boundary, or the theory above proves nothing");
    }

    /// <summary>The flag follows the format, exactly as LibreOffice's own setting does.</summary>
    [Fact]
    public void OnlyTheWordPresetKeepsTableRowsWithTheNext()
    {
        PaginationOptions.Word.KeepsTableRowsWithNext.ShouldBeTrue();
        PaginationOptions.Default.KeepsTableRowsWithNext.ShouldBeFalse();
    }

    /// <summary>Which page the table's first row landed on.</summary>
    private static int PageOfTable(WordProcessingPages pages)
    {
        for (int page = 0; page < pages.Pages.Count; page++)
        {
            if (RowsOn(pages.Pages[page]) > 0) return page;
        }

        return -1;
    }

    /// <summary>Which page a marker string landed on.</summary>
    private static int PageOfText(WordProcessingPages pages, string marker)
    {
        for (int page = 0; page < pages.Pages.Count; page++)
        {
            if (TextOn(pages, page).Contains(marker, StringComparison.Ordinal)) return page;
        }

        return -1;
    }

    /// <summary>How many of the table's own rows landed on a page.</summary>
    /// <remarks>
    /// Repeated headings redrawn on a continuation are deliberately not counted — <c>FirstRow</c> and
    /// <c>RowEnd</c> are where the table resumed, which is the question here.
    /// </remarks>
    private static int RowsOn(LaidOutPage page)
        => page.Tables.Sum(table => Math.Max(0, table.RowEnd - table.FirstRow));

    /// <summary>
    /// The text of one page, which has to come from the page set rather than the line: a
    /// <see cref="PlacedLine"/> carries glyph runs and an index, and <c>WordProcessingPages.TextOf</c>
    /// is what turns those back into characters.
    /// </summary>
    private static string TextOn(WordProcessingPages pages, int page)
        => string.Concat(pages.Pages[page].Lines.Select(pages.TextOf));

    private static WordProcessingPages Paginate(string body)
    {
        using IDocument document = Open(body);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>
    /// Filler down the page, then an optional caption, then a table of <paramref name="rows"/> rows.
    /// </summary>
    /// <param name="rows">How many rows the table has, headings included.</param>
    /// <param name="keeps">
    /// Whether each row but the last carries <c>w:keepNext</c> on its first cell's paragraph, which is
    /// the one paragraph <c>ShouldRowKeepWithNext</c> reads.
    /// </param>
    /// <param name="headings">How many leading rows are <c>w:tblHeader</c>.</param>
    /// <param name="caption">Whether a paragraph precedes the table.</param>
    /// <param name="captionKeeps">Whether that paragraph carries <c>w:keepNext</c>.</param>
    /// <param name="keepsLast">
    /// Whether the <em>last</em> row keeps too, which is a different rule from the rest of them: it binds
    /// the table to what follows rather than holding the table together.
    /// </param>
    /// <param name="after">How many lines of paragraph follow the table.</param>
    /// <param name="filler">
    /// How many filler lines precede it, overriding <see cref="Filler"/>. Swept by the keep tests, whose
    /// answer otherwise depends on where one synthetic's row height happens to fall.
    /// </param>
    private static string Document(
        int rows,
        bool keeps,
        int headings = 0,
        bool caption = false,
        bool captionKeeps = true,
        bool keepsLast = false,
        int after = 0,
        int? filler = null)
    {
        string lines = string.Concat(Enumerable.Range(0, filler ?? Filler).Select(
            i => $"<w:p><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>filler {i}</w:t></w:r></w:p>"));

        string head = caption
            ? $"""
               <w:p><w:pPr>{(captionKeeps ? "<w:keepNext/>" : string.Empty)}</w:pPr>
                 <w:r><w:rPr><w:sz w:val="24"/></w:rPr><w:t>CAPTION</w:t></w:r></w:p>
               """
            : string.Empty;

        string body = string.Concat(Enumerable.Range(0, rows).Select(i => $"""
              <w:tr>
                <w:trPr>{(i < headings ? "<w:tblHeader/>" : string.Empty)}</w:trPr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                  <w:p><w:pPr>{(keeps && (keepsLast || i < rows - 1) ? "<w:keepNext/>" : string.Empty)}</w:pPr>
                    <w:r><w:rPr><w:sz w:val="24"/></w:rPr><w:t>row {i}</w:t></w:r></w:p>
                </w:tc>
              </w:tr>
            """));

        string tail = string.Concat(Enumerable.Range(0, after).Select(
            i => $"<w:p><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>after {i}</w:t></w:r></w:p>"));

        return $"""
            {lines}{head}
              <w:tbl>
                <w:tblPr><w:tblW w:w="5000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                {body}
              </w:tbl>
            {tail}
            """;
    }

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "table-split.docx");
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

        // The settings part is not optional even though nothing here reads it: without it LibreOffice —
        // and this reader, which follows it — never applies the OOXML compatibility defaults, and the
        // document lays out under Writer's instead. See PaginationOptions.KeepsSpacingAtTopOfPage.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {body}
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream package = new();

        using (ZipArchive archive = new(package, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/settings.xml", Settings);
            Add(archive, "word/document.xml", document);
        }

        package.Position = 0;
        return package;
    }

    private static void Add(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        byte[] bytes = Encoding.UTF8.GetBytes(content);
        entry.Write(bytes, 0, bytes.Length);
    }
}
