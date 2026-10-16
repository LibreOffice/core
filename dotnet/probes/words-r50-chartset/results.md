# words-r50-chartset — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
corpus `/c/sandbox/workdir/sample-files`, worktree `wt-words-r50` on base `ac147b7e5bb`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Read `prediction.md` beside this first; it was
committed at `26ec9f0727a`, before anything was changed.

## Scoreboard

| | words |
|---|---|
| baseline (`MANIFEST.tsv` status column, reproduced) | **300 / 337** |
| after | **309 / 337** |
| gains | 10 |
| regressions | 1 |

Baseline reproduction: `batch-check.sh … 'words/chartset-*' … 6` gave
`TOTAL 137  MATCH 110  MISMATCH 27  REF-CANNOT-RENDER 0`, and the 27 mismatching paths were
**exactly** the 27 rows the manifest marks `words / chartset-* / open`. Document for document,
not just in total.

## The prediction against the measurement

Predicted **+5** (086, 088 from fix B; 080, 084, 089 from fix A), explicitly declining to
predict the six `Project_Timeline_Template` documents on the grounds that five of them fail on
`text` and "a page-count change alone will not close that".

Measured **+10 −1 = +9**. The prediction was right about all five named documents and **wrong
about the reason it excluded the timelines**:

| document | before | after |
|---|---|---|
| `084_…Editable_Layout` | `pages` 2/1 | match |
| `086_…Gray_Theme` | `pages` 2/1 | match |
| `088_…Quality_layout` | `pages` 2/1 | match |
| `080_…Black_Theme` | `pages` 2/1 | match |
| `089_…Simlpe_Format` | `pages` 2/1 | match |
| `011_Project_Timeline_Template_Beautiful_Theme` | `text` 41/158 | match, **158/158** |
| `013_Project_Timeline_Template_Blue_Background` | `text` 91/98 | match, 98/98 |
| `016_Project_Timeline_Template_Complete_Guide` | `text` 153/159 | match, 159/159 |
| `017_Project_Timeline_Template_Customizable_Format` | `pages,words` 2/1, 72/89 | match, 86/89 |
| `018_Project_Timeline_Template_Editable_Format` | `text` 102/108 | match, 108/108 |
| `AFS-050-004-F2_0i.docx` | match | **`words` 2503/2228** |

**The words were missing because they were falling off the page.** 011 gained 117 words from a
change whose whole content is where a table is drawn — the brief's own guess that "one cause is
being seen through two gate columns" was right, and my prediction talked myself out of it. The
lesson to carry: a `text` verdict on a one-page document is not evidence that the defect is in
the text; a one-page document has nowhere to put what it drops.

## What was changed

### Fix B — a paragraph holding only floating drawings is sized by its mark

`DocxLayoutSource` emits one `AnchorCharacter` (U+0001) per `w:drawing`, floating or inline, so
`walker.Text.Length == 0 ? mark : body` took the **body** style for a paragraph whose only
content was a `wp:anchor`. Writer puts such a drawing in a fly and leaves the paragraph empty,
so the mark is what sizes it. Where the mark states a smaller size than `docDefaults`, we made
the paragraph several times too tall.

Eleven authored variants of `088_…Quality_layout`, one variable at a time, rendered through both
stacks. The four that carry the argument:

| variant | ours | ref |
|---|---:|---:|
| A original — anchored drawing, mark `w:sz="4"` (2 pt) | 2 | 1 |
| B drawing run deleted, mark 2 pt | 1 | 1 |
| C drawing kept, mark raised to `w:sz="22"` | 2 | 2 |
| D drawing deleted, mark 11 pt | 2 | 2 |

C and D **refute** "the mark's size is ignored for an empty paragraph": it was always honoured.
A and B isolate the cause to the drawing run alone. Seven more — `posOffset` 0, −900000 and
−266065, `wp:extent cy` cut to 9525, `behindDoc="1"`, `wrapNone`→`wrapSquare`,
`relativeFrom="paragraph"`→`"page"` — all stay 2/1, so **no property of the frame is involved**.
After the fix all eleven agree with the reference, and C and D still correctly take two pages.

