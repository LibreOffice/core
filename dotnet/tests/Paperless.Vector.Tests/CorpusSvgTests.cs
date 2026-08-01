using System.IO.Compression;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The two corpus documents that carry a real SVG, read as a document would read them.
/// </summary>
/// <remarks>
/// <para>
/// The unit tests decode SVG source held in a string; this one decodes it out of a package,
/// which is where it actually lives. It also settles the question the corpus was built to
/// settle: whether LibreOffice's DOCX export keeps the vector or substitutes the raster
/// fallback. It keeps it — 769 bytes of SVG beside a 3 803-byte PNG — so preferring the
/// vector is a real win rather than a hypothetical one.
/// </para>
/// <para>
/// The ZIP is opened directly rather than through <c>Paperless.Containers</c>. What is under
/// test is the picture, and pulling the package readers in would make a failure here
/// ambiguous between two layers.
/// </para>
/// </remarks>
public sealed class CorpusSvgTests
{
    private const string Relationships = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    private static readonly string Corpus = Locate();

    [Fact]
    public void TheOdfPackageCarriesAnSvgThatDraws()
    {
        byte[] svg = Entry(Path.Combine(Corpus, "svg-picture.odt"), "Pictures/logo.svg");

        VectorImages.For(svg).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(svg);

        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(40, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(20, 0.01);

        Recorder sink = new();
        image.Draw(sink, new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(40), Length.FromMillimetres(20)));

        // A rounded rectangle, a clipped circle, an arc and a wordmark: fills, a stroke and
        // text, all of which have to survive for the picture to be worth preferring.
        sink.Fills.Count.ShouldBeGreaterThanOrEqualTo(2);
        sink.Strokes.ShouldNotBeEmpty();
        sink.Runs.ShouldHaveSingleItem().Text.ShouldBe("PAPERLESS");

        // The picture fills its frame rather than sitting in a corner of it.
        sink.Ink.Width.Millimetres.ShouldBeGreaterThan(35);
    }

    [Fact]
    public void TheDocxKeptTheVectorBesideItsRasterFallback()
    {
        string docx = Path.Combine(Corpus, "svg-picture.docx");

        XElement blip = Xml(docx, "word/document.xml")
            .Descendants(XName.Get("blip", "http://schemas.openxmlformats.org/drawingml/2006/main"))
            .ShouldHaveSingleItem();

        BlipReference.Choice choice = BlipReference.Choose(blip);
        choice.IsVector.ShouldBeTrue();
        choice.FallbackRelationshipId.ShouldNotBeNull();

        // Resolve both through the part relationships, exactly as a reader would.
        Dictionary<string, string> targets = Relations(docx, "word/_rels/document.xml.rels");

        byte[] vector = Entry(docx, "word/" + targets[choice.RelationshipId!]);
        byte[] raster = Entry(docx, "word/" + targets[choice.FallbackRelationshipId!]);

        VectorImages.For(vector).ShouldNotBeNull();
        VectorImages.For(raster).ShouldBeNull();

        // The fallback is five times the size and fixed at one resolution; that is what
        // preferring the vector avoids.
        raster.Length.ShouldBeGreaterThan(vector.Length);

        VectorImage image = VectorImages.Decode(vector);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(40, 0.01);
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

    /// <summary>
    /// The corpus directory, found by walking up from the test binary.
    /// </summary>
    /// <remarks>
    /// The tests run from <c>bin/Debug/net10.0/linux-x64</c>, and the corpus is not copied
    /// beside them — it is several hundred files and copying it per project would cost more
    /// than the walk.
    /// </remarks>
    private static string Locate()
    {
        DirectoryInfo? directory = new(AppContext.BaseDirectory);

        while (directory is not null)
        {
            string candidate = Path.Combine(directory.FullName, "tests", "corpus", "features");
            if (Directory.Exists(candidate)) return candidate;

            directory = directory.Parent;
        }

        throw new DirectoryNotFoundException("The test corpus could not be found above the test binary.");
    }
}
