# The sheets track's largest ink divergence is 413 conditional formats nobody read

Round 94, seated by `probes/sheet-shapefill-r92/results.md` §8 — the passing sheets set re-ranked
on summed unsigned ink. Base `725bbbfd2`, worktree `/home/user/wt-sheetink`, reference
`/opt/libreoffice26.2/program/soffice` (LibreOffice 26.2.4.2) with the five tarball font confounds
moved aside. **The reference half of every rendering figure is the bank at
`/home/user/gate-orig-r83/ref/`, reused rather than re-rendered**; only our half was run, which a
diff confined to `dotnet/src` makes sound. Load average at the start of the round was 1.13 and no
other sweep was running.

**There is no `Task`/subagent tool in this container**, so every page reading below is my own and
is contaminated by knowing what I was looking for. Each is corroborated by arithmetic that does
not depend on it — see §3.

## 1. Seat one is `cfRule type="expression"`, and it is an entire unimplemented feature

`ink-ranking.tsv`'s head, `TK-Syllabus-Comparison-Document-v2.xlsx`: **304.57 % summed unsigned
ink over 1235 pages, 142 MAJOR, and it matches the gate.** It carries no drawing at all — 26.2.4.2's
own `--convert-to fods` of it states zero `draw:frame` and zero `draw:custom-shape` — so the shape
work of the last two rounds could not have reached it.

What it does state is **413 `<conditionalFormatting>` blocks and 413 `<dxf>` entries**, and every
one of the 413 rules is `type="expression"`. `XlsxConditionalFormats`' own remarks named this
exactly and left it: *"Only `colorScale` is read. `expression`, `cellIs`, `dataBar`, `iconSet` and
the six text predicates reach 60 further documents between them and need a formula evaluator."*

**The six distinct `dxf` entries were read out of the reference rather than out of the
specification.** `soffice --convert-to fods` writes each rule out as a `ConditionalStyle_N` cell
style with a `calcext:condition` naming it — eight seconds, no rendering — and the six are:

| how many | what 26.2.4.2's own view of the file says |
|---:|---|
| 287 | `fo:color="#00b050"` with `style:text-line-through-style="none"` |
| 70 | `fo:color="#ff0000"` with a solid line-through |
| 30 | `fo:background-color="#afabab"`, theme `light2` at `lummod 7500` |
| 21 | `fo:color="#00b050"` |
| 3 | the grey background and a text property |
| 2 | `fo:color="#ff0000"` |

So the reference draws that workbook's answer columns in green, its withdrawn rows struck through
in red, and a grey block over the cells a rule greys out; we drew all of it black, unstruck and
unfilled.

**`tk-syllabus-comparison-document-v5.xlsx`, rank 8, is the same document revised, and the 6× is
this.** v2 states **413** expression rules; v5 states **3** — the revision baked the formatting into
the cells and left 728 unused `dxf` entries behind. Per page, v2's ink is `304.57/1235 = 0.2466`
and v5's `46.55/855 = 0.0544`, a ratio of **4.53**; after the fix v2 is 0.1665 per page and the
ratio is **3.06**. The brief's *"if the two revisions differ by 6× on the same measure, that
difference is itself a lead"* is right and the lead is the rule count.

## 2. What was implemented, and the three rules that decide it

`XlsxConditionalStyles` reads every `cfRule` that names a `dxfId`, evaluates it per cell, and hands
the fill to `SheetFormatting.SetConditionalBackground` — the layer a colour scale already used —
and the font to a new differential `SheetConditionalText` laid over `SheetCellFormats`.

- **Exactly one rule wins a cell, and two matching rules' properties are never merged.**
  `ScDocument::GetCondResult` (`sc/source/core/data/documen4.cxx`) walks the formats covering the
  cell and returns the **first** non-empty style name's item set; `ScConditionalFormat::GetCellStyle`
  (`conditio.cxx`) returns the first matching entry's style within a format. A reader that unioned
  a font rule and a fill rule would paint combinations the reference never draws.
