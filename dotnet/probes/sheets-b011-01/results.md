# sheets-b011-01 — a wrapping cell's lines stop where its row does, and the ones that stop are never drawn

Round of 2026-08-14 on branch `wt-sheets-b011`. Reference binary **LibreOffice 26.2.4.2
620(Build:2)**; banked references at `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`, reused
rather than re-rendered. `SOURCE_DATE_EPOCH=1700000000` for every rendering compared here.
`fc-match "DejaVu Sans"` answers DejaVu; `fc-match Calibri` answers Carlito; `check-env.sh` green.

`prediction.md` beside this was written and committed before any reach was measured
(`c4f7dc81fda`); it is scored in §8.

The document is `sheets/batch-011/xls/T0A0D0000090006XLSE.xls` — 162/162 pages, **42471 words
against the reference's 40382**, +2089, +5.2%.

---

## 1. What kind of round this was: a real defect, and the three cheap explanations are all out

The brief asked for the determinism check and the raster check first. Both were run, both say no,
and the third shape in `TODO.raster-ceiling.md` says no as well.

| test | result | so |
|---|---|---|
| `pdfimages -list` on the banked reference | **0 images in 162 pages** | the reference rasterises nothing; shape 1 cannot apply |
| the reference converted **four** times by the same 26.2.4.2 | 40696 raw words and 162 pages **every time**, and the banked copy is the same 40696/162 | not the `fse` instability |
| whitespace-stripped character streams | 258 687 ours against 245 196 — **+13 491 characters** | not a `pdftotext` tokenisation artefact in either direction |

The character-stream test is the one that decided it. 126 of the 162 pages are character-exact;
all of the excess is on 36 pages between 55 and 104. So we were drawing 13 491 characters of real
content that the reference does not draw at all — which is the opposite of every shape that file
records, and the only one that a renderer can fix.

## 2. The rule, and how it was measured

### It is not a clip, and reading it as one is what hid it for three rounds

`sheets-clip-01` reproduced the *horizontal* half of `AdjustAreaParamClipRect` and wrote, in
`ClipTo`'s own remarks, that the vertical half was open but that "nothing measured in the corpus
turns on it". That is right about the clip and wrong about the cell: **the truncation is not a clip
at all and is not in `output2.cxx`'s clipping code.** It is two lines in two different files:

```cpp
rParam.mpEngine->EnableSkipOutsideFormat(rParam.meVerJust==SvxCellVerJustify::Top
    || rParam.meVerJust==SvxCellVerJustify::Standard);      // sc/…/output2.cxx:3115
```

```cpp
// Stop processing if allowed and this is outside of the paper size height.
// Format at least two lines though, in case something detects whether
// the text has been wrapped or something similar.
if( mbSkipOutsideFormat && nLine > 2
    && !maStatus.AutoPageHeight() && maPaperSize.Height() < nCurrentPosY )
    break;                                                 // editeng/…/impedit3.cxx:1801-1806
```

with a coarser guard one level up that refuses a whole paragraph whose first line would start past
the paper (`impedit3.cxx:676-680`, `nPara != 0`).

The distinction is the whole of why the word gate could see this and could not see
`sheets-clip-01`: **a clip removes ink and leaves every glyph in the PDF's text layer, and this
removes the line before it is ever laid out.** It is upstream of drawing, so `pdftotext` reads the
difference.

The room is the cell's own — `rAlignRect.GetHeight() - nTopM - nBottomM`
(`DrawEditParam::calcPaperSize`, `output2.cxx:2684-2700`) — and **only a wrapping cell has any**:
`calcPaperSize` is called under `if (rParam.mbBreak)` and nothing else, so a cell that does not
wrap keeps the initial `Size(1000000, 1000000)`.

### The tree is 27.2-alpha, so the rule was fitted to 26.2.4.2's own output

An authored twelve-row sweep, Liberation Sans 10 pt in a 4 cm column, row heights 0.4 cm to
3.2 cm, pitch 11.20 pt, ODF's default 0.035 cm cell margin a side. Predicting
`max(4, floor(paperHeight / pitch) + 1)`:

