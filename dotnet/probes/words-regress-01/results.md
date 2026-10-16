# words/regress-01 — the two `done-015` regressions, taken apart

Round `words-regress-01`, 2026-08-15, worktree `wt-w-regress`. Reference LibreOffice **26.2.4.2**,
Carlito / Caladea / Liberation / DejaVu all resolving, `libreoffice-math` still absent. References
reused from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, never re-rendered;
`SOURCE_DATE_EPOCH=1700000000` on every render that is diffed.

Prediction written and committed before the fixes and before any sweep: `prediction.md`, commit
`769d91c8941`.

**Both seats were found. Both are fixed. Neither document's verdict flips**, because each turned out
to sit behind a *second*, older defect that this round did not close. The two fixes are still net
positive on the corpus — two other documents pass that did not — and the two seats are named
precisely enough that the next round can go straight at them. Section 9 says what is left.

## 0. The measurement trap that cost this round two hours, and the numbers it spoiled

Recorded first because everything below depends on it, and because it is a new shape.

To find out whether a residual 0.30 pt mattered, I patched `LineSpacing.cs` with a throwaway
one-twip hack, built, measured, and then restored the file with **`mv LineSpacing.cs.before
LineSpacing.cs`**. `mv` preserves the source file's modification time, so the restored file looked
*older* than the compiled `XRay.Office.Text.dll` — and MSBuild's up-to-date check skipped the project.
**Every build for the next two hours silently carried the hack**, including three builds whose whole
purpose was to be free of it, and including one that reported `0 Warning(s), 0 Error(s)` in 14
seconds.

It was caught by a contradiction, not by suspicion: our line height at 10 pt was one twip *above*
the exactly-scaled value on Liberation Sans, and the arithmetic in the source says it cannot be.
`rm -rf src/XRay.Office.Text/{obj,bin}` and a rebuild made it go away, and the binary then rendered
`Sample_SQMS_Program.docx` **byte-identically to the session's very first build** — which was clean
all along.

- **`cp file file.before` and `cp file.before file` — never `mv` back.** `dotnet/CLAUDE.md` already
  says copy rather than `git stash`; the trap is in the *restore*, and `cp` sets a fresh mtime while
  `mv` and `git checkout` (which does update it) do not behave alike. `touch` the file after any
  restore.
- **A build that is a no-op reports success.** There is no output that distinguishes "nothing needed
  rebuilding" from "the thing you just changed was skipped".
- Everything measured on a contaminated binary was discarded and re-measured. The figures in this
  document are all from the clean tree; where a contaminated figure was quoted mid-round it is
  called out below.

Baseline, established on the clean session-start binary **before anything was changed**, and again
at the end: `XRay.Office.Fidelity.Tests` **Failed 30, Passed 520, Skipped 0, Total 550** — the briefed
number exactly, both times.

## 1. Seat one — `Sample_SQMS_Program.docx`, 60 pages against 61

### It is not a uniform shortfall

Pages 1–58 are line for line identical. The whole divergence is **1.30 pt of row height accumulated
on page 59**, which lets a four-line follow part fit where the reference takes three. Read out of
the two PDFs' own row rules rather than from the ink:

| page-59 row | ours (before) | reference | Δ |
|---|---:|---:|---:|
| the repeated header row | 64.00 | 64.30 | **−0.30** |
| the follow part of the row split from page 58 | 15.30 | 16.30 | **−1.00** |
| every other row on the page (30.10 / 43.90 / 18.30) | equal | equal | 0 |

Two independent causes, and only one of them is closed.

### (A) A split row's parts do not carry the paragraph's spacing — fixed

`probe-rowsplit-spacing.py` sweeps `w:spacing w:before` and `w:after` independently over a row cut
across a page and renders each through the installed 26.2.4.2. The reference's follow part is
`before + remaining lines + after + border` in **all eight** combinations; ours was that **less
`before`**, exactly, every time:

| before, after (pt) | ref follow | ours before the fix | ours after |
|---|---:|---:|---:|
| 0, 0 | 14.30 | 14.30 | 14.30 |
| 1, 0 | 15.30 | 14.30 | 15.30 |
| 0, 1 | 15.30 | 15.30 | 15.30 |
| 1, 1 | 16.30 | 15.30 | 16.30 |
| 2, 0 | 16.30 | 14.30 | 16.30 |
| 0, 2 | 16.30 | 16.30 | 16.30 |
| 2, 2 | 18.30 | 16.30 | 18.30 |
| 5, 3 | 22.30 | 17.30 | 22.30 |

So **a split row's two parts add up to more than the unsplit row, by exactly its space-before.**

Two competing readings were refuted rather than merely not adopted. `probe-rowsplit.py`'s `solo`
variant — one cell, no siblings — kills "an empty sibling cell is re-laid-out on the follow part",
which predicts the same 1.00 pt on the real document and nothing at all here. And
`probe-rowsplit-paras.py` moves the cut to a paragraph *boundary*: the reference's follow part
carries the space-before there too, so the rule is not special to a mid-paragraph cut.

That probe also found the rule's other half, which the first fix did not cover: with two paragraphs
of one line each cut between them the reference gives **two** parts of `before + line + after`, so a
part whose last line *completes* a paragraph is charged that paragraph's space-after as well. Both
halves are one statement:

> **A part of a split row spans from the top of the block its first line is in to the top of the
> block that follows its last line** — not from line top to line bottom. In the middle of a
> paragraph the two are the same; at either end of one they differ by its spacing.

This is Writer's `AddParaSpacingToTableCells` seen from the other end. The tree already honours that
setting at the *bottom* of a cell — `PlacedFlow.Advance` charges a cell for its final paragraph's
space-after — and did not honour it at the top of a follow.

**Changed:** `src/XRay.Office.WordProcessing/Layout/TableLayouter.cs` only. `HeightAt` measures a
follow part from `line.Top − UpperSpaceAbove(flow, line)` and ends it at the following block's top;
`Sliced` moves the drawn text down by the same amount so the ink lands where the height says.

After it, our follow part matches the reference on **8 of 8** spacing combinations and **7 of 8**
paragraph-boundary ones (the eighth is the probe's own artefact: it searches for a filler count
independently per renderer, so that row compares two different cuts).

### (B) Our 10 pt line height is one twip short — **not fixed**, and it is what is left

`probe-lineheight.py` measures 195 (face, size) pairs — Liberation Serif and Sans, Carlito, Caladea,
DejaVu Sans, every half-point from 5 to 24 pt — as baseline-to-baseline distances in the reference's
own text matrices. **We agree with 26.2.4.2 on 173 and differ on 22, always by exactly 0.05 pt (one
twip), in both directions.** Our value is `round(exact)` on all 195; LibreOffice's is not.

Liberation Serif at 10 pt — the size the SQMS header row is set in — is **11.55** in the reference
and **11.50** here, and four line gaps plus a descent is the 0.30 pt in the table above. Liberation
Sans at 10 pt is 11.50 on both sides, though the two faces' `hhea` sums are *identical*
(2355/2048), so whatever LibreOffice does is not a function of the summed metric.

It was not reconstructed, and the failure is recorded rather than papered over. `probe-ascent.py`
measures ascent and line height separately, one page per size, and both round irregularly:

- **No device grid fits.** Brute force over every resolution from 72 to 6000 dpi × {exact, rounded,
  ceiled, floored} ppem × 27 combinations of per-component rounding reproduces **none** of the 81
  ascent measurements exactly; the best partial is 77 of 81 at an implausible 4563 dpi.
- **Nor does any plain rounding of the exact value.** Ceiling to the twip is right on 15 of
  Liberation Serif's 18 sizes and wrong on three, and one of those three (10.0 pt) is a twip *above*
  the ceiling.
- The 1/100 mm round trip, `hhea` versus `OS/2` win versus typo metrics, and leading-above-ascent
  were each tried and each fails on a size the others get right.