- **What a `dxf` is silent about falls through to the cell's own pattern**, because a conditional
  style's item set holds only the items the rule states. Hence a differential record with nullable
  fields rather than a whole `SheetCellFormat`. The `dxf` toggles are three-valued for the same
  reason: **287 of this document's 413 rules state `<strike val="0"/>`**, which *removes* a
  strikethrough the cell states of its own, and 26.2.4.2 writes exactly that out as
  `style:text-line-through-style="none"`.
- **A `dxf`'s fill states its colour in `bgColor`, which is the opposite of a cell's.**
  `Fill::finalizeImport` (`sc/source/filter/oox/stylesbuffer.cxx`) begins `if (mbDxf)` and, when
  the fill colour is used and the pattern is absent or solid, moves `maFillColor` — the `bgColor` —
  into `maPatternColor` and forces the pattern solid. **2734 of the corpus's `dxf` fills state only
  a background**, so a reader taking `fgColor` here, as a cell's `patternFill` needs, finds nothing
  on any of them.

**The formula subset is deliberately narrow and it is where the corpus is** (`cfrule-census.txt`,
`census-cfrules.py`). Of the 601 `expression` rules in 34 documents, **461 are the single shape
`<reference> = "<text>"`** and twenty more are the same shape against a number or a second
reference; the rest are `AND`, `MOD(ROW())`, `ISERROR`, `TODAY`, defined names and `#REF!`, which
need an interpreter — see §7 for the grouping. `cellIs` (123 rules, 18 documents) is comparison against the cell's own value
and is read too. A rule this cannot evaluate paints nothing, which is the answer the reader gave
before this existed.

Two implementation details are load-bearing and both are measured rather than assumed. **The
formula is written for the top-left cell of the whole `sqref` and shifted for every other cell in
it** — which is what `calcext:base-cell-address` states in the reference's own view — so `H2="x"`
on `S1:S1048576` tests a different `H` on every row and `$H$2="x"` tests one cell for all of them.
And **a rule's `sqref` is routinely a whole column**: all 413 of this document's are
`<col>1:<col>1048576`, so the ranges are clamped to the rows and columns the sheet actually states
before anything is walked, with a total position budget behind that.

## 3. What moved on the witness, and the corroboration that is not a reading

Ours against the banked 26.2.4.2 reference, `pdf-image-diff.py` over all 1235 pages:

| | sum \|ink\|% | MAJOR pages | pages improved | pages worsened |
|---|---:|---:|---:|---:|
| base `725bbbfd2` | **304.57** | 142 | | |
| with the rules | **205.57** | **66** | 469 | 21 |

**The arithmetic that does not depend on my reading of a page** is a census of the colour-setting
operators in the three content streams — base, after, and the reference — counted over all 1235
pages with PyMuPDF (`colour-census.py`):

| | green `#00B050` | grey `#AFABAB` |
|---|---:|---:|
| base | 1157 | 146 |
| **after** | **3558** | 690 |
| **26.2.4.2** | **3549** | 436 |

The green count goes from 1157 — the cells that state a green font outright — to **3558 against the
reference's 3549**, over 1235 pages, with nothing fitted. *The grey column is an operator count and
not an area*: the reference coalesces adjacent grey cells into fewer rectangles, which is the
`901 grey fills against one` artefact `sheet-shapefill-r92` recorded, so read it as "the fill is now
emitted" and read the ink table for how much of it lands in the right place.

`tk-page1213-before.png` and `tk-page1213-after.png` are before/after against the reference at
120 dpi on page 1213, and `tk-page52-after.png` is §4's residual.

## 4. What is left on this document, and it is a different seat

**The residual 205.57 is concentrated in the first hundred pages** — 93.5 of it on pages 1-100,
65.8 on 101-500, 12.7 on 501-900, 33.6 on 901-1235 — and it is not colour. On page 52 the two
renderings hold the same rows in the same order with **ours two rows lower than the reference's**,
so a page earlier in that sheet took two fewer rows and the offset persists until the sheet ends.
The document's total page count is 1235 on both sides, so it is a within-sheet drift that recovers,
not a pagination failure. That is a row-height question and is left with its seat.

