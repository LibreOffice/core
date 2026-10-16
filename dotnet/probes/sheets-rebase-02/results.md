# Sheets rebase-02 — results

Reference: **LibreOffice 26.2.4.2 620(Build:2)** with `fonts-dejavu-core` installed, banked at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/` — 171 PDFs, per-format identity
`stem__ext.pdf`. Ours: worktree `/c/sandbox/workdir/wt-sheets-a`, branch `wt-sheets-a`, based on
`a5d453fae3f`. Every render pinned `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC`.

`prediction.md` in this directory was written and committed **before** any measurement.

Per-document tables: `parity-head.tsv` (ours at HEAD) and `parity-final.tsv` (ours with both
fixes), both against the corrected reference, in `batch-check.sh`'s own columns.

---

## 1. The sheets scoreboard — **143 of 171**

| | match | mismatch | ref-failed | ours-failed |
|---|---:|---:|---:|---:|
| ours at `a5d453fae3f` | **142** | 29 | 0 | 0 |
| ours with both fixes below | **143** | 28 | 0 | 0 |

Final verdict histogram: 143 `match`, 19 `words`, 5 `pages,words`, 4 `pages`.

Both halves of this pair are DejaVu-correct, which no previous sheets figure was. Scored with
`batch-check.sh`'s three predicates verbatim — page count, then extractable words outside a 2%
*and* >3-absolute band, then unembedded fonts. **Expect it to be restated**: the gate's word check
is being changed elsewhere so that a token needs a letter or a digit, and 19 of the 28 remaining
mismatches are `words`-only, so that change lands squarely on this number.

### The instrument, re-validated rather than inherited

The predecessor's `ours-vs-stored.sh` was reused; its three predicates are byte-identical to
`batch-check.sh`'s. Before believing it:

* **The reference bank was checked against its own manifest.** All 171 `stem__ext.pdf` files
  exist and every `pdfinfo` page count equals `ref-baseline-all.tsv`'s `refpages`: 171 checked,
  0 mismatched, 0 missing. The identity is *derived from each path*, never joined on a field
  index — which is the failure mode that manufactured a "534 of 534 changed" result last session.
* **Known-answer run on `sheets/batch-001`: 10/10**, reproducing the predecessor's claim, with 9
  of 10 word counts equal to the digit.
* **Load stability.** Eight outliers were re-rendered **solo**, nothing else running: `sectors`
  227, `orbus_togaf_tool_csq` 33, `ODs-February` 154, `CIS_Debian` 109, `grants-2005` 219,
  `essd-16-3433` 4p/956w, `activespecs` 266, `afn` 270 — all identical to the 8-worker sweep.
  Four small-drift documents were re-rendered solo too and also reproduced exactly. Our column is
  deterministic and not load-sensitive.

### Refutation — the predecessor's 135/171 was withdrawn for the wrong reason

`sheets-rebase-01` withdrew its `135/171` on the grounds that the pair was *mismatched*: our
renders made after DejaVu was installed, the reference made before. The timestamps say otherwise.

```
dpkg: fonts-dejavu-core installed          2026-08-13 15:53:15
predecessor's ours/ bank finished          2026-08-13 15:52:22
```

The `ours/` bank was rendered **53 seconds before DejaVu existed**. Both halves of that
measurement were pre-DejaVu, so `135/171` was internally consistent — a valid measurement of a
wrong-font world, not a mismatched pair. Withdrawing it was still right; the reason given was
wrong.

The direct evidence: re-rendering ours at the same source with DejaVu present moved **our own
column on 31 of 171 documents**, including `afn` 178→270 pages (the reference's 270),
`SIL_TDB648` 70→89, `activespecs` 240→266, `dragon-175066A` 15→13 (reference 13). Our engine
resolves system fonts, so the font set is an input to *both* halves of the gate. That is the
finding worth carrying: **the gate has a fourth input nobody was declaring** — corpus, code,
LibreOffice version, and font set.

135 → 142 is therefore the font's effect on the *score*, not the correction of a mismatch.
Verdict movement between the two: 10 into `match`, 3 out of it
(`2017-04-27`/`2020-01-29-Lease-Transition-Records-Checklist`, `SIL_TDB605`), 2 partial.

---

## 2. `7-memento-2015-transports-aeriens-b.xls` — the inferred step, executed

Reference page 2: **63,765 px** `#0066CC`, **1 px** `#003366` (reproduced here to the digit).

| | `#0066CC` px | `#003366` px |
|---|---:|---:|
| reference | 63,765 | 1 |
| ours, at HEAD | 18,061 | 847 |
| **ours, fixed** | **64,628** | **0** |

Vector, the same page, vertical ink as a union of intervals so segment granularity cannot flatter
it:

