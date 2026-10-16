using System.Globalization;
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
/// A document's own fonts, from <c>svg:font-face-uri</c> through to the face a run is drawn with.
/// </summary>
/// <remarks>
/// <para>
/// The ODF twin of <see cref="PptxEmbeddedFontTests"/>, and it covers a gap that had been open
/// for as long as the ODF readers have existed: <c>git grep font-face-uri</c> over the sources
/// returned nothing, so an ODF document's embedded faces were never loaded. Six of the converted
/// corpus's 302 <c>.odp</c> carry one — <c>Sean Monogue.odp</c> embeds four
/// <c>Fonts/Font_Verdana_*.ttf</c> and <c>Ramp Up Campaign - French.odp</c> eight
/// <c>Font_Alegreya_Sans_*.ttf</c> — and neither family exists on this machine, so every run in
/// both was measured against DejaVu Sans while 26.2.4.2 embedded the document's own face in its
/// PDF.
/// </para>
/// <para>
/// Written end to end, and synthesised rather than added to the corpus, for the reason the deck
/// suite states: the payload has to be a face that <em>is</em> installed, so that "the embedded
/// face was used" and "the substitute was used" are told apart by a property of the face — being
/// monospaced — rather than by a path.
/// </para>
/// </remarks>
public class OdpEmbeddedFontTests
{
    /// <summary>The family the declaration states, which nothing on any machine has installed.</summary>
    private const string Declared = "Alegreya Sans Regular Bold";

    [Fact]
    public void AnEmbeddedFaceIsDrawnWithInsteadOfASubstitute()
    {
        InstalledFace payload = Monospaced();

        GlyphRun run = FirstRun(Packaged(regular: File.ReadAllBytes(payload.Path)), "deck.odp");

        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.FamilyName.ShouldBe(payload.FamilyName);
        face.IsFixedPitch.ShouldBeTrue();

        run.Font.RequestedFamily.ShouldBe(Declared);
    }

    [Fact]
    public void ADocumentThatEmbedsNothingForTheFamilyStillSubstitutes()
    {
        // The control, and the path 296 of the 302 converted `.odp` take. Without it this suite
        // would pass against a resolver that used an embedded face for every request it saw.
        GlyphRun run = FirstRun(Packaged(regular: null), "deck.odp");

        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.IsFixedPitch.ShouldBeFalse();
        face.FamilyName.ShouldNotBe(Monospaced().FamilyName);
    }

    /// <remarks>
    /// <c>xmloff</c> reads no style off the declaration at all —
    /// <c>XMLFontStyleContextFontFaceUri::SetAttribute</c> handles <c>xlink:href</c> and nothing
    /// else — so which of a family's files is the bold one is decided by that file's own
    /// <c>OS/2</c>. Here the two payloads are told apart by pitch, and the <em>bold</em> one is
    /// the proportional face: a reader that trusted <c>loext:font-weight</c> would pass this test
    /// only by accident, and this document deliberately writes that attribute the other way
    /// round.
    /// </remarks>
    [Fact]
    public void TheStyleIsReadOutOfTheFaceAndNotOffTheDeclaration()
    {
        InstalledFace mono = Monospaced();
        InstalledFace serifBold = SerifBold();

        byte[] document = Packaged(
            regular: File.ReadAllBytes(mono.Path),
            bold: File.ReadAllBytes(serifBold.Path),
            boldRun: true,
            lieAboutWeights: true);

        OpenTypeFace face = OpenTypeFace.ReadFile(FirstRun(document, "deck.odp").Font.FaceKey)
            .ShouldNotBeNull();

        face.FamilyName.ShouldBe(serifBold.FamilyName);
        face.IsFixedPitch.ShouldBeFalse();
    }

    [Fact]
    public void ADeclarationWithOnlyARegularFaceAnswersABoldRunWithIt()
    {
        // As LibreOffice does: it has one face registered for the family and emboldens it
        // synthetically rather than abandoning the author's typeface for a bold run.
        InstalledFace payload = Monospaced();

        byte[] document = Packaged(regular: File.ReadAllBytes(payload.Path), boldRun: true);

        OpenTypeFace face = OpenTypeFace.ReadFile(FirstRun(document, "deck.odp").Font.FaceKey)
            .ShouldNotBeNull();

        face.FamilyName.ShouldBe(payload.FamilyName);
    }

