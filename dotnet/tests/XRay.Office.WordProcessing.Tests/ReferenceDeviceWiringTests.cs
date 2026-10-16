using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Every Writer document is measured on a reference device; one compatibility flag chooses which.
/// </summary>
/// <remarks>
/// <para>
/// The arithmetic is <see cref="MetricGrid"/>'s and is proved against LibreOffice's own PDFs in
/// <c>XRay.Office.Text.Tests.ReferenceGridTests</c>. This file is about the wiring, which is the half
/// that was wrong: the grid existed, it was correct, and it was handed to the layout only for the
/// handful of documents setting <c>w:usePrinterMetrics</c>. Everything else was scaled exactly, which
/// is a thing Writer never does — <c>DocumentDeviceManager::CreateVirtualDevice_</c> always makes a
/// device, and <c>getReferenceDevice</c> always returns one.
/// </para>
/// <para>
/// Asserted through a read package rather than on a constructed <c>PageParagraph</c>, because a
/// constructed one inherits the default and would pass however the readers behave.
/// </para>
/// </remarks>
public sealed class ReferenceDeviceWiringTests
{
    [Fact]
    public void ADocxIsLaidOutOnTheVirtualReferenceDevice()
    {
        // The device, which is what this file is about: 8640 dpi, advances unquantised.
        Paragraphs(printerMetrics: false).ShouldAllBe(
            paragraph => paragraph.Metrics == MetricGrid.Reference.AsWordDocument());
    }

    [Fact]
    public void ADocxAlsoCarriesWordsEastAsianLineScale()
    {
        // `AsWordDocument` is the document's `MS_WORD_COMP_GRID_METRICS` compatibility flag rather
        // than a property of the device — it travels beside the resolution because
        // `lcl_ApplyCjkHeightAdjustment` is asked both questions at once. It is off by default and
        // the Word filters turn it on, which is measurable: the same two lines of WenQuanYi Zen Hei
        // at 12 pt are 406 twips apart out of a .docx and 325 out of a .fodt. See
        // `EastAsianLineScaleTests`.
        Paragraphs(printerMetrics: false)
            .ShouldAllBe(paragraph => paragraph.Metrics!.Value.ScalesEastAsianFaces);

        // And the device underneath it is unchanged.
        Paragraphs(printerMetrics: false).ShouldAllBe(paragraph => paragraph.Metrics!.Value.Dpi == 6 * 1440);
    }

    [Fact]
    public void ADocxAskingForPrinterMetricsIsLaidOutOnThePrinter()
    {
        // The flag still means what it meant. `w:usePrinterMetrics` becomes
        // PrinterIndependentLayout::DISABLED in the writerfilter and getReferenceDevice hands out an
        // SfxPrinter instead — a coarser grid, not the absence of one. The compatibility flag is
        // independent of which device that is, so it comes along.
        Paragraphs(printerMetrics: true).ShouldAllBe(
            paragraph => paragraph.Metrics == MetricGrid.Printer.AsWordDocument());
    }

    [Fact]
    public void ATenPointLiberationSerifLineIsElevenPointFiveFiveTall()
    {
        // The single measurement this whole round turned on. LibreOffice 26.2.4.2 draws consecutive
        // baselines of a 10 pt Liberation Serif paragraph 11.55 pt apart — 231 twips; scaling the
        // face's 2355 design units gives 229.98, which rounds to 230, and the one twip between them
        // is four line gaps and a descent on `words/done-015/docx/Sample_SQMS_Program.docx`, whose
        // page 59 it decides.
        PageParagraph paragraph = Paragraphs(printerMetrics: false)[0];
        MeasuredParagraph measured = paragraph.Measure();

        measured.MeasureLine(0, paragraph.Text.Length).Height.Twips.ShouldBe(231);
    }

    private static List<PageParagraph> Paragraphs(bool printerMetrics)
    {
        using IDocument document = ReadDocx(printerMetrics);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Blocks.OfType<PageParagraph>()];
    }

    private static IDocument ReadDocx(bool printerMetrics)
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

        string settings = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>{(printerMetrics ? "<w:usePrinterMetrics/>" : string.Empty)}</w:compat>
            </w:settings>
            """;

        // Liberation Serif at 10 pt, named outright so the fixture cannot pass by substitution: it is
        // the one (face, size) pair whose exactly-scaled height and LibreOffice's differ.
        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr><w:rPr>
                    <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                    <w:sz w:val="20"/>
                  </w:rPr></w:pPr>
                  <w:r>
                    <w:rPr>
                      <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                      <w:sz w:val="20"/>
                    </w:rPr>
                    <w:t>Hxg</w:t>
                  </w:r>
                </w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream package = new();
        using (ZipArchive archive = new(package, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", settings);
            Write(archive, "word/document.xml", Document);
        }

        package.Position = 0;
        using DocumentSource source = DocumentSource.FromStream(package, "reference-device.docx");
        return new WordProcessingReader().Read(source);

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
