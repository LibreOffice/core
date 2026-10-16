using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A footnote cited from inside a table cell belongs at the foot of the page the cell was drawn on.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice makes no distinction at all between a note cited from body text and one cited from a cell:
/// the note hangs on the page frame the citing text frame sits in, whatever chain of cell and table frames
/// lies between them. X-Ray.Office read the note, numbered it and then dropped it — the paginator gathered
/// notes on the <c>PageParagraph</c> branch of the top-level flow only, and nothing walked the cells of a
/// placed table.
/// </para>
/// <para>
/// It is worth a structural test rather than only a corpus verdict because the symptom is silent: the page
/// count does not move, no diagnostic is raised, and the only trace is a page short by the note's words.
/// Three of the 200 documents on the words track cite a footnote from a table —
/// <c>TE.CAO.00125 … OJT Logbook</c>, <c>FO.FCTOA.00010 …</c> and
/// <c>EHEST-SMS-Safety-Management-Manual-V2</c> — and all three lost it.
/// </para>
/// <para>
/// The companion half is that the note takes room out of the page, which is what makes it a pagination
/// matter rather than a drawing one, and is asserted below through the separator: a page with a note area
/// has a rule above it, and a page with no notes has neither.
/// </para>
/// </remarks>
public sealed class TableCellFootnoteTests
{
    /// <summary>The note is placed at the foot of the page, from an anchor inside a cell.</summary>
    [Fact]
    public void ANoteCitedFromACellIsPlacedAtTheFootOfItsPage()
    {
        LaidOutPage page = FirstPage(inTable: true);

        PlacedFlow notes = page.Notes.ShouldNotBeNull(
            "a footnote cited from a table cell is still a footnote and belongs at the foot of the page");

        NoteText(notes).ShouldContain(
            "supervision of the training",
            Case.Sensitive,
            "the note's own body should be laid out in the page's note area");
    }

    /// <summary>The note area is charged for, which is what the rule above it says.</summary>
    /// <remarks>
    /// The separator is drawn only when there is something to separate, so its presence is the cheapest
    /// statement that pagination believes in the note rather than merely that something was drawn.
    /// </remarks>
    [Fact]
    public void APageCarryingACellsNoteReservesTheNoteArea()
    {
        FirstPage(inTable: true).NoteSeparator.ShouldNotBeNull(
            "a page with notes carries the rule above them");
    }

    /// <summary>
    /// The control: the same note cited from an ordinary paragraph, which already worked.
    /// </summary>
    /// <remarks>
    /// Here so that a regression in the note machinery as a whole cannot be mistaken for this defect, and
    /// so that the two cases are asserted to produce the same answer rather than merely both passing.
    /// </remarks>
    [Fact]
    public void TheSameNoteCitedFromBodyTextIsPlacedTheSameWay()
    {
        LaidOutPage page = FirstPage(inTable: false);

        NoteText(page.Notes.ShouldNotBeNull()).ShouldContain("supervision of the training", Case.Sensitive);
    }

    /// <summary>A document citing nothing has no note area and no rule.</summary>
    [Fact]
    public void APageCitingNoNoteHasNeitherNotesNorARule()
    {
        LaidOutPage page = FirstPage(inTable: true, cite: false);

        page.Notes.ShouldBeNull();
        page.NoteSeparator.ShouldBeNull();
    }

    /// <summary>The text a page's note area holds.</summary>
    private static string NoteText(PlacedFlow notes)
        => string.Join(" ", notes.Blocks.OfType<PageParagraph>().Select(paragraph => paragraph.Text));

    private static LaidOutPage FirstPage(bool inTable, bool cite = true)
    {
        using DocumentSource source =
            DocumentSource.FromStream(BuildPackage(inTable, cite), "table-cell-footnote.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBeGreaterThan(0);
        return pages.Pages[0];
    }

    private static MemoryStream BuildPackage(bool inTable, bool cite)
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
              <Override PartName="/word/footnotes.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"/>
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
              <Relationship Id="rId2" Target="footnotes.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes"/>
            </Relationships>
            """;

        // Without a settings part LibreOffice never applies its OOXML compatibility defaults, and a fixture
        // minimal enough to be obviously correct is often minimal enough to answer a different question.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        const string Footnotes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:footnotes xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:footnote w:type="separator" w:id="-1"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>
              <w:footnote w:id="1">
                <w:p>
                  <w:r><w:footnoteRef/></w:r>
                  <w:r><w:t xml:space="preserve"> the day-to-day supervision of the training is done by a
                            supervisor who is not the assessor.</w:t></w:r>
                </w:p>
              </w:footnote>
            </w:footnotes>
            """;

        string citation = cite
            ? """<w:r><w:footnoteReference w:id="1"/></w:r>"""
            : string.Empty;

        string content = inTable
            ? $"""
              <w:tbl>
                <w:tblPr><w:tblW w:w="8000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                <w:tblGrid><w:gridCol w:w="8000"/></w:tblGrid>
                <w:tr>
                  <w:tc>
                    <w:tcPr><w:tcW w:w="8000" w:type="dxa"/></w:tcPr>
                    <w:p><w:r><w:t>Supervisor Data</w:t></w:r>{citation}</w:p>
                  </w:tc>
                </w:tr>
              </w:tbl>
              """
            : $"""<w:p><w:r><w:t>Supervisor Data</w:t></w:r>{citation}</w:p>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {content}
                <w:p><w:r><w:t>after</w:t></w:r></w:p>
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="720" w:footer="720"/>
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
            Write(archive, "word/footnotes.xml", Footnotes);
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
