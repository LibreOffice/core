using System.Text;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.MsBinary.Records;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Tests that a legacy PowerPoint text-range hyperlink becomes an EditEngine <em>field</em>, and
/// that a text range naming a hyperlink the deck does not declare becomes nothing at all.
/// </summary>
/// <remarks>
/// <para>
/// <c>PPTTextObj</c> builds a <c>SvxFieldItem(SvxURLField(…), EE_FEATURE_FIELD)</c> for an
/// <c>InteractiveInfo</c> whose <c>exHyperlinkId</c> matches an entry of
/// <c>SdrPowerPointImport::m_aHyperList</c> (<c>filter/source/msfilter/svdfppt.cxx</c>:6907-6941),
/// splits it across the <c>PPTCharPropSet</c>s the range covers (<c>:7069</c>, <c>:7090</c>),
/// imposes the colour scheme's hyperlink slot on each (<c>:7060</c>, <c>:7094</c>) and forces the
/// underline attribute on (<c>:7054-7056</c>).
/// </para>
/// <para>
/// <strong>The list lookup is the legacy twin of DrawingML's non-empty hyperlink property map,
/// and it is not a formality.</strong> Three of the corpus's 51 <c>.ppt</c> state text ranges —
/// 60 of them, 57 in <c>BUS-Chapter 05.ppt</c> alone — and declare no hyperlink whatsoever, and
/// 26.2.4.2 draws every one of those ranges in the body's own colour, word-broken and not
/// underlined. See <c>probes/ppt-autofit-r84/results.md</c>.
/// </para>
/// <para>
/// Synthetic records throughout: every <c>.ppt</c> in <c>tests/corpus</c> was written by
/// LibreOffice's own exporter, which writes no <c>InteractiveInfo</c> at all, so none of these
/// cases is reachable from a committed deck.
/// </para>
/// </remarks>
public class PptHyperlinkFieldTests
{
    /// <summary>The mask bit a character run sets for its colour, <c>PPT_CharAttr_FontColor</c>.</summary>
    private const uint StatesCharacterColour = 0x0004_0000;

    /// <summary>A literal colour word, which the format marks with <c>0xFE</c> in its top byte.</summary>
    private static uint Literal(byte red, byte green, byte blue)
        => 0xFE000000u | ((uint)blue << 16) | ((uint)green << 8) | red;

    [Fact]
    public void TheDeclaredHyperlinkIdsAreReadFromTheDocumentsExObjList()
    {
        (DffRecordBuffer stream, DffRecordHeader document) = Document(ExObjList(13, 15, 17));

        PptHyperlinks.Read(stream, document).ShouldBe([13u, 15u, 17u], ignoreOrder: true);
    }

    [Fact]
    public void ADocumentWithNoExObjListDeclaresNoHyperlinkAtAll()
    {
        // The `BUS-Chapter 05.ppt` shape: 57 live text ranges and not one hyperlink declared, so
        // `m_aHyperList` is empty and the loop at svdfppt.cxx:6907 never enters its body.
        (DffRecordBuffer stream, DffRecordHeader document) = Document([]);

        PptHyperlinks.Read(stream, document).ShouldBeEmpty();
    }

    [Fact]
    public void ATextRangeNamingADeclaredHyperlinkCoversItsCharacters()
    {
        PptTextRun run = Read("Visit http://example.org now", link: 6, from: 6, to: 24, declared: [6]);

        run.Hyperlinks.ShouldNotBeNull().ShouldHaveSingleItem().ShouldBe(new PptHyperlinkRange(6, 24));
        run.IsLinked(5).ShouldBeFalse();
        run.IsLinked(6).ShouldBeTrue();
        run.IsLinked(23).ShouldBeTrue();
        run.IsLinked(24).ShouldBeFalse();
    }

    [Fact]
    public void ATextRangeNamingAHyperlinkTheDeckDoesNotDeclareIsNotAField()
    {
        // The condition the brief this round answered asked for, and the one that keeps 60 of the
        // corpus's 171 live text ranges as ordinary text.
        PptTextRun run = Read("Visit http://example.org now", link: 6, from: 6, to: 24, declared: [9]);

        (run.Hyperlinks ?? []).ShouldBeEmpty();
    }

