using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A row whose cells all hide their end mark and hold nothing is exactly <c>w:trHeight</c> tall, not at
/// least that tall.
/// </summary>
/// <remarks>
/// <para>
/// <c>DomainMapperTableHandler::endTableGetRowProperties</c> — "we have CellHideMark on all cells, and
/// also all cells are empty: force the row height to be exactly as specified, and not just as the minimum
/// suggestion" (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:1157-1162). This is
/// what a graph-paper grid is made of: 48 rows of 36 cells declaring 180 twips each, well under the
/// 13.7 pt an 11 pt Calibri line asks for.
/// </para>
/// <para>
/// <strong>What counts as empty depends on the compatibility mode</strong>, and that is a second
/// LibreOffice rule feeding this one: below mode 15 a table cell's paragraph has its trailing spaces,
/// tabs and no-break spaces trimmed (tdf#77417,
/// <c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:3032-3045), so a cell holding one no-break
/// space <em>is</em> empty in a Word 2010 file and is not in a Word 2013 one. Both references agree on
/// all 24 cells of that matrix — see <c>probes/words-hidemark-rowheight/</c>.
/// </para>
/// </remarks>
public sealed class HiddenCellMarkRowHeightTests
{
    /// <summary>Ten empty hidden-mark rows of 180 twips are 9 pt apart, not 13.7.</summary>
    [Fact]
    public void EmptyHiddenMarkRowsAreExactlyTheDeclaredHeight() =>
        RowHeight(hideMark: true, text: null, mode: 15).ShouldBe(9.0, 0.05);

    /// <summary>Without the hidden mark the same rows keep their floor and grow to the text.</summary>
    [Fact]
    public void WithoutTheHiddenMarkTheHeightIsStillAFloor() =>
        RowHeight(hideMark: false, text: null, mode: 15).ShouldBeGreaterThan(13.0);

    /// <summary>A cell holding real text is not empty, hidden mark or not.</summary>
    [Fact]
    public void ACellHoldingTextIsNotEmpty() =>
        RowHeight(hideMark: true, text: "x", mode: 15).ShouldBeGreaterThan(13.0);

    /// <summary>Below mode 15 a cell holding one no-break space is empty, so the row is exact.</summary>
    [Fact]
    public void ANoBreakSpaceIsEmptyBelowCompatibilityFifteen() =>
        RowHeight(hideMark: true, text: "\u00a0", mode: 14).ShouldBe(9.0, 0.05);

    /// <summary>At mode 15 the same no-break space is content, so the row keeps its floor.</summary>
    /// <remarks>
    /// The pair is the whole point: <c>084_Printable_Graph_Paper_Template_Editable_Layout</c> declares
    /// mode 14 and draws 9 pt rows, and the identical table in a mode 15 file draws 13.9 pt ones.
    /// </remarks>
    [Fact]
    public void ANoBreakSpaceIsContentAtCompatibilityFifteen() =>
        RowHeight(hideMark: true, text: "\u00a0", mode: 15).ShouldBeGreaterThan(13.0);

    /// <summary>A file stating no compatibility mode is treated as the modern one.</summary>
    /// <remarks>
    /// <c>0 &lt; nMode &amp;&amp; nMode &lt;= 14</c> — the lower bound matters, and both references
    /// confirm it: the probe's <c>f-None-hide-nbsp</c> draws 13.92 pt where <c>f-14-hide-nbsp</c> draws
    /// 9.12.
    /// </remarks>
    [Fact]
    public void ANoBreakSpaceIsContentWhenNoModeIsStated() =>
        RowHeight(hideMark: true, text: "\u00a0", mode: null).ShouldBeGreaterThan(13.0);

    /// <summary>A vertically merged cell keeps the row on its floor, however empty it is.</summary>
    /// <remarks>
    /// <c>lcl_hideMarks</c>: "if anything is vertically merged, the row must not be set to fixed as
    /// Writer's layout doesn't handle that well".
    /// </remarks>
    [Fact]
    public void AVerticallyMergedCellKeepsTheFloor() =>
        RowHeight(hideMark: true, text: null, mode: 15, merged: true).ShouldBeGreaterThan(13.0);

    /// <summary>A row that declares no height is left alone: an exact nothing draws nothing.</summary>
    [Fact]
    public void ARowWithNoDeclaredHeightIsUntouched() =>
        RowHeight(hideMark: true, text: null, mode: 15, height: null).ShouldBeGreaterThan(13.0);

