using Paperless.Core.Numbering;

namespace Paperless.WordProcessing.Layout;

/// <summary>Where a class of note collects.</summary>
/// <remarks>
/// A footnote is always at the foot of its page; an <em>endnote</em> is not always at the end of the document,
/// which is the part that surprises. Every Word-family format can put endnotes at the end of each *section*
/// instead, and LibreOffice's own DOC export writes exactly that — so a document round-tripped through DOC has
/// its endnotes in the page-bottom note area of the section's last page rather than on pages of their own.
/// ODF has no per-section option at all, so its endnotes are always <see cref="DocumentEnd"/>.
/// </remarks>
public enum NotePlacement
{
    /// <summary>At the foot of the page that cites it, which is what a footnote is.</summary>
    PageBottom,

    /// <summary>
    /// In the note area of the last page of the citing note's section.
    /// </summary>
    /// <remarks>
    /// Writer's <c>FTNEND_ATTXTEND</c>, from RTF's <c>\aendsec</c>, OOXML's <c>w:pos w:val="sectEnd"</c> and
    /// the DOP's <c>epc</c> of zero. Measured on a single-section document: the notes land where the same
    /// document's footnotes do, at the foot of the page.
    /// </remarks>
    SectionEnd,

    /// <summary>On pages of their own after the last page of the document.</summary>
    DocumentEnd,
}

/// <summary>How a note's citation is written.</summary>
/// <remarks>
/// The formats every one of the four spells, under four different names — ODF's
/// <c>style:num-format</c>, OOXML's <c>w:numFmt</c>, RTF's <c>\ftnn*</c> family and the DOP's
/// <c>nfcFtnRef</c>. Word's <c>chicago</c> sequence is here because it is one of the values a real file
/// carries, not because it is common.
/// </remarks>
public enum NoteNumberFormat
{
    /// <summary>1, 2, 3 — what a footnote takes when nothing says otherwise.</summary>
    /// <remarks>
    /// Named for the digits rather than for what each format calls it, because <c>Decimal</c> is a type name
    /// and a member called that reads as the number rather than as the sequence.
    /// </remarks>
    Arabic,

    /// <summary>i, ii, iii — what an <em>endnote</em> takes when nothing says otherwise.</summary>
    LowerRoman,

    /// <summary>I, II, III.</summary>
    UpperRoman,

    /// <summary>a, b, c.</summary>
    LowerLetter,

    /// <summary>A, B, C.</summary>
    UpperLetter,

    /// <summary>
    /// Word's <c>chicago</c> sequence: *, †, ‡, § repeating in doubled runs.
    /// </summary>
    /// <remarks>
    /// The one format that is not an arithmetic progression, and the reason it is a case rather than a
    /// character list: the fifth mark is <c>**</c> rather than a fifth symbol.
    /// </remarks>
    Chicago,
}

