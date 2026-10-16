# The rule at the seat (working notes; folded into results.md)

## Where ODF states the height kind

`XMLTextFrameContext_Impl` reads the frame's geometry from two attribute lists — the *content*
element's (`draw:text-box`, `draw:image`, …) and then the `draw:frame`'s
(`xmloff/source/text/XMLTextFrameContext.cxx`:1113-1116), in that order.

* `svg:height` on `draw:frame` writes `nHeight` (:965-978).
* `fo:min-height` on `draw:text-box` writes the **same** `nHeight` and additionally sets
  `bMinHeight` (:997-1010).
* `Create()` then sets `Height` when `nHeight > 0` (:643-646) and `SizeType` to
  `(bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN : SizeType::FIX`
  (:655-661).

So the frame's stated height is a *floor* exactly when the text box carries `fo:min-height`,
and a frame stating neither keeps the text frame's own default `SizeType`, which is `VARIABLE`
— grow with no floor, the same behaviour as a floor of nought.

## The growth itself

`SwFlyFrame::Format` (`sw/source/core/layout/fly.cxx`:1549-1570):

```
nRemaining = CalcContentHeight(pAttrs, nMinHeight, nUL);
if (IsMinHeight() && (nRemaining + nUL) < nMinHeight) nRemaining = nMinHeight - nUL;
if (nRemaining < MINFLY) nRemaining = MINFLY;
… aRectFnSet.SetHeight(aPrt, nRemaining); … AddBottom(aFrm, nRemaining + nUL);
```

* `CalcContentHeight` (`fly.cxx`:3538-3588) sums the fly's lower frames' heights.
* `nUL = pAttrs->CalcTopLine() + pAttrs->CalcBottomLine()` — each edge's padding plus its border
  width plus its shadow space (`SwBorderAttrs::CalcTopLine_`,
  `sw/source/core/layout/frmtool.cxx`:2474-2486, over `SvxBoxItem::CalcLineSpace`,
  `editeng/source/items/frmitems.cxx`:3717-3755).
* `MINFLY` is 23 twips (`sw/inc/swtypes.hxx`:59).
* The stated minimum is compared against `nRemaining + nUL` — the **whole frame**, insets
  included.

## The trap that cost the first sixteen probes

A `draw:frame` whose `draw:style-name` names an **automatic graphic style with no parent style**
is not imported as a Writer text frame at all: `XMLTextFrameContext`'s constructor sets
`m_HasAutomaticStyleWithoutParentStyle` (:1374-1394, *"New distinguish attribute between Writer
objects and Draw objects is: Draw objects have an automatic style without a parent style
(#i51726#)"*) and `createFastChildContext` then routes the element to
`XMLShapeImportHelper::CreateFrameChildContext` (:1500-1507). A drawing shape fits itself to its
text by a different rule and ignores `fo:min-height` outright — the first probe set stated
`fo:min-height="3in"` on a two-line box and 26.2.4.2 drew it 27.65 pt tall.

Every height-less `draw:frame` LibreOffice's own exporter writes names `Frame` as its parent, so
the corpus is unaffected; the probes had to name one before they measured anything.
