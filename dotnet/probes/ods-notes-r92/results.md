# Round 92 — the ODF sheet's notes, and the two things that put a row on Calc's *measured* branch

    ours   = XRay.Office.Cli @ 1184c6316 (base) and @ 1184c6316 + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2 0229ac93fcf0…
    corpus = /home/user/corpus-odf/sheets — 307 `.ods`, 26.2.4.2's own conversion of the corpus
             /home/user/sample-files/sheets — the same 307 documents as `.xlsx`/`.xls`/`.xlsm`
    fonts  = the five tarball confounds as `probes/ods-track-r88` left them; none moved this round
    rule   = batch-check.sh of 2026-09-05 (column 9, `glyphs`, within max(2 %, 15))
    date   = 2026-09-10

## 0. The headline

| | base | head |
|---|---:|---:|
| `.ods` of 307 | 274 | **278** |
| the same documents as `.xlsx`/`.xls`/`.xlsm`, of 307 | 294 | **294** |

`REF-CANNOT-RENDER 0` in all four sweeps, and **no row failed on either side in either run**, so
the totals are comparable without any exclusion — `compare-ods.txt` and `compare-orig.txt` state
that explicitly rather than leaving it to be assumed. **Four verdicts gained, none lost**, on the
two seats the brief named:

| document | before | after | reference |
|---|---|---|---|
| `Hazard Analysis Template.ods` | `pages,words` 2 pages / 2131 glyphs | **match**, 3 / 3309 | 3 / 3313 |
| `RMP 2011-2014 and Inventory.ods` | `pages` 36 / 97 952 | **match**, 38 / 98 381 | 38 / 98 381 |
| `hdss-bulletin-index-2019-2022.ods` | `pages` 21 / 18 383 | **match**, 24 / 18 407 | 24 / 18 407 |
| `Special-Procedures_2025-07-10.ods` | `pages` 21 / 153 843 | **match**, 22 / 153 843 | 22 / 153 843 |

The `.ods` figure is not comparable with `ods-track-r88`'s 273: that was a different commit and
the eight merges between them moved a row on their own. The comparison that means something is
base against head at one commit, and it is above.

## 1. Seat one — `style:print="… annotations …"`, and the two inputs it needed

The brief's reading is exactly right and nothing in it had to be revised. The expensive half — the
`ScPrintFunc::DoNotes` port, the `"GW99999:"` mark column, the column-major order, the 200-twip
advance — was already built and already proven on `Hazard Analysis Template.xls`. What the ODF
path was missing is its two inputs, and each hides differently.

**The flag is a token inside a list.** `style:print` is space-separated and `annotations` is the
word: `PROP_PrintAnnotations` is mapped from it by `XMLPMPropHdl_Print(XML_ANNOTATIONS)`
(`xmloff/source/style/PageMasterStyleMap.cxx`:80,
`xmloff/source/style/PageMasterPropHdlFactory.cxx`:85), and `ScPrintFunc` reads the item into
`aTableParam.bNotes` (`sc/source/ui/view/printfun.cxx`:944). One line in `OdsPrintSetup`.

**The notes are fastened to their cells by containment**, as `office:annotation` children of the
`table:table-cell`, so the address comes from the walk. It has to be a second walk of the table
rather than a read of the content tree, because `OdfContentReader` deliberately *hoists* an
annotation into a section of its own — which is right for extraction and loses the cell.
`OdsNotes` is that walk.

**And the author line is inside the text rather than in `dc:creator`.** Every annotation in these
two documents says `<dc:creator>Unknown Author</dc:creator>`, and 26.2.4.2 prints none of them:
`RMP`'s page 38 reads `B54:` then `Elina Zheleva:` then `ex OPS.026`, which is the mark followed
by the annotation's own two paragraphs. A reader that composed the creator with the text would
print the wrong name on every note in the corpus.

Reach is **2 of the 307** and both were failing, exactly as the brief said. `RMP`'s note pages
come out character-exact against the reference and `Hazard`'s within 4 glyphs of 3313 — the same
four the `.xls` original is short by, so the residual is shared with a row that already passes.

### An instrument note in `XRay.Office.Spreadsheets/TODO.md` is wrong and is corrected

> *"LibreOffice's flat-ODS export drops cell annotations entirely … `office:annotation` appears
> zero times in the `.fods` and twenty-four times in the `.ods` of the same workbook."*

It does not. Measured on both witnesses against 26.2.4.2: `RMP 2011-2014 and Inventory.ods` holds
**12** `<office:annotation` and its `--convert-to fods` holds **12**; `Hazard Analysis
Template.ods` holds **8** and its flat export holds **8**. The likely origin of the wrong figure
is the instrument rather than the binary — a packaged `content.xml` is one long line, so
`grep -c` on it answers 1 where `grep -o | wc -l` answers 12, and the same command on a
pretty-printed `.fods` answers 12. **Count occurrences, not lines**, and the flat-ODF trick is
usable for notes after all: this round's `sheet-print-notes.fods` fixture *is* 26.2.4.2's own
`--convert-to fods` of the existing `.xls` fixture, annotations and all.

