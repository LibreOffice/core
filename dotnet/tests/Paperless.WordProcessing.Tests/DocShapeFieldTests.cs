using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Ww8;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The <c>SHAPE</c> field, which is how a DOC says that a shape with a full floating anchor is set in
/// the run of text after all.
/// </summary>
/// <remarks>
/// <para>
/// The rule is <c>SwWW8ImplReader::IsInlineEscherHack</c> (<c>ww8par.hxx:1737</c>) — the innermost open
/// field is a <c>SHAPE</c> — and it is worth a file of its own because the evidence for it is spread
/// over three structures that a reader has no reason to consult together. The text says
/// <c>U+0008 U+0001</c>; the <c>FSPA</c> at the first of them describes a floating shape with a wrap and
/// two origins; and the only thing that contradicts the <c>FSPA</c> is a byte in the <c>PlcFld</c>, which
/// is a table a layout walk otherwise never opens.
/// </para>
/// <para>
/// Both corpus documents that exercise it are LibreOffice's own DOC exports, which is what makes the
/// pairing trustworthy: <c>vector-picture-text.doc</c> holds three pictures written this way and
/// <c>word-features.doc</c> one text box, and neither was authored to have a <c>SHAPE</c> field in it.
/// </para>
/// </remarks>
public sealed class DocShapeFieldTests
{
    /// <summary>
    /// The field table gives a field's type, which the markers in the text do not.
    /// </summary>
    /// <remarks>
    /// Read straight from the document rather than inferred from a frame, because everything else here
    /// depends on this byte and a wrong one would show up only as a shape in a plausible wrong place.
    /// <c>vector-picture-text.doc</c> declares exactly three fields and all three are <c>SHAPE</c>.
    /// </remarks>
    [Fact]
    public void TheFieldTableNamesEachFieldsType()
    {
        Ww8FieldTypes fields = FieldsOf("vector-picture-text.doc");

        fields.Count.ShouldBe(3);
        fields.At(12).ShouldBe(Ww8FieldTypes.Shape);
        fields.At(47).ShouldBe(Ww8FieldTypes.Shape);
        fields.At(87).ShouldBe(Ww8FieldTypes.Shape);

        // Not a beginning: 20 is the separator of the first field and 23 its end, and neither carries a
        // type at all — the byte in that place is 0xFF and a flags byte. A reader keying off position
        // alone rather than off `FLD.ch` would return 255 and 128 here.
        fields.At(20).ShouldBeNull();
        fields.At(23).ShouldBeNull();
    }

    /// <summary>
    /// A document with no fields costs nothing and reports nothing.
    /// </summary>
    /// <remarks>
    /// The common case by a wide margin, and the one that decides whether reading this table at all is
    /// affordable — every DOC's layout walk now asks for it.
    /// </remarks>
    [Fact]
    public void ADocumentWithNoFieldsHasAnEmptyTable()
    {
        FieldsOf("frame-wrap.doc").Count.ShouldBe(0);
        Ww8FieldTypes.Parse([]).Count.ShouldBe(0);
        Ww8FieldTypes.Parse([1, 2, 3]).Count.ShouldBe(0);
    }

    /// <summary>
    /// A shape inside a <c>SHAPE</c> field is as-character, and takes room on its line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The defect this fixes: all three of <c>vector-picture-text.doc</c>'s pictures came back
    /// <see cref="FrameAnchor.Character"/> with <see cref="TextWrap.Through"/>, which moves no text at
    /// all, so each picture was drawn over the paragraph after it. The word count could not see it —
    /// the row read 13/13 through the whole of the defect — which is why it is pinned on the anchor
    /// and on the room rather than on a rendering.
    /// </para>
    /// <para>
    /// The sizes are the <c>FSPA</c>'s own. That is the part of the record the hack keeps: it replaces
    /// the horizontal orientation and the anchor and leaves the rectangle, so a frame that came out the
    /// right size floating is still the right size in the line.
    /// </para>
    /// </remarks>
    [Fact]
    public void APictureInAShapeFieldIsAnchoredAsCharacter()
    {
        using IDocument document = Open("vector-picture-text.doc");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<PageFrame> frames = [.. pages.Pages[0].Frames.Select(placed => placed.Frame)];
        frames.Count.ShouldBe(3);

        foreach (PageFrame frame in frames)
        {
            frame.Anchor.ShouldBe(FrameAnchor.AsCharacter);
            frame.Wrap.ShouldBe(TextWrap.Through);
            frame.Size.Width.Twips.ShouldBe(4535);
        }

        frames[0].Size.Height.Twips.ShouldBe(3401);
        frames[1].Size.Height.Twips.ShouldBe(3401);
        frames[2].Size.Height.Twips.ShouldBe(2267);
    }

