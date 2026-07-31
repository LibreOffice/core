using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that the page geometry Paperless reads is the geometry LibreOffice renders.
/// </summary>
/// <remarks>
/// <para>
/// One source document in five formats, all describing the same distinctive page — 18 by 24 cm with
/// four different margins, a 1.2 cm header and a 1 cm footer, none of them a default anything could
/// accidentally agree with. Each format states this differently enough that a single shared assertion
/// is worth more than four separate ones: DOCX and DOC and RTF give the body's top margin and the
/// header's distance from the page edge, while ODF gives the header's distance and its height and
/// leaves the body's margin to be worked out.
/// </para>
/// <para>
/// The ground truth is LibreOffice's own PDF rather than the numbers in the files. The page box gives
/// the sheet size; the first body line's top gives the top margin; the first body word's left edge
/// gives the left margin; and the header's top gives the header distance. Checking against the files
/// would only prove the reader can read what it was told, which is the easy half — the interesting
/// question is whether the four readings agree with where Writer actually puts the text.
/// </para>
/// <para>
/// The last test closes the loop: the text width these margins imply is fed to the line filler, and
/// the resulting line divisions are compared with LibreOffice's. That is the assertion that would fail
/// if a margin were right to the millimetre and wrong to the twip, because a line break is decided at
/// a finer resolution than a millimetre.
/// </para>
/// </remarks>
public sealed class PageGeometryComparisonTests : IDisposable
{
    /// <summary>
    /// How far a measurement may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// Two thirds of a point. Every format but ODF stores this geometry in twips and ODF stores it in
    /// hundredths of a millimetre, so a round trip through LibreOffice's exporters quantises each
    /// dimension twice; and a PDF word box's top edge is the line box's, which sits a fraction above
    /// the text. Anything larger is a real disagreement about the geometry.
    /// </remarks>
    private const double TolerancePoints = 0.7;

    /// <summary>The paragraph the corpus documents contain, needed to check the line divisions.</summary>
    private const string Paragraph =
        "Extraction and rendering are separate paths through the library, because extraction is the "
        + "common case and must not pay for fonts, layout or a rasteriser it never uses.";

    /// <summary>The corpus files, one source document exported to every format Paperless reads.</summary>
    private static readonly string[] EveryFormat =
    [
        "page-geometry.fodt", "page-geometry.odt", "page-geometry.docx",
        "page-geometry.doc", "page-geometry.rtf",
    ];

    /// <summary>The plain documents, none of which states any section properties.</summary>
    private static readonly string[] PlainDocuments =
    [
        "prose-docx.docx", "prose-doc.doc", "prose-rtf.rtf", "prose-odt.odt", "prose-fodt.fodt",
    ];

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-pages").FullName;

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
    [InlineData("page-geometry.fodt")]
    [InlineData("page-geometry.odt")]
    [InlineData("page-geometry.docx")]
    [InlineData("page-geometry.doc")]
    [InlineData("page-geometry.rtf")]
    public void EveryFormatReadsThePageLibreOfficeRenders(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        PageGeometry page = FirstPage(path);
        RenderedPage rendered = Render(path);

        // The sheet itself. A page size read wrongly is the one error that cannot be compensated for
        // anywhere downstream, because every margin is measured from an edge.
        Close(page.Size.Width, rendered.Width, "page width");
        Close(page.Size.Height, rendered.Height, "page height");

        // Where the body text starts, which is the number pagination and line breaking both depend on.
        Close(page.Margins.Left, rendered.BodyLeft, "left margin");
        Close(page.Margins.Top, rendered.BodyTop, "top margin");

        // And where the header sits, which is the property the four formats disagree most about how to
        // state — Word measures it from the page edge, ODF measures the body from it.
        Close(page.HeaderDistance, rendered.HeaderTop, "header distance");
    }

    [Fact]
    public void AllFiveFormatsAgreeWithEachOther()
    {
        // Cross-format agreement, asserted separately because it fails differently: if one reader is
        // wrong the test above says which measurement, and this one says the readers disagree even
        // where they might all be plausibly wrong together.
        List<(string File, PageGeometry Page)> pages =
        [
            .. EveryFormat.Select(name => (name, FirstPage(Corpus.Require(name)))),
        ];

        PageGeometry first = pages[0].Page;
        foreach ((string file, PageGeometry page) in pages.Skip(1))
        {
            Close(page.Size.Width, first.Size.Width.Points, $"{file}: page width");
            Close(page.Size.Height, first.Size.Height.Points, $"{file}: page height");
            Close(page.Margins.Left, first.Margins.Left.Points, $"{file}: left margin");
            Close(page.Margins.Right, first.Margins.Right.Points, $"{file}: right margin");
            Close(page.Margins.Top, first.Margins.Top.Points, $"{file}: top margin");
            Close(page.Margins.Bottom, first.Margins.Bottom.Points, $"{file}: bottom margin");
            Close(page.HeaderDistance, first.HeaderDistance.Points, $"{file}: header distance");
            Close(page.HeaderHeight, first.HeaderHeight.Points, $"{file}: header height");
            Close(page.FooterHeight, first.FooterHeight.Points, $"{file}: footer height");
            page.Columns.ShouldBe(first.Columns, $"{file}: columns");
            page.IsLandscape.ShouldBe(first.IsLandscape, $"{file}: orientation");
        }
    }

