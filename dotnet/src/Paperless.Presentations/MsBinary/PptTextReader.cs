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
/// <remarks>
/// The fields past the bullet are the ones only layout reads, and they mean nothing on their own:
/// <paramref name="Mask"/> says which the run actually stated, and everything it does not name
/// falls through to the master's per-level defaults. A run states its alignment and its indents
/// far more often than it states its bullet, so treating a zero as "left, no indent" rather than
/// as "unstated" moves the text of every inherited paragraph in the deck.
/// </remarks>
/// <param name="Length">How many characters the run covers, including its terminating return.</param>
/// <param name="Depth">The outline level, zero for the first.</param>
/// <param name="HasBullet">
/// Whether the paragraph carries a bullet, or null when it states nothing and the master's
/// per-level default decides.
/// </param>
/// <param name="BulletCharacter">The bullet's character, when the paragraph states one.</param>
/// <param name="Mask">The property mask, which says which of the fields below were stated.</param>
/// <param name="Alignment">0 left, 1 centre, 2 right, 3 justified.</param>
/// <param name="LineFeed">A percentage when positive, eighths of a point when negative.</param>
/// <param name="SpaceBefore">The space above, in the same two units.</param>
/// <param name="SpaceAfter">The space below, in the same two units.</param>
/// <param name="TextOffset">Where the text starts, in master units of a 576th of an inch.</param>
/// <param name="BulletOffset">Where the bullet starts, in the same units.</param>
/// <param name="BulletFont">The bullet's index into the document's font collection.</param>
/// <param name="BulletHeight">The bullet's size as a percentage of the text's.</param>
/// <param name="BulletColour">The bullet's packed colour word.</param>
public readonly record struct PptParagraphRun(
    int Length,
    int Depth,
    bool? HasBullet,
    char? BulletCharacter,
    uint Mask = 0,
    ushort Alignment = 0,
    short LineFeed = 0,
    short SpaceBefore = 0,
    short SpaceAfter = 0,
    ushort TextOffset = 0,
    ushort BulletOffset = 0,
    ushort BulletFont = 0,
    ushort BulletHeight = 0,
    uint BulletColour = 0)
{
    /// <summary>Whether the run's mask names a property, so its value is the run's own.</summary>
    /// <param name="bit">The mask bit, as <c>PPT_ParaAttr_*</c> numbers them.</param>
    public bool States(uint bit) => (Mask & bit) != 0;
}

/// <summary>One character run's properties, covering <paramref name="Length"/> characters.</summary>
/// <param name="Length">How many characters the run covers.</param>
/// <param name="Emphasis">The emphasis the run states.</param>
/// <param name="Stated">
/// Which kinds of emphasis the run's mask claims. Everything else falls through to the master's
/// per-level defaults, which is where a PowerPoint title's boldness normally lives.
/// </param>
/// <param name="Mask">The property mask, which says which of the fields below were stated.</param>
/// <param name="FontIndex">The index into the document's font collection.</param>
/// <param name="FontHeight">The size in points.</param>
/// <param name="Colour">The packed colour word, in the text spelling.</param>
/// <param name="Escapement">
/// How far off the baseline the run sits, as a signed percentage of its size. Carried as well as
/// folded into <paramref name="Emphasis"/> because the magnitude is what places the run:
/// <c>svdfppt.cxx:5764-5775</c> puts the value straight into a <c>SvxEscapementItem</c>, and the
/// flag alone cannot say whether a file asked for 30% or for 100%.
/// </param>
public readonly record struct PptCharacterRun(
    int Length,
    RunEmphasis Emphasis,
    RunEmphasis Stated,
    uint Mask = 0,
    ushort FontIndex = 0,
    ushort FontHeight = 0,
    uint Colour = 0,
    short Escapement = 0)
{
    /// <summary>Whether the run's mask names a property, so its value is the run's own.</summary>
    /// <param name="bit">The mask bit, as <c>PPT_CharAttr_*</c> numbers them.</param>
    public bool States(uint bit) => (Mask & bit) != 0;
}