    /// <remarks>
    /// Flat ODF has no package to put a part in, so the bytes travel as an
    /// <c>office:binary-data</c> child of the <c>svg:font-face-uri</c> —
    /// <c>XMLFontStyleContextFontFaceUri::createFastChildContext</c> takes it through
    /// <c>XMLBase64ImportContext</c>. The same face, reached the other way.
    /// </remarks>
    [Fact]
    public void AFlatDocumentCarriesItsFaceAsBase64()
    {
        InstalledFace payload = Monospaced();

        GlyphRun run = FirstRun(
            Encoding.UTF8.GetBytes(Flat(Convert.ToBase64String(File.ReadAllBytes(payload.Path)))),
            "deck.fodp");

        OpenTypeFace face = OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull();
        face.IsFixedPitch.ShouldBeTrue();
        face.FamilyName.ShouldBe(payload.FamilyName);
    }

    [Fact]
    public void APartThatIsNotAFontIsReportedAndDoesNotStopTheDocument()
    {
        byte[] document = Packaged(
            regular: Encoding.UTF8.GetBytes("this is not a font, it is a sentence"));

        using IDocument read = new PresentationReader().Read(
            DocumentSource.FromStream(new MemoryStream(document), "deck.odp"));

        GlyphRun run = FirstRun(read);

        OpenTypeFace.ReadFile(run.Font.FaceKey).ShouldNotBeNull().IsFixedPitch.ShouldBeFalse();
        read.Diagnostics.ShouldContain(d => d.Code == "PL2261");
    }

    // ------------------------------------------------------------------------------ fixtures