**It is nonetheless demonstrably the only thing left on this document.** A throwaway build carrying
fix (A) *and* a one-twip addition at 10 pt renders `Sample_SQMS_Program.docx` at **61 pages with
every page's word count equal to the reference's, all 61 of them** — including the trailing
header-and-footer-only page 61. Fix (A) alone leaves it at 60. That experiment is the reason the
seat can be named with confidence and is not evidence for any particular rounding rule.

Changing a line height globally moves every document in the corpus and is the highest-risk area
`dotnet/CLAUDE.md` names. It needs its own round, against VCL's metric path rather than against a
curve fit.

## 2. Seat two — `airbus-pdf-information-package_v1-4.docx`, 1272 words against 1299

### The brief's "59 words short on page 9" is real and is not a page-9 defect

The −27 total is **one missing repeat of the table's header row**, worth about 30 tokens. The
reference's table runs onto page 9 and repeats its heading there; ours ends on page 8, so page 9
holds only the contact table. Redistributing content between pages cannot change a total, and the
total was −27 before this round and −27 after it — which is itself the proof that the deficit is a
whole repeated row and not scattered loss.

### The seat: `w:tblStylePr` was read by nothing — fixed

`grep -rn "tblStylePr\|cnfStyle\|tblLook" dotnet/src` returned **nothing at all**. Conditional
table-style formatting was unimplemented, not mis-mapped.

The table names `PlainTable1`, whose `<w:tblStylePr w:type="firstRow"><w:rPr><w:b/></w:rPr>` makes
the heading row bold, with `w:tblLook w:firstRow="1"` switching it on. Bold is wider, so the header
wraps onto more lines, so the row is taller, so fewer body rows fit per page. Confirmed in the
operators and not in a raster — page 6, before the fix:

| | first heading cell | fourth heading cell | face |
|---|---|---|---|
| reference | `Mapping` / `ID` on two lines | four lines | `LiberationSans-Bold` |
| ours, before | `Mapping ID` on one | three lines | `LiberationSans` |
| ours, after | `Mapping` / `ID` on two | four lines | `LiberationSans-Bold` |

The document supplies its own control: the run holding `(do not change!)` carries an explicit
`<w:b w:val="0"/>`, which is only meaningful if something above it is turning bold on.

**Census before measuring, by parsing every part of every file rather than grepping.** Of the 134
DOCX-family files in the words track, **14 declare a `w:tblStylePr` and 7 name such a style from a
table** — and every one of those 7 has a `firstRow` layer carrying `w:rPr`, with `w:tblLook
w:firstRow="1"`. That was the reach ceiling, fixed in advance; §5 shows all 7 moved and nothing else
did.

**Changed:** a new `Ooxml/WordTableStyleConditions.cs` (`w:tblLook`, in both the named-attribute and
the 2007 hexadecimal-bitmask spellings, and the §17.7.6 layer order), `WordStyles` (the conditional
`w:rPr` layers, and a table-style layer in `ResolveRunProperty`/`RunPropertyLayers` placed under both
style chains and over the document defaults, as §17.7.2 has it), `WordParagraphFormats` (threaded
through), and `DocxLayoutSource`/`.Tables` (the layers resolved per *cell*, since which layer a cell
is in is a property of where it sits).

The table style deliberately does **not** take part in §17.7.3's toggle cancellation. That rule
cancels a toggle set by a paragraph style *and* a character style — the two chains a run names — and
a table style is an outer third that the run is in by position. Cancelling against it would turn a
heading row's bold *off* in any table whose cells use a bold paragraph style, which is not what
Word or Writer draws.

### What is left on this document

The header row now matches the reference exactly, and the table still ends one row-group early. The
residue is a pagination difference **on page 4**, where our table begins 8.1 pt higher than the
reference's and one row there is 1.65 pt shorter — enough that we fit an extra row and the
reference pushes it over. It is a different defect from either of this round's and was not chased.

At −27 against a band of 25.98 the document fails by **one word**. That is not an argument for
tuning anything: the missing row is worth about thirty.

