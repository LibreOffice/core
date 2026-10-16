# ink-pass-r92 — a Calc drawing is clipped to the page's own cell block

Base `d945fefb6`, worktree `/home/user/wt-inkpass`, branch `agent/inkpass`, 2026-09-10.
Reference **26.2.4.2** — `/opt/libreoffice26.2/program/soffice`, the TDF tarball with the five
font confounds already moved aside. `/usr/bin/soffice` is 24.2.7.2 and was not used.
Our half rendered by `dotnet/tools/XRay.Office.Cli` built in this worktree, `SOURCE_DATE_EPOCH`
pinned on both legs. Reference PDFs reused from the bank at `/home/user/gate-orig-r83/ref`
(947 documents); nothing was re-rendered through `soffice` except the two authored probes.

## 0. How this round was run, and the reader that does not exist here

The brief asked for **at least two readings delegated to fresh subagents**. This container has
no such tool: there is no `Task`, and `ToolSearch` over the deferred set returns `TaskStop`,
`SendMessage`, `Monitor` and the GitHub tools and no agent launcher. `create_session` spawns a
sibling in its own container that cannot open `/home/user/...` and has no channel back — the
same finding four previous rounds recorded. **So every reading below is mine and is
contaminated**, and each is corroborated by a measurement that does not depend on it: the
content-stream clip rectangles in §2, the before/after ink table in §5, and the gate columns in
§6. Where a claim rests on the picture alone it is labelled as such.

## 1. What the pictures said

Ranked from the brief's table, `pdf-image-diff.py` was run on
`NAS-Infrastructure-Roadmaps-Weather__pptx`, `FAA_Form_337__ppt`,
`044_Cash_flow_forecast_Use_this_template_b8fa1f35__xlsx` and `pres_ioc_phuket__ppt`. The one
that read as a single coherent cause rather than as scattered reflow was the workbook:
**four of its five pages MAJOR, and three of the four hints say `ink we draw that the reference
does not`.**

Composed pairs, as looked at:

| image | what it shows |
|---|---|
| `044-page2-before.png` | page 2 agrees. Both draw the title block, the month table and the chart's left part. The month *labels* differ (`February 2023` against `October 2026`) and that is the volatile-date confound, not a defect |
| `044-page4-before.png` | page 4. The reference draws the chart's last three categories only, `Nov Dec Jan`, in the right half of the page. We draw **all twelve**, from the left paper edge, with the value-axis labels cut off at x = 0. Both agree exactly on where the title block's tail sits |
| `044-page4-after.png` | the same pair after the fix |
| `pilot-logbook-page18-after.png` | the largest of the six documents this makes *worse*, and why — §5 |

The reading gave direction and kind — *the reference cuts this object where we let it run* — and
could not decide between three causes: the object placed at a different x, the object drawn at a
different size, or the object clipped. Those are the three §2 separates.

## 2. The instrument: the reference states its own clip, and we state none

`make-clip-probe.py` authors a minimal `.xlsx` — sixteen 14-character columns, eighty rows, one
`xdr:twoCellAnchor` rectangle with a solid fill and a black outline running from B2 to N70 — so
the rectangle crosses both a page-column and a page-row break. 26.2.4.2 paginates it 2 × 3.

`clip-probe.tsv` is the whole answer, read out of the content streams:

| | page 1 | page 3 | page 5 |
|---|---|---|---|
| 26.2.4.2 | `50.4 58.28 466.214 729.609 re W* n` | `… 466.186 …` | `… 310.791 …` |
| this tree, before | *no clip at all* | *no clip* | *no clip* |
| this tree, after | `50.4 57.752 466.1858 …` | `… 466.1858 …` | `… 310.7906 …` |

466.186 pt is **exactly six columns of 77.697 pt** — the block — where the paper's text area is
494.9 and the page 595.3. The reference emits that rectangle around *every* copy of the object,
on all six pages, and 310.791 pt on the last band is the four columns that band prints. We agree
with it horizontally to **0.0004 pt** on the widest band and 0.0004 on the narrowest.

