# words-r51-flies — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, corpus `/c/sandbox/workdir/sample-files` at `5fd4b17`,
worktree `wt-words-r50` on branch `wt-words-r51`, base `6798de946ce`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

Read `prediction.md` and `prediction-2.md` beside this first. Both were committed before the sweep
that measured them — `74ec384ef70` before anything was changed at all, `8705ab9e479` before the
third fix was swept.

## Scoreboard

| | words |
|---|---|
| baseline (`MANIFEST.tsv` status column, reproduced) | **309 / 337** |
| after fixes A and B | 311 / 337 |
| after fix C | **316 / 337** |
| gains | **7** |
| regressions | **0** |

### Baseline reproduction

`batch-check.sh … 'words/*' … 8` reported `TOTAL 355  MATCH 324  MISMATCH 31`. **That total is
wrong, and the brief said it would be**: 355 rows for 337 documents, the extra 18 being upper-case
alias directory entries earlier rounds' `look.py` runs materialised on this case-insensitive mount.
Scored against `MANIFEST.tsv`'s path list instead: **309 match, 28 open, and 0 disagreements with
the manifest's status column, document for document**.

`look.py` is fixed in this branch — it now walks directory entries and matches case-insensitively
rather than probing `stem.EXT` by stat. Seven `pair.sh` runs and one `look.py` run later the words
tree still holds 355 entries, so no new alias was created. The 18 that exist are unchanged; nothing
can remove them safely, since they are the same inodes.

## Prediction against measurement

| | predicted | measured |
|---|---|---|
| fixes A + B | **+2**, to 311 | **+2**, to 311 |
| fix C | **+5**, to 316 | **+5**, to 316 |
| regressions | 0 | **0** |

Both named documents for A and B moved and nothing else in the corpus changed **even in its page or
word numbers** — 335 of the 337 rows were byte-for-byte identical between the two sweeps.

For C, all five predicted documents moved and both of the named at-risk ones — `078` at zero slack
and `026` at two — moved *towards* the reference rather than away, to exact agreement. `008` behaved
exactly as predicted: it improved from 57 to 66 raw words against 70 and stayed open, 4 short of a
band of 3.

## Verdict movement, per document

| document | batch | before | after |
|---|---|---|---|
| `057_Organogram_Template_Vertical_Colorful_Theme` | `chartset-006` | `words` 1/1, 21/36 | **`match`** 36/36 |
| `025_Unit_Circle_Chart_Cos_and_Sin_Model` | `chartset-007` | `words` 1/1, 121/141 | **`match`** 141/141 |
| `030_Unit_Circle_Chart_Points_System` | `chartset-009` | `words` 1/1, 101/114 | **`match`** 114/114 |
| `026_Unit_Circle_Chart_Four_Quadrants` | `chartset-010` | `match` 98/99 | `match` **99/99** |
| `056_Organogram_Template_Square_Theme` | `chartset-010` | `words` 1/1, 24/56 | **`match`** 56/56 |
| `068_Work_Breakdown_Structure_Template_Green_Theme` | `chartset-011` | `words` 1/1, 19/86 | **`match`** 85/86 |
| `008_Free_Genogram_Diagram_Template_Green_and_Yellow` | `chartset-012` | `words` 57/70 | `words` **66/70** — still open |
| `071_Storyboard_Template_Cartoon_Theme` | `chartset-014` | `words` 1/1, 6/21 | **`match`** 21/21 |
| `078_Storyboard_Template_Pink_and_Gray_Theme` | `chartset-014` | `match` 51/54 | `match` **54/54** |
| `AFS-050-004-F2_0i` | `done-014` | `words` 8/8, 2503/2228 | **`match`** 2228/2228 |

**Nothing else moved.** The other 327 documents are identical in verdict, page count and word count
across the base and final sweeps, `done-*` batches included — the sweep was the whole family, not a
selection, so the `done-*` re-sweep is inside it.

## The three changes

### A. A positioned table's continuation re-floated the whole table

`Paginator.Fill`'s table arm is re-entered on every page a table touches: a split table carries on
with `paragraphIndex` still on it and `lineIndex` at the row that did not fit. `PlaceFloatedTable`
reads neither, so on the continuation page it floated the table **again, from row 0, entire**, on
top of the part already drawn. Guarded to `lineIndex == 0 && rowDrawn == Length.Zero`, which is the
same qualification the `StartsNewPage` test three lines above it already carries.

`AFS-050-004-F2_0i.docx`: 2503/2228 → **2228/2228**, 8/8 pages, and a token multiset diff that is
empty in both directions.

### B. A `v:group` inside a `v:group` was dropped, and everything inside it with it

`DocxVmlFrames.Group` had `if (member.Name.LocalName is "group") continue;`. It now recurses: a
nested group's `left/top/width/height` are bare numbers in the parent's `coordsize` space and
resolve to a rectangle by the arithmetic the flat case already does, and that rectangle is the
child's own origin and extent.

`068_Work_Breakdown_Structure_Template_Green_Theme`: 19/86 → **85/86**.

### C. A nested DrawingML group's own `a:off` was dropped

