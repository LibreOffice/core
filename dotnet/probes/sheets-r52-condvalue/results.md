# Round 52 — sheets — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; worktree `wt-sheets-r50`, branch `wt-sheets-r52`,
base `442b0298d80`. Read `prediction.md` and `prediction-addendum.md` beside this file first —
both were committed (`6812e8ff019`, `8e214f29695`) before the change each predicts and before
anything was rendered with it.

## 1. Baseline reproduced

`batch-check.sh sample-files 'sheets/*' … 8` → `TOTAL 325 MATCH 279 MISMATCH 46`. Scored against
`MANIFEST.tsv`'s 307 sheets paths — the raw total counts 18 case-alias directory entries twice —
that is **268 match, 39 mismatch**, and the 39 mismatching paths are **exactly** the 39 rows the
manifest marks `open`. Reproduced to the document. Nothing stored had to be discarded.

## 2. Result

**sheets 268 → 271 of 307.** Predicted **+1 then +1**; measured **+1 then +2**.

**Zero regressions.** No document went from `match` to anything else, and **0 of 307 page counts
changed**, as both predictions said.

Six documents moved **on our side**; every one of them was named in advance:

| document | before | after | verdict |
|---|---|---|---|
| `077_Inventory_list_with_highlighting…xlsx` | 335/323 | **323/323** | `words` → **`match`** |
| `049_Expenses_calculator_c351f3d0.xlsx` | 213/332 | **330**/332 | `words` → **`match`** |
| `037_Personal_money_tracker…xlsx` | 442/505 | **501**/505 | `words` → **`match`** |
| `DynamicBubbleChart.xlsx` | 309/341 | **349**/341 | `words`, unchanged |
| `066_Agile_Gantt_chart_08f9de45.xlsx` | 948/933 | **930**/933 | `match`, unchanged |
| `036_Simple_to-do_list…xlsx` | 78/77 | **77**/77 | `match`, unchanged |

Seven more rows changed **on the reference side only** and none of them is this round:
`047_Date_tracker_Gantt_chart` (818 → 819, and 838 in an intermediate sweep — it oscillates),
`00514292`, `PBN Matrix NAAs (V01)`, `tk-syllabus-comparison-document-v5`,
`TK-Syllabus-Comparison-Document-v2`, `SIL_TDB648`, `FAA-2019-0995-0002_attachment_2`. Splitting
the diff by which side moved, as `CLAUDE.md` § "This container" requires, is what separates the
six real movements from those seven calendar ones. **Our column never moves for a document this
round did not touch.**

Three predicted landing values were hit exactly — `077` at 323, `036` at 77, `DynamicBubbleChart`
at 349 — and the two misses are both explained by named blind spots (§ 3.3, § 4.4).

## 3. Change one — a conditional format that hides the cell's value

### 3.1 What it is

`077_Inventory_list_with_highlighting` states, in its worksheet's `extLst`:

```xml
<x14:iconSet iconSet="3Flags" showValue="0" custom="1">
  <x14:cfvo type="percent"><xm:f>0</xm:f></x14:cfvo>
  <x14:cfvo type="num"><xm:f>-1</xm:f></x14:cfvo>
  <x14:cfvo type="num"><xm:f>1</xm:f></x14:cfvo>
  <x14:cfIcon iconSet="NoIcons" iconId="0"/>
  <x14:cfIcon iconSet="NoIcons" iconId="0"/>
  <x14:cfIcon iconSet="3Flags" iconId="0"/>
</x14:iconSet>  <xm:sqref>B6:B30</xm:sqref>
```

`showValue="0"` suppresses the cell's **text**, not the icon: `output2.cxx:1691-1698` clears
`bDoCell` before the string is laid out. The part that is not obvious from the schema is the
exception — `ScIconSetFormat::GetIconSetInfo` returns **nullptr** when the band a value falls in
has a `NoIcons` entry in a custom icon vector (`colorscale.cxx:1231-1239`, the `-1` put there by
`IconSetRule::importIcon`), and a cell with no icon information keeps its text. **One rule hides
some of its cells and prints the rest.**

Bands are the **last** threshold a value satisfies, not the first (`colorscale.cxx:1200-1215`);
the comparison is `>=` unless the threshold carries `gte="0"` (`condformatbuffer.cxx:118-124`);
and `percent`/`percentile`/`min`/`max` resolve against the sorted numeric values inside the rule's
own range (`ScColorFormat::getValues`, `colorscale.cxx:504-573`).

