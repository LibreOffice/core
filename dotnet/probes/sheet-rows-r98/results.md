# Round 98 — two rows that are the wrong height, and they are two causes

    ours   = XRay.Office.Cli @ ac7745deb (base) and @ ac7745deb + this round's diff (head)
    ref    = /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2, plus the banked
             reference PDFs at /home/user/gate-odf-r80/ref (the 307 `.ods`) and
             /home/user/gate-orig-r83/ref (the 947 originals)
    corpus = /home/user/corpus-odf/sheets — 307 `.ods`, 26.2.4.2's own conversion of the corpus
    rule   = batch-check.sh of 2026-09-05 (column 9, `glyphs`, within max(2 %, 15)), transcribed
             into `sweep-ours.py` — copied unchanged from `probes/ods-residue-r95`
    fonts  = the tarball confounds as `probes/ods-track-r88` left them; none moved this round
    date   = 2026-09-11

## 0. The headline

| seat | what it is |
|---|---|
| **O6** `sistem-rekod-markah-srm` | **closed, fixed.** Two contradicting measurements, and both were right: an ODF conditional style's font search runs through its *parents* to `Default`, but only where a condition actually **holds**. 22 pages of 26 → **26 of 26**, with page 1's fonts and colours matching the reference span for span. |
| **O16** `TK-Syllabus` | **not closed, and it is a different cause.** Measured, not argued: the workbook's 413 conditional styles resolve to Calibri 11 pt and so do its cells, so O6's mechanism cannot move a twip of it. What remains is the *line count* of wrapped rows. |
| **O17** `alle einzeln` | not reached. |

**They are two causes and the measurement that separates them is in §3.**

## 1. O6 — the two measurements did not contradict, and the discriminator is whether the condition fires

`probes/ods-residue-r95` §4 left this seat holding two results that could not both be rules:

- Six one-attribute variants of `sistem-rekod-markah-srm-_-rekod-master.ods` said the forty
  student rows' 298.2 twips follow the **document default's 11 pt** and not the cells' Arial 9 —
  rewriting every `9pt` to `6pt` leaves 298.2, rewriting every `11pt` to `20pt` gives 522.1.
- A clean four-cell probe said the opposite: an 11 pt conditional cell under a 20 pt `Default`
  measures **298**, one line of its *own* font.

**Both are correct, and the probe was measuring something other than what it claimed.** Its
conditional cell's condition never fired.

### 1.1 The parent chain, established on the document

`variant.sh` converts a flat ODF back through 26.2.4.2 and prints `style:row-height` for every row
of sheet `6A` — eight seconds, no rendering. Five one-attribute variants of the reference's own
`fods` of the document, rows 3–42 (`C4:T43` in one-based terms, exactly the conditional range):

| variant | rows 3–42 |
|---|---|
| `p0` as it stands | 298.2 |
| `p1` `style:parent-style-name="Default"` deleted from `ConditionalStyle_2` | 298.2 — and the round trip writes the parent back, because the importer re-parents a parentless cell style to `Default` |
| `p3` re-parented to a new **empty** style that itself has no parent | 298.2 — same reason, the chain still ends at `Default` |
| `p5` that intermediate style given `fo:font-size="7pt"` | **276.0** — the 7 pt line is under the floor |
| `p2` `fo:font-size="9pt"` put on `ConditionalStyle_2` itself | **276.0** |
| `p4` `Default` moved to 20 pt, nothing else touched | **522.1** on rows 3–42 against 489.3 elsewhere |

So the size comes from the applied conditional style's item set **searched with parent
inheritance**, and it wins over the cell's own. That is `lcl_populateresult`
(`sc/source/core/data/patattr.cxx`:608-637 in the C++ checkout), which asks
`pCondSet->GetItemIfSet(nWhich)` — and `GetItemIfSet`'s `bSrchInParent` defaults to **true**
(`include/svl/itemset.hxx`:201). `ScPatternAttr::fillFontOnly` (`:640-676`) takes the face, the
size and the weight through it, and `ScColumn::GetNeededSize` calls that with
`pCondSet = rDocument.GetCondResult(...)` before measuring anything (`column2.cxx`:283-292).

*This tree is not the reference binary's source and could not be checked against 26.2.4.2. The five
variants above are the measurement; the citation is a different version's explanation of it.*