| row height | paperH / pitch | reference draws | predicted |
|---:|---:|---:|---:|
| 11.310 pt | 0.83 | 4 | 4 |
| 17.008 | 1.34 | 4 | 4 |
| 22.677 | 1.85 | 4 | 4 |
| 28.318 | 2.35 | 4 | 4 |
| 33.987 | 2.86 | 4 | 4 |
| 39.713 | 3.37 | 4 | 4 |
| 45.298 | 3.87 | 4 | 4 |
| 50.995 | 4.37 | 5 | 5 |
| 56.665 | 4.88 | 5 | 5 |
| 68.003 | 5.89 | 6 | 6 |
| 79.313 | 6.90 | 7 | 7 |
| 90.652 | 7.92 | 8 | 8 |

**Twelve of twelve.** Six further authored cases pin the guard rather than the arithmetic, and each
of them is a way the port could have been wrong:

| case | reference | what it settles |
|---|---|---|
| vertical `bottom`, row far too short | **all 60 words**, 15 lines | the guard names Top and Standard and nothing else |
| vertical `middle` | **all 60 words** | likewise |
| vertical unstated (`Standard`) | **4 lines**, and still placed from the row's bottom | Standard *is* in the guard, although it draws like Bottom |
| no wrap, 20 hard-break paragraphs, 1 cm row | **all 20** | no wrap ⇒ no paper ⇒ no truncation |
| wrap, 6 one-line paragraphs, 1 cm row | **3** | the paragraph guard runs *before* the four-line allowance |
| paperH an exact multiple: 58 pt row, 5.002 pitches | **6 lines** | the comparison is strict, so `ceil` would answer 5 and be wrong |

And on the corpus document's own decisive row — page 55's last, 427.21 → 286.58 pt, margin 40
twips (the BIFF filter's, not the pool's 20), pitch 11.197 — `floor(136.63 / 11.197) + 1 = 13`, and
the reference draws **exactly 13**.

### It is not the optimal-height branch

`output2.cxx:3255-3261` tests `CRFlags::ManualSize` and decides only **whether a hard clip
rectangle is emitted**; both sides of it truncate. Measured both ways rather than reasoned:

- an authored **manual-height** row is truncated *and* carries a `re W* n` rectangle exactly as
  tall as the row;
- `T0A0D0000090006XLSE.xls`'s **optimal-height** rows are truncated and its reference page 55
  carries **no clip operator at all** beyond the page's own.

That mattered to the implementation: had the port gated on the row's `IsOptimalSize`, it would have
fixed half the cases and missed the other half.

### Why it happens at all, which is worth saying plainly

Calc measures a row's optimal height against a **96 dpi `VirtualDevice`** with the em, the ascent
and the descent each quantised to whole pixels (`SheetOptimalRowHeights`' own remarks), and then
draws the text at printer resolution. At 10 pt the measuring device runs about 2.5 % narrow, so it
fits more per line than the drawing device does and reserves a row one line short. **A wrapped cell
that overflows its row is therefore the normal case rather than a malformed document**, and Calc's
answer to its own coarseness is to stop formatting.

## 3. What changed

`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs`, and nothing else in the tree.

- **`SkipOutsideFormat` added**, with `MinimumFormattedLines = 4`. It walks the wrapped lines
  accumulating the pitch, stops after the first line whose cumulative bottom passes the paper
  height, and refuses a paragraph after the first whose start is already past it. It walks rather
  than divides because the engine's comparison is strict and an exact multiple gets one line more
  than the multiple.
- **`Wrap` now also reports which lines begin a paragraph**, because the four-line allowance is
  counted per paragraph and not per cell. A field's lines are one paragraph whatever its
  representation holds.
- Called from `Place` for a cell that **wraps**, is **not rotated**, and whose vertical alignment
  is **Top or Standard**.
- The stale comment beside `textTop`/`textBottom` — "a wrapped cell taller than its row is exactly
  the case that would lose a line to it", said of the clip — is corrected in place. It was true
  that such a cell loses lines and false that the clip is how.

Deliberately **not** done, and each is a separate measurement:

- **The vertical hard clip for a manual-height row.** `output2.cxx:3255-3261` cuts a manual row's
  ink at the row edge while keeping the glyphs; our clip rectangle is still the union of the cell
  and its text. That moves ink and cannot move a word count, so it is `sheets-clip-01`'s shape and
  belongs with it.
- **`DrawRotated`.** Calc's turned cells go to `DrawEditTopBottom`/`BottomTop`/`AsianVertical`,
  none of which calls `EnableSkipOutsideFormat`, so the rule genuinely does not reach them.

## 4. Measured reach — the sheets track, 171 documents

Two full sweeps, before and after, of the whole track scored against the **banked** references with
`batch-check.sh`'s three checks and thresholds (page count; letter-or-digit words in a 2 %+3 band;
unembedded fonts).

