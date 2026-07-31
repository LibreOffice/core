using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Units;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that pages break where LibreOffice breaks them.
/// </summary>
/// <remarks>
/// <para>
/// The last measurement in the chain, and the one every earlier error surfaces in. A wrong advance width
/// moves a line break, a moved line break changes a paragraph's line count, a changed line count moves a
/// page break, and from there every page differs. So a test that agrees with Writer on which paragraph
/// starts each page of a twenty-page document is evidence about the whole measurement path and not only
/// about the loop that fills the pages.
/// </para>
/// <para>
/// The comparison is by content rather than by geometry: LibreOffice renders to PDF, the words on each
/// page are read back, and the first words of each page are matched against the paginator's. Comparing
/// the page a paragraph lands on is what a reader would notice; comparing exact vertical positions would
/// fail for a rounding difference nobody could see.
/// </para>
/// </remarks>
public sealed class PaginationComparisonTests : IDisposable
{
    private const double PageWidthCm = 21.0;
    private const double PageHeightCm = 29.7;
    private const double MarginCm = 2.0;
    private const double SizePoints = 12;

    /// <summary>How many numbered paragraphs the multi-page documents contain.</summary>
    private const int ParagraphCount = 60;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-pagination").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Theory]
    [InlineData(1.0)]
    [InlineData(1.5)]
    [InlineData(2.0)]
    public void PagesBreakWhereLibreOfficeBreaksThem(double proportion)
    {
        // Three spacings, because the page break moves with the line height and the three give three
        // different sets of breaks over the same text. Single spacing alone would let a line-height
        // error hide: it is the one case where the natural height needs no arithmetic.
        AssertPagination(
            ParagraphFormat.Default with { LineSpacing = LineSpacingRule.Multiple(proportion) },
            $"fo:line-height=\"{(proportion * 100).ToString("0", CultureInfo.InvariantCulture)}%\"");
    }

    [Fact]
    public void ParagraphSpacingChangesWhereThePagesBreak()
    {
        // Space before and after are added to the running height, so they change the break as surely as
        // the line height does — and Writer *adds* the two where Word takes the larger, which is what
        // PaginationOptions is for. This checks Writer's behaviour, since the reference is Writer.
        AssertPagination(
            ParagraphFormat.Default with
            {
                SpaceBefore = Length.FromPoints(6),
                SpaceAfter = Length.FromPoints(6),
            },
            "fo:margin-top=\"6pt\" fo:margin-bottom=\"6pt\"");
    }

    [Fact]
    public void AParagraphSplitAcrossAPageKeepsAllItsLines()
    {
        // The property that has to hold whatever the constraints do: pagination places lines, it does
        // not lose them. A split that dropped a line would look like a slightly short page rather than
        // like a bug, which is exactly why it is worth asserting separately.
        (List<LaidOutPage> pages, List<PageParagraph> paragraphs) =
            Paginate(ParagraphFormat.Default, PaginationOptions.Default);

        int expected = paragraphs.Count;
        pages.SelectMany(p => p.Lines)
            .Select(l => l.ParagraphIndex)
            .Distinct()
            .Count()
            .ShouldBe(expected, "every paragraph has to appear on some page");

        // And each paragraph's lines appear once, in order, with no gaps — the check that catches a
        // split that repeated or skipped a line rather than carrying it over.
        foreach (IGrouping<int, PlacedLine> group in pages
                     .SelectMany(p => p.Lines)
                     .GroupBy(l => l.ParagraphIndex))
        {
            List<int> indices = [.. group.Select(l => l.LineIndex)];
            indices.ShouldBe([.. Enumerable.Range(0, indices.Count)], $"paragraph {group.Key}");
        }
    }

    [Fact]
    public void KeepTogetherMovesAParagraphRatherThanSplittingIt()
    {
        OpenTypeFace face = Carlito();

        // A tall paragraph that cannot fit in what is left of a page it does not start.
        List<PageParagraph> paragraphs =
        [
            Paragraph(face, Filler(40), ParagraphFormat.Default),
            Paragraph(face, Filler(20), ParagraphFormat.Default with { KeepTogether = true }),
        ];

        List<LaidOutPage> pages = new Paginator().Paginate(paragraphs, Section());

        // The second paragraph appears on exactly one page. Splitting it would put its index on two.
        pages.Count(p => p.Lines.Any(l => l.ParagraphIndex == 1)).ShouldBe(1);
    }