## 2. Seat two — a row is *measured* rather than computed for two reasons that are not its string's

### What the four rows actually are

Three of the four `pages` rows with identical glyph counts are one defect, and it is not the four
the brief grouped: `017_Timeline_Templates` is something else (§4). The three are
`Special-Procedures_2025-07-10`, `hdss-bulletin-index-2019-2022` — which the brief did not list
among the four — and the seat-one document `RMP`, whose page shortfall was the notes.

### The instrument, which is the useful half

Do not infer a row height from a rendered pitch. **`soffice --convert-to fods` prints the height
Calc computed for every row**, as `style:row-height` on the automatic row style, and on a
scaled sheet that is the only way to separate a row-height question from a print-scale one:
`Special-Procedures` prints at `style:scale-to="39%"`, so its rows arrive on the page at 5.373 pt
against the reference's 5.804 and neither number is a row height. `rowheights.py` reads them.

### The measurement

| document | rows | 26.2.4.2 | this tree, base | this tree, head |
|---|---|---:|---:|---:|
| `Special-Procedures_2025-07-10.ods` | 6–200 | **298** | 276 | 298 |
| | 201+ | 300 (stored) | 300 | 300 |
| `hdss-bulletin-index-2019-2022.ods` | 0 | 276 | 276 | 276 |
| | 1–200 | **298** | 276 | 298 |
| | 201+ | 300 (stored) | 300 | 300 |

The 200-row cutoff is `OdsPrintSetup.RecalculatedRowLimit` and was already right on both — the
rows past it agree exactly on both sides, which is the control that says the disagreement is the
*value* and not the rule.

276 and 298 are the two branches of `ScColumn::GetOptimalHeight` and nothing else. 276 is
`lcl_GetAttribHeight`'s arithmetic for Calibri 11 — `trunc(220 × 1.18) + 40 − 23` — and a probe
sweeping twelve font sizes (`sizes.py`) reproduces the reference at every one of them: 6/8/9/10 pt all land on the 256-twip floor, 10.5 → 264, 11 → 276, 12 → 300, 14 → 347,
18 → 441, 24 → 583. 298 is one *measured* EditEngine line: the em rounded to 15 device pixels at
96 dpi, an ascent of 14 and a descent of 4, plus a pixel of margin either side, over Calc's
rounded `nPPTY` of 0.067 — `(14 + 4 + 1 + 1) / 0.067`. `StandingEditLine` has computed exactly
that since round 56; it was simply never asked.

### What puts a cell on the measured branch, by one-attribute variant

`variants.py` patches one thing out of each real document and asks 26.2.4.2 for its own answer:

    Special-Procedures  as it stands                      rows 6-200   298.2
                        <calcext:conditional-formats/> removed         276.0
    hdss-bulletin-index as it stands                      rows 1-200   298.2
                        every <text:a> rewritten to its text           276.0
                        (row 0 — the header, the one row with no link) 276.0 either way

So:

- **A conditional format clears `bStdOnly` outright**, whatever the condition says and whether or
  not it fires: `if (bStdOnly && !pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
  bStdOnly = false;` (`sc/source/core/data/column2.cxx`:937-941, under the comment *"conditional
  formatting: loop all cells"*). It is the **pattern** that is tested, so a rule declared over a
  whole column makes every row of that column measured.
- **A hyperlink cell is an `EditTextObject` holding one field**, which is the same object a rich
  string makes and which `SheetEditCellTests` has covered since round 56 — reached here by a
  different door, and the door was never opened.

Both are one line of the cell's own face, so both are `StandingEditLine`. The change is four
extra terms in one predicate in `SheetOptimalRowHeights`, plus the two readers that had to supply
the ranges: `OdsConditionalFormats` and `XlsxSheetReader.ReadConditionalRanges`.

### The spelling, and the one document it leaves behind

`calcext:conditional-formats` is the extension namespace's, and it is what LibreOffice writes.
Censused over the 307 converted `.ods`: **95 state `calcext:conditional-format`**, 54 state a
`style:map` in `content.xml`, and exactly **one states a `style:map` and no `calcext:`** —
`2025_Active_Civil_Airmen_Statistics_FINAL.ods`. That one is deliberately not read: recovering it
means resolving every automatic cell style back to the cells naming it, and the document fails on
pages in the *other* direction (39 against 36, unchanged at both binaries), so the rule would not
close it. **74 of the 307 hold a `text:a`.**

## 3. Reach, cost and confinement

**Confinement, shown rather than asserted.** The whole diff is inside `XRay.Office.Spreadsheets`.
110 words and slides documents — `.docx`/`.doc`/`.pptx`/`.ppt` from the original corpus and
`.odt`/`.odp` from the converted one, every ninth and every seventeenth path — rendered at both
binaries under `SOURCE_DATE_EPOCH`, `obj`/`bin` of the changed project cleared before each build:
**110 of 110 byte-identical, with nothing masked.** `render-ours.sh`, `confine.list`.

**Reach on the two sheet tracks**, PDF creation dates masked and compared byte for byte:

| | renderings moved | gate columns moved | verdicts moved |
|---|---:|---:|---:|
| `.ods`, 307 | **64** | 5 | 4 gained, 0 lost |
| `.xlsx`/`.xls`/`.xlsm`, 307 | **16** | 1 | 0 |

The one original-track row whose columns moved is
`ecopy of 2016 statistical bulletin…xlsx`, and only its raw-word count: 25 528 glyphs against
25 548 before and after, `match` either way. **One of the sixteen is a `.xls`** —
`environment-edb-docs-edb-emissions-databank`, whose hyperlink cells reach the widened predicate
through `XlsWorkbookReader`'s existing `HyperlinkRanges`. The BIFF `CONDFMT` record is *not* read,
so the conditional half of the rule does not reach `.xls` at all; that is left, and it is why
`Special-Procedures_2025-07-10.xls` — which matches at 22/22 before and after — does not move.

**The 59 `.ods` renderings that moved without moving a gate column move the right way.** Matching
text lines between our rendering and the reference's and taking the mean |Δy| over the matched
pairs: of the 64 movers, **57 get closer to 26.2.4.2, 4 get further and 3 are level**. The largest
gains are the documents the verdicts moved on — `hdss` 160.98 → 0.01 pt over 1402 matched lines,
`wiley-cancelled-title-list` 128.71 → 0.00 over 3986, `Special-Procedures` 77.03 → 0.02 over
16 766, `Chicago.List.2025` 61.75 → 0.03 — and the four that worsen are small
(`ecopy of 2016` 0.08 → 1.17 pt) with one instrument artefact: `Hazard Analysis Template` reads
0.08 → 15.70 because its matched-line count goes 36 → 108 when the two note pages appear, so the
matcher is pairing across a different page structure. Its gate row is `pages,words` → `match`.

## 4. What is left, with its seat

- **`017_Timeline_Templates_for_Excel_b88faee6.ods`**, 2 pages against 3, is **not** a row-height
  row. 26.2.4.2's own recomputation of it agrees with ours everywhere that matters — it shortens
  `VerticalTimeline`'s rows 2–65 from the stored 285.1 to 276 and we do the same — and the page
  the reference has that we do not is **blank**: its `VerticalTimeline` sheet prints over two
  pages with nothing on the second. That is a print-area question and the seat is
  `ScDrawLayer::GetPrintArea` maxed into `ScDocument::GetPrintArea` (`documen2.cxx`:644-664),
  which `SheetDrawingArea.Extend` models; our extent for that sheet's shapes is short.
- **`sistem-rekod-markah-srm-_-rekod-master.ods`**, 22 against 26, is a **page-column** question
  and not a vertical one. The reference prints five pages per sheet block and we print three for
  the first two blocks and five for the last three: its pages 2 and 4 are narrow spill columns
  carrying `C | 0 | C | D | 0 | D | E | 40 | E`, and we fit those columns onto the previous page.
  Same document, same rows, so it is column widths or the page-column split, not row heights.
- **The `style:map` spelling of a conditional format**, 1 of 307 (above).
- **BIFF `CONDFMT`**, so the conditional half does not reach `.xls`. Nothing in the corpus needs
  it — the `.xls` column is 64 of 64 — but the two spellings of one workbook now disagree about a
  row height, which is a thing a later round should know rather than rediscover.

## Files

| file | what it is |
|---|---|
| `sweep.sh` | `batch-check-tmo.sh` plus `$REUSE_REF`, a directory of reference PDFs to copy instead of re-rendering |
| `base-ods-rows.tsv`, `head-ods-rows.tsv` | the `.ods` column at both binaries, 274 → 278 of 307 |
| `base-orig-rows.tsv`, `head-orig-rows.tsv` | the same documents as `.xlsx`/`.xls`/`.xlsm`, 294 → 294 |
| `compare-ods.txt`, `compare-orig.txt` | `compare.py`'s output for each pair |
| `compare.py` | joins two sweeps, excluding every row that failed on either side in either run |
| `variants.py` | the one-attribute variants, read back through `--convert-to fods` |
| `rowheights.py` | row-height ranges of an ODF spreadsheet, packaged or flat |
| `sizes.py` | what 26.2.4.2 computes for a standard row over twelve font sizes |
| `render-ours.sh`, `confine.list` | our half only, under `SOURCE_DATE_EPOCH`, for the confinement check |
| `movers.txt` | the 64 + 16 renderings that moved, dates masked |
