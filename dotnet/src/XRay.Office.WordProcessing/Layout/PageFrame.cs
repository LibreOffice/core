using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Vector;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// What a floating frame is fixed to, which decides what moves it.
/// </summary>
/// <remarks>
/// The distinction is not decoration: a page-anchored frame stays where it is however the text reflows,
/// and a paragraph-anchored one follows its paragraph onto the next page. Writer's
/// <c>RndStdIds::FLY_AT_PAGE</c>, <c>FLY_AT_PARA</c>, <c>FLY_AT_CHAR</c> and <c>FLY_AS_CHAR</c>.
/// </remarks>
public enum FrameAnchor
{
    /// <summary>To a paragraph: the frame sits beside it and moves with it.</summary>
    Paragraph,

    /// <summary>To a character position, which is a finer origin for the same behaviour.</summary>
    Character,

    /// <summary>In the text, as one very large character on a line of its own making.</summary>
    AsCharacter,

    /// <summary>To the page, so reflowing the text does not move it.</summary>
    Page,
}

/// <summary>
/// How body text behaves where a frame is in its way.
/// </summary>
/// <remarks>
/// <para>
/// The names are Writer's <c>css::text::WrapTextMode</c> rather than any one format's, because the four
/// formats spell the same six things differently and one of the spellings is actively misleading: ODF's
/// <c>style:wrap="none"</c> does <em>not</em> mean "no wrapping" — it means no text beside the frame at
/// all, so the text goes above and below it. ODF's word for "ignore the frame" is <c>run-through</c>.
/// </para>
/// </remarks>
public enum TextWrap
{
    /// <summary>The text ignores the frame and runs under or over it. ODF's <c>run-through</c>.</summary>
    Through,

    /// <summary>No text beside the frame: it goes above and below. ODF's <c>none</c>.</summary>
    TopAndBottom,

    /// <summary>Text on both sides. ODF's <c>parallel</c>, OOXML's <c>bothSides</c>.</summary>
    Both,

    /// <summary>Text on the frame's left only, so the frame reaches the end margin.</summary>
    Left,

    /// <summary>Text on the frame's right only, so the frame reaches the start margin.</summary>
    Right,

    /// <summary>
    /// Whichever side has more room, decided per frame. ODF's <c>dynamic</c>, OOXML's <c>largest</c>.
    /// </summary>
    Optimal,
}

/// <summary>
/// What Writer builds for an anchored object, which is what decides whether it is captured on its
/// page.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwAnchoredObjectPosition</c>'s constructor
/// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:125-144) asks a
/// <em>different question of the two object kinds</em>, and its own comment says so:
/// </para>
/// <code>
/// if (mbIsObjFly) bConsidered = bWrapThrough &amp;&amp; !bTextBox;   // draw case: wrap through OR not a textbox
/// else            bConsidered = bWrapThrough || !bTextBox;   // fly case:  wrap through AND not a textbox
/// mbDoNotCaptureAnchoredObj = bConsidered &amp;&amp; !mbFollowTextFlow &amp;&amp; DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE;
/// </code>
/// <para>
/// So under a format that sets <c>DoNotCaptureDrawObjsOnPage</c> — DOCX and RTF, and no other, see
/// <see cref="PaginationOptions.CapturesAnchoredObjectsOnPage"/> — a picture and a shape carrying a
/// text box are captured unless they wrap through, and <b>a shape with no text box is never captured,
/// whatever its wrap</b>. This tree's own remarks used to say the escape was wrap-through and nothing
/// else, which is wrong for the third of those.
/// </para>
/// <para>
/// Censused over <c>MANIFEST.tsv</c>'s 272 DOCX, every <c>wp:anchor</c> and every absolutely
/// positioned VML shape (<c>probes/words-close-r95/capture-census.py</c>): of 6055 objects the two
/// readings differ on <b>4, in 4 documents</b> — a draw object with no text box that does not wrap
/// through. It is modelled because it is the rule, not because it moves anything.
/// </para>
/// </remarks>
public enum FrameObjectKind
{
    /// <summary>
    /// A Writer fly: a picture, an OLE object or chart, or a text frame. Captured unless it wraps
    /// through. The default, which is the answer for every reader that does not distinguish the kinds
    /// — they are the formats that capture everything anyway.
    /// </summary>
    Fly,

    /// <summary>
    /// A drawing object carrying a text box, so <c>SwTextBoxHelper::isTextBox</c> answers true for its
    /// draw format and it is captured unless it wraps through — the same answer as a fly's.
    /// </summary>
    TextBoxShape,

    /// <summary>
    /// A drawing object with no text box, which <c>bConsidered</c> makes exempt from the capture
    /// whatever its wrap.
    /// </summary>
    Shape,
}

/// <summary>
/// Where an anchored object's anchor sits on the page, which two of the capture's own conditions ask
/// about.
/// </summary>
/// <remarks>
/// <para>
/// Both are in <c>SwAnchoredObjectPosition::ImplAdjustVertRelPos</c>
/// (<c>anchoredobjectposition.cxx</c>:504-667). The <c>compatibilityMode</c> 15 narrowing of the
/// capture area to the page body needs <c>mpAnchorFrame-&gt;FindBodyFrame()</c> to find a body frame
/// whose upper is this page (:568-573), which a header, a footer and a footnote anchor have not; and
/// tdf#123002 leaves a header- or footer-anchored object alone outright once its top is past the
/// area's bottom (:637-647).
/// </para>
/// </remarks>
public enum FrameAnchorPlace
{
    /// <summary>In the page's body: body text, or a cell of a table in it.</summary>
    Body,

    /// <summary>In the running head or foot.</summary>
    Furniture,

    /// <summary>Somewhere that is neither — the notes area, or another frame's own text.</summary>
    Elsewhere,
}

/// <summary>What a frame's horizontal position is measured from.</summary>
public enum FrameHorizontalOrigin
{
    /// <summary>The sheet's own left edge.</summary>
    Page,

    /// <summary>The text area — inside the page margins. ODF's <c>page-content</c>, OOXML's <c>margin</c>.</summary>
    PageMargin,

    /// <summary>The column the anchor is in, which for single-column text is the text area.</summary>
    Column,

    /// <summary>The anchor paragraph's own rectangle, indents included.</summary>
    Paragraph,

    /// <summary>The anchoring character's position.</summary>
    Character,

    /// <summary>
    /// The band between the sheet's left edge and the text area: OOXML's <c>leftMargin</c>, and VML's
    /// <c>left-margin-area</c> and <c>outer-margin-area</c>.
    /// </summary>
    /// <remarks>
    /// <c>RelOrientation::PAGE_LEFT</c>, whose alignment area is the page's <em>left margin</em> and
    /// not the page — <c>nWidth = GetLeftMargin(page)</c> against an offset at the page's left edge
    /// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:769-778). So an offset
    /// against it and an offset against the page agree, and every other alignment does not.
    /// </remarks>
    LeftMarginArea,

    /// <summary>
    /// The band between the text area and the sheet's right edge: OOXML's <c>rightMargin</c>, and
    /// VML's <c>right-margin-area</c> and <c>inner-margin-area</c>.
    /// </summary>
    /// <remarks>
    /// <c>RelOrientation::PAGE_RIGHT</c> — <c>nWidth = GetRightMargin(page)</c> measured from
    /// <c>GetPrtRight(page)</c> (the same file, :779-788), so even a plain offset starts at the text
    /// area's right edge rather than at the sheet's left one.
    /// </remarks>
    RightMarginArea,
}

/// <summary>What a frame's vertical position is measured from.</summary>
public enum FrameVerticalOrigin
{
    /// <summary>The sheet's own top edge.</summary>
    Page,

    /// <summary>The text area — inside the page margins.</summary>
    PageMargin,

