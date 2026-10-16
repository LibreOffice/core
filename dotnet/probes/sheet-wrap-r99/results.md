# Round 99 — the sheets residue: a trailing empty paragraph, a merged fill, and a posture nobody states

    ours   = XRay.Office.Cli @ 1acd84bea (base) and @ 1acd84bea + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2, plus the banked
             reference PDFs at /home/user/gate-odf-r80/ref (the 307 `.ods`) and
             /home/user/gate-orig-r83/ref (the 947 originals)
    corpus = /home/user/corpus-odf/sheets — 307 `.ods`; /home/user/sample-files/sheets —
             243 `.xlsx`/`.xlsm` and 64 `.xls`
    rule   = column 9, `glyphs`, within max(2 %, 15); `gate-columns.py` applies it to one
             directory of our renderings against the banked reference
    fonts  = the five tarball confounds as `dotnet/CLAUDE.md` records them; none moved this round
    date   = 2026-09-11

## 0. The headline

| seat | what it was | what it is |
|---|---|---|
| **O16** `TK-Syllabus` | *"a trailing empty paragraph, and then the wrap"* | **the trailing empty paragraph is the whole of the row-height error, and the brief's second cause is an instrument artefact.** Row for row against 26.2.4.2's own resolved heights, **82909 of 82924 rows already agreed** at the base; the fifteen that did not are all +1 line and thirteen of them are this rule. **Fixed**: 15 → 2 disagreeing rows, `.xlsx` ink **205.57 → 158.42** and MAJOR **66 → 34**; over the whole reach, **759.30 → 665.13** and **318 → 252** |
| **O32** `TOGAF9-Tool-ConfReqts-CSQ` | uncharacterised | **characterised, not fixed.** Its row heights are exact (739 of 739) and its page-1 text agrees span for span; three of its four MAJOR pages are **a merged cell's background painted only as wide as the printed column block** where 26.2.4.2 paints it across the whole merge and lets the paper clip it — **3.54 of the 16.76**. The fourth MAJOR page is a displaced-marks region and is not this; the other 10.43 over 24 pages is diffuse |
| **O33** BIFF shape run colour and posture | unmeasured | **censused, half of it filed nil.** **0 of 367** `TXO` formatting runs in the 64 `.xls` state an italic, and 26.2.4.2's own resolved view of the nine shape-path workbooks holds **0 italic spans**. The colour half is **119 spans in 2 of 64** and is left seated |
| **O17** `alle einzeln` | not reached | not reached |

**The brief for O16 was wrong twice and the measurement is what says so.** *"There is a second
cause and it is the wrap itself"* rests on round 98's *"over pages 1-60 the reference draws 8521
lines to our 7987"*. The reference's own **row heights** disagree with ours on **15 rows in
82924** (§1), which no wrap defect can produce — and the 534-line gap itself is an artefact of
the instrument (§1.4). Both halves of that are measured below.

## 1. O16 — the wrap is not a second cause, and the row heights say so before anything is rendered

