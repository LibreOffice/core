# Sheets round twenty-seven — probe data

Base commit `e4b9cf255`, verified before measuring. Reference renderer LibreOffice 24.2.7.2.
Both whole-track sweeps ran against a checksummed copy of the CLI, and the copy's
`Paperless*.dll` hashes were diffed against the tree's before each run started.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `e4b9cf255`, before any change |
| `after-whole-track.tsv` | the same on the final tree |
| `reach-font.tsv` | all 171 rendered by both CLIs across the `CHFONT` change, bytes compared |
| `reach-colour.tsv` | the same across the fill change |
| `census.py`, `chart-font-census.txt` | which BIFF chart substreams state a font, by a real record parse |
| `fmt.py`, `chart-format-census.txt` | the same for `CHAREAFORMAT`/`CHLINEFORMAT` |
| `biffchart.py` | the per-document dumper the two censuses grew out of |
| `score.py` | turns a `rows.tsv` into matches, page error, exact counts, word error |
| `reach.sh` | renders a track with two CLIs under a pinned clock and diffs the bytes |
| `mutate-chartfont.sh`, `mutate-chartformat.sh` | the reintroduced bugs each test was verified against, and their output |

Both sweeps: 171 rows, no duplicate path. `SOURCE_DATE_EPOCH` is pinned in every reach run so
the seventeen documents that print a date in a header cannot contribute.

## The baseline reproduces the brief, and the brief's word error does not reproduce itself

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | **145/171** | **90** | **154** | 43203 |
| `after-whole-track.tsv` | **146/171** | 91 | 154 | **42848** |

Per batch at the baseline: 001–009 all pass, then 010 7/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4 — round twenty-six's closing line to the digit.
`diff` against round twenty-six's committed after-sweep is **one row**, and that row's difference
is on the *reference* side.

The briefed word error was 43198. `score.py` run over round twenty-six's own committed
`after-chartface-whole-track.tsv` gives **43202**, so the briefed figure is four below what its
own probe data scores under this arithmetic. Nothing follows from it except that the
mine-against-mine comparison is the one to read; the difference is in the scoring, not the
measurement.

One baseline row needed re-running: `ECA Sinters.xls` came back `ref-failed` because an `soffice`
wedged well past its own `timeout 240` with a defunct child the wrapper could not reap. Killed and
re-run alone it matches at 163/163 pages, and the spliced row is what the totals above count.
**Third round running for this same document**, which makes it a property of the document under
load rather than a change in what we draw.

## `XlsChartReader` set no font at all

`CHFONT` (0x1026) holds nothing but an index into the workbook's `FONT` buffer
(`XclImpChFont::ReadChFont`, `sc/source/filter/excel/xichart.cxx:941`), so the resolution goes back
through `XlsCellFormats`. Which text an index dresses is decided entirely by where the record sits:
`XclImpChChart::GetDefaultText` (`xichart.cxx:3956-3969`) hands the chart's **global** default text
to the title and the legend in every generation, and to the axis labels too in BIFF5; only BIFF8
splits those onto the axes-set default. The order implemented is that one, then the first `CHFONT`
the substream states.

**A byte-level census does not work and a record-level one does.** A `BOF` signature search reports
zero on a workbook that plainly draws charts, because a chart substream's `BOF` is an ordinary
record inside the OLE2 `Workbook` stream. `census.py` walks OLE2 and then the record stream:

- **6 of the 61 OLE2 workbooks on the track hold a chart substream**, 15 substreams between them.
- Every one of the 15 states a `CHFONT`, and **every one states a single family throughout** —
  global default, axes-set default and first-stated all agree, in all 15.
- Four of the six state **Arial**, two state **Calibri**.

That last line is the whole prediction, and it held: **Arial resolves through fontconfig to
Liberation Sans, which is exactly the default the unwired consumer already used**, so only the two
Calibri workbooks can move.

### Reach, measured by rendering: 2 of 171, and they are the two the census named

`reach-font.tsv` — all 171 rendered by both CLIs with the clock pinned, bytes compared:
**2 differ, 169 byte-identical**, and they are `EHEST-Pre-departure-checklist…xls` and
`TOGAF9-Tool-ConfReqts-CSQ.xls`. `EHEST` embedded no Carlito at all where the reference embeds two;
it now embeds Carlito-Regular, taking its embedded-font count from 4/6 to 5/6.

