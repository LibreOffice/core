# Round 107 — an Escher pattern fill, a cell hidden one stage too early, and a "row grid" that is a chart

    ours   = XRay.Office.Cli @ d07a9ce75 (base) and @ d07a9ce75 + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 — and the banked
             reference PDFs at /home/user/gate-orig-r83/ref
    corpus = /home/user/sample-files/sheets — 241 `.xlsx`, 2 `.xlsm` and 64 `.xls`, 307 in all;
             60 words, 60 slides and 40 converted `.ods` as the control
    rule   = ink is `|ink|%` from `pdf-image-diff.py`, summed unsigned over the document's pages
    date   = 2026-09-12

## 0. How to read this file, because the round was interrupted

The container running this round was restarted with the work unbuilt and unmeasured, and the
tree was committed as it stood in `227770915 wip(sheetdraw): salvaged from a container restart,
unvalidated`. **Everything below was re-derived after that**, on a clean build, against fresh
instruments; nothing was carried over on the strength of the earlier draft having said it.

Three things came out differently from the pre-restart draft and are marked where they appear:

- every LibreOffice line-number citation in the code was **off by a few lines** and has been
  corrected against `/home/user/libreoffice-core` (§5.3);
- the pre-restart draft's O41 numbers came from a `cmp.py` that was never committed, so they
  could not be checked; they are **replaced** by `gauge-rules.py`/`gauge-rules.tsv`, which are
  (§3);
- the pattern-tile evidence is **stronger** than the draft's stripe-counting: our four tiles and
  26.2.4.2's four are compared pixel for pixel with no rasteriser in between (§1.4).

The pre-restart sweep survived and was re-run from scratch against a freshly published head. The
two are **identical row for row including every md5** (§4.2), so it is that measurement, not a
recollection of it.

## 1. The headline

| seat | what it was | what it is now |
|---|---|---|
| **O40** an Escher pattern, texture or picture fill is painted as its solid foreground colour | *"2 shapes in 1 corpus `.xls`"*, and the whole of round 103's net regression | **fixed.** `apron-area.xls`'s four `mso_fillPattern` shapes are drawn as recoloured 8 × 8 tiles that match 26.2.4.2's own four `draw:fill-image` PNGs **on all 64 pixels of each**, at four alphas matching its four `draw:opacity` values, on a 6.0000 pt tile against the reference's drawn 5.9943. Ink **1.51 → 0.44** and the document's one MAJOR page clears |
| **O39** a `showValue="0"` cell loses its text one stage too early | a row measured as though it were empty | **fixed, and the corpus rendering reach is nil rather than two documents.** `XlsxHiddenValues` is deleted; `SheetDecoration.HidesValue` carries it alone at paint time. The drawn row pitch on `sheet-cf-icon-set-x14-custom.xlsx` goes **13.7764 → 14.8819 pt** against 26.2.4.2's own `style:row-height="0.2071in"` (14.9112). **3** corpus documents state the attribute, not 2, and **all three render byte-identically** either way; what moves on them is extraction, towards what 26.2.4.2's own csv export writes |
| **O41** `EHEST`'s row grid is about 2.5 pt out on some sheets | *"a row-height question rather than a drawing one"* | **the mechanism is refuted and the seat stays open, re-characterised.** On pages 13 and 15 **all 32 of the sheet's own full-width rules agree with 26.2.4.2 to 0.02 pt**. The rules r103 measured are the **embedded gauge chart's value-axis gridlines** — 18 of them, up to 7.2 pt out, and drawn black where the reference draws `#808080` — and the disagreement is that chart's **plot rectangle**: ours 404.75 × 150.78 pt against 413.75 × 160.04, inside an outer frame the two agree on to 0.2 pt |

---

## 2. O40 — the pattern fill

### 2.1 The census, redone with grouped shapes read

`fill-census.py` assembles each substream's drawing the way `XlsDrawingCollector` does —
`MSODRAWING` records and the `CONTINUE`s directly behind one, at that substream's own `BOF`
depth, so an embedded chart's drawing is a stream of its own — walks it as a record tree, and
reports `fillType` (property 384) per `msofbtSpContainer` together with `fFilled`, `fillColor`,
`fillBackColor`, `fillOpacity` and the form of `fillBlip`.

