using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// <c>w:rPr/w:w</c>: the character width, which squeezes a run's glyphs across.
/// </summary>
/// <remarks>
/// <para>
/// VCL applies it by setting the font's width away from its height —
/// <c>Font::SetAverageFontWidth</c>, reached from <c>SvxCharScaleWidthItem</c>, which
/// <c>DomainMapper</c> fills from <c>LN_EG_RPrBase_w</c> unchanged. It is neither tracking, which
/// puts a gap between glyphs and leaves them their own shape, nor a font size, which would change
/// the line's height as well.
/// </para>
/// <para>
/// Nothing read it. Censused over the corpus, <b>1677 <c>w:w</c> across 20 DOCX</b> — 237 of them
/// saying 100 and so meaning nothing, and <b>1226 of the remaining 1440 saying 99</b>. The documents
/// carrying it are the ones the project's own notes reach for when they discuss reflow.
/// </para>
/// <para>
/// The figures below are measured against 24.2.7.2 in
/// <c>dotnet/probes/words-character-scale/</c>: <c>Hamburgefonstiv 12345</c> at 12 pt in Liberation
/// Serif comes to 83.928 pt unscaled, and to 82.879 / 79.732 / 75.535 / 41.958 / 125.892 / 167.856 at
/// 99, 95, 90, 50, 150 and 200 per cent.
/// </para>
/// </remarks>
public sealed class CharacterScaleTests
{
    /// <summary>A width halves the run and doubles it, on the measurement.</summary>
    [Theory]
    [InlineData(50, 0.5)]
    [InlineData(90, 0.9)]
    [InlineData(95, 0.95)]
    [InlineData(150, 1.5)]
    [InlineData(200, 2.0)]
    public void AStatedWidthScalesTheRun(int percent, double expected)
    {
        Length bare = Width(Sentence, percent: null);
        Length scaled = Width(Sentence, percent: percent);

        (scaled.Emu / (double)bare.Emu).ShouldBe(expected, 0.001);
    }

    /// <summary>
    /// 99 per cent is not 0.99, because the width VCL builds the face at is a whole twip.
    /// </summary>
    /// <remarks>
    /// A 12 pt run is 240 twips tall and <c>trunc(240 x 99 / 100)</c> is 237, so the face is 237/240
    /// wide. The reference measures 82.879 pt against 83.928, which is 0.98750 to five places — and it
    /// is the corpus's commonest value by a factor of ten, so getting this one wrong would be getting
    /// nearly all of them wrong by a quarter of a per cent.
    /// </remarks>
    [Fact]
    public void NinetyNinePerCentLandsOnTheTwipBelowIt()
    {
        Length bare = Width(Sentence, percent: null);
        Length scaled = Width(Sentence, percent: 99);

        (scaled.Emu / (double)bare.Emu).ShouldBe(237.0 / 240.0, 0.0005);
    }

    /// <summary>100 and an absent width are the same paragraph.</summary>
    [Fact]
    public void AHundredPerCentIsNoScaleAtAll()
        => Width(Sentence, percent: 100).ShouldBe(Width(Sentence, percent: null));

    /// <summary>A width outside <c>ST_TextScale</c>'s range is ignored rather than obeyed.</summary>
    /// <remarks>Zero would collapse the run to nothing, which is worse than reading none.</remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(601)]
    public void AWidthOutsideTheRangeIsIgnored(int percent)
        => Width(Sentence, percent: percent).ShouldBe(Width(Sentence, percent: null));

    /// <summary>
    /// Tracking is added after the squeeze rather than squeezed with it.
    /// </summary>
    /// <remarks>
    /// Measured: the probe's run at 50 per cent with a 40-twip spacing comes to 69.900 pt against
    /// 41.958 unscaled and 111.804 tracked but unscaled — which is the squeezed text plus the whole of
    /// the tracking, not half of it.
    /// </remarks>
    [Fact]
    public void TrackingIsNotSqueezedWithTheGlyphs()
    {
        Length bare = Width(Sentence, percent: null);
        Length half = Width(Sentence, percent: 50);
        Length tracked = Width(Sentence, percent: null, spacing: 40);
        Length both = Width(Sentence, percent: 50, spacing: 40);

        long slack = Length.FromTwips(2).Emu;
        Math.Abs((both - half).Emu - (tracked - bare).Emu).ShouldBeLessThanOrEqualTo(slack);
    }

