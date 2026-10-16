using System.Text;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Numbering;
using XRay.Office.MsBinary.Records;
using XRay.Office.Text.Encodings;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// One text run as PowerPoint stores it: the characters, and the paragraph and character
/// property runs that divide them.
/// </summary>
/// <param name="Kind">Which master style the run inherits from, per its <c>TextHeaderAtom</c>.</param>
/// <param name="Text">The characters, with the terminator and any trailing NULs removed.</param>
/// <param name="Paragraphs">The paragraph property runs, in order.</param>
/// <param name="Characters">The character property runs, in order.</param>
/// <param name="Ruler">
/// The shape's own per-level indents, when it carries a <c>TextRulerAtom</c>.
/// </param>
/// <param name="Extended">
/// The shape's <c>ExtendedParagraphAtom</c> entries, which a character run selects into by index.
/// Empty when the shape carries none.
/// </param>
/// <param name="Hyperlinks">
/// The character ranges the shape's text-range hyperlinks cover, when the deck declares the
/// hyperlinks they name. Empty for extraction, which resolves no fields.
/// </param>
public sealed record PptTextRun(
    PptTextKind Kind,
    string Text,
    IReadOnlyList<PptParagraphRun> Paragraphs,
    IReadOnlyList<PptCharacterRun> Characters,
    PptTextRuler? Ruler = null,
    IReadOnlyList<PptExtendedParagraph>? Extended = null,
    IReadOnlyList<PptHyperlinkRange>? Hyperlinks = null)
{
    /// <summary>Whether the character at a position is inside a text-range hyperlink.</summary>
    /// <param name="position">A position in <see cref="Text"/>.</param>
    public bool IsLinked(int position)
    {
        if (Hyperlinks is not { Count: > 0 } ranges) return false;

        foreach (PptHyperlinkRange range in ranges)
        {
            if (position >= range.Start && position < range.End) return true;
        }

        return false;
    }

    /// <summary>The extension a character run selects, or null when it selects none.</summary>
    /// <param name="index">The run's <see cref="PptCharacterRun.ExtendedIndex"/>.</param>
    public PptExtendedParagraph? ExtensionAt(int index)
        => Extended is { } entries && index >= 0 && index < entries.Count ? entries[index] : null;
}

/// <summary>
/// One text-range hyperlink: the characters of a shape's text that are drawn as a field.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A <c>.ppt</c> hyperlink is an EditEngine field, exactly as a DrawingML one is.</strong>
/// <c>PPTTextObj</c>'s <c>PPT_PST_InteractiveInfo</c> case builds a
/// <c>SvxFieldItem(SvxURLField(…), EE_FEATURE_FIELD)</c> for the range
/// (<c>filter/source/msfilter/svdfppt.cxx</c>:6936) and the second seat splits it across the
/// <c>PPTCharPropSet</c>s the range covers, giving each its own field whose representation is
/// that portion's own characters (<c>:7069</c> for a portion the range swallows whole and
/// <c>:7090</c> for the one it ends inside). So the range becomes <em>one field per character
/// run</em>, which is the same shape as DrawingML's one field per <c>a:r</c>.
/// </para>
/// <para>
/// The record pair is <c>InteractiveInfo</c> and a <em>sibling</em>
/// <c>TxInteractiveInfoAtom</c> holding two 32-bit character positions; a zero end means no
/// range at all and no entry is made (<c>:6926-6933</c>). Whether the pair becomes a field is
/// decided elsewhere — see <see cref="PptHyperlinks"/>.
/// </para>
/// </remarks>
/// <param name="Start">The first character the link covers.</param>
/// <param name="End">One past the last.</param>
public readonly record struct PptHyperlinkRange(int Start, int End);

