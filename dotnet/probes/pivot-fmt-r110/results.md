# The pivot's own `<format>` records, which r108 censused as nil and which decide four properties

Round 108 seated O45 on five properties a pivot cell keeps and the reference throws away, and on
one approximation. This round censuses the four unmeasured properties, **retracts one of r108's
censuses**, implements the half that retraction exposed, and settles the approximation with an
authored workbook.

**The headline.** The reference does not only empty the range. `ScDPOutput::Output` ends with
`maFormatOutput.apply` (`dpoutput.cxx`:1190), which lays the pivot's own `<format>`/`dxf` records
over the generated styles — and r108's census of those records as **nil** was an instrument
artefact. There are **286 of them across 12 of the corpus's 28 pivot parts**.

**So clearing is not half a rule that can be shipped on its own, and the first cut of this round
shipping it was wrong.** Scored by *value* against 26.2.4.2's own resolved view of the 9878 cells
of the corpus's nineteen generatable pivot rectangles, clearing alone is **worse than merging** for
the size (9783 against 9872) and for the colour (9873 against 9876) — the two together are what
match:

| property | merge (what r108 shipped) | clear alone | clear + `dxf` |
| --- | ---: | ---: | ---: |
| face | 9877 | 9878 | **9878** |
| size | 9872 | 9783 | **9878** |
| colour | 9876 | 9873 | **9878** |
| fill | 9806 | 9752 | **9869** |

So this round reads the records. The nine cells still wrong are one workbook's field-button row and
corner cell, and every one is an under-application.

**What that is worth on the page: `033_Event_planning_tracker` goes from 11.27 to 0.15 of summed
unsigned ink and its MAJOR page clears** — the black band over 17.4 % of it, which r108 recorded as
a fill of unknown origin, is a `dxf` whose `<patternFill>` states a background and no pattern type.
Over the eleven pivot-bearing documents, 21.01 → **9.56**, four renderings moving and 303 of the
307 sheets documents byte-identical.

**And the colour margin the first cut of this round shipped on does not survive a value
comparison.** `clearfour.py` scored the boolean *does this property differ from the document's
default*, on which clearing the colour reads as 9873 against 9804 — a 69-cell gain. Compared as
*values*, with a theme colour resolved through the workbook's own theme and tint, merging is 9876
and clearing 9873: **clearing the colour alone is a net loss of three cells, not a gain of
sixty-nine.** The 69 were `033_Event_planning_tracker` body cells where the reference draws the
automatic colour over a black `dxf` fill that neither model painted, so they took no ink at all.
The boolean instrument counted them and the ink measurement disagreed with it — 21.01 → 21.14 —
and the ink was right.

---

## The retraction: `<format>` is never a direct child of `<pivotTableDefinition>`

`probes/pivot-res-r108/format-census.py` asked `root.findall(m('format'))` — a direct-child
search — and a `<format>` element is a child of `<formats>`. It therefore reported *"0 of the 28
pivot parts state a single `<format>` element"*, and r108's write-up, the O44 register row and
`SheetPivotStyle`'s remarks all carry that number.

`format-census.py` here is that script with one path changed. Run over the same eleven workbooks:

| workbook | parts stating `<format>` | records | what the `dxf` entries state |
| --- | ---: | ---: | --- |
| `033_Event_planning_tracker` | 1 | 84 | align, border, fill, font, numfmt |
| `053_Personal_asset_inventory` | 1 | 40 | fill, font |
| `026_Monthly_cash_flow_statement` | 4 | 96 | fill, font |
| `049_Expenses_calculator` | 1 | 23 | fill, font |
| `035_Project_plan_for_law_firms` | 1 | 18 | align, font+fill+border+align+numfmt+prot, numfmt |
| `037_Personal_money_tracker` | 2 | 23 | align, font, numfmt |
| `DynamicBubbleChart` | 1 | 1 | align |
| `007_Contextures_chart_sample` | 1 | 1 | align |
| **total** | **12 of 28** | **286** | every record names a `dxfId` and every `dxf` states something |

All five `applyNumberFormats` / `applyBorderFormats` / `applyFontFormats` / `applyPatternFormats` /
`applyAlignmentFormats` flags are `0` on every one of these definitions, and 26.2.4.2 applies the
formatting regardless — so the flags are not a filter to model.

