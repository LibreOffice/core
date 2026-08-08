using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A paragraph's <c>w:pBdr</c> draws a rule and takes the room the rule needs.
/// </summary>
/// <remarks>
/// <para>
/// Every figure below is LibreOffice 24.2.7.2's own, read off the PDF it produced for the same markup:
/// Liberation Sans at 11 pt on US Letter with 72 pt margins puts three plain paragraphs' baselines at
/// 709.70, 697.05 and 684.40, a plain pitch of 12.65 pt, and each border side moves the paragraph past
/// it by exactly <c>w:sz/8 + w:space</c>.
/// </para>
/// <para>
/// The measurements here are made in <em>pitch</em> rather than in absolute position, because the
/// absolute figure also carries the first line's ascent and the top margin and would fail for reasons
/// that have nothing to do with borders.
/// </para>
/// </remarks>
public sealed class ParagraphBorderTests
{
    /// <summary>
    /// The allowance a side takes is its width plus its distance, and nothing else.
    /// </summary>
    /// <remarks>
    /// <c>w:sz</c> is in eighths of a point and <c>w:space</c> in whole points — the one place in
    /// WordprocessingML that unit appears on a border, and reading it as twips puts a 10 pt distance at
    /// half a point. The four rows are four of the sixteen probes the rule was measured on.
    /// </remarks>
    [Theory]
    [InlineData(18, 1, 3.25)]
    [InlineData(18, 0, 2.25)]
    [InlineData(4, 1, 1.50)]
    [InlineData(24, 10, 13.00)]
    public void ATopBorderPushesTheParagraphDownByItsWidthPlusItsDistance(int sz, int space, double points)
    {
        IReadOnlyList<Length> plain = Baselines(Borderless);
        IReadOnlyList<Length> bordered = Baselines(Bordered("top", sz, space));

        // The gap from the paragraph above, which is the only one a top border changes.
        Length before = plain[1] - plain[0];
        Length after = bordered[1] - bordered[0];

        (after - before).ShouldBe(Length.FromPoints(points));

        // And nothing below it moves any further: the paragraph after keeps its plain pitch.
        (bordered[2] - bordered[1]).ShouldBe(plain[2] - plain[1]);
    }

    /// <summary>The same, on the other side.</summary>
    [Fact]
    public void ABottomBorderPushesTheParagraphAfterItDown()
    {
        IReadOnlyList<Length> plain = Baselines(Borderless);
        IReadOnlyList<Length> bordered = Baselines(Bordered("bottom", 18, 1));

        (bordered[1] - bordered[0]).ShouldBe(plain[1] - plain[0]);
        ((bordered[2] - bordered[1]) - (plain[2] - plain[1])).ShouldBe(Length.FromPoints(3.25));
    }

    /// <summary>
    /// The allowance adds to <c>w:spacing</c> rather than collapsing against the paragraph above.
    /// </summary>
    /// <remarks>
    /// The reason the allowance is carried on <see cref="PageParagraph.BorderAbove"/> instead of being
    /// folded into <c>ParagraphFormat.SpaceBefore</c>. Folded in, it would be compared against the
    /// previous paragraph's space-after wherever <c>PaginationOptions.CollapsesSpacing</c> is on — which
    /// is every DOCX that does not state <c>w:doNotUseHTMLParagraphAutoSpacing</c> — and, being smaller,
    /// would vanish. Measured: 12.65 + 12 + 3.25 = 27.90 pt on both sides of the bordered paragraph.
    /// </remarks>
    [Fact]
    public void TheAllowanceSurvivesSpacingCollapse()
    {
        IReadOnlyList<Length> spaced = Baselines(Bordered("top", 18, 1, spacingTwips: 240));

        (spaced[1] - spaced[0]).ShouldBe(Length.FromPoints(12.65 + 12 + 3.25));
    }

