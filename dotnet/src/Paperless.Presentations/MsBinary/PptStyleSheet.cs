using Paperless.Core.Extraction;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// The character defaults one outline level of one master text style states.
/// </summary>
/// <param name="Flags">
/// The bit attributes, in the numbering <c>include/filter/msfilter/svdfppt.hxx:1416</c> gives:
/// bit 0 bold, 1 italic, 2 underline, 4 shadow, 8 strikeout, 9 embossed.
/// </param>
/// <param name="FontIndex">The index into the document's font collection.</param>
/// <param name="AsianFontIndex">The east-Asian or complex font index; 0xFFFF for none.</param>
/// <param name="FontHeight">The size in points.</param>
/// <param name="Colour">
/// The colour, either a literal <c>0x00BBGGRR</c> or, when the top byte is not zero, an index
/// into the page's colour scheme.
/// </param>
/// <param name="Escapement">Superscript above zero, subscript below it, as a percentage.</param>
public readonly record struct PptCharacterLevel(
    ushort Flags,
    ushort FontIndex,
    ushort AsianFontIndex,
    ushort FontHeight,
    uint Colour,
    short Escapement)
{
    /// <summary>The emphasis these defaults amount to.</summary>
    public RunEmphasis Emphasis => PptCharacterStyle.ToEmphasis(Flags, Escapement);
}

/// <summary>
/// The paragraph defaults one outline level of one master text style states.
/// </summary>
/// <remarks>
/// <para>
/// Extraction needs only the bullet; layout needs the rest, so everything
/// <c>PPTParaSheet::Read</c> keeps is kept here too
/// (<c>filter/source/msfilter/svdfppt.cxx:3925-4062</c>). Tab stops are still skipped rather
/// than stored — they are the one field whose size the mask alone does not give, and nothing
/// consumes them yet.
/// </para>
/// <para>
/// The measurements are in the format's own units and are converted by whoever uses them: the
/// two offsets are in master units of a 576th of an inch, the line feed and the two paragraph
/// distances are a percentage when positive and eighths of a point when negative, and the bullet
/// height is a percentage of the text's size.
/// </para>
/// </remarks>
/// <param name="BulletFlags">Bit 0 is "this level draws a bullet".</param>
/// <param name="BulletCharacter">The bullet's code point, in whatever font the level names.</param>
/// <param name="BulletFont">The bullet's index into the document's font collection.</param>
/// <param name="BulletHeight">The bullet's size as a percentage of the text's.</param>
/// <param name="BulletColour">The bullet's packed colour word.</param>
/// <param name="Alignment">0 left, 1 centre, 2 right, 3 justified.</param>
/// <param name="LineFeed">The line spacing.</param>
/// <param name="SpaceBefore">The space above the paragraph.</param>
/// <param name="SpaceAfter">The space below it.</param>
/// <param name="TextOffset">Where the paragraph's text starts, from the text rectangle's edge.</param>
/// <param name="BulletOffset">Where its bullet starts, from the same edge.</param>
/// <param name="DefaultTab">
/// How far apart the stops a tab advances to are, in master units. PowerPoint's own default is
/// 0x240 — 576 master units, one inch — which is twice a word processor's half-inch stop, so a
/// paragraph positioned by tabs lands in a visibly different place under the wrong one.
/// </param>
public readonly record struct PptParagraphLevel(
    ushort BulletFlags,
    ushort BulletCharacter,
    ushort BulletFont = 0,
    ushort BulletHeight = 100,
    uint BulletColour = 0x08000000,
    ushort Alignment = 0,
    short LineFeed = 100,
    short SpaceBefore = 0,
    short SpaceAfter = 0,
    ushort TextOffset = 0,
    ushort BulletOffset = 0,
    ushort DefaultTab = PptParagraphLevel.PowerPointDefaultTab)
{
    /// <summary>PowerPoint's own default tab distance: 0x240 master units, which is one inch.</summary>
    public const ushort PowerPointDefaultTab = 0x240;

    /// <summary>Whether a paragraph at this level draws a bullet unless it says otherwise.</summary>
    public bool HasBullet => (BulletFlags & 0x0001) != 0;
}

