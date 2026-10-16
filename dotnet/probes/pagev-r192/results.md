# An embedded object follows the text flow, so "the page" means the page body

Round 192, continuing `probes/ofpie-r190` §4, which measured the displacement on
`028_Unit_Circle_Chart_Optimized_Graph.docx` and left it unimplemented for want of a mechanism:
the reference draws that chart **71.6 pt lower than its `relativeFrom="page"` offset states**, a
minimal `wps:wsp` fixture at the same relation is drawn at the sheet's own top edge, and
`layoutInCell` switches the document while `GraphicImport`'s only documented uses of that attribute
are guarded by `IsInTable()`.

## 1. The mechanism, read out of the tree

**The missing variable is the object kind, and the guard really is absent on one path.**

- A chart's (or an OLE object's) `a:graphicData` becomes a `com.sun.star.drawing.OLE2Shape` —
  `Shape::setChartType`, `oox/source/drawingml/shape.cxx`:348-358.
- `DomainMapper_Impl::PopShapeContext`'s OLE2Shape branch throws that shape away and builds a
  `SwXTextEmbeddedObject` in its place (`DomainMapper_Impl.cxx`:5096-5112).
- `ImportGraphic` then copies the position properties onto that embedded object and, at **:9792**,
  sets `IsFollowingTextFlow` straight from `m_pGraphicImport->GetLayoutInCell()`. There is **no
  `IsInTable()` test on that line**, where both of the shape and graphic paths have one
  (`GraphicImport.cxx`:1316-1318 and :1859-1861) — which is why a census of
  `PROP_FOLLOW_TEXT_FLOW`'s writers concluded the property is always false outside a table and was
  one seat short.
- `m_bLayoutInCell` is initialised **true** (`GraphicImport.cxx`:340), so an anchor stating nothing
  follows the text flow as well; `layoutInCell="0"` outside a table clears it, because the
  compatibility force-on beside it needs `IsInTable()` (:707-712).

**And one function turns that property into four separate consequences.**
`SwEnvironmentOfAnchoredObject::GetVertEnvironmentLayoutFrame`
(`environmentofanchoredobject.cxx`:64-95) answers `FindPageFrame()` when the object does not follow
the text flow, and otherwise walks up from the anchor to the first cell, fly, header, footer,
footnote, **page body** or page frame. For a paragraph in the body that is the page body frame,
whose top is `w:pgMar/@w:top` below the sheet's. Then:

1. **the base.** `SwToContentAnchoredObjectPosition::CalcPosition`'s offset arm measures a
   `PAGE_FRAME` (and a `PAGE_PRINT_AREA_TOP`) offset from `GetTop(rPageAlignLayFrame.getFrameArea())`
   (`tocntntanchoredobjectposition.cxx`:590-596) — the body's top, not the sheet's. That is the
   71.6 pt.
2. **whether it is captured at all.** `mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow
   && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE` (`anchoredobjectposition.cxx`:125-144): the middle term is
   now false, so a wrap-through embedded object is captured where a wrap-through picture is not.
3. **what it is captured in.** `ImplAdjustVertRelPos`:544-573 takes the *page* frame's rectangle
   whenever the anchor is outside a table, and its `compatibilityMode` 15 narrowing to the body
   needs `rPageAlignLayFrame.IsPageFrame()` — which a body frame is not. So a following object is
   held inside the **sheet**, which is the opposite of what the flag's name suggests.
4. ~~**only at the top.**~~ **Withdrawn — implemented, then refuted by the corpus.** See §7.

**The horizontal is untouched, and that is a property of the same file rather than an omission.**
`GetHoriEnvironmentLayoutFrame`'s walk stops at a cell, a fly or a page and has no body frame in it
(:34-61), so a body anchor's horizontal environment is the page frame either way — which agrees with
r190's reading of `028`, where only the frame's `y` was wrong.

## 2. What is implemented

`PageFrame.FollowsTextFlow`, set by `DocxFrames` for an anchored drawing holding a chart whose
`layoutInCell` is not explicitly off, and by no other reader — the other three word-processing
readers build no embedded object for this layout to place. `FrameLayout.Place` then applies (1) for
`FrameVerticalOrigin.Page`, (2), (3) and (4).

