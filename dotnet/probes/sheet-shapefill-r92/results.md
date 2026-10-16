# A BIFF shape's fill and outline are a palette reference, and the reader honoured only literals

Round 92, seated by `probes/sheet-shapefill-r90/findings.md`. Base `ddadc94c1`, worktree
`/home/user/wt-shapefill`, reference `/opt/libreoffice26.2/program/soffice` — LibreOffice
26.2.4.2 — with the five tarball font confounds moved aside as of 2026-09-07. The reference half
of every rendering figure is the bank at `/home/user/gate-orig-r83/ref/`, reused rather than
re-rendered; only our half was run, which a diff confined to `dotnet/src` makes sound.

## 1. What was wrong

`sheets/done-014/xls/TICAPCapability_Final.xls` passes the gate and diverged on summed unsigned
ink — **20.66 %** measured here, against the seating probe's 16.87 % with its own instrument — with
7.28 % of it on page 3 alone: the reference draws a white panel and a 0.42 pt black border around
the `Instructions` sheet's text and we drew neither.

Round 84 read the four Escher ink properties for the first time and took two positions, both
recorded in `EscherInk`'s own remarks as deliberate conservatism:

- **presence is the test** — a colour is taken only where the shape states one, although MS-ODRAW
  defaults `fillColor` to white and `lineColor` to black;
- **only the literal `MSO_CLR` form is honoured** — the top byte decides whether the other three
  are a literal `0x00BBGGRR` or a reference, and a reference yielded nothing rather than a wrong
  colour.

Both are wrong, and together they read almost nothing. Censused over the 64 corpus `.xls`
(`census-msoclr.py`, output in `msoclr-census.txt`): of the **106** fill and line colours a
worksheet shape states, **14 are literals and 92 are palette references**.

| form | count | documents |
|---|---:|---:|
| `fill` scheme index 80 (note background) | 35 | 6 |
| `line` scheme index 64 (window text) | 29 | 3 |
| `fill` **literal** | 12 | 4 |
| `fill` scheme index 67 (button face) | 11 | 2 |
| `fill` scheme index 78 (chart window background) | 6 | 5 |
| `fill` scheme index 9 | 4 | 1 |
| `fill` scheme index 65 (window background) | 3 | 1 |
| `line` scheme index 77 | 2 | 1 |
| `line` **literal** | 2 | 1 |
| `fill` scheme index 43, 22 | 2 | 2 |

**Reach: 12 of the 64 `.xls` state a scheme reference on a worksheet shape.** No `syscolor` form
occurs anywhere, which is why the system-colour branch is left unimplemented.

TICAP's two `Instructions` text boxes state `fillColor 0x08000041` and `lineColor 0x08000040` and
nothing else at all — palette 65 and 64, Excel's *window background* and *window text*, so white
and black (`XclDefaultPalette::GetDefColor`, `sc/source/filter/excel/xlstyle.cxx`:141-163).

## 2. Three rules, and only one of them is "the shape said so"

`EscherColour` is `SvxMSDffManager::MSO_CLR_ToColor` (`filter/source/msfilter/msdffimp.cxx`:3420),
and `EscherInk` now follows `ApplyFillAttributes`/`ApplyLineAttributes` (`:904`, `:1313`) rather
than reading the two booleans with a hardcoded fallback.

- **The colour is usually a reference.** Scheme when `nUpper & 0x08` (index = the low *word*) or
  when `nUpper & 0x19` without `0x10` (index = the top byte); a bare `nUpper & 4` with no low bits
  is a third scheme form; everything else is the literal. An unresolvable reference falls back per
  property — white for a fill, black for a line.
- **Absence is not "no ink".** `mso_PropSetDefaults` (`filter/source/msfilter/dffpropset.cxx`)
  gives property 385 the value `0xffffff` and 448 zero, so a filled shape stating no colour is
  white and a stroked one stating none is black. `014_Contextures_chart_sample_991ecfc5.xls`'
  `Rectangle 6` is the corpus witness: `fLine` true, no `lineColor`, and 26.2.4.2 draws
  `svg:stroke-color="#000000"`.
