# Calc (Spreadsheet) Architecture Reference

Scope: LibreOffice Calc (`sc/`) document model, formula engine, file-format
import (XLSX/XLSB, XLS/BIFF8, ODS), and rendering/pagination — written to
guide a from-scratch C# reimplementation that must (a) extract content
faithfully and (b) produce headless, paginated visual output equivalent to
LibreOffice's own screen/print rendering.

All paths are relative to the repo root (`/home/user/libreoffice-core`)
unless stated otherwise. Citations are `path:line`.

---

## Table of contents

- A. Document model (ScDocument / ScTable / ScColumn / cell storage / attributes)
- B. Formula engine (compiler, interpreter, dependency tracking, function catalogue)
- C. XLSX/OOXML import
- D. XLS (BIFF8/BIFF5/BIFF2) import
- E. ODS import
- F. Rendering / printing / pagination
- G. Top ~20 features that dominate visual fidelity
- Appendix: C# reimplementation notes and phased plan

---

## A. Calc document model

### A.1 Big picture

```
ScDocument (one per spreadsheet file)
 ├─ ScDocumentPool (SfxItemPool)         -- shared attribute items (fonts, borders, formats…)
 ├─ SvNumberFormatter                    -- number format codes, per-doc
 ├─ ScRangeName (global defined names)
 ├─ std::vector<std::unique_ptr<ScTable>>  maTabs   -- one per sheet
 │    ScTable (one per sheet)
 │     ├─ ScColumnData[]  (attributes only, for unallocated columns)
 │     ├─ ScColumn maCols[MAXCOLCOUNT]   -- allocated per-column storage
 │     │    ScColumn : protected ScColumnData
 │     │     ├─ ScAttrArray            -- run-length attribute runs (SfxItemSet patterns)
 │     │     ├─ sc::CellStoreType maCells         (mdds multi_type_vector)
 │     │     ├─ sc::CellTextAttrStoreType maCellTextAttrs (script type / text width cache)
 │     │     ├─ sc::CellNoteStoreType maCellNotes  (comments)
 │     │     ├─ sc::BroadcasterStoreType maBroadcasters (dependency listeners)
 │     │     └─ sc::SparklineStoreType maSparklines
 │     ├─ mpRowHeights : ScFlatUInt16RowSegments   -- RLE row heights (twips)
 │     ├─ mpHiddenRows / mpHiddenCols, mpFilteredRows, mpColFlags
 │     ├─ ScConditionalFormatList
 │     ├─ ScDBCollection (autofilter/database ranges)
 │     ├─ pOutlineTable, print ranges, page style name
 │     └─ ScDrawLayer page anchor (drawing objects live in ScDrawLayer, not ScTable)
 ├─ ScDrawLayer : SdrModel                -- one drawing layer for the whole doc (per-sheet SdrPage)
 ├─ ScDPCollection                        -- pivot tables (ScDPObject list)
 └─ ScStyleSheetPool                      -- cell styles / page styles
```

Key files:
- `sc/inc/document.hxx` (3084 lines) — the `ScDocument` god-object: every
  cross-cutting operation (recalc, broadcast, copy/paste, outline, print
  areas, defined names, DP, drawing, external refs…) is a method on it.
- `sc/inc/table.hxx` (1529 lines) — `ScTable`, one per sheet.
- `sc/inc/column.hxx` (1069 lines) + `column.cxx/column2.cxx/column3.cxx/column4.cxx`
  (~3300-3800 lines each) — `ScColumn`, the actual cell storage and 90% of
  per-cell logic (row height computation, script-type detection, autofilter
  matching, etc).

### A.2 Cell storage: mdds `multi_type_vector`

Calc does **not** store one object per cell. Each column (`ScColumn`) holds
several parallel **mdds `multi_type_vector`** ("mtv") containers — a
segmented, typed run-length column store, conceptually like Arrow
column chunks: a sequence of typed blocks (`double[]`, `SharedString[]`,
`ScFormulaCell*[]`, `empty`-run, …), so runs of same-typed values
(e.g. 10,000 doubles) are one block, not 10,000 boxed cells.

Definitions: `sc/inc/mtvelements.hxx:44-153`.

```cpp
// sc/inc/mtvelements.hxx
const mdds::mtv::element_t element_type_broadcaster  = element_type_user_start;
const mdds::mtv::element_t element_type_celltextattr = element_type_user_start+1;
const mdds::mtv::element_t element_type_string       = element_type_user_start+2;
const mdds::mtv::element_t element_type_edittext     = element_type_user_start+3;
const mdds::mtv::element_t element_type_formula      = element_type_user_start+4;
const mdds::mtv::element_t element_type_cellnote     = element_type_user_start+5;
const mdds::mtv::element_t element_type_sparkline    = element_type_user_start+6;
// standard: element_type_double (numeric), element_type_empty, element_type_uint16

typedef mdds::mtv::soa::multi_type_vector<CellStoreTraits> CellStoreType;   // maCells
typedef mdds::mtv::soa::multi_type_vector<CellTextAttrTraits> CellTextAttrStoreType;
typedef mdds::mtv::soa::multi_type_vector<CellNoteStoreType> CellNoteStoreType;
typedef mdds::mtv::soa::multi_type_vector<BroadcasterTraits> BroadcasterStoreType;
typedef mdds::mtv::soa::multi_type_vector<SparklineTraits> SparklineStoreType;
```

`ScColumn` (`sc/inc/column.hxx:178-197`) holds these five parallel stores,
each indexed by row (0..MAXROW), **independently segmented** — i.e. a block
boundary in `maCells` need not align with one in `maCellTextAttrs`. Cell
*value* type is therefore literally the mtv block's element type:
`CELLTYPE_VALUE` (double), `CELLTYPE_STRING` (`svl::SharedString` — interned
string), `CELLTYPE_EDIT` (`EditTextObject*` — rich text with per-run
formatting/fields), `CELLTYPE_FORMULA` (`ScFormulaCell*`), or empty (no
element = `CELLTYPE_NONE`).

Iteration/position handling uses `sc::ColumnBlockPosition` (cached iterators
into all 4 stores at once, `sc/inc/mtvelements.hxx:158-177`) to avoid O(log n)
descent on every single-cell access when doing bulk operations — this is a
recurring perf pattern throughout `column*.cxx`.

**Note on formulas**: `element_type_formula` blocks store `ScFormulaCell*`
directly (not the value); the formula's cached *result* lives inside
`ScFormulaCell::aResult` (a `ScFormulaResult`, see A.5), not in the cell
store. So getting "the value of a formula cell" always means: get the
`ScFormulaCell*` from the block, then ask it for its cached result (which
may trigger recalculation if dirty).

**Attributes are separate from cell storage.** Formatting (font, borders,
number format, alignment…) is *not* in the mdds column at all — it's in a
completely separate structure, `ScAttrArray` (per column), see A.3. This
means "what's the font of B5" and "what's the value of B5" are two unrelated
lookups into two unrelated data structures, each independently run-length
encoded. A C# port must model this as two parallel sparse/RLE stores.

### A.3 Cell value wrapper types

- `ScCellValue` (`sc/inc/cellvalue.hxx:32-100`) — an **owning** variant
  (`std::variant<monostate, double, SharedString, EditTextObject*, ScFormulaCell*>`),
  used to copy a cell value out of the document (clipboard, undo, etc).
- `ScRefCellValue` (`sc/inc/cellvalue.hxx:108-192`) — a **non-owning**
  reference/view into a cell in-place in the document's mtv store (a tagged
  union of `double`/`const SharedString*`/`const EditTextObject*`/`ScFormulaCell*`
  + `CellType meType`); this is what most read paths use to avoid copies.

`CellType` enum (`sc/inc/global.hxx`) — `CELLTYPE_NONE`, `CELLTYPE_VALUE`,
`CELLTYPE_STRING`, `CELLTYPE_FORMULA`, `CELLTYPE_EDIT`.

### A.4 Addressing

`sc/inc/address.hxx` — `ScAddress` (col,row,tab as `SCCOL`/`SCROW`/`SCTAB`,
packed into a `sal_Int64` for hashing), `ScRange` (two addresses), `ScRangeList`.
Sheet limits (`address.hxx:60-75`):
```
MAXCOLCOUNT = 16*1024   (columns A..XFD, Excel-2007-compatible)
MAXROWCOUNT = 1024*1024 (1,048,576 rows)
// "Jumbo sheets" opt-in: MAXROWCOUNT_JUMBO = 16*1024*1024
```
`ScSheetLimits` (`sc/inc/sheetlimits.hxx`) parameterizes this per-document
(normal vs "jumbo" row count), so limits aren't 100% compile-time constants
in newer code — check `rDoc.MaxRow()`/`MaxCol()` rather than hardcoding.