Re-run in full for this write-up; the output is **identical** to the committed
`fill-census.txt`/`fill-census-summary.txt`:

| | |
|---|---:|
| `.xls` read (one is not OLE2 and is skipped) | **63 of 64** |
| shape containers in them | **562** |
| stating `mso_fillPattern`, `mso_fillTexture` or `mso_fillPicture` | **4**, in **1** document |
| of which pattern / texture / picture | **4 / 0 / 0** |

So `EscherInk.Read`'s comment — *"2 shapes in 1 corpus `.xls`"* — is now **4 in 1**, all
`apron-area.xls`'s, all patterns. **None of the four is a grouped shape**, so the count was stale
for a different reason than the seat supposed; what had been hiding two of them is not
established and is not needed. The census does **not** see a shape container that arrived as a
bare `CONTINUE` behind an `OBJ` or a `TXO` (r103 §1.1), so 562 and 4 are floors.

The four rows, verbatim from `fill-census.txt`:

| shape | type | fillColor | fillBackColor | fillOpacity | fillBlip |
|---|---|---|---|---:|---:|
| `apron-area.xls` | pattern | `0x08000037` (palette 55) | *unstated* | 26214 = 0.400 | 6 |
| `apron-area.xls` | pattern | `0x08000037` (palette 55) | *unstated* | 22938 = 0.350 | 4 |
| `apron-area.xls` | pattern | `0x08000017` (palette 23) | *unstated* | 32768 = 0.500 | 6 |
| `apron-area.xls` | pattern | `0x08000017` (palette 23) | *unstated* | 39322 = 0.600 | 4 |

*A correction to r103 that falls out of the same script.* That round's §1.5 reports **489** shape
containers, **299** setting `fAutoTextMargin` and **138** carrying the group flag. Re-running its
own `autotm.py` with the file list quoted gives **562 / 361 / 149**: seven of the 64 paths hold a
space, and an unquoted `$(cat …)` split them into words. The **44** child anchors in **2**
documents reproduce exactly, and no conclusion of that round turns on the other three numbers.

### 2.2 What 26.2.4.2 resolves there, read out of its own view rather than a rendering

`soffice --convert-to fods` on `apron-area.xls` takes eight seconds and renders nothing. Its
content gives exactly four graphic styles `draw:fill="bitmap"` — `gr9`, `gr14`, `gr15`, `gr16`,
each on a `draw:custom` shape — naming four `draw:fill-image` PNGs and carrying
`draw:opacity` of **50 %, 60 %, 40 % and 35 %**, which are the four `fillOpacity` values of §2.1
to the tenth of a percent. Every other filled shape in the document is `solid` or `none`.

The four PNGs are 8 × 8 truecolour and decode to exactly two colours apiece: `#969696` or
`#808080` where the workbook's blip has a **clear** bit, `#ffffff` where it has a set one. Palette
entries 55 and 23 of that workbook's `PALETTE` are `#969696` and `#808080`.

