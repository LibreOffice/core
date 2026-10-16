using System.Diagnostics;
using System.Globalization;
using System.IO.Compression;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// A real document through the whole pipeline: what it says about itself, where it links and what its
/// bookmarks are, as they come out of the PDF.
/// </summary>
/// <remarks>
/// The unit tests in <see cref="PdfNavigationTests"/> have a page tell the sink what to declare. These
/// start from a word-processing file, so they answer the other question: does layout declare the right
/// things — a hyperlink's run, a bookmark's line, a heading's level — and does the file say so on the
/// right page.
/// </remarks>
public sealed class DocumentLinkTests
{
    private const string Hyperlink =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink";

    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 3, 5, 12, 30, 0, TimeSpan.Zero),
    };

    /// <summary>
    /// Two chapters on two pages, an external link and an internal one on the first, and a sub-heading.
    /// </summary>
    private static byte[] Chapters()
    {
        const string heading1 =
            "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"{0}\" w:name=\"{1}\"/>"
            + "<w:r><w:t>{2}</w:t></w:r><w:bookmarkEnd w:id=\"{0}\"/></w:p>";

        StringBuilder body = new();
        body.AppendFormat(CultureInfo.InvariantCulture, heading1, 1, "_Toc100", "Introduction");
        body.Append(
            "<w:p><w:r><w:t xml:space=\"preserve\">See </w:t></w:r>"
            + "<w:hyperlink r:id=\"rId10\"><w:r><w:t>the project site</w:t></w:r></w:hyperlink>"
            + "<w:r><w:t xml:space=\"preserve\"> and </w:t></w:r>"
            + "<w:hyperlink w:anchor=\"_Toc200\"><w:r><w:t>the details</w:t></w:r></w:hyperlink>"
            + "<w:r><w:t>.</w:t></w:r></w:p>");
        body.Append("<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>");
        body.AppendFormat(CultureInfo.InvariantCulture, heading1, 2, "_Toc200", "Details");
        body.Append("<w:p><w:pPr><w:pStyle w:val=\"Heading2\"/></w:pPr><w:r><w:t>A smaller heading</w:t></w:r></w:p>");
        body.Append("<w:p><w:r><w:t>Body text of the second chapter.</w:t></w:r></w:p>");

        const string core =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
            + "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:dcterms=\"http://purl.org/dc/terms/\" "
            + "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
            + "<dc:title>Project Handbook</dc:title><dc:creator>Grace Hopper</dc:creator>"
            + "<dc:subject>Onboarding</dc:subject><dc:language>en-GB</dc:language>"
            + "</cp:coreProperties>";

        return GeneratedDocuments.DocxFromBody(
            body.ToString(),
            $"<Relationship Id=\"rId10\" Type=\"{Hyperlink}\" Target=\"https://example.com/site\" TargetMode=\"External\"/>",
            core);
    }

    private static (byte[] Pdf, IDocument Document) Render(byte[] docx, PdfRenderOptions? options = null)
    {
        DocumentSource source = DocumentSource.FromBytes(docx, "chapters.docx");
        IDocument document = OfficeDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using MemoryStream output = new();
        new PdfRenderer(options ?? Reproducible).Render(document, pages, output);
        return (output.ToArray(), document);
    }

    [Fact]
    public void AHyperlinkBecomesAUriAnnotationOnTheRunsPage()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        List<int> pages = PageObjects(pdf);
        pages.Count.ShouldBe(2);

        KeyValuePair<int, string> link = pdf.Dictionaries()
            .Single(o => o.Value.Contains("/URI(https://example.com/site)", StringComparison.Ordinal));

        double[] rect = Rectangle(link.Value);
        (rect[2] - rect[0]).ShouldBeGreaterThan(40, "the link covers its words, which are wider than a few points");
        (rect[3] - rect[1]).ShouldBeInRange(8, 30);

        // On the first page and not the second.
        pdf.Dictionaries()[pages[0]].ShouldContain($"{link.Key} 0 R");
        pdf.Dictionaries()[pages[1]].ShouldNotContain($"{link.Key} 0 R");
    }

    [Fact]
    public void ALinkToABookmarkPointsAtThePageAndLineTheBookmarkIsOn()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        List<int> pages = PageObjects(pdf);

        string internalLink = pdf.Dictionaries().Values
            .Single(v => v.Contains("/Subtype/Link", StringComparison.Ordinal)
                         && v.Contains("/Dest[", StringComparison.Ordinal));

        Match destination = Regex.Match(internalLink, @"/Dest\[(\d+) 0 R/XYZ ([\d.]+) ([\d.]+) null\]");
        destination.Success.ShouldBeTrue(internalLink);

        // The second chapter starts the second page, so the target is that page and near its top: A4 is
        // 841.89 pt tall and the top margin is 72 pt.
        int.Parse(destination.Groups[1].Value, CultureInfo.InvariantCulture).ShouldBe(pages[1]);
        double.Parse(destination.Groups[3].Value, CultureInfo.InvariantCulture).ShouldBeInRange(700, 842);

        // The same name is in the name tree, so an outside link can use it too.
        pdf.Text.ShouldContain("(_Toc200)[");
    }

    [Fact]
    public void HeadingsBecomeANestedOutlineThatPointsAtTheirPages()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        List<int> pages = PageObjects(pdf);
        Dictionary<int, string> objects = pdf.Dictionaries();

        Dictionary<string, (int Id, string Body)> entries = objects
            .Select(o => (o.Key, o.Value, Title: Regex.Match(o.Value, @"/Title\((.*?)\)")))
            .Where(o => o.Title.Success && o.Value.Contains("/Dest[", StringComparison.Ordinal))
            .ToDictionary(o => o.Title.Groups[1].Value, o => (o.Key, o.Value));

        entries.Keys.OrderBy(k => k, StringComparer.Ordinal)
            .ShouldBe(["A smaller heading", "Details", "Introduction"]);

        entries["Introduction"].Body.ShouldContain($"/Dest[{pages[0]} 0 R");
        entries["Details"].Body.ShouldContain($"/Dest[{pages[1]} 0 R");
        entries["A smaller heading"].Body.ShouldContain($"/Dest[{pages[1]} 0 R");

        // Level 2 hangs off the level 1 before it.
        entries["A smaller heading"].Body.ShouldContain($"/Parent {entries["Details"].Id} 0 R");
        entries["Details"].Body.ShouldContain($"/First {entries["A smaller heading"].Id} 0 R");
        entries["Details"].Body.ShouldContain($"/Prev {entries["Introduction"].Id} 0 R");

        pdf.Text.ShouldContain("/PageMode/UseOutlines");
    }

    [Fact]
    public void TheDocumentsOwnTitleAuthorAndLanguageReachTheFile()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        string info = pdf.Dictionaries().Values.Single(v => v.Contains("/Producer", StringComparison.Ordinal));
        info.ShouldContain("/Title(Project Handbook)");
        info.ShouldContain("/Author(Grace Hopper)");
        info.ShouldContain("/Subject(Onboarding)");

        pdf.Text.ShouldContain("/Lang(en-GB)");
        pdf.Text.ShouldContain("/Type/Metadata");
    }

    [Fact]
    public void TextStaysSearchableAlongsideTheAnnotations()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        // Every font that draws text carries a ToUnicode map, which is what search and copy read.
        pdf.Text.ShouldContain("/ToUnicode");
        pdf.ContentStreams().Count.ShouldBe(2);
    }

    [Fact]
    public void LinksAndOutlineAreAbsentFromADocumentThatHasNone()
    {
        byte[] plain = GeneratedDocuments.Docx(3);
        (byte[] bytes, IDocument document) = Render(plain);
        using IDisposable _ = document;
        PdfFile pdf = PdfFile.Parse(bytes);

        pdf.Text.ShouldNotContain("/Subtype/Link");
        pdf.Text.ShouldNotContain("/Outlines");
    }

    [Fact]
    public void RenderDocumentLaysOutAndRendersInOneCall()
    {
        using IDocument document = OfficeDocument.Open(DocumentSource.FromBytes(Chapters(), "chapters.docx"));
        using MemoryStream output = new();

        new PdfRenderer(Reproducible).RenderDocument((IPaginatedDocument)document, output);

        PdfFile pdf = PdfFile.Parse(output.ToArray());
        pdf.Text.ShouldContain("/Title(Project Handbook)");
        PageObjects(pdf).Count.ShouldBe(2);
    }

    [Fact]
    public void AnIndependentReaderAgreesOnLinksOutlineAndMetadata()
    {
        (byte[] bytes, IDocument document) = Render(Chapters());
        using IDisposable _ = document;

        string? script = PythonReader();
        if (script is null)
        {
            Assert.Skip("python3 with pypdf is not available.");
            return;
        }

        string path = Path.Combine(Path.GetTempPath(), $"xray-links-{Guid.NewGuid():N}.pdf");
        File.WriteAllBytes(path, bytes);
        try
        {
            using Process process = Process.Start(new ProcessStartInfo("python3", ["-I", "-c", script, path])
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
            })!;
            string output = process.StandardOutput.ReadToEnd();
            string error = process.StandardError.ReadToEnd();
            process.WaitForExit();
            process.ExitCode.ShouldBe(0, error);

            using JsonDocument report = JsonDocument.Parse(output);
            JsonElement root = report.RootElement;

            root.GetProperty("pages").GetInt32().ShouldBe(2);
            root.GetProperty("title").GetString().ShouldBe("Project Handbook");
            root.GetProperty("author").GetString().ShouldBe("Grace Hopper");
            root.GetProperty("lang").GetString().ShouldBe("en-GB");

            // The outline as the reader sees it: title, depth, and the page index it resolves to.
            root.GetProperty("outline").EnumerateArray()
                .Select(e => (e.GetProperty("title").GetString(), e.GetProperty("depth").GetInt32(), e.GetProperty("page").GetInt32()))
                .ShouldBe([("Introduction", 0, 0), ("Details", 0, 1), ("A smaller heading", 1, 1)]);

            JsonElement links = root.GetProperty("links");
            links.GetArrayLength().ShouldBe(2);
            links.EnumerateArray()
                .Select(e => (e.GetProperty("page").GetInt32(), e.GetProperty("uri").GetString(), e.GetProperty("target").GetInt32()))
                .ShouldBe([(0, "https://example.com/site", -1), (0, null, 1)], ignoreOrder: true);

            root.GetProperty("text").GetString()!.ShouldContain("Introduction");
            root.GetProperty("text").GetString()!.ShouldContain("the project site");
        }
        finally
        {
            File.Delete(path);
        }
    }

    // ------------------------------------------------------------------------------------------------

    private static string? PythonReader()
    {
        try
        {
            using Process probe = Process.Start(new ProcessStartInfo("python3", ["-I", "-c", "import pypdf"])
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
            })!;
            probe.WaitForExit();
            if (probe.ExitCode != 0) return null;
        }
        catch (Exception exception) when (exception is System.ComponentModel.Win32Exception or IOException)
        {
            return null;
        }

        return """
            import json, sys
            from pypdf import PdfReader
            r = PdfReader(sys.argv[1])
            def walk(items, depth, out):
                for it in items:
                    if isinstance(it, list):
                        walk(it, depth + 1, out)
                    else:
                        out.append({"title": it.title, "depth": depth, "page": r.get_destination_page_number(it)})
            pages = [p.indirect_reference.idnum for p in r.pages]
            outline = []
            walk(r.outline, 0, outline)
            links = []
            for i, page in enumerate(r.pages):
                for a in page.get("/Annots", []) or []:
                    a = a.get_object()
                    if a.get("/Subtype") != "/Link":
                        continue
                    uri = None
                    target = -1
                    if "/A" in a and "/URI" in a["/A"]:
                        uri = str(a["/A"]["/URI"])
                    if "/Dest" in a and not isinstance(a["/Dest"], str):
                        target = pages.index(a["/Dest"][0].idnum)
                    links.append({"page": i, "uri": uri, "target": target})
            meta = r.metadata or {}
            root = r.trailer["/Root"]
            print(json.dumps({
                "pages": len(r.pages),
                "title": meta.get("/Title"),
                "author": meta.get("/Author"),
                "lang": str(root.get("/Lang")) if "/Lang" in root else None,
                "outline": outline,
                "links": links,
                "text": "\n".join(p.extract_text() for p in r.pages),
            }))
            """;
    }

    private static List<int> PageObjects(PdfFile pdf)
    {
        Match kids = Regex.Match(pdf.Text, @"/Kids\[([^\]]*)\]");
        kids.Success.ShouldBeTrue();
        return [.. Regex.Matches(kids.Groups[1].Value, @"(\d+) 0 R")
            .Select(m => int.Parse(m.Groups[1].Value, CultureInfo.InvariantCulture))];
    }

    private static double[] Rectangle(string annotation)
    {
        Match rect = Regex.Match(annotation, @"/Rect\[([^\]]+)\]");
        rect.Success.ShouldBeTrue(annotation);
        return [.. rect.Groups[1].Value.Split(' ').Select(n => double.Parse(n, CultureInfo.InvariantCulture))];
    }
}