**Banked**: `format-census.py`, `format-census.txt`.

---

## (1) The census: face, size, colour and fill inside a pivot range

`statedface.py`, the same shape as r108's `statedborders.py` and with three differences that
matter. It counts a cell's **resolved** format — its own `s`, else its row's when the row is
`customFormat`, else its column's, else `cellXfs[0]` — where `statedborders.py` counted only cells
carrying an `s` of their own. It counts a property as present only when it **differs from the
sheet's own default `cellXf`**, which is what the clearing would leave, rather than when it is
merely stated. And it counts the font's declared generic class — `<family val="N"/>` — as a
property of its own beside the face's name, because it is: `035_Project_plan_for_law_firms` states
`Cambria` `family="1"` on part of its pivot where its default states `Cambria` `family="2"`, and
the first census written here compared only the names and reported that document as stating
nothing at all. The sweep found it moving anyway, which is how the omission was caught.

Base rate is the cell count of the rectangle. Over all 28 pivot ranges of the eleven pivot-bearing
workbooks, and then over the 19 that are worksheet-cached — the only ones a grid is generated for,
and so the only ones the clearing can reach:

| property | all 28 ranges, 10086 cells | the 19 generatable ranges, 9878 cells |
| --- | ---: | ---: |
| face | 1 (0.01 %) | 1 (0.01 %) |
| declared class | 93 (0.92 %) | 93 (0.94 %) |
| size | 101 (1.00 %) | 101 (1.02 %) |
| colour | 191 (1.89 %) | 79 (0.80 %) |
| **fill** | 112 (1.11 %) | **0 (0.00 %)** |
| *border* (r108) | *0* | *0* |

**The fill is nil where it can be reached.** All 112 cells stating one are in `026` and `053`,
whose caches are `type="external"`: no data pilot is created for them, nothing is cleared and
nothing is generated. So the fill joins the border — by the seat's own rule, no code is written
for either.

Only three documents state any of the rest. The face is one cell, `037_Personal_money_tracker`'s
`B19`; the declared class is 91 cells of `033`, 1 of `035` and that same cell of `037`; the size
is 91 of `033` and 10 of `037`; the colour is 78 of `033` and that cell of `037`.

*The 9878 here is the stated `ref` rectangles; r108's 9970 is the **cleared** rectangle, the union
of `ref` with the page-field rows above it and the computed table. None of the eleven workbooks
states a page field, so on this corpus the two rectangles coincide and the difference is only in
what each script counted.*

**Banked**: `statedface.py`, `statedface.txt`.

---

## (2) The reference's own answer, cell for cell and value by value

Three models, scored against 26.2.4.2's own resolved view of every cell of every generatable
pivot rectangle, taken from `soffice --convert-to fods`:

- **merge** — the cell keeps what the workbook states, which is what this tree shipped before;
- **clear** — the cell is taken back to the document's `Normal` cell style and nothing else;
- **clear + dxf** — cleared, then the pivot's own `<format>` records laid over it, which is the
  reference's own rule.

`check-dxf.py` compares **values**, not "differs from the default": a theme colour is resolved
through the workbook's own theme and tint and compared as a hex string, a size as points, a face
as the family the reference's own `<style:font-face>` names. Where the reference writes
`style:use-window-font-color="true"` the colour it draws is the automatic one, which the `.fods`
does not state — 72 cells, all `033`'s — and those are excluded from the colour column rather than
counted for whichever model happens to agree.

| property | cells | merge | clear | clear + dxf |
| --- | ---: | ---: | ---: | ---: |
| face | 9878 | 9877 | 9878 | **9878** |
| size | 9878 | 9872 | 9783 | **9878** |
| colour | 9878 | 9876 | 9873 | **9878** |
| fill | 9878 | 9806 | 9752 | **9869** |

Per document, every one of the nineteen ranges is exact under clear+dxf except
`049_Expenses_calculator`, which is 45 of 54 on the fill: its field-button row `B3:G3`, its corner
`B4`, its grand-total column header `G4` and its grand-total row label `B11` take a fill the model
does not place. All nine are under-applications — the model paints nothing where the reference
paints `#F9FAF5`. `check-dxf.txt`.

### The null that proves the clearing is total