/// <summary>
/// One shape's <c>TextRulerAtom</c>: per-outline-level indents that override the master's.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A shape may state its own indents, and they are not in its paragraph properties.</strong>
/// PowerPoint writes them into a separate record beside the text — <c>PPT_PST_TextRulerAtom</c>,
/// 4006 — holding a flags word and then, for each of the five outline levels that the flags name,
/// where that level's text starts and where its bullet starts. The paragraph property runs say
/// nothing about them, so a reader that only reads those falls through to the master's levels and
/// puts every indent and every bullet in the wrong place.
/// </para>
/// <para>
/// <c>PPTTextRulerInterpreter</c>, <c>filter/source/msfilter/svdfppt.cxx:3021-3084</c>. The flags
/// are: bit 0 a default tab, bit 1 a level count that nothing reads, bit 2 a tab-stop list, then
/// <c>8 &lt;&lt; level</c> for that level's text offset and <c>256 &lt;&lt; level</c> for its
/// bullet offset — and the fields appear in exactly that interleaved order, so a flag missed is a
/// misread of everything after it rather than one absent value.
/// </para>
/// <para>
/// Measured on <c>slides/done-014/ppt/Aerospace_Journey_of_Flight_Chapter_*.ppt</c> page 5, whose
/// body shape carries a 24-byte ruler with flags <c>0x1EF9</c>: it states
/// <c>textOfs[0] = 152</c>, <c>textOfs[1] = 419</c> and <c>bulletOfs[1] = 304</c> master units
/// where the master's Body levels say 228, 495 and 304. The reference draws that page's
/// level-zero text 18.99 pt from the text area's left edge (152 units), its level-one text
/// 52.38 pt (419) and its level-one bullet 38.01 pt (304) — the ruler's three numbers to a
/// hundredth of a point, and none of the master's.
/// </para>
/// </remarks>
/// <param name="DefaultTab">The tab interval in master units, or null when the ruler states none.</param>
/// <param name="TextOffsets">Where each level's text starts, null for a level the flags omit.</param>
/// <param name="BulletOffsets">Where each level's bullet starts, on the same terms.</param>
public sealed record PptTextRuler(
    ushort? DefaultTab,
    IReadOnlyList<ushort?> TextOffsets,
    IReadOnlyList<ushort?> BulletOffsets)
{
    /// <summary>How many outline levels a ruler can speak for.</summary>
    public const int Levels = 5;

    /// <summary>The text offset the ruler states for a level, or null when it states none.</summary>
    /// <param name="level">The outline level, zero for the first.</param>
    public ushort? TextOffset(int level)
        => level >= 0 && level < TextOffsets.Count ? TextOffsets[level] : null;

    /// <summary>The bullet offset the ruler states for a level, or null when it states none.</summary>
    /// <param name="level">The outline level, zero for the first.</param>
    public ushort? BulletOffset(int level)
        => level >= 0 && level < BulletOffsets.Count ? BulletOffsets[level] : null;
}

