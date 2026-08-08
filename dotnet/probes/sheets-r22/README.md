# Sheets round twenty-two — probe data

Measured at `9cffaa02a` (base) and at this round's `/BaseFont` commit (after). Two workers,
`batch-check.sh` against a snapshot of the CLI so a rebuild could not disturb a running sweep.
Both sweeps: 171 rows, no duplicate path, zero `ref-failed`.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep before any change — 144/171, page error 94, 153 exact |
| `after-basefont-whole-track.tsv` | the same after the `/BaseFont` fix — identical in all four numbers |
| `ops-survey-001-009.tsv` | `pdf-ops.py diff --only text` over all 89 *matching* documents of batches 001–009 |
| `fonts-ours-before.tsv` | `/BaseFont` names our renderings carried, per document, before the fix |
| `fonts-ours-after.tsv` | the same after it |
| `fonts-reference.tsv` | what LibreOffice's own renderings carry, for comparison |

## Reading the sweep rows

`path · ext · ourPages/refPages · ourWords/refWords · ourFonts/refFonts · unembedded · verdict`

## Reading the survey rows

`id · onlyOurs · onlyRef · differing · faceDiffs · sizeDiffs · showsOnlyDiffs · topKinds`

`showsOnly` counts records whose *only* reported difference is operator granularity
(`shows N vs M`) — the known `Tj`-splitting artefact, which is benign. A document whose
`differing` equals its `showsOnly` and which has no one-sided record is clean; 23 of the 89 are.

**One harness trap that cost a run and is worth passing on**: `pdf-ops.py diff` exits non-zero
when records differ, which is the normal case. A wrapper written as
`rep=$(… ) || rep=""` therefore discards the output on precisely the documents worth looking at
— it blanked 78 of these 89 rows and reported them as `FAILED`. Decide success by whether the
report parses, never by the status.

## The font files

Names have their six-letter subset prefix stripped and are sorted and deduplicated, so these
cannot be used to count *how many* fonts a document embeds — only which faces it names. That
matters: a statistic I tried to derive from them ("documents naming two faces alike") came out
zero before and after and is degenerate for exactly this reason. The figure that is sound is
how many documents' name sets equal the reference's: **11 before, 131 after, of 171.**
