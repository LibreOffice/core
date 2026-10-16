# `cfRule type="expression"`: what grammar the corpus actually needs

Round 144, measured 2026-09-17 on `/home/user/sample-files` (947-document corpus).
Everything below is produced by `census.py` in this directory; `summary.txt` is its
stdout and every figure in this file appears in it or in one of the `.tsv` beside it.

```sh
python3 dotnet/probes/cfexpr-r144/census.py > dotnet/probes/cfexpr-r144/summary.txt
```

It reads only the corpus and writes only into this directory. It renders nothing, so
it needs neither `soffice` nor a build, and it is deterministic — no volatile function
is evaluated and no reference is invoked.

## 1. The predicate, and where it was read from

The tree's `expression` arm is
`XlsxConditionalStyles.ConditionOf` (`dotnet/src/XRay.Office.Spreadsheets/Ooxml/XlsxConditionalStyles.cs`),
which for `type="expression"` is `formulas.Count == 1 ? Comparison.Parse(formulas[0]) : null`.

The predicate itself was read from
`dotnet/src/XRay.Office.Spreadsheets/Layout/SheetConditions.cs`, at these lines
(the file was read at 17:4x on 2026-09-17; line numbers are as the file stands now):

| what | lines |
|---|---|
| `Operand.Parse` — quoted string, invariant `double.TryParse`, `TRUE`/`FALSE`, else a reference | 119–143 |
| `Operand.Reference` — refuses any `!`; optional `$`, 1–3 ASCII letters, optional `$`, ASCII digits only, row ≥ 1 | 170–203 |
| `Comparison.Parse` — trim, strip one leading `=`, take the **first** operator from the list that occurs at depth 0, and return null if either side fails | 255–285 |
| `Comparison.Operators` — `<>`, `<=`, `>=`, `=`, `<`, `>`, longest first | 287 |
| `Comparison.IndexOfOperator` — a single pass tracking `"` and paren depth; an operator inside a quoted string or inside any parenthesis is invisible | 298–320 |

`census.py`'s `parse_operand`, `parse_reference`, `index_of_operator` and
`comparison_parse` are a line-for-line port of those five, and the port's comment cites
them. Two consequences of the C# that matter and that a paraphrase loses:

* `Comparison.Parse` **does not backtrack**. Once an operator from the list is found at
  depth 0, a failing operand returns null rather than trying the next operator. So
  `AND($C8>=valHStart, $C8<=valHEnd)` is refused because it has no depth-0 operator at
  all, while `#REF!<=TODAY()+7` is refused at the first `<=` it finds.
* A top-level `*`, `+`, `-` or `/` is not an operator here, so
  `(MOD(ROW(),2)<>0)*($B12="Income")` has no depth-0 operator and is refused.

**Note on the working tree.** While this census was being taken,
`SheetConditions.cs` gained an `Expression` record and a new `SheetFormula.cs`
appeared (both uncommitted, `git status` at the time of writing), and
`ConditionOf`'s `expression` arm became
`Comparison.Parse(...) ?? Expression.Parse(...)`. That is a peer agent implementing
exactly this area. **The census is against the committed predicate** — `Comparison.Parse`
alone, whose body is unchanged — which is the right baseline: the "refused" set below is
precisely the set an implementation has to cover.

## 2. What was walked

`census.py` opens every file under the corpus root whose first two bytes are `PK` and
whose zip holds `xl/workbook.xml`. Extensions `.xlsx .xlsm .xltx .xltm .xlam` are taken
case-insensitively; `.xls .xlsb .zip` and extensionless files are sniffed for the same
signature.

* **244 OPC spreadsheets**, of which **1 has a lying extension**:
  `sheets/done-010/xls/Special-Procedures_2025-07-10.xls` is an OPC zip. (The 243 in
  `XlsxConditionalStyles`' own remarks is the extension-keyed count; this is that plus
  the sniffed one.)
