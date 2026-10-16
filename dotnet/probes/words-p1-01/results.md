# words-p1-01 — the Holdover regression, and the printer that is not 300 dpi

Round `words-p1-01`, 2026-08-15, worktree `wt-w-p1`, branch `wt-w-p1`. Reference LibreOffice
**26.2.4.2** 620(Build:2); `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`, Carlito, Caladea, Liberation
all resolving, `check-env.sh` clean. References reused from
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, never re-rendered. `SOURCE_DATE_EPOCH=1700000000`
on every render that is diffed.

Prediction written and committed after the baseline sweep and before any diagnostic probe:
`prediction.md`, commit `e324e07899c`.

**Two defects, both localised by measurement rather than by reading, and the second is not a
XRay.Office defect at all — it is a stored figure that stopped reproducing.** The words track goes
from 173 to **174 of 200**, and the two documents the brief named go from **+18 to −2** and from
**+23 to +1**. Σ|page error| over the whole track falls from **92 to 50**; Σ|word error| from 4672
to 4174. Reach is **11 of 200** renderings, 1 verdict won and **0 lost**.

**No seat in `words/pagination-001` closed.** It was 3 of 10 and it is 3 of 10. That is stated first
because it is the thing the brief asked for and did not get; §6 says which documents are where and
why I stopped.

---

## 1. The baseline, measured rather than inherited

All 200 `words` documents rendered with a binary built from this worktree's HEAD (`886bcde7091`) and
verdicted against the banked references with `probes/lineheight-01/verdict.py`.

| | measured | briefed |
|---|---|---|
| `words/pagination-001` | 3 match of 10, **7 failing** | 7 failing ✓ |
| `words/done-*` | 158 match of 159 | one known failure ✓ |
| whole `words` track | 173 of 200 | — |
| `XRay.Office.Fidelity.Tests` | 30 failed of 550, 0 skipped | 30 of 550 ✓ |

The seven failing were exactly the seven the previous round left, unchanged by the line-height and
reference-device merges. The one standing `done-*` mismatch is
`airbus-pdf-information-package_v1-4.docx`, 1272 words against 1299, as briefed. The Fidelity 30
were captured **by name** before anything was changed and compared by name again at the end.

## 2. `FAA 2025-26 Holdover Tables` — the +2.88 pt step, and it is `tdf#118521`

The previous round's characterisation was exactly right and its guess at the seat was not.

`TABLE ADJ-28` onward, one extra page per table for twenty tables. The page is a table plus a NOTES
list plus a two-line CAUTIONS block, and the CAUTIONS body was the thing spilling. Measuring the
NOTES-heading-to-first-note gap on **every page of both renderings** — not on one page — showed two
populations in the reference and one in ours:

```
reference          ours
  74 x 16.81 pt     109 x 16.81 pt
  31 x 13.81 pt       0
   4 x 16.62           4 x 16.62-ish
```

Three points, on 31 pages, on a document whose pages are one line from full. The gap *above* the
heading agrees on all 31, so only the unstated half of the spacing diverged.

**Those 31 are exactly the 31 `NOTES` headings that carry a direct `<w:spacing w:before="80"/>`**,
against 76 that carry nothing. Read out of the document's own `word/document.xml`, and confirmed
through LibreOffice's own import: `--convert-to fodt` gives the 76 the style
`Heading_20_4` at `fo:margin-bottom="0.0835in"` (120 twips) and the 31 an automatic style `P293` at
`fo:margin-bottom="0.0417in"` (**60 twips**) — the parent style's own `w:after`, not Writer's pool row.

### The rule

`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:3110-3138, `tdf#118521`:

> set paragraph top or bottom margin based on the paragraph style if we already set the other margin
> with direct formatting

