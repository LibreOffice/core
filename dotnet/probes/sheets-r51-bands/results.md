# Round 51 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r51`,
base `bd0f5ac1cf2`. Read `prediction.md` beside this file first — its two halves were committed
(`3ca22455c84`, `aef9ad145a3`) before the sweeps they predict.

## 1. Baseline reproduced

`batch-check.sh sample-files 'sheets/*' … 6` → `TOTAL 325 MATCH 277 MISMATCH 48`. The 325 is the
case-insensitive mount counting five documents twice under two spellings of one inode; case-folded
it is **307 documents, 267 match, 40 mismatch**, and the 40 mismatching paths are exactly the 40
rows `MANIFEST.tsv` marks `open`. Reproduced to the document. Nothing stored had to be discarded.

## 2. Result

**sheets 267 → 268 of 307.** Predicted +1, measured +1.

| document | before | after |
|---|---|---|
| `020_Free_Blood_Pressure_Chart…xlsx` | `words` 6/6 pages **117/133** words | `match` 6/6 **133/133** |

`133/133` was the *predicted* landing value, derived before the change from the reference drawing
the footer on 4 of the document's 6 pages at 4 word-tokens a page. **Zero regressions**: no
document went from `match` to anything else, and `sheets/done-*` is 156 of 156.

Nine documents changed on our side. Six were predicted to the digit, one to a range, one was
flagged in advance as at risk, and one was the census's own named blind spot:

| document | before → after | predicted? |
|---|---|---|
| `020_Free_Blood_Pressure…` | 117/133 → **133/133** | exactly |
| `021_Control_Chart_Template…` | 925/921 → **921/921** | exactly |
| `018_Weight_Loss_Chart…` | 968/958 → 960/958 | exactly |
| `022_Pareto_Chart_Template…` | 191/192 → 190/192 | exactly |
| `2012-GA-Survey-Chapter-5…xls` | 501/495 → **495/495** | exactly |
| `2012-GA-Survey-Chapter-6…xls` | 636/624 → **624/624** | exactly |
| `023_Waterfall_Chart_Template…` | no change | predicted no change |
| `fm-provider-service-measures.xlsx` | 21245/21348 → **21347**/21348 | "up by at most ~90"; actual +102 |
| `FAA-2019-0995-0002_attachment_2.xlsx` | 9995/9995 → **10015**/9995 | "unknown — at risk" |
| `PC1000.xls` | 856/855 → 848/855 | **not predicted** |

**Page counts: 0 of 307 changed**, as predicted.

`PC1000.xls` is the prediction's blind spot #1 firing exactly where it said it would: the census
parsed OPC worksheet XML only and could not see BIFF `HEADER`/`FOOTER` records. It moved in the
right direction — its `Page` count went 10 → 9 and its `V1000` 1 → 0, which is **exactly** the
reference's 9 and 0.

`FAA-2019-0995-0002` is the one place the round leaves the tree further from the reference: +20
words, five `PAGE`, five `OF` and ten page numbers on a header the reference does not draw. It
still matches, with a band of 199.9. § 5 is what is known about it and it is unresolved.

## 3. What was changed

1. **`XlsxPrintSetup` and `XlsbPrintSetup` never set `HeaderGap`/`FooterGap`**, so both inherited
   `SheetPrintSetup`'s ODF default of **142 twips (7.1 pt)**. `SheetPageDecoration.DrawBand` lays
   text into `bandHeight − gap` and returns early on a negative rectangle, so **every XLSX/XLSB
   header or footer whose two margins leave under 7.1 pt was dropped outright** — no ink, no
   words. Calc's own distance is `max(0, statedBand − nominal)`
   (`sc/source/filter/oox/pagesettings.cxx:1029-1041`); `XlsPrintSetup` has had the rule since it
   was written and the other two readers simply never called it. Now shared as
   `SheetBandHeight.BodyDistance`.
2. **A band shorter than its text starts at the band's own top edge**, not bottom-aligned on the
   margin — `nDif = max(0, paperHeight − textHeight)` (`printfun.cxx:1876-1912`).
3. **A band whose two margins are equal draws nothing.**
4. **`&K` swallows the six characters after it, whatever they are**
   (`pagesettings.cxx:639-647`). Reading them as hex digits and stopping at the first non-hex one
   leaves Excel's theme form `&K01+049` drawing `+049`.

The six authored margin variants that established (1)–(3), all rendered both ways:

