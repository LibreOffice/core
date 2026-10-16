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
/// Tests that a binary PowerPoint bullet takes its colour from the file's <em>flag</em> rather
/// than from the colour word beside it.
/// </summary>
/// <remarks>
/// <para>
/// PowerPoint writes a <c>bulletColor</c> into a paragraph's properties whether or not the bullet
/// has a colour of its own, and gates it behind a separate bit — <c>PPT_ParaAttr_BuHardColor</c>,
/// bit two of the bullet-flags word the low four mask bits share. With the flag clear the word is
/// meaningless and the bullet is drawn in the colour of the paragraph's <em>first character
/// run</em>, which is how a red heading gets a red bullet without the file ever saying so
/// (<c>PPTParagraphObj::GetAttrib</c>, <c>filter/source/msfilter/svdfppt.cxx:5891-5916</c> for a
/// paragraph's own set and <c>:6019-6055</c> for the fall-through to the master's level).
/// </para>
/// <para>
/// <strong>The cost of reading the word regardless is not subtle and the gate cannot see it.</strong>
/// Measured on <c>slides/batch-007/ppt/architecture6.ppt</c>: all eighty of its bullets came out
/// <c>#000000</c> against a reference drawing seventy-four in the run's own <c>#46424D</c> and six
/// in red. Across the slides track the same fault reached twenty-six of the corpus's fifty-two
/// binary decks and 935 bullet glyphs, and not one word count, page count or font-embedding column
/// moves with it.
/// </para>
/// <para>
/// Synthetic records throughout, and deliberately so. Every <c>.ppt</c> in <c>tests/corpus</c> was
/// written by LibreOffice's own exporter, which sets the hard-colour flag on everything it emits,
/// so the case that matters — the flag <em>clear</em> beside a colour word that must be ignored —
/// cannot be reached from a committed deck at all.
/// </para>
/// </remarks>
public class PptBulletColourTests
{
    /// <summary>The mask bit for the shared bullet-flags word, <c>PPT_ParaAttr_BulletOn</c>.</summary>
    private const uint StatesBulletOn = 0x0000_0001;

    /// <summary>The mask bit for <c>PPT_ParaAttr_BuHardColor</c>.</summary>
    private const uint StatesBulletHardColour = 0x0000_0004;

    /// <summary>The mask bit for <c>PPT_ParaAttr_BulletColor</c>.</summary>
    private const uint StatesBulletColour = 0x0000_0020;

    /// <summary>The mask bit a character run sets for its colour, <c>PPT_CharAttr_FontColor</c>.</summary>
    private const uint StatesCharacterColour = 0x0004_0000;

    /// <summary>Bit zero of the bullet-flags word: the paragraph draws a bullet.</summary>
    private const ushort BulletOnFlag = 0x0001;

    /// <summary>Bit two of the same word: the colour beside it is the bullet's own.</summary>
    private const ushort BulletHardColourFlag = 0x0004;

    /// <summary>A literal colour word, which the format marks with <c>0xFE</c> in its top byte.</summary>
    private static uint Literal(byte red, byte green, byte blue)
        => 0xFE000000u | ((uint)blue << 16) | ((uint)green << 8) | red;

    [Fact]
    public void TheBulletFlagsWordSurvivesTheReadRatherThanOnlyItsFirstBit()
    {
        // The low four mask bits share one word, and only its bit zero is "this paragraph is
        // bulleted". Bits one, two and three are BuHardFont, BuHardColor and BuHardHeight, and
        // each decides whether the value beside it means anything. Keeping bit zero alone is
        // what made every binary PowerPoint bullet take a colour its file had not asked for.
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("Point\r");

        StyleBuilder style = new();
        style.Paragraph(count: 6, depth: 0, mask: 0x0000000F, [BulletOnFlag | BulletHardColourFlag]);
        style.Characters(count: 6, flags: 0);
        builder.Style(style);

        PptParagraphRun paragraph = builder.Read().ShouldNotBeNull().Paragraphs[0];

        paragraph.HasBullet.ShouldBe(true);
        paragraph.BulletFlags.ShouldBe((ushort)(BulletOnFlag | BulletHardColourFlag));
    }

    [Fact]
    public void ABulletWhoseHardColourFlagIsClearTakesTheColourOfTheParagraphsFirstRun()
    {
        // The file states a blue bullet colour and leaves the flag clear, so the blue is a word
        // PowerPoint wrote and meant nothing by. LibreOffice ignores it and draws the bullet in
        // the run's red; a null marker colour is what SlideMarker already spells that with.
        SlideParagraph paragraph = Paragraph(
            bulletFlags: BulletOnFlag,
            mask: StatesBulletOn | StatesBulletColour);

        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBeNull();
        paragraph.Runs[0].Colour.ShouldBe(new Colour(0xFF, 0x00, 0x00));
    }

    [Fact]
    public void ABulletWhoseHardColourFlagIsSetKeepsTheColourTheFileStates()
    {
        // The same blue word, now with the flag that makes it the bullet's own. The run is still
        // red, so a reader that had simply stopped reading the word would fail here.
        SlideParagraph paragraph = Paragraph(
            bulletFlags: (ushort)(BulletOnFlag | BulletHardColourFlag),
            mask: StatesBulletOn | StatesBulletHardColour | StatesBulletColour);

        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBe(new Colour(0x00, 0x00, 0xFF));
        paragraph.Runs[0].Colour.ShouldBe(new Colour(0xFF, 0x00, 0x00));
    }