/// <summary>
/// What the running fields inside a run's text stand for on the page being drawn.
/// </summary>
/// <remarks>
/// PowerPoint writes a field as one asterisk in the text plus an atom beside it saying which
/// field that asterisk is (<c>svdfppt.cxx:6984</c> tests the character for <c>0x2a</c>). The
/// values are a property of the <em>page</em> rather than of the run, which is the whole point:
/// one shape on the master carries the slide number of every slide under it.
/// </remarks>
/// <param name="SlideNumber">What the slide-number field shows, normally the page's position.</param>
/// <param name="Date">What the date field shows.</param>
/// <param name="Header">What the header field shows.</param>
/// <param name="Footer">What the footer field shows.</param>
public readonly record struct PptFieldValues(
    string? SlideNumber = null,
    string? Date = null,
    string? Header = null,
    string? Footer = null)
{
    /// <summary>Whether any field has a value, so a run needs scanning for markers at all.</summary>
    public bool IsEmpty => SlideNumber is null && Date is null && Header is null && Footer is null;
}

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
    /// <param name="fields">
    /// What the run's running fields stand for on the page being read. Left empty by extraction,
    /// which has no page to resolve them against and reports the marker characters as they stand.
    /// </param>
    public static PptTextRun? Read(
        DffRecordBuffer stream, int start, int end, PptFieldValues fields = default)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptTextKind kind = PptTextKind.Other;
        string? text = null;
        DffRecordHeader? style = null;
        List<(int Position, string Value)>? markers = null;

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

                case PptRecordTypes.SlideNumberMCAtom:
                    Mark(ref markers, content, fields.SlideNumber);
                    break;

                case PptRecordTypes.GenericDateMCAtom:
                    Mark(ref markers, content, fields.Date);
                    break;

                case PptRecordTypes.HeaderMCAtom:
                    Mark(ref markers, content, fields.Header);
                    break;

                case PptRecordTypes.FooterMCAtom:
                    Mark(ref markers, content, fields.Footer);
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

        if (markers is not null) text = Substitute(text, markers, paragraphs, characters);

        return new PptTextRun(kind, text, paragraphs, characters);
    }

    /// <summary>The character a field occupies in the text until something resolves it.</summary>
    private const char FieldMarker = '*';

    /// <summary>Records where a field marker sits, when the page has a value to put there.</summary>
    private static void Mark(
        ref List<(int Position, string Value)>? markers, ReadOnlySpan<byte> content, string? value)
    {
        if (value is null || content.Length < 2) return;
        (markers ??= []).Add((DffRecordBuffer.ReadUInt16(content), value));
    }

    /// <summary>
    /// Replaces each field marker with what the page says it stands for.
    /// </summary>
    /// <remarks>
    /// Applied from the back so that earlier positions stay valid, and the property runs are
    /// stretched with the text: a run's count is a character count, so leaving it alone would put
    /// every character after a two-digit page number under the wrong paragraph's properties.
    /// A marker that is not the asterisk the format promises is left alone rather than
    /// overwriting a real character.
    /// </remarks>
    private static string Substitute(
        string text,
        List<(int Position, string Value)> markers,
        List<PptParagraphRun> paragraphs,
        List<PptCharacterRun> characters)
    {
        markers.Sort(static (a, b) => b.Position.CompareTo(a.Position));

        foreach ((int position, string value) in markers)
        {
            if (position < 0 || position >= text.Length) continue;
            if (text[position] != FieldMarker) continue;

            text = string.Concat(text.AsSpan(0, position), value, text.AsSpan(position + 1));

            int delta = value.Length - 1;
            if (delta == 0) continue;

            if (Covering(paragraphs, position, static r => r.Length) is { } p)
            {
                paragraphs[p] = paragraphs[p] with
                {
                    Length = Math.Max(paragraphs[p].Length + delta, 0),
                };
            }

            if (Covering(characters, position, static r => r.Length) is { } c)
            {
                characters[c] = characters[c] with
                {
                    Length = Math.Max(characters[c].Length + delta, 0),
                };
            }
        }

        return text;
    }

    /// <summary>The index of the run covering a character position, or null past the last.</summary>
    private static int? Covering<T>(List<T> runs, int position, Func<T, int> length)
    {
        int covered = 0;

        for (int i = 0; i < runs.Count; i++)
        {
            covered += Math.Max(length(runs[i]), 1);
            if (position < covered) return i;
        }

        return null;
    }

    /// <summary>Turns a run into content paragraphs, splitting on carriage returns.</summary>
    /// <remarks>
    /// Bullets become a list marker rather than being dropped, matching what the ODF path does
    /// with the same deck: a bullet is text the reader sees. A symbol-font bullet lives in a
    /// Private Use Area and means nothing outside that font, so it is normalised the same way.
    /// </remarks>
    /// <param name="run">The text run to convert.</param>
    /// <param name="styles">
    /// The style sheet of the master the run's page belongs to, when one was found. Everything
    /// the run does not state falls through to it, per outline level — the boldness of a title
    /// above all, which PowerPoint records once in the master rather than once per slide.
    /// </param>
    /// <param name="fonts">
    /// The document's font collection, which says whether a bullet's face carries the symbol
    /// character set — the difference between a filled circle and the letter sharing its code.
    /// </param>
    public static List<ContentParagraph> ToParagraphs(
        PptTextRun run, PptStyleSheet? styles = null, PptFontTable? fonts = null)
    {
        ArgumentNullException.ThrowIfNull(run);

        List<ContentParagraph> paragraphs = [];
        EmphasisMap emphasis = new(run.Characters);

        int start = 0;

        while (start <= run.Text.Length)
        {
            int stop = run.Text.IndexOf(ParagraphSeparator, start);
            int length = (stop < 0 ? run.Text.Length : stop) - start;

            PptParagraphRun properties = PropertiesAt(run.Paragraphs, start);

            PptParagraphLevel level = styles?.Paragraph(run.Kind, properties.Depth) ?? default;
            RunEmphasis inherited = styles?.Character(run.Kind, properties.Depth).Emphasis
                                    ?? RunEmphasis.None;

            bool bulleted = properties.HasBullet ?? level.HasBullet;
            char? marker = properties.BulletCharacter
                           ?? (level.BulletCharacter != 0 ? (char)level.BulletCharacter : null);
            ushort markerFont = properties.States(StatesBulletFont)
                ? properties.BulletFont
                : level.BulletFont;

            ContentParagraph paragraph = new()
            {
                // Only outline text is a list: an ordinary text box that happens to inherit a
                // bullet from the master would otherwise report every line as a list item.
                ListLevel = bulleted ? properties.Depth : null,
                ListMarker = bulleted && marker is { } bullet
                    ? OutlineNumbers.NormaliseBullet(Symbolised(bullet, fonts, markerFont).ToString())
                    : null,
            };

            foreach ((int runStart, int runLength, RunEmphasis runEmphasis)
                     in emphasis.Slice(start, length, inherited))
            {
                string slice = run.Text.Substring(runStart, runLength).Replace(LineBreak, '\n');
                if (slice.Length == 0) continue;
                paragraph.Children.Add(new ContentRun { Text = slice, Emphasis = runEmphasis });
            }

            paragraphs.Add(paragraph);

            if (stop < 0) break;
            start = stop + 1;
        }

        // A run that ends with a return has one empty paragraph after it, which is an artefact
        // of the terminator rather than a paragraph the author wrote.
        if (paragraphs.Count > 1 && paragraphs[^1].Children.Count == 0) paragraphs.RemoveAt(paragraphs.Count - 1);

        return paragraphs;
    }

    /// <summary>The mask bit a paragraph sets when it states its bullet's own face.</summary>
    private const uint StatesBulletFont = 0x0000_0010;

    /// <summary>
    /// A bullet character read the way its own face means it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A bullet whose face carries the symbol character set is a glyph slot, not a
    /// letter.</strong> PowerPoint stores Wingdings' filled circle as <c>0x6C</c>, which is the
    /// letter <c>l</c> in every other face; drawing it as one puts a lower-case L at the head of
    /// every bulleted line, which is what this reader used to do wherever the symbol font was not
    /// installed. LibreOffice moves the low byte into the Private Use Area first —
    /// <c>nBuChar &amp;= 0x00ff; nBuChar |= 0xf000;</c>
    /// (<c>PPTNumberFormatCreator::GetNumberFormat</c>, <c>svdfppt.cxx:3767-3771</c>) — and then
    /// resolves the absent face to OpenSymbol, whose slot there is a bullet.
    /// </para>
    /// <para>
    /// Moving it into the Private Use Area is all that is done here, because
    /// <see cref="OutlineNumbers.NormaliseBullet"/> already answers the second half the same way
    /// for every family: a code point that means nothing outside one font becomes U+2022. The
    /// alternative — carrying the Private Use Area code point through to the page as LibreOffice
    /// does — would need the symbol face substituted as well, and would put a character no
    /// consumer can interpret into extracted text.
    /// </para>
    /// <para>
    /// The same recode is owed to <c>Ww8Numbering</c>, whose <c>Symbol</c> and <c>Wingdings</c>
    /// bullets reach <see cref="OutlineNumbers.NormaliseBullet"/> by the same route and with the
    /// same result; it is not done here because that file belongs to the word-processing path.
    /// </para>
    /// </remarks>
    /// <param name="character">The bullet character the file states.</param>
    /// <param name="fonts">The document's font collection, or null when none was read.</param>
    /// <param name="font">The bullet's index into that collection.</param>
    internal static char Symbolised(char character, PptFontTable? fonts, ushort font)
        => fonts is not null && fonts.IsSymbol(font)
            ? (char)(0xF000 | (character & 0x00FF))
            : character;

    /// <summary>
    /// The paragraph properties covering the character at <paramref name="start"/>.
    /// </summary>
    /// <remarks>
    /// A paragraph property run is <em>not</em> one paragraph. Its count is a character count,
    /// and a writer is free to cover several paragraphs with one run — LibreOffice clones the
    /// property set at every carriage return inside the count
    /// (<c>filter/source/msfilter/svdfppt.cxx:5081-5090</c>). Pairing the <em>n</em>th run with
    /// the <em>n</em>th paragraph instead loses the depth and the bullet of every paragraph
    /// after the first such run: <c>sd/qa/unit/data/ppt/hanging-indent.ppt</c> writes two runs
    /// for three paragraphs and the third came out unindented and unbulleted where LibreOffice
    /// renders it at level two with a bullet.
    /// </remarks>
    private static PptParagraphRun PropertiesAt(IReadOnlyList<PptParagraphRun> runs, int start)
    {
        int position = 0;

        foreach (PptParagraphRun run in runs)
        {
            position += Math.Max(run.Length, 1);
            if (start < position) return run;
        }

        // Past the last run stated, the last one still stands; a writer that under-counts is
        // commoner than one that over-counts.
        return runs.Count > 0 ? runs[^1] : default;
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

            ushort? bulletFlags = null;
            char? bulletCharacter = null;
            ushort bulletFont = 0, bulletHeight = 0, alignment = 0, textOffset = 0, bulletOffset = 0;
            short lineFeed = 0, spaceBefore = 0, spaceAfter = 0;
            uint bulletColour = 0;

            if ((mask & 0x0000000F) != 0) bulletFlags = Take16(content, ref position);
            if ((mask & 0x00000080) != 0) bulletCharacter = (char)Take16(content, ref position);
            if ((mask & 0x00000010) != 0) bulletFont = Take16(content, ref position);
            if ((mask & 0x00000040) != 0) bulletHeight = Take16(content, ref position);
            if ((mask & 0x00000020) != 0) bulletColour = Take32(content, ref position);
            if ((mask & 0x00000800) != 0) alignment = (ushort)(Take16(content, ref position) & 3);
            if ((mask & 0x00001000) != 0) lineFeed = Signed(content, ref position);
            if ((mask & 0x00002000) != 0) spaceBefore = Signed(content, ref position);
            if ((mask & 0x00004000) != 0) spaceAfter = Signed(content, ref position);
            if ((mask & 0x00000100) != 0) textOffset = Take16(content, ref position);
            if ((mask & 0x00000400) != 0) bulletOffset = Take16(content, ref position);
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
                // A paragraph that names no bullet flags at all leaves the decision to the
                // master; one that names them and clears bit 0 has turned the bullet off.
                (mask & 0x00000001) != 0 ? (bulletFlags & 1) != 0 : null,
                bulletCharacter,
                mask,
                alignment,
                lineFeed,
                spaceBefore,
                spaceAfter,
                textOffset,
                bulletOffset,
                bulletFont,
                bulletHeight,
                bulletColour));

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

            ushort flags = 0;
            short escapement = 0;
            ushort fontIndex = 0, fontHeight = 0;
            uint colour = 0;

            // The flags word is present only when the mask's low half asks for something in it,
            // and its bits are the same bits — a mask that names bold but a flags word that does
            // not set it means bold is explicitly off.
            if ((mask & 0xFFFF) != 0) flags = Take16(content, ref position);

            if ((mask & 0x00010000) != 0) fontIndex = Take16(content, ref position);
            if ((mask & 0x00200000) != 0) Skip(ref position, 2);   // east-asian typeface
            if ((mask & 0x00400000) != 0) Skip(ref position, 2);   // ANSI typeface
            if ((mask & 0x00800000) != 0) Skip(ref position, 2);   // symbol typeface
            if ((mask & 0x00020000) != 0) fontHeight = Take16(content, ref position);
            if ((mask & 0x00040000) != 0) colour = Take32(content, ref position);

            if ((mask & 0x00080000) != 0)
            {
                escapement = unchecked((short)Take16(content, ref position));
            }

            if (position > content.Length) break;

            characters.Add(new PptCharacterRun(
                Math.Max(count, 0),
                PptCharacterStyle.ToEmphasis(flags, escapement),
                PptCharacterStyle.Stated(mask),
                mask,
                fontIndex,
                fontHeight,
                colour,
                escapement));
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

    private static uint Take32(ReadOnlySpan<byte> content, ref int position)
    {
        uint value = position + 4 <= content.Length
            ? DffRecordBuffer.ReadUInt32(content[position..])
            : 0u;
        position += 4;
        return value;
    }

    /// <summary>A word read as signed, which the line feed and the two distances are.</summary>
    private static short Signed(ReadOnlySpan<byte> content, ref int position)
        => unchecked((short)Take16(content, ref position));

    private static void Skip(ref int position, int bytes) => position += bytes;

    /// <summary>Maps a character offset in a run's text to the emphasis stated for it.</summary>
    private readonly struct EmphasisMap(IReadOnlyList<PptCharacterRun> runs)
    {
        /// <summary>The emphasis-uniform stretches of a range, in order.</summary>
        /// <param name="start">The first character offset of the range.</param>
        /// <param name="length">How many characters the range covers.</param>
        /// <param name="inherited">
        /// The master's emphasis for this paragraph's outline level, which stands wherever a
        /// run's own mask claims nothing.
        /// </param>
        public IEnumerable<(int Start, int Length, RunEmphasis Emphasis)> Slice(
            int start, int length, RunEmphasis inherited)
        {
            if (runs.Count == 0)
            {
                if (length > 0) yield return (start, length, inherited);
                yield break;
            }

            int end = start + length;
            int position = 0;

            foreach (PptCharacterRun run in runs)
            {
                int runEnd = position + run.Length;
                int from = Math.Max(position, start);
                int to = Math.Min(runEnd, end);
                if (to > from) yield return (from, to - from, Resolve(run, inherited));
                position = runEnd;
                if (position >= end) yield break;
            }

            // Text past the last stated run keeps the master's emphasis; a writer that
            // under-counts is commoner than one that over-counts, and dropping the tail would
            // lose characters.
            if (position < end) yield return (position, end - position, inherited);
        }

        /// <summary>
        /// One run's effective emphasis: its own where it claims one, the master's elsewhere.
        /// </summary>
        private static RunEmphasis Resolve(PptCharacterRun run, RunEmphasis inherited)
            => (inherited & ~run.Stated) | (run.Emphasis & run.Stated);
    }
}