    /// <summary>The anchor paragraph's top.</summary>
    Paragraph,

    /// <summary>The anchoring line's top, which for a one-line anchor is the paragraph's.</summary>
    Line,

    /// <summary>
    /// The band between the sheet's top edge and the body: OOXML's <c>topMargin</c>, VML's
    /// <c>top-margin-area</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>RelOrientation::PAGE_PRINT_AREA_TOP</c>, and reading the layout's own switch is not enough
    /// to get it right: <c>GetVertAlignmentValues</c> puts it in the <em>same case</em> as
    /// <c>PAGE_FRAME</c> and answers the whole page's height
    /// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:327-335), which would make
    /// a centred frame land halfway down the sheet. The positioner overrides that for exactly this
    /// relation: <c>SwToContentAnchoredObjectPosition::CalcPosition</c> substitutes
    /// <c>nHeightBetweenOffsetAndMargin</c> — the offset plus
    /// <c>page-&gt;GetTopMargin() + headerFrame-&gt;GetPaintArea().Height()</c>, which is the body's
    /// own top — for the alignment height under both <c>CENTER</c> and <c>BOTTOM</c>
    /// (<c>tocntntanchoredobjectposition.cxx</c>:305-306, :367-370 and :407-410).
    /// </para>
    /// <para>
    /// The two readings differ by the length of a page and the measurement settles it: on A4 with a
    /// 72 pt top margin, a 20 pt band centred against <c>topMargin</c> is drawn by both installed
    /// references at <b>26.00 pt</b> — <c>(72 − 20) / 2</c> — and not at 410.95.
    /// </para>
    /// </remarks>
    TopMarginArea,

    /// <summary>
    /// The band between the body and the sheet's bottom edge: OOXML's <c>bottomMargin</c>, VML's
    /// <c>bottom-margin-area</c>.
    /// </summary>
    /// <remarks>
    /// <c>RelOrientation::PAGE_PRINT_AREA_BOTTOM</c>, whose origin is
    /// <c>PrtWithoutHeaderAndFooter().Bottom</c> — the body's bottom, not the page's print area's
    /// (<c>tocntntanchoredobjectposition.cxx</c>:617-628) — and whose height is the bottom margin with
    /// each footer frame's height added back
    /// (<c>anchoredobjectposition.cxx</c>:365-389), which reaches the sheet's own edge.
    /// </remarks>
    BottomMarginArea,
}

/// <summary>How a frame sits inside its horizontal origin.</summary>
public enum FrameHorizontalAlignment
{
    /// <summary>At a stated distance from the origin's start edge.</summary>
    Offset,

    /// <summary>Flush with the origin's start edge.</summary>
    Left,

    /// <summary>Centred in the origin.</summary>
    Centre,

    /// <summary>Flush with the origin's end edge.</summary>
    Right,

    /// <summary>Towards the binding: left on a right-hand page, right on a left-hand one.</summary>
    Inside,

    /// <summary>Away from the binding.</summary>
    Outside,
}

/// <summary>How a frame sits inside its vertical origin.</summary>
public enum FrameVerticalAlignment
{
    /// <summary>At a stated distance below the origin's top edge.</summary>
    Offset,

    /// <summary>Flush with the origin's top.</summary>
    Top,

    /// <summary>Centred in the origin.</summary>
    Middle,

    /// <summary>Flush with the origin's bottom.</summary>
    Bottom,
}

/// <summary>
/// A floating frame: a rectangle of content anchored somewhere in the text, that body text flows round.
/// </summary>
/// <remarks>
/// <para>
/// Deliberately not a <see cref="PageBlock"/>. A block is something the paginator stacks; a frame is
/// something it <em>places</em>, at a position derived from an anchor and an origin rather than from
/// where the last block ended. The two would only share the list, and putting a frame in it would mean
/// every consumer of a block list having to skip one.
/// </para>
/// <para>
/// A frame's own content is blocks, so a text frame containing a table needs no second layout path — it
/// goes through <see cref="FlowLayouter"/> exactly as a header or a table cell does. An image frame
/// carries no blocks and is recorded by its rectangle, since decoding the raster is a separate matter and
/// the wrap does not depend on it.
/// </para>
/// </remarks>
public sealed record PageFrame
{
    /// <summary>How big the frame is.</summary>
    public required DocSize Size { get; init; }

    /// <summary>
    /// The whole shape group this frame is one member of, or null for a frame that stands alone.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group is one anchored object holding many shapes, and the members are placed <em>relative to
    /// it</em>: the anchor's position and alignment decide where the group's rectangle goes, and each
    /// member sits at a fixed offset inside that rectangle. Carrying the group's size here is what lets a
    /// centred or right-aligned group still be resolved once and its members follow — aligning each
    /// member by its own width would spread a letterhead across the page.
    /// </para>
    /// <para>
    /// The members are flattened into siblings rather than nested, because that is what the layout engine
    /// can place: <see cref="FrameLayout"/> resolves one rectangle per frame and a group's member is a
    /// rectangle like any other. What the flattening must not do is punch a hole in the text per member,
    /// so a member takes <see cref="TextWrap.Through"/> and the group's own envelope keeps the wrap.
    /// </para>
    /// </remarks>
    public DocSize? GroupSize { get; init; }

    /// <summary>Where this frame sits inside <see cref="GroupSize"/>, from the group's top-left.</summary>
    public DocPoint GroupOffset { get; init; }

    /// <summary>What the frame is fixed to.</summary>
    public FrameAnchor Anchor { get; init; } = FrameAnchor.Paragraph;

    /// <summary>
    /// How body text behaves beside it.
    /// </summary>
    /// <remarks>
    /// <see cref="TextWrap.Through"/> by default, which is the harmless answer: a frame whose wrap could
    /// not be read leaves the text exactly where it would have been rather than moving all of it.
    /// </remarks>
    public TextWrap Wrap { get; init; } = TextWrap.Through;

    /// <summary>
    /// What Writer builds for this object, which decides whether the page capture reaches it.
    /// </summary>
    /// <remarks>
    /// <see cref="FrameObjectKind"/> carries the rule and the census. Only the DOCX reader sets it:
    /// the other three formats capture every content anchor, so the distinction cannot change an
    /// answer there and a wrong guess would.
    /// </remarks>
    public FrameObjectKind ObjectKind { get; init; }

    /// <summary>
    /// Whether the object follows the text flow, which changes what <em>the page</em> means to its
    /// vertical position.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>IsFollowingTextFlow</c>. Its pool default is <b>false</b>
    /// (<c>sw/source/core/bastyp/init.cxx</c>:437) and both of <c>GraphicImport</c>'s writes of
    /// <c>PROP_FOLLOW_TEXT_FLOW</c> are gated on the anchor being inside a table
    /// (<c>GraphicImport.cxx</c>:1316-1318 for a shape and :1859-1861 for a graphic), which is why this
    /// is false for every shape and every picture and why the whole term was left out of
    /// <see cref="FrameLayout"/> for several rounds.
    /// </para>
    /// <para>
    /// <b>An embedded object is the exception, and its seat carries no such guard.</b> A chart's or an
    /// OLE object's <c>a:graphicData</c> becomes a <c>com.sun.star.drawing.OLE2Shape</c>
    /// (<c>oox/source/drawingml/shape.cxx</c>:348-358), which <c>DomainMapper_Impl</c> replaces with a
    /// <c>SwXTextEmbeddedObject</c> (<c>DomainMapper_Impl.cxx</c>:5096-5112) and then sets
    /// <c>IsFollowingTextFlow</c> on straight from <c>layoutInCell</c> with no <c>IsInTable()</c> test at
    /// all (:9792). And <c>m_bLayoutInCell</c> is initialised <b>true</b> (<c>GraphicImport.cxx</c>:340),
    /// so an anchor that states nothing follows the text flow too — only an explicit
    /// <c>layoutInCell="0"</c> outside a table clears it.
    /// </para>
    /// <para>
    /// What it does to the position is <see cref="FrameLayout.Place"/>'s business; see there for the
    /// three things that change and for the measurement on
    /// <c>028_Unit_Circle_Chart_Optimized_Graph.docx</c>. Only the DOCX reader sets it: the other three
    /// word-processing readers build no embedded object, so the attribute has nothing to reach there.
    /// </para>
    /// </remarks>
    public bool FollowsTextFlow { get; init; }

