using System.IO.Compression;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The corpus documents that carry a real WMF, read the way a reader would read them.
/// </summary>
/// <remarks>
/// <para>
/// A WMF is almost always <em>embedded</em>, so the realistic test is a document containing
/// one. These two also settle a question the fixture was built to settle: whether LibreOffice's
/// ODT and DOCX exports keep the metafile or rasterise it. Both keep it — 426 bytes of WMF in
/// each package — so decoding it is a real win rather than a hypothetical one, and the ODT's
/// 9 685-byte PNG preview beside it is exactly the fallback that decoding avoids.
/// </para>
/// <para>
/// The package is opened directly rather than through <c>Paperless.Containers</c>: what is
/// under test is the picture, and pulling the package readers in would make a failure here
/// ambiguous between two layers.
/// </para>
/// </remarks>
public sealed class CorpusWmfTests
{
    private static readonly string Corpus = Locate();

    [Fact]
    public void TheStandaloneMetafileDrawsEveryKindOfRecordItHolds()
    {
        byte[] wmf = File.ReadAllBytes(Path.Combine(Corpus, "wmf-shapes.wmf"));

        VectorImages.For(wmf).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(wmf);

        image.IsEmpty.ShouldBeFalse();
        image.Diagnostics.ShouldBeEmpty();

        // 8000 logical units at the placeable header's 2540 to the inch: exactly 80 mm.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(60, 0.01);

        Recorder sink = new();
        image.Draw(sink, new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(80), Length.FromMillimetres(60)));

        // A rectangle, an ellipse, a second rectangle and a pie fill; five outlines stroke;
        // one text run. Every family of record in the file has to survive for the picture to be
        // worth preferring over the raster fallback beside it.
        sink.Fills.Count.ShouldBe(4);
        sink.Strokes.Count.ShouldBe(5);
        sink.Runs.ShouldHaveSingleItem().Text.ShouldBe("Paperless WMF");

        // The picture fills its frame rather than sitting in a corner of it.
        sink.Ink.Width.Millimetres.ShouldBeGreaterThan(70);
    }

    [Fact]
    public void TheOdfPackageKeptTheMetafileRatherThanRasterisingIt()
    {
        string odt = Path.Combine(Corpus, "wmf-picture.odt");

        string name = Names(odt).Single(n => n.EndsWith(".wmf", StringComparison.OrdinalIgnoreCase));
        byte[] wmf = Entry(odt, name);

        VectorImages.For(wmf).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(wmf);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);

        // The PNG preview LibreOffice writes beside it is twenty times the size and fixed at one
        // resolution. That is what decoding the metafile avoids.
        string preview = Names(odt).Single(n => n.StartsWith("Pictures/", StringComparison.Ordinal)
            && n.EndsWith(".png", StringComparison.OrdinalIgnoreCase));
        Entry(odt, preview).Length.ShouldBeGreaterThan(wmf.Length * 10);
    }

    [Fact]
    public void TheDocxReferencesTheMetafileFromItsBlip()
    {
        string docx = Path.Combine(Corpus, "wmf-picture.docx");

        XElement blip = Xml(docx, "word/document.xml")
            .Descendants(XName.Get("blip", "http://schemas.openxmlformats.org/drawingml/2006/main"))
            .ShouldHaveSingleItem();

        // No SVG alternative here, so the blip's own relationship is the picture — which is the
        // ordinary case and the one BlipReference.Choose has to leave alone.
        BlipReference.Choice choice = BlipReference.Choose(blip);
        choice.IsVector.ShouldBeFalse();
        choice.RelationshipId.ShouldNotBeNull();

        Dictionary<string, string> targets = Relations(docx, "word/_rels/document.xml.rels");
        byte[] picture = Entry(docx, "word/" + targets[choice.RelationshipId!]);

        // The bytes decide, not the part name: this one is a metafile and the vector seam finds
        // it without anything having declared a media type.
        VectorImages.For(picture).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(picture);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);
    }

    [Fact]
    public void AMetafileIsFoundByItsBytesWhateverItsMediaTypeSays()
    {
        byte[] wmf = File.ReadAllBytes(Path.Combine(Corpus, "wmf-shapes.wmf"));

        // Mislabelled parts are unremarkable in real packages, so the sniff has to win.
        VectorImages.For(wmf).ShouldBeOfType<Wmf.WmfImageDecoder>();
        VectorImages.IsVectorMediaType("image/x-wmf").ShouldBeTrue();
        VectorImages.IsVectorMediaType("image/wmf; charset=binary").ShouldBeTrue();
    }

    private static IEnumerable<string> Names(string package)
    {
        using ZipArchive archive = ZipFile.OpenRead(package);
        return [.. archive.Entries.Select(e => e.FullName)];
    }

    private static byte[] Entry(string package, string name)
    {
        using ZipArchive archive = ZipFile.OpenRead(package);
        ZipArchiveEntry entry = archive.GetEntry(name).ShouldNotBeNull();

        using Stream stream = entry.Open();
        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }

    private static XElement Xml(string package, string name)
        => XElement.Parse(System.Text.Encoding.UTF8.GetString(Entry(package, name)));

    private static Dictionary<string, string> Relations(string package, string name)
    {
        XNamespace ns = "http://schemas.openxmlformats.org/package/2006/relationships";
        Dictionary<string, string> targets = new(StringComparer.Ordinal);

        foreach (XElement relationship in Xml(package, name).Elements(ns + "Relationship"))
        {
            if (relationship.Attribute("Id")?.Value is { } id
                && relationship.Attribute("Target")?.Value is { } target)
            {
                targets[id] = target;
            }
        }

        return targets;
    }

    private static string Locate()
    {
        DirectoryInfo? directory = new(AppContext.BaseDirectory);

        while (directory is not null)
        {
            string candidate = Path.Combine(directory.FullName, "tests", "corpus", "features");
            if (Directory.Exists(candidate)) return candidate;
            directory = directory.Parent;
        }

        throw new DirectoryNotFoundException("tests/corpus/features");
    }
}