## 5. Reach

**55 of the 947 corpus documents state a `cfRule` naming a `dxf`**, 1784 rules between them, and
all 55 are `.xlsx` — the legacy `.xls` `CONDFMT`/`CF` records are a separate reader and are
untouched. The three largest are `Application_Compliance_Checklist_5_Apr_2021.xlsx` (702),
`TK-Syllabus-Comparison-Document-v2.xlsx` (413) and
`flightstandards-doc-Cross-reference-table_version02.xlsx` (358).

## 6. Confinement

Rendered **our half of the whole 947-document corpus twice**, at the round's base and with the
change, under `SOURCE_DATE_EPOCH`, one output directory per *document* (`sweep-ours.py`,
`sweep-base-hashes.tsv`, `sweep-after-hashes.tsv`). Both legs completed 947 of 947 with no failure
on either side.

- **21 renderings differ and 926 are byte-identical.** All 21 are `.xlsx` and all 21 are on the
  sheets track; the words and slides tracks do not change by a byte, and neither does any `.xls`,
  `.xlsm` or `.doc`/`.ppt`.
- **All 21 state a `cfRule` naming a `dxf`**, and 31 of the 52 that do are unchanged — a document
  whose rules are all `containsText`, all unevaluable, or all false paints exactly what it painted.

*Each leg used a frozen copy of the binary outside the tree rather than the tree's own
`XRay.Office.Cli`, so the build the round's tests needed could not swap it mid-sweep; `dotnet/CLAUDE.md`'s
"a sweep and a rebuild must never overlap" is about the tree's binary and that is not what was
measured.* One refinement landed after the sweep — a rule's range is clamped to a bounded `<col>`
run as well as to the cells the sheet writes — and it is **inert on the corpus**: all 55 documents
stating a `dxf`-naming rule were re-rendered with the committed binary and 51 of 51 that the sweep
had banked came back byte-identical.

### What the 21 do to the ink

Scored against the r83 reference bank with `score-ink.py`, which drives `pdf-image-diff.py`
(`ink-movers.tsv`):

| | |
|---|---|
| improve on summed \|ink\|% | **12** |
| worsen | **3** |
| level | 6 |
| sum over the 21 | **354.87 → 254.85** |
| MAJOR pages over the 21 | **165 → 86** |

The largest improvements after the witness are `Data-Architecture-Tool-Fit-Assessment-Template`
5.19 → 1.14, `078_Modern_inventory_list` 2.24 → **0.02** and `077_Inventory_list_with_highlighting`
1.86 → **0.02**. *`071_Four-week_project_timeline` is a mover the instrument cannot score: we draw
one page and the reference two, so `pdf-image-diff.py` refuses and its row is a zero rather than a
measurement.*

### The one worsening that is worth more than the fix

**`Computer and Software Services_50 State Comparison.xlsx` goes 20.15 → 27.95** and the cause is
not the rule engine. Its two `cellIs equal 0` rules name a `dxf` whose fill is
`<fgColor indexed="11"/><bgColor indexed="13"/>`, and the workbook overrides the palette with
fourteen entries written **`ffRRGGBB`** — index 13 is `ff7030a0`, a purple. We paint that purple on
every cell holding zero. **26.2.4.2 paints nothing**, and its own view of the file says why:
`--convert-to fods` gives all three of that workbook's `ConditionalStyle_N` a
`fo:background-color="#ffffff"`. The mechanism is in the palette reader rather than in the rule —
`ColorPalette::importPaletteColor` builds `::Color(ColorTransparency, decodeIntegerHex(rgb))` and
`decodeIntegerHex_impl` (`oox/source/helper/attributelist.cxx`:72-79) is
`o3tl::toUInt32(value, 16)` cast to signed, so an eight-digit entry's **top byte becomes the
transparency** instead of being dropped.