That is `DffPropertyReader::ApplyFillAttributes` exactly
(`filter/source/msfilter/msdffimp.cxx`:1401-1443 **in this tree**, which declares
27.2.0.0.alpha0+ and is not the 26.2.4.2 binary's source — read by hand for this write-up):
guard the recolouring on the bitmap being 8 × 8 (`:1404-1406`), test each pixel against
`Color(0)` and write `fillBackColor` there and `fillColor` everywhere else (`:1432-1435`), both
defaulting to `COL_WHITE` (`:1408-1414`). **Both legs agree** — the source explains it and the
reference's own resolved view states the answer.

### 2.3 How big one tile is, measured on both sides

A pattern states no tile size, so the tile is the bitmap's own — pixels at the device's
resolution, which headless LibreOffice reports as 96 dpi (`SvpSalGraphics::GetResolution`,
`vcl/headless/svpgdi.cxx`:44 in this tree, checked). That is 0.75 pt per pixel and 6 pt per tile.

`tile-pitch.py`, over the two PDFs of page 1 (`tile-pitch.txt`):

| | how it draws the fill | tile |
|---|---|---:|
| ours | 8 × 8 image XObject tiled; the step between placements, on 37 scanlines of 4 shapes | **6.0000 pt** |
| 26.2.4.2 | one rasterised 629 × 209 image under a soft mask, placed 151.910 pt wide; 51 stripes per scanline, two per tile | **5.9943 pt** |

0.0057 pt, or 0.1 %, and it needs no free parameter.

### 2.4 Whether the tile is the right tile — the strongest arm, and it is new

The reference's four resolved tiles are in its `fods` as PNGs and ours are in our PDF as 8 × 8
`DeviceRGB` image XObjects. `tile-check.py` decodes both without a codec and compares them
(`tile-check.txt`):

    obj 3   =  Bitmap_20_1   #969696 #969696 #ffffff #ffffff #ffffff #ffffff #ffffff #969696
    obj 5   =  Bitmap_20_2   #969696 #969696 #ffffff #ffffff #969696 #969696 #ffffff #ffffff
    obj 7   =  Bitmap_20_3   #808080 #808080 #ffffff #ffffff #ffffff #ffffff #ffffff #808080
    obj 9   =  Bitmap_20_4   #808080 #808080 #ffffff #ffffff #808080 #808080 #ffffff #ffffff
    # alpha ours   0.349 0.4 0.502 0.6
    # alpha ref    35% 35% 40% 40% 50% 50% 60% 60%

**Four for four, on all 64 pixels of each, and each our tile matches exactly one of theirs.** So
the blip resolution, the DIB's bottom-up row order, the black test, the palette lookup and the
opacity are all right at once rather than only the geometry — and the diagonal blip, whose row
order a vertically symmetric test could not have caught, is among them.

### 2.5 The fix

- **`EscherPatternFill`** (`XRay.Office.MsBinary/Escher`) turns a blip's bytes and two colours into
  the tile. `EscherBlips` already hands a DIB out behind a synthesised `BITMAPFILEHEADER`, so the
  pixels come from `DeviceIndependentBitmap.ReadPixels`, which is arithmetic over a byte array
  and needs no codec; the recolouring is the reference's black test; anything that is not an
  8 × 8 bitmap is handed on as authored, which is what the reference does with a texture and a
  picture. `Extent` is the tile's size at 96 dpi and `Opacity` reads the 16.16 fraction.
- **`EscherInk.Read`** no longer resolves the three bitmap fill types to `fillColor`. It reports
  `Ink.BitmapFill` instead and leaves `Ink.Fill` null; `HasInk` counts it, so such a shape is
  still kept.
- **`XlsDrawing.TextureOf`** resolves `fillBlip` against the workbook's blip store, resolves the
  two colours through the workbook's palette, and answers a `SheetShapeTexture`.
- **`SheetShapeInk.Draw`** composes it into a `BitmapPaint` against the shape's own box, applying
  the print zoom to the tile as it already does to the stroke width. *Whether the zoom scales the
  tile is not measured here* — `apron-area.xls` prints at `style:scale-to="100%"`, so no document
  in the corpus separates the two answers.
- **`SheetDrawing.HasInk`** counts a texture. Without it `SheetPageGraphics.DrawInside` culls the
  shape before it is placed, the tile is built and thrown away, and the rendering merely loses
  the wrong grey block instead of gaining the right hatch.

### 2.6 The witness

`apron-area.xls`, three pages, against the banked 26.2.4.2 reference (`movers.tsv`, and the
per-page verdicts re-read by hand):

| | pages | sum `\|ink\|%` | MAJOR | page 1 |
|---|---:|---:|---:|---|
| this round's base (= r103's head) | 3 | **1.51** | 1 | 1.47, MAJOR |
| head | 3 | **0.44** | **0** | 0.41, *shifted* |

Page 1's six flagged regions at the base include three *"a fill or background shading we draw and
the reference does not"* and one *"marks displaced or reshaped"*; at the head the page's verdict
is no longer MAJOR.

