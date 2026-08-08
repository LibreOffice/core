# Sheets round twenty-six — probe data

Base commit `d7fd6cf13`. Reference renderer LibreOffice 24.2.7.2. Every sweep ran against a
checksummed copy of the CLI so a rebuild could not disturb it; the copy's `Paperless*.dll` hashes
were diffed against the tree's before each run started, and the `measuring …` line named the copy.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `d7fd6cf13`, before any change |
| `after-rowheight-whole-track.tsv` | the same after the row-height change alone |
| `after-drawing-whole-track.tsv` | the same after the drawing change on top |
| `mkrot.py` | writes the flat-ODS row-height probe |
| `rot-probe-reference.fods` | LibreOffice's own round trip of it — the row heights this is fitted to |
| `mkdraw.py` | writes the flat-ODS drawing probe |
| `mutate.sh`, `mutate-draw.sh` | the reintroduced bugs each test was verified against |

All three sweeps: 171 rows, no duplicate path, zero `ref-failed`.

## The baseline reproduces the brief, and reproduces r25 row for row

| | matches | abs page error | exact page counts |
|---|---|---|---|
| `base-whole-track.tsv` | **145/171** | **90** | **154** |

Per batch: 001–009 all pass, then 010 7/10, 011 6/10, 012 8/10, 013 8/10, 014 9/10, 015 5/9,
016 4/9, 017 6/10, 018 3/4. `diff` against round twenty-five's committed after-sweep is empty —
all 171 rows identical — so the base commit is the one that was briefed.

## Batch 010 is bounded by another track's work, and here is the evidence

Its three failures are exactly the three the brief predicted, and nothing here can move them:

| document | verdict | why |
|---|---|---|
| `EHEST-Pre-departure-checklist…xls` | 24/24 pages, 8018/8382 words | `ChartLabel`'s font — the slides track's work this round |
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 46/46 pages, 4650/4808 words | the same |
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 38/38 pages, 1342/1610 words | a date category axis and a reference count inflated by one `Tj` per glyph; reachable ≈1409 against a 1578 threshold |

So the round was spent on the one lead the brief left open, which turned out to reach eight other
documents as well.

## A turned cell's row is its text's width — 216 of 216 probe rows exact

`mkrot.py` writes six angles × three sizes × two wrap settings × six string lengths, every row
asking for an optimal height. LibreOffice's flat-ODF round trip of it states the height it computed
for each, and all 216 are now reproduced exactly. Three findings, each separable in the fixture and
each caught by its own reintroduced bug:

- **Exactly ninety degrees is an orientation, not an angle.** `ScPatternAttr::GetCellOrientation`
  (`patattr.cxx:529-547`) reads 9000 and 27000 as `BottomUp` and `TopBottom`, and `nRotate` is only
  read when the orientation came back `Standard` (`column2.cxx:231-238`). The quarter turns
  therefore take a branch whose height is simply the text's width — uncapped, and with no floor,
  because `bStdAllowed` (`:925`) gates both `SC_ROT_BREAK_FACTOR`'s cap and the sheet's minimum on
  that same test. **A single letter in a 10 pt cell asks for 149 twips at 90° and 257 at 45°.**
- **Nothing is wrapped in the measurement.** The whole string is measured on one line whatever the
  wrap flag says; the flag reaches the answer only as the cap, which the quarter turns escape. At
  eleven point a wrapping 45° cell stops at 1373 twips however long its text is and the same cell at
  90° reaches 7358.
- **A glyph advances by whole device pixels**, so a string's width is the sum of its rounded
  advances and not the rounded sum. Fourteen of the eighteen distinct widths agree under either
  reading; the four twelve-point ones differ by up to 1.4%, which is what settles it. Restoring the
  rounded total fails 18 of the 36 test cases, so it is load-bearing on half the fixture.

The sheet's minimum moved inside `AttributeHeight`, which is where `lcl_GetAttribHeight` has it
(`column2.cxx:889`). Every row that had a floor still has one; the quarter-turned rows, correctly,
do not.

**Found by the probe and fixed alongside**: the ODF reader *clamped* a rotation angle instead of
folding it, so 270° became +90 and 315° became +90 as well. Reach on this corpus is nil — no sheets
document is ODF with a turned cell — and without it six of the probe's sheets could not be read.

### Its reach, measured by rendering, is one document and it moves nothing

`after-rowheight-whole-track.tsv` is **row for row identical to the baseline** — same 145/171, same
page error 90, same 154 exact, same word count on every one of the 171. Rendering all 171 with both
CLIs and comparing with `/CreationDate`, `/ModDate` and `/ID` masked: **1 of 171 changes what is
drawn**, `Keywords_Mapping_Graphs_and_Charts.xlsx`. That is the honest headline for this half.

A second document appeared in that comparison and is the known false positive: `PBN Matrix NAAs
(V01).xlsx` prints the date in its header, and rendered with `SOURCE_DATE_EPOCH` set it is
byte-identical between the two CLIs. Round twenty-five's floor of 17 clock-dependent documents is
still there for any sweep that does not pin the instant.

## A turned cell's lines are a block, laid out and then turned

Every line of a turned cell was drawn at one point — the cell's bottom-left corner. Measured on
`Keywords_Mapping_Graphs_and_Charts.xlsx` page 43: four records at exactly `(363.49, 262.53)`, and
every rotated column head 10.48 pt left of where the reference puts it.