| bottom | footer | stated band | ours before | ours after | reference |
|---:|---:|---:|---|---|---|
| 0.30 in | 0.30 in | 0.0 pt | not drawn | not drawn | **not drawn** |
| 0.30 | 0.25 | 3.6 pt | **not drawn** | 770.449 | 770.370 |
| 0.35 | 0.25 | 7.2 pt | 762.852 (3.9 out) | 766.849 | 766.770 |
| 0.50 | 0.25 | 18.0 pt | 762.852 | 762.852 | 762.320 |
| 0.30 | 0.10 | 14.4 pt | 773.652 | 773.652 | 773.120 |
| 0.75 | 0.30 | 32.4 pt | 759.252 | 759.252 | 758.720 |

The two previously-broken rows now sit **0.079 pt** from the reference, better than the 0.53 pt
`pdftotext -bbox` ink-box offset that the already-correct rows carry.

## 4. Tests

Nine tests in `SheetSmallBandTests`, on a new authored five-sheet fixture
`sheet-small-band-xlsx.xlsx` (`Pinned` 3.6 pt, `Zero` 0 pt, `Roomy` 25.2 pt, `Snug` 14.4 pt,
`Spill` a three-line footer). Every expectation is read off LibreOffice 26.2.4.2's own PDF of that
fixture. Five mutations run through `verify-test.sh`:

| mutation | detected by |
|---|---|
| the gap goes back to the ODF default | `AFooterBandTooShortForItsTextIsStillDrawn`, `…StartsAtTheBandsOwnTopEdge`, `APinnedBandKeepsNoGapAndABandThatFitsKeepsTheDefault`, `TheDistanceIsNothingOnAPinnedBandAndTheFallbackOnOneThatFits`, `TheThemeFormOfTheColourCodeLeavesNoTextBehind` |
| the top-edge clamp removed | `AFooterBandTooShortForItsTextStartsAtTheBandsOwnTopEdge` |
| the clamp **widened** to the text rectangle's top | `ABandThatOnlyJustFitsIsNotLiftedToItsBandTop` |
| the zero-band guard relaxed | `AFooterBandWhoseMarginsAreEqualIsNotDrawnAtAll` |
| `&K` back to a hex scan | three of the above |

**Seven of nine verified by reintroduction, and in both directions** — the rule is pinned against
being removed *and* against being widened. The remaining two are drift guards and are labelled as
such in the file: `AFooterBandWithRoomToSpareStillSitsOnItsFooterMargin` (no mutation reaches it;
it is there so the clamp cannot creep into the case that already worked) and
`AFooterThatOverflowsThePaperLeavesItsLastLineOffThePage`, which records § 5's refutation.

`dotnet build -v q -nologo` → **0 warnings, 0 errors.** Ten non-Fidelity projects run individually
and totalled by hand: **4565 passed, 0 failed, 1 skipped**, against the merged base's 4556/0/1 —
a delta of exactly the nine new tests, all in `XRay.Office.Spreadsheets` (886 → 895). `Fidelity` is
**521 passed, 31 failed, 552 total**, byte-for-byte the merged base's figure, so this change moves
none of the pre-existing failures.

## 5. The round's own refutation: the per-line band clip

Sweep 1 produced the `FAA-2019-0995-0002` regression above, and three authored probes isolated it
to one variable: at the same 3.6 pt band, text on the first line is drawn by both sides, and the
same text behind eight empty lines is drawn by us and by LibreOffice not at all. I committed a
prediction addendum, implemented a per-line clip against the band's bottom edge, and swept again.

**It was wrong, and the second sweep said so.** It fixed `FAA` (back to 9995 exactly) and broke
`fm-provider-service-measures.xlsx`, whose two-line footer lost its second line on six pages —
21347 (one word off the reference) → 21317 (thirty-one off). That is the blind spot the addendum
had named in advance.

Twelve authored single-sheet probes then mapped what the reference actually does, one variable
apart, at a 3.6 pt band with 9 pt text:

| band | content | reference draws |
|---|---|---:|
| header | 1 / 2 / 3 / 9 text lines | **1 / 2 / 3 / 9** |
| footer | 1 / 2 / 3 / 9 text lines | **1 / 2 / 2 / 2** |
| header | 8 empty lines then 1 text line | **0** |
| footer | 8 empty lines then 1 text line | 0 |
| header, **64.8 pt** band | 1 empty line then 1 text line | 1 |
| header, **64.8 pt** band | 8 empty lines then 1 text line | **0** |