**On r103's net regression.** The seat says that round's base for this document was 0.88 and its
nine movers summed 31.44 → 31.58. Those two figures are **r103's, not re-measured here** — this
round has no build of r103's base. Taking them at face value, −1.07 on this document puts the
same nine at **30.51**, which is below 31.44 as well as below 31.58.

---

## 3. O41 — the mechanism is refuted, and the seat is a chart

### 3.1 The two populations of horizontal rule, separated

`gauge-rules.py` takes every full-width horizontal stroke of a page in both PDFs, splits them
into the sheet's own cell rules and the embedded gauge chart's value-axis gridlines by whether
they are inset from the printed width, and pairs each of ours with the nearest of the
reference's **inside its own population** (`gauge-rules.tsv`, pages 13 and 15):

| population | count | worst `\|dy\|` | our colour | reference's |
|---|---:|---:|---|---|
| the sheet's own rules | **32** | **0.02 pt** | `#000000` | `#000000` |
| the gauge's gridlines | **18** | **7.20 pt** | `#000000` | `#808080` |

**The row grid is not out.** That is the seat refuted, on its own two pages, by a script that is
in the probe directory.

### 3.2 What the three rules r103 measured actually were

r103 measured *"three horizontal rules at 267.5 / 289.0 / 310.5 pt where 26.2.4.2 draws three
grey ones at 264.8 / 287.7 / 310.6"*. Converting `gauge-rules.tsv`'s bottom-origin y to the
top-origin those figures are in (page height 842 pt), page 15's top three gauge gridlines are
ours **267.6 / 289.1 / 310.6** against the reference's **265.0 / 287.8 / 310.7**. Both sides
reproduce r103 to a tenth of a point, and *the reference's are grey* — which r103 also recorded
and which no cell border on the page is.

So they are the gauge's gridlines, not rows.

### 3.3 The disagreement is one rectangle, and it is a scale rather than an offset

The residuals down page 15's eight gauge gridlines are 6.66, 5.33, 4.00, 2.68, 1.37, 0.05, −1.30,
−2.60 pt — **linear in index, through zero near the middle**, at 1.32 pt per interval. Page 13's
ten run 7.20 … −2.07 at 1.02 pt per interval. That is not a displaced grid; it is a grid of the
wrong height about a shared centre, and it falls out of the plot rectangles:

| | x0 | y0 | x1 | y1 | size |
|---|---:|---:|---:|---:|---|
| chart frame, ours | 51.00 | 408.82 | 535.04 | 604.00 | 484.04 × 195.18 |
| chart frame, 26.2.4.2 | 50.98 | 407.83 | 535.14 | 603.80 | 484.16 × 195.97 |
| plot area, ours | **73.31** | 423.66 | 478.06 | **574.44** | **404.75 × 150.78** |
| plot area, 26.2.4.2 | **65.19** | 417.00 | 478.94 | **577.04** | **413.75 × 160.04** |