When the paragraph's own `w:pPr` sets an unequal subset of {top margin, bottom margin, contextual
spacing}, each unset margin is written **as direct formatting** from `GetPropertyFromParaStyleSheet`
— which walks the DOCX `w:basedOn` chain and then `w:docDefaults` (`DomainMapper_Impl.cxx`:1556-1628)
and **never consults Writer's pool**. `words-pagination-01`'s pool completion is a property of the
Writer style; that walk cannot reach it.

`probes/words-p1-01/direct-one-sided-spacing.py` authors the chain and reads `fo:margin-*` out of the
importer:

```
paragraph states               top   bottom
nothing                        120      120     the pool completion
w:spacing w:line only          120      120     an element is not a setting
w:spacing w:before="80"         80       60     the DOCX chain, not the pool
w:spacing w:before="0"           0       60     stated, not non-zero, is the trigger
w:spacing before and after      80       40
w:contextualSpacing only       120       60     no w:spacing at all, and it still fires
```

### The probe's first version could not answer its own question, and that is the lesson

It ran one declaration order — parent before child — and every row read 60 whatever the rule was,
because the pool completion only fires when the parent is declared *after* the child. A clean table
of identical numbers looked like a refutation of the hypothesis and was a statement about the sample.
This is `lineheight-01` §7(a)'s trap in a second costume, found the same way: by noticing that the
control row (`style-only`) disagreed with the real document. The script now runs **both** orders side
by side and `FAA 2025-26 Holdover Tables.docx` has its `Heading4` at style 4 and its
`Notes/Cautions Heading` at style 186, so child-first is the column that answers.

### What it bought

`FAA 2025-26 Holdover Tables.docx`: **185 → 165** against 167. The `TABLE ADJ-28`-onward runaway is
gone entirely; the drift is now a flat −1 from `ADJ-1` and −2 from `ADJ-20`, holding to the last page.

`24-25_FAA_Holdover_Tables.docx`: **did not move at all**, 154 against 155. None of its NOTES
headings carries the direct `w:spacing`. The prediction that the pair was one defect with one fix was
wrong, and this is the second round running in which "same family, same fix" has failed on this pair.

## 3. `A_320.doc` — the printer reference device is 600 dpi, not 300

24 of our 141 pages carried only the three-line running header. The brief's reading of them — a
one-page MMEL table pushed whole to the next page — is right about the effect and the cause is one
level down.

Each MMEL page is a table padded with a filler row to exactly one page, then an **empty paragraph
carrying `fo:break-before="page"`**, then the next table. LibreOffice puts the empty paragraph at the
top of a page and the table under it; we put the empty paragraph on a page of its own because our
table no longer fitted beneath it. Measured off the table's own drawn borders on page 20: ours 646.75
pt tall against the reference's 629.95 — **16.8 pt**, accumulated at 0.40 pt on each of about 43
lines.

The line pitch on that page is **13.00 pt** for us and **12.60 pt** for the reference, Liberation
Sans at 11 pt. Neither is the 12.65 the reference device gives, and `A_320.doc`'s `Dop` really does
set `fUsePrinterMetrics` (read directly: `0x054 = 84000000`, `0x1fc = 84000000`, top bit set). So
both sides were on a printer and the printers differed.

### The measurement, three ways

`probes/printer-metric-advance.py` already varies the flag on one authored body. **Re-run against
the installed 26.2.4.2 it does not reproduce its own stored control**:

| face | pt | printer, measured now | 600 dpi | 300 dpi | probe's stored figure |
|---|---:|---:|---:|---:|---:|
| Liberation Serif | 9 | 10.300 | **10.30** | 10.60 | 10.60 |
| Liberation Serif | 10 | 11.550 | **11.55** | 11.55 | 11.55 |
| Liberation Serif | 11 | 12.750 | **12.75** | 12.75 | 12.75 |
| Liberation Serif | 12 | 13.800 | **13.80** | 13.95 | 13.95 |
| Liberation Sans | 9 | 10.350 | **10.35** | 10.35 | 10.35 |
| Liberation Sans | 10 | 11.500 | **11.50** | 11.55 | — |
| Liberation Sans | 11 | 12.600 | **12.60** | 13.00 | 13.00 |
| Liberation Sans | 12 | 13.800 | **13.80** | 13.95 | 13.95 |