| | x ≈ 120 (band's leading edge) | x ≈ 512 (trailing edge) |
|---|---|---|
| reference | `#0066CC`, 5 blocks, 436.41 pt | `#0066CC`, 5 blocks, 436.41 pt |
| ours at HEAD | `#0066CC`, 26.58 pt | **`#003366`**, 408.47 pt + `#0066CC` 26.58 pt |
| **ours, fixed** | `#0066CC`, 5 blocks, **436.48 pt** | `#0066CC`, 5 blocks, **436.48 pt** |

Same blocks, same colours, same rows, 0.02% apart. The residual 863 px (1.35%) of raster excess
is antialiasing: we emit 34 per-row segments where LibreOffice merges 5 collinear runs.

### 2.1 The brief's framing is refuted — the defect was mostly *missing* ink

The brief describes it as "an off-page column paints a border on our page", i.e. extra ink. The
847 px of wrong-colour `#003366` is real and is gone, but it was 1.3% of the divergence. The other
98.7% was **45,704 px of `#0066CC` we never painted at all**. This was flagged in `prediction.md`
as the thing I expected to refute, and P3.2 — my own prediction that `#0066CC` would stay below
25,000 — is refuted by the same measurement.

### 2.2 The cause, measured from the bytes: a cell XF's "used" flags

The rows that lost their box use `XF` 115 of that workbook:

```
r8..r15 c1  LABEL  xf115 cell par=54 bu=0 au=1
             L=1:#0066CC R=1:#0066CC T=1:#0066CC B=1:#0066CC pat=1 fore=#99CCFF
   parent -> xf54 STYLE par=4095 bu=1 au=1
             L=1:#0066CC R=1:#0066CC T=1:#0066CC B=1:#0066CC pat=1 fore=#99CCFF
```

The record **carries** a thin `#0066CC` box; its border-used flag is clear; its parent style
carries the identical box. `XlsDecorationTable.FormatOf` honoured the flag literally and returned
`SheetCellBorders.None`. The fill survived only because `au=1`.

Calc does not honour the flag. `XclImpXF::CreatePattern`
(`sc/source/filter/excel/xistyle.cxx:1291-1294`):

```cpp
if( !mbBorderUsed )
    mbBorderUsed = !pParentXF->mbBorderUsed || !(maBorder == pParentXF->maBorder);
if( !mbAreaUsed )
    mbAreaUsed   = !pParentXF->mbAreaUsed   || !(maArea   == pParentXF->maArea);
```

The flag is turned **on** whenever the parent states nothing or states something different, so a
cleared flag survives only when the parent states the *same* thing — and what the cell then
inherits through its style sheet is that same thing. **Both branches end on the record's own
bytes.** For a cell XF with a parent in range the flag decides nothing; only a missing parent
(`4095`, or out of range) leaves it in force, and a style XF has no parent to consult.

Fix: `dotnet/src/XRay.Office.Spreadsheets/MsBinary/XlsCellDecoration.cs`, with the parent index
threaded in from `XlsWorkbookReader.ReadXfDecoration`. **This defect is not in the brief and
nobody asked for it**; it was found underneath the one that was.

### 2.3 The clip-range rule — confirmed, and confirmed *against LibreOffice*, not by inference

The predecessor's control-flow reading of `Edges.Build` was right about the mechanism: at HEAD
`Resolve(own.Right = None, column2.Left = #003366)` returned the off-band neighbour, which is
exactly the 847 px. It was wrong about it being the fix — with the XF read corrected, `Resolve`
returns `own` on a tie and the clip rule is inert on that page (identical pixel counts with and
without it).

So the rule was established separately, and **measured rather than inferred**. New fixture
`dotnet/tests/corpus/features/sheet-band-clip.fods`: three 10 cm columns on a 17 cm printable
width, one column per page; row 1 states the A/B edge from the left in red, row 2 states the same
edge from the right in blue. Rendered by LibreOffice 26.2.4.2 itself:

| page | band | LibreOffice draws |
|---|---|---|
| 1 | column A | one **red** stroke at x 340.10, row 1 only — row 2's blue is not there |
| 2 | column B | one **blue** stroke at x 56.66, row 2 only — row 1's red is not there |
| 3 | column C | nothing |

Ours now reproduces all three exactly (x 340.13 / 56.69, same colours, same rows). One correction
to the predecessor's citation, which matters because it nearly rules the whole thing out:
`ScOutputData::DrawFrame` sets the clip range only `if( mrTabInfo.mbPageMode )`
(`sc/source/ui/view/output.cxx:1567`) — but `ScPrintFunc::PrintPage` passes `bPageMode = true`
(`sc/source/ui/view/printfun.cxx:1612-1614`), so it *is* in force for ordinary printing and not
only for page-break preview.

Fix: `SheetPageDecoration.Edges.Build` — the four `Resolve` calls at the band's outer edges take
the in-band cell's own style. A "band" is a contiguous run, not "first and last on the page", so
the same discontinuity test the bottom edge already used is applied to the top and left.

---

## 3. `sectors-defense-and-aerospace.xlsx` — a **version** effect, not a font effect

Settled, and by a test that does not depend on the page count at all:

| | pre-font reference | corrected reference |
|---|---|---|
| pages | 449 | **449** |
| words | 23,964 | **23,964** |
| file size | 1,084,225 B | **1,084,225 B** |
| fonts | `BAAAAA+Carlito-Regular` | `BAAAAA+Carlito-Regular` |

One face, Carlito, embedded; **no DejaVu and no WenQuanYi anywhere in it**, in either bank. The
font set cannot reach this document, so the whole of 227 → 449 is left to 24.2.7.2 → 26.2.4.2.
P2.1 and P2.3 both right. Caveat, stated because it cannot be removed: the 227 is a *stored*
number from another container; no 24.2.7.2 binary exists here, so this is measured-vs-stored, not
a controlled A/B.

### Refutation — the font's page movement is *not* all in one direction

The brief states the font alone moves 11 sheets page counts (43 pages) and 36 word counts, "every
page change in the same direction — fewer pages with DejaVu, because it is narrower than the face
it displaced". The three counts reproduce **exactly**. The direction does not: **6 of the 11 gain
pages with DejaVu, 5 lose them.**

| document | no DejaVu | with DejaVu | |
|---|---:|---:|---|
| `Application_for_authorisation_as_crowdfunding_service_provider.xlsx` | 44 | 48 | +4 |
| `SIL_TDB605.xls` | 42 | 44 | +2 |
| `SIL_TDB609.xls` | 47 | 50 | +3 |
| `SIL_TDB648.xlsx` | 78 | 90 | +12 |
| `flightstandards-doc-Cross-reference-table_version02.xlsx` | 461 | 464 | +3 |
| `seihon_zassi_kikou_20221215.xlsx` | 82 | 84 | +2 |
| `NPIAS_App_A.xls` | 126 | 124 | −2 |
| `afn-afn-20250801-fy25-jan25-mar25.xlsx` | 282 | 270 | −12 |
| `airports_6.xlsx` | 18 | 17 | −1 |
| `dragon-175066A.xlsx` | 14 | 13 | −1 |
| `june_2025_published.xlsx` | 16 | 15 | −1 |

The substitution is confirmed face-by-face — `SIL_TDB648` goes `BAAAAA+WenQuanYiZenHei` →
`BAAAAA+DejaVuSans`, and gains a `DejaVuSans-Bold` that had collapsed into the same WenQuanYi
face for want of a bold. "Narrower" is not the mechanism; **losing the weight distinction** is at
least as much of it, and it moves pagination both ways.

Also useful: with the font corrected, `SIL_TDB648`'s reference goes 88 (24.2) → 90 (26.2), not
88 → 78. Most of that document's apparent version movement was the missing font.

---

## 4. Measured reach of the two fixes

Byte-identical renders under a pinned epoch, 171 documents, and a static census of the same class
from the input bytes. Both instruments, independently:

| fix | renders whose bytes change | verdicts moved |
|---|---:|---:|
| XF used-flags | **2** of 171 | +1 (`aircraft_analysis_2016-04-27.xls`, `pages` → `match`) |
| band clip range | **63** of 171 | **0** |

* **The XF class is genuinely rare.** A census of every OLE2 workbook in the track — cell XFs
  carrying border or fill bytes with the flag clear and a parent in range — finds the class in
  exactly **2 of 61** workbooks, and they are exactly the two whose renders changed. Two
  instruments, one answer. My prediction of 20–90 was wrong by an order of magnitude.
* **`aircraft_analysis_2016-04-27.xls` moved because decoration is part of the print range.**
  44 → 46 pages, matching the reference, with its word count unchanged at 14,525: the two extra
  pages carry recovered borders, not text. That document is one of the five in the "page-split
  cluster" the predecessor recorded as unexplained.
* **The clip rule moves 63 renders and no verdicts**, which is the predicted case (P3.3) and the
  reason it needed its own instrument. Direction was measured as pixel distance from the
  reference, page by page, before against after: **14 pages closer, 1 further by a single pixel**
  (page 1 of one document that is 42 and 20 px closer on two other pages), the rest unchanged.
  Every non-trivial change moves toward the reference.
* **Run over documents that already match**: no `match` became a mismatch under either fix. Of
  the 63 byte-changed renders, most were already matching, and all still are.

---

## 5. Prediction scoring

| # | Prediction | Outcome |
|---|---|---|
| P1.1 | corrected score in 140–155, point estimate 147 | **right on the band** (142 at HEAD, 143 fixed); point estimate 5 high |
| P1.2 | ≥1 document flips `match` → mismatch | **right** — 3 did |
| P1.3 | `pages` dominates `words`; 0 `ours-failed` | **half wrong** — `words` dominates 19 to 4; `ours-failed` 0 ✓ |
| P1.4 | `batch-001` reproduces 10/10 | **right** |
| P2.1 | `sectors` is a version effect; reference unchanged at 449 | **right**, to the digit |
| P2.3 | its font list is identical between the two banks | **right** |
| P3.1 | `#003366` goes to 0 or 1 | **right** (0) — by a different mechanism than predicted |
| P3.2 | `#0066CC` stays below 25,000 | **wrong** — 64,628 against the reference's 63,765 |
| P3.3 | the fix moves **zero** verdicts | **right for the clip fix** (0); wrong for the XF fix (+1) |
| P3.4 | reach 20–90 documents | **right for the clip fix** (63); wrong for the XF fix (2) |
| P3.5 | ≥5 matching documents change bytes, none leaves `match` | **right** on both halves |
| P4 (what I expected to refute) | the brief frames the defect as extra ink; it is mostly missing ink | **right** |

The brief's pattern held twice more: three figures it quoted (11 / 43 / 36) reproduced *exactly*
while the sentence attached to them — "every page change in the same direction" — is false; and
the predecessor's withdrawal of `135/171` reproduced as a number while its stated reason did not
survive a timestamp check.

---

## 6. Build and tests

`dotnet build XRay.Office.slnx`: **0 warnings, 0 errors**.

Ten non-Fidelity projects: **3461 total, 0 failed** — 3454 on this branch's base plus the 7 tests
added here. (The coordinator's 3458 is four higher: this branch predates the `wt-words-b` merge.)
`XRay.Office.Spreadsheets` 621 → **628**.

`XRay.Office.Fidelity.Tests`: 510 passed / **40 failed**, and the failing set is **identical**
with and without these changes — 21 distinct test names, verified by building and running the
pre-fix tree with the new tests held out. Pre-existing, and attributable to the 26.2.4.2
reference, not to this round.

### Verified by reintroduction (`verify-test.sh`), not drift guards

| mutation | detected by |
|---|---|
| `xf.StatesBorder \|\| ownAttributes` → `xf.StatesBorder` | `ACellFormatPaintsItsBorderWhenTheUsedFlagIsClear` |
| `(xf.StatesArea \|\| ownAttributes)` → `xf.StatesArea` | `ACellFormatPaintsItsFillWhenTheUsedFlagIsClear` |
| `HasStyleParent` → `return true` | `AFormatWhoseParentDoesNotExistStillHonoursTheClearedFlag`, `AStyleFormatIsDecidedByItsOwnFlagAlone` |
| trailing edge back to `Resolve(own.Right, neighbour.Left)` | `TheLastColumnOfABandStatesItsOwnTrailingEdge` |
| `firstColumnOfBand` forced false | all three of `SheetBandClipTests` |

All 7 new tests are detectors. **None is a drift guard.** The two guard cases
(`AFormatWhoseParentDoesNotExist…`, `AStyleFormatIsDecidedByItsOwnFlagAlone`) exist to stop the
rule being over-applied and are detectors of exactly that.

---

## 7. What this round could not see

* **The gate is blind to everything the two fixes touch.** Colour, stroke position and ink
  coverage move none of page count, word count or unembedded fonts. "143 unchanged after the clip
  fix" is evidence the fix is *safe*, and no evidence at all that it is right; the pixel-distance
  instrument in §4 is what carries that.
* **No 24.2.7.2 binary exists here.** Every "version effect" is measured-here against
  stored-elsewhere.
* **The reference bank was not re-rendered.** Any nondeterminism in `soffice` is invisible; only
  individual documents were re-run solo.
* **Byte reach over-counts.** One changed stroke and a re-paginated document both read as
  "differs". The XF census is the independent check that made 2 believable; the clip rule's 63 has
  no such second instrument, only the direction measurement.
* **`pdftotext | wc -w` is not the text**, and 19 of the 28 remaining mismatches are `words`-only
  — the category most exposed to the pending change in that predicate.
* **The two remaining large `pages` divergences are untouched and unexplained**:
  `orbus_togaf_tool_csq.xls` 33/75 and `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx`
  154/175, plus `sectors` 227/449 where the *reference* is the thing that moved.
