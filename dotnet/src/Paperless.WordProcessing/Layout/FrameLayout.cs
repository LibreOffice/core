using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// Resolves a floating frame's anchor and origin into a rectangle on a page.
/// </summary>
/// <remarks>
/// <para>
/// Two independent choices per axis, which is why a frame's position takes four fields rather than a
/// point: an <em>origin</em> — the page, the text area, the column, the anchor paragraph — and an
/// <em>alignment</em> inside it, which is either a stated distance or one of the edges. All four formats
/// state both, and none of them states a plain coordinate: ODF pairs <c>style:horizontal-rel</c> with
/// <c>style:horizontal-pos</c>, OOXML pairs <c>wp:positionH/@relativeFrom</c> with either
/// <c>wp:posOffset</c> or <c>wp:align</c>.
/// </para>
/// <para>
/// The paragraph origins are the ones that make placement a two-pass affair: where the anchor paragraph
/// sits is a result of pagination, and what the frame does to the text is an input to it. See
/// <see cref="Paginator"/> for how that loop is closed.
/// </para>
/// </remarks>
public static class FrameLayout
{
    /// <summary>
    /// Where a frame goes on a page.
    /// </summary>
    /// <param name="frame">The frame.</param>
    /// <param name="geometry">The page it is on.</param>
    /// <param name="column">The rectangle of the column its anchor is in.</param>
    /// <param name="anchorTop">
    /// The top of the anchor paragraph, in page coordinates. Ignored for the origins that do not depend
    /// on it, which is what makes a page-anchored frame placeable before anything has been paginated.
    /// </param>
    /// <param name="anchorLineTop">
    /// The top of the anchoring <em>line</em>, which differs from <paramref name="anchorTop"/> by the
    /// paragraph's own upper spacing — see <see cref="PlacedLine.ParagraphTop"/>. Only
    /// <see cref="FrameVerticalOrigin.Line"/> uses it; it defaults to the paragraph's top, which is what
    /// the two are on a paragraph whose gap above collapsed to nothing.
    /// </param>
    /// <param name="rightHandPage">
    /// True when the page is a right-hand one, which is the only thing the inside and outside alignments
    /// differ by.
    /// </param>
    public static DocRect Place(
        PageFrame frame,
        PageGeometry geometry,
        DocRect column,
        Length anchorTop,
        bool rightHandPage = true,
        Length? anchorLineTop = null)
    {
        ArgumentNullException.ThrowIfNull(frame);
        ArgumentNullException.ThrowIfNull(geometry);

        DocRect page = new(Length.Zero, Length.Zero, geometry.Size.Width, geometry.Size.Height);
        DocRect text = geometry.TextArea;

        DocRect horizontal = frame.HorizontalOrigin switch
        {
            FrameHorizontalOrigin.Page => page,
            FrameHorizontalOrigin.PageMargin => text,
            FrameHorizontalOrigin.Column => column,
            _ => column,
        };

        Length lineTop = anchorLineTop ?? anchorTop;

        DocRect vertical = frame.VerticalOrigin switch
        {
            FrameVerticalOrigin.Page => page,
            FrameVerticalOrigin.PageMargin => text,
            FrameVerticalOrigin.Line =>
                new DocRect(column.X, lineTop, column.Width, text.Bottom - lineTop),
            _ => new DocRect(column.X, anchorTop, column.Width, text.Bottom - anchorTop),
        };

        bool towardsBinding = frame.HorizontalAlignment == FrameHorizontalAlignment.Inside == rightHandPage;

        // A member of a shape group is aligned by its *group* and then offset within it — see
        // PageFrame.GroupSize. Aligning it by its own size instead would centre each member of a
        // centred group separately, which stacks a letterhead's boxes on top of one another.
        DocSize aligned = frame.GroupSize ?? frame.Size;

        Length x = frame.HorizontalAlignment switch
        {
            FrameHorizontalAlignment.Left => horizontal.X,
            FrameHorizontalAlignment.Centre =>
                horizontal.X + ((horizontal.Width - aligned.Width) / 2),
            FrameHorizontalAlignment.Right => horizontal.Right - aligned.Width,
            FrameHorizontalAlignment.Inside or FrameHorizontalAlignment.Outside =>
                towardsBinding ? horizontal.X : horizontal.Right - aligned.Width,
            _ => horizontal.X + frame.HorizontalOffset,
        };

        Length y = frame.VerticalAlignment switch
        {
            FrameVerticalAlignment.Top => vertical.Y,
            FrameVerticalAlignment.Middle => vertical.Y + ((vertical.Height - aligned.Height) / 2),
            FrameVerticalAlignment.Bottom => vertical.Bottom - aligned.Height,
            _ => vertical.Y + frame.VerticalOffset,
        };

        return new DocRect(
            x + frame.GroupOffset.X, y + frame.GroupOffset.Y, frame.Size.Width, frame.Size.Height);
    }
}

