using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A character drawn from a fallback face keeps the lean of the run it came out of.
/// </summary>
/// <remarks>
/// <para>
/// Round 56 put the slant on each reader's "does this paragraph's formatting vary" predicate, and
/// left 1 611 sheared glyphs short on 39 of the 337 words documents — of which 289 sit in faces
/// <b>no document names</b>: WenQuanYi Zen Hei 177 and OpenSymbol 112, led by
/// <c>手机免提系统TSB.doc</c> at 82 and <c>A320SimNotes.doc</c> at 75. Those faces arrive at
/// <c>PageDrawing.ByFace</c> through <c>FontItemiser</c>, and the reference naming them came from
/// a reverse lookup <em>with no request to compare against</em>, so it could not set the lean.
/// </para>
/// <para>
/// The assertion is on the drawn <see cref="GlyphRun"/> rather than on the resolver, because the
/// call site is the half that was missing: the resolver's overload can be right while the run that
/// reaches the page is built without it. Measured over the whole corpus, the words track leaned
/// <b>0</b> of the 6 616 glyphs it draws in those faces against the reference's 289 of 9 391.
/// </para>
/// </remarks>
public sealed class FallbackObliqueDrawingTests
{
    /// <summary>U+6C49 汉, which no Latin face installed for this project covers.</summary>
    private const int Han = 0x6C49;

    /// <summary>Latin, Chinese, Latin, so the split happens twice and the middle piece is the one.</summary>
    private const string FixtureText = "ab汉字cd";

    /// <summary>A family with no italic anywhere, so its own italic is synthetic.</summary>
    private const string NoItalic = "Zqxwv Nonesuch";

    private static readonly Length Size = Length.FromPoints(12);

    [Fact]
    public void NeitherFixtureFaceCoversTheChineseAndNeitherHasAnItalicCjkToFallTo()
    {
        // Two premises in one, and both would silently invert the file if they changed: the Latin
        // face must not grow a CJK range, and the covering face must not have an italic of its own
        // -- if it did, the right answer would be a real italic face and not a synthetic lean.
        Latin(italic: false).HasGlyphFor(Han).ShouldBeFalse();
        Latin(italic: true).HasGlyphFor(Han).ShouldBeFalse();
        Fonts.FallbackFor(Han).ShouldNotBeNull().IsItalic.ShouldBeFalse();
    }

    [Fact]
    public void AFallbackFaceInAnItalicRunIsDrawnLeaning()
        => Fallback(Draw(Paragraph(Latin(italic: true))))
            .Font.SyntheticOblique.ShouldBeTrue();

    [Fact]
    public void AFallbackFaceInAnUprightRunIsNotDrawnLeaning()
        // The control that separates the fix from "the fallback face always leans". Without it
        // every other assertion here is satisfied by a constant.
        => Fallback(Draw(Paragraph(Latin(italic: false))))
            .Font.SyntheticOblique.ShouldBeFalse();

    [Fact]
    public void AFallbackFaceInheritsALeanTheRunOnlyHasSynthetically()
    {
        // The arm a reading of the primary face alone would lose, and it is the arm that matters:
        // a family with no italic installed is exactly the kind of family that also has to fall
        // back. Measured on 26.2.4.2 as `cjk-italic-none`, six sheared glyphs in all six formats.
        OpenTypeFace primary = Fonts.LoadOpenType(Fonts.Resolve(new FontRequest(NoItalic, 400, true)));
        FontReference reference = Fonts.Resolve(new FontRequest(NoItalic, 400, true));

        primary.IsItalic.ShouldBeFalse("the fixture needs a family with no italic installed");
        reference.SyntheticOblique.ShouldBeTrue();

        Fallback(Draw(Paragraph(primary, reference))).Font.SyntheticOblique.ShouldBeTrue();
    }

    [Fact]
    public void TheLatinRunsAroundItAreUnchanged()
    {
        // The fallback piece is the only one that moves. A change that leaned the whole paragraph
        // would pass every assertion above.
        List<GlyphRun> drawn = Draw(Paragraph(Latin(italic: true)));

        drawn.Count.ShouldBeGreaterThan(1);
        drawn.Where(run => !run.Font.FaceKey.Equals(Fallback(drawn).Font.FaceKey, StringComparison.Ordinal))
            .ShouldAllBe(run => !run.Font.SyntheticOblique);
    }