    [Fact]
    public void TheHardColourFlagFallsThroughToTheMastersLevelWhenTheParagraphDoesNotNameIt()
    {
        // A paragraph states its bullet and its colour without naming BuHardColor at all, which
        // is the common shape: the flag lives once on the master's outline level and every slide
        // under it inherits. Reading only the paragraph's own word would treat the level's
        // "this colour is hard" as absent and hand the bullet the run's colour instead.
        PptStyleSheet styles = StyleSheetWithLevelFlags(
            (ushort)(BulletOnFlag | BulletHardColourFlag));

        SlideParagraph paragraph = Paragraph(
            bulletFlags: BulletOnFlag,
            mask: StatesBulletOn | StatesBulletColour,
            styles: styles);

        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBe(new Colour(0x00, 0x00, 0xFF));
    }

    [Fact]
    public void AMastersLevelWithTheFlagClearLeavesTheBulletOnItsRunsColour()
    {
        // The mirror of the test above, and the one that fails on a reader which read the level's
        // flags word but tested the wrong bit: bit zero is set on both levels here.
        PptStyleSheet styles = StyleSheetWithLevelFlags(BulletOnFlag);

        SlideParagraph paragraph = Paragraph(
            bulletFlags: BulletOnFlag,
            mask: StatesBulletOn | StatesBulletColour,
            styles: styles);

        paragraph.Marker.ShouldNotBeNull().Colour.ShouldBeNull();
    }

    /// <summary>
    /// One bulleted paragraph reading "Point", drawn in red, whose bullet states blue.
    /// </summary>
    /// <param name="bulletFlags">The bullet-flags word the paragraph carries.</param>
    /// <param name="mask">Which properties the paragraph claims to have stated.</param>
    /// <param name="styles">The master's style sheet, or null for the built-in defaults.</param>
    private static SlideParagraph Paragraph(
        ushort bulletFlags, uint mask, PptStyleSheet? styles = null)
    {
        PptTextRun run = new(
            PptTextKind.Body,
            "Point\r",
            [
                new PptParagraphRun(
                    Length: 6,
                    Depth: 0,
                    HasBullet: true,
                    BulletCharacter: '•',
                    Mask: mask,
                    BulletColour: Literal(0x00, 0x00, 0xFF),
                    BulletFlags: bulletFlags),
            ],
            [
                new PptCharacterRun(
                    Length: 6,
                    Emphasis: RunEmphasis.None,
                    Stated: RunEmphasis.None,
                    Mask: StatesCharacterColour,
                    Colour: Literal(0xFF, 0x00, 0x00)),
            ]);

        SlideTextBody body = PptTextBody.Build(
            run,
            styles,
            PptColourScheme.Default,
            PptFontTable.Empty,
            Margins.Zero,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();

        return body.Paragraphs[0];
    }

    /// <summary>
    /// A style sheet whose body outline levels all carry <paramref name="flags"/> as their
    /// bullet-flags word, built from a synthetic <c>TxMasterStyleAtom</c>.
    /// </summary>
    private static PptStyleSheet StyleSheetWithLevelFlags(ushort flags)
    {
        List<byte> atom = [.. BitConverter.GetBytes((ushort)1)];
        atom.AddRange(BitConverter.GetBytes(0x0000000Fu));   // the shared bullet-flags word
        atom.AddRange(BitConverter.GetBytes(flags));
        atom.AddRange(BitConverter.GetBytes(0u));            // the level's character mask

        List<byte> master = Container(
            PptRecordTypes.MainMaster,
            Record(PptRecordTypes.TxMasterStyleAtom, (ushort)PptTextKind.Body, [.. atom]));

        DffRecordBuffer buffer = new([.. master]);
        buffer.TryReadHeader(0, out DffRecordHeader header).ShouldBeTrue();
        return PptStyleSheet.Read(buffer, header, null);
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

    private static List<byte> Record(ushort type, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)(instance << 4);
        return
        [
            (byte)versionAndInstance, (byte)(versionAndInstance >> 8),
            (byte)type, (byte)(type >> 8),
            .. BitConverter.GetBytes((uint)payload.Length),
            .. payload,
        ];
    }

    /// <summary>Assembles the records of one client textbox.</summary>
    private sealed class RecordBuilder
    {
        private readonly List<byte> _bytes = [];

        public void TextHeader(PptTextKind kind)
            => Record(PptRecordTypes.TextHeaderAtom, [.. BitConverter.GetBytes((uint)kind)]);

        public void TextChars(string text)
            => Record(PptRecordTypes.TextCharsAtom, Encoding.Unicode.GetBytes(text));

        public void Style(StyleBuilder style)
            => Record(PptRecordTypes.StyleTextPropAtom, style.Build());

        public PptTextRun? Read()
        {
            DffRecordBuffer buffer = new([.. _bytes]);
            return PptTextReader.Read(buffer, 0, buffer.Length);
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

    /// <summary>Assembles a style atom: the paragraph runs, then the character runs.</summary>
    private sealed class StyleBuilder
    {
        private readonly List<byte> _paragraphs = [];
        private readonly List<byte> _characters = [];

        public void Paragraph(int count, int depth, uint mask = 0, ushort[]? fields = null)
        {
            _paragraphs.AddRange(BitConverter.GetBytes((uint)count));
            _paragraphs.AddRange(BitConverter.GetBytes((ushort)depth));
            _paragraphs.AddRange(BitConverter.GetBytes(mask));
            foreach (ushort field in fields ?? []) _paragraphs.AddRange(BitConverter.GetBytes(field));
        }

        public void Characters(int count, ushort flags)
        {
            _characters.AddRange(BitConverter.GetBytes((uint)count));
            _characters.AddRange(BitConverter.GetBytes(0u));
            _characters.AddRange(BitConverter.GetBytes(flags));
        }

        public byte[] Build() => [.. _paragraphs, .. _characters];
    }
}