* **809 worksheets** parsed.
* **601 `cfRule type="expression"` rules in 34 documents** — which reproduces the figure
  in `XlsxConditionalStyles`' remarks exactly.

### 2.1 `x14:cfRule` in a worksheet's `extLst`

The walk covers them: it iterates `{…/2009/9/main}conditionalFormatting` anywhere in the
worksheet and reads `{…/2006/main}sqref` and `{…/2006/main}f` off each rule.

**It found 26 `x14:conditionalFormatting` blocks holding 27 `x14:cfRule`, of which
`iconSet` 18 and `dataBar` 9 — and `expression` zero.** So the extension list contributes
**0 expression rules** to this census. (An independent raw-text scan of every
`xl/worksheets/*.xml` in the corpus agrees: 27 `x14:cfRule`, the same two types.)

The current reader cannot see any of them in any case:
`XlsxConditionalStyles.ReadRules` walks `Xlsx.Children(worksheet, "conditionalFormatting")`,
which is direct SpreadsheetML-namespace children of the worksheet root.

## 3. Accepted against refused

| | rules | documents |
|---|---:|---:|
| accepted by `Comparison.Parse` | **483** | 32 |
| refused | **118** | **23** |
| total | 601 | 34 |

Per rule: `rules.tsv`. Per document: `by-document.tsv`.

Of the 118 refused, **0** have `formulas.Count != 1` and **0** lack a usable `dxfId`, so
every one of them is refused by the formula grammar and nothing else.

The 483 accepted are dominated by one shape: 223 distinct formulas, the commonest being
`H1="x"` (108 rules), `I1="x"` (29) and `H2="x"` (24) — consistent with the "461 of the
single shape `<reference> = "<text>"`" already recorded in the source.

## 4. Token inventory of the refused rules

### 4.1 Functions — `functions.tsv`

`can_paint_rules`/`can_paint_cells` count only the refused rules that pass §6's test.

| function | calls | docs | cells | calls after expansion | cells after expansion | can-paint rules | can-paint cells |
|---|---:|---:|---:|---:|---:|---:|---:|
| `AND` | 57 | 9 | 38444 | 57 | 38444 | 56 | 35756 |
| `TODAY` | 14 | 6 | 10079 | 14 | 10079 | 8 | 5263 |
| `MEDIAN` | **0** | 0 | 0 | **7** | **7800** | 5 | 7800 |
| `MOD` | 24 | 4 | 6695 | 24 | 6695 | 24 | 6695 |
| `LEN` | 6 | 1 | 4536 | 6 | 4536 | 6 | 4536 |
| `ROW` | 22 | 3 | 3575 | 22 | 3575 | 22 | 3575 |
| `ROUNDDOWN` | 5 | 2 | 3136 | 5 | 3136 | 5 | 3136 |
| `COLUMN` | 2 | 1 | 3120 | 2 | 3120 | 2 | 3120 |
| `INT` | **0** | 0 | 0 | **2** | **3120** | 2 | 3120 |
| `ISBLANK` | 1 | 1 | 2016 | 1 | 2016 | 1 | 2016 |
| `NOT` | 1 | 1 | 2016 | 1 | 2016 | 1 | 2016 |
| `ISERROR` | 12 | 4 | 1654 | 12 | 1654 | 12 | 1654 |
| `MID` | 2 | 1 | 1520 | 2 | 1520 | 2 | 1520 |
| `EOMONTH` | 18 | 1 | 426 | 18 | 426 | 9 | 426 |
| `_xlfn.ISFORMULA` | 1 | 1 | 186 | 1 | 186 | 1 | 186 |
| `ABS` | 2 | 1 | 18 | 2 | 18 | 2 | 18 |

**Un-expanded: 14 functions. After defined-name expansion: 16** — `MEDIAN` and `INT` are
reached only through a name, and they are not a tail: `MEDIAN` covers 7800 cells, more
than nine of the fourteen that are visible without expanding.

### 4.2 Defined names — `names.tsv`