| | match | pages | pages,words | words |
|---|---:|---:|---:|---:|
| before | 155 | 6 | 3 | 7 |
| after | **156** | 6 | 3 | **6** |

- **Page counts moved: 0 of 171.** Pagination is decided by `SheetOptimalRowHeights` and
  `SheetPagination`, both upstream; a line that is not drawn cannot change a row height. This was
  the prediction's tripwire and it held.
- **171 of 171 documents still render**; no timeouts, no failures.
- **15 of 171 documents render differently.** Thirteen of them move a word count; on the other two
  (`edb-emissions-databank v27-NewFormat (web).xlsx` and `fm-provider-service-measures.xlsx`) only
  the clip rectangle moves, because the lines dropped were empty paragraphs, and the extracted text
  is character-identical.
- **Track-wide `Σ|ours − ref|` on the word column: 26 964 before, 24 411 after.**

Every document whose word count moved:

| document | before | after | reference | Δ before | Δ after |
|---|---:|---:|---:|---:|---:|
| `T0A0D0000090006XLSE.xls` | 42471 | **40379** | 40382 | +2089 | **−3** |
| `afn-afn-20250801-fy25-jan25-mar25.xlsx` | 73003 | 72830 | 72843 | +160 | −13 |
| `CSA_CCM_v1.2.xls` | 15768 | 15662 | 15666 | +102 | −4 |
| `tk-syllabus-comparison-document-v5.xlsx` | 234751 | 234650 | 234666 | +85 | −16 |
| `seihon_zassi_kikou_20221215.xlsx` | 48318 | 48256 | 48257 | +61 | −1 |
| `State-Medicaid-Payment-Policies-…xlsx` | 40448 | 40420 | 40411 | +37 | +9 |
| `RMP 2011-2014 and Inventory.xls` | 18418 | 18396 | 18396 | +22 | **0** |
| `arp-sop-300-Exhibit-A-Table-Templates.xlsx` | 3684 | 3664 | 3665 | +19 | −1 |
| `2023-qhp-form-and-rate-combined-checklist-final.xlsx` | 13951 | 13940 | 13936 | +15 | +4 |
| `114339-PROP-P127508-PUBLIC-…xlsx` | 2339 | 2329 | 2326 | +13 | +3 |
| `sectors-defense-and-aerospace.xlsx` | 23046 | 23037 | 22997 | +49 | +40 |
| `SLSA_Directory_031423.xlsx` | 5790 | 5782 | 5786 | +4 | −4 |
| `orbus_togaf_tool_csq.xls` | 32188 | 32183 | 46780 | −14592 | −14597 |

**Eleven closer, one unchanged in distance, one further.** The one that moved further is
`orbus_togaf_tool_csq.xls`, by five words on a document that is 14 592 short and fails on **pages**
— its printed column block is wrong, which `sheets-clip-01` §4 already named as the next thing to
fix there. Nothing about this change made that document worse in any sense a reader would
recognise.

`RMP 2011-2014 and Inventory.xls` is worth one line on its own: **+22 to exactly 0**, and it is in
the same batch as the round's document.

### The residue on the round's own document