`033_Event_planning_tracker` with its `<formats>` element deleted and **nothing else changed**,
converted by 26.2.4.2: all ninety-one cells of `B5:N11` come back at the `Default` cell style — no
face, no size, no colour, no fill — with only the generated bold on the grand-total row. With the
element in place the same ninety-one carry a 12 pt `Consolas`-modern on a black ground.

That is a single-variable experiment on a real corpus workbook, and it is worth more than the
authored fixture in §4: it says the clearing removes **every** property this round censused, and
that everything the reference draws over a pivot afterwards is either one of the four generated
styles or a `dxf`.

**Banked**: `check-dxf.py`, `check-dxf.txt`, `dxf-model.py`, `clearfour.py` (the earlier boolean
instrument, kept because §0 quotes it), `fods-face.py`.

## (3) What is implemented, and three places the C++ tree is not the binary

`XlsxPivotFormats` is `FormatOutput` — the `dxfs` table, the `<pivotArea>` and its references, the
line matcher, and the placement — and `XlsxPivotGrid` calls it after the generated styles, which is
where `maFormatOutput.apply` sits. `SheetPivotStyle` gains `Dxf`, applied last, so a record's
weight beats the generated `Pivot Table Result` bold; a record's fill goes into the sheet's
decoration beside the generated borders.

**The clearing is now total for all five**: the face, its declared generic class, the size and
the colour come from the `Normal` `cellStyleXf`, `SheetFormatting.ClearBackgrounds` takes the fill
off the rectangle, and the records put back whatever they state.

*The fill half of the clearing was found by the fixture rather than by the corpus, which is the
whole reason to author one.* The census says **no** cell of the corpus's nineteen generatable
ranges states a fill, so nothing real could show it; the authored workbook states one on every
cell of its pivot, 26.2.4.2 removes it, and this tree kept it. The first cut of that fixture could
not have shown it either — its `dxf` fill and its cells' own fill were both yellow, so the two
candidate sources agreed by accident. They are different colours now.

**Three arms of the C++ tree read here are not in 26.2.4.2, and each was settled by applying one
format at a time to `033` and reading the reference's own `.fods`** — 84 records, one file each:

| the tree | 26.2.4.2 | how it was settled |
| --- | --- | --- |
| `tryHandleGrandTotals` (`PivotTableFormatOutput.cxx`:582) sends a `grandRow="1"` record to the grand-total row alone | no short circuit: such a record is matched like any other | `033`'s four `grandRow` **data** records each paint the whole data area — `#9` and `#22` white over `C6:N11`, `#29` black — and its `grandRow` **label** records paint nothing at all, which is what ordinary matching gives a label with no references. Four records, four predictions, and the grand-total path predicts none of them. |
| an `<alignment>` in a `dxf` reaches the cell | it reaches nothing | `033` states nine, three of which match its whole data area; the reference's automatic style for those cells carries no `fo:text-align` and no `fo:margin-left`. Which is why r108's 0 disagreements over 9970 cells on the justification and the indent still stand. |
| `PivotAreaType` is parsed | and never used | `PivotTableFormat::finalizeImport` reads `dataOnly`, `labelOnly`, `outline`, `grandRow`, `grandCol`, `offset`, `fieldPosition` and the references, and nothing else — so `type="all"`, `type="button"` and `type="origin"` behave as `normal`. |

**Two rules of the matcher that decide whole documents and are not what the markup looks like it
says.** A format's kind is Data when `dataOnly` — whose default is **true** — else Label when
`labelOnly`, else None, and a None record applies nothing at all because `applyMatchedLines` has an
arm for Label and an arm for Data and no third one; six of `033`'s eighty-four are inert that way.
And a record with **no references matches every line** through the broad path, so a bare
`<pivotArea outline="0"/>` paints the whole data area — which is how one record blackens
`033`.

**And the `dxf` fill rule is the one that produces the black.** A `<patternFill>` stating a
`bgColor` and **no** `patternType` becomes a *solid* fill, and the colour it paints is the
`bgColor`: the arm at `stylesbuffer.cxx`:1988-1994 moves `maFillColor` — which `importBgColor`
filled at :1890 — *into* `maPatternColor` and sets `mnPattern = XML_solid`, so the `fgColor`'s own
value is pushed aside into `maFilterPatternColor` and never painted. What makes `033`'s band
**black** is that its `bgColor` is `auto="1"`, and an automatic colour in the pattern position
resolves against the window **text** colour.