600 dpi fits **8 of 8**; 300 dpi fits 3. Two corpus documents' banked references agree
independently — `A_320.doc` gives Liberation Sans 11 pt at 12.60 and Sans Bold 12 pt at 13.80, and
`150_5300_13_chg10.doc` gives Liberation Serif 9 / 9.5 / 10 pt at 10.30 / 10.80 / 11.55, where 300 dpi
predicts 10.60 / 11.30 / 11.55.

The stored 300 dpi figures were not misread. `MetricGrid` cited `PPDParser` defaulting both axes to
300 with no PPD (`vcl/unx/generic/printer/ppdparser.cxx`:1500, :1524) and that is a real default that
this container does not take. **Nothing in the file decides it**, which is why it is a measurement and
not a constant — and why the C++ was the wrong place to have read it. `refdev-01` §8 made the same
correction for Calc from the other direction: the tree names `MSO1` and the binary uses `PDF1`.

The same re-run moves the advance verdict the same way: `exact-em@600` is the best rule at 0.022 pt
mean error where every 300 dpi rule is above 0.68.

### What it bought

`A_320.doc`: **141 → 119** against 118, and 25 near-empty pages became 3 against the reference's 2.

## 4. What was changed

| file | change |
|---|---|
| `src/XRay.Office.Text/Fonts/LineSpacing.cs` | `MetricGrid.Printer` is 600 dpi; the stale 300 dpi prose corrected in four places |
| `src/XRay.Office.WordProcessing/Ooxml/WordStyles.cs` | the pool completion is marked, `PoolCompletedSide` |
| `src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs` | `GroupedMargins` — tdf#118521 |
| `src/XRay.Office.WordProcessing/Ooxml/{WordCompatibility,DocxLayoutSource}.cs` | prose |
| `src/XRay.Office.WordProcessing/Ww8/{Ww8DocumentProperties,DocReader}.cs` | prose |

The completion is marked rather than removed because both rules are real and they apply to different
things: the style keeps Writer's pool row, and a paragraph that sets one margin directly resolves the
other through the DOCX chain. Collapsing them either way loses one of the two, and
`AParagraphStatingNothingKeepsTheStylesPoolCompletion` is the control that catches it.

## 5. Reach, and the three tracks

All 200 `words` documents rendered twice — once with a binary built from `886bcde7091`, once with this
branch — with `SOURCE_DATE_EPOCH` set, so the two runs are byte-comparable with nothing masked.
Verdicts against the banked references.

| track | renderings changed | before | after | won | lost | Σ\|page err\| | Σ\|word err\| |
|---|---:|---:|---:|---:|---:|---|---|
| words | **11 of 200** | 173 | **174** | 1 | **0** | 92 → **50** | 4672 → **4174** |
| slides | — | 147 | 147 | 0 | 0 | — | — |
| sheets | — | 163 | 163 | 0 | 0 | — | — |

Every document whose page or word count moved:

```
150_5300_13_chg10__doc     table-001         80 -> 78   ref 77   pages,words -> pages
150_5300_13_chg12__doc     pagination-002    32 -> 31   ref 31   pages -> MATCH
150_5300_13_chg8__doc      table-001         21 -> 20   ref 18   pages -> pages
A_320__doc                 pagination-001   141 -> 119  ref 118  pages -> pages
FAA 2025-26 Holdover…docx  pagination-001   185 -> 165  ref 167  pages -> pages
```

**11 changed and 5 moved a count.** The one verdict won is another group's — `150_5300_13_chg12.doc`
in `pagination-002` — and `150_5300_13_chg10.doc`'s word count came back inside its band. Nothing
regressed anywhere.