    [Fact]
    public void KeepWithNextMovesAHeadingToJoinItsBody()
    {
        OpenTypeFace face = Carlito();

        // The classic case: a heading at the foot of a page whose body will not fit under it. Both have
        // to move, or the heading is stranded — which is the single most visible pagination fault a
        // reader notices.
        List<PageParagraph> paragraphs =
        [
            Paragraph(face, Filler(42), ParagraphFormat.Default),
            Paragraph(face, "A heading that must not be stranded",
                ParagraphFormat.Default with { KeepWithNext = true }),
            Paragraph(face, Filler(20), ParagraphFormat.Default),
        ];

        List<LaidOutPage> pages = new Paginator().Paginate(paragraphs, Section());

        int headingPage = pages.First(p => p.Lines.Any(l => l.ParagraphIndex == 1)).Index;
        int bodyPage = pages.First(p => p.Lines.Any(l => l.ParagraphIndex == 2)).Index;

        headingPage.ShouldBe(bodyPage, "the heading has to be on the same page as its body");
    }

    [Fact]
    public void OrphanControlRefusesToLeaveASingleLineBehind()
    {
        OpenTypeFace face = Carlito();

        // Two orphan lines required. A page with room for exactly one line of the next paragraph must
        // take none of it, so the whole paragraph moves — which is why a document with orphan control
        // sometimes has a visibly short page.
        ParagraphFormat controlled = ParagraphFormat.Default with { OrphanLines = 2, WidowLines = 2 };

        for (int filler = 38; filler <= 46; filler++)
        {
            List<PageParagraph> paragraphs =
            [
                Paragraph(face, Filler(filler), ParagraphFormat.Default),
                Paragraph(face, Filler(30), controlled),
            ];

            List<LaidOutPage> pages = new Paginator().Paginate(paragraphs, Section());
            LaidOutPage firstPage = pages[0];

            int onFirstPage = firstPage.Lines.Count(l => l.ParagraphIndex == 1);
            if (onFirstPage > 0)
            {
                onFirstPage.ShouldBeGreaterThanOrEqualTo(
                    2, $"filler {filler}: one line left behind breaks orphan control");
            }

            // And the same at the other end: never a single line carried over.
            int onLastPage = pages[^1].Lines.Count(l => l.ParagraphIndex == 1);
            if (pages.Count > 1 && onLastPage > 0)
            {
                onLastPage.ShouldBeGreaterThanOrEqualTo(
                    2, $"filler {filler}: one line carried over breaks widow control");
            }
        }
    }

    [Fact]
    public void APageBreakBeforeStartsAFreshPage()
    {
        OpenTypeFace face = Carlito();

        List<PageParagraph> paragraphs =
        [
            Paragraph(face, "First", ParagraphFormat.Default),
            Paragraph(face, "Second", ParagraphFormat.Default with { StartsNewPage = true }),
            Paragraph(face, "Third", ParagraphFormat.Default),
        ];

        List<LaidOutPage> pages = new Paginator().Paginate(paragraphs, Section());

        pages.Count.ShouldBe(2);
        pages[0].Lines.Select(l => l.ParagraphIndex).ShouldBe([0]);
        pages[1].Lines.Select(l => l.ParagraphIndex).ShouldBe([1, 2]);
    }

    [Fact]
    public void WriterDropsSpacingAtTheTopOfAPageAndWordKeepsIt()
    {
        OpenTypeFace face = Carlito();

        // A compatibility difference, not a preference. The same document laid out both ways puts the
        // first baseline of every page after the first in two different places, which then changes how
        // much fits and where the following break falls.
        // An explicit page break rather than a filler sized to overflow, so the test says what it means:
        // the second paragraph certainly starts a page, and the only question is whether its space
        // survives. Sizing a filler to just overflow would make the test fail for a line-height change
        // rather than for the behaviour it is about.
        ParagraphFormat spaced = ParagraphFormat.Default with
        {
            SpaceBefore = Length.FromPoints(40),
            StartsNewPage = true,
        };
        List<PageParagraph> paragraphs =
        [
            Paragraph(face, "Ends the first page", ParagraphFormat.Default),
            Paragraph(face, "Starts the second page", spaced),
        ];

        LaidOutPage writer = new Paginator(PaginationOptions.Default)
            .Paginate(paragraphs, Section())[^1];
        LaidOutPage word = new Paginator(PaginationOptions.Word)
            .Paginate(paragraphs, Section())[^1];

        writer.Index.ShouldBe(1, "the break has to have produced a second page");
        writer.Lines[0].Top.ShouldBe(Length.Zero, "Writer drops the space at the top of a page");
        word.Lines[0].Top.ShouldBe(
            Length.FromPoints(40), "Word keeps it, pushing the first line down");
    }