    private static InstalledFace Monospaced()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        return index.Best("DejaVu Sans Mono", 400, italic: false)
            ?? index.Best("Liberation Mono", 400, italic: false)
            ?? index.Faces.First(face => face.IsFixedPitch);
    }

    private static InstalledFace SerifBold()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        return index.Best("Liberation Serif", 700, italic: false)
            ?? index.Best("DejaVu Serif", 700, italic: false)
            ?? index.Faces.First(face => !face.IsFixedPitch);
    }

    private static GlyphRun FirstRun(byte[] document, string name)
    {
        using IDocument read = new PresentationReader().Read(
            DocumentSource.FromStream(new MemoryStream(document), name));

        return FirstRun(read);
    }

    private static GlyphRun FirstRun(IDocument document)
    {
        RecordingDrawingSink sink = new();
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        pages.Count.ShouldBe(1);
        pages[0].Draw(sink);

        return sink.Pages[0].Runs.Select(drawn => drawn.Run).First();
    }

    /// <summary>The declarations, the shape and the one paragraph, shared by both containers.</summary>
    private static string Content(string declarations, bool boldRun)
        => "<office:font-face-decls>" + declarations + "</office:font-face-decls>"
           + "<office:automatic-styles>"
           + "<style:page-layout style:name=\"PL1\"><style:page-layout-properties"
           + " fo:page-width=\"25.4cm\" fo:page-height=\"19.05cm\" fo:margin-top=\"0cm\""
           + " fo:margin-bottom=\"0cm\" fo:margin-left=\"0cm\" fo:margin-right=\"0cm\"/>"
           + "</style:page-layout>"
           + "<style:style style:name=\"dp1\" style:family=\"drawing-page\">"
           + "<style:drawing-page-properties draw:fill=\"none\"/></style:style>"
           + "<style:style style:name=\"gr1\" style:family=\"graphic\">"
           + "<style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\""
           + " draw:auto-grow-height=\"false\" draw:auto-grow-width=\"false\"/></style:style>"
           + "<style:style style:name=\"P1\" style:family=\"paragraph\"><style:text-properties"
           + $" fo:font-size=\"24pt\" style:font-name=\"{Declared}\""
           + (boldRun ? " fo:font-weight=\"bold\"" : string.Empty)
           + "/></style:style>"
           + "</office:automatic-styles>"
           + "<office:master-styles><style:master-page style:name=\"Default\""
           + " style:page-layout-name=\"PL1\" draw:style-name=\"dp1\"/></office:master-styles>"
           + "<office:body><office:presentation>"
           + "<draw:page draw:name=\"one\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">"
           + "<draw:frame draw:style-name=\"gr1\" draw:name=\"box\" draw:layer=\"layout\""
           + " svg:x=\"2cm\" svg:y=\"2cm\" svg:width=\"18cm\" svg:height=\"4cm\">"
           + "<draw:text-box><text:p text:style-name=\"P1\">Restez en Forme</text:p></draw:text-box>"
           + "</draw:frame></draw:page>"
           + "</office:presentation></office:body>";

    /// <summary>One <c>style:font-face</c> naming its faces by package part.</summary>
    private static string Declarations(bool regular, bool bold, bool lieAboutWeights)
    {
        if (!regular && !bold) return $"<style:font-face style:name=\"{Declared}\" svg:font-family=\"{Declared}\"/>";

        StringBuilder uris = new();
        if (regular)
        {
            uris.Append(Uri("Fonts/font1.ttf", lieAboutWeights ? "bold" : "normal"));
        }

        if (bold)
        {
            uris.Append(Uri("Fonts/font2.ttf", lieAboutWeights ? "normal" : "bold"));
        }

        return $"<style:font-face style:name=\"{Declared}\" svg:font-family=\"{Declared}\">"
               + "<svg:font-face-src>" + uris + "</svg:font-face-src></style:font-face>";

        static string Uri(string href, string weight)
            => $"<svg:font-face-uri xlink:href=\"{href}\" xlink:type=\"simple\""
               + $" loext:font-style=\"normal\" loext:font-weight=\"{weight}\">"
               + "<svg:font-face-format svg:string=\"truetype\"/></svg:font-face-uri>";
    }

    private static string Flat(string base64)
        => "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           + "<office:document " + Namespaces
           + " office:version=\"1.3\""
           + " office:mimetype=\"application/vnd.oasis.opendocument.presentation\">"
           + Content(
               $"<style:font-face style:name=\"{Declared}\" svg:font-family=\"{Declared}\">"
               + "<svg:font-face-src><svg:font-face-uri xlink:type=\"simple\">"
               + "<svg:font-face-format svg:string=\"truetype\"/>"
               + $"<office:binary-data>{base64}</office:binary-data>"
               + "</svg:font-face-uri></svg:font-face-src></style:font-face>",
               boldRun: false)
           + "</office:document>";

    private static byte[] Packaged(
        byte[]? regular,
        byte[]? bold = null,
        bool boldRun = false,
        bool lieAboutWeights = false)
    {
        MemoryStream buffer = new();

        using (ZipArchive zip = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(zip, "mimetype",
                Encoding.UTF8.GetBytes("application/vnd.oasis.opendocument.presentation"));

            if (regular is not null) Write(zip, "Fonts/font1.ttf", regular);
            if (bold is not null) Write(zip, "Fonts/font2.ttf", bold);

            Write(zip, "META-INF/manifest.xml", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\"?><manifest:manifest"
                + " xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\""
                + " manifest:version=\"1.3\">"
                + "<manifest:file-entry manifest:full-path=\"/\""
                + " manifest:media-type=\"application/vnd.oasis.opendocument.presentation\"/>"
                + "<manifest:file-entry manifest:full-path=\"content.xml\""
                + " manifest:media-type=\"text/xml\"/>"
                + "</manifest:manifest>"));

            Write(zip, "content.xml", Encoding.UTF8.GetBytes(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                + "<office:document-content " + Namespaces + " office:version=\"1.3\">"
                + Content(
                    Declarations(regular is not null, bold is not null, lieAboutWeights),
                    boldRun)
                + "</office:document-content>"));
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive zip, string name, byte[] content)
    {
        using Stream entry = zip.CreateEntry(name, CompressionLevel.NoCompression).Open();
        entry.Write(content, 0, content.Length);
    }

    private const string Namespaces =
        "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" "
        + "xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" "
        + "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" "
        + "xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" "
        + "xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" "
        + "xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" "
        + "xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" "
        + "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
        + "xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\"";

    static OdpEmbeddedFontTests() => CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;
}
