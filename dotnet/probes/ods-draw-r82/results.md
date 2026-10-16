# round 82 — the `.ods` column: a shape anchored in a cell is a drawing, not the cell

`.ods` **255 → 268 of 307**, measured against LibreOffice **26.2.4.2** on the converted corpus.

## Environment

| | |
|---|---|
| tree | `/home/user/wt-odsdraw`, branch `agent/odsdraw`, base `e6be864e4` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything here** |
| corpus | `/home/user/corpus-odf`, 26.2.4.2's own conversion of the 947-document corpus; the `.ods` column is 307 files |
| reference bank | `/home/user/gate-odf-r78/ref`, rendered at 26.2.4.2 — the same bytes rounds 77 and 80 scored against |
| original track | `/home/user/sample-files/sheets`, 307 `.xls`/`.xlsx`/`.xlsm` |
| C++ read-only | `/home/user/libreoffice-core` |
| fonts | system fontconfig; all five tarball confound families aside |
| load | a whole-corpus gate ran in the main checkout throughout and a second agent swept the `.odt` column beside it. This round used two workers. Load average 3–19 on four cores. |
| date | 2026-09-08 |

`sweep-ours.sh` re-renders our half alone and applies `batch-check.sh`'s verdict rule — the same
`awk` band, the same floor of 15 alphanumeric characters — to the banked reference bytes. The
reuse is sound because the diff is confined to `dotnet/src` and cannot reach `soffice`.

**The base was re-derived rather than trusted, and it reproduces round 80 exactly.** `before.tsv`
is this round's base at `e6be864e4`, rendered fresh; against `probes/ods-resid-r80/after.tsv` it
agrees on verdict, page count and alphanumeric count for **304 of 307 rows**, and the three that
differ are all this container's contention rather than the tree — see *Four rows are the box and
not the tree* below.

## The column

| | match | pages | pages,words | words | ours-failed |
|---|---:|---:|---:|---:|---:|
| before, `e6be864e4` | 251 | 6 | 10 | 37 | 3 |
| before, the four timeouts scored on their own | **255** | 6 | 9 | 37 | 0 |
| after | 267 | 6 | 3 | 30 | 1 |
| after, the timeout scored on its own | **268** | 6 | 3 | 30 | 0 |

**Sixteen verdicts move and every one of them is a gain; none is lost.** Three of the sixteen are
the contention rows coming back, so the tree's own movement is **thirteen**, and the same figure
comes out of the exclusion recipe `dotnet/CLAUDE.md` prescribes: dropping every row that failed on
either side in either run leaves 303 comparable rows on which `match` goes **251 → 264**.

| document | before | after | reference |
|---|---:|---:|---:|
| `SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA` | 11 pages, 1244 glyphs | **4, 2798** | 4, 2779 |
| `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016` | 27, 34 403 | **24, 40 106** | 24, 39 744 |
| `arp-sop-300-Exhibit-A-Table-Templates` | 15, 19 011 | **11, 19 526** | 11, 19 784 |
| `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0` | 17, 7005 | **16, 7846** | 16, 7846 |
| `027_Simple_personal_cash_flow_statement` | 11, 7585 | **10, 7981** | 10, 7949 |
| `Part_129_Operators` | 22, 19 223 | **23, 19 595** | 23, 19 595 |
| `Part_375_Operators` | 19, 11 849 | 19, **12 221** | 19, 12 221 |
| `035_Project_plan_for_law_firms` | 4, 3482 | 4, **3879** | 4, 3879 |
| `PC1000` | 13, 3667 | 13, **3831** | 13, 3831 |
| `apron-area` | 3, 1726 | 3, **1814** | 3, 1814 |
| `TDA_Smoke-Detectors` | 28, 17 766 | 28, **18 138** | 28, 18 137 |
| `026_Monthly_cash_flow_statement` | 11, 7571 | 11, **7896** | 11, 7852 |
| `070_Equipment_inventory_list` | 1, 1175 | 1, **1036** | 1, 1028 |