    [Fact]
    public void ADocumentWithNoParagraphsStillHasOnePage()
    {
        // A caller drawing pages has to be given one. An empty list would make "how many pages does this
        // document have" answer zero for a blank document, which no reader agrees with.
        List<LaidOutPage> pages = new Paginator().Paginate([], Section());

        pages.Count.ShouldBe(1);
        pages[0].IsEmpty.ShouldBeTrue();
        pages[0].Size.Width.ShouldBeGreaterThan(Length.Zero);
    }

    [Fact]
    public void ASectionThatRestartsNumberingIsHonoured()
    {
        OpenTypeFace face = Carlito();
        WritingSection section = Section() with { RestartPageNumberAt = 7 };

        List<LaidOutPage> pages = new Paginator()
            .Paginate([Paragraph(face, Filler(120), ParagraphFormat.Default)], section);

        pages.Count.ShouldBeGreaterThan(1);
        pages[0].Number.ShouldBe(7);
        pages[1].Number.ShouldBe(8);

        // The index and the number are separate on purpose: a section can restart at anything, so
        // deriving one from the other would misnumber every document that does.
        pages[0].Index.ShouldBe(0);
    }

    [Fact]
    public void AParagraphTallerThanAPageIsSplitDespiteKeepTogether()
    {
        OpenTypeFace face = Carlito();

        // Keep-together on a paragraph far too tall for any page. The constraint cannot be honoured, so
        // it is overridden and the paragraph splits — which is what Writer does. Honouring it instead
        // would mean placing nothing on page after page until the guard tripped.
        List<PageParagraph> paragraphs =
        [
            Paragraph(face, Filler(200), ParagraphFormat.Default with { KeepTogether = true }),
        ];

        Paginator paginator = new();
        List<LaidOutPage> pages = paginator.Paginate(paragraphs, Section());

        paginator.WasTruncated.ShouldBeFalse();
        pages.Count.ShouldBeGreaterThan(1);
        pages.ShouldAllBe(p => p.Lines.Count > 0, "no page may be left empty");

        // And nothing is lost on the way: every line of the paragraph appears exactly once, in order.
        List<int> indices = [.. pages.SelectMany(p => p.Lines).Select(l => l.LineIndex)];
        indices.ShouldBe([.. Enumerable.Range(0, indices.Count)]);
    }

    // ------------------------------------------------------------------ the comparison itself

    private void AssertPagination(ParagraphFormat format, string odfProperties)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        (List<LaidOutPage> pages, List<PageParagraph> paragraphs) =
            Paginate(format, PaginationOptions.Default);

        List<List<string>> rendered = RenderedPages(odfProperties);

        pages.Count.ShouldBe(
            rendered.Count,
            $"page count differs for {odfProperties}: Paperless {pages.Count}, "
            + $"LibreOffice {rendered.Count}");