/// <summary>
/// The per-outline-level text defaults a master states, one set per
/// <see cref="PptTextKind"/>.
/// </summary>
/// <remarks>
/// <para>
/// A slide states only what differs from its master. A title whose
/// <c>StyleTextPropAtom</c> names a mask of <c>0x040000</c> has stated its colour and nothing
/// else, so its boldness, size and typeface all come from here — which is why a reader without
/// this reports a bold PowerPoint title as unemphasised while the same deck saved as ODF reports
/// it bold.
/// </para>
/// <para>
/// The defaults live in <c>TxMasterStyleAtom</c> records, one per instance, hanging directly off
/// the <c>MainMaster</c> container, plus one for <see cref="PptTextKind.Other"/> in the
/// document's <c>Environment</c>. Ported from <c>PPTStyleSheet</c>'s constructor and
/// <c>PPTCharSheet::Read</c>/<c>PPTParaSheet::Read</c>,
/// <c>filter/source/msfilter/svdfppt.cxx:3793-4320</c>.
/// </para>
/// <para>
/// Three traps are worth naming, because each parses without error and yields plausible wrong
/// formatting rather than an exception:
/// </para>
/// <list type="bullet">
/// <item>
/// The <em>first</em> level of an atom uses a different field order from every later level —
/// alignment at mask bit <c>0x0800</c> becomes <c>0x0F00</c>, the left margin moves from
/// <c>0x0100</c> to <c>0x8000</c>, and the tab-stop array from <c>0x100000</c> to
/// <c>0x200000</c> (<c>svdfppt.cxx:3925-4010</c>, the <c>bFirst</c> branch). Reading every
/// level the same way misplaces the character properties that follow.
/// </item>
/// <item>
/// The four instances above <see cref="PptTextKind.Other"/> prefix <em>every</em> level with an
/// unexplained word and never use the first-level layout (<c>svdfppt.cxx:4253-4260</c>).
/// </item>
/// <item>
/// A level inherits from the level above it, but only for the five low instances
/// (<c>svdfppt.cxx:4247</c>); the four high ones start from a copy of the body or title sheet
/// instead, taken at the moment their atom is met, which is why the atoms have to be read in
/// the order the file writes them.
/// </item>
/// </list>
/// </remarks>
public sealed class PptStyleSheet
{
    /// <summary>
    /// How many outline levels a style may state.
    /// </summary>
    /// <remarks>
    /// <c>nMaxPPTLevels</c>, <c>include/filter/msfilter/svdfppt.hxx:171</c>. Every writer uses
    /// five; the extra five exist because the field counting them is a word and files claiming
    /// more have been seen.
    /// </remarks>
    public const int MaxLevels = 10;

    /// <summary>
    /// The deepest level a character run may resolve against.
    /// </summary>
    /// <remarks>
    /// LibreOffice clamps a portion's depth to four (<c>svdfppt.cxx:5423</c>) while clamping a
    /// paragraph's to nine, so the two lookups genuinely differ for a deck nested deeper than
    /// five.
    /// </remarks>
    public const int MaxCharacterLevel = 4;

    private const int Instances = 9;

    private readonly PptCharacterLevel[][] _characters;
    private readonly PptParagraphLevel[][] _paragraphs;

    private PptStyleSheet(PptCharacterLevel[][] characters, PptParagraphLevel[][] paragraphs)
    {
        _characters = characters;
        _paragraphs = paragraphs;
    }

    /// <summary>The character defaults for one text kind at one outline level.</summary>
    public PptCharacterLevel Character(PptTextKind kind, int level)
        => _characters[Index(kind)][Math.Clamp(level, 0, MaxCharacterLevel)];

    /// <summary>The paragraph defaults for one text kind at one outline level.</summary>
    public PptParagraphLevel Paragraph(PptTextKind kind, int level)
        => _paragraphs[Index(kind)][Math.Clamp(level, 0, MaxLevels - 1)];

