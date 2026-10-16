# Two chart-layout seats that are not one seat: a fly cut down to its page, and tdf#48041

Round 97, seats **O11** and **O23**, worktree `/home/user/wt-chartfit2`, branch
`agent/chartfit2`, base `1d4ef72a3` (HEAD merged in before the round). Reference
`/opt/libreoffice26.2/program/soffice` (LibreOffice 26.2.4.2). **The reference half of every
rendering figure is the bank at `/home/user/gate-orig-r83/ref/`, reused rather than
re-rendered**; only our half was swept, which a diff confined to `dotnet/src` makes sound.

This round resumes `72c1472e2`, a WIP salvaged from a container restart: unbuilt, untested,
with no write-up and no conclusions to re-check. Everything below is this round's own
verification of it, and the two places it was wrong are marked.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. Nothing here rests on a reading: every reference figure is a number pulled out
of 26.2.4.2's own `.fodt` or its own PDF.

---

## 0. The brief's hypothesis, tested first, and refuted

The brief asked for one thing before anything else: *test the shared-cause hypothesis before
splitting the work.* The proposal was that both seats are the same defect — that we lay a chart
out at the wrong size, so the reference's estimator and ours see different lengths and the
1/2/5 ladder lands one rung apart.

**It is refuted, by one measurement, and the two seats have nothing in common.**

`axis-variants.py` rebuilds `048_Expense_trends_budget` with **one attribute changed at a time**
and renders each through 26.2.4.2 (`axis-variants.txt`):

| variant | what changed | 26.2.4.2 draws | axis span | label height |
|---|---|---|---|---|
| `base` | nothing | step **100** — `500 400 300 200 100` | 120.80 pt | 9.04 pt |
| `fmt` | `c:numFmt formatCode` alone, `#,##0;;` → `#,##0` | step **50** — `500 450 … 50 0` | 151.01 pt | 9.04 pt |
| `nozoom` | the sheet's `fitToPage` and `pageSetup/@scale` alone, removed | step **100** | 180.29 pt | 13.42 pt |
| `both` | the two together | step **50** | 225.39 pt | 13.42 pt |

`nozoom` is the whole answer. **Making the chart half again as large does not change the step**,
and 180.29/13.42 = 13.4 label heights is as far over the ten-interval ceiling as 120.80/9.04 =
13.4 already was — the length cap is saturated in every row and cannot be what separates them.
The **format code** is what separates them, in both size regimes. Size is not in it, so O11 is
not a scaling defect and O23's fix cannot be its fix.

The two turn out to be:

* **O23** — a Writer *fly* bigger than its page is cut down to the page, proportionally when it
  holds a picture or an OLE object. Nothing to do with `ChartLayout`; the seat was misfiled, and
  §1 says why round 95 could not have seen it.
* **O11** — tdf#48041: an axis whose ticks format to the same string twice has its automatic
  interval cap lowered, and the comparison starts against an **empty** string. Nothing to do
  with size.

---

## 1. O23 — the frame does not coincide, and the flat ODT says so in eight seconds

Round 95 (`probes/words-close-r95/results.md` §1.5) established that the frame's **position**
coincides: `023`'s chart is 682.1 pt wide on a 595.3 pt page, so the horizontal clamp saturates
and three different stated offsets give one rendering on both sides. That is correct and this
round does not disturb it. What it did not ask is what **size** the reference resolves, and the
answer is that it is not the stated one.

`framesize.py` rewrites one `wp:extent` at a time and converts to `.fodt`, which prints
26.2.4.2's own resolved model without rendering anything (`framesize.txt`). The chart states
682.10 x 493.50 pt on a 595.30 x 841.89 pt page:

| stated | 26.2.4.2 resolves | |
|---|---|---|
| 682.10 x 493.50 (as authored) | **595.30 x 430.70** | width alone over |
| 708.66 x 157.48 | **595.30 x 132.29** | width alone over |
| 314.96 x 905.51 | **292.79 x 841.89** | height alone over |
| 708.66 x 905.51 | **595.30 x 760.65** | both over, width by more |
| 393.70 x 236.22 | 393.70 x 236.22 | neither — unchanged |
| 341.04 x 246.75 (half) | 341.04 x 246.76 | neither — unchanged |