**No fixture in the tests states Arial**, for the same reason the census matters: an Arial fixture
passes whether the family is read or not.

## The BIFF chart path drew no fills whatever

Measured on `EHEST` page 8 with `pdf-ops.py`: **0 fill operators against the reference's 7**, while
the OOXML chart path on `Keywords_Mapping_Graphs_and_Charts.xlsx` page 19 emits 20 against 21.
`ChartPlot` already carried `Background`, `PlotBackground` and each series' `Fill` and `Line`; only
the reader was missing, so a bar was stroked and never filled.

`fmt.py` censuses what the six chart-bearing workbooks actually state:

| | records | automatic |
|---|---:|---:|
| `CHAREAFORMAT` | 114 | **0** |
| `CHLINEFORMAT` | 155 | 6, all in one document |

**Not one automatic area on the track.** An automatic area takes its colour from an Excel chart
palette this reader does not have, and the census says nothing here needs it — so it is not
implemented, and a chart stating `AUTO` keeps the layout's own default exactly as before. That is a
limit rather than a rule and the test that pins it says so.

### `CHESCHERFORMAT` overrides `CHAREAFORMAT`, and reading only the palette looks right

The C++ carries it as a comment beside `XclImpChFrameBase::ConvertAreaBase`: *"CHESCHERFORMAT
overrides CHAREAFORMAT (even if it is auto)"*. All nine of `EHEST`'s chart substreams state their
three filled series at palette indices 24, 10 and 13 — `#9999FF`, `#FF0000`, `#FFFF00` — and the
reference draws `#6699FF`, `#FF0000`, `#FFFF00`. **Two of the three agree by palette and the first
does not**, because that series and the plot wall each carry a `CHESCHERFORMAT` as well, stating
`0x02FF9966` and `0x02F8F8F8`. With both read, page 8's chart background, plot wall and first
series all match the reference exactly.

The record's payload is a bare DFF `msofbtOPT`, which `EscherPropertyTable.Read` already parses for
every shape in a `.xls`, a `.doc` and a `.ppt`. 20 such records over 3 of the 6 documents.

**A near miss worth recording.** The first version rejected any colour word with a non-zero top
byte, on the reasoning that a kind flag means an index into a table the substream does not carry.
Every one of these words has top byte `0x02` — a literal Excel merely says came from its palette —
so the guard rejected the whole population and the render did not move at all. Only the
palette-index, scheme-index and system-index bits are now rejected.

### Reach: 4 of 171, against a census ceiling of 6, and the two-document gap is explained

`reach-colour.tsv`: **4 differ, 167 byte-identical** — `EHEST`, `TOGAF9`, `orbus_togaf_tool_csq.xls`
and `Template Pilot Logbook JAR-FCL V3.0.xls`. The two that did not move are the
`2012-GA-Survey-Chapter-5/6` pair, and the reason is not that their charts state nothing: **neither
renderer draws a single fill anywhere in either document**, because their charts sit on sheets the
printout does not reach. Both match at 3/3 and 5/5 pages.

## What moved on the gate

`batch-010` goes **7/10 → 8/10**, and the track **145 → 146**.

| document | before | after |
|---|---|---|
| `EHEST-Pre-departure-checklist…xls` | 24/24 pages, 8018/8382 words, 4/6 fonts, `words` | 24/24, **8373/8382**, 5/6, **`match`** |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 28/28, 24217/24097, `match` | 28/28, 24215/24097, `match` |

Absolute word error 43203 → **42848**.

**Page error goes 90 → 91 and that is the reference moving, not us.** The one row is
`ans_mappings_of_eccairs_terms.xlsx`, whose *our* column is 192 pages and 28195 words in both
sweeps while the reference's is 191/28183 in one and 190/28181 in the other. It is an `.xlsx` with
no BIFF chart, and both reach runs report it byte-identical across every CLI here.

## A stale binary that looked exactly like a stale snapshot

The seam check — rendering the track with the sweep's snapshot and with the tree — came back with
**three documents differing**, which reads as "the sweep measured something other than what is
committed" and is the failure the skill warns about. It was the other way round.