/// <summary>
/// How one class of note is numbered: in what sequence, and from what value.
/// </summary>
/// <remarks>
/// <para>
/// Two values, because the formats state two and a document can change either alone — a legal brief
/// numbering its footnotes from 100 is as common as one numbering them in roman. The <em>restart</em> rules
/// (per page, per chapter, per section) are deliberately absent: they need to be applied while pages are
/// being filled rather than while the document is being read, so a reader cannot resolve them and a value
/// here would be a lie.
/// </para>
/// <para>
/// The defaults differ between the two classes and that is measured rather than assumed: LibreOffice cites
/// footnotes 1, 2, 3 and endnotes i, ii, iii on a document that states nothing at all.
/// </para>
/// </remarks>
/// <param name="Format">The sequence the citation is written in.</param>
/// <param name="StartAt">
/// The number the first note of the class takes. One in almost every document, and not always: it is what
/// makes the second volume of a book continue the first's footnote numbering.
/// </param>
public readonly record struct NoteNumbering(NoteNumberFormat Format, int StartAt)
{
    /// <summary>Where notes of this class collect.</summary>
    public NotePlacement Placement { get; init; }

    /// <summary>What a footnote takes when the document states nothing: 1, 2, 3, at the foot of the page.</summary>
    public static NoteNumbering Footnotes { get; } =
        new(NoteNumberFormat.Arabic, 1) { Placement = NotePlacement.PageBottom };

    /// <summary>What an endnote takes: i, ii, iii, at the end of the document.</summary>
    public static NoteNumbering Endnotes { get; } =
        new(NoteNumberFormat.LowerRoman, 1) { Placement = NotePlacement.DocumentEnd };

    /// <summary>The default for a class.</summary>
    /// <param name="isEndnote">True for an endnote.</param>
    public static NoteNumbering DefaultFor(bool isEndnote) => isEndnote ? Endnotes : Footnotes;

    /// <summary>
    /// The citation the <paramref name="index"/>th note of this class carries, counted from zero.
    /// </summary>
    /// <remarks>
    /// From zero rather than from one so that the start value composes cleanly: the first note is
    /// <see cref="StartAt"/> whatever that is. A start value below one is clamped rather than rejected,
    /// because the sequences have no zeroth term and a file can say anything.
    /// </remarks>
    /// <param name="index">How many notes of this class came before, counted from zero.</param>
    public string Citation(int index)
    {
        int value = Math.Max(1, StartAt + Math.Max(0, index));

        return Format switch
        {
            NoteNumberFormat.LowerRoman => OutlineNumbers.Roman(value, upperCase: false),
            NoteNumberFormat.UpperRoman => OutlineNumbers.Roman(value, upperCase: true),
            NoteNumberFormat.LowerLetter => OutlineNumbers.Alphabetic(value, upperCase: false),
            NoteNumberFormat.UpperLetter => OutlineNumbers.Alphabetic(value, upperCase: true),
            NoteNumberFormat.Chicago => Chicago(value),
            _ => OutlineNumbers.Digits(value),
        };
    }

    /// <summary>
    /// The <c>chicago</c> mark for a value: *, †, ‡, § and then each doubled, tripled and so on.
    /// </summary>
    /// <remarks>
    /// Word's own rule, and Writer's <c>SvxNumberType</c> agrees: the sequence does not run out of symbols,
    /// it repeats them. So the fifth footnote is <c>**</c> rather than a fifth character.
    /// </remarks>
    private static string Chicago(int value)
    {
        int at = (value - 1) % ChicagoMarks.Length;
        int repeats = ((value - 1) / ChicagoMarks.Length) + 1;

        return new string(ChicagoMarks[at], Math.Min(repeats, MaxChicagoRepeats));
    }

    /// <summary>The four marks Word's <c>chicago</c> sequence cycles through.</summary>
    private static readonly char[] ChicagoMarks = ['*', '†', '‡', '§'];

    /// <summary>How many times one mark is repeated before the sequence stops growing.</summary>
    /// <remarks>
    /// A guard rather than a rule: a document with two thousand footnotes would otherwise ask for a citation
    /// five hundred characters wide, which no line can hold and nothing sensible can do with.
    /// </remarks>
    private const int MaxChicagoRepeats = 16;

    /// <summary>
    /// The format a name states, or null when the name is one this does not model.
    /// </summary>
    /// <remarks>
    /// One parser for ODF's <c>style:num-format</c> and OOXML's <c>w:numFmt</c>, because the two spell the
    /// same set of sequences differently and neither spelling is a superset: ODF states the format by
    /// <em>example</em> — the literal string "1", "i", "I", "a" or "A" — while OOXML names it
    /// (<c>decimal</c>, <c>lowerRoman</c>). Returning null for anything else is what leaves the class's
    /// default in place, which is the right answer for the formats not modelled here.
    /// </remarks>
    /// <param name="stated">The attribute's value.</param>
    public static NoteNumberFormat? Parse(string? stated) => stated switch
    {
        null or "" => null,

        // ODF, which gives an example of the sequence rather than naming it.
        "1" or "01" => NoteNumberFormat.Arabic,
        "i" => NoteNumberFormat.LowerRoman,
        "I" => NoteNumberFormat.UpperRoman,
        "a" => NoteNumberFormat.LowerLetter,
        "A" => NoteNumberFormat.UpperLetter,

        // OOXML, which names it.
        "decimal" => NoteNumberFormat.Arabic,
        "lowerRoman" => NoteNumberFormat.LowerRoman,
        "upperRoman" => NoteNumberFormat.UpperRoman,
        "lowerLetter" => NoteNumberFormat.LowerLetter,
        "upperLetter" => NoteNumberFormat.UpperLetter,
        "chicago" => NoteNumberFormat.Chicago,

        _ => null,
    };
}