### 1.2 The condition has to hold, and that is what the clean probe was missing

`mkprobe.py` writes a two-row flat ODF: a plain cell and a conditional one, same style but for the
`style:map`, under a `Default` whose size is stated separately. Six probes at 26.2.4.2:

| probe | Default | cell | conditional cell holds | rows |
|---|---|---|---|---|
| `q1` | Calibri 20 | Calibri 11 | `"E"` | 276.0 / **298.2** |
| `q2` | Calibri 40 | Arial 9 | `"E"` | 256.3 / **256.3** |
| `q3` | Calibri 40 | Arial 9 | `"E"`, plus a `calcext:` block | 256.3 / 256.3 |
| `q4` | Calibri 40 | Arial 9 | the number **5** | 256.3 / **984.8** |
| `q5` | Calibri 40 | Arial 9 | a formula whose result is **`#N/A`** | 256.3 / **984.8** |
| `q6` | Calibri 40 | Arial 9 | the number 5, condition `>1000` | 256.3 / 256.3 |

`q1` **reproduces round 95's clean probe exactly**, 276 against 298.2 — and `q2` shows what it was
measuring. With the cell at 9 pt and the default at 40 the two candidate answers are 256 and 985,
and the answer is 256: the condition `cell-content()<40` against the string `E` never fires, so
`GetCondResult` hands back nothing and the cell is measured in its own font. What round 95's probe
measured was the *other* half of the seat — a conditional format clearing `bStdOnly`
(`column2.cxx`:937-941), which moves a row from the arithmetic estimate to a measured line at the
**same** size. Its two cells were both 11 pt, so 276 → 298.2 is that switch and nothing else.

`q4` and `q5` are the firing cases and they answer 984.8, one line of Calibri 40 in a cell whose own
font is Arial 9. `q6` is the control: the same numeric cell under a condition that cannot hold.

**Why the document fires where the text cells do not** is `lcl_GetCellContent`
(`conditio.cxx`:766-800): a formula cell yields a *number* when its result `IsValue()`, and an
error result is a value — zero. `IsValidStr` (`:1156-1183`) returns false outright for a numeric
operand against a string cell unless the operator is *not equal*. `sistem`'s `C4:T43` holds twelve
`VLOOKUP` cells per row, six returning `E` and six returning `#N/A`; the six errors are numeric zero,
`0 < 40` holds, and they alone are measured at the default's 11 pt. Confirmed at the document rather
than derived: rewriting every `<40` to `<-1000` drops rows 3–42 to **276.0**, and rewriting it to
`>-1000` leaves them at **298.2**.

### 1.3 It is visible in the reference's ink, and the two spellings of one workbook disagree

The rule reaches the drawing as well as the height — the same `fillFontOnly` builds the font
`ScDrawStringsVars` paints with. Spans of the reference's own page 1, by font, size and colour:

| | `#N/A` cells | `E` cells |
|---|---|---|
| `…srm-_-rekod-master__ods.pdf` (banked) | **Carlito-Regular 11.0, `#ff0000`** ×40 | LiberationSans 9.01, `#000000` ×80 |
| `…srm-_-rekod-master__xlsx.pdf` (banked) | LiberationSans 9.01 ×40 | LiberationSans 9.01 ×80 |

Same workbook, same binary, two spellings, and the `.ods` draws an Arial 9 cell in Calibri 11.
**`StylesBuffer::createDxfStyle` calls `rStyleSheet.ResetParent()`** before filling a conditional
style from a `<dxf>` (`sc/source/filter/oox/stylesbuffer.cxx`:3225-3234), so the SpreadsheetML path
has no parent to search; the ODF import parents its conditional styles like any other cell style.
That is the asymmetry, and it is measured at the ink before it is read in the source.

### 1.4 What changed in the tree

`OdsConditionalText` is the whole of the behaviour change: it reads each
`calcext:conditional-format`'s conditions and applied style names, resolves each applied style
through `OdsCellFormats.ResolveNamed` — the same parent walk a cell's own style takes — evaluates
the decidable conditions per cell, and hands the first matching one's **complete** font to the
`SheetConditionalText` overlay `XlsxConditionalStyles` already fills for SpreadsheetML. Every font
property is set rather than left null, because in the ODF model every one of them resolves through
the same parent search. `SheetConditionalText` gained a `DeclaredFontClass` so that a replaced face
takes its own fallback class with it; the SpreadsheetML reader leaves it null and is unaffected.

