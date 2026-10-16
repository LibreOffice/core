# Sheets rebase-01 — predictions committed BEFORE measuring

Written 2026-08-13, before running `ref-baseline.sh`, before opening the `.xls` bytes,
before reading the BIFF reader. Reference binary here is **26.2.4.2 620(Build:2)**; every
stored figure on the track was taken against **24.2.7.2**.

## Task 1 — re-baseline

- P1.1 I predict the sweep renders **≥ 168 of 171** sheets documents without failure
  (soffice reference failures on this corpus have historically been ~0).
- P1.2 I predict the coarse prior figure ("16 of 171 changed, 305 total |Δ|") is broadly
  reproducible but **not to the digit** — I expect the *count* of changed documents to land
  in 12–22 and the total |Δ| within ±20% of 305.
- P1.3 **`sectors-defense-and-aerospace.xlsx` 227 → 449.** I predict this is **real, not a
  load artifact.** Reasoning: a truncated-under-load render yields *fewer* pages, not more;
  449 is not a plausible truncation of 227. A near-doubling in the *upward* direction is the
  signature of a pagination-policy change (scaling / fit-to-page / column-break default),
  which is exactly the class of thing a 24→26 major bump moves. I am explicitly betting
  against the brief's framing here.
  - P1.3a Secondary: I predict the page *size* of the reference PDF is unchanged between
    renders, i.e. the doubling is a column-split (sheet spilling to a second page-column),
    so I expect ~449 ≈ 2 × 227 minus a small remainder, not an arbitrary number.
- P1.4 `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` 109 → 88 and `grants-2005.xls` 220 → 201:
  I predict both reproduce to the digit. Downward moves of ~20% are consistent with row
  height / font metric changes compacting content, and these are the same axis (row height)
  that the track has already confirmed as live.
- P1.5 I predict I will **not** find a complete stored per-document reference page-count
  table for all 171 sheets docs in the repo, so a full movement table will be partly
  "no stored value to compare against". I expect to find stored values for well under half.

## Task 2 — MERGEDCELLS / `7-memento-2015-transports-aeriens-b.xls`

- P2.1 I predict the `MERGEDCELLS` record (BIFF `0x00E5`) **is present** in the workbook
  stream and **does** contain a range covering the columns whose left edges are at
  x = 119.99 and x = 512.39 on page 2. (The reference draws a 115.28 pt vertical there;
  the reference cannot invent a merge.)
- P2.2 I predict the defect is **in our BIFF reader, not downstream** — ~65/35. Most likely
  single cause, in rough order: (a) only the *first* `MERGEDCELLS` record per sheet is read
  and this file emits several (the record holds max 1027 ranges and Excel splits beyond
  that, and many writers split much earlier); (b) the record is skipped because it appears
  after a position our sheet-substream loop stops at; (c) the ranges are read but a
  single-row-tall merge (`firstRow == lastRow`) or a merge with only one row is filtered out
  as "not a merge".
- P2.3 The observed symptom — **one** 13.1 pt segment where the reference draws 115.28 pt —
  is ~8.8× short. I predict the covered block is ~9 rows tall and we are emitting the left
  edge for exactly one row, i.e. we know about the *cell* but not the *span*. That points at
  (c)/downstream more than at (a). If (a) were true I would expect **zero** segments, not one.
  I flag this tension now: my structural prediction P2.2 and my arithmetic prediction P2.3
  disagree, and P2.3 is the better-evidenced one.

## Task 3 — page-split cluster (reference side only)

- P3.1 I predict at least 3 of the 5 (`RMP 2011-2014 Rev1 sd.xls`,
  `FAA-2019-0995-0002_attachment_2.xlsx`, `CSJU List of Beneficiaries 2015.xlsx`,
  `aircraft_analysis_2016-04-27.xls`, `FY2018_Q4_UAS_Sightings.xlsx`) have an **unchanged**
  reference page count vs. the stored figure, i.e. the version bump does not explain the
  cluster and the cluster remains ours.
- P3.2 Specifically for `aircraft_analysis_2016-04-27.xls` (stored 44/46) I predict the
  reference stays at **46**.

## Task 4 — the column-fit predicate

- P4.1 I predict the refutation **holds**: column widths remain identical and row heights
  remain the live axis on 26.2.4.2. Probability ~70%.
- P4.2 I predict that if anything moved, it moved in *row height*, and that a minimal
  single-variable sheet will show row height responding to font size while column width
  does not respond to content length (no autofit on load).

## What this round's instruments CANNOT see — written before the sweep

There is **no `XRay.Office.Cli`** (nuget.org is 403, package cache empty). Therefore:

- I can measure only the **reference** column. Every "ours" figure in this round is
  **inherited from stored notes, not measured**, and any statement of the form "we differ
  by N" is an *inference* chained on a stored number taken against a *different binary*.
  I will label every such statement.
- I cannot run `batch-check.sh`, cannot produce a pass/fail verdict, cannot confirm any fix.
- For Task 2 I can read the input bytes and read our source. I **cannot** execute our reader,
  so "the range reaches / does not reach `StatedMerges`" is established by **reading control
  flow**, not by observing a value. A second reader path (xlsx) or an inherited/default code
  path could reach the same field differently and my read would not see it.
- The census in Task 2 sees only **this one file's** `MERGEDCELLS` records. It cannot tell
  me corpus reach, cannot see the xlsx path (`mergeCells` element), cannot see merges
  implied by anything other than an explicit record, and cannot see whether a range that
  *is* parsed is later filtered by the decoration path.
- For Task 4, a minimal authored sheet establishes reference *behaviour*; it says nothing
  about whether **we** match that behaviour. A "holds" verdict means "the reference still
  behaves as the refutation assumed", not "we are correct".
- Per-document `|ink|%` and any raster comparison are unavailable (they need our render).
