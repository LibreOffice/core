using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What a <c>w:cols</c> with no <c>w:space</c> means by the gap between its columns.
/// </summary>
/// <remarks>
/// <para>
/// 1.25 cm, which <c>SectionPropertyMap</c> states as <c>m_nColumnDistance( 1249 )</c> — hundredths of a
/// millimetre — and only overwrites where the attribute is present
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:424). The WW8 side says the same figure in
/// twips: <c>ReadUSprm(pSep, pIds[4], 708)</c>, commented "default distance 1.25 cm".
/// </para>
/// <para>
/// It is the column <em>width</em> that makes the default matter rather than the gutter: the width is the
/// measure less the gaps, divided by the count, so reading the silence as zero widens every column by
/// half a gap and breaks every line of the section late.
/// </para>
/// <para>
/// Every multi-column section in the sample corpus's DOCX states the attribute, so this is asserted
/// against a hand-built package rather than against a corpus measurement — and said so plainly rather
/// than borrowing the WW8 side's evidence, which is a different reader.
/// </para>
/// </remarks>
public sealed class DocxColumnGapTests
{
    /// <summary>Letter, less an inch of margin each side, is the measure the columns divide.</summary>
    private static readonly Length Measure = Length.FromTwips(12240 - 1440 - 1440);

    /// <summary>1.25 cm.</summary>
    private static readonly Length DefaultGap = Length.FromTwips(708);

    [Fact]
    public void AColumnsElementWithNoSpaceTakesAnInchAndAQuarterCentimetre()
    {
        LaidOutPage page = Paginate(space: null);

        page.ColumnCount.ShouldBe(2);
        page.ColumnGap.ShouldBe(DefaultGap);
        page.ColumnArea(0).Width.ShouldBe((Measure - DefaultGap) / 2);
    }

    [Fact]
    public void AStatedSpaceStillWins()
    {
        LaidOutPage page = Paginate(space: 360);

        page.ColumnGap.ShouldBe(Length.FromTwips(360));
        page.ColumnArea(0).Width.ShouldBe((Measure - Length.FromTwips(360)) / 2);
    }

    /// <summary>A stated zero is a statement, and is honoured as one.</summary>
    /// <remarks>
    /// The distinction the default turns on: silence is 1.25 cm and <c>w:space="0"</c> is nothing. A
    /// reader that treated the attribute's *value* of zero as absent would give this document a gutter it
    /// asked not to have.
    /// </remarks>
    [Fact]
    public void AStatedZeroIsHonoured()
    {
        Paginate(space: 0).ColumnGap.ShouldBe(Length.Zero);
    }

    private static LaidOutPage Paginate(int? space)
    {
        MemoryStream package = BuildPackage(space);
        using DocumentSource source = DocumentSource.FromStream(package, "columns.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages[0];
    }

    private static MemoryStream BuildPackage(int? space)
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

        // Carried deliberately: without a settings part a hand-built DOCX misses LibreOffice's OOXML
        // compatibility defaults, and several synthetics built without one have given clean, consistent,
        // wrong answers.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        string cols = space is { } stated
            ? $"""<w:cols w:num="2" w:space="{stated}"/>"""
            : """<w:cols w:num="2"/>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:r><w:t>Two columns of text on one page.</w:t></w:r></w:p>
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="720" w:footer="720" w:gutter="0"/>
                  {cols}
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