(page 15, bottom-origin, read out of both PDFs' path operators.)

So the anchor is right and the bands inside the chart are not: our value-axis band is 8.12 pt
wider than the reference's and our category-axis band 6.66 pt taller. Two further differences on
the same object, recorded rather than explained: **we draw a 4.25 pt outward tick at every
gridline and the reference draws none**, and **the reference strokes both the frame and the
gridlines in `#808080` where we stroke them black**.

### 3.4 Reach, and why it is worth a round of its own

The gauge is on **9 of `EHEST`'s 24 pages** — 8, 10, 13, 15, 17, 19, 20, 22 and 24 — and those
nine carry **5.45 of the document's 8.06** summed unsigned ink. Nothing else in the document is
near that. *(These two figures are inherited from the pre-restart draft and were not re-measured;
the plot-rectangle and gridline figures above were.)*

**Left open.** It is a chart-layout question — where `ChartLayout` puts a plot rectangle inside an
embedded BIFF chart's frame, and how wide a band an axis' labels and ticks reserve — and it is
not bounded to one document's row heights, which is what the seat said. Whether the same band
rule is wrong on every BIFF chart or only on this one is **not measured here**.

---

## 4. O39 — the hidden value, and the row it was shrinking

### 4.1 The two answers this tree had to one question

`ScOutputData::DrawStrings` clears `bDoCell` for a cell whose data bar or icon set says
`showValue="0"`, and the seat's line numbers are this tree's exactly (checked by hand):

```
1691:    // skip text in cell if data bar/icon set is set and only value selected
1692:    if ( bDoCell )
1693:    {
1694:        if(pInfo->pDataBar && !pInfo->pDataBar->mbShowValue)
1695:            bDoCell = false;
1696:        if(pInfo->pIconSet && !pInfo->pIconSet->mbShowValue)
1697:            bDoCell = false;
1698:    }
```

That is paint time, long after `ScColumn::GetOptimalHeight` has settled the row. This tree
answered the same question twice: `SheetDecoration.HidesValue`, asked by
`SpreadsheetPages.DrawCell` — the mechanism-correct one, and **already in the tree at the base**
— and `XlsxHiddenValues`, a separate partial `iconSet`/`dataBar` reader that dropped the cell's
text in `XlsxSheetReader.ReadCell`.

The second one shortens the row because of a rule that has nothing to do with the string. Our
`SheetOptimalRowHeights` skips a cell with no text, as `ScColumn::GetNeededSize` returns zero for
an *empty cell* before it looks at the pattern (`sc/source/core/data/column2.cxx`:98-102), so
such a cell never reaches the branch that says a cell **whose pattern carries a conditional
format** is measured through the EditEngine rather than through the arithmetic (`:936-941`). A
`showValue="0"` cell is by construction covered by a conditional format, so dropping its text is
exactly the way to make it take the wrong branch.

### 4.2 Re-deriving the deletion rather than inheriting it

Deleting a reader is a large decision, so it was re-argued from scratch. Four things had to hold
and each was checked:

1. **The paint-time answer predates this round.** `SheetDecoration.HidesValue` and its one caller
   `SpreadsheetPages.DrawCell` are untouched by this diff — they are in the base. So this is the
   removal of a *duplicate*, not the replacement of one mechanism by another.
2. **Nothing else read it.** `XlsxHiddenValues` had exactly two call sites, both in
   `XlsxSheetReader`; the build is clean with the type gone.
3. **Its coverage is not wider than the survivor's.** The two are exercised against each other by
   tests that were already in the tree: `SheetHiddenValueTests` asserts, over five pages of
   fixtures, which cells draw their number — the `NoIcons` band that keeps its text, the custom
   band that loses it, a non-custom set that hides everything it covers, a string cell inside a
   hidden range, a rule with no `showValue`, a data bar, and a `gte="0"` threshold. All seven pass
   with the reader gone. Had `XlsxIconSets`/`XlsxDataBars` covered less, those are the tests that
   would have failed.
4. **The rows that shrink are the ones that should not have.** §4.3.

### 4.3 The measurement

`soffice --convert-to fods` on `tests/corpus/features/sheet-cf-icon-set-x14-custom.xlsx` writes
one row style, `ro1`, on **all five** of its `table:table-row`s, at
`style:row-height="0.2071in"` — **14.9112 pt**.

The rows hold an icon and no text, so there is no glyph to measure them by; what there is, is the
clip rectangle the renderer pushes round each icon cell. `row-pitch.py` reads them
(`row-pitch.txt`):

| | drawn row height | drawn pitches |
|---|---:|---|
| base | 13.7764 pt | 13.7763, 13.7764, 13.7764 |
| head | **14.8819 pt** | 14.8819, 14.8819, 14.8818 |
| 26.2.4.2 | 14.9112 pt (`fods`) | — |

Base is **1.135 pt short**; head is **0.029 pt short**. `SheetConditionalRowHeightTests` already
pins 298 twips for a Calibri 11 cell on the measured branch and 276 is the arithmetic one,
`trunc(220 × 1.18) + 40 − 23`; the new test asserts the 298.

### 4.4 The confinement, which is the part the seat warned about

Restoring the text to `ContentTableCell` reaches extraction, `XlsxChartRanges` and the print-area
scan. Measured rather than reasoned about.

`showvalue-census.py` walks every worksheet part of all 307 corpus spreadsheets and counts the
`iconSet`/`dataBar` elements whose `showValue` is off, split by namespace. Re-run for this
write-up; **identical** to the committed `showvalue-census.txt`:

| document | rules |
|---|---|
| `066_Agile_Gantt_chart` | `x14:iconSet` × 6 |
| `077_Inventory_list_with_highlighting` | `x14:iconSet` × 1 |
| `036_Simple_to-do_list` | `main:dataBar` × 1 |

**Three documents, not the two the seat names** — the third states it on a main-namespace data
bar, which an `iconSet`-shaped census does not see.

All three are **byte-identical at base and head** in the full sweep of §5.2. So the print-area
scan and the chart ranges are demonstrably untouched on every document in the corpus that can
reach this code, and **the rendering reach of O39 is 0 of 307 sheets renderings**.

*Why the fixture's rows move and the corpus's three do not is worth stating rather than glossing:
the branch this restores only bites where the row's height is being computed from its contents.
None of the three corpus documents has a hidden-value cell in such a row.* That is an observation
from the sweep, not a rule derived from the source, and it is the one part of §4 that rests on a
null result.

What does change is extraction, and it changes in the right direction
(`extraction-077.txt`): `xray-office extract --format text` on
`077_Inventory_list_with_highlighting` goes from **1545 to 1557 characters**, the twelve added
being the `1`s of the *For reorder* column — and 26.2.4.2's own `--convert-to csv` of the same
workbook writes `,1,IN0001,…` for exactly those rows. The reference's text export has always
carried them; ours did not. The `0`s of the `NoIcons` band were already carried by both.

### 4.5 The fix

`XlsxHiddenValues.cs` is deleted and its two lines in `XlsxSheetReader` with it.
`XlsxIconSets` and `XlsxDataBars` — round 96/97's readers — already cover strictly more than it
did (both namespaces, the `NoIcons` bucket, the extension-only rule, the reverse and `gte`
rules), and they are what fill the `SheetIcon`/`SheetDataBar` that `HidesValue` asks.

---

## 5. Corpus reach, confinement and tests

### 5.1 What can change behaviour, statically

| file | reachable from |
|---|---|
| `XRay.Office.MsBinary/Escher/EscherPatternFill.cs` (new) | `XlsDrawing` alone |
| `XRay.Office.MsBinary/Escher/EscherInk.cs` | `XlsDrawing` alone — `grep EscherInk\.` finds one caller, `XlsDrawing.cs`:418-419 |
| `XRay.Office.MsBinary/Escher/EscherRecordTypes.cs` | five new `const`, no code path |
| `XRay.Office.Spreadsheets/MsBinary/XlsDrawing.cs` | `.xls` |
| `XRay.Office.Spreadsheets/Layout/SheetDrawings.cs` | a new record and a field only `XlsDrawing` sets |
| `XRay.Office.Spreadsheets/Layout/SheetShapeInk.cs`, `SheetPageGraphics.cs` | one more parameter, null everywhere but `.xls` |
| `XRay.Office.Spreadsheets/Ooxml/XlsxSheetReader.cs`, `XlsxHiddenValues.cs` (deleted) | the `xlsx` family |

`XRay.Office.WordProcessing` and `XRay.Office.Presentations` reference none of these types, and an
`.ods` reaches `SheetShapeInk` with `texture` null because only `XlsDrawing` sets it. So the whole
diff is the sheets track by construction; the sweeps below measure it there and control it
elsewhere.

### 5.2 The sheets sweep — 307 documents, both legs

`sweep.sh sheets.list`: every document rendered at base and at head under
`SOURCE_DATE_EPOCH=1700000000`, each render in a directory of its own, `%%EOF`-checked and
hashed (`sheets-sweep.tsv`).

| | |
|---|---:|
| rows, against 307 in `sheets.list`, no duplicates | **307** |
| failed to render at either leg | **0** |
| byte-identical at base and head | **306** |
| **movers** | **1** |

    apron-area.xls  40573f3e868b91a93b597d6826fde873 -> 6948f1dd83f9f7cd2635d7d41621a691

This sweep was run twice — once before the container restart, once afterwards against a freshly
published head — and the two files are **identical row for row, including all 614 md5s**.

The citation corrections of §5.5 landed after that publish, so the head was published a **third**
time from the final source and `apron-area.xls` re-rendered: same md5,
`6948f1dd83f9f7cd2635d7d41621a691`. They are XML doc comments and change no IL, but the tree the
sweeps measure is now checked to be the tree that is committed rather than assumed to be.

### 5.3 The movers scored

`score.sh` re-renders each mover at both legs and scores it on ink against its banked 26.2.4.2
reference (`movers.tsv`):

| document | pages | base `\|ink\|%` | head `\|ink\|%` | base MAJOR | head MAJOR |
|---|---:|---:|---:|---:|---:|
| `apron-area.xls` | 3 | 1.51 | **0.44** | 1 | **0** |

One document moves, it improves, and its MAJOR page clears. Nothing worsens because nothing else
moves.

### 5.4 The control — words, slides and converted `.ods`

`sweep.sh confine.list`, the same instrument over **160** documents outside the `.xls`/`.xlsx`
sheets population: 59 `.docx` + 1 `.doc`, 54 `.pptx` + 6 `.ppt`, and 40 converted `.ods`
(`confine-sweep.tsv`).

| | |
|---|---:|
| rows | **160** |
| failed at either leg | **0** |
| byte-identical | **160** |
| movers | **0** |

### 5.5 Tests

Clean build of `XRay.Office.slnx` from an emptied tree — every `bin/` and `obj/` under the worktree
had been deleted — **0 warnings, 0 errors** under `TreatWarningsAsErrors`.

Each project run on its own and totalled by hand, because a whole-solution run is the one most
likely to truncate silently (`tests-nonfidelity.log`, `tests-fidelity.log`):

| project | passed | failed | skipped |
|---|---:|---:|---:|
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 |
| `XRay.Office.Core.Tests` | 544 | 0 | 0 |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 |
| `XRay.Office.OpenDocument.Tests` | 160 | 0 | 0 |
| `XRay.Office.Presentations.Tests` | 1081 | 0 | 0 |
| `XRay.Office.Rendering.Tests` | 164 | 0 | 0 |
| `XRay.Office.Spreadsheets.Tests` | 1308 | 0 | 0 |
| `XRay.Office.Text.Tests` | 728 | 0 | 0 |
| `XRay.Office.Vector.Tests` | 309 | 0 | 0 |
| `XRay.Office.WordProcessing.Tests` | 1938 | 0 | 0 |
| **ten non-fidelity projects** | **6600** | **0** | **0** |
| `XRay.Office.Fidelity.Tests` | **542** | **10** | **0** |

The ten fidelity failures are exactly the known names and no eleventh:
`PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` × 4 (`paginated.fodt`,
`.doc`, `.rtf`, `.docx`), `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` × 4
(`list-label-overrun.odt`, `.docx`, `.doc`, `.fodt`),
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)` and
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)`.
552 discovered, 0 skipped, so the project covered everything it has.

