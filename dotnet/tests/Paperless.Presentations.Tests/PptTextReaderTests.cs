using System.Text;
using Paperless.Core.Extraction;
using Paperless.MsBinary.Records;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the text records a shape's client textbox holds.
/// </summary>
/// <remarks>
/// Synthetic records, because the corpus cannot reach these cases: LibreOffice's PPT exporter
/// writes every string as a <c>TextCharsAtom</c> and never as the eight-bit
/// <c>TextBytesAtom</c>, so the byte form — and with it the code-page question that is the
/// commonest source of mangled legacy text — has no coverage from a LibreOffice-written file.
/// </remarks>
public class PptTextReaderTests
{
    [Fact]
    public void AnEightBitRunIsDecodedAsWindows1252RatherThanAsLatin1()
    {
        // 0x92 is a right single quotation mark in Windows-1252 and an unassigned control in
        // ISO-8859-1. LibreOffice decodes the atom as Windows-1252 at svdfppt.cxx:5273, and a
        // reader that chose Latin-1 would turn every curly apostrophe into a control character.
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Other);
        builder.TextBytes([.. "It"u8, 0x92, .. "s here"u8]);

        PptTextRun run = builder.Read().ShouldNotBeNull();
        run.Text.ShouldBe("It’s here");
        run.Kind.ShouldBe(PptTextKind.Other);
    }

    [Fact]
    public void TextStopsAtTheFirstNulRatherThanRunningToTheRecordLength()
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("Visible\0Padding that is not text");

        builder.Read().ShouldNotBeNull().Text.ShouldBe("Visible");
    }

    [Fact]
    public void CarriageReturnsSplitParagraphsAndVerticalTabsDoNot()
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("One\rTwo\vstill two\rThree\r");

        List<ContentParagraph> paragraphs = PptTextReader.ToParagraphs(builder.Read().ShouldNotBeNull());

        // A vertical tab is a line break inside a paragraph — what PowerPoint stores when a
        // title is broken by hand — so it must not start a new one. The run ends with a
        // return, and the empty paragraph that implies is an artefact rather than content.
        paragraphs.Count.ShouldBe(3);
        paragraphs[1].GetText().ShouldBe("Two\nstill two\n");
        paragraphs[2].GetText().ShouldBe("Three\n");
    }

    [Fact]
    public void CharacterRunsAreSlicedPerParagraphRatherThanAcrossTheWholeRun()
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Other);
        builder.TextChars("Plain\rBold text");

        // One paragraph run covering both paragraphs, and two character runs whose boundary
        // falls inside the second: "Plain\r" plain, then bold from "Bold" onwards.
        StyleBuilder style = new();
        style.Paragraph(count: 16, depth: 0);
        style.Characters(count: 6, flags: 0);
        style.Characters(count: 9, flags: 0x0001);
        builder.Style(style);

        List<ContentParagraph> paragraphs = PptTextReader.ToParagraphs(builder.Read().ShouldNotBeNull());

        paragraphs.Count.ShouldBe(2);
        paragraphs[0].Children.OfType<ContentRun>().Single().Emphasis.ShouldBe(RunEmphasis.None);
        paragraphs[1].Children.OfType<ContentRun>().Single().Emphasis.ShouldBe(RunEmphasis.Bold);
    }

    [Fact]
    public void ARunWhoseCountsUnderShootTheTextStillReportsEveryCharacter()
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Other);
        builder.TextChars("Counted and then some");

        StyleBuilder style = new();
        style.Paragraph(count: 22, depth: 0);
        style.Characters(count: 7, flags: 0x0002);
        builder.Style(style);

        List<ContentParagraph> paragraphs = PptTextReader.ToParagraphs(builder.Read().ShouldNotBeNull());

        // Dropping the uncounted tail would lose text; a writer that under-counts is commoner
        // than one that over-counts, so the remainder keeps the run's default formatting.
        paragraphs[0].GetText().ShouldBe("Counted and then some\n");
        paragraphs[0].Children.OfType<ContentRun>().First().Emphasis.ShouldBe(RunEmphasis.Italic);
        paragraphs[0].Children.OfType<ContentRun>().Last().Emphasis.ShouldBe(RunEmphasis.None);
    }

    [Fact]
    public void AParagraphsBulletAndDepthComeFromTheOptionalFieldsTheMaskNames()
    {
        RecordBuilder builder = new();
        builder.TextHeader(PptTextKind.Body);
        builder.TextChars("Point\r");

        // Bullet flags, a bullet character and a left margin, in the order the mask's bits are
        // numbered rather than the order the bits appear — reading them in bit order instead
        // puts every later field two bytes out and yields a plausible wrong indent.
        StyleBuilder style = new();
        style.Paragraph(count: 6, depth: 2, mask: 0x0000018F, [0x000F, 0xF0B7, 0x0088]);
        style.Characters(count: 6, flags: 0);
        builder.Style(style);

        List<ContentParagraph> paragraphs = PptTextReader.ToParagraphs(builder.Read().ShouldNotBeNull());

        paragraphs[0].ListLevel.ShouldBe(2);
        paragraphs[0].ListMarker.ShouldBe("•");
    }

    /// <summary>Assembles the records of one client textbox.</summary>
    private sealed class RecordBuilder
    {
        private readonly List<byte> _bytes = [];

        public void TextHeader(PptTextKind kind)
            => Record(PptRecordTypes.TextHeaderAtom, [.. BitConverter.GetBytes((uint)kind)]);

        public void TextChars(string text)
            => Record(PptRecordTypes.TextCharsAtom, Encoding.Unicode.GetBytes(text));

        public void TextBytes(byte[] bytes) => Record(PptRecordTypes.TextBytesAtom, bytes);

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
            _characters.AddRange(BitConverter.GetBytes(flags == 0 ? 0u : 0x0000FFFFu & flags));
            if (flags != 0) _characters.AddRange(BitConverter.GetBytes(flags));
        }

        public byte[] Build() => [.. _paragraphs, .. _characters];
    }
}