    private static int Index(PptTextKind kind)
    {
        int index = (int)kind;
        return index >= 0 && index < Instances ? index : (int)PptTextKind.Other;
    }

    /// <summary>
    /// Builds a master's style sheet from its <c>TxMasterStyleAtom</c> records.
    /// </summary>
    /// <param name="stream">The document stream.</param>
    /// <param name="master">The <c>MainMaster</c> container.</param>
    /// <param name="environment">
    /// The document's <c>Environment</c> container, when it has one. Its own
    /// <c>TxMasterStyleAtom</c> outranks the master's for <see cref="PptTextKind.Other"/>, and
    /// suppresses it entirely — LibreOffice records the observation that the environment's copy
    /// wins at <c>svdfppt.cxx:4117</c>.
    /// </param>
    public static PptStyleSheet Read(
        DffRecordBuffer stream, DffRecordHeader master, DffRecordHeader? environment)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptCharacterLevel[][] characters = new PptCharacterLevel[Instances][];
        PptParagraphLevel[][] paragraphs = new PptParagraphLevel[Instances][];

        for (int instance = 0; instance < Instances; instance++)
        {
            characters[instance] = Defaults((PptTextKind)instance);
            paragraphs[instance] = BulletDefaults((PptTextKind)instance);
        }

        PptStyleSheet sheet = new(characters, paragraphs);

        bool environmentStatedTextInShape = false;
        if (environment is { } container)
        {
            foreach (DffRecordHeader record in stream.Children(container))
            {
                if (record.Type != PptRecordTypes.TxMasterStyleAtom) continue;
                sheet.ReadAtom(stream.Content(record), PptTextKind.Other);
                environmentStatedTextInShape = true;
                break;
            }
        }

        bool[] stated = new bool[Instances];

        foreach (DffRecordHeader record in stream.Children(master))
        {
            if (record.Type != PptRecordTypes.TxMasterStyleAtom) continue;
            if (record.Instance >= Instances) continue;

            PptTextKind kind = (PptTextKind)record.Instance;
            if (kind == PptTextKind.Other && environmentStatedTextInShape) continue;

            // The four high instances are a variation on body or title rather than a style of
            // their own, so they start from whichever of those has already been read.
            if (Variation(kind) is { } basis) sheet.CopyFrom(basis, kind);

            sheet.ReadAtom(stream.Content(record), kind);
            stated[(int)kind] = true;
        }

        // A master that states none of the four keeps them as copies rather than as the bare
        // defaults, so a deck with no subtitle style still puts a subtitle in the body's font.
        for (int instance = (int)PptTextKind.CentreBody; instance < Instances; instance++)
        {
            if (stated[instance]) continue;
            if (Variation((PptTextKind)instance) is { } basis)
            {
                sheet.CopyFrom(basis, (PptTextKind)instance);
            }
        }