Slides and sheets were re-rendered whole and verdicted: **147 of 163** and **163 of 171**, both
exactly the figures `lineheight-01` and `refdev-01` recorded. Neither could have moved —
`MetricGrid.Printer` has no consumer outside `XRay.Office.WordProcessing` and `WordParagraphFormats` is
the DOCX reader — but it is cheaper to check than to argue.

### `words/done-*`

**158 match of 159, before and after.** No `done-*` document lost its verdict. The one standing
mismatch is `airbus-pdf-information-package_v1-4.docx` at 1272 words against 1299, unchanged and
unrelated — `words-regress-01` §2 established it is a missing repeat of a header row.

Whole corpus: **484 of 534**, up one.

## 6. What was not done, said plainly

Nothing in `words/pagination-001` changed verdict. Two documents moved a very long way and stopped
one and two pages short, and five were not attempted at all.

| document | before | after | why it stopped |
|---|---|---|---|
| `A_320.doc` | 141/118 | **119/118** | §6(a) — one page, one table, diagnosed |
| `FAA 2025-26 Holdover Tables.docx` | 185/167 | **165/167** | §6(b) — two pages, one of them diagnosed |
| `24-25_FAA_Holdover_Tables.docx` | 154/155 | 154/155 | unmoved; not this defect |
| `ESPN-R - MCF - RA - Ed1.docx` | 59/58 | 59/58 | **not attempted** |
| `absrc-pac-01-info-note-en.doc` | 6/7 | 6/7 | **not attempted** |
| `report-template.docx` | 19/20 | 19/20 | **not attempted** |
| `template---tpr-…docx` | 8/7 | 8/7 | **not attempted** |

The four "not attempted" are the four the previous round left first-divergence notes for, and this
round adds nothing to them. That is two rounds in which they have been described and not touched.

### (a) `A_320.doc`'s last page: one header row, one page

Exactly **one page of the 119** draws the MMEL header row wrong, and it is page 28. The row's left
cell (`Aircraft:` / `AIRBUS INDUSTRIE …`) is drawn at the *bottom* of a row 83.7 pt tall where the
reference draws it at the top of a row 28.9 pt tall; the right cell (`Revision No. 35` / `Date:`) is
at the top in both. Everything below is pushed down about three lines, the table ends 23 pt lower
than the reference's, and the empty page-break paragraph after it no longer fits — which is the
remaining page.

**Read blind and confirmed in the operators.** A reviewer that had never seen the document, was given
no numbers and was forbidden to read the repository reported, unprompted:

> the upper rendering's first table row is taller than the lower one's — by roughly three text lines'
> worth … within that row, the upper rendering pushes "Aircraft: / AIRBUS INDUSTRIE …" downward, to
> the bottom of the cell, while the lower rendering places it at the top. The right cell's text stays
> at the **top** in both.

and independently listed the causes the image cannot decide between — leading empty paragraphs in the
cell, a stated row height, a space-before applied at the top of a cell, or cell padding — with the
measurement that separates each. The same reviewer confirmed line breaking, wording, column
boundaries, indents and inter-line spacing are **identical**, which rules the metrics out. The ODF
import gives the row `style:vertical-align="top"` on both cells and a style byte-identical to the 14
tables that render correctly, so the row property is not the difference; what is left is the cell's
own content, and that is where the next round should measure.

### (b) `FAA 2025-26 Holdover Tables`'s last two pages: a cross-reference that does not expand

The reference spills a page at `ADJ-19` that we do not, and the cause is a `REF` field. Note 11 of
that table reads, in the reference:

> … (Table Adj-51 provides adjusted allowance times for Type IV EG fluids and Table **Adj-52:
> Adjusted Allowance Times for SAE Type IV Propylene Glycol (PG) Fluids¹˒²** provides …)