    /// <summary>
    /// A <c>w:pBdr</c> is inherited side by side, so a paragraph stating one side keeps its style's other.
    /// </summary>
    /// <remarks>
    /// Measured against LibreOffice on a probe whose style states a 2.25 pt bottom rule and whose
    /// paragraph states only a 0.75 pt top: the paragraph moves down by 1.75 pt and the one after it by a
    /// further 3.25, which is both sides applying. Taking the innermost <c>w:pBdr</c> whole instead loses
    /// the style's side — and 294 of the words track's 1078 <c>w:pBdr</c> elements are in
    /// <c>styles.xml</c>.
    /// </remarks>
    [Fact]
    public void AStylesBorderSurvivesTheParagraphStatingADifferentSide()
    {
        ParagraphBorderSet borders = BorderedParagraph(StyleWithBottomRule, "<w:pStyle w:val=\"Boxed\"/>"
            + "<w:pBdr><w:top w:val=\"single\" w:sz=\"6\" w:space=\"1\" w:color=\"FF0000\"/></w:pBdr>");

        borders.Top!.Value.Width.ShouldBe(Length.FromPoints(0.75));
        borders.Bottom!.Value.Width.ShouldBe(Length.FromPoints(2.25));
    }

    /// <summary>
    /// A stated <c>w:val="none"</c> beats the style below it, and takes no room.
    /// </summary>
    /// <remarks>
    /// A silence and a stated <em>no border</em> are different answers: the first lets the style speak
    /// and the second stops it. This is how a header paragraph switches off the rule its style gives it,
    /// which is the commonest shape the property takes in the corpus.
    /// </remarks>
    [Fact]
    public void AStatedNoneBeatsTheStyleAndTakesNoRoom()
    {
        ParagraphBorderSet borders = BorderedParagraph(StyleWithBottomRule, "<w:pStyle w:val=\"Boxed\"/>"
            + "<w:pBdr><w:bottom w:val=\"none\" w:sz=\"0\" w:space=\"0\" w:color=\"auto\"/></w:pBdr>");

        borders.Bottom!.Value.Draws.ShouldBeFalse();
        borders.Below.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// Two consecutive paragraphs bordered alike are one box: no rule between, no allowance spent.
    /// </summary>
    /// <remarks>
    /// Measured — the second of two identically boxed paragraphs sits one plain pitch below the first,
    /// 12.65 pt, where two separately boxed ones would be 6.50 pt further apart. Resolved in the reader
    /// because the join changes the height as well as the picture, and a rule drawn where no room was
    /// reserved lands on the text.
    /// </remarks>
    [Fact]
    public void TwoParagraphsBorderedAlikeShareOneBox()
    {
        string box = "<w:pBdr>"
            + "<w:top w:val=\"single\" w:sz=\"18\" w:space=\"1\" w:color=\"000000\"/>"
            + "<w:bottom w:val=\"single\" w:sz=\"18\" w:space=\"1\" w:color=\"000000\"/></w:pBdr>";

        List<PageParagraph> paragraphs = Paragraphs(
            string.Empty,
            Paragraph("Alpha") + Paragraph("Bravo", box) + Paragraph("Delta", box) + Paragraph("Charlie"));

        paragraphs[1].Borders!.Top!.Value.Draws.ShouldBeTrue();
        paragraphs[1].Borders!.Bottom.ShouldBeNull();
        paragraphs[2].Borders!.Top.ShouldBeNull();
        paragraphs[2].Borders!.Bottom!.Value.Draws.ShouldBeTrue();

        IReadOnlyList<Length> baselines = Baselines(
            Paragraph("Alpha") + Paragraph("Bravo", box) + Paragraph("Delta", box) + Paragraph("Charlie"));

        (baselines[2] - baselines[1]).ShouldBe(Length.FromPoints(12.65));
        (baselines[1] - baselines[0]).ShouldBe(Length.FromPoints(12.65 + 3.25));
        (baselines[3] - baselines[2]).ShouldBe(Length.FromPoints(12.65 + 3.25));
    }

    /// <summary>
    /// A <c>w:between</c> draws in that gap, and keeps its distance on both sides of itself.
    /// </summary>
    /// <remarks>
    /// Measured at 2.75 pt for <c>w:sz="6" w:space="1"</c> — 0.75 + 1 + 1 — against the 12.65 pt plain
    /// pitch, which is what <see cref="ParagraphBorder.Trailing"/> exists to express. Rare: 5 of the 158
    /// <c>w:between</c> elements in the words track state a rule at all.
    /// </remarks>
    [Fact]
    public void ABetweenRuleSeparatesTwoJoinedParagraphs()
    {
        string box = "<w:pBdr>"
            + "<w:top w:val=\"single\" w:sz=\"18\" w:space=\"1\" w:color=\"000000\"/>"
            + "<w:bottom w:val=\"single\" w:sz=\"18\" w:space=\"1\" w:color=\"000000\"/>"
            + "<w:between w:val=\"single\" w:sz=\"6\" w:space=\"1\" w:color=\"FF0000\"/></w:pBdr>";

        IReadOnlyList<Length> baselines = Baselines(
            Paragraph("Alpha") + Paragraph("Bravo", box) + Paragraph("Delta", box) + Paragraph("Charlie"));

        (baselines[2] - baselines[1]).ShouldBe(Length.FromPoints(12.65 + 2.75));
    }

    /// <summary>
    /// Left and right borders draw without narrowing the text.
    /// </summary>
    /// <remarks>
    /// LibreOffice grows the box outward — with <c>w:sz="24" w:space="10"</c> against a 72 pt margin the
    /// rule's left end lands at 59.00 pt, inside the page margin — so a bordered paragraph breaks its
    /// lines exactly where an unbordered one would. That is why only the top and bottom reach the
    /// paginator.
    /// </remarks>
    [Fact]
    public void SideBordersDoNotTakeVerticalRoom()
    {
        IReadOnlyList<Length> plain = Baselines(Borderless);
        IReadOnlyList<Length> sides = Baselines(Bordered("left", 24, 10));

        sides.Count.ShouldBe(plain.Count);
        for (int i = 0; i < plain.Count; i++) sides[i].ShouldBe(plain[i]);
    }

    private const string StyleWithBottomRule =
        "<w:style w:type=\"paragraph\" w:styleId=\"Boxed\"><w:name w:val=\"Boxed\"/><w:pPr>"
        + "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"18\" w:space=\"1\" w:color=\"000000\"/></w:pBdr>"
        + "</w:pPr></w:style>";

    private static string Borderless => Paragraph("Alpha") + Paragraph("Bravo") + Paragraph("Charlie");

    private static string Bordered(string side, int sz, int space, int spacingTwips = 0)
        => Paragraph("Alpha")
           + Paragraph(
               "Bravo",
               $"<w:pBdr><w:{side} w:val=\"single\" w:sz=\"{sz}\" w:space=\"{space}\" "
               + "w:color=\"000000\"/></w:pBdr>",
               spacingTwips)
           + Paragraph("Charlie");

    private const string RunProperties =
        "<w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\"/><w:sz w:val=\"22\"/></w:rPr>";

    private static string Paragraph(string text, string properties = "", int spacingTwips = 0)
        => "<w:p><w:pPr>"
           + $"<w:spacing w:before=\"{spacingTwips}\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/>"
           + properties + RunProperties + "</w:pPr>"
           + $"<w:r>{RunProperties}<w:t>{text}</w:t></w:r></w:p>";

    /// <summary>The baseline of every line on page one, in order down the page.</summary>
    private static IReadOnlyList<Length> Baselines(string body, string styles = "")
    {
        using IDocument document = Open(body, styles);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Pages[0].Lines.Select(line => line.Baseline)];
    }

