using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// The things in a PDF that are not drawing: its information, its links and its outline.
/// </summary>
/// <remarks>
/// Pages here are built by hand and tell the sink what to declare, so what is checked is the writer's
/// half of the bargain — that a declared link becomes an annotation on the right page at the right place,
/// and an outline entry a nested bookmark that points where it said. Whether the layout declares the
/// right things for a real document is <c>DocumentLinkTests</c>'s question.
/// </remarks>
public sealed class PdfNavigationTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 3, 5, 12, 30, 0, TimeSpan.Zero),
    };

    private static DocRect Box(double x, double y, double width, double height)
        => new(Length.FromPoints(x), Length.FromPoints(y), Length.FromPoints(width), Length.FromPoints(height));

    private static DocPoint Point(double x, double y) => new(Length.FromPoints(x), Length.FromPoints(y));

    [Fact]
    public void AnExternalLinkBecomesAUriAnnotationWhoseRectangleIsFlippedToPdfSpace()
    {
        PdfFile pdf = Write(new DrawnPage(DrawnPage.A4, sink =>
            sink.AddLink(Box(72, 100, 200, 20), "https://example.com/a b?q=é")));

        (int annotationId, string annotation) = Only(pdf, "/Subtype/Link");
        annotation.ShouldContain("/A<</S/URI/URI(https://example.com/a%20b?q=%C3%A9)>>");

        // A4 is 841.8898 pt tall; y-down 100..120 is y-up 721.8898..741.8898.
        double[] rect = Rectangle(annotation);
        rect[0].ShouldBe(72, 0.01);
        rect[2].ShouldBe(272, 0.01);
        rect[1].ShouldBe(841.8898 - 120, 0.01);
        rect[3].ShouldBe(841.8898 - 100, 0.01);

        // And the page names it, which is what makes it clickable.
        pdf.Text.ShouldContain($"/Annots[{annotationId} 0 R]");
    }

    [Fact]
    public void AnInternalLinkNamesTheObjectOfThePageTheDestinationIsOn()
    {
        PdfFile pdf = Write(
            new DrawnPage(DrawnPage.A4, sink => sink.AddInternalLink(Box(50, 50, 100, 12), "chapter-two")),
            new DrawnPage(DrawnPage.A4, sink => { }) { Index = 1 },
            new DrawnPage(DrawnPage.A4, sink => sink.AddDestination("chapter-two", Point(72, 144))) { Index = 2 });

        List<int> pages = PageObjects(pdf);
        pages.Count.ShouldBe(3);

        string annotation = Only(pdf, "/Subtype/Link").Body;
        Match destination = Regex.Match(annotation, @"/Dest\[(\d+) 0 R/XYZ ([\d.]+) ([\d.]+) null\]");
        destination.Success.ShouldBeTrue(annotation);

        // The third page, though the link is on the first: the destination is resolved after every page exists.
        int.Parse(destination.Groups[1].Value, CultureInfo.InvariantCulture).ShouldBe(pages[2]);
        double.Parse(destination.Groups[2].Value, CultureInfo.InvariantCulture).ShouldBe(72, 0.01);
        double.Parse(destination.Groups[3].Value, CultureInfo.InvariantCulture).ShouldBe(841.8898 - 144, 0.01);

        // The annotation hangs off the first page only.
        PageDictionary(pdf, pages[0]).ShouldContain("/Annots[");
        PageDictionary(pdf, pages[1]).ShouldNotContain("/Annots");
        PageDictionary(pdf, pages[2]).ShouldNotContain("/Annots");

        // And the destination is reachable by name from outside the document.
        pdf.Text.ShouldContain($"/Dests<</Names[(chapter-two)[{pages[2]} 0 R/XYZ");
    }

    [Fact]
    public void AnInternalLinkWithNoDestinationIsDroppedRatherThanWrittenDangling()
    {
        PdfFile pdf = Write(new DrawnPage(DrawnPage.A4, sink =>
        {
            sink.AddInternalLink(Box(50, 50, 100, 12), "nowhere");
            sink.AddLink(Box(50, 80, 100, 12), "https://example.com/");
        }));

        Regex.Count(pdf.Text, "/Subtype/Link").ShouldBe(1);
        pdf.Text.ShouldNotContain("nowhere");
    }

    [Fact]
    public void ADestinationDeclaredTwiceKeepsTheFirst()
    {
        PdfFile pdf = Write(
            new DrawnPage(DrawnPage.A4, sink => sink.AddDestination("here", Point(10, 10))),
            new DrawnPage(DrawnPage.A4, sink => sink.AddDestination("here", Point(10, 10))) { Index = 1 });

        List<int> pages = PageObjects(pdf);
        pdf.Text.ShouldContain($"(here)[{pages[0]} 0 R/XYZ");
        pdf.Text.ShouldNotContain($"(here)[{pages[1]} 0 R/XYZ");
    }

    [Fact]
    public void LinksFollowTheTransformInForceWhenTheyAreAdded()
    {
        PdfFile pdf = Write(new DrawnPage(DrawnPage.A4, sink =>
        {
            sink.Save();
            sink.Transform(AffineTransform.Translation(Length.FromPoints(100).Emu, Length.FromPoints(50).Emu));
            sink.AddLink(Box(10, 10, 30, 10), "https://example.com/");
            sink.Restore();
        }));

        double[] rect = Rectangle(Only(pdf, "/Subtype/Link").Body);
        rect[0].ShouldBe(110, 0.01);
        rect[2].ShouldBe(140, 0.01);
        rect[3].ShouldBe(841.8898 - 60, 0.01);
    }

    [Fact]
    public void TheOutlineIsATreeWithCountsAndLinksBetweenSiblings()
    {
        PdfFile pdf = Write(
            new DrawnPage(DrawnPage.A4, sink =>
            {
                sink.AddOutlineEntry(1, "One", Point(72, 72));
                sink.AddOutlineEntry(2, "One A", Point(72, 200));
                sink.AddOutlineEntry(2, "One B", Point(72, 300));
                sink.AddOutlineEntry(3, "One B i", Point(72, 400));
            }),
            new DrawnPage(DrawnPage.A4, sink =>
            {
                sink.AddOutlineEntry(1, "Two", Point(72, 72));

                // A jump of two levels nests under the nearest shallower entry rather than inventing one.
                sink.AddOutlineEntry(3, "Two deep", Point(72, 150));
            }) { Index = 1 });

        Dictionary<int, string> objects = pdf.Dictionaries();
        Dictionary<string, (int Id, string Body)> byTitle = objects
            .Select(o => (o.Key, Body: o.Value, Title: Regex.Match(o.Value, @"/Title\((.*?)\)")))
            .Where(o => o.Title.Success)
            .ToDictionary(o => o.Title.Groups[1].Value, o => (o.Key, o.Body));

        byTitle.Keys.OrderBy(k => k, StringComparer.Ordinal)
            .ShouldBe(["One", "One A", "One B", "One B i", "Two", "Two deep"]);

        string root = objects.Values.Single(v => v.Contains("/Type/Outlines", StringComparison.Ordinal));
        root.ShouldContain($"/First {byTitle["One"].Id} 0 R/Last {byTitle["Two"].Id} 0 R/Count 6");

        byTitle["One"].Body.ShouldContain($"/Next {byTitle["Two"].Id} 0 R");
        byTitle["One"].Body.ShouldContain($"/First {byTitle["One A"].Id} 0 R/Last {byTitle["One B"].Id} 0 R/Count 3");
        byTitle["One A"].Body.ShouldContain($"/Parent {byTitle["One"].Id} 0 R");
        byTitle["One A"].Body.ShouldContain($"/Next {byTitle["One B"].Id} 0 R");
        byTitle["One B"].Body.ShouldContain($"/Prev {byTitle["One A"].Id} 0 R");
        byTitle["One B"].Body.ShouldContain($"/Count 1");
        byTitle["Two"].Body.ShouldContain($"/Prev {byTitle["One"].Id} 0 R");
        byTitle["Two deep"].Body.ShouldContain($"/Parent {byTitle["Two"].Id} 0 R");

        List<int> pages = PageObjects(pdf);
        byTitle["One B i"].Body.ShouldContain($"/Dest[{pages[0]} 0 R/XYZ 72 {Number(841.8898 - 400)} null]");
        byTitle["Two deep"].Body.ShouldContain($"/Dest[{pages[1]} 0 R/XYZ 72 {Number(841.8898 - 150)} null]");

        pdf.Text.ShouldContain("/PageMode/UseOutlines");
    }

    [Fact]
    public void NavigationCanBeSwitchedOff()
    {
        DrawnPage page = new(DrawnPage.A4, sink =>
        {
            sink.AddLink(Box(1, 1, 10, 10), "https://example.com/");
            sink.AddDestination("x", Point(1, 1));
            sink.AddOutlineEntry(1, "Heading", Point(1, 1));
        });

        PdfFile plain = Write(Reproducible with { WriteLinks = false, WriteOutline = false }, page);
        plain.Text.ShouldNotContain("/Subtype/Link");
        plain.Text.ShouldNotContain("/Outlines");
        plain.Text.ShouldNotContain("/Names");

        PdfFile full = Write(page);
        full.Text.ShouldContain("/Subtype/Link");
        full.Text.ShouldContain("/Outlines");
    }

    [Fact]
    public void PageLabelsAreWrittenWhenTheNumbersPrintedDifferFromThePositions()
    {
        PdfFile numbered = Write(Labelled("1", "2", "3"));
        numbered.Text.ShouldNotContain("/PageLabels");

        // A cover numbered 0 cannot be expressed (a range starts at 1), so it is left out entirely.
        Write(Labelled("0", "1", "2")).Text.ShouldNotContain("/PageLabels");

        PdfFile restarted = Write(Labelled("1", "2", "5", "6"));
        restarted.Text.ShouldContain("/PageLabels<</Nums[0<</S/D/St 1>>2<</S/D/St 5>>]>>");

        // A label that is not a number (a sheet's name) means the rest could not be described either.
        Write(Labelled("1", "Summary", "3")).Text.ShouldNotContain("/PageLabels");
    }

    [Fact]
    public void TheInformationDictionaryCarriesTheDocumentsMetadata()
    {
        DocumentMetadata metadata = new()
        {
            Title = "Quarterly Report",
            Author = "Ada Lovelace",
            Subject = "Q3",
            Keywords = ["finance", "north"],
            GeneratorApplication = "LibreOffice/24.2",
            Language = "en_US",
        };

        PdfFile pdf = Write(Reproducible with { Metadata = metadata }, new DrawnPage(DrawnPage.A4, sink => { }));

        string info = pdf.Dictionaries().Values.Single(v => v.Contains("/Producer", StringComparison.Ordinal));
        info.ShouldContain("/Title(Quarterly Report)");
        info.ShouldContain("/Author(Ada Lovelace)");
        info.ShouldContain("/Subject(Q3)");
        info.ShouldContain("/Keywords(finance, north)");
        info.ShouldContain("/Creator(LibreOffice/24.2)");
        info.ShouldContain("/Producer(X-Ray.Office)");
        info.ShouldContain("/CreationDate(D:20240305123000Z)");

        // The language is a catalogue entry, in BCP 47 form.
        pdf.Text.ShouldContain("/Lang(en-US)");
        pdf.Text.ShouldContain("/ViewerPreferences<</DisplayDocTitle true>>");
    }

    [Fact]
    public void ATitleOutsideLatinOneIsWrittenAsUtf16()
    {
        PdfFile pdf = Write(
            Reproducible with { Metadata = new DocumentMetadata { Title = "報告書" } },
            new DrawnPage(DrawnPage.A4, sink => { }));

        // FEFF followed by the three code points, big-endian.
        pdf.Text.ShouldContain("/Title<FEFF5831544A66F8>");
    }

    [Fact]
    public void UnusableMetadataIsLeftOutRatherThanWrittenAsGarbage()
    {
        DocumentMetadata metadata = new()
        {
            Title = "   ",
            Author = "Bad\u0000Char\u0007s\nand a newline",
            Language = "English",
            Keywords = ["", "ok"],
        };

        PdfFile pdf = Write(Reproducible with { Metadata = metadata }, new DrawnPage(DrawnPage.A4, sink => { }));

        string info = pdf.Dictionaries().Values.Single(v => v.Contains("/Producer", StringComparison.Ordinal));
        info.ShouldNotContain("/Title");
        info.ShouldContain("/Author(BadChars and a newline)");
        info.ShouldContain("/Keywords(ok)");

        // "English" is a word and not a language tag.
        pdf.Text.ShouldNotContain("/Lang");
    }

    [Fact]
    public void AnXmpPacketCarriesTheSameFactsAndIsWellFormed()
    {
        DocumentMetadata metadata = new()
        {
            Title = "Fish & Chips <menu>",
            Author = "Ada",
            Keywords = ["a", "b"],
            Language = "de-DE",
            GeneratorApplication = "Writer",
        };

        PdfFile pdf = Write(Reproducible with { Metadata = metadata }, new DrawnPage(DrawnPage.A4, sink => { }));

        (string Dictionary, byte[] Data, bool Deflated) metadataStream = pdf.Streams()
            .Single(s => s.Dictionary.Contains("/Type/Metadata", StringComparison.Ordinal));
        metadataStream.Dictionary.ShouldContain("/Subtype/XML");
        metadataStream.Deflated.ShouldBeFalse("XMP is read by scanners that do not inflate");

        string packet = Encoding.UTF8.GetString(metadataStream.Data);
        packet.ShouldStartWith("<?xpacket begin=");
        packet.ShouldEndWith("<?xpacket end=\"w\"?>");

        // Everything between the processing instructions is XML, whatever the title held.
        int open = packet.IndexOf("<x:xmpmeta", StringComparison.Ordinal);
        int close = packet.IndexOf("</x:xmpmeta>", StringComparison.Ordinal) + "</x:xmpmeta>".Length;
        XElement xmp = XElement.Parse(packet[open..close]);

        XNamespace dc = "http://purl.org/dc/elements/1.1/";
        XNamespace pdfNs = "http://ns.adobe.com/pdf/1.3/";
        XNamespace xmpNs = "http://ns.adobe.com/xap/1.0/";
        xmp.Descendants(dc + "title").Single().Value.ShouldBe("Fish & Chips <menu>");
        xmp.Descendants(dc + "creator").Single().Value.ShouldBe("Ada");
        xmp.Descendants(dc + "subject").Single().Elements().Single().Elements().Select(e => e.Value).ShouldBe(["a", "b"]);
        xmp.Descendants(dc + "language").Single().Value.ShouldBe("de-DE");
        xmp.Descendants(pdfNs + "Producer").Single().Value.ShouldBe("X-Ray.Office");
        xmp.Descendants(xmpNs + "CreatorTool").Single().Value.ShouldBe("Writer");
        xmp.Descendants(xmpNs + "CreateDate").Single().Value.ShouldStartWith("2024-03-05T12:30:00");

        pdf.Text.ShouldContain("/Metadata ");

        // And it can be left out.
        Write(Reproducible with { Metadata = metadata, WriteXmp = false }, new DrawnPage(DrawnPage.A4, sink => { }))
            .Text.ShouldNotContain("/Type/Metadata");
    }

    [Fact]
    public void WithNoMetadataTheFileStillStatesItsProducerAndDate()
    {
        PdfFile pdf = Write(Reproducible, new DrawnPage(DrawnPage.A4, sink => { }));

        string info = pdf.Dictionaries().Values.Single(v => v.Contains("/Producer", StringComparison.Ordinal));
        info.ShouldBe("<</Producer(X-Ray.Office)/CreationDate(D:20240305123000Z)/ModDate(D:20240305123000Z)>>");
        pdf.Text.ShouldNotContain("/Lang");
        pdf.Text.ShouldNotContain("/Outlines");
        pdf.Text.ShouldNotContain("/Annots");
    }

    [Fact]
    public void TheSameInputGivesTheSameBytes()
    {
        static byte[] Render() => Bytes(Reproducible with
        {
            Metadata = new DocumentMetadata { Title = "T", Author = "A" },
        }, new DrawnPage(DrawnPage.A4, sink =>
        {
            sink.AddLink(Box(1, 1, 10, 10), "https://example.com/");
            sink.AddOutlineEntry(1, "H", Point(1, 1));
        }));

        Render().ShouldBe(Render());
    }

    // ------------------------------------------------------------------------------------------------

    private static IPage[] Labelled(params string[] labels)
        => [.. labels.Select((label, index) => new LabelledPage(index, label))];

    private static PdfFile Write(params IPage[] pages) => Write(Reproducible, pages);

    private static PdfFile Write(PdfRenderOptions options, params IPage[] pages)
        => PdfFile.Parse(Bytes(options, pages));

    private static byte[] Bytes(PdfRenderOptions options, params IPage[] pages)
    {
        using MemoryStream output = new();
        new PdfRenderer(options).Render(new DrawnPages(pages), output);
        return output.ToArray();
    }

    /// <summary>The one object whose body contains a marker.</summary>
    private static (int Id, string Body) Only(PdfFile pdf, string marker)
    {
        KeyValuePair<int, string> found = pdf.Dictionaries()
            .Single(o => o.Value.Contains(marker, StringComparison.Ordinal));
        return (found.Key, found.Value);
    }

    private static double[] Rectangle(string annotation)
    {
        Match rect = Regex.Match(annotation, @"/Rect\[([^\]]+)\]");
        rect.Success.ShouldBeTrue(annotation);
        return [.. rect.Groups[1].Value.Split(' ').Select(n => double.Parse(n, CultureInfo.InvariantCulture))];
    }

    /// <summary>The object numbers of the pages, in page order, as the page tree lists them.</summary>
    private static List<int> PageObjects(PdfFile pdf)
    {
        Match kids = Regex.Match(pdf.Text, @"/Kids\[([^\]]*)\]");
        kids.Success.ShouldBeTrue();
        return [.. Regex.Matches(kids.Groups[1].Value, @"(\d+) 0 R")
            .Select(m => int.Parse(m.Groups[1].Value, CultureInfo.InvariantCulture))];
    }

    private static string PageDictionary(PdfFile pdf, int id) => pdf.Dictionaries()[id];

    private static string Number(double value) => PdfNumber(value);

    private static string PdfNumber(double value)
        => Math.Round(value, 4, MidpointRounding.AwayFromZero).ToString("0.####", CultureInfo.InvariantCulture);

    private sealed class LabelledPage(int index, string label) : IPage
    {
        public int Index => index;
        public DocSize Size => DrawnPage.A4;
        public string? Label => label;

        public void Draw(IDrawingSink sink)
        {
            sink.BeginPage(Size);
            sink.EndPage();
        }
    }
}