    [Theory]
    [InlineData("page-geometry.fodt")]
    [InlineData("page-geometry.odt")]
    [InlineData("page-geometry.docx")]
    [InlineData("page-geometry.doc")]
    [InlineData("page-geometry.rtf")]
    public void TheTextWidthTheMarginsImplyBreaksLinesWhereLibreOfficeDoes(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string? fontPath = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(fontPath is null, "Carlito is not installed; see check-env.sh");

        string path = Corpus.Require(fileName);
        PageGeometry page = FirstPage(path);
        OpenTypeFace face = OpenTypeFace.ReadFile(fontPath!).ShouldNotBeNull();

        // The end-to-end check. A margin can be right to the millimetre and still wrong: a line break
        // is decided by whether a word's last glyph passes the margin, which is a comparison at EMU
        // resolution. So the width these margins imply is fed to the filler and the divisions compared.
        LineFiller filler = new(new TextMeasurer(face));
        List<string> actual =
        [
            .. filler
                .Fill(
                    Paragraph,
                    Length.FromPoints(12),
                    page.TextWidth,
                    options: new ShapingOptions(Language: "en-GB"))
                .Select(line => line.VisibleTextIn(Paragraph).ToString())
                .Select(Normalise)
                .Where(text => text.Length > 0),
        ];

        List<string> reference = [.. Render(path).Lines.Where(line => !IsFurniture(line))];

        actual.Count.ShouldBe(
            reference.Count,
            $"{fileName}: line count differs at a text width of {page.TextWidth.Points:F2} pt."
            + $"\n  LibreOffice:\n    {string.Join("\n    ", reference)}"
            + $"\n  Paperless:\n    {string.Join("\n    ", actual)}");

        for (int i = 0; i < reference.Count; i++)
        {
            actual[i].ShouldBe(reference[i], $"{fileName}: line {i + 1} differs.");
        }
    }

    [Fact]
    public void ADocumentWithNoSectionPropertiesStillHasOneSection()
    {
        // Never an empty list: every format means "one section of default geometry" by saying nothing,
        // not "no pages". A caller that had to handle an empty list would have to invent the defaults
        // itself, and would invent different ones.
        foreach (string name in PlainDocuments)
        {
            using IDocument document = Open(Corpus.Require(name));
            IWordProcessingDocument word = document.ShouldBeAssignableTo<IWordProcessingDocument>()!;

            word.Sections.ShouldNotBeEmpty(name);
            word.Sections[0].Page.TextWidth.ShouldBeGreaterThan(Length.Zero, name);
            word.Sections[0].Page.TextHeight.ShouldBeGreaterThan(Length.Zero, name);
        }
    }

    [Fact]
    public void FurnitureFallsBackToTheDefaultSlotRatherThanDisappearing()
    {
        // The slot rules, which every format shares and none states. The easy mistake is to give a
        // first page nothing when the section has only a default header — a section that does not ask
        // for a different first page still has a header on its first page.
        WritingBody defaultHeader = new(WritingBodyKind.Header);
        WritingSection section = new()
        {
            Headers = new Dictionary<PageFurnitureSlot, WritingBody>
            {
                [PageFurnitureSlot.Default] = defaultHeader,
            },
        };

        section.HeaderFor(pageNumber: 1, isFirstPageOfSection: true).ShouldBeSameAs(defaultHeader);
        section.HeaderFor(pageNumber: 2, isFirstPageOfSection: false).ShouldBeSameAs(defaultHeader);
        section.FooterFor(pageNumber: 1, isFirstPageOfSection: true).ShouldBeNull();

        // And a first-page header is used only when the section asks for one, since a document can
        // carry the part without enabling it.
        WritingBody firstHeader = new(WritingBodyKind.Header);
        WritingSection titled = section with
        {
            Headers = new Dictionary<PageFurnitureSlot, WritingBody>
            {
                [PageFurnitureSlot.Default] = defaultHeader,
                [PageFurnitureSlot.First] = firstHeader,
            },
        };

        titled.HeaderFor(1, isFirstPageOfSection: true).ShouldBeSameAs(defaultHeader);
        (titled with { HasDifferentFirstPage = true })
            .HeaderFor(1, isFirstPageOfSection: true)
            .ShouldBeSameAs(firstHeader);
    }