/// <summary>
/// One pass's answer to where a document's frames are, and what they do to the text around them.
/// </summary>
/// <remarks>
/// <para>
/// Built from a set of laid-out pages and fed back into the next pagination of the same blocks. The two
/// halves it carries are the two directions the circularity runs in: <see cref="AttachedTo"/> is where
/// the frames go, and <see cref="ObstaclesFor"/> is what that costs the text — and each is derived from
/// where the anchor paragraphs landed <em>last</em> time, which is the only sequence in which the
/// dependency can be broken.
/// </para>
/// <para>
/// A block's position is taken from its <em>first</em> line, which is what an anchor means: a paragraph
/// split across a page break anchors its frames on the page it starts on, and its continuation flows
/// round whatever is on the page it ends on. Writer instead moves the anchor with the follow frame in
/// some cases; the difference shows only for a frame anchored to a paragraph that straddles a break.
/// </para>
/// </remarks>
internal sealed class FrameResolution
{
    private readonly Dictionary<int, List<PlacedFrame>> _byPage;
    private readonly Dictionary<int, FrameObstacles> _byBlock;
    private readonly List<long> _signature;
    private readonly int _frames;

    private FrameResolution(
        Dictionary<int, List<PlacedFrame>> byPage,
        Dictionary<int, FrameObstacles> byBlock,
        List<long> signature,
        int frames)
    {
        _byPage = byPage;
        _byBlock = byBlock;
        _signature = signature;
        _frames = frames;
    }

    /// <summary>True when the document has no frame that has been placed anywhere.</summary>
    public bool IsEmpty => _frames == 0;