    /// <summary>
    /// It hangs below its line rather than resting on the baseline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The other half, and the half that a reader would get wrong by following the ordinary
    /// as-character rule. <c>ProcessEscherAlign</c> replaces the horizontal orientation for the inline
    /// case and leaves the vertical alone (<c>ww8graf.cxx:2436-2439</c>); these shapes state
    /// <c>posrelv</c> 3 — relative to the line — with <c>posv</c> 0 and a top of nought, which
    /// <c>SwAsCharAnchoredObjectPosition</c> resolves to a position of nought relative to the baseline
    /// and <c>SwFlyCntPortion::SetBase</c> then turns into an ascent of nought.
    /// </para>
    /// <para>
    /// Measured on <c>word-features.doc</c>, whose box is the clearest case because it has text in it:
    /// LibreOffice keeps the anchor line's baseline at 455.51 pt and draws the box's own first line at
    /// 466.71, <em>below</em> it. Resting the box on the baseline instead put the sentence at 477.71 —
    /// 22 pt of the page moved by one frame — which is the number that made the whole item look like it
    /// needed a line filler that could split a line, when what it needed was this.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShapeFieldsFrameHangsBelowTheBaseline()
    {
        using IDocument document = Open("word-features.doc");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PlacedFrame box = pages.Pages[0].Frames
            .Where(placed => placed.Frame.Anchor == FrameAnchor.AsCharacter)
            .ShouldHaveSingleItem();

        box.Frame.InlineAscent.ShouldBe(Length.Zero);

        // Its left edge is its anchor's own offset along the line — 134.01 pt against LibreOffice's
        // 134.00 — where a frame placed at its paragraph's corner instead landed at 56.70 and drew over
        // the sentence it belongs after.
        box.Area.X.Points.ShouldBe(134.01, 0.02);
    }

    /// <summary>
    /// The anchor characters of a <c>SHAPE</c> field leave nothing in the text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice inserts a character for one of these only when the graphic fails —
    /// <c>if (!pResult) cInsert = ' '</c>, <c>ww8par.cxx:3637</c> — and for the <c>U+0001</c> of a
    /// <c>U+0008 U+0001</c> pair inside a <c>SHAPE</c> field it takes neither branch, so <c>cInsert</c> stays
    /// at nought and nothing is inserted at all.
    /// </para>
    /// <para>
    /// It is not cosmetic. The two characters shaped to 18.67 pt of <c>.notdef</c> between "Before the
    /// box." and "After the box." in <c>word-features.doc</c> — invisible for as long as the box was
    /// misplaced far to the left, and exactly the width by which the sentence overshot once the box was
    /// put in the right place. With them gone, "After" starts at 306.40 pt against LibreOffice's 306.54.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShapeFieldsAnchorCharactersLeaveNothingInTheLine()
    {
        using IDocument document = Open("word-features.doc");
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageParagraph paragraph = pages.Paragraphs
            .First(candidate => candidate.Text.StartsWith("Before the box.", StringComparison.Ordinal));

        paragraph.Text.ShouldNotContain('');
        paragraph.Text.ShouldNotContain('');
        paragraph.Text.ShouldBe("Before the box.  After the box.");
    }

    private static IDocument Open(string name)
        => new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    /// <summary>
    /// A document's body field table, read through the reader's own streams.
    /// </summary>
    /// <remarks>
    /// Parsed directly rather than reached through a frame, for the same reason
    /// <see cref="DocDrawingTests"/> reads the anchor table that way: what is under test is the decode
    /// of a table the document deliberately does not expose, and a caller wants frames.
    /// </remarks>
    private static Ww8FieldTypes FieldsOf(string name)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(name));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        byte[] wordDocument = StreamOf(file, DocReader.WordDocumentStreamName)!;
        Ww8Fib fib = Ww8Fib.Parse(wordDocument);
        byte[] table = StreamOf(file, fib.UsesTable1Stream ? "1Table" : "0Table") ?? [];

        if (!fib.Has(Ww8FibTable.BodyFields)) return Ww8FieldTypes.Empty;

        List<Diagnostic> diagnostics = [];
        int at = (int)fib.FileOffset(Ww8FibTable.BodyFields);
        int length = (int)fib.Length(Ww8FibTable.BodyFields);
        Ww8FieldTypes fields = Ww8FieldTypes.Parse(table.AsSpan(at, length), diagnostics);

        diagnostics.ShouldNotContain(
            diagnostic => diagnostic.Severity >= DiagnosticSeverity.Warning, $"{name} diagnostics");

        return fields;
    }

    private static byte[]? StreamOf(CompoundFile file, string name)
    {
        IPackagePart? part = file.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new();
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}