## 3. A third defect, found by a blind reader and confirmed in the operators

The `page-vision` pair for airbus page 6 went to a fresh subagent with no numbers, no repository and
no way to run a command. It reported the header row as three lines and four, in a heavier weight,
**in both halves** — the defect's own description, negated, by a reader who had never been told what
it was.

It reported two absences, and the skill's rule is that an absence is confirmed in the PDF's
operators and never in a downscaled raster. One was real and one was not:

- **Real.** *"the bottom half shows light grey fills on several body rows … in the top half all body
  rows are plain white"*. Page 6 carries **64 fills in the reference and 3 in ours**. That is the
  `w:tcPr` half of the same conditional table style — `band1Horz` shading `F2F2F2` — which this
  round deliberately did not implement. Found independently, by someone looking at a page.
- **Not real.** *"`71:VA`, `VAT number` and `Yes` are missing from the top half"*. They are on our
  **page 5**: the row straddles the 5/6 boundary in our rendering and sits whole on page 6 in the
  reference. `pdftotext` reads `VAT number` **twice in each** document. It would have been reported
  as lost text had it not been checked.

### The second pair: a blind reader found seat one's page break unprompted

A different fresh subagent got `Sample_SQMS_Program.docx` page 59 under the same conditions, and
located the defect without being told there was one. Asked only where each half's page ends, it
quoted the two halves' last lines and reported that ours fits one extra line —
*"(DPS) this element is superseded by the DPS (re: App E.4. for DPS requirements). Use T.E.4.:"* —
and that the reference instead leaves *"a visible band of empty white space (roughly one to two
lines' worth) between the bottom table rule and the footer"*. That is §1's 1.30 pt, described from
the picture.

It also reported the turned `Rating` labels as three runs reading bottom-to-top in **both** halves —
the `w:textDirection` fix from `words-table-01` holding, checked by someone who had never heard of
it. Its one doubt, that the reference's rotated glyphs look *"compressed … heavier/smeared"*, is the
per-glyph `Tm`+`Tj` placement LibreOffice uses for turned text rendering at 170 dpi rather than a
difference in the layout: both sides start the run at the same point (676.35/676.40, 493.30) and
`pdftotext` reads `Rating` **three times on each side**. Confirmed in the operators, as the skill
requires, and not in the raster it was seen in.

## 4. Regression: `words/done-*`, the whole track

`batch-check.sh`, 159 documents, full gate, on the clean fixed binary:

```
TOTAL 159  MATCH 157  MISMATCH 2  REF-CANNOT-RENDER 0
```

The two mismatches are the two documents this round is about, with the verdicts they already had:

| document | before | after |
|---|---|---|
| `Sample_SQMS_Program.docx` | `pages` 60/61 | `pages` 60/61 |
| `airbus-pdf-information-package_v1-4.docx` | `words` 1272/1299 | `words` 1272/1299 |

**No `done-*` document changed verdict in either direction.** The falsification test written into
`prediction.md` — "if more than two `done-*` documents change verdict, the census is wrong rather
than the implementation lucky" — did not fire.

## 5. Reach, over all 200 words documents

Rendered twice — once with the fixed tree, once with a binary built from the same files reverted
(copied aside, never `git stash`; and see §0 for how the restore itself can lie) — with
`SOURCE_DATE_EPOCH` set, so the two runs are byte-comparable with nothing masked. Verdicts against
the banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, all 200 resolved,
`batch-check.sh`'s three checks column for column.

- **30 of 200 renderings changed**; 170 byte-identical.
- **2 verdicts moved, both to `match`. None was lost.**
- Track total **163 → 165**.
- Summed |page error| over the 30 that moved: **39 → 36**. Summed |word error|: **1488 → 1471**.

| document | group | before | after |
|---|---|---|---|
| `FRE-03_mcar_part-3_and_IS_v2.9.docx` | `metrics-001` | `pages`, 77 p against 76 | **`match`**, 76 |
| `review-welsh-government-communications-mister-peter-mandelson.docx` | `metrics-001` | `pages`, 16 against 14 | **`match`**, 14 |

