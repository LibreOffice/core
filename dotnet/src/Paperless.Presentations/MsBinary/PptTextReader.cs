using System.Text;
using Paperless.Core.Extraction;
using Paperless.Core.Numbering;
using Paperless.MsBinary.Records;
using Paperless.Text.Encodings;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// One text run as PowerPoint stores it: the characters, and the paragraph and character
/// property runs that divide them.
/// </summary>
/// <param name="Kind">Which master style the run inherits from, per its <c>TextHeaderAtom</c>.</param>
/// <param name="Text">The characters, with the terminator and any trailing NULs removed.</param>
/// <param name="Paragraphs">The paragraph property runs, in order.</param>
/// <param name="Characters">The character property runs, in order.</param>
public sealed record PptTextRun(
    PptTextKind Kind,
    string Text,
    IReadOnlyList<PptParagraphRun> Paragraphs,
    IReadOnlyList<PptCharacterRun> Characters);

/// <summary>One paragraph's properties, covering <paramref name="Length"/> characters.</summary>
/// <param name="Length">How many characters the run covers, including its terminating return.</param>
/// <param name="Depth">The outline level, zero for the first.</param>
/// <param name="HasBullet">Whether the paragraph carries a bullet.</param>
/// <param name="BulletCharacter">The bullet's character, when the paragraph states one.</param>
public readonly record struct PptParagraphRun(
    int Length, int Depth, bool HasBullet, char? BulletCharacter);

/// <summary>One character run's emphasis, covering <paramref name="Length"/> characters.</summary>
/// <param name="Length">How many characters the run covers.</param>
/// <param name="Emphasis">The emphasis the run states.</param>
public readonly record struct PptCharacterRun(int Length, RunEmphasis Emphasis);

/// <summary>
/// Reads the text records inside a shape's client textbox.
/// </summary>
/// <remarks>
/// <para>
/// A run's characters arrive in one of two atoms: <c>TextCharsAtom</c> holding UTF-16, or
/// <c>TextBytesAtom</c> holding one byte per character. The byte form is <em>not</em> in the
/// document's code page — LibreOffice decodes it as Windows-1252
/// (<c>filter/source/msfilter/svdfppt.cxx:5273</c>), and the specification defines it as the low
/// byte of a UTF-16 unit, which agrees for everything except the 0x80–0x9F range where
/// Windows-1252 has printable characters and ISO-8859-1 has controls. Following LibreOffice
/// there means a curly quotation mark written by PowerPoint reads back as one.
/// </para>
/// <para>
/// Paragraph boundaries are carriage returns inside the text, not record boundaries. A vertical
/// tab is a soft line break within a paragraph — PowerPoint uses it where a title's text is
/// broken by hand.
/// </para>
/// </remarks>
public static class PptTextReader
{
    /// <summary>The character PowerPoint uses to end a paragraph.</summary>
    public const char ParagraphSeparator = '\r';

    /// <summary>The character PowerPoint uses for a line break inside a paragraph.</summary>
    public const char LineBreak = '\v';

    /// <summary>
    /// Reads the text records in a range, or null when the range holds no characters.
    /// </summary>
    /// <param name="stream">The document stream.</param>
    /// <param name="start">The first record's offset.</param>
    /// <param name="end">One past the last byte to consider.</param>
    public static PptTextRun? Read(DffRecordBuffer stream, int start, int end)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptTextKind kind = PptTextKind.Other;
        string? text = null;
        DffRecordHeader? style = null;

        foreach (DffRecordHeader record in stream.Range(start, end))
        {
            ReadOnlySpan<byte> content = stream.Content(record);

            switch (record.Type)
            {
                case PptRecordTypes.TextHeaderAtom:
                    uint stated = DffRecordBuffer.ReadUInt32(content);
                    kind = stated <= (uint)PptTextKind.QuarterBody
                        ? (PptTextKind)stated
                        : PptTextKind.Other;
                    break;

                case PptRecordTypes.TextCharsAtom:
                    text ??= DecodeUtf16(content);
                    break;

                case PptRecordTypes.TextBytesAtom:
                    text ??= DecodeBytes(content);
                    break;

                case PptRecordTypes.StyleTextPropAtom:
                    style ??= record;
                    break;

                default:
                    break;
            }
        }

        if (text is null || text.Length == 0) return null;

        (List<PptParagraphRun> paragraphs, List<PptCharacterRun> characters) =
            style is { } header
                ? ReadStyle(stream.Content(header), text.Length)
                : ([], []);

