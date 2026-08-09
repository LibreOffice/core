# Sheets round thirty — probe data

Base commit `946b3defc`, verified before measuring. Reference renderer LibreOffice 24.2.7.2. Both
whole-track sweeps ran against a checksummed copy of the CLI, and the copy's `Paperless*.dll`
hashes were diffed against the tree's before each run started.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `946b3defc`, before any change |
| `after-margin-whole-track.tsv` | the same on the final tree |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |
| `reach.sh` | renders a track with two CLIs under a pinned clock and diffs the bytes |
| `mutate.sh` | the reintroduced bugs the round's tests were verified against |

Both sweeps: 171 rows, no duplicate path, zero `ref-failed` — and `ECA Sinters.xls`, which the last
three rounds each had to re-run alone, came back on the first pass both times.

## The baseline reproduces the brief

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | **146/171** | **90** | **154** | 42859 |
| `after-margin-whole-track.tsv` | **147/171** | 90 | 154 | **42322** |

Per batch at the baseline: 001–009 all pass, then 010 8/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4 — the brief's line to the digit, and round
twenty-seven's closing figure.

The briefed word error was 42848 and this scores 42859. The eleven are one document:
`Keywords_Mapping_Graphs_and_Charts.xlsx` reads 4635 here against the 4647 round twenty-six
recorded and the 4695 round twenty-seven's prose quoted. Its verdict (`words`) is the same under
all three. Nothing follows from it beyond the standing rule that the mine-against-mine comparison
is the one to read.

## An `.xls` cell's margin is 40 twips and every other format's is 20

`ATTR_MARGIN` is a cell attribute whose pool default is `SvxMarginItem(20, 20, 20, 20)`
(`svx/source/items/algitem.cxx:123-132`, installed at `docpool.cxx:145`). `XclImpXF::CreatePattern`
ends by overriding it on **every pattern the BIFF filter builds**:

```cpp
// Excel's cell margins are different from Calc's default margins.
SvxMarginItem aItem(40, 40, 40, 40, ATTR_MARGIN);
ScfTools::PutItem(rItemSet, aItem, bSkipPoolDefs);
                          sc/source/filter/excel/xistyle.cxx:1349-1351
```

It is unconditional — cell XFs and style XFs alike — and it is **the only line in all of
`sc/source/filter` that touches the item**, so SpreadsheetML, XLSB and ODF keep the 20. `Paperless`
had it as one constant, `SheetTextLayout.CellMargin`, shared by all three readers.

### It was being read as a page-origin offset, and it is not one

The difference is visible in every `.xls` comparison this project has ever made, and it is written
down in `pdf-ops.py`'s own tolerance comment: *"measured on one workbook, every show sits about
1 pt apart (51.39 against 52.38, 743.75 against 742.73) because the two put their page origin in
slightly different places"*. Those numbers are
`underlying-holdings-…-state-street-emu-esg-screened-index-equity-fund.xls`, and the reading is
wrong. **A page-origin offset moves every run the same way; a margin moves left-aligned text right
and right-aligned text left.** On that document's page 6 the reference draws

| | left-aligned `Fx` | right-aligned `31895` |
|---|---|---|
| ours, before | 51.392 | 121.032 … 148.904 |
| the reference | 52.384 | 120.076 … 147.901 |

— the reference 0.99 pt to the *right* of ours on the one and 0.96 pt to the *left* on the other,
which no origin can do and 20 twips on each side does exactly.

The fixture triple settles it without the corpus. `sheet-cell-text` exists as `.xls`, `.xlsx` and
`.fods` holding the same cells, and LibreOffice's own PDFs of the three put the left-aligned `Lft`
of A1 at **58.68 pt** in the `.xls` and **57.69 pt** in the other two, with the two baselines the
same 0.99 pt apart in the other direction because the cells are bottom-aligned. Ours now reproduces
all three to within 0.05 pt.

### The half that moves a number: how much of a clipped string survives

The same `nTotalMargin` reaches `ScOutputData::GetOutputArea`, so the room a cell has for its text
is the column less *both* margins — and that room is the numerator of the ratio the shortening
uses (`fVisibleRatio * nTextLen + 1`, `output2.cxx:2216-2227`). Doubling the margins takes about
four per cent off the ratio, which on a cell near a character boundary is one glyph.

Measured on the document above, whose columns are 49.75 pt and whose second column holds the same
49-character string on every row: we kept `State Stree` and the reference keeps `State Stre`, on
every row of every one of its first five pages. Its extractable word count was **4743 against
4991** and is now **4988**, which is the match this round wins.

Worth stating because it is the trap the skill warns about: the visible/total ratio implied by the
reference's glyph counts is 0.948–0.962 of ours across nine independent cells of one page, a single
scalar. Two hypotheses fit a scalar — a 4% wider text measurement or a 4% narrower cell — and the
*sign of the right-aligned* runs is the observation that separates them.