Every squeezed row keeps the stated aspect ratio to within a twip: 493.50 x 595.30/682.10 =
430.72, 905.51 x 595.30/708.66 = 760.66, 314.96 x 841.89/905.51 = 292.83.

### 1.1 The mechanism

`SwFlyFreeFrame::CheckClip` (`sw/source/core/layout/flylay.cxx`:471, called from `MakeAll`
at :251). Read in **this tree**, which is `27.2.0.0.alpha0+` and not the reference binary's
source — the measurements above and in §1.2 are the leg that stands on 26.2.4.2 itself.

* :493 — `if ((bBot || bRig) && !IsDraggingOffPageAllowed(FindFrameFormat(GetDrawObj())))`.
* :514 and :534 — it **first gives up the position**: `SetPosX(max(aClip.Left(), nClipRig -
  aFrm.Width()))` and the same down the page. If the position actually moved, `bAgain` is set,
  the size is invalidated and the frame is laid out again; **only if it did not move** does
  control reach the squeeze. For a frame that *fits* the page, moving it always resolves the
  overflow, so the frames whose size this changes are exactly those **bigger than the page** —
  which is why the model here is a size test and not a move plus a test.
* :571-585 — the two axes are cut to the clip rectangle independently.
* :598-627 — and then made proportional when `Lower()->IsNoTextFrame()` and the lower is an
  OLE node or its environment is not auto-sized: when both axes were cut, **the bigger change
  is the relevant one** — the other axis is restored to what it was and recomputed from the
  ratio.
* :638-648 — for an **OLE node only**, the squeezed size is written back into the frame format.
  That is why `--convert-to fodt` can see it at all.
* The escape is `SwAnchoredObject::IsDraggingOffPageAllowed`
  (`sw/source/core/layout/anchoredobject.cxx`:790-801): `DISABLE_OFF_PAGE_POSITIONING` **and**
  a wrap-through object. `sw/source/writerfilter/filter/WriterFilter.cxx`:333 sets the setting,
  one line below the `DoNotCaptureDrawObjsOnPage` that O8 already carries — so it is on for
  DOCX and RTF and off for WW8 and ODF.
* An **as-character** object never reaches any of this: `SwFlyInContentFrame` derives from
  `SwFlyFrame` and not from `SwFlyFreeFrame` (`sw/source/core/inc/flyfrms.hxx`:212 against
  :150), and `CheckClip` is the latter's method.

### 1.2 Four one-attribute variants that separate the wrap from the node kind

The flat ODT can only show an OLE object, because the write-back is OLE-only. `nodekind.py`
renders instead and reads the drawn width back out of 26.2.4.2's PDF:

| variant | node | wrap | 26.2.4.2 draws |
|---|---|---|---|
| the chart as authored | OLE | `wrapSquare` | **595.30 x 430.70** at x = 0 |
| `chartnone` — wrap alone | OLE | `wrapNone` | **not squeezed** (its pie grows by 682.10/595.30 = 1.146; 309.95 x 409.10 against 270.55 x 356.90) |
| `picthrough` — the picture given the chart's extent | graphic | `wrapNone` (as authored) | **not squeezed** — 682.10 x 493.50, running 334 pt off the sheet |
| `picsquare` — the same, wrap alone | graphic | `wrapSquare` | **595.30 x 430.70** at x = 0 |

So the **wrap** decides and the node kind does not, which is `IsDraggingOffPageAllowed`
measured rather than read; and a graphic node is squeezed exactly as an OLE node is.

Two more, from the corpus rather than from a synthesised variant, read out of the banked
reference PDFs:

* `fleetfastfacts16nov2023.docx` anchors a 606.90 x 231.60 pt picture, `wrapSquare`, on a
  595.30 pt page. 26.2.4.2 draws it **595.30 x 227.20 at x = 0** — 231.60 x 595.30/606.90 =
  227.17.
* `tibs_guidelines_2.docx` holds a **686.20 pt `wp:inline` picture on a 612 pt page** and
  26.2.4.2 draws all 686.20 pt of it, off the sheet;
  `PES-Technical-Report-Template_Jan_2019.docx`'s 612.50 x 792.70 pt inline picture on a
  612 x 792 pt page likewise. That is the as-character exemption, measured.