A header is never cut. A footer stops at two lines — and two is exactly how many 11.2 pt lines fit
between a footer band top of 770.4 pt and the 792 pt page edge, the third starting at 792.8. So
the boundary is the **paper**, not the band.

**And then the paper-edge version was measured and also dropped.** Rendering all twelve probes
with that clip in and with it out gives **identical results — the same ten agreeing and the same
two differing** — because the PDF writer already discards a run whose baseline is off the media
box. The rule buys nothing the page boundary does not already give, so it was reverted rather than
kept on the grounds of being right in principle. `3e4f4f50344` is the revert and it carries the
measurement.

**What is left genuinely unexplained**, and is the FAA case: a header of eight empty lines followed
by one text line is drawn by us and by LibreOffice not at all, at *both* band sizes tried, while a
header of nine text lines out of the same 3.6 pt band keeps all nine and a header of one empty line
plus text keeps it. It is not a parser artefact of `&R` followed by a newline — the one-blank-line
probe rules that out. Twenty words, on a document that still matches by 180.

## 6. The briefed lead — the chart legend selection rule — is not what the brief says it is

The brief's first target, on the strength of two r50 blind reviewers naming *"the reference draws a
legend, ours draws none"* on `003_advanced_excel_pie` and `057_Simple_balance_sheet` while two
others saw the reverse. Two of the three claims in that framing do not survive.

**`003_advanced_excel_pie`: the reading was an artefact of which page was shown.** Its
`chart1.xml` declares `<c:legend><c:legendPos val="r"/>`, and `DrawingChartPlot.LegendOf` maps a
present `c:legend` to a position, so the legend is in our model. `pdftotext` of both sides shows
**both draw a five-entry `M1`…`M5` legend — on page 2.** The chart is wider than the page and the
legend lives in the right-hand strip; `pair.sh --worst` selects page 1, which carries the pie and
no legend, and the swatch a reviewer read as a legend key there is the reference's **M1 data
label placed outside the pie**. A fresh reviewer of mine reproduced the same misreading from the
same page, which is the useful part: it is the instrument, not the reader.

**And the pie family's actual defect is not a legend at all.** All four
`advanced_excel_pie` documents are 5 words short and every one of those 5 words is on page 2:
the reference's strip holds `M1; Actual; 93; 17%` whole plus `trend` and `07;` where ours holds
`rend`, `7%` and `100; 19%`. Direction: **the reference places the M1 data label further right,
outside the pie, so it falls wholly onto page 2; we keep it inside the pie, where the horizontal
page split cuts it.** Four documents, one defect, two words past the band each.

**`057_Simple_balance_sheet` is a real hit, and it is a naming defect rather than a selection
one.** A fresh blind reviewer of page 3, given nothing but the image, independently reported "the
reference draws a legend below the plot, a cyan swatch labelled *Column C* and a red one *Column
D*; ours draws no legend at all". Its `chart11.xml` declares `<c:legend><c:legendPos val="b"/>` —
so we have the legend and do not draw it. Its series carry **no `c:tx`**, and *Column C* /
*Column D* are names LibreOffice synthesises from the sheet columns. The untested hypothesis for
the next round: **we drop a legend entry whose series has no name, and a legend with no entries
draws nothing.** That is not "the selection rule disagrees in both directions"; the reverse
direction (`037`, `029`) was not tested here and stays open.