**`PAGE_PRINT_AREA_TOP` shares (1)'s branch in the C++ and is deliberately left.** Substituting the
body there would collapse the band `probes/frame-area-r85` measured against 26.2.4.2 to nothing, and
that measurement was taken on a *shape*, so it says nothing about the following case. `census-embedded.py`
is the census that decides whether any corpus document pairs a `topMargin` vertical relation with an
embedded object.

**The in-table half stays unmodelled.** Inside a table such an object is captured in its *cell*
(`anchoredobjectposition.cxx`:576-591) and this tree has no rectangle for a cell here.

## 3. Instruments

| file | what it does |
|---|---|
| `fixture.py` | eight minimal DOCX: **shape against chart** at the same `wp:anchor`, × `w:top` 1440/2880, `layoutInCell="0"`, and the attribute absent |
| `census-embedded.py` | every corpus DOCX anchor holding a chart or OLE object, by `layoutInCell`, by `positionV` relation, and by whether it is in a `w:tbl` |
| `sweep.py` | our half of a target list, one output directory per document, `SOURCE_DATE_EPOCH` pinned |
| `confine.py` | which of two sweeps' documents moved, and each mover's worst page against a banked reference |

`fixture.py` is the discriminating experiment and the reason this round exists: r190's fixture varied
the *relation* and held the object kind at a shape, so it could only ever refute "page means margin".
Two files differing in nothing but whether the anchored object is a `wps:wsp` or a `c:chart` separate
the object kind from every other candidate.

## 4. Measured: the object kind decides it, and eight of eight reproduce

`fixture.py`'s eight files through 26.2.4.2 (`0229ac93fcf0d7cbc6376066c6f35021cef002dc`) and through
this tree at the change. The object's top edge in document coordinates, read out of the page: the
shape is its one red fill and the chart its own outer `#D9D9D9` border, which spans the frame
(215.2 × 143.2 pt against the stated 216 × 144). Every file states the same anchor — `positionH`
72 pt and `positionV` 180 pt, both `relativeFrom="page"`.

| fixture | 26.2.4.2 | ours | base it implies |
|---|---:|---:|---|
| `shape-top1440` | 180.00 | 179.99 | the **sheet** |
| `shape-top2880` | 180.00 | 179.99 | the sheet — `w:top` moves it not at all |
| `shape-incell0` | 180.00 | 179.99 | the sheet |
| `shape-noincell` | 180.00 | 179.99 | the sheet |
| `chart-top1440` | **252.35** | 251.99 | the **body**, 72 pt down |
| `chart-top2880` | **324.35** | 323.99 | the body, 144 pt down — one for one with `w:pgMar/@w:top` |
| `chart-incell0` | **180.35** | 179.99 | the sheet: `layoutInCell="0"` clears it |
| `chart-noincell` | **252.35** | 251.99 | the body — an absent attribute is *true* |

**That is the whole argument in one table.** Two files identical but for whether the anchored object
is a `wps:wsp` or a `c:chart` are drawn 72.35 pt apart, and the distance is the top margin. The
0.36 pt this tree sits above the reference on every chart row is the two writers' own border-origin
constant — it is flat across all four and the shape rows agree to 0.01, which is what says it is not
part of the rule. **8 of 8 reproduced with no free parameter.**

`probes/ofpie-r190/page-anchor-fixture.py` could not have found this: it varied the *relation* and
held the object kind at a shape, so every one of its variants is the first four rows of that table.

## 5. Reach, by markup

`census-embedded.py` over the corpus's 272 DOCX: **5 embedded-object anchors in 5 documents**, all
five `Unit_Circle` charts, **all `layoutInCell="1"`, all outside a `w:tbl`**, and their vertical
relations are `page` on two (`027`, `028`) and `margin` on three (`021`, `023`, `029`).

So consequence (1) — the base — moves **two documents**, because `margin` is `PAGE_PRINT_AREA` and
resolves to the body either way. Consequences (2), (3) and (4) do not depend on the relation and
reach all five. **No corpus DOCX pairs a `topMargin` relation with an embedded object**, which is
what retires the `PAGE_PRINT_AREA_TOP` half recorded in §2, and none is inside a table, which is what
keeps the unmodelled cell capture out of reach.

Nothing outside the words track can move at all: `PageFrame.FollowsTextFlow` has one writer,
`DocxFrames`, and with the flag false all four expressions in `FrameLayout.Place` reduce to exactly
what they were.

## 6. Suites