### 1.3 Two things the salvaged WIP had wrong, and one it had right by construction

* It claimed the picture arm from the flat ODT's `picbig` row — "the same document's picture
  given the chart's extent keeps 682.10 x 493.51, which is the write-back being OLE-only and
  not the squeeze being OLE-only". The conclusion is right and the row does not support it:
  the picture in that document is `wrapNone`, so it is exempt for a *different* reason and the
  row is consistent with both readings. §1.2's `picsquare`/`picthrough` pair is what separates
  them, and it needed a rendering, not a `.fodt`.
* Its line numbers check out against this tree and were left alone: `CheckClip` 471-706, the
  escape's guard :493, `CalcClipRect` :1232, the write-back :638. The one that does not is the
  test file's `:1577-1616` for tdf#48041's block, which is :1578-1600 with the lowering at
  :1611-1615; corrected.
* It never handled as-character objects — and does not have to, because `FrameLayout.Place` is
  not called for one (`FrameLayout.cs`:722 skips them; they hang on a line in `HangInline`).
  That is luck rather than design, so it is now written down in both the method and the test.

---

## 2. O11 — tdf#48041, and the comparison that starts against an empty string

`VCartesianAxis::estimateMaximumAutoMainIncrementCount`
(`chart2/source/view/axes/VCartesianAxis.cxx`:1559-1616, this tree) is the whole of the
automatic interval cap. `probes/chart-axis-r87` ported its length arm — `nTotalAvailable /
nSingleNeeded`, :1610 — and stopped there. The other arm is :1578-1600:

```
// tdf#48041: do not duplicate the value labels because of rounding
if (m_aAxisProperties.m_nAxisType != css::chart2::AxisType::DATE)
{
    FixedNumberFormatter aFixedNumberFormatterTest(...);
    OUString sPreviousValueLabel;                       // :1582
    sal_Int32 nSameLabel = 0;
    for (auto const & nLabel: m_aAllTickInfos[0])
    {
        OUString sValueLabel = aFixedNumberFormatterTest.getFormattedString(...);
        if (sValueLabel == sPreviousValueLabel) { nSameLabel++; ... }
        else nSameLabel = 0;
        sPreviousValueLabel = sValueLabel;
    }
}
...
if ( nMaxSameLabel > 0 )                                 // :1611-1615
{
    sal_Int32 nRetNoSameLabel = m_aAllTickInfos[0].size() / (nMaxSameLabel + 1);
    if ( nRet > nRetNoSameLabel ) nRet = nRetNoSameLabel;
}
```

**`sPreviousValueLabel` is seeded empty and the first tick is compared against it before it is
ever assigned.** An axis whose first tick formats to nothing therefore scores one repeat
although no two of its labels are alike, and its cap is halved. That is not a curiosity: it is
the whole of `048_Expense_trends_budget`.

`048`'s value axis states `c:numFmt formatCode="#,##0;;"` — a positive section and two empty
ones, so zero and every negative draw as nothing — over a 0…500 range. The first pass runs at
the ceiling of ten intervals, so `m_aAllTickInfos[0]` holds eleven ticks 0, 50, … 500; the
first formats to `""`, which equals the seed; `nMaxSameLabel` is 1; `11 / 2 = 5`; and five
intervals over 0…500 is the 1/2/5 x 10^k ladder's **100** where ten of them is its 50
(`chart2/source/view/axes/ScaleAutomatism.cxx`:878-895 for the ladder,
`setMaximumAutoMainIncrementCount` at :142-150 for the floor of two and the ceiling of ten).

The one-attribute variants in §0 are the 26.2.4.2 leg: with `formatCode` alone changed to
`#,##0` — one tick fewer formatting to nothing — the reference itself steps by 50.

**`probes/chart-axis-r86/findings.md` carries a retraction and this round did not build on it.**
Its claim that `MaximumAutoIntervalCount = 10` is a misread constant is wrong (it is the first
pass' ceiling: `lcl_getMaximumAutoIncrementCount`, `ScaleAutomatism.cxx`:43-49, returns 10 for
every axis type but `DATE` and `MAXIMUM_MANUAL_INCREMENT_COUNT` for that one), and its step formula is the logarithmic branch. Neither is used here.

---

## 3. What was implemented

Two changes, in two files, each guarded by the condition its own arm measured.