The same `showValue` reaches a data bar (`mbOnlyBar`, `condformatbuffer.cxx:386`). The x14 data-bar
extension carries no `showValue` and `ExtCfDataBarRule::importDataBar` never touches `mbOnlyBar`
(`condformatbuffer.cxx:1710-1715`), so the plain element is the only source — which is why `036`'s
value is hidden although its x14 twin says nothing about it.

New file `XRay.Office.Spreadsheets/Ooxml/XlsxHiddenValues.cs`, plus one line in
`Ooxml/XlsxSheetReader.ReadCell`. A worksheet that states no hiding rule returns a shared empty
instance without walking its cells.

### 3.2 Two round-50 claims about `077` are refuted by the markup and by the reference PDF

- **"thirteen holding `0`, which neither side draws" — wrong.** The reference draws **all
  thirteen**, and so do we. `pdftotext -layout` of the two baseline PDFs puts `0` on rows 3, 4, 5,
  7, 9, … on *both* sides and nothing where we drew `1`. The whole 12-token gap is the twelve
  `1`s. Round 51's blind reviewer was right about this and round 50's census sentence was wrong.
- **"column B's style is `numFmtId=165` (`"$"#,##0`), under which we should draw `$1`" — wrong.**
  Column B's cells are `s="16"`, and `cellXfs[16]` is `numFmtId="0"`, General. `165` belongs to
  `cellXfs[19]`, which is column H (*Inventory value*). We draw a bare `1` because a bare `1` is
  what General says. **There is no number-format defect on this document**, and round 50's open
  sub-puzzle had a false premise on both halves.

### 3.3 The census, its control group, and the one miss

`showValue` occurs in **exactly three** of the corpus's 946 documents, all sheets, and every
occurrence is a false. Ten further documents carry `iconSet`/`dataBar` rules with `showValue`
absent — nine of them passing — and were the control group: **none of them moved**, including
`076_Inventory_list_accessibility_guide` (1 icon set, 4 data bars) at exactly 1114 before and
after.

`066_Agile_Gantt_chart` lost **18** tokens where the prediction said 15. The rule was right and
the count was wrong: my markup census used a regex whose value-matching skipped self-closing
`<c/>` elements, under-counting the `1`s by one per sheet. The direction was then settled properly
with `pdftotext -bbox`, which is the useful part — per page, our render carried exactly six
`1`/`2` tokens the reference did not, at (266,182), (345,309), (384,214), (519,405), (526,405),
(534,405), and after the change **our `1`/`2` set on the gantt grid is identical to the
reference's**. The residual 3-token gap is the volatile calendar header — the reference's date
range holds a day "2" and our cached Feb-2023 range does not.

### 3.4 Tests

Seven tests in `SheetHiddenValueTests`, on a new authored five-sheet fixture
`sheet-hidden-values-xlsx.xlsx` (`dotnet/probes/sheets-r52-condvalue/make-fixture.py` authors it).
**Every expectation is read out of LibreOffice 26.2.4.2's own PDF of the fixture**, which extracts
as `CUSTOMR 11 22` / `PLAINROW PLAINSTRING` / `SHOWNRO 88 99` / `BARROW` / `GTEROW 50` — five
branches, all confirmed by the reference before a line of the test was written, and our render of
the same fixture is character-identical on all five.

Six mutations through `verify-test.sh`:

| mutation | detected by |
|---|---|
| the hook removed — nothing is ever hidden | `ACellInACustomBandWithARealIconLosesItsText`, `AnIconSetThatIsNotCustomHidesEveryNumberItCovers`, `ADataBarWithTheValueHiddenDrawsNoNumber`, `AThresholdMarkedGteZeroExcludes…` |
| the `NoIcons` exception ignored (the rule read as a property of the range) | `ACellInACustomBandOfNoIconsKeepsItsText`, `AThresholdMarkedGteZeroExcludes…` |
| `showValue`'s default flipped | `AnIconSetWithNoShowValueHidesNothing` |
| the numeric gate widened to string cells | `AStringCellInsideAHiddenRangeKeepsItsText` |
| the **first** matching band taken instead of the last | `ACellInACustomBandWithARealIconLosesItsText`, `AThresholdMarkedGteZeroExcludes…` |
| `gte` ignored | `AThresholdMarkedGteZeroExcludesTheValueSittingExactlyOnIt` |

**Seven of seven verified by reintroduction, and in both directions** — pinned against removal and
against widening. No drift guards.

## 4. Change two — a slicer choice that wins and draws nothing

### 4.1 Two blind readings found the same object

