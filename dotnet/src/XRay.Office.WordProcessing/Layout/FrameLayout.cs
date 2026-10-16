using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Layout;

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
    /// <param name="bodyArea">
    /// The page's body rectangle as it was actually laid out — <see cref="LaidOutPage.BodyArea"/>, which
    /// is <paramref name="geometry"/>'s text area with whatever a running head or foot that outgrew its
    /// reserved room did to it. Only <see cref="FrameVerticalOrigin.PageMargin"/> uses it, and it
    /// defaults to the stated text area, which is what the two are on a page whose furniture fits.
    /// </param>
    /// <param name="capturesOnPage">
    /// Whether a content-anchored frame is pulled back inside the page —
    /// <c>SwAnchoredObjectPosition::ImplAdjustVertRelPos</c>. See
    /// <see cref="PaginationOptions.CapturesAnchoredObjectsOnPage"/> for which formats set it.
    /// </param>
    /// <param name="capturesWrappedObjects">
    /// Whether an object the <c>DoNotCaptureDrawObjsOnPage</c> flag does <em>not</em> exempt is
    /// captured anyway — see <see cref="PaginationOptions.CapturesWrappedObjects"/>, which is where
    /// the rule and its C++ seats are written out. Ignored when <paramref name="capturesOnPage"/> is
    /// set, since that already captures everything.
    /// </param>
    /// <param name="narrowsCaptureToBody">
    /// Whether the area a captured object is held inside narrows from the sheet to the page's body —
    /// DOCX <c>compatibilityMode</c> 15, see
    /// <see cref="PaginationOptions.NarrowsCaptureToBody"/>.
    /// </param>
    /// <param name="anchorPlace">
    /// Where the anchor sits on the page, which decides two of the capture's own conditions. See
    /// <see cref="FrameAnchorPlace"/>.
    /// </param>
    /// <param name="disablesOffPagePositioning">
    /// Writer's <c>DisableOffPagePositioning</c>, which exempts a wrap-through object from
    /// <c>SwFlyFreeFrame::CheckClip</c> — see
    /// <see cref="PaginationOptions.DisablesOffPagePositioning"/> and <see cref="Squeezed"/>.
    /// </param>
    public static DocRect Place(
        PageFrame frame,
        PageGeometry geometry,
        DocRect column,
        Length anchorTop,
        bool rightHandPage = true,
        Length? anchorLineTop = null,
        DocRect? bodyArea = null,
        bool capturesOnPage = true,
        bool capturesWrappedObjects = false,
        bool narrowsCaptureToBody = false,
        FrameAnchorPlace anchorPlace = FrameAnchorPlace.Body,
        bool disablesOffPagePositioning = false)
    {
        ArgumentNullException.ThrowIfNull(frame);
        ArgumentNullException.ThrowIfNull(geometry);

        DocRect page = new(Length.Zero, Length.Zero, geometry.Size.Width, geometry.Size.Height);
        DocRect text = geometry.TextArea;
        DocRect body = bodyArea ?? text;

        // The two margin *bands* are their own alignment areas rather than the page — `PAGE_LEFT` is
        // `GetLeftMargin(page)` wide from the sheet's left edge and `PAGE_RIGHT` is
        // `GetRightMargin(page)` wide from `GetPrtRight(page)`
        // (`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:769-788). Only the y and the
        // height of these rectangles are unused, so they carry the page's.
        //
        // Measured in `dotnet/probes/frame-area-r85/`, a 40 x 20 pt band on A4 with 72 pt margins.
        // Band left edge in points, both installed references identical on every row:
        //
        //   fixture                    reference   ours before
        //   h-leftmargin-0                  0.00          0.00   <- an offset agrees by accident
        //   h-leftmargin-centre            16.00        277.75
        //   h-leftmargin-right             32.00        555.25
        //   h-rightmargin-0               523.25          0.00
        //   h-rightmargin-centre          539.25        277.75
        //   h-outsidemargin-0              72.00          0.00
        //
        // `h-leftmargin-0` is why reading only offsets would have missed this: the band and the page
        // share a left edge, so the defect is invisible until something is aligned in it.
        DocRect horizontal = frame.HorizontalOrigin switch
        {
            FrameHorizontalOrigin.Page => page,
            FrameHorizontalOrigin.PageMargin => text,
            FrameHorizontalOrigin.Column => column,
            FrameHorizontalOrigin.LeftMarginArea =>
                new DocRect(page.X, page.Y, text.X - page.X, page.Height),
            FrameHorizontalOrigin.RightMarginArea =>
                new DocRect(text.Right, page.Y, page.Right - text.Right, page.Height),
            _ => column,
        };

        Length lineTop = anchorLineTop ?? anchorTop;

        // An object that follows the text flow is positioned against a different frame, and for a body
        // anchor that frame is the page *body* rather than the page.
        //
        // One function decides it for every vertical question below:
        // `SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame`
        // (`sw/source/core/objectpositioning/environmentofanchoredobject.cxx`:64-95) answers
        // `FindPageFrame()` when the object does not follow the text flow, and otherwise walks up from the
        // anchor to the first cell, fly, header, footer, footnote, **page body** or page frame. So for a
        // paragraph in the body the answer is the page body frame, whose top is `w:pgMar/@w:top` below the
        // sheet's.
        //
        // Three things follow, all in the offset arm of
        // `SwToContentAnchoredObjectPosition::CalcPosition`
        // (`tocntntanchoredobjectposition.cxx`:533-620) and in `ImplAdjustVertRelPos`:
        //
        //   * a `PAGE_FRAME` offset is measured from that frame's top (:590-596), which is the whole of
        //     the 71.6 pt below;
        //   * the object is captured at all, because `mbDoNotCaptureAnchoredObj` is
        //     `bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE` — see
        //     `FrameObjectKind` — and the middle term is now false; and
        //   * it is captured in the **page**, not in the body: `ImplAdjustVertRelPos`:544-573 takes the
        //     page frame's own rectangle whenever the anchor is outside a table, and its
        //     `compatibilityMode` 15 narrowing needs `rPageAlignLayFrame.IsPageFrame()`, which a body
        //     frame is not.
        //
        // **`bCheckBottom = !DoesObjFollowsTextFlow()` is NOT a fourth, and the corpus refuted it.** That
        // expression is real, at `tocntntanchoredobjectposition.cxx`:457 in the *alignment* arm and at
        // :678 and :718 in the offset one — and the offset arm has a third call, at :810-813, which
        // **omits the argument** and so takes its `= true` default (`anchoredobjectposition.hxx`:185-192).
        // That is the path an offset which does not fit its upper's print area takes, under the comment
        // *"do not follow text flow respectively align at 'page areas', but stay inside given
        // environment"*, and it is the one a page-relative chart reaches.
        // `027_Unit_Circle_Chart_Graphical_Chart` is the witness: 470.30 pt of chart at a body base of
        // 156.95 + 111.65 would reach 738.90 on a 595.30 pt landscape page, and 26.2.4.2 draws its top at
        // **125.00**, which is `595.30 - 470.30` to the hundredth. Skipping the bottom correction drew it
        // at 268.60 and took that document from 9.48 to 28.42 `diff%`.
        //
        // The horizontal is untouched, and that is a property of the same file rather than an omission:
        // `GetHoriEnvironmentLayoutFrame`'s walk stops at a cell, a fly or a page and has no body frame in
        // it (:34-61), so a body anchor's horizontal environment is the page either way.
        //
        // Measured on `028_Unit_Circle_Chart_Optimized_Graph.docx`, whose chart is an embedded object with
        // `layoutInCell="1"` and a `relativeFrom="page"` `wp:posOffset` of 175.52 pt on a page whose
        // `w:pgMar/@w:top` is 1440 twips: 26.2.4.2 draws it **71.6 pt lower than the offset states**, which
        // is that margin. `probes/ofpie-r190/variants.py`'s ten one-attribute rewrites are where that is
        // measured: `w:top="0"` renders the chart where an unfollowing frame goes and `w:top="2880"` moves
        // it by the extra 72 pt, so the base tracks that margin one for one; `margin` renders identically
        // to `page`; and an offset of -200 pt is drawn at 0.36 — the **sheet's** own top rather than the
        // body's, which is the third bullet above. That table's `layoutInCell="0"` row is the one to
        // re-measure before it is built on; `probes/pagev-r192/results.md` §4 says why.
        //
        // `probes/pagev-r192/fixture.py` is the experiment that separates the object kind from everything
        // else: two minimal DOCX identical but for whether the anchored object is a `wps:wsp` or a chart.
        // `probes/ofpie-r190/page-anchor-fixture.py` built the shape half alone, 26.2.4.2 drew it at the
        // sheet's top edge, and that refuted "page means margin" without explaining the document.
        //
        // Only a body anchor is modelled. The walk's other stops are a header or footer frame, a footnote
        // and a table cell, and this tree has a rectangle for none of them here — `FrameAnchorPlace` is as
        // far as it goes. A header-anchored embedded object therefore keeps the page, which is what it had.
        bool followsTextFlow = frame.FollowsTextFlow && anchorPlace == FrameAnchorPlace.Body;

        // The margin-relative area runs from the *header frame's bottom to the footer frame's top*, not
        // from `w:top` to `w:bottom`. `RelOrientation::PAGE_PRINT_AREA` takes the page's print area and
        // then walks the page frame's lowers, subtracting each header frame's height from the area and
        // adding it to the offset and subtracting each footer frame's height —
        // `sw/source/core/objectpositioning/anchoredobjectposition.cxx`:336-361. Since Writer's DOCX
        // import makes the page's own top margin `w:header` and gives the header frame `w:top - w:header`
        // as a *dynamic* height (`dmapper/PropertyMap.cxx`:1148), the two coincide exactly while the
        // running heads fit the room those margins reserve, and part company when one outgrows it — which
        // is the same quantity `Paginator.PushedDownBy` and `PulledUpBy` already apply to the body, so
        // the body's own rectangle is the answer.
        //
        // Measured in `dotnet/probes/words-margin-print-area/`, a 200 x 50 pt band centred vertically
        // against the margin on A4 with `w:top` = `w:header` = 708 twips. Band centre, in points, both
        // installed references identical on every row:
        //
        //   fixture       reference   body top   ours before
        //   none            402.62      35.4       402.75
        //   hdr-empty       409.38      48.9       402.75
        //   hdr-3line       423.38      76.8       402.75
        //   hdr-roomy       435.00     100.0       435.00     <- reserved room not exceeded
        //   ftr-3line       400.25      35.4       402.75
        //   hdr3-ftr3       421.00      76.8       402.75
        //
        // `hdr-roomy` is the control that says it is the *frame* and not the header's content: a header
        // of one line inside a 64.6 pt reservation moves nothing. Horizontally there is no such rule —
        // the header/footer walk in the horizontal case is guarded by `aRectFnSet.IsVert()`
        // (the same file, :824), so it applies to a vertical writing mode only.
        //
        // The two vertical margin bands are bounded by the *body*, so they are the complement of
        // `PageMargin` above and move with a running head that outgrew its reserved room in the same
        // way. `PAGE_PRINT_AREA_TOP` runs from the sheet's top to the body's top — see
        // `FrameVerticalOrigin.TopMarginArea` for why the layout's own switch does not say so — and
        // `PAGE_PRINT_AREA_BOTTOM` from the body's bottom to the sheet's.
        //
        // Same probe, a 60 x 20 pt band, both installed references identical on every row:
        //
        //   fixture                    reference   ours before
        //   v-topmargin-0                   0.00         72.00
        //   v-topmargin-centre             26.00        411.00
        //   v-topmargin-bottom             52.00        750.00
        //   v-bottommargin-0              770.00         72.00
        //   v-bottommargin-centre         796.00        411.00
        //   v-bottommargin-bottom         821.75        750.00
        DocRect vertical = frame.VerticalOrigin switch
        {
            // `PAGE_FRAME` against the vertical environment frame, which is the body for an object that
            // follows the text flow. `PAGE_PRINT_AREA` needs no such branch: the C++ takes
            // `rPageAlignLayFrame.getFramePrintArea()` and only reaches for
            // `PrtWithoutHeaderAndFooter()` when that frame is the page (:597-607, :336-342), so a body
            // frame's own area is the answer either way and `body` already is it.
            //
            // `PAGE_PRINT_AREA_TOP` — `TopMarginArea` below — *does* share `PAGE_FRAME`'s branch in both
            // places the C++ decides this (:590-591 and `anchoredobjectposition.cxx`:327-334), so the same
            // substitution is its rule too, and it is deliberately left: it would collapse the band this
            // tree measured against 26.2.4.2 (`probes/frame-area-r85`, six fixtures) to nothing, that
            // measurement was taken on a *shape* and so says nothing about the following case, and the
            // corpus cannot decide it — `census-embedded.py` finds **5 embedded-object anchors in 5
            // documents, stating `page` twice and `margin` three times and `topMargin` not once**.
            FrameVerticalOrigin.Page => followsTextFlow ? body : page,
            FrameVerticalOrigin.PageMargin => body,
            FrameVerticalOrigin.TopMarginArea =>
                new DocRect(page.X, page.Y, page.Width, body.Y - page.Y),
            FrameVerticalOrigin.BottomMarginArea =>
                new DocRect(page.X, body.Bottom, page.Width, page.Bottom - body.Bottom),
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

        Length placedX = x + frame.GroupOffset.X;
        Length placedY = y + frame.GroupOffset.Y;

        // Whether this frame is captured at all, and in what.
        //
        // `mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow &&
        // DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE` (`anchoredobjectposition.cxx`:125-144), and `bConsidered`
        // is `bWrapThrough && !bTextBox` for a fly and `bWrapThrough || !bTextBox` for a draw object —
        // see `FrameObjectKind`. So under DOCX's flag a picture and a shape carrying a text box are
        // captured unless they wrap through, and a shape with no text box never is.
        //
        // `mbFollowTextFlow` is the middle term and it is the *embedded object's* escape from the whole
        // product: false for a shape and for a picture, because both of `GraphicImport`'s writes of
        // `PROP_FOLLOW_TEXT_FLOW` are gated on the anchor being inside a table
        // (`GraphicImport.cxx`:1316-1318 and :1859-1861) and the pool default is false
        // (`sw/source/core/bastyp/init.cxx`:437) — and true for a chart or an OLE object whatever its
        // anchor, since `DomainMapper_Impl.cxx`:9792 sets it from `layoutInCell` with no such guard. See
        // `PageFrame.FollowsTextFlow` and `followsTextFlow` above.
        //
        // A *table cell's* own capture is still not modelled: inside a table the object is held in its
        // cell (`anchoredobjectposition.cxx`:576-591), an area this tree does not have. 552 of the
        // corpus's 6055 positioned objects are inside a `w:tbl`, in 40 of 272 documents;
        // `probes/words-seat-r94/anchor-census.txt`.
        bool considered = frame.Wrap == TextWrap.Through || frame.ObjectKind == FrameObjectKind.Shape;
        bool captured = capturesOnPage
            || (capturesWrappedObjects && (!considered || followsTextFlow));

        // And the area it is captured in. `ImplAdjustVertRelPos`:562-573 narrows it from the page to
        // the page *body* frame under its own comment — "Instead of using the top of the page as the
        // vertical limit, DOCX compatibilityMode 15 started to use the text body as the vertical limit
        // for most paragraph or line-oriented anchored non-wrapthrough objects" — for every vertical
        // relation except `PAGE_FRAME` and `PAGE_PRINT_AREA`, which are `FrameVerticalOrigin.Page` and
        // `PageMargin` here. The two margin *bands* are `PAGE_PRINT_AREA_TOP` and `_BOTTOM`, which are
        // different enumerators and are narrowed.
        //
        // The narrowing also needs a body frame to narrow to: `mpAnchorFrame->FindBodyFrame()` walking
        // up to a page body frame whose upper is this page (:568-573). A header, a footer and a
        // footnote anchor have none, so their objects stay bounded by the sheet — which is what keeps
        // this off `b053-19`'s and `Case-Study-Heathrow-Airport`'s header pictures, the two documents
        // the previous round's wide rule cost the most.
        //
        // The horizontal capture never narrows: `ImplAdjustHoriRelPos` (:674-722) takes the page
        // frame's own rectangle with no such branch.
        //
        // And an object following the text flow is not narrowed either, which is the opposite of what the
        // name suggests: the narrowing sits inside `if (!bVert && bCompat15 && ... &&
        // rPageAlignLayFrame.IsPageFrame() && ...)` (:562-566), and for such an object that frame is the
        // page *body* rather than the page, so the test fails and the area stays the sheet. The branch it
        // is in was already entered, because the alternative needs `bFollowTextFlow && IsInTab()`
        // (:544-546) and this tree models no in-table capture.
        DocRect verticalArea = narrowsCaptureToBody
            && anchorPlace == FrameAnchorPlace.Body
            && !followsTextFlow
            && frame.VerticalOrigin is not (FrameVerticalOrigin.Page or FrameVerticalOrigin.PageMargin)
                ? body
                : page;

        // tdf#123002: a header- or footer-anchored object whose top has already passed the area's
        // bottom is returned unadjusted (:637-647), because an anchor in the furniture may legitimately
        // put its object on another page and pulling it back would strand it. The guard there is
        // `DisableOffPagePositioning`, which `WriterFilter.cxx`:333 sets one line below the
        // `DoNotCaptureDrawObjsOnPage` this branch is reached under — so for DOCX the two travel
        // together. The RTF reader captures unconditionally instead (`probes/rtf-shape-r73`, 21 probes,
        // all body anchors), so it does not reach this and the deviation stays where that round
        // measured it.
        bool offPage = !capturesOnPage
            && anchorPlace == FrameAnchorPlace.Furniture
            && placedY > verticalArea.Bottom;

        return Squeezed(
            frame,
            page,
            new DocRect(
                captured ? CapturedOnPageAcross(frame, page, placedX) : placedX,
                captured && !offPage ? CapturedOnPage(frame, verticalArea, placedY) : placedY,
                frame.Size.Width,
                frame.Size.Height),
            disablesOffPagePositioning);
    }

    /// <summary>
    /// A frame bigger than its page, cut down to the page — and proportionally when what it holds is
    /// a picture or a chart.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwFlyFreeFrame::CheckClip</c> (<c>sw/source/core/layout/flylay.cxx</c>:471-660), called from
    /// <c>MakeAll</c> (:251) once the frame's position and size are both valid, on <em>every</em> free
    /// fly and independently of the capture <see cref="CapturedOnPageAcross"/> models. Its clip
    /// rectangle is the anchor's page frame — <c>CalcClipRect</c> (:1232-1275) takes
    /// <c>pClipFrame-&gt;getFrameArea()</c> for a fly at content whenever
    /// <c>CONSIDER_WRAP_ON_OBJECT_POSITION</c> is set, which every Word import sets, and intersects it
    /// with the root's, which changes nothing.
    /// </para>
    /// <para>
    /// <strong>The rule reduces to "bigger than the page".</strong> The C++ first gives up the
    /// position — <c>SetPosX(max(aClip.Left(), nClipRig − aFrm.Width()))</c> (:530-534) and the same
    /// down the page — and only squeezes if the frame still does not fit, which for a frame that
    /// <em>fits</em> the page it never does. So the only frames whose size this changes are the ones
    /// wider or taller than the sheet, and for them the move lands the frame flush with the page's own
    /// edge, which is where the squeeze then starts from. Modelling it this way rather than as a move
    /// plus a test keeps every frame that fits exactly where it was.
    /// </para>
    /// <para>
    /// <strong>Proportional for a picture or an OLE object, and only for them</strong> (:598-627):
    /// <c>Lower()-&gt;IsNoTextFrame()</c>, which is a <c>SwGrfNode</c> or a <c>SwOLENode</c> and is not
    /// a text frame's text. When both axes are over, <em>the bigger change is the relevant one</em>
    /// (:602-612) — the other axis is restored and then recomputed from the ratio, so the aspect is
    /// kept.
    /// </para>
    /// <para>
    /// <strong>Measured at 26.2.4.2 through the flat ODT, which prints the resolved model without
    /// rendering anything</strong> — and it is visible there because :638-648 writes the squeezed size
    /// back into the frame format for an OLE node. On
    /// <c>023_Unit_Circle_Chart_Circular_Percentage</c>, whose chart states 682.10 x 493.50 pt on a
    /// 595.30 x 841.89 pt page, one <c>wp:extent</c> changed at a time:
    /// </para>
    /// <list type="table">
    ///   <item><term>682.10 x 493.50 (as authored)</term><description><b>595.30 x 430.70</b></description></item>
    ///   <item><term>708.66 x 157.48 — width alone over</term><description><b>595.30 x 132.29</b></description></item>
    ///   <item><term>314.96 x 905.51 — height alone over</term><description><b>292.79 x 841.89</b></description></item>
    ///   <item><term>708.66 x 905.51 — both, width by more</term><description><b>595.30 x 760.65</b></description></item>
    ///   <item><term>393.70 x 236.22 — neither</term><description>unchanged</description></item>
    /// </list>
    /// <para>
    /// <strong>The flat ODT can only see an OLE object, so the other three cases were rendered</strong>
    /// (<c>probes/chart-fit-r97/nodekind.py</c>), one attribute changed at a time on the same
    /// document and the drawn width read back out of 26.2.4.2's PDF. The write-back at :638-648 is
    /// what puts a squeeze in the flat ODT and it is OLE-only, but the squeeze itself is not:
    /// </para>
    /// <list type="table">
    ///   <item><term>the chart, <c>wrapSquare</c></term><description>682.10 -> <b>595.30 x 430.70</b></description></item>
    ///   <item><term>the chart, wrap alone changed to <c>wrapNone</c></term><description><b>not squeezed</b> — its pie grows by 682.10/595.30</description></item>
    ///   <item><term>the picture given the chart's extent, <c>wrapNone</c> as authored</term><description><b>not squeezed</b> — drawn 682.10 x 493.50, running 334 pt off the sheet</description></item>
    ///   <item><term>the same picture, wrap alone changed to <c>wrapSquare</c></term><description>682.10 -> <b>595.30 x 430.70</b></description></item>
    /// </list>
    /// <para>
    /// So it is the wrap that decides and not the node kind, which is the escape below; and a graphic
    /// node is squeezed exactly as an OLE node is. The corpus carries the same rule unprompted:
    /// <c>fleetfastfacts16nov2023.docx</c> anchors a 606.90 x 231.60 pt picture on a 595.30 pt page
    /// and 26.2.4.2 draws it at <b>595.30 x 227.20</b>, flush with the left edge.
    /// </para>
    /// <para>
    /// <strong>An as-character object is not a free fly and is never squeezed.</strong>
    /// <c>SwFlyInContentFrame</c> derives from <c>SwFlyFrame</c> and not from <c>SwFlyFreeFrame</c>
    /// (<c>sw/source/core/inc/flyfrms.hxx</c>:212 against :150), whose method this is. Measured:
    /// <c>tibs_guidelines_2.docx</c> holds a 686.20 pt <c>wp:inline</c> picture on a 612 pt page and
    /// 26.2.4.2 draws all 686.20 pt of it, and <c>PES-Technical-Report-Template_Jan_2019.docx</c>'s
    /// 612.50 x 792.70 pt one on a 612 x 792 pt page likewise. Nothing is needed here for that:
    /// <see cref="Place"/> is not called for an as-character frame at all — those hang on a line in
    /// <c>HangInline</c> — so this method only ever sees a free fly.
    /// </para>
    /// <para>
    /// <strong>Three of the C++'s own sub-conditions are not modelled</strong>, each of which only
    /// stops the frame being *moved* first and so squeezes a frame that fits: a fly in a header
    /// (:497-503), a fly carrying anchored objects of its own, and a fly inside a table. And the
    /// escape is <c>SwAnchoredObject::IsDraggingOffPageAllowed</c>
    /// (<c>anchoredobject.cxx</c>:790-801), which needs <c>DisableOffPagePositioning</c> — whose only
    /// setter anywhere in <c>sw/</c> is <c>sw/source/writerfilter/filter/WriterFilter.cxx</c>:333, the
    /// <em>OOXML</em> filter's <c>setTargetDocument</c>, and <strong>not</strong> RTF's:
    /// <c>RtfFilter::setTargetDocument</c> (<c>RtfFilter.cxx</c>:191-195) assigns the document and
    /// sets no property at all, and <c>filter/source/config/fragments/filters/Rich_Text_Format.xcu</c>
    /// names <c>com.sun.star.comp.Writer.RtfFilter</c> as its service — <em>and</em> a wrap-through
    /// object.
    /// </para>
    /// </remarks>
    /// <param name="frame">The frame, for what it holds.</param>
    /// <param name="page">The page rectangle, which is the clip rectangle.</param>
    /// <param name="placed">Where the frame landed, at the size the file states.</param>
    /// <param name="disablesOffPagePositioning">The document setting the escape needs.</param>
    private static DocRect Squeezed(
        PageFrame frame, DocRect page, DocRect placed, bool disablesOffPagePositioning)
    {
        // A *drawing object* is not a fly and never reaches `SwFlyFreeFrame::CheckClip` at all: it
        // is an `SwAnchoredDrawObject`, whose `MakeObjPos` adjusts the position and nothing else.
        //
        // **That includes a shape carrying a text box**, which `CapturesAnchoredObjectsOnPage`'s
        // `bConsidered` treats as a fly and this must not. `SwTextBoxHelper` pairs a *draw* format
        // with a fly format; the fly holds the text and goes through `CheckClip`, and the shape —
        // its fill, its outline, the rectangle that is actually drawn — stays a draw object. A
        // `PageFrame` here is the drawing, so both shape kinds are exempt.
        //
        // Measured, because reading it the other way regressed three corpus documents before it
        // was caught. `ABCD-WB-08-00 Weight and Balance Report`, `ABCD-FE-01-00 Flight Envelope`
        // and `ABCD-SDE-23-00 - Avionic System Description` each state a `wrapSquare`
        // `wp:anchor`/`wps:txbx` banner in a running head — 739.25 x 56.85 pt on a 595.30 pt page
        // for the first two, 839.80 x 56.85 for the third — and 26.2.4.2 draws all of
        // 739.2 and 839.8 of them, running off the sheet. The same documents' *pictures* are cut:
        // `b053-19.docx`'s header picture is 612.5 pt on a 612 pt page and the reference draws it
        // 612.0 x 87.75, which is this method's own arithmetic.
        //
        // Only the DOCX reader tells the kinds apart (`FrameObjectKind`), so an ODF or RTF custom
        // shape wider than its page is squeezed here where the reference would leave it — the same
        // gap `CapturesWrappedObjects` records, in the same place.
        if (frame.ObjectKind is FrameObjectKind.Shape or FrameObjectKind.TextBoxShape) return placed;

        if (disablesOffPagePositioning && frame.Wrap == TextWrap.Through) return placed;

        Length width = placed.Width;
        Length height = placed.Height;
        bool across = width > page.Width;
        bool down = height > page.Height;
        if (!across && !down) return placed;

        Length cut = across ? page.Width : width;
        Length shortened = down ? page.Height : height;

        // A picture's and a chart's aspect is kept; a text frame's two axes are cut independently.
        if (frame.Chart is not null || frame.IsImage
            || frame.Image is not null || frame.Vector is not null)
        {
            // Both over: the bigger change decides, and the other axis goes back to what it was
            // before the ratio is applied to it.
            if (across && down)
            {
                if (width - cut > height - shortened) shortened = height;
                else cut = width;
            }

            if (cut != width && width.Emu != 0)
            {
                shortened = height * ((double)cut.Emu / width.Emu);
            }
            else if (shortened != height && height.Emu != 0)
            {
                cut = width * ((double)shortened.Emu / height.Emu);
            }
        }

        return new DocRect(
            across ? page.X : placed.X,
            down ? page.Y : placed.Y,
            cut,
            shortened);
    }

    /// <summary>
    /// A content-anchored frame pulled back inside the page it is on.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwAnchoredObjectPosition::ImplAdjustVertRelPos</c>
    /// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:504-667), which is what its
    /// own comment calls <em>"adjust calculated vertical in order to keep object inside 'page'
    /// alignment layout frame"</em>. Two corrections in order: the bottom first —
    /// <c>nTopOfAnch + nAdjustedRelPosY + aObjSize.Height() &gt; aPgAlignArea.Bottom()</c> pulls the
    /// object up until its bottom rests on the area's — and then the top, which pushes it back down if
    /// the first correction has taken it above. A frame taller than the page therefore ends flush with
    /// the top and overflows the bottom, which is the order the C++ applies and not an accident of it.
    /// </para>
    /// <para>
    /// <b>Only a content anchor is captured.</b> <c>SwToLayoutAnchoredObjectPosition</c> — a
    /// <c>FLY_AT_PAGE</c> fly — never calls this at all
    /// (<c>tolayoutanchoredobjectposition.cxx</c>:100-131 sets the position and then only grows the
    /// page in browse mode), so a page-anchored frame may hang off the sheet and is left alone here.
    /// The area is <c>rPageFrame.getFrameArea()</c> whenever
    /// <c>CONSIDER_WRAP_ON_OBJECT_POSITION</c> is set, which every Word import sets, and narrows to the
    /// page <em>body</em> under DOCX <c>compatibilityMode</c> 15 — the caller decides which, see
    /// <see cref="PaginationOptions.NarrowsCaptureToBody"/>.
    /// </para>
    /// <para>
    /// The one escape is <c>SwAnchoredObject::IsDraggingOffPageAllowed</c>
    /// (<c>sw/source/core/layout/anchoredobject.cxx</c>:790-801), which needs
    /// <c>DisableOffPagePositioning</c> <em>and</em> a wrap-through object.
    /// <b>This used to say that no DOC or DOCX import sets that flag, and it is set for two of the
    /// four formats:</b> <c>sw/source/writerfilter/filter/WriterFilter.cxx</c>:333 sets it for every
    /// writerfilter import, one line below the <c>DoNotCaptureDrawObjsOnPage</c> the paragraph above
    /// cites, so it is live for DOCX and RTF and absent from WW8 and ODF. It fires on none of the
    /// twenty-one RTF probes in <c>dotnet/probes/rtf-shape-r73/</c>, two of which are wrap-through,
    /// so what makes it inert there is not yet established.
    /// </para>
    /// <para>
    /// Measured on <c>words/done-013/doc/omrIMInterpretiveGuideLine.doc</c> against 26.2.4.2 with the
    /// tarball's <c>LiberationSansNarrow</c> moved aside, so the two stacks resolve the same faces.
    /// Its <c>COMMENTS AND QUESTIONS</c> block is a WW8 APO — <c>sprmPDyaAbs</c> 100.90 pt from a
    /// paragraph whose top is 666.45 pt down a 792 pt page, 36.00 pt tall — so its stated bottom is
    /// 803.35, <b>11.35 pt below the sheet</b>, and its second line was drawn at a baseline of −0.70.
    /// The reference draws the same block with its bottom flush at 791.95. Every one of page 1's other
    /// 30 lines already agreed to 0.05 pt in both directions; the frame was the only thing that did
    /// not.
    /// </para>
    /// </remarks>
    /// <param name="frame">The frame, for its anchor and its height.</param>
    /// <param name="area">
    /// The rectangle the frame is captured in: the sheet, or the page's body under DOCX
    /// <c>compatibilityMode</c> 15 — see <see cref="PaginationOptions.NarrowsCaptureToBody"/>.
    /// </param>
    /// <param name="y">The position the origin and the offset gave it.</param>
    private static Length CapturedOnPage(PageFrame frame, DocRect area, Length y)
    {
        if (frame.Anchor is not (FrameAnchor.Paragraph or FrameAnchor.Character)) return y;

        Length height = frame.Size.Height;
        if (y + height > area.Bottom) y = area.Bottom - height;
        if (y < area.Y) y = area.Y;

        return y;
    }

    /// <summary>
    /// The same capture across the page, which is the other half of the same rule.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwAnchoredObjectPosition::ImplAdjustHoriRelPos</c>
    /// (<c>sw/source/core/objectpositioning/anchoredobjectposition.cxx</c>:674-721) — <em>"adjust
    /// calculated horizontal in order to keep object inside 'page' alignment layout frame"</em> — sits
    /// beside <c>ImplAdjustVertRelPos</c> and is reached through the same
    /// <c>mbDoNotCaptureAnchoredObj</c> guard (<c>anchoredobjectposition.hxx</c>:268-273). It applies
    /// the two corrections in the same order: the right edge first, so a frame wider than the page ends
    /// flush with the left and overflows the right.
    /// </para>
    /// <para>
    /// A fly gets the same treatment a second time from <c>SwFlyFreeFrame::CheckClip</c>
    /// (<c>sw/source/core/layout/flylay.cxx</c>:471-545), which moves it left until its right edge
    /// rests on the clip rectangle's and never past that rectangle's own left.
    /// </para>
    /// <para>
    /// <b>The area is the page, not its text area.</b> Measured against 26.2.4.2 on fifteen one-shape
    /// RTF probes — five <c>\shpwr</c> values × <em>fits</em>, <em>overflows right</em>,
    /// <em>overflows left</em> — on A4 with a 3139-twip left margin: a 10723-twip box offered
    /// 3730 twips is drawn at <b>1183</b>, which is <c>paperw − width</c> to the twip, and a box offered
    /// −861 is drawn at <b>0</b>. A box that begins entirely off the sheet is moved too. See
    /// <c>dotnet/probes/rtf-shape-r73/results.md</c>.
    /// </para>
    /// </remarks>
    /// <param name="frame">The frame, for its anchor and its width.</param>
    /// <param name="page">The page rectangle, which is the area a frame is captured in.</param>
    /// <param name="x">The position the origin and the offset gave it.</param>
    private static Length CapturedOnPageAcross(PageFrame frame, DocRect page, Length x)
    {
        if (frame.Anchor is not (FrameAnchor.Paragraph or FrameAnchor.Character)) return x;

        Length width = frame.Size.Width;
        if (x + width > page.Right) x = page.Right - width;
        if (x < page.X) x = page.X;

        return x;
    }

    /// <summary>
    /// The frame as tall as its own text, for a frame whose stated height is only a floor.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="PageFrame.GrowsToContent"/> carries the rule and its citations; this is the
    /// arithmetic, in <c>SwFlyFrame::Format</c>'s own order
    /// (<c>sw/source/core/layout/fly.cxx</c>:1549-1570): the content height, raised to the stated
    /// minimum less the insets, raised again to <c>MINFLY</c>, and the insets added back. Doing the
    /// two clamps in the other order would let a frame with a large inset and no content come out
    /// shorter than <c>MINFLY</c>.
    /// </para>
    /// <para>
    /// <strong>This runs before <see cref="Place"/>, and needs to.</strong> The round that first
    /// placed an ODF frame supposed that growing one would mean inverting the order of the two,
    /// because a frame's rectangle is what its content is laid out in. It does not:
    /// <see cref="Place"/> is a pure function of the frame's <em>size</em> and the page and anchor
    /// geometry, and what the content's own layout needs is the frame's <em>width</em>, which every
    /// format states outright. So the height can be measured in a pass that runs first, and only a
    /// rule where placement fed back into measurement would need the inversion.
    /// </para>
    /// <para>
    /// <strong>The inset taken is <see cref="PageFrame.Padding"/> alone, where Writer's <c>nUL</c> is
    /// the padding <em>plus</em> the border width plus the shadow space.</strong> That is deliberate
    /// and it is consistency rather than an approximation of the rule: <c>FrameResolution.Content</c>
    /// insets a frame's text by the padding and nothing else, so taking a wider inset here would
    /// leave the frame taller than the text it was measured from. Of the 59 growing frames in the
    /// 338 converted <c>.odt</c>, 53 declare <c>fo:border="none"</c> and the other five declare
    /// 0.06 pt or 0.74 pt, so the whole of what this leaves out is under a point and a half, on
    /// five frames.
    /// </para>
    /// </remarks>
    /// <param name="frame">The frame as the reader gave it.</param>
    /// <param name="collapsesSpacing">See <see cref="FlowLayouter.LayOut"/>.</param>
    /// <param name="addsCellLineSpacing">See <see cref="PaginationOptions.AddsCellLineSpacing"/>.</param>
    /// <returns>The frame, grown, or exactly the frame given when it does not grow.</returns>
    public static PageFrame Grown(
        PageFrame frame, bool collapsesSpacing = false, bool addsCellLineSpacing = false)
    {
        ArgumentNullException.ThrowIfNull(frame);
        if (!frame.GrowsToContent) return frame;

        Length insets = frame.Padding.Top + frame.Padding.Bottom;
        Length width = frame.Size.Width - frame.Padding.Left - frame.Padding.Right;

        Length inside = frame.Blocks.Count == 0 || width <= Length.Zero
            ? Length.Zero
            : FlowLayouter.HeightOf(
                frame.Blocks, width, 0, collapsesSpacing, addsCellLineSpacing);

        inside = Length.Max(inside, (frame.HeightFloor ?? frame.Size.Height) - insets);
        inside = Length.Max(inside, MinimumFlyHeight);

        Length height = inside + insets;

        return height == frame.Size.Height
            ? frame
            : frame with { Size = new DocSize(frame.Size.Width, height) };
    }

    /// <summary>
    /// The floor under every fly's print area: <c>MINFLY</c>, 23 twips
    /// (<c>sw/inc/swtypes.hxx</c>:59, <em>"Minimal size for FlyFrames"</em>).
    /// </summary>
    private static readonly Length MinimumFlyHeight = Length.FromTwips(23);
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
    private readonly Dictionary<PageParagraph, List<WrapObstacle>> _byAnchor;

    private readonly List<long> _signature;
    private readonly int _frames;

    private FrameResolution(
        Dictionary<int, List<PlacedFrame>> byPage,
        Dictionary<int, FrameObstacles> byBlock,
        Dictionary<PageParagraph, List<WrapObstacle>> byAnchor,
        List<long> signature,
        int frames)
    {
        _byPage = byPage;
        _byBlock = byBlock;
        _byAnchor = byAnchor;
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
    /// <param name="addsCellLineSpacing">
    /// Whether a table inside a frame grows its cells by their last paragraph's proportional line
    /// spacing — see <see cref="PaginationOptions.AddsCellLineSpacing"/>.
    /// </param>
    /// <param name="capturesOnPage">
    /// Whether a content-anchored frame is pulled back inside its page — see
    /// <see cref="PaginationOptions.CapturesAnchoredObjectsOnPage"/>, which is where the rule and the
    /// formats it applies to are written out.
    /// </param>
    /// <param name="capturesWrappedObjects">
    /// Whether an object the <c>DoNotCaptureDrawObjsOnPage</c> flag does not exempt is captured
    /// anyway — see <see cref="PaginationOptions.CapturesWrappedObjects"/>.
    /// </param>
    /// <param name="narrowsCaptureToBody">
    /// Whether the area a captured object is held inside narrows from the sheet to the page's body —
    /// see <see cref="PaginationOptions.NarrowsCaptureToBody"/>.
    /// </param>
    /// <param name="disablesOffPagePositioning">
    /// Whether a wrap-through object is exempt from the page clip — see
    /// <see cref="PaginationOptions.DisablesOffPagePositioning"/>.
    /// </param>
    public static FrameResolution Of(
        IReadOnlyList<PageBlock> blocks,
        IReadOnlyList<PaginatedSection> sections,
        IReadOnlyList<LaidOutPage> pages,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false,
        bool capturesOnPage = true,
        bool capturesWrappedObjects = false,
        bool narrowsCaptureToBody = false,
        bool disablesOffPagePositioning = false)
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
                    page.ColumnArea(line),
                    page.BodyArea.Y + line.Top,
                    page.BodyArea.Y + line.ParagraphTop);
            }
        }

        Dictionary<int, List<PlacedFrame>> byPage = [];
        Dictionary<int, List<WrapObstacle>> obstaclesByPage = [];
        // By identity, not by value. A `PageParagraph` is a record, so two empty cells hold two equal
        // paragraphs — and the banner table that made this necessary has exactly that shape, one empty
        // anchor paragraph per masthead cell. Keyed by value they would share one entry and each cell
        // would flow round the other's logo.
        Dictionary<PageParagraph, List<WrapObstacle>> byAnchor =
            new(ReferenceEqualityComparer.Instance);
        List<long> signature = [];
        int frames = 0;

        // A frame whose stated height is only a floor is made as tall as its own text before anything
        // is placed, and the answer is memoised because the same frame is asked for once per pass and
        // measuring one costs a whole layout of its blocks. By identity, since two frames of equal
        // value are still two frames.
        Dictionary<PageFrame, PageFrame> grown = new(ReferenceEqualityComparer.Instance);

        PageFrame Sized(PageFrame frame)
        {
            if (!frame.GrowsToContent) return frame;
            if (grown.TryGetValue(frame, out PageFrame? already)) return already;

            PageFrame sized = FrameLayout.Grown(frame, collapsesSpacing, addsCellLineSpacing);
            grown[frame] = sized;
            return sized;
        }

        // One paragraph's frames, given the rectangle its origins are measured in and where its own top
        // ended up. Shared by the body's blocks and by the flows, which differ only in how those two are
        // arrived at — the body's from a placed line, a flow's from the flow's own area.
        void PlaceFrames(
            PageParagraph paragraph,
            int pageIndex,
            LaidOutPage page,
            DocRect origin,
            Length anchorTop,
            Length anchorLineTop,
            FrameAnchorPlace anchorPlace)
        {
            PageGeometry geometry = sections[
                Math.Clamp(page.SectionIndex, 0, sections.Count - 1)].Section.Page;

            foreach (PageFrame stated in paragraph.Frames)
            {
                // An as-character frame is not placed against an origin at all — it hangs on a line, at
                // the position its anchor character occupies. That needs the line rather than the
                // paragraph, so it is done by the walk below.
                if (stated.Anchor == FrameAnchor.AsCharacter) continue;

                PageFrame frame = Sized(stated);

                DocRect area = FrameLayout.Place(
                    frame,
                    geometry,
                    origin,
                    anchorTop,
                    rightHandPage: page.Number % 2 == 1,
                    anchorLineTop: anchorLineTop,
                    bodyArea: page.BodyArea,
                    capturesOnPage: capturesOnPage,
                    capturesWrappedObjects: capturesWrappedObjects,
                    narrowsCaptureToBody: narrowsCaptureToBody,
                    anchorPlace: anchorPlace,
                    disablesOffPagePositioning: disablesOffPagePositioning);

                frames++;
                signature.Add(area.X.Emu);
                signature.Add(area.Y.Emu);
                signature.Add(area.Width.Emu);
                signature.Add(area.Height.Emu);

                if (!byPage.TryGetValue(pageIndex, out List<PlacedFrame>? placed))
                {
                    byPage[pageIndex] = placed = [];
                }

                placed.Add(new PlacedFrame(
                    frame, area, Content(frame, area, collapsesSpacing, addsCellLineSpacing)));

                // A run-through frame is not an obstacle at all: it neither narrows a line nor pushes one
                // down, which is exactly what `SwTextFly::ForEach` skips it for.
                if (frame.Wrap == TextWrap.Through) continue;

                if (!obstaclesByPage.TryGetValue(pageIndex, out List<WrapObstacle>? list))
                {
                    obstaclesByPage[pageIndex] = list = [];
                }

                WrapObstacle obstacle = new(Widened(area, frame.Spacing), frame.Wrap);
                list.Add(obstacle);

                if (!byAnchor.TryGetValue(paragraph, out List<WrapObstacle>? mine))
                {
                    byAnchor[paragraph] = mine = [];
                }

                mine.Add(obstacle);
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
            foreach (PageFrame stated in paragraph.Frames)
            {
                if (stated.Anchor != FrameAnchor.AsCharacter) continue;
                if (stated.AnchorOffset < line.Box.Line.Start) continue;

                PageFrame frame = Sized(stated);

                // One past the last character belongs to the *next* line, except on the last line of
                // the paragraph, where there is no next one. That is not an edge case worth skipping:
                // RTF appends nothing to the text for a picture, so a paragraph holding only a picture
                // is an empty paragraph with an anchor at offset zero and a line running 0 to 0 — the
                // most ordinary way for a document to carry a logo.
                if (frame.AnchorOffset >= line.Box.Line.End && !line.Box.Line.EndsParagraph) continue;
                if (frame.AnchorOffset > line.Box.Line.End) continue;

                // Its top is the baseline less however much of it stands above one: the whole height for an
                // ordinary inline picture, and nought for a shape that hangs below the line instead. The
                // height in question is the *outer* one, grown by `wp:effectExtent`, because that is the
                // rectangle Writer rests on the baseline —
                // `SwAsCharAnchoredObjectPosition::GetObjBoundRectInclSpacing`, the object's rectangle
                // enlarged by its spacing. The drawing itself keeps the size the file states and sits
                // inside that rectangle, so it is *not* offset by the extent's left and top edges here.
                //
                // Both installed references agree on where the shape lands and it is not where either
                // half of that sentence alone would put it: measured in
                // `dotnet/probes/words-inline-effectextent/` and again in
                // `dotnet/probes/words-inline-shape-ink/`, LibreOffice paints a shape's fill and
                // outline at the outer top plus the top extent — 96.50 pt against 85.75 for a 10.8 pt
                // extent — while laying the *text* of a shape carrying a `wps:txbx` out at the outer top
                // regardless, its "INSIDE" run staying at 104.66 pt in both. That is its draw-shape and
                // TextBox halves disagreeing rather than a rule, and it cannot be met by one rectangle:
                // the rectangle here is the *text's*, and `PlacedFrame.Ink` is the drawing's, offset
                // below it by `PageFrame.InlineInkOffset`.
                //
                // A *turned* drawing is offset differently again — centred in its line box in both
                // axes rather than moved by the extent's left edge — and both rules live on
                // `PageFrame.InlineOffset`, which carries the measurements.
                //
                // **Horizontally there is no such disagreement, and the two edges are not symmetric.**
                // `make-x-fixture.py` is the same fixture laid across a line as `LEFT` + drawing +
                // `RIGHT`, and with a 10.8 pt *left* extent both halves of the object move right by it
                // together: the fill's own column band goes 103.50 -> 114.25 pt and the `INSIDE` run of a
                // `wps:txbx` goes 155.95 -> 166.75, on both installed references, while a 10.8 pt *top*
                // extent moves neither of them by anything. So x takes the left extent and y does not
                // take the top one — which is `SwAsCharAnchoredObjectPosition::CalcPosition`
                // (`sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx`:129-133) moving
                // the anchor point by both spacings, with only the vertical one lost again by the
                // TextBox half failing to follow.
                DocPoint inside = frame.InlineOffset;

                // A member of a group or a `wpc:wpc` canvas takes its place *inside* the drawing,
                // and it is the drawing that hangs on the line: what rests on the baseline is the
                // envelope's rectangle, not the member's, and the member is then offset within it.
                // `Placed` has always done both for an anchored drawing — `PageFrame.GroupSize` is
                // the envelope's rectangle and `GroupOffset` where the member sits in it — and this
                // path did neither, so every member of an *as-character* group or canvas was drawn
                // at the drawing's own top-left corner, a dozen shapes on one spot. Censused over
                // the corpus, `wpc:wpc` alone is 9 canvases across 4 documents.
                DocSize outer = frame.GroupSize is { } group
                    ? new DocSize(
                        group.Width + frame.EffectExtent.Left + frame.EffectExtent.Right,
                        group.Height + frame.EffectExtent.Top + frame.EffectExtent.Bottom)
                    : frame.InlineExtent;

                DocRect placedAt = new(
                    area.X + line.Box.Left + PageDrawing.OffsetOnLine(paragraph, line, frame.AnchorOffset)
                        + inside.X + frame.GroupOffset.X,
                    area.Y + line.Baseline - (frame.InlineAscent ?? outer.Height) + inside.Y
                        + frame.GroupOffset.Y,
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
                placed.Add(new PlacedFrame(
                    frame, placedAt, Content(frame, placedAt, collapsesSpacing, addsCellLineSpacing)));
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
                placement.Top,
                FrameAnchorPlace.Body);
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

                HangInline(paragraph, index, page.ColumnArea(line), line);
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
            foreach ((PlacedFlow flow, FrameAnchorPlace place) in FlowsOn(pages[index]))
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
                        flow.Area.Y + line.Top,
                        place);
                }
            }
        }

        // And the frames anchored inside another frame's own text, which is the case a table in a text
        // box makes ordinary: the outer frame's rectangle has to exist before an inner frame can be
        // placed against it, so this runs after the loops above rather than beside them. Each round of
        // the loop places the frames one nesting level deeper and enqueues whatever it placed, since a
        // frame it placed may itself hold text holding a frame.
        //
        // Writer draws no distinction of its own here — an anchored object belongs to the page whatever
        // it is nested in, exactly as the flow loop above says — and the omission was silent in the way
        // a missing anchored object always is: the outer frame is drawn, filled and stroked, and only
        // its inner shapes' text is absent. Measured on the converted `.odt` corpus, where LibreOffice's
        // exporter writes a Word text box holding a table as exactly this shape: eight documents hold
        // 857 alphanumeric characters in a shape nested inside a frame, **six of them were failing the
        // gate for exactly that many**, and on three the loss was the document's whole shortfall
        // against the reference to the character.
        List<(int Page, PlacedFrame Frame)> pending = [];

        foreach ((int pageIndex, List<PlacedFrame> placed) in byPage)
        {
            foreach (PlacedFrame frame in placed) pending.Add((pageIndex, frame));
        }

        for (int depth = 0; depth < FlowLayouter.MaxNesting && pending.Count > 0; depth++)
        {
            List<(int Page, PlacedFrame Frame)> deeper = [];

            foreach ((int pageIndex, PlacedFrame outer) in pending)
            {
                if (outer.Content is not { } content) continue;

                byPage.TryGetValue(pageIndex, out List<PlacedFrame>? before);
                int already = before?.Count ?? 0;

                foreach (PlacedFlow flow in Inside(content))
                {
                    foreach (PlacedLine line in flow.Lines)
                    {
                        if (line.ParagraphIndex < 0 || line.ParagraphIndex >= flow.Blocks.Count) continue;
                        if (flow.Blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;
                        if (paragraph.Frames.Count == 0) continue;

                        HangInline(paragraph, pageIndex, flow.Area, line);

                        if (!line.StartsParagraph) continue;

                        PlaceFrames(
                            paragraph,
                            pageIndex,
                            pages[pageIndex],
                            flow.Area,
                            flow.Area.Y + line.ParagraphTop,
                            flow.Area.Y + line.Top,
                            FrameAnchorPlace.Elsewhere);
                    }
                }

                if (!byPage.TryGetValue(pageIndex, out List<PlacedFrame>? after)) continue;

                for (int index = already; index < after.Count; index++)
                {
                    deeper.Add((pageIndex, after[index]));
                }
            }

            pending = deeper;
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

        return new FrameResolution(byPage, byBlock, byAnchor, signature, frames);
    }

    /// <summary>The obstacles one block's lines must flow around, or null when it has none.</summary>
    public ILineObstacles? ObstaclesFor(int block)
        => _byBlock.TryGetValue(block, out FrameObstacles? obstacles) ? obstacles : null;

    /// <summary>
    /// The frames one paragraph anchors, in page coordinates, or null when it anchors none that wrap.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Keyed by the paragraph rather than by a block index, which is the whole point of it: a frame
    /// anchored inside a table cell has no top-level block to be keyed by, so
    /// <see cref="ObstaclesFor(int)"/> answers null for every one of them and the frame is drawn while
    /// obstructing nobody — the cell's own anchor paragraph included. Writer draws no such distinction:
    /// <c>SwTextFly</c> is a property of the text frame being formatted, and a cell is a text frame.
    /// </para>
    /// <para>
    /// Only the paragraph's <em>own</em> frames, not every frame overlapping the cell. Writer would take
    /// the page's whole sorted object list, so a body frame lying across a table narrows its cells too;
    /// this is the half that a cell cannot reach any other way, and it is the half that cannot change a
    /// document holding no frame anchored in a cell.
    /// </para>
    /// </remarks>
    /// <param name="anchor">The paragraph the frames are anchored to.</param>
    public IReadOnlyList<WrapObstacle>? AnchoredIn(PageParagraph anchor)
        => _byAnchor.TryGetValue(anchor, out List<WrapObstacle>? obstacles) ? obstacles : null;

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
    /// <returns>
    /// Each flow with where its paragraphs sit on the page, which the capture asks about — see
    /// <see cref="FrameAnchorPlace"/>. A cell inherits its table's place, so a cell of a table in the
    /// running head is furniture and a cell of a body table is body.
    /// </returns>
    private static IEnumerable<(PlacedFlow Flow, FrameAnchorPlace Place)> FlowsOn(LaidOutPage page)
    {
        // A running head is very often a *table* — a logo in the first cell and the document's title
        // and revision in the next — so a flow's own tables have to be walked as well as the body's.
        // Walking only `page.Tables` reached every body cell and no furniture cell, which loses the
        // logo of any document that lays its header out that way. Measured on
        // `UG.CAO.00133 … Language.docx`, whose `word/header1.xml` is one two-column table holding a
        // 542925 EMU `wp:inline` picture: the reference draws it on page 1 and on the three landscape
        // pages that use `header6.xml`, and we drew it nowhere.
        foreach ((PlacedFlow furniture, FrameAnchorPlace place) in Furniture(page))
        {
            yield return (furniture, place);

            foreach (PlacedTable table in furniture.Tables)
            {
                foreach (PlacedFlow flow in CellFlows(table, 0)) yield return (flow, place);
            }
        }

        foreach (PlacedTable table in page.Tables)
        {
            foreach (PlacedFlow flow in CellFlows(table, 0)) yield return (flow, FrameAnchorPlace.Body);
        }
    }

    /// <summary>The page's furniture flows: the header, the footer and the notes area.</summary>
    /// <remarks>
    /// The notes area is <see cref="FrameAnchorPlace.Elsewhere"/> rather than furniture: a footnote
    /// frame is in the page's footnote container, so it has no body frame to be narrowed to — but
    /// tdf#123002's escape names the header and the footer alone
    /// (<c>anchoredobjectposition.cxx</c>:641-643 tests <c>IsFooterFrame() || IsHeaderFrame()</c>).
    /// </remarks>
    private static IEnumerable<(PlacedFlow Flow, FrameAnchorPlace Place)> Furniture(LaidOutPage page)
    {
        if (page.Header is { } header) yield return (header, FrameAnchorPlace.Furniture);
        if (page.Footer is { } footer) yield return (footer, FrameAnchorPlace.Furniture);
        if (page.Notes is { } notes) yield return (notes, FrameAnchorPlace.Elsewhere);
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

    /// <summary>One placed frame's own flows: its text, and the cells of every table inside it.</summary>
    /// <remarks>
    /// The frame-shaped twin of <see cref="FlowsOn"/>, and separate from it because a frame's content is
    /// not on the page's own lists — it hangs off the <see cref="PlacedFrame"/>, which exists only once
    /// the frame has been placed.
    /// </remarks>
    /// <param name="content">The frame's laid-out content.</param>
    private static IEnumerable<PlacedFlow> Inside(PlacedFlow content)
    {
        yield return content;

        foreach (PlacedTable table in content.Tables)
        {
            foreach (PlacedFlow flow in CellFlows(table, 0)) yield return flow;
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
    /// <para>
    /// Through <see cref="FlowLayouter"/>, which is what makes a frame's paragraphs behave like a header's
    /// or a cell's without a second layout path — a frame containing a table works because a cell already
    /// does. An image frame has no blocks and gets null, since the raster is decoded elsewhere and the
    /// rectangle is all the wrap ever needed.
    /// </para>
    /// <para>
    /// A frame of stated height is then cut to what fits, per
    /// <see cref="PageFrame.HasFixedHeight"/> — a shape does not overflow the way a running head does.
    /// The two are opposite on purpose and the distinction is Writer's: a header that cannot hold its
    /// text draws it past the bottom (see the remarks on <see cref="FlowLayouter.LayOut"/>), while a
    /// text box that cannot hold its text never formats the rest. Both were measured; a text box in a
    /// header is the shape that needs them both to be right at once.
    /// </para>
    /// </remarks>
    private static PlacedFlow? Content(
        PageFrame frame, DocRect area, bool collapsesSpacing, bool addsCellLineSpacing)
    {
        if (frame.Blocks.Count == 0) return null;

        DocRect inside = area.Deflate(frame.Padding);

        PlacedFlow? flow = FlowLayouter.LayOut(
            frame.Blocks,
            inside,
            Length.Zero,
            collapsesSpacing: collapsesSpacing,
            addsCellLineSpacing: addsCellLineSpacing);

        if (flow is not null && frame.HasFixedHeight) flow = FlowLayouter.Truncated(flow, inside.Height);

        return Anchored(flow, frame.TextAlignment, inside.Height);
    }

    /// <summary>
    /// The flow moved down to sit where the shape's <c>wps:bodyPr/@anchor</c> asks.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same arithmetic a table cell's <c>w:vAlign</c> gets — spare height, then none, half or all
    /// of it — and it was missing here entirely, so every shape's text sat against the top of its box
    /// whatever the file said. It is invisible on a text box sized to its text and unmissable on a
    /// shape sized to be a shape: the Venn diagram templates put their labels in circles two or three
    /// times a line tall, and a label against the top of its circle lands outside the ink it names.
    /// </para>
    /// <para>
    /// Measured against the ink rather than the advance — <see cref="FlowLayouter.Extent"/> rather
    /// than <see cref="PlacedFlow.Advance"/> — because centring is about where the text looks
    /// centred, and the advance carries the last paragraph's space-after, which would push the block
    /// up by half of a gap nothing draws.
    /// </para>
    /// <para>
    /// Only the flow's rectangle moves; its lines are positioned relative to that rectangle and come
    /// along. A nested table inside the frame carries page coordinates and would not, which is what
    /// <c>TableLayouter.ShiftFlow</c> exists for — a frame holding a table and asking for centred
    /// text would need it, and no corpus document does both.
    /// </para>
    /// </remarks>
    private static PlacedFlow? Anchored(PlacedFlow? flow, VerticalTextAlignment alignment, Length height)
    {
        if (flow is null || alignment == VerticalTextAlignment.Top) return flow;

        Length spare = height - FlowLayouter.Extent(flow);
        if (spare <= Length.Zero) return flow;

        Length offset = alignment == VerticalTextAlignment.Middle ? spare / 2 : spare;

        return flow with
        {
            Area = new DocRect(flow.Area.X, flow.Area.Y + offset, flow.Area.Width, flow.Area.Height),
        };
    }

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
public readonly record struct WrapObstacle(DocRect Area, TextWrap Wrap);

/// <summary>
/// A flow's own anchored frames, and where the flow sits on the page, so its paragraphs can be
/// obstructed by them.
/// </summary>
/// <remarks>
/// <para>
/// The body's paragraphs reach their obstacles by block index — see
/// <see cref="FrameResolution.ObstaclesFor(int)"/> — and a paragraph inside a table cell has no block
/// index to be reached by. This is the same information carried the other way round: a lookup by the
/// anchoring paragraph, plus the page coordinates the flow's own origin corresponds to, since
/// <see cref="FrameObstacles"/> works in page coordinates and a cell is laid out at its own zero.
/// </para>
/// <para>
/// <see cref="Below"/> is how the origin follows a nested flow down: a table inside a cell, and a cell
/// inside that table, each shift the origin without changing the lookup.
/// </para>
/// </remarks>
/// <param name="lookup">The frames a paragraph anchors, or null when it anchors none that wrap.</param>
/// <param name="origin">Where the flow's own (0, 0) sits on the page.</param>
public sealed class AnchoredObstacles(
    Func<PageParagraph, IReadOnlyList<WrapObstacle>?> lookup, DocPoint origin)
{
    /// <summary>The obstacles a paragraph at an offset down the flow sees, or null when it sees none.</summary>
    /// <remarks>
    /// The offset is where the flow has reached rather than where the paragraph's text starts, so a
    /// paragraph with space above it is measured against the frame from a hair too high. Writer has the
    /// same ordering problem — the space above is part of the layout being computed — and the difference
    /// can only matter for a frame whose edge falls inside that gap.
    /// </remarks>
    /// <param name="paragraph">The paragraph being laid out.</param>
    /// <param name="top">How far down the flow it starts.</param>
    public ILineObstacles? For(PageParagraph paragraph, Length top)
        => lookup(paragraph) is { Count: > 0 } list
            ? new FrameObstacles([.. list], origin.Y + top, origin.X)
            : null;

    /// <summary>The same lookup, with the origin moved to a nested flow's own top-left.</summary>
    /// <param name="offset">Where the nested flow starts, in this flow's coordinates.</param>
    public AnchoredObstacles Below(DocPoint offset)
        => new(lookup, new DocPoint(origin.X + offset.X, origin.Y + offset.Y));
}

/// <summary>The obstacles one flow's own anchored frames make, positioned on the page.</summary>
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
/// <strong>A line whose box merely touches the frame's top edge is not affected.</strong> Writer's
/// rectangles are inclusive — <c>SwRect::Bottom() == Top() + Height() - 1</c> — so a frame at twip 2210
/// and a line ending at 2210 do not overlap, and neither reference wraps that line once the tie is
/// broken either way by a single twip. Only the <em>horizontal</em> edges gain a twip back from the
/// arithmetic around the fly portion, which is why text resumes 3402 rather than 3401 twips along from
/// a frame 2268 twips wide at 1134. See <see cref="FrameObstacles.Inflation"/>.
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
    /// How much wider than its geometry a frame's hole in the text is, on each side.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One twip, and it is not a fudge: Writer's rectangles are inclusive, so a frame 2268 twips wide at
    /// 1134 has <c>Right() == 3401</c> and the arithmetic around the fly portion adds a twip back, which
    /// is why text resumes 3402 twips along rather than 3401.
    /// </para>
    /// <para>
    /// <strong>Horizontally only.</strong> It used to be applied on all four sides, on the reading that a
    /// line whose box bottom is exactly a frame's top is already obstructed. That reading is wrong against
    /// 26.2.4.2 and the measurement is in
    /// <c>probes/words-frame-parallel/results.md</c>: moving the frame six twips up makes the touching line
    /// wrap in six of six documents, moving it six twips down leaves it alone in six of six, and at exact
    /// equality the reference answers three each way — on the frame's <em>width</em>, its horizontal
    /// position and its height, none of which can change the vertical relation. So the reference's rule
    /// away from the boundary is a strict overlap, and on the boundary it has no rule at all.
    /// </para>
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
            obstacle.Area.Y,
            obstacle.Area.Width + (Inflation * 2),
            obstacle.Area.Height);

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