**`FrameLayout.Squeezed`** (`dotnet/src/XRay.Office.WordProcessing/Layout/FrameLayout.cs`), applied
to what `Place` returns. A frame wider or taller than its page is cut to the page; when it holds a
picture or a chart the aspect is kept, and when both axes are over, the axis whose change is
larger decides and the other is recomputed from the ratio. It does nothing to a frame that fits,
nothing to a drawing object (which is an `SwAnchoredDrawObject` and not a fly), nothing to an
as-character object (`Place` is not called for one), and nothing to a wrap-through object under
`DisableOffPagePositioning`.

**`PaginationOptions.DisablesOffPagePositioning`**, off by default and set true by the DOCX
reader only. The RTF reader goes through the same writerfilter and `WriterFilter.cxx`:333 sets the
setting for it too, but `PaginationOptions.CapturesAnchoredObjectsOnPage` is true there, so RTF
captures everything and its frames never reach the branch — which keeps `probes/rtf-shape-r73`'s
21 measured probes exactly where they were.

**`ChartLayout.IntervalsThatReadDifferently`**
(`dotnet/src/XRay.Office.Core/Charts/ChartLayout.cs`), the second half of the automatic interval
cap. It formats the previous pass' ticks through the axis' number format, counts the longest run
of consecutive equal strings — starting the comparison against the empty string, as :1582 does —
and lowers the cap to `ticks / (longest + 1)`. A date axis is exempt, as :1578 is.

### 3.1 Tests

| test | fails at the base | passes after |
|---|---|---|
| `ChartAxisRepeatedLabelTests` (3) | `AnEmptyFirstTickHalvesTheIntervalCount` | all 3 |
| `FrameSqueezedToPageTests` (14) | 9 of 14 | all 14 |

The whole suite at HEAD, read out of each run's own last line: the ten non-fidelity projects
**0 failed of 6482** (Containers 109, Core 524, Markup 259, OpenDocument 146, Presentations
1045, Rendering 164, Spreadsheets 1271, Text 728, Vector 309, WordProcessing 1927), and
`XRay.Office.Fidelity.Tests` **542 passed / 10 failed of 552** — PageDrawing x4, TabStop x4,
SheetDrawing, JustificationShrink. The same run was taken at the base build before the sweeps,
and it printed the same ten names and the same 542 / 10 / 552, so the baseline is this session's
own measurement and not a number carried in from the brief.

"At the base" here is the two mechanisms switched off in place rather than the files reverted:
the base signature of `FrameLayout.Place` has no `disablesOffPagePositioning`, so a revert makes
the round's own tests fail to *compile*, and a compile error is not a failing test.

### 3.2 What O23 looks like now

`spans.py` pairs the two renderings' text spans **on their text** rather than on their index —
one unmatched span shifts an index pairing by one and turns an agreeing page into a table of
hundred-point displacements, which is how a first run of this read.

`023_Unit_Circle_Chart_Circular_Percentage`, ours against the banked 26.2.4.2
(`spans-023-before.txt`, `spans-023-after.txt`):

```
            before              after
      dx       dy         dx       dy   text
   -0.13    -0.04      -0.13    -0.04   Unit Circle
      --       --         --       --   Chart                  (ours only, both)
   41.98     6.58       0.10    -0.64   4th Qtr
   41.96     6.61       0.09    -0.61   9%
   32.51    12.65       0.17    -0.56   3rd Qtr
   32.40    12.68       0.05    -0.53   10%
   66.15    30.98       0.22    -0.30   1st Qtr
   28.56    31.84       0.09    -0.27   2nd Qtr
   66.07    31.02       0.15    -0.26   59%
   28.56    31.88       0.08    -0.24   23%
   -0.08     0.00      -0.08     0.00   (the eight body paragraphs)
worst |dx| or |dy|     66.15              0.64
```

The eight pie data labels the seat was filed for — *"32 pt right of 26.2.4.2's"*, and they are
28.6 to 66.2 pt right of them — are now within **0.22 pt in x and 0.64 pt in y**.

**One span is left and it is not this seat's.** The 36 pt heading `Unit Circle Chart` sits in a
331 pt text box; we draw its second line and 26.2.4.2 draws only the first. That is text-box
overflow, and it is **not caused by the squeeze**: 26.2.4.2 draws only `Unit Circle` in the
`chartnone` and `picsquare` variants too, where the chart frame is not squeezed at all.