**15 names are referenced directly by a refused rule**, across 7 documents:

`Actual`, `ActualBeyond`, `Cash_Minimum`, `FlagPercent`, `MaxDiastolic`, `MaxSystolic`,
`PercentComplete`, `PercentCompleteBeyond`, `Plan`, `period_selected`, `task_end`,
`task_progress`, `task_start`, `valHEnd`, `valHStart`.

**2 further names are reached transitively**: `PeriodInActual` and `PeriodInPlan`, both
in `072_Gantt_project_planner`. **Nothing is unresolved** — every name a refused rule
mentions resolves against the workbook's `definedNames`, at depth ≤ 2.

Eleven of the seventeen expand to a single qualified cell reference
(`'Cash flow forecast'!$J$4`, `'Project schedule'!$F1`, …). The other six are
`072_Gantt_project_planner`'s, and they are the expensive ones — `PercentComplete`
expands to 384 characters of nested `MEDIAN`, `INT`, `*`, `+` and `=` over
`'Project Planner'!A$4` and six relative columns.

### 4.3 Operators — `operators.tsv`

| operator | occurrences | after expansion |
|---|---:|---:|
| `=` | 65 | 74 |
| `<=` | 47 | 47 |
| `>=` | 46 | 46 |
| `+` | 41 | 52 |
| `-` | 42 | 47 |
| `*` | 17 | 30 |
| `>` | 15 | 22 |
| `<>` | 14 | 14 |
| `<` | 13 | 15 |
| `/` | 2 | 2 |

No `^`, no `&`, no `%`. (An earlier cut of the script reported two `&`; they were inside
the single-quoted sheet name `'Settings & Calculations'`, and the tokeniser now blanks
single-quoted names as well as double-quoted strings before counting operators.)

### 4.4 Reference forms — `reference-forms.tsv`

| form | occurrences | docs | after expansion | docs after |
|---|---:|---:|---:|---:|
| `A$1` (absolute row) | 112 | 5 | 112 | 5 |
| `$A1` (absolute column) | 89 | 7 | 89 | 7 |
| `A1` (fully relative) | 26 | 8 | 26 | 8 |
| `$A$1` | 22 | 3 | 22 | 3 |
| `Sheet!A1` | **0** | 0 | **91** | **6** |

**No range (`A1:B2`), no whole-column (`A:B`), and no structured/table reference appears
anywhere in the refused set**, before or after expansion. The qualified form appears only
after expansion — every defined name in the corpus writes its target as
`'Sheet Name'!$C$18` — so cross-sheet resolution is entirely a consequence of supporting
defined names, and is needed in 6 documents.

`#REF!` appears in **18 rules in 3 documents** covering 1901 cells.

### 4.5 Shapes — `shapes.tsv`

A coarse top-level classification of the 118, for scoping:

| shape | rules | docs | cells | can-paint rules | can-paint cells |
|---|---:|---:|---:|---:|---:|
| `AND(...)` of 2–3 comparisons | 42 | 9 | 37604 | 41 | 34916 |
| bare defined name / bare `TRUE` | 6 | 1 | 7866 | 5 | 7800 |
| comparison with a function on one side | 21 | 8 | 5928 | 20 | 3800 |
| bare function call, no comparison | 14 | 6 | 3400 | 14 | 3400 |
| boolean arithmetic `(...)*(...)` | 12 | 1 | 1994 | 12 | 1994 |
| contains `#REF!` | 18 | 3 | 1901 | 18 | 1901 |
| comparison against a defined name | 5 | 3 | 1648 | 5 | 1648 |

Two of those shapes are not boolean. `MOD(COLUMN(),2)` and `PercentComplete` have no
comparison in them at all; LibreOffice's `ScConditionMode::Direct` fires on a non-zero
*number*, so an implementation needs a numeric result, not a predicate.

## 5. Ranked by cells covered — `by-cells.tsv`

Top of the ranking (full table in the TSV):