    /// <summary>What the horizontal position is measured from.</summary>
    public FrameHorizontalOrigin HorizontalOrigin { get; init; } = FrameHorizontalOrigin.Paragraph;

    /// <summary>How it sits inside that origin.</summary>
    public FrameHorizontalAlignment HorizontalAlignment { get; init; } = FrameHorizontalAlignment.Offset;

    /// <summary>The distance from the origin's start edge, when the alignment is an offset.</summary>
    public Length HorizontalOffset { get; init; }

    /// <summary>What the vertical position is measured from.</summary>
    public FrameVerticalOrigin VerticalOrigin { get; init; } = FrameVerticalOrigin.Paragraph;

    /// <summary>How it sits inside that origin.</summary>
    public FrameVerticalAlignment VerticalAlignment { get; init; } = FrameVerticalAlignment.Offset;

    /// <summary>The distance below the origin's top edge, when the alignment is an offset.</summary>
    public Length VerticalOffset { get; init; }

    /// <summary>
    /// How far text must stay clear of the frame on each side.
    /// </summary>
    /// <remarks>
    /// Writer keeps this as the frame's own margins and adds it to the rectangle before asking what a line
    /// overlaps — <c>SwAnchoredObject::GetObjRectWithSpaces</c>. So it widens the hole in the text without
    /// moving the frame, which is why it is here rather than folded into the position.
    /// </remarks>
    public Margins Spacing { get; init; }

    /// <summary>
    /// The room an as-character drawing's effects need beyond its extent, from <c>wp:effectExtent</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shadow, a glow or a fat stroke paints outside the <c>wp:extent</c> the file states, and
    /// <c>wp:effectExtent</c> is how much on each side. For a <c>wp:inline</c> LibreOffice folds it
    /// straight into the object's own margins — <c>GraphicImport.cxx</c>:1036-1055, guarded by
    /// <c>IMPORT_AS_DETECTED_INLINE</c> and a zero rotation, and commented there
    /// <em>"EffectExtent contains all needed additional space, including fat stroke and shadow. Simple
    /// add it to the margins."</em> Those margins are then part of the portion Writer hangs on the line:
    /// <c>SwFlyCntPortion::SetBase</c> sizes itself from
    /// <c>SwAsCharAnchoredObjectPosition::GetObjBoundRectInclSpacing()</c>, which is the object's
    /// rectangle enlarged by its spacing.
    /// </para>
    /// <para>
    /// So it grows the line rather than the drawing, which is why it is a margin here and not folded
    /// into <see cref="Size"/> — the shape is still painted at the size the file gives it.
    /// </para>
    /// <para>
    /// Measured against both installed references, which agree to the twip on every fixture, in
    /// <c>dotnet/probes/words-inline-effectextent/</c>. One 50.4 pt shape between two text lines, the
    /// gap between those lines against a zero-extent control: <c>27432</c> EMU adds <strong>4.30 pt</strong>
    /// (2 x 2.16 rounded to the twip), <c>91440</c> adds <strong>14.40 pt</strong> (2 x 7.2) and
    /// <c>137160</c> adds <strong>21.60 pt</strong> (2 x 10.8). A top-only extent and a bottom-only
    /// extent each add half of that, so the two edges are independent and additive.
    /// </para>
    /// <para>
    /// Empty for a <c>wp:anchor</c>. LibreOffice does fold the extent into a floating drawing's wrap
    /// margins as well, by a different and much longer route; the note on <see cref="Spacing"/> records
    /// why that one is deliberately not read yet.
    /// </para>
    /// </remarks>
    public Margins EffectExtent { get; init; }

    /// <summary>
    /// How much room the drawing takes on its line: <see cref="Size"/> grown by
    /// <see cref="EffectExtent"/>, and for a turned drawing the room its <em>turned</em> rectangle
    /// needs as well.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Upright, this is the box the file states grown by the four extent edges, and the drawing sits
    /// at its top-left corner plus the left edge — see <see cref="InlineOffset"/>.
    /// </para>
    /// <para>
    /// <strong>Turned, the same two rectangles are both still here and neither is the answer on its
    /// own.</strong> LibreOffice keeps <em>both</em>: the object it lays out is the turned snap
    /// rectangle, and the margins round it are the difference between that and the rectangle Word
    /// reserved — <c>GraphicImport.cxx</c>:1055-1090, which takes Word's base rectangle, applies
    /// Word's own width/height swap to it, expands it by the effect extent, and sets each margin to
    /// the signed gap between that and the snap rectangle. The horizontal margins keep their sign and
    /// the vertical ones are clamped at nought (<c>GraphicImport.cxx</c>:1245-1249, tdf#141880), so
    /// the room taken comes out as
    /// </para>
    /// <list type="bullet">
    ///   <item><description>across: Word's box, whatever the turn does to the drawing; and</description></item>
    ///   <item><description>down: the larger of Word's box and the turned one.</description></item>
    /// </list>
    /// <para>
    /// Measured in <c>dotnet/probes/words-inline-rotated-bbox/</c> on a 144 x 50.4 pt black
    /// rectangle, both installed references identical. Room on the line, in points, against a
    /// zero-degree control of 144.00 x 50.40:
    /// </para>
    /// <list type="table">
    ///   <item><term>20 deg</term><description>144.00 x <b>96.60</b> — the turned height, Word's width</description></item>
    ///   <item><term>20 deg, extent 137160</term><description><b>165.60</b> x 96.60 — the extent still grows the width</description></item>
    ///   <item><term>45 deg</term><description><b>50.40</b> x <b>144.00</b> — the swap, and it beats the turned 137.46</description></item>
    ///   <item><term>90 deg</term><description>50.40 x 144.00</description></item>
    ///   <item><term>135 deg</term><description>144.00 x <b>137.46</b> — no swap at 135, so the turned height wins</description></item>
    ///   <item><term>315 deg</term><description>144.00 x 137.46</description></item>
    ///   <item><term>20 deg, 144 x 144</term><description>144.00 x <b>184.57</b></description></item>
    /// </list>
    /// <para>
    /// The 45-degree row is the one that settles the shape of the rule: the turned box is
    /// 137.46 square there, and both references take <b>144.00</b> — Word's swapped height — which
    /// only a rule that keeps both rectangles can produce.
    /// </para>
    /// </remarks>
    public DocSize InlineExtent
    {
        get
        {
            DocSize word = WordInlineBox;
            if (RotationDegrees == 0) return word;

            Length turned = TurnedSize.Height;
            return new DocSize(word.Width, turned > word.Height ? turned : word.Height);
        }
    }

