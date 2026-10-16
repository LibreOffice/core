# The rest of the conditional-format rule families, and why 1215 rules move three renderings

Round 96, seated by `probes/sheet-ink-r94/results.md` §7 and register entry **O20**. Base
`ecd15f9ac`, worktree `/home/user/wt-condformat`, reference `/opt/libreoffice26.2/program/soffice`
(LibreOffice 26.2.4.2). **The reference half of every rendering figure is the bank at
`/home/user/gate-orig-r83/ref/`, reused rather than re-rendered**; only our half was run, which a
diff confined to `dotnet/src` makes sound. Load average was 5.26 at the start of the round and
2.72 at the end, with other rounds building in their own worktrees throughout; no gate figure is
quoted here, and the two sweeps below are our half alone at two frozen binaries, which contention
cannot bias in either direction.

**There is no `Task`/subagent tool in this container**, so no page was read by anyone
uncontaminated. Nothing below rests on a reading: the fixtures' ground truth is 26.2.4.2's own
PDF read back as filled rectangles and coloured text spans, and the corpus result is an operator
census beside `pdf-image-diff.py`.

---

## 1. What LibreOffice does, and the four places it is not the specification

`ScConditionMode` (`sc/inc/conditio.hxx`:61-89) has 25 values and
`CondFormatRule::finalizeImport` (`sc/source/filter/oox/condformatbuffer.cxx`:799-905) is the whole
of how a `cfRule`'s `type` becomes one. Read end to end, four of its arms answer differently from
what 18.3.1.10 would lead a reader to write, and each was then confirmed against the reference's
own view of a real corpus document **and** of a fixture authored for it.

**(1) `containsBlanks` and `notContainsBlanks` are not modes at all.** `finalizeImport`:860-866
gives them a *replacement formula* — `LEN(TRIM(#B))=0` and `LEN(TRIM(#B))>0`, where `#B` is
`FormulaProcessorBase::generateAddress2dString(ranges.GetTopLeftCorner(), false)` — clears
`maModel.maFormulas`, appends that, and sets `ScConditionMode::Direct`. So **a cell holding only
spaces is blank**, which the specification's "blank" is not, and **a number is never blank**
because `TRIM` stringifies it first. 26.2.4.2's own `--convert-to fods` of
`Application_Compliance_Checklist_5_Apr_2021.xlsx` writes each of its 99 such rules out as
`formula-is(LEN(TRIM([.M270]))=0)`, and its rendering of `sheet-cf-blank-cells.xlsx` paints the
three-space `A2` with the *blank* rule's colour.

**(2) A text rule's `<formula>` is dead markup; the `text` attribute is the rule.** For
`containsText`, `notContainsText`, `beginsWith` and `endsWith`, `finalizeImport`:938-947 builds a
token array holding **only** `maModel.maText`, interned — the `NOT(ISERROR(SEARCH("xxx",A373)))`
Excel writes beside it is never compiled. That is why `--convert-to fods` writes them as
`contains-text("xxx")` and `ends-with("Closed")` with no reference in them at all. A reader that
took the formula would need `SEARCH` and `ISERROR` to answer what one attribute states.

**(3) A text rule folds case for a text cell and does not for a numeric one.**
`ScConditionEntry::IsValidStr` (`conditio.cxx`:1219-1229) lowercases both sides through
`ScGlobal::getCharClass()` unless the document is case-sensitive, which it is not by default;
`IsValid`, the numeric arm (`:1130-1143`), stringifies the cell with `OUString::number(nArg)` and
calls a bare `indexOf` with no lowercasing anywhere in it. A numeric cell is therefore searched as
its *number*, not as what is drawn — `1234.5` shown as a currency is searched as `1234.5`.

**(4) A multi-range `sqref`'s base cell is not the componentwise minimum.**
`ScRangeList::GetTopLeftCorner` (`sc/source/core/tool/rangelst.cxx`:1142-1155) returns the
smallest range *start* under `ScAddress`'s own ordering, and that ordering is `(tab, col, row)` —
`std::make_tuple(nTab, nCol, nRow) <=> …`, `sc/inc/address.hxx`:396. **Excel writes the formula
against the componentwise minimum and LibreOffice reads it against the smallest column, ties by
row**, so on a `sqref` naming several ranges the two disagree about which cell a relative
reference resolves to. This one is not an inference:

- `Application_Compliance_Checklist_5_Apr_2021.xlsx` states `$G376="N/A"` on
  `G443:G444 D491 G446:G490 G377`, whose componentwise minimum is `D377` and whose top-left corner
  is `D491`. 26.2.4.2's own fods writes `calcext:base-cell-address="…D491"`, and gives the sibling
  block anchored at `A377` a resolved `formula-is(#ref!="N/A")` — the shift took the row negative.