- **An unstated boolean is the *shape type's* answer.** The same defaults table gives property 447
  the value `0x001C` and 511 `0x001E`, so `fFilled` and `fLine` both default true — and
  `ApplyFillAttributes` then clears the bit again unless the shape stated it *hard* or the type is
  filled (stroked) by default. `mso_DefaultFillingTable` and `mso_DefaultStrokingTable`
  (`svx/source/customshapes/EnhancedCustomShapeGeometry.cxx`:6156-6213) are the tables, and they
  reduce to `stated ? statedValue : defaultForType`. A text box and a rectangle are filled and
  stroked by default; a picture frame is neither.

A stated boolean is honoured in both directions, which the corpus needs: **216** worksheet shapes
state property 511 as `0x00080000` — `fLine` hard and *false* — and **52** state 447 as
`0x00100010`.

## 3. Three object kinds must not take the ink, and that is measured

Calc replaces the DFF-built `SdrObject` for a chart, a TBX form control and an OLE object — the
three that `SetCustomDffObj(true)` marks (`sc/source/filter/excel/xiescher.cxx`:1666, 2066, 2954)
— and the replacement is not the object `ApplyAttributes` filled, so the fill and the line go with
it. `XlsDrawingCollector.KeepsEscherInk` is that test.

**The OLE half is the shape's own `pictureId` (267), not its object type.** A plain BIFF8 picture
and an embedded object are both `ftCmo` type 8 and `SvxMSDffManager::ImportGraphic` separates them
on exactly that property (`msdffimp.cxx`:4025-4030): with it the shape becomes an `SdrOle2Obj` and
loses its attributes, without it an `SdrGrafObj` keeps them.

Established at the reference rather than derived, with `soffice --convert-to fods` — an
eight-second instrument that renders nothing and prints the reference's own answer for every
shape (`expected-ink.py`, `expected-ink.tsv`):

| document | shape | Escher states | 26.2.4.2 draws |
|---|---|---|---|
| `TICAPCapability_Final` | `Text 229`, `Text 230` (text box) | fill 65, line 64 | `#ffffff`, `#000000`, 0.0102 in |
| `TICAPCapability_Final` | `Picture 228` (OLE, `pictureId`) | fill 65, line 64, both hard true | **fill none, stroke none** |
| `TICAPCapability_Final` | `OptionButton1-3` (control) | line 64 | **fill none, stroke none** |
| `014_Contextures…` | `Chart 1` (chart) | fill 78, `fFilled` hard true | **fill none, stroke none** |
| `014_Contextures…` | `Rectangle 6` | `fFilled` false, `fLine` true, no colour | fill none, `#000000`, 0.0102 in |
| `PC1000` | `Rectangle 16` | fill 43 (a `PALETTE` entry) | `#ffff99` |
| `PC1000` | `Text 1/4/5/24` | fill 9 | `#ffffff` |
| `Template Pilot Logbook` | comment captions | fill 80 | `#ffffc0` |

`#ffffc0` is `svtools`' `CALCNOTESBACKGROUND` default, and it is read out of the reference rather
than guessed. **Index 67, the button face, is deliberately absent from `XlsCellFormats.SchemeColour`**:
it is the desktop theme's colour (`0xEFEFEF` in headless `svp`, `vcl/headless/svpframe.cxx`:464),
and all eleven corpus shapes that name it are form controls, which take no Escher ink at all.

## 4. What moved

TICAP page 3, ours against the banked 26.2.4.2 reference:

| | diff % | \|ink\| % |
|---|---:|---:|
| page 2 before | 28.30 | 6.52 |
| page 2 after | 6.51 | **0.32** |
| page 3 before | 31.14 | 7.28 |
| page 3 after | 4.98 | **0.27** |

The panel lands at `(85.9, 61.1, 515.6, 433.5)` against the reference's
`(85.9, 61.0, 515.8, 433.8)` and is stroked at 0.428 pt against 0.42 — no free parameter anywhere
in it; the width is the format's own 9525 EMU default through the sheet's print scale.