The same reviewer ranked something else above the legend on that page: **our rotated category
labels are spread over a wider span than the plot, overlap each other, and the last one runs off
the chart card entirely.** That matches `057`'s token census, which is a storm of two- and
three-letter fragments (ours 474 words against the reference's 278) — and `057` is a `pagination`
failure at 4 pages against 3, so the legend alone was never going to move its verdict.

## 7. Six blind readings, and what else they found

Six fresh subagents, none of which read this brief, the source, or any project document; each saw
one paired image and nothing else; each asked to describe both halves separately, give direction,
and say what looked identical. Beyond § 6:

- **`077_Inventory_list_with_highlighting` — r50's open sub-puzzle is answered.** The reviewer
  reports that the reference draws a **red flag glyph** in the twelve `1` cells and the digit `0`
  in the thirteen `0` cells, paints a **full-width pale olive fill across those twelve rows**, and
  **strikes through every cell of the three discontinued rows**. So the question "why does the
  reference draw neither the ones nor the zeros" had a false premise: it draws the zeros and
  replaces the ones with a symbol. `numFmtId="165"` is not the format in force on those cells; a
  conditional format is. Twelve tokens, band 6.46 — **one verdict**.
- **`068_Blue_inventory_list` — the entire 10-word deficit is two autoshapes.** The reference
  draws two arrow-shaped buttons at the top right, "INVENTORY PICK LIST" and "BIN LOOKUP"; we draw
  nothing there. The reviewer also, unprompted, reported our "INVENTORY LIST" title in grey
  against the reference's teal — a theme-colour resolution failure on a passing element.
- **`017_Timeline_Templates_for_Excel` — we omit the whole timeline.** The navy vertical spine,
  five circular year badges with white text and drop shadows, and every blue leader line: the
  reference draws them, we draw the year numbers as bare black text. Its +6 words are the opposite
  direction — two sentences the reference clips to their shape and we draw whole.
- **`065_Weight_loss_tracker` — the `aaaa` number-format token.** We draw the literal `aaaa`
  twelve times where the reference draws `Thursday`. The reviewer also caught that our dark table
  block is about 40% of the reference's width with the column positions identical, so it is the
  fill extent and not the columns.
- **Volatile dates reach 16 of the 40 open documents**, not the ~7 the brief carried. Every one of
  them shows a today-derived date on the reference side against a cached 2021–2024 date on ours.

## 8. An instrument fact: the reference half of the gate is not reproducible for date-bearing sheets

Three sweeps hours apart, our binary pinned by `SOURCE_DATE_EPOCH`. **Four documents' reference
word counts moved with the wall clock and ours did not**: `047_Date_tracker_Gantt_chart` 844 → 819,
`PBN Matrix NAAS (V01).xlsx` 5546 → 5543, `tk-syllabus-comparison-document-v5.xlsx` 234665 →
234666, `SIL_TDB648.xlsx` 7496 → 7500. `soffice` recalculates `TODAY()` at the real clock and
nothing in the harness pins it.

Two consequences worth carrying: a stored reference figure for one of these documents is a figure
about a *day*; and a sweep diff must be split by which side moved before any of it is attributed
to a change. Splitting this round's diff that way is what separated its nine real movements from
three that were the calendar.

## 9. Shared layer

**No.** The diff touches six files, all in `XRay.Office.Spreadsheets`:
`Layout/SheetHeaderFooter.cs`, `Layout/SheetBandHeight.cs`, `Layout/SheetPageDecoration.cs`,
`Ooxml/XlsxPrintSetup.cs`, `Xlsb/XlsbPrintSetup.cs`, `MsBinary/XlsPrintSetup.cs`. Nothing in
`Core`, `Containers`, `Text`, `Vector`, `Rendering`, `Markup` or `XRay.Office.Ooxml` is touched, so
words and slides cannot be reached and no cross-track sweep is owed. `WordProcessing` (1052) and
`Presentations` (747) are green and `Fidelity` is identical to the base, which is the corroboration
rather than the claim.

## 10. Proposed `MANIFEST.tsv` reclassification

`MANIFEST.tsv` lives in the corpus repository and was not touched. One row:

| path | from | to |
|---|---|---|
| `sheets/chartset-012/xlsx/020_Free_Blood_Pressure_Chart_and_Printable_Blood_Pressure_Log_8f8dcc39.xlsx` | `status=open`, `kind=text` | `status=done` |

## 11. What the next round should do first

1. **`077_Inventory_list_with_highlighting`** — a blind reviewer has now named the mechanism
   (conditional formatting: an icon or symbol replacing the value, a full-row differential fill,
   and a full-row strikethrough). Twelve tokens against a band of 6.46: one verdict, and the
   template family it belongs to is large.
2. **The `057` legend-entry hypothesis** — a legend whose series have no `c:tx` may be losing its
   entries and drawing nothing. Test it on an authored chart before touching the selection rule,
   and test the reverse direction (`037`, `029`) at the same time; this round did not.
3. **The pie family's data-label placement** — four documents, one defect, two words each past the
   band, and the observable is precise: the reference moves the `bestFit` label for the first pie
   slice outside the pie and we keep it inside.
4. **The eight-blank-line header** — twelve probes in `probes/sheets-r51-bands/` bracket it and
   none explains it. Twenty words on `FAA-2019-0995-0002_attachment_2.xlsx`.
