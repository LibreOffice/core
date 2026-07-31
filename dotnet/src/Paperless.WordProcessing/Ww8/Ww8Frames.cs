using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// Turns a WW8 shape — an <c>FSPA</c> anchor plus the Escher <c>SpContainer</c> it names — into the
/// layout engine's floating frame.
/// </summary>
/// <remarks>
/// <para>
/// The division of labour between the two records is the thing to hold on to, because neither is
/// enough on its own. The <c>FSPA</c> carries the rectangle, the two position origins and the wrap;
/// the shape's <c>OPT</c> property table carries the distances text must keep from it, the insets
/// between its edges and its own text, and its fill and line. Nothing states both, and a reader
/// consulting only one of them produces a frame in the right place that the text ignores, or a
/// correctly avoided frame at the origin.
/// </para>
/// <para>
/// Word's <c>posrelh</c>/<c>posrelv</c> pair, in the shape's <em>tertiary</em> property table,
/// overrides the <c>FSPA</c>'s own <c>nbx</c>/<c>nby</c> when it is present — and only when it is
/// present, which is why <see cref="EscherPropertyTable.Has"/> is asked rather than the value being
/// compared against zero. Zero is a meaningful origin (the page's printable area), so treating "not
/// stated" and "stated zero" alike moves every shape that relies on the <c>FSPA</c> to the page
/// margin. LibreOffice makes the same distinction with a <c>std::optional</c>
/// (<c>ww8graf.cxx:2316</c>).
/// </para>
/// </remarks>
public static class Ww8Frames
{
    /// <summary>
    /// How far text keeps clear of a shape that states no distance of its own, horizontally.
    /// </summary>
    /// <remarks>
    /// 114935 EMU — 181 twips, near enough an eighth of an inch but not exactly it — which is
    /// LibreOffice's default for the DOC path (<c>ww8par.cxx:1001</c>) and is confirmed by a round
    /// trip: converting <c>frame-wrap.doc</c>, whose shape states none of the four properties, back
    /// to ODF yields <c>fo:margin-left="0.1256in"</c>. Deliberately not the RTF reader's 0.2 cm; the
    /// two importers disagree and each matches its own format's rendering.
    /// </remarks>
    private const int DefaultWrapDistanceEmu = 114935;

    /// <summary>The inset between a shape's edge and its own text, when it states none.</summary>
    /// <remarks>
    /// 0.1 inch horizontally and 0.05 inch vertically (<c>ww8par.cxx:727</c>). A shape stating zero
    /// gets zero: <c>frame-wrap.doc</c> states all four as zero and LibreOffice draws its first line
    /// flush with the shape's left edge, at 56.7 pt where the shape's corner is 56.7 pt.
    /// </remarks>
    private const int DefaultTextInsetHorizontalEmu = 91440;

    /// <inheritdoc cref="DefaultTextInsetHorizontalEmu"/>
    private const int DefaultTextInsetVerticalEmu = 45720;

    /// <summary>A line's thickness when the shape states none: one point, in EMUs.</summary>
    private const uint DefaultLineWidthEmu = 9525;