The snapshot is right and the *tree's* build was wrong, and the reference settles it in one line:
on `TOGAF9` page 21 the reference fills `(52.84, 318.36)-(613.70, 567.38)`, the snapshot fills
`(52.84, 319.16)-(613.56, 567.73)`, and the tree fills `(83.80, 343.43)-(432.11, 524.19)`.

The cause is the mutation cycle, one step beyond the trap already recorded. Reintroducing a bug and
restoring the file with `mv backup source` **preserves the backup's older modification time**, so
MSBuild's up-to-date check sees a source older than the assembly beside it and skips the rebuild —
and a plain `dotnet build` then reports success while the binary still contains the defect. The
recorded trap is that `--no-build` measures the defect; this is that `dotnet build` does too. A
`touch` on the file and a rebuild made the tree byte-identical to the snapshot on that document.

Both mutation scripts here use `cp`/`mv` in exactly that shape, so **rebuild with a `touch` before
measuring anything after running one.**

## Tests

`XlsChartFontTests` 9 cases, `XlsChartFormatTests` 12, on `BiffChartFixture` — a synthetic BIFF8
workbook writer both share. Synthetic because the properties are decided by *where* a record sits,
and only a file written for the purpose can put the same record in two places; the corpus's own
chart workbooks each state one answer throughout.

Every reintroduced bug fails at least one case. There is no drift guard in either file.

| mutation | cases that fail |
|---|---|
| `CHFONT` never read | 8 |
| the `FONT` buffer not handed to `Build` | 8 |
| the first `CHFONT` wins, no default-text order | 3 |
| the axes-set default outranks the global one | 1 |
| any `CHFONT` taken as the open default text | 1 |
| a `CHDEFAULTTEXT` id kept until a `CHTEXT` arrives | 1 |
| the `FONT` buffer indexed by record ordinal | 1 |
| `CHAREAFORMAT` never read | 6 |
| `CHLINEFORMAT` never read | 1 |
| `CHESCHERFORMAT` never read | 2 |
| the Escher format does not supersede | 1 |
| the `fFilled` boolean ignored | 1 |
| an MSO scheme word read as a literal | 1 |
| the `AUTO` flag ignored | 1 |
| `EXC_PATT_NONE` filled anyway | 1 |
| any frame taken as the background or wall | 1 |
| the axes-set frame taken as the chart background | 6 |
| one fill for every series | 1 |
| a series fill filed as the chart background | 2 |

**Two mutations failed nothing on the first pass and the fix was to remove code, not to add a
case.** The innermost-container test and clearing the open default-text identifier when its
`CHTEXT` closes were each the other's backstop. The clear is unreachable — any `CHFONT` with a
`CHTEXT` innermost had that `CHTEXT`'s own header assign the identifier on the way in — so it was
deleted rather than left as an untested comfort, and a fixture with an empty global default text
plus a font on the axis now discriminates the test that does the work.

The two colour cases asserting about `ChartPlot.Series` also failed at first, for a reason worth
keeping: **a series whose values do not resolve is dropped rather than drawn empty**, so it could
carry no fill either. The fixture grew a worksheet, a `SUPBOOK` and an `EXTERNSHEET` so a `tRef3d`
reaches a real number.

## Left alone deliberately

- **`Template Pilot Logbook JAR-FCL V3.0.xls`** — the brief says not to try to pass the word gate on
  it and nothing here tried. Its rendering did change (it is one of the four the fill change
  reaches) and its verdict did not.
- **Automatic area and line colours.** Not implemented, because Excel's automatic chart palette is a
  table this reader does not have and the census finds zero automatic areas and six automatic lines
  on the whole track — all six in the document above.
- **A chart's text weight.** The reference embeds Carlito-**Bold** on `EHEST` and we do not: its
  chart titles are 18 pt bold and `ChartPlot` carries a family and no weight. Widening it is a
  `Paperless.Core/Charts` change and reaches all three consumers, so it is not this track's alone.
- **`CHDATERANGE`.** Untouched; the one document that needs a date axis is the one above.

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere:

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations | Fidelity |
|---|---|---|---|---|---|---|---|---|---|---|
| 249 | 109 | 240 | 291 | 119 | 259 | 125 | 683 | **566** | 529 | 550 |

Every count is the briefed known-good except Spreadsheets, which is 545 plus this round's 21.