**Z-order is right and checked rather than assumed.** On page 3 the white fill is drawing index
**901 of 902**, after all 901 grey cell fills and before the shape's own text: a 100 dpi raster
reads `(255, 255, 255)` inside the panel, `(191, 191, 191)` outside it, and dark pixels on the
text rows inside it. That is Calc's print order — the drawing layer after the cell backgrounds and
the strings (`sc/source/ui/view/printfun.cxx`:1651-1703).

`ticap-page3-triptych.png` is before / after / reference at 110 dpi.

**Six renderings move on the ink fix alone, and three of them get worse.** Summed unsigned ink
against the banked reference, at the round's base and with the ink fix:

| document | base | ink fix | with §5's clip |
|---|---:|---:|---:|
| `TICAPCapability_Final.xls` | 20.66 | **7.45** | 7.45 |
| `EHEST-Pre-departure-checklist.xls` | 14.16 | 14.95 | **8.89** |
| `PC1000.xls` | 2.77 | 5.11 | **1.79** |
| `SIL_TDB609.xls` | 2.73 | **1.07** | 1.36 |
| `SIL_TDB605.xls` | 1.99 | **1.00** | 1.18 |
| `apron-area.xls` | 1.41 | 1.53 | **1.23** |
| sum | 43.72 | 31.11 | **21.90** |

The middle column is what the brief asked for and the third is what the round landed; §5 is why.
**No gate verdict moves on the ink fix**: it adds no glyph and no page, as expected.

## 5. And the clip the fix uncovered, which is worth more than the fix

**Three of the six movers got *worse* on ink and all three were the same thing: we paint a
drawing that overhangs the printed block where the reference clips it away.** It could not show
before, because a shape with no fill and no outline has nothing to overhang with.
`PC1000.xls`' `Rectangle 16` is the witness — 244 pt wide, starting 6 pt inside the sheet's last
column, so 62 pt of it hang past the block's right edge, and on pages 2 onwards all but a 4 pt
sliver hangs off the top. 26.2.4.2 writes the whole rectangle into the content stream and then
clips it away; its page 2 opens the figure with

    q 55.389 552.019 681.846 23.981 re
    W* n
    1 1 0.6 rg ... f*

and that rectangle is the page's own cell block. **A reader counting the PDF's path operators
would call that a shape the reference draws. It is not showing it.**

The rectangle is one this tree already computes: `SheetPageGraphics.ReachesTheBlock` has used it
to *cull* a drawing since the round that closed `Part_375_Operators.xlsx`, with the same citation
— `ScOutputData::PrePrintDrawingLayer` builds it and hands it to `SdrView::BeginDrawLayers` as the
paint **region** (`sc/source/ui/view/output3.cxx`:41-102), and `ScPrintFunc::PrintArea` calls the
pair once per printed area (`printfun.cxx`:1641, 1651-1713). It is a clip as well as a cull, and
only the cull half was implemented. `SheetPageGraphics.Block` is now both.

### What it moves

| | sheets renderings |
|---|---:|
| bytes changed | 164 of 307 |
| **pixels changed** | **81** |
| bytes changed, pixels identical | 83 |

**Count the pixels, not the bytes**: a clip emitted round the drawing pass changes the content
stream of every page carrying a drawing, and on half of them nothing lands differently.
`pixel-diff.py` is the instrument.

Of the 81 that do move (`ink-base.tsv`, `ink-after.tsv`, joined in `ink-movers.tsv`):

| | |
|---|---|
| improve on summed \|ink\|% | **68** |
| worsen | **3** (by 0.07, 0.12 and 0.48) |
| level | 10 |
| summed \|ink\|% over the 81 | **331.20 → 238.54** |
| MAJOR pages | **148 → 85** |

The largest movers are `TDA_Smoke-Detectors.xlsx` 12.67 → 2.03,
`microsoft_learn_multi_chart_examples.xlsx` 8.98 → 0.34, `044_Cash_flow_forecast` 8.39 → 1.84,
`SIL_TDB648.xlsx` 9.51 → 3.44 and `EHEST-Pre-departure-checklist` 14.16 → 8.89.

### And the gate *can* see this half