    /// <summary>
    /// Builds the frame a shape stands for, or null when the shape is not something to place.
    /// </summary>
    /// <param name="anchor">The <c>FSPA</c> that anchors it in the text.</param>
    /// <param name="shape">The shape it names, or null when the drawing does not hold it.</param>
    /// <param name="offset">Where the anchor sits in its paragraph's text.</param>
    /// <param name="blocks">The shape's own text, already laid out into blocks.</param>
    public static PageFrame? Build(
        Ww8ShapeAnchor anchor, EscherShape? shape, int offset, IReadOnlyList<PageBlock> blocks)
    {
        if (anchor.Width <= 0 || anchor.Height <= 0) return null;
        if (shape is not null && (shape.IsDeleted || shape.Properties.Boolean(EscherPropertyIds.Hidden)))
        {
            return null;
        }

        EscherPropertyTable properties = shape?.Properties ?? EscherPropertyTable.Empty;
        EscherPropertyTable host = shape?.TertiaryProperties ?? EscherPropertyTable.Empty;

        // posh/posv, the pair beside the two origins: 0 means "at the stated offset" and 1 to 5 name
        // an edge to align against, at which point the FSPA's coordinate is not used at all. A reader
        // that always takes the offset places a centred shape wherever its last position happened to
        // be — which is a plausible coordinate, so the error looks like an arithmetic slip rather than
        // an unread field.
        Ww8ShapeOrigin horizontal = Origin(host, EscherPropertyIds.HorizontalRelation, anchor.HorizontalOrigin);
        Ww8ShapeOrigin vertical = Origin(host, EscherPropertyIds.VerticalRelation, anchor.VerticalOrigin);

        if (anchor.IsPageRelative)
        {
            // bRcaSimple: the rectangle is stated against the page whatever the origins say, and
            // LibreOffice overwrites both fields rather than reinterpreting the coordinates
            // (ww8graf.cxx:2856).
            horizontal = Ww8ShapeOrigin.Page;
            vertical = Ww8ShapeOrigin.Page;
        }

        bool lined = properties.Boolean(EscherPropertyIds.Lined, fallback: true);
        Length lineWidth = Length.FromEmu(
            lined ? properties.Value(EscherPropertyIds.LineWidth, DefaultLineWidthEmu) : 0);

        return new PageFrame
        {
            Size = new DocSize(Length.FromTwips(anchor.Width), Length.FromTwips(anchor.Height)),

            // Always to a character. Word has no paragraph anchor for a drawing — the FSPA names a
            // character position and nothing else — so LibreOffice's importer makes every floating
            // shape FLY_AT_CHAR unconditionally (ww8graf.cxx:2356).
            Anchor = FrameAnchor.Character,
            AnchorOffset = offset,
            Wrap = WrapOf(anchor),
            HorizontalOrigin = horizontal switch
            {
                Ww8ShapeOrigin.PageMargin => FrameHorizontalOrigin.PageMargin,
                Ww8ShapeOrigin.Page => FrameHorizontalOrigin.Page,
                Ww8ShapeOrigin.Character => FrameHorizontalOrigin.Character,
                _ => FrameHorizontalOrigin.Paragraph,
            },
            HorizontalAlignment = host.Value(EscherPropertyIds.HorizontalPosition) switch
            {
                1 => FrameHorizontalAlignment.Left,
                2 => FrameHorizontalAlignment.Centre,
                3 => FrameHorizontalAlignment.Right,
                4 => FrameHorizontalAlignment.Inside,
                5 => FrameHorizontalAlignment.Outside,
                _ => FrameHorizontalAlignment.Offset,
            },
            HorizontalOffset = Length.FromTwips(anchor.Left),
            VerticalOrigin = vertical switch
            {
                Ww8ShapeOrigin.PageMargin => FrameVerticalOrigin.PageMargin,
                Ww8ShapeOrigin.Page => FrameVerticalOrigin.Page,
                Ww8ShapeOrigin.Character => FrameVerticalOrigin.Line,
                _ => FrameVerticalOrigin.Paragraph,
            },
            VerticalAlignment = host.Value(EscherPropertyIds.VerticalPosition) switch
            {
                1 or 4 => FrameVerticalAlignment.Top,
                2 => FrameVerticalAlignment.Middle,
                3 or 5 => FrameVerticalAlignment.Bottom,
                _ => FrameVerticalAlignment.Offset,
            },
            VerticalOffset = Length.FromTwips(anchor.Top),
            Spacing = WrapSpacing(properties, lineWidth),
            Padding = TextInsets(properties),
            Fill = properties.Boolean(EscherPropertyIds.Filled, fallback: true)
                ? Colour(properties.Value(EscherPropertyIds.FillColour, 0x00FFFFFF))
                : null,
            BorderColour = lined ? Colour(properties.Value(EscherPropertyIds.LineColour)) : null,
            BorderWidth = lineWidth,
            IsImage = blocks.Count == 0,
            Blocks = blocks,
            Name = shape?.Name,
        };
    }

    /// <summary>Which text-box story holds a shape's own text, or -1 when it has none.</summary>
    /// <remarks>
    /// The <c>lTxid</c> property packs two numbers: the story's <strong>one-based</strong> index in
    /// the text-box PLCF in its high half, and which link of a chained text box this shape is in its
    /// low half (<c>ww8par.cxx:997</c>, and <c>GetTxbxTextSttEndCp</c> does the
    /// <c>SetIdx(nTxBxS - 1)</c>). Reading the whole word as an index finds no story at all, and
    /// reading the high half as though it were zero-based finds the previous shape's.
    /// </remarks>
    public static int TextStoryIndex(EscherShape? shape)
    {
        uint id = shape?.Properties.Value(EscherPropertyIds.TextId) ?? 0;
        return id == 0 ? -1 : (int)((id >> 16) & 0xFFFF) - 1;
    }

    /// <summary>The origin a property states, falling back on the one the <c>FSPA</c> states.</summary>
    private static Ww8ShapeOrigin Origin(
        EscherPropertyTable host, ushort property, Ww8ShapeOrigin fallback)
    {
        if (!host.Has(property)) return fallback;

        uint stated = host.Value(property);

        // Anything outside the four defined values is read as the page, which is what
        // ProcessEscherAlign's `nCntRelTo > value ? value : 1` comes to.
        return stated <= (uint)Ww8ShapeOrigin.Character
            ? (Ww8ShapeOrigin)stated
            : Ww8ShapeOrigin.Page;
    }