**Cost, measured on the whole column rather than censused.** Of the 303 comparable rows, **276 do
not change either count** and 27 move; over those 27 the sum of `|ours − reference|` alphanumeric
characters goes **17 884 → 3914**, 23 closer and 3 further. The three further are
`037_Personal_money_tracker` (3 → 16 of 2443), `081_Project_performance_report` (2 → 8 of 2032)
and `076_Inventory_list_accessibility_guide` (145 → 150 of 4782); all three are `match` before and
after except the last, which was and remains `words`.

## 1. A shape anchored in a Calc cell is in the drawing layer, and no cell question can see it

**Rule.** ODF fastens a cell-anchored drawing by *containment*: `ScXMLExport`'s `WriteShapes`
writes it as a child of the `table:table-cell` it belongs to, so a walk that reads the cell reads
the shape. Calc never reads it that way. A drawing object is an `SdrObject` on the sheet's draw
page and every question the print path asks about a *cell* goes through the column's own cell
storage:

- `ScColumn::GetOptimalHeight` walks `maCells` and measures cells
  (`sc/source/core/data/column2.cxx`:894-949) — a row is not made taller by an object over it;
- `ScTable::GetCellArea` counts cells, not objects (`table1.cxx`:1091-1120);
- `ScTable::ExtendPrintArea` spills a *cell's* string into the empty cells beside it
  (`table1.cxx`:2127, `MaybeAddExtraColumn` at `:2218`);
- and an object reaches the page by a different route entirely, `ScDrawLayer::GetPrintArea`
  (`drwlayer.cxx`:1344-1424), which `ScDocument::GetPrintArea` maxes into the cell answer
  (`documen2.cxx`:644-664).

`OdfContentReader` appended those paragraphs into the `ContentTableCell`, and four separate seats
then took `cell.GetText()` as the cell's own text: `SheetLayout.UsedRange`,
`SheetLayout.LastDataRowByColumn`, `SheetOptimalRowHeights` and `SheetTextOverflow`. So a text box
made its row as many lines tall as it had paragraphs, spilled its longest line across the columns
beside it, and made a cell holding nothing but a shape count as content.

**Established at the reference with no free parameter**, on
`dotnet/tests/corpus/features/sheet-shape-text.fods` — four rows of one short string each, with a
five-paragraph text box anchored in B2. 26.2.4.2 draws the four cell strings at baselines 66.388,
79.173, 91.957 and 104.741, **a flat 12.784 pt apart**, so the text box costs its row nothing at
all; its own five lines are drawn from the shape's own corner over the rows beneath.

`ContentTableCell.GetOwnText()` is the seat. The reader now wraps a spreadsheet cell's shape in a
`ContentSection` of kind `Frame` — the same marker the word-processing path has always used for a
text box — and `GetOwnText` skips it while `GetText` does not, so **extraction is unchanged**: a
caller indexing the sheet still finds the text box's sentences under B2.

## 2. So the shape has to be drawn, and the sheet reader read only `draw:frame`

Taking the text out of the cell without putting it anywhere would have lost the words. The other
half of the round is therefore `OdsShapeText`, which builds the `SheetShapeText` the SpreadsheetML
and BIFF readers have built since round 56, and `OdsDrawings.Shapes`, which yields **every**
`draw:` element a cell holds rather than `draw:frame` alone.

**Census over the 307 converted `.ods`** (`census.py`, `census-shapes.txt`), walking through the
two transparent wrappers `draw:g` and `draw:a`:

| kind in a cell | count | with a `text:p` | with ink in it | characters | documents |
|---|---:|---:|---:|---:|---:|
| `draw:custom-shape` | **604** | 604 | **137** | 25 675 | 54 |
| `draw:frame` | 495 | 343 | 0 | 0 | 134 |
| `draw:control` | 72 | 0 | 0 | 0 | 5 |
| `draw:line` | 41 | 41 | 0 | 0 | 1 |
| `draw:connector` | 31 | 31 | 0 | 0 | 2 |

So the text is entirely in the custom shapes — **137 of them in 32 documents** — and the other
four kinds carry none. A shape that embeds nothing and inks nothing is still answered null, which
keeps it out of `SheetDrawingArea` as well; that is a deliberate under-count, because
`ScDrawLayer::GetPrintArea` widens the block to cover *every* object, and it is left with its seat
below.

### 2a. Two of the four styles a shape's text could inherit from reach it, and two do not