Those are the only two page counts that moved anywhere in the track.

**The conditional-table-style fix reached exactly its census ceiling and no further.** All 7
documents that name a style declaring a `w:tblStylePr` are among the 30 that changed; the other 23
name no such style at all, so they moved on the split-row rule. Nothing without either construct
moved by a byte.

## 6. Tests

15 new tests in two files, plus one authored fixture.

`tests/XRay.Office.WordProcessing.Tests/TableRowSplitSpacingTests.cs` — 5 tests. Each states a
measured rule and none hard-codes a font metric: the line height is read back off the flow the
layouter produced, and the cut is *searched for* rather than computed, because computing it would
need the rule under test.

`tests/XRay.Office.WordProcessing.Tests/ConditionalTableStyleTests.cs` — 10 tests: both `w:tblLook`
spellings and the precedence between them, the §17.7.6 layer order, a layer the look switched off,
the unconditional layer, and the whole path end to end over
`tests/corpus/features/table-style-first-row.docx`.

That fixture is its own control, and LibreOffice 26.2.4.2's own PDF of it is the ground truth: a
style whose `firstRow` layer is bold and whose `lastRow` layer is italic, with a `w:tblLook` asking
for the first row and **not** the last, and a heading cell that says `w:b w:val="0"` outright. The
reference embeds `LiberationSerif-Bold` and `LiberationSerif` and **no italic face at any weight** —
so the heading's first cell is bold, its second is not, and nothing in the last row is italic.
`make-fixture.py` authors it.

### Verified failing against the unfixed behaviour

Reverted one file at a time, so the API stays and only the behaviour goes:

| reverted | result |
|---|---|
| `TableLayouter.cs` only | `TableRowSplitSpacingTests` **3 failed**, 2 passed |
| the conditional-layer wiring only | `ConditionalTableStyleTests` **1 failed**, 9 passed |

The three that fail are the three behavioural claims: the follow part's space-before, the sum
exceeding the row, and the space-after on a part that finishes a paragraph. The two that pass are
deliberately the controls — the no-space-before case, where the parts *must* add up to the row, and
the guard that no line is lost or drawn twice. Of the ten conditional tests, nine are reader-level
and exercise an API that does not exist at all in the unfixed tree; the end-to-end one is the only
one that can be run against it, and it fails.

### Counts, every project run individually

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 337 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 349 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| WordProcessing | 865 | 0 | 0 |
| Spreadsheets | 832 | 0 | 0 |
| Presentations | 694 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |
| total | 4535 | 30 | 1 |

Fidelity is **30 failed of 550** — the baseline, same tests, none gained and none lost, measured
before anything was changed and again at the end. WordProcessing is 850 + the 15 new tests. Build is
0 warnings, 0 errors.

## 7. Predictions, scored

Six right, five wrong. The wrong ones are all on the same axis and it is worth naming: I predicted
that closing a seat would close its document, on two documents that were *briefed* as sitting on
knife edges. A seat and a verdict are different things, and a document that a correct fix moved onto
a boundary is a document with something else wrong with it.

| | claim | conf | outcome |
|---|---|---:|---|
| P1 | fix (A) alone flips SQMS to 61 pages | 75% | **wrong** — 60. It flips with (A) *and* the 10 pt twip, which is exactly what P3 said would not be done |
| P2 | it is still a knife edge; the reference's page-59 row split must be reproduced, not just the count | 60% | **right, and it fired** — the count did not move and neither did the split |
| P3 | (B), the line height, is not fixed this round | 85% | **right**, and for the stated reason: no rounding chain fits 195 measurements |
| P4 | fix (A) changes fewer than 20 of the 200 renderings | 65% | **wrong** — 23 moved on (A) alone (30 less the 7 that name a conditional style) |
| P5 | airbus flips to `match` | 70% | **wrong** — still −27, one word outside a 25.98 band |
| P6 | its page count stays at 9 | 40% | **right** — 9, and the risk I named did not materialise |
| P7 | a `pagination-002` document moves its page count | 50% | **wrong** — the only two page counts that moved are in `metrics-001` |
| P8 | no `done-*` document regresses | 55% | **right** — 157 of 159 before and after, the same two |
| P9 | Fidelity stays 30 of 550 | 80% | **right** — exactly, twice |
| P10 | every other project at zero failures; WordProcessing gains only the new tests | 70% | **right** — 850 + 15 |
| P11 | 10–35 renderings move and the track's `match` count does not fall | 60% | **right** — 30, and 163 → 165 |
| P12 | the new tests fail against the unfixed tree | 75% | **right** — 3 of 5 and 1 of 10, the rest controls or API-level |

