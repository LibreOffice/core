using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// A Word <em>text frame</em> — what WW8 calls an APO — as the paragraph properties state it.
/// </summary>
/// <remarks>
/// <para>
/// Word has two unrelated ways of putting a block of text somewhere on the page, and only one of them
/// is a drawing. A <em>text box</em> is an Escher shape with an <c>FSPA</c> anchoring it and its text in
/// a separate story; a <em>text frame</em> is nothing but a run of ordinary body paragraphs that each
/// carry the same handful of paragraph sprms saying where the block goes. Nothing in the main stream
/// marks the run's beginning or end — it is delimited by the sprms ceasing to match, which is exactly
/// how LibreOffice finds it (<c>SwWW8ImplReader::TestSameApo</c>,
/// <c>sw/source/filter/ww8/ww8par6.cxx:2823</c>, comparing two <c>WW8FlyPara</c>s field by field).
/// </para>
/// <para>
/// A reader that does not know about the second kind lays those paragraphs out in the flow, where they
/// take vertical room they were never meant to take and push everything after them down the page. That
/// is not a rare shape: 28 of the 66 <c>.doc</c> files in the corpus's words track import with at least
/// one, because a masthead, a pull quote or a sidebar in a Word 97 document is normally one of these.
/// </para>
/// <para>
/// The fields mirror <c>WW8FlyPara</c> (<c>sw/source/filter/ww8/ww8par.hxx:208</c>) and are compared the
/// same way, so <see cref="IsEmpty"/> and equality decide the run's extent between them.
/// </para>
/// </remarks>
/// <param name="Binding">
/// <c>sprmPPc</c>'s byte: bits 6–7 are the horizontal origin, bits 4–5 the vertical one.
/// </param>
/// <param name="XOffset">
/// <c>sprmPDxaAbs</c>, in twips — or one of five negative sentinels naming an edge to align against.
/// </param>
/// <param name="YOffset">
/// <c>sprmPDyaAbs</c>, in twips, with the same sentinel convention on a different set of values.
/// </param>
/// <param name="Width"><c>sprmPDxaWidth</c>, in twips; ten or less means "as wide as it needs to be".</param>
/// <param name="Height">
/// <c>sprmPWHeightAbs</c>, in twips. Bit 15 asks for the stated height to be a minimum rather than an
/// exact size, which is why it is masked off before the value is used.
/// </param>
/// <param name="FromTextX"><c>sprmPDxaFromText</c>: how far text keeps clear, left and right.</param>
/// <param name="FromTextY"><c>sprmPDyaFromText</c>: the same above and below.</param>
/// <param name="Wrap">
/// <c>sprmPWr</c>. Two is the default rather than zero, which is why <see cref="IsEmpty"/> has to treat
/// the two alike — <c>WW8FlyPara::IsEmpty</c> says so in as many words.
/// </param>
/// <param name="StatesVerticalPosition">
/// Whether <c>sprmPDyaAbs</c> was stated at all, by the paragraph or by its style. Word ignores the
/// stated vertical origin when it was not, leaving the frame relative to the text — #i8798#, and
/// <c>WW8FlyPara::Read</c> rewrites <paramref name="Binding"/> to say so.
/// </param>
public readonly record struct Ww8TextFramePosition(
    byte Binding,
    short XOffset,
    short YOffset,
    short Width,
    short Height,
    short FromTextX,
    short FromTextY,
    byte Wrap,
    bool StatesVerticalPosition)
{
    /// <summary>What a paragraph stating none of the sprms has.</summary>
    /// <remarks>
    /// <see cref="Wrap"/> is two rather than nought: <c>WW8FlyPara</c>'s constructor seeds it that way
    /// and calls the value "Default: wrapping" (<c>ww8par6.cxx:1660</c>).
    /// </remarks>
    public static Ww8TextFramePosition None { get; } = new() { Wrap = 2 };

    /// <summary>
    /// True when nothing here asks for a frame.
    /// </summary>
    /// <remarks>
    /// A wrap of nought counts as absent alongside the default of two, which is
    /// <c>WW8FlyPara::IsEmpty</c>'s rule verbatim (<c>ww8par6.cxx:1871</c>): "wr of 0 like 2 appears to
    /// me to be equivalent for checking here".
    /// </remarks>
    public bool IsEmpty =>
        (this with { Wrap = Wrap == 0 ? (byte)2 : Wrap, StatesVerticalPosition = false }) == None;

    /// <summary>Which of the four horizontal origins <see cref="Binding"/> names.</summary>
    public byte HorizontalBinding => (byte)((Binding & 0xC0) >> 6);

    /// <summary>Which of the four vertical origins it names.</summary>
    public byte VerticalBinding => (byte)((Binding & 0x30) >> 4);
}

/// <summary>
/// The blocks one text frame holds, and where the file puts it.
/// </summary>
/// <param name="Position">Where it goes.</param>
/// <param name="Blocks">The paragraphs taken out of the flow to fill it.</param>
public sealed record Ww8LayoutTextFrame(
    Ww8TextFramePosition Position,
    IReadOnlyList<Ww8LayoutBlock> Blocks);

/// <summary>
/// Turns a <see cref="Ww8TextFramePosition"/> into the layout engine's floating frame.
/// </summary>
/// <remarks>
/// The whole of this mapping is <c>WW8SwFlyPara</c>'s constructor
/// (<c>sw/source/filter/ww8/ww8par6.cxx:1885</c>), which is where the sentinel offsets and the binding
/// bits are turned into an origin and an alignment.
/// </remarks>
public static class Ww8TextFrames
{
    /// <summary>Word's minimum frame extent, in twips — <c>MINFLY</c> in <c>swtypes.hxx</c>.</summary>
    private const short MinimumExtent = 23;