    /// <summary>
    /// Where the drawing's own rectangle sits inside <see cref="InlineExtent"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Upright it is the extent's <em>left</em> edge and no vertical offset at all, which is not
    /// symmetrical and is measured: LibreOffice moves an as-character object by both its left and its
    /// upper spacing (<c>SwAsCharAnchoredObjectPosition::CalcPosition</c>,
    /// <c>sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx</c>:129-133) and then
    /// loses the vertical half again wherever the object is a shape carrying a <c>wps:txbx</c>, whose
    /// TextBox does not follow its draw shape. See <c>FrameLayout.HangInline</c>.
    /// </para>
    /// <para>
    /// Turned, it is a centring in both axes, because the margins that surround a turned object are
    /// symmetrical by construction — each is half the gap between Word's box and the snap rectangle —
    /// and the drawing's own rectangle shares its centre with that snap rectangle. Measured on the
    /// same fixtures, the drawn rectangle's left edge in points against a line starting at 103.50:
    /// 20 deg <b>99.25</b> (its 152.25 pt turned box centred in Word's 144), 45 deg <b>60.00</b>
    /// (137.25 centred in the swapped 50.40, so it hangs into the margin), 135 deg <b>106.75</b>.
    /// </para>
    /// </remarks>
    public DocPoint InlineOffset
    {
        get
        {
            if (RotationDegrees == 0) return new DocPoint(EffectExtent.Left, Length.Zero);

            DocSize box = InlineExtent;
            return new DocPoint(
                (box.Width - Size.Width) / 2,
                (box.Height - Size.Height) / 2);
        }
    }

    /// <summary>
    /// How far below <see cref="InlineOffset"/> the drawing's own ink is painted.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The vertical half of the effect extent, and it moves the <em>drawing</em> without moving the
    /// text of a <c>wps:txbx</c> — which is why it is not folded into <see cref="InlineOffset"/>,
    /// whose horizontal half moves both. <see cref="PlacedFrame.Ink"/> carries the measurement that
    /// establishes the split.
    /// </para>
    /// <para>
    /// Zero for a turned drawing, whose two rectangles are centred in one another instead
    /// (<see cref="InlineOffset"/>), and zero for everything <see cref="EffectExtent"/> is empty
    /// for — an anchored drawing, and a plain picture, which LibreOffice converts to a Writer
    /// graphic object before the margin code can reach it.
    /// </para>
    /// </remarks>
    public Length InlineInkOffset
        => RotationDegrees == 0 ? EffectExtent.Top : Length.Zero;

    /// <summary>
    /// The rectangle Word reserved on the line: the stated extent with Word's own width/height swap,
    /// grown by the effect extent.
    /// </summary>
    /// <remarks>
    /// <c>lcl_doMSOWidthHeightSwap</c> (<c>GraphicImport.cxx</c>:533-548) swaps the two about the
    /// rectangle's centre when the angle, truncated to whole degrees and taken modulo 180, lands in
    /// <c>[45, 135)</c>. That half-open interval is the reason 45 degrees and 135 degrees behave
    /// differently on an oblong, and the fixtures in <see cref="InlineExtent"/> show both.
    /// </remarks>
    private DocSize WordInlineBox
    {
        get
        {
            (Length width, Length height) = SwapsWidthAndHeight
                ? (Size.Height, Size.Width)
                : (Size.Width, Size.Height);

            return new DocSize(
                width + EffectExtent.Left + EffectExtent.Right,
                height + EffectExtent.Top + EffectExtent.Bottom);
        }
    }

    /// <summary>The bounding box of <see cref="Size"/> turned by <see cref="RotationDegrees"/>.</summary>
    /// <remarks>
    /// Snapped to the twip, which is the grid LibreOffice's own snap rectangle lives on.
    /// </remarks>
    private DocSize TurnedSize
    {
        get
        {
            double radians = RotationDegrees * Math.PI / 180.0;
            double across = Math.Abs(Math.Cos(radians));
            double down = Math.Abs(Math.Sin(radians));
            double width = Size.Width.Emu;
            double height = Size.Height.Emu;

            return new DocSize(
                Twips((width * across) + (height * down)),
                Twips((width * down) + (height * across)));

            static Length Twips(double emu)
                => Length.FromTwips(Length.FromEmu((long)Math.Round(emu)).Twips);
        }
    }

    /// <summary>Whether Word reserved the drawing's height across the line and its width down it.</summary>
    private bool SwapsWidthAndHeight
    {
        get
        {
            if (RotationDegrees == 0) return false;

            // Truncated to whole degrees and then taken modulo 180, both exactly as
            // `(nMSOAngle / 60000) % 180` does on a `sal_Int32` — so a negative angle stays negative
            // and never swaps, which is LibreOffice's behaviour rather than a simplification.
            int degrees = (int)RotationDegrees % 180;
            return degrees is >= 45 and < 135;
        }
    }

    /// <summary>Where the anchoring character sits in the paragraph's text, for a character anchor.</summary>
    public int AnchorOffset { get; init; }

    /// <summary>
    /// How much of an as-character frame sits above the baseline, or null for all of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Null is the ordinary inline picture, which rests its bottom on the baseline, and is the default so
    /// that the three readers that had no vertical rule to state keep the numbers they were measured
    /// against. Zero is the other end: the frame hangs entirely below the line and raises its descent
    /// instead, which is what Writer does for a fly whose position relative to the baseline comes back at
    /// nought or more (<c>SwFlyCntPortion::SetBase</c>).
    /// </para>
    /// <para>
    /// Only DOC sets it, and only for a shape a <c>SHAPE</c> field made as-character: those state a
    /// vertical orientation of <c>TEXT_LINE</c> with no offset, which resolves to nought. Ignored for
    /// every other anchor, since only an as-character frame has a baseline to be measured from.
    /// </para>
    /// </remarks>
    public Length? InlineAscent { get; init; }

    /// <summary>A text frame's own content, empty for an image.</summary>
    public IReadOnlyList<PageBlock> Blocks { get; init; } = [];

    /// <summary>The inset between the frame's edge and its text.</summary>
    public Margins Padding { get; init; }

    /// <summary>
    /// True when the frame's height is stated rather than grown from its text, so content taller than
    /// the frame is not formatted at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shape's text body either grows to fit its text — DrawingML's <c>a:spAutoFit</c>, VML's
    /// <c>mso-fit-shape-to-text</c> — or keeps the height the file states. In the second case Writer
    /// formats only the lines that fit and simply does not lay the rest out: they are absent from the
    /// PDF's text operators, not merely clipped by a painting rectangle, so <c>pdftotext</c> cannot
    /// find them either.
    /// </para>
    /// <para>
    /// <strong>Measured on the installed 26.2.4.2</strong>, not inferred — see
    /// <c>dotnet/probes/words-extra-01/</c>. Sixty authored boxes of stated heights from 1 pt to 100 pt,
    /// at three inset sizes, holding six paragraphs of 8 pt text, give one rule:
    /// <em>a line is formatted iff its top offset is strictly less than the box's content height</em>,
    /// and the first line is always formatted however short the box. The obvious alternative — a line
    /// is kept when it fits entirely — is refuted by a 10 pt box with zero insets, which draws two
    /// lines of a ~9.6 pt face.
    /// </para>
    /// <para>
    /// <c>a:normAutofit</c> does <em>not</em> disable it: LibreOffice does not shrink the text, it
    /// truncates exactly as <c>a:noAutofit</c> does. Neither does <c>bodyPr/@vertOverflow</c>, whose
    /// <c>overflow</c> and <c>clip</c> values render identically. Only autofit-to-text spares the
    /// content.
    /// </para>
    /// <para>
    /// False by default, which is the behaviour every format had before this existed. The DOCX reader
    /// sets it because that is where it was measured; the ODF, WW8 and RTF readers do not, and a round
    /// wanting it there should measure those importers rather than assume the rule transfers.
    /// </para>
    /// </remarks>
    public bool HasFixedHeight { get; init; }