Address text parsing/formatting supports A1, R1C1, and ODF `Sheet1.A1`
conventions — `ScAddress::Format`, `formula::FormulaGrammar::AddressConvention`.

### A.5 Formula cells

`sc/inc/formulacell.hxx` — `ScFormulaCell` (`SvtListener` subclass — it *is*
its own dependency-listener object; see B.3). Key members
(`formulacell.hxx:121-160`):

- `pCode` — `ScTokenArray*`, the compiled RPN token array. If the cell
  belongs to a **formula group** (see below), `pCode` is a *non-owning*
  alias into the group's shared `ScTokenArray`.
- `aResult` — `ScFormulaResult` (cached last-computed value: double/string/error/matrix).
- Dirty-flag bits: `bDirty` (needs recalc), `bTableOpDirty`, `bChanged`,
  `bRunning` (reentrancy guard, detects circular refs), `bCompile`,
  `bSubTotal`, `bIsIterCell` (in a circular-ref iteration group), `mbFreeFlying`
  (used outside a normal sheet grid — e.g. in a conditional-format formula).
- `cMatrixFlag : ScMatrixMode` — `NONE` / `Formula` (top-left of an array
  formula) / `Reference` (a follower cell referencing the master via
  `ocMatRef`).
- `mbDynamicArrayMaster`, `mbAutoDynamicArrayEligible` — Excel-365-style
  dynamic array spilling support.

**Formula groups** (`ScFormulaCellGroup`, `formulacell.hxx:71-113`): when a
formula is filled down a column with only relative-row changes, LO detects
this ("shared formula" on import for XLS/XLSX, or created on paste-fill) and
groups the cells so they **share one token array** and get interpreted
together — this is the basis for:
  - vectorized/SIMD evaluation (`arraysumfunctor.hxx`, `arraysumSSE2.cxx`),
  - optional **OpenCL** GPU evaluation (`sc/source/core/opencl/`),
  - optional **multi-threaded** group interpretation
    (`InterpretFormulaGroupThreading`, `formulacell.cxx:174-180`, `5602+`).
  A group has `mnWeight` (a computed cost heuristic) and only groups above
  `mnOpenCLMinimumFormulaGroupSize` are offloaded (`formulacell.cxx:5221`).
  **For a C# port doing extraction/rendering (not iterative what-if
  recalculation performance), formula groups can be safely treated as "N
  cells that happen to share a compiled formula" — group membership doesn't
  change semantics, just performance.**

### A.6 Attributes: `ScPatternAttr` / `ScAttrArray` / `SfxItemSet` / ATTR_* pool

Calc reuses the **SfxItemPool/SfxItemSet** attribute mechanism shared with
Writer/Impress (`svl/`, `svx/`). Each *distinct* formatting combination is a
pooled, refcounted `ScPatternAttr` (`sc/inc/patattr.hxx`) — effectively an
`SfxItemSet` keyed by "which-id" (`ATTR_FONT`, `ATTR_BORDER`, …) whose values
are individual `SfxPoolItem` subclasses (many literally shared with Writer,
e.g. `SvxFontItem`, `SvxBoxItem` for borders, `SvxBrushItem` for background).

Which-ids and their concrete item classes — the authoritative list is
`sc/inc/scitems.hxx:100-203`:

| ATTR_* range | Meaning | Item class |
|---|---|---|
| 100–121 | Font family/height/weight/posture/underline/overline/strikeout/contour/shadow/color/lang, ×3 for Western/CJK/CTL | `SvxFontItem`, `SvxFontHeightItem`, `SvxWeightItem`, `SvxPostureItem`, `SvxUnderlineItem`, `SvxColorItem`, `SvxLanguageItem`… |
| 122 | `ATTR_USERDEF` — user XML attr container (not saved binary) | `SvXMLAttrContainerItem` |
| 123–128 | Wordline, char relief, hyphenate, script-space, hanging punctuation, forbidden rules |
| 129–130 | `ATTR_HOR_JUSTIFY` / `ATTR_HOR_JUSTIFY_METHOD` — horizontal alignment | `SvxHorJustifyItem` (Standard/Left/Center/Right/Block/Repeat) |
| 131 | `ATTR_INDENT` | `ScIndentItem` |
| 132–133 | `ATTR_VER_JUSTIFY` / method — vertical alignment | `SvxVerJustifyItem` |
| 134 | `ATTR_STACKED` — stacked/vertical text | `ScVerticalStackCell` |
| 135–136 | `ATTR_ROTATE_VALUE` (angle, 1/100°) / `ATTR_ROTATE_MODE` |
| 137–138 | `ATTR_VERTICAL_ASIAN`, `ATTR_WRITINGDIR` (CTL/RTL) |
| 139 | `ATTR_LINEBREAK` — wrap text | `ScLineBreakCell` |
| 140 | `ATTR_SHRINKTOFIT` | `ScShrinkToFitCell` |
| 141–142 | `ATTR_BORDER_TLBR`/`BLTR` — diagonal cell borders |
| 143 | `ATTR_MARGIN` — cell padding | `SvxMarginItem` |
| 144–145 | `ATTR_MERGE` (span col/row count), `ATTR_MERGE_FLAG` (`ScMF` — is this cell hidden-by-merge, autofilter arrow, pivot buttons…) | `ScMergeAttr`, `ScMergeFlagAttr` |
| 146–147 | `ATTR_VALUE_FORMAT` (number format **key**, `sal_uInt32` into `SvNumberFormatter`), `ATTR_LANGUAGE_FORMAT` |
| 148 | `ATTR_BACKGROUND` | `SvxBrushItem` (color/pattern/bitmap) |
| 149 | `ATTR_PROTECTION` | `ScProtectionAttr` (locked/hidden formula) |
| 150–151 | `ATTR_BORDER` (4 side lines), `ATTR_BORDER_INNER` (dialog-only "inner" template, not per-cell) | `SvxBoxItem`, `SvxBoxInfoItem` |
| 152 | `ATTR_SHADOW` | `SvxShadowItem` |
| 153 | `ATTR_VALIDDATA` — data-validation rule id | `SfxUInt32Item` |
| 154 | `ATTR_CONDITIONAL` — conditional-format id list | `ScCondFormatItem` |
| 155 | `ATTR_HYPERLINK` | `SfxStringItem` |
| **100–155** | = **`ATTR_PATTERN_START..ATTR_PATTERN_END`**: the "cell pattern" range that composes a `ScPatternAttr` |
| 156–188 | Page-style attributes (margins, paper size/bin, header/footer content+on/off, scale-to-page, print grid/headers/formulas/notes, `ATTR_HIDDEN`) — these apply to **page styles**, not cells |

(`sc/inc/scitems.hxx:100-203`; item classes declared in `sc/inc/attrib.hxx`.)

`ScPatternAttr` wraps an `SfxItemSet` over the `ATTR_PATTERN_START..END`
range plus an optional named **cell style** (`ScStyleSheet`) it falls back
to for unset items — i.e. **attribute resolution is: explicit direct
formatting on the pattern → named style's pattern → document "Default" style**.
This parent-chain lookup (`SfxItemSet::GetItemState`/`Put` semantics) is the
single most important thing to replicate for visual fidelity: a cell that
looks bold might have no direct `ATTR_FONT_WEIGHT` at all and only inherit
it from its style.

**Storage**: `ScAttrArray` (`sc/inc/attarray.hxx:79-173`) is a **simple
run-length-encoded vector**, `std::vector<ScAttrEntry>` where each
`ScAttrEntry{ SCROW nEndRow; CellAttributeHolder aPattern; }` means "rows
[previous.nEndRow+1 .. nEndRow] all share pattern `aPattern`"
(`sc/inc/attarray.hxx:79-110`). This is a *second*, independent RLE
structure from the mdds cell-value store — column formatting granularity is
usually much coarser than value granularity (e.g. "whole column bold" is one
`ScAttrEntry` regardless of how many distinct values are in the 1M rows).
`ScPatternAttr` instances themselves are deduplicated/pooled by
`CellAttributeHelper` (`sc/inc/patattr.hxx:58-97`) keyed by content — so
`GetPattern(row)==GetPattern(row2)` pointer-equality is a valid and heavily
used fast-path equality check.

**C# design implication**: model formatting as its own column-independent
sparse RLE map from row → "resolved style token" (an interned/deduplicated
value), completely decoupled from cell content, and give it a 3-level
resolution chain (direct → named style → default). Do **not** try to store
formatting per-cell in whatever structure you use for cell values.

### A.7 Number formats