        // Which paragraph starts each page is the thing a reader would notice, and the thing every
        // upstream measurement error shows up in.
        for (int i = 0; i < pages.Count; i++)
        {
            string expected = FirstWordsOf(rendered[i]);
            string actual = FirstWordsOfPage(pages[i], paragraphs);

            actual.ShouldBe(
                expected,
                $"{odfProperties}: page {i + 1} starts differently."
                + $"\n  LibreOffice: {expected}\n  Paperless:   {actual}");
        }
    }

    private static (List<LaidOutPage> Pages, List<PageParagraph> Paragraphs) Paginate(
        ParagraphFormat format, PaginationOptions options)
    {
        OpenTypeFace face = Carlito();
        List<PageParagraph> paragraphs =
        [
            .. Enumerable.Range(1, ParagraphCount).Select(n => Paragraph(face, BodyText(n), format)),
        ];

        return (new Paginator(options).Paginate(paragraphs, Section()), paragraphs);
    }

    /// <summary>
    /// The first few words of a page, as the paginator has it.
    /// </summary>
    /// <remarks>
    /// From the line's own character range rather than from the paragraph's text, so a page starting
    /// mid-paragraph is compared against the words LibreOffice actually put there rather than against
    /// the paragraph's opening.
    /// </remarks>
    private static string FirstWordsOfPage(LaidOutPage page, List<PageParagraph> paragraphs)
    {
        if (page.Lines.Count == 0) return string.Empty;

        PlacedLine first = page.Lines[0];
        string text = paragraphs[first.ParagraphIndex].Text;
        return FirstWordsOf([.. first.Box.Line.VisibleTextIn(text).ToString()
            .Split(' ', StringSplitOptions.RemoveEmptyEntries)]);
    }

    private static string FirstWordsOf(List<string> words)
        => string.Join(' ', words.Take(4));

    /// <summary>
    /// Numbered paragraphs, so that the page a paragraph lands on is visible in its own text.
    /// </summary>
    /// <remarks>
    /// The number is what makes a failure diagnosable: "page 4 starts at paragraph 19 rather than 18"
    /// says which way the error goes, where a comparison of unnumbered prose says only that it differs.
    /// </remarks>
    private static string BodyText(int number) =>
        $"Paragraph {number.ToString(CultureInfo.InvariantCulture)} of the pagination sample, which is "
        + "long enough to occupy several lines at the width of the page so that the arithmetic of "
        + "filling a page has something to add up.";

    private static string Filler(int lines) =>
        string.Join(' ', Enumerable.Range(0, lines * 8).Select(i =>
            $"word{i.ToString(CultureInfo.InvariantCulture)}"));

    private static PageParagraph Paragraph(OpenTypeFace face, string text, ParagraphFormat format)
        => new()
        {
            Text = text,
            Face = face,
            Format = format,
            EmSize = Length.FromPoints(SizePoints),
            Language = "en-GB",
        };

    private static WritingSection Section() => new()
    {
        Page = new PageGeometry
        {
            Size = new Core.Geometry.DocSize(
                Length.FromMillimetres(PageWidthCm * 10),
                Length.FromMillimetres(PageHeightCm * 10)),
            Margins = PageMargins.Uniform(Length.FromMillimetres(MarginCm * 10)),
        },
    };

    private static OpenTypeFace Carlito()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    // ------------------------------------------------------------------------- the reference

    /// <summary>The words on each rendered page, in order.</summary>
    private List<List<string>> RenderedPages(string odfProperties)
    {
        string key = odfProperties.GetHashCode(StringComparison.Ordinal)
            .ToString("x8", CultureInfo.InvariantCulture);

        string source = Path.Combine(_workDirectory, $"pagination-{key}.fodt");
        File.WriteAllText(source, Document(odfProperties));

        string pdf = _libreOffice.ConvertToPdf(source, _workDirectory);
        return WordsPerPage(pdf);
    }

    private static List<List<string>> WordsPerPage(string pdfPath)
    {
        System.Diagnostics.ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add("-bbox");
        start.ArgumentList.Add(pdfPath);
        start.ArgumentList.Add("-");

        using System.Diagnostics.Process process =
            System.Diagnostics.Process.Start(start)
            ?? throw new InvalidOperationException("pdftotext did not start");

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);

        Assert.SkipWhen(
            process.ExitCode != 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<List<string>> pages = [];
        foreach (Match page in Regex.Matches(output, "<page[^>]*>(.*?)</page>", RegexOptions.Singleline))
        {
            List<string> words =
            [
                .. Regex.Matches(page.Groups[1].Value, "<word[^>]*>([^<]*)</word>")
                    .Select(m => System.Net.WebUtility.HtmlDecode(m.Groups[1].Value)),
            ];

            pages.Add(words);
        }

        return pages;
    }

    /// <summary>
    /// The same numbered paragraphs as a flat ODF document.
    /// </summary>
    /// <remarks>
    /// No headers, footers, orphan or widow control, so the only thing deciding a page break is how much
    /// text fits — which is what the comparison is about. Writer's own defaults for orphans and widows
    /// are switched off explicitly, since inheriting them would compare Paperless's unconstrained
    /// pagination against Writer's constrained one.
    /// </remarks>
    private static string Document(string paragraphProperties)
    {
        string size = SizePoints.ToString(CultureInfo.InvariantCulture);
        string body = string.Join(
            "\n   ",
            Enumerable.Range(1, ParagraphCount).Select(n =>
                $"<text:p text:style-name=\"Measured\">"
                + $"{System.Security.SecurityElement.Escape(BodyText(n))}</text:p>"));

        return $"""
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
            office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
         <office:automatic-styles>
          <style:page-layout style:name="Measured">
           <style:page-layout-properties fo:page-width="{PageWidthCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:page-height="{PageHeightCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-left="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-right="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-top="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"
                                         fo:margin-bottom="{MarginCm.ToString(CultureInfo.InvariantCulture)}cm"/>
          </style:page-layout>
         </office:automatic-styles>
         <office:master-styles>
          <style:master-page style:name="Standard" style:page-layout-name="Measured"/>
         </office:master-styles>
         <office:styles>
          <style:style style:name="Measured" style:family="paragraph">
           <style:paragraph-properties fo:text-align="start" fo:text-indent="0cm"
                                       fo:hyphenate="false" fo:orphans="0" fo:widows="0"
                                       {paragraphProperties}/>
           <style:text-properties style:font-name="Carlito" fo:font-family="Carlito"
                                  fo:font-size="{size}pt" fo:language="en" fo:country="GB"/>
          </style:style>
         </office:styles>
         <office:body>
          <office:text>
           {body}
          </office:text>
         </office:body>
        </office:document>
        """;
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }
}