    /// <summary>
    /// The wrap the <c>FSPA</c>'s two fields ask for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>nwr</c> says what kind of hole the text leaves and <c>nwrk</c> which side of it the text
    /// may use, and only two of <c>nwr</c>'s six values consult the second — which is the detail that
    /// makes a naive reading wrong, because a shape whose <c>nwr</c> is 1 or 3 usually carries a
    /// stale <c>nwrk</c> that would then decide the wrap. LibreOffice guards it with the same
    /// explicit <c>(2 == nwr) || (4 == nwr)</c> (<c>ww8graf.cxx:2729</c>).
    /// </para>
    /// <para>
    /// The two tight modes, 4 and 5, ask for a contour wrap round the shape's own outline. They are
    /// read as the square wrap their <c>nwrk</c> names, which is the same hole with straight sides —
    /// the same approximation the DOCX reader makes for <c>wp:wrapTight</c>.
    /// </para>
    /// </remarks>
    private static TextWrap WrapOf(Ww8ShapeAnchor anchor) => anchor.Wrap switch
    {
        1 => TextWrap.TopAndBottom,
        3 => TextWrap.Through,
        2 or 4 => anchor.WrapSide switch
        {
            1 => TextWrap.Left,
            2 => TextWrap.Right,
            3 => TextWrap.Optimal,
            _ => TextWrap.Both,
        },
        _ => TextWrap.Both,
    };

    /// <summary>
    /// How far text must stay clear of the shape on each side.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The four <c>dxWrapDist</c> properties, plus <em>half the shape's line width</em>. The second
    /// term is not decoration: Word states a shape's rectangle as the path its outline runs along,
    /// while what Writer keeps text away from is the object's bounding rectangle, which the stroke
    /// straddles. Measured on <c>frame-wrap.doc</c>, whose shape is 2267 twips wide at the column's
    /// left edge with a 15-twip line and no stated distances: LibreOffice draws its right edge at
    /// 170.05 pt — exactly 3401 twips, the shape's own coordinate — and resumes the body text at
    /// 179.55 pt, which is 3591 twips. The gap is 190, and 181 + 7 + the 2 twips that the same
    /// document's ODF form also shows account for it; leaving the line width out lands the wrapped
    /// lines 0.35 pt short.
    /// </para>
    /// <para>
    /// In <see cref="PageFrame.Spacing"/> rather than folded into the rectangle, because the shape's
    /// own text and its border are drawn at the coordinates the file states — the frame does not
    /// move, the hole in the text merely gets bigger.
    /// </para>
    /// </remarks>
    private static Margins WrapSpacing(EscherPropertyTable properties, Length lineWidth)
    {
        Length half = Length.FromTwips(lineWidth.Twips / 2);

        return new Margins(
            Distance(EscherPropertyIds.WrapDistanceLeft, DefaultWrapDistanceEmu) + half,
            Distance(EscherPropertyIds.WrapDistanceTop, 0) + half,
            Distance(EscherPropertyIds.WrapDistanceRight, DefaultWrapDistanceEmu) + half,
            Distance(EscherPropertyIds.WrapDistanceBottom, 0) + half);

        Length Distance(ushort property, uint fallback)
            => Twips(properties.Value(property, fallback));
    }

    /// <summary>The inset between the shape's edges and its own text.</summary>
    private static Margins TextInsets(EscherPropertyTable properties)
        => new(
            Twips(properties.Value(EscherPropertyIds.TextInsetLeft, DefaultTextInsetHorizontalEmu)),
            Twips(properties.Value(EscherPropertyIds.TextInsetTop, DefaultTextInsetVerticalEmu)),
            Twips(properties.Value(EscherPropertyIds.TextInsetRight, DefaultTextInsetHorizontalEmu)),
            Twips(properties.Value(EscherPropertyIds.TextInsetBottom, DefaultTextInsetVerticalEmu)));

    /// <summary>
    /// One EMU measurement of the drawing layer's, on Writer's whole-twip grid.
    /// </summary>
    /// <remarks>
    /// Signed, because a stated distance can be negative and the property table reports the raw
    /// thirty-two bits; and rounded to whole twips for the reason the DOCX reader gives — a frame
    /// edge half a twip out is what decides whether a line touching it is narrowed at all.
    /// </remarks>
    private static Length Twips(uint value)
        => Length.FromTwips(Length.FromEmu(unchecked((int)value)).Twips);

    /// <summary>
    /// One <c>MSO_CLR</c> as a colour.
    /// </summary>
    /// <remarks>
    /// The channel order is the drawing layer's own and is the reverse of the one every XML format
    /// uses: the low byte is red and the third is blue, so the corpus document's <c>0x00A46534</c> is
    /// <c>#3465A4</c> and not the near-black it would be read as the other way round. The top byte is
    /// a type tag — a palette or scheme index rather than a literal colour — and only type 0 is a
    /// plain value; the rest are read as their literal bytes, which is wrong but visible, until the
    /// scheme lookup exists.
    /// </remarks>
    private static Colour Colour(uint value)
        => new((byte)(value & 0xFF), (byte)((value >> 8) & 0xFF), (byte)((value >> 16) & 0xFF));
}