    /// <summary>Builds the frame, or null when the position asks for nothing placeable.</summary>
    /// <param name="frame">The frame and its blocks.</param>
    /// <param name="blocks">Its blocks, already converted to the layout engine's types.</param>
    /// <param name="columnWidth">The text area's width, which an auto-width frame falls back on.</param>
    public static PageFrame? Build(
        Ww8LayoutTextFrame frame, IReadOnlyList<PageBlock> blocks, Length columnWidth)
    {
        ArgumentNullException.ThrowIfNull(frame);
        ArgumentNullException.ThrowIfNull(blocks);

        Ww8TextFramePosition position = frame.Position;

        // Bit 15 asks for a minimum rather than a fixed height, and layout gives a frame its content's
        // height in either case — so the flag is masked off and the value read as a floor.
        short height = (short)(position.Height & 0x7FFF);
        if (height <= MinimumExtent) height = MinimumExtent;

        short width = position.Width;
        Length frameWidth = width <= 10
            ? (columnWidth > Length.Zero ? columnWidth : Length.FromTwips(2268))
            : Length.FromTwips(Math.Max(width, MinimumExtent));

        (FrameVerticalAlignment verticalAlignment, Length verticalOffset, bool inline) =
            VerticalOf(position.YOffset);
        (FrameHorizontalAlignment horizontalAlignment, Length horizontalOffset) =
            HorizontalOf(position.XOffset);

        return new PageFrame
        {
            Size = new Core.Geometry.DocSize(frameWidth, Length.FromTwips(height)),

            // At-paragraph, not at-character: an APO has no anchor character anywhere in the text, and
            // the paragraph it hangs on is the first one left in the flow after its own were taken out.
            Anchor = FrameAnchor.Paragraph,
            AnchorOffset = 0,
            Wrap = WrapOf(position.Wrap),
            HorizontalOrigin = position.HorizontalBinding switch
            {
                0 => FrameHorizontalOrigin.Column,
                1 => FrameHorizontalOrigin.PageMargin,
                _ => FrameHorizontalOrigin.Page,
            },
            HorizontalAlignment = horizontalAlignment,
            HorizontalOffset = horizontalOffset,

            // #i18732#: a dyaAbs of nought means the frame sits in line with the text it was written
            // beside, whatever origin the binding bits name.
            VerticalOrigin = inline
                ? FrameVerticalOrigin.Paragraph
                : position.VerticalBinding switch
                {
                    0 => FrameVerticalOrigin.PageMargin,
                    1 => FrameVerticalOrigin.Page,
                    _ => FrameVerticalOrigin.Paragraph,
                },
            VerticalAlignment = verticalAlignment,
            VerticalOffset = verticalOffset,
            Spacing = new Core.Geometry.Margins(
                Length.FromTwips(position.FromTextX),
                Length.FromTwips(position.FromTextY),
                Length.FromTwips(position.FromTextX),
                Length.FromTwips(position.FromTextY)),
            Blocks = blocks,
        };
    }

    /// <summary>
    /// Word's six wrap modes, mapped as <c>WW8SwFlyPara</c>'s constructor maps them
    /// (<c>sw/source/filter/ww8/ww8par6.cxx:1913</c>).
    /// </summary>
    /// <remarks>
    /// Nought — <c>ST_Wrap: auto</c> — and anything unrecognised become <c>DYNAMIC</c> there, which is
    /// this engine's <see cref="TextWrap.Optimal"/>: whichever side has more room.
    /// </remarks>
    private static TextWrap WrapOf(byte wrap) => wrap switch
    {
        1 or 3 => TextWrap.TopAndBottom,
        2 or 4 => TextWrap.Both,
        5 => TextWrap.Through,
        _ => TextWrap.Optimal,
    };

    /// <summary>
    /// The vertical alignment a <c>dyaAbs</c> names, and the offset left over when it names none.
    /// </summary>
    /// <remarks>
    /// Four of the values are sentinels rather than distances, and nought is a fifth meaning "in line
    /// with the surrounding text" — <c>ww8par6.cxx:1985</c>.
    /// </remarks>
    private static (FrameVerticalAlignment Alignment, Length Offset, bool Inline) VerticalOf(short y)
        => y switch
        {
            0 => (FrameVerticalAlignment.Offset, Length.Zero, true),
            -4 => (FrameVerticalAlignment.Top, Length.Zero, false),
            -8 => (FrameVerticalAlignment.Middle, Length.Zero, false),
            -12 => (FrameVerticalAlignment.Bottom, Length.Zero, false),
            _ => (FrameVerticalAlignment.Offset, Length.FromTwips(y), false),
        };

    /// <summary>The horizontal alignment a <c>dxaAbs</c> names, on the same pattern.</summary>
    /// <remarks>
    /// Nought is an edge here rather than a distance — it means "left" — which is the one place the two
    /// axes disagree (<c>ww8par6.cxx:2010</c>).
    /// </remarks>
    private static (FrameHorizontalAlignment Alignment, Length Offset) HorizontalOf(short x)
        => x switch
        {
            0 => (FrameHorizontalAlignment.Left, Length.Zero),
            -4 => (FrameHorizontalAlignment.Centre, Length.Zero),
            -8 => (FrameHorizontalAlignment.Right, Length.Zero),
            -12 => (FrameHorizontalAlignment.Inside, Length.Zero),
            -16 => (FrameHorizontalAlignment.Outside, Length.Zero),
            _ => (FrameHorizontalAlignment.Offset, Length.FromTwips(x)),
        };
}