    [Fact]
    public void TheLeaningFallbackIsStillNamedWellEnoughToBeEmbedded()
    {
        // Rebuilding the reference to carry the lean must not lose the face key, or the PDF
        // announces a font it does not carry -- which the corpus gate scores as a failure, and
        // rightly: a reader without that font installed sees nothing.
        FontReference reference = Fallback(Draw(Paragraph(Latin(italic: true)))).Font;

        reference.FaceKey.ShouldNotBeNullOrEmpty();
        File.Exists(reference.FaceKey.Split('#')[0]).ShouldBeTrue();
    }

    [Fact]
    public void TheLeanMovesNoAdvance()
    {
        // The whole argument for this being safe to ship without moving a page count: a synthetic
        // oblique is the `c` term of a text matrix and shears outlines, not widths. The corpus
        // bears it out at 0 page counts changed on all three tracks, and this says it where a
        // mutation can reach it.
        //
        // The fallback piece alone, because the Latin pieces around it are NOT comparable: this
        // fixture's italic arm resolves them to LiberationSerif-Italic, a genuinely different
        // face with genuinely different advances. Comparing whole paragraphs measured the wrong
        // thing and said so by failing.
        Length leaning = Advance(Fallback(Draw(Paragraph(Latin(italic: true)))));
        Length upright = Advance(Fallback(Draw(Paragraph(Latin(italic: false)))));

        upright.ShouldBeGreaterThan(Length.Zero, "the fixture has to measure as something");
        leaning.ShouldBe(upright);
    }

    /// <summary>The reader end of it, so the wiring is asserted and not only the layout seam.</summary>
    /// <remarks>
    /// The seam tests above build their paragraph, so they hold when the layout is right and the
    /// readers hand nothing down. This one reads a package.
    /// </remarks>
    [Fact]
    public void TheDocxReaderReachesThisEndToEnd()
    {
        using IDocument document = ReadDocx();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageParagraph paragraph = pages.Blocks.OfType<PageParagraph>().First(p => p.Text.Length > 0);

        Fallback(Draw(paragraph)).Font.SyntheticOblique.ShouldBeTrue();
    }

    // ------------------------------------------------------------------------------- fixtures

    private static GlyphRun Fallback(List<GlyphRun> drawn)
        => drawn.Single(run => run.Text.Length > 0 && run.Text[0] is '汉');

    private static Length Advance(GlyphRun run)
    {
        Length total = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) total += glyph.Advance;
        return total;
    }

    private static PageParagraph Paragraph(OpenTypeFace face, FontReference? font = null)
        => new()
        {
            Text = FixtureText,
            Face = face,
            Font = font ?? Fonts.ReferenceFor(face),
            EmSize = Size,
            Fallback = Fonts,
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
            Box: new Text.Layout.LineBox(
                new Text.Layout.TextLine(
                    0, paragraph.Text.Length, paragraph.Text.Length, Length.Zero, EndsParagraph: true),
                Length.Zero,
                Length.Zero,
                Length.FromPoints(14),
                Length.FromPoints(11),
                Length.Zero),
            Top: Length.Zero);

    private static IDocument ReadDocx()
    {
        const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
        const string PkgR = "http://schemas.openxmlformats.org/package/2006/relationships";
        const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

        string types = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        string root = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{PkgR}">
              <Relationship Id="rId1" Target="word/document.xml" Type="{R}/officeDocument"/>
            </Relationships>
            """;

        // Every slot names the Latin face, so nothing can draw the Chinese but the coverage check;
        // w:i and w:iCs together, because OOXML files CJK under the east-Asian slot.
        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="{W}"><w:body><w:p><w:r>
              <w:rPr>
                <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"
                          w:eastAsia="Liberation Serif" w:cs="Liberation Serif"/>
                <w:i/><w:iCs/>
              </w:rPr>
              <w:t xml:space="preserve">{FixtureText}</w:t>
            </w:r></w:p></w:body></w:document>
            """;

        MemoryStream package = new();
        using (ZipArchive archive = new(package, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", types);
            Write(archive, "_rels/.rels", root);
            Write(archive, "word/document.xml", document);
        }

        package.Position = 0;
        using DocumentSource source = DocumentSource.FromStream(package, "fallback-oblique.docx");
        return new WordProcessingReader().Read(source);

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

    private static SystemFontResolver Fonts { get; } = new(SystemFontIndex.Build());

    private static OpenTypeFace Latin(bool italic)
        => Fonts.LoadOpenType(Fonts.Resolve(new FontRequest("Liberation Serif", 400, italic)));
}