### Reach

The gate registers a difference on **35 of the 171 rows**: 34 `.xls` and one `.xlsx`, and the
`.xlsx` is `PBN Matrix NAAs (V01).xlsx`, whose *reference* word count moved by one — the
non-determinism this file has recorded twice before.

A byte-level reach run was started and **stopped after 35 documents** because three agents on four
cores had the load average above 17; the partial is unambiguous and is quoted as a partial:
**15 of 15 `.xls` differ byte for byte and 20 of 20 `.xlsx` are identical.** So the reach is the
track's 62 `.xls` documents and nothing else, of which about half move far enough for the gate to
see it.

### What did not move

**No page count, anywhere.** Page error 90 and 154 exact counts on both sides, and batches 001–009
stay 89/89. That is the outcome to be suspicious of and it has an explanation: `RowHeightsAreManual`
is set outright for BIFF8 (`XlsWorkbookReader`, from `XclImpColRowSettings::Convert`), so no `.xls`
row height is recomputed and the margin cannot reach the one quantity that decides pagination. The
margin's other three consumers — placement, clipping and the print-area extension — move ink and
words and not paper.

Absolute word error 42859 → **42322**. Four documents move by twenty words or more, three of them
towards the reference:

| document | before | after |
|---|---|---|
| `underlying-holdings-…-equity-fund.xls` | 4743/4991, `words` | **4988/4991, `match`** |
| `laufende-nip-vorhaben-hyland.xls` | 6514/6579 | **6579/6579** |
| `fy2011-aip-grants.xls` | 54933/54488 | 54755/54488 |
| `fy2010-aip-grants.xls` | 63775/63452 | 63113/63452 |

The last is the honest cost: it was 323 words over and is now 339 under, so its absolute error is
16 worse. It matched before and matches still.

`batch-016` goes **4/9 → 5/9** and the track **146 → 147**.

## Tests

`SheetCellMarginTests`, 7 cases on the `sheet-cell-text` fixture triple. Every asserted coordinate
is LibreOffice 24.2.7.2's own, read off its PDFs of the three files.

| mutation | cases that fail |
|---|---|
| the BIFF filter's 40 twips reduced to the pool's 20 | 3 |
| the placement reads the constant instead of the cell's margin | 2 |
| the BIFF reader never states a margin on its formats | 3 |
| the sheet's fallback format keeps the pool's margin | 1 |
| the row-height arithmetic pins the margin at Excel's 40 | 7 |
| the wrapped-cell margin pixels pinned at Excel's two | 4 |
| the wrapping paper's margin pinned at the pool default | **0** |
| the print-area extension reads the pool default | **0** |

The two rows at zero are reported as they came out rather than argued away, and they are not the
same kind of gap.

**The wrapping paper's margin cannot be discriminated by anything in the tree, and the reason is
structural.** `RowHeightsAreManual` is set outright for BIFF8, so no `.xls` row height is ever
recomputed; every document that *does* reach the recomputation is a format whose margin is the
pool's 20, which is exactly what the mutation pins it to. It is an equivalent formulation on
everything reachable rather than an untested behaviour — which is also why the two row-height
mutations above it *are* detected: pinning them at **Excel's** value changes what a `.fods` and an
`.xlsx` compute, and seven and four existing cases say so. The three sites are kept as a faithful
read of `lcl_GetAttribHeight` rather than reverted to a constant that would be wrong for the BIFF5
case the class already models.

**The print-area extension is genuinely reachable and simply not discriminated.** A 2 pt change in
the width a string needs only matters when it crosses a whole column boundary, and no fixture puts
a string that close to one. Unmeasured on the corpus: the whole-track sweep ran with the correct
code and no run was made with the constant, so whether any of the 62 `.xls` documents changes a
page over it is an open question rather than a settled no.


## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere:

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations | Fidelity |
|---|---|---|---|---|---|---|---|---|---|---|
| 264 | 109 | 240 | 291 | 119 | 259 | 125 | 690 | **573** | 538 | 550 |

Every count is the briefed known-good except Spreadsheets, which is 566 plus this round's 7.

`Paperless.Fidelity.Tests` took an hour and three quarters to return, at a load average between 15
and 22 with three agents on four cores. It was written down as unrun rather than waited on, and then
it finished: 550 of 550, none skipped. The note is left here because the reasoning that produced it
stands — **a fidelity count taken under that load is exactly the one this project's own notes say
can be silently truncated**, so the number is only worth having because it is the discovered count
and matches the expected one. Had it come back at 353 the correct reading would have been "the run
was truncated", not "the suite regressed".
