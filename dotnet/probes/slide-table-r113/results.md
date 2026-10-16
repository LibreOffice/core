# slide-table-r113 — a `.ppt` table's rows and its rules

Round 113, slides track, seat `agent/slidetable`, two questions handed over by round 112:
**O54** (a `.ppt` table's row heights are the group's own rectangles) and **O55** (its rules are
drawn two and a half times too heavy). The round's own census of O55's reach was cancelled by the
coordinator mid-round — it had already been done, and is `probes/strokes-r113` / **O56** — so what
is here is the two mechanisms and their fix on the `.ppt` side only.

Both close, and they are the same mechanism seen twice: **the reference does not draw a `.ppt`
table's group at all.** `CreateTable` (`filter/source/msfilter/svdfppt.cxx`:7569) discards the
rectangles and the lines and builds one `SdrTableObj`, so the rows come out of `TableLayouter` and
the rules come out of the cells' `BorderLine2`. Neither number is in the file.

Four results.

1. **A rule's width goes through three integer steps and comes out at a fraction of what the file
   states.** `floor(emu / 360)` hundredths of a millimetre, then `max(1, that / 4)` for the
   `BorderLine2`, then that value read as **twips** by the table's view contact — so the drawn
   width is `border / 20` points. `Thailand17` page 11 states 12700 and 28575 EMU and 26.2.4.2
   strokes **0.40 and 0.9499 pt**; this tree strokes 1.0 and 2.25. **O55 closes on the `.ppt`
   side.**
2. **A row's height is `max(stated, minimum)` and then a second layout that can take it back
   down.** Filling the cells lays the table out at the minimums; handing it the group's rectangle
   afterwards makes `NbcSetLogicRect` see a height change and pass `bFitHeight` **true**, and
   `TableLayouter::distribute` does not reset its budget between passes — so a deficit one row
   cannot absorb drives *every* row to its own minimum. That is the whole of
   `2015-Civil-Rights-Website-training` page 48, whose reference rows are all smaller than the
   rectangles that seeded them.
3. **Measured over the ten `.ppt` in the corpus that hold a table group**, on the 34 pages that
   move: stroke widths matching 26.2.4.2's go from **67 of 416 to 412 of 416**, and the worst
   row-edge error over the 22 pages that still differ goes from **249.04 pt to 129.19**, of which
   **127.71 is `architecture6` alone** and is a text-measurement defect the base has too.