`svl/source/numbers/zforlist.cxx` — `SvNumberFormatter`: format codes are
strings like `#,##0.00;[RED]-#,##0.00` or `YYYY-MM-DD`, each interned to a
`sal_uInt32` key (which is what `ATTR_VALUE_FORMAT` stores). Formatting for
display goes through `SvNumberFormatter::GetOutputString(...)`
(`svl/source/numbers/zforlist.cxx:2093`, dispatching into per-locale
`SvNFEngine`/`SvNumberformat::GetOutputString`). A format entry
(`SvNumberformat`) parses into up to 4 semicolon-separated subformats
(positive;negative;zero;text) with its own color/condition, matching Excel's
number-format-code language nearly 1:1 (they share heritage) — currency
symbols, `[$-xxx]` locale tags, date/time tokens, scientific notation,
fractions, percentages, `General`.

**"General" format & column-width-dependent rendering**: the `General`
format auto-picks a representation (integer vs scientific vs fixed decimals)
*based on how many characters fit in the column* — this is one of the
trickiest things to reproduce (see G, item on `###`/General format).

### A.8 Row heights, column widths, hidden/frozen

- Column widths: `ScColumn`/`ScTable::SetColWidth`/`GetColWidth`
  (`sc/inc/table.hxx:864-891`), stored as `sal_uInt16` **twips** per column
  (flat array, one entry per column — columns are far fewer than rows so no
  RLE needed).
- Row heights: `ScFlatUInt16RowSegments* mpRowHeights` (`sc/inc/table.hxx:192`),
  an RLE segment tree (`mdds::flat_segment_tree`-based,
  `sc/inc/segmenttree.hxx:128`) mapping row ranges → height in **twips**.
  `STD_COL_WIDTH` = 64pt in twips, `STD_ROWHEIGHT_DIFF = 23` twips margin
  correction (`sc/inc/global.hxx:107-116`). `MAX_COL_WIDTH`/`MAX_ROW_HEIGHT`
  = 1 meter in twips (`global.hxx:110-113`).
- Hidden rows/cols: separate `ScFlatBoolRowSegments`/col-flags bitsets, not
  part of the height segment tree.
- Freeze/split panes: view-level state (`ScViewData`), not part of
  `ScDocument` proper — persisted per-sheet in ODS (`table:table-view-settings`? actually
  config item) and in XLSX `sheetViews/pane` (`viewsettings.cxx`).

### A.9 Conditional formatting