and in ours the bold stretch is just `Adj-52`. The reference's expansion is a whole caption and costs
it one more line, hence one more page. This is a field-expansion defect rather than a metric one and
it also moves the word count; it wants its own round. The other page arrives before `ADJ-1` and was
not diagnosed.

### (c) The advance rule on the printer device is not exact

At 600 dpi `floor(N · advance · round(size/72 · 600) / upem)` reproduces **37 of the probe's 96 rows**
and the other 59 within two twips. At 300 dpi the same rule was out by up to 137 twips, so this is a
hundredfold improvement on an open residue rather than a new one. Dropping the truncation fits 52 of
96 but is **worse on 17**, so the evidence does not choose between them and the floor stays because it
is what `vcl` says. Pinned by
`MetricGridTests.TheAdvanceRuleIsNotExactAndTheResidueIsRecordedRatherThanHidden` so that a change
that fixes it fails there rather than passing in silence.

### (d) The NOTES list numbering still runs away

`words-pagination-01` found it blind and it is still there: page 40 emits `1, 196, 197 … 205` where
the reference emits `1 … 11`. Visible in this round's page dumps as `582`, `583`, `585` … It does not
move the gate, because the labels are the same *count* of tokens. Untouched.

## 7. Tests

**Fidelity is 30 failed of 550, 0 skipped, and it is the same 30 by name** — captured to
`fid-base-names.txt` on the unmodified tree before anything was changed and compared name by name at
the end. Not just the count: 0 new, 0 newly fixed, the sorted lists identical.

| project | passed | failed | skipped | before |
|---|---:|---:|---:|---|
| Core | 337 | 0 | 0 | 337 |
| Containers | 109 | 0 | 0 | 109 |
| Text | **565** | 0 | 0 | 563 |
| Vector | 295 | 0 | 0 | 295 |
| Rendering | 150 | 0 | 1 | 150 |
| Markup | 259 | 0 | 0 | 259 |
| OpenDocument | 125 | 0 | 0 | 125 |
| WordProcessing | **912** | 0 | 0 | 903 |
| Spreadsheets | 853 | 0 | 0 | 853 |
| Presentations | 717 | 0 | 0 | 717 |
| **Fidelity** | **520** | **30** | **0** | **30 of 550** |
| total | 4842 | 30 | 1 | |

Every project run individually. Build is 0 warnings, 0 errors. No flaky run was seen; every count is
from a single pass. **One anomaly, seen twice and not in the tests.** `dotnet build XRay.Office.slnx` died with
`Fatal error. Internal CLR error. (0x80131506)` on the first build of the session and again on the
last, and succeeded unchanged on the immediate retry both times — 0 warnings, 0 errors. Nothing was
edited between the failure and the success either time. It is the shape `dotnet/CLAUDE.md` records
for the *test* host wearing a new hat: a build that dies outright is at least loud, where the
truncated-run failure it warns about is silent. Recorded so the next agent re-runs rather than
investigates.

### New: `DirectOneSidedSpacingTests`, 9 tests

Fixture `tests/corpus/features/direct-one-sided-spacing.docx`, written by the same probe that
measured the expectations, in the declaration order that can answer. **3 of the 9 fail against the
unfixed behaviour** and the other 6 are controls that must not move — including
`AParagraphStatingNothingKeepsTheStylesPoolCompletion` and a three-row theory on a custom style where
the two rules give the same number and the fixture *cannot* show the difference, named so that it is
not mistaken for evidence.

### Changed: every printer-grid expectation

19 test cases across `MetricGridTests`, `ReferenceGridTests`, `EastAsianLineScaleTests` and
`PrinterMetricsTests` stated 300 dpi's answers as LibreOffice's. Each is now the freshly measured
26.2.4.2 figure with its source named in the file. Three things worth recording:

- `OnAPrinterGridTheSameFaceRoundsUpToLibreOfficesAnswer` is renamed, because at 600 dpi the printer
  grid is sometimes *shorter* than the design units and "rounds up" is now false. Its 12 pt row was
  dropped: the two devices agree there, and **a case a broken tree passes by accident is not a test.**