`GroupTransform.Composed(inner)` added `inner.ShiftX`, and `TransformOf` never sets a shift on any
path — it returns `0, 0` always. So a nested `a:grpSpPr/a:xfrm/a:off` was discarded and every nested
group's members were laid out as though the group sat at its parent's own origin. **The scale
composed correctly throughout**, which is why the members came out the right size in the wrong
place — the hardest form of this defect to see, and why five rounds of ink metrics never named it.

`Around(group, inner)` replaces it, mapping the nested group's own offset through the enclosing
transform exactly as a leaf's is mapped.

Raw `pdftotext | wc -w`, ours before → after against the reference:

| document | before | after | reference |
|---|---:|---:|---:|
| `056_Organogram_Template_Square_Theme` | 24 | **56** | 56 |
| `057_Organogram_Template_Vertical_Colorful_Theme` | 21 | **36** | 36 |
| `025_Unit_Circle_Chart_Cos_and_Sin_Model` | 126 | **141** | 141 |
| `071_Storyboard_Template_Cartoon_Theme` | 11 | **41** | 41 |
| `030_Unit_Circle_Chart_Points_System` | 107 | **116** | 118 |
| `008_Free_Genogram_Diagram_Template_Green_and_Yellow` | 57 | **66** | 70 |

## Refutations

### 1. The brief's item 1 — "the wrap" — is not what `AFS-050-004-F2_0i` was

The brief, and round 50's own results, said: *a body fly takes Writer's parallel surround, we place
flies correctly and never push text clear of them, and the witness is `AFS` page 3 — 364 words
against the reference's 53, 318 extra tokens and none missing.*

The 318 reproduce exactly. **They are the same positioned table drawn twice.** Four independent
measurements, taken before anything was changed:

- **Not one of the 318 is a string the reference never draws.** Every one is a repeat.
- Our page 2 against our page 3, as token multisets: **five tokens only on page 2** — the heading
  `IASA Checklist Section Assignment` and the page number — and one only on page 3, its page number.
- An authored variant with all four `w:tblpPr` elements deleted and nothing else changed renders
  **8 pages and 2384 words**, the reference's own raw total to the token, with page 3 falling from
  364 to 46.
- Traced through `Fill`: `FLOW block=23 page=1 from=0 to=36 placed=True`, then
  `FLOAT block=23 page=2`, with no `MoveTrailingGroupToNextPage` between them.

There is now **no witness in this corpus for a missing text wrap around a body fly**, and
implementing one would change all ten graph papers and five timelines with nothing asking for it.
`RunsIntoTheFly` — which declines to float when the following flow would run into the fly — is the
model that produces the reference's page counts on all fifteen, and it still does.

### 2. `MaxGroupNesting = 8` is not why the organogram templates lose their shapes

This was the other standing hypothesis and it is the cheaper one to test, so it was tested first.
`056`'s deepest chain is ten containers — `wgp > grpSp ×6 > grpSp > grpSp > wsp` — so the bound
genuinely bites. Raising it from 8 to **64** changes the word count of `056`, `057`, `025`, `030`,
`008` and `071` by **zero**, every one of them. The bound is real and costs nothing; the offset was
the whole defect.

### 3. The Work Breakdown three are not one class, and six of the seven "`wpg:wgp` witnesses" are not witnesses

The brief grouped `068` with `065` and `069`, and named `057`, `025`, `030`, `008` and `071` as
carrying a `wpg:wgp` and therefore being the same defect with seven witnesses. Measured:

- `065`, `068` and `069` hold **no DrawingML shapes at all** — they are pure VML. And `065` and
  `069` hold **no nested `v:group`**; only `068` does. The three are not one class on this axis.
- **Exactly one** words document in 337 holds a nested `v:group` that this reader reaches: `068`.
  `056`, `057`, `025`, `030`, `008` and `071` each hold 19–40 nested VML groups and every one is
  inside an `mc:Fallback`. An authored variant settles that we do not read them: `056` with its
  entire `mc:Fallback` deleted renders **24 words, exactly what the unmodified document renders**.