| cells | document | sqref | formula | can paint |
|---:|---|---|---|---|
| 2688 | `085_Simple_Gantt_chart` | `I4:BL31 I21:BL25 …` | `AND(TODAY()>=I$5, TODAY()<J$5)` | no |
| 2128 | `015_Free_Gantt_Chart_Template_for_Excel` | `K6:BN43` | `K$6=TODAY()` | no |
| 2016 | `015_Free_Gantt_Chart_Template_for_Excel` | `K8:BN43` | `AND($E8<=K$6,ROUNDDOWN(($F8-$E8+1)*$H8,0)+$E8-1>=K$6)` | yes |
| 2016 | `015_Free_Gantt_Chart_Template_for_Excel` | `K8:BN43` | `AND(NOT(ISBLANK($E8)),$E8<=K$6,$F8>=K$6)` | yes |
| 1680 ×3 | `066_Agile_Gantt_chart` | `I7:BL36` | `AND(TODAY()>=I$7,TODAY()<J$7)` | yes |
| 1560 ×8 | `072_Gantt_project_planner` | `H5:BO30` | `PercentComplete`, `Actual`, `Plan`, `MOD(COLUMN(),2)`, … | yes |

Total `sqref` area over the 118 refused rules is **60341 cells**; over the 115 that can
paint, **55459**.

**Documents holding refused rules**, refused-rule count and refused cells
(`by-document.tsv`, ordered by refused count):

| document | expr rules | refused | refused cells | refused that can paint |
|---|---:|---:|---:|---:|
| `sheets/chartset-011/xlsx/066_Agile_Gantt_chart_08f9de45.xlsx` | 42 | 42 | 28146 | 42 |
| `sheets/chartset-014/xlsx/027_Simple_personal_cash_flow_statement_675c6584.xlsx` | 12 | 12 | 1994 | 12 |
| `sheets/chartset-010/xlsx/072_Gantt_project_planner_dde00e33.xlsx` | 10 | 10 | 12606 | 9 |
| `sheets/chartset-009/xlsx/085_Simple_Gantt_chart_d65b8377.xlsx` | 9 | 9 | 4928 | 8 |
| `sheets/chartset-008/xlsx/023_Waterfall_Chart_Template_for_Excel_349f7689.xlsx` | 8 | 8 | 228 | 8 |
| `sheets/done-003/xlsx/VOR Candidate Discontinuance List 2026-01-08.xlsx` | 8 | 8 | 1419 | 8 |
| `sheets/chartset-009/xlsx/015_Free_Gantt_Chart_Template_for_Excel_414e86bf.xlsx` | 4 | 4 | 6272 | 3 |
| `sheets/chartset-013/xlsx/019_Free_Blood_Sugar_Chart_for_Excel…xlsx` | 4 | 4 | 1540 | 4 |
| `sheets/chartset-007/xlsx/040_Blood_pressure_tracker_872b6833.xlsx` | 2 | 2 | 16 | 2 |
| `sheets/chartset-007/xlsx/083_Project_tracker_Use_this_template_b38d057c.xlsx` | 2 | 2 | 18 | 2 |
| `sheets/chartset-009/xlsx/024_Free_Kids_Chore_Chart_Template_ad74bb34.xlsx` | 2 | 2 | 162 | 2 |
| `sheets/chartset-011/xlsx/071_Four-week_project_timeline…xlsx` | 5 | 2 | 56 | 2 |
| `sheets/done-011/xlsx/Application_Compliance_Checklist_5_Apr_2021.xlsx` | 32 | 2 | 1048 | 2 |
| `sheets/table-001/xlsx/FAA-2019-0995-0002_attachment_2.xlsx` | 2 | 2 | 1520 | 2 |
| `sheets/chartset-006/xlsx/044_Cash_flow_forecast…xlsx` | 1 | 1 | 12 | 1 |
| `sheets/chartset-006/xlsx/088_To-do_list_with_progress_tracker…xlsx` | 1 | 1 | 40 | 1 |
| `sheets/chartset-007/xlsx/037_Personal_money_tracker_a57957bb.xlsx` | 3 | 1 | 1 | 1 |
| `sheets/chartset-007/xlsx/082_Project_to_do_list_28f80082.xlsx` | 1 | 1 | 54 | 1 |
| `sheets/chartset-010/xlsx/047_Date_tracker_Gantt_chart_bf34f3a8.xlsx` | 1 | 1 | 13 | 0 |
| `sheets/chartset-011/xlsx/028_Budget_summary_report_4aeefd13.xlsx` | 1 | 1 | 186 | 1 |
| `sheets/chartset-012/xlsx/036_Simple_to-do_list…xlsx` | 3 | 1 | 42 | 1 |
| `sheets/chartset-014/xlsx/038_Baby_growth_chart_bb9cd672.xlsx` | 1 | 1 | 20 | 1 |
| `sheets/chartset-014/xlsx/039_Baby_growth_tracker_e295ae56.xlsx` | 1 | 1 | 20 | 1 |