/// <summary>
/// One entry of a shape's <c>ExtendedParagraphAtom</c>: a picture bullet or an automatic number.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A numbered PowerPoint list does not say so anywhere a paragraph property run can be
/// seen.</strong> The paragraph still states an ordinary bullet — often the master's round dot —
/// and the numbering lives in a separate structure in the shape's private data, selected per
/// <em>character</em> run by four bits of its flags word. A reader that resolves only the
/// paragraph properties therefore draws a bullet where the reference draws <c>(a)</c> or
/// <c>I.</c>, with no sign in the file that anything was missed.
/// </para>
/// <para>
/// <c>StyleTextProp9::Read</c>, <c>filter/source/msfilter/svdfppt.cxx:4812-4830</c>. The layout is
/// three masks with optional fields hanging off each: the paragraph mask, then
/// <c>buBlip</c> under <c>0x00800000</c>, <c>hasAnm</c> under <c>0x02000000</c>,
/// <c>anmScheme</c> under <c>0x01000000</c> and a PP10 extension under <c>0x04000000</c>; then a
/// character mask with one optional word; then a special-info mask with two.
/// </para>
/// <para>
/// Measured on two corpus decks. <c>slides/done-008/ppt/RESPA_-_Section_8_Webinar.ppt</c>'s
/// page-4 body carries a 52-byte atom whose first entry is
/// <c>mask=0x03800000 buBlip=0xFFFF hasAnm=1 anmScheme=0x00010008</c> — scheme 8, a lower-case
/// letter in parentheses, starting at 1 — and the reference labels that slide's four paragraphs
/// <c>(a) (b) (c) (d)</c> while we draw four round bullets. Its second entry states nothing, and
/// the character run covering the trailing paragraph selects it, which is why the reference
/// leaves that paragraph unlabelled.
/// <c>slides/done-004/ppt/undp_presentation_revised_17_may.ppt</c> carries one entry,
/// <c>hasAnm=1 anmScheme=0x00010007</c> — scheme 7, an upper-case roman numeral with a full stop,
/// starting at 1 — reached through the document's presentation-rules route rather than through a
/// shape, and the reference numbers its outline <c>I. II. III. IV. V. VI.</c>
/// </para>
/// </remarks>
/// <param name="Mask">
/// The paragraph mask the entry opens with, <c>mnExtParagraphMask</c>. It is kept because it is
/// what decides the <em>merge</em> with the master's level: <c>ImplGetExtNumberFormat</c> takes a
/// field from the master only where the paragraph's own mask does not name it, and skips the merge
/// entirely when the mask names all three (<c>svdfppt.cxx:3407-3446</c>). Without it, "the entry
/// states no picture bullet" and "the entry says nothing about picture bullets" are the same
/// value and the level's own blip can never be inherited.
/// </param>
/// <param name="BulletBlip">
/// The picture-bullet index into the document's BLIP store, or <c>0xFFFF</c> for none.
/// </param>
/// <param name="HasAutoNumber">Whether the paragraphs it covers are automatically numbered.</param>
/// <param name="Scheme">
/// The numbering scheme in the low word and the start value in the high word, as
/// <c>ImplGetExtNumberFormat</c> reads them (<c>svdfppt.cxx:3466-3630</c>).
/// </param>
public readonly record struct PptExtendedParagraph(
    uint Mask,
    ushort BulletBlip,
    bool HasAutoNumber,
    uint Scheme)
{
    /// <summary>The value meaning "no picture bullet".</summary>
    public const ushort NoBulletBlip = 0xFFFF;

    /// <summary>The scheme's own number, which names the alphabet and the punctuation.</summary>
    public int SchemeKind => (int)(Scheme & 0xFFFF);

    /// <summary>
    /// The number the run starts at, or null when the entry does not state one.
    /// </summary>
    /// <remarks>
    /// <c>rStartNumbering = nAnmScheme &gt;&gt; 16</c>, applied only when the entry states
    /// <c>hasAnm</c> and the value is not −1 (<c>svdfppt.cxx:3629-3637</c>).
    /// </remarks>
    public int? Start
    {
        get
        {
            int stated = (int)(Scheme >> 16);
            return HasAutoNumber && stated >= 0 && stated != 0xFFFF ? stated : null;
        }
    }
}

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
/// <param name="BulletFlags">
/// The bullet-flags word the low four mask bits share, kept whole.
/// <para>
/// Bit 0 is already folded into <paramref name="HasBullet"/>, but the other three are not
/// redundant with it: bits 1, 2 and 3 are <c>PPT_ParaAttr_BuHardFont</c>,
/// <c>BuHardColor</c> and <c>BuHardHeight</c>, and each says whether the value beside it is the
/// bullet's own or a word PowerPoint wrote and meant nothing by
/// (<c>filter/source/msfilter/svdfppt.cxx:4881-4883</c>). Discarding them made every binary
/// PowerPoint bullet take a colour the file had not asked for.
/// </para>
/// </param>
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
    uint BulletColour = 0,
    ushort BulletFlags = 0)
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
/// <param name="ExtendedIndex">
/// Which <see cref="PptExtendedParagraph"/> the run selects, from bits 10–13 of its flags word.
/// Zero when the run's mask does not name them, which is also the first entry — the same
/// conflation <c>PPTStyleTextPropReader::ReadCharProps</c> makes (<c>svdfppt.cxx:5171-5182</c>).
/// </param>
public readonly record struct PptCharacterRun(
    int Length,
    RunEmphasis Emphasis,
    RunEmphasis Stated,
    uint Mask = 0,
    ushort FontIndex = 0,
    ushort FontHeight = 0,
    uint Colour = 0,
    short Escapement = 0,
    int ExtendedIndex = 0)
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
    /// <param name="extended">
    /// The shape's <c>ExtendedParagraphAtom</c> entries, which live outside this range — see
    /// <see cref="ReadExtendedParagraphs"/>.
    /// </param>
    /// <param name="hyperlinks">
    /// The identifiers the deck's <c>ExObjList</c> declares, from <see cref="PptHyperlinks"/>.
    /// Null for extraction, which draws nothing and so needs no field.
    /// </param>
    public static PptTextRun? Read(
        DffRecordBuffer stream,
        int start,
        int end,
        PptFieldValues fields = default,
        IReadOnlyList<PptExtendedParagraph>? extended = null,
        IReadOnlySet<uint>? hyperlinks = null)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptTextKind kind = PptTextKind.Other;
        string? text = null;
        DffRecordHeader? style = null;
        PptTextRuler? ruler = null;
        List<(int Position, string Value)>? markers = null;
        List<PptHyperlinkRange>? links = null;

        // The identifier of the InteractiveInfo the walk has just stepped over, because the
        // range it covers is in the record *after* it rather than inside it: LibreOffice seeks
        // to the end of the container and requires the very next header to be a
        // TxInteractiveInfoAtom, putting the record back and giving up on the range when it is
        // not (svdfppt.cxx:6911-6921).
        uint? pending = null;

        foreach (DffRecordHeader record in stream.Range(start, end))
        {
            ReadOnlySpan<byte> content = stream.Content(record);

            uint? previous = pending;
            pending = null;

            if (record.Type == PptRecordTypes.InteractiveInfo)
            {
                pending = HyperlinkReference(stream, record);
                continue;
            }

            if (record.Type == PptRecordTypes.TxInteractiveInfoAtom
                && previous is { } reference
                && hyperlinks is not null
                && hyperlinks.Contains(reference)
                && content.Length >= 8)
            {
                int from = (int)DffRecordBuffer.ReadUInt32(content);
                int to = (int)DffRecordBuffer.ReadUInt32(content[4..]);

                // A zero end is not a range: LibreOffice makes no field entry for one, so the
                // text it would have covered is drawn exactly as it stands.
                if (to > from && from >= 0) (links ??= []).Add(new PptHyperlinkRange(from, to));
                continue;
            }

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

                case PptRecordTypes.TextRulerAtom:
                    ruler ??= ReadRuler(content);
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

        if (markers is not null) text = Substitute(text, markers, paragraphs, characters, links);

        return new PptTextRun(kind, Broken(kind, text), paragraphs, characters, ruler, extended, links);
    }

    /// <summary>
    /// A slide title's returns, which are line breaks and not paragraph ends.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A <c>PageTitle</c> text object has exactly one paragraph however many returns it
    /// holds.</strong> <c>PPTStyleTextPropReader::Init</c> rewrites every <c>0x0d</c> to
    /// <c>0x0b</c> — a vertical tab, EditEngine's soft break — when the text header names
    /// instance 0, and records a <c>PPT_SPEC_NEWLINE</c> marker for every other instance
    /// (<c>filter/source/msfilter/svdfppt.cxx</c>:5241-5246 for the Unicode record and
    /// :5261-5266 for the byte one). Only the markers split paragraphs, so a two-line title
    /// arrives as one paragraph with a break in it rather than as two paragraphs — which is a
    /// paragraph space, a second bullet and a second first-line indent's worth of difference.
    /// </para>
    /// <para>
    /// <c>TSS_Type::PageTitle</c> is 0 and <c>TSS_Type::Title</c> — the title of a title slide,
    /// <see cref="PptTextKind.CentreTitle"/> — is 6 (<c>include/filter/msfilter/svdfppt.hxx</c>
    /// :157-169); the rewrite is on the first only, and following the name rather than the number
    /// would apply it to the wrong one.
    /// </para>
    /// </remarks>
    private static string Broken(PptTextKind kind, string text)
        => kind == PptTextKind.Title && text.Contains(ParagraphSeparator, StringComparison.Ordinal)
            ? text.Replace(ParagraphSeparator, LineBreak)
            : text;

    /// <summary>
    /// Which hyperlink an <c>InteractiveInfo</c> names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// <c>exHyperlinkId</c> is the second word of the <c>InteractiveInfoAtom</c>, after the
    /// sound reference (<c>ReadPptInteractiveInfoAtom</c>,
    /// <c>filter/source/msfilter/svdfppt.cxx:227-240</c>).
    /// </remarks>
    private static uint? HyperlinkReference(DffRecordBuffer stream, DffRecordHeader info)
    {
        foreach (DffRecordHeader child in stream.Children(info))
        {
            if (child.Type != PptRecordTypes.InteractiveInfoAtom) continue;

            ReadOnlySpan<byte> content = stream.Content(child);
            return content.Length >= 8 ? DffRecordBuffer.ReadUInt32(content[4..]) : null;
        }

        return null;
    }

    /// <summary>
    /// The <c>TextRulerAtom</c> of a shape's own client textbox, for the text it only refers to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shape whose text is an <c>OutlineTextRefAtom</c> keeps its <em>ruler</em> in its own
    /// client textbox while the characters live in the document's slide list, and the reference
    /// reads the two from those two places: <c>PPTTextObj</c>'s constructor locates
    /// <c>PPT_PST_TextRulerAtom</c> inside <c>aClientTextBoxHd</c> and remembers its file offset
    /// <strong>before</strong> patching that header to point at the referenced text
    /// (<c>filter/source/msfilter/svdfppt.cxx</c>, the <c>nTextRulerAtomOfs</c> block), then builds
    /// its <c>PPTTextRulerInterpreter</c> from that offset once the patch has happened.
    /// </para>
    /// <para>
    /// <strong>Every ruler in the two decks this matters to is in exactly that position.</strong>
    /// Counted over the record tree of both: <c>ws_prod-…-M.017-(French)-France.ppt</c> holds
    /// <b>54</b> <c>TextRulerAtom</c> records and
    /// <c>ws_prod-g-doc-Events-Part-M-presentation.ppt</c> <b>3</b>, and in both files
    /// <em>every one of them</em> sits in a client textbox that also holds an
    /// <c>OutlineTextRefAtom</c> — 54 of 54 and 3 of 3. Read through
    /// <see cref="Read(DffRecordBuffer,int,int,PptFieldValues,IReadOnlyList{PptExtendedParagraph},IReadOnlySet{uint})"/>
    /// alone, which returns before it reaches the ruler on that branch, not one of the 57 was ever
    /// applied.
    /// </para>
    /// </remarks>
    /// <param name="stream">The document stream.</param>
    /// <param name="start">Where the shape's client textbox content begins.</param>
    /// <param name="end">Where it ends.</param>
    internal static PptTextRuler? RulerIn(DffRecordBuffer stream, int start, int end)
    {
        ArgumentNullException.ThrowIfNull(stream);

        foreach (DffRecordHeader record in stream.Range(start, end))
        {
            if (record.Type != PptRecordTypes.TextRulerAtom) continue;

            return ReadRuler(stream.Content(record));
        }

        return null;
    }

    /// <summary>
    /// Reads a <c>TextRulerAtom</c>, or returns null when it states nothing this consumes.
    /// </summary>
    /// <remarks>
    /// The field order is the interleaved one <see cref="PptTextRuler"/> describes, and it is the
    /// whole content of the record: a flag skipped is not one value lost but every value after it
    /// read from the wrong offset. The tab-stop list is stepped over rather than kept, because
    /// nothing on the slide path consumes an explicit tab stop yet; its size is two words per
    /// stop, which the count preceding it gives.
    /// </remarks>
    private static PptTextRuler? ReadRuler(ReadOnlySpan<byte> content)
    {
        if (content.Length < 4) return null;

        int position = 0;
        uint flags = Take32(content, ref position);

        ushort? defaultTab = null;
        ushort?[] textOffsets = new ushort?[PptTextRuler.Levels];
        ushort?[] bulletOffsets = new ushort?[PptTextRuler.Levels];

        // Bit 1 is a level count that LibreOffice reads and discards; it still occupies a word.
        if ((flags & 0x0002) != 0) Skip(ref position, 2);
        if ((flags & 0x0001) != 0) defaultTab = Take16(content, ref position);

        if ((flags & 0x0004) != 0)
        {
            int stops = Take16(content, ref position);
            Skip(ref position, 4 * stops);
        }

        for (int level = 0; level < PptTextRuler.Levels; level++)
        {
            if ((flags & (8u << level)) != 0) textOffsets[level] = Take16(content, ref position);
            if ((flags & (256u << level)) != 0) bulletOffsets[level] = Take16(content, ref position);
        }

        bool states = defaultTab is not null;
        foreach (ushort? value in textOffsets) states |= value is not null;
        foreach (ushort? value in bulletOffsets) states |= value is not null;

        return states ? new PptTextRuler(defaultTab, textOffsets, bulletOffsets) : null;
    }

    /// <summary>
    /// A shape's <c>ExtendedParagraphAtom</c> entries, from its <c>ClientData</c>.
    /// </summary>
    /// <remarks>
    /// The record is three levels down and the middle level is an <em>atom</em> that holds
    /// records, so it is reached explicitly rather than by a container walk:
    /// <c>ClientData</c> → <c>ProgTags</c> → a <c>ProgBinaryTag</c> whose opening
    /// <c>CString</c> reads <c>___PPT9</c> → its <c>BinaryTagData</c> →
    /// <c>ExtendedParagraphAtom</c>. Mirrors <c>SeekToContentOfProgTag</c>
    /// (<c>svdfppt.cxx:6547-6551</c>).
    /// </remarks>
    /// <param name="stream">The document stream.</param>
    /// <param name="clientData">The shape's <c>ClientData</c> record, when it has one.</param>
    public static IReadOnlyList<PptExtendedParagraph>? ReadExtendedParagraphs(
        DffRecordBuffer stream, DffRecordHeader? clientData)
    {
        ArgumentNullException.ThrowIfNull(stream);

        if (clientData is not { } data) return null;

        foreach (DffRecordHeader record in PptProgTags.Content(stream, data))
        {
            if (record.Type != PptRecordTypes.ExtendedParagraphAtom) continue;

            return ReadExtendedParagraphAtom(stream.Content(record));
        }

        return null;
    }

    /// <summary>
    /// Reads an <c>ExtendedParagraphAtom</c>'s entries, in the order a character run indexes them.
    /// </summary>
    /// <remarks>
    /// Every field is optional and every mask decides the size of what follows it, so an
    /// unrecognised bit is not one value lost but every entry after it read from the wrong offset.
    /// The three masks and their fields are <c>StyleTextProp9::Read</c>'s, in its order.
    /// </remarks>
    public static IReadOnlyList<PptExtendedParagraph> ReadExtendedParagraphAtom(
        ReadOnlySpan<byte> content)
    {
        List<PptExtendedParagraph> entries = [];
        int position = 0;

        while (position + 4 <= content.Length)
        {
            uint paragraph = Take32(content, ref position);

            ushort blip = PptExtendedParagraph.NoBulletBlip;
            bool numbered = false;
            uint scheme = 0;

            if ((paragraph & 0x0080_0000) != 0) blip = Take16(content, ref position);
            if ((paragraph & 0x0200_0000) != 0) numbered = Take16(content, ref position) != 0;
            if ((paragraph & 0x0100_0000) != 0) scheme = Take32(content, ref position);
            if ((paragraph & 0x0400_0000) != 0) Skip(ref position, 4);

            uint character = Take32(content, ref position);
            if ((character & 0x0010_0000) != 0) Skip(ref position, 4);

            uint special = Take32(content, ref position);
            if ((special & 0x0000_0020) != 0) Skip(ref position, 4);
            if ((special & 0x0000_0040) != 0) Skip(ref position, 2);

            if (position > content.Length) break;

            entries.Add(new PptExtendedParagraph(paragraph, blip, numbered, scheme));
        }

        return entries;
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
        List<PptCharacterRun> characters,
        List<PptHyperlinkRange>? links = null)
    {
        markers.Sort(static (a, b) => b.Position.CompareTo(a.Position));

        foreach ((int position, string value) in markers)
        {
            if (position < 0 || position >= text.Length) continue;
            if (text[position] != FieldMarker) continue;

            text = string.Concat(text.AsSpan(0, position), value, text.AsSpan(position + 1));

            int delta = value.Length - 1;
            if (delta == 0) continue;

            // A hyperlink range is stated in the characters the file holds, and a running field
            // is one character there and several on the page. The markers are applied from the
            // end backwards for exactly this reason, so a range wholly before the marker needs
            // no correction, one wholly after it moves, and one straddling it grows.
            if (links is not null)
            {
                for (int i = 0; i < links.Count; i++)
                {
                    PptHyperlinkRange range = links[i];
                    int from = range.Start > position ? range.Start + delta : range.Start;
                    int to = range.End > position ? range.End + delta : range.End;
                    links[i] = new PptHyperlinkRange(from, to);
                }
            }

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
                bulletColour,
                bulletFlags ?? 0));

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
                escapement,
                (mask & 0x3C00) != 0 ? (flags & 0x3C00) >> 10 : 0));
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