The census in §2 is the part that behaved best, and it is the one thing here that was fixed before
any measurement: 7 documents predicted, exactly those 7 moved.

## 8. Contradicting the brief

- **"That page-9 shortfall is the real defect."** It is not a page-9 defect at all. Page 9 is short
  because the *table ends a page early*, and the missing 27 words are one repeat of a header row.
  The brief's own arithmetic — "a small genuine improvement on page 9 closes it" — points at the
  wrong page; nothing that can be done to page 9 alone will move the total, because moving content
  between pages does not change a document's word count.
- **"Its three turned cells … so laying them out correctly removed 5.5 pt from a 61-page
  document."** True of the *cause*, but the consequence is not distributed: pages 1–58 of our
  rendering and the reference's are identical line for line, and the entire difference is 1.30 pt on
  page 59. It is one page's arithmetic, not an accumulated drift.
- **"The reference's page 61 holds only a header and a footer … the whole difference is one trailing
  paragraph."** Confirmed, and worth restating precisely: page 61 holds **no body text whatsoever**,
  only the three-line footer block. What spills is the empty paragraph a DOCX must carry after a
  body-final table.

## 9. Not done

- **The 10 pt line height (§1B).** The one thing standing between `Sample_SQMS_Program.docx` and a
  page-exact match. Measured, bounded to 22 of 195 (face, size) pairs at one twip each, direction
  known per pair, cause not reconstructed. `probe-lineheight.py` and `probe-ascent.py` reproduce the
  whole table in one command each.
- **`airbus`'s page-4 pagination (§2).** An 8.1 pt offset above the table and a 1.65 pt row, not
  investigated.
- **The `w:tcPr` half of a conditional table style (§3).** Shading and borders. 64 fills against 3
  on one page of one document, found by a blind reviewer. It costs no gate column, which is exactly
  the blindness `dotnet/CLAUDE.md` warns the gate has.
- **The `w:pPr` half, and the band layers.** `mde087077~283.docx`'s `firstRow` layer carries a
  `w:pPr` this round does not apply. No used style in the words track carries `w:rPr` on a band
  layer, so the bands were left out rather than implemented against nothing measurable.
- **The other families.** The split-row rule was measured on the DOCX importer and applies to every
  table this layouter lays out, DOC and RTF and ODT included. That is a deliberate choice — it is a
  *layout* rule and Writer applies `AddParaSpacingToTableCells` to DOC and RTF too — but it was
  measured on one importer, and the ODF side of it is unmeasured.

## Files

```
src/XRay.Office.WordProcessing/Layout/TableLayouter.cs            the split-row rule
src/XRay.Office.WordProcessing/Ooxml/WordTableStyleConditions.cs  w:tblLook and the layer order
src/XRay.Office.WordProcessing/Ooxml/WordStyles.cs                the conditional w:rPr layers
src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs      threaded through
src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs          the per-cell field
src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.Tables.cs   resolved per cell
tests/XRay.Office.WordProcessing.Tests/TableRowSplitSpacingTests.cs
tests/XRay.Office.WordProcessing.Tests/ConditionalTableStyleTests.cs
tests/corpus/features/table-style-first-row.docx
probes/words-regress-01/                                        every probe above, runnable
```
