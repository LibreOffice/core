# words/missing-001 — results

Round worked in `wt-w-missing`, measured against the installed **LibreOffice 26.2.4.2** and the
banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`. `check-env.sh` clean;
`fc-match "DejaVu Sans"` → DejaVu Sans; Carlito, Caladea and Liberation all present.

`prediction.md` beside this file was committed before the measuring sweeps (78bf2db) and is
scored at the end.

## The seats, in one line each

| # | seat | file | reach on the words track |
|---|---|---|---|
| 1 | The paginator's table arm never collected the notes its cells cite | `Layout/Paginator.cs`, new `Layout/PlacedNotes.cs` | 3 documents |
| 2 | No margin line numbering existed at all | new `Layout/LineNumbering.cs`, `Layout/PageDrawing.cs`, `Ooxml/DocxLayoutSource.cs` | 1 document |

### 1. A footnote cited from a table cell was read, numbered and dropped

Notes were gathered on the `PageParagraph` branch of the top-level flow only —
`Paginator.cs`:1055, `notes.AddRange(NotesIn(paragraph, layout, lineIndex, allowed))`. The
`PageTable` branch beside it placed cells and gathered nothing, so a note whose anchor sat in a
cell never reached the page: no room reserved, nothing drawn, no diagnostic. `NoteRenumbering`
had the same blind spot in its own copy of the rule, which is why the walk now lives once, in
`PlacedNotes`.

Two halves went in. The collection — `notes.AddRange(PlacedNotes.In(part.Placed))`, taken from
the *placed part* rather than from the table, so a split row charges each of its pages for the
notes that page actually drew. And the room: the table arm now places against
`columnBottom - NoteHeight(notes) - (used + before)` instead of the bare column bottom, with one
retry when the part's own notes shorten the column further. One step rather than a fixed point,
and the retry is discarded unless it places strictly less — Writer damps the identical
circularity in `txtftn.cxx`:560 under the comment *"We break the oscillation"*.

### 2. Margin line numbers

`w:sectPr/w:lnNumType`. Writer holds line numbering as one document-wide `SwLineNumberInfo`
however many sections state it, and the DOCX importer says so outright
(`DomainMapper.cxx`:1213), so it sits on `PaginationOptions` and the last `w:lnNumType` in
document order wins.

Applied as a pass over the finished pages, which is exact rather than convenient: a margin number
is drawn outside the text area and cannot move a line, so there is no feedback loop and nothing
during the fill has to pay for it.

Everything positional was read out of the reference's **text operators**, on page 11 of
`xx_SETIS_PWS_template_10.19.22.docx`:

* 10 pt Liberation Serif, against a 12 pt body — the `Line Numbering` character style declares
  nothing (`DocumentStylePoolManager.cxx`:1593 falls through the switch) so it takes the
  document's default character size, not the paragraph's;
* right-aligned: one-digit numbers at x=52.95, two-digit at 47.95, three-digit at 42.95 — one
  5 pt digit advance apart, one right edge at 57.95 pt;
* 0.5 cm in from the 72.1 pt text edge, which is `SwLineNumberInfo`'s own `m_nPosFromLeft`
  default and the value the importer substitutes for an absent `w:distance`;
* on the line's own baseline;
* **a table charges the counter nothing** — 364…384 runs down to the line above the table and
  385 resumes below it.

Four authored probes rendered through 26.2.4.2 settle the size: with no `w:sz` in `docDefaults`
the numbers are 10 pt whether `Normal` says 12 pt or 20 pt. **One thing those probes show that is
not modelled:** a `docDefaults` of 20, 24 or 32 pt all draw at 11.70 pt, so LibreOffice caps the
size somewhere and the cap is not derived here. No corpus document states a default size.

## The footnote corpus reach — the brief's open question, answered

The brief asked whether footnotes are a widespread feature gap, suspected "may be the largest
single item on the track", and said the reach was unmeasured. Measured over all 200:

* **13 of 200 carry OOXML footnotes**; no `.doc`, `.rtf` or `.odt` in the track carries one.
* **Footnotes already worked.** Nine of the thirteen drew their note bodies before this round,
  four of those in `done-*` batches that pass the gate.
* The four that did not all cite from **inside a table cell**, and exactly three documents on the
  track do that: `TE.CAO.00125`, `FO.FCTOA.00010 Application for a Part-ORA ATO Approval.docx`
  and `EHEST-SMS-Safety-Management-Manual-V2.docx`. (The fourth non-matching row of my crude
  presence probe, `PES-Technical-Report-Template_Jan_2019`, is a probe artefact — its note text is
  absent from the *reference's* extraction too.)

So the answer is **3 documents, not 30, and not a feature gap** — the feature was there and one
branch of the paginator did not use it. `w:lnNumType` appears in **1** document of 200. I worked
footnotes first on that 3-vs-1 ordering, which was the right call by reach and the wrong one by
word count: line numbering was worth 548 words on its one document and the footnote 118 on its.

## Measured reach

200 documents rendered twice with `SOURCE_DATE_EPOCH=1700000000 TZ=UTC`, once with the tree as
found and once with both fixes, and the PDFs diffed byte for byte
(`probes/words-missing-01/render-words.sh`).

**4 of 200 changed.** Nothing else moved by one byte.

| document | before | after |
|---|---|---|
| `TE.CAO.00125 … OJT Logbook` | `words` 15/15 pages, 2675/2793 | **`match`**, 2793/2793 exact |
| `xx_SETIS_PWS_template_10.19.22` | `words` 15/15, 4375/4923 | **`match`**, 4920/4923 |
| `FO.FCTOA.00010 … ATO Approval` | `match`, 16/16, 4492/4513 | `match`, 4514/4513 — Δ21 → Δ1 |
| `EHEST-SMS-Safety-Management-Manual-V2` | `pages` 80/82, 19182/19222 | `pages` 80/82, 19199/19222 |

All four moved **towards** the reference; none moved away. The table-room reservation, which was
the part of the change with real regression surface, cost nothing measurable: not one document
outside those four changed a byte.

Whole-track gate against the banked references: **167 / 200 → 169 / 200**.

## The `done-*` result

`batch-check.sh` over `words/done-*`, 159 documents, both halves re-rendered:

```
TOTAL 159  MATCH 157  MISMATCH 2  REF-CANNOT-RENDER 0
```

The two are exactly the two the brief said to expect, and both were already failing on the
baseline sweep taken before any change:

```
words/done-015/docx/Sample_SQMS_Program.docx            pages  60/61   15954/15967
words/done-015/docx/airbus-pdf-information-package_v1-4.docx  words  9/9  1272/1299
```

`words/missing-001` under the same harness: **TOTAL 5 MATCH 3 MISMATCH 2**.

## Test counts

Every project run individually on the built tree.

| project | passed | failed | skipped |
|---|---:|---:|---:|
| XRay.Office.Core | 337 | 0 | 0 |
| XRay.Office.Containers | 109 | 0 | 0 |
| XRay.Office.Text | 367 | 0 | 0 |
| XRay.Office.Vector | 295 | 0 | 0 |
| XRay.Office.Rendering | 150 | 0 | 1 |
| XRay.Office.Markup | 259 | 0 | 0 |
| XRay.Office.OpenDocument | 125 | 0 | 0 |
| XRay.Office.WordProcessing | 893 | 0 | 0 |
| XRay.Office.Spreadsheets | 847 | 0 | 0 |
| XRay.Office.Presentations | 710 | 0 | 0 |
| **XRay.Office.Fidelity** | **520** | **30** | **0** |
| total | 4612 | 30 | 1 |

The fidelity figure is the briefed baseline, **30 failed of 550**, unchanged and with 0 skipped —
so the suite covered everything it should. The one skip is
`PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType`, pre-existing and unrelated. None of the
30 names a document either fix touches.

### The new tests, and that they fail without the fixes

`tests/XRay.Office.WordProcessing.Tests/TableCellFootnoteTests.cs` (4 cases) and
`LineNumberingTests.cs` (9 cases), both building their DOCX in memory.

Verified by reverting the *behaviour* while keeping the API — the two call sites, so the test
project still compiles — with `rm -rf obj bin` before the rebuild:

```
Failed!  - Failed: 8, Passed: 5, Skipped: 0, Total: 13
```

The 8 are every case that asserts the new behaviour. The 5 that still pass are the controls, and
they are controls on purpose: a footnote cited from body text is placed as before; a page citing
nothing has neither notes nor a rule; a document with no `w:lnNumType` draws no numbers.
Two line-number cases used `ShouldAllBe`, which is vacuously true of an empty list, so they were
given an explicit `ShouldNotBeEmpty` first — before that they were among the five and should not
have been.

Restored with `cp file.before file && touch file`, never `mv`, plus `rm -rf obj bin`, and then
checked the way CLAUDE.md says to check it: `TE.CAO.00125` and `xx_SETIS` re-rendered from the
restored binary are **byte-identical** to the run this document's reach figures come from.

## Things found that contradict the brief, or that it did not know

1. **`words/missing-001` holds five documents, not four.** `MANIFEST.tsv` agrees. The brief named
   `33004.docx`, which already passes, and omitted
   `CRIF - Spécification technique - Socle applicatif.docx`, which fails at 27/29 pages and
   6260/6618 words. The brief's own warning about a list written from pre-regroup notes applied to
   itself.
2. **`33004.docx` is closed**, at 47/47 pages and 11980/11963 words. The brief's suspicion was
   right and the `w:textDirection` fix is what did it. It should move out of `missing-001`.
3. **"We render no footnotes at all" is not what is wrong.** Footnotes render on nine of the
   thirteen corpus documents that have them. The gap was one branch of the paginator.
4. **The corpus reach is 3, not "possibly thirty".** A footnote feature gap reaching thirty
   documents would have been the largest item on the track; this one is not, and the brief's
   instruction to order by reach was right even though the estimate it was based on was not.
5. **Two claims from the blind page review are refuted by the operators**, which is the brief's
   own rule working. A fresh reviewer given page 3 of `TE.CAO.00125` with no numbers and no
   hypothesis reported (a) that the reference sets the table's labels in italic where we set them
   upright, and (b) that our footnote's last line is overwritten by the footer. Neither survives
   `pdf-ops.py dump`: **no italic face appears in either PDF** on that page (both draw
   `DejaVuSans` and `DejaVuSans-Bold` and nothing else), and our footnote's five baselines sit at
   80.05…42.85 with the footer at 33.40 — about 9 pt of clearance, no overlap. What the reference
   *does* on those cells is draw text horizontally scaled (`10.58pt` at `10.00w`), which at
   117 dpi reads as a different face. The reviewer's other observations held up.

## What the review found that is real and is not mine

The blind reviews are in the round's evidence rather than in this file's claims, but three
observations are worth carrying forward as separate defects:

* **`xx_SETIS`: the table header row is not shaded.** The reference greys all four header cells
  and we leave them white. The brief noted this too, and it does not move the gate.
* **`xx_SETIS`: the first-line indent after a `(a)`/`(b)` label inside a table cell is ~12–15 px
  wider than the reference's**, and it costs a mid-word break — we set "Responsivene / ss to
  Customer" where the reference sets "Responsiveness / to Customer". Second and later lines of the
  same cell agree exactly, so it is the label-to-text step and not the cell's indents.
* **`TE.CAO.00125`: our "Practical type training data" table carries a `Location` row on page 3
  that the reference does not** — the reference's rows are taller and push it over the break. A
  row-height difference, not a lost row.

## What I did not do, and why

`May 25 bulletin focus on carers in the workplace.docx` and
`CRIF - Spécification technique - Socle applicatif.docx` were **not attempted**. Both still fail
exactly as they did on the baseline. Neither is a bounded change: the bulletin is missing two
graphics *and* a whole trailing section (five of our pages against the reference's four, with two
of ours empty), and `CRIF` is undiagnosed — the brief does not mention it because the brief did
not know it was in the group. Two seats closed with their reach measured is the brief's own
preference over four touched.

One further gap is left open deliberately: **`NoteRenumbering` still does not descend into
tables.** A document whose notes restart per page *and* cites one from a table cell would count
that note but be unable to rewrite its citation, since `Apply` rewrites top-level blocks only.
`NoteRenumbering.Applies` is false for all but one document in the corpus and that one has no
in-table anchor, so fixing it would be speculative work with real regression surface on the one
document that does exercise the path.

## Scoring the prediction

| # | predicted | measured | verdict |
|---|---|---|---|
| P1 | 3 of 5 in `missing-001` match; bulletin and CRIF unchanged | 3 of 5, same two failing with the same verdicts | **right** |
| P2 | 169 / 200 on the words track; `FO.FCTOA.00010` and `EHEST` do *not* flip | 169 / 200; neither flipped | **right, including the reasoning** |
| P3 | 3–10 documents change byte for byte; floor 4; nothing gets worse | **4**; nothing got worse | **right, at the floor** |
| P4 | 157 / 159 `done-*`, the two named | 157 / 159, exactly those two | **right** |
| P5 | both tests fail unfixed, pass fixed; fidelity 30 or 28 | 8 of 13 fail unfixed, 13 pass fixed; fidelity **30** | **right on the tests, and the "or 28" was wrong** |

P5's soft half is the one to record honestly: I allowed that the fidelity count might drop to 28
if the two closed documents were among the 30. They are not — the fidelity suite runs authored
feature documents, not the corpus, so no corpus verdict can move it. That was a sloppy hedge
rather than a prediction, and writing it down is how it stops being repeated.

P3's range was too wide in the direction that costs nothing to be wrong about. The floor was
argued from the census and was exactly right; the upper bound was a hedge against the table-room
reservation, and the reservation turned out to touch nothing outside the four. A tighter
prediction — "4, and the reservation moves nothing" — was available from the census and I did not
make it.

## Reproducing

```sh
export SOURCE_DATE_EPOCH=1700000000 TZ=UTC
CLI=$PWD/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

bash probes/words-missing-01/render-words.sh "$CLI" /abs/out/after 6
python3 probes/words-missing-01/score-against-banked.py /abs/out/after

XRAY_OFFICE_CLI="$CLI" .claude/skills/corpus-batches/scripts/batch-check.sh \
    /c/sandbox/workdir/sample-files 'words/done-*' /abs/out/bc-done 6 > sweep.log 2>&1
grep '^TOTAL' sweep.log
```

`.claude/` is absent from this worktree's checkout — the scripts were run from
`/c/sandbox/workdir/libreoffice-core/.claude/`, which is the same tracked content.
