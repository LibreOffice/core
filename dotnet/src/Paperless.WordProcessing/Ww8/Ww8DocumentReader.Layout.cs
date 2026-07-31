using System.Text;
using Paperless.Core.Globalization;
using Paperless.Core.Units;

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
    public readonly record struct Ww8LayoutParagraph(
        string Text,
        Text.Layout.ParagraphFormat Format,
        string? FamilyName,
        Length Size,
        int Weight,
        bool IsItalic,
        string? Language,
        bool IsInTable);

    /// <summary>The document's font table, read on demand.</summary>
    private Ww8FontTable? _fonts;

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
        int start = 0;

        for (int index = 0; index < text.Length && paragraphs.Count < MaxLayoutParagraphs; index++)
        {
            char character = text[index];
            int position = body.Start + index;

            switch (character)
            {
                case ParagraphMark:
                    paragraphs.Add(Describe(current.ToString(), body.Start + start, position));
                    current.Clear();
                    start = index + 1;
                    continue;

                case CellMark:
                    // A cell or row boundary. The paragraph before it belongs to a table, which this
                    // pass does not lay out, so it is closed and marked rather than dropped — a caller
                    // that skips it still counts the paragraphs the same way.
                    paragraphs.Add(Describe(current.ToString(), body.Start + start, position));
                    current.Clear();
                    start = index + 1;
                    continue;

                case Special.SectionMark:
                    paragraphs.Add(Describe(current.ToString(), body.Start + start, position));
                    current.Clear();
                    start = index + 1;
                    continue;

                case Special.LineBreak:
                    current.Append(LineSeparator);
                    continue;

                case Special.Tab:
                    current.Append('\t');
                    continue;

                case Special.NonBreakingHyphen:
                    current.Append(NonBreakingHyphen);
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
                    current.Append(AnchorCharacter);
                    continue;

                default:
                    if (character >= ' ' || character == '') current.Append(character);
                    continue;
            }
        }

        if (current.Length > 0 || paragraphs.Count == 0)
        {
            paragraphs.Add(Describe(current.ToString(), body.Start + start, body.End - 1));
        }

        return paragraphs;
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
    private Ww8LayoutParagraph Describe(string text, int start, int markPosition)
    {
        Ww8LayoutFormat layout = ResolveLayoutFormat(markPosition);
        Ww8ParagraphFormat paragraph = ResolveParagraphFormat(markPosition);

        // The run properties at the paragraph's first character, which is what its text is mostly set
        // in. A paragraph whose runs differ in size is measured in this one; the tallest run should set
        // the line's height, and that needs the runs walked.
        Ww8LayoutFormat character = ResolveCharacterLayout(
            Math.Min(Math.Max(start, 0), Math.Max(markPosition, 0)));

        Length size = character.FontSizeHalfPoints is { } halves and > 0 and <= 4000
            ? Length.FromPoints(halves / 2.0)
            : Length.FromPoints(10);

        return new Ww8LayoutParagraph(
            text,
            layout.ToParagraphFormat(size),
            character.FontIndex is { } index ? Fonts.Name(index) : null,
            size,
            character.IsBold == true ? 700 : 400,
            character.IsItalic == true,
            character.LanguageId is { } id and > 0 and <= ushort.MaxValue
                ? WindowsLanguages.TagOf((ushort)id)
                : null,
            paragraph.IsInTable);
    }

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
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        ReadOnlyMemory<byte> direct = _characterProperties.Find(byteOffset);

        ushort styleIndex = ParagraphStyleIndexAt(position);

        Ww8LayoutFormat format = default;
        foreach (ReadOnlyMemory<byte> inherited in _styles.ResolveCharacterChain(styleIndex))
        {
            format = ApplyLayoutSprms(format, inherited);
        }

        return ApplyLayoutSprms(format, direct);
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
    }
}