**25 sheets verdicts are gained, every one of them `glyphs` → `match`.** Clipped text is dropped
by PyMuPDF and by `pdftotext` alike, and the reference's own counts already have it dropped, so
the characters we lose are characters we should never have been drawing. Scored with the gate's
own rule — `max(2 %, 15)` on alphanumeric characters — against the banked 26.2.4.2 reference, over
all 307 (`gate-sheets.tsv`):

| | match | glyphs | pages |
|---|---:|---:|---:|
| base | 262 | 40 | 3 |
| after | **287** | **15** | 3 |

Four of the 25 land on the reference's count **exactly**: `SSRO_Quarterly_Statistical_Bulletin`
2783 → 2532 against 2532, `044_Cash_flow_forecast` 2313 → 2200 against 2200,
`064_Small_business_cash_flow` 1803 → 1635 against 1635, and `Foreign_SA-CAT-I_and_CAT-II-III`
7842 → 7558 against 7557. **That is the strongest evidence in this round that the rule is the
reference's own** — the scorer knew nothing about the clip and the numbers meet on the nose.

*The instrument is this round's own scorer, not `batch-check.sh`, and the reference half is the
r83 bank; the base leg is this worktree's own build at `ddadc94c1`, so the two legs are
like-for-like.*

## 6. Confinement

Rendered our half of the **whole 947-document corpus** twice, at the round's base and with the ink
fix, under `SOURCE_DATE_EPOCH`, one output directory per document (`sweep-ours.py`, `movers.txt`):

- **6 renderings differ, all `.xls`, all on the sheets track**; the other **941 are
  byte-identical**, the words and slides tracks included.

The clip was then measured over the sheets track alone, which is complete for it: every
spreadsheet in the corpus is under `sheets/` (`MANIFEST.tsv`'s families are words 338 = docx +
doc, slides 302 = pptx + ppt, sheets 307 = xlsx + xls + xlsm), and `SheetPageGraphics` is reached
only from `SpreadsheetPages`.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed**, the ten being the four
`PageDrawingComparisonTests.EveryLineIsDrawn`, the four
`TabStopComparisonTests.AListLabelsTabAdvance`, `SheetDrawingComparisonTests.APictureIsDrawn` and
`JustificationShrinkComparisonTests` — unchanged. `APictureIsDrawn` runs on `sheet-rich-text.fods`
and `.xlsx`, cannot reach the BIFF path, and fails on the same anchor clamp as before. The ten
non-fidelity projects total **6366 passed, 0 failed**, after one test's assertion was corrected
rather than worked around: `SheetPictureCropTests.AnUncroppedPictureIsNotClipped` asserted that no
page emits a clip it does not need, and is now `AnUncroppedPictureTakesOnlyTheDrawingLayersClip`
with the measurement above in its remarks.

## 7. What was refuted, and what is left

**Refuted — `EscherInk`'s own caution.** *"Reading the defaults would put a white box under the
text of every shape that mentions neither, which is a confident answer about documents this
project has not measured."* The fear is right and *presence of a colour* is the wrong guard
against it: what stops the white box is the shape **type's** own fill and stroke defaults plus the
hard-attribute test, which is what the reference uses. A picture frame stating nothing is exactly
the case the remark feared, and `mso_DefaultFillingTable` marks it unfilled.

**Refuted — the seating probe's page-1 stroke count.** *"Pages 1-3 are the `Instructions` sheet
and we draw no stroked path at all there"* is true, and the reference's counterpart is not a
shape: its one page-1 stroke is a zero-height rectangle at `(143.8, 256.6, 211.2, 256.6)` 0.304 pt
wide, which is a **text underline**, and four of page 2's five and two of page 3's three are the
same. The shape accounts for one stroke per page and no more.

**Refuted — that the other two readers share the defect.** It is BIFF-only. 26.2.4.2's own `.ods`
of `TICAPCapability_Final` states `draw:fill="solid" draw:fill-color="#ffffff"` and
`draw:stroke="solid" svg:stroke-color="#000000"` outright, and `OdsShapeInk` has read all five
attributes since round 84: this tree draws that twin's panel at `(85.9, 61.1, 515.8, 433.8)`
against the reference's `(85.9, 61.0, 515.8, 433.8)`, stroked at 0.419 pt against 0.42 — while the
`.xls` of the same workbook drew nothing. The DrawingML side states a theme colour rather than a
palette index and was closed in round 84.