    // ------------------------------------------------------------------------- the machinery

    private static IDocument Open(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }

    private static PageGeometry FirstPage(string path)
    {
        using IDocument document = Open(path);
        IWordProcessingDocument word = document.ShouldBeAssignableTo<IWordProcessingDocument>()!;
        return word.Sections[0].Page;
    }

    private static void Close(Length actual, double expectedPoints, string what)
        => Math.Abs(actual.Points - expectedPoints).ShouldBeLessThanOrEqualTo(
            TolerancePoints,
            $"{what}: {actual.Points:F3} pt vs {expectedPoints:F3} pt");

    /// <summary>What LibreOffice's own rendering says the page is.</summary>
    /// <param name="Width">The page box's width in points.</param>
    /// <param name="Height">The page box's height in points.</param>
    /// <param name="HeaderTop">The top of the header's first line.</param>
    /// <param name="BodyTop">The top of the body's first line.</param>
    /// <param name="BodyLeft">The left edge of the body's first word.</param>
    /// <param name="Lines">Every line of text, the furniture included, in order.</param>
    private readonly record struct RenderedPage(
        double Width,
        double Height,
        double HeaderTop,
        double BodyTop,
        double BodyLeft,
        IReadOnlyList<string> Lines);

    private RenderedPage Render(string documentPath)
    {
        string pdf = _libreOffice.ConvertToPdf(documentPath, _workDirectory);

        (double width, double height) = PageBox(pdf);
        List<(double Top, double Left, string Text)> lines = LinesOf(pdf);

        Assert.SkipWhen(lines.Count < 3, "the rendered page has no header, body and footer");

        // The header is the first line and the footer the last, because the corpus documents have one
        // of each and a body between them. Identifying them by position rather than by their text keeps
        // the assertions independent of what the furniture says.
        (double Top, double Left, string Text) header = lines[0];
        (double Top, double Left, string Text) body = lines.First(l => !IsFurniture(l.Text));

        return new RenderedPage(
            width, height, header.Top, body.Top, body.Left, [.. lines.Select(l => l.Text)]);
    }

    /// <summary>
    /// True for the running head and foot, which are not body text.
    /// </summary>
    /// <remarks>
    /// Matched on their content because the corpus documents put known words there. Distinguishing them
    /// by vertical position instead would need the margins this test is trying to verify.
    /// </remarks>
    private static bool IsFurniture(string line)
        => line is "Running head" or "Running foot";

    private static (double Width, double Height) PageBox(string pdfPath)
    {
        string output = RunPdfToText(pdfPath, bbox: true);

        Match match = Regex.Match(output, "<page width=\"([0-9.]+)\" height=\"([0-9.]+)\"");
        match.Success.ShouldBeTrue("pdftotext reported no page box");

        return (
            double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture),
            double.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture));
    }

    /// <summary>
    /// The words grouped into lines by their vertical position.
    /// </summary>
    /// <remarks>
    /// A PDF has no lines, only positioned text, so words whose tops are within a point of each other
    /// are taken to be on one line. Safe here because the line pitch is over fourteen points.
    /// </remarks>
    private static List<(double Top, double Left, string Text)> LinesOf(string pdfPath)
    {
        string output = RunPdfToText(pdfPath, bbox: true);

        List<(double Top, double Left, string Text)> lines = [];
        List<string> current = [];
        double currentTop = double.NaN;
        double currentLeft = 0;

        foreach (Match match in Regex.Matches(
                     output, "<word xMin=\"([0-9.]+)\" yMin=\"([0-9.]+)\"[^>]*>([^<]*)</word>"))
        {
            double left = double.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture);
            double top = double.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture);
            string word = System.Net.WebUtility.HtmlDecode(match.Groups[3].Value);

            if (double.IsNaN(currentTop) || Math.Abs(top - currentTop) <= 1.0)
            {
                if (double.IsNaN(currentTop))
                {
                    currentTop = top;
                    currentLeft = left;
                }
                current.Add(word);
                continue;
            }

            lines.Add((currentTop, currentLeft, string.Join(' ', current)));
            current = [word];
            currentTop = top;
            currentLeft = left;
        }

        if (current.Count > 0) lines.Add((currentTop, currentLeft, string.Join(' ', current)));
        return lines;
    }

    private static string RunPdfToText(string pdfPath, bool bbox)
    {
        System.Diagnostics.ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        if (bbox) start.ArgumentList.Add("-bbox");
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

        return output;
    }

    private static string Normalise(string text)
        => string.Join(' ', text.Split(' ', StringSplitOptions.RemoveEmptyEntries));

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