The vertical figures do not agree, and that is a **separate, pre-existing difference this probe
happens to expose**: with no `sheetFormatPr` and no `ht` on any row, the reference gives every
row 12.800 pt and fits 57 to a page, and we compute an optimal 13.777 and fit 53. The clip
follows whatever the block is, so this shows up in the clip's height. Left alone — it is a row
metric, not a clip, and it does not reach the corpus, whose workbooks state their heights.

The same instrument on the repository's own fixture
`tests/corpus/features/sheet-drawing-across-break.xlsx`: 26.2.4.2 writes
`50.4 749.48 444.756 38.409 re W* n` on both pages, we wrote none, and we now write
`50.4 746.5709 444.6709 41.3291` — horizontal agreement 0.085 pt.

## 3. The rule, in LibreOffice's own source

`ScPrintFunc::PrintPage` prints the drawing layer through `ScOutputData`
(`sc/source/ui/view/printfun.cxx`:1640-1651 and :1695-1713), and the region it hands over is the
page's **cells**, not its paper:

- `ScOutputData::PrePrintDrawingLayer` (`sc/source/ui/view/output3.cxx`:41-104) builds `aRect`
  from the widths of columns `mnX1…mnX2` and the heights of rows `mnY1…mnY2` (`:59-78`) and calls
  `pLocalDrawView->BeginDrawLayers(mpDev, aRectRegion, /*bDisableIntersect*/ true)` (`:95`);
- `SdrPaintView::BeginDrawLayers` passes it through `OptimizeDrawLayersRegion`, which with
  `bDisableIntersect` returns it unchanged (`svx/source/svdraw/svdpntv.cxx`:697-720, :753-778),
  into `SdrPageWindow::PrepareRedraw` → `SetRedrawRegion`
  (`svx/source/svdraw/sdrpagewindow.cxx`:212-224);
- `SdrPageWindow::RedrawAll` / `RedrawLayer` put it on the `DisplayInfo` (`:347`, `:404`);
- `ObjectContactOfPageView::DoProcessDisplay` pushes it onto the device:
  `pOutDev->Push(PushFlags::CLIPREGION); pOutDev->IntersectClipRegion(rRedrawArea);`
  (`svx/source/sdr/contact/objectcontactofpageview.cxx`:163-171).

Cell text does **not** go through it — `printfun.cxx`:1681-1683 sets a clip region and clears it
again immediately, and `DrawStrings`/`DrawEdit` run outside any — which is why a spilled cell
string may leave the block and a shape may not. The `/Figure` marked-content element in the
reference's PDF carries the `W* n` and the `/P` elements do not.

## 4. The fix

`SheetPageGraphics.Draw` computed the block already, and used it only to **cull** — a drawing
anchored in a band this page does not print was skipped. It never **clipped**, so a drawing that
reached the block was drawn whole and the paper was the only bound. The block is now a
`DocRect` (`BlockOf`), the cull is `ReachesTheBlock(box, block)` against it, and a drawing that
`LeavesTheBlock` is painted under `sink.ClipPathKeepingText(GraphicsPath.Rectangle(block))`.

Two decisions, both measured rather than chosen:

- **`ClipPathKeepingText`, not `ClipPath`.** LibreOffice clips the output device and its PDF
  writer goes on emitting the text objects inside the clip. On `044` page 4 the reference paints
  three category labels and `pdftotext` reads all twelve, plus the `BIDE REALTY` of the title
  block whose ink is cut. Measured directly, §6.
- **Conditional.** The clip is emitted only when the box actually leaves the block, so a drawing
  that fits its page is byte-identical to before. Unconditional would have put a `q`/`W n`/`Q`
  round every picture in the corpus for no visible effect — the same reason
  `DrawPicture`'s crop clip is conditional.

## 5. Reach and cost

Whole sheets track, 307 documents, rendered twice from this worktree — once with the patch and
once with `SheetPageGraphics.cs` reverted, `obj`/`bin` cleared on that leg, `SOURCE_DATE_EPOCH`
pinned — and compared byte for byte (`sweep-ours.sh`; one directory per *document*, never per
worker slot):

| | |
|---|---:|
| renderings identical | **233** |
| renderings changed | **74** |

`|ink|%` against the banked 26.2.4.2 reference, summed per document over its pages, before and
after, for those 74 (`score-ink.py`, `ink.tsv`):