---

## 4. Corpus reach, measured

Both arms were swept over the whole corpus at `SOURCE_DATE_EPOCH=1757462400`, our half only,
`sweep-ours.py` with `movers.py` over the md5 of each rendering. **947 of 947 rendered on every
run and none failed.**

### 4.1 The axis arm: 1 of 947

Swept with the arm switched off and again with it on, over all 947 documents:

```
compared 947 of 947; differ 1
MOVED   048_Expense_trends_budget_18d1e8ba__xlsx   sheets/chartset-014/xlsx/048_...xlsx
```

`048`'s value axis, read out of the three PDFs with `axisticks.py`:

| | labels | step | pitch | span |
|---|---|---|---|---|
| ours, before | `500 450 400 … 50` | 50 | 15.102 | 135.92 |
| ours, after | `500 400 300 200 100` | **100** | **30.204** | **120.82** |
| 26.2.4.2 | `500 400 300 200 100` | **100** | **30.199** | **120.80** |

Sum `\|ink\|%` over its 14 pages 0.24 → 0.23; no page count anywhere.

`census.py` predicted **2** documents whose value axis states a `formatCode` with an empty
section, and the second — `034_Personal_net_worth_calculator`, `formatCode=";;;"` on two charts —
does **not** move. Its every section is empty, so every tick formats alike, which in the C++
lowers the estimate to `11 / 11 = 1` and then to the floor of two; it does not move here because
the attribute also carries `sourceLinked="1"` and this tree takes the source cell's format.
Whether 26.2.4.2 does the same is **not settled**: the axis draws no labels either way, so the
rendering cannot be read for it, and the ink is 0.01 over seven pages on both sides.

### 4.2 The frame arm: 5 of 338 word-processing renderings

`FrameLayout` lives in `XRay.Office.WordProcessing` and no spreadsheet or presentation reader
calls it, so the frame arm was swept over the `words/` column — **338 of 338** — with the arm
off and again with it on:

| document | before | after | |
|---|---:|---:|---|
| `023_Unit_Circle_Chart_Circular_Percentage` | 2.83 | **0.59** | the seat |
| `fleetfastfacts16nov2023` | 0.74 | **0.04** | its one MAJOR page gone |
| `028_Unit_Circle_Chart_Optimized_Graph` | 3.59 | **3.47** | |
| `b053-19` | 0.50 | 0.50 | level on ink; see below |
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 1.84 | **3.47** | **worse** |
| | **9.50** | **8.07** | 3 better, 1 level, 1 worse |

No page count changes on any of the five, and the other 333 word renderings are byte-identical.

**`b053-19` is level on ink and is the cleanest confirmation of the rule in the round.** Its
running head holds a picture 612.5 pt wide on a 612 pt page. 26.2.4.2 draws it `0.0 … 612.0`
by `14.2 … 101.95`; we drew `0.0 … 612.5` by `14.25 … 102.0` and now draw
`0.0 … 612.0` by `14.25 … 101.93`. The ink does not notice half a point on a header rule; the
geometry does.

**`021` is worse and the cause is not the frame.** Its chart is a *3D pie*, and its eight data
labels are 17 to 83 pt away from 26.2.4.2's **before** this change and 23 to 68 pt away after —
the frame it sits in is now the reference's own (`framesize.py`: 647.35 x 375.15 pt stated,
595.30 x 344.94 resolved, and we place 595.30 x 344.9), and the labels inside it are placed by an
algorithm that is wrong either way. Putting the frame right moved wrong labels to a place that
overlaps the reference's less. **This is a real regression on that document and it is not
defended**; it is seated as **O31** rather than worked here, because fixing it means the 3D
pie's label placement and not the fit.

### 4.3 One arm was wrong, the corpus caught it, and this is what it cost

The first words sweep found **8** movers, not 5. The three extra were
`ABCD-WB-08-00 Weight and Balance Report`, `ABCD-FE-01-00 Flight Envelope` and
`ABCD-SDE-23-00 - Avionic System Description`, and in each the thing that changed was a
`wrapSquare` `wp:anchor`/`wps:txbx` banner in a running head — 739.25 x 56.85 pt on a 595.30 pt
page for the first two, 839.80 x 56.85 for the third — which we had begun cutting to 595.30.

