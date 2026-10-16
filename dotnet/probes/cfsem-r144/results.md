# `cfRule type="expression"` semantics in LibreOffice Calc — probe r144

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**
(every conversion was run as `timeout -k 30 300 … -env:UserInstallation=file:///tmp/cfsem-r144-*`
and the output file was checked to exist and be non-empty before anything was read from it).

C++ tree read here is `/home/user/libreoffice-core`, version **27.2.0.0.alpha0+** — *not* the
reference binary's source. Every claim below therefore names its two legs separately:
**[src]** = the seat in this tree, **[bin]** = a measurement against 26.2.4.2.

Witness: `sheets/chartset-010/xlsx/072_Gantt_project_planner_dde00e33.xlsx`
(copied here as `072_Gantt_project_planner_dde00e33.xlsx`).

## Instruments

| file | what it does |
| --- | --- |
| `mkxlsx.py` | builds a minimal one-sheet `.xlsx` by hand (zipfile + workbook/sheet/styles XML) |
| `readfods.py` | parses a `.fods`: per-cell computed values, `calcext:condition`, `table:named-expression` |
| `readfills.py` | maps painted fills in a PDF back to cells by the address text each probe cell carries |
| `witness_read.py` | same for the witness, anchored on the period numbers in row 4 and the `Activity nn` labels in column B |
| `predict.py` / `probe4_check.py` | an independent Python implementation of the semantics below, diffed against the reference's own painting of all 1560 cells of `H5:BO30` |

Two bugs in my own `.fods` reader had to be fixed before any comparison was trustworthy, and both
would have produced confident wrong answers:
`table:table-row` elements nested inside `table:table-header-rows` (present here because
`_xlnm.Print_Titles` is `$3:$4`) are not direct children of `table:table`, and
`table:number-columns-repeated` on a *valued* cell fills every column it covers (the witness writes
`E12`/`F12` as one repeated cell). Before the second fix the predictor reported 5 mismatches that
were entirely my reader's fault.

---

## 1. The base cell and relative shifting

### 1a. The rule formula's base is the `sqref`'s **smallest range start under `(tab, col, row)`**

**[src]** `sc/source/filter/oox/condformatbuffer.cxx:548` — `CondFormatRule::appendFormula` compiles
the `<formula>` text at `mrCondFormat.getRanges().GetTopLeftCorner()` (same at `:579` for the
binary `.xlsb` path and `:933` for the shared base address).
`sc/source/core/tool/rangelst.cxx:1142` — `ScRangeList::GetTopLeftCorner` returns the `aStart` that
is smallest under `ScAddress::operator<`, and `sc/inc/address.hxx:396` defines that as
`std::make_tuple(nTab, nCol, nRow)` — i.e. **column dominates row**, and it is *not* the
componentwise minimum.

**[bin]** `probe3.xlsx` (`probe3_build.py`) puts one rule on several multi-range `sqref`s and asks
26.2.4.2 for its own view via `--convert-to fods`:

| `sqref` | componentwise min | `calcext:base-cell-address` reported by 26.2.4.2 |
| --- | --- | --- |
| `D10 B12` | B10 | **Sheet1.B12** |
| `B22 D20` | B20 | **Sheet1.B22** |
| `F30:G30 D32 B34` | B30 | **Sheet1.B34** |
| `D40 B42` | B40 | **Sheet1.B42** |

The order the ranges are listed in makes no difference (rows 1 and 2 are the same two shapes with
the listing reversed). The painted result in `out/probe3.pdf` agrees with those bases and not with
the componentwise minima: for `sqref="D10 B12"` with rule `$A10=1` and `A8=1, A10=0, A12=1`, only
**D10** is filled — i.e. row offset `10-12 = -2` applied at D10 gives A8. A B10 base would have
filled B12 instead.

**[bin, real corpus]** `Application_Compliance_Checklist_5_Apr_2021.xlsx` (sheet `Sample Compliance
Document List`) carries `sqref="G443:G444 D491 G446:G490 G377"` with formula `$G376="N/A"`.
Componentwise minimum is **D377** and the formula is written against it (row offset 0 relative to
row 377; the sibling block `A92:AA375 …` uses `$G92` against componentwise min A92). 26.2.4.2's own
fods says `calcext:base-cell-address="'Sample Compliance Document List'.D491"`. So the divergence
between Calc's base and Excel's is real, present in this corpus, and reproduced by the reference
binary. Across the whole corpus 224 multi-range `sqref`s occur and **15** of them are cases where
the two orderings disagree (`scan_corpus_sqrefs.py` reproduces the count; `flightstandards-doc-Cross-reference-table_version02.xlsx`
supplies most of them).