    /// <summary>
    /// Resolves every frame against a pagination.
    /// </summary>
    /// <param name="blocks">The document's blocks, whose paragraphs carry the frames.</param>
    /// <param name="sections">The sections, for the page geometry a frame's origins are measured in.</param>
    /// <param name="pages">Where the blocks landed.</param>
    /// <param name="collapsesSpacing">
    /// Whether the paragraphs inside a frame collapse their spacing against one another rather than adding
    /// it — see <see cref="FlowLayouter.LayOut"/>. A frame's own text is a frame's text like a cell's.
    /// </param>
    public static FrameResolution Of(
        IReadOnlyList<PageBlock> blocks,
        IReadOnlyList<PaginatedSection> sections,
        IReadOnlyList<LaidOutPage> pages,
        bool collapsesSpacing = false)
    {
        Dictionary<int, Placement> placements = [];

        for (int index = 0; index < pages.Count; index++)
        {
            LaidOutPage page = pages[index];

            // Only the page a block *starts* on, since that is where its anchor is. The first line wins
            // because the lines are in order, and a page later in the list cannot hold an earlier start.
            foreach (PlacedLine line in page.Lines)
            {
                if (!line.StartsParagraph) continue;
                if (placements.ContainsKey(line.ParagraphIndex)) continue;

                placements[line.ParagraphIndex] = new Placement(
                    index,
                    page,
                    page.ColumnArea(line.Column),
                    page.BodyArea.Y + line.Top,
                    page.BodyArea.Y + line.ParagraphTop);
            }
        }

        Dictionary<int, List<PlacedFrame>> byPage = [];
        Dictionary<int, List<WrapObstacle>> obstaclesByPage = [];
        List<long> signature = [];
        int frames = 0;

        // One paragraph's frames, given the rectangle its origins are measured in and where its own top
        // ended up. Shared by the body's blocks and by the flows, which differ only in how those two are
        // arrived at — the body's from a placed line, a flow's from the flow's own area.
        void PlaceFrames(
            PageParagraph paragraph,
            int pageIndex,
            LaidOutPage page,
            DocRect origin,
            Length anchorTop,
            Length anchorLineTop)
        {
            PageGeometry geometry = sections[
                Math.Clamp(page.SectionIndex, 0, sections.Count - 1)].Section.Page;

            foreach (PageFrame frame in paragraph.Frames)
            {
                // An as-character frame is not placed against an origin at all — it hangs on a line, at
                // the position its anchor character occupies. That needs the line rather than the
                // paragraph, so it is done by the walk below.
                if (frame.Anchor == FrameAnchor.AsCharacter) continue;

                DocRect area = FrameLayout.Place(
                    frame,
                    geometry,
                    origin,
                    anchorTop,
                    rightHandPage: page.Number % 2 == 1,
                    anchorLineTop: anchorLineTop);

                frames++;
                signature.Add(area.X.Emu);
                signature.Add(area.Y.Emu);
                signature.Add(area.Width.Emu);
                signature.Add(area.Height.Emu);

                if (!byPage.TryGetValue(pageIndex, out List<PlacedFrame>? placed))
                {
                    byPage[pageIndex] = placed = [];
                }

                placed.Add(new PlacedFrame(frame, area, Content(frame, area, collapsesSpacing)));

                // A run-through frame is not an obstacle at all: it neither narrows a line nor pushes one
                // down, which is exactly what `SwTextFly::ForEach` skips it for.
                if (frame.Wrap == TextWrap.Through) continue;

                if (!obstaclesByPage.TryGetValue(pageIndex, out List<WrapObstacle>? list))
                {
                    obstaclesByPage[pageIndex] = list = [];
                }

                list.Add(new WrapObstacle(Widened(area, frame.Spacing), frame.Wrap));
            }
        }

        // One line's as-character frames, hung on the line rather than placed against an origin.
        //
        // Writer's `SwFlyCntPortion`: an as-character frame is a portion of the line's own text, so it
        // sits where the character it replaces sits and its bottom rests on the baseline. Measured
        // against LibreOffice's PDF of `picture-anchor.fodt`, whose first line's baseline is at 756.839
        // and whose picture's bottom edge is at 756.889 — one twip below it, which is the same inclusive
        // rectangle that gives `FrameObstacles` its inflation. The horizontal position is 183.35, which
        // is the text's own 56.8 plus the width of "An inline picture follows: " and nothing else.
        void HangInline(
            PageParagraph paragraph, int pageIndex, DocRect area, PlacedLine line)
        {
            foreach (PageFrame frame in paragraph.Frames)
            {
                if (frame.Anchor != FrameAnchor.AsCharacter) continue;
                if (frame.AnchorOffset < line.Box.Line.Start) continue;

                // One past the last character belongs to the *next* line, except on the last line of
                // the paragraph, where there is no next one. That is not an edge case worth skipping:
                // RTF appends nothing to the text for a picture, so a paragraph holding only a picture
                // is an empty paragraph with an anchor at offset zero and a line running 0 to 0 — the
                // most ordinary way for a document to carry a logo.
                if (frame.AnchorOffset >= line.Box.Line.End && !line.Box.Line.EndsParagraph) continue;
                if (frame.AnchorOffset > line.Box.Line.End) continue;

                // Its top is the baseline less however much of it stands above one: the whole height for an
                // ordinary inline picture, and nought for a shape that hangs below the line instead.
                DocRect placedAt = new(
                    area.X + line.Box.Left + PageDrawing.OffsetOnLine(paragraph, line, frame.AnchorOffset),
                    area.Y + line.Baseline - (frame.InlineAscent ?? frame.Size.Height),
                    frame.Size.Width,
                    frame.Size.Height);

                frames++;
                signature.Add(placedAt.X.Emu);
                signature.Add(placedAt.Y.Emu);
                signature.Add(placedAt.Width.Emu);
                signature.Add(placedAt.Height.Emu);

                if (!byPage.TryGetValue(pageIndex, out List<PlacedFrame>? placed))
                {
                    byPage[pageIndex] = placed = [];
                }

                // Never an obstacle, whatever the document says its wrap is. An as-character frame is
                // part of the text rather than something the text flows around, which is why Writer
                // gives it a portion and not an entry in `SwTextFly`'s object list.
                placed.Add(new PlacedFrame(frame, placedAt, Content(frame, placedAt, collapsesSpacing)));
            }
        }

        for (int index = 0; index < blocks.Count; index++)
        {
            if (blocks[index] is not PageParagraph paragraph || paragraph.Frames.Count == 0) continue;
            if (!placements.TryGetValue(index, out Placement placement)) continue;

            PlaceFrames(
                paragraph,
                placement.Index,
                placement.Page,
                placement.Column,
                placement.ParagraphTop,
                placement.Top);
        }

        for (int index = 0; index < pages.Count; index++)
        {
            LaidOutPage page = pages[index];
            IReadOnlyList<PageBlock> own = page.Blocks ?? blocks;

            foreach (PlacedLine line in page.Lines)
            {
                if (line.ParagraphIndex < 0 || line.ParagraphIndex >= own.Count) continue;
                if (own[line.ParagraphIndex] is not PageParagraph paragraph) continue;
                if (paragraph.Frames.Count == 0) continue;

                HangInline(paragraph, index, page.ColumnArea(line.Column), line);
            }
        }

        // And the frames anchored in a flow rather than in the body: a table cell, a header, a footer, a
        // footnote. Writer places all of these the same way, because an anchored object belongs to the
        // *page* however deeply its anchor is nested — which is visible in LibreOffice's own rendering of
        // `frame-in-header.fodt`, where a frame anchored in the running head hangs past the header area
        // and the first four body lines divide round it. So a flow's frames go on the page's list beside
        // the body's and become obstacles for the body's text, exactly as a body frame does.
        for (int index = 0; index < pages.Count; index++)
        {
            foreach (PlacedFlow flow in FlowsOn(pages[index]))
            {
                foreach (PlacedLine line in flow.Lines)
                {
                    if (line.ParagraphIndex < 0 || line.ParagraphIndex >= flow.Blocks.Count) continue;
                    if (flow.Blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;
                    if (paragraph.Frames.Count == 0) continue;

                    // An inline picture in a header or a cell hangs on whichever of the flow's lines holds
                    // its anchor, which need not be the paragraph's first — unlike a floating frame, whose
                    // origin is the paragraph and which is therefore placed only once.
                    HangInline(paragraph, index, flow.Area, line);

                    if (!line.StartsParagraph) continue;

                    // The flow's own rectangle is what a paragraph-, column- or text-area-relative origin
                    // resolves against inside a flow, and it is exact: LibreOffice draws the frame of
                    // `frame-in-cell.fodt` at 73.70 pt, which is the table's left edge plus the cell's
                    // 0.1 cm padding plus the frame's own 0.5 cm offset and nothing else.
                    PlaceFrames(
                        paragraph,
                        index,
                        pages[index],
                        flow.Area,
                        flow.Area.Y + line.ParagraphTop,
                        flow.Area.Y + line.Top);
                }
            }
        }

        Dictionary<int, FrameObstacles> byBlock = [];

        // In block order, because the signature is compared position by position and a dictionary's
        // enumeration order is not a contract.
        foreach (int index in placements.Keys.Order())
        {
            Placement placement = placements[index];
            if (!obstaclesByPage.TryGetValue(placement.Index, out List<WrapObstacle>? list)) continue;

            byBlock[index] = new FrameObstacles(list, placement.Top, placement.Column.X);

            // The obstructed blocks' own positions belong in the signature as well as the frames', and
            // that is not obvious: the frames can settle while the text has not. A paragraph *below* one
            // that grew by wrapping starts lower than the pass before believed, and its lines were
            // measured against the frame at the height it used to be — so comparing only the rectangles
            // declares victory with the paragraph after the frame still wrapped round nothing.
            signature.Add(index);
            signature.Add(placement.Index);
            signature.Add(placement.Top.Emu);
        }

        return new FrameResolution(byPage, byBlock, signature, frames);
    }

    /// <summary>The obstacles one block's lines must flow around, or null when it has none.</summary>
    public ILineObstacles? ObstaclesFor(int block)
        => _byBlock.TryGetValue(block, out FrameObstacles? obstacles) ? obstacles : null;

    /// <summary>The same pages with their frames attached.</summary>
    public List<LaidOutPage> AttachedTo(List<LaidOutPage> pages)
    {
        ArgumentNullException.ThrowIfNull(pages);

        for (int index = 0; index < pages.Count; index++)
        {
            if (_byPage.TryGetValue(index, out List<PlacedFrame>? frames))
            {
                pages[index] = pages[index] with { Frames = frames };
            }
        }

        return pages;
    }

    /// <summary>True when the frames came out in exactly the same places as another pass's.</summary>
    public bool SameAs(FrameResolution other)
    {
        ArgumentNullException.ThrowIfNull(other);
        return _signature.SequenceEqual(other._signature);
    }

    /// <summary>
    /// Every flow on a page whose paragraphs may anchor a frame: the furniture, the notes, and the cells
    /// of every table, however deeply the tables nest.
    /// </summary>
    /// <remarks>
    /// A flow is a flow — <see cref="PlacedFlow"/> is one type for a header, a footer, a cell and a
    /// footnote — so there is one walk rather than four. What is deliberately <em>not</em> walked is a
    /// placed frame's own content: a frame anchored inside another frame would need the outer one's
    /// rectangle before the inner one could be placed, and the outer one is being placed by this loop.
    /// No format in the corpus writes one and Writer treats it as anchored to the page.
    /// </remarks>
    private static IEnumerable<PlacedFlow> FlowsOn(LaidOutPage page)
    {
        if (page.Header is { } header) yield return header;
        if (page.Footer is { } footer) yield return footer;
        if (page.Notes is { } notes) yield return notes;

        foreach (PlacedTable table in page.Tables)
        {
            foreach (PlacedFlow flow in CellFlows(table, 0)) yield return flow;
        }
    }

    /// <summary>The flows of a table's cells, and of any table inside one of them.</summary>
    /// <param name="table">The table.</param>
    /// <param name="nesting">How many tables enclose it, bounded as <see cref="FlowLayouter"/> bounds it.</param>
    private static IEnumerable<PlacedFlow> CellFlows(PlacedTable table, int nesting)
    {
        if (nesting >= FlowLayouter.MaxNesting) yield break;

        foreach (PlacedTableCell cell in table.Cells)
        {
            if (cell.Content is not { } content) continue;

            yield return content;

            foreach (PlacedTable inner in content.Tables)
            {
                foreach (PlacedFlow flow in CellFlows(inner, nesting + 1)) yield return flow;
            }
        }
    }

    /// <summary>The rectangle text has to keep clear of, which is the frame's plus its wrap spacing.</summary>
    /// <remarks>
    /// Writer's <c>SwAnchoredObject::GetObjRectWithSpaces</c>. Measured: a frame given
    /// <c>fo:margin-top="0.5cm"</c> starts narrowing lines one line earlier than the same frame with no
    /// margin, and one 11 pt line is 269 twips against the margin's 283 — so the spacing widens the hole
    /// rather than moving the frame, which stays where its border is drawn.
    /// </remarks>
    private static DocRect Widened(DocRect area, Margins spacing)
        => new(
            area.X - spacing.Left,
            area.Y - spacing.Top,
            area.Width + spacing.Left + spacing.Right,
            area.Height + spacing.Top + spacing.Bottom);

    /// <summary>A text frame's own content, laid out inside it.</summary>
    /// <remarks>
    /// Through <see cref="FlowLayouter"/>, which is what makes a frame's paragraphs behave like a header's
    /// or a cell's without a second layout path — a frame containing a table works because a cell already
    /// does. An image frame has no blocks and gets null, since the raster is decoded elsewhere and the
    /// rectangle is all the wrap ever needed.
    /// </remarks>
    private static PlacedFlow? Content(PageFrame frame, DocRect area, bool collapsesSpacing)
        => frame.Blocks.Count == 0
            ? null
            : FlowLayouter.LayOut(
                frame.Blocks,
                area.Deflate(frame.Padding),
                Length.Zero,
                collapsesSpacing: collapsesSpacing);

    /// <param name="Index">Which page the block starts on.</param>
    /// <param name="Page">That page.</param>
    /// <param name="Column">The rectangle of the column it starts in.</param>
    /// <param name="Top">Where its first line's box top sits, in page coordinates.</param>
    /// <param name="ParagraphTop">
    /// Where a frame anchored to the paragraph measures its offset from: the same point less the
    /// paragraph's own upper spacing. See <see cref="PlacedLine.ParagraphTop"/>.
    /// </param>
    private readonly record struct Placement(
        int Index, LaidOutPage Page, DocRect Column, Length Top, Length ParagraphTop);
}

/// <summary>
/// One frame as the line filler sees it: the rectangle text must avoid, and which side it may pass on.
/// </summary>
/// <param name="Area">The rectangle, spacing already added.</param>
/// <param name="Wrap">Which side text may pass on.</param>
internal readonly record struct WrapObstacle(DocRect Area, TextWrap Wrap);

/// <summary>
/// What a page's frames do to one paragraph's lines: the port of Writer's <c>SwTextFly</c>.
/// </summary>
/// <remarks>
/// <para>
/// The question a line filler asks is "given the stretch I wanted at this height, what do I get", and
/// Writer answers it in <c>SwTextFormatter::CalcFlyWidth</c> by building a rectangle for the line,
/// intersecting it against the anchored objects, and either shortening the line or pushing its start
/// along. This does the same thing over a list of rectangles resolved once per page.
/// </para>
/// <para>
/// Three rules are ported deliberately, and each was checked against LibreOffice's own rendering of a
/// 4 × 3 cm frame on an A4 page with 2 cm margins:
/// </para>
/// <list type="bullet">
/// <item><description>
/// <strong>The wrap side widens the rectangle rather than choosing between two of them.</strong>
/// <c>SwTextFly::AnchoredObjToRect</c> calls <c>CalcRightMargin</c> for a left wrap and
/// <c>CalcLeftMargin</c> for a right wrap, so "text on the left only" is expressed as a frame that
/// reaches the end margin. Top-and-bottom does both, which is what leaves the line with nowhere to go.
/// </description></item>
/// <item><description>
/// <strong>A line whose box merely touches the frame's top edge is already affected.</strong>
/// Measured: a frame anchored at the top of the second paragraph narrows the <em>last line of the
/// first</em>, whose box bottom is exactly the frame's top. Writer's rectangles are inclusive
/// (<c>SwRect::Bottom() == Top() + Height() - 1</c>) and the arithmetic around the fly portion adds a
/// twip back, so the effective rectangle is one twip larger than its geometry on every side — which is
/// also why text resumes 3402 rather than 3401 twips along from a frame 2268 twips wide at 1134.
/// </description></item>
/// <item><description>
/// <strong>The optimal wrap is decided per frame from the room on each side</strong>, per
/// <c>SwTextFly::GetSurroundForTextWrap</c>: a side with less than 2 cm of room is not wrapped on at
/// all, and a frame wider than 1.5 cm gets only its roomier side.
/// </description></item>
/// </list>
/// </remarks>
internal sealed class FrameObstacles : ILineObstacles
{
    /// <summary>
    /// How much larger than its geometry a frame's hole in the text is, on every side.
    /// </summary>
    /// <remarks>
    /// One twip, and it is not a fudge: Writer's rectangles are inclusive, so a fly at twip 1941 has
    /// <c>Top() == 1941</c> and the line above it has <c>Bottom() == 1940</c>, and yet the two are treated
    /// as meeting. Measured at both edges — the frame two paragraphs down narrows the line whose box ends
    /// exactly where it begins, and text after a frame resumes one twip past its right edge.
    /// </remarks>
    private static readonly Length Inflation = Length.FromTwips(1);