    /// <summary>
    /// True when <see cref="DocSize.Height"/> of <see cref="Size"/> is a <em>floor</em> the frame's own
    /// text grows past, rather than the height the frame keeps.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>SwFrameSize::Minimum</c>, and the exact opposite end of
    /// <see cref="HasFixedHeight"/>: a fixed frame formats only the lines that fit and never lays the
    /// rest out, and a minimum one is made as tall as everything it holds. Neither is the default —
    /// a frame that says nothing keeps the height the file states and draws whatever overflows past
    /// its bottom edge, which is what every format did before this existed and is still what a
    /// running head does.
    /// </para>
    /// <para>
    /// <strong>The rule is <c>SwFlyFrame::Format</c></strong>
    /// (<c>sw/source/core/layout/fly.cxx</c>:1549-1608), and it is three clamps in one order:
    /// <c>nRemaining</c> starts as <c>CalcContentHeight</c> — the sum of the fly's lower frames'
    /// heights (<c>fly.cxx</c>:3538-3588) — is raised to <c>nMinHeight − nUL</c> when the frame is
    /// shorter than its stated minimum, is raised again to <c>MINFLY</c> (23 twips,
    /// <c>sw/inc/swtypes.hxx</c>:59), and the frame's height is then <c>nRemaining + nUL</c>. So
    /// <em>the stated minimum is the whole frame's height, insets included</em>, while the content
    /// height is the print area's.
    /// </para>
    /// <para>
    /// <c>nUL</c> is <c>CalcTopLine() + CalcBottomLine()</c>, which is each edge's padding plus its
    /// border width plus its shadow space (<c>SwBorderAttrs::CalcTopLine_</c>,
    /// <c>sw/source/core/layout/frmtool.cxx</c>:2474-2486, over
    /// <c>SvxBoxItem::CalcLineSpace</c>, <c>editeng/source/items/frmitems.cxx</c>:3717-3755).
    /// <see cref="FrameLayout"/> uses <see cref="Padding"/> alone for it, because that is exactly what
    /// it insets a frame's text by; see the remarks there for what that costs.
    /// </para>
    /// <para>
    /// <strong>Measured on 26.2.4.2</strong> over sixteen authored frames,
    /// <c>dotnet/probes/odt-frame-r75/</c>, by how far the frame pushes the body paragraph below it:
    /// two 12 pt lines give 27.6 pt and four give 55.2; a 12 pt space-<em>before</em> on each of two
    /// paragraphs adds 24 and on each of four adds 48, so every one of them counts <em>including the
    /// first</em>; a 12 pt space-<em>after</em> on each of two adds 12 and on each of four adds 36, so
    /// the <em>last</em> paragraph's is dropped. That asymmetry is Writer's, not an artefact: a text
    /// frame's own area carries its upper space, and its lower space is only ever realised as the
    /// upper space of whatever follows it (<c>SwFlowFrame::CalcUpperSpace</c>), which for the last
    /// frame in a fly is nothing. It is why the measure here is
    /// <see cref="FlowLayouter.Extent"/> and not <see cref="PlacedFlow.Advance"/>.
    /// </para>
    /// </remarks>
    public bool GrowsToContent { get; init; }

    /// <summary>
    /// The least <see cref="GrowsToContent"/> may leave the frame, or null when the stated height is
    /// itself that floor.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two formats mean different things by a frame that fits its text. ODF's
    /// <c>fo:min-height</c> is a floor and <c>svg:height</c> is where the frame starts, so a frame
    /// with more text than it states grows and one with less keeps its height — which is
    /// <see cref="GrowsToContent"/> with no floor stated here. DrawingML's <c>a:spAutoFit</c> discards
    /// the stated <c>a:ext</c> entirely: the shape is exactly as tall as its text, in both directions.
    /// </para>
    /// <para>
    /// <strong>Measured on 26.2.4.2</strong> over twelve authored boxes,
    /// <c>dotnet/probes/words-spautofit-r182/</c>: one, three and six 16 pt lines in a box stating
    /// either 200 pt or 40 pt of <c>cy</c>. With <c>a:noAutofit</c> the fill is 200.00 and 40.00 pt,
    /// the stated height and nothing else. With <c>a:spAutoFit</c> it is <b>25.55, 62.45 and
    /// 117.80 pt whichever <c>cy</c> is stated</b> — 18.45 pt a line plus the 7.2 pt of insets, the
    /// stated height read neither as a floor nor as a ceiling. So this is <see cref="Length.Zero"/>
    /// for such a shape, and the <c>MINFLY</c> clamp is then all that is left under it.
    /// </para>
    /// </remarks>
    public Length? HeightFloor { get; init; }

    /// <summary>
    /// True when the frame is painted <em>behind</em> the document's text rather than over it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Z-order, not layout: nothing about where a line breaks or where the frame sits depends on this,
    /// and the layouters never read it. It decides one thing — whether
    /// <see cref="PageDrawing.Draw"/> emits the frame before the header and body or after them.
    /// </para>
    /// <para>
    /// In Writer this is the <c>SvxOpaqueItem</c>: false puts the fly on the <em>hell</em> layer and
    /// true on <em>heaven</em> (<c>sw/source/core/layout/fly.cxx</c>:1129-1138), and every importer
    /// reaches paint order by setting that one item. The two readers derive it differently because the
    /// two formats state it differently, and both rules are LibreOffice's own:
    /// </para>
    /// <list type="bullet">
    ///   <item>
    ///     <description>
    ///     <strong>WW8</strong> — <c>bMoveToBackground = bDrawHell || ((header||footer) &amp;&amp; nwr == 3)</c>
    ///     (<c>sw/source/filter/ww8/ww8graf.cxx</c>:2833). <c>bDrawHell</c> is the Escher
    ///     <c>DFF_Prop_fPrint</c> group's <c>fBehindDocument</c> bit
    ///     (<c>filter/source/msfilter/msdffimp.cxx</c>:5547). The <c>FSPA</c>'s own <c>fBelowText</c>
    ///     is deliberately <em>not</em> consulted — the comment beside the C++ says in terms that its
    ///     value "can be neglected" (#i46794), and a reader that trusts it instead gets a different
    ///     answer on exactly the documents this matters for.
    ///     </description>
    ///   </item>
    ///   <item>
    ///     <description>
    ///     <strong>DOCX</strong> — <c>m_bOpaque</c> in
    ///     <c>sw/source/writerfilter/dmapper/GraphicImport.cxx</c>. It starts as
    ///     <c>!IsInHeaderFooter()</c> (:342), so a drawing anchored in a header or footer is behind the
    ///     text whatever else it says; <c>behindDoc="1"</c> clears it (:698-702); and for
    ///     <c>wrapSquare</c>, <c>wrapThrough</c>, <c>wrapTight</c> and <c>wrapTopAndBottom</c> a file
    ///     targeting Word 2013 or later restores it (:1589, :1697, tdf#137850) — so under a modern
    ///     compatibility mode <c>behindDoc</c> is honoured for <c>wrapNone</c> alone.
    ///     </description>
    ///   </item>
    /// </list>
    /// <para>
    /// False by default, which is what every reader did before this existed.
    /// </para>
    /// </remarks>
    public bool BehindText { get; init; }