### 1b. A defined name's base is **A1**, not the `sqref` top-left

**[src]** `sc/source/filter/oox/defnamesbuffer.cxx:276` — `DefinedName::getScTokens` compiles the
name's formula at `ScAddress aReferenceAddr(0, 0, sheet)`, i.e. **A1** (of sheet 0 for a global
name, of its own sheet for a sheet-local one). Nothing about the consuming conditional format
enters into it.
`sc/source/core/tool/refdata.cxx:212` — `ScSingleRefData::SetAddress` stores a relative component as
`rAddr - rPos`, so a name's stored offset is `(written address) - A1`.

**[bin]** The witness's own fods gives every name
`table:base-cell-address="$'Project Planner'.$A$1"` — `Plan`, `Actual`, `ActualBeyond`,
`PercentComplete`, `PercentCompleteBeyond`, `PeriodInActual`, `PeriodInPlan`, `period_selected`
alike — while the conditional format on the same sheet reports
`calcext:base-cell-address="'Project Planner'.H5"`. The two bases are independent.

Consequence, and it is the one that matters for the Gantt: **an `.xlsx` defined name can never have
a negative relative offset**, because A1 is the minimum address. `$C1` means offset `+0` rows;
`A$4` means offset `+0` columns; `$C5` would mean offset `+4` rows.

### 1c. Resolution is per **formatted cell**, for both plain references and names

**[src]** `sc/source/core/data/documen4.cxx:1099-1105` — `ScDocument::GetCondResult` builds
`ScAddress aPos(nCol, nRow, nTab)` from the cell being formatted and passes it down.
`sc/source/core/data/conditio.cxx:1873` `ScConditionalFormat::GetCellStyle` → `IsCellValid(rCell, rPos)`
→ `sc/source/core/data/conditio.cxx:684` `ScConditionEntry::Interpret(rPos)`, which, when
`bRelRef1`, builds a throw-away `ScFormulaCell(mrDoc, rPos, *pFormula1)` **at the formatted cell**
and interprets that. `bRelRef1` comes from `lcl_HasRelRef` at
`sc/source/core/data/conditio.cxx:82`, which returns true for any relative col/row/tab component,
**recurses through `ocName` into the defined name's own token array** (`:110-120`), and also
returns true for `ocRow`, `ocColumn`, `ocSheet`, `ocCell`.
`sc/source/core/tool/compiler.cxx:5649-5664` splices the name's cloned token array in at RPN-compile
time and then calls `SetRelNameReference()` + `MoveRelWrap()` against that cell's `aPos`;
`sc/source/core/tool/refdata.cxx:193` `ScSingleRefData::toAbs` does the actual
`offset + rPos.Col()/Row()/Tab()`.

**[bin, ordinary cells]** `probe1.xlsx` → `out/probe1.fods`, names all based at A1:

| name | formula | at K5 | K6 | K7 | K8 |
| --- | --- | --- | --- | --- | --- |
| `RowRel` | `Sheet1!$C1` | 305 (=C5) | 306 (=C6) | 307 (=C7) | 308 (=C8) |
| `RowRel5` | `Sheet1!$C5` | 309 (=C9) | 310 (=C10) | 311 (=C11) | 312 (=C12) |
| `ColRel` | `Sheet1!A$4` | at column L → 12 (=L4), for every row |
| `Free` | `Sheet1!A1` | `Err:522` at M5..M8 — it resolves to the using cell itself (circular) |

**[bin, inside a conditional format]** `probe2.xlsx`, rules over `B9:E9` and `H13:H16`:

* `NameColRel=1` with `NameColRel = Sheet1!A$1` fills **B9 and D9** (B1=1, D1=1; C1=E1=0) — bit-for-bit
  the same pattern as the plain relative rule `B$1=1` over `B8:E8`. If the name had been resolved
  once at the base cell B9, all four cells would have shared one answer.
* `NameRowRel=1` with `NameRowRel = Sheet1!$A1` fills **H13 and H15** (A13=1, A15=1; A14=A16=0).