    /// <summary>Writer's <c>TEXT_MIN</c>: wrap only on a side with at least 2 cm for the text.</summary>
    private static readonly Length MinimumTextSide = Length.FromTwips(1134);

    /// <summary>Writer's <c>FRAME_MAX</c>: wrap on both sides only up to a frame width of 1.5 cm.</summary>
    private static readonly Length MaximumBothSides = Length.FromTwips(850);

    /// <summary>How many times a line may be pushed down before it is left where it is.</summary>
    /// <remarks>
    /// A guard rather than a rule. Each descent moves the line past one frame's bottom, so the loop can
    /// only run once per frame; the bound stops a frame with a degenerate rectangle from spinning.
    /// </remarks>
    private const int MaxDescents = 64;

    private readonly List<WrapObstacle> _obstacles;
    private readonly Length _origin;
    private readonly Length _left;

    /// <summary>Creates the obstacles one paragraph sees.</summary>
    /// <param name="obstacles">The page's frames, in page coordinates.</param>
    /// <param name="paragraphTop">Where the paragraph's own top sits on the page.</param>
    /// <param name="areaLeft">The left edge the paragraph's indents are measured from.</param>
    public FrameObstacles(List<WrapObstacle> obstacles, Length paragraphTop, Length areaLeft)
    {
        ArgumentNullException.ThrowIfNull(obstacles);
        _obstacles = obstacles;
        _origin = paragraphTop;
        _left = areaLeft;
    }

