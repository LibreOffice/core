using System.Globalization;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A document asking for margin line numbers gets them, on its body lines and nowhere else.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:sectPr/w:lnNumType</c>, which Writer holds as one document-wide <c>SwLineNumberInfo</c> however
/// many sections state it. XRay.Office drew nothing at all: one corpus document,
/// <c>xx_SETIS_PWS_template_10.19.22.docx</c>, asks for numbering and was short by about 45 words on each
/// of its 15 pages — 548 in all, which is very nearly the whole of its word deficit.
/// </para>
/// <para>
/// The figures asserted here are the reference's, read out of its PDF's text operators on page 11 of that
/// document rather than from a raster: the numbers are 10 pt Liberation Serif, right-aligned with their
/// right edge at 57.95 pt against a text edge at 72.1 pt, which is the 0.5 cm
/// <c>SwLineNumberInfo</c> initialises <c>m_nPosFromLeft</c> to. The one-, two- and three-digit numbers sit
/// at three different left edges — 52.95, 47.95 and 42.95 — and one right one, which is what establishes
/// the alignment as right rather than left.
/// </para>
/// </remarks>
public sealed class LineNumberingTests
{
    /// <summary>Every body line is numbered, counting from one, when the document asks for every line.</summary>
    [Fact]
    public void EveryBodyLineIsNumberedWhenTheDocumentAsksForEveryLine()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1));

        page.LineNumbers.Count.ShouldBe(
            page.Lines.Count, "one number per body line, blank paragraphs included");

        page.LineNumbers.Select(mark => mark.Text)
            .ShouldBe([.. Enumerable.Range(1, page.Lines.Count).Select(n => n.ToString(CultureInfo.InvariantCulture))]);
    }

    /// <summary>A document that says nothing gets no numbers at all.</summary>
    /// <remarks>
    /// The control that matters most, because numbering is stated by exactly one document in two hundred:
    /// a reader that switched it on by default would draw a number beside every line of the corpus.
    /// </remarks>
    [Fact]
    public void ADocumentSayingNothingIsNotNumbered()
    {
        FirstPage(string.Empty).LineNumbers.ShouldBeEmpty();
    }

    /// <summary>
    /// The numbers are right-aligned, their right edge the stated distance in from the text edge.
    /// </summary>
    /// <remarks>
    /// The <em>right</em> edge is what is carried and what is fixed; a left-aligned reading would put the
    /// one- and two-digit numbers at the same x, which the reference does not.
    /// </remarks>
    [Fact]
    public void TheNumbersRightEdgeSitsTheStatedDistanceInFromTheText()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1, distanceTwips: 283));

        Length expected = page.BodyArea.X - Length.FromTwips(283);

        // Asserted non-empty first: `ShouldAllBe` is vacuously true of a page carrying no numbers, which
        // is exactly the state this test exists to rule out.
        page.LineNumbers.ShouldNotBeEmpty();
        page.LineNumbers.ShouldAllBe(mark => mark.RightBaseline.X == expected);
    }

    /// <summary>The distance defaults to Writer's 0.5 cm when the document states none.</summary>
    [Fact]
    public void AnUnstatedDistanceIsWritersHalfCentimetre()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1));

        page.LineNumbers[0].RightBaseline.X.ShouldBe(page.BodyArea.X - Length.FromTwips(283));
    }

    /// <summary>A number sits on its line's own baseline.</summary>
    [Fact]
    public void ANumberSitsOnTheBaselineOfTheLineItCounts()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1));

        // Before the loop, which a page with no numbers would run zero times.
        page.LineNumbers.ShouldNotBeEmpty();

        for (int i = 0; i < page.LineNumbers.Count; i++)
        {
            page.LineNumbers[i].RightBaseline.Y.ShouldBe(
                page.BodyArea.Y + page.Lines[i].Baseline, $"number {i + 1} is off its line's baseline");
        }
    }

    /// <summary>
    /// A count of five prints every fifth number and counts all of them.
    /// </summary>
    /// <remarks>
    /// Both halves: a reader that filtered the <em>counting</em> rather than the drawing would print 1, 2,
    /// 3 beside the fifth, tenth and fifteenth lines.
    /// </remarks>
    [Fact]
    public void ACountOfFivePrintsEveryFifthNumberAndCountsTheRest()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 5));

        page.LineNumbers.Select(mark => mark.Text).ShouldBe(
            [.. Enumerable.Range(1, page.Lines.Count).Where(n => n % 5 == 0).Select(n => n.ToString(CultureInfo.InvariantCulture))]);
    }

    /// <summary>A document can state the number its first line takes.</summary>
    [Fact]
    public void ADocumentCanStateTheNumberItStartsFrom()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1, start: 364));

        page.LineNumbers[0].Text.ShouldBe("364");
        page.LineNumbers[1].Text.ShouldBe("365");
    }

    /// <summary>
    /// A table's lines are neither numbered nor counted.
    /// </summary>
    /// <remarks>
    /// Measured on the reference rather than read out of a specification: on page 11 of the corpus
    /// document the numbers run to 384 on the line above a five-row table and resume at 385 on the first
    /// line below it, so the table charged the counter nothing.
    /// </remarks>
    [Fact]
    public void ATablesLinesAreNeitherNumberedNorCounted()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1), withTable: true);

        page.Tables.ShouldNotBeEmpty("the fixture is meant to contain a table");

        page.LineNumbers.Count.ShouldBe(page.Lines.Count);
        page.LineNumbers.Select(mark => mark.Text)
            .ShouldBe([.. Enumerable.Range(1, page.Lines.Count).Select(n => n.ToString(CultureInfo.InvariantCulture))]);
    }

    /// <summary>
    /// A paragraph asking to be skipped is neither numbered nor counted.
    /// </summary>
    /// <remarks>
    /// <c>w:suppressLineNumbers</c>, Writer's <c>SwFormatLineNumber::IsCount</c>. The counting half is the
    /// one a reader gets wrong: skipping only the drawing leaves a gap in the sequence where the reference
    /// has none.
    /// </remarks>
    [Fact]
    public void ASuppressedParagraphIsNeitherNumberedNorCounted()
    {
        LaidOutPage page = FirstPage(LineNumberType(countBy: 1), suppressSecondParagraph: true);

        page.LineNumbers.Count.ShouldBe(page.Lines.Count - 1);
        page.LineNumbers.Select(mark => mark.Text)
            .ShouldBe([.. Enumerable.Range(1, page.Lines.Count - 1).Select(n => n.ToString(CultureInfo.InvariantCulture))]);
    }

    private static string LineNumberType(int countBy, int start = 0, int distanceTwips = 0)
    {
        string startAttribute = start > 0 ? $""" w:start="{start}" """.Trim() : string.Empty;
        string distanceAttribute =
            distanceTwips > 0 ? $""" w:distance="{distanceTwips}" """.Trim() : string.Empty;

        return $"""<w:lnNumType w:countBy="{countBy}" w:restart="continuous" {startAttribute} {distanceAttribute}/>""";
    }

    private static LaidOutPage FirstPage(
        string lineNumbering, bool withTable = false, bool suppressSecondParagraph = false)
    {
        using DocumentSource source = DocumentSource.FromStream(
            BuildPackage(lineNumbering, withTable, suppressSecondParagraph), "line-numbering.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBeGreaterThan(0);
        return pages.Pages[0];
    }

    private static MemoryStream BuildPackage(
        string lineNumbering, bool withTable, bool suppressSecondParagraph)
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

        // Twelve short paragraphs, each one line, so that "one number per line" and "every fifth number"
        // are both statements about a sequence long enough to have a shape.
        string suppressed = suppressSecondParagraph
            ? "<w:pPr><w:suppressLineNumbers/></w:pPr>"
            : string.Empty;

        string paragraphs = string.Join(
            "\n",
            Enumerable.Range(1, 12).Select(i =>
                $"<w:p>{(i == 2 ? suppressed : string.Empty)}<w:r><w:t>Line {i} alpha bravo.</w:t></w:r></w:p>"));

        string table = withTable
            ? """
              <w:tbl>
                <w:tblPr><w:tblW w:w="8000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                <w:tblGrid><w:gridCol w:w="8000"/></w:tblGrid>
                <w:tr><w:tc><w:tcPr><w:tcW w:w="8000" w:type="dxa"/></w:tcPr>
                  <w:p><w:r><w:t>cell one</w:t></w:r></w:p>
                  <w:p><w:r><w:t>cell two</w:t></w:r></w:p>
                </w:tc></w:tr>
              </w:tbl>
              """
            : string.Empty;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {paragraphs}
                {table}
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="720" w:footer="720"/>
                  {lineNumbering}
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
