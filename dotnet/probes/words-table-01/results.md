# words/table-001 — rotated table-cell text drawn upright

Round `words-table-01`, 2026-08-14. Worktree `wt-w-table`, reference LibreOffice **26.2.4.2**,
fonts complete (`fc-match "DejaVu Sans"` → DejaVu, Carlito, Caladea all present).

Prediction written and committed before any measurement: `prediction.md`, commit `d32c0576e80`.

## The seat

`grep -rn "textDirection" dotnet/src` returned **nothing**. `w:textDirection` was read by no
reader, and no cell model, layouter or painter carried a text direction at all. This was an
unimplemented property, not a mis-mapping.

With nothing reading it, a `btLr` label was laid out upright at the *column's* width — a few
points for a label column — so every line held one glyph and the cell became as tall as the label
had characters. One mechanism, two symptoms: on a logbook the row was already tall enough to
absorb the height and only the text layer shattered; on the EASA form it was not, and the label
column grew over two extra pages.

Census over the whole words track, by parsing every part of every file rather than grepping:
**111 occurrences of `w:textDirection` in 10 of the 200 documents, every one of them `btLr`.**
No `tbRl`, no RTF `\cltxbtlr`, no ODF equivalent anywhere in the track. That is the reach
ceiling, fixed before anything was measured.

## What LibreOffice 26.2.4.2 actually does

Established on **45 generated probe documents** rendered through the installed binary and read
out of the PDF's own operators. The C++ tree here is 27.2-alpha and was used only to find the
name of the thing (`DomainMapperTableManager.cxx`:325-350); every number below is measured.

1. `btLr` is written with the text matrix `0 1 -1 0 x y` — a quarter turn anticlockwise, glyphs
   advancing **up** the page, one `Tm`+`Tj` per glyph. Lines stack **rightwards**, one line
   height apart.
2. **A turned cell contributes nothing to its row's height.** Not one line's worth: a row holding
   only turned cells collapses to zero and LibreOffice draws neither its text nor its borders
   (`q1-solo-1`, `q1-solo-18`, `q6-two` — three probes, no ink at all in any of them). This is
   what dissolves the apparent circularity, and it is the fact the obvious implementation gets
   wrong.
3. **The line breaks at the cell's ordinary inner height** — frame height less the two half grid
   lines and less the *vertical* padding. Horizontal padding does not shorten it. Pinned by a
   five-twip sweep of `w:trHeight`: the four-to-five glyph boundary sits at **exactly 500 twips =
   25.00 pt** in all three of {0.5 pt borders, no borders, 10 pt top and bottom cell margin},
   whose row frames are 25.5, 25.0 and 45.5 pt tall. A "the turn swaps the padding too" reading
   would have moved that boundary in two of the three; it moved in none.
4. **A line whose stack offset falls outside the cell is dropped, not clipped** — there is no
   text-showing operator for it, so it is absent from the text layer as well as the ink. A 50 pt
   column (inner 38.7 pt) draws four 11.55 pt lines, the fourth overhanging, and not the fifth.
5. `w:vAlign` places the line **stack** horizontally — top 71.20, centre 110.00, bottom 148.80 pt
   on one fixture. Paragraph `w:jc` runs along the vertical.
6. `tbRl` and `tbRlV` render identically to one another (`0 -1 1 0`); `lrTbV` and `tbLrV` render
   identically to no attribute at all. Both confirm LibreOffice's own collapse of six values to
   three, including its "we can't handle these" drop of `tbLrV`.

## What changed

Four files, all in `XRay.Office.WordProcessing`.

- **`Layout/PageTable.cs`** — a `CellTextDirection` enum, `PageTableCell.TextDirection` and
  `IsTurned`, and `PlacedTableCell.ContentTransform`. The turned cell's flow stays in its own
  upright coordinates and the transform maps it onto the page, so every consumer of `PlacedFlow`
  keeps working on one kind of flow and the line boxes, glyph runs and tab stops inside stay
  measured along the text's own direction.
- **`Layout/TableLayouter.cs`** — a turned cell is skipped in pass one and charges the row
  nothing; `Turned` lays it out in pass two against the settled row height, drops the lines that
  start outside, aligns the stack by `vAlign`, and builds the quarter turn. `Offset` moves the
  transform rather than the flow (moving both would place the label twice); `SliceRow`,
  `HeightAt` and `Sliced` exclude turned cells from a row split, which cannot divide text that
  runs across the row.
