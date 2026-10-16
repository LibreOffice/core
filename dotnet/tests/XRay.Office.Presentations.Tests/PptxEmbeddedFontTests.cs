using System.Buffers.Binary;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core;
using XRay.Office.Core.Documents;

using XRay.Office.Core.Graphics;
using XRay.Office.Presentations;
using XRay.Office.TestKit;
using XRay.Office.Text.Fonts;
using Shouldly;
using Xunit;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A deck's own fonts, from <c>p:embeddedFontLst</c> through to the face a run is drawn with.
/// </summary>
/// <remarks>
/// <para>
/// Written end to end rather than against the reader, because the defect this covers was never
/// in a reader: <c>Ramp Up Campaign - French.pptx</c> carried three usable faces and every run in
/// it was drawn in DejaVu Sans, which is wider — so every block gained a line, five overprinted
/// one another, and the last paragraph was clipped off the slide. The gate reported that as 19
/// missing words. A test on the container alone would have passed throughout.
/// </para>
/// <para>
/// The deck is synthesised here rather than added to the corpus for one reason worth stating: the
/// payload has to be a face that is <em>installed</em>, so that "the embedded face was used" and
/// "the substitute was used" can be told apart by a property of the face rather than by a path. A
/// monospaced payload under a family name nothing has installed does exactly that — substitution
/// lands on a proportional face, and the embedded one does not.
/// </para>
/// </remarks>
public class PptxEmbeddedFontTests
{
    /// <summary>The family name the deck declares, which nothing on any machine has installed.</summary>
    private const string Declared = "Alegreya Sans Regular Bold";

    [Fact]
    public void AnEmbeddedFaceIsDrawnWithInsteadOfASubstitute()
    {
        InstalledFace payload = Monospaced();

        GlyphRun run = FirstRun(Deck(regular: File.ReadAllBytes(payload.Path)));

        // The deck's own face, identified by what it is rather than by where it was written.
        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.FamilyName.ShouldBe(payload.FamilyName);
        face.IsFixedPitch.ShouldBeTrue();

        // And the request is still recorded as what the document asked for, so a comparison can
        // still see which name was served.
        run.Font.RequestedFamily.ShouldBe(Declared);
    }

    [Fact]
    public void ADeckThatEmbedsNothingForTheFamilyStillSubstitutes()
    {
        // The control, and the path all but three documents in the slides track take. Without it
        // this suite would pass just as well against a resolver that used an embedded face for
        // every request it ever saw.
        GlyphRun run = FirstRun(Deck(regular: null));

        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.IsFixedPitch.ShouldBeFalse();
        face.FamilyName.ShouldNotBe(Monospaced().FamilyName);
    }

    [Fact]
    public void ABoldRunTakesTheBoldEntryAndAPlainOneDoesNot()
    {
        InstalledFace mono = Monospaced();
        InstalledFace serif = Serif();

        byte[] deck = Deck(
            regular: File.ReadAllBytes(mono.Path),
            bold: File.ReadAllBytes(serif.Path),
            boldRun: true);

        // One `p:embeddedFont` carries up to four styles under one name, and the run picks among
        // them the way `SystemFontIndex.Best` picks among installed faces.
        OpenTypeFace face = OpenTypeFace.ReadFile(FirstRun(deck).Font.FaceKey).ShouldNotBeNull();
        face.FamilyName.ShouldBe(serif.FamilyName);

        byte[] plain = Deck(
            regular: File.ReadAllBytes(mono.Path),
            bold: File.ReadAllBytes(serif.Path),
            boldRun: false);

        OpenTypeFace plainFace = OpenTypeFace.ReadFile(FirstRun(plain).Font.FaceKey).ShouldNotBeNull();
        plainFace.FamilyName.ShouldBe(mono.FamilyName);
    }

    [Fact]
    public void AnEntryWithOnlyARegularFaceAnswersABoldRunWithIt()
    {
        // Which is what LibreOffice does: it has one face registered for the family and emboldens
        // it synthetically rather than abandoning the author's typeface for a bold run.
        InstalledFace payload = Monospaced();

        byte[] deck = Deck(regular: File.ReadAllBytes(payload.Path), boldRun: true);

        OpenTypeFace face = OpenTypeFace.ReadFile(FirstRun(deck).Font.FaceKey).ShouldNotBeNull();
        face.FamilyName.ShouldBe(payload.FamilyName);
    }