- `tests/corpus/features/printer-metrics{,-off}.docx` had to be **rebuilt**. They were 12 pt Arial,
  chosen in 2024 because that is where 300 dpi showed largest; 600 dpi sets 12 pt at exactly 100
  device pixels, so the flag changed nothing and `PrinterMetricsTests` asserted a difference the
  binary no longer makes. Rebuilt at 16 pt — the largest separation in a sweep of every half point
  from 8 to 16, 0.20 pt — by `probes/words-p1-01/remake-printer-metrics-docx.py`, which also
  re-measures both packages through `soffice` so the numbers in the test come from the binary.
- `ReferenceGridTests.APrintersGridIsStillTheOtherAnswer` gained a second face and size, because the
  printer grid moves in **both** directions with size and a single row that only ever grew invited the
  wrong generalisation.

### Verified failing against the unfixed behaviour

Two separate reverts, so each half is proved on its own.

| reverted | result |
|---|---|
| `MetricGrid.Printer` back to 300 dpi | Text **18 failed** of 565, WordProcessing **1** of 912 |
| `GroupedMargins` returning its layers untouched | `DirectOneSidedSpacingTests` **3 failed**, 6 passed |

The single WordProcessing failure under the first revert is `PrinterMetricsTests`, and the spacing
tests being unmoved by it is by design — they name no device.

### The mtime trap, guarded

The tree was built nine times across two revert/restore cycles and several prose passes. Every
restore is `cp` followed by `touch`, with `rm -rf src/<project>/{obj,bin}` before the rebuild — and
after each restore a subset of the corpus was re-rendered and compared **byte for byte** against the
run being claimed: **50 of 50** on `words/done-00[1-4]` plus `words/pagination-001` after the reverts,
and **10 of 10** again after the last prose rebuild.

## 8. Predictions, scored

Five right, five wrong, one unsettled. The two that mattered most — the two named seats — were both
wrong about the cause and both right about how close the document would land, which is the
uncomfortable pattern `words-pagination-01` recorded as well.

| | claim | conf | outcome |
|---|---|---:|---|
| P1 | the `ADJ-28` step is a block measured once per table, and the block is the repeated header | 45% | **wrong on the seat.** It is one block per table and the block is the NOTES *heading's* space-after, not a header row |
| P2 | the two Holdover documents are one defect with one fix | 55% | **wrong** — `24-25_FAA` did not move a page. None of its NOTES headings carries the direct `w:spacing` |
| P3 | `FAA 2025-26` lands within ±3 of 167 | 35% | **right** — 165 |
| P4 | `A_320`'s blank pages are a table-fits-the-page test made against the wrong height | 50% | **wrong.** The fit test is right; the table was 2.8% too tall because the printer is 600 dpi |
| P5 | `A_320` lands within ±5 of 118 | 30% | **right** — 119, and it was the lowest-confidence number on the sheet |
| P6 | at least two of the seven close outright | 45% | **wrong** — none did |
| P7 | no `done-*` document loses its verdict | 70% | **right** — 158 of 159 either side |
| P8 | Fidelity no worse than 30 of 550 | 50%→60% | **right** — exactly 30, and the same 30 by name |
| P9 | more than 20 of the 200 renderings change | 60% | **wrong** — 11. Both fixes are narrower than they look: one reaches 31 paragraphs of one document, the other reaches the 8 DOCs that set one flag |
| P10 | at least one of the four first-divergence documents is not attempted | 80% | **right**, and it was all four |
| P11 | the five ±1 documents are five different causes | 65% | **unsettled** — only one of them was measured |

## 9. Contradicting the brief, and the record