**26.2.4.2 draws all 739.2 and 839.8 pt of them**, read out of the banked reference PDFs. A
shape carrying a text box is a *draw* object paired with a fly: the fly holds the text and goes
through `CheckClip`, and the shape — the rectangle that is actually drawn — stays an
`SwAnchoredDrawObject` and never does. `PageFrame` models the drawing, so `FrameObjectKind`
`TextBoxShape` had to be exempt beside `Shape`. It is the opposite answer from
`CapturesAnchoredObjectsOnPage`'s `bConsidered`, which treats the same object *as* a fly, and
copying that reading is how it went wrong.

Three documents of ink on which the score barely moved at all: 2.08 → 2.08, 7.40 → 7.40 and
unscoreable (`ABCD-FE-01-00` renders 14 pages against the reference's 16, on **both** sides, so
`pdf-image-diff.py` produces no rows for it). **Ranking on the total would not have found this.**
What found it was asking, for every mover, what the reference draws there.

### 4.4 Confinement

* Axis arm: **946 of 947** renderings byte-identical.
* Frame arm: **333 of 338** word renderings byte-identical; the other 609 are not reachable —
  no spreadsheet or presentation reader constructs a `PageFrame`.
* No document in the corpus changed its page count, and none failed to render on any of the
  four sweeps.

### 4.5 Where `census.py` under-counts, said plainly

It predicted 4 squeezable OOXML word documents and the sweep found 5. The one it missed is
`b053-19`, whose oversize picture is in `word/header1.xml`: the census reads
`word/document.xml` alone. It also cannot see `.doc`, `.rtf` or `.odt` at all. Its 21 `oversize`
and 4 `squeezed` are therefore lower bounds and the sweep is what measures reach — 8 of the 22
"oversize" anchors it *does* find are `wp:inline`, which is 36 %, so a census of stated extents
without the inline test would have over-predicted by about a third in the other direction.

---

## 5. What this does not settle

* **The text-frame arm is read, not measured.** `CheckClip` cuts the two axes of *any* free fly
  before it asks whether the lower is a `SwNoTextFrame`, so a text frame bigger than its page is
  cut non-proportionally. The corpus states no witness: the only oversize text boxes in it are
  `docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx`'s three, and all three are
  exempt for other reasons (two `wp:inline`, one wrap-through). The arm is in because leaving it
  out would need its own justification; its reach here is **nil**, and that is a census, not a
  failure to find a witness.
* **Three of `CheckClip`'s own sub-conditions are not modelled**, each of which only stops the
  frame being *moved* first and therefore squeezes a frame that fits: a fly in a header
  (:505-507), a fly carrying anchored objects of its own, and a fly whose anchor is in a table
  (both :502). So is the `HoriOrientation::LEFT` restore at :541-546, which likewise squeezes a
  frame that fits. Modelling any of them means squeezing frames that fit the page, which is the
  direction that regresses, and none has a corpus witness.
* **tdf#112443's gate on the *move*** (`!bDisableOffPagePositioning || nOld <= nClipBot`, :512 and
  :531) is not modelled either. It can only leave a frame where it was; the squeeze that follows
  is unaffected.
* **`.doc`, `.rtf` and `.odt` were swept but not censused.** `census.py` reads OOXML markup only,
  so its four predicted documents are a lower bound on the OOXML side and say nothing about the
  other three readers. §4's sweep is what covers them.
* **The C++ tree is not the reference binary's source.** `configure.ac`:21 is
  `27.2.0.0.alpha0+`; the binary is 26.2.4.2. Every line number above is *this tree*. The
  26.2.4.2 leg of each arm is the `.fodt` and the rendered PDFs, which stand on their own.

### 3.3 A third thing the WIP had wrong, and this one would have cost a regression

Its prose said `DisableOffPagePositioning` is *"set by `WriterFilter.cxx`:333 for DOCX **and**
RTF"*, and that the RTF reader leaves it false only because 21 probes in `probes/rtf-shape-r73`
found no shape using the escape. Both halves are wrong, and a later round acting on the prose
rather than the code would have set it for RTF.