> **Corrected after the round.** This paragraph first said the solid takes the *pattern* colour and
> that an unstated pattern colour is what is automatic. The reader that was shipped
> (`XlsxPivotFormats.ReadFill`) was always right — it reads the `bgColor` — but the prose was not.
> `033` cannot separate the two readings, because its `fgColor` is unstated *and* its `bgColor` is
> `auto="1"`, so both paint black; a workbook stating a real `bgColor` under no `patternType`
> would.

The same file also states `patternType="none"` beside a `bgColor`, which applies nothing.
`Fill::finalizeImport`'s `mbDxf` arm, `stylesbuffer.cxx`:1988-2009.

**What is still not modelled, each with its reason.** A record's `offset` (no corpus pivot states
one), its `fieldPosition`, a `dxf`'s `<alignment>` (measured above as reaching nothing), a `dxf`'s
`<border>` (nil on this corpus) and its `<numFmt>` (a separate question from formatting). The
subtotal-reference arm is transcribed but untested: no corpus pivot states a
`defaultSubtotal` reference either.

## (4) The default-style half, settled by an authored workbook

`tests/corpus/regression/pivot-default-style.xlsx`, written by `make-default-fixture.py`. Three
fonts that differ in face, size and colour at once:

| where | font |
| --- | --- |
| the `Normal` `cellStyleXf` | Liberation Sans 11, black |
| `cellXfs[0]` | Liberation Serif 18, red |
| `cellXfs[1]`, on every cell of the pivot's rectangle | Liberation Mono 8, bold, green, yellow fill |

and three sheets: `Data` (the cache's source), `Cleared` (a hidden-header pivot over `A1:C4`, every
cell stating `cellXfs[1]`), and `Plain` — the same block three times with `cellXfs[1]`, with
`s="0"`, and with no `s` at all.

**26.2.4.2's own `.fods` of it:**

| cells | resolved to |
| --- | --- |
| `Cleared` `A1:C4`, all twelve | **Liberation Sans 11 black** — the `Normal` style. Row 4 additionally bold, which is the generated `Pivot Table Result`/`Title`. |
| `Plain` `A7:C10` (`s="0"`) | Liberation Serif 18 red — `cellXfs[0]` |
| `Plain` `A1:C4` (`s="1"`) | Liberation Mono 8 bold green on yellow — untouched |
| `Plain` `A13:C16` (no `s`) | the `Normal` style |

So **the clearing falls back to the `Normal` `cellStyleXf`, not to `cellXfs[0]`**, and every one
of the four properties is cleared when no `<format>` record puts one back — which is §2's null
again, on an authored file where the whole state can be stated rather than inferred.

`XlsxCellFormatTable` now carries `StyleDefault`, the `Normal` `cellStyleXf` resolved through the
same `Resolve` the cell formats go through, and `XlsxPivotGrid.Apply` takes it instead of
`SheetCellFormats.SheetDefault` — which is not the same entry twice over, because `SheetDefault` is
`cellXfs[0]` *or*, where the sheet states a `<col>` spanning to the last column, that column's
format (`XlsxSheetFormats.cs`:60-68). `033` and `035` both state one, at 11 pt where their
`cellXfs[0]` is 10. `XlsxCellFormats.NormalStyleXf` is the `builtinId="0"` lookup,
the same rule `XlsxStyles.DefaultFormatId` already applies to number formats and `probes/numfmt-r68`
already probed for them.

**The trap the corpus skill warns about caught this fixture once and was caught by a control.** The
first build omitted `<pivotCaches>` from `workbook.xml`; the pivot did not import at all, the cells
came back exactly as written, and the file read as evidence that *nothing* is cleared. What exposed
it was counting `table:data-pilot-table` in the output — 0 against the 8 that r108's known-good
fixture produces. The fixture that ships is r108's `make-fixture.py` with `styles.xml` and the sheet
list changed, so it is a modification of a file already known to import.

**And a second fixture beside it, for the other half.** `regression/pivot-format-records.xlsx` is
this file with a `<formats>` element added and nothing else changed — five records, one per arm —
so the two are a single-variable experiment on the whole subsystem. 26.2.4.2's own `.fods` of it:

| cells | resolved to |
| --- | --- |
| `A1`, `B1` — the corner and the first data field's header | the `Normal` style, Liberation Sans 11 black, **no fill** although the cells state `#ffff00` |
| `C1` | red, from a label record with one reference on the data dimension naming index 1 |
| `B2:C4`, the data area | Liberation Mono 8 pt **bold** on `#00b050` |
| `A4` | bold and otherwise the `Normal` style — the generated grand-total `Title` |
| anywhere | never the 18 pt Liberation Serif one of the five records states |

Five records, five predictions, five confirmations — including the two arms that separate
26.2.4.2 from the C++ tree read here: the bold comes from a `grandRow="1"` record that lands on
the whole data area, and the 18 pt from a `dataOnly="0"` record with no `labelOnly`, which is
inert.

**Banked**: `make-default-fixture.py`, `make-format-fixture.py`, `fods-face.py`.

---

## (5) The other half of the default-style finding, sized and not landed

`XlsxSheetFormats` builds a sheet's own default — what a cell resolves to when neither it, its row
nor its column states a format — from `cellXfs[0]` (`XlsxSheetFormats.cs`:52). The fixture above
says that is the wrong entry there too: its `Plain` sheet's `A13:C16` states no `s` at all and
26.2.4.2 gives it the `Normal` style, not `cellXfs[0]`. LibreOffice's own reason is in the reader —
`rAttribs.getInteger(XML_s, -1)` and an immediate return on a negative id — and
`XlsxStyles.DefaultFormatId` already applies exactly this rule to number formats.

**It is not landed here, because unlike the pivot half it is not nil-reach.** Censused over the
sheets track: of the **242** `.xlsx`/`.xlsm` that carry both a `cellXfs` and a `cellStyleXfs`,
**6 give `cellXfs[0]` and the `Normal` `cellStyleXf` different content** —
`jobs-bulletin-51-22-december-2025` (`fontId` 1 against 0),
`sectors-defense-and-aerospace` (`fontId` 3 against 0, plus a wrap and a top alignment),
`esurf-12-135-2024-t01`, `essd-16-3433-2024-t02`, `Published_Issuances_2024` and
`ans_mappings_of_eccairs_terms` (each an `<alignment>` on one side only). None of the six holds a
pivot table, so the pivot change is unaffected by them; but changing the *sheet* default is a
change to six real renderings and belongs to a seat that sweeps for it.

*Banked in this round's `normalcmp.py`.*

---

## (6) Reach

**Confinement.** All **307** sheets-track documents rendered at both binaries with
`SOURCE_DATE_EPOCH` fixed, `%%EOF` checked before hashing, each render deleted as it went:
**0 failures, 0 truncations, and 4 documents moved** — `033_Event_planning_tracker`,
`035_Project_plan_for_law_firms`, `037_Personal_money_tracker` and `049_Expenses_calculator`. The
other **303 are byte-identical**, the 64 `.xls` among them as the control, since the change is
inside `XlsxPivotFormats`, `XlsxPivotGrid` and `XlsxCellFormats`, which only the OOXML spreadsheet
path enters. `sheets-sweep-r110.tsv`.

Four of the seven generatable ranges carrying `<format>` records move and three do not, and the
three are explained rather than missed: `DynamicBubbleChart` and `007_Contextures_chart_sample`
state one record each and it is an alignment `dxf`, which reaches nothing.

**Page counts and alphanumeric characters** — the two checks `batch-check.sh` makes, in that
order, taken against the banked 26.2.4.2 reference in `/home/user/gate-orig-r83/ref` rather than
re-rendering it. Column 9's `glyphs`, not the token count:

| document | ref pages / glyphs | base | head |
| --- | --- | --- | --- |
| `alle einzeln` | 186 / 278868 | 186 / 278869 | 186 / 278869 |
| `Keywords_Mapping_Graphs_and_Charts` | 46 / 27201 | 46 / 27295 | 46 / 27295 |
| `DynamicBubbleChart` | 5 / 1728 | 5 / 1723 | 5 / 1723 |
| `049_Expenses_calculator` | 4 / 1548 | 4 / 1558 | 4 / 1558 |
| `007_Contextures_chart_sample` | 7 / 1849 | 7 / 1836 | 7 / 1836 |
| `037_Personal_money_tracker` | 5 / 2497 | 5 / 2476 | 5 / 2476 |
| `033_Event_planning_tracker` | 3 / 2650 | 3 / 2720 | 3 / 2720 |
| `035_Project_plan_for_law_firms` | 4 / 3879 | 4 / 3923 | 4 / 3923 |
| `027_Simple_personal_cash_flow_statement` | 10 / 7949 | 10 / 7932 | 10 / 7932 |
| `026_Monthly_cash_flow_statement` | 11 / 7852 | 11 / 7864 | 11 / 7864 |
| `053_Personal_asset_inventory` | 2 / 226 | 4 / 257 | 4 / 257 |