Reach census: 37 of 271 corpus DOCX-family documents hold at least one such paragraph, 17 with
an explicit `w:pPr/w:rPr/w:sz`.

### Fix A — a positioned body table is a fly, not a block in the flow

`w:tblpPr` makes the table a fly in Writer (`TablePositionHandler::getTablePosition`).
`FlowLayouter` has floated one in a running head since round 44 and its remarks said of the
body: *"no measurement was taken there."* This is that measurement.

**The position law.** Predicting the reference's first horizontal table rule from the page
geometry and `w:tblpY`/`w:vertAnchor` alone, over the eight positioned graph-paper templates:

| doc | anchor | tblpY | predicted | reference | ours before | ours after |
|---|---|---:|---:|---:|---:|---:|
| 080 | page | 1786 | 752.59 | 751.84 | 769.40 | 752.10 |
| 084 | page | 1025 | 790.64 | 790.09 | 769.40 | 790.15 |
| 089 | page | 1741 | 754.84 | 754.54 | 769.65 | 754.60 |
| 082 | page | 1513 | 766.24 | 765.44 | 769.40 | 765.75 |
| 085 | page | 1606 | 761.59 | 760.44 | 769.27 | 760.98 |
| 087 | page | 1025 | 790.64 | 789.84 | 769.46 | 790.21 |
| 083 | (none→text) | 525 | 743.40 | 743.34 | 769.65 | 743.40 |
| 081 | (none→text) | 579 | 494.35 | 491.15 | 520.41 | 491.46 |

Seven of eight within 1.15 pt of the reference before the change (081 was 3.2 pt out by hand
arithmetic); **all ten within 0.55 pt after it**, 081 included, so the hand residual was mine
and not the law's.

**But the position is not what costs a verdict — the flow is.** On 080 both sides draw the
identical 86 strokes on page 1: the table fits either way. The reference then draws the
document's four remaining texts on page 1 at y = 814.29 / 783.09 / 765.94 and its logo with
them; we drew the same four texts and the same logo on **page 2, at y = 814.30 / 783.70 /
765.95** — the same offsets, one page later.

Two guards, each measured:

- **A table taller than its column stays in the flow.** Writer's fly-held table splits across
  pages; nothing here can, so floating one that does not fit would draw it off the bottom and
  lose the rest. This is a guard, not the rule: a positioned table longer than a page is still
  laid out wrongly and no gate column shows it.
- **A fly over the point the flow has reached does not swallow a line with ink.** 084 and 087
  both anchor their grid *above* the top margin, so the fly covers the flow's starting position
  in both. 084's following flow is one empty paragraph and 26.2.4.2 gives **one** page; 087's is
  an empty paragraph then a `Title: ___ Date: ___` line and it gives **two**. Emptying 087's two
  text runs and re-rendering brings it back to one — one variable, and the answer follows it.
  Without this guard the sweep was 307/337 with four regressions; with it, 309/337 with one.

## The one regression, and its mechanism

`AFS-050-004-F2_0i.docx`, `done-014`: **8/8 pages, 2503 words against 2228.** Page counts are
now *right* (they were 6/8 in the unguarded run) and the word count is over the band.

Per page, ours against the reference: `505 368 364 412 363 372 258 60` against
`505 361 53 412 323 383 254 93`. **Page 3: ours 364, the reference 53.** A multiset diff of the
tokens gives **318 extra in ours and 0 missing** — we draw everything the reference draws and
318 words more, all of them the chapter headings of one of its four positioned tables.

The cause is the half of the model that is deliberately not implemented: this document's flow
starts *clear* of the fly, so the guard lets it float, and then the flow grows *into* the fly
and is drawn straight through it. Writer wraps it past. Fixing this is the wrap, and the wrap is
the next round's first move (see below).

## Refutations