Two details cost a rebuild each and are worth recording:

- **`calcext:apply-style-name` names the style by its *display* name.** The style element is keyed
  by the encoded `style:name` — `ConditionalStyle_2` against `ConditionalStyle_5f_2`, since
  `SvXMLExport::EncodeStyleName` escapes the underscore — so a lookup by `style:name` misses every
  conditional style in a LibreOffice-written file, silently, and the resolver then answers its own
  10 pt default. The intermediate state was visible only as `#N/A` drawn at 10.01 pt.
- **A condition this cannot decide must stop the walk, not be skipped.** Calc applies the *first*
  matching entry (`ScConditionalFormat::GetCellStyle`), so applying a later decidable one over an
  earlier undecidable one would paint what the reference does not.

`contains-text` and `formula-is` are not evaluated. They are the corpus's two commonest forms, and
leaving them undecided is exactly the tree's behaviour before this existed.

### 1.5 Reach, with its base rate

`condfont-reach.py` walks every `.ods` of the converted corpus, resolves each applied conditional
style and each in-range cell's effective style through the parent chain, and evaluates the numeric
conditions the way `IsCellValid` does.

| | documents |
|---|---:|
| state a `calcext:conditional-format` with a condition | **60** |
| hold an in-range cell whose font would differ if its condition held (upper bound) | **45** |
| hold a cell where a **decidable** condition holds *and* the font differs | **5** |

So the upper bound is broad and the implemented reach is not: 45 of 60 documents carry cells that
*could* move and 5 of them carry a condition this can decide. The 40 that cannot are
`contains-text` and `formula-is`, and they are left exactly as they were. That is also why this
does not repeat round 95's `+1 −1`: `flightstandards-doc-Cross-reference-table_version02.ods` has
**5210 candidate cells and 0 decidable firing ones**, so the reader does not touch it.

The five: `sistem-rekod-markah-srm` (1200 firing cells, Arial 9 → Calibri 11),
`Computer and Software Services_50 State Comparison` (24), `032_Business_expenses_budget` (52, a
face-name difference only), `031_Business_expense_budget` (40, Posterama 10 → 9) and
`041_Business_budget` (10, Calibri 12 → 11).

## 2. The corpus measurement, and why it is 60 documents rather than 307

**The change cannot reach a document that states no conditional format, and that is a property of
the code rather than a hope about it.** `OdsConditionalText.Read` returns an empty dictionary
unless the sheet holds a `calcext:conditional-formats` element, and
`SheetCellFormats.WithConditionalText` returns `this` — the same instance — for an empty one. So
the 247 `.ods` of the converted corpus that state no conditional format render byte for byte as
they did.

That is shown as well as argued: **every eighth of those 247 — 31 documents — rendered at the
round's base and at head under `SOURCE_DATE_EPOCH`, one output directory per document, are
31 of 31 byte-identical**, every PDF checked for `%%EOF` before it was hashed. `render-ours.sh`,
`confine.list`, `confine-head.tsv`, `confine-base.tsv`.

The 60 that *can* move were swept at both binaries against the banked 26.2.4.2 reference, by the
gate's own rule:

| | base | head |
|---|---:|---:|
| `match` | **49** | **50** |
| `pages` | 1 | 0 |
| `pages,words` | 1 | 1 |
| `words` | 9 | 9 |
| rows excluded for a failure on either side in either run | 0 | 0 |

**Exactly one row moves, and it is the witness**: `sistem-rekod-markah-srm-_-rekod-master.ods`
goes from **22 pages of 26** to **26 of 26** at 16180 glyphs of 16180 either way — `pages` to
`match`. Nothing is lost. `flightstandards-doc-Cross-reference-table_version02.ods`, which round
95's fitted rule took from 461 pages to 464, is **461 of 461 before and after**: its 358 conditions
are `contains-text` and `formula-is`, so this reader does not decide them and does not touch it.