    /// <summary>The pitch of the ten rows of the fixture's table, in points.</summary>
    private static double RowHeight(
        bool hideMark, string? text, int? mode, bool merged = false, int? height = 180)
    {
        string mark = hideMark ? "<w:hideMark/>" : string.Empty;
        string run = text is null
            ? string.Empty
            : $"<w:r><w:t xml:space=\"preserve\">{text}</w:t></w:r>";

        string Cell(int index) =>
            "<w:tc><w:tcPr><w:tcW w:w=\"1200\" w:type=\"dxa\"/>"
            + (merged && index == 1 ? "<w:vMerge w:val=\"restart\"/>" : string.Empty)
            + mark + "</w:tcPr>"
            + "<w:p><w:pPr><w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>"
            + run + "</w:p></w:tc>";

        string rowProperties = height is null
            ? string.Empty
            : $"<w:trPr><w:trHeight w:val=\"{height}\"/></w:trPr>";

        StringBuilder rows = new();
        for (int row = 0; row < 10; row++)
        {
            rows.Append("<w:tr>").Append(rowProperties);
            for (int column = 0; column < 3; column++) rows.Append(Cell(column));
            rows.Append("</w:tr>");
        }

        string body =
            "<w:tbl><w:tblPr><w:tblW w:w=\"3600\" w:type=\"dxa\"/></w:tblPr>"
            + "<w:tblGrid><w:gridCol w:w=\"1200\"/><w:gridCol w:w=\"1200\"/>"
            + "<w:gridCol w:w=\"1200\"/></w:tblGrid>"
            + rows
            + "</w:tbl><w:p/>";

        using DocumentSource source =
            DocumentSource.FromStream(Package(body, mode), "hidemark.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PlacedTable table = pages.Pages[0].Tables.ShouldHaveSingleItem();
        return table.Area.Height.Points / 10.0;
    }

    private static MemoryStream Package(string body, int? mode)
    {
        const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

        string compat = mode is null
            ? string.Empty
            : "<w:compat><w:compatSetting w:name=\"compatibilityMode\" "
              + $"w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"{mode}\"/></w:compat>";

        // A hand-built DOCX with no `word/settings.xml` gets different importer defaults, so the part is
        // always written even when it states nothing.
        string settings = $"<w:settings xmlns:w=\"{W}\">{compat}</w:settings>";

        string document =
            $"<w:document xmlns:w=\"{W}\"><w:body>{body}"
            + "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
            + "<w:pgMar w:top=\"1134\" w:right=\"1134\" w:bottom=\"1134\" w:left=\"1134\" "
            + "w:header=\"709\" w:footer=\"709\" w:gutter=\"0\"/></w:sectPr></w:body></w:document>";

        const string ContentTypes =
            "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
            + "<Default Extension=\"rels\" "
            + "ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
            + "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
            + "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd."
            + "openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
            + "<Override PartName=\"/word/settings.xml\" ContentType=\"application/vnd."
            + "openxmlformats-officedocument.wordprocessingml.settings+xml\"/>"
            + "<Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd."
            + "openxmlformats-officedocument.wordprocessingml.styles+xml\"/></Types>";

        const string RootRelationships =
            "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            + "<Relationship Id=\"rId1\" Target=\"word/document.xml\" Type=\"http://schemas."
            + "openxmlformats.org/officeDocument/2006/relationships/officeDocument\"/></Relationships>";

        const string DocumentRelationships =
            "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            + "<Relationship Id=\"rId1\" Target=\"settings.xml\" Type=\"http://schemas."
            + "openxmlformats.org/officeDocument/2006/relationships/settings\"/>"
            + "<Relationship Id=\"rId2\" Target=\"styles.xml\" Type=\"http://schemas."
            + "openxmlformats.org/officeDocument/2006/relationships/styles\"/></Relationships>";

        string styles =
            $"<w:styles xmlns:w=\"{W}\"><w:docDefaults><w:rPrDefault><w:rPr>"
            + "<w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\"/><w:sz w:val=\"22\"/>"
            + "</w:rPr></w:rPrDefault><w:pPrDefault><w:pPr>"
            + "<w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/>"
            + "</w:pPr></w:pPrDefault></w:docDefaults></w:styles>";

        MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            void Write(string path, string content)
            {
                using StreamWriter writer = new(archive.CreateEntry(path).Open(), Encoding.UTF8);
                writer.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
                writer.Write(content);
            }

            Write("[Content_Types].xml", ContentTypes);
            Write("_rels/.rels", RootRelationships);
            Write("word/_rels/document.xml.rels", DocumentRelationships);
            Write("word/settings.xml", settings);
            Write("word/styles.xml", styles);
            Write("word/document.xml", document);
        }

        stream.Position = 0;
        return stream;
    }
}
