using System.IO.Compression;
using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The corpus documents that carry a real EMF, read the way a reader would read them.
/// </summary>
/// <remarks>
/// <para>
/// Three fixtures, each settling a different question. <c>emf-shapes.emf</c> is hand-built so
/// that a test can name the millimetre it expects; <c>emf-paths.emf</c> is what LibreOffice's
/// own EMF export looks like, which is almost entirely path records and nothing else; and
/// <c>wmf-embedded-emf.wmf</c> is the file that settles the dual-format question, because it is
/// a WMF carrying the whole picture a second time as an EMF.
/// </para>
/// <para>
/// The packages are opened directly rather than through <c>Paperless.Containers</c>: what is
/// under test is the picture, and pulling the package readers in would make a failure here
/// ambiguous between two layers.
/// </para>
/// </remarks>
public sealed class CorpusEmfTests
{
    private static readonly string Corpus = Locate();

    [Fact]
    public void TheHandBuiltMetafileDrawsEveryKindOfRecordItHolds()
    {
        byte[] emf = File.ReadAllBytes(Path.Combine(Corpus, "emf-shapes.emf"));

        VectorImages.For(emf).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(emf);

        image.IsEmpty.ShouldBeFalse();
        image.IsTruncated.ShouldBeFalse();

        // A frame of 8000 by 6000 hundredths of a millimetre against a reference device of
        // 8000 by 6000 pixels to 80 by 60 mm: one logical unit is exactly 1/100 mm.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(60, 0.01);

        Recorder sink = new();
        image.Draw(sink, new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(80), Length.FromMillimetres(60)));

        // A rectangle, an ellipse, a filled path and a gradient fill; three outlines stroke —
        // the rectangle's, the ellipse's and the dashed polyline's — plus the path's own.
        sink.Fills.Count.ShouldBe(4);
        sink.Strokes.Count.ShouldBe(4);
        sink.Runs.ShouldHaveSingleItem().Text.ShouldBe("Paperless EMF");

        // The picture fills its frame rather than sitting in a corner of it.
        sink.Ink.Width.Millimetres.ShouldBeGreaterThan(65);
    }

    [Fact]
    public void LibreOfficesOwnExportIsAlmostEntirelyPathRecords()
    {
        byte[] emf = File.ReadAllBytes(Path.Combine(Corpus, "emf-paths.emf"));

        VectorImage image = VectorImages.Decode(emf);

        image.IsEmpty.ShouldBeFalse();
        image.Diagnostics.ShouldBeEmpty();

        // 190 x 277 mm, from rclFrame — which is what the header states and not what the 718 by
        // 1047 pixel reference device would give on its own.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(190, 0.05);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(277, 0.05);

        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        // The whole drawing arrives through BeginPath/EndPath/FillPath: LibreOffice converts
        // every shape it exports into a path, which is why this fixture is worth keeping
        // alongside the hand-built one that exercises the shape records instead.
        sink.Fills.ShouldNotBeEmpty();
        sink.Runs.ShouldHaveSingleItem().Text.ShouldBe("Paperless EMF");
    }

    [Fact]
    public void AWmfCarryingACompleteEmfReplaysTheEmfAndSaysSo()
    {
        byte[] wmf = File.ReadAllBytes(Path.Combine(Corpus, "wmf-embedded-emf.wmf"));
        byte[] emf = File.ReadAllBytes(Path.Combine(Corpus, "emf-paths.emf"));

        // 14 032 bytes of EMF inside an 18 276-byte WMF: 77 % of the file is the second copy.
        wmf.Length.ShouldBeGreaterThan(emf.Length);

        VectorImage image = VectorImages.Decode(wmf);

        Diagnostic chosen = image.Diagnostics.ShouldHaveSingleItem();
        chosen.Code.ShouldBe("PL6030");
        chosen.Message.ShouldContain("the EMF was drawn");

        // The two representations are the same picture, so replaying either gives the same
        // extent — which is what makes replaying both a doubling rather than a supplement.
        VectorImage direct = VectorImages.Decode(emf);
        image.IntrinsicSize.ShouldBe(direct.IntrinsicSize);
        image.Content.Count.ShouldBe(direct.Content.Count);
    }

    [Fact]
    public void TheOdfPackageKeptTheMetafileRatherThanRasterisingIt()
    {
        string odt = Path.Combine(Corpus, "emf-picture.odt");

        string name = Names(odt).Single(n => n.EndsWith(".emf", StringComparison.OrdinalIgnoreCase));
        byte[] emf = Entry(odt, name);

        VectorImages.For(emf).ShouldNotBeNull();

        VectorImage image = VectorImages.Decode(emf);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.01);

        // The PNG preview LibreOffice writes beside it is ten times the size and fixed at one
        // resolution. That is what decoding the metafile avoids.
        string preview = Names(odt).Single(n => n.StartsWith("Pictures/", StringComparison.Ordinal)
            && n.EndsWith(".png", StringComparison.OrdinalIgnoreCase));
        Entry(odt, preview).Length.ShouldBeGreaterThan(emf.Length * 10);
    }

    [Fact]
    public void TheDocxStoresAnEmfUnderAWmfPartNameThatNothingDeclaresATypeFor()
    {
        string docx = Path.Combine(Corpus, "emf-picture.docx");

        XElement blip = Xml(docx, "word/document.xml")
            .Descendants(XName.Get("blip", "http://schemas.openxmlformats.org/drawingml/2006/main"))
            .ShouldHaveSingleItem();

        BlipReference.Choice choice = BlipReference.Choose(blip);
        choice.RelationshipId.ShouldNotBeNull();

        Dictionary<string, string> targets = Relations(docx, "word/_rels/document.xml.rels");
        string target = targets[choice.RelationshipId!];

        // The sharpest form of the argument for sniffing: LibreOffice writes a genuine EMF into
        // a part called ".wmf", and [Content_Types].xml declares no default for that extension
        // at all. Neither the name nor the declared type identifies the picture; only the bytes
        // do, and the first forty-four of them say " EMF".
        target.ShouldEndWith(".wmf", Case.Insensitive);
        Xml(docx, "[Content_Types].xml").ToString().ShouldNotContain("\"wmf\"");

        byte[] picture = Entry(docx, "word/" + target);
        VectorImages.For(picture).ShouldBeOfType<Emf.EmfImageDecoder>();

        VectorImage image = VectorImages.Decode(picture);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Millimetres.ShouldBe(80, 0.05);
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
        Dictionary<string, string> targets = [];

        foreach (XElement relation in Xml(package, name).Elements())
        {
            if (relation.Attribute("Id")?.Value is { } id && relation.Attribute("Target")?.Value is { } target)
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

        throw new DirectoryNotFoundException("The corpus directory could not be found.");
    }
}