**Four of the five census candidates change their bytes and one does not**, rendered at both
binaries and hashed (`five-base.tsv`, `five-head.tsv`): `031_Business_expense_budget`,
`032_Business_expenses_budget`, `041_Business_budget` and `sistem` move;
`Computer and Software Services_50 State Comparison` does not, so the census over-predicted it by
one. Only `sistem` moves a *gate column* — the other three change a font size or a face without
changing a page or an alphanumeric character, which no gate column can see. That is the same
blindness `probes/sheet-ink-r94` recorded, arriving on a document the gate calls a pass.

*A whole-column figure is not quoted, because this round did not measure one: the full 307-document
sweep was started and abandoned under contention after the box's load average passed 20 with two
other rounds sweeping. What is above is the comparison that means anything — base against head, one
bank, one scorer — over every document the change can reach, with the other 247 held byte-identical
by a control.*

## 3. O16 — `TK-Syllabus` is not this, and the measurement that says so

The seat: 93.5 of the residual 205.57 `|ink|%` is on pages 1–100, the rows are in the same order
with ours two rows lower, and the page count is 1235 on both sides.

**O6's mechanism has essentially nil reach on it, and that is a measurement rather than a
judgement.** 26.2.4.2's own `fods` of the workbook resolves 413 `ConditionalStyle_N`, **every one
of them parented to `Default` and stating no text property at all**, and `Default` is
**Calibri 11 pt** — which is what 1790 of its automatic cell styles state outright and what
another 1018 inherit. So the parent search hands back the font the cell already has. The census
agrees from the other end: of 66982 cells inside a conditional range, **76 could differ at all**
and **0 hold a decidable firing condition**.

**What the drift is instead is the line count of wrapped rows.** The reference's own row heights
for that workbook are quantised — 299.952, 566.928, 835.056, 1103.76, 1373.04, 1641.312 twips, a
base plus 268.3 per extra line — so every row height in it is a statement about how many lines a
cell wrapped into.

Rendering our first 60 pages against the banked reference:

| | |
|---|---:|
> **Correction, round 99.** The two line counts in the table below, and the "534-line gap"
> drawn from them, are **withdrawn**. They are PyMuPDF `get_text` *line* counts, which measure
> two PDF writers' text-object habits rather than layout: the reference emits `157` and the cell
> beside it as two `line`s where we emit one, with every character at identical coordinates. The
> gap survives round 99's fix unchanged (8544 / 8010) precisely because it was never a layout
> quantity, and on *spans* the count runs the other way — 10998 reference against 11529 ours.
> Measured properly against the reference's own resolved `style:row-height`, **82909 of 82924
> rows already agreed at this round's base** and all fifteen that did not were exactly one line
> short. The seat this section named, `WrappedHeight`/`ParagraphsOf`, was also wrong: both
> already count a trailing empty paragraph. See `probes/sheet-wrap-r99`.

| our drawn lines, pages 1–60 | 7987 |
| the reference's | **8521** |
| pages where the reference draws more | 35 |
| pages where it draws fewer | 5 |
| pages where the two agree | 20 |

The first divergence is on **page 2**, and it is exactly one line: every baseline from
`A standard format has been applied to each sheet…` down is **13.5 pt lower** in the reference —
one row above it is one line taller there — while the seventeen lines above it agree to 0.1 pt.
Page 1's text differs by two characters at a wrap boundary, which is the same defect one line up.

### 3.1 The first divergence, attributed to a row and confirmed by variant

The page-2 divergence is **row 5 of `Reader Instructions`**, and the arithmetic closes on it with
nothing fitted. That sheet's `pm3` page layout is A4 portrait with 0.75 in margins, so page 1 takes
rows 0–4 (533.34 pt of row) and page 2 opens on row 5. The reference gives row 5 **4864.752
twips — 243.24 pt**, and its next block is drawn at 298.3; we give it 229.6 and draw ours at 284.8.
One line of 13.4 pt, exactly.

**The cell holds eighteen paragraphs and the eighteenth is empty** — `Contents`, a lead-in and
fifteen subject lines, then a bare `<text:p><text:span/></text:p>`. We draw seventeen lines and
size the row for seventeen; the reference draws the same seventeen and sizes it for eighteen.

One attribute settles it. Deleting that trailing empty paragraph from the reference's own `fods`
and converting it back through 26.2.4.2 moves the row from **4864.752 to 4597.2 twips** — 267.55
twips, one line — and 4597.2 is 229.86 pt, which is what this tree already computes. So on that
row everything is right except that a trailing empty paragraph is not counted.

