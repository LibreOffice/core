# r180 — fine-tuning the comparison tools

Three of the previous four rounds found the instrument rather than the tree. This round went
looking for the rest, and the first thing it found was that **the toolkit already encoded most
of the lessons and I had not been using it**: `wordsink-r174`'s `rank.py` reinvented
`track-ink-sweep.sh` with a worse measure, and `trace-text.py` — which rewrites a document so
every word is unique — already solves the repeated-label pairing trap that produced a phantom
55–264 pt displacement in `vmlgroup-r172`.

What follows is the defects that were really there, each measured before and after.

## 1. Only two of ten scripts honoured `REF_SOFFICE`, and the two binaries disagree

`/usr/bin/soffice` is 24.2.7.2; the tree is calibrated to 26.2.4.2. `batch-check.sh` resolves
`${REF_SOFFICE:-soffice}` and announces the version; `verdict.py` deliberately scores against
both. **Every other tool hard-coded `soffice` and said nothing** —
`track-ink-sweep.sh` (the *ink* sweep), `ref-baseline.sh` (which **banks** the reference half),
`corpus-parity.sh`, `look.py` (which `page-vision` tells you to use for every page reading),
`first-divergence.py`, `line-anatomy.py`, `lo-convert.sh` and `lo-extract.sh`.

So an ink sweep run beside a gate sweep measured a different binary, in silence.

**It is not hygiene — the two disagree.** `lo-convert.sh` on one corpus document, the only
thing varying being the binary:

```
reference /usr/bin/soffice -- LibreOffice 24.2.7.2   -> 16 page(s)
reference /opt/libreoffice26.2/… -- 26.2.4.2         -> 15 page(s)
```

and the whole sweep over `words/done-005`, same tree, same corpus:

| reference | result |
|---|---|
| PATH, 24.2.7.2 | `TOTAL 10  MATCH 9`, ABS-INK **8.55**, 5 MAJOR pages |
| `REF_SOFFICE`, 26.2.4.2 | `TOTAL 10  MATCH 10`, ABS-INK **1.31**, 1 MAJOR page |

One banked `pages` failure for a document that matches, and `loi_format_letter_of_intent`
alone went 5.47 to 0.04 of ink with 4 MAJOR pages to none. All ten now read `${REF_SOFFICE:-soffice}`
and **print the resolved path and version as their first line**. `sweeps.txt` holds both runs.

## 2. `pdf-image-diff.py` gated page *counts* but not page *alignment*

Its existing guard refuses outright when the counts differ, and the comment says exactly why.
It does not cover the other half: two documents can hold the same number of pages and still
carry different content on them, where a block lost early is made up later.

`misaligned_pages` now reads both text layers in one `pdftotext` call each and warns, and
`track-ink-sweep.sh` carries the count into `ink.tsv` as a **`drift`** column. Validated
against five documents whose pagination was established through a separate channel — every one
hit its expected value:

| expected | got | first page | document |
|---:|---:|---:|---|
| 0 | **0** | — | `Annex-10-…-GCAA` (aligned; quantisation only) |
| 0 | **0** | — | `f445896e…` (aligned) |
| 1 | **1** | 293 | `02_mcar_part-2_and_IS_v2.10` (aligned after r175) |
| 37 | **37** | 111 | `SPA-02_mcar_part-2_and_IS_v2.9` (**equal page counts**, content offset) |
| 49 | **49** | 73 | `24-25_FAA_Holdover_Tables` (one word-parity page lost) |

The Holdover's page 73 is independent corroboration: that is exactly where its MAJOR pages
begin, from the pixels rather than from the text.

**Zero false positives** in the batch that was checked by hand: both `drift 1` rows on
`words/done-005` are real — `2024-12_Comlux` page 3 opens on different content, and
`part-147_approval list` page 2 shares only its running head (true ratio 0.31).

It is a screen and not a verdict, and the header in `ink.tsv` says so: a page of dense numerals
can fall below the threshold while being the right page.

## 3. Two of `compare-images.py`'s six metrics lacked the tolerance the other four have

`differing_fraction` and `differing_tiles` apply `DIFF_TOLERANCE`; `mean_abs_error` and
`max_tile_error` do not. On a synthetic half-shaded page differing by one grey level:

```
differing_fraction 0.0    differing_tiles 0    ink_delta 0.0
mean_abs_error     0.00196     max_tile_error 0.00392
```

Rather than redefine a published measurement — this project has been bitten by two figures
circulating under one name, and `track-ink-sweep.sh`'s own header records it — `diagnose()`
now **names** the cause when no pixel differs past the tolerance and the mean is still not
zero:

> MATCH — no pixel differs past the tolerance. The residual mean of 0.50/255 is colour
> quantisation across a flat fill, not a rendering difference; do not rank on it.

The control still reads correctly: a 20×20 block moved 40 px gives `differing_tiles` 6,
`shifted_tiles` 1, `max_tile_error` 0.125 and `DIFFERS`.

## What did *not* need changing

- `pdf-image-diff.py`'s page-count gate was already right, and already commented with the
  reason.
- `ink_delta`, `differing_fraction`, `differing_tiles`, `shifted_tiles` and
  `row_profile_shift` are all quantisation-safe as they stand.
- `verdict.py` compares the faces before scoring and runs both binaries; it needed nothing.
- `trace-text.py` already solves repeated-label pairing.

## The documentation gap that explains the whole round

**No `SKILL.md` mentioned `REF_SOFFICE` at all**, although `batch-check.sh` had honoured it for
a long time and `CLAUDE.md` documents it. That is why eight sibling scripts never got it. It is
now in `libreoffice-reference/SKILL.md` as the primary note, with pointers from
`render-comparison` and `corpus-batches`, and those two also gained the alignment and
quantisation sections and a blunt one: **do not write your own scorer** — it has produced a
wrong published call twice, both times a hand-rolled mean absolute difference.

`validate.py` re-runs all three checks.
