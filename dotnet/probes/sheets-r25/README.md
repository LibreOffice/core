# Sheets round twenty-five — probe data

Base commit `4448c2a3f`. Reference renderer LibreOffice 24.2.7.2. Every sweep here ran against a
checksummed copy of the CLI, so a rebuild during a sweep could not disturb it; the `measuring …`
line on each run named the copy, and the copy's `Paperless*.dll` hashes were diffed against the
tree's before the run started.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `4448c2a3f`, before any change |
| `after-drawing-continue-whole-track.tsv` | the same after this round's reader change |
| `reach-base-vs-after.tsv` | every document rendered with both CLIs and the bytes compared |
| `reproducibility-back-to-back.tsv` | every document rendered twice in succession, same binary |
| `reproducibility-pinned.tsv` | every document rendered twice under two time zones, `SOURCE_DATE_EPOCH` set, **no masking at all** |
| `clock-dependence.tsv` | the same two time zones with the variable unset and timestamps masked |

All six: 171 rows, no duplicate path.

## The baseline, and it reproduces exactly

| | matches | abs page error | exact page counts | abs word error |
|---|---|---|---|---|
| `base-whole-track.tsv` | 144/171 | 94 | 153 | 43694 |
| `after-drawing-continue-whole-track.tsv` | **145/171** | **90** | **154** | 43694 |

Per batch at the baseline: 001–009 all pass, then 010 6/10, 011 6/10, 012 8/10, 013 8/10,
014 9/10, 015 5/9, 016 4/9, 017 6/10, 018 3/4. That is round twenty-four's closing figure to the
digit, per batch as well as in total. After the change only `batch-010` moves, 6/10 → **7/10**;
every other batch is unchanged, and 001–009 are still 89/89 with page error 0.

One baseline row needed re-running. `ECA Sinters.xls` came back `ref-failed` because an `soffice`
wedged past its own `timeout 240` — the child went defunct and the wrapper waited 44 minutes — on
a machine carrying three agents at a load average above 20. Killed and re-run alone it matches at
163/163 pages and 38356/38651 words, and the spliced row is what the totals above count. **A
`ref-failed` on a document that rendered before is an infrastructure question**, and this is the
second mechanism for it after a full disk: a hung converter that its own timeout does not reap.

## `INDEX_Digital_Transformation_Toolkits.xls`: a bare `CONTINUE` is Escher

The brief carried a hypothesis — a printed range not reaching the drawings anchored below it,
`SheetDrawingArea` rather than the Escher reader. **It is neither.** Walking the file's record
stream settles it in one read:

```
sheet 12:  MSODRAWING x25, 8034 bytes    CONTINUE x70    OBJ x95
           MSODRAWING:228 OBJ:38  (x1)
           MSODRAWING:324 OBJ:38  (x9)
           MSODRAWING:326 OBJ:38  (x15)
           CONTINUE:326   OBJ:38  (x70)
```

Excel writes one `MSODRAWING` per shape with that shape's `OBJ` after it, and **stops writing
`MSODRAWING` once the sheet's Escher stream passes the 8224-byte record ceiling** — 25 records,
8034 bytes — writing the remaining 70 shapes as `CONTINUE` in the identical interleaving.
`BiffRecordReader` absorbs continuations into the record before them so that a caller never sees
a boundary, and the record before each of these is an `OBJ`. So 70 shapes were read as an object
record's payload and lost, which is exactly 25 of 95 pictures drawn and 20 pages against 24.

The predecessor's own measurement said this and the sentence beside it read the other way: *"a
shape walk that stopped after 25 shapes would not put the first 15 in exactly the right places
and then thin the sixteenth onwards."* A walk truncated at 25 does precisely that — the first 25
are placed exactly, and the reference's pages that hold only shapes 26 onwards do not exist for
us at all. The table in `sheets-r24/README.md` is a correct measurement of a truncation, read as
a refutation of one.

LibreOffice reaches the same answer structurally: `XclImpDrawing::ReadMsoDrawing`
(`sc/source/filter/excel/xiescher.cxx:4021`) turns its stream's own continuation handling off with
`ResetRecord(false)` and loops over exactly `MSODRAWING`, `MSODRAWINGSEL`, `CONTINUE`, `OBJ` and
`TXO`, stopping at the first record that is none of them. `InDrawingBlock` in `XlsWorkbookReader`
is that loop's set; `BiffRecordReader` no longer joins a `CONTINUE` to an `OBJ`, which is what
LibreOffice's `ReadObj` also never sees.

### Reach, measured by rendering — and a census that overstates it eighteenfold

`reach-base-vs-after.tsv`: all 171 rendered with both CLIs under one fixed zone within minutes of
each other, `/CreationDate`, `/ModDate` and `/ID` masked. **1 of 171 changes what is drawn**, and
it is the document above. The other 170 are byte-identical.

