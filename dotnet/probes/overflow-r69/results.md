# Two "text and the box it should stay in" defects, and what the box turned out to be

**Round 69, measured 2026-09-06 at `ddb05e4e5`.** Environment, stated once because a stored
figure is evidence about an environment and not about the code:

| | |
|---|---|
| ours | `XRay.Office.Cli` from `/home/user/wt-overflow`, `XRAY_OFFICE_BUNDLED_FONTS` unset |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, the calibration target |
| ref24 | `/usr/bin/soffice` — **24.2.7.2**, which is what `batch-check.sh` scores against |
| fonts | system fontconfig; the tarball's eight Latin duplicates and its Noto moved aside |
| corpus | `/home/user/sample-files`, 947 documents; gate at `2f4709c08` banked at `/home/user/gate-2f47/` |

**Neither briefed document names a Narrow family**, so the `LiberationSansNarrow` confound
described in `dotnet/CLAUDE.md` does not touch either figure: `048` names Calibri and Century
Gothic, `070` Arial and Century Gothic, and the two sides embed **identical face sets** —
DejaVu Sans, DejaVu Sans-Bold, Carlito, Carlito-Bold on `048`; DejaVu Sans, Liberation Sans,
Liberation Sans-Bold, Liberation Serif on `070`.

---

## 1. `048_Expense_trends_budget` — the block, not the cell, and it is centred off the page

### What the reading got right, and in what way it was wrong

The blind reviewer saw the reference losing text at **both** ends and ours only at the right, and
named centring. That direction is right and the *kind* is not: nothing about a **cell's**
horizontal alignment is involved. The measurement that settles it is that the offset is a
**constant**.

Differencing our page 1 against 26.2.4.2's, word by word, on the words the two share:

| line | shift | spread within the line | words matched |
|---|---:|---:|---:|
| all nineteen | **−167.06 to −167.42 pt** | ≤ 0.74 | 1 to 15 |

One number for the whole page. A per-cell alignment cannot do that; a page origin can. And the
sheet says so outright — `xl/worksheets/sheet17.xml`, the `tips` sheet, carries
`<printOptions horizontalCentered="1"/>` and one column of **152.33 characters**.

### What 26.2 does and why

`ScPrintFunc::PrintPage` (`sc/source/ui/view/printfun.cxx`:2149-2170) sums this page's column
widths into `nDataWidth`, adds `PRINT_HEADER_WIDTH` when the row and column headings print, and
then writes

```cpp
nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2;        // LTR or RTL
```

with **no clamp**, and `nTopSpace` the same way at `:2188`. When the columns do not fit, the
offset is negative and the block is drawn hanging off both edges of the paper. That is the whole
mechanism, and the case is reachable at all only because **a single column cannot be split**: a
block of several columns wider than the page paginates into a second page column instead.

### The seat

`SpreadsheetPages.BodyOrigin`
(`dotnet/src/XRay.Office.Spreadsheets/Layout/SpreadsheetPages.cs`:506-517). It read

```csharp
Length spare = area.Width - Extent(Columns(x).Select(c => c.Width));
if (spare > Length.Zero) x += spare / 2;
```

The guard was in the commit that first implemented the flag (`68e89c397`) and carries no
measurement. Removing it, and folding the heading strip into the extent the way `printfun` folds
`PRINT_HEADER_WIDTH` into `nDataWidth`, is the fix.

The heading half is transcribed rather than measured, and the write-up says so: **no corpus
sheet both prints headings and centres** — 0 of 243 xlsx-family workbooks — so the corpus has no
witness for it either way. What it corrects is a body placed half a heading strip too far in,
because the strip was added to the origin and left out of the extent.

### Before and after

| | pages | alphanumerics | vs ref26 | verdict vs ref26 |
|---|---:|---:|---:|---|
| ours before | 14 | 4088 | +102 | `words` |
| **ours after** | 14 | **3999** | **+13** | **match** |
| ref26 | 14 | 3986 | | |
| ref24 | 14 | 3986 | | |

After the fix the per-line shift against 26.2.4.2 is **−0.06 to −0.40 pt** on all nineteen lines,
worst single word 0.80 — the reconstructed-position floor `dotnet/CLAUDE.md` describes, not a
residual offset. The remaining +13 is inside the gate's own floor of 15.

### Reach

**73 of the corpus's 243 xlsx-family workbooks** state `horizontalCentered` on at least one
sheet (170 sheets), and 9 state `verticalCentered` (11 sheets). The clamp only bit the subset
whose block also overflows, which is why the flag looked implemented. `census.py centring`.

---

## 2. `070_Equipment_inventory_list` — three symptoms, **two** causes, and one of them is not this round's

All three of the reviewer's observations live in one place: `xl/drawings/drawing11.xml`, in the
`mc:Fallback` Excel writes beside each of three slicers.

