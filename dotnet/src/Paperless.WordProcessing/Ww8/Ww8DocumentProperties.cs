using System.Buffers.Binary;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The document-wide layout decisions in a WW8 <c>Dop</c>.
/// </summary>
/// <remarks>
/// <para>
/// Only the fields layout needs, and there are not many: nearly everything in a <c>Dop</c> is a view
/// setting, a print option or a compatibility flag with no visible effect. The two that matter are the
/// default tab interval, which no paragraph states and every tab depends on, and whether the two
/// paragraph spacings add or the larger wins.
/// </para>
/// <para>
/// A fixed layout at fixed offsets, so the fields are read positionally rather than by walking a record.
/// The offsets come from <c>WW8Dop::WW8Dop</c> (<c>sw/source/filter/ww8/ww8scan.cxx</c>), which reads the
/// stream in order and comments each field's offset as it goes — the only reliable way to count past the
/// bitfields, since several bytes hold eight unrelated flags each.
/// </para>
/// </remarks>
public readonly record struct Ww8DocumentProperties
{
    /// <summary>Where <c>dxaTab</c> sits: a signed word, ten bytes in.</summary>
    private const int TabIntervalOffset = 0x0A;

    /// <summary>
    /// Where the second word of compatibility options sits.
    /// </summary>
    /// <remarks>
    /// Only Word 2000 and later write a <c>Dop</c> this long, which is why the length is checked rather
    /// than assumed: a Word 97 file stops well before here, and reading past its end would invent a flag.
    /// </remarks>
    private const int CompatibilityOptions2Offset = 0x200;

    /// <summary>The defaults, for a document whose <c>Dop</c> is missing or too short to read.</summary>
    /// <remarks>
    /// Half an inch, and spacings that do <em>not</em> collapse. Both are what Word itself defaults to, and
    /// the collapsing flag defaults to set in LibreOffice's own reader — a document written before the flag
    /// existed predates HTML auto-spacing entirely.
    /// </remarks>
    public static Ww8DocumentProperties Default { get; } = new()
    {
        DefaultTabInterval = Length.FromTwips(720),
        CollapsesSpacing = false,
        FootnoteNumbering = Layout.NoteNumbering.Footnotes,
        EndnoteNumbering = Layout.NoteNumbering.Endnotes,
    };

    /// <summary>The interval at which tabs fall when no stop covers them.</summary>
    public Length DefaultTabInterval { get; init; }

    /// <summary>
    /// True when two paragraphs' spacings collapse to the larger rather than adding.
    /// </summary>
    /// <remarks>
    /// The <em>negation</em> of <c>fDontUseHTMLAutoSpacing</c>, because the flag names what is switched off:
    /// HTML auto-spacing is the collapsing behaviour, so a document that does not use it adds the two
    /// spacings. One paragraph's space-after is worth a visible amount, so getting this backwards moves
    /// every line after the first paragraph boundary.
    /// </remarks>
    public bool CollapsesSpacing { get; init; }

    /// <summary>How the document's footnotes are numbered.</summary>
    /// <remarks>
    /// The DOP states the two classes in three different places: <c>nFootnote</c> and <c>nEdn</c> hold the
    /// first number and <c>nfcFootnoteRef</c> and <c>nfcEdnRef</c> the sequence, packed as bit fields into two
    /// different sixteen-bit words. Both start values are one-based, so a document that means "start at one"
    /// says one — unlike ODF, whose attribute is an offset.
    /// </remarks>
    public Layout.NoteNumbering FootnoteNumbering { get; init; }

    /// <inheritdoc cref="FootnoteNumbering"/>
    public Layout.NoteNumbering EndnoteNumbering { get; init; }

    /// <summary>Reads what layout needs from a <c>Dop</c>, falling back to the defaults per field.</summary>
    /// <param name="dop">The document properties stream.</param>
    public static Ww8DocumentProperties Parse(ReadOnlySpan<byte> dop)
    {
        Ww8DocumentProperties properties = Default;

        if (dop.Length >= TabIntervalOffset + 2)
        {
            short interval = BinaryPrimitives.ReadInt16LittleEndian(dop[TabIntervalOffset..]);

            // A zero or negative interval would make a tab advance nowhere and a layout loop; the bound
            // above it rejects the absurd rather than the merely wide.
            if (interval is > 0 and <= 31680) properties = properties with
            {
                DefaultTabInterval = Length.FromTwips(interval),
            };
        }

        // The first footnote number, at 0x02: two bits of restart rule and then the value. Zero means the
        // document said nothing, which is why LibreOffice's own import guards on it before subtracting one.
        if (dop.Length >= FootnoteNumberOffset + 2)
        {
            int packed = BinaryPrimitives.ReadUInt16LittleEndian(dop[FootnoteNumberOffset..]) >> 2;
            if (packed > 0) properties = properties with
            {
                FootnoteNumbering = properties.FootnoteNumbering with { StartAt = packed },
            };
        }

        // The first endnote number, packed the same way at 0x34.
        if (dop.Length >= EndnoteNumberOffset + 2)
        {
            int packed = BinaryPrimitives.ReadUInt16LittleEndian(dop[EndnoteNumberOffset..]) >> 2;
            if (packed > 0) properties = properties with
            {
                EndnoteNumbering = properties.EndnoteNumbering with { StartAt = packed },
            };
        }

        // Both sequences share one word at 0x36, four bits each, above the two that hold the endnote
        // position. Reading the wrong field is not an error a document reports: it is a footnote numbered in
        // roman because the endnote's format was picked up.
        if (dop.Length >= NoteFormatsOffset + 2)
        {
            ushort word = BinaryPrimitives.ReadUInt16LittleEndian(dop[NoteFormatsOffset..]);

            // The lowest two bits are `epc`, where zero means "collect at the end of the section" and three
            // means the end of the document. LibreOffice's own WW8 export writes zero, which is why a document
            // round-tripped through DOC has its endnotes at the foot of a page rather than on pages of their
            // own — and why reading this is what makes a DOC agree with the file it came from.
            properties = properties with
            {
                EndnoteNumbering = properties.EndnoteNumbering with
                {
                    Placement = (word & 0x0003) == 0
                        ? Layout.NotePlacement.SectionEnd
                        : Layout.NotePlacement.DocumentEnd,
                },
            };

            properties = properties with
            {
                FootnoteNumbering = properties.FootnoteNumbering with
                {
                    Format = FormatOf((word & 0x003C) >> 2, Layout.NoteNumberFormat.Arabic),
                },
                EndnoteNumbering = properties.EndnoteNumbering with
                {
                    Format = FormatOf((word & 0x03C0) >> 6, Layout.NoteNumberFormat.LowerRoman),
                },
            };
        }

        if (dop.Length >= CompatibilityOptions2Offset + 4)
        {
            uint options = BinaryPrimitives.ReadUInt32LittleEndian(dop[CompatibilityOptions2Offset..]);
            properties = properties with { CollapsesSpacing = (options & 0x00000004) == 0 };
        }

        return properties;
    }

    /// <summary>
    /// The sequence an <c>MSONFC</c> code names, or a fallback for the codes not modelled.
    /// </summary>
    /// <remarks>
    /// The numbers come from <c>WW8ListManager::GetSvxNumTypeFromMSONFC</c>, which is also where the ordering
    /// surprise is: upper roman is 1 and <em>lower</em> roman is 2, and upper letter comes before lower in the
    /// same way. A reader that assumed the lower-case form came first gets I where the document says i. The
    /// codes past nine are the East Asian and enclosed-numeral sequences, which fall back rather than being
    /// approximated.
    /// </remarks>
    /// <param name="code">The four-bit code.</param>
    /// <param name="fallback">What to use for a code this does not model.</param>
    private static Layout.NoteNumberFormat FormatOf(int code, Layout.NoteNumberFormat fallback)
        => code switch
        {
            0 => Layout.NoteNumberFormat.Arabic,
            1 => Layout.NoteNumberFormat.UpperRoman,
            2 => Layout.NoteNumberFormat.LowerRoman,
            3 => Layout.NoteNumberFormat.UpperLetter,
            4 => Layout.NoteNumberFormat.LowerLetter,
            9 => Layout.NoteNumberFormat.Chicago,
            _ => fallback,
        };

    /// <summary>Where the first footnote number sits in the <c>Dop</c>, above two bits of restart rule.</summary>
    private const int FootnoteNumberOffset = 0x02;

    /// <summary>Where the first endnote number sits, packed the same way.</summary>
    private const int EndnoteNumberOffset = 0x34;

    /// <summary>Where both classes' sequence codes sit, four bits each in one word.</summary>
    private const int NoteFormatsOffset = 0x36;
}