`WriterFilter.cxx`:333 is the **OOXML** filter's `setTargetDocument`.
`filter/source/config/fragments/filters/Rich_Text_Format.xcu` names
`com.sun.star.comp.Writer.RtfFilter` as RTF's `FilterService`, and
`RtfFilter::setTargetDocument` (`sw/source/writerfilter/filter/RtfFilter.cxx`:191-195) assigns
`m_xDstDoc` and sets no document property at all. Grepping the whole of `sw/` for the setting
finds **one** setter outside `SwXDocumentSettings`' own plumbing: that line. The two filters share
a tokeniser and a domain mapper, not a settings block — RTF does not get
`DisableOffPagePositioning`, and `probes/rtf-shape-r73` was measuring
`DoNotCaptureDrawObjsOnPage`, a different setting one line above, on draw objects that never
reach `CheckClip` in the first place.

The **code** was right: only `DocxReader` sets it, so an oversize RTF, `.doc` or `.odt` fly is
squeezed whatever it wraps, which is what the reference does. Only the comments needed fixing,
and they are fixed in all four places they appeared.

---

## 6. Where each seat ends

**O11 — closed, fixed in this tree.** Mechanism read out of `VCartesianAxis.cxx`:1578-1615 and
confirmed at 26.2.4.2 by one-attribute variants; three tests, one of which fails at the base;
reach **1 of 947** renderings, and on that one the drawn label set, step and pitch become the
reference's.

**O23 — closed as stated, with a named residual that is not it.** The seat is
*"`023` draws its pie's data labels 32 pt right of 26.2.4.2's inside an identical frame"*, and
both halves are now answered: the frames were **not** identical — the reference resolves
595.30 x 430.70 where the file states 682.10 x 493.50 — and once ours does too, the eight labels
land within **0.22 pt in x and 0.64 pt in y**. Mechanism read out of `flylay.cxx`:471-706 and
measured at 26.2.4.2 through the flat ODT and through four rendered one-attribute variants;
fourteen tests, nine of which fail at the base; reach **5 of 338** word renderings.

Two things are left seated rather than claimed:

* **`021_Unit_Circle_Chart_3D_Pie_Chart` is worse** — 1.84 → 3.47 — and its cause is the 3D
  pie's own data-label placement, which is 17-83 pt out before and 23-68 pt out after. Seated as
  **O31**.
* **`023`'s own remaining span** is its 36 pt heading's second line, drawn by us and not by
  26.2.4.2, in a 331 pt text box. Text-box overflow, and present in the reference's unsqueezed
  variants too, so not this seat's.

**The brief's shared-cause hypothesis is refuted** (§0) and the two seats had nothing in common.
The brief's own ink figures — `023` 16.424, `021` 12.663, `027` 11.265 — are on a different
scale or a different tree from `score-ink.py`'s: this round measures `027`, which neither arm
touches and whose rendering is byte-identical before and after, at **3.22** on both sides.

---

## 7. Files

| file | what |
|---|---|
| `axis-variants.py`, `axis-variants.txt` | `048`'s value axis, one attribute changed at a time, rendered by 26.2.4.2 |
| `axisticks.py` | the value-axis label reader, copied from `probes/chart-axis-r87` |
| `framesize.py`, `framesize.txt` | what size 26.2.4.2 resolves an anchored object to, out of the flat ODT |
| `nodekind.py`, `nodekind.txt` | which node kinds and wraps it squeezes, rendered |
| `spans.py`, `spans-023-before.txt`, `spans-023-after.txt` | drawn text spans paired on their text, with displacements |
| `census.py`, `census.tsv` | the markup census behind §4.5 |
| `sweep-ours.py`, `movers.py`, `score-ink.py` | the sweep, the hash diff and the ink ranking |
| `sweep-keep.txt` | which renderings the sweeps kept on disk |
| `sweep-after-hashes.tsv` | the whole-corpus sweep at HEAD |
| `sweep-axis-off-hashes.tsv` | the whole-corpus sweep with the axis arm off |
| `sweep-frame-off-words-hashes.tsv`, `sweep-frame-on-words-hashes.tsv` | the `words/` sweeps either side of the frame arm |
| `ink-frame-movers.tsv`, `ink-axis-mover.tsv` | the per-document scores in §4.1 and §4.2 |