    /// <summary>
    /// The anchor's <c>relativeHeight</c> — where this frame sits in the stack, low to high.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>wp:anchor</c> declares its own place in the z order, and it is <em>not</em> the order the
    /// anchors appear in the document. Painting in document order is therefore wrong whenever a file
    /// declares them out of order, which real templates do constantly: measured over the five corpus
    /// documents where the fault showed, all five declare <c>relativeHeight</c> on every anchor and
    /// <em>none</em> of the five is in document order.
    /// </para>
    /// <para>
    /// The symptom is not subtle and does not look like a z-order fault. A background shape declared
    /// late paints over content declared early, so the page loses text the renderer did in fact draw:
    /// <c>045_Visual_Product_Roadmap</c> shows <c>2021</c> at content-stream offset 2473 and fills the
    /// black box over it at 4180; <c>060_Human_Body_Concept_Map</c> draws the whole slide and then the
    /// grey ground across all of it. Every pixel metric reports that as missing content.
    /// </para>
    /// <para>
    /// Zero when the anchor does not declare one, which sorts it below anything that does — and since
    /// the sort is stable, equal values keep document order, which is Word's own tie-break.
    /// </para>
    /// <para>
    /// <strong>It is a <c>long</c> because two different declarations land in it and they do not share
    /// a range.</strong> DrawingML's <c>relativeHeight</c> is an unsigned 32-bit value; VML's
    /// <c>z-index</c> is signed, and LibreOffice sorts <em>every</em> shape that declares one above
    /// <em>every</em> <c>relativeHeight</c> whatever the two numbers are —
    /// <c>GraphicZOrderHelper::adjustRelativeHeight</c>, <c>sw/source/writerfilter/dmapper/
    /// GraphicHelpers.cxx:279-330</c>: "in general, all z-index-defined shapes appear on top of
    /// relativeHeight graphics regardless of the value". <see cref="Ooxml.DocxVmlFrames"/> therefore
    /// offsets a <c>z-index</c> by 2^32, which is above the whole unsigned range and keeps both
    /// families' internal ordering intact.
    /// </para>
    /// </remarks>
    public long ZOrder { get; init; }

    /// <summary>
    /// The shape's <c>a:prstGeom/@prst</c>, or null when it states none and is a plain box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A Word document's anchored shapes declare geometry and it was never read.</strong>
    /// <c>a:prstGeom</c> is resolved on the slide side — <c>PptxSlideLayout</c> feeds it to
    /// <see cref="XRay.Office.Ooxml.DrawingML.CustomShapeGeometry"/> — and the DOCX reader consulted
    /// the same <c>spPr</c> for fill and outline while ignoring the preset, so every shape in a
    /// Word file was drawn as its bounding rectangle whatever it asked for.
    /// </para>
    /// <para>
    /// The catalogue was never the problem: all 187 presets are in
    /// <c>PresetShapeGeometry.txt</c>, this side simply did not ask. Six corpus templates showed it
    /// at once, and they are ordinary business documents rather than exotica: a timeline's
    /// milestone circles came out as squares (<c>ellipse</c>, 32 uses across the six), a roadmap's
    /// chevrons as bars (<c>homePlate</c>, 33), plus <c>diamond</c>, <c>rightArrow</c>,
    /// <c>roundRect</c> and <c>bentConnector3</c>.
    /// </para>
    /// </remarks>
    public string? Preset { get; init; }

    /// <summary>
    /// The path the shape states outright, in its own coordinates with the origin at its top left,
    /// or null when it states a preset or nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:custGeom</c> — a shape whose guides and paths the file writes out rather than naming.
    /// It was not read at all on this side, so all <b>124 of them across 21 corpus documents</b>
    /// were painted as their bounding rectangles. The storyboard templates are where it shows:
    /// their rings came out as squares and their arrows, being rotated squares, as diamonds.
    /// </para>
    /// <para>
    /// Resolved when the drawing is read rather than when it is drawn, unlike
    /// <see cref="Preset"/>, because a custom geometry is evaluated from the shape's own guide
    /// formulae and the shape's extent is known at that point — where a preset is a name that
    /// costs nothing to carry and is cheapest evaluated once the placed rectangle is in hand.
    /// </para>
    /// <para>
    /// Two paths and not one, because a subpath states whether it is filled and whether it is
    /// stroked, and every connector is one open subpath saying <c>fill="none"</c>. Filling the
    /// whole outline of one draws a solid blob where the file states a line.
    /// </para>
    /// </remarks>
    public GraphicsPath? FillOutline { get; init; }

    /// <summary>The part of <see cref="FillOutline"/>'s geometry that is stroked, or null.</summary>
    /// <remarks>
    /// Set together with <see cref="FillOutline"/> and never on its own; the two differ only where
    /// the shape's subpaths state <c>fill="none"</c> or <c>stroke="false"</c>.
    /// </remarks>
    public GraphicsPath? StrokeOutline { get; init; }

    /// <summary>The <c>a:avLst</c> values the shape states, by name.</summary>
    /// <remarks>
    /// Null when it states none, which is not the same as an empty set: the preset's own defaults
    /// apply, and they are what make an unadjusted <c>roundRect</c> round rather than square.
    /// </remarks>
    public IReadOnlyDictionary<string, double>? Adjustments { get; init; }

    /// <summary>
    /// How far the shape is turned about its own centre, clockwise, in degrees.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:xfrm/@rot</c>, in degrees rather than the file's sixtieths of one. Zero for the great
    /// majority of frames, which is why it is a plain <c>double</c> rather than something nullable:
    /// no rotation and a rotation of nothing are the same drawing.
    /// </para>
    /// <para>
    /// The extent is stated <em>unrotated</em> and the rotation is applied about the centre of that
    /// rectangle, so <see cref="Size"/> is the shape's own width and height whatever this says. It
    /// is the drawing that turns, not the box: a connector 22 pt wide and nothing tall at 270° is
    /// still 22 pt long, drawn down the page instead of across it.
    /// </para>
    /// <para>
    /// <b>The wrap is not turned with it.</b> LibreOffice wraps text round a rotated shape's
    /// enclosing rectangle, which is larger; this still wraps round the stated one. That is visible
    /// only for a rotated shape that text flows beside, and the corpus's rotated shapes are
    /// overwhelmingly connectors and arrows inside groups, which wrap through.
    /// </para>
    /// </remarks>
    public double RotationDegrees { get; init; }

    /// <summary>
    /// How far the frame's own text is turned, clockwise, in degrees — which is not always the
    /// same as <see cref="RotationDegrees"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>wps:bodyPr/@rot</c> is the angle of the text itself, and it is stated absolutely rather
    /// than as an addition to the shape's: a shape turned 345° whose body states <c>rot="0"</c>
    /// carries upright text across a slanting box. Where the body states nothing, the text takes
    /// the shape's angle, which is the ordinary case of a label turning with the thing it labels.
    /// </para>
    /// <para>
    /// It is not a corner of the schema. <b>Every one of the 112 rotated text-bearing shapes in the
    /// 271-document corpus states <c>rot="0"</c></b> — 107 plainly and 5 with <c>upright="1"</c>
    /// beside it — so treating the shape's angle as the text's would have been wrong on all 112.
    /// The reference settles it too: <c>025_Unit_Circle_Chart_Cos_and_Sin_Model</c> arranges 32
    /// labels round a circle at 32 different angles and LibreOffice draws every one of them
    /// horizontal.
    /// </para>
    /// </remarks>
    public double TextRotationDegrees { get; init; }

    /// <summary>
    /// Where the frame's own text sits when the frame is taller than the text is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>wps:bodyPr/@anchor</c>. Top for the great majority, which is why it is the default and why
    /// this went unnoticed: a text box sized to its text shows nothing either way.
    /// </para>
    /// <para>
    /// It shows on a shape sized to be a shape. Censused over the 271 corpus <c>docx</c>, <b>132
    /// text-bearing shapes across 20 documents</b> ask for <c>ctr</c> — the Venn diagram templates
    /// are eight of the twenty, and their labels sit in circles two or three times the height of a
    /// line, so a label drawn against the top of its circle lands outside the ink it names.
    /// </para>
    /// <para>
    /// <c>just</c> and <c>dist</c> are read as top. They ask for the <em>lines</em> to be spread
    /// through the box rather than for the block to be moved, which is a different mechanism, and
    /// LibreOffice's own importer takes neither: no corpus document states either.
    /// </para>
    /// </remarks>
    public VerticalTextAlignment TextAlignment { get; init; }

    /// <summary>The marker at the start of a line, if it carries one.</summary>
    /// <remarks>
    /// <c>a:headEnd</c>. An arrowhead is a filled polygon beside the shaft rather than a property
    /// of the pen, so it is carried here as what the file said and built at drawing time by
    /// <see cref="LineEnds"/> — which needs the placed line to know where the point goes.
    /// </remarks>
    public LineEnd HeadEnd { get; init; }