Eleven further documents state expression rules and **all** of them are accepted —
including `TK-Syllabus-Comparison-Document-v2.xlsx`, whose 413 rules are all accepted
already, so none of this reaches the workbook that seated the area.

## 6. A rule count is not a reach figure

For every refused rule `rules.tsv` records four independent reasons it might paint
nothing at all.

| test | refused rules failing it |
|---|---:|
| sheet is `hidden` or `veryHidden` | **0** |
| `dxfId` resolves to a `dxf` that paints nothing (border-only, or a `patternFill` that resolves to no colour) | **3** |
| `sqref` misses the extent the reader walks (`ReadSheet`'s own: every stated `<row>`/`<c>` plus `<cols>/@max`) | **1** |
| `sqref` misses a stated print area | **0** (22 of the 118 are on a sheet that states one) |

**115 of the 118 refused rules can actually paint something.** The three that cannot are:

* `085_Simple_Gantt_chart`, `AND(TODAY()>=I$5, TODAY()<J$5)` over 2688 cells — `dxfId`
  names a border-only `dxf`.
* `015_Free_Gantt_Chart_Template_for_Excel`, `K$6=TODAY()` over 2128 cells — `dxfId 2` is
  a border-only `dxf`.
* `072_Gantt_project_planner`, `TRUE` over `B31:BO31` — `dxfId 1` is border-only **and**
  the range sits one row below the walked extent `B1:BO30`.

Note the shape of that: the two largest refused rules in the whole corpus are both among
the three that cannot paint.

**One further caveat, stated separately because it is about the reference and not about
the tree.** 18 of the 115 — all of `066_Agile_Gantt_chart`'s row-36 rules and
`Application_Compliance_Checklist`'s two `#REF!="N/A"` rules — have `#REF!` in the
formula. An error propagates, so 26.2.4.2 cannot paint those either, whatever an
implementation does with them. This is not an evaluation of what the reference computes;
it is the one case where the grammar itself is an error token.

**Excluding those: 97 refused rules, covering 53558 cells, in 21 documents.**

## 7. Files

| file | what |
|---|---|
| `census.py` | the whole measurement; reads the corpus, writes everything else here |
| `summary.txt` | its stdout — every headline figure above |
| `rules.tsv` | one row per expression rule: document, sheet, state, origin, sqref, cells, dxfId, priority, accepted, formula, the three reach tests, the two extents |
| `by-cells.tsv` | the 118 refused rules ranked by `sqref` area, with `can_paint` |
| `by-document.tsv` | per-document totals |
| `functions.tsv` | function inventory, un-expanded and expanded, with can-paint columns |
| `names.tsv` | every defined name a refused rule references, its definition, its transitive expansion and the chain |
| `operators.tsv` | operator inventory |
| `reference-forms.tsv` | reference-form inventory |
| `shapes.tsv` | top-level shape classification |