4. **`Thailand17` page 11 is now exact in both.** Rules 0.40/0.95 against 0.40/0.9499; nine row
   edges within **0.02 pt**; and the 78 text spans' baselines go from a worst 2.72 pt off the
   reference to **0.03**, mean 1.388 to **0.001**.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidetable`, `agent/slidetable`, base **`f160c8a2c`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used only by `XRay.Office.Fidelity.Tests` |
| corpora | `/home/user/sample-files` (51 `.ppt`, 251 `.pptx`) and `/home/user/corpus-odf` (302 `.odp`) |
| working directory | `/home/user/r113-work` |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is **not** the reference binary's source |

**Both legs, as this track requires.** Every `file:line` below is the `27.2.0.0.alpha0+` checkout
and is the *explanation*; the evidence is 26.2.4.2's own output — its flat ODP of two decks and its
PDFs of ten.

---

# 1. O55 — where a `.ppt` table's rule gets its weight

## 1.1 It is not a line by the time it is drawn

`CreateTable` walks the group twice. Members that are **not** lines become cells; members that are
lines go to `ApplyCellLineAttributes` (`svdfppt.cxx`:7513) and become the neighbouring cells'
`LeftBorder`, `TopBorder`, `RightBorder` or `BottomBorder`. `IsLine` (`:7183`) is an `SdrPathObj`
that is a line with two points, and the only shape `SvxMSDffManager::ImportShape` builds one of is
an unextruded `mso_sptLine` (`msdffimp.cxx`:4403-4412).

So the question is not what width the line states but what width a **cell border** is drawn at, and
there are three steps between the two.

| | |
|---|---|
| `msdffimp.cxx`:1048-1049 | `ScaleEmu` — `rVal = rVal * mfEmu` with `mfEmu = 1/360` for a `Map100thMM` model (`:3228`), assigned back into a `sal_Int32`, so it **truncates**. 12700 EMU → **35**, not 35.28 |
| `svdfppt.cxx`:7519-7521 | `LineWidth = std::max(sal_Int32(1), XATTR_LINEWIDTH / 4)` — integer again. 35 → **8**; 79 → **19**. The reference's own comment is "Avoid width = 0, the min value should be 1" |
| `viewcontactoftableobj.cxx`:183-184 | `constexpr double fTwipsToMM(o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100)); return svx::frame::Style(&aLine, fTwipsToMM);` — the hundredths of a millimetre are scaled **as though they were twips** |

`2540/1440 × 72/2540` is `72/1440`, so one unit is exactly **a twentieth of a point** and the whole
chain is

```
drawn_pt = max(1, floor(emu / 360) / 4) / 20
```

## 1.2 Both ends of it measured at 26.2.4.2

**The file.** `dumpgroup.py` and `pptlines.py` over the ten corpus `.ppt` that hold a table group
(`pptlines.txt`): **377 line members in 34 groups**, five distinct stated widths.

| stated | EMU | → 1/100 mm | → border | → predicted | members |
|---|---:|---:|---:|---:|---:|
| 1 pt | 12700 | 35 | 8 | **0.40 pt** | 262 |
| 1.5 pt | 19050 | 52 | 13 | **0.65 pt** | 3 |
| 2.25 pt | 28575 | 79 | 19 | **0.95 pt** | 97 |
| 3 pt | 38100 | 105 | 26 | **1.30 pt** | 15 |

**The reference's own resolved view.** `soffice --headless --convert-to fodp` on `Thailand17.ppt`
states its cells' borders as `0.23pt` (261 of them) and `0.54pt` (130) — which are 8 and 19
hundredths of a millimetre exactly, the middle step of the chain, before the twips scale
(`fodp-borders.txt`).

**The reference's own PDF.** Page 11, read with PyMuPDF: **13 strokes at 0.4 and 4 at 0.9499**,
against this tree's 13 at 1.0 and 4 at 2.25. `0.9499` is `19/20` to the printer's rounding.

That is the whole of round 112's "2.50 and 2.37, close but not equal": the two ratios are
`35.2778/(4×20)` with a truncation at two of the three steps, and the truncation is worth 2.3 % on
the 1 pt rule and 4.8 % on the 2.25 pt one. **It is one conversion, wrong three times, and not a
substituted default.**

## 1.3 It is not O56

The coordinator's census found 29 documents drawing ≥1.5× the reference's mean stroke width, 27 of
them spreadsheets, 16 of them drawing a flat **0.75 pt** — our own device default standing in for a
resolved set. **This is not that.** Ours drew 1.0 and 2.25 pt on `Thailand17` page 11 and 1.5 pt on
`concepts-surrounding-cloud-computing`, which are the file's own stated widths, faithfully read;
what was missing was the reference's own mangling of them. `.pptx` is clean at 0 of 61 in that
census precisely because `DrawingTableGeometry.BorderWidth` has had the `oox` version of this chain
since it was written — rounding where this truncates and halving where this quarters, because the
two filters build the `BorderLine2` differently. **They are deliberately not one function.**

---

# 2. O54 — where a `.ppt` table's rows come from

## 2.1 The first layout: `max(stated, minimum)`

`CreateTableRows` (`svdfppt.cxx`:7339) seeds the rows from the **non-line** members' snap-rect tops
and measures the last one to the group's own bottom. `TableLayouter::LayoutTableHeight`
(`svx/source/table/tablelayouter.cxx`:724-795) then takes, per row,
`max(stated, tallest single-row cell's getMinimumHeight())`, deferring a row-spanning cell to its
**last** row.

`Cell::getMinimumHeight` (`svx/source/table/cell.cxx`:686-728) is the cell's text laid out at
`TakeTextAnchorRect`'s width — the cell less its left and right text distances — **plus one
hundredth of a millimetre**, plus the upper and lower distances.

On `Thailand17` page 11 that is the whole story: every one of the nine rows is below its minimum,
so nothing is left for the second layout to take back.

| | header | the eight body rows |
|---|---:|---:|
| the group's rectangles | 38.75 pt | 35.87 |
| 26.2.4.2's flat ODP (`ro9`, `ro10`) | **1.383 cm** | **1.277 cm** |
| 26.2.4.2's PDF | 39.20 pt | 36.20 |
| this tree, head | **39.20** | **36.20** |

## 2.2 The second layout, which is a shrink, and which round 112 could not have guessed

`2015-Civil-Rights-Website-training` page 48 is the page that shows it. Its five rectangles state
1270, 1102, 1098, 2037 and 4150 hundredths of a millimetre; the minimums are 1108, 1108, 1108, 1108
and 5024. `max(stated, minimum)` gives 1270, 1108, 1108, 2037, 5024 — and 26.2.4.2's own flat ODP
gives **1.108 cm four times and 5.024 cm once**, `style:use-optimal-row-height="false"` on both
styles. Two rows came out *smaller* than the rectangles that seeded them, which `max` cannot do.

The mechanism is the order `CreateTable` does things in.

1. Filling the cells modifies the table, and every modification re-lays it out
   (`SdrTableObjImpl::update`, `svx/source/table/svdotable.cxx`:640-651), so by the end the
   object's rectangle is **the sum of the maxima**.
2. `CreateTable`:7712 then calls `pTable->SetSnapRect(pGroup->GetSnapRect())` →
   `NbcSetSnapRect` → **`NbcSetLogicRect`** (`svdotable.cxx`:1925-1938), which compares the new
   open height with the current one, finds it different, and calls
   `NbcAdjustTextFrameWidthAndHeight(!bHeight, !bWidth)` — which is
   `LayoutTable(rect, bFitWidth, **bFitHeight = true**)`.
3. `LayoutTableHeight`:849 therefore calls
   `distribute(maRows, rArea.getOpenHeight() - nCurrentHeight)` with a **negative** amount.

And `distribute` (`tablelayouter.cxx`:499-547) does not shrink proportionally in general. It takes
each row's share, clamps any row that fell below its minimum, and runs again — but **`nDistribute`
is not reset between runs**, and the clamping step subtracts the shortfall from it a second time.
So the second pass asks for strictly more than the first, and any deficit large enough to push one
row under its minimum walks the rest of them down to theirs. Traced by hand on this page it is
three passes: `[1270,1108,1108,2037,5024]` → `[929,…,1489,…]` → `[1108,…,1489,…]` →
`[1108,1108,1108,1108,5024]`. That is the reference's answer, exactly.

`bConstrainsBroken` is only ever set by the `rLayout.mnSize < rLayout.mnMinSize` test at the foot of
the loop; the three `|= o3tl::checked_*` above it return true on **overflow**, not on a normal
subtraction, so they never fire.

**Why this could not have been read off `Thailand17` alone.** There, every row is already at its
minimum, so `nCurrentWidth` is zero, `distribute` returns without touching anything, and
`max(stated, minimum)` and "always the minimum" give the same nine numbers. It takes a second
document to separate them, and a third to rule out "always the minimum": measured over the 10
table-bearing documents, "always the minimum" scores **1050.4** against `max`-then-`distribute`'s
**129.19** on the row-edge instrument below.

## 2.3 The one thing not modelled, and why

`getMinimumHeight` measures an **empty** cell as one empty paragraph of the draw outliner's own
font. That font is not in the file and cannot be recovered from it, so a row none of whose cells
this tree can measure keeps **what the file states** rather than contributing zero.

The guard is load-bearing rather than theoretical: without it the last row of
`joint_user_outcomes_michael_fullerton_29.06.12` page 15 is shrunk to **nothing** by `distribute`.
With it that row is 19.25 pt against the reference's 19.42, and the page's worst row-edge error is
**0.17 pt** instead of 19.42.

---

# 3. What changed

Three files, all in the binary-PowerPoint reader.

* **`PptShapeGeometry`** — `LineShape` (20), `ThreeDimensionalFlags` (703) and its `Extruded` bit,
  which together are the reference's `IsLine`.
* **`PptSlideLayout`** — `TableBorderWidth`, the three-step chain of §1.1, applied by `Line` to a
  table group's rule members only; `IsTableRule`; `TableRows`, which builds the grid from the
  group's members and measures each row's minimum through `SlideTextLayout.Height`; and one line in
  `Place` that puts a member on the laid-out grid.
* **`PptTableRows`** (new) — the grid itself, `TableLayouter::distribute`, and the map from a
  member's stated rectangle onto the laid-out one.

Nothing outside a `.ppt` table group can reach any of it: `Context.Rows` is null everywhere else
and `Line`'s new arm is behind `context.InTable`.

---

# 4. Reach, measured

## 4.1 The ten documents that hold a table group

`probes/slides-bullet2-r112/tablecensus.py` over the 51 `.ppt`: **34 table groups in 10 documents,
1089 members**. All ten were rendered at both legs and against 26.2.4.2
(`/home/user/r113-work/t-base`, `t-d`, `t-ref`), `SOURCE_DATE_EPOCH` pinned.

**Stroke widths** (`widthmatch.py`, `widths.txt`) — over the **34 pages** where the two legs differ
at all, how many of our strokes find an unclaimed reference stroke of the same width to within
0.02 pt:

| | matched |
|---|---:|
| base | **67** of 416 |
| head | **412** of 416 |
| reference items on those pages | 416 |

The four that do not match are three rules `concepts-surrounding-cloud-computing` page 11 does not
draw at all and one extra on `pods05` page 56, both of which the base has too.

**Row edges** (`rowedges.py`, `rowedges.txt`) — the worst |Δy| between a leg's drawn horizontal
rules and the reference's, over the pages where any leg differs by more than 0.05 pt:

| | pages | Σ worst |Δy| |
|---|---:|---:|
| base | 22 | **249.04 pt** |
| head | 22 | **129.19 pt** |
| head, excluding `architecture6` | 17 | **1.48 pt** |

`architecture6`'s five pages are **127.71 of the remaining 129.19** and are not this seat: the
reference draws **22** text lines in the wide cell of its page 10 where this tree draws 19, in the
same five subsetted fonts, so the two disagree about the text before they disagree about the row.
The base is equally wrong there (127.66) and the change moves it by 0.05.

**Text baselines.** On the pages where the two legs' text moved at all and the span counts pair:
`Thailand17` 11 worst 2.72 → **0.03** pt, `joint_user…` 15 worst 35.80 → **0.51**,
`undp_presentation_revised_17_may` 11 worst 20.10 → **0.03**, `iep-amount-frequency-for-webinar` 15
0.05 → 0.03.

## 4.2 The whole slide corpus, base against head

`probes/slides-bullet2-r112/confine.py` — pages from the real page tree via PyMuPDF, alphanumeric
characters, and the PDF's md5, one directory per document, deleting each render as it goes, with
**`SOURCE_DATE_EPOCH=1700000000` pinned inside the script**. Both CLIs were built before either
sweep started and neither was rebuilt while one was running.

```sh
probes/slides-bullet2-r112/confine.py <cli> /home/user/sample-files  ppt.list   out.tsv 3
probes/slides-bullet2-r112/confine.py <cli> /home/user/sample-files  pptx.list  out.tsv 3
probes/slides-bullet2-r112/confine.py <cli> /home/user/corpus-odf    odp.list   out.tsv 3
```

| column | documents | rendered both legs | **renderings that move** | page counts differing | alphanumeric counts differing |
|---|---:|---:|---:|---:|---:|
| `.ppt` | 51 | 51 | **10** | 0 | 0 |
| `.pptx` | 251 | 251 | **0** | 0 | 0 |
| `.odp` | 302 | 302 | **0** | 0 | 0 |
| **total** | **604** | **604** | **10** | **0** | **0** |

**The ten movers are exactly the ten documents `tablecensus.py` names**, predicted before the sweep
and with nothing outside the prediction: `2015-Civil-Rights-Website-training`, `Thailand17`,
`architecture6`, `concepts-surrounding-cloud-computing-…`, `iep-amount-frequency-for-webinar`,
`joint_user_outcomes_michael_fullerton_29.06.12`, `outlook_of_nigerian_pension_sector`, `pods05`,
`pres_ioc_phuket`, `undp_presentation_revised_17_may`. The other 594 renderings are byte-identical.
`base-*.tsv` and `head-*.tsv`.

## 4.3 The registered statistic

`sizesweep.py` over the 51 `.ppt`, scored with `slides-r107/sizescore.py` against the banked
26.2.4.2 table (`base-sizes.tsv`, `head-sizes.tsv`). Both legs rendered this round rather than
taken from the record.

| | base = `f160c8a2c` | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **6** | **6** |
| documents holding one | 6 | 6 |
| total \|size error\| over 1534 pages | **47.11** | **47.11** |
| **fixed / newly wrong** | — | **0 / 0** |
| pages whose dominant size moved at all | — | **0 of 1534** |

Unchanged, as it should be: this round moves geometry and ink and not one character's size. The
base column reproduces round 112's head column exactly, and the six that remain are round 111's
raster ceiling (3), the seventh confound (1) and the two terminal pages.

---

# 5. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Debug` at **0 warnings, 0 errors**, then each project alone with
`--no-build`, individually rather than as a solution.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 579 / 579 |
| Markup | 259 / 259 |
| OpenDocument | 160 / 160 |
| Rendering | 164 / 164 |
| Spreadsheets | 1352 / 1352 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1938 / 1938 |
| **Presentations** | **1193 / 1193** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else, confirmed **by name** in the log:
`TabStopComparisonTests` x4 (`list-label-overrun.doc/.docx/.fodt/.odt`),
`PageDrawingComparisonTests` x4 (`paginated.doc/.docx/.fodt/.rtf`),
`SheetDrawingComparisonTests` x1 (`sheet-rich-text.xlsx`) and
`JustificationShrinkComparisonTests` x1 (`justify-shrink-2013.docx`). **No eleventh.** Every project
reported its full total and none was aborted. Presentations' 1193 is the base's 1180 plus the
**13** new `PptTableRowsTests`.

**Whether the new tests would have been red at the base.** The five `TableBorderWidth` cases and
the two clamp cases call a method the base does not have, so "measured failing before" is not
available for them and is not claimed; what is claimed is that their outputs are the widths
26.2.4.2's own PDF strokes, measured in §1.2. The six `PptTableRows` cases likewise call a type the
base does not have — two of them (`ADeficitOneRowCannotAbsorbTakesEveryRowToItsMinimum`,
`ARowShorterThanItsTextGrowsToTheText`) carry the reference's own numbers for the two documents
that separate the three candidate rules, and one (`AGroupTallerThanItsMembersStartsAtItsOwnTopAndSharesTheSlack`)
has no corpus reach at all and says so.

---

# 6. Looking at the page

**The blind reading this round's own predecessor asked for cannot be run from a seat** — a subagent
has no subagent tool in this container, established twice, and round 112's O55 was found only
because the *parent* spawned the reader. So the reading below is the seat's own and contaminated,
and every conclusion above rests on arithmetic that does not depend on it: the three integer steps
and the reference's own `0.23pt`/`0.54pt`, the reference's own `1.383cm`/`1.277cm`/`1.108cm`, and
the two instruments of §4.1.

`Thailand17` page 11, both halves at 110 dpi, composed with `page-vision/scripts/compose.py`
(1100 × 1724, stacked, shown at 100 %): the two halves now read as the same table — same rule
weight, same nine row bands, same text in the same cells. Round 112's reader described our rules as
"two to three times heavier"; that is gone.

**The page worth sending to an independent reader** is `architecture6` page 10, which is the only
remaining error above a point in this family and which no one has looked at: the reference fits 22
lines into the wide cell of a two-column table and this tree fits 19, at the same font and the same
size, and the question a reader could answer that a metric cannot is *what the extra three lines
are*.

---

# 7. What remains open

| | what it is | where |
|---|---|---|
| `architecture6` pages 10, 14, 21, 24, 27 | the reference wraps a wide table cell into 22 lines where this tree makes 19, so the row minimums differ by up to 55 pt. Not a row-height rule and not a font substitution — the same five subsetted faces on both sides. Base and head are equally wrong | **new seat, see below** |
| the rules' end overshoot | the reference extends every border by **half the crossing border's pen** at each end — 0.475 pt on `Thailand17` page 11 — and this tree draws the line member's own extent. `SlideTable` already models this for `.pptx`; the `.ppt` path draws the group's line shapes and does not | ~0.5 pt, not seated |
| an empty cell's minimum | the reference measures one empty paragraph of the draw outliner's own font; this tree keeps the stated row height instead. Worth 0.17 pt on the one page where it is visible | §2.3 |
| `SlideTable.RowHeights`' span subtraction | the shared `.pptx`/`.odp` reader subtracts every covered row from a row-spanning cell's deferred minimum; the reference's loop guard `(nMRow > 0) && (nMRow < nRow)` skips the subtraction entirely when the span starts at row 0 (`tablelayouter.cxx`:830). Reproduced in the `.ppt` path, **not** changed in the shared one — that is a different reader and needs its own measurement | unmeasured |
| `SlideTable` and `distribute` | the shared reader has `max(stated, minimum)` and no second layout at all, so a `.pptx` table whose rows overflow its frame is not walked back down. Whether an `a:tbl` reaches `NbcSetLogicRect` the same way is not established here | unmeasured |

## Seat numbers

O54 and O55 both close on the `.ppt` side. **`architecture6`'s wide-cell line count is taken as
O57** — O56 is the coordinator's spreadsheet-border class, pushed this round as `c7d39b8e7`, and a
parallel round is live, so the parent resolves any collision.