        return sheet;
    }

    /// <summary>
    /// Which of the two real styles an instance is a variation on, or null when it is one.
    /// </summary>
    private static PptTextKind? Variation(PptTextKind kind) => kind switch
    {
        PptTextKind.CentreBody or PptTextKind.HalfBody or PptTextKind.QuarterBody => PptTextKind.Body,
        PptTextKind.CentreTitle => PptTextKind.Title,
        _ => null,
    };

    private void CopyFrom(PptTextKind source, PptTextKind target)
    {
        _characters[(int)target] = [.. _characters[(int)source]];
        _paragraphs[(int)target] = [.. _paragraphs[(int)source]];
    }

    /// <summary>Reads one <c>TxMasterStyleAtom</c> into the levels of one text kind.</summary>
    private void ReadAtom(ReadOnlySpan<byte> content, PptTextKind kind)
    {
        if (content.Length < 2) return;

        int stated = DffRecordBuffer.ReadUInt16(content);
        int levels = Math.Min(stated, MaxLevels);
        int position = 2;

        PptCharacterLevel[] characters = _characters[(int)kind];
        PptParagraphLevel[] paragraphs = _paragraphs[(int)kind];

        // Instances above "text in a shape" prefix every level with a word nothing documents,
        // and never take the first level's field order.
        bool prefixed = kind > PptTextKind.Other;
        bool first = !prefixed;

        for (int level = 0; level < levels && position < content.Length; level++)
        {
            if (level > 0 && !prefixed)
            {
                characters[level] = characters[level - 1];
                paragraphs[level] = paragraphs[level - 1];
            }

            if (prefixed) position += 2;

            paragraphs[level] = ReadParagraph(content, ref position, paragraphs[level], first);
            characters[level] = ReadCharacter(content, ref position, characters[level]);
            first = false;

            if (position > content.Length) break;
        }
    }

    /// <summary>Reads one level's paragraph properties.</summary>
    private static PptParagraphLevel ReadParagraph(
        ReadOnlySpan<byte> content, ref int position, PptParagraphLevel inherited, bool first)
    {
        uint mask = Take32(content, ref position);
        PptParagraphLevel level = inherited;

        if ((mask & 0x0000000F) != 0)
        {
            // Only the bits the mask names are replaced; the rest keep what the level above
            // gave them.
            ushort stated = Take16(content, ref position);
            ushort touched = (ushort)(mask & 0x000F);
            level = level with
            {
                BulletFlags = (ushort)((level.BulletFlags & ~touched) | (stated & touched)),
            };
        }

        if ((mask & 0x00000080) != 0)
            level = level with { BulletCharacter = Take16(content, ref position) };
        if ((mask & 0x00000010) != 0)
            level = level with { BulletFont = Take16(content, ref position) };
        if ((mask & 0x00000040) != 0)
            level = level with { BulletHeight = Take16(content, ref position) };
        if ((mask & 0x00000020) != 0)
            level = level with { BulletColour = Take32(content, ref position) };

        if (first)
        {
            if ((mask & 0x00000F00) != 0)
                level = level with { Alignment = (ushort)(Take16(content, ref position) & 3) };
            if ((mask & 0x00001000) != 0)
                level = level with { LineFeed = Signed(content, ref position) };
            if ((mask & 0x00002000) != 0)
                level = level with { SpaceBefore = Signed(content, ref position) };
            if ((mask & 0x00004000) != 0)
                level = level with { SpaceAfter = Signed(content, ref position) };
            if ((mask & 0x00008000) != 0)
                level = level with { TextOffset = Take16(content, ref position) };
            if ((mask & 0x00010000) != 0)
                level = level with { BulletOffset = Take16(content, ref position) };
            if ((mask & 0x00020000) != 0)
                level = level with { DefaultTab = Take16(content, ref position) };
            if ((mask & 0x00200000) != 0) SkipTabStops(content, ref position);
            if ((mask & 0x00040000) != 0) position += 2;   // baseline
            if ((mask & 0x00080000) != 0) position += 2;   // the Asian line-break flags
            if ((mask & 0x00100000) != 0) position += 2;   // text direction
        }
        else
        {
            if ((mask & 0x00000800) != 0)
                level = level with { Alignment = (ushort)(Take16(content, ref position) & 3) };
            if ((mask & 0x00001000) != 0)
                level = level with { LineFeed = Signed(content, ref position) };
            if ((mask & 0x00002000) != 0)
                level = level with { SpaceBefore = Signed(content, ref position) };
            if ((mask & 0x00004000) != 0)
                level = level with { SpaceAfter = Signed(content, ref position) };
            if ((mask & 0x00008000) != 0)
                level = level with { DefaultTab = Take16(content, ref position) };
            if ((mask & 0x00000100) != 0)
                level = level with { TextOffset = Take16(content, ref position) };
            if ((mask & 0x00000200) != 0) position += 2;
            if ((mask & 0x00000400) != 0)
                level = level with { BulletOffset = Take16(content, ref position) };
            if ((mask & 0x00010000) != 0) position += 2;   // baseline
            if ((mask & 0x000E0000) != 0) position += 2;   // the three wrap flags share one word
            if ((mask & 0x00100000) != 0) SkipTabStops(content, ref position);
            if ((mask & 0x00200000) != 0) position += 2;   // text direction
        }

        // Anything above the documented bits is still a word each, and skipping it is the only
        // way the character properties that follow land in the right place.
        for (uint rest = mask >> 22; rest != 0; rest >>= 1)
        {
            if ((rest & 1) != 0) position += 2;
        }

        return level;
    }

    /// <summary>A word read as signed, which the line feed and the two distances are.</summary>
    private static short Signed(ReadOnlySpan<byte> content, ref int position)
        => unchecked((short)Take16(content, ref position));

    /// <summary>Reads one level's character properties.</summary>
    private static PptCharacterLevel ReadCharacter(
        ReadOnlySpan<byte> content, ref int position, PptCharacterLevel inherited)
    {
        uint mask = Take32(content, ref position);
        PptCharacterLevel level = inherited;

        if ((mask & 0x0000FFFF) != 0)
        {
            // The named bits are cleared and then re-set from the flags word, so a mask that
            // names bold with the bit clear turns an inherited bold off.
            ushort stated = Take16(content, ref position);
            ushort flags = (ushort)((level.Flags & ~(ushort)mask) | stated);
            level = level with { Flags = flags };
        }

        if ((mask & 0x00010000) != 0) level = level with { FontIndex = Take16(content, ref position) };
        if ((mask & 0x00200000) != 0) level = level with { AsianFontIndex = Take16(content, ref position) };
        if ((mask & 0x00400000) != 0) position += 2;   // ANSI typeface
        if ((mask & 0x00800000) != 0) position += 2;   // symbol typeface
        if ((mask & 0x00020000) != 0) level = level with { FontHeight = Take16(content, ref position) };

        if ((mask & 0x00040000) != 0)
        {
            uint colour = Take32(content, ref position);

            // A colour whose top byte is clear names no scheme slot and no literal either;
            // LibreOffice substitutes the background (svdfppt.cxx:3861).
            level = level with { Colour = (colour & 0xFF000000) != 0 ? colour : SchemeBackground };
        }

        if ((mask & 0x00080000) != 0)
        {
            level = level with { Escapement = unchecked((short)Take16(content, ref position)) };
        }

        if ((mask & 0x00100000) != 0) position += 2;

        for (uint rest = mask >> 24; rest != 0; rest >>= 1)
        {
            if ((rest & 1) != 0) position += 2;
        }

        return level;
    }

    private static void SkipTabStops(ReadOnlySpan<byte> content, ref int position)
    {
        int stops = Take16(content, ref position);
        position += stops * 4;
    }

    private static ushort Take16(ReadOnlySpan<byte> content, ref int position)
    {
        ushort value = position >= 0 && position + 2 <= content.Length
            ? DffRecordBuffer.ReadUInt16(content[position..])
            : (ushort)0;
        position += 2;
        return value;
    }

    private static uint Take32(ReadOnlySpan<byte> content, ref int position)
    {
        uint value = position >= 0 && position + 4 <= content.Length
            ? DffRecordBuffer.ReadUInt32(content[position..])
            : 0;
        position += 4;
        return value;
    }

    /// <summary>The colour-scheme slot a run with no colour of its own falls back to.</summary>
    private const uint SchemeBackground = 0x08000000;

    /// <summary>The scheme slot ordinary text uses.</summary>
    private const uint SchemeText = 0x08000001;

    /// <summary>The scheme slot title text uses.</summary>
    private const uint SchemeTitleText = 0x08000003;

    /// <summary>
    /// The character defaults an instance starts from before any atom is read.
    /// </summary>
    /// <remarks>
    /// From <c>PPTCharSheet</c>'s constructor, <c>svdfppt.cxx:3793</c>. They matter: a deck
    /// whose master states no atom for an instance still has to report a title at 44 points and
    /// a note at 12.
    /// </remarks>
    private static PptCharacterLevel[] Defaults(PptTextKind kind)
    {
        (ushort height, uint colour) = kind switch
        {
            PptTextKind.Title or PptTextKind.CentreTitle => ((ushort)44, SchemeTitleText),
            PptTextKind.Body or PptTextKind.CentreBody
                or PptTextKind.HalfBody or PptTextKind.QuarterBody => ((ushort)32, SchemeText),
            PptTextKind.Notes => ((ushort)12, SchemeText),
            _ => ((ushort)24, SchemeText),
        };

        PptCharacterLevel level = new(0, 0, 0xFFFF, height, colour, 0);
        PptCharacterLevel[] levels = new PptCharacterLevel[MaxLevels];
        Array.Fill(levels, level);
        return levels;
    }

    /// <summary>
    /// The paragraph defaults an instance starts from, from <c>PPTParaSheet</c>'s constructor
    /// (<c>svdfppt.cxx:3880-3922</c>).
    /// </summary>
    /// <remarks>
    /// Body text starts bulleted and with a fifth of a line above it; a note starts with a
    /// thirtieth of an inch. Neither is stated in a file that leaves the level alone, so a reader
    /// that started every level at zero would set an outline's paragraphs solid.
    /// </remarks>
    private static PptParagraphLevel[] BulletDefaults(PptTextKind kind)
    {
        (ushort flags, short spaceBefore, uint bulletColour) = kind switch
        {
            PptTextKind.Title or PptTextKind.CentreTitle
                => ((ushort)0, (short)0, SchemeTitleText),
            PptTextKind.Body or PptTextKind.CentreBody
                or PptTextKind.HalfBody or PptTextKind.QuarterBody
                => ((ushort)1, (short)0x14, SchemeBackground),
            PptTextKind.Notes => ((ushort)0, (short)0x1E, SchemeBackground),
            _ => ((ushort)0, (short)0, SchemeBackground),
        };

        PptParagraphLevel[] levels = new PptParagraphLevel[MaxLevels];
        Array.Fill(
            levels,
            new PptParagraphLevel(flags, 0x2022, BulletColour: bulletColour, SpaceBefore: spaceBefore));
        return levels;
    }
}