```xml
<a:solidFill><a:prstClr val="white"/></a:solidFill>
<a:ln w="1"><a:solidFill><a:prstClr val="green"/></a:solidFill></a:ln>
...
<a:bodyPr vertOverflow="clip" horzOverflow="clip"/>
<a:t>This shape represents a table slicer. …of Excel.

If the shape was modified…</a:t>
```

### The paragraph break and the clip are ONE cause

The `a:t` holds two bare `U+000A`. LibreOffice never sees them as characters: every importer
hands its string to the EditEngine — `TextRun::insertAt` calls `xText->insertString`
(`oox/source/drawingml/textrun.cxx`:124) — and `ImpEditEngine::ImpInsertText`
(`editeng/source/editeng/impedit2.cxx`:2864-2983) normalises the line ends with
`convertLineEnd(rStr, LINEEND_LF)`, walks from separator to separator, and calls
`ImpInsertParaBreak` at each one that is not the end of the string. Its own comment names the
two-in-a-row case: `// Start == End => empty line`.

We shaped the separator instead, so it came out zero-width and joined the sentences into
`Excel.If`. The consequence is not cosmetic: **the body was one line slot shorter**, and the
boxes hold exactly five slots, so the `vertOverflow="clip"` machinery that landed at `49428378d`
never fired. Reading this as "a lost paragraph" *and* "text not clipped" counts one cause twice.

Measured on the three shapes, ours after the fix against 26.2.4.2, PDF text origins:

| shape | reference slots | ours before | ours after |
|---|---|---|---|
| 1 (81.4 pt wide) | 2 + blank + 2, 6th clipped | 5 lines, nothing clipped | 2 + blank + 2, 6th clipped |
| 2 (62.9 pt wide) | 3 + blank + 1, 6th–7th clipped | 5 lines, nothing clipped | 3 + blank + 1, clipped |
| 3 (49.6 pt wide) | 3 + blank + 1 | 5 lines | 4 lines, one wrap earlier |

### `horzOverflow` is not the horizontal sibling of that clip, and nothing reads it

The brief's first candidate was the horizontal spelling of the round-68 clip. **It does not
exist.** `TextBodyPropertiesContext` stores the attribute as a bare string
(`oox/source/drawingml/textbodypropertiescontext.cxx`:83), which is put into a grab bag for
round-tripping (`oox/source/drawingml/shape.cxx`:2189) and re-exported
(`oox/source/export/drawingml.cxx`:4141 and 4379). Those are the only three uses in the tree and
all three are writes. Only `vertOverflow` sets a property —
`PROP_TextClipVerticalOverflow`, `:85-97`. The corroborating measurement is that our wrap
already agrees with the reference: across the twelve drawn lines of the three boxes our line
widths match 26.2.4.2's to **0.03–0.07 pt**, so no horizontal clip is being missed.

And the corpus cannot witness one either way (`census.py overflow`): **516 worksheet bodies in
34 documents state `horzOverflow`, and every single one of them also states `vertOverflow`.**
There is no document where reading the horizontal attribute could change an outcome the
vertical one does not already decide.

### The second cause: the shape line height carried the external leading

Making the break visible exposed a 3.8% error in the baseline pitch, and it is a second,
independent defect. `SheetBandText.ShapeLineHeightAt` was `ascent + descent + lineGap` — carried
under its own name since round 60 precisely because that half had **never been measured**, with
a code comment saying so.

`gen-shapeline.py` builds a workbook of sixteen wrapping text boxes — four faces × four sizes,
nothing else on the sheet, no print scale — so a baseline pitch is read straight off 26.2.4.2's
own text origins. `score-shapeline.py` scores three laws against the 19 boxes that wrap to three
lines or more (`shapeline.txt`):

| law | mean \|error\| | worst |
|---|---:|---:|
| `ascent + descent + lineGap` (what we had) | **0.2369 pt** | 1.02 pt at 24 pt |
| metrics through Calc's 720 dpi grid | 0.0345 pt | 0.096 pt |
| **`ascent + descent`, no device** | **0.0081 pt** | 0.020 pt |

So the leading is the missing half and the **device is not**: a drawing object's text is
formatted against the model's own reference device, which is `RefDevMode::MSO1` at 8640 dpi
(`sc/source/core/data/documen8.cxx`:182-193) and is indistinguishable from no grid at these
sizes. `IsAddExtLeading()` is false in EditEngine, which every other EditEngine user in
`MetricGrid` has said all along.

**Why it survived nine rounds: two of the four faces have no line gap.** Carlito's is zero and
DejaVu Sans' is zero, so the two laws agree exactly on them; Liberation Sans' is 67/2048 and
Liberation Serif's 87/2048. `070`'s notices fall back to Liberation Serif, and the Carlito
workbooks the shape path was built against could not have shown it.

