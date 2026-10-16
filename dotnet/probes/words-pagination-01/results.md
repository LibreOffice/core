# words/pagination-001 — results

Measured on this container: LibreOffice **26.2.4.2** 620(Build:2), `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf`, Carlito and Caladea present. `SOURCE_DATE_EPOCH=1600000000` throughout.
Reference PDFs reused from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`.

## Scoreboard

| | before | after |
|---|---:|---:|
| `words/pagination-001` | 0 of 10 | **2 of 10** |
| `words/done-*` | — | **157 match of 159**, the 2 another agent owns |
| `XRay.Office.Fidelity.Tests` | 30 failed of 550 | **30 failed of 550**, 0 skipped |

The two seats are `ESPN-R - MCF - Manual - Ed1.0 - For Publication.docx` (34 → **35**, ref 35)
and `FO.FCTOA.00010 Application for a Part-ORA ATO Approval.docx` (15 → **16**, ref 16).

```
1447.doc                                3/4     unchanged, not attempted
A_320.doc                             141/118   unchanged, not attempted
absrc-pac-01-info-note-en.doc           6/7     unchanged, not attempted
24-25_FAA_Holdover_Tables.docx        154/155   was 142/155
ESPN-R - MCF - Manual                  35/35    MATCH   (was 34/35)
ESPN-R - MCF - RA - Ed1.docx           59/58    unchanged count, ink overflow reduced
FAA 2025-26 Holdover Tables.docx      185/167   was 154/167
FO.FCTOA.00010                         16/16    MATCH   (was 15/16)
report-template.docx                   19/20    unchanged, not attempted
template---tpr-…-with-guidance.docx     8/7     unchanged, not attempted
```

## The brief describes a different group, and this is the first thing to fix in the next one

Five documents the brief names as members are in `words/pagination-002`:
`150-5370-10H.docx`, `AC-150-5370-10G-updated-201604.docx`,
`EHEST-SMS-Safety-Management-Manual-V2.docx`, `150_5300_13_chg12.doc` and
`docs-quality-MA.IMS.00001-…`. On disk and in `MANIFEST.tsv` alike. And `1447.doc`, which the
brief says is in `ceiling-001` and is not this group's business, is in `pagination-001`.

So **only one of the two promised pairs was in range**. The FAA AC pair belongs to whoever has
`pagination-002`. `A_320.doc` at **+23** is this group's largest single gap and the brief does
not mention it.

## Four defects fixed

### 1. A half-stated `w:spacing` on a style Writer already has

`WordStyles.CompleteOneSidedSpacing` filled the unstated half from Writer's pool row for the
**parent** style's `w:name`, and gave nought when Writer had no style of that name. That last
clause is wrong when the style *itself* is one of Writer's headings: it is found in the pool
rather than created, with Writer's own hierarchy still under it, and reads its `Heading` base's
12 pt above / 6 pt below.

`one-sided-spacing-source.py` measures it on 26.2.4.2 — sixteen variants over four name pairings
times both declaration orders, plus a fifteen-name sweep — reading `fo:margin-bottom` straight
out of `--convert-to fodt`. The child states `w:before="480"`, a control that never appears in an
answer, so **"mirror the stated value" is refuted outright**.

```
child          parent         child declared first   parent declared first
heading 4      heading 2      120                    360   (the parent's own w:after)
heading 4      Custom Par     120  <- the cell that moves
Custom Kid     heading 2      120                    360
Custom Kid     Custom Par       0                    360
```

Only Heading 1-9, Title and Subtitle answer from the style's own end. `Caption`, `List`,
`Quote` and `Body Text` read nought there while still reading 120, 140 and 140 as a *parent* —
so the two ends need two tables, and the change is **additive**: it fires only where the old
reading gave nought.

The prior for this — that lower-case `body text` is a pool name worth (0, 7 pt) — is now in
doubt: it measures nought below on 26.2.4.2 where `Body Text` measures 140. Left alone
deliberately, and recorded in `WriterPoolSpacing`.

### 2. A `w:trHeight` floor sits *under* the row's borders

`TableLayouter` added the border to the content and then raised the total to the floor, so a row
resting on its floor came out exactly the floor tall. LibreOffice raises the content to the floor
and adds the border on top.

`row-min-height-border.py` sweeps the border width against a fixed 24 pt floor:

| `w:sz` | border | LibreOffice | ours (before) |
|---:|---:|---:|---:|
| 0 | 0 pt | 24.00 | 24.00 |
| 4 | 0.5 pt | 24.50 | 24.00 |
| 8 | 1 pt | 25.00 | 24.00 |
| 16 | 2 pt | 26.00 | 24.00 |
| 24 | 3 pt | 27.00 | 24.00 |

That the gap tracks the border **exactly** is what makes this a rule rather than a constant. Both
corpus documents that show it draw a `w:sz="4"` grid, so "a flat half point" fits them just as
well and is refuted only here. `hRule="exact"` measures the other way — 24.00 on both sides at
`w:sz="16"` — so a clipped row's height really is the whole of it, and the obvious symmetry is
wrong.

This is the largest of the four by reach: **85 of the 200 words documents render differently**
because of it.

### 3. `w:cantSplit` is overridden for a row taller than a page

`SwTabFrame::Split`, `sw/source/core/layout/tabfrm.cxx`:1161 — *"A row larger than the entire
page ought to be allowed to split regardless of setting, otherwise it has hidden content and that
makes no sense"*. It compares against the page **body** print height, not the room left on the
current page, and it outranks `w:cantSplit` but not the exact-height or repeated-heading tests,
which is the ordering `MaySplit` now has.

`ESPN-R - MCF - RA - Ed1.docx` has one such row, about 440 pt under a 424 pt landscape body. It
did not change the page count. It did remove the worst of a visible defect: that document drew
ink at **y = 597.0 on a 595.30 pt page**, and 8 landscape pages exceeded the reference's own
maximum of y = 522.5. Now 7 do, and the 597.0 page is gone. The reference has none.

### 4. A page break before a table was eaten inside the table

DOCX has no break-before on a table: the break sits on the empty paragraph in front of it as
`<w:br w:type="page"/>`, and the deferred flag was only ever *read* where a paragraph was read —
so a break landing in front of a table was consumed by the first paragraph inside the table's
first cell, where it means nothing. `PageTable` gained a `StartsNewPage`; the paginator's table
arm now makes the same test its paragraph arm makes.

LibreOffice's own import is the evidence: `ESPN-R - MCF - Manual` has two such paragraphs and
**zero** `w:pageBreakBefore` anywhere, yet its flat XML carries `fo:break-before="page"` on
`Table12` and `Table13`, the only two in the file.

This is the fix that bought both seats.

## What went wrong with the diagnosis, and it is worth writing down

**The brief's mechanism for the Holdover pair was measurably exact and bought nothing.** It says
the reference has 16.80 pt above its NOTES list where we have 10.80. That is right to the
hundredth: on page 20, which is content-identical on both sides, the NOTES-heading-to-first-note
gap read **10.81 ours / 16.81 reference**, and defect 1 above moves ours to **16.81** — a
0.00 pt residual. A blind `page-vision` reviewer, given no numbers, independently reported that
"the reference leaves a blank line's worth of space between the bold NOTES heading and the first
numbered note; ours leaves none", at NOTES and at CAUTIONS both.

And fixing it changed **not one page** on either document. 214 headings, 6 pt each, zero pages.

The 13-page gap was defect 2 all along. The pages the reference spills are in Appendix A, where
its trailing CAUTIONS bullet lands on its own 34-word page; our pages held one table row more,
because 30-odd rows at half a point short is one line. After the row fix the table on page 20
aligns to the reference within **±0.13 pt** where it was **−4.13**.

The lesson is the standing one in a new shape: a measurement that lands exactly is evidence that
you have measured something real, and no evidence at all that it is what you were looking for.

## Two documents got worse, and one of them badly

`FAA 2025-26 Holdover Tables.docx` went **154 → 185** against a reference of 167, where its twin
went 142 → **154** against 155. Same file family, same fix, opposite outcomes. Tracked by table
number, the two agree within ±2 through the whole main body and most of Appendix A, and then from
`TABLE ADJ-28` onward we emit one spill page per table for twenty tables. On those pages the
reference fits 641 words and we fit 622 — one row short, the mirror image of the bug just fixed.
The residual there measures **+2.88 pt** arriving as a single step rather than as a per-row drift,
so it is one block and not the row rule over-applying.

`150_5300_13_chg8.doc` (`words/table-001`, already open) went 20 → 21 against a reference of 18.

Both were already failing. Neither is a passing document regressed. But 154 → 185 is worse by the
gate's own arithmetic than the 154 it started at, and calling this round a straightforward
improvement for that document would be false.

## Reach, measured from what resolves

All 200 `words` documents rendered twice — once with this branch, once with a binary built from
`HEAD` — and diffed. Not grepped.

- **85 of 200** renderings change at all.
- **6 of 200** change page count.
- 4 move closer to the reference, 2 further.

```
150_5300_13_chg8__doc                 20 -> 21   ref 18   worse   (table-001)
24-25_FAA_Holdover_Tables__docx      142 -> 154  ref 155  better
EHEST-SMS-Safety-Management-V2__docx  79 -> 80   ref 82   better  (pagination-002)
ESPN-R - MCF - Manual__docx           34 -> 35   ref 35   MATCH
FAA 2025-26 Holdover Tables__docx    154 -> 185  ref 167  worse
FO.FCTOA.00010 …__docx                15 -> 16   ref 16   MATCH
```

The gap between 85 and 6 is the honest summary of the round: most of what was fixed is
sub-page geometry the page-count gate cannot see, and `EHEST-SMS` moving is a free
half-fix for whoever owns `pagination-002`.

## Two defects found blind and left unfixed

A `page-vision` reviewer given the two Holdover documents and no numbers found both.

1. **Our NOTES list numbering runs away.** On page 40 we emit `1, 196, 197 … 205` where the
   reference emits `1 … 11`; on page 20, `1, 12 … 21`. Confirmed in the text layer, not the
   raster. Item 1 is right and every item after it is wrong, so the superscript markers inside
   the table no longer point at the notes they name. Still present after all four fixes —
   re-checked on the final binary. It does not move the gate, because the labels are the same
   *count* of tokens.
2. **Content overflows into the footer.** An orphaned repeat of a table's header block runs past
   the body area, its cell borders crossing the footer rule and striking through "Page 73 of 87".
   Confirmed at 200 dpi on a crop rather than on the downscaled pair. Related to defect 3 above
   and not cured by it.

## Prediction, scored

`prediction.md` was committed before any sweep of the fixed binary.

| | verdict |
|---|---|
| **P1** Holdover pair both match | **Wrong**, and wrong about the cause. One landed at −1, the other at +18, and the mechanism I predicted was worth zero pages. |
| **P2** no `done-*` regression | **Right**, including the part I called low-confidence. `PES-Technical-Report` never moves, because its parent `List Paragraph` leaves the branch additive. |
| **P3** the other eight do not move | **Wrong**, favourably. Two of them matched, from two defects I had not yet found when I wrote it. |
| **P4** reach is 4 changed, 2 improved | **Wrong on magnitude.** True of defect 1 alone; the round reaches 85 of 200. |
| **P5** both blind-reviewer defects survive | **Right**, verified on the final binary. |
| **P6** fidelity no worse than 30 of 550 | **Right**, exactly 30 of 550, 0 skipped, 550 discovered. |

Two of six right, one right-in-my-favour, and the headline prediction wrong in both outcome and
cause. The prediction that held best is the one I argued from a structural property of the change
(additive) rather than from a size estimate.

## Tests

New, all verified failing against a tree built from `HEAD` and passing on this branch:

- `OneSidedStyleSpacingBuiltInChildTests` — 5 tests, 1 fails unfixed (the other 4 are controls
  that must not move). Fixture
  `tests/corpus/features/style-one-sided-spacing-builtin-child.docx`, authored *and* read back
  through `soffice` by `make-builtin-child-corpus.py`, so the file and its expectations come from
  one script.
- `TablePaginationRulesTests` — 10 tests, **6 fail unfixed**: four of the five border-sweep rows
  (the zero-border row is the control and passes either way), the cantSplit override, and the
  break-before-table.

Every project run individually:

| project | result |
|---|---|
| Core | 337 passed, 0 failed |
| Containers | 109 passed, 0 failed |
| Text | 349 passed, 0 failed |
| Vector | 295 passed, 0 failed |
| Rendering | 150 passed, 0 failed, 1 skipped |
| Markup | 259 passed, 0 failed |
| OpenDocument | 125 passed, 0 failed |
| Spreadsheets | 832 passed, 0 failed |
| Presentations | 694 passed, 0 failed |
| WordProcessing | 865 passed, 0 failed |
| **Fidelity** | **520 passed, 30 failed**, 0 skipped, 550 discovered |

`XRay.Office.Vector.Tests` reported 295 passed on every run here; the intermittent phantom failures
recorded in `CLAUDE.md` did not appear.

## Not attempted, said plainly

Six of the ten: `A_320.doc` (+23, the group's largest and the brief's blind spot), `1447.doc`,
`absrc-pac-01-info-note-en.doc`, `report-template.docx`, `template---tpr-…` and the remaining
half of `ESPN-R - MCF - RA`. What is known about four of them, from a first-divergence pass:

- `report-template.docx` (19/20) — diverges at page 12, where the reference moves a whole
  `GSTableCaption` + table onto the next page and we keep it. A keep-with-next question.
- `template---tpr-…` (8/7) — diverges at page 3, and we are the *longer* one, which puts it in
  the opposite family from most of this group.
- `absrc-pac-01-info-note-en.doc` (6/7) — the two extractions disagree in *order* on page 1, so
  the divergence is frame or text-box ordering before it is pagination.
- `1447.doc` — the brief identifies this as the line-height law, a specification for a future
  round. Not touched, and the 75-point table in `words-pages-01/results.md` was not refitted.

## The one thing to take into the next round

`FAA 2025-26 Holdover Tables` from `TABLE ADJ-28` onward. It is a **+2.88 pt step, arriving in one
block**, on a document whose geometry is otherwise correct to a tenth of a point, and it is worth
twenty pages. That is a far better-conditioned target than anything left in this group.