- On the authored fixture `sheet-cf-multi-range-anchor.xlsx`, `$B1="hit"` on `B3 A5`, the fods says
  `base-cell-address="Rules.A5"` and the PDF **paints A5 and not B3**. Anchoring on `A3` paints
  exactly the other cell, so the fixture separates the two readings rather than merely exercising
  one.

**15 blocks in 3 corpus documents** have a componentwise minimum different from their top-left
corner; the tree used the componentwise minimum and now uses the reference's.

**A fifth, smaller correction, also the reference's.** Precedence between two
`<conditionalFormatting>` blocks is **document order, then priority within a block** — not a
global sort on `priority`. A cell's pattern carries an `ScCondFormatIndexes`, an
`o3tl::sorted_vector<sal_uInt32>` of *format* indices (`sc/inc/attrib.hxx`:271);
`ScDocument::GetCondResult` (`documen4.cxx`:1119-1143) walks it in that order and the index is
handed out by `AddCondFormat` as each block finalises, so it is document order. Within a block
`CondFormat::insertRule` (`condformatbuffer.cxx`:1173-1180) keys `maRules` by priority in a
`std::map`, so those are ascending. The tree sorted globally by priority.

**And two evaluation rules that are not in the importer at all.**
`ScConditionEntry::FillCache` (`conditio.cxx`:804-859) walks `pCondFormat->GetRange()` — the whole
`sqref`, not the sheet — skipping every empty cell, and counts numbers in one map keyed by value
and strings in another keyed by the lowercased string. `IsDuplicate` (`:861-885`) then consults
only the map its own cell belongs to, so **the number 7 and the string `7` are not duplicates of
each other**; the fixture shows the reference leaving a string `7` plain beside two red numeric
`7`s. And the same function's `if(nRow == mrDoc.MaxRow()) ShrinkToUsedDataArea` (`:820-828`) is the
reference's own version of the clamp this reader already applies.

---

## 2. The census, re-run, and where the brief's differs

Counted 2026-09-11 by parsing every worksheet of all **243** corpus `.xlsx`/`.xlsm`
(`census-cfrules.py`, `cfrule-census.txt`), rather than by matching `type="…"` as text:

| `cfRule` type | rules | documents | state after this round |
|---|---:|---:|---|
| `containsText` | 927 | 5 | **read** |
| `expression` | 601 | 34 | read (r94) |
| `cellIs` | 123 | 18 | **read already at r94** — the brief's table has this wrong |
| `containsBlanks` | 99 | 1 | **read** |
| `colorScale` | 43 | 38 | read, `XlsxConditionalFormats` |
| `duplicateValues` | 20 | 5 | **read** |
| `dataBar` | 9 | 6 | left, see §5 |
| `endsWith` | 5 | 1 | **read** |
| `notContainsBlanks` | 3 | 2 | **read** |
| `iconSet` | 2 | 2 | left, see §5 |
| `beginsWith`, `notContainsText`, `uniqueValues`, `containsErrors`, `notContainsErrors`, `top10`, `aboveAverage`, `timePeriod` | **0** | 0 | nil reach, not implemented |

The brief's `dataBar` 18 and `iconSet` 20 are over-counts, exactly as it warned they might be: the
strings appear again as *element* names and inside the `x14` extension list, so a grep for
`type="dataBar"` across the raw XML sees each rule more than once. `colorScale` 41 → 43 and
`cellIs` "not read" are the other two corrections.

**And an extension-keyed census misses a mislabelled file.** `Special-Procedures_2025-07-10.xls`
is `Microsoft Excel 2007+` — a zip — carrying **6 `containsText` rules**, and no census filtered on
`.xlsx`/`.xlsm` can see it. It is one of the three documents this round moves. Census by content.

---

## 3. What moved, over the whole corpus

Rendered **our half of all 947 corpus documents twice**, at the round's base and with the change,
under `SOURCE_DATE_EPOCH`, one output directory per *document*, each leg driven by a frozen copy of
the binary outside the tree so no build could swap it mid-sweep (`sweep-base-hashes.tsv`,
`sweep-after-hashes.tsv`). Both legs completed **947 of 947** with no failure on either side.

**3 renderings differ and 944 are byte-identical.** All three are SpreadsheetML on the sheets
track; the words and slides tracks do not change by a byte, and neither does any `.doc`, `.ppt`,
`.xls` that really is BIFF, `.ods` or `.odt`.