`fodsrows.py` (round 98's) prints 26.2.4.2's resolved `style:row-height` for every row of a
workbook out of one five-second `--convert-to fods`. `rowdump.cs` is the other half and was the
round's first build: a thirty-line harness over `SheetLayout.Grid.Rows` that prints ours in the
same shape. `rowdiff.py` joins the two and converts each height to a line count with the
document's own quantum, `(h − 299.952) / 268.3 + 1`.

At the base, over all sixteen sheets of `TK-Syllabus-Comparison-Document-v2.ods`:

| | |
|---|---:|
| rows compared | **82924** |
| rows agreeing to within a twip | **82909** (100.0 %) |
| rows differing | **15** |
| line delta, ref − ours | **+1 on all fifteen** |

`tk-base.tsv` is the fifteen. There is no distribution of errors: a run of 82924 rows in which
fifteen are one line short, all by exactly one line, is one rule with a handful of witnesses
rather than a wrap that is systematically wrong.

*The coverage is not an artefact of the join. Each sheet's reference and our own row sets
intersect in all but one or two trailing default rows — 5825 of 5826 on `010 Air Law`, 6003 of
6005 on `021 AGK`, and so on — and the script caps a run at 5000 rows so that the
1 048 575-row tail of each sheet is not expanded.*

### 1.1 The one rule, and where it actually sits

Round 98 attributed the first divergence to `Reader Instructions` row 5 — eighteen paragraphs of
which the eighteenth is empty, 4864.752 twips at the reference against our 4597 — and predicted
the seat was `SheetOptimalRowHeights.WrappedHeight` and `ParagraphsOf`. **It is neither.**

`ParagraphsOf` already counts a trailing empty paragraph, and so does `SheetTextLayout.LineCount`:
both split on the break before anything is wrapped, and `"a\n".Split('\n')` is two pieces. What
does not count it is the **layouter**, which only a cell in several formats hands its whole
string to — `LineCount` splits first and lays each paragraph out on its own, so the empty one
never has to survive a layout at all.
Measured directly, with `ParagraphLayouter` over Liberation Sans at 11 pt in a 200 pt area
(`rowdump.cs break`):

| text | lines the layouter returns |
|---|---:|
| `"a"` | 1 |
| `"a\n"` | **1** |
| `"a\nb"` | 2 |
| `"a\nb\n"` | **2** |
| `"a\n\n"` | 2 |
| `"a\nb\n\n"` | 3 |

So the layouter closes a paragraph on its break and never opens the empty one after it, and it
drops **only the final one** — an interior empty paragraph is a line. `SheetTextLayout.LineCount`
answers 2 for `"a\n"` where `RichLineRanges` answers 1, and `RichPixels` is the only caller of
`RichLineRanges`. That is the whole defect: **a plain cell's trailing empty paragraph was already
a line of its row and a rich one's was not.**

`Reader Instructions` row 5 is a rich cell — 34 formatting portions — which is why it took the
wrong branch. `ContentTableCell.GetOwnText` strips exactly one trailing newline and has done since
the round that recorded why, so the character that says the paragraph is there survives into the
string; nothing upstream needed changing.

The fix is one line in `RichLineRanges`: when the text ends in a hard break, append the empty
range `(text.Length, text.Length)`. No portion covers it, so `RichPixels` measures it at the
cell's own face — which is what EditEngine gives a paragraph holding no portion of its own, and
what that method's own remarks already said it would do.

### 1.2 The result, row for row

| | base | head |
|---|---:|---:|
| rows compared | 82924 | 82924 |
| rows differing from 26.2.4.2 | **15** | **2** |
| total reserved lines, ref − ours | 15 | **2** |

The two that remain are `021 AGK Airframe, Systems...` row 12 (ref 1103.8, ours 835.0) and
`031-Mass-&-Balance-final` row 76 (ref 566.9, ours 298.0). **Neither holds a trailing empty
paragraph** — `trailing-empty.py` lists all 54 such cells in the workbook and neither row is among
them — so they are a genuine wrap or paragraph-count residue, and they are two rows in 82924.

### 1.4 The 534 missing lines do not exist — the instrument counts PyMuPDF's grouping

The gap survives the fix exactly, which is what sent this round to look at it: over pages 1-60 of
the `.xlsx`, PyMuPDF reports **8544 lines for the reference and 8010 for us, at the base and at
head alike**. Round 98 measured 8521 against 7987 on the `.ods` — the same 534.

It is not missing text. Page 11 opens on the same row on both sides and the whole difference is in
one column, x = 139.7, where the reference is grouped into 45 lines and we into 25. Reading the
band y = 100 to 130 out of both:

    ref     y 110.2  x 118.0  '157'
            y 110.2  x 139.7  'State what primary action should be carried out by an '
            y 110.2  x 435.4  '010.05.06.00.02'
            y 110.2  x 517.4  '010.05.06.01.02'
            y 123.7  x 139.7  'intercepted aircraft.'

    ours    y 110.2  x 118.1  '157 State what primary action should be carried out by an '
            y 110.2  x 435.5  '010.05.06.00.02'
            y 110.2  x 517.5  '010.05.06.01.02'
            y 123.7  x 139.7  'intercepted aircraft.'

**Every character is in the same place on both sides**; PyMuPDF merged our `157` and the cell
beside it into one `line` and did not merge the reference's. The count runs the other way on
spans — **10998 for the reference against 11529 for us over the same sixty pages** — so neither
number is a count of drawn lines, and the two disagree in opposite directions.

That is the same trap `dotnet/CLAUDE.md` records for the words track in the other direction
(*"this tree writes a justified line as one text object per stretch and 26.2.4.2 as one per
line"*), arriving on the sheets track. **A `get_text` line count is a statement about two PDF
writers' text-object habits, not about layout**; the reference's own `style:row-height` is, and it
costs one five-second conversion.

### 1.3 What it is worth on the page

Rendered whole at the base and at head under `SOURCE_DATE_EPOCH` and scored against the banked
26.2.4.2 reference with `pdf-image-diff.py`:

| | pages | base sum `\|ink\|%` | head | base MAJOR | head MAJOR |
|---|---:|---:|---:|---:|---:|
| `TK-Syllabus-Comparison-Document-v2.xlsx` | 1235 | **205.57** | **158.42** | 66 | **34** |
| `TK-Syllabus-Comparison-Document-v2.ods` | 1235 | 378.18 | 338.70 | 124 | **95** |

**205.57 and 66 reproduce the seated figure exactly**, which is the control on the instrument: it
is `probes/sheet-ink-r94`'s number, measured again from a fresh render against the same bank.

*The `.ods` twin of the same workbook is nearly twice as far out as the `.xlsx` and has never been
the seated document. It is not this rule — both spellings improve by about the same 40 points —
and it is worth a look on its own.*

## 2. Reach, measured rather than censused

`trailing-census.py` reads the markup of every `.ods` and `.xlsx`/`.xlsm` in the two sheets
corpora and counts cells whose last of two or more paragraphs is empty:

| | documents | cells | of which in several formats |
|---|---:|---:|---:|
| `.ods` (307) | **35** | 889 | 125 |
| `.xlsx`/`.xlsm` (243) | **28** | 372 | 60 |
| | **63** | **1261** | **185** |

*The third column is a proxy read off the markup — an ODF cell carrying a `text:span` anywhere in
it, a SpreadsheetML string built of more than one `r` — and not the reader's own portion count, so
it bounds the reachable set rather than naming it. The sweep below is what settles the reach.*

**A construct count is not a reach figure — N11 again, and this time the ratio is 6 to 1.** All 63
candidates and a control of 61 non-candidates (every eighth of the other 487) were rendered at the
base and at head under `SOURCE_DATE_EPOCH`, one output directory per document, each PDF checked
for `%%EOF` before it was hashed (`sweep.py`, `base.tsv`, `head.tsv`):

| | |
|---|---:|
| renderings compared | 124 |
| **candidates that move** | **10 of 63** |
| **controls that move** | **0 of 61** |
| failures on either leg | 0 |

The ten are `TK-Syllabus-Comparison-Document-v2` and `tk-syllabus-comparison-document-v5` in both
spellings, `State-Medicaid-Payment-Policies-for-Medicare-Cost-Sharing` in both,
`fm-provider-service-measures` in both, `flightstandards-doc-Cross-reference-table_version02.xlsx`
and `PBN Matrix NAAs (V01).xlsx`. The 53 documents that do not move hold the construct only in
cells that are in one format — where the rule was already right — or that are not the tallest in
their row.

**No gate column moves anywhere.** `gate-columns.py` over the ten, ours against the banked
reference (`gate-movers.txt`): page counts **10 of 10 identical** before and after and all equal
to the reference's; alphanumeric characters **10 of 10 identical** to the character; verdicts
**10 of 10 `match`** both times. A row that grows by one empty line adds no glyph and, on these
ten, moves no page — which is the argument for ranking on ink rather than on the scoreboard.

**And the ink, for all ten, each against its own banked reference** (`score.sh`,
`score-base.tsv`, `score-head.tsv`; one output directory per document, `%%EOF`-checked):

| document | pages | base sum `\|ink\|%` | head | base MAJOR | head MAJOR |
|---|---:|---:|---:|---:|---:|
| `TK-Syllabus-Comparison-Document-v2.ods` | 1235 | 378.18 | **338.70** | 124 | **95** |
| `TK-Syllabus-Comparison-Document-v2.xlsx` | 1235 | 205.57 | **158.42** | 66 | **34** |
| `tk-syllabus-comparison-document-v5.ods` | 855 | 46.86 | 46.86 | 27 | 27 |
| `tk-syllabus-comparison-document-v5.xlsx` | 855 | 46.55 | 46.55 | 37 | 37 |
| `fm-provider-service-measures.xlsx` | 38 | 21.67 | 21.67 | 5 | 5 |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 464 | 18.17 | **12.50** | 39 | **35** |
| `fm-provider-service-measures.ods` | 35 | 14.55 | 14.55 | 3 | 3 |
| `State-Medicaid-Payment-Policies…ods` | 18 | 12.73 | **11.08** | 7 | **6** |
| `State-Medicaid-Payment-Policies…xlsx` | 18 | 12.52 | **12.25** | 7 | 7 |
| `PBN Matrix NAAs (V01).xlsx` | 25 | 2.50 | 2.55 | 3 | 3 |
| **the ten** | | **759.30** | **665.13** | **318** | **252** |

**Five improve, one worsens by 0.05, four change their bytes and not their ink to two decimals.**
The one that worsens is `PBN Matrix NAAs (V01).xlsx`, whose ten trailing-empty cells are one rich;
it is round 98's `PC1000` observation again — a taller row is not automatically more ink in the
right place, and 0.05 over 25 pages is at the scorer's own noise. The four that are level are
`tk-syllabus-comparison-document-v5` and `fm-provider-service-measures` in both spellings: a row
grew somewhere that no page boundary and no region of a 512-pixel raster could see.

## 3. O32 — `TOGAF9-Tool-ConfReqts-CSQ.xls` is a merged cell's background, and it is not geometry

Reproduced first, so that the seat's number is this round's: **16.76 summed unsigned ink over 28
pages with 4 MAJOR** (`ink-togaf.txt`), against `probes/biff-reader-r98/ink-before.tsv`'s 16.76 /
28 / 4.

**Two candidates are eliminated before any picture is looked at.**

- *Row heights.* `fodsrows.py` against `rowdump.cs`: **739 of 739 rows agree to within a twip**,
  every row of every sheet, and every one is `opt=false` — a BIFF8 workbook's heights are manual
  (`read.cxx`'s `#if 0` around `AdjustRowHeight`, as `probes/ods-residue-r95` records).
- *Text.* Page 1 holds **13 spans on each side**, at the same x to 0.04 pt, in the same faces at
  the same sizes; page 10 holds 373 against 372 and its fill areas agree to 0.1 %.

**What differs is how wide a fill is drawn.** On page 1, ours are thirty rectangles all
50.40–72.54 and the reference's are three all 50.34–632.21 — one `re` item per path on both sides,
so this is not the coalescing artefact. 632.21 − 50.34 is 581.87, which is that sheet's column A
(22.14 pt) plus column B (559.70 pt) to the hundredth. The page's printed block is **column A
alone**: column B's text is on page 2 in both renderings.

Page 4 says which rows do it. Its first two rows are `table:number-columns-spanned="2"` in
26.2.4.2's own `fods` — a merge across A:B — and they are exactly the two the reference draws to
610.02 where ours stop at the block's 242.19; its third row is unmerged and both sides draw it to
242.19. Page 26 is the same shape: two of its rows run to 614.6 where ours stop at the block's
456.2.

So: **26.2.4.2 paints a merged cell's background across the whole merge and lets the paper clip
it, and this tree paints each covered cell's own rectangle and so stops at the page's last
column.** That is a deliberate choice with its reasoning written down —
`SheetPageDecoration.DrawBackgrounds`' remark says painting per covered cell "survives a block
split across two pages, which one extended rectangle would not" — and on a page whose block ends
well inside the paper it is what costs the ink. The C++ side of it is
`ScOutputData::DrawBackground`'s column loop, which extends a run by `ATTR_MERGE`'s column count
and breaks only at `nCol > mnX2 + 2` (`sc/source/ui/view/output.cxx`:1148-1172 **in this tree**,
which is 27.2.0.0.alpha0+ and not the reference binary's source; the measurement above is the
26.2.4.2 arm and stands on its own).

**What it is worth, page by page rather than in one number.** Three of the four MAJOR pages are
this and nothing else — page 1 at 1.80, page 4 at 1.22 and page 26 at 0.52, each of them one
*"a fill or background shading the reference has and we do not"* region in a band across the top
of the page, **3.54 of the 16.76**. The fourth MAJOR page, 21, is **not** this: its 2.79 is a
single displaced-marks region covering 26.74 % of the page with only 0.38 % of it the fill band,
and it has not been characterised. The remaining 10.43 over 24 pages is diffuse — 30 to 80 small
displaced-glyph regions a page at 0.4 to 0.9 each, with the fill areas and the span counts already
agreeing (page 10: 9792.3 pt² against 9802.3, 373 spans against 372).

**Not fixed**, because the change is to the painter and its reach is every workbook with a merge
crossing a page-column boundary, which this round had no budget to sweep. The seat now names a
cause, a file and a page-by-page share.

## 4. O33 — the posture is nil and the colour is 119 spans in two documents

`SheetShapeRun` carries a face, a size and a weight. The census asks what the corpus's `TXO`
formatting runs would add to it, and it is asked twice.

**From the records** (`txo-colour-census.py`, `txo-colour.tsv`, `txo-colour2.txt`). Each `TXO`'s
run array is paired with the `ftCmo` object type of the `OBJ` before it, so that a cell comment —
whose text never goes through the shape path — is separated out, and the terminator run, which
names a character index past the string, is excluded:

| | boxes | applied runs | stating italic | stating a non-black colour |
|---|---:|---:|---:|---:|
| text boxes and buttons (types 6, 7) | 108 | **263** | **0** | 43 |
| cell comments (type 25) | 47 | 104 | **0** | — |

**0 of 367.** No `TXO` run in any of the 64 `.xls` states an italic, notes included. The colour is
resolved through the workbook's own `PALETTE` where it states one and the BIFF8 defaults
otherwise, with `0x7FFF` read as automatic and BIFF's missing font index 4 accounted for.

**From the reference** (`shape-span-colours.py`). 26.2.4.2's own `--convert-to fods` of all nine
workbooks whose `TXO` sit on a shape-path object, every span of every `draw:` element resolved
through its style chain:

| | non-black spans | italic spans |
|---|---:|---:|
| `EHEST-Pre-departure-checklist` | **113** | 0 |
| `PC1000` | **6** | 0 |
| the other seven | 0 | 0 |

So the two legs agree on the posture and are the reason to trust it: **the posture half of O33 has
nil reach on this corpus and can be filed.** They disagree on the *size* of the colour half — 43
runs against 119 spans — and the reference's figure is the one to use, because the record-side
parse assumes one `CONTINUE` of characters before the run array and `EHEST`'s instruction boxes
are long enough to need several. The two documents are the same two either way.

`SheetShapePainter` draws every shape run under `Paint.Solid(Colour.Black)` (`:155`), so those 119
spans are painted black and the reference paints them `#00ff00`, `#ffcc00` and `#ff0000`. Both
documents are already in round 98's mover list, so the colour would land on renderings that have
already moved. **Left seated**, with the posture half closed: extending the record reaches the
painter and all three shape-text readers, and 119 spans in 2 of 64 documents did not justify that
inside a round whose main seat was elsewhere.

*The colour census also confirms one thing about `TOGAF9-Tool-ConfReqts-CSQ`: its single
colour-stating run is not drawn in colour by 26.2.4.2, whose resolved view of that workbook holds
**0** non-black shape spans. O32 is not O33.*

## 5. O17 — not reached

`alle einzeln.xlsx` was not worked. Its seat stands as `probes/o17-pivot-r97` left it. The one
thing this round adds to it for free is the instrument: `rowdump.cs rows` against `fodsrows.py`
answers the grid-geometry half of that seat's suspicion in two commands and no rendering, exactly
as it did for `TK-Syllabus` and `TOGAF9` here.

## 6. Tests

Ten non-fidelity projects, run individually and totalled here rather than through the solution:
Core **521**, Containers **109**, Markup **259**, Text **728**, OpenDocument **146**,
Presentations **1045**, Rendering **164**, Vector **309**, WordProcessing **1924**,
Spreadsheets **1286** — 0 failed and 0 skipped in every one. Solution build 0 warnings, 0 errors.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552 discovered**, and the ten are exactly
the known names: `PageDrawingComparisonTests` x4 (`paginated.docx`/`.fodt`/`.rtf`/`.doc`),
`TabStopComparisonTests` x4 (`list-label-overrun.fodt`/`.docx`/`.odt`/`.doc`),
`SheetDrawingComparisonTests` (`sheet-rich-text.xlsx`) and `JustificationShrinkComparisonTests`
(`justify-shrink-2013.docx`). Every count above was read out of this round's own output rather
than carried in from a brief; of Spreadsheets' 1286, six are the new class below, read from a
filtered run of it.

`tests/corpus/features/sheet-wrap-trailing-paragraph.fods` and
`SheetTrailingParagraphHeightTests` are the new coverage: five rows in an 8 cm wrapping Calibri 11
column, two of them the same characters as two others with one empty paragraph added at the end.
The expected heights were read out of 26.2.4.2's own flat-ODF export of the file before anything
was asserted — 0.3937 in against 0.5799 in, twice — and the fifth row is the single-line control
that keeps the fixture off the 256-twip floor.

**Two of its six assertions fail at the base and four pass**, and which four is the point:
reverting `SheetTextLayout.cs` to `1acd84bea` and rebuilding gives `Failed: 2, Passed: 4`, the two
being the rich trailing-empty row and the step over it. The *plain* trailing-empty row passes at
the base, which is the control that separates this rule from the one `LineCount` already had.

## 7. What this round could not settle

- **Two rows of `TK-Syllabus` are still a line short** — `021 AGK Airframe, Systems...` row 12 and
  `031-Mass-&-Balance-final` row 76 — and neither holds a trailing empty paragraph. Two rows in
  82924 is not a seat; recorded so that the next reader of `WrappedHeight` knows the residue is
  two rows and not a distribution.
- **`TK-Syllabus`' `.ods` spelling is 338.70 where its `.xlsx` is 158.42**, on identical page
  counts and within 104 characters of each other. Something in the ODF path costs that workbook
  180 points of ink that the SpreadsheetML path does not, and this round did not look for it.
- **O32 is characterised and not fixed**, §3. The change is one rectangle's width in
  `SheetPageDecoration.DrawBackgrounds` and its reach is every workbook with a merge crossing a
  page-column boundary; it needs its own sweep.
- **O33's colour half is measured and not implemented**, §4. 119 spans in 2 of the 64 `.xls`.
- **`Wrap` is deliberately left as it is.** The drawing path drops the same trailing empty
  paragraph and the reference draws no glyph for it either, so nothing visible moves — but a
  middle- or bottom-anchored cell would place its text differently for it, and no corpus witness
  was found to measure that against.

## 8. Files

| file | what it is |
|---|---|
| `rowdump.cs` | the round's first instrument: our resolved row heights, cell texts, portion counts and cell fills for one sheet, and the layouter's line count for a probe string. Built as a scratch console project against `src/XRay.Office/XRay.Office.csproj`, named `XRay.Office.Spreadsheets.Tests` so that the internals are visible |
| `fodsrows.py` | round 98's, unchanged: the reference's resolved row heights out of one `--convert-to fods` |
| `rowdiff.py` | the join — per-row heights, the line-count delta against the document's own quantum, and a histogram |
| `tk-base.tsv`, `tk-head.tsv` | the fifteen disagreeing rows at the base and the two at head |
| `trailing-empty.py` | every cell of a flat ODF whose last of several paragraphs is empty |
| `trailing-census.py`, `trailing-ods.tsv`, `trailing-xlsx.tsv` | the same census over the two sheets corpora, reading the markup directly |
| `sweep.py`, `cand.list`, `control.list`, `base.tsv`, `head.tsv` | the reach sweep: 63 candidates and 61 controls at both binaries, hashed |
| `render-keep.sh`, `movers.list`, `score.sh` | the ten movers rendered and scored on ink at both binaries |
| `score-base.tsv`, `score-head.tsv`, `gate-movers.txt` | their ink and their gate columns |
| `ink-togaf.txt` | `TOGAF9-Tool-ConfReqts-CSQ` per page against the banked reference |
| `fills.py`, `spans.py` | one page's filled rectangles and text spans, ours beside the reference's — the two instruments §3 is built on |
| `txo-colour-census.py`, `txo-colour.tsv`, `txo-colour.txt`, `txo-colour2.txt` | O33's record-side census, with the object type behind each `TXO` |
| `shape-span-colours.py` | O33's reference-side census: every drawing shape's spans with the colour and slant 26.2.4.2 resolved |