    /// <summary>
    /// The glyphs are drawn squeezed, not merely placed closer together.
    /// </summary>
    /// <remarks>
    /// The distinction is the whole of the drawing half: advancing the pen by half and leaving the
    /// glyphs their own width would overlap every one of them with the next. Both backends have the
    /// operator for it — a PDF text matrix's <c>a</c> term and Skia's <c>SKFont.ScaleX</c> — so the
    /// run carries the factor rather than the backend guessing it from the advances.
    /// </remarks>
    [Fact]
    public void TheDrawnRunCarriesTheSqueezeForTheBackend()
    {
        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r><w:rPr><w:sz w:val="24"/><w:w w:val="50"/></w:rPr>
                <w:t xml:space="preserve">{Sentence}</w:t></w:r>
            </w:p>
            """);

        PageRun run = paragraph.Runs.ShouldHaveSingleItem();

        run.WidthPerCent.ShouldBe(50);
        run.WidthScale.ShouldBe(0.5, 0.0001);
    }

    /// <summary>A scaled run inside an unscaled paragraph survives the uniform-paragraph shortcut.</summary>
    /// <remarks>
    /// The reader drops the run list for a paragraph whose runs all agree with the paragraph's own
    /// format, and a run that measures differently has to defeat that or it is measured at the
    /// paragraph's width. Tracking already did; this is the same duty and a sharper one, because a
    /// width multiplies the whole advance rather than adding to it.
    /// </remarks>
    [Fact]
    public void AScaledRunIsNotFoldedIntoAnUnscaledParagraph()
    {
        PageParagraph paragraph = Paragraph("""
            <w:p>
              <w:r><w:t xml:space="preserve">plain </w:t></w:r>
              <w:r><w:rPr><w:w w:val="50"/></w:rPr><w:t>squeezed</w:t></w:r>
            </w:p>
            """);

        paragraph.Runs.Count.ShouldBe(2);
        paragraph.Runs[1].WidthPerCent.ShouldBe(50);
    }

    /// <summary>The twip grid is the em size's, so the same percentage differs by size.</summary>
    /// <remarks>
    /// 99 per cent of a 12 pt em truncates to 237 of 240 twips and 99 per cent of a 10 pt em to 198 of
    /// 200 — 0.98750 against 0.99000. Both are the reference's, and a reader using the percentage
    /// itself would be wrong on the first and right on the second.
    /// </remarks>
    [Fact]
    public void TheGridIsTheRunsOwnEmSize()
    {
        TextWidthScale.Of(Length.FromPoints(12), 99).ShouldBe(237.0 / 240.0, 1e-9);
        TextWidthScale.Of(Length.FromPoints(10), 99).ShouldBe(198.0 / 200.0, 1e-9);
        TextWidthScale.Of(Length.FromPoints(12), 100).ShouldBe(1.0);
        TextWidthScale.Of(Length.Zero, 50).ShouldBe(0.5);
    }

    private const string Sentence = "Hamburgefonstiv 12345";

    private static Length Width(string sentence, int? percent, int? spacing = null)
    {
        // 24 half-points, so the em is 240 twips and the truncation the reference does is visible: at
        // the package's own default of 10 pt every percentage in this file divides 200 exactly and the
        // grid could not be told from the percentage.
        string properties = "<w:rPr><w:sz w:val=\"24\"/>"
            + (percent is { } w ? $"<w:w w:val=\"{w}\"/>" : string.Empty)
            + (spacing is { } s ? $"<w:spacing w:val=\"{s}\"/>" : string.Empty)
            + "</w:rPr>";

        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r>{properties}<w:t xml:space="preserve">{sentence}</w:t></w:r>
            </w:p>
            """);

        return paragraph.Measure().WidthBetween(0, paragraph.Text.Length);
    }

    private static PageParagraph Paragraph(string body)
    {
        using IDocument document = Open(body);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First(block => block.Text.Length > 0);
    }

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "character-scale.docx");
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
                {body}
                <w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr>
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