**Not one page count and not one alphanumeric count moves**, which is what a formatting change
should do and is stated rather than assumed — and it is why **no gate verdict can move on this
round in either direction**. `reach-r110.tsv`.

**What does move is the ink, and it is the largest figure this seat has produced.** Summed
unsigned `|ink|%` against the same banked reference, both legs rendered in one run:

| document | base | head | Δ |
| --- | ---: | ---: | ---: |
| `033_Event_planning_tracker` | 11.27 | **0.15** | **−11.12** |
| `049_Expenses_calculator` | 0.92 | **0.62** | −0.30 |
| `035_Project_plan_for_law_firms` | 1.13 | 1.11 | −0.02 |
| `037_Personal_money_tracker` | 1.25 | 1.24 | −0.01 |
| the other seven | | | 0.00 |
| **total** | **21.01** | **9.56** | **−11.45** |

**And one MAJOR verdict clears**, 4 → 3: `033`'s page 3, which r108 recorded as *"a fill or
background shading the reference has and we do not covers 17.4 % of that page"* and left
unexplained. It is the `dxf` fill, and that page is now at 0.15 of summed unsigned ink over the
document's three. `ink-r110.tsv`.

*Read against the first cut of this round, which cleared the colour alone and shipped it on a
cell-for-cell score: that was **+0.13** of ink, and the instrument it was shipped on — a boolean
"does this property differ from the default" — is the one the ink disagreed with. Both halves
together are −11.45. A property cleared without the half that puts it back is not half a fix.*

---

## Tests

Every project was run on its own so a truncated run cannot hide as a pass.

Ten non-fidelity projects — Containers 109, Core 560, Markup 259, OpenDocument 160,
Presentations 1107, Rendering 164, Spreadsheets **1342**, Text 728, Vector 309,
WordProcessing 1938 — **6676 passed, 0 failed, 0 skipped**.

*`XRay.Office.Vector.Tests` reported 1 failed of 309 on one run and 309 of 309 on the two after it,
with nothing in this round able to reach that project; the failing run was concurrent with another
session's whole-suite run on the same host. It is the invented-failure-under-load case
`CLAUDE.md` records, and it is written down rather than left out.*

*The Spreadsheets figure is this branch's base of 1333 plus this round's nine. It is **not** the
1337 the merged HEAD reports, which is the same 1333 plus `agent/chartinner`'s four — two totals
agreeing is not two trees agreeing.*

`XRay.Office.Fidelity.Tests`: **Failed: 10, Passed: 542, Total: 552** — the known ten and no
eleventh: `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` on
`paginated.fodt`/`.doc`/`.docx`/`.rtf`,
`TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` on
`list-label-overrun.doc`/`.fodt`/`.docx`/`.odt`,
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` on `sheet-rich-text.xlsx`,
and `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt` on
`justify-shrink-2013.docx`.

**Nine new tests over the two authored fixtures, and six of the nine fail at `a311b00e2`.**
`SheetPivotDefaultStyleTests` is four and `SheetPivotFormatRecordsTests` five; with
`SheetPivotStyle.cs`, `SheetDecoration.cs`, `XlsxCellFormats.cs`, `XlsxPivotGrid.cs` and
`XlsxReader.cs` taken back to the base and `XlsxPivotFormats.cs` removed, the six that fail are
the clearing of the face and the size, the clearing of the colour, the broad-match record, the
`dxf` fill, the label record, and the cleared base under the records. The three that pass are the
controls that must hold either way: a cell stating `cellXfs[0]` outside the pivot, the inert
record reaching nothing, and the generated grand-total bold.

`SheetPivotPackedTests` and `SheetPivotGridTests`, r107's and r108's fifteen, are unchanged and
still pass.

---

## (7) What is left, and what it is worth

- **Nine cells of `049_Expenses_calculator`.** Its field-button row, its corner cell, its
  grand-total column header and its grand-total row label take a `#F9FAF5` fill the matcher does
  not place. Removing the label-axis guard — `bMatchRows = (eType != Label) || bHasRowReferences`,
  `PivotTableFormatOutput.cxx`:678-681 — recovers two of the nine and costs **27** on `033`, so the
  guard is in 26.2.4.2 and the remaining seven are something else. The button row is not reachable
  by `applyMatchedLines` at all, which places a column label at `nColumnHeaderStartRow + n` and
  never at the table's own first row.
