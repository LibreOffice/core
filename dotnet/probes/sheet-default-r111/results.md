# The sheet default is the `Normal` `cellStyleXf`, and this tree used `cellXfs[0]`

Round 110 settled this for the cells a pivot table clears and handed on the sheet-wide case with a
census beside it: *6 of 242 corpus workbooks give `cellXfs[0]` and the `Normal` `cellStyleXf`
different content*. This round re-runs that census, asks the reference itself what it resolved
those workbooks' cells to, lands the change, and measures what it is worth.

**The headline, in three parts.**

1. **The rule holds, and the corpus confirms it rather than merely failing to contradict it.**
   26.2.4.2's own `.fods` builds the `Default` cell style out of the `Normal` `cellStyleXf` in all
   six, and `sectors-defense-and-aerospace.xlsx` and `Published_Issuances_2024.xlsx` are the first
   *real* documents that can say so: their `cellXfs[0]` states an alignment their `Normal` entry
   does not, and the reference's `Default` carries the `Normal` one.

2. **The witness set is smaller than six.** Three of the six differ only in `<alignment>`
   attributes whose every value is the schema's own default, so the resolved formats are equal and
   nothing separates the two readings there. **Three of 243** differ effectively — the base rate is
   243 `.xlsx`/`.xlsm`, of 307 sheets-track documents; the other 64 are `.xls` and take a different
   default entirely.

3. **The reach on the page is nil, and that is measured rather than assumed.** In 26.2.4.2's own
   resolved view of all six, **not one non-empty cell resolves to the `Default` cell style** — every
   cell that carries text states an `s`, or sits under a `<col>` that does. The confinement sweep
   over all **307** sheets-track documents at both binaries agrees: **0 moved, 0 failed**, every PDF
   byte-identical. So no page count and no glyph count can move either, and none does.

**What the change is worth anyway** is that two readers of the same question stop disagreeing.
`XlsxStyles.DefaultFormatId` has taken the `Normal` route for the **number format** of an unstated
cell since `probes/numfmt-r68`; `XlsxSheetFormats` took `cellXfs[0]` for its **font, alignment and
number format**. On a workbook separating the two entries by a number format, this tree would have
*extracted* the cell in one format and *drawn* it in another, and no reading of either file would
explain the pair. The corpus has no such workbook, which is why the disagreement survived to r111.

---

## (1) The witness set, and why it is three and not six

`census.py` over the sheets track's 243 `.xlsx`/`.xlsm` — every one of which carries both a
`cellXfs` and a `cellStyleXfs`, so the base rate for every count below is **243 of 307**.

*r110 quotes 242 for the same question. Re-run with the paths quoted, `normalcmp.py` — r110's own
script, banked here unchanged as the raw-markup leg — reports 243 of 243 with none skipped. The
missing one is a quoting artefact of that round's invocation, not a workbook.*