`T0A0D0000090006XLSE.xls` is now 40379 against 40382, and the whitespace-stripped character streams
are 245 145 against 245 196 — **51 characters apart out of 245 196, 0.02 %**, down from 13 491.
Every remaining block is one of two things and neither is this rule:

- two places where our line breaks one word earlier than the reference's, so the last line the
  budget allows carries less (`To provide a`, `applications`) — the ~0.1 % advance divergence
  `CLAUDE.md` records as a real open defect with a known seat;
- soft hyphens and `pdftotext` de-hyphenation (`frame­work`, `MPEG-2`), which are tokenisation on
  both sides and move nothing.

## 5. Regression

`sheets/batch-011` first, then `sheets/batch-001` through `011` together, both against the banked
references.

| range | documents | before | after |
|---|---:|---:|---:|
| `sheets/batch-011` | 10 | 9 match, 1 words | **10 match** |
| `sheets/batch-001…011` | 109 | 104 match, 5 words | **105 match, 4 words** |

The four that remain are the documented ceilings, and **every one keeps the same two numbers on
each side of the change**:

| document | ours / ref, before and after | why it is a ceiling |
|---|---|---|
| `2017-04-27-Lease-Transition-Records-Checklist-FINAL-1.xlsx` | 2323 / 2498 | the reference's tokenisation shatters |
| `2020-01-29-…` (its twin) | 2323 / 2498 | the same |
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 1587 / 1531 | one `Tj` per glyph on its rotated tick labels |
| `fse_identification_form.xlsx` | 440 / 427 | the reference is not deterministic on it |

**Zero regressions**, on the batch range and on the whole track alike.

One arithmetic note that is not a discrepancy in the work but would look like one: the brief quotes
**96 of 99** for batches 001–010 and this scores **95 of 99** before the change and 95 after. The
document is `fse_identification_form.xlsx`, whose reference is the one this project has measured as
non-deterministic — the banked PDF is the run that drops a sentence (427) and a fresh conversion is
usually the run that keeps it (440, a match). Scoring against the bank is the more reproducible
choice and it costs that one row. It is the same document in both counts and it did not move.

## 6. Tests

`dotnet/tests/corpus/features/sheet-vclip-row.fods` — a nine-row authored fixture whose header
carries every reference figure and the arithmetic behind it, and
`dotnet/tests/XRay.Office.Spreadsheets.Tests/SheetVerticalOverflowTests.cs`, eight tests against it.

**Verified to fail against the unfixed tree**: with the call to `SkipOutsideFormat` disabled and
nothing else changed, **5 of 8 fail** — the top-aligned truncation, the four-line minimum, the
exact-multiple case, the paragraph guard and the Standard-alignment case. The 3 that pass are the
deliberate controls: bottom/middle alignment, a non-wrapping cell, and a cell that fits its row.
A test suite where the controls also failed would not have been testing the rule.

Every test project, run individually, on the finished tree:

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Containers | 109 | 0 | 0 |
| Core | 332 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| Presentations | 679 | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Spreadsheets | 770 | 0 | 0 |
| Text | 349 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| WordProcessing | 819 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |

The Fidelity 30 of 550 is the branch's inherited baseline, established **before** anything was
changed and unmoved by it. Spreadsheets' 770 includes the eight new tests. The single Rendering skip is the
branch's own and predates this round.

## 7. Verification by looking

Four page pairs at 120 dpi, each handed to a separate fresh subagent that had never seen the
document, was told nothing about the numbers, and was forbidden to read the repository or run any
command. Two before the change and two after, on the same two pages.

**Page 95, before** — the reviewer found the defect without being pointed at it, and separated it
from everything else on the page:

> "At the ~74.5% boundary, **ours draws ~9–10 extra lines that the reference omits entirely**,
> producing an overprinted band from ~75% to ~86% of page height. The reference is clean there.
> This is the largest visual delta on the page and it is entirely one-directional: extra ink on
> ours, never on the reference." … "All five box borders sit at the **same vertical positions** on
> both halves. Row heights are therefore the same; this is not a row-height bug."

