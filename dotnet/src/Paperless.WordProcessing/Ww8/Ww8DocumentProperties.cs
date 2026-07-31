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

        if (dop.Length >= CompatibilityOptions2Offset + 4)
        {
            uint options = BinaryPrimitives.ReadUInt32LittleEndian(dop[CompatibilityOptions2Offset..]);
            properties = properties with { CollapsesSpacing = (options & 0x00000004) == 0 };
        }

        return properties;
    }
}