**The raw markup differs in six** (`normalcmp-r111.txt`, r110's script):

| workbook | `cellXfs[0]` states | the `Normal` `cellStyleXf` states |
|---|---|---|
| `jobs-bulletin-51-22-december-2025` | `fontId="1"` | `fontId="0"` |
| `sectors-defense-and-aerospace` | `fontId="3"`, `<alignment vertical="top" wrapText="1"/>` | `fontId="0"`, no alignment |
| `Published_Issuances_2024` | `<alignment horizontal="left" vertical="top"/>` | no alignment |
| `esurf-12-135-2024-t01` | `<alignment horizontal="general" vertical="bottom" textRotation="0" wrapText="false" shrinkToFit="false"/>` | no alignment |
| `essd-16-3433-2024-t02` | the same five attributes | no alignment |
| `ans_mappings_of_eccairs_terms` | `<alignment vertical="bottom" wrapText="0" shrinkToFit="0" readingOrder="0"/>` | no alignment |

**Three of the six resolve to the same format.** The last three state an `<alignment>` whose every
attribute carries the schema's own default value. The element's mere presence forces
`applyAlignment` true (`Xf::importXf`, `stylesbuffer.cxx`:2186) so it is not ignored — it is
applied, and applying `vertical="bottom"` to a format whose vertical alignment is already
`bottom` changes nothing. `census.py` folds those defaults out; `normalcmp.py` does not, which is
the whole difference between six and three.

**And the remaining three differ less than the ids suggest**:

| workbook | the difference that survives |
|---|---|
| `jobs-bulletin-51-22-december-2025` | both fonts are **Calibri 11**. `fontId="1"` is `<sz val="11"/><name val="Calibri"/>` and nothing else; `fontId="0"` adds `<color rgb="FF000000"/>`, `<family val="2"/>` and `<scheme val="minor"/>`. So: the **declared generic class** (unstated against *swiss*) and an explicit black where the other is automatic. |
| `sectors-defense-and-aerospace` | its four fonts are **byte-identical** — `<sz val="12"/><color rgb="FF000000"/><name val="Calibri"/>` four times — so `fontId="3"` against `fontId="0"` is nil. What survives is the **alignment**: `vertical="top" wrapText="1"` against nothing. |
| `Published_Issuances_2024` | the **alignment** alone: `horizontal="left" vertical="top"` against nothing. |

The declared generic class is not cosmetic in this tree — it decides which installed face a missing
one falls back to (`SheetDeclaredFontShapeTests`), and Calibri is not installed here.

**Banked**: `census.py`, `census.txt`, `normalcmp-r111.txt`, `xlsx-list.txt`.

---

## (2) The reference's own answer, without rendering anything

`soffice --convert-to fods` on each of the six, eight seconds apiece. A `.fods` states the
document's `Default` cell style outright, so the question *"which of the two entries did 26.2.4.2
build it from"* is answered by reading one element.

| workbook | 26.2.4.2's `Default` `table-cell` style | which entry that is |
|---|---|---|
| `sectors-defense-and-aerospace` | `style:vertical-align="bottom"`, **no** `fo:wrap-option` | the **`Normal`** entry — `cellXfs[0]` would give `top` and `wrap` |
| `Published_Issuances_2024` | `style:vertical-align="bottom"`, **no** `fo:text-align` | the **`Normal`** entry — `cellXfs[0]` would give `left`/`top` |
| `jobs-bulletin-51-22-december-2025` | `style:font-family-generic="swiss"`, `fo:font-size="11pt"` | the **`Normal`** entry — `cellXfs[0]`'s font declares no class |
| the other three | `vertical-align="bottom"`, no wrap, no text-align | consistent with both, as §1 predicts |

Three real documents, three predictions, three confirmations — and the three that predict nothing
predict nothing for a stated reason rather than by coming out equal.

**And the authored fixture, re-measured this round rather than taken from r110.**
`tests/corpus/regression/pivot-default-style.xlsx` states Liberation Sans 11 black in the `Normal`
`cellStyleXf`, Liberation Serif 18 red in `cellXfs[0]` and Liberation Mono 8 bold green in
`cellXfs[1]`. Converted here by 26.2.4.2, its `Plain` sheet:

| cells | resolved to |
|---|---|
| `A13:C13`, stating **no `s` at all** | the `Default` style — Liberation Sans 11 black, the **`Normal`** entry |
| `A7:C7`, stating `s="0"` | Liberation Serif 18 red — `cellXfs[0]` |
| `A1:C1`, stating `s="1"` | Liberation Mono 8 bold green |

That is the single-variable experiment: one file, three blocks differing only in their `@s`, and
the unstated block and the `s="0"` block come back in different fonts.

### The measurement that decides the reach, taken before any code was written

`fods-default-cells.py` walks the reference's own resolved view and asks which cells actually land
on the `Default` style — a cell's own `table:style-name`, else its row's, else its column's
`table:default-cell-style-name`.

**Across all six workbooks and all 24 of their sheets: 0 non-empty cells.** Every cell carrying
text states a format of its own or sits under a column that states one. `sectors-defense-and-aerospace`'s
`Key` sheet does carry a five-column run defaulting to `Default`, and it is empty.

So the change cannot move ink on this corpus, and that was known before the sweep confirmed it.

**Banked**: `fods-default-cells.py`, `fods-default.txt`.

---

## (3) What this tree used, and what the rule actually is

`XlsxSheetFormats.Read` built the sheet's fallback from `cellXfs[0]` (`:52`, `builder.SetSheetDefault(pooled[0])`).

**It is not a blind substitution, because `SheetDefault` answers two questions at once.** A sheet
that states a `<col>` spanning to the last column has that column's format as its default — the
run is a statement about the sheet rather than about sixteen thousand columns, and
`ScTable::ApplyPatternArea` puts exactly such a run into `aDefaultColData`
(`sc/source/core/data/table2.cxx`:2980-2999) instead of allocating columns for it. That arm was
already here (`XlsxSheetFormats.cs`:60-68) and is unchanged. So the rule is:

> a cell that states no `s`, in a row that states none and a column that states none, takes the
> **`Normal` `cellStyleXf`** — unless the sheet states a full-width `<col style>`, whose format
> replaces it.

`Published_Issuances_2024` is the corpus workbook that states one (`<col>` naming `cellXfs[5]`,
Montserrat 9), which is a second reason its `cellXfs[0]` reaches nothing.

**One guard had to move with it.** `SheetCellFormats.Builder.SetSheetDefault` ignored an index of
0 — harmless while the first caller was `pooled[0]` and `_sheet` already started at 0, and wrong
once the sheet default can be set twice: a full-width `<col>` whose format happens to be the plain
`SheetCellFormat.Default` must be able to take a non-default `Normal` style back off. Pool index 0
is a legitimate answer here, unlike for the cell, row and column setters, where it means *not
stated*.

---

## (4) What is implemented, and the reader it had to agree with

- `XlsxSheetFormats.Read` interns `XlsxCellFormatTable.StyleDefault` — the `Normal` `cellStyleXf`,
  resolved through the same `Resolve` the cell formats go through, which r110 added and used for
  the pivot clearing alone — and makes that the sheet default. The full-width `<col>` arm still
  overrides it.
- A **rich** cell's runs are resolved over the same base. `ReadRichCell` took an index into
  `cellXfs` and fell back to `0`; it now takes the resolved `SheetCellFormat` and falls back to
  `StyleDefault`, so a rich cell stating no `s` is not drawn against a different default from the
  plain cell beside it.
- `SheetCellFormats.Builder.SetSheetDefault` accepts pool index 0, as §3 says.

**The neighbouring reader, checked rather than assumed.** `XlsxStyles.DefaultFormatId` already
reads the `builtinId="0"` `cellStyle`'s `xfId` for the **number format** an unstated cell takes
(`probes/numfmt-r68`, measured on both installed binaries). The resolved `SheetCellFormat` carries
a `NumberFormat` too, and before this round the sheet default's came from `FormatFor(0)` —
`cellXfs[0]`'s `numFmtId` — while the extraction path's came from `styles.Default`. The two now
come from the same entry. No corpus workbook separates them (all 243 give the two entries the same
`numFmtId`), so this fixes nothing that is currently visible and removes a way for the tree to be
incoherent.

**Two neighbouring readers checked and deliberately *not* changed:**

- `XlsxCellDecoration` has no sheet default at all: a cell with no `s` gets no fill and no border,
  where the `Normal` entry's `fillId`/`borderId` would in principle reach it. Nil on this corpus —
  all 243 give the `Normal` entry `fillId="0" borderId="0"`, and so does every `cellXfs[0]`. Left
  open below rather than written blind.
- `XlsWorkbookReader` and `OdsCellFormats` each set their own sheet default and are untouched: a
  BIFF document's is the pattern `XclImpXF::CreatePattern` builds, and an ODF document's is the
  `Default` cell style by construction.

---

## (5) Reach, measured by rendering

**Confinement.** All **307** sheets-track documents rendered at both binaries with
`SOURCE_DATE_EPOCH` fixed, `%%EOF` checked before hashing and each render deleted as it went
(`sweep.sh`, r110's, unchanged):

```sh
sweep.sh sheets-all.txt /home/user/r111sd-base /home/user/r111sd-head sheets-sweep-r111.tsv
```

> **307 documents, 0 failed, 0 truncated, and 0 moved.** Every PDF is byte-identical between the
> two binaries, the three witness workbooks included.

That is a stronger statement than the two checks `batch-check.sh` makes, and it contains them: a
byte-identical PDF has the same page count and the same alphanumeric characters by construction.
The 64 `.xls` of the track are the control — the change is inside `XlsxSheetFormats` and
`SheetCellFormats`, which only the OOXML spreadsheet path enters — and they are identical too, as
are the 240 `.xlsx` the census says cannot separate the two entries.

**Page counts and alphanumeric characters for the six witnesses anyway**, taken against the banked
26.2.4.2 reference in `/home/user/gate-r111/ref` rather than re-rendering it. Column 9's `glyphs`,
not the token count (`reach.sh`, r110's; `reach-r111.tsv`):

| document | ref pages / glyphs | base | head |
|---|---|---|---|
| `esurf-12-135-2024-t01` | 1 / 658 | 1 / 658 | 1 / 658 |
| `Published_Issuances_2024` | 1 / 4294 | 1 / 4294 | 1 / 4294 |
| `essd-16-3433-2024-t02` | 4 / 2349 | 4 / 2346 | 4 / 2346 |
| `jobs-bulletin-51-22-december-2025` | 12 / 10033 | 12 / 10033 | 12 / 10033 |
| `ans_mappings_of_eccairs_terms` | 191 / 184828 | 191 / 184827 | 191 / 184827 |
| `sectors-defense-and-aerospace` | 449 / 139231 | 449 / 139163 | 449 / 139163 |

Not one page count and not one glyph count moves, so **no gate verdict can move on this round in
either direction**, and none does.

*One of the six is on this session's own list of reference renders that are not reproducible run to
run — `ans_mappings_of_eccairs_terms`, `probes/gate-r111/ref-reproducibility.tsv`. It does not
matter here: base and head are byte-identical, so nothing is being credited to this tree against
that reference at all. The three residuals above (−3, −1, −68) are this tree's standing position
against 26.2.4.2 and are untouched by this round.*

**The ink was not measured, deliberately.** `pdf-image-diff.py` compares rasterised pages; two
byte-identical PDFs raster to identical pages, so the figure is 0.00 by construction on all 307 and
running it would be an expensive way to restate the hash.

**So the honest headline is that nothing on the page moved.** The change is a correctness fix with
nil measured reach on this corpus, and it is written down as one rather than quoted as a census of
six workbooks and left to imply a score.

---

## (6) Tests

Every project run on its own so a truncated run cannot hide as a pass.

Ten non-fidelity projects — Containers 109, Core 573, Markup 259, OpenDocument 160,
Presentations 1114, Rendering 164, Spreadsheets **1352**, Text 728, Vector 309,
WordProcessing 1938 — **6706 passed, 0 failed, 0 skipped**.

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552, Skipped: 0** — the known ten
and no eleventh: `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` on
`paginated.fodt`/`.doc`/`.docx`/`.rtf`,
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` on
`list-label-overrun.doc`/`.fodt`/`.docx`/`.odt`,
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` on `sheet-rich-text.xlsx`, and
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt` on
`justify-shrink-2013.docx`. `tests-fidelity.log`.

*The Spreadsheets figure is the merged head's 1346 plus this round's six. It is not r110's 1342 —
that branch's base was different, and two totals agreeing would not be two trees agreeing anyway.*

**Six new tests, `XlsxSheetDefaultFormatTests`, and two of the six fail at `a362d41f9`.** The two
that fail are the two that assert the sheet default itself — the face/size/colour, and the
alignment and wrap. The other four are controls that must hold either way and do:

| test | at base |
|---|---|
| `ACellStatingNoStyleTakesTheNormalCellStyleAndNotCellXfsZero` | **FAIL** |
| `TheUnstatedCellTakesNeitherTheAlignmentNorTheWrapOfCellXfsZero` | **FAIL** |
| `ACellStatingStyleZeroStillTakesCellXfsZero` | pass — the control that would catch swapping the two tables |
| `ABoundedColumnStyleStillWinsOverTheNormalCellStyle` | pass |
| `AFullWidthColumnStyleReplacesTheSheetDefault` | pass — the arm §3 says is not a substitution |
| `TheFormatTableSeparatesTheNormalStyleFromCellXfsZero` | pass — that the fixture separates them at all |

**No new corpus fixture.** The authored file this round needs already exists —
`tests/corpus/regression/pivot-default-style.xlsx`, r110's, whose `Plain` sheet states the same
block three times with `s="1"`, with `s="0"` and with no `s` at all. The third of those was written
for this question and left unasserted; `SheetPivotDefaultStyleTests` asserts the first two.
The new tests use an in-memory package instead, in the shape of
`XlsxUnstyledCellFormatTests` — because the case that needed separating is the *corpus* shape
(an `<alignment>` on the `cellXfs` side only, a bounded `<col style>`, and a full-width one), and
building it as XML in the test states the whole experiment where the reader can see it. The
corpus skill's own trap applies: a fixture minimal enough to be obviously correct can be minimal
enough to answer a different question, and here the guard is the four control tests rather than a
count of imported objects.

---

## (7) What is left, and why

- **The decoration half of the same question.** `XlsxCellDecoration` gives a cell that states no
  `s` no fill and no border at all, where the `Normal` `cellStyleXf`'s `fillId`/`borderId` should
  reach it exactly as its `fontId` now does. **Nil on this corpus**: all 243 `.xlsx`/`.xlsm` give
  the `Normal` entry `fillId="0" borderId="0"`, and so does every `cellXfs[0]`. It is a one-line
  omission with no witness, and writing it blind would be a change no measurement could check —
  the seat's own rule. An authored fixture would settle it in an afternoon.
- **The rich-text base still ignores the column.** `ReadRichCell` resolves a cell's own `s`, then
  its row's, then the sheet default — and skips the `<col style>` between them, which the plain
  path honours. Unchanged this round because it is a different bug from this seat's; not censused.
- **`XlsxStyles.DefaultFormatId` and `XlsxCellFormats.NormalStyleXf` are the same lookup written
  twice**, in two files, over the same `cellStyles` element. They agree today and there is no test
  that they must. Neither was merged into the other here because they are read at different times
  from different roots.
- **The `.xls` and `.xlsb` sides are untouched and are different questions.** A BIFF workbook's
  default is the pattern `XclImpXF::CreatePattern` builds (`XlsWorkbookReader`:874) and XLSB builds
  no `SheetCellFormats` at all.
- **Nine cells of `049_Expenses_calculator`**, the second piece this seat was offered, was **not
  opened**. r110 sized it as a real matcher question — the field-button row is not reachable by
  `applyMatchedLines`, which places a column label at `nColumnHeaderStartRow + n` and never at the
  table's own first row — and the naive fix costs 27 cells on `033`. It is not a follow-on of the
  sheet default and would have needed its own scoring run.

---

## Commands

```sh
#  (1) the census
python3 census.py xlsx-list.txt                       # 243 with both tables, 3 effectively differ
python3 ../pivot-fmt-r110/normalcmp.py "$@"           # the raw-markup leg, 6

#  (2) the reference's own answer, eight seconds a workbook
/opt/libreoffice26.2/program/soffice -env:UserInstallation=file:///tmp/prof \
    --headless --convert-to fods --outdir /tmp/fods <workbook>
python3 fods-default-cells.py /tmp/fods/*.fods        # which cells land on `Default`: 0 non-empty
python3 ../pivot-fmt-r110/fods-face.py /tmp/fods/pivot-default-style.fods Plain A1:C1 A7:C7 A13:C13

#  (5) reach
dotnet publish tools/XRay.Office.Cli/XRay.Office.Cli.csproj -c Debug -o <dir>   # once per leg
./sweep.sh sheets-all.txt <base-cli> <head-cli> sheets-sweep-r111.tsv       # 307, 0 moved
./reach.sh witness.txt <base-cli> <head-cli> /home/user/gate-r111/ref reach-r111.tsv
```

**Banked**: `census.py`, `census.txt`, `normalcmp-r111.txt`, `xlsx-list.txt`,
`fods-default-cells.py`, `fods-default.txt`, `sweep.sh`, `sheets-sweep-r111.tsv`, `reach.sh`,
`reach-r111.tsv`, `tests-fidelity.log`.