| | base \|ink\|% | after \|ink\|% | pages | MAJOR |
|---|---:|---:|---:|---|
| `wiley-cancelled-title-list__xlsx` | 0.42 | **0.01** | 60 | 0 → 0 |
| `6880ac7361ca…ST Capability List Rev.16 - Web__xlsx` | 27.04 | **26.85** | 217 | 0 → 0 |
| `Special-Procedures_2025-07-10__xls` | 0.16 | 0.16 | 22 | 0 → 0 |
| sum | **27.62** | **27.02** | | |

**The corroboration that is not a reading** is an exact span census — the colour of every text span
PyMuPDF reports, which unlike a fill-operator count is immune to the reference's rectangle
coalescing:

| | our base | **ours after** | **26.2.4.2** |
|---|---:|---:|---:|
| `wiley` — `#FFC7CE` fills / `#9C0006` spans | 0 / 0 | **2 / 2** | **2 / 2** |
| `ST Capability` — `#9C0006` spans | 0 | **18** | **16** |
| `Special-Procedures` — `#008000` / `#FF0000` spans | 0 / 0 | **1 / 2** | 2 / 3 |

`wiley` is exact on both columns. On `ST Capability` the two sides agree **span for span,
text for text, on pages 25, 26, 27 and 28** — all sixteen of the reference's — and the two extra
are one value on pages 1 and 85; §4 says what they are. On `Special-Procedures` the three spans we
paint are the reference's three *conditional* ones exactly; its other two, `green font` and
`red font` on page 1, are a legend cell's own rich-text runs and not a conditional format at all.

### The result the round is actually about: **a rule count is not a reach figure**

1215 unread rules reduce to **three renderings**, and the reason is worth more than the fix. Of the
nine candidate documents that did not move (`ink-nonmovers.tsv`), every one is explained, and none
of the explanations is a defect:

- **`flightstandards-doc-Cross-reference-table_version02.xlsx`, 358 `containsText` rules, 18.17 %
  ink.** Every rule searches for `xxx` and **no cell on the sheet holds it** — the leftovers of a
  search-highlight the author never removed. The reference paints none of them either.
- **`Application_Compliance_Checklist_5_Apr_2021.xlsx`, 668 rules, 6.54 % ink.** 287 `containsText`
  + 50 `containsBlanks` + 4 `endsWith` are on a `state="veryHidden"` sheet; the remaining 277 + 49
  + 1 are on the visible sheet's columns **L and M, and that sheet hides columns 10 to 19**. Zero
  visible cells.
- **`fm-provider-service-measures` (21.67 %), `FAA-2019-0995-0002_attachment_2` (11.97 %) and
  `SSRO_Quarterly_Statistical_Bulletin` (0.10 %)** state 14 `duplicateValues` rules between them
  over 357 positions, and **no value occurs twice in any of them**.
- `Chicago.List.2025` searches 720 positions for `1924` and no cell holds it;
  `Computer and Software Services` searches one cell, `F4`, which does not hold `No Sales Tax`.
- `028_Budget_summary_report` and `036_Simple_to-do_list` have 66 `notContainsBlanks` hits between
  them, all in helper columns outside the printed block.

So the three ink seats the brief hoped these families would take — 27.04, 21.67 and 18.17 — are
**not conditional formatting**, and `Application_Compliance_Checklist` is not either. They keep
their seats and this round has retired the hypothesis rather than the divergence.

---

## 4. The one over-paint, and it is a string-normalisation question

`ST Capability List` paints `D5758620001301` red on pages 1 and 85 where the reference paints
nothing. The two cells are `A3`, whose shared string is **`"\tD5758620001301"` with a leading
tab**, and `A2805`, which is `"D5758620001301"`. To the reference those are different keys and
neither is a duplicate; to this tree they are the same string, because `XlsxCellText.Of` drops a
lone `U+0009` in a string holding no line feed — which is the right answer for **drawing** the
cell, measured over three spellings of the character, and the wrong one for **comparing** it.

**The drawn text and the compared value are two different strings**, and this reader has only the
first. Closing it means keeping the raw shared string beside the normalised one. Reach: one cell in
one corpus document, two spans; recorded as **O21** rather than taken, because the same document's
other sixteen spans already agree exactly and the change is an API widening for two.

---

## 5. What is left, and what is deliberately not taken

**`dataBar` (9 rules, 6 documents) and `iconSet` (2 rules, 2 documents) are left, and they belong
to the other file.** On the reference side they are `ScDataBarFormat` and `ScIconSetFormat` in
`sc/source/core/data/colorscale.cxx` and not `ScConditionEntry` at all: like a colour scale they
state no format and compute their answer from the numbers in their own range, so their home here is
`XlsxConditionalFormats`, whose remarks now say so. They also *draw* — a bar and a glyph over a cell
— rather than format one, which is a different sink from `SetConditionalBackground`. Eleven rules
in seven documents is the whole reach, and none of the seven is in the sheets ink ranking's top
forty.