| | |
|---|---:|
| summed \|ink\|% before | **316.40** |
| summed \|ink\|% after | **227.20** |
| better / worse / level | **64 / 6 / 3** (one unscoreable, page counts differ before and after alike) |

The largest movers: `TDA_Smoke-Detectors` 12.67 → 2.03, `microsoft_learn_multi_chart_examples`
8.98 → 0.34, `044_Cash_flow_forecast` 8.39 → 1.84, `SIL_TDB648` 9.51 → 3.44,
`EHEST-Pre-departure-checklist` 14.16 → 8.24, `DynamicBubbleChart` 9.05 → 4.55,
`Foreign_SA-CAT-I_and_CAT-II-III_Pub_0` 4.06 → 0.10.

**The six that get worse are one class and worth knowing.** The clip makes *our own* block
boundary visible. On `Template Pilot Logbook JAR-FCL V3.0__xls` page 18
(`pilot-logbook-page18-after.png`, the +0.48) the chart's right-hand slice starts at the same x
on both sides and ours now ends 22 pt short of the reference's, because our page band's right
edge is 22 pt left of theirs — a column-width or band-break difference that was previously
hidden by drawing past it. Same shape as the chart-fit round's eight worseners: *a correct clip
exposes an incorrect edge*. None exceeds +0.48 and the six total +1.44 against the 90.64 the
other 64 give back.

Words and slides are untouched by construction — the change is inside
`XRay.Office.Spreadsheets/Layout` and no word-processing or presentation layout reaches it.

## 6. The gate cannot see any of this, and that is the point

For every one of the 74 movers, page count and alphanumeric character count are **identical**
before and after (checked with `pdfinfo` and `pdftotext` over both sweeps: 0 of 74 differ). A
clip removes ink and leaves the glyphs in the text layer, exactly as it does for the reference,
so no gate column — pages, characters, unembedded fonts — can move in either direction. Score
this on ink and on the drawn objects.

## 7. Tests

`SheetStraddlingDrawingTests` gains `TheStraddlingShapeIsCutAtTheBlockAndNotAtThePaper` (a
`[Theory]` over both pages of the existing fixture, asserting the clip's left and right against
26.2.4.2's own `50.4 … 495.156` and that the box really crosses it) and
`ADrawingInsideItsBlockIsNotClipped` as the control that keeps the clip conditional. Its class
remark carried the wrong half of the rule — *"letting the device discard what falls off the
paper"* — and is corrected.

The negative control is measured rather than asserted: the base binary emits **no clip at all**
on that fixture (`clip-probe.tsv`, rows `base`), so both new assertions fail with the defect put
back.

Suite: nine non-fidelity projects run individually, 5133 passed / 0 failed / 0 skipped;
`XRay.Office.Spreadsheets.Tests` 1229 passed / 0 failed; `XRay.Office.Fidelity.Tests`
**542 passed / 10 failed**, the ten being exactly `PageDrawingComparisonTests.EveryLineIsDrawn` ×4,
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`
and `JustificationShrinkComparisonTests`. Build 0 warnings.

## 8. Left standing

- **`044_Cash_flow_forecast` is not closed at 1.84.** Its remaining ink is the volatile-date
  confound (its months are `EDATE`/`TODAY`-derived and the reference recalculates on load) plus a
  ~1.3 pt horizontal offset of the centred block on the continuation pages.
- **The row-height difference §2 exposes.** A `.xlsx` row stating no `ht` and a workbook stating
  no `sheetFormatPr`: 26.2.4.2 uses 12.800 pt and we compute an optimal 13.777. Not measured
  beyond the one authored probe, and no corpus document is known to reach it.
- **The six worseners' block edges.** Each is a page-band boundary a few points from the
  reference's; that is a column-width or break question and a round of its own.
- **The other four documents from the brief's ranking** — `NAS-Infrastructure-Roadmaps-Weather`,
  `FAA_Form_337`, `pres_ioc_phuket`, `ws_prod-…-European-Safety-Strategy-Initiative` — were
  diffed and not worked. All three of the first read as reflow (`marks displaced or reshaped`
  over most of the page) rather than as a single missing object; `FAA_Form_337` page 67 and
  `pres_ioc_phuket` page 26 each carry a genuine `ink missing from ours` region worth a look.