- They were nevertheless the same *shape* of defect one layer up, in the DrawingML reader — which
  the round-50 blind reviewer had named (*"the group's child-offset/child-extent transform not
  being mapped onto the parent's"*) and which took a third instrument to find.

## What the blind readings were worth this round

Seven pages went to seven fresh subagents, each forbidden to read any project file or run any
command, each asked to describe the halves separately before comparing.

- **`068`, before**: *"the surviving items are exactly the top two levels of the tree, in tree
  order, not a random scatter and not a partial-column truncation … there is no piling up and no
  overlapping; the failure is omission."* That is fix B, from the page, and it agrees with round
  50's reviewer, who proposed the depth discriminator itself.
- **`056`, before**: *"only 5 blue leaf boxes survive, and they are piled into the left edge of the
  page, a single vertical stack … the remaining 20 leaves are absent as boxes; 4 of them survive as
  naked text with no rectangle."* That is fix C, and the reviewer's "piled into the left edge" is
  precisely a lost `a:off`.
- **`AFS` page 3, before**: correctly reported that ours carries the *entire* checklist table where
  the reference carries only its final row, and that ours draws its footer *through* the table's
  last cell. It did not — and could not — see that the table was drawn twice; it named the
  candidates and asked for page 2, which is exactly the measurement that settled it.
- **`056` and `068`, after**: both confirm the fixes and hand over the next defect. `056` now draws
  26 boxes in the reference's own 5 × 5 lattice, columns matching to a few pixels — and **draws none
  of the ~15 connectors** the reference draws. `068` now draws all 41 labels on the right grid and
  **0 of 41 box outlines and fills, and none of the 41 connectors**.
- One reading was a **warning about the method**, and it is the same one round 50 recorded: `097`'s
  reviewer measured every gap on the page to a few pixels and got them all right, while `028`'s
  reviewer reported our legend as 16 entries in 2 rows against the reference's 12 in 3. Counts of
  many similar items are not reliable from an image; positions and directions are.

## Left open, in the order the next round should take it

1. **VML shapes are drawn with no fill and no stroke.** `068` now puts all 41 labels in the right
   place and draws **0 boxes and 0 connectors**; `065` (28/41) and `069` (108/117) are the same
   family and both still open. Its `v:rect`s carry `fillcolor="#e2efd9 [665]"` and
   `strokecolor="#70ad47 [3209]"` — a theme-indexed VML colour — and its `v:shape`
   `type="#_x0000_t32"` connectors are straight-line geometry we do not paint. No gate column sees
   any of it, which is why it is first: it is the largest thing that is visibly wrong on a page that
   now passes.
2. **DrawingML connector shapes are not drawn either.** `056` is now word-exact and still missing
   the ~15 lines its reviewer counted. Its connectors are `wsp` with a zero `cx` or `cy` extent —
   `ext cx="0" cy="3834765"` — and `DocxFrames.Leaf` rejects any member whose mapped rectangle has
   `Width <= 0 || Height <= 0`. That predicate is the suspect and it is one line.
3. **`008_Free_Genogram_Diagram_Template_Green_and_Yellow`, 66 against 70**, is the only member of
   the group family still open and is now 4 words from the band. It holds 5 `wpg:wgp` and 85
   `grpSp`, the most of any words document.
4. **Chart data labels — 4 documents, and now the largest word gaps on the track.** `028` is
   191/327, `027` 261/378, `029` 107/114, `024` 95/105. This round's blind reading of `028`
   transcribed both halves: the reference's labels read `Branch 1 Stem 2 Leaf 5 / 15%` where ours
   read `Branch 1`, our legend repeats `Branch 1` seven times where the reference gives twelve
   distinct names, and **our chart carries no percentages at all** — a `c:multiLvlStrRef` taken at
   its first level plus an unread `showPercent`.
5. **The three `pages 1/2` documents — `097`, `012`, `015` — and this round has their mechanism
   half-measured.** The reference's page 2 is **blank in all three**: zero word elements, and zero
   non-white pixels at 30 dpi on `097` and `015` (21 pixels on `012`). All three end with a table
   followed by the mandatory empty `w:p`, and the classic cause is that paragraph having nowhere to
   go. On `097` the arithmetic supports it: the reference's lowest ink on page 1 sits at 758.2 pt
   against our 738.7, the body bottom is 770.4 pt, and an empty 11 pt Cambria paragraph needs about
   12.7 pt — so the reference has 12.2 pt of room and we have 31.7 pt. **Our table is about 20 pt
   too short**, and `097`'s blind reviewer measured where: the reference leaves ~78 px above both
   the History and the Approvals tables where we leave ~30–35, and we leave ~60 px more than it does
   under the `Document Control` band. `012` and `015` do **not** share that shape — `012`'s lowest
   ink is identical on both sides — so they are at least two causes, not one.
6. **A positioned table taller than its column is still stacked** rather than split in a fly. No
   corpus document fails on it.
7. `metrics-001` remains untouched, per the brief.

## Proposed `MANIFEST.tsv` reclassification

`/c/sandbox/workdir/sample-files` is a separate checkout and was **not** committed to from here.
Seven rows for the parent to apply, `status open` → `status done`:

```
words  chartset-006  words/chartset-006/docx/057_Organogram_Template_Vertical_Colorful_Theme_724b143c.docx
words  chartset-007  words/chartset-007/docx/025_Unit_Circle_Chart_Cos_and_Sin_Model_1ed01892.docx
words  chartset-009  words/chartset-009/docx/030_Unit_Circle_Chart_Points_System_23b5ef07.docx
words  chartset-010  words/chartset-010/docx/056_Organogram_Template_Square_Theme_38f373d4.docx
words  chartset-011  words/chartset-011/docx/068_Work_Breakdown_Structure_Template_Green_Theme_63e69ed8.docx
words  chartset-014  words/chartset-014/docx/071_Storyboard_Template_Cartoon_Theme_ae113de2.docx
words  done-014      words/done-014/docx/AFS-050-004-F2_0i.docx
```

`008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme` stays `open / text` at 66/70.

## Files

- `prediction.md` — committed at `74ec384ef70`, before anything was changed.
- `prediction-2.md` — committed at `8705ab9e479`, before the sweep that measured fix C.