    /// <inheritdoc/>
    public bool IsEmpty => _obstacles.Count == 0;

    /// <inheritdoc/>
    public LineSpace SpaceFor(Length top, Length height, LineSpace wanted)
    {
        if (_obstacles.Count == 0) return wanted;

        Length wantedLeft = _left + wanted.Left;
        Length wantedRight = wantedLeft + wanted.Width;
        Length lineTop = _origin + top;
        Length descent = Length.Zero;

        for (int attempt = 0; attempt < MaxDescents; attempt++)
        {
            Length lineBottom = lineTop + height;
            Length coveredTo = wantedLeft;
            Length endsAt = wantedRight;
            Length blockedTo = Length.Zero;
            bool blocked = false;

            // Left to right through the covered stretches, exactly as Writer walks its sorted object
            // list. Two different things can happen and the difference is which side of the line the
            // frame is on: one that starts at or before the line's own start pushes the start along,
            // and one that starts after it ends the line early. Treating the second as "no obstacle"
            // is the mistake that leaves a frame at the end margin with text running under it — the
            // one wrap side a frame at the start margin never exercises.
            foreach (WrapObstacle obstacle in Sorted())
            {
                DocRect area = Widen(obstacle, wantedLeft, wantedRight);

                if (area.Bottom <= lineTop || area.Top >= lineBottom) continue;
                if (area.Right <= coveredTo || area.Left >= wantedRight) continue;

                if (area.Left > coveredTo)
                {
                    endsAt = area.Left;
                    blockedTo = Length.Max(blockedTo, obstacle.Area.Bottom + Inflation);
                    blocked = true;
                    break;
                }

                coveredTo = area.Right;
                blockedTo = Length.Max(blockedTo, obstacle.Area.Bottom + Inflation);
                blocked = true;
            }

            if (coveredTo < endsAt)
            {
                return new LineSpace(coveredTo - _left, endsAt - coveredTo, descent);
            }

            // Nothing left of the line: it drops below whichever frame closed it and tries again, which
            // is Writer's dummy line — `SwFlyPortion` with `bFullLine`, whose height is the frame's.
            if (!blocked || blockedTo <= lineTop) break;

            descent += blockedTo - lineTop;
            lineTop = blockedTo;
        }

        return new LineSpace(wanted.Left, wanted.Width, descent);
    }