    /// <summary>The marker at the end of a line. <c>a:tailEnd</c>, and much the commoner of the two.</summary>
    public LineEnd TailEnd { get; init; }

    /// <summary>The frame's background, or null when it has none.</summary>
    public Colour? Fill { get; init; }

    /// <summary>
    /// The frame's background when it is a gradient rather than a flat colour, or null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Beside <see cref="Fill"/> rather than replacing it, and the two are never both set. A
    /// gradient cannot be built here because a <see cref="GradientPaint"/> holds absolute points
    /// and this frame does not yet know where on the page it lands — so what is carried is the
    /// shape's own description of the ramp, and <c>PageDrawing</c> turns it into a paint against
    /// the area the frame was placed in.
    /// </para>
    /// <para>
    /// Keeping <see cref="Fill"/> a colour is not a compromise for the sake of the callers. It is
    /// what the automatic font colour resolves against — a frame's fill decides whether the text
    /// on it comes out black or white — and that question wants one colour whatever the shape is
    /// painted with. A gradient-filled frame therefore answers it the way an unfilled one does,
    /// which is what it did before this existed.
    /// </para>
    /// </remarks>
    public GradientDescription? Gradient { get; init; }

    /// <summary>The frame's border colour, or null when it has no border.</summary>
    public Colour? BorderColour { get; init; }

    /// <summary>How thick that border is.</summary>
    public Length BorderWidth { get; init; }

    /// <summary>
    /// The preset naming the border's dash pattern — <c>a:prstDash/@val</c> — or null for a solid line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Kept as the preset's name rather than as an expanded array because the array depends on the pen
    /// width and the cap as well as on the name, and <see cref="Core.Graphics.DashPresets"/> already
    /// holds that arithmetic — ported from <c>lclConvertPresetDash</c>
    /// (<c>oox/source/drawingml/lineproperties.cxx</c>:60-83) and <c>XDash::CreateDotDashArray</c>
    /// (<c>svx/source/xoutdev/xattr.cxx</c>:503-640) — for the chart, table and slide paths. Storing
    /// the name keeps one expansion rather than four.
    /// </para>
    /// <para>
    /// Null covers <c>solid</c> and an unrecognised token alike, which is deliberate and is
    /// <see cref="Core.Graphics.DashPresets"/>'s own rule rather than this one's.
    /// </para>
    /// </remarks>
    public string? BorderDash { get; init; }

    /// <summary>How the border's ends, and each of its dashes, are capped.</summary>
    /// <remarks>
    /// <c>a:ln/@cap</c>, default <c>flat</c>. It is carried beside the dash because it changes the
    /// pattern's arithmetic as well as the line's ends — see <see cref="BorderDash"/>.
    /// </remarks>
    public LineCap BorderCap { get; init; }

    /// <summary>
    /// How far inside its own rectangle the frame's border is stroked, zero for on the edge.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A frame's border is normally its rectangle. One thing draws a box <em>inside</em> the room it
    /// takes: Writer's legacy form checkbox, whose portion is a square of the line's whole text height
    /// and whose drawn rectangle is that square deflated by a hard <c>delta = 25</c> twips on every side
    /// (<c>SwTextPaintInfo::DrawCheckBox</c>, <c>sw/source/core/text/inftxt.cxx</c>:1266). The two
    /// cannot be folded into one number, because the outer square is what the line is charged and the
    /// inner one is what the page shows.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 over seven sizes and five faces, <c>probes/words-r56/formcheckbox.py</c>:
    /// at 12 pt Liberation Serif the text height is 276 twips and the square drawn is 226, at 24 pt it
    /// is 552 and 502, and at 8 pt 184 and 134 — a constant 50 twips at every size, which is what says
    /// it is an inset rather than a proportion.
    /// </para>
    /// </remarks>
    public Length BorderInset { get; init; }

    /// <summary>True when the frame is crossed corner to corner as well as bordered.</summary>
    /// <remarks>
    /// A checked form checkbox, and nothing else: <c>DrawCheckBox</c> strokes the same inset rectangle
    /// and then both of its diagonals. Distinct from <see cref="IsLine"/>, which is a shape that
    /// <em>is</em> one diagonal and has no rectangle at all.
    /// </remarks>
    public bool IsCrossed { get; init; }

    /// <summary>
    /// True when the frame is a straight line across its own rectangle rather than a box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one preset shape whose outline is not the rectangle it is anchored by, and the commonest
    /// drawing in a form: a rule, a strike across a block, the cross over an unused half of a
    /// certificate. It has no area, so it has neither a fill nor a rectangular border — it is stroked
    /// corner to corner in <see cref="BorderColour"/> at <see cref="BorderWidth"/>.
    /// </para>
    /// <para>
    /// A flag rather than a shape-geometry model, because that is the shape of the answer here: every
    /// other preset really is drawn inside its rectangle, and the general evaluator that would draw the
    /// rest of them is a separate piece of work. Drawing this one as a box is not a small error —
    /// its fill defaults to opaque white, so it hides the text it was drawn over.
    /// </para>
    /// </remarks>
    public bool IsLine { get; init; }

    /// <summary>
    /// True when a line frame runs from its bottom-left corner to its top-right rather than from its
    /// top-left to its bottom-right.
    /// </summary>
    /// <remarks>
    /// Mirroring once turns one diagonal into the other and mirroring twice turns it back, so this is
    /// the <em>exclusive or</em> of the shape's two flip flags rather than either of them. A cross is
    /// two of these shapes over one rectangle, distinguished by nothing else.
    /// </remarks>
    public bool IsLineMirrored { get; init; }

    /// <summary>
    /// Whether the line runs from its far end to its near one — which only an arrowhead can see.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:xfrm/@flipH</c> mirrors a shape about its own centre, and for a line that means the
    /// same segment traversed the other way. <see cref="IsLineMirrored"/> — the exclusive-or of the
    /// two flips — already picks the right <em>diagonal</em>, so nothing about the ink depended on
    /// the direction and it was never carried.
    /// </para>
    /// <para>
    /// An arrowhead depends on it entirely. The organogram templates join their boxes with
    /// horizontal connectors that carry a tail arrow, are flipped horizontally, and are then turned
    /// through 270° — so the arrow the reference draws pointing <em>down</em> came out pointing up,
    /// on every one of them. The rotation was right and the flip was the missing half.
    /// </para>
    /// </remarks>
    public bool IsLineReversed { get; init; }

    /// <summary>True when the frame holds a picture rather than text.</summary>
    /// <remarks>
    /// Separate from <see cref="Image"/> and <see cref="Vector"/>, because they answer different
    /// questions. This is what the document <em>declared</em> the frame to be, which is what the wrap
    /// and the extraction tree go by; the others are whether bytes were found and what kind they turned
    /// out to be. A picture whose package part is missing, and a PICT nobody here decodes, both set this
    /// and leave the other two null.
    /// </remarks>
    public bool IsImage { get; init; }

    /// <summary>
    /// The picture the frame holds, still in the bytes the file stored, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// Built with <see cref="RasterImage.Encoded"/> and never decoded here: a reader that decoded would
    /// pull a codec into the extraction path, which the layering forbids and which is the reason the IR
    /// carries encoded bytes at all. Whichever backend wants pixels asks <c>RasterImageDecoder.Ensure</c>
    /// for them, and one that only wants to pass a JPEG through to <c>DCTDecode</c> never decodes at all.
    /// </remarks>
    public RasterImage? Image { get; init; }