        return new PptTextRun(kind, text, paragraphs, characters);
    }

    /// <summary>Turns a run into content paragraphs, splitting on carriage returns.</summary>
    /// <remarks>
    /// Bullets become a list marker rather than being dropped, matching what the ODF path does
    /// with the same deck: a bullet is text the reader sees. A symbol-font bullet lives in a
    /// Private Use Area and means nothing outside that font, so it is normalised the same way.
    /// </remarks>
    public static List<ContentParagraph> ToParagraphs(PptTextRun run)
    {
        ArgumentNullException.ThrowIfNull(run);

        List<ContentParagraph> paragraphs = [];
        EmphasisMap emphasis = new(run.Characters);

        int paragraphIndex = 0;
        int start = 0;

        while (start <= run.Text.Length)
        {
            int stop = run.Text.IndexOf(ParagraphSeparator, start);
            int length = (stop < 0 ? run.Text.Length : stop) - start;

            PptParagraphRun properties = paragraphIndex < run.Paragraphs.Count
                ? run.Paragraphs[paragraphIndex]
                : default;

            ContentParagraph paragraph = new()
            {
                // Only outline text is a list: an ordinary text box that happens to inherit a
                // bullet from the master would otherwise report every line as a list item.
                ListLevel = properties.HasBullet ? properties.Depth : null,
                ListMarker = properties.HasBullet && properties.BulletCharacter is { } bullet
                    ? OutlineNumbers.NormaliseBullet(bullet.ToString())
                    : null,
            };

            foreach ((int runStart, int runLength, RunEmphasis runEmphasis)
                     in emphasis.Slice(start, length))
            {
                string slice = run.Text.Substring(runStart, runLength).Replace(LineBreak, '\n');
                if (slice.Length == 0) continue;
                paragraph.Children.Add(new ContentRun { Text = slice, Emphasis = runEmphasis });
            }

            paragraphs.Add(paragraph);
            paragraphIndex++;

            if (stop < 0) break;
            start = stop + 1;
        }

        // A run that ends with a return has one empty paragraph after it, which is an artefact
        // of the terminator rather than a paragraph the author wrote.
        if (paragraphs.Count > 1 && paragraphs[^1].Children.Count == 0) paragraphs.RemoveAt(paragraphs.Count - 1);

        return paragraphs;
    }

    /// <summary>Decodes a <c>TextCharsAtom</c>, stopping at the first NUL.</summary>
    private static string DecodeUtf16(ReadOnlySpan<byte> content)
    {
        int usable = Math.Min(content.Length, MaxTextBytes) & ~1;
        string text = Encoding.Unicode.GetString(content[..usable]);
        return Trim(text);
    }

    /// <summary>Decodes a <c>TextBytesAtom</c>, stopping at the first NUL.</summary>
    private static string DecodeBytes(ReadOnlySpan<byte> content)
    {
        int usable = Math.Min(content.Length, MaxTextBytes);
        string text = LegacyCodePages.Get(LegacyCodePages.WindowsWestern).GetString(content[..usable]);
        return Trim(text);
    }

    /// <summary>
    /// The most bytes one text atom may contribute.
    /// </summary>
    /// <remarks>
    /// LibreOffice clamps to 0xFFFE at <c>svdfppt.cxx:5211</c>, because the run offsets that
    /// index into the text are sixteen-bit and a longer atom cannot be addressed by them.
    /// </remarks>
    public const int MaxTextBytes = 0xFFFE;

    private static string Trim(string text)
    {
        int nul = text.IndexOf('\0', StringComparison.Ordinal);
        return nul >= 0 ? text[..nul] : text;
    }

    /// <summary>
    /// Reads a <c>StyleTextPropAtom</c>: the paragraph runs, then the character runs.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two variable-length arrays back to back, with nothing between them saying where the
    /// first ends. The only way to find the boundary is to add up the paragraph runs' counts
    /// until they cover the text, which means a single misread optional field silently reads
    /// the character array from the wrong place — the trap that costs the most time here,
    /// because the result parses and looks like formatting rather than like an error.
    /// </para>
    /// <para>
    /// A paragraph run's count includes its terminating carriage return, so the counts sum to
    /// the text length plus one. The character runs do not, and sum to the text length exactly.
    /// </para>
    /// </remarks>
    private static (List<PptParagraphRun>, List<PptCharacterRun>) ReadStyle(
        ReadOnlySpan<byte> content, int textLength)
    {
        List<PptParagraphRun> paragraphs = [];
        int position = 0;
        int covered = 0;

        while (covered <= textLength && position + 10 <= content.Length)
        {
            int count = (int)DffRecordBuffer.ReadUInt32(content[position..]);
            int depth = DffRecordBuffer.ReadUInt16(content[(position + 4)..]);
            position += 6;

            uint mask = DffRecordBuffer.ReadUInt32(content[position..]);
            position += 4;

            ushort bulletFlags = 0;
            char? bulletCharacter = null;

            if ((mask & 0x0000000F) != 0) bulletFlags = Take16(content, ref position);
            if ((mask & 0x00000080) != 0) bulletCharacter = (char)Take16(content, ref position);
            if ((mask & 0x00000010) != 0) Skip(ref position, 2);   // bullet typeface
            if ((mask & 0x00000040) != 0) Skip(ref position, 2);   // bullet size
            if ((mask & 0x00000020) != 0) Skip(ref position, 4);   // bullet colour
            if ((mask & 0x00000800) != 0) Skip(ref position, 2);   // alignment
            if ((mask & 0x00001000) != 0) Skip(ref position, 2);   // line spacing
            if ((mask & 0x00002000) != 0) Skip(ref position, 2);   // space before
            if ((mask & 0x00004000) != 0) Skip(ref position, 2);   // space after
            if ((mask & 0x00000100) != 0) Skip(ref position, 2);   // left margin
            if ((mask & 0x00000400) != 0) Skip(ref position, 2);   // indent
            if ((mask & 0x00008000) != 0) Skip(ref position, 2);   // default tab size
            if ((mask & 0x00100000) != 0)
            {
                // The tab-stop array counts itself in a leading word and then holds two words
                // per stop, so it is the one field whose size the mask alone does not give.
                int stops = Take16(content, ref position);
                Skip(ref position, stops * 4);
            }

            if ((mask & 0x00010000) != 0) Skip(ref position, 2);   // baseline
            if ((mask & 0x000E0000) != 0) Skip(ref position, 2);   // the three wrap flags share one word
            if ((mask & 0x00200000) != 0) Skip(ref position, 2);   // text direction

            if (position > content.Length) break;

            paragraphs.Add(new PptParagraphRun(
                Math.Max(count, 0),
                Math.Clamp(depth, 0, 8),
                (bulletFlags & 1) != 0,
                bulletCharacter));

            if (count <= 0) break;
            covered += count;
        }

        List<PptCharacterRun> characters = [];
        covered = 0;

        while (covered < textLength && position + 8 <= content.Length)
        {
            int count = (int)DffRecordBuffer.ReadUInt32(content[position..]);
            position += 4;
            uint mask = DffRecordBuffer.ReadUInt32(content[position..]);
            position += 4;

            RunEmphasis emphasis = RunEmphasis.None;

            // The flags word is present only when the mask's low half asks for something in it,
            // and its bits are the same bits — a mask that names bold but a flags word that does
            // not set it means bold is explicitly off.
            if ((mask & 0xFFFF) != 0)
            {
                ushort flags = Take16(content, ref position);
                if ((flags & 0x0001) != 0) emphasis |= RunEmphasis.Bold;
                if ((flags & 0x0002) != 0) emphasis |= RunEmphasis.Italic;
                if ((flags & 0x0004) != 0) emphasis |= RunEmphasis.Underline;
                if ((flags & 0x0100) != 0) emphasis |= RunEmphasis.Strikethrough;
            }

            if ((mask & 0x00010000) != 0) Skip(ref position, 2);   // typeface
            if ((mask & 0x00200000) != 0) Skip(ref position, 2);   // east-asian typeface
            if ((mask & 0x00400000) != 0) Skip(ref position, 2);   // ANSI typeface
            if ((mask & 0x00800000) != 0) Skip(ref position, 2);   // symbol typeface
            if ((mask & 0x00020000) != 0) Skip(ref position, 2);   // size
            if ((mask & 0x00040000) != 0) Skip(ref position, 4);   // colour

            if ((mask & 0x00080000) != 0)
            {
                short escapement = unchecked((short)Take16(content, ref position));
                if (escapement > 0) emphasis |= RunEmphasis.Superscript;
                else if (escapement < 0) emphasis |= RunEmphasis.Subscript;
            }

            if (position > content.Length) break;

            characters.Add(new PptCharacterRun(Math.Max(count, 0), emphasis));
            if (count <= 0) break;
            covered += count;
        }

        return (paragraphs, characters);
    }

    private static ushort Take16(ReadOnlySpan<byte> content, ref int position)
    {
        ushort value = position + 2 <= content.Length
            ? DffRecordBuffer.ReadUInt16(content[position..])
            : (ushort)0;
        position += 2;
        return value;
    }

    private static void Skip(ref int position, int bytes) => position += bytes;

    /// <summary>Maps a character offset in a run's text to the emphasis stated for it.</summary>
    private readonly struct EmphasisMap(IReadOnlyList<PptCharacterRun> runs)
    {
        /// <summary>The emphasis-uniform stretches of a range, in order.</summary>
        public IEnumerable<(int Start, int Length, RunEmphasis Emphasis)> Slice(int start, int length)
        {
            if (runs.Count == 0)
            {
                if (length > 0) yield return (start, length, RunEmphasis.None);
                yield break;
            }

            int end = start + length;
            int position = 0;

            foreach (PptCharacterRun run in runs)
            {
                int runEnd = position + run.Length;
                int from = Math.Max(position, start);
                int to = Math.Min(runEnd, end);
                if (to > from) yield return (from, to - from, run.Emphasis);
                position = runEnd;
                if (position >= end) yield break;
            }

            // Text past the last stated run keeps no emphasis; a writer that under-counts is
            // commoner than one that over-counts, and dropping the tail would lose characters.
            if (position < end) yield return (position, end - position, RunEmphasis.None);
        }
    }
}
