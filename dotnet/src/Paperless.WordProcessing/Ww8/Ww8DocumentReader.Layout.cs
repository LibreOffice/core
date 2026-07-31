using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Globalization;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// The layout pass: the body's paragraphs with the formatting that decides where their text goes.
/// </content>
/// <remarks>
/// A second walk over the same character range the content pass reads, for the same reason the XML
/// formats have one — the content tree discards font sizes, indents and spacing because extraction must
/// not pay for them. Here it is a second walk rather than a second parse: the piece table, the
/// formatting tables and the stylesheet are all already built, so this costs a scan and a sprm decode
/// per paragraph.
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>How many paragraphs are read before the rest are ignored.</summary>
    public const int MaxLayoutParagraphs = 200000;

    /// <summary>
    /// One paragraph, with its text and the formatting layout needs.
    /// </summary>
    /// <param name="Text">The paragraph's text, without its terminating mark.</param>
    /// <param name="Format">Its resolved layout properties.</param>
    /// <param name="FamilyName">The family the document names, or null when it names none.</param>
    /// <param name="Size">The em size.</param>
    /// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
    /// <param name="IsItalic">True when the text is italic.</param>
    /// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
    /// <param name="IsInTable">
    /// True when the paragraph is inside a table. Tables are laid out as grids rather than as a run of
    /// paragraphs, so a caller filling pages skips these rather than stacking them.
    /// </param>
    /// <param name="Colour">The colour the paragraph's mark carries, or null for the automatic colour.</param>
    /// <param name="Runs">
    /// The stretches its character formatting divides it into, in order. Always populated, even where the
    /// whole paragraph is uniform — the layout source decides whether they are worth carrying, since it is
    /// the only party that can compare two <em>resolved</em> faces rather than two requested families.
    /// </param>
    public readonly record struct Ww8LayoutParagraph(
        string Text,
        Text.Layout.ParagraphFormat Format,
        string? FamilyName,
        Length Size,
        int Weight,
        bool IsItalic,
        string? Language,
        bool IsInTable,
        Colour? Colour = null,
        IReadOnlyList<Ww8LayoutRun>? Runs = null);

    /// <summary>
    /// One stretch of a paragraph's text and the character formatting in force over it.
    /// </summary>
    /// <remarks>
    /// A CHPX covers a range of the file rather than of the paragraph, and one paragraph can span several
    /// — that is how WW8 stores a bold word. The ranges here are into the paragraph's <em>text</em>, which
    /// is not the same thing: an optional hyphen and a field marker occupy a character position in the file
    /// and none on the page, so each shifts everything after it.
    /// </remarks>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="FamilyName">The family the document names, or null when it names none.</param>
    /// <param name="Size">The em size.</param>
    /// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
    /// <param name="IsItalic">True when the text is italic.</param>
    /// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
    /// <param name="Colour">The colour the text is drawn in, or null for the automatic colour.</param>
    public readonly record struct Ww8LayoutRun(
        int Start,
        int Length,
        string? FamilyName,
        Length Size,
        int Weight,
        bool IsItalic,
        string? Language,
        Colour? Colour)
    {
        /// <summary>One past the run's last character.</summary>
        public int End => Start + Length;
    }

    /// <summary>The document's font table, read on demand.</summary>
    private Ww8FontTable? _fonts;

    /// <summary>The document's <c>Dop</c>, read on demand.</summary>
    private Ww8DocumentProperties? _properties;

    /// <summary>
    /// The document-wide layout decisions: the default tab interval and how spacings combine.
    /// </summary>
    /// <remarks>
    /// Read on demand and cached, because extraction never asks — and a document whose <c>Dop</c> is
    /// missing gets the defaults rather than an exception, which is the same leniency every other table
    /// gets here.
    /// </remarks>
    public Ww8DocumentProperties DocumentProperties =>
        _properties ??= _fib.Has(Ww8FibTable.DocumentProperties)
            ? Ww8DocumentProperties.Parse(Slice(Ww8FibTable.DocumentProperties))
            : Ww8DocumentProperties.Default;

    /// <summary>The families the document's <c>sprmCRgFtc0</c> indexes name.</summary>
    public Ww8FontTable Fonts =>
        _fonts ??= _fib.Has(Ww8FibTable.FontNames)
            ? Ww8FontTable.Parse(Slice(Ww8FibTable.FontNames))
            : Ww8FontTable.Empty;

    /// <summary>
    /// Reads the body's paragraphs with the formatting layout needs.
    /// </summary>
    /// <remarks>
    /// The body range only. The other subdocuments are furniture and notes, which a page assembles
    /// separately once it knows which section it belongs to.
    /// </remarks>
    public List<Ww8LayoutParagraph> ReadLayoutParagraphs()
    {
        Ww8Range body = Ranges.Body;
        List<Ww8LayoutParagraph> paragraphs = [];
        if (body.Length <= 0) return paragraphs;

        string text = _pieces.ReadText(body.Start, body.End, _diagnostics);
        if (text.Length == 0) return paragraphs;

        StringBuilder current = new();

        // The source position of each character in `current`. A paragraph's text is not a slice of the
        // file's — optional hyphens and field markers are dropped — so a run's range cannot be recovered
        // from the offsets afterwards and has to be carried alongside.
        List<int> positions = [];
        int start = 0;

        for (int index = 0; index < text.Length && paragraphs.Count < MaxLayoutParagraphs; index++)
        {
            char character = text[index];
            int position = body.Start + index;

            switch (character)
            {
                case ParagraphMark:
                    paragraphs.Add(
                        Describe(current.ToString(), positions, body.Start + start, position));
                    current.Clear();
                    positions.Clear();
                    start = index + 1;
                    continue;

                case CellMark:
                    // A cell or row boundary. The paragraph before it belongs to a table, which this
                    // pass does not lay out, so it is closed and marked rather than dropped — a caller
                    // that skips it still counts the paragraphs the same way.
                    paragraphs.Add(
                        Describe(current.ToString(), positions, body.Start + start, position));
                    current.Clear();
                    positions.Clear();
                    start = index + 1;
                    continue;

                case Special.SectionMark:
                    paragraphs.Add(
                        Describe(current.ToString(), positions, body.Start + start, position));
                    current.Clear();
                    positions.Clear();
                    start = index + 1;
                    continue;

                case Special.LineBreak:
                    Emit(current, positions, LineSeparator, position);
                    continue;

                case Special.Tab:
                    Emit(current, positions, '\t', position);
                    continue;

                case Special.NonBreakingHyphen:
                    Emit(current, positions, NonBreakingHyphen, position);
                    continue;

                case Special.OptionalHyphen:
                    continue;

                case Special.FieldBegin or Special.FieldSeparator or Special.FieldEnd:
                    // A field's instruction and its result are both in the text; the content pass
                    // distinguishes them. For measurement the markers themselves have no width and the
                    // instruction is not shown, but skipping only the markers is close enough here and
                    // wrong only for a document whose fields are unusually long — recorded in the TODO.
                    continue;

                case Special.Picture or Special.DrawnObject or Special.AnnotationReference
                    or Special.AutoNumberedReference:
                    Emit(current, positions, AnchorCharacter, position);
                    continue;

                default:
                    if (character >= ' ' || character == '')
                    {
                        Emit(current, positions, character, position);
                    }
                    continue;
            }
        }

        if (current.Length > 0 || paragraphs.Count == 0)
        {
            paragraphs.Add(
                Describe(current.ToString(), positions, body.Start + start, body.End - 1));
        }

        return paragraphs;
    }

    /// <summary>
    /// Appends one character, recording where in the file it came from.
    /// </summary>
    /// <remarks>
    /// The pairing is what lets a CHPX's file range become a range of the paragraph's text. Appending to
    /// the builder directly anywhere would silently desynchronise the two and misattribute a run's
    /// formatting to the text beside it.
    /// </remarks>
    private static void Emit(
        StringBuilder text, List<int> positions, char character, int position)
    {
        text.Append(character);
        positions.Add(position);
    }

    /// <summary>
    /// The character a line break becomes: U+2028, whose UAX #14 class is a mandatory break.
    /// </summary>
    private const char LineSeparator = '\u2028';

    /// <summary>The non-breaking hyphen a WW8 U+001E becomes.</summary>
    private const char NonBreakingHyphen = '\u2011';

    /// <summary>The character an anchor occupies, matching the other formats' readers.</summary>
    private const char AnchorCharacter = '\u0001';

    /// <summary>
    /// Resolves one paragraph's formatting from its mark's position.
    /// </summary>
    /// <remarks>
    /// The mark's position, not the paragraph's start, because that is where WW8 stores a paragraph's
    /// properties — the PAPX describing a paragraph is found by looking up the character position of the
    /// mark that ends it. Looking up the first character instead finds the <em>previous</em> paragraph's
    /// properties, which is a mistake that produces a document formatted one paragraph out of step.
    /// </remarks>
    private Ww8LayoutParagraph Describe(
        string text, List<int> positions, int start, int markPosition)
    {
        Ww8LayoutFormat layout = ResolveLayoutFormat(markPosition);
        Ww8ParagraphFormat paragraph = ResolveParagraphFormat(markPosition);

        // The run properties at the paragraph's mark, which is what its mark carries and what an empty
        // paragraph is as tall as. The text's own formatting comes from the runs below.
        Ww8LayoutFormat character = ResolveCharacterLayout(
            Math.Min(Math.Max(start, 0), Math.Max(markPosition, 0)));

        Length size = SizeOf(character);

        return new Ww8LayoutParagraph(
            text,
            layout.ToParagraphFormat(size) with
            {
                DefaultTabInterval = DocumentProperties.DefaultTabInterval,
            },
            character.FontIndex is { } index ? Fonts.Name(index) : null,
            size,
            character.IsBold == true ? 700 : 400,
            character.IsItalic == true,
            LanguageOf(character),
            paragraph.IsInTable,
            character.Colour,
            ReadRuns(text, positions, markPosition));
    }

    /// <summary>
    /// The stretches a paragraph's character formatting divides its text into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Walked by position rather than by reading the CHPX table's own boundaries, because those are file
    /// offsets and a paragraph's text is not a slice of the file: the piece table can move between
    /// eight-bit and sixteen-bit pieces mid-paragraph, and this pass drops characters. So the cost is one
    /// piece lookup per character — but only one CHPX decode per actual run, because the table reports the
    /// byte range each entry covers and a position still inside it reuses the last answer.
    /// </para>
    /// <para>
    /// The paragraph style's character half is resolved once, outside the walk: it is constant within a
    /// paragraph, and it is the half that makes a heading's runs large and bold.
    /// </para>
    /// </remarks>
    private List<Ww8LayoutRun> ReadRuns(string text, List<int> positions, int markPosition)
    {
        List<Ww8LayoutRun> runs = [];
        if (text.Length == 0 || positions.Count == 0) return runs;

        Ww8LayoutFormat inherited = CharacterStyleFormat(markPosition);
        int count = Math.Min(text.Length, positions.Count);

        ReadOnlyMemory<byte> properties = default;
        int cachedFrom = 0;
        int cachedTo = 0;
        bool cached = false;

        for (int index = 0; index < count; index++)
        {
            int byteOffset = _pieces.FileOffsetOf(positions[index]);

            if (!cached || byteOffset < cachedFrom || byteOffset >= cachedTo)
            {
                (properties, cachedFrom, cachedTo) =
                    _characterProperties.FindWithRange(byteOffset);
                cached = true;

                // A table with no entry for this offset reports an empty range, which would make every
                // character a fresh lookup. Treating the one character as the range stops that.
                if (cachedTo <= cachedFrom) cachedTo = cachedFrom + 1;
            }

            Ww8LayoutFormat format = ApplyCharacterException(inherited, properties);
            Ww8LayoutRun run = new(
                index,
                1,
                format.FontIndex is { } font ? Fonts.Name(font) : null,
                SizeOf(format),
                format.IsBold == true ? 700 : 400,
                format.IsItalic == true,
                LanguageOf(format),
                format.Colour);

            if (runs.Count > 0 && MatchesFormatting(runs[^1], run))
            {
                runs[^1] = runs[^1] with { Length = runs[^1].Length + 1 };
                continue;
            }

            runs.Add(run);
        }

        return runs;
    }

    /// <summary>True when two runs' formatting is identical, whatever their ranges.</summary>
    private static bool MatchesFormatting(Ww8LayoutRun a, Ww8LayoutRun b)
        => string.Equals(a.FamilyName, b.FamilyName, StringComparison.Ordinal)
           && a.Size == b.Size
           && a.Weight == b.Weight
           && a.IsItalic == b.IsItalic
           && string.Equals(a.Language, b.Language, StringComparison.Ordinal)
           && a.Colour == b.Colour;

    /// <summary>
    /// The em size a character format states, defaulting to ten points.
    /// </summary>
    /// <remarks>
    /// Ten rather than twelve, because that is what Word's own default is for a document whose stylesheet
    /// states none. The bound rejects the absurd rather than the merely large: 4000 half-points is 2000 pt.
    /// </remarks>
    private static Length SizeOf(Ww8LayoutFormat format)
        => format.FontSizeHalfPoints is { } halves and > 0 and <= 4000
            ? Length.FromPoints(halves / 2.0)
            : Length.FromPoints(10);

    private static string? LanguageOf(Ww8LayoutFormat format)
        => format.LanguageId is { } id and > 0 and <= ushort.MaxValue
            ? WindowsLanguages.TagOf((ushort)id)
            : null;

    /// <summary>
    /// The layout sprms in force on a paragraph, style chain first and its own last.
    /// </summary>
    private Ww8LayoutFormat ResolveLayoutFormat(int position)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        (ushort styleIndex, ReadOnlyMemory<byte> direct) =
            Ww8FormattingTable.SplitParagraphProperties(_paragraphProperties.Find(byteOffset));

        Ww8LayoutFormat format = default;
        foreach (ReadOnlyMemory<byte> inherited in _styles.ResolveChain(styleIndex))
        {
            format = ApplyLayoutSprms(format, inherited);
        }

        return ApplyLayoutSprms(format, direct);
    }

    /// <summary>
    /// The character sprms in force at a position, resolved through the paragraph style's own half.
    /// </summary>
    /// <remarks>
    /// A paragraph style carries a CHPX as well as a PAPX, and that half is how a heading style makes its
    /// runs large and bold — so it has to be applied before the direct formatting, exactly as the content
    /// pass does for emphasis.
    /// </remarks>
    private Ww8LayoutFormat ResolveCharacterLayout(int position)
        => ApplyCharacterException(
            CharacterStyleFormat(position),
            _characterProperties.Find(_pieces.FileOffsetOf(position)));

    /// <summary>
    /// Applies one CHPX over an inherited format: its character style first, then its own sprms.
    /// </summary>
    /// <remarks>
    /// The two halves cannot be applied in one pass, because the sprm naming the character style sits
    /// inside the same grpprl as the direct formatting — so a single pass would lay the style's properties
    /// over the direct ones that were meant to override them.
    /// <para>
    /// Skipping the style half entirely is worse than a subtle ordering bug, and it is the mistake that is
    /// easy to make: LibreOffice's own DOC export writes emphasis as a character style rather than as
    /// direct sprms, so a reader that only decodes the exception finds a document with no bold in it at
    /// all.
    /// </para>
    /// </remarks>
    private Ww8LayoutFormat ApplyCharacterException(
        Ww8LayoutFormat inherited, ReadOnlyMemory<byte> exception)
    {
        Ww8LayoutFormat format = inherited;

        // Index zero is not "no character style" — in WW8 the stylesheet is one table and istd 0 is
        // *Normal*, a paragraph style. Resolving its chain here would lay the document's default font size
        // over the paragraph style's own, so every run of an 11 pt paragraph would come out at 12.
        if (CharacterStyleIndexIn(exception) is var styleIndex and not 0)
        {
            foreach (ReadOnlyMemory<byte> fromStyle in _styles.ResolveCharacterChain(styleIndex))
            {
                format = ApplyLayoutSprms(format, fromStyle);
            }
        }

        return ApplyLayoutSprms(format, exception);
    }

    /// <summary>
    /// The character formatting a position's paragraph style contributes, without its direct formatting.
    /// </summary>
    /// <remarks>
    /// Separated out because it is constant within a paragraph while the direct formatting is not, so a
    /// run walk resolves this once and layers each CHPX over it — rather than re-walking the style chain
    /// for every run.
    /// </remarks>
    private Ww8LayoutFormat CharacterStyleFormat(int position)
    {
        Ww8LayoutFormat format = default;
        foreach (ReadOnlyMemory<byte> inherited in
                 _styles.ResolveCharacterChain(ParagraphStyleIndexAt(position)))
        {
            format = ApplyLayoutSprms(format, inherited);
        }

        return format;
    }

    /// <summary>
    /// Applies one grpprl's layout sprms.
    /// </summary>
    /// <remarks>
    /// The ids come from LibreOffice's <c>sprmids.hxx</c>, which states each operand's size on the same
    /// line — the half that decides whether the <em>following</em> sprm decodes at all. Both the modern
    /// and the Word 97 forms of the indents and the alignment are handled, because a document saved by
    /// any version of Word may carry either and they are different numbers.
    /// </remarks>
    private static Ww8LayoutFormat ApplyLayoutSprms(
        Ww8LayoutFormat format, ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                case LayoutSprms.Justification or LayoutSprms.Justification80:
                    format = format with { Justification = sprm.Byte };
                    break;

                case LayoutSprms.LeftIndent or LayoutSprms.LeftIndent80:
                    format = format with { LeftIndent = sprm.SignedWord };
                    break;
                case LayoutSprms.RightIndent or LayoutSprms.RightIndent80:
                    format = format with { RightIndent = sprm.SignedWord };
                    break;
                case LayoutSprms.FirstLineIndent or LayoutSprms.FirstLineIndent80:
                    format = format with { FirstLineIndent = sprm.SignedWord };
                    break;

                case LayoutSprms.SpaceBefore:
                    format = format with { SpaceBefore = sprm.Word };
                    break;
                case LayoutSprms.SpaceAfter:
                    format = format with { SpaceAfter = sprm.Word };
                    break;

                case LayoutSprms.LineSpacing:
                {
                    // An LSPD: a signed spacing then a flag, and the flag changes the first field's
                    // unit as well as its meaning.
                    ReadOnlySpan<byte> operand = sprm.Operand.Span;
                    if (operand.Length >= 4)
                    {
                        format = format with
                        {
                            LineSpacing = System.Buffers.Binary.BinaryPrimitives
                                .ReadInt16LittleEndian(operand),
                            IsMultipleLineSpacing = System.Buffers.Binary.BinaryPrimitives
                                .ReadUInt16LittleEndian(operand[2..]) != 0,
                        };
                    }
                    break;
                }

                case LayoutSprms.ColourIndex:
                    format = format with
                    {
                        Colour = sprm.Byte < IcoPalette.Length ? IcoPalette[sprm.Byte] : null,
                    };
                    break;

                case LayoutSprms.Colour:
                {
                    // A COLORREF, which is 0x00bbggrr as a little-endian DWORD — so the bytes arrive as
                    // red, green, blue, flag, and taking them in that order is the same swap
                    // LibreOffice's own BGRToRGB performs. All four set is COL_AUTO, the automatic
                    // colour, which is not the same thing as opaque white.
                    ReadOnlySpan<byte> operand = sprm.Operand.Span;
                    bool automatic = operand.Length >= 4 && operand[..4] is [0xFF, 0xFF, 0xFF, 0xFF];

                    format = format with
                    {
                        Colour = operand.Length >= 3 && !automatic
                            ? Colour.FromRgb((uint)((operand[0] << 16) | (operand[1] << 8) | operand[2]))
                            : null,
                    };
                    break;
                }

                case LayoutSprms.TabStops:
                    format = format with
                    {
                        TabStops = ApplyTabChange(format.TabStops, sprm.Operand.Span),
                    };
                    break;

                case LayoutSprms.KeepTogether:
                    format = format with { KeepTogether = sprm.Byte != 0 };
                    break;
                case LayoutSprms.KeepWithNext:
                    format = format with { KeepWithNext = sprm.Byte != 0 };
                    break;
                case LayoutSprms.PageBreakBefore:
                    format = format with { StartsNewPage = sprm.Byte != 0 };
                    break;
                case LayoutSprms.WidowControl:
                    format = format with { HasWidowControl = sprm.Byte != 0 };
                    break;
                case LayoutSprms.ContextualSpacing:
                    format = format with { HasContextualSpacing = sprm.Byte != 0 };
                    break;

                case LayoutSprms.FontSize:
                    format = format with { FontSizeHalfPoints = sprm.Word };
                    break;
                case LayoutSprms.FontIndex:
                    format = format with { FontIndex = sprm.Word };
                    break;
                case LayoutSprms.Bold:
                    format = format with
                    {
                        IsBold = sprm.ResolveToggle(format.IsBold ?? false),
                    };
                    break;
                case LayoutSprms.Italic:
                    format = format with
                    {
                        IsItalic = sprm.ResolveToggle(format.IsItalic ?? false),
                    };
                    break;
                case LayoutSprms.Language or LayoutSprms.Language80:
                    format = format with { LanguageId = sprm.Word };
                    break;

                default:
                    break;
            }
        }

        return format;
    }

    /// <summary>The layout sprms, from LibreOffice's <c>sprmids.hxx</c>.</summary>
    private static class LayoutSprms
    {
        internal const ushort Justification80 = 0x2403;
        internal const ushort KeepTogether = 0x2405;
        internal const ushort KeepWithNext = 0x2406;
        internal const ushort PageBreakBefore = 0x2407;
        internal const ushort RightIndent80 = 0x840E;
        internal const ushort LeftIndent80 = 0x840F;
        internal const ushort FirstLineIndent80 = 0x8411;
        internal const ushort LineSpacing = 0x6412;
        internal const ushort SpaceBefore = 0xA413;
        internal const ushort SpaceAfter = 0xA414;
        internal const ushort WidowControl = 0x2431;
        internal const ushort RightIndent = 0x845D;
        internal const ushort LeftIndent = 0x845E;
        internal const ushort FirstLineIndent = 0x8460;
        internal const ushort Justification = 0x2461;
        internal const ushort ContextualSpacing = 0x246D;

        internal const ushort Bold = 0x0835;
        internal const ushort Italic = 0x0836;
        internal const ushort FontSize = 0x4A43;
        internal const ushort FontIndex = 0x4A4F;
        internal const ushort Language80 = 0x486D;
        internal const ushort Language = 0x4873;
        internal const ushort ColourIndex = 0x2A42;
        internal const ushort Colour = 0x6870;
        internal const ushort TabStops = 0xC60D;
    }

    /// <summary>
    /// Applies one <c>sprmPChgTabsPapx</c> to the stops accumulated so far.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The sprm is a <em>change</em>, not a list, and its operand packs three arrays whose lengths depend on
    /// each other: a count of deletions, that many two-byte positions, a count of insertions, that many
    /// two-byte positions, and finally that many one-byte descriptors. Reading the descriptors from the
    /// wrong offset gives every stop a plausible-looking wrong alignment, so the bounds are checked the way
    /// <c>SwWW8ImplReader::Read_Tab</c> checks them — a record claiming more than it carries is discarded
    /// whole rather than half-read.
    /// </para>
    /// <para>
    /// A descriptor's low three bits are the alignment and the next three the leader. Alignment 4 is a bar
    /// tab — a vertical rule rather than an advance — which LibreOffice ignores here, and so does this: a
    /// bar recorded as a left stop would put a column boundary where the document asked for a line.
    /// </para>
    /// </remarks>
    private static List<TabStop> ApplyTabChange(
        IReadOnlyList<TabStop>? inherited, ReadOnlySpan<byte> operand)
    {
        List<TabStop> stops = inherited is null ? [] : [.. inherited];
        if (operand.Length < 1) return stops;

        int deletions = operand[0];
        if (operand.Length < (2 * deletions) + 2) return stops;

        int insertions = operand[(2 * deletions) + 1];

        // 2 + 2*del + 2*ins + 1*ins, which is the length the record needs to describe what it claims.
        if (2 + (2 * deletions) + (3 * insertions) > operand.Length) return stops;

        for (int i = 0; i < deletions; i++)
        {
            long position = Length
                .FromTwips(BinaryPrimitives.ReadUInt16LittleEndian(operand[(1 + (2 * i))..])).Emu;

            stops.RemoveAll(stop => stop.Position.Emu == position);
        }

        int positions = (2 * deletions) + 2;
        int descriptors = positions + (2 * insertions);

        for (int i = 0; i < insertions && stops.Count < MaxTabStops; i++)
        {
            byte descriptor = operand[descriptors + i];
            int alignment = descriptor & 0x7;
            if (alignment == 4) continue;

            stops.Add(new TabStop(
                Length.FromTwips(
                    BinaryPrimitives.ReadUInt16LittleEndian(operand[(positions + (2 * i))..])),
                alignment switch
                {
                    1 => TabAlignment.Centre,
                    2 => TabAlignment.Right,
                    3 => TabAlignment.DecimalSeparator,
                    _ => TabAlignment.Left,
                },
                ((descriptor >> 3) & 0x7) switch
                {
                    1 => '.',
                    2 => '-',
                    3 or 4 => '_',
                    _ => '\0',
                }));
        }

        stops.Sort((left, right) => left.Position.Emu.CompareTo(right.Position.Emu));
        return stops;
    }

    /// <summary>How many tab stops a paragraph may declare before the rest are ignored.</summary>
    /// <remarks>
    /// A guard on untrusted input. Word's own limit is 64 and a real paragraph uses a handful; each stop
    /// costs a lookup on every tab in the paragraph.
    /// </remarks>
    public const int MaxTabStops = 256;

    /// <summary>
    /// The seventeen colours a <c>sprmCIco</c> index names.
    /// </summary>
    /// <remarks>
    /// Copied from <c>SwWW8ImplReader::GetCol</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>), whose order is
    /// not the obvious one: index 2 is <em>light</em> blue and index 9 is blue, so the palette runs bright
    /// colours first and dark ones second. Index 0 is the automatic colour and is null rather than black,
    /// so the document's own default applies.
    /// </remarks>
    private static readonly Colour?[] IcoPalette =
    [
        null,
        Colour.FromRgb(0x000000),
        Colour.FromRgb(0x0000FF),
        Colour.FromRgb(0x00FFFF),
        Colour.FromRgb(0x00FF00),
        Colour.FromRgb(0xFF00FF),
        Colour.FromRgb(0xFF0000),
        Colour.FromRgb(0xFFFF00),
        Colour.FromRgb(0xFFFFFF),
        Colour.FromRgb(0x000080),
        Colour.FromRgb(0x008080),
        Colour.FromRgb(0x008000),
        Colour.FromRgb(0x800080),
        Colour.FromRgb(0x800000),
        Colour.FromRgb(0x808000),
        Colour.FromRgb(0x808080),
        Colour.FromRgb(0xC0C0C0),
    ];
}
