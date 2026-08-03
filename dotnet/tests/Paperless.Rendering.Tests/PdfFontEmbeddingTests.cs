using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// Every face a rendered PDF references must carry a font program in the same file.
/// </summary>
/// <remarks>
/// <para>
/// The instrument this suite did not have. Every other check on a rendered document looks at
/// <em>where</em> the ink went — page counts, pen positions, glyph counts, extracted words — and a
/// face that is named but not embedded moves none of them. The layout is right, the pen is right,
/// <c>pdftotext</c> gives the right words, and a reader opening the file draws whatever it happens
/// to have under that name, or tofu. That is a purely visual defect with no numerical symptom, and
/// it survived in two of the three families for as long as those families could render at all.
/// </para>
/// <para>
/// What it was: <c>SlideTextLayout</c> and <c>SheetBandText</c> rebuilt the
/// <c>FontReference</c> the PDF writer is handed with <c>FaceKey = FamilyName</c>, throwing away
/// the resolver's own key — which is the font <em>file's path</em>
/// (<c>InstalledFace.FaceKey</c>, <c>SystemFontResolver.cs:22</c>). <c>FileFontProvider</c> opens
/// that key as a path, so a family name loads nothing, and <c>PdfFontCatalogue</c> then writes a
/// <c>/FontDescriptor</c> with no <c>/FontFile2</c>. The word-processing family had the same hole
/// in its list labels for DOC, DOCX and RTF, where the reference was resolved and simply not
/// carried onto the label.
/// </para>
/// <para>
/// Measured before the fix, with <c>pdffonts</c>: <c>deck-features.pptx</c> reported
/// <c>AAAAAA+LiberationSans … emb no</c> and <c>BAAAAA+OpenSymbol … emb no</c> while its text
/// extracted at 43 of 43 words matching LibreOffice; <c>sheet-features.ods</c> embedded its two
/// cell faces and not its header's; <c>word-features.docx</c> embedded six faces and not the two
/// its list labels are drawn in. Forty-one such rows across the corpus, in forty-one files whose
/// page and word counts were all correct.
/// </para>
/// <para>
/// The assertion is deliberately structural rather than a count of embedded fonts: a count would
/// have to be revised whenever a document's faces change, and would pass a file that embedded one
/// face twice and another not at all. Descriptor by descriptor is the question a reader asks.
/// </para>
/// </remarks>
public sealed partial class PdfFontEmbeddingTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    /// <summary>
    /// One document per family and per reader, since the reference is built by the reader.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both a deck and a sheet with a chart, because a chart's labels are laid out through the
    /// same two helpers and were unembedded for the same reason; and the three word-processing
    /// binaries separately, because DOC, DOCX and RTF each build their list labels in their own
    /// reader and each had to be fixed on its own.
    /// </para>
    /// <para>
    /// <strong>A named trap.</strong> <c>word-features.doc</c> and <c>word-features.docx</c> both
    /// render to <c>word-features.pdf</c> if a test writes to a directory named after the stem —
    /// which is how a sweep silently measures one file twice and reports the other's result. This
    /// test renders into memory and never names a file, which is the cheapest way not to have the
    /// problem; the corpus sweep in <c>tests/corpus/render-sweep.txt</c> has the same warning at
    /// the top and it has cost three agents an hour each.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("deck-features.pptx")]
    [InlineData("slides-features.odp")]
    [InlineData("ppt-features.ppt")]
    [InlineData("chart-bar-deck.pptx")]
    [InlineData("sheet-features.ods")]
    [InlineData("sheet-ooxml-features.xlsx")]
    [InlineData("xls-features.xls")]
    [InlineData("chart-bar-sheet.xlsx")]
    [InlineData("word-features.docx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    [InlineData("text-features.odt")]
    public void EveryFaceAPdfReferencesIsEmbeddedInIt(string fileName)
    {
        PdfFile pdf = PdfFile.Parse(Render(fileName));
        List<(string Name, bool Embedded)> faces = Faces(pdf);

        faces.ShouldNotBeEmpty($"{fileName}: rendered no text at all, so this proves nothing");

        string[] missing = [.. faces.Where(face => !face.Embedded).Select(face => face.Name)];

        missing.ShouldBeEmpty(
            $"{fileName}: {missing.Length} of {faces.Count} faces are referenced with no font "
            + $"program — {string.Join(", ", missing)}. A reader draws these in whatever it has "
            + "under that name, or in nothing. The cause is a FontReference whose FaceKey names "
            + "a family instead of the resolver's key, which is the font file's path.");
    }

    /// <summary>
    /// A substituted face is embedded too, which is the case the fix is really about.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>deck-features.pptx</c> asks for <em>Arial</em> and <em>Symbol</em> and neither is
    /// installed on a Linux machine, so <c>SystemFontResolver</c> substitutes Liberation Sans and
    /// OpenSymbol through LibreOffice's own chain. Embedding the substitute is the correct answer
    /// — the original cannot be embedded because its bytes are not here — and it is strictly
    /// better than naming the substitute and embedding nothing, because a reader that <em>does</em>
    /// have Arial would then draw Arial at Liberation Sans' advances.
    /// </para>
    /// <para>
    /// It also pins the direction of the fix. Rebuilding the reference from the loaded
    /// <c>OpenTypeFace</c> got the substituted <em>family</em> right, which is why the deck's PDF
    /// said <c>LiberationSans</c> all along; what it could not get was the file, because an
    /// <c>OpenTypeFace</c> is a parsed table directory and has no memory of where it was read
    /// from.
    /// </para>
    /// </remarks>
    [Fact]
    public void ASubstitutedFaceIsEmbeddedUnderTheNameItWasSubstitutedFor()
    {
        PdfFile pdf = PdfFile.Parse(Render("deck-features.pptx"));
        List<(string Name, bool Embedded)> faces = Faces(pdf);

        // Neither family the deck names survives resolution here, so seeing either would mean the
        // machine has changed under the test rather than that the writer has.
        faces.ShouldNotContain(face => face.Name.Contains("Arial", StringComparison.Ordinal));
        faces.ShouldNotContain(face => face.Name.Contains("Symbol,", StringComparison.Ordinal));

        faces.ShouldAllBe(face => face.Embedded);
        pdf.FontPrograms().Count.ShouldBe(faces.Count, "one program per referenced face");
    }

    /// <summary>
    /// A deck's bullet is embedded, which is the one face nothing else in the deck uses.
    /// </summary>
    /// <remarks>
    /// Worth its own assertion because an outline marker is resolved on its own path —
    /// <c>SlideTextLayout.EmitMarker</c> rather than <c>Emit</c> — and a fix to the run path alone
    /// would leave every bullet in every deck unembedded while the sweep above still passed for
    /// the body text. It is also the face a reader is least likely to have: a missing Liberation
    /// Sans is substituted invisibly, and a missing OpenSymbol is a row of empty boxes where the
    /// bullets should be.
    /// </remarks>
    [Fact]
    public void ADecksOutlineBulletIsEmbedded()
    {
        List<(string Name, bool Embedded)> faces = Faces(PdfFile.Parse(Render("deck-features.pptx")));

        (string Name, bool Embedded) bullet = faces
            .Where(face => face.Name.Contains("OpenSymbol", StringComparison.Ordinal))
            .ShouldHaveSingleItem();

        bullet.Embedded.ShouldBeTrue($"the deck's bullet face {bullet.Name} carries no program");
    }

    /// <summary>
    /// A list label is embedded, in every word-processing format that draws one.
    /// </summary>
    /// <remarks>
    /// The label is not in the paragraph's text and does not travel on its runs, so it is the one
    /// piece of a word-processing page whose reference is built somewhere else — once per reader.
    /// ODT carried it and the other three did not, which is exactly the kind of hole a per-family
    /// sweep misses and a per-reader theory does not.
    /// </remarks>
    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    [InlineData("text-features.odt")]
    public void AListLabelIsDrawnFromAnEmbeddedFace(string fileName)
    {
        PdfFile pdf = PdfFile.Parse(Render(fileName));

        // Every face, because a label is set in the item's own family for a numbered level and in a
        // symbol family for a bullet: which of the file's faces the label uses is the reader's
        // business, and all of them have to be embedded either way.
        Faces(pdf).ShouldAllBe(face => face.Embedded);
    }

    /// <summary>
    /// The name of every PDF font in the file, and whether its descriptor carries a program.
    /// </summary>
    /// <remarks>
    /// Read out of the file rather than off the catalogue, so that the assertion is about what a
    /// reader will find: the writer could hold the right bytes and fail to reference them, and the
    /// question is whether the <em>file</em> is complete. Font dictionaries and descriptors are
    /// written as plain uncompressed objects — see <c>PdfDocumentWriter.SetStream</c>, which
    /// stores font programs uncompressed for an unrelated reason — so the whole chain is legible
    /// without inflating anything.
    /// </remarks>
    private static List<(string Name, bool Embedded)> Faces(PdfFile pdf)
    {
        Dictionary<int, string> objects = pdf.Dictionaries();
        List<(string, bool)> faces = [];

        foreach ((int _, string body) in objects.OrderBy(entry => entry.Key))
        {
            // The trailing slash matters: "/Type/FontDescriptor" begins with "/Type/Font", and a
            // plain substring test collects the descriptors as well as the fonts.
            if (!body.Contains("/Type/Font/", StringComparison.Ordinal)) continue;

            Match descriptor = DescriptorReference().Match(body);
            descriptor.Success.ShouldBeTrue($"a /Type/Font with no /FontDescriptor: {body}");

            int id = int.Parse(descriptor.Groups[1].Value, CultureInfo.InvariantCulture);
            string named = objects.TryGetValue(id, out string? found) ? found : string.Empty;

            Match name = BaseFont().Match(body);

            // Either stream key counts, and both have to. A CFF-flavoured OpenType face is only
            // admissible as /FontFile3 (PDF 1.7 §9.9), so a test that insisted on /FontFile2 would
            // fail the correct output for every .otf on the machine — and asserting /FontFile2
            // alone is what let a CFF face be written under it, which poppler answers by dropping
            // the font and drawing nothing. See PdfFontCatalogue.IsCompactFontFormat.
            faces.Add((
                name.Success ? name.Groups[1].Value : "<unnamed>",
                named.Contains("/FontFile2 ", StringComparison.Ordinal)
                || named.Contains("/FontFile3 ", StringComparison.Ordinal)));
        }

        return faces;
    }

    /// <summary>Renders a corpus document to a PDF in memory.</summary>
    /// <remarks>
    /// In memory on purpose: two corpus documents differing only in extension render to the same
    /// file name, and a test that never names a file cannot be caught by that.
    /// </remarks>
    private static byte[] Render(string fileName)
    {
        string path = Corpus.Require(fileName);

        using IDocument document = PaperlessDocument.Open(path);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using MemoryStream output = new();
        new PdfRenderer(Reproducible).Render(pages, output);
        return output.ToArray();
    }

    [GeneratedRegex(@"/FontDescriptor (\d+) 0 R")]
    private static partial Regex DescriptorReference();

    [GeneratedRegex(@"/BaseFont/([^/ >]+)")]
    private static partial Regex BaseFont();
}