1. **"The graph-paper five are one defect with five witnesses."** Refuted. They are **two**
   defects, three witnesses and two, and the five *passing* siblings are what separate them:
   082, 085 and 087 carry the same `w:tblpPr` and pass, so the position alone costs nothing.
   086 and 088 carry no positioned table at all and fail for an unrelated reason. Working the
   cluster was still right — it is what produced the control — but the cluster was not a cause.
2. **"A chartset word failure is a tokenisation ceiling."** Refuted for 18 of 19. The
   whitespace-stripped charstream comparison (COMMON.md §3, script `charstream.py` beside this)
   was run on every chartset word-count failure: **18 have genuinely different characters**.
   Only `069_Work_Breakdown_Structure_Template_Professional_Format` has the same characters —
   the same 371, as a multiset — and it is a real ceiling; see the reclassification below.
3. **"The paragraph mark's `w:sz` is ignored for an empty paragraph."** Refuted by variants C
   and D, on both stacks.
4. **"Some property of the anchored frame — its offset, extent, wrap or paint order — decides
   whether it costs a page."** Refuted by seven variants; only the existence of the drawing run
   matters.
5. **A trap, not a claim, and it cost me one sweep.** `look.py` searches the corpus with
   `rglob(f"{stem}.{ext.upper()}")`. On this case-insensitive host mount that lookup materialises
   an **alias directory entry** — same inode, `cmp` identical, `git status` clean, `git ls-files`
   sees one file. Every later directory glob then sees the document twice. My chartset baseline
   swept 137; after six `pair.sh` runs the same tree swept 143, and `words/*` swept **355 rather
   than 337**, double-counting 18 documents. Nothing was written and nothing needs cleaning, but
   a sweep total taken after a `look.py`/`pair.sh` run is inflated and its per-document rows are
   duplicated. **Score against `MANIFEST.tsv`'s path list, not against the sweep's own TOTAL.**
   CLAUDE.md's note that "four files are upper-case on disk" is very likely the same artefact
   seen from the other side; 12 of the 18 aliases predate this round and correspond to documents
   earlier rounds ran `look.py` on.

## Proposed `MANIFEST.tsv` reclassification

`/c/sandbox/workdir/sample-files` is a separate checkout and was **not** committed to from here.
One row, for the parent to apply:

```
words  chartset-013  words/chartset-013/docx/069_Work_Breakdown_Structure_Template_Professional_Format_1e02dce1.docx
    -  status open  kind text
    +  status open  kind ceiling
```

The evidence, and it is decisive rather than suggestive:

- `pdftotext` of both PDFs, all whitespace stripped: **371 characters on each side, and the same
  371** (equal as a multiset; the order differs).
- The *only* token difference is that **the reference splits `SUBTASK` into `SU` + `BTASK`, nine
  times**. `collections.Counter` diff: only-ours `{'SUBTASK': 9}`, only-ref `{'SU': 9,
  'BTASK': 9}`.
- That is the whole gap: 117 − 108 = 9, against a band of `max(2% × 117, 3) = 3`.

**Ours is the better output** — one token per word where the reference's PDF fragments it — and
`wc -w` scores it as failure. This is `TODO.raster-ceiling.md`'s class reached by a different
route (operator granularity rather than rasterisation), and it cannot be won by fixing anything.

## The blind readings, and what they are worth

Six pages were rendered with `pair.sh --page 1` and handed to six fresh subagents, each
forbidden to read any project file or run any command, each asked to describe the halves
separately before comparing. Two of the six independently produced the direction that led to
this round's fixes, before I had either diagnosis in hand:

- **080** — *"the reference has three text lines … ours has none. Ours is missing all of it —
  the space is blank white, not occupied by shifted content"*, plus the missing footer logo.
  That is fix A, seen from the page.
- **011** — *"the week grid is drawn first/highest, the title collides with grid rows 8–10, the
  lorem paragraphs are absent, only the clipped tops of 2 of 9 words appear at the page edge"*,
  and the reviewer's own separating measurement was *"render page 2 of ours; if the lorem
  appears there it is pure vertical displacement"*. Also fix A, and the reviewer named the right
  discriminator without being told anything.

