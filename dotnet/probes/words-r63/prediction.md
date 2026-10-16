# words-r63 — prediction, committed before any post-change rendering

Base `43142b73ccf`, branch `wt-words-r63`, worktree `wt-words-r50`. Baseline reproduced before
this file was written: `batch-check.sh … 'words/*' … 8` gives
`TOTAL 355 MATCH 340 MISMATCH 15 REF-CANNOT-RENDER 0`, scored against `MANIFEST.tsv`'s own
337-path list as **323 of 337, zero disagreements with the manifest's status column**.

Two changes are planned and they are predicted separately, because they reach different documents
and the second is the one that can go wrong quietly.

---

## Change 1 — a `w:tblStylePr` layer's `w:tcPr/w:shd`, and the horizontal and vertical bands

Round 62 §3 read the rule out of `012` and named the seat. `WordTableStyleConditions.Names`
resolves the layer names but omits the four band layers, and `WordStyle` keeps only the `w:rPr`
half of each layer, so a conditional cell shade is read by nothing.

### The reach, resolved rather than declared

`tblstyle-reach.py` walks every table of every body part of the manifest's 271 `.docx`, resolves
`w:tblStyle` → `w:tblLook` → the cell's layers most specific first → the `w:basedOn` chain, and
counts only cells that state no `w:shd` of their own:

```
cells that would gain a fill from a conditional w:tcPr layer
  documents : 42
  cells     : 733
by layer  : band1Horz 573, firstRow 123, firstCol 19, band2Horz 16, nwCell 2
by fill   : D3DFEE 330, F2F2F2 200, 4F81BD 99, FFFFFF 26, DEEAF6 24, DBDBDB 18, …
```

The declaration-level census is **733 against 34 977** — thirty-three documents carry Word's whole
built-in style set as latent styles and name three of them — which is `COMMON.md`'s "estimate reach
from what a shape resolves to" in its plainest form, so the resolved figure is the one predicted
against.

### Prediction

| quantity | predicted |
|---|---|
| renderings whose bytes change | **38 to 46** |
| verdicts **gained** | **0** |
| verdicts **lost** | **0** |
| page counts changed | 0 |
| extractable word counts changed | 0 |
| font lists changed | 0 |
| `012` page 1 fills, ours → | **19 → 75**, matching the reference operator for operator: `#F2F2F2` 0 → 48, `#FFFFFF` 0 → 8, `#000000` 12 unchanged |
| `012` page 1 strokes, ours | **2, unchanged** — the `firstRow` `w:tcBorders` half is not in this change |

**Falsification.** A cell shade changes no metric the gate reads. So *if any page count, word count
or font list moves anywhere in the 337, this change did something other than paint a cell* and the
result is to be reported as a miss whatever the verdict total says.

### The one route by which it could move a line, and it is measured to be closed

Adding the bands to `Names` feeds them to `TableStyleRunProperties` as well, and a band layer
carrying a `w:rPr` **would** change how text is measured. Two documents declare such layers
(`te.iors.00048-002 SUP Questionnaire.docx`, `EHEST-SMS-Safety-Management-Manual-V2.docx`) and in
both the styles are latent: **no table in the corpus names a style whose `w:basedOn` chain reaches
a band layer carrying a `w:rPr` — 0 of 271**, checked through the full chain and not only the named
style. That is the blind spot round 62's band remark was left open for, and it is now closed by
measurement rather than by argument.

### What this census cannot see

* **`w:cnfStyle`.** A row or cell may name its own conditional regions directly instead of having
  them inferred from position. Neither the census nor the implementation reads it, so a document
  using it under-reads in both and the two agree while both being wrong.
* **A table inside a `w:txbxContent`.** The walk visits it; the layout may not lay it out at all,
  so those cells over-read.
* **`w:gridSpan` and vertical bands.** The band index is counted on the *grid* column, and a row
  whose cells span unevenly puts a cell in a different band from the one Word would. Only 6
  `band2Vert` and 3 038 `band1Vert` declarations exist and none resolves onto a corpus cell, so
  this is expected to be inert — but it is the term most likely to be wrong.
* **Which rows the banding excludes.** Implemented as "not counted when the cell is in a
  `firstRow`/`lastRow` region the table asked for", which `012` confirms (its bands land on table
  rows 2, 4, 6, 8, so the header is excluded) and nothing else in the corpus tests.
* **The reference's own answer.** The census says what *we* would paint, not what LibreOffice
  paints. `012` is the only document where the two have been compared operator for operator.
* **The `.doc` and `.rtf` readers** have no `w:tblStylePr` and are outside this entirely.

---

## Change 2 — an automatic font colour over a *semi-transparent* shape fill

Round 59 measured two counter-witnesses to "a text box's own fill decides the automatic font
colour" and round 62 deliberately shipped nothing because of them. `alphaauto.py` and
`threshold.py` settle it: **both witnesses carry a transparency** — `<a:alpha val="52941"/>` and
`<v:fill opacity="26214f"/>` — and `ApplyAutoColor` asks
`SdrAllFillAttributesHelper::getAverageColor(aGlobalRetoucheColor)`, which interpolates the fill
toward white by that transparency before `Color::IsDark` sees it. Eleven arms over three fill
colours land on three different predicted flip transparencies (9.571 %, 37.454 %, 62.222 %).

So the shapes **are** Writer text boxes, the fill **is** consulted, and the missing term is alpha.
This change passes a text box's own fill, blended toward white by its transparency, as the
background an automatic font colour resolves against — and reads `v:fill/@opacity`, which the VML
reader does not currently read at all.

### Prediction

| quantity | predicted |
|---|---|
| renderings whose bytes change | **2 to 9** |
| verdicts **gained** | **0** |
| verdicts **lost** | **0** |
| glyphs we draw white that the reference draws **black** (round 59's LONG column) | **34, unchanged** — this is the control the whole change is gated on |
| `069` page 1 text shows, ours | **87 black / 0 white, unchanged** |
| `docs-quality-MA.IMS.00001-…` text shows, ours | unchanged |
| VML shapes whose *drawn* fill becomes translucent | see below |

**The control is round 59's own.** That round's first cut turned 383 glyphs white that the
reference draws black and *nothing in the gate said a word*, because painting text out of a page
moves no page count, no word count and no font list. So the falsification here is not a verdict at
all: **if the LONG column moves off 34, the change is wrong however the sweep scores.**

### What this census cannot see

* **How many text boxes hold an automatically-coloured run at all.** A run stating `w:color`
  explicitly is unaffected, and the reader resolves that through style chains the census does not
  walk.
* **Whether the anchor half should ship too.** Round 62's rule has a second limb — a `noFill` box
  continues to its *anchor's* background — which is what `012`'s white title needs. It is **not**
  in this change: the anchor is not reachable from `PageDrawing.DrawFrame`, which draws frames from
  a per-page list. `012`'s title stays black and the divergence stays open.
* **The drawn fill.** Reading `v:fill/@opacity` also makes the shape's own rectangle translucent
  where we paint it opaque today. That is a second, visible consequence on the same documents and
  it is deliberately included, because painting a 40 %-opaque fill at full strength is wrong
  independently of what the text colour does — but the census cannot say how many VML shapes in
  the corpus state an opacity, and that number is measured before the change ships.
* **Slides and sheets.** `v:fill` is read by `XRay.Office.WordProcessing/Ooxml/DocxVmlFrames.cs` and
  by `XRay.Office.Spreadsheets/Ooxml/XlsxNoteCaptions.cs`. If the change touches only the former, the
  other two tracks cannot move by construction — a falsifiable claim for the parent's sweep.