    /// <summary>The obstacles in left-to-right order, which is the order the walk depends on.</summary>
    private IEnumerable<WrapObstacle> Sorted()
        => _obstacles.OrderBy(obstacle => obstacle.Area.X.Emu);

    /// <summary>
    /// The rectangle a frame really takes out of a line, once its wrap side has widened it.
    /// </summary>
    /// <remarks>
    /// The port of <c>SwTextFly::CalcLeftMargin</c> and <c>CalcRightMargin</c>: "LEFT means that the text
    /// must flow on the left of the frame, that is the frame expands to the right edge of the print area
    /// or to the next frame". Widening to the <em>next frame</em> is not done here — this widens to the
    /// margin, which is the same thing whenever a line has one frame on it, and every corpus case does.
    /// </remarks>
    private static DocRect Widen(WrapObstacle obstacle, Length wantedLeft, Length wantedRight)
    {
        DocRect area = new(
            obstacle.Area.X - Inflation,
            obstacle.Area.Y - Inflation,
            obstacle.Area.Width + (Inflation * 2),
            obstacle.Area.Height + (Inflation * 2));

        TextWrap wrap = obstacle.Wrap == TextWrap.Optimal
            ? Resolve(area, wantedLeft, wantedRight)
            : obstacle.Wrap;

        Length left = wrap is TextWrap.Right or TextWrap.TopAndBottom
            ? Length.Min(area.X, wantedLeft)
            : area.X;

        Length right = wrap is TextWrap.Left or TextWrap.TopAndBottom
            ? Length.Max(area.Right, wantedRight)
            : area.Right;

        return new DocRect(left, area.Y, right - left, area.Height);
    }