- **"The fundamental line height is now exact. Every pagination residue you find is therefore a
  *block* being mis-measured, not a per-line drift."** Not on the eight DOC files that set
  `fUsePrinterMetrics`. There the line height was out by up to 2.8% *per line* — 0.40 pt on every line
  of `A_320.doc` — because `lineheight-01` fixed the virtual reference device and the printer device
  beside it was never re-measured on this container. The brief's premise is right for 192 of the 200
  and it is the 8 that were holding the group's largest gap.
- **`probes/printer-metric-advance.py`'s stored control does not reproduce.** It records the printer
  pitch for Liberation Serif at 9/10/11/12 pt as 10.60/11.55/12.75/13.95 and Liberation Sans at
  9/11/12 as 10.35/13.00/13.95. Run today, unchanged, against 26.2.4.2 it gives
  10.300/11.550/12.750/13.800 and 10.350/12.600/13.800. Its advance conclusion moves with it: "exact
  on all 96 rows" at 300 dpi is 16 of 96 today, and the best 600 dpi rule is 52.
- **`MetricGridTests`' whole expectation set was 300 dpi's**, including the two figures it attributes
  to `A_320.doc` and `150_5300_13_chg10.doc` directly. Both documents' banked 26.2.4.2 references
  disagree with it. Neither the tests nor the probe were wrong when written; the container changed
  underneath them and nothing in the harness declares the printer any more than it declares the font
  set — which is `dotnet/CLAUDE.md`'s own lesson arriving in a third place.
- **`tests/corpus/features/printer-metrics.docx` was a fixture that could no longer discriminate.**
  That is worse than a wrong expectation, because it fails whichever way the code behaves and gives no
  hint why. Worth a habit: a fixture built at the size where an effect is *largest* is a fixture whose
  usefulness depends on the effect's shape, and that shape can move.
- **"`FAA 2025-26 … regressed to 185 … while its twin improved to 154 … Same family, same fix,
  opposite outcomes."** Right, and the corollary the brief drew — that one fix would settle both — is
  wrong for the second round running. The twin has none of the 31 direct-spacing headings.
- **"24 of our 141 pages carry only a three-line running header … each one-page MMEL table pushed
  whole to the next page instead of fitting on the page whose header we already drew."** Right in
  every particular except the mechanism: the table is not pushed by a fit test, it is pushed because
  the empty `fo:break-before="page"` paragraph in front of it takes a page of its own once our table
  is 16.8 pt too tall to sit under it.

## Files

```
src/XRay.Office.Text/Fonts/LineSpacing.cs                     MetricGrid.Printer is 600 dpi
src/XRay.Office.WordProcessing/Ooxml/WordStyles.cs            PoolCompletedSide
src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs  GroupedMargins, tdf#118521
src/XRay.Office.WordProcessing/Ooxml/WordCompatibility.cs     prose
src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs      prose
src/XRay.Office.WordProcessing/Ww8/Ww8DocumentProperties.cs   prose
src/XRay.Office.WordProcessing/Ww8/DocReader.cs               prose
tests/XRay.Office.WordProcessing.Tests/DirectOneSidedSpacingTests.cs   9, three fail unfixed
tests/XRay.Office.WordProcessing.Tests/PrinterMetricsTests.cs          re-measured, new fixture size
tests/XRay.Office.Text.Tests/MetricGridTests.cs                        re-measured throughout
tests/XRay.Office.Text.Tests/ReferenceGridTests.cs                     re-measured, second face added
tests/XRay.Office.Text.Tests/EastAsianLineScaleTests.cs                the device it travels with
tests/corpus/features/direct-one-sided-spacing.docx                  new fixture
tests/corpus/features/printer-metrics{,-off}.docx                    rebuilt at 16 pt
probes/words-p1-01/direct-one-sided-spacing.py            both declaration orders, and why
probes/words-p1-01/direct-one-sided-spacing.txt           the table as run
probes/words-p1-01/remake-printer-metrics-docx.py         the fixture, and its own re-measurement
probes/words-p1-01/prediction.md                          committed before any diagnosis
```