The other four are unused leads and are recorded below because they are the next round's stock.
One of them is a warning about the method rather than a finding: **088's reviewer reported the
reference having ~29–30 columns against our ~24, and both grids have exactly 21 vertical rules
drawn to within 0.05 pt.** The skill says counts of many similar items are not reliable from an
image and this is that failure happening; everything else in that reading was right.

## Left open, in the order the next round should take it

1. **The wrap, and it is now the largest single item on this track.** A body fly takes Writer's
   default parallel surround, so text does not run through it — it is pushed clear. We now place
   the fly correctly and never push. The measurement is already banked: `AFS-050-004-F2_0i`
   page 3, reference 53 words against our 364, 318 extra tokens and 0 missing. Fixing it should
   recover that document and is a precondition for the positioned-table work being *right*
   rather than merely scoring better. Do it in the paragraph arm of `Paginator.Fill`: keep the
   floated fly's span per column, and when a line with ink would land inside it, move the flow
   to the fly's bottom. Then re-measure the ten graph papers and the five timelines, which is
   what it can break.
2. **Positioned tables taller than a page are still stacked** — the explicit guard in
   `PlaceFloatedTable`. Writer's fly-held table splits; ours cannot. Nothing in the corpus fails
   on it today, which is why it is second.
3. **Shape text inside groups: 3 documents, 20 to 67 words each, and a named discriminator.**
   `068_Work_Breakdown_Structure_Template_Green_Theme` is 19 words against 86. Its blind
   reviewer, with no access to anything but the image, reported the reference drawing 41 filled
   and bordered shapes plus the whole connector tree and ours drawing **0 boxes, 0 connectors
   and 6 of 41 labels** — and noticed that *"ours renders exactly the root plus the header row
   and nothing deeper — that is a suspicious cut line"*, proposing the test: **if every rendered
   item is nesting depth ≤ 1 and every missing item is depth ≥ 2, it is a recursion limit and
   not a fill/stroke problem.** `056_Organogram_Template_Square_Theme` (24/56) is the same
   shape from a different angle: the reference draws 25 boxes, we draw 5, and ours are piled
   into the top-left quadrant and overlapping each other — which its reviewer read as the
   group's child-offset/child-extent transform not being mapped onto the parent's. Both
   documents carry a `wpg:wgp`; so do 057, 025, 030, 008 and 071, all open on `text`. **That is
   seven witnesses to one candidate**, and `DocxFrames.ReadAll` already flattens groups, so the
   census to run first is: shapes declared against shapes emitted, by depth.
4. **Chart data labels: 4 documents, and the largest single word gaps left.**
   `028_Unit_Circle_Chart_Optimized_Graph` is 191 against 327. Its blind reviewer, comparing the
   two pies, reported that the reference's labels read `Branch 3 Stem 6 Leaf 14 / 15%` where
   ours read only `Branch 3`, that our legend repeats `Branch 1` seven times where the
   reference gives twelve distinct names, and that **our chart carries no percentages at all**.
   That is a multi-level category reference (`c:multiLvlStrRef`) taken at its first level, plus
   an unread `showPercent`. 027 (261/378), 024 (95/105) and 029 (107/114) are the same family,
   and `pie-chart-result.docx` (30/40) may be. Arithmetic on 028: 12 legend entries × 4 lost
   tokens + ~15 labels × 4 + ~15 percentages ≈ 123, against the 136 actually missing.
5. **The three `pages 1/2` timelines that did not move** — 097, 012, 015 — are the *opposite*
   sign from the graph papers (we are one page short, not one long) and were not investigated.
6. `metrics-001` remains untouched, per the brief.

## Files

- `prediction.md` — committed before any change, at `26ec9f0727a`.
- `charstream.py` — the whitespace-stripped charstream comparison, run over every word-count
  failure in the chartset batches. Takes `base/rows.tsv` and the `ours`/`ref` PDF trees a
  `batch-check.sh` run leaves behind.