- **`Layout/PageDrawing.cs`** — `DrawCellContent` pushes the transform and draws the flow through
  the code every backend already had. Nothing in the PDF or Skia backends needed changing.
- **`Ooxml/DocxLayoutSource.Tables.cs`** — reads `w:textDirection`, mapping the six values to the
  three LibreOffice maps them to rather than the three the specification implies.

### Agreement with the reference, on the probes

Our output reproduces LibreOffice's rotated-cell layout to about **0.1 pt**, with the line breaks
identical rather than merely close:

| probe | reference | ours |
|---|---|---|
| `q5-h500` (25.5 pt row) | 3, 3, 1 glyphs at x = 71.20, 82.75, 94.30 | 3, 3, 1 at 71.20, 82.70, 94.20 |
| `q5-h2000` (100.5 pt row) | 7 glyphs, one line, x = 71.20 | 7 glyphs, one line, x = 71.20 |
| `p2-len53` (drop rule) | 4 lines drawn, fifth absent | 4 lines drawn, fifth absent |
| `p3-w400` (20 pt column) | 1 line | 1 line |
| `q1-solo-18` (row of only turned cells) | nothing drawn | nothing drawn |
| `q3` `vAlign` top/centre/bottom | 71.20 / 110.00 / 148.80 | 71.20 / 110.05 / 148.90 |
| `q4` `jc` centre | first glyph at y = 748.99 | 749.00 |

## Did one fix close all three? Yes.

`batch-check.sh`, `words/table-001`, before and after:

| document | before | after | reference |
|---|---|---|---|
| `A1. EASA Form 2.docx` | 9 p / 2399 w — `pages,words` | **7 p / 2207 w — `match`** | 7 p / 2205 w |
| `B11. TE.CAO.00129  Experience  logbook.docx` | 6 p / 1329 w — `words` | **6 p / 1253 w — `match`** | 6 p / 1247 w |
| `approvals-and-standardisation-…-logbook.docx` | 6 p / 1168 w — `words` | **6 p / 1096 w — `match`** | 6 p / 1098 w |

Three of three, from one change, with no per-document special casing. The two `.doc` files in the
group are untouched and still fail — see *Not done* below.

## Reach, from what resolves

All 200 words documents rendered twice — once with the fix, once with a binary built from the
four files reverted (copied aside, **not** stashed) — with `SOURCE_DATE_EPOCH` set so the two runs
are byte-comparable with nothing masked.

**8 of 200 renderings changed.** All eight are among the ten documents that carry
`w:textDirection`; the other two are unmoved because their turned cells sit where the change makes
no difference. Nothing without the property moved by a byte, which is the containment claim
stated as a measurement rather than as an argument.