The census says 18. Scanning all 61 OLE2 workbooks on the track for a `CONTINUE` inside a drawing
block finds one in **18** of them, from 2 records to 190. That is the ceiling in the sense the
skill means, and the ratio is worth keeping: **18 documents state the condition and one changes a
pixel.** The others' extra shapes are ones we would not draw anyway — a shape with no picture, no
text and no chart contributes nothing — or sit in chart substreams that were already assembled.

## Our PDF writer is reproducible. The clock is what moves.

The brief asked for the source of a non-reproducible PDF writer to be found and fixed, and named
`Paperless.Rendering`. **It is not in `Paperless.Rendering`, and the writer is not the problem.**

- `reproducibility-back-to-back.tsv` — every one of the 171 rendered twice in succession by the
  same binary, `/CreationDate`, `/ModDate` and `/ID` masked: **171 same, 0 different.**
- `clock-dependence.tsv` — the same 171 rendered under two time zones a day apart, timestamps
  still masked: **17 differ.** Those 17 draw a date on the page.
- `reproducibility-pinned.tsv` — the same two time zones with `SOURCE_DATE_EPOCH` set, and **no
  masking whatsoever**: **171 same, 0 different.**

The mechanism is `SheetHeaderContext.Printed`, whose default was `DateTime.Now`, read afresh
inside *every page's* header context. `&D` prints it as a date and `&T` as a time, so a workbook
carrying either renders different bytes as soon as the clock passes the field's own resolution.
Confirmed directly rather than inferred: rendering `PBN Matrix NAAs (V01).xlsx` under `TZ` +14 and
`TZ` −12 puts `08/09/2026` and `08/08/2026` in its header, and the header is `&R&D`.

This is correct behaviour — LibreOffice prints the current date there too — and it is also a
**17-in-171 false-positive floor under every byte-level reach measurement the last three rounds
made**. So the instant is now taken once per printout, stamped onto the pages beside `PageCount`
as `ScHeaderFieldData` holds it, and `SheetPrintInstant` honours `SOURCE_DATE_EPOCH`;
`RenderCommand` passes the same value to `PdfRenderOptions.CreationDate`, a hook that already
existed and whose doc comment already said it was for exactly this.

**Read as UTC, not converted to local time**, and that was wrong in the first version. With the
conversion in place all 13 documents of a first sample differed under two zones; the whole point
of pinning is that the output stops depending on the environment. The test only caught it once it
set `TZ` as well as the variable.

`SOURCE_DATE_EPOCH` unset — every ordinary run, including both whole-track sweeps here — reads the
wall clock exactly as before. `reach-base-vs-after.tsv` and both sweeps confirm it: 170 of 171
byte-identical between the base CLI and this one.

### What was touched, and what deliberately was not

`Paperless.Spreadsheets` and one method in `Paperless.Cli`. **`Paperless.Rendering` was not
touched at all**, and neither was `Paperless.Core`. `BiffRecordReader` lives in
`Paperless.Spreadsheets.MsBinary` and is not shared with the DOC or PPT readers, so the drawing
change cannot reach the other two tracks either.

## Tests

`XlsDrawingContinuationTests`, 2 cases. Synthetic BIFF8 because the case needs more shapes on one
sheet than fit in 8224 bytes of Escher — about 25 pictures — and the corpus document that has them
is a binary file a unit test cannot carry.

| mutation | cases that fail |
|---|---|
| `OBJ` absorbs its trailing `CONTINUE` again | 1 |
| a `CONTINUE` closes the drawing block | 1 |
| the drawing block never opens | 1 |

The second case is the control: the identical Escher bytes cut across two `MSODRAWING` records
instead. It passes under all three mutations, which is what makes it a control — **it is a drift
guard and is labelled as one.**

`SheetPrintInstantTests`, 13 cases. Two fail when `Now()` stops reading the variable, two when the
range guard on the epoch is dropped, one when the pinned instant is converted to local time.
**No case catches the instant being read per page rather than per job**, and none can: the
difference is microseconds and the coarsest thing that prints it is `&T`, which is a minute. That
half of the change rests on the corpus measurement above, not on a test.

## Test counts

Every project run individually, whole output captured, **0 skipped** everywhere:

| Core | Containers | Text | Vector | Rendering | Markup | OpenDocument | WordProcessing | Spreadsheets | Presentations | Fidelity |
|---|---|---|---|---|---|---|---|---|---|---|
| 243 | 109 | 240 | 291 | 119 | 259 | 125 | 619 | **498** | 528 | 547 |

Every count is the briefed known-good except Spreadsheets, which is 483 plus this round's 15.
