# Round 144 — a Gantt chart drawn out of conditional formats (seat O93), and the hatch mix under it

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. The C++ tree read here is
`/home/user/libreoffice-core` at **27.2.0.0.alpha0+**, which is *not* that binary's source, so
every claim below names its two legs separately: **[src]** = the seat in this tree, **[bin]** = a
measurement against 26.2.4.2's own output.

Witness: `sheets/chartset-010/xlsx/072_Gantt_project_planner_dde00e33.xlsx`.

Two sibling directories hold the work this stands on and are not repeated here:

| directory | what it holds |
| --- | --- |
| `probes/cfexpr-r144/` | the grammar census — every `expression` rule in the corpus, what it is made of, and what the refused ones cover |
| `probes/cfsem-r144/` | the semantics, measured one arm at a time against 26.2.4.2 via `--convert-to fods`, plus `predict.py`, an independent Python implementation that reproduces the reference's painting of **all 1560 cells** of the witness' `H5:BO30` |

---

## 0. The headline

`072_Gantt_project_planner` draws its whole chart out of conditional formats — it holds no
drawing part at all. At the round's base this tree drew **7 filled paths against 26.2.4.2's
1453**. After it, reading our render and the reference's through the same instrument
(`cfsem-r144/witness_read.py`, which anchors on the period numbers in row 4 and the `Activity nn`
labels in column B):

```
cells compared: 1560   mismatches vs 26.2.4.2: 0
```

Every one of the 1560 cells of the chart takes the colour 26.2.4.2 gives it. The same comparison
against `cfsem-r144/predict.py` — which was written from the reference's semantics without seeing
this tree — is also 1560 of 1560.

## 1. Two defects, not one, and only the first is O93

**O93 is the evaluator.** `XlsxConditionalStyles.ConditionOf` sent an `expression` rule to
`Comparison.Parse`, which reads exactly one operator between two operands. All eight of the
witness' rules name a **defined name** — `PercentComplete`, `Actual`, `Plan` and so on, each of
which refers to further names — so all eight were refused and the chart came out blank.

**The second defect was found while verifying the first, and without it the fix cannot be
seen.** With the evaluator in and nothing else, the witness went from 7 filled paths to 1568
against 1453 — but the *colours* were wrong on **117 of the 1560 cells**, always the same way
round: we painted `PercentComplete`'s dark solid where the reference paints `Actual`,
`ActualBeyond` or `Plan`. A unit probe of the evaluator gave the right answers for the same cells,
so the evaluator was right and the reader was feeding it something else. It was not: the three
band rules were **firing correctly and painting the wrong colour**, because all three state the
same foreground and this tree took the foreground whole.

That is worth writing down as a method note. *A wrong colour and a wrong condition are
indistinguishable in a fill census, and a chart built out of one rule per band makes them look
identical.* An hour went into instrumenting the anchor, the sheet values and the defined names —
all three innocent — before the `dxf` table was read.

## 2. A hatched fill is one blended colour, and the blend is arithmetic

**[src]** Calc has no hatched cell background. `Fill::finalizeImport`
(`sc/source/filter/oox/stylesbuffer.cxx`:1978-2056) mixes the pattern colour into the fill colour
at a weight the pattern *name* decides and stores the single result:

- `lclGetMixedColor` (`:1846-1856`) is `(pattern − fill) × alpha / 0x80 + fill` per component, in
  **integer** arithmetic;
- `alpha` comes from the seventeen-entry switch at `:2014-2035` — `lightUp` is `0x20`, `solid` is
  `0x80`, `gray0625` is `0x08`;
- an **automatic** pattern colour resolves to the window *text* colour and an automatic fill
  colour to the window background (`:2038-2050`) — black and white;
- a solid fill is the same arithmetic at full weight, so it is not a special case: it is the
  general one with `alpha = 0x80`, which returns the pattern colour exactly.

**The `mbDxf` branch (`:1985-2002`) is why a `dxf` states its colour in `bgColor`.** A
`PatternFillModel` built for a `dxf` starts with all three parts marked *unused* (`:1753-1762`),
and the branch then does three things in order: a stated background under no pattern or a solid
one is **moved onto the pattern colour** and the pattern forced solid; a solid pattern stating
neither colour is turned off outright; everything else is left alone. So `mnPattern`'s initial
`XML_none` survives whenever a `dxf` states no `patternType` and no `bgColor`, and such a fill
paints nothing however loudly its `fgColor` shouts.

**[bin]** The witness' three band rules state the identical foreground — the theme's accent,
`#735773` — over three different backgrounds, and 26.2.4.2 paints them
`#DCD5DC` (over white), `#B5A1B5` (over the accent at a 60 % tint, `#CAB9CA`) and `#D6BCA8` (over
the second accent at a 60 % tint, `#F6DDB9`). Solving the mix for its weight from the reference's
own pixels gives **0.244, 0.246 and 0.243** on the three components of the first two and 0.244 on
the third — against `lightUp`'s `0x20 / 0x80 = 0.25`, with the shortfall exactly the truncation.