Verdicts against the banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`,
all 200 resolved:

| | before | after |
|---|---:|---:|
| words track `match` | 159 | **162** |

Five verdicts moved, four of them the right way:

| document | before | after | ours | reference |
|---|---|---|---|---|
| `A1. EASA Form 2.docx` | `pages,words` | **`match`** | 9 p → 7 p | 7 p |
| `B11. TE.CAO.00129  Experience  logbook.docx` | `words` | **`match`** | 1329 w → 1253 w | 1247 w |
| `approvals-and-standardisation-…-logbook.docx` | `words` | **`match`** | 1168 w → 1096 w | 1098 w |
| `33004.docx` (group `missing-001`) | `pages` | **`match`** | 48 p → 47 p | 47 p |
| `Sample_SQMS_Program.docx` (group `done-015`) | `match` | **`pages`** | 61 p → 60 p | 61 p |

## The regression, and why it is being accepted

`words/done-*`, 159 documents, full gate: **158 match, 1 mismatch** — the mismatch is
`Sample_SQMS_Program.docx`, which this round cost. It is the one prediction that was wrong (P7,
"zero documents change verdict").

Taken apart rather than waved at:

- The document holds **three** turned cells, all in **one** row, all 366–367 twips wide, all
  containing the word `Rating`, and that row also states `w:trHeight w:val="1270"` (63.5 pt).
- Read upright at 18.3 pt less padding, `Rating` was six single-glyph lines ≈ 69 pt, which beat
  the 63.5 pt floor and made the row about 5.5 pt taller than it should be. Read correctly it
  charges nothing and the floor decides.
- So the fix removed **≈ 5.5 pt** from a 61-page document.
- The reference's page 61 holds **nothing but its header and footer** (124 raw tokens against
  1900-odd on a full page). Its body ends within a few points of the page-60 boundary and spills
  one trailing paragraph. Our body, 5.5 pt shorter, now fits.
- Page for page the two agree: our pages 59 and 60 hold the same content as the reference's, and
  the whole difference is that one trailing paragraph.

This is a knife-edge exposed, not a defect introduced. The previous pass was luck — a spurious
5.5 pt cancelling an accumulated shortfall of the same size, which is the standing ~0.1% advance
divergence recorded in `dotnet/CLAUDE.md`. Restoring the spurious height to keep the verdict would
be fitting the engine to one document against measured behaviour.

Net over the corpus: **+4 verdicts gained, −1 lost.**

## Looking at the pages

Four pairs sent to four **fresh subagents**, each given one image, the labels inside the image, no
numbers, no access to the repository and no way to run a command. Two of the four pairs were
initially the wrong pages — the brief's "page 6" was a page number from the *unfixed* 9-page
rendering — and the two reviewers who got them said so by finding no rotated text at all, which is
itself a useful control on the method.

The reviewer who saw the page that does carry the labels, blind, reported for **both** halves:
five rotated runs, *"all reading bottom-to-top (rotated 90° counter-clockwise), all legible as
whole words with normal letter spacing, **not broken into single letters**"* — and transcribed
`COMPONENTS OTHER THAN COMPLETE ENGINES OR AUXILIARY POWER UNITS`, `SPECIALISED SERVICES` twice.
That is the defect's own description, negated, by a reader who had never been told what it was.

**One reported absence was checked in the operators and is not real.** The same reviewer reported
that the reference's trailing words (`AUXILIARY POWER UNITS`, `SERVICES`) were *"not visible
anywhere"*. They are: the reference's page 5 carries turned glyphs at three distinct stack
positions, and `pdftotext` reads every one of those words on both sides, once each. The second
stacked line is a thin rotated run that did not survive to 150 dpi. This is exactly the failure
the `page-vision` skill warns about — confirm absence in the PDF's own operators, never in a
downscaled raster — and it would have been relayed as a reference defect had it not been checked.

## The raster-ceiling row: a false positive, corrected

`TODO.raster-ceiling.md` claimed **+38** ours-only words on page 6 of
`approvals-and-standardisation-…`. It was never a ceiling: both sides draw the same 47×90 JPEG on
that page and neither rasterises anything else.

| page 6 | ours before | ours after | reference |
|---|---:|---:|---:|
| words | 157 | **121** | **121** |
| raw `wc -w` | 160 | 123 | 123 |

Zero excess, not a residue. The row is struck out in `TODO.raster-ceiling.md` with the arithmetic
and the general lesson — the flag's condition is "our page holds tokens the reference's does not",
which *any* token-manufacturing defect satisfies, and sixteen of the thirty-seven flagged pages
carry no metafile. `raster-ceiling-pages.tsv` is corrected to match.

## Tests

`tests/XRay.Office.WordProcessing.Tests/TurnedCellTests.cs`, 16 tests. Each asserts a measured fact
rather than an implementation detail: the reader's six-to-three mapping, zero row-height
contribution, the collapse of a row of only turned cells, the break at the inner height, the
padding asymmetry that separates the implemented rule from the plausible wrong one, the drop
rule, both turn directions asserted on their basis vectors, `vAlign` across the stack, and that
`Offset` moves the label once.

**Verified failing against the unfixed behaviour**, in two runs, by reverting one file at a time
so the API stays and only the behaviour goes:

| reverted | result |
|---|---|
| `TableLayouter.cs` only | **9 failed**, 7 passed |
| `DocxLayoutSource.Tables.cs` only | **3 failed** (`btLr`, `tbRl`, `tbRlV`), 13 passed |

12 of the 16 fail without the fix. The four that do not are deliberately the controls — the
`lrTb`, `lrTbV`, `tbLrV` and absent-attribute cases, which assert that nothing changed.

### Counts, every project run individually

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 332 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 349 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| WordProcessing | 843 | 0 | 0 |
| Spreadsheets | 770 | 0 | 0 |
| Presentations | 679 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |
| total | 4432 | 30 | 1 |

**Fidelity is 30 failed of 550, which is the baseline established before anything was changed.**
Same 30 tests, none gained, none lost. Build is 0 warnings, 0 errors.

## Predictions, scored

| | claim | outcome |
|---|---|---|
| P1 | one fix closes all three | **right** — three of three to `match` |
| P2 | EASA form 9 → 7 pages | **right** — 7/7, and words 2207 against 2205 |
| P3 | single-character tokens collapse | **right** — the blind reviewer read whole words on both sides |
| P4 | words land in the band | **right** on all three, and within 2 words on two of them |
| P5 | 3–6 of 200 renderings change, no verdict moves but the three | **half right** — 8 changed, not 3–6, and two verdicts moved outside the group. Under-estimated in both directions: I did not expect an unrelated document to be *fixed*, nor one to be broken |
| P6 | fidelity stays 30 of 550 | **right** — exactly 30, same tests |
| P7 | `words/done-*` unchanged | **wrong** — `Sample_SQMS_Program.docx` lost a page. The falsification test I wrote for myself ("any `done-*` document changing") fired, and the cause turned out to be the knife-edge above rather than a modelling error |
| P8 | the raster-ceiling row is a false positive | **right** — zero excess after the fix |

Six right, one half right, one wrong. The wrong one was the prediction I had marked *high*
confidence, and it was wrong for the reason predictions about a corpus usually are: it reasoned
about the code path (only turned cells are reachable) and not about what a *correct* change
removes from a document already sitting on a boundary.

## Not done

- **`words/table-001/doc/150_5300_13_chg8.doc`** — 20 pages against 18. Untouched. Tables taller
  than the reference so footnote items orphan, plus a separate genuine raster ceiling on page 8.
- **`words/table-001/doc/150_5300_13_chg10.doc`** — 80 pages against 77, 23866 words against
  23360. Untouched, and not investigated at all.

Neither is a rotated-cell document; the whole group's `btLr` content is in the three DOCX files.

## Residual defects found on the way, none of them this round's

Recorded because they were seen properly and will otherwise be re-derived.

1. **`A1. EASA Form 2.docx` paginates ahead of the reference even though the totals now agree.**
   Page 2 holds 246 words against 199, and the drift starts on page 2 — before any rotated cell.
   By page 5 we are a whole section ahead. The gate is blind to it now that the page count and
   word total match, which is the standing warning in `dotnet/CLAUDE.md` made concrete.
2. **The same document draws none of its 30 checkboxes.** The blind reviewer counted an empty
   square on every one of the 22 `C1…C22` rows and on each of the 8 NDT method rows in the
   reference, and none in ours. Our `C`-rows are correspondingly ~35% shorter (16.7 px against
   22.5 px at 150 dpi), which is most of defect 1.
3. **`B11. TE.CAO.00129  Experience  logbook.docx`: the *reference* prints "Page 3 of 6" on every
   one of its six pages**, where we print 1…6. Ours is right and 26.2.4.2 is wrong. Worth knowing
   before anyone treats a page-field difference on that document as ours.
4. **The same logbook's ID column numbering diverges**: we run 1–7, 9, 12–16 where the reference
   runs 1–13. Our list counter is consuming values the reference does not, near two
   vertically-merged cell groups. Independent of this round — the token counts still land in the
   band.

## Reproducing

Probe generators and the sweep are under
`/tmp/…/scratchpad/wtable/` and are not committed; they are 45 generated DOCX files, three
`mk*.py` scripts and a rendering pair. What is worth keeping is in the tests and in the remarks
on `TableLayouter.Turned`, both of which carry the measured numbers rather than pointing at them.

```sh
# the group, then the regression sweep — both, in that order
export XRAY_OFFICE_CLI=$PWD/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
.claude/skills/corpus-batches/scripts/batch-check.sh /c/sandbox/workdir/sample-files 'words/table-001' out 3
.claude/skills/corpus-batches/scripts/batch-check.sh /c/sandbox/workdir/sample-files 'words/done-*'   out 6
```