**The eight families with no corpus witness are recorded as nil reach and not implemented.**
`beginsWith`, `notContainsText` and `uniqueValues` are one arm apiece of predicates now in the file
and would cost three lines; they are still code no document exercises, and a fixture authored to
test them would be testing my reading of `IsValidStr` against my reading of `IsValidStr`.

**The seam, if anyone wants a shared evaluator.** `agent/odsresidue` owns the ODF `style:map`
spelling and the BIFF `CONDFMT`/`CF` path, and this round did not reach across. The right shape if
the three are ever unified is visible from here: **the condition and the difference are already
separate types** — `ICondition` with a single `Holds(sheet, row, column, anchor…)`, and a
`Difference` that is a `SheetConditionalText` plus an optional fill — and only `ConditionOf` and
`ReadDifferences` are SpreadsheetML. An ODF or BIFF reader would supply its own two and reuse the
rest, including the ordering rule of §1 and the `Sheet` value model. The seam is left where it is
rather than moved, because two of the three consumers do not exist yet.

---

## 6. The fixtures

Five, under `dotnet/tests/corpus/features/`, one per family implemented plus one for the anchor,
each about 2.5 kB, each with a unique stem — `soffice --convert-to` names its output after the stem
alone. `make-fixtures.py` authors them and every part a real writer emits is present:
`[Content_Types].xml`, both `_rels` parts, `workbook.xml`, `styles.xml` with a real `dxfs` table,
`sharedStrings.xml` and one worksheet.

| fixture | what it separates |
|---|---|
| `sheet-cf-contains-text.xlsx` | unanchored, case-folded, and a numeric cell searched as its number |
| `sheet-cf-ends-with.xlsx` | tail only, case-folded, and a cell shorter than the needle |
| `sheet-cf-blank-cells.xlsx` | three spaces are blank; `0` is not; and both rules on one range |
| `sheet-cf-duplicate-values.xlsx` | the case-folded key, and `7` against `"7"` |
| `sheet-cf-multi-range-anchor.xlsx` | `GetTopLeftCorner` against the componentwise minimum — the two paint **opposite** cells |

**Each fixture's expectation is 26.2.4.2's own output, not a prediction.** Each was converted
twice: `--convert-to fods` for the rule the reference thinks it imported
(`fixture-reference/rules.txt`) and `--convert-to pdf` for which cells it then painted, read back
as filled rectangles and coloured text spans (`fixture-reference/painted.txt`).

**Sanity-checked against real documents, which the `office-corpus` skill requires and which
matters here.** The two findings a fixture could most easily have invented are the blank rule and
the anchor, and both were established on a corpus document *first* — the `LEN(TRIM(…))` spelling on
`Application_Compliance_Checklist`'s 99 rules and the `D491` base cell on the same file's
`$G376="N/A"` block — and only then reproduced on a minimal file. The fixtures agree with the
corpus documents on both.

`XlsxConditionalPredicateTests` reads the five fixtures out of the package and asserts the
reference's answers. **All five fail at the round's base and pass with the change**, checked by
reverting the two source files, rebuilding and re-running: `Failed: 5, Passed: 0` before,
`Failed: 0, Passed: 15` after (the five new beside `XlsxConditionalStyleTests`' ten).

---

## 7. Confinement and the suite

- Both sweep legs used a frozen copy of the binary outside the tree; rebuilding the after leg from
  clean `obj`/`bin` under `SOURCE_DATE_EPOCH` reproduces `XRay.Office.Spreadsheets.dll` **byte for
  byte** (`29b12a3dd7986191df25616bd01c7d38`).
- 944 of 947 renderings byte-identical, 3 movers, 0 failures either leg.
- Zero build warnings. Ten non-fidelity projects: 521 + 728 + 109 + 309 + 164 + 259 + 146 + 1260 +
  1864 + 1044 = **6404 passed, 0 failed, 0 skipped**. `XRay.Office.Fidelity.Tests`: **542 passed, 10
  failed**, and the ten are exactly the expected set — `PageDrawingComparisonTests.EveryLineIsDrawn`
  ×4, `TabStopComparisonTests.AListLabelsTabAdvance` ×4,
  `SheetDrawingComparisonTests.APictureIsDrawn` and `JustificationShrinkComparisonTests`.
- **No gate verdict can move**: a colour and a fill add no alphanumeric character and no page, and
  no mover's page count changes. The gate was not re-run.