**New tests, and how each is shown to fail at the base.** Neither is demonstrated by running it
against a base build, because `EscherPatternFill` does not exist there and the file would not
compile; both are demonstrated against the base **binary's own output** instead, which is a
stronger statement about the tree than a red test would be:

- `EscherPatternFillTests` (5 cases). The expected pixels are 26.2.4.2's, taken from its own
  `draw:fill-image` PNGs (§2.4). At the base `EscherInk.Read` answers `Fill = #808080` and
  `BitmapFill` does not exist, and the base binary draws four solid grey blocks where the head
  draws the four tiles — the 1.51 → 0.44 of §5.3.
- `SheetHiddenValueRowHeightTests` (2 cases). It asserts 298 twips on row 1 and that the cell
  keeps its text while the drawing drops it. The base binary draws that row **13.7764 pt** high
  against the head's 14.8819 (§4.3), which is 276 twips, so the first assertion fails at the base
  by measurement; the second is the text the extraction of §4.4 shows the base does not carry.

**Citations corrected.** Every LibreOffice reference in the new code was re-read line by line in
`/home/user/libreoffice-core`, and five of them were off:
`msdffimp.cxx` 1388-1441 → **1401-1443**, 1364-1374 → **1367-1376** (twice), 1406-1412 →
**1408-1414**, 1443-1449 → **1444-1451**, and 1428-1434 → **1432-1435**. `output2.cxx`:1691-1698
and `svpgdi.cxx`:44 are right as written. As always these are *this tree*, which declares
27.2.0.0.alpha0+ and is not the 26.2.4.2 binary's source; every arm above also has a leg measured
against the binary itself.