`dotnet build XRay.Office.slnx` **0 warnings, 0 errors**, and
`XRay.Office.WordProcessing.Tests` **2200 passed, 0 failed** against the 2189 of the round's base — the
eleven new cases being `FrameFollowsTextFlowTests`' seven methods, four of them theories.

## 7. A fourth consequence was implemented and the corpus refuted it

**`bCheckBottom = !DoesObjFollowsTextFlow()` is real and it is not the rule for this case.** The
expression appears three times — at `tocntntanchoredobjectposition.cxx`:457 in the *alignment* arm and
at :678 and :718 in the offset one — and the offset arm has a **fourth** `AdjustVertRelPos` call, at
:810-813, which **omits the argument** and so takes the `= true` default
(`anchoredobjectposition.hxx`:185-192). That call sits under *"do not follow text flow respectively
align at 'page areas', but stay inside given environment"*, and it is the path an offset that does not
fit its upper's print area takes — which is the path a page-relative chart reaches.

`027_Unit_Circle_Chart_Graphical_Chart` is the witness and it is unambiguous. Its chart is 470.30 pt
tall on a 595.30 pt landscape page with `w:top="3139"` (156.95 pt) and an offset of 111.65 pt, so the
body base puts its bottom at 738.90 — and 26.2.4.2 draws its top at **125.00**, which is
`595.30 − 470.30` to the hundredth. Leaving the bottom unchecked drew it at 268.60:

| | 26.2.4.2 | with the bottom skipped | with it kept |
|---|---:|---:|---:|
| `027` chart top, doc y | **125.00** | 268.60 | **125.00** |
| `027` worst-page `diff%` | — | 28.42 | **4.61** (9.48 at the base) |

So **three consequences, not four**. It is worth writing down twice: the fourth was read out of the
source, it was read correctly, and the source had one more call site than the reading did.

## 8. Confinement, and what the two movers score

`sweep.py` over the words track's 337 documents at the round's base and again with the change, our
half only, `SOURCE_DATE_EPOCH` pinned, one output directory per document:

**2 moved, 335 byte-identical, 0 unrendered** — and the two are exactly the two whose chart states
`relativeFrom="page"`. The three `margin` ones do not move by a byte, which is the prediction in §5
arriving as a measurement: `PAGE_PRINT_AREA` resolves to the body either way.

| document | worst-page `diff%` | `\|ink\|%` | chart top, ours → 26.2.4.2 |
|---|---|---|---|
| `027_Unit_Circle_Chart_Graphical_Chart` | 9.48 → **4.61** | 0.56 → 0.73 | 470.30 → 470.30, **exact** |
| `028_Unit_Circle_Chart_Optimized_Graph` | 16.78 → **14.18** | 3.32 → 3.60 | 247.49 → 247.85, the 0.36 constant |

Both improve on `diff%`, which is the metric to rank a chart on, and both rise slightly on `|ink|%` —
the expected sign, since `|ink|` is signed per region before the page's absolute value is taken and a
frame moving into place cancels less than one sitting 72 pt away.

**`028` is scored against its *authored* reference here, and that is a change from
`probes/ofpie-r190` §4.** That round scored it against the `v-incell0` variant because our chart was
71.6 pt out of place and a page-fraction metric could only measure the displacement. With the
displacement gone the authored reference is the right one — and scoring the fixed tree against
`v-incell0` gives 18.89, which is that same displacement measured in the other direction. *A control
built to remove a defect stops being a control once the defect is fixed.*

Nothing outside the words track can move: `PageFrame.FollowsTextFlow` has one writer, and with the
flag false every expression in `FrameLayout.Place` reduces to what it was.

**And one banked row has to be re-measured before it is built on.**
`probes/ofpie-r190/results.md`'s variant table gives `layoutInCell="0"` a frame top of **214.94**
and annotates it *"i.e. ours"*, while `w:top="0"` gives **175.86** and is annotated the same way, and
the prose says the frame is 71.6 pt out. Those three cannot all be true of one quantity: 247.86 −
175.86 is exactly 72.00 and 247.86 − 214.94 is 32.92. The linearity in `w:top` is what §1 predicts
and is not in doubt; the 214.94 is either a different fill measured on a different rendering or a
transcription. Settle it from the PDFs before treating the `incell0` variant as the control, and
correct whichever file is wrong.