    /// <summary>
    /// Which side an "optimal" wrap comes out on, per <c>SwTextFly::GetSurroundForTextWrap</c>.
    /// </summary>
    /// <remarks>
    /// The rule reads oddly and is worth stating in full, because guessing "the wider side" gets two of
    /// the four cases wrong. A frame clear of the measure altogether wraps on both sides. Otherwise the
    /// room on each side is measured; a frame wider than 1.5 cm loses its narrower side outright; a side
    /// with less than 2 cm left is dropped as unusable; and if that leaves nothing on either side the
    /// answer is both sides rather than neither, because Writer prefers the mode that gives the same
    /// result on a re-layout.
    /// </remarks>
    private static TextWrap Resolve(DocRect area, Length wantedLeft, Length wantedRight)
    {
        if (area.Right < wantedLeft || area.X > wantedRight) return TextWrap.Both;

        Length left = area.X - wantedLeft;
        Length right = wantedRight - area.Right;

        if (area.Width > MaximumBothSides)
        {
            if (left < right) left = Length.Zero;
            else right = Length.Zero;
        }

        if (left <= Length.Zero && right <= Length.Zero) return TextWrap.Both;

        if (left < MinimumTextSide) left = Length.Zero;
        if (right < MinimumTextSide) right = Length.Zero;

        if (left > Length.Zero) return right > Length.Zero ? TextWrap.Both : TextWrap.Left;
        return right > Length.Zero ? TextWrap.Right : TextWrap.TopAndBottom;
    }
}