`sc/inc/conditio.hxx` — `ScConditionalFormat` owns a list of
`std::unique_ptr<ScFormatEntry>` (`conditio.hxx:561-566`). `ScFormatEntry`
is a base class with concrete subtypes (`conditio.hxx:225-282`,
`ScFormatEntry::Type` enum): `Condition` (`ScConditionEntry` — comparison
operator + one/two formula expressions, `ScConditionMode` enum listing Equal,
Less, Greater, Between, Duplicate, Top10, AboveAverage, BeginsWith,
ContainsText, etc, `conditio.hxx:61-89`), `ExtCondition`, `Colorscale`
(2/3-color scale), `Databar`, `Iconset`, `Date` (date-relative like "last 7
days"). A cell references its applicable format(s) by id via
`ATTR_CONDITIONAL`/`ScCondFormatItem`, and the id list is resolved through
`ScConditionalFormatList` on the sheet. **For rendering**, conditional
formats each resolve to either a full/partial `ScPatternAttr`-like override
(color scale/databar/iconset compute a color procedurally from the value's
rank within the range) that is applied **after** normal cell-pattern
resolution, i.e. conditional formatting wins over static formatting when
both apply (except borders, which don't participate in CF).

### A.10 Merges, notes, sheet-level structures

- **Merged cells**: `ATTR_MERGE` (span) on the anchor (top-left) cell +
  `ATTR_MERGE_FLAG` (`ScMF::Hor`/`Ver`) on all covered cells marking them
  "overlapped" (`sc/inc/attrib.hxx:58-118`). Rendering must look up the
  anchor cell's content/format and paint it across the merged rectangle;
  covered cells' own content is normally empty (UI prevents entering data
  there) but formatting attributes can still exist on them independently.
- **Cell notes/comments**: `ScPostIt` (`sc/inc/postit.hxx`), stored in the
  column's `maCellNotes` mtv store — carries an `EditTextObject` (rich text)
  and an optional caption `SdrCaptionObj` (drawing-layer object) for
  visibility/positioning, author, date.
- **Defined names**: `ScRangeName` (`sc/inc/rangenam.hxx:186+`) — global, or
  per-sheet (`ScTable::mpRangeName`). Each `ScRangeData` wraps a compiled
  `ScTokenArray` (so a named range is really a stored formula, resolved
  relative to a "base position").
- **Autofilter / data validation**: `ScDBCollection`/`ScDBData` (database
  ranges incl. autofilter range), `ScValidationData` (`ATTR_VALIDDATA` id →
  validation rule: list/whole-number/decimal/date/time/text-length/custom
  formula + input/error messages).
- **Drawing layer / charts / OLE**: `ScDrawLayer : SdrModel`
  (`sc/inc/drwlayer.hxx`) — one drawing model for the whole document, one
  `SdrPage` per sheet, holding `SdrObject`s: shapes, `SdrOle2Obj` (charts and
  embedded OLE objects), images, form controls, cell-note caption objects.
  Anchoring to cells uses `ScDrawObjData` (start/end cell + fractional
  offset) so objects reflow correctly when rows/cols resize — analogous to
  OOXML `twoCellAnchor`.
- **Pivot tables**: `ScDPObject` (`sc/inc/dpobject.hxx:116+`) — holds the
  pivot **cache** (`ScDPCache`, the flattened source data snapshot,
  `dpcache.cxx`), the **save data** (`ScDPSaveData` — field layout: which
  fields are row/col/page/data, subtotals, sort), and produces an output
  range via `ScDPOutput` (`dpoutput.cxx`) which is realized back into normal
  sheet cells + a `ScDPObject`-owned table-output flag range. For pure
  extraction, treat a pivot table's *rendered* cell grid the same as any
  other cells (they *are* real cells with `ScMF::DpTable` merge-flags for
  button decorations) — the pivot *definition* is secondary unless you need
  to recompute it.

---

## B. Formula engine

### B.1 Pipeline overview

```
 formula text  ──(ScCompiler / FormulaCompiler)──▶ ScTokenArray (RPN tokens)
                         │ tokenizes per-grammar (ODFF, Excel A1, Excel R1C1, English, native UI)
                         ▼
                 OpCode + operand tokens (refs, doubles, strings, matrices, names, external refs)
                         │
      ScFormulaCell::pCode  (owned, or shared via ScFormulaCellGroup)
                         │
                (on demand / dirty) 
                         ▼
                 ScInterpreter::Interpret()  -- stack machine, one call per opcode
                         │ push/pop `formula::FormulaToken` on a 512-slot stack
                         ▼
                 ScFormulaResult (double | string | error | ScMatrixRef)
```

### B.2 Compiler

- Generic core: `formula/source/core/api/FormulaCompiler.cxx` (3766 lines) —
  shared infra for all Office apps that embed the `formula` module (Calc,
  and formerly Chart). Handles: tokenizing raw text into an intermediate
  symbol list, operator precedence, function-name → `OpCode` resolution
  per-grammar, building the final `ScTokenArray`/`FormulaTokenArray`
  RPN sequence (shunting-yard-like), and the reverse (RPN → formula text,
  `CreateStringFromTokenArray`) for exporting/redisplaying.
- Calc-specific: `sc/source/core/tool/compiler.cxx` (7502 lines) —
  `ScCompiler : formula::FormulaCompiler` — Calc reference syntax (A1/R1C1/
  ODF `Sheet.A1`/`[Sheet.A1]` external-doc refs), sheet-name resolution,
  `ScRangeData`/named-range lookup, table-references (`ocTableRef` — Excel
  structured references `Table1[Column]`), DB-area refs, add-in functions,
  macro calls.
- **Grammar** enum `formula::FormulaGrammar::Grammar`
  (`include/formula/grammar.hxx:69-154`): combines a *language* (ODFF, PODF
  [ODF 1.1 legacy], ENGLISH, NATIVE, OOXML, XL_ENGLISH, API) with an
  *address convention* (CALC_A1, EXCEL_A1, EXCEL_R1C1, ODF) and *reference
  bracket style* (whether `[...]` wraps refs, ODFF-style). Concrete
  constants: `GRAM_ODFF`, `GRAM_NATIVE_XL_A1`, `GRAM_NATIVE_XL_R1C1`,
  `GRAM_ENGLISH_XL_A1`, `GRAM_OOXML`, `GRAM_PODF`, etc
  (`grammar.hxx:71-146`). **`GRAM_ODFF` is the storage/interchange default**
  (`grammar.hxx:154`); UI display uses the locale's `GRAM_NATIVE_UI`.
- **OpCode catalogue**: `include/formula/opcode.hxx` — a single big `enum
  OpCode : sal_uInt16` (**~516 named opcodes** by grep count) covering
  control tokens (`ocPush`, `ocIf`, `ocChoose`, `ocLet`, `ocLambda`,
  `ocOpen/Close/Sep`), reference/array tokens (`ocMatRef`, `ocDBArea`,
  `ocTableRef` + `ocTableRefItemAll/Headers/Data/Totals/ThisRow` for Excel
  structured refs), and then **every built-in worksheet function**
  (`ocSum`, `ocVLookup`, `ocXLookup`, `ocIfs`, …) each with its own OpCode
  value — i.e. functions are not a runtime string dispatch table, they're
  compiled to a dense integer opcode and the interpreter mostly `switch`es
  on it directly (`sc/source/core/tool/interpr1.cxx`'s giant switch, plus
  interpr2..8.cxx for groups of functions).
- **Function catalogue / name resolution per grammar**:
  `sc/source/core/data/funcdesc.cxx` — `ScFunctionMgr`/`ScFuncDesc` binds
  each `OpCode` to: a resource-string display name (localized), category,
  argument descriptions (for the function wizard/tooltips), and Excel/ODF
  name-mapping tables live in the compiler/formula-parser layers per grammar
  (e.g. `formulabase.cxx` in the oox filter — `FunctionParserBase` maps
  OOXML function names ↔ opcodes, `sc/source/filter/oox/formulabase.cxx`).
  ODFF names are typically `ORG.OPENOFFICE.*`-prefixed extension functions
  for things Excel doesn't have; standard functions map 1:1 by name modulo
  a handful of renames (e.g. `CEILING.MATH` vs `CEILING`).

### B.3 Interpreter

`sc/source/core/inc/interpre.hxx` (private header) + `interpr1..8.cxx`
(interpr1.cxx alone is **14,500 lines**) — `ScInterpreter`:
- Stack machine: `const formula::FormulaToken* pStack[MAXSTACK]`,
  **`MAXSTACK = 512`** (`sc/source/core/inc/interpre.hxx:154,260-261`,
  `ScTokenStack` also 512). Each opcode handler (`ScAverage()`, `ScVLookup()`,
  hundreds of `Sc*()` methods) pops its operands off the stack, computes,
  pushes a result token.
- `nGlobalError : FormulaError` — the "sticky" current error state within one
  formula evaluation; propagates through arithmetic per Excel/ODF error
  rules (error operand → error result) rather than throwing exceptions.
- Error codes: `include/formula/errorcodes.hxx:30+` — a flat `enum class
  FormulaError` with the familiar user-visible codes as named values e.g.
  `NoValue = 519` (#VALUE!), `NoRef = 524` (#REF!), `NoName = 525` (#NAME?),
  `DivisionByZero = 532` (#DIV/0!), `CircularReference = 522`, plus many
  interpreter-internal-only pseudo-errors (`RetryCircular`, `ElementNaN`,
  `NotNumericString`) that never surface to the user but drive iteration/
  array-formula control flow internally.
- **Matrices / array formulas**: `sc/inc/scmatrix.hxx` + `scmatrix.cxx`
  (3836 lines) — `ScMatrix` is a dense 2D value grid (double/string/bool/
  error per cell) used for `{=...}` array formulas and any function that
  operates element-wise over a range (SUMPRODUCT, array-entered formulas,
  spilled dynamic arrays). `ScJumpMatrix` (`jumpmatrix.cxx`) handles
  IF()-like branching *inside* an array context (each array element may take
  a different branch).
- **Implicit intersection**: when a plain (non-array) formula references a
  multi-cell range where a scalar is expected, Calc reduces the range to the
  single cell in the same row/column as the formula (classic Lotus/Excel
  "@"-intersection behavior) — handled in the reference-fetching helpers in
  `interpr4.cxx`/`interpr1.cxx` rather than a separate pass.
- **Iterative/circular references**: controlled by `ScDocOptions`
  (iteration on/off, max iteration count, min change threshold); a cell
  detected as part of a cycle is flagged `bIsIterCell`, gets
  `nSeenInIteration` bumped each pass, and `FormulaError::CircularReference`/
  `RetryCircular` drive convergence checking across passes
  (`ScFormulaCell::InterpretTail`, `formulacell.cxx`).

### B.4 Dependency tracking / recalculation

- Every `ScFormulaCell` is itself an `SvtListener` (`formulacell.hxx:121`) —
  cells subscribe directly to the `SvtBroadcaster`s of the cells/ranges they
  reference (from `maBroadcasters` mtv store, A.2). On edit, `ScDocument::
  Broadcast()`/`ScColumn`'s area-broadcast machinery notifies listeners,
  which call `ScFormulaCell::SetDirty()` (`sc/source/core/data/
  formulacell.cxx:2935`) → `SetDirtyVar()` (`:2969`), marking the cell (and
  transitively anything listening on it) for recalculation without
  recomputing immediately (lazy/pull recalc — value is actually recomputed
  the next time it's *read*, via `ScFormulaCell::Interpret()`/`InterpretTail`).
- **Range/area dependencies** (as opposed to single-cell) are tracked via
  `ScBroadcastAreaSlotMachine` / `ScBroadcastAreaSlot`
  (`sc/source/core/inc/bcaslot.hxx:143-161`, `bcaslot.cxx` 1286 lines) — the
  sheet is partitioned into "slots" (spatial hash buckets over rows) each
  holding the set of `ScBroadcastArea`s (rectangular ranges with listener
  lists) that intersect it, so "did anything change in A1:A1000" broadcast
  can avoid a full linear scan of a million cells.
- **Formula-group listening**: `sc::FormulaGroupAreaListener`
  (`grouparealistener.hxx`) lets a whole shared-formula group listen to a
  range as one unit instead of each of N cells separately (huge win for
  "=A1*2" filled down 100k rows watching a single input range).
- Full-document recalc entry point: `ScDocument::CalcAll()`/`CalcAfterLoad()`
  (called once after every import, since caches like SST or shared strings
  don't include formula results — LO always recomputes on load unless the
  file explicitly disables it, though most filters *do* also import the
  cached result from the file initially via `ScFormulaResult`, then mark
  dirty and recalc).

**C# implication**: a read-only extraction/rendering tool does not need the
listener/broadcast machinery at all — you can do a single topological (or
just iterative-until-stable) recalculation pass after loading, given the
compiled token arrays + resolved references, exactly mirroring "load, then
CalcAll()". Real dependency graph tracking only matters if you plan
incremental re-editing, not one-shot render/extract.

---

## C. XLSX / OOXML import

Calc's OOXML filter lives at `sc/source/filter/oox/` (import contexts) with
private headers in `sc/source/filter/inc/` (NOT `include/oox/xls/` — that
directory doesn't exist; xls-specific oox headers are module-private under
`sc/source/filter/inc/`, unusually for the oox-using filters). Shared/
generic OOXML infrastructure (opc container, shared strings base classes,
theme, drawingml, VML) is in `oox/source/` + `include/oox/`.

### C.1 Fragment/context architecture (shared with Writer/Impress OOXML)

`include/oox/core/fragmenthandler2.hxx:35-95` — `FragmentHandler2` is a SAX
fast-parser handler that, crucially, **dual-dispatches** between:
- **XML** mode: `onCreateContext`/`onStartElement`/`onCharacters`/`onEndElement`
- **Binary (BIFF12/XLSB) record** mode: `onCreateRecordContext`/
  `onStartRecord`/`onEndRecord`, fed a `SequenceInputStream` of raw record
  bytes instead of SAX events.

Concrete context classes (e.g. `SheetDataContext`,
`sc/source/filter/oox/sheetdatacontext.cxx`) implement **both** paths in
the same class — e.g. `importRow(AttributeList)` for XML `<row>` vs
`importRow(SequenceInputStream&)` for the BIFF12 `ROW` record
(`sheetdatacontext.cxx:84` vs `:412`), and cell records dispatch on
`BIFF12_ID_CELL_BOOL/BLANK/DOUBLE/ERROR/RK/RSTRING/SI/STRING` plus
`BIFF12_ID_FORMULA_*`/`MULTCELL_*` variants (`sheetdatacontext.cxx:254-281`).
**This means LibreOffice's XLSX and XLSB import share ~all of their sheet
parsing logic** — only the outer tokenizer differs (SAX XML vs. binary
record reader). A C# implementation can mirror this by having one internal
"cell events" model fed by either an XML reader or a binary-record reader.

### C.2 Module layout (`sc/source/filter/oox/`)

| File | Responsibility |
|---|---|
| `workbookfragment.cxx` / `workbookhelper.hxx` | Top-level `<workbook>`; owns shared buffers (styles, shared strings, themes) accessible to all other fragments via `WorkbookHelper` mixin |
| `worksheetfragment.cxx` / `worksheethelper.hxx` | Per-sheet `<worksheet>`: dispatches to sheetData, cols, mergeCells, sheetPr, conditional formatting, data validations, autofilter, scenarios, page setup, drawing |
| `sheetdatacontext.cxx` / `sheetdatabuffer.cxx` | `<sheetData>`/`<row>`/`<c>` — the actual cell grid, both XML and BIFF12 |
| `stylesbuffer.cxx` / `stylesfragment.cxx` | `styles.xml`: numFmts, fonts, fills, borders, `cellXfs`/`cellStyleXfs` (the `Xf` = cell format record), `dxf` (differential formats for conditional formatting/tables), cell styles |
| `sharedstringsbuffer.cxx` / `sharedstringsfragment.cxx` / `richstring*.cxx` | `sharedStrings.xml` (SST) with rich-text runs |
| `themebuffer.cxx` | `theme1.xml` — theme colors/fonts referenced by style indices |
| `condformatbuffer.cxx` / `condformatcontext.cxx` | `<conditionalFormatting>` |
| `autofilterbuffer.cxx` / `autofiltercontext.cxx` | `<autoFilter>` |
| `defnamesbuffer.cxx` | `<definedNames>` |
| `tablebuffer.cxx` / `tablecolumnsbuffer.cxx` / `tablefragment.cxx` | Excel Tables (`table1.xml`, structured refs) |
| `pivotcachebuffer.cxx`/`pivotcachefragment.cxx`/`pivottablebuffer.cxx`/`pivottablefragment.cxx` | Pivot caches + pivot table defs |
| `drawingbase.cxx` / `drawingfragment.cxx` | `<drawing>` → `oneCellAnchor`/`twoCellAnchor`/`absoluteAnchor`, delegates shape geometry to shared `oox/drawingml` |
| `commentsbuffer.cxx`/`commentsfragment.cxx`, `threadedcommentsfragment.cxx` | Legacy + threaded comments |
| `externallinkbuffer.cxx`/`externallinkfragment.cxx` | External workbook references |
| `pagesettings.cxx` | `<pageSetup>`/`<headerFooter>`/margins/scale |
| `formulaparser.cxx`/`ooxformulaparser.cxx`/`formulabase.cxx`/`formulabuffer.cxx` | OOXML formula text ↔ `ScTokenArray`, shared-formula (`t="shared"`) reconstruction |
| `numberformatsbuffer.cxx` | `numFmts` custom format codes |
| `excelfilter.cxx` | Top-level `ImportFilter` UNO service entry point |

Shared strings, styles, and theme are populated once from `WorkbookHelper`
and referenced by index everywhere else — same general shape as raw OOXML
(`sst` index in `<c t="s"><v>N</v></c>`, style index `s="N"` on `<c>` into
`cellXfs`).

### C.3 XLSB (binary OOXML)

**Fully supported**, not a stub: `.xlsb` is a registered type
(`filter/source/config/fragments/types/MS_Excel_2007_Binary.xcu:21`,
extension `xlsb`) and is handled by exactly the dual XML/BIFF12 context
classes described in C.1 — same `sc/source/filter/oox/*` code, fed BIFF12
records instead of XML elements. (There is *also* a separate, unrelated
debug-only "oox dumper" tool mentioned in `excelfilter.cxx:77-81` for
introspecting raw XLSB bytes during development — don't confuse that with
actual import support.) BIFF12 record ids referenced throughout
`worksheetfragment.cxx`/`sheetdatacontext.cxx` (e.g.
`BIFF12_ID_WORKSHEET`, `BIFF12_ID_SHEETDATA`, `BIFF12_ID_ROW`,
`BIFF12_ID_CELL_*`, `BIFF12_ID_CONDFORMATTING`, `BIFF12_ID_DATAVALIDATIONS`,
`BIFF12_ID_AUTOFILTER`, `BIFF12_ID_MERGECELLS`, `BIFF12_ID_OLEOBJECTS`,
`worksheetfragment.cxx:69-536`).

---

## D. XLS (BIFF8, and legacy BIFF5/BIFF2) import

Path: `sc/source/filter/excel/` (`.cxx`) + `sc/source/filter/inc/` (headers,
prefix `xi*` = import, `xe*` = export, `xl*` = shared/const).

### D.1 Stream / record framework

`XclImpStream` (`sc/source/filter/inc/xistream.hxx:277+`) wraps an
`SvStream` and provides record-oriented reads: `StartNextRecord()` advances
to the next `[Id:u16][Size:u16][data]` record header, and **automatically
follows `CONTINUE` records** transparently when a record's declared payload
exceeds what fits in the current record — callers just keep calling
`Read*()` and cross-record boundaries are invisible
(`xistream.hxx:230-276` doc comment). It also supports pushed/popped stream
positions and one persistent "global position" for jump-and-return reads
(needed e.g. to peek ahead at a BOF/EOF pair while continuing linear scan).

Substream structure: every BIFF stream (workbook globals, each worksheet,
chart sheets, macro sheets) is delimited by a `BOF`/`EOF` record pair;
`BOF.wType` distinguishes globals (`EXC_BOF_GLOBALS = 0x0005`) from a normal
worksheet, and `XclTools::SkipSubStream()` is used to skip substream types
not relevant to the current import stage (`read.cxx:1101-1104`). BIFF
version auto-detection: `XclImpStream::DetectBiffVersion()`.

### D.2 Encryption

`XclImpDecrypter`/`XclImpBiff5Decrypter` (`sc/source/filter/excel/
xistream.cxx:37-195`) — supports:
- **XOR obfuscation** (older "protect with password", weak, key+hash pair
  verified against BOF, `mnKey`/`mnHash`, `xistream.cxx:110-146`),
- **RC4** (`maCodec` is `MSCodec_XOr` or an RC4 codec depending on branch;
  standard Office-97 RC4 stream cipher, re-keyed roughly every 16 bytes –
  see `OnUpdate`/`Skip` logic, `xistream.cxx:189-195`),
- **RC4 CryptoAPI ("Std97")** via `::msfilter::MSCodec_Std97`
  (`xistream.cxx:160-165`, shared with the DOC/PPT binary filters in
  `msfilter/`), used for the stronger XLS encryption variant.
Encryption is applied transparently inside `XclImpStream::Read()` once a
decrypter is set (`SetDecrypter`), so record-parsing code above never
special-cases encrypted vs plain files (only the workbook-globals BOF area
and a few record types are explicitly exempted from decryption per the spec).

### D.3 Record catalogue (import dispatch)

`sc/source/filter/excel/read.cxx` is the master state-machine dispatcher
(`ImportExcel::Read()`), a big `switch(eCurrent){ switch(nRecId){...} }`
over import states (`EXC_STATE_*`: before-first-sheet globals, in-sheet,
etc). Confirmed record IDs actually handled include (non-exhaustive,
`read.cxx:238-1245`):

- **Structure**: `BOF`/`EOF` (all BIFF2..8 variants), `DIMENSIONS`
  (`EXC_ID2/3_DIMENSIONS`), `CODENAME`.
- **Cells**: `BLANK`/`ReadBlank`, `INTEGER` (BIFF2-only), `NUMBER`,
  `LABEL`, `BOOLERR` (bool or error value cell), `RK` (packed number, see
  below), `FORMULA` (`Formula25()`), `SHRFMLA` (shared-formula group def),
  `MULRK` (`0xBD`, run of RK values across contiguous columns), `MULBLANK`
  (`0xBE`, run of blanks), `RSTRING` (`0xD6`, rich string cell, BIFF5+),
  `LABELSST` (`0xFD`, string-by-SST-index, BIFF8 only), `ARRAY`
  (`Array34()`, array-formula range def), `TABLE`/`TableOp` (data-table
  what-if).
- **Row/col**: `ROW` (`0x0208`), `DEFCOLWIDTH`(`0x55`), `COLINFO` (`0x7D`),
  `STANDARDWIDTH` (`0x99`), `DEFAULTROWHEIGHT` (`0x0225`).
- **Formatting/misc sheet state**: `WSBOOL` (sheet flags), `COUNTRY`,
  `CODEPAGE`, `SHEETPROTECT`/`PASSWORD` (`0x12`/`0x13`), `MERGEDCELLS`
  (`Cellmerging()`, `0xE5`), `FEATHDR`/`FEAT` (table/list features).
- **Drawing/objects**: `MSODRAWING` (Escher blob, delegated to shared
  `oox`/escher drawing decode via `GetCurrSheetDrawing().ReadMsoDrawing`),
  `OBJ` (legacy BIFF5-style object, fallback path), `NOTE` (cell comment).
- **Print/page**: `HORPAGEBREAKS`/`VERPAGEBREAKS`, `HEADER`/`FOOTER`,
  margins (`LEFTMARGIN`/`RIGHTMARGIN`/`TOPMARGIN`/`BOTTOMMARGIN`),
  `PRINTHEADERS`, `PRINTGRIDLINES`, `HCENTER`/`VCENTER`, `SETUP`, `IMGDATA`.
- **Filtering/validation/conditional**: `AUTOFILTERINFO`, `AUTOFILTER`,
  `CONDFMT`/`CF` (conditional format container + individual rules), `DVAL`/
  `DV` (data validation container + individual rules).
- **Query/external data**: `QSI`, `WQSTRING`, `PQRY`, `WQSETT`, `WQTABLES`.
- **Pivot tables**: `SXVIEW`, `SXVD`, `SXVI`, `SXIVD`, `SXPI`, `SXDI`,
  `SXVDEX`, `SXEX`, `SXVIEWEX9`, `SXADDL` — a whole parallel BIFF pivot
  table sub-grammar (`xlpivot.hxx`/`xipivot.cxx`).
- **Hyperlinks/labels**: `HLINK`, `LABELRANGES`.
- **Window/view**: `WINDOW2`, `SCL` (zoom), `PANE`, `SELECTION`.

Cell-record → sheet write always funnels through `ScDocumentImport`
(`GetDocImport()`, `read.cxx:1279`) which is a bulk-insert-optimized façade
over `ScDocument`/`ScColumn` (append-oriented, avoids repeated attribute
array re-splits during a straight top-to-bottom import).

**RK number encoding** (`sc/source/filter/inc/xlconst.hxx:124-131`): Excel's
compact 32-bit numeric cell encoding — top 30 bits are either an IEEE double
with the low mantissa bits truncated (`EXC_RK_DBL`) or a 30-bit integer
(`EXC_RK_INT`), and bit 0 (`EXC_RK_100FLAG`) means "divide the decoded value
by 100" (so RK can represent 2 decimal places compactly, e.g. currency).
Both `RK` and `MULRK` records use this; a from-scratch parser must
replicate the exact bit-unpacking (mask `0xFFFFFFFC`) or numeric values will
be silently wrong.

### D.4 Formula token decoding

`sc/source/filter/excel/xiformula.cxx` — `XclImpFormulaCompiler::
CreateFormula()` (`xiformula.cxx:101`, delegating to
`XclImpFmlaCompImpl::CreateFormula()` at `:66`) walks the raw BIFF token
stream (Excel's own RPN encoding — `tExcel opcodes`, distinct from LO's
`OpCode` enum but 1:1 mappable) and produces a `ScTokenArray`. This is a
substantial standalone sub-module (token-class disambiguation — Excel
encodes "ref"/"value"/"array" variants of the same operator as three
different byte codes — handled via the class-selecting tables in
`xlformula.cxx`/`xlformula.hxx`).

### D.5 BIFF version coverage

Constants for **BIFF2 through BIFF8** exist and are actively branched on
throughout `read.cxx` (`EXC_BIFF2`/`EXC_BIFF5` in `xlconst.hxx:31-34`, and
per-record `EXC_ID2_*`/`EXC_ID3_*`/`EXC_ID4_*`/`EXC_ID5_*` variants for the
same logical record across Excel versions 2.1/3/4/5-95/97+) — i.e.
LibreOffice's XLS import genuinely supports the full legacy lineage down to
Excel 2.1 (BIFF2, 1987-era `.xls`), not just BIFF8/97. A from-scratch C#
importer aimed at "real-world files" can almost certainly skip BIFF2-4 and
target BIFF5+BIFF8 only (BIFF8 = Excel 97-2003, the format actually still
seen in the wild) — but be aware BIFF5-specific quirks (shorter record size
fields, different string encoding without the BIFF8 "compressed/uncompressed
unicode" flag byte) are a real, separate code path (`XclImpBiff5Decrypter`
being its own class from the BIFF8 codec is one visible seam,
`xistream.cxx:110`).

---

## E. ODS import

Path: `sc/source/filter/xml/` (Calc-specific) + shared table/style/number
infra in `xmloff/source/table/`, `xmloff/source/style/`.

### E.1 Structure

ODS is a zipped package of XML parts (`content.xml`, `styles.xml`,
`meta.xml`, `settings.xml`) parsed via the shared `xmloff` SAX-based
import-context framework (same framework used by Writer/Impress ODF
import) — a tree of `SvXMLImportContext` subclasses, one per element name,
each returning child-context factories.

Key Calc contexts (`sc/source/filter/xml/`):

| File | Element |
|---|---|
| `xmlimprt.cxx`/`.hxx` | `ScXMLImport : SvXMLImport` — the root import service, table of context factories |
| `xmlbodyi.cxx` | `office:spreadsheet` body |
| `xmltabi.cxx` | `table:table` (one sheet) |
| `xmlcoli.cxx` | `table:table-column` (+ `number-columns-repeated`) |
| `xmlrowi.cxx` | `table:table-row` (+ `number-rows-repeated`) |
| `xmlcelli.cxx` | `table:table-cell` / `table:covered-table-cell` — **the core cell importer**, `ScXMLTableRowCellContext` |
| `xmlconti.cxx` | Text/paragraph content of a cell (for rich-text / multi-paragraph cells → `EditTextObject`) |
| `xmlcondformat.cxx` | `calcext:conditional-formats` |
| `xmlcvali.cxx` | `table:content-validation` (data validation) |
| `xmldrani.cxx` | `table:database-range` |
| `xmldpimp.cxx` | `table:data-pilot-tables` (pivot) |
| `xmlannoi.cxx` | `office:annotation` (cell comments) |
| `xmlstyle.cxx`/`xmlstyli.cxx` | Cell/column/row/table `style:style` families, incl. number formats |
| `xmlsubti.cxx` | Outline/subtotal groups |
| `xmlnexpi.cxx` | Named expressions (`table:named-range`) |
| `xmlexternaltabi.cxx` | External-reference tables |

### E.2 Cell value/type and formula handling

Each `table:table-cell` carries `office:value-type` (`float`, `string`,
`boolean`, `date`, `time`, `currency`, `percentage`) plus a matching value
attribute (`office:value`, `office:date-value`, `office:string-value`, …).
Cell content: `ScXMLTableRowCellContext` reads the value-type attributes and
either directly commits a value/string cell, or — if `table:formula` is
present — parses/compiles the ODFF-namespaced formula string and calls
`SetFormulaCell()` (`sc/source/filter/xml/xmlcelli.cxx:1024`, called at
`:1170`, `:1439`, `:1449`) to install a `ScFormulaCell`. Rich text (multiple
paragraphs/spans inside the cell) accumulates via nested `<text:p>` contexts
(`xmlconti.cxx`) into an `EditTextObject` instead of a plain string.
`table:formula="of:=SUM([.A1:.A10])"` — the `of:` (or bare) prefix + `[...]`
bracketed, `.`-separated ODFF reference syntax is what `GRAM_ODFF` parses.

There's an ODF-1.2-era transitional detail worth citing:
`calcext:value-type="error"` is written by LO 4.1+ for error-valued cells
under **extended** ODF (`mbNewValueType` flag, `xmlcelli.cxx:136,1540-1568`)
— i.e. two generations of "how is an error cell's type marked" exist in the
wild and an importer needs to handle both.

### E.3 Repeated columns/rows

`table:number-columns-repeated` / `table:number-rows-repeated` are a
**compression** feature (a run of N identical columns/rows/cells is written
once with a repeat count) — `nRepeatedRows`/`nColsRepeated` in
`xmlcelli.cxx:125,1224-1373` fan a single parsed cell/row back out to N
actual rows/columns in the document (with the caveat that a *formula* cell
cannot legally be repeated as-is — `xmlcelli.cxx:1460` asserts repeated
formula cells "not possible now", i.e. real files never do this and
per-cell relative-reference adjustment would be needed if they did). This
is functionally identical in spirit to Excel's `MULRK`/`MULBLANK` compact
records (D.3) — **both formats have a "run of similar cells" compression
concept that a C# reader must expand**, just with opposite defaults (XLS
compresses only specific record types; ODS can repeat any cell/row/column).

---

## F. Rendering / output / pagination

### F.1 `ScOutputData` — screen & print cell rendering

`sc/source/ui/view/output.cxx` (2908), `output2.cxx` (5384, mostly text/
edit-engine drawing), `output3.cxx` (289) implement `ScOutputData`, the
single class used for **both** on-screen grid painting and (via
`ScPrintFunc`, F.2) print/PDF page rendering — i.e. there is exactly one
cell-rendering code path in LO, parameterized by an `OutputDevice*` and an
`ScOutputType` (screen/printer/preview). A from-scratch renderer should
adopt the same shape: one "draw cells into rectangle X" routine reused for
both interactive and paginated output.

Rendering order (inferred from method layout, `output.cxx`):
1. `DrawDocumentBackground()`/`DrawBackground()` — cell background fills
   (`output.cxx:864,1069`), including conditional-format/DataBar
   backgrounds.
2. `DrawShadow()`/`DrawExtraShadow()` — cell shadow attribute.
3. `DrawFrame()`/`DrawRotatedFrame()` (`output.cxx:1457,1596`) — cell
   borders, via `svx::frame::Array` (`include/svx/framelinkarray.hxx:62`,
   `svx::frame::Array` — a border-segment-merging engine shared with
   Writer/Calc table borders that resolves adjoining-cell border conflicts
   by priority, `ScHasPriority()` in `sc/inc/attrib.hxx:56`, so e.g. a
   thicker/darker border from a neighbor wins over a thinner one at a
   shared edge — this "line priority" resolution is essential for pixel-
   accurate borders and easy to miss in a naive per-cell border painter).
4. `DrawStrings()`/`LayoutStrings()`/`LayoutStringsImpl()`
   (`output2.cxx:1490-2296`) — plain (non-edit-engine) text, the fast path
   for simple left/right/center-aligned single-line text.
5. `DrawEdit()` (`output2.cxx:4503`) dispatching to
   `DrawEditStandard`/`DrawEditBottomTop`/`DrawEditTopBottom`/
   `DrawEditStacked`/`DrawEditAsianVertical` (`output2.cxx:3018-4238`) — the
   `EditEngine`-based path used for wrapped text, rich text, rotated text,
   vertical/stacked (CJK) text, and shrink-to-fit; `DrawRotated()`
   (`output2.cxx:4710`) for arbitrary rotation angles.
6. `DrawGrid()` (`output.cxx:344`) — the light gray gridlines (screen only,
   never printed unless "print grid" is on) and page-break indicator lines.
7. `DrawNoteMarks()`/`DrawFormulaMarks()`/`DrawSparklines()`/
   `DrawChangeTrack()`/`DrawClipMarks()` — overlay decorations (red comment
   triangle, blue formula-changed corner marker, sparklines, track-changes
   colored borders, clipboard marching-ants).

Key sub-mechanics worth reproducing precisely:
- **Overflow into neighboring empty cells**: `GetOutputArea()`
  (`output.cxx:1204`) computes the actual drawable rectangle for a cell's
  text, which — for left/right/center-aligned unwrapped text that's wider
  than the cell — **extends into adjacent cells only while they are
  visually empty** (checked via `IsAvailable`/`IsEmptyCellText`,
  `output.cxx:1130,1178`), stopping at the first non-empty neighbor. This is
  the classic "long text spills over empty cells to the right" Excel/Calc
  behavior and it's asymmetric: numbers do *not* overflow (they show `###`
  instead when too narrow, per G below) while text does.
- **Clip marks**: `SetClipMarks()`/`ShowClipMarks()` (`output2.cxx:3371,
  3400`) draw the small red/black triangle indicating text is clipped
  rather than overflowing (when overflow is blocked by a non-empty neighbor).
- **Merged cells**: `GetMergeOrigin()` (`output.cxx:953`) resolves a covered
  cell back to its merge anchor so the anchor's content paints across the
  full merged rectangle exactly once.
- **Shrink-to-fit**: `ShrinkEditEngine()` (`output2.cxx:2444`) scales font
  size down (via the EditEngine, not naive text scaling) until content fits
  the cell — an iterative measure-and-retry, not a closed-form calculation.
- **Rich text in cells**: any cell with per-character formatting, multiple
  paragraphs, wrap, rotation, or fields (e.g. from ODS multi-paragraph
  cells or Excel rich strings) is rendered through a full `EditEngine`
  instance (`sc/source/core/tool/editutil.cxx`, `ScEditUtil`,
  `ScFieldEditEngine`) rather than a single `DrawText` call — meaning
  correct line-breaking/kerning requires an actual rich-text layout engine,
  not string measurement.

### F.2 Printing & pagination

`sc/source/ui/view/printfun.cxx` — `ScPrintFunc` is the pagination engine:
computes how a print range (or the whole used area) is chopped into pages
given page size, margins, scale (or "fit to N pages"), repeated header
rows/columns, and manual/automatic page breaks, then calls into
`ScOutputData` per page.

Relevant methods (`printfun.cxx`):
- `InitParam()` (`:852`) — reads page-style attributes (`ATTR_PAGE_*` —
  size, margins, header/footer on/off + content, scale, scale-to-pages,
  center-horizontally/vertically, print grid/headers/formulas/notes,
  top-down vs left-right page ordering) into the print parameters.
- `CountPages()` (`:2489`) — the core pagination algorithm: walks
  columns/rows accumulating widths/heights against the printable page area
  (page size minus margins minus header/footer reserved height), inserting
  a page break whenever the running total would exceed it (or at a manual
  `RowBreak`/`ColBreak`), producing `nPagesX`/`nPagesY` and per-page
  column/row ranges.
- `CalcZoom()` (`:2816`) / `GetScaleData()` (`:1123`) — "fit to N pages
  wide/tall" iteratively searches for a zoom percentage that makes the
  content fit exactly N pages (binary-search-like refinement over
  `CountPages()`).
- `ResetBreaks()` (`:3032`) — manual page-break positions
  (`ScTable`-stored, user-inserted or auto) vs. computed automatic breaks.
- `PrintHF()` (`:1784`) — header/footer text (with `&[Page]`/`&[Date]`/
  `&[File]`-style placeholder codes, parsed similarly to Writer/Calc shared
  header/footer field syntax) plus `MakeEditEngine()` (`:1743`) for
  formatted header/footer rich text.
- `PrintColHdr()`/`PrintRowHdr()`/`LocateColHdr()`/`LocateRowHdr()`
  (`:1417-1548`) — "repeat rows/columns on every page" (frozen header
  band), independent of the main scrolling content area per page.
- `DoNotes()`/`PrintNotes()` (`:1930,2004`) — "print comments" mode (a
  trailing page listing all cell notes, or inline).
- `PrintPage()` (`:2070`) — renders one page: header/footer, then delegates
  the cell grid to `ScOutputData`/`DrawToDev()`, then borders (`DrawBorder`,
  `:1278`) and any OLE/drawing objects overlapping the page rectangle.

**For a C# headless paginated renderer, `CountPages()`'s algorithm (accumulate
column widths / row heights against remaining page space, honoring page-
break records and repeated header bands) is the single most important
routine to port faithfully** — get this wrong and every page after page 1
misaligns.

### F.3 Number-format-dependent rendering

`SvNumberFormatter::GetOutputString(...)` (`svl/source/numbers/
zforlist.cxx:2093`, `SvNFEngine::GetOutputString` `:2069,2105`) turns a
raw double + a format key into the exact display string + an optional
override color (format codes can specify `[RED]` etc per-subformat). The
`General`/`Standard` format's behavior is **column-width aware**: LO must
know the cell's current pixel/twip width to decide between plain integer,
fixed-decimal, or scientific notation, and to decide whether to show `###`
placeholders instead of truncating (`ScColumn`'s `GetNeededSize`/
`ScNeededSizeOptions`, `sc/inc/column.hxx:95-105`, is the "how wide would
this need to be" companion query used both for auto-fit-column-width and
for the "does it fit, or should I show ###" decision at render time).
**This is a two-way coupling a naive port easily misses: format code
depends on the value, but "does the formatted string fit" depends on
column width, which is unrelated to formatting policy in most other
software.**

---

## G. Top ~20 features that dominate visual fidelity

Ranked roughly by (frequency in real-world files) × (visual impact if wrong).

1. **Number formats + `General` width-dependent rendering / `###` overflow.**
   Gets almost every numeric cell wrong if mishandled; see F.3.
2. **Font resolution chain (direct → cell style → default) for every text
   attribute** (family, size, weight, italic, color, underline) — most
   cells have *no* direct font attributes and rely entirely on named
   styles; getting the 3-level `SfxItemSet` fallback wrong makes whole
   sheets render in the wrong typeface/size.
3. **Cell borders with neighbor-priority resolution** (`ScHasPriority`,
   `svx::frame::Array`) — naive "each cell draws its own 4 borders"
   produces doubled/misaligned/wrong-color lines wherever two cells
   disagree about a shared edge.
4. **Row height / column width units and defaults**, incl. auto-height from
   wrapped/rotated content and the `STD_ROWHEIGHT_DIFF` fudge factor —
   errors compound across a whole sheet and shift every subsequent row.
5. **Text alignment**: horizontal (left/center/right/justify/fill/repeat)
   × vertical (top/middle/bottom) × wrap × shrink-to-fit × rotation angle ×
   stacked/vertical (CJK) — six largely-orthogonal axes that combine, each
   individually common.
6. **Text overflow into empty neighbor cells vs. clipping vs. `###`** — very
   visible (long labels are ubiquitous) and format-dependent (numbers never
   overflow; strings do, until a non-empty neighbor blocks it).
7. **Cell background fill** (solid color; gradient/bitmap fills are rare in
   practice but colored fills are everywhere, esp. via conditional
   formatting).
8. **Conditional formatting** (highlight rules, color scales, data bars,
   icon sets) — extremely common in "real" business spreadsheets and
   invisible if the CF engine isn't evaluated at render time (it's not
   baked into static cell attributes).