**The rounding is measured and is not a detail.** C++ integer division truncates towards zero, so
a component whose pattern is darker than its fill rounds *up*: `#735773` over `#CAB9CA` at
`lightUp`'s weight is `#B5A1B5`, and flooring would give `#B4A1B4`. The reference draws
`#B5A1B5`. `XlsxPatternFill.Component` is written the way the source writes it for that reason.

## 3. What changed

`XlsxPatternFill` is the rule above, in one place, and **both** readers go through it —
`XlsxConditionalStyles` with `differential: true` and `XlsxCellDecoration` with `false`. The two
used to disagree in the way the old comments described: the `dxf` reader took the foreground and
the cell reader took the background, each right at `solid` and each wrong at every hatch.

`SheetFormula` is the evaluator and is described in `cfsem-r144/results.md`; the only thing worth
repeating here is the confinement guarantee, which is **structural rather than measured**:
`ConditionOf` still tries `Comparison.Parse` first, so what reaches the evaluator is exactly the
set of formulas that used to paint nothing at all.

## 4. Reach, censused by what a fill can *paint*

`census.py`; `census.txt` is its output.

| | occurrences | documents |
| --- | ---: | ---: |
| `dxf` hatches | **11** | **3** |
| `fills` hatch entries | 254 | 244 |
| …of those, named by a `cellXfs` entry | **10** | **3** |

**The 254 is the trap and the 10 is the figure.** Excel writes
`<patternFill patternType="gray125"/>` into index 1 of every workbook it saves, so counting the
*element* finds one per file — 243 of the 254 — and says nothing at all: not one of them is named
by a `cellXfs` entry, so no cell can take it. Census what a fill can paint, not how many fills
there are. (This is C9 arriving on a different element: round 96 learned the same lesson counting
`cfRule`s rather than the cells they reach.)

The six documents between the two rows are:

```
dxf hatches     064_Small_business_cash_flow (7 lightUp), 072_Gantt_project_planner (3),
                062_Run_chart (1 darkDown)
cell hatches    063_Sales_pipeline (lightUp), 072_Gantt_project_planner (lightUp),
                064_Small_business_cash_flow (darkUp)
```

## 5. Reach and confinement, measured

The sheets track — 307 documents, taken from `MANIFEST.tsv` — rendered twice with nothing but the
binary changing, under `SOURCE_DATE_EPOCH=0`, one output directory per document
(`par-sweep.sh`, `sweep-note.md`, `diff-legs.py`):

```
documents scored: 307      failed on either leg: 0
byte-identical:   294      moved: 13
```

`sweep-classified.txt` tags each mover by what this round could reach in it — a `cellXfs` entry
naming a hatched `fills` entry, a hatched `dxf`, or an `expression` rule the two-operand reader
refused (taken from `cfexpr-r144/by-document.tsv` rather than re-derived). **Every one of the 13
is explained and nothing else moved.** Eleven are the evaluator, one is the cell hatch alone, and
the Gantt is both.

Scored against 26.2.4.2 on the 13 (`score.py`, `ink.tsv`) — all three legs rendered on the same
day and **without** `SOURCE_DATE_EPOCH`, because several of these workbooks state `TODAY()` and
pinning our half alone would score a date difference as ink:

| | base | after |
| --- | ---: | ---: |
| summed unsigned ink over the 13 | **24.10** | **9.98** |
| MAJOR pages | **13** | **4** |

**11 improve, 2 are level, none worsens**, and **no page count moves** — all 13 already agreed
with the reference's page count and still do. The largest movers are
`VOR Candidate Discontinuance List` 5.95 → **0.21**, `066_Agile_Gantt_chart` 3.58 → **0.96**,
`027_Simple_personal_cash_flow_statement` 2.98 → **0.65** and the witness itself 1.11 → **0.24**.

**No gate verdict can move and none did**: the alphanumeric count is identical between the two
legs on all 13, which is what a colour and a fill cost. That is the `w:pgBorders` shape again —
if this round were scored by the gate it would read as having done nothing at all.

The one document that moved its bytes and not its ink is `019_Free_Blood_Sugar_Chart`, 4.49 both
ways: its four refused rules now evaluate and paint, and what they paint is too small a part of
nine pages for the page-level measure to register against a residual it already had.

## 6. What the confinement sweep caught, which the tests did not

The first pass of this round moved **15** documents, and two of them —
`075_Idea_planner_tasks` and `Special-Procedures_2025-07-10.xls` — held no hatch and no refused
expression rule. They were a defect this round had just introduced.

Both state a `dxf` of the shape `<patternFill patternType="none"><bgColor auto="1"/></patternFill>`
— a font-only rule writing its "no fill" out longhand. The first cut of `XlsxPatternFill` read an
absent `patternType` and a stated `none` as the same thing, so the `mbDxf` swap fired on them, the
automatic background became an automatic *pattern* colour, and the cell was painted **black**.
LibreOffice does not: `mbPatternUsed` is set by the attribute's mere presence, so the swap's
`!mbPatternUsed || mnPattern == XML_solid` is false for a stated `none` and `mnPattern == XML_none`
then turns the fill off.