Corroborated independently by the committed fixture: on `sheet-shape-line-separator.xlsx`
26.2.4.2 stacks 12 pt Liberation Sans at **13.41 pt** and ours now does too, where the
leading-bearing law gives 13.80.

### Before and after

| | pages | alphanumerics | vs ref26 |
|---|---:|---:|---:|
| ours before | 1 | 1128 | +100 |
| **ours after** | 1 | **1004** | **−24** |
| ref26 | 1 | 1028 | |
| ref24 | 1 | 943 | |

**The gate verdict cannot move on this document and that is not the fix's fault.** Its gate
reference is 24.2.7.2, which draws 943 — 85 fewer than 26.2.4.2 — because 24.2 draws Excel's own
*"This chart isn't available in your version of Excel"* fallback and 26.2 does not.
`probes/mismatch-classify-01/` files it as `refs-disagree`, *read, do not score*.

### What is left, with the arithmetic

The residual −24 is **one wrap decision in the narrowest of the three boxes**. The reference fits
`supported in this version of Excel.` on one line of a 45.33 pt column and we break it, which
costs that box its last drawn line. Three things are measured about it:

- **Our box is not narrower.** The three shapes' text origins sit +0.05, +0.07 and +0.06 pt right
  of the reference's — a constant, so both edges move together and `available` is the same.
- **Our advances are not wider.** Our drawn ink extents run +0.03 to +0.07 pt over the
  reference's on all twelve lines, which is +0.08% and is exactly the artefact
  `dotnet/CLAUDE.md` documents: the reference's PDF declares `floor(hmtx × 1000 / upem)`, so a
  59-glyph line reconstructed from those widths comes out ~0.5/1000 em per glyph — 0.096 pt —
  narrow.
- **The slack is smaller than either.** The reference's own line leaves 0.09 pt of a 45.33 pt
  column.

The named hypothesis for whoever wants it: a drawing object's character height is stored in
**1/100 mm**, so 11 pt is 388 and comes back as 10.99843 pt — 0.014% narrow, or 0.006 pt on this
line, which is the order of the slack. Rendering the probe workbook shows LibreOffice drawing 8,
11, 16 and 24 pt as 7.99, 11.00, 15.99 and 24.01. It is not implemented here: one witness, a
0.02% change to every sheet shape's type, and no measurement saying which way the other 614
bodies would move.

### The missing box is a feature, not a bug, and it is mostly somebody else's

The reference paints a **white fill and a green hairline outline** on each of the three shapes —
`fill (344.53, 519.72)-(425.89, 540.96) #FFFFFF` then `stroke … #008000`, and twice more — and we
paint neither. That is not a defect in the clip or in the text: **a worksheet `xdr:sp`'s fill and
outline are not read at all.** `SheetDrawing.Fill` and `.Stroke` exist and are set from exactly
one place, `XlsxNoteCaptions`, for shown cell comments.

`census.py shapebox`, whole corpus:

| | shapes | documents |
|---|---:|---:|
| worksheet `xdr:sp` | **644** | 49 |
| carrying `xdr:style` (a theme `fillRef`/`lnRef`) | 583 | 36 |
| declaring `a:ln` | 454 | 46 |
| declaring `a:solidFill` | 421 | 32 |
| declaring `a:ln/a:solidFill` | 205 | 21 |

and the colour references are `sysClr` 246, `schemeClr` **332**, `prstClr` 38, `srgbClr` 10 —
over half of them theme lookups, before the 583 `xdr:style` references which are nothing but
theme lookups. The presets are 28 distinct shapes including `star5`, `heart`, `cloud`, `sun`,
`lightningBolt` and `irregularSeal1`, so the rectangle `SheetPageGraphics` strokes for a note
caption is the wrong outline for 438 of the 644.

This round did not do it, deliberately, and the line it declines to cross is stated in the
brief: **`wt-numfmt` owns theme colours.** It is a round of its own — colour resolution through
`DrawingColour`/`DrawingStyleMatrix`, and `CustomShapeGeometry.FillOutline`/`StrokeOutline`
instead of `GraphicsPath.Rectangle` — and half of it would be worse than none, because a
rectangle drawn around a `star5` is a defect the corpus does not currently have. The gate cannot
see any of it: a fill and an outline add no glyphs and no pages.

**Also not this round's, and separate again:** `xray-office extract` surfaces no worksheet shape
text at all. `extract` on `070` finds none of the three notices, which the rendering path draws.
That is the round-67 lesson — making something draw does not make it extract — arriving on the
sheets track.

---

## The sheets track, re-scored — and why the bank is not the baseline

The sweep re-rendered **our half only** and scored it against the banked gate's own reference
PDFs (`sweep-ours.sh`, 307 documents in 14 minutes). Two controls before any number below:

- **307 of 307** banked sheets rows found a swept row, and none was extra.
- The reference half, re-derived from the banked PDFs rather than taken from the stored
  numbers, is **equal on 307 of 307** in both pages and glyphs. So nothing below is the
  reading having changed.

**The bank is at `2f4709c08` and this round is based on `ddb05e4e5`, two commits later.**
Comparing the sweep to the bank shows five verdict movements and eleven changed glyph counts —
and most of them are those two commits, not this round. `base-movers.tsv` renders all eleven at
`ddb05e4e5` itself to separate them:

| document | bank | base `ddb05e4e5` | after | ref | whose |
|---|---:|---:|---:|---:|---|
| `048_Expense_trends_budget` | 4088 | 4088 | **3999** | 3986 | **this round** |
| `070_Equipment_inventory_list` | 1128 | 1128 | **1004** | 943 | **this round** |
| `037_Personal_money_tracker` | 2522 | 2534 | **2476** | 2505 | **this round** |
| `EHEST-Pre-departure-checklist.xls` | 39462 | 39462 | **39506** | 39768 | **this round** |
| `062_Run_chart` | 680 | 643 | 643 | 645 | chart-sheet fix |
| `057_Simple_balance_sheet` | 2207 | 2185 | 2185 | 1877 | chart-sheet fix |
| `068_Blue_inventory_list` | 1141 | 1199 | 1199 | 1196 | round 68's clip |
| `076_Inventory_list_accessibility_guide` | 4949 | 4793 | 4793 | 4782 | round 68's clip |
| `056_Quarterly_sale_report` | 636 | 642 | 642 | 629 | earlier |
| `030_Basic_balance_sheet` | 1723 | 1727 | 1727 | 1644 | earlier |
| `18-02RD301_ILS_components.xls` | 155230 | 152726 | 152726 | 155230 | earlier |

So **this round moves 4 of the sheets track's 307 documents and no page count anywhere.**

| verdict | at `ddb05e4e5` | after |
|---|---:|---:|
| match | **260** | **261** |
| words | 39 | 38 |
| pages | 2 | 2 |
| pages,words | 6 | 6 |

**One verdict moved and it moved forward:** `048_Expense_trends_budget` `words` → `match`. None
moved backwards. 260 is the figure the brief carries for the base commit, which the sweep
reproduces exactly.

**The two documents the gate cannot score, screened against 26.2.4.2 by hand** — both are
`match` before and after under the gate's own 24.2 reference, so the gate is blind to which way
they went:

| document | before | after | 26.2.4.2 | moved |
|---|---:|---:|---:|---|
| `037_Personal_money_tracker` | 2534 (+37) | 2476 (**−21**) | 2497 | **closer** |
| `EHEST-Pre-departure-checklist.xls` | 39462 (−282) | 39506 (**−238**) | 39744 | **closer** |

`037` is the separator fix — its residual against 26.2 is a chart value axis' label padding
(`100%` against ` 100%`), which is the secondary-axis track's and not this one's. `EHEST` is a
BIFF workbook with no `a:t` at all, so its +44 is the shape line height alone, and it moves
toward the target.

## Verification

| | baseline at `ddb05e4e5` | after |
|---|---|---|
| `dotnet build XRay.Office.slnx -v q -nologo` | 0 warnings, 0 errors | **0 warnings, 0 errors** |
| ten non-fidelity projects, run individually, totalled by hand | 5845 / 0 failed / 0 skipped | **5850 / 0 / 0** (+5 new tests) |
| `XRay.Office.Fidelity.Tests` | 542 / 10 failed / 0 skipped of 552 | **542 / 10 / 0** |
| sheets track, gate rule | match **260** of 307 | match **261** of 307 |

The ten projects were run twice — once before and once after the source was restored from the
base-commit experiment — and both runs total **5850** with the same per-project counts, which is
the control on the `cp`-then-`touch` trap `dotnet/CLAUDE.md` records. The fidelity failures are
the same ten names as the baseline: `PageDrawing` ×4, `TabStop` ×4, `SheetDrawing`,
`JustificationShrink`. None of them is on this round's surface and none was re-run in isolation,
because none of them moved.

## Reach of what did change

| change | surface |
|---|---|
| print centring, both axes | **73 of 243** xlsx-family workbooks state a centring flag; 170 sheets horizontal, 11 vertical |
| line separator in an `a:t` | **19 of 615** worksheet shape text bodies, in **9 of 947** documents, 38 separators |
| shape line height | **all 615** shape text bodies; visible only on the ones whose face has a line gap |

The separator census over the other two tracks: **one** `a:t` in the whole slides corpus carries
a separator (`Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx`) and **none** in words.
Whether `XRay.Office.Presentations` handles it was not measured; the sheets seat fixed here is
`SheetShapePainter` and does not reach either.