9. **Merged cells** — content anchor + spanning rectangle + border
   suppression on interior edges.
10. **Rich text runs within a single cell** (bold word inside a sentence,
    mixed colors) — requires an actual mini rich-text layout, not
    string-level formatting.
11. **Freeze panes / repeated print rows-columns** — affects both
    interactive scroll rendering and every page after page 1 in print
    output.
12. **Page breaks / scale-to-fit / margins for paginated (PDF-like) output**
    — this *is* the deliverable for "headless paginated output"; see F.2.
13. **Gridlines on/off** (screen default on, print default off) — subtle
    but immediately "looks wrong" if inverted.
14. **Hidden rows/columns and filtered rows** (autofilter) — must be
    excluded from both layout and printed output, not just grayed out.
15. **Header/footer content with placeholder codes** (`&[Page]`, `&[Date]`,
    sheet name, etc.) for printed pages.
16. **Cell comments/notes indicator** (small red triangle) and
    hover/print-as-list behavior.
17. **Number format locale sensitivity** (date/currency/thousand-separator
    formatting differs by locale embedded in the format code, not just the
    UI locale).
18. **Diagonal borders** (`ATTR_BORDER_TLBR`/`BLTR`) — rare but distinctly
    wrong-looking (a diagonal line through a cell) if dropped.