    [Fact]
    public void ACompressedEntryFallsBackToSubstitutionRatherThanLosingTheText()
    {
        // MicroType Express, which this reader does not decode and 18 of the slides track's 28
        // embedded parts use. The face is unavailable; the text is not.
        byte[] deck = Deck(
            regular: File.ReadAllBytes(Monospaced().Path),
            compressed: true);

        GlyphRun run = FirstRun(deck);

        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.IsFixedPitch.ShouldBeFalse();
        run.Text.ShouldContain("Restez en Forme");
    }

    [Fact]
    public void APartThatIsNotAFontIsReportedAndDoesNotStopTheDeck()
    {
        byte[] deck = Deck(regular: Encoding.UTF8.GetBytes("this is not a font, it is a sentence"));

        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromStream(new MemoryStream(deck), "deck.pptx"));

        GlyphRun run = FirstRun(document);

        OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull().IsFixedPitch.ShouldBeFalse();
        document.Diagnostics.ShouldContain(d => d.Code == "PL2260");
    }

    // ------------------------------------------------------------------------------ fixtures

    private static InstalledFace Monospaced()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        return index.Best("DejaVu Sans Mono", 400, italic: false)
            ?? index.Best("Liberation Mono", 400, italic: false)
            ?? index.Faces.First(face => face.IsFixedPitch);
    }

    private static InstalledFace Serif()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        return index.Best("Liberation Serif", 400, italic: false)
            ?? index.Best("DejaVu Serif", 400, italic: false)
            ?? index.Faces.First(face => !face.IsFixedPitch);
    }

    private static GlyphRun FirstRun(byte[] deck)
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromStream(new MemoryStream(deck), "deck.pptx"));

        return FirstRun(document);
    }

    private static GlyphRun FirstRun(IDocument document)
    {
        RecordingDrawingSink sink = new();
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBe(1);
        pages[0].Draw(sink);

        return sink.Pages[0].Runs.Select(drawn => drawn.Run).First();
    }

    /// <summary>
    /// A one-slide deck naming <see cref="Declared"/>, optionally embedding faces for it.
    /// </summary>
    private static byte[] Deck(
        byte[]? regular,
        byte[]? bold = null,
        bool boldRun = false,
        bool compressed = false)
    {
        MemoryStream buffer = new();

        using (ZipArchive zip = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            List<string> fontRels = [];
            List<string> fontElements = [];

            if (regular is not null)
            {
                Write(zip, "ppt/fonts/font1.fntdata", Container(regular, compressed));
                fontRels.Add(Rel("rId9", "font", "fonts/font1.fntdata"));
                fontElements.Add("<p:regular r:id=\"rId9\"/>");
            }

            if (bold is not null)
            {
                Write(zip, "ppt/fonts/font2.fntdata", Container(bold, compressed: false));
                fontRels.Add(Rel("rId10", "font", "fonts/font2.fntdata"));
                fontElements.Add("<p:bold r:id=\"rId10\"/>");
            }

            string list = fontElements.Count == 0
                ? string.Empty
                : "<p:embeddedFontLst><p:embeddedFont>"
                  + $"<p:font typeface=\"{Declared}\" charset=\"0\"/>"
                  + string.Concat(fontElements)
                  + "</p:embeddedFont></p:embeddedFontLst>";

            Write(zip, "[Content_Types].xml", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                + "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                + "<Default Extension=\"fntdata\" ContentType=\"application/x-fontdata\"/>"
                + "<Override PartName=\"/ppt/presentation.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml\"/>"
                + "<Override PartName=\"/ppt/slides/slide1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>"
                + "</Types>"));

            Write(zip, "_rels/.rels", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                + Rel("rId1", "officeDocument", "ppt/presentation.xml")
                + "</Relationships>"));

            Write(zip, "ppt/presentation.xml", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\"?><p:presentation "
                + "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
                + "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                + "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                + "<p:sldIdLst><p:sldId id=\"256\" r:id=\"rId2\"/></p:sldIdLst>"
                + list
                + "<p:sldSz cx=\"9144000\" cy=\"6858000\"/>"
                + "<p:notesSz cx=\"6858000\" cy=\"9144000\"/>"
                + "</p:presentation>"));

            Write(zip, "ppt/_rels/presentation.xml.rels", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                + Rel("rId2", "slide", "slides/slide1.xml")
                + string.Concat(fontRels)
                + "</Relationships>"));

            Write(zip, "ppt/slides/slide1.xml", Encoding.UTF8.GetBytes(Slide(boldRun)));
        }

        buffer.Position = 0;
        return buffer.ToArray();
    }

    private static string Slide(bool bold) =>
        "<?xml version=\"1.0\"?><p:sld "
        + "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
        + "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"
        + "<p:cSld><p:spTree>"
        + "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
        + "<p:grpSpPr/>"
        + "<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Body\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>"
        + "<p:spPr><a:xfrm><a:off x=\"457200\" y=\"457200\"/><a:ext cx=\"8229600\" cy=\"2743200\"/></a:xfrm>"
        + "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr>"
        + "<p:txBody><a:bodyPr/><a:lstStyle/><a:p>"
        + $"<a:r><a:rPr lang=\"fr-FR\" sz=\"2400\"{(bold ? " b=\"1\"" : string.Empty)}>"
        + $"<a:latin typeface=\"{Declared}\"/></a:rPr>"
        + "<a:t>Soyez Prets - Restez en Forme</a:t></a:r>"
        + "</a:p></p:txBody></p:sp>"
        + "</p:spTree></p:cSld></p:sld>";

    private static string Rel(string id, string type, string target)
        => $"<Relationship Id=\"{id}\" "
           + $"Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/{type}\" "
           + $"Target=\"{target}\"/>";

    private static void Write(ZipArchive zip, string name, byte[] bytes)
    {
        using Stream entry = zip.CreateEntry(name).Open();
        entry.Write(bytes);
    }

    /// <summary>
    /// Wraps a face in the EOT container a <c>.fntdata</c> part actually holds.
    /// </summary>
    /// <remarks>
    /// Written out here as well as in <c>EmbeddedOpenTypeTests</c>, deliberately: this file must
    /// compile and run against a tree that has no EOT reader in it, which is how these tests were
    /// shown to fail before the fix rather than merely to be new.
    /// </remarks>
    private static byte[] Container(byte[] font, bool compressed)
    {
        MemoryStream buffer = new();
        BinaryWriter writer = new(buffer);

        writer.Write(0u);                                    // EOTSize, patched below
        writer.Write((uint)font.Length);                     // FontDataSize
        writer.Write(0x0002_0002u);                          // Version
        writer.Write(compressed ? 0x0000_0005u : 0u);        // Flags: SUBSET | TTCOMPRESSED
        writer.Write(new byte[10]);                          // FontPANOSE
        writer.Write((byte)0);                               // Charset
        writer.Write((byte)0);                               // Italic
        writer.Write(400u);                                  // Weight
        writer.Write((ushort)0);                             // fsType
        writer.Write((ushort)0x504C);                        // MagicNumber
        writer.Write(new byte[16 + 8 + 4 + 16]);             // ranges, checksum, reserved

        foreach (string value in (string[])[Declared, "Regular", "1.0", Declared, string.Empty])
        {
            byte[] utf16 = Encoding.Unicode.GetBytes(value);
            writer.Write((ushort)0);
            writer.Write((ushort)utf16.Length);
            writer.Write(utf16);
        }

        writer.Write(0u);                                    // RootStringCheckSum
        writer.Write(0u);                                    // EUDCCodePage
        writer.Write((ushort)0);                             // Padding
        writer.Write((ushort)0);                             // SignatureSize
        writer.Write(0u);                                    // EUDCFlags
        writer.Write(0u);                                    // EUDCFontSize
        writer.Write(font);
        writer.Flush();

        byte[] bytes = buffer.ToArray();
        BinaryPrimitives.WriteUInt32LittleEndian(bytes, (uint)bytes.Length);
        return bytes;
    }
}
