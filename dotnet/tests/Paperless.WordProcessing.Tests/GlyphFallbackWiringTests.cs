using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A character the paragraph's face cannot draw is measured and drawn in a face that can.
/// </summary>
/// <remarks>
/// <para>
/// Both halves of glyph fallback were written and neither was ever connected.
/// <see cref="Paperless.Text.Itemisation.FontItemiser"/> splits a run at the characters its face has
/// no glyph for; <see cref="SystemFontResolver"/> answers "what face has this character"; and
/// <c>ItemisationOptions.GlyphFallback</c>, the property that joins them, was not assigned anywhere
/// in the tree — readers, layout, tools or tests. So every word-processing document was laid out with
/// no coverage check at all.
/// </para>
/// <para>
/// The symptom is worse than a wrong face, and it is why this is asserted on the drawn glyphs rather
/// than on the resolver: an uncovered character shapes to <c>.notdef</c>, which draws that face's
/// missing-glyph box <em>at that face's <c>.notdef</c> advance</em>. The text is invisible and the
/// line breaks in the wrong place. Measured on the corpus document <c>手机免提系统TSB.doc</c>, where
/// every Chinese character came out a box and LibreOffice drew all of them from WenQuanYi Zen Hei —
/// and where the word gate scored the loss of the document's entire content as four words, because
/// <c>wc -w</c> in the POSIX locale counts a word only where it sees a printable ASCII byte.
/// </para>
/// <para>
/// The measuring and drawing passes are asserted separately because they reach the split by different
/// routes — <c>PageParagraph.Itemisation</c> for the first, <c>PageDrawing.ByFace</c> for the second
/// — and a fix to one alone gives a page whose glyphs are right and whose advances are not, or the
/// reverse. Neither assertion names a font: which face covers CJK depends on what is installed, so
/// the test asks the drawn face whether it has the glyph instead of asking what it is called.
/// </para>
/// </remarks>
public sealed class GlyphFallbackWiringTests
{
    /// <summary>U+6C49 汉, which no Latin face installed for this project covers.</summary>
    private const int Han = 0x6C49;

    private static readonly Length Size = Length.FromPoints(12);

    [Fact]
    public void ALatinFaceCoversNoneOfTheChineseInTheFixture()
        // The premise the rest of the file rests on. Were Liberation Serif to grow a CJK range, every
        // assertion below would pass while testing nothing, and this says so first.
        => LatinFace.HasGlyphFor(Han).ShouldBeFalse();

    [Fact]
    public void TheMeasuringPassSplitsARunAtACharacterItsFaceCannotDraw()
    {
        MeasuredParagraph measured = Paragraph(withFallback: true).Measure();

        IEnumerable<OpenTypeFace> faces = measured.Runs.Select(run => run.Run.Face);
        faces.ShouldContain(face => face.HasGlyphFor(Han));
    }

    [Fact]
    public void WithoutAResolverTheMeasuringPassLeavesTheRunWhole()
    {
        MeasuredParagraph measured = Paragraph(withFallback: false).Measure();

        // Exactly the pre-fix behaviour, kept explicit: a caller that supplies no resolver — an
        // extraction-only one, or a unit test — pays for no coverage checks and gets the run it gave.
        measured.Runs.ShouldAllBe(run => run.Run.Face == LatinFace);
    }

    [Fact]
    public void TheDrawingPassPutsTheCharacterInAFaceThatHasIt()
    {
        List<GlyphRun> drawn = Draw(Paragraph(withFallback: true));

        // Every glyph drawn, and none of them .notdef. Checking the face is not enough on its own:
        // a run handed to the right face at the wrong offsets still draws boxes.
        drawn.SelectMany(run => run.Glyphs).ShouldAllBe(glyph => glyph.GlyphId != 0);
        drawn.Sum(run => run.Glyphs.Count).ShouldBe(FixtureText.Length);
    }

    [Fact]
    public void WithoutAResolverTheDrawingPassStillDrawsTheMissingGlyphBox()
        // The defect itself, asserted so the fix cannot be mistaken for the shaper having changed.
        => Draw(Paragraph(withFallback: false))
            .SelectMany(run => run.Glyphs)
            .ShouldContain(glyph => glyph.GlyphId == 0);

    /// <summary>
    /// A document read by the DOCX reader arrives at the layout with a resolver on its paragraphs.
    /// </summary>
    /// <remarks>
    /// The seam tests above construct their paragraph, so they hold when the layout is right and the
    /// four readers set nothing — which is exactly the state this change found the tree in, and
    /// exactly what a mutation run proved they miss. This one reads a package, so it fails if the
    /// reader stops passing the resolver down.
    /// </remarks>
    [Fact]
    public void TheDocxReaderGivesItsParagraphsAFallbackResolver()
    {
        using IDocument document = ReadDocx(FixtureText);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageParagraph paragraph = pages.Blocks.OfType<PageParagraph>().First();

        paragraph.Fallback.ShouldNotBeNull();
        Draw(paragraph).SelectMany(run => run.Glyphs).ShouldAllBe(glyph => glyph.GlyphId != 0);
    }

    private static IDocument ReadDocx(string text)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        // The run names a Latin face outright, so the fixture cannot pass by the reader happening to
        // resolve a CJK font for it — the coverage check is the only thing that can draw this text.
        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:r>
                    <w:rPr><w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/></w:rPr>
                    <w:t xml:space="preserve">{text}</w:t>
                  </w:r>
                </w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream package = new();
        using (ZipArchive archive = new(package, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", document);
        }

        package.Position = 0;
        using DocumentSource source = DocumentSource.FromStream(package, "glyph-fallback.docx");
        return new WordProcessingReader().Read(source);

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

    /// <summary>Latin, Chinese and Latin again, so the split has to happen twice and not once.</summary>
    private const string FixtureText = "ab汉字cd";

    private static PageParagraph Paragraph(bool withFallback)
        => new()
        {
            Text = FixtureText,
            Face = LatinFace,
            EmSize = Size,
            Fallback = withFallback ? Fonts : null,
        };

    private static List<GlyphRun> Draw(PageParagraph paragraph)
    {
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400));

        return
        [
            .. PageDrawing
                .RunsIn(area, Line(paragraph), paragraph, highlights: null, rules: null)
                .Select(pair => pair.Run),
        ];
    }

    private static PlacedLine Line(PageParagraph paragraph)
        => new(
            ParagraphIndex: 0,
            LineIndex: 0,
            Box: new LineBox(
                new TextLine(
                    0, paragraph.Text.Length, paragraph.Text.Length, Length.Zero, EndsParagraph: true),
                Length.Zero,
                Length.Zero,
                Length.FromPoints(14),
                Length.FromPoints(11),
                Length.Zero),
            Top: Length.Zero);

    private static SystemFontResolver Fonts { get; } = new(SystemFontIndex.Build());

    private static OpenTypeFace LatinFace { get; } =
        Fonts.LoadOpenType(Fonts.Resolve(new FontRequest("Liberation Serif", 400, false)));
}