**Left, with its seat: a BIFF text box's `TXO` formatting runs.** `XlsDrawingCollector.ReadText`
takes the string and stops; `TextOf` builds one run per line at a hardcoded ten point in the
default face. The runs are eight bytes each in the `TXO`'s second `CONTINUE` — a character offset
and a `FONT` index (`XclImpDrawing::ReadTxo`, `sc/source/filter/excel/xiescher.cxx`:4242) — and
`XlsCellFormats.FontAt` already turns that index into a face, a size and a weight. Reach
(`census-txo.py`, `txo-census.txt`): **155 text boxes with text in 17 `.xls`, 522 runs, of which
62 boxes in 13 documents state more than the opening run.** On TICAP page 3 the reference draws
its shape text at 6.30 pt with five `LiberationSerif-Bold` spans and we draw all of it at 5.70 pt
regular; that is most of the 0.27 % residual there, and the two underlines the reference strokes
inside the panel are the rest.

**Left: the system-colour branch of `MSO_CLR_ToColor`.** Twenty desktop-theme colours and six
recursive "use this shape's other colour" forms, whose answer depends on the machine rather than
on the file. **Zero of the corpus's 106 worksheet-shape colours state one**, so it falls back with
everything else rather than inventing a theme.

**Left: palette index 67, the button face.** Eleven shapes in two documents name it and every one
is a form control, which takes no Escher ink at all. Headless `svp` answers `0xEFEFEF`
(`vcl/headless/svpframe.cxx`:464) and a desktop would answer something else.

## 8. The passing set, re-ranked on ink

`ink-ranking.tsv` scores every one of the 307 sheets documents against the banked 26.2.4.2
reference after both changes and joins the gate verdict to it (305 scoreable; two have no usable
reference in the bank). **287 pass and 41 of those are at 5 % or worse on summed unsigned ink** —
which is the whole argument for ranking this way, since not one of the 41 is visible to the gate.

The top of the passing set:

| rank | document | sum \|ink\|% | pages | MAJOR |
|---:|---|---:|---:|---:|
| 1 | `TK-Syllabus-Comparison-Document-v2.xlsx` | 304.57 | 1235 | 142 |
| 2 | `alle einzeln.xlsx` | 225.44 | 186 | 36 |
| 3 | `Background_Declaration_Template.xls` | 136.07 | 25 | 15 |
| 4 | `grants-2005.xls` | 96.11 | 201 | 45 |
| 5 | `atspp_pay_tables.xlsx` | 69.62 | 101 | 15 |
| 6 | `CSJU List of Recipients of funds 2013-2020.xlsx` | 64.89 | 97 | 9 |
| 7 | `orbus_togaf_tool_csq.xls` | 51.20 | 75 | 24 |
| 8 | `tk-syllabus-comparison-document-v5.xlsx` | 46.55 | 855 | 37 |
| 9 | `6880ac7361ca1b99a9230811_ST Capability List Rev.16 - Web.xlsx` | 27.04 | 217 | 0 |
| 10 | `fm-provider-service-measures.xlsx` | 21.67 | 38 | 5 |

**`TICAPCapability_Final.xls` has gone from the top of a 48-document sample to 32nd of 287**, and
`EHEST-Pre-departure-checklist` sits just above it at 26th. Ranks 1-8 are the next seats: three of
them (`Background_Declaration_Template`, `orbus_togaf_tool_csq`, `TOGAF9-Tool-ConfReqts-CSQ`) are
`.xls` carrying worksheet shapes and are the obvious place to point the `TXO` run reader §7 leaves
open, and `6880ac7361ca…` is the one in the top ten with **no MAJOR page at all**, so its 27 % is
spread thin rather than concentrated — a different kind of defect and worth a look on its own.

*Beware the denominator: this is 305 of the 307 sheets documents scored against the r83 reference
bank, not the whole corpus, and `alle einzeln.xlsx` and `TK-Syllabus-Comparison-Document-v2.xlsx`
are 186 and 1235 pages, so a summed percentage over pages is not comparable between two documents
of very different length. Read it beside the MAJOR column.*
