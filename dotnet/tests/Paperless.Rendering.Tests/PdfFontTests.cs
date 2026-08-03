using System.Buffers.Binary;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// The fonts a written PDF carries: what was embedded, how it is addressed, and what it says.
/// </summary>
/// <remarks>
/// <para>
/// This is where the claim "real searchable text, not outlines" is actually checked. Three
/// separate things have to hold at once and each fails invisibly on its own:
/// </para>
/// <list type="bullet">
///   <item>The embedded face has to hold the glyphs the content stream asks for, renumbered
///     to the codes the content stream already used — a font whose glyph 3 is not the code 3
///     draws the wrong letters and nothing else notices.</item>
///   <item>The face's character map has to make a one-byte code select that glyph, which is
///     the identity table LibreOffice writes and hb-subset does not.</item>
///   <item>The <c>ToUnicode</c> CMap has to take the code back to the character, or the text
///     cannot be found or copied and the whole reason the display list carries a cluster map
///     is wasted.</item>
/// </list>
/// </remarks>
public sealed class PdfFontTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void TheEmbeddedFaceHoldsOnlyTheGlyphsTheDocumentUses()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11));
        PdfFile pdf = Write(run);

        byte[] font = pdf.FontPrograms().ShouldHaveSingleItem();
        int distinct = run.Glyphs.Select(g => g.GlyphId).Distinct().Count();

        // Plus .notdef, which every font keeps at index zero. A whole Liberation Serif is 340 kB
        // and this is the difference between a page of text costing that and costing a few
        // hundred bytes.
        NumberOfGlyphs(font).ShouldBe(distinct + 1);
        font.Length.ShouldBeLessThan(20_000, "a nine-glyph subset should be small");
    }

    [Fact]
    public void AnEmbeddedFaceAddressesItsGlyphsByCode()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        byte[] font = Write(TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11)))
            .FontPrograms().ShouldHaveSingleItem();

        byte[] cmap = Table(font, "cmap").ShouldNotBeNull();

        // Platform 1, encoding 0, format 0, and the identity: exactly what LibreOffice embeds —
        // measured on its own export of prose-odt.odt, whose codes 1 to 28 map to glyphs 1 to 28.
        // hb-subset builds a Unicode cmap instead, which addresses nothing a PDF asks for, so it
        // is thrown away and this written in its place.
        BinaryPrimitives.ReadUInt16BigEndian(cmap.AsSpan(2)).ShouldBe((ushort)1, "one subtable");
        BinaryPrimitives.ReadUInt16BigEndian(cmap.AsSpan(4)).ShouldBe((ushort)1, "platform: Macintosh");
        BinaryPrimitives.ReadUInt16BigEndian(cmap.AsSpan(6)).ShouldBe((ushort)0, "encoding: Roman");
        BinaryPrimitives.ReadUInt16BigEndian(cmap.AsSpan(12)).ShouldBe((ushort)0, "format 0");

        for (int code = 0; code < NumberOfGlyphs(font); code++) cmap[18 + code].ShouldBe((byte)code);
    }

    [Fact]
    public void EveryCodeSelectsTheGlyphTheDocumentAskedFor()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11));
        PdfFile pdf = Write(run);
        byte[] font = pdf.FontPrograms().ShouldHaveSingleItem();

        // The advance is the fingerprint. If the subsetter had renumbered the glyphs its own way
        // rather than to the codes the content stream already used, the subset would still be a
        // valid font holding the right glyphs — and every letter on the page would be a different
        // one. Comparing each code's advance in the subset against the original face's advance
        // for the glyph it was meant to be catches exactly that.
        Dictionary<byte, ushort> codes = CodesInOrder(pdf, run);

        foreach ((byte code, ushort glyph) in codes)
        {
            Advance(font, code).ShouldBe(
                (ushort)TestFace.Face.AdvanceOf(glyph),
                $"code {code} should carry glyph {glyph}'s advance");
        }

        codes.Count.ShouldBeGreaterThan(4, "too few glyphs to prove anything");
    }

    [Fact]
    public void TheWidthsInTheFileAreTheWidthsInTheFace()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        GlyphRun run = TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11));
        PdfFile pdf = Write(run);

        Match widths = Regex.Match(pdf.Text, @"/FirstChar 0/LastChar (\d+)/Widths\[([^\]]*)\]");
        widths.Success.ShouldBeTrue();

        string[] stated = widths.Groups[2].Value.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        double scale = 1000.0 / TestFace.Face.UnitsPerEm;

        foreach ((byte code, ushort glyph) in CodesInOrder(pdf, run))
        {
            double expected = Math.Round(TestFace.Face.AdvanceOf(glyph) * scale, 4);
            double actual = double.Parse(stated[code], CultureInfo.InvariantCulture);

            // Four decimals rather than the integer LibreOffice writes, and the reason is the
            // accumulated drift: a tenth of a thousandth of an em is 0.004 pt at eleven point, and
            // a sixty-glyph line rounded per glyph ends a quarter of a point short of where layout
            // put it.
            actual.ShouldBe(expected, 0.0001, $"code {code}");
        }
    }

    [Fact]
    public void ToUnicodeTakesEveryCodeBackToItsCharacter()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        const string text = "Paperless renders";
        PdfFile pdf = Write(TestFace.Run(text, new DocPoint(Points(56.7), Points(120)), Points(11)));

        Dictionary<byte, string> map = pdf.ToUnicode("F1");
        map.ShouldNotBeEmpty();

        // Replay the content stream through the map and get the sentence back. This is what a
        // reader's "find" and "copy" do, and a PDF of outlines could not do it at all.
        StringBuilder recovered = new();
        foreach (Match shown in Regex.Matches(pdf.ContentStreams().Single(), "<([0-9A-F]+)>"))
        {
            string hex = shown.Groups[1].Value;
            for (int i = 0; i < hex.Length; i += 2)
            {
                byte code = byte.Parse(
                    hex.AsSpan(i, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                recovered.Append(map.GetValueOrDefault(code, "�"));
            }
        }

        recovered.ToString().ShouldBe(text);
    }

    [Fact]
    public void AFaceUsedBeyondItsFirstTwoHundredAndFiftyFiveGlyphsBecomesTwoFonts()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // A simple TrueType font in a PDF addresses its glyphs with one byte, so a face used more
        // than 255 ways needs a second PDF font. LibreOffice pays the same cost the same way. The
        // run is built by glyph id directly rather than from text, because reaching 300 distinct
        // glyphs through a Latin string would need a document in several scripts.
        List<PositionedGlyph> glyphs = [];
        List<int> clusters = [];
        for (int i = 1; i <= 300; i++)
        {
            glyphs.Add(new PositionedGlyph(
                (ushort)i, new DocPoint(Length.FromPoints(i * 2), Length.Zero), Length.FromPoints(2)));
            clusters.Add(i - 1);
        }

        GlyphRun run = new()
        {
            Font = TestFace.Reference,
            FontSize = Points(11),
            Origin = new DocPoint(Points(20), Points(120)),
            Glyphs = glyphs,
            Text = new string('x', 300),
            ClusterMap = clusters,
        };

        PdfFile pdf = Write(run);

        pdf.FontCount().ShouldBe(2, "255 codes fit in the first font and the rest in a second");
        pdf.FontPrograms().Count.ShouldBe(2);

        // And the content stream has to switch between them mid-run, or the glyphs past the 255th
        // are drawn from the wrong font.
        pdf.ContentStreams().Single().ShouldContain("/F2 11 Tf");
    }

    [Fact]
    public void ARealDocumentEmbedsAFaceForEveryFamilyItSets()
    {
        Assert.SkipUnless(Corpus.IsAvailable, "the test corpus is not present");

        string path = Corpus.Require("mixed-runs.docx");
        using IDocument document = OpenDocument(path);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(pages, buffer);
        PdfFile pdf = PdfFile.Parse(buffer.ToArray());

        // Regular, bold and italic are three faces, not three styles of one: the display list
        // resolves each to its own file, and a PDF that embedded one of them and named the other
        // two would render the whole document in the same weight.
        pdf.FontPrograms().Count.ShouldBeGreaterThan(1);
        pdf.FontCount().ShouldBe(pdf.FontPrograms().Count);
        pdf.Text.ShouldContain("/Subtype/TrueType");

        // Every embedded face carries a subset tag, which is what tells a reader it is not the
        // whole font and must not be substituted for one.
        Regex.Count(pdf.Text, @"/BaseFont/[A-Z]{6}\+").ShouldBe(pdf.FontPrograms().Count);
    }

    [Fact]
    public void AFaceThatCannotBeFoundIsNamedRatherThanEmbedded()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        // A key that is not a path is what a hand-built page or a document's own embedded font
        // produces. Nothing to embed is not an error: the pen positions and the ToUnicode are
        // still right, and only the glyph shapes fall back to whatever the reader has. A backend
        // that threw here would turn a cosmetic problem into no document at all.
        GlyphRun run = TestFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11));
        GlyphRun unresolved = run with
        {
            Font = new FontReference { FamilyName = "Nothing At All", FaceKey = "not-a-path" },
        };

        PdfFile pdf = Write(unresolved);

        pdf.FontPrograms().ShouldBeEmpty();
        pdf.Text.ShouldContain("/BaseFont/AAAAAA+NothingAtAll");
        pdf.ContentStreams().Single().ShouldContain("Tj");
    }

    /// <summary>
    /// A CFF-flavoured face is named and not embedded, rather than embedded as a TrueType one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>/FontFile2</c> promises <c>glyf</c> outlines and a reader that believes it and finds
    /// <c>OTTO</c> is entitled to stop. Poppler does: on <c>16 - UTM - (NASA).pptx</c> it
    /// reported *"Mismatch between font type and embedded font file"* and then *"No font in
    /// show"* 161 times, leaving 161 glyph runs blank on a document whose page count, word count
    /// and <c>pdffonts</c> embedding check all passed. So the assertion here is that the file
    /// does <em>not</em> claim a TrueType program for a face that has none.
    /// </para>
    /// <para>
    /// The mechanism was not what the key alone suggested. Correcting it to
    /// <c>/FontFile3 /Subtype /OpenType</c> under a <c>/Type1</c> dictionary — which is what PDF
    /// 1.7 §9.9 asks for — makes poppler accept the file and still draw nothing useful: a Type1
    /// dictionary selects glyphs by <em>name</em> through the CFF charset, and our codes are
    /// glyph indices, so an 18 pt Loma probe came back as a row of tofu boxes. Embedding one
    /// properly needs a <c>/Differences</c> name encoding, and a CID-keyed CFF needs a composite
    /// <c>/Type0</c> font on top of that. Until then, naming the face without embedding it is the
    /// one option that puts real words on the page.
    /// </para>
    /// </remarks>
    [Fact]
    public void ACffFlavouredFaceIsNotClaimedToBeTrueType()
    {
        Assert.SkipUnless(TestCffFace.IsAvailable, "no CFF-flavoured face on this machine");

        PdfFile pdf = Write(TestCffFace.Run("Paperless", new DocPoint(Points(56.7), Points(120)), Points(11)));

        pdf.FontPrograms().ShouldBeEmpty("a CFF face must not be written as a TrueType program");
        pdf.Text.ShouldNotContain("/FontFile");

        // Everything except the outlines survives: the widths are still the face's own, so the
        // pen positions and the line breaks are the ones layout measured.
        pdf.Text.ShouldContain("/Widths[");
        pdf.Text.ShouldContain("/FontDescriptor ");
    }

    // ------------------------------------------------------------------------- the machinery

    private static PdfFile Write(GlyphRun run)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(
                DrawnPage.A4, sink => sink.DrawGlyphRun(run, Paint.Solid(Colour.Black)))),
            buffer);

        return PdfFile.Parse(buffer.ToArray());
    }

    private static IDocument OpenDocument(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }

    private static Length Points(double value) => Length.FromPoints(value);

    /// <summary>Which glyph each code stands for, read back out of the content stream.</summary>
    private static Dictionary<byte, ushort> CodesInOrder(PdfFile pdf, GlyphRun run)
    {
        Dictionary<byte, ushort> codes = [];
        int glyph = 0;

        foreach (Match shown in Regex.Matches(pdf.ContentStreams().Single(), "<([0-9A-F]+)>"))
        {
            string hex = shown.Groups[1].Value;
            for (int i = 0; i < hex.Length && glyph < run.Glyphs.Count; i += 2, glyph++)
            {
                byte code = byte.Parse(
                    hex.AsSpan(i, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture);
                codes[code] = run.Glyphs[glyph].GlyphId;
            }
        }

        return codes;
    }

    private static int NumberOfGlyphs(byte[] font)
        => BinaryPrimitives.ReadUInt16BigEndian(Table(font, "maxp")!.AsSpan(4));

    /// <summary>A glyph's advance in the subset, from its <c>hmtx</c>.</summary>
    private static ushort Advance(byte[] font, int glyph)
    {
        byte[] hhea = Table(font, "hhea")!;
        byte[] hmtx = Table(font, "hmtx")!;

        int metrics = BinaryPrimitives.ReadUInt16BigEndian(hhea.AsSpan(34));
        int index = Math.Min(glyph, metrics - 1);
        return BinaryPrimitives.ReadUInt16BigEndian(hmtx.AsSpan(index * 4));
    }

    private static byte[]? Table(byte[] font, string tag)
    {
        int count = BinaryPrimitives.ReadUInt16BigEndian(font.AsSpan(4));

        for (int i = 0; i < count; i++)
        {
            int record = 12 + (16 * i);
            if (Encoding.ASCII.GetString(font, record, 4) != tag) continue;

            int offset = (int)BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record + 8));
            int length = (int)BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record + 12));
            return font[offset..(offset + length)];
        }

        return null;
    }
}