19. **Drawing objects / embedded charts anchored to cells** (two-cell
    anchor semantics — must reflow with row/col resize) — high visual
    impact when present, though not in every file.
20. **Row/column outline (grouping) buttons and pivot table
    button/expand-collapse decorations** — lower frequency but distinctly
    recognizable when missing from a pivot-heavy workbook.

---

## Appendix: C# reimplementation notes and phased plan

### Data model recommendations

- **Two independent stores per sheet**, both sparse/RLE by row (and, for
  column-level things, by column): (1) cell *content* — value/type/formula
  text+compiled-tokens/cached-result, and (2) cell *formatting* — a
  resolved, interned "style" reference with a 3-level fallback
  (direct-run → named-style → sheet-default). Do not merge these into one
  "Cell" object per (row,col) unless memory/complexity trade-offs are
  irrelevant for your target file sizes — real spreadsheets have "a whole
  column formatted, a handful of cells with data" patterns that this
  separation captures for free (mirrors A.2/A.6 exactly).
- Model addresses as `(int Col, int Row, int Sheet)` with Excel-compatible
  limits (16384 cols × 1,048,576 rows) as the default ceiling; don't
  hard-code assumptions that break if a "jumbo"/larger-limit file needs
  supporting later.
- Represent a formula as `{ string OriginalText/Grammar, Token[] Compiled,
  CachedResult }`, and support "groups" only as an optimization detail, not
  a semantic one (B.5's note) — for extraction/rendering you can always
  treat every formula cell independently.
- Number formats: port (or bind to) an ICU-like locale-aware formatter that
  accepts Excel/ODF-style format codes directly — don't try to re-derive
  format codes into .NET `ToString` format strings, the semantics
  (conditions, colors, `General` width-awareness, AM/PM, elapsed-time
  brackets `[hh]`) don't map cleanly.

### Phased plan

**Phase 1 — Extraction only, no rendering.**
1. ODS reader (XML is easiest to get right first; validates the data model).
   Cover: cell types/value-types, `table:formula` (ODFF grammar only),
   repeated rows/cols expansion, basic cell styles → resolved formatting,
   merges, notes, defined names.
2. XLSX reader reusing the same internal cell-event model (mirrors oox's
   XML/binary dual-dispatch design, C.1) — cover SST, styles (`cellXfs`
   chain: `Xf → font/fill/border/numFmt` indices, resolved against
   `cellStyleXfs` for named styles), shared formulas, merges, conditional
   formatting, data validation, defined names, tables.
3. XLSB reader as "same XLSX cell-event model, fed by a BIFF12 binary
   record reader" instead of XML — validates the shared-model bet from #2.
4. XLS (BIFF8) reader — separate token/record decoder (D.3/D.4); reuse nothing
   from #2/#3 except the resulting in-memory document model. Support RC4
   decryption if password-protected files are in scope.
5. Formula engine: a compiler (text/tokens ↔ RPN, all 4 grammars at least
   for round-tripping what each format natively stores) + a stack-based
   interpreter covering the ~80/20 of functions actually seen in the wild
   (arithmetic, SUM/AVERAGE/COUNT family, IF/lookup family, text functions,
   date/time) with error-code semantics matching `FormulaError` values.
   Recalculate everything once after load (no incremental dependency graph
   needed for pure extraction).

**Phase 2 — Headless visual rendering.**
6. Build a single "paint one page rectangle" routine (mirrors `ScOutputData`,
   F.1) operating against an abstract 2D drawing surface (so it can target
   SkiaSharp/System.Drawing/PDF-graphics equally): backgrounds → borders
   (with neighbor-priority resolution) → text (fast path for simple
   left/right/center single-line; full rich-text/EditEngine-equivalent path
   for wrap/rotate/rich-runs/shrink-to-fit) → gridlines/overlays.
7. Column/row measurement: port `GetNeededSize`-equivalent logic (needed for
   `General` format `###` decisions and for optional auto-fit) using an
   actual text-shaping/measurement library, not naive char-count heuristics
   (CJK/RTL/ligatures will be visibly wrong otherwise).
8. Conditional formatting evaluation at render time (color scales/data
   bars/icon sets are rank-within-range computations over the visible
   range, not stored per-cell).
9. Pagination engine mirroring `ScPrintFunc::CountPages`/`CalcZoom` (F.2):
   page-break accumulation over column widths/row heights against
   margins-adjusted page size, scale-to-fit search, repeated header
   rows/columns, manual page breaks.
10. Header/footer field substitution (`&[Page]` etc.) and print-specific
    toggles (gridlines default-off, print-area restriction, hidden/filtered
    row exclusion).

**Phase 3 — Fidelity hardening**, roughly in the priority order of section G:
merges, rich text runs, freeze-pane-driven repeated bands, drawing-object
anchoring/reflow, diagonal borders, locale-sensitive number formatting,
comments, outline/pivot decorations.

### Key files to keep as a running reference while implementing

- Document model: `sc/inc/document.hxx`, `sc/inc/table.hxx`,
  `sc/inc/column.hxx`, `sc/inc/mtvelements.hxx`, `sc/inc/cellvalue.hxx`,
  `sc/inc/formulacell.hxx`, `sc/inc/patattr.hxx`, `sc/inc/attarray.hxx`,
  `sc/inc/scitems.hxx`, `sc/inc/attrib.hxx`, `sc/inc/conditio.hxx`.
- Formula engine: `include/formula/opcode.hxx`, `include/formula/grammar.hxx`,
  `include/formula/errorcodes.hxx`, `sc/source/core/tool/compiler.cxx`,
  `sc/source/core/inc/interpre.hxx`, `sc/source/core/tool/interpr1.cxx`
  (+ 2..8), `sc/inc/scmatrix.hxx`, `sc/source/core/inc/bcaslot.hxx`.
- XLSX/XLSB: `include/oox/core/fragmenthandler2.hxx`,
  `sc/source/filter/inc/workbookhelper.hxx`,
  `sc/source/filter/oox/sheetdatacontext.cxx`,
  `sc/source/filter/oox/stylesbuffer.cxx`.
- XLS: `sc/source/filter/inc/xistream.hxx`,
  `sc/source/filter/excel/xistream.cxx`, `sc/source/filter/excel/read.cxx`,
  `sc/source/filter/excel/xiformula.cxx`,
  `sc/source/filter/inc/xlconst.hxx`.
- ODS: `sc/source/filter/xml/xmlcelli.cxx`, `xmlimprt.cxx`, `xmltabi.cxx`.
- Rendering: `sc/source/ui/view/output.cxx`, `output2.cxx`,
  `sc/source/ui/view/printfun.cxx`, `sc/source/ui/inc/printfun.hxx`,
  `svl/source/numbers/zforlist.cxx`, `include/svx/framelinkarray.hxx`.