Four fresh subagents, none of which read this brief, the source, or any project document; each saw
one paired image and nothing else; each asked to describe both halves separately, give direction
and location, and say what looked identical. **No page was chosen by `--worst`.** Each was chosen
because a per-page token count said that page carried the document's whole word deficit — a
criterion stated before the images were built.

Two of them, on **unrelated documents and unrelated pages**, reported the same object with the
same direction: *"the reference draws green-outlined boxes reading `This shape represents a
slicer…` and ours draws nothing there"* — three in a row under the chart on
`049_Expenses_calculator` page 1, two stacked in the right margin on `037_Personal_money_tracker`
page 3. Per `HANDOVER.md` § 7 that was checked before being treated as corroboration: the two
reports are about the **same object**, not merely the same sentence. `pdftotext` then confirmed it
independently — the reference draws the advisory **3 / 2 / 1** times on `049` / `037` /
`DynamicBubbleChart` and we drew it **0** times.

### 4.2 The mechanism, and a test that passed for the wrong reason

`OoxmlXml.ResolveAlternateContent` takes an `mc:Choice` when every prefix its `Requires` names
resolves into `OoxmlNamespaces.UnderstoodExtensions`. All three documents write
`<mc:Choice Requires="a14">` around a slicer `graphicFrame`, and **`a14` is `DrawingML2010`, which
is in that set** — so the choice won, the frame had no reader, and the anchor produced no ink and
no words at all.

Round 50 wrote the chartex rule and left a comment saying it "deliberately does not generalise to
the identically-shaped slicer placeholder, because the reference draws that one", with a test —
`OoxmlAlternateContentTests.ASlicerChoiceStillLosesToItsFallback` — asserting exactly that. **The
test passed for the wrong reason**: its helper left the `a14` prefix unbound, so the fallback won
by the *general* rule and the shape the corpus actually contains was never exercised. This is the
project's dominant pattern found inside the project's own test rather than inside a brief: the
claim reproduces, and the sentence attached to it is about a different case.

The fix is the exact mirror of the chartex constant and just as narrow: a choice whose
`a:graphicData/@uri` is the 2010 slicer URI **loses to a sibling `mc:Fallback`**, whatever its
`Requires` says. No fallback, no change.

**The general lesson, which is bigger than the element:** `Requires` names the *vocabulary* a
choice is written in, not whether its content is something a reader can draw. For a slicer the two
answers differ, and MCE gives no way to tell them apart — only the content does.

### 4.3 Reach, measured

Every corpus document parsed and every `mc:Choice` whose `Requires` resolves **entirely** to
understood namespaces collected with the `a:graphicData/@uri` inside it:

| uri inside an understood `mc:Choice` | documents | families |
|---|---:|---|
| `…/word/2010/wordprocessingShape` | 108 | words |
| `…/word/2010/wordprocessingGroup` | 51 | words |
| `…/word/2010/wordprocessingCanvas` | 4 | words |
| **`…/drawing/2010/slicer`** | **3** | **sheets** |
| `…/drawingml/2006/picture` | 1 | words |

The 2010 slicer URI appears anywhere in the corpus bytes in **7** documents, **all sheets**. The
other four — `Part_129_Operators`, `Part_375_Operators`, `TDA_Smoke-Detectors` (all `done`) and
`070_Equipment_inventory_list` (`open`) — write it under `Requires="sle15"`, which is not
understood, so their fallback was already being taken; **all four are unchanged in the sweep**, as
predicted. `070` already draws its three "table slicer" advisories on both sides; its 12-word gap
is our advisory text **wrapping at different points** (`Excel.If`, `TableThis`, `ofversion` are
joined tokens in ours), which is a tokenisation difference inside a shape we do draw.

### 4.4 The one miss, and it is a named blind spot

`037_Personal_money_tracker` was predicted to **overshoot to ~522 against 505 and stay failing**.
It landed at **501 against 505 and matched.** The estimate of 40 word-gate tokens per advisory was
too high because `pdftotext` joins tokens across a line break in a narrow box — blind spot #5 of
the addendum, named in advance and measured on `070` before the change was written. The direction
of the error is the safe one: the advisory is *worth less* than budgeted, so a document predicted
to overshoot instead landed inside the band. `DynamicBubbleChart` landed at **349**, the predicted
figure to the digit, and stays 8 words outside a band of 6.82 — the deliberate 1.2-word miss.

Two font columns also moved toward the reference and were not predicted at all: `049` 4/5 → **5/5**
and `DynamicBubbleChart` 2/3 → **3/3**. The fallback shape brings a face we now embed.

### 4.5 Tests

`ASlicerChoiceStillLosesToItsFallback` is kept and re-commented as the general-rule case, and the
helper now takes the namespace its `Requires` prefix is bound to. Three tests added, all
detectors:

| mutation | detected by |
|---|---|
| the slicer key never matches (the defect put back) | `ASlicerChoiceLosesToItsFallbackEvenWhenItsRequiresIsUnderstood` |
| the rule widened to **any** graphic inside an understood choice | `AnUnderstoodChoiceThatIsNotASlicerStillBeatsItsFallback` |
| the "a fallback must exist" guard dropped | `ASlicerChoiceWithNoFallbackBesideItIsStillTaken` |

**Three of three verified by reintroduction, in both directions.** The widening mutation is the one
that matters: it would swap the shape content of 108 words documents for their VML twins, and the
census row above is why that test exists.

## 5. The round's second result: the `057` legend hypothesis is confirmed, costed and not implemented

Round 51 left the hypothesis *a legend whose series carry no `c:tx` loses its entries, and a legend
with no entries draws nothing*. Tested by authoring two copies of `057_Simple_balance_sheet`
differing in **one thing** — a `<c:tx><c:v>…</c:v></c:tx>` on each of its two series — and
rendering both through `soffice` and through XRay.Office:

| variant | reference legend | our legend |
|---|---|---|
| series carry `c:tx` | `SERIESALPHA`, `SERIESBETA` | **`SERIESALPHA`, `SERIESBETA`** |
| series carry no `c:tx` (the corpus file) | **`Column C`, `Column D`** | **nothing** |

**Confirmed exactly, and the mechanism is located to a line**: `ChartLayout.Entries`
(`XRay.Office.Core/Charts/ChartLayout.cs:3198`) drops a series with no name, and a legend with no
entries draws nothing. LibreOffice's names come from the **Calc data provider**, which synthesises
`STR_COLUMN + " " + column letter` from the series' *values* range
(`sc/source/ui/unoobj/chart2uno.cxx:3173`) — `$C$…` and `$D$…` give `Column C` and `Column D`.

**It is deliberately not implemented, and here is the costing.** Censused over all 946 documents,
a chart that declares a legend and has at least one series without `c:tx` occurs in **2**:

- `057_Simple_balance_sheet` — worth exactly **4 tokens** (`Column`, `Column`, `C`, `D` are the
  *entire* only-in-reference token set), on a document that fails on **page count** (4 against 3),
  so it cannot move a verdict.
- `bitesize-writing-a-report.pptx` — **slides, and currently `done`**. Its chart is a **pie**, so
  its legend enumerates categories rather than series and the `Entries` pie branch already handles
  it; rendered both ways here, its only divergences are URL tokenisation and `Other`/`Others`.

So the change would touch `XRay.Office.Core` — a shared layer — for **zero verdict gain** and a
non-zero risk to a passing slides deck. Measured and left; the next round has the mechanism and
the number.

**And the "reverse direction" half of the round-50 legend framing does not survive either.**
`037` and `029` were named as documents where *we* draw a legend the reference does not:

- **`037` is not a legend defect at all.** Every chart of its that declares a legend has `c:tx` on
  every series. Its 63-word deficit was the **slicer advisory** — which is what § 4 fixed, and it
  now matches.
- **`029` is a chart-data defect, not a legend one.** A blind reviewer of its page 2, given nothing
  but the image, reported that *the reference's "Income and expenses by month" chart has no bars at
  all* — an empty plot with the axis collapsed to its `$0–$12` default — where we draw a `$4,000`
  bar, a `$2,476` bar and `$0 $0` at every other month. The legend we draw and it does not follows
  from that: a legend with empty series has nothing to enumerate. The same reviewer found its
  second chart plots 18 bars in ours against 17 in the reference, the extra one being the `Total`
  category, which rescales the whole axis ($3,000 against $900). All three charts state
  `<c:plotVisOnly val="1"/>`. **That is the next round's hypothesis on `029`, and it is not about
  legends.**

## 6. Four blind readings, and what else they found

Beyond § 4.1 and § 5:

- **`029_Annual_budget` page 2** — as above. Whole +29 deficit accounted for: 20 `$0` labels plus
  `$4,000`/`$2,476` on a chart the reference leaves empty, plus the `Total` bar and the `INCOME`/
  `EXPENSES` legend that follow from it.
- **`049_Expenses_calculator` page 1** — beyond the slicers, the reviewer independently found the
  pivot captions: the reference's legend leads with `Category` and its total column reads
  `Total Result` where ours reads `Grand Total`. That is round 50's PivotTable-regeneration class,
  now with a second witness and worth 1 token on this document.
- **`048_Expense_trends_budget` page 1** — the reviewer reported a constant ~270 px horizontal
  offset and honestly flagged that a composite artefact could explain it. It is not one: both PDFs
  are 612 × 792 and both have 14 pages, and the reference's page 1 begins at `TIPS` at x = 8.2
  where ours begins at `TEMPLATE` at x = 51.4 — the two sides' **horizontal strips do not line
  up**. Nothing is missing; the tiling differs. **A methodological consequence worth carrying: the
  per-page word-count criterion I used to choose review pages is meaningless for any document whose
  horizontal strips are offset**, and `048`'s "+29 on page 1" was such a case.

## 7. Shared layer — a measurement is owed and here is the census

**Yes.** Change two touches **`XRay.Office.Ooxml`** (`OoxmlXml.cs`, `OoxmlNamespaces.cs`), used by
words, slides and sheets. Change one is `XRay.Office.Spreadsheets` only.

The reach is keyed on a single literal — the 2010 slicer graphic-data URI — and a byte search of
every part of all 946 corpus documents finds it in **7 documents, all sheets, 0 words, 0 slides**;
of those, 3 sit inside a choice whose `Requires` is understood and are the three that moved. **The
parent should still run the cross-track sweep.** If anything does move, the documents to look at
are the 108 `wps`, 51 `wpg` and 4 `wpc` words documents in § 4.3, which share the code path but not
the key. `WordProcessing` (1066) and `Presentations` (772) are green and `Fidelity` is byte-identical
to the base at 521/31/552, which is corroboration rather than the claim.

## 8. Build and tests

`dotnet build -v q -nologo` → **0 warnings, 0 errors.**

Ten non-Fidelity projects, run one at a time and totalled by hand: **4614 passed, 0 failed,
1 skipped**, against the base's 4604/0/1 — a delta of exactly the **ten** new tests, seven in
`SheetHiddenValueTests` and three in `OoxmlAlternateContentTests`, all in `XRay.Office.Spreadsheets`
(895 → 905). `Fidelity` is **521 passed, 31 failed, 552 total**, byte-for-byte the base's figure,
so neither change moves any pre-existing failure.

`sheets/done-*` and the whole track were swept together: `TOTAL 325 MATCH 284`, **271 of 307**
against the manifest's path list, **zero regressions, zero page-count changes**.

## 9. Proposed `MANIFEST.tsv` reclassification

`MANIFEST.tsv` lives in the corpus repository and was not touched. Three rows, `status=open` →
`status=done`:

| path |
|---|
| `sheets/chartset-011/xlsx/077_Inventory_list_with_highlighting_Use_this_template_36d8d57a.xlsx` |
| `sheets/chartset-013/xlsx/049_Expenses_calculator_c351f3d0.xlsx` |
| `sheets/chartset-007/xlsx/037_Personal_money_tracker_a57957bb.xlsx` |

## 10. What the next round should do first

1. **`029_Annual_budget` and `plotVisOnly`.** A blind reviewer found the reference plots *nothing*
   on a chart we plot fully, and drops the `Total` category from a second one, rescaling its axis
   from $3,000 to $900. All three of its charts state `<c:plotVisOnly val="1"/>`. If the source
   rows are hidden and LibreOffice honours "plot visible cells only" while we do not, that is one
   mechanism explaining an empty plot, a missing category, an axis scale and a legend at once —
   and it is worth 29 tokens against a band of 6.24 on an open document. **Measure whether those
   rows are hidden before implementing anything.**
2. **`DynamicBubbleChart`, 8 words outside a band of 6.82.** The nearest miss on the track after
   this round, and its remaining gap is now small enough to be a single named thing.
3. **The pie family's `bestFit` data-label placement** — still 4 documents, one defect, ~2 words
   each past the band; untouched by this round.
4. **The eight-blank-line header** — twelve probes in `probes/sheets-r51-bands/` bracket it and
   none explains it; twenty words on `FAA-2019-0995-0002`.

Recorded from blind readings and still unworked: `068_Blue_inventory_list`'s 10-word deficit is two
undrawn arrow autoshapes plus a grey-for-teal title colour; `017_Timeline_Templates` is missing its
navy spine, five year badges and every leader line; `065` draws the literal `aaaa` where the
reference draws `Thursday`; `070_Equipment_inventory_list`'s 12-word gap is our advisory shape
wrapping at different points from the reference's, which is a text-box width question and not a
content one.