    private static List<PageParagraph> Paragraphs(string styles, string body)
    {
        using IDocument document = Open(body, styles);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return [.. pages.Paragraphs];
    }

    private static ParagraphBorderSet BorderedParagraph(string styles, string properties)
    {
        List<PageParagraph> paragraphs = Paragraphs(
            styles, Paragraph("Alpha") + Paragraph("Bravo", properties) + Paragraph("Charlie"));

        return paragraphs[1].Borders.ShouldNotBeNull();
    }

    private static IDocument Open(string body, string styles)
    {
        MemoryStream package = BuildPackage(body, styles);
        using DocumentSource source = DocumentSource.FromStream(package, "paragraph-border.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string body, string styles)
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
              <Relationship Id="rIdS" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        // US Letter with 72 pt margins, which is what the reference figures were measured on.
        const string Section =
            """<w:sectPr><w:pgSz w:w="12240" w:h="15840"/><w:pgMar w:top="1440" w:right="1440" """
            + """w:bottom="1440" w:left="1440"/></w:sectPr>""";

        string document =
            """<?xml version="1.0" encoding="UTF-8"?>"""
            + """<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">"""
            + $"<w:body>{body}{Section}</w:body></w:document>";

        string styleSheet =
            """<?xml version="1.0" encoding="UTF-8"?>"""
            + """<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">"""
            + styles + "</w:styles>";

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/document.xml", document);
            Write(archive, "word/styles.xml", styleSheet);
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