---

## 6. What this round could not settle

- **O41 is not fixed**, §3. The mechanism the seat named is refuted and the real one is measured;
  what remains is the plot-rectangle layout of an embedded BIFF chart, which is a chart-track
  subsystem rather than a hunk. Two further differences on the same object are recorded and not
  explained: the 4.25 pt outward ticks we draw and it does not, and its grey gridlines against
  our black ones.
- **Whether the print zoom scales a pattern tile is unmeasured.** `apron-area.xls` is the only
  document that states a pattern fill and it prints at 100 %, so no corpus document separates
  "the tile scales with the drawing layer" from "the tile is an absolute size". This tree scales
  it, by the same argument the stroke width is scaled.
- **A texture fill stating no `fillWidth`/`fillHeight` and carrying an encoded raster is
  stretched rather than tiled**, because its natural size is not known without a codec and the
  readers do not have one. **0 of the 562 corpus shape containers state a texture or a picture
  fill**, so no document exercises it.
- **Why O40's corpus count was stale is not established.** The seat supposed the two missing
  shapes were grouped; none of the four is a grouped shape, so that explanation is wrong and no
  other was measured. It does not affect the fix.
- **O39's nil rendering reach is a null result**, §4.4. Three documents state the attribute and
  none of them has a hidden-value cell in a content-measured row; that is what the sweep says,
  not a rule read out of the reference.