**Round twenty-five's half of that measurement was mispaired, and the correction is worth having
even though the conclusion stands.** It reported the reference as stepping those lines at
`(394.38, 761.89 / 731.90 / 716.91)`. Those three records are the reference's ordinary
regular-weight data column, which *we also draw* at `(394.38, 761.86 / 731.86 / 716.87)`; the
reference's rotated heads are the `Carlito-Bold` records at x = 340.70, 373.98, 404.87, 447.65,
487.28 and 513.41, each cell written one `Tj` per glyph up the page. So the reference does not step
a wrapped rotated cell's lines at all on that page — **it does not wrap the cell**, which is the
third finding below and a different defect from the one the pairing suggested. The 10.48 pt is the
constant offset between the two sets of head positions and it reproduces on all six columns.

Calc hands the whole EditEngine block to `DrawText_ToPosition` with an orientation
(`output2.cxx:3707`, `:3947`), so the lines are laid out unturned from an anchor and the *block* is
turned about it. `mkdraw.py` writes a probe that states every row height and turns the header and
footer off, so both renderers' page bodies start together and the positions can be compared
directly. Three consequences, each caught by its own reintroduced bug:

| | ours | reference |
|---|---|---|
| 90°, one line, two cells | 83.47 / 540.99, 83.47 / 360.99 | 83.48 / 540.99, 83.48 / 360.99 |
| 90°, wrapped into two | 83.47 / 360.99 and 96.90 / 360.99 | 83.48 / 360.99 and 96.97 / 360.99 |
| −90°, one line, two cells | 76.00 / 553.14, 76.00 / 626.43 | 76.00 / 553.10, 76.00 / 626.40 |
| −90°, wrapped, second line | 76.00 / 492.47 | 76.00 / 492.41 |

- the first line's baseline is an **ascent** into the block and each line after it a line height
  further;
- a **clockwise** cell hangs from a different corner, its block running down and to the left;
- a wrapping cell **breaks against the cell's height**, because the paper a vertically oriented
  cell's EditEngine gets is its align rectangle turned with it (`calcPaperSize`, `:2691`). The
  probe's 53-character string takes two lines in a 2.5 in row and would take eight in the 1.5 in
  column.

**One case is recorded as a shortfall and not as a pass.** On the clockwise wrapping sheet the
reference starts both lines at 492.41 and we start the first at 494.96. The 2.55 pt is exactly the
width of the space that ends that line: EditEngine leaves a line's trailing blank out of the width
it aligns by. It is a general text-layout rule rather than anything about turning, and no corpus
document is turned clockwise.

**An obliquely turned cell is left exactly as it was, deliberately.** `DrawRotated` centres its
block across the column and lifts its anchor by the block's height times the cosine
(`:5290-5330`), which is not `DrawEditBottomTop`'s rule; the probe shows the reference disagreeing
with us on the 45° sheets and nothing in the corpus is turned by anything but a quarter to fit the
second formula against. Half-fixing it would have been worse than leaving it.

### Reach, and what it did to the gate

`after-drawing-whole-track.tsv` against the baseline: **11 of 171 documents change what is drawn**,
confirmed by re-rendering each with `SOURCE_DATE_EPOCH` set so the clock cannot contribute. Eight of
the eleven are `.xls`.

**That number is also a correction to my own census.** Counting `textRotation` in `xl/styles.xml`
found four documents on this track and I nearly quoted it as the ceiling; the BIFF workbooks state
their rotation in an `XF` record, which no zip census can see, and they are two thirds of the real
population. A census that cannot read one of the corpus's two containers is a ceiling on the half
it can read, and nothing at all on the other.

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| base | 145/171 | 90 | 154 | 43694 |
| after both changes | 145/171 | 90 | 154 | **43198** |

**No verdict moves, and no page count moves.** Nine documents' word counts change, four of them
towards the reference and five away, for a net improvement of 496 in absolute word error. The five
that go the wrong way go there for the same reason the change is right: a heading that used to wrap
into six lines scored six `pdftotext` tokens and now scores one, while the reference draws it as one
line written a glyph at a time and poppler still finds two or three words in it. The ink is closer
and the token count is further, which is the word gate measuring operator granularity rather than
text.

The pixel instrument agrees where it can see anything at all. `pdf-image-diff.py` over three of the
eleven, against the reference: `programs contact list as of 07-01-10.xls` goes from **6.25 `|ink|%`
over three major pages to 4.89 over two**, and `Keywords_Mapping` and `cy06_primary_np_comm` do not
move — their changes are below what 512 pixels on the long edge can resolve, and `Keywords_Mapping`'s
eleven major pages are its chart pages, which are the font gap and not this.

## Tests

`SheetRotatedRowHeightTests`, 36 cases covering 216 row heights. `SheetRotatedTextPlacementTests`,
9 cases. Every one of the eleven reintroduced bugs fails at least one case, and there are no drift
guards in either file:

| mutation | cases that fail |
|---|---|
| a quarter turn treated as a rotation | 12 |
| the wrap cap dropped | 12 |
| the floor applied to every row | 12 |
| a string's width rounded once rather than per glyph | 18 |
| a turned cell left unmeasurable, as before | 36 |
| an ODF angle clamped rather than folded | 6 |
| the first line's ascent dropped | 7 |
| every line drawn on one origin | 2 |
| one anchor for both signs of the angle | 4 |
| the along-line offset dropped | 4 |
| a turned cell wrapped at the column width | 5 |

Note the sixth row: clamping instead of folding fails only the six 315° sheets and **not** the six
270° ones, because clamping 270 to +90 happens to give the same six row heights a correct −90 does.
The drawing probe is what separates them.

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere:

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations | Fidelity |
|---|---|---|---|---|---|---|---|---|---|---|
| 247 | 109 | 240 | 291 | 119 | 259 | 125 | 660 | **543** | 528 | 550 |

Every count is the briefed known-good except Spreadsheets, which is 498 plus this round's 45.