    /// <summary>
    /// The vector picture the frame holds — an SVG, a WMF, an EMF or an EMF+ — or null when it holds
    /// none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A display list rather than pixels, and it needed nothing in <c>XRay.Office.Core</c>:
    /// <c>VectorImage</c> already is the abstraction a frame wants — <c>Draw(IDrawingSink, DocRect)</c>
    /// plus an intrinsic size, immutable and replayable — and the layering already permits this library
    /// to name it. A Core interface would have had those two members and one implementation.
    /// </para>
    /// <para>
    /// <strong>Not decoded until something draws.</strong> See <see cref="FramePicture"/> for the
    /// measurement that decided it; RTF and DOC read their pictures on the extraction path, where a
    /// second of font resolution would be paid by a caller that only wanted the words.
    /// </para>
    /// <para>
    /// <see cref="Image"/> may be set beside this, and means the raster fallback of a DrawingML
    /// <c>svgBlip</c> — what a consumer that cannot read SVG would have shown. Nothing else sets both.
    /// </para>
    /// </remarks>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>
    /// How much of the picture each edge throws away, or <see cref="PictureCropFractions.None"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Applied where the frame is placed, not where it is read.</strong> A crop draws the
    /// whole picture into a rectangle <em>larger</em> than the frame and clips to the frame, and
    /// the frame's rectangle is <see cref="PlacedFrame.Area"/> — resolved by <c>FrameLayout</c>
    /// against the anchor, the origin and the alignment, none of which a reader has. So
    /// <see cref="PageDrawing"/> does both halves together; see <see cref="FramePicture.Crop"/>.
    /// </para>
    /// <para>
    /// Set on the <c>.doc</c> path from Escher properties 256–259. The other three front ends
    /// state a crop too — <c>a:srcRect</c> in DOCX, <c>fo:clip</c> in ODF, <c>\piccropl</c> and its
    /// siblings in RTF — and none of them is read yet.
    /// </para>
    /// </remarks>
    public PictureCropFractions Crop { get; init; }

    /// <summary>
    /// The chart the frame holds, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The third thing a frame's rectangle can be filled with, beside <see cref="Image"/> and
    /// <see cref="Vector"/>, and it is a <em>model</em> rather than a picture: the marks are composed
    /// into the rectangle at drawing time by <c>XRay.Office.Core.Charts</c>, exactly as a slide's and a
    /// sheet's are. Nothing about the wrap depends on it, which is why it sits beside the picture
    /// rather than replacing it — a chart frame whose part could not be read still reserves its room.
    /// </para>
    /// <para>
    /// A DOCX states one as a <c>w:drawing</c> whose <c>a:graphicData</c> names the chart namespace and
    /// carries a relationship to a <c>c:chartSpace</c> part; an ODT as a <c>draw:frame</c> holding a
    /// <c>draw:object</c> whose sub-document root is a <c>chart:chart</c>. Both arrive here as one
    /// <see cref="ChartPlot"/>, so <c>PageDrawing</c> has one case rather than two.
    /// </para>
    /// </remarks>
    public ChartPlot? Chart { get; init; }

    /// <summary>
    /// The family a chart's labels are set in, or null for the drawing code's own default.
    /// </summary>
    /// <remarks>
    /// Beside <see cref="Chart"/> rather than inside it, because <see cref="ChartPlot"/> carries type
    /// <em>sizes</em> and no family — the decks and workbooks it was built for each have one obvious
    /// answer and Writer does not. Measured with <c>pdffonts</c> on LibreOffice's own PDFs:
    /// <c>chart2/qa/extras/data/odt/chart.odt</c> draws its chart in Liberation Sans and
    /// <c>docx/chart.docx</c> draws the same chart in Carlito, because an OOXML chart's text takes the
    /// theme's minor latin face and an ODF chart's takes the office default. Measuring both in one face
    /// leaves every label the wrong width, which moves the plot area rather than only the ink.
    /// </remarks>
    public string? ChartFontFamily { get; init; }

    /// <summary>
    /// True when the frame is not a drawing at all, but a portion the text layer paints itself.
    /// </summary>
    /// <remarks>
    /// One thing wears this: a form check box. Writer strokes it in
    /// <c>SwTextPaintInfo::DrawCheckBox</c> (<c>sw/source/core/text/inftxt.cxx</c>) as part of painting
    /// the line, and it never becomes an <c>SdrObject</c>; it is a <see cref="PageFrame"/> here only
    /// because that is the vehicle this model has for something that takes room on a line and draws
    /// geometry. The distinction is not cosmetic — everything that <em>does</em> go through the draw
    /// layer inherits its inclusive rectangle, which is a twip taller than the object; see
    /// <c>PageContent.InlineObjects</c>.
    /// </remarks>
    public bool IsTextPortion { get; init; }

    /// <summary>What the frame was called in the document, for diagnostics.</summary>
    public string? Name { get; init; }
}

/// <summary>
/// A frame after it has been given a rectangle on a page.
/// </summary>
/// <param name="Frame">What was placed.</param>
/// <param name="Area">Where its <em>text</em> went, in page coordinates.</param>
/// <param name="Content">Its own text laid out inside that rectangle, or null when it has none.</param>
/// <remarks>
/// Two rectangles rather than one, because an inline drawing genuinely has two — see
/// <see cref="Ink"/>. They are equal for every frame that states no top effect extent, which is
/// nearly all of them.
/// </remarks>
public sealed record PlacedFrame(PageFrame Frame, DocRect Area, PlacedFlow? Content = null)
{
    /// <summary>
    /// Where the frame's own drawing — its fill, its outline, its picture and its chart — is
    /// painted, which is not always where its text is laid out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>wp:inline</c> drawing's line box is grown by <c>wp:effectExtent</c> on all four sides
    /// and the drawing then sits <em>inside</em> that box. Across, the whole object moves by the
    /// left edge and both halves follow it, so <see cref="PageFrame.InlineOffset"/> carries that in
    /// the frame's own position. Down, the two halves of LibreOffice part company: the draw shape's
    /// fill and outline are painted at the outer top <em>plus</em> the top extent, while a shape
    /// carrying a <c>wps:txbx</c> lays its text out at the outer top regardless, because
    /// <c>SwTextBoxHelper</c> never carries the offset
    /// <c>SwAsCharAnchoredObjectPosition::CalcPosition</c> applied
    /// (<c>sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx</c>:129-133).
    /// </para>
    /// <para>
    /// One rectangle cannot be in two places, which is why there are two. Measured in
    /// <c>dotnet/probes/words-inline-shape-ink/</c> on a 144 x 50.4 pt inline drawing between two
    /// 12 pt lines, both installed references identical on every row — the fill's own band top and
    /// the <c>INSIDE</c> run of a text box, in PDF points from the page top:
    /// </para>
    /// <list type="table">
    ///   <item><term>no extent</term><description>fill 85.75, <c>INSIDE</c> 104.66</description></item>
    ///   <item><term><c>t</c> 27432 (2.16 pt)</term><description>fill <b>88.00</b></description></item>
    ///   <item><term><c>t</c> 91440 (7.2 pt)</term><description>fill <b>93.00</b></description></item>
    ///   <item><term><c>t</c> 137160 (10.8 pt)</term><description>fill <b>96.50</b>, <c>INSIDE</c> <b>104.66</b> — unmoved</description></item>
    ///   <item><term><c>b</c> 137160</term><description>fill 85.75 — the bottom edge moves nothing</description></item>
    /// </list>
    /// <para>
    /// It is the drawing and not the rectangle: an <c>ellipse</c> preset's curves move by the same
    /// 10.75 pt, and so does a picture that keeps its shape by declaring an <c>a:effectLst</c>.
    /// </para>
    /// </remarks>
    public DocRect Ink
    {
        get
        {
            Length down = Frame.InlineInkOffset;
            return down == Length.Zero
                ? Area
                : new DocRect(Area.X, Area.Y + down, Area.Width, Area.Height);
        }
    }
}