It is the sheets-track instance of the rule `probes/odp-visual-r80` established for a slide: an
empty paragraph is measured, and it is measured at its own attributes rather than skipped.

**It is not the whole of O16.** Censused over the workbook, **1738 cells hold more than one
paragraph and 54 of those end in an empty one, across 52 rows** of the 7602 the reference
recomputes — nowhere near the 534-line gap over sixty pages. The rest is the *wrapped* line count
of single-paragraph cells, and the reference's own distribution is the instrument for it:

| lines the reference reserved | rows (recomputed) | rows (stated) |
|---:|---:|---:|
| 1 | 2997 | 64 |
| 2 | 2430 | 53 |
| 3 | 1145 | 41 |
| 4 | 542 | 39 |
| 5 | 251 | 19 |
| 6 | 119 | 10 |

**So O6 and O16 are two causes.** The instrument for the next round is
`fodsrows.py`: it gives the reference's per-row line count for all 1235 pages' worth of rows
directly out of one eight-second conversion, `(height − 299.952) / 268.3 + 1`, so a per-row
comparison against `SheetOptimalRowHeights.WrappedHeight` needs no rendering at all. The seat is
that method's line count — its own remarks say it was fitted to thirty probe rows — and
`ParagraphsOf`'s treatment of a trailing empty one, not `GetNeededSize`'s font construction.

## 4. Tests, and what else this round did not do

Ten non-fidelity projects, run individually: **109 / 521 / 259 / 146 / 1045 / 164 / 1275 / 728 /
309 / 1913, 0 failed and 0 skipped in every one.** Solution build 0 warnings, 0 errors.
`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552 discovered**, and the ten are exactly
the known names — `PageDrawingComparisonTests` ×4, `TabStopComparisonTests` ×4,
`SheetDrawingComparisonTests` ×1, `JustificationShrinkComparisonTests` ×1. The counts above
were read out of this round's own output rather than carried in from a brief — the spreadsheets
project is 1275 because this round adds four assertions to it.

`tests/corpus/features/sheet-conditional-font-source.fods` and
`SheetConditionalFontSourceTests` are the new coverage: four rows whose expected heights
(256, 256, 985, 985 twips) were read off 26.2.4.2 before anything was asserted, and whose two
*unmoved* rows are the controls that separate this rule from the one round 95 fitted.

## 5. O17 — not reached

`alle einzeln.xlsx` was not worked. `probes/o17-pivot-r97` characterises it and its seat stands
unchanged; grid geometry did not fall out of either of the two above, since one is a font source
and the other a wrapped line count in a workbook that holds no pivot table.

## Files

| file | what it is |
|---|---|
| `condfont-reach.py` | the reach census: applied conditional styles resolved through the parent chain, conditions evaluated as `IsCellValid` evaluates them |
| `condfont-reach.tsv` | its output — 60 documents, 45 upper bound, 5 decidable |
| `fodsrows.py` | resolved row heights per sheet out of a flat ODF; the round's primary instrument |
| `fodscells.py` | a sheet's cells with their styles, value types and formulas |
| `fodsstyles.py` | cell styles resolved through their parent chain |
| `mkprobe.py` | the minimal two-row conditional-font probe, parameterised by the two sizes and faces |
| `variant.sh` | converts a flat ODF back through 26.2.4.2 and prints its row heights |
| `variants.md` | every variant run this round, with the command and the answer |
| `sweep-ours.py` | our half of a column scored against a banked reference by the gate's own rule (copied unchanged from `probes/ods-residue-r95`) |
| `render-ours.sh` | our half of a document list under `SOURCE_DATE_EPOCH`, one directory per document, `%%EOF`-checked and hashed |
| `head-ods-rows.tsv`, `base-ods-rows.tsv` | the 60 conditional-format `.ods` at head and at base |
| `compare-ods.txt` | the join, excluding any row that failed on either side in either run |
| `confine.list`, `confine-head.tsv`, `confine-base.tsv` | the 31-document confinement control, 31 of 31 byte-identical |
| `five-base.tsv`, `five-head.tsv` | the census's five candidates hashed at both binaries — four move, one does not |