This is the part that had to be measured rather than derived, and the natural reading is wrong.
The box properties come from the shape's `draw:style-name` graphic style; the **character**
properties come from the paragraph's own `text:style-name` and its spans' and from **nowhere
else**. Neither the graphic style's `style:text-properties` nor the `draw:text-style-name`
paragraph style reaches a run on a Calc sheet.

Four one-attribute variants of the fixture, rendered through 26.2.4.2 (`variants.py`, output in
`var/` and `varout/`), all leave the rendering identical:

| variant | third paragraph, which names no style |
|---|---|
| control | 11.99 pt LiberationSerif at x 184.507 |
| the graphic style's `fo:font-size="18pt"` removed | **identical** |
| that size changed to 8 pt | **identical** |
| `fo:font-size="14pt"` put on the `draw:text-style-name` style | **identical** |
| `draw:text-style-name` removed outright | **identical** |

11.99 pt in Liberation Serif is the EditEngine pool's 12 pt and `DefaultFontType::LATIN_TEXT`,
which `SheetShapeText.DefaultSize` and `.DefaultFamily` already carry. **The control ran too**:
naming that same style on the `text:p` itself moves the line to x 223.058, centred, and a
paragraph style stating `fo:font-size="14pt"` and `fo:font-family="Liberation Mono"` draws it at
14 pt in LiberationMono. So the file is not being ignored — those two levels simply do not carry.
`SdXMLShapeContext::SetStyle` resolves `draw:text-style-name` through the *shape import's* own
automatic-styles context (`xmloff/source/draw/ximpshap.cxx`:740-757), and Calc's import registers
its paragraph automatic styles elsewhere.

### 2b. What the graphic style does decide, and where each rule comes from

| property | ODF | C++ | default |
|---|---|---|---|
| the four insets | `fo:padding-left`… | `SDRATTR_TEXT_LEFTDIST` … | **0**, `svx/source/svdraw/svdattr.cxx`:247-250 |
| wrap | `fo:wrap-option` | `PROP_TextWordWrap`, `sdpropls.cxx`:157 | on, `svdattr.cxx`:267 |
| vertical anchor | `draw:textarea-vertical-align` | `PROP_TextVerticalAdjust`, `:140`, enum `:658-665` | TOP, `include/svx/sdtaitm.hxx`:38 |
| horizontal | `draw:textarea-horizontal-align` | `PROP_TextHorizontalAdjust`, `:139`, enum `:649-656` | **BLOCK**, `sdtaitm.hxx`:64 |
| clip the overflow | **`style:overflow-behavior`** | `PROP_TextClipVerticalOverflow`, `:159` | off |

Each is checked on the fixture: the insets put the first glyph at x 184.507 against the 184.479
that cell B2's left edge plus `svg:x` plus `fo:padding-left` compute; `center` moves all four lines
to 210.472; `bottom` moves the block down 15.959 pt. Under the default BLOCK the *paragraph's* own
`fo:text-align` decides, which is why the fixture's centred paragraph moves alone.

**`style:overflow-behavior` is the ODF spelling of DrawingML's `vertOverflow="clip"`**, and it maps
to the same UNO property through a named boolean whose true token is `clip` and whose false one is
`auto-create-new-frame` (`xmloff/source/style/prhdlfac.cxx`:483-487). **95 of the 137 inked shapes
state it.** That is one more instance of the rule `dotnet/CLAUDE.md` records five times over — an
ODF attribute that exists under a name the specification's own namespace does not suggest — and it
wears the fourth disguise, an attribute that looks like it has no ODF equivalent at all.