- **The `<alignment>`, `<border>` and `<numFmt>` halves of a `dxf`**, each measured as nil or as
  reaching nothing (§3).
- **A record's `offset`, its `fieldPosition`, and the subtotal-reference arm.** No corpus pivot
  states any of the three, so the transcription of the first two is absent and the third is
  written and unwitnessed.
- **The sheet default itself is still `cellXfs[0]`,** where 26.2.4.2 uses the `Normal`
  `cellStyleXf` for a cell stating no `s`. §5 sizes it at 6 of 242 corpus workbooks — not nil, so
  it needs a sweep of its own.
- **`053_Personal_asset_inventory` still cannot be scored.** 4 pages against the reference's 2 at
  base and at head, so `pdf-image-diff.py` reports none.

## Citations, re-checked by hand

Every `file:line` was re-opened in `/home/user/libreoffice-core`. **That tree is not the reference
binary's source** — `configure.ac` declares 27.2.0.0.alpha0+, the binary is 26.2.4.2, and the
checkout is one bulk import — so each is *this tree*, and on this subsystem the two are measurably
different in three places (§3). The arm that measures the actual reference is the
`--convert-to fods` oracle throughout.

| Citation | What it should be | Verified |
| --- | --- | --- |
| `pivottablebuffer.cxx`:1322-1338 | `PivotTable::finalizeImport`, the `clearContents(… HARDATTR \| STYLES …)` over the stated range | yes |
| `pivottablebuffer.cxx`:1415-1417 | the `maFormats` loop that finalizes each `PivotTableFormat` | yes |
| `PivotTableFormat.cxx` (whole file) | `finalizeImport` reads `dataOnly`, `labelOnly`, `outline`, `grandRow`, `grandCol`, `offset`, `fieldPosition` and the references — and never `meType` | yes |
| `dpoutput.cxx`:1190 | `maFormatOutput.apply(*mpDocument)`, the last statement of `outputDataResults` | yes |
| `dpoutput.cxx`:1193-1226 | `ScDPOutput::Output` — `CalcSizes`, `maFormatOutput.prepare`, then `DeleteAreaTab(…, ALL)` | yes |
| `PivotTableFormatOutput.cxx`:171-200 | `prepare`, and `nMaxNumberOfIndices` assigned rather than maxed | yes |
| `PivotTableFormatOutput.cxx`:322-416 | `findMatchingLines`, its two passes and the broad fallback | yes |
| `PivotTableFormatOutput.cxx`:582 | `tryHandleGrandTotals` — present here, absent from 26.2.4.2's behaviour | yes |
| `PivotTableFormatOutput.cxx`:657 | `FormatOutput::apply` | yes |
| `PivotTableFormatOutput.cxx`:678-681 | the label axis guard, which 26.2.4.2 *does* have | yes |
| `PivotTableFormatOutput.cxx`:685-688 | `nColumnHeaderStartRow` | yes |
| `stylesbuffer.cxx`:1978-2009 | `Fill::finalizeImport`, its `mbDxf` arm and the `XML_none` early out | yes |
| `stylesbuffer.cxx`:1740-1762, 1866-1892 | `PatternFillModel`'s dxf defaults; `fgColor` is the pattern colour and `bgColor` the fill colour | yes |
| `workbookhelper.cxx`:727-747 | `finalizeWorkbookImport` runs `getPivotTables().finalizeImport()` **after** every sheet | yes |
| `workbookfragment.cxx`:556-571 | `importSheetFragments` then `finalizeWorkbookImport` | yes |
| `XlsxSheetFormats.cs`:52, :60-68 | `SetSheetDefault(pooled[0])`, and a full-width `<col>` overriding it | yes |