Corrected, both documents are byte-identical to the base again and the sweep reports 13.

**The lesson is the sweep's, not the rule's.** Every test in this round passed with that defect in
place, because a test is written for the markup a round is thinking about and this markup is the
markup nobody thinks about. *A confinement sweep is not a formality that confirms a reach figure:
it is the only instrument in the round that looks at the documents you did not choose.* The
`patternType="none"` rows of `XlsxPatternFillTests.AFillThatPaintsNothingAnswersNothing` exist
because the sweep found them.

## 7. The reference states the mixed colour itself

`reference-mixed-colours.py`, `reference-mixed-colours.txt`. 26.2.4.2's own `--convert-to ods` of
the witness writes each conditional style's resolved background as a literal
`fo:background-color`, so the mix can be checked **with no rasteriser, no tolerance and no anchor
anywhere in the comparison**:

| style | reference states | what the `.xlsx` states |
| --- | --- | --- |
| `_25__20_complete` | `#735773` | solid, `bgColor` theme 7 |
| `…_20_legend` (percent beyond) | `#e9ab51` | solid, `bgColor` theme 9 |
| `Actual_20_legend` | `#b5a1b5` | `lightUp`, `fgColor` theme 7 over `bgColor` theme 7 tint 0.6 |
| `Actual_20__28_beyond…` | `#d6bca8` | `lightUp`, `fgColor` theme 7 over `bgColor` theme 9 tint 0.6 |

All four agree with this tree exactly. It is the `--convert-to fods` instrument the `.rtf` and
sheets rounds already record, reaching a question about *colour* — and it should have been the
round's first call rather than its last: it states the answer where a fill census only shows a
consequence of it.

**It also settles the ODF reader**, which needs no mixing at all: the colour arrives already
mixed, because LibreOffice's exporter writes what `Fill::finalizeImport` computed.

## 8. Tests, and what pins them

| file | what |
| --- | --- |
| `SheetFormulaParseTests` | 27, the grammar — including what it must refuse |
| `XlsxPatternFillTests` | 21, the mix, the weights, the `mbDxf` swap and the four fills that paint nothing |
| `XlsxConditionalExpressionTests` | 1, the witness' shape end to end, against 26.2.4.2's own colours |
| `XlsxConditionalStyleTests` | `AFormulaThisCannotEvaluatePaintsNothing` **asserted the opposite until this round** and is rewritten in two: one arm for the formula that is now evaluated, one for a function outside the vocabulary that still is not |

Pinned by three mutations (`mutations.txt`), each rebuilt from a `cp`-restored source with the
project's `obj`/`bin` untouched and the file `touch`ed:

| mutation | caught by |
| --- | ---: |
| the hatch is not mixed, the foreground is taken whole | **13** tests |
| the evaluator is not reached, only the two-operand reader | **2** |
| a stated `patternType="none"` is folded together with an absent one | **2** |

## 9. What is left

- **`ISERROR` is refused on purpose and cannot be answered as the reader stands.**
  `XlsxConditionalStyles.ValueOf` stores an error cell as **blank** (`case "e": return null;`), so
  an `ISERROR` this evaluated would answer false wherever the reference answers true. 18 of the
  corpus's refused rules contain `#REF!`, which the reference cannot evaluate either.
- **`MID`, `EOMONTH` and `_XLFN.ISFORMULA`** appear in `cfexpr-r144/summary.txt`'s function set
  and are not implemented; each parses and answers nothing, so its rule paints nothing.
- **The BIFF reader has both of this round's defects and neither is fixed** — seated as O102.

## 10. Suite state

Every project run separately, counts compared against the previous known-good:

```
Containers 109   Core 591   Markup 259   OpenDocument 194   Presentations 1205
Rendering 164    Spreadsheets 1466   Text 750   Vector 309   WordProcessing 2013
Fidelity  542 passed, 10 failed of 552, 0 skipped
```

The ten are the families this project leaves failing on purpose and two named ones — four
`TabStopComparisonTests`, four `PageDrawingComparisonTests` on `paginated.*`, one
`JustificationShrinkComparisonTests`, and `SheetDrawingComparisonTests` on
`sheet-rich-text.xlsx`, whose own remark files it as 26.2.4.2 clamping a full-cell anchor offset.
**None of the ten can be reached by this round, and that is checked rather than asserted**: of the
36 fidelity fixtures carrying a `patternFill` hatch or an `expression` rule, the hatch is
`gray125` at `fills` index 1 in all 36 — never named by a `cellXfs` entry — and the one expression
rule, on `sheet-cf-multi-range-anchor.xlsx`, is a two-operand comparison the old reader already
accepted.
