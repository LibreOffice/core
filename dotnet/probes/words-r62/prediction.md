# words-r62 — prediction, committed before any behavioural change

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r62`, base
`337bc9fe17c`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; sweep `TMPDIR` on the host mount.

## Baseline, reproduced before anything was written

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 338 MISMATCH 17 REF-CANNOT-RENDER 0`, scored
against `MANIFEST.tsv`'s own 337-path list: **321 of 337, zero disagreements with the manifest's
status column, document for document.** The 18 extra rows are the case-insensitive mount's alias
entries. `/` was at 71 % throughout and the sweep's `TMPDIR` was
`/c/sandbox/workdir/scratch-r62-words/tmp`.

## The change

**Split a positioned (fly-held) table across a page.** 26.2.4.2 marks every DOCX floating table's
frame splittable without exception — `DomainMapperTableHandler.cxx`:1765, *"A text frame created
for floating tables is always allowed to split"* — and the continuation is laid out at the top of
the next page's text area. `PlaceFloatedTable` places such a table whole and its remarks say why:
*"Writer's fly-held table does split across pages; nothing here can, so floating one that does not
fit would draw it off the bottom of its page and lose the rest."* The machinery to cut a table at a
page boundary already exists — `PlaceTablePart` takes `from`, `drawn` and `room` and returns the
row the next page resumes at — so what is missing is the *carrying*, not the cutting.

The reference's own geometry on `012`, read out of its content stream and not inferred:

| | reference draws |
|---|---|
| page 1, row 1 top | `y = 128.10` from the sheet's top — the 72 pt margin plus `w:tblpY="1122"` = 56.1 pt |
| page 1, rows | eight, at 128.10, 179.00, 229.40, 279.80, 330.20, 380.60, 431.00, 481.40; the last ends 531.75, and the body ends 540.00 |
| page 2, row 9 | `12.40 489.65 99.95 50.35 re f*` — top edge at **72.00 from the sheet's top, the top margin exactly** |

So the continuation's offset from the top of the next page's text area is **nought**, and that is
what will be implemented.

**The tall-table guard is deliberately kept.** `if (height > area.Height) return false` leaves a
table taller than a whole column in the flow. It is not what Writer does, and this round does not
change it — see the census below for the two documents that would move if it were dropped, both of
which pass today.

## What is predicted to change

| document | pages now | pages predicted | reference | verdict |
|---|---:|---:|---:|---|
| `012_Project_Timeline_Template_Black_and_Brown_Theme_35c76550.docx` | 1 | **2** | 2 | `pages` → **match** |
| `015_Project_Timeline_Template_Colored_Background_6434b0e8.docx` | 1 | **2** | 2 | `pages` → **match** |

**Verdict movement predicted: +2, from 321 to 323 of 337. Regressions predicted: 0.**

Changed renderings predicted: **2 to 6** of 355. Two are certain; the band allows for a positioned
table whose *content-sized* rows overflow where its declared ones do not — which the census below
cannot see.

Extractable words: predicted unchanged on every document. Both target tables' overflowing rows are
**empty**, which is why these two documents already read 49/49 and 50/50 words against the
reference. If a word count moves anywhere, the change did something other than what is claimed.

## The census, and what it cannot see

`floattable-census.py`, over the manifest's 271 `.docx`:

```
documents holding a positioned table :   40 of 271 .docx
positioned tables                    :   46
vertAnchor                           : {'text': 29, 'page': 17}
tables whose declared rows overflow  :    4 in 4 documents
```

The four, by declared overflow:

| overflow | rows | document | today |
|---:|---:|---|---|
| 994.15 pt | 123 | `words/pagination-001/docx/ESPN-R - MCF - RA - Ed1.docx` | **passes**; taller than a column, so the guard leaves it in the flow — **not predicted to move** |
| 42.20 pt | 9 | `words/chartset-008/docx/012_…` | fails on pages — **predicted to move** |
| 25.55 pt | 71 | `words/done-005/docx/part-147_approval list_20230119.docx` | **passes**; 782 pt against a 714.30 pt body, so the guard leaves it in the flow — **not predicted to move** |
| 19.15 pt | 14 | `words/chartset-011/docx/015_…` | fails on pages — **predicted to move** |

**What the census cannot see, written down before the sweep:**

* **A row that sizes to its content.** Height is taken from `w:trHeight` where stated and from a
  240-twip floor otherwise, so every table whose rows grow to fit their text is *under*-counted.
  This is the largest blind spot and the reason the changed-rendering band is 2–6 rather than 2.
* **Where a `vertAnchor="text"` table actually sits.** The census assumes the flow above it is
  empty, so a table half way down a page is under-counted. 29 of the 46 are `text`.
* **Header, footer and text-box tables**, and tables nested in a cell — none are read.
* **The other readers.** Only `.docx` is scanned. The 66 `.doc` paths, and every ODF text document,
  are invisible to it; whether their readers produce a positioned table at all is not asserted here.
* **Style-borne `w:tblpPr`.** Only a `w:tblPr/w:tblpPr` written on the table itself is counted; a
  table style stating one would be missed.

## Blind spots in the gate, for the same change

The gate reads page count, extractable words and font embedding. It cannot see:

* whether the continuation row is drawn **in the right place** on the new page — only that a page
  exists. A continuation drawn at the wrong offset would score identically. This is checked
  separately, against the reference's own `re` operator, and by a blind page reading.
* the 56 fill operations `012`'s page 1 is missing (75 against our 19), which no gate column reads.

## The other items, and what is predicted for them

* **`012`'s missing fills and its white `COL_AUTO` title** — a rule to be *established*, not
  implemented. Round 59 removed a frame-fill arm that turned 383 glyphs white which the reference
  draws black; round 61 found one witness pointing the other way. **Nothing ships here unless a
  probe separates the two directions.** Predicted verdict movement from this item: **0**.
* **`097`'s remaining 1.65 pt** — a body paragraph holding an inline image. Predicted verdict
  movement: **0**; `097` already matches.
* **The `.doc` label slant** and the Carlito class — not predicted to be reached this round.

## Falsification

If `012` and `015` close but the continuation row's top edge on page 2 is not **72.00 pt** from the
sheet's top on both, the page count was bought and the placement was not. If any word count moves,
the change did something other than split an empty row off the bottom of a page.