- **r103's own figures for its net regression are quoted, not re-measured**, §2.6.
- The gauge's page list and their share of `EHEST`'s ink (§3.4) are inherited from the
  pre-restart draft and were not re-measured.

## 7. Files

| file | what it is |
|---|---|
| `results.md` | this |
| `fill-census.py`, `fill-census.txt`, `fill-census-summary.txt` | every Escher shape container in the 64 `.xls` with its fill type, its two fill colours, its opacity and the form of its `fillBlip` |
| `tile-check.py`, `tile-check.txt` | our four resolved pattern tiles against 26.2.4.2's four, pixel for pixel, plus the alpha each side states |
| `tile-pitch.py`, `tile-pitch.txt` | how wide one tile is drawn, on each side, off the two PDFs |
| `showvalue-census.py`, `showvalue-census.txt`, `showvalue-summary.txt` | every corpus worksheet's `iconSet`/`dataBar` elements whose `showValue` is off, by namespace |
| `row-pitch.py`, `row-pitch.txt` | the drawn row pitch of the O39 fixture at both legs, against the reference's own `style:row-height` |
| `extraction-077.txt` | the extraction diff O39 causes, beside 26.2.4.2's own csv export |
| `gauge-rules.py`, `gauge-rules.tsv` | every full-width horizontal rule of `EHEST` pages 13 and 15, sheet rules and gauge gridlines separated, ours paired with the reference's |
| `sweep.sh`, `sheets.list`, `sheets-sweep.tsv` | the 307 sheets documents rendered at both binaries, `%%EOF`-checked and hashed |
| `confine.list`, `confine-sweep.tsv` | the same over 60 words, 60 slides and 40 converted `.ods` |
| `score.sh`, `movers.tsv` | each mover rendered again and scored on ink against its banked reference |
| `tests-nonfidelity.log`, `tests-fidelity.log` | the two test runs §5.5 quotes, as they came out |