/// <summary>
/// Turns PowerPoint's character bit attributes into the content tree's emphasis.
/// </summary>
public static class PptCharacterStyle
{
    /// <summary>The emphasis a flags word and an escapement amount to.</summary>
    /// <param name="flags">The bit attributes, as the format numbers them.</param>
    /// <param name="escapement">A raised or lowered baseline, as a percentage.</param>
    public static RunEmphasis ToEmphasis(ushort flags, short escapement)
    {
        RunEmphasis emphasis = RunEmphasis.None;

        if ((flags & 0x0001) != 0) emphasis |= RunEmphasis.Bold;
        if ((flags & 0x0002) != 0) emphasis |= RunEmphasis.Italic;
        if ((flags & 0x0004) != 0) emphasis |= RunEmphasis.Underline;
        if ((flags & 0x0100) != 0) emphasis |= RunEmphasis.Strikethrough;

        if (escapement > 0) emphasis |= RunEmphasis.Superscript;
        else if (escapement < 0) emphasis |= RunEmphasis.Subscript;

        return emphasis;
    }

    /// <summary>
    /// Which kinds of emphasis a mask claims, so the rest can be inherited.
    /// </summary>
    /// <remarks>
    /// The low half of a character mask uses the same bit numbering as the flags word it
    /// introduces, so a mask naming bold with the flag clear means bold is explicitly
    /// <em>off</em> rather than unstated — the distinction that decides whether the master's
    /// default applies (<c>svdfppt.cxx:5455</c>).
    /// </remarks>
    /// <param name="mask">The character run's property mask.</param>
    public static RunEmphasis Stated(uint mask)
    {
        RunEmphasis stated = RunEmphasis.None;

        if ((mask & 0x0001) != 0) stated |= RunEmphasis.Bold;
        if ((mask & 0x0002) != 0) stated |= RunEmphasis.Italic;
        if ((mask & 0x0004) != 0) stated |= RunEmphasis.Underline;
        if ((mask & 0x0100) != 0) stated |= RunEmphasis.Strikethrough;

        // Escapement is one field carrying both directions, so stating it states both.
        if ((mask & 0x00080000) != 0) stated |= RunEmphasis.Superscript | RunEmphasis.Subscript;

        return stated;
    }
}
