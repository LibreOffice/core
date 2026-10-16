# words/extra-001 — prediction

Written **before** implementing the fix and before any reach sweep. Scored in `results.md`.

Baseline established first, on the unchanged tree:
`XRay.Office.Fidelity.Tests` — **Failed 30, Passed 520, Skipped 0, Total 550**, which is the
briefed baseline exactly.

## What the brief said, and what the group actually holds

The brief describes two clusters over five documents. The group holds **seven**; `762.doc`
and `info-bulletin-601.doc` are not mentioned in it at all.

### Already established by measurement, not predicted

These were settled before this file was written and are recorded here so the predictions
below are read against them rather than against the brief.

- **Cluster A is not `mc:AlternateContent`.** `OoxmlXml.ResolveAlternateContent` is correct
  and demonstrably runs on header parts (`DocxFile.LoadHeaderOrFooter` → `OoxmlXml.TryLoad`
  → `Normalise`). Page 1 of `ABCD-WB-08-00` is +2 words, not doubled; if both branches were
  drawn every page would be near-doubled. The real cause is a **fixed-height text box whose
  content is taller than the box**: `word/header1.xml`'s first box is 15.00 pt tall with
  `a:noAutofit` and holds four paragraphs of 8 pt text. LibreOffice formats only the lines
  that fit; we format all of them.
- **The measured LibreOffice 26.2.4.2 rule**, from 60 authored probes
  (`probe-textbox-overflow.py`): a line of a fixed-size shape's text body is formatted
  **iff its top offset is strictly less than the box's content height**
  (height less `tIns`/`bIns`), and **the first line is always formatted** however short the
  box. Verified against `wp:extent` heights of 1–100 pt at three inset sizes; the
  fully-inside variant (`top + height <= available`) is refuted by h=10 pt drawing two lines.
  `a:spAutoFit` and VML `mso-fit-shape-to-text:t` disable the truncation; `a:normAutofit`
  and `vertOverflow="overflow"`/`"clip"` do **not** — LibreOffice truncates in all three.
  VML `v:textbox` truncates identically to DrawingML `wps:txbx`, and a table inside a box
  truncates by row on the same rule.
- **Cluster B is not our defect and is already closed.** It is the *reference's*
  table-only-header import defect, diagnosed in round 43, recorded in `TODO.batches.md` §2
  and pinned by `tests/XRay.Office.WordProcessing.Tests/SectionInheritedHeaderTests.cs` with
  the explicit instruction that following the reference here requires deleting a test that
  says why not to. LibreOffice copies a section's header into a following section only when
  the source header holds at least one top-level `w:p`; both `header1.xml` and `header6.xml`
  of `UG.CAO.00133` have `['tbl']` as their only top-level child. That prior finding was
  calibrated against 24.2.7.2; **it reproduces on 26.2.4.2** — stripping every `even`/`first`
  reference from the real document leaves the reference still heading 5 pages of 18, and six
  authored two-section shapes all inherit normally.

So **A and B are two different bugs, and only A is ours.** They are not one
section/header-inheritance defect wearing different clothes: A is a shape-level text
overflow that fires inside headers, footers and the body alike, and B is a section-level
inheritance failure in LibreOffice's importer that we deliberately do not reproduce.

## Predictions

| # | Claim | Confidence |
|---|---|---|
| P1 | Truncating fixed-height text-box content to the measured rule makes `ABCD-WB-08-00` page-exact **and** word-exact — i.e. it flips to a pass | 45% |
| P2 | `ABCD-SDE-23-00` does **not** flip: its +391 is header surplus plus split TOC dot-leaders, and the leaders are a separate defect | 60% |
| P3 | `ABCD-FE-01-00` does **not** flip; it is 14 pages against 15 and the fix moves words, not pages | 85% |
| P4 | Neither `UG.CAO` document moves at all — cluster B is untouched | 90% |
| P5 | `762.doc` and `info-bulletin-601.doc` are **not** "extra" documents: both are one page short with word deltas of −2 and −4, so the group's own classification is wrong for two of its seven | 80% |
| P6 | Reach across the 200 banked `words` references: **10–30 documents change ink**, and **1–3 change verdict**, net non-negative | 50% |
| P7 | No regression: `words/done-*` re-run gives the same verdict for every document it gave one for before | 70% |
| P8 | `XRay.Office.Fidelity.Tests` stays at 30 failed / 550 total | 75% |
| P9 | The fix needs a new `PageFrame` property and one truncation site in `FrameLayout.Content`; no change to `OoxmlXml` at all | 80% |

## What would refute the approach

If truncating drops content the reference **does** draw on documents outside this group, it
means our line heights inside boxes differ from Writer's, and the truncation would be
amplifying a metric error rather than fixing an overflow. That shows up as documents losing
words in the reach sweep. If more than a couple do, the rule goes behind the autofit flag
only where the file states `noAutofit` explicitly rather than by default.