**Modelling that was tried and is refuted, which is the useful half.** Making a palette entry with a
non-zero top byte answer *no colour* — the faithful reading of `::Color(ColorTransparency, …)` —
takes the same document to **68.17**, because its *stated* cell fills name the same indices and the
reference does draw those. So the transparency reaches the conditional style and does **not** reach
an ordinary fill, and where the two part company was not established. The change was reverted and
the binary re-verified byte-identical to the swept one. **Reach if anyone takes it: exactly one
corpus document** — 35 of the 55 workbooks stating an `indexedColors` table carry a non-zero top
byte somewhere and this is the only one whose cells or rules name such an entry by index.

The other two worsenings are small and are left described rather than diagnosed:
`sistem-rekod-markah-srm-_-rekod-master` 4.01 → 4.86 (five `cellIs lessThan 40` rules painting red
text) and `031_Business_expense_budget` 0.29 → 0.30.

### The gate

No gate verdict can move on this change and it was not re-run: a colour, a strikethrough and a cell
fill add no alphanumeric character and no page, which is the same argument `w:pgBorders` and the
Escher-ink round made. The one column that could see it is `pages`, and no mover's page count
changes — `071_Four-week_project_timeline` is one page against the reference's two before and after.

## 7. What was refuted, and what is left with its seat

**Refuted — that rank 2 and rank 3 are the same defect.** `alle einzeln.xlsx` (225.44 % over 186
pages, rank 2) states **no conditional formatting at all**; what it holds that nothing else in the
top ten does is a `pivotTable` part over `A4:I1013`. `Background_Declaration_Template.xls` (136.07 %
over 25 pages, rank 3) is BIFF, whose conditional formatting is `CONDFMT`/`CF` records and a
different reader. Neither moves on this round and both keep their seat.

**Refuted — `XlsxConditionalFormats`' own estimate of what the rest of the rule types need.** Its
remarks say the six non-`colorScale` families *"need a formula evaluator, a comparison, or a bar and
icon geometry"*. The comparison half is right; the formula half is not, for the shape the corpus
actually holds — 461 of the 601 `expression` rules are one comparison between a relative reference
and a string literal, and the relative shift is arithmetic on the `sqref`'s own top-left corner.

**Left: the `expression` rules this cannot evaluate**, and they are one small family each. Grouped
over the 578 rules the census's top thirty shapes account for: **481 are a simple comparison** and
are read; the rest are `AND`/`OR`/`NOT` of two or three comparisons (35), `MOD(ROW(),N)` banding
(21), a formula holding `#REF!` (17), an `IS*` predicate (12), a date function such as `TODAY()` or
`EOMONTH` (7) and five others, with a 23-rule tail below the thirtieth shape. The `AND` and `MOD`
forms would need only a boolean layer over what is here; `#REF!` cannot be answered at all.

**Left: `containsText` and its five siblings — 1040 rules in 8 documents**, of which 933 are
`containsText` in six. They are a predicate over the cell's own text and need no reference
resolution at all, so they are the cheapest thing left in this area; they were not taken because
none of the six documents is in the ink ranking's top forty and this round had one seat.

**Left, and it is a real difference from Calc: a matching rule whose `dxf` states only something
this reader does not model — a `numFmt`, a border, an alignment — is skipped here and would
*block* lower-priority rules at the reference,** because `GetCondResult` stops at the first style
it finds whether or not that style paints anything this tree draws. It can only matter where two
rules overlap on one cell, which no corpus document in the reach does.

**Left, and still seat two: a BIFF text box's `TXO` formatting runs.** 155 boxes in 17 `.xls`, 62 of
them mixed-format, exactly as `sheet-shapefill-r92` §7 left it. Nothing in this round touches the
BIFF path.

**Left: the row drift of §4**, which is now the largest single thing on the sheets track's worst
document and is a row-height question rather than a formatting one.