    [Fact]
    public void ATextRangeWhoseEndIsZeroIsNotARangeAtAll()
    {
        // svdfppt.cxx:6926 makes no field entry for a zero end, so the characters are drawn as
        // they stand rather than from position zero.
        PptTextRun run = Read("Visit http://example.org now", link: 6, from: 0, to: 0, declared: [6]);

        (run.Hyperlinks ?? []).ShouldBeEmpty();
    }

    [Fact]
    public void TheRangeRecordMustBeTheInteractiveInfosNextSiblingRatherThanAnywhereAfterIt()
    {
        // svdfppt.cxx:6911-6921 seeks to the end of the container and requires the very next
        // header to be a TxInteractiveInfoAtom, putting the record back when it is not.
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("Visit http://example.org now");
        builder.InteractiveInfo(6);
        builder.TextRuler();                    // anything at all between the two
        builder.TextRange(6, 24);

        (builder.Read([6]).ShouldNotBeNull().Hyperlinks ?? []).ShouldBeEmpty();
    }

    [Fact]
    public void ExtractionResolvesNoHyperlinkBecauseItDrawsNothing()
    {
        // The reader is shared with `PptContentBuilder`, which passes no declared set: a field's
        // representation is the run's own characters, so extraction reports the same text either
        // way and must not pay for the walk.
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("Visit http://example.org now");
        builder.InteractiveInfo(6);
        builder.TextRange(6, 24);

        (builder.Read(null).ShouldNotBeNull().Hyperlinks ?? []).ShouldBeEmpty();
    }

    [Fact]
    public void ALinkedStretchIsItsOwnFieldRunEvenInsideOneCharacterRun()
    {
        // One character property run covers the whole paragraph and the link covers the middle of
        // it, so the portion has to be split three ways -- which is what svdfppt.cxx:7080-7091
        // does when it clones the PPTCharPropSet at the range's end.
        SlideParagraph paragraph = Paragraph(from: 6, to: 24, declared: [6]);

        paragraph.Runs.Count.ShouldBe(3);
        paragraph.Runs[0].IsField.ShouldBeFalse();
        paragraph.Runs[1].IsField.ShouldBeTrue();
        paragraph.Runs[2].IsField.ShouldBeFalse();
        paragraph.Runs[1].Start.ShouldBe(6);
        paragraph.Runs[1].Length.ShouldBe(18);
    }

    [Fact]
    public void AFieldRunTakesTheSchemesHyperlinkSlotAndAnUnderlineTheFileNeverStated()
    {
        SlideParagraph paragraph = Paragraph(from: 6, to: 24, declared: [6]);

        // The default scheme's slot six -- PPT_COLSCHEME_A_UND_HYPERLINK, svdfppt.cxx:145.
        paragraph.Runs[1].Colour.ShouldBe(new Colour(0x00, 0x00, 0x99));
        paragraph.Runs[1].IsUnderlined.ShouldBeTrue();

        // And the run either side keeps the red the file states, which is the control that a
        // reader recolouring the whole portion would fail.
        paragraph.Runs[0].Colour.ShouldBe(new Colour(0xFF, 0x00, 0x00));
        paragraph.Runs[0].IsUnderlined.ShouldBeFalse();
    }

    [Fact]
    public void AFieldRunLosesTheEmphasisItStatedForItself()
    {
        // svdfppt.cxx:7054-7056 sets the underline bit in the attribute mask and then *assigns*
        // `mnFlags = 1 << PPT_CharAttr_Underline` rather than or-ing it, so every emphasis the
        // portion stated for itself is turned off and stated as off. A bold linked run is drawn
        // light.
        SlideParagraph paragraph = Paragraph(
            from: 6, to: 24, declared: [6], emphasis: RunEmphasis.Bold | RunEmphasis.Italic);

        paragraph.Runs[0].Weight.ShouldBe(700);
        paragraph.Runs[0].IsItalic.ShouldBeTrue();
        paragraph.Runs[1].Weight.ShouldBe(400);
        paragraph.Runs[1].IsItalic.ShouldBeFalse();
        paragraph.Runs[1].IsUnderlined.ShouldBeTrue();
    }