### 1d. Off-sheet rebasing

**[bin]** `probe3` case `sqref="D40 B42"`, rule `$A1=1` (base B42 → row offset −41): **D40 is not
filled** (row 40−41 = −1), B42 is. A rule reference that rebases off the sheet simply never fires.
**[bin]** `probe5b.xlsx`: a *name* `Sheet1!XFC$1` (column offset 16382) read from column D
(overflow by 2) returns **XFD1's value**, not column B's — so an overflowing reference coming from a
name saturates at the last column/row rather than wrapping modularly, even though the code path is
`ScCompiler::MoveRelWrap` (`sc/source/core/tool/compiler.cxx:5663` →
`sc/source/core/tool/refupdat.cxx:465`, whose `lcl_MoveItWrap` adds `nMask+1` to a negative index —
the negative index it sees is `toAbs`'s INVALID sentinel, which is why the result saturates).
**Could not establish:** whether that saturation is stable across versions or is an artefact of the
INVALID sentinel value. It does not arise in this corpus (see 1b: xlsx name offsets are ≥ 0).

---

## 2. Truth and arithmetic

### 2a. What a comparison evaluates to

**[src]** `sc/source/core/tool/interpr1.cxx:850` `ScInterpreter::Compare` sets
`nCurFmtType = nFuncFmtType = SvNumFormatType::LOGICAL` (`:943`) and returns
`sc::CompareFunc(aComp)` (`sc/source/core/tool/compare.cxx:54`), ±1/0; the caller pushes a double.
A boolean is a number with a logical format, not a distinct type.

**[bin]** `out/probe1.fods`: `=(1=1)` → `office:value-type="boolean" office:boolean-value="true"`,
and `=TYPE((1=1))` → **1** (number). `=(1=1)*(2=2)` → 1, `=(1=2)*(1=1)` → 0,
`=(1=1)*5` → 5, `=(1=1)+(1=1)` → **2**, `=(1=2)+(1=1)` → 1. `TYPE` of the products/sums is 1.

So: `*` over comparisons is a plain numeric AND; `+` is a plain numeric OR that **can exceed 1**
(`PercentCompleteBeyond` relies on this: `(A$4<INT(…)) + (A$4=$E1)` may be 2, and 2 is still true).

### 2b. What makes the rule fire

**[src]** `sc/source/filter/oox/condformatbuffer.cxx:840` maps `XML_expression` →
`ScConditionMode::Direct`. The seat that decides is
**`sc/source/core/data/conditio.cxx:1272-1277`**:

```cpp
bool ScConditionEntry::IsCellValid( const ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    const_cast<ScConditionEntry*>(this)->Interpret(rPos); // Evaluate formula
    if ( eOp == ScConditionMode::Direct )
        return nVal1 != 0.0;
    …
```

`nVal1` is filled by `Interpret` at `sc/source/core/data/conditio.cxx:707-719`: a numeric result
gives `nVal1 = value`; **a string result sets `bIsStr1 = true` and `nVal1 = 0.0`** (`:716-718`), so
it can never fire. The formatted cell's own content (`rCell`) is never looked at for an expression
rule — the function returns before reaching it.

**[bin]** `probe2.xlsx`, one rule per row over `B..E`, read out of `out/probe2.pdf`:

| rule | fired? |
| --- | --- |
| `3` | **yes**, all four cells — non-zero is enough, TRUE is not required |
| `0` | no |
| `-1` | **yes** — negative counts |
| `0.5` | **yes** — fractional counts |
| `"x"` | no — a string result never fires |
| `""` | no |
| `1/0` | no — an error never fires |
| `B$1=1` | yes at B and D only — a boolean TRUE fires |

### 2c. Rule order

**[src]** `sc/source/filter/oox/condformatbuffer.cxx:1280` sorts the imported formats by priority
(tdf#138601); `ScConditionalFormat::GetCellStyle` (`conditio.cxx:1873`) returns the **first** entry
whose condition is valid and stops. **[bin]** the witness's fods lists the eight rules in
`priority` order 1,3,4,5,6,7,11,12 and the painting obeys first-match (see §5).

---

## 3. The functions

| function | **[src]** seat | behaviour | **[bin]** measurement (`out/probe1.fods`) |
| --- | --- | --- | --- |
| `MEDIAN(a,b,c)` | `interpr3.cxx:3392` → `GetMedian` at `interpr3.cxx:3368` | odd count → middle element; even count → mean of the two middles; **zero numeric arguments → `Err:502`** | `MEDIAN(5;1;10)`=5, `MEDIAN(5;10;1)`=5, `MEDIAN(3;1;2)`=2, `MEDIAN(1;2)`=1.5 |
| argument collection | `interpr3.cxx:3962` `GetNumberSequenceArray` | a **direct single-cell reference is pushed only `if (aCell.hasNumeric())`** — an empty or text cell is *skipped*, shrinking the argument count; a computed scalar (`svDouble`) is always pushed | `MEDIAN([.H2];1;2)` with H2 empty → **1.5** (= median of {1,2}), `MEDIAN([.H2];0;5)` → **2.5**, `MEDIAN(1;[.G2];3)` with G2="hello" → **2** |
| `INT` | `interpr2.cxx:957` `PushDouble(rtl::math::approxFloor(GetDouble()))` | floor, not truncation | `INT(-2.5)` = **−3**, `INT(2.9)`=2, `INT(2.5)`=2 |
| `MOD` | `interpr2.cxx:2388` `fNum - approxFloor(fNum/fDenom)*fDenom`; `fDenom==0` → `#DIV/0!` | floored modulo — result takes the **divisor's** sign | `MOD(-3;2)` = **1**, `MOD(-1;2)` = **1**, `MOD(7;2)`=1, `MOD(6;2)`=0 |
| `COLUMN()` | `interpr1.cxx:4696`, `nVal = aPos.Col() + 1` | 1-based column of the **cell the formula is running in** | see below |
| `ROW()` | `interpr1.cxx:4809`, `nVal = aPos.Row() + 1` | 1-based row of the same | see below |

### `COLUMN()` / `ROW()` inside a conditional format answer the **formatted cell**, not the base cell

**[src]** `lcl_HasRelRef` (`conditio.cxx:82`) explicitly lists `ocRow`, `ocColumn`, `ocSheet`,
`ocCell` as making the formula position-dependent (`// #i34474# function result dependent on cell
position`), which forces the per-cell free-flying `ScFormulaCell` at `rPos`, whose `aPos` is what
`ScColumn()`/`ScRow()` read.

**[bin]** `probe2.xlsx`: rule `MOD(COLUMN(),2)` over `B7:E7` (base **B7**, column 2) fills **C7 and
E7** — columns 3 and 5. A base-cell answer would have been constant across the row. Rule
`MOD(ROW(),2)` over `G13:G16` (base G13) fills **G13 and G15**. In ordinary cells the same holds:
`=ROW()` in N5..N8 → 5,6,7,8; `=COLUMN()` in O5..O8 → 15,15,15,15; and wrapped in a name
(`ColNo = COLUMN()`, `RowNo = ROW()`) the answers are identical.

---

## 4. Empty and text cells

**[src]** Three distinct seats, and they disagree with each other on purpose:

1. **Arithmetic**: an empty cell converts to `0.0`; a text cell raises `#VALUE!`
   (`ScInterpreter::GetDouble`/`GetCellValue`; the visible consequence is measured below).
2. **Comparison**: `sc/source/core/tool/interpr1.cxx:873-895` classifies the operand as
   *empty* / *value* / *string* and hands it to `sc::CompareFunc`
   (`sc/source/core/tool/compare.cxx:54`), whose rules are:
   * empty == empty; **empty == 0.0**; **empty == ""**; empty < any positive number;
     empty > any negative number; empty < any non-empty string;
   * `fRes = -1; // number is less than string` — **any number is less than any text**, so
     `text > 0` is TRUE and `text = 0` is FALSE.
3. **`MEDIAN` argument collection**: `GetNumberSequenceArray` (`interpr3.cxx:3985-3990`) *skips* a
   non-numeric single reference altogether (§3).

**[bin]** `out/probe1.fods`, with `H2` empty and `G2` = `"hello"`:

| expression | result |
| --- | --- |
| `=[.H2]` | 0 |
| `=[.H2]*1`, `=[.H2]+1` | 0, 1 |
| `=[.H2]=0` | **TRUE** |
| `=[.H2]=""` | **TRUE** (the same empty cell equals both) |
| `=[.H2]>0` | FALSE |
| `=ISBLANK([.H2])`, `=COUNT([.H2])`, `=TYPE([.H2])` | TRUE, 0, 1 |
| `=[.G2]>0` | **TRUE** — text sorts above every number |
| `=[.G2]=0`, `=[.G2]<0` | FALSE, FALSE |
| `=[.G2]*1` | **`#VALUE!`** — text in arithmetic is an error, not 0 |
| `=ISERROR([.G2]*1)` | TRUE |
| `=MEDIAN([.H2];1;2)` | 1.5 — the empty reference was dropped |
| `=MEDIAN(1;[.G2];3)` | 2 — the text reference was dropped |

The practical consequence inside the Gantt: in `MEDIAN(A$4, $C1, $C1+$D1-1)` an empty `$C1`
**removes one argument** (leaving a two-element median, i.e. an average) while the *computed*
third argument `$C1+$D1-1` is still evaluated with `$C1 = 0` and is always present. Empty and
"zero" are not interchangeable here.

**[bin, through the real conditional-format path]** `probe4.xlsx` (`probe4_build.py`) is the witness
with `F12`, `D14`, `C20`, `F25` blanked, `E24` replaced by the text `n/a` and `C22` by the text
`TBD`. All six rows repaint differently from the original, e.g.

```
row 24 orig  F6D F2F FFF F2F FFF F2F FFF F2F FFF F2F FFF F2F FFF B5A B5A B5A B5A B5A D6B F2F
row 24 var   F6D F2F FFF F2F FFF F2F FFF F2F FFF F2F FFF F2F FFF DCD DCD DCD DCD DCD FFF F2F
```

(the Actual/ActualBeyond bars collapse to Plan because `$E1` is text, so `$E1+$F1-1` is an error and
the error never fires), and

```
row 22 orig  … B5A B5A B5A B5A B5A B5A D6B …
row 22 var   … D6B D6B D6B D6B D6B D6B D6B …
```

(`$C1` is the text `TBD`, so `PeriodInPlan` loses an argument and `Plan`/`Actual` change, while
`$C1>0` is still TRUE because text > 0).

---

## 5. End-to-end confirmation on the witness

`predict.py` implements exactly the rules above in Python — names based at A1, per-formatted-cell
rebasing, `sc::CompareFunc`'s empty/text ordering, `MEDIAN` dropping non-numeric references,
floored `INT`/`MOD`, first-match-wins over the eight rules, fire iff the result is a number ≠ 0 —
reads the driver values from the reference's own `.fods`, and diffs the predicted fill against the
colour the reference actually painted in every one of the **1560** cells of `H5:BO30`.

```
$ python3 predict.py
cells compared: 1560   mismatches: 0
```

and on the empty/text variant:

```
probe4: compared 1560 mismatches 0
```

The eight `dxf`s resolve (from the fods' `ConditionalStyle_*` styles) to
`PercentComplete #735773`, `PercentCompleteBeyond #e9ab51`, `Actual #b5a1b5`,
`ActualBeyond #d6bca8`, `Plan #dcd5dc`, `H$4=period_selected #f6ddb9`,
`MOD(COLUMN(),2) #f2f2f2`, `MOD(COLUMN(),2)=0 #ffffff`, and all eight appear in the painting.

---

## What I could NOT establish

* Whether the saturating behaviour of a name reference that rebases past the last column/row
  (§1d) is intentional or an artefact of `toAbs`'s INVALID sentinel — it is measured, not
  explained, and it does not occur in this corpus.
* The **producer**-side rule ("Excel writes the formula against the componentwise minimum") is
  supported here only by reading corpus files
  (`Application_Compliance_Checklist_5_Apr_2021.xlsx`, where `$G376` with componentwise minimum
  D377 gives offset 0 and Calc's D491 gives offset −115). I did not run Excel; that leg rests on
  file contents plus the internal consistency of the sibling rules in the same workbook.
* `stopIfTrue` was not probed. Calc's `GetCellStyle` returns the first matching entry
  unconditionally, so for the *style* the flag looks inert, but I did not measure it.
* Case sensitivity of text comparison is taken from
  `ScInterpreter::Compare` reading `GetDocOptions().IsIgnoreCase()`; only the default
  (case-insensitive) was exercised.