**Page 95, after** — the same page, a different reviewer:

> "The overflow/clipping bug is reproduced exactly: same four seams where a paragraph's last line
> overprints the border and the next cell's first line; same truncation of the final paragraph
> mid-sentence at the bottom; box 1 non-overflowing in both." … line counts per box
> "8 / 14 / 14 / 13 / 14" on **both** halves.

**Page 55, before**: "the reference truncates the content; ours completes it … ours ends at ~88 %
of page height, the reference at ~69 %". **After**: "Both halves clip the same final row at the
same y, with the bottom border striking through the glyphs the same way; both truncate
mid-sentence."

Both post-change reviewers, independently and without being asked about it, reported the *same*
residual and gave it the right direction: **our text wraps one word earlier than the reference's**,
on a subset of cells, with the row geometry identical. That is the advance divergence, and having
two blind readings of it is worth more than the character diff in §4 saying the same thing.

### A second defect the fixture found, not fixed here

Row 6 of `sheet-vclip-row.fods` is a **non-wrapping** cell holding twenty hard-break paragraphs.
The reference draws twenty lines; **we draw one**. `Wrap` — which is what splits at a hard break —
is reached only for a cell that wraps, while Calc sends any cell holding a break to an EditEngine
whatever its wrap setting (`DrawEditParam::hasLineBreak`, `output2.cxx:2730`). The fixture and its
test carry the evidence; the test asserts only the token count, which is the question this round
owns, and says in a comment why the line count is not asserted. It is a real defect and worth its
own round; nothing in the corpus was measured against it here.

## 8. Scoring the prediction

| # | predicted | outcome |
|---|---|---|
| 1 | the rule, stated as measured rather than predicted | reproduced by the shipped port on all eighteen authored cases and on the corpus |
| 2.1 | the change is confined to `SheetTextLayout.Place` | **correct** — one file, plus a fixture and a test file |
| 2.2 | the document lands at 40382 ± 150, verdict `words` → `match` | **correct, and better than predicted**: 40379, three words out |
| 2.3 | 0 page counts move; if one moves I have changed something I did not mean to | **correct**, 0 of 171 |
| 2.4 | 15–45 documents' word counts move | **wrong, and under the band**: **13**. The over-estimate came from assuming every wrapped cell taller than its row is common; it is common *within* a few documents rather than spread across the track |
| 2.5 | +1 to +4 to `match`, 0 to 2 away | **correct at the bottom of the range**: +1, 0 away |
| 2.6 | Fidelity stays 30 of 550, 0 skipped | **correct** — 30 failed, 520 passed, 0 skipped, before and after |
| 2.7 | batches 001–011 no worse | **correct**: 104 → 105 of 109, and the four remaining failures keep the same two numbers each |
| 2.8 | this is not `sheets-overflow-01`, `-clip-01` or `-wrap-01` | **correct**, and §2 says why more precisely than the prediction did: those three move ink or geometry and cannot reach the text layer |
| 3 | the four-line minimum is fitted, not read | it survived the corpus and the fixture, but it is still fitted — the 27.2-alpha guard reads `nLine > 2` and I have not reconciled the off-by-one against 26.2.4.2's own source |
| 3 | fields untested | still untested; the four-line minimum hides it on every corpus row looked at |

The prediction's one real miss is 2.4, and it is the harmless direction. Its most useful line was
2.3 — naming the page count as the tripwire before the sweep made a 0 mean something.

## 9. Files

- `prediction.md` — committed at `c4f7dc81fda`, before any reach was measured.
- `dotnet/tests/corpus/features/sheet-vclip-row.fods` — the authored fixture; its header holds
  every reference figure and the arithmetic.
- `dotnet/tests/XRay.Office.Spreadsheets.Tests/SheetVerticalOverflowTests.cs` — eight tests, five of
  which fail against the unfixed tree.
- `dotnet/src/XRay.Office.Spreadsheets/Layout/SheetTextLayout.cs` — `SkipOutsideFormat`,
  `MinimumFormattedLines`, `Wrap`'s paragraph starts, and the corrected comment.