    [Fact]
    public void ABulletWhoseParagraphOpensOnALinkKeepsTheColourTheLinkReplaced()
    {
        // A soft bullet takes the first portion's colour, and `GetAttrib` reaches past the link
        // for it (svdfppt.cxx:6037-6042). Without that every bulleted paragraph beginning with a
        // hyperlink would take a bullet in the scheme's hyperlink slot.
        SlideParagraph paragraph = Paragraph(from: 0, to: 5, declared: [6], bulleted: true);

        paragraph.Runs[0].IsField.ShouldBeTrue();
        paragraph.Runs[0].Colour.ShouldBe(new Colour(0x00, 0x00, 0x99));
        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBe(new Colour(0xFF, 0x00, 0x00));
    }

    [Fact]
    public void ABulletWhoseParagraphDoesNotOpenOnALinkStillTakesItsRunsColour()
    {
        // The control: a null marker colour is what SlideMarker already spells "the first run's",
        // and the rule above must not turn every bullet into an explicit colour.
        SlideParagraph paragraph = Paragraph(from: 6, to: 24, declared: [6], bulleted: true);

        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBeNull();
    }

    /// <summary>
    /// One paragraph reading <c>Visit http://example.org now</c> in red, with a text-range
    /// hyperlink over <c>[from, to)</c>.
    /// </summary>
    /// <param name="from">The link's first character.</param>
    /// <param name="to">One past its last.</param>
    /// <param name="declared">The hyperlink identifiers the deck declares.</param>
    /// <param name="emphasis">The emphasis the character run states for itself.</param>
    /// <param name="bulleted">Whether the paragraph draws a bullet.</param>
    private static SlideParagraph Paragraph(
        int from,
        int to,
        uint[] declared,
        RunEmphasis emphasis = RunEmphasis.None,
        bool bulleted = false)
    {
        const string Text = "Visit http://example.org now";

        PptTextRun read = Read(Text, link: 6, from: from, to: to, declared: declared);

        PptTextRun run = new(
            PptTextKind.Body,
            Text,
            [
                new PptParagraphRun(
                    Length: Text.Length,
                    Depth: 0,
                    HasBullet: bulleted,
                    BulletCharacter: '•'),
            ],
            [
                new PptCharacterRun(
                    Length: Text.Length,
                    Emphasis: emphasis,
                    Stated: emphasis,
                    Mask: StatesCharacterColour,
                    Colour: Literal(0xFF, 0x00, 0x00)),
            ],
            Hyperlinks: read.Hyperlinks);

        SlideTextBody body = PptTextBody.Build(
            run,
            null,
            PptColourScheme.Default,
            PptFontTable.Empty,
            Margins.Zero,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();

        return body.Paragraphs[0];
    }

    /// <summary>One client textbox holding a text-range hyperlink, read.</summary>
    /// <param name="text">The shape's characters.</param>
    /// <param name="link">The <c>exHyperlinkId</c> the <c>InteractiveInfoAtom</c> names.</param>
    /// <param name="from">The range's start.</param>
    /// <param name="to">The range's end.</param>
    /// <param name="declared">The identifiers the deck's <c>ExObjList</c> declares.</param>
    private static PptTextRun Read(string text, uint link, int from, int to, uint[] declared)
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars(text);
        builder.InteractiveInfo(link);
        builder.TextRange(from, to);

        return builder.Read(declared).ShouldNotBeNull();
    }

    /// <summary>A <c>Document</c> container holding <paramref name="payload"/>, and its buffer.</summary>
    /// <remarks>Shared with <see cref="PptHyperlinkBlobTests"/>, which caps the same list.</remarks>
    internal static (DffRecordBuffer Stream, DffRecordHeader Document) Document(
        IEnumerable<byte> payload)
    {
        DffRecordBuffer buffer = new([.. Container(PptRecordTypes.Document, payload)]);
        buffer.TryReadHeader(0, out DffRecordHeader header).ShouldBeTrue();
        return (buffer, header);
    }