Censused over the same 137 (`gstyles`, reproduced in `census-shapes.txt`'s companion): all 137
state all four paddings, both text-area alignments and `fo:wrap-option`; 94 anchor top, 41 middle;
135 are horizontally BLOCK and 2 centred; 129 wrap and 8 do not; and all 104 that state
`draw:fit-to-size`/`style:shrink-to-fit` state them **false**, so no autofit is involved.

### 2c. Two rules inside the text body, both measured and both the opposite of the obvious

**A `text:tab` draws no glyph and is not in the reference's text layer.** Read character by
character out of 26.2.4.2's PDF of the fixture, `the` ends at 244.642 and `shape` begins at
245.735 — 1.09 pt of advance and no character. This reader contributes nothing for one, which
loses that 1.09 pt; **30 tabs in 5 of the 307**.

**A percentage `fo:font-size` is passed over rather than applied.** A span stating
`fo:font-size="50%"` inside a paragraph style stating 14 pt is drawn by 26.2.4.2 at **14 pt**, not
at 7: in ODF the proportion is of the *parent style's* size, and an automatic text style has no
parent. Multiplying the enclosing level's size would have halved it. No shape in the column states
one, so this is the safe reading of an unwitnessed case rather than a measured law about
proportions in general.

## 3. A turned shape's rectangle is the transform's, and the end cell must not be added to it

The one document this round made *worse* before it was finished, and the fix is a rule worth
keeping. `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.ods` went 17 pages against the reference's 16 to
**20** as soon as its three text boxes became drawings.

`TextBox 1` is turned a half turn — `draw:transform="rotate (-3.14159…) translate (…)"` — and so
states no `svg:x`, while carrying `table:end-cell-address="….AS3"`. Taking the top-left corner
from the transform (which is 23.65 inches *left* of the anchor cell, the rotation having put it
there) and the far corner from the end cell (47.73 inches of columns to AS) gives a rectangle
neither statement describes, and it widened the printed block by four pages.

**Both halves are measured.** Dropping the three shapes takes us to 16 and leaves 26.2.4.2 at 16;
dropping only the end-cell attributes takes us to 16 and leaves 26.2.4.2 at 16; dropping only that
one shape's end cell does the same. So the end cell decides nothing there. **And the control is
the other way round**: `endcell.py` puts a right-aligned 1-inch text box at A2 on 1-inch columns
and 26.2.4.2 draws its line at x **289.644** with `table:end-cell-address="Probe.E2"` against
**73.644** without it — the four inches the end cell states, which this tree reproduces at 289.686
and 73.686. So the end cell governs a shape that states `svg:x`, and a transform governs one that
does not; the two are the same rectangle stated twice and must not be combined.

**Reach: 2 shapes in 2 of the 307** state a transform, no `svg:x` and an end cell
(`transform-endcell-census.py`, `census-transform-endcell.txt`). `SIL_TDB648`'s 74 grouped,
transformed watermarks are unaffected — the end cell there is on the `draw:g` — and it is 88 pages
of 88 before and after.

## 4. What is left, classified

`classify.py` puts each failing row in the first of four classes that fits, in decreasing order of
how specifically it explains a divergence: an inked shape in a cell, an embedded chart, a volatile
`TODAY()`/`NOW()`/`RAND()`, or none of them.

| class | before | after |
|---|---:|---:|
| chart | 25 | **23** |
| shape | 13 | **5** |
| other | 12 | 9 |
| volatile | 3 | 3 |
| total failing rows | 53 | 40 |

**The 46 character rows the brief names split 25 chart, 12 shape, 6 other and 3 volatile**, and
they are now **33** — 23, 4, 3 and 3. The 15 pagination rows split 6 shape and 9 other, and are
now **9**.

**The chart class is the largest thing left in the column and it is one question, not twenty-five.**
Two of them were read directly against the reference. On
`052_Manufacturing_output_chart` and `058_Social_media_engagement_data` — 357 of 489 and 356 of 488
characters — the reference draws a data label on every point of the series and we draw none, and
its value axis runs 0…80 in tens where ours runs 0…12 in twos on the same data of 25…73. The
missing labels are the whole of the character gap and the axis is a second defect beside it. Both
documents also print a different date column from the reference, which is the volatile class
arriving inside a chart-class document; the two are separable and the labels are the larger half.

## 5. The original `.xlsx`/`.xls` track does not move

`render-hash.sh` renders a track with one binary and records pages, alphanumeric characters and an
md5 of the PDF with its `/CreationDate` and XMP `dc:date` masked, under a fixed
`SOURCE_DATE_EPOCH`, deleting each PDF as it goes. Run over `/home/user/sample-files/sheets` — 307
`.xls`/`.xlsx`/`.xlsm` — with this round's binary, `orig-after.tsv` is **identical on all 307 rows**
to `probes/ods-resid-r80/orig-after.tsv`, which was taken at this round's base for the sheets track,
with no failure on either side. It was run twice, once after each of the round's two builds, with
the same result.

That is what the gating predicts, and the gating is checkable rather than argued.
`OdsShapeText` has one caller and `OdsDrawings.Read` has one caller, `OdsSpreadsheetDocument`;
`OdfContentReader.CellShapesAreOwnFlow` is set by `OdsReader` alone. The one change that is *not*
behind an ODF gate is `ContentTableCell.GetOwnText()`, and it answers exactly `GetText()` for a
cell holding no section — which no SpreadsheetML or BIFF cell does, because both readers hoist a
chart and a note out of the cell.

## 6. What this brief got wrong

**(a) "15 pagination rows and 46 rows failing on characters" — both correct**, re-derived here
from the gate rows rather than taken: verdicts containing `pages` number 15 and verdicts containing
`words` number 46 in `probes/ods-resid-r80/after.tsv`, and this round's own base sweep reproduces
both once its three contention rows are set aside. The brief's warning about the previous round's
two miscounts is accurate and the correction has held.

**(b) "253 cells in 46 of the 307."** That is round 80's census of *cells holding a multi-paragraph
shape*, and it is not the reach of the defect. Counted by shape rather than by cell, and by whether
the shape has ink rather than whether it has a `text:p`, it is **137 inked shapes in 32 documents**
— and the 46 documents include 14 in which every such shape is empty, where nothing could ever have
moved.

**(c) "380 `draw:custom-shape`, 148 `draw:a`, 78 `draw:control`, 31 `draw:connector`, 17
`draw:line`."** Counted here over a cell's *direct* children, which is what round 77's census did:
**371, 72, 72, 31, 17**. The `draw:line` and `draw:connector` figures are exact; `draw:a` is out by
a factor of about two, which is the shape of the `grep -r` doubling `dotnet/CLAUDE.md` records.
Counted the way that matters — descending through `draw:g` and `draw:a`, which are transparent
wrappers — it is **604 custom shapes, 41 lines, 31 connectors and 72 controls**, and *none of the
last three carries a single character*. Only the custom shapes were worth this round.

**(d) "Neither holds a multi-paragraph *cell*, so the shape text is the whole of it."** Correct for
both, and the mechanism is wider than the row height the brief attributes it to: on `SSRO` the
cost was also that its 14-paragraph methodology note *spilled* across the sheet and that a cell
holding nothing but the shape counted as content. 11 pages to 4 is more than a row.

**(e) "The presentations reader draws custom shapes; the question is what a sheet does
differently."** The answer is: nearly everything, and reusing `OdpSlideLayout` would have been
wrong. A slide's shape is a `PlacedShape` on an absolute page with its own fill, geometry, autofit
and text body; a sheet's is a `SheetDrawing` anchored to cells and painted by `SheetShapePainter`,
which already existed and already had every rule this needed. What a sheet does differently in the
*reader* is the finding of §2a — on a Calc sheet the graphic style's character properties and
`draw:text-style-name` do not reach a run, and on a slide they do, which is why `OdfTextBody`
resolves a four-level cascade and `OdsShapeText` resolves two.

**(f) "A pagination defect can be a font defect — rule out the face before blaming geometry."**
Ruled out rather than assumed: every one of this round's page-count movements is reproduced by a
one-attribute variant that changes no face at all — the fixture's flat 12.784 pt row pitch, and
`Foreign_SA-CAT`'s 16 pages with its end cells deleted.

## 7. What is left, with its seat

- **The chart class, 23 of the 40 remaining failing rows.** The two documents read against the
  reference in §4 point at *data labels on a line series* and at *the value axis' scale*, and
  neither is a sheet question. `OdfChartPlot` and `Core/Charts`.
- **A worksheet shape's fill and outline are still not read, in any format.** 467 of the 604
  custom shapes in these cells carry no text and are therefore not drawn at all, and 137 of them
  are drawn as bare text over whatever is under them. No gate column can see either, which is the
  `w:pgBorders` shape again. `OdsShapeText` reads the graphic style already, so the fill and stroke
  are a few lines from where they are needed; the harder half is that such a shape then also
  *widens the printed block*, which `ScDrawLayer::GetPrintArea` does for every object and this
  reader does for none of the empty ones.
- **`076_Inventory_list_accessibility_guide`, 4932 characters against 4782.** The only shape-class
  document that moved further from the reference. Its fourteen inked shapes are `roundRect` and
  `round1Rect` navigation buttons, whose preset text rectangle is resolved here at its *default*
  adjustment because ODF states the adjustment as `draw:modifiers` in the shape's own coordinate
  space rather than as DrawingML's hundred-thousandths. A wider text rectangle clips one line
  fewer. `OdsShapeText.Preset`.
- **`TICAPCapability_Final`, 24 781 against 27 777.** The largest character gap left in the column
  and still 11% short after this round's 6056 → 2996. Three of its shapes are inked; whatever else
  it loses is not shape text.
- **`017_Timeline_Templates_for_Excel` 2 pages against 3, `Special-Procedures_2025-07-10` 21
  against 22.** Both are character-exact now and short by one page; both hold inked shapes whose
  extent is presumably what keeps the reference's last page alive.
- **`draw:control`, `draw:connector` and `draw:line` are read as nothing.** 72, 31 and 41 in cells,
  none of them carrying a character. A control's caption is drawn by the control and a line has an
  outline; both are invisible to the gate.
- **The performance tail.** `Global_Market_Forecast` takes 220 s against the gate's 240 s bound and
  is a `match` when it finishes — 567 pages of 567 and 305 549 glyphs of 305 549. Unchanged from
  round 80's 215 s.

## 8. The contamination check, and an inert mask found by running it

`dotnet/CLAUDE.md` asks for one document to be re-rendered after a sweep and byte-compared against
that sweep's own copy. Done on three of this round's movers after the final build: all three differ,
and **all three differ in exactly five bytes, every one of them inside
`/CreationDate(D:20260908…Z)`**. Nothing else moved, the file lengths are equal to the byte, and the
final build was a doc-comment change — so the sweep measured the binary this round ships.

Running that check found something else. `render-hash.sh` masks `/CreationDate ([^)]*)` with a
space before the parenthesis, and `xray-office` writes `/CreationDate(D:…)` with none, so **the mask
this round inherited never matched anything**. It cost nothing here because the same script exports
`SOURCE_DATE_EPOCH`, which pins the date in the ink as well as in the metadata and is what actually
made the two original-track runs comparable. The banked copy has the pattern corrected, with a note
saying it was inert as run.

## 9. Four rows are the box and not the tree, and the tell is in the `pages` column

Three documents are `match` in round 80's bank and in this round's after sweep, and failed in this
round's *base* sweep with seven workers running on four cores: `certification-type-…-Light-Prop`
and `Laser Report 2024 FOIA __Oct (1)` as `ours-failed`, and **`STC_WebList` as `pages,words` with
an empty page count and zero glyphs** — a 240 s timeout that landed after the PDF had begun to be
written, which is exactly the shape the brief warns of. A banked `ours-failed` count undercounts
timeouts, and the discriminator is a row whose own page cell is blank.

## Files

| | |
|---|---|
| `sweep-ours.sh` | render our half of the `.ods` column and score against a banked reference |
| `score.sh` | score two directories of already-rendered PDFs with the gate's own rule |
| `render-hash.sh` | render a track and hash it, one PDF at a time, for a two-binary comparison |
| `compare.py`, `moved.txt` | diff two row files: verdict counts and every row that moved |
| `census.py` | shapes in cells by kind, by whether they carry a `text:p` and by whether it has ink |
| `census-shapes.txt`, `census-direct.txt`, `census-ink.txt` | its three outputs |
| `classify.py`, `classify.txt`, `classify-after.txt` | the failing rows by shape / chart / volatile / other |
| `variants.py` | the eight one-attribute variants of the fixture §2a is established on |
| `endcell.py` | the three-case probe that shows an end cell decides a shape stating `svg:x` |
| `transform-endcell-census.py`, `census-transform-endcell.txt` | shapes stating both a transform and an end cell |
| `before.tsv`, `after.tsv` | the column at `e6be864e4` and at this round's HEAD |
| `orig-after.tsv` | the original sheets track at this round's HEAD, against round 80's |