    /// <summary>An <c>ExObjList</c> declaring one <c>ExHyperlink</c> per identifier.</summary>
    internal static List<byte> ExObjList(params uint[] ids)
    {
        List<byte> links = [];
        foreach (uint id in ids)
        {
            links.AddRange(Container(
                PptRecordTypes.ExHyperlink,
                Record(PptRecordTypes.ExHyperlinkAtom, BitConverter.GetBytes(id))));
        }

        return Container(PptRecordTypes.ExObjList, links);
    }

    private static List<byte> Container(ushort type, IEnumerable<byte> payload)
    {
        List<byte> body = [.. payload];
        return
        [
            0x0F, 0x00,
            (byte)type, (byte)(type >> 8),
            .. BitConverter.GetBytes((uint)body.Count),
            .. body,
        ];
    }

    private static List<byte> Record(ushort type, byte[] payload) =>
    [
        0x00, 0x00,
        (byte)type, (byte)(type >> 8),
        .. BitConverter.GetBytes((uint)payload.Length),
        .. payload,
    ];

    /// <summary>Assembles the records of one client textbox.</summary>
    private sealed class RecordBuilder
    {
        private readonly List<byte> _bytes = [];

        public void TextHeader(PptTextKind kind)
            => Record(PptRecordTypes.TextHeaderAtom, [.. BitConverter.GetBytes((uint)kind)]);

        public void TextChars(string text)
            => Record(PptRecordTypes.TextCharsAtom, Encoding.Unicode.GetBytes(text));

        /// <summary>An <c>InteractiveInfo</c> naming one of the deck's hyperlinks.</summary>
        public void InteractiveInfo(uint id)
        {
            List<byte> atom =
            [
                .. BitConverter.GetBytes(0u),          // nSoundRef
                .. BitConverter.GetBytes(id),          // nExHyperlinkId
                4, 0, 0, 0,                            // action, oleVerb, jump, flags
                8, 0, 0, 0,                            // hyperlinkType and three unread bytes
            ];

            List<byte> body =
            [
                0x00, 0x00,
                unchecked((byte)PptRecordTypes.InteractiveInfoAtom),
                (byte)(PptRecordTypes.InteractiveInfoAtom >> 8),
                .. BitConverter.GetBytes((uint)atom.Count),
                .. atom,
            ];

            _bytes.AddRange(
            [
                0x0F, 0x00,
                unchecked((byte)PptRecordTypes.InteractiveInfo),
                (byte)(PptRecordTypes.InteractiveInfo >> 8),
                .. BitConverter.GetBytes((uint)body.Count),
                .. body,
            ]);
        }

        /// <summary>The <c>TxInteractiveInfoAtom</c> that must follow it.</summary>
        public void TextRange(int from, int to)
            => Record(
                PptRecordTypes.TxInteractiveInfoAtom,
                [.. BitConverter.GetBytes((uint)from), .. BitConverter.GetBytes((uint)to)]);

        /// <summary>An empty ruler, used only to separate two records.</summary>
        public void TextRuler() => Record(PptRecordTypes.TextRulerAtom, BitConverter.GetBytes(0u));

        public PptTextRun? Read(uint[]? declared)
        {
            DffRecordBuffer buffer = new([.. _bytes]);
            return PptTextReader.Read(
                buffer, 0, buffer.Length,
                hyperlinks: declared is null ? null : new HashSet<uint>(declared));
        }

        private void Record(ushort type, byte[] payload)
        {
            _bytes.Add(0);
            _bytes.Add(0);
            _bytes.Add((byte)type);
            _bytes.Add((byte)(type >> 8));
            _bytes.AddRange(BitConverter.GetBytes((uint)payload.Length));
            _bytes.AddRange(payload);
        }
    }
}
