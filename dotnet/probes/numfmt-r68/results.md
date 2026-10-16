# Four number-format defects, one theme-colour defect, and one that is not a defect

**Measured 2026-09-06 in `/home/user/wt-numfmt`, branch `agent/numfmt`, base `0fc357beb`.**
Environment, stated once because a stored figure is evidence about an environment and not about
the code:

| | |
|---|---|
| ours | `XRay.Office.Cli` built from this worktree, `XRAY_OFFICE_BUNDLED_FONTS` unset |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, the tree's target, with the eight Latin duplicate faces and the Latin Noto moved aside |
| ref24 | `/usr/bin/soffice` — **24.2.7.2**, which is what `batch-check.sh` renders the reference half with |
| corpus | `/home/user/sample-files`, 947 documents; banked gate at `2f4709c08` in `/home/user/gate-2f47/` |
| brief | `probes/mismatch-classify-01/classification.tsv`, five defects on four documents |

**Every expectation below was read off a rendered page**, through
`pymupdf`'s span list rather than through `pdftotext`, which cannot show a figure space or a
colour. **On every one of the questions asked, 24.2.7.2 and 26.2.4.2 answer identically** — so
none of these is the version gap and all of them are ours. Where a case could only be settled by
a file the corpus does not contain, the probe builds one.

Scripts:

```sh
python3 make-codes.py   codes.xlsx     # 30 cells: built-in ids, hour placeholders, day names
python3 make-default.py default.xlsx   # which cell format an unstyled cell takes
python3 census-defaults.py             # reach of the default/row/column rules, whole corpus
python3 census-theme-fonts.py          # reach of the theme font colour, whole corpus
python3 census-scatter-axis.py         # reach of the scatter-over-category-axis format
```

---

## 1. `042_Business_monthly_budget_4e4d092f.xlsx` — an accounting format applied to nothing

**What the file says.** Its cells carry no `s` attribute at all: `<c r="D6"><v>54000</v></c>`, in a
row with no `customFormat` and a column with no `style`. `cellXfs[0]` and the `Normal`
`cellStyle`'s `cellStyleXfs[0]` both name **built-in id 40**, which the file does not spell out.

**What 26.2 answers.** `54,000.00`, `(6,000.00)`, `500.00` — id 40 is
`#,##0.00_);[RED](#,##0.00)`. 24.2 the same.

**What we answered.** `54000`, `-6000`, `500` — `General`.

**Mechanism.** Two independent faults, and the brief was right that it is *one* rule and not
three.

- A cell that states no `s` was given `NumberFormatCode.General`. LibreOffice reads an absent
  `@s` as **no XF at all** — `rAttribs.getInteger(XML_s, -1)`,
  `sc/source/filter/oox/sheetdatacontext.cxx`:371 — and `SheetDataBuffer::setCellFormat` returns
  immediately on a negative id (`sheetdatabuffer.cxx`:721). The cell then shows whatever the
  sheet already put there.
- What the sheet already put there is settled by `default.xlsx`, which gives the Default cell
  style, `cellXfs[0]`, a `<col style>` and a `customFormat` row four distinguishable formats.
  **Both binaries answer identically on all six cells:**

  | cell | 26.2.4.2 | 24.2.7.2 | the rule |
  |---|---|---|---|
  | no `s` | `1.0` | `1.0` | the **Default cell style** — `cellStyleXfs[` the `Normal` `cellStyle`'s `xfId` `]`, *not* `cellXfs[0]` |
  | `s="0"` | `1.000` | `1.000` | `cellXfs[0]` |
  | in a `<col style="2">` | `1.00000` | `1.00000` | the column's |
  | in a `<row s="3" customFormat="1">` | `1.0000000` | `1.0000000` | the row's, over the column's |
  | in a `<row s="3">` with no `customFormat` | `1.0` | `1.0` | the row does **not** reach it |
  | `s="1"` inside either | `1` | `1` | the cell's own wins over both |

  So: **cell > row (`customFormat` only) > column > Default cell style.**

**Seat.** `XlsxStyles.DefaultFormatId` and `XlsxStyles.FormatFor`
(`src/XRay.Office.Spreadsheets/Ooxml/XlsxStyles.cs`), plus `XlsxSheetReader.ColumnStyles` and the
row's `customFormat` (`XlsxSheetReader.cs`).

**Reach**, censused over all 947 (`census-defaults.py`):

| | documents |
|---|---:|
| a cell states no `s` and the Default cell style is not `General` | **3** |
| a `<col style>` names a non-`General` format | **77** |
| a `<row s customFormat="1">` names one | **6** |
| `cellXfs[0]` and the Default cell style disagree | **0** |

That last row is why the first rule had to be probed: **no corpus document can separate
`cellXfs[0]` from the Default cell style**, so a fix guessed from the schema would have been
untestable against the corpus and would have looked right.

**Before / after.** Text-identical to 26.2.4.2 apart from one `pdftotext -layout` line-wrapping
artefact. `Net sales 60,000.00 54000 54000.000006 -6000` → `60,000.00 54,000.00 54,000.00
(6,000.00)`.

---

## 2. `065_Weight_loss_tracker_ff1c89af.xlsx` — two faults, and a third that is not ours

### 2a. `aaaa` printed literally

**What the file says.** `<numFmt numFmtId="166" formatCode="mm/dd/yy\ aaaa"/>`.

**What both binaries answer.** `Sunday`. **What we answered.** the four characters `aaaa`.

**Mechanism.** `NumberFormatSection.Parse`'s `IsDateTimeLetter` covers `y m d h s g e b` and not
`a`, so the run fell through to the per-character literal branch. `AAA`/`AAAA` and LibreOffice's
own `NN`/`NNN`/`NNNN` share one case in `svl/source/numbers/zformat.cxx`:3983-4008, and the
keyword table is `zforscan.cxx`:60-77.

Three things that table does not tell you, all measured (`codes.xlsx`, `runs.xlsx`, `nnn.xlsx`,
both binaries agreeing on every row):

- **The lengths do not pair up.** `NN` goes with `AAA` on `SHORT_DAY_NAME` and **`NNN` with
  `AAAA`** on `LONG_DAY_NAME`; only `NNNN` appends the locale's day-of-week separator (:4004).
  So `nnn` is a *long* name. Both binaries draw `Sun`, `Sunday`, `Sunday, `. This was
  implemented the obvious way first and the probe caught it.
- **The scanner is greedy from the left.** `aaaaa` draws `Sundaya` and `nnnnn` draws
  `Sunday, n` — the longest key first and the tail after it, not `aSunday`.
- **An `A` key switches the calendar and an `N` key does not.** `ImpIsOtherCalendar`
  (`zformat.cxx`:3453-3480) answers true for a subformat holding `AAA`, `AAAA`, `EC`, `EEC`,
  `R`, `RR`, `G`, `GG` or `GGG` — and for none of the `N` keys — after which
  `SwitchToOtherCalendar` (:3486-3512) renders the **month and the day** in the locale's first
  non-Gregorian calendar, leaving the year Gregorian. Under en-US that is the Jewish calendar:
  serial 44794 draws `05/24/22 Sunday` under `mm/dd/yy aaaa` and `08/21/22 Sunday` under
  `mm/dd/yy nnn`; serial 46194 — 21 June 2026 — draws `04/06/26 Sunday` and
  `Tammuz 06 2026 Sunday` under `mmmm dd yyyy aaaa`, against `06/21/26` under `mm/dd/yy` alone.
  Both binaries.

**And the same keys reach ODF, where they were inert and are not any more.**
`OdfNumberFormat` compiled `number:day-of-week` to `NNN` for a short one and `NNNN` for a long
one, which drew literal letters while the keys were unimplemented and would have drawn a long
name and a long-name-plus-comma the moment they were. It is `NN` and `NNN`:
`SvXMLNumFormatContext::AddNfKeyword` rewrites an incoming `NNNN` to `NNN` and restores the
separator only when the following `<number:text>` holds exactly it
(`xmloff/source/style/xmlnumfi.cxx`:2037-2041 and :955-970). Measured with a hand-built flat ODS
(`dow.fods`) through both binaries: short draws `Sun`, long draws `Sunday`, no trailing comma.
The corpus holds no ODF, so this is unmeasurable there; the path that reaches it is a chart
axis, since an ODF *cell* carries its display string and we use that.

**Seat.** `NumberFormatSection.DayNameRun` and `NumberFormatter.DateField`'s `'w'` case.

**Reach: 1 document.** `aaa`/`aaaa` appears in the format codes of exactly one workbook of 947,
and a bare `n` run in none.

**What is left.** The calendar switch is **not** reproduced — the day name is exact and the date
beside an `A` key stays Gregorian. Implementing a Hebrew calendar for one corpus document is not
worth it; instead a subformat carrying an `A` key reports `HasUnreproducedDirective`, which is
the mechanism the tree already uses for `[NatNum]`, `[DBNum]` and `[~buddhist]`, so a reader can
raise a diagnostic rather than presenting a guess. An `N` key does not switch the calendar and is
reproduced exactly.

**Before / after.** `08/21/22 aaaa` → `08/21/22 Sunday`. 26.2 prints `04/06/26 Sunday` on the
same cell, and the remaining distance is the calendar plus 2c below.

### 2b. an unformatted serial where a date belongs

**What the file says.** The chart is an `areaChart` + `scatterChart` combination. Its horizontal
axis is `<c:dateAx>` with `<c:numFmt formatCode="m/d" sourceLinked="1"/>` and one `<c:valAx>`.

**What we drew.** `44790 44800 44810 … 44880` along the bottom.

**What 26.2 draws.** 26 rotated labels at a three-day pitch, as glyph **outlines** — the chart's
own anisotropic scale turns a rotated string into a shear the PDF text state cannot carry, so
`pdftotext` reports nothing there. Patching the axis' `formatCode` to `d` makes them short enough
to be drawn upright as real text, and they then read as 26 clusters from x 65.1 to x 684.1 at a
24.15 pt pitch — a **date** scale, ticks every three days from 44794 to 44869.

**Mechanism.** `ChartAxes.Read` calls a chart a scatter only when `category is null && value.Count
>= 2`, which is right; but `DomainScale`/`DomainFormat` were then taken from `axes.Domain` alone,
where the axis text, the axis visibility and the labels all already take
`axes.Domain ?? axes.Category`. With one `c:valAx` there is no domain element, so the ticks along
the horizontal scale were written through `General`.

**Seat.** `DrawingChartPlot`'s `DomainFormat = FormatOf(axes.Domain ?? axes.Category)`.

**Reach: 1 document.** Of 947, **32** hold a scatter or bubble group, **4** of those state a
`c:catAx`/`c:dateAx` rather than a second `c:valAx`, and **1** gives that axis a format other than
`General`.

**Before / after.** `44790 44800 … 44880` → `8/17 8/27 9/6 … 11/15`.

**What is left.** The tick *positions* are still the numeric auto-scale's, where 26.2 uses the
date scale's three-day ticks. The scale decides where the scatter points sit and not only how a
tick reads, so it is deliberately not given the same fallback; that is a chart-scale round with
the same one-document reach.

### 2c. the dates themselves — not ours

Every date cell on that sheet is `=TODAY()-n`. The reference recalculates on load and we print
the value cached in the file, so its `04/06/26` against our `08/21/22` is the volatile-recalc
class `probes/mismatch-classify-01/results.md` already files under six documents. Untouched.

---

## 3. `Template Pilot Logbook JAR-FCL V3.0.xls` — 124 cells reading `00:00`

**What the file says.** 126 cells carry XF 156/157, whose `ifmt` is **built-in 20** and which the
file's `FORMAT` records do not define. One further cell carries built-in **14**.

**What both binaries answer.** `0:00`, `2:20`, and `11/10/2003`.
**What we answered.** `00:00`, `02:20`, and `10/11/2003`.

**Mechanism, and it is the interesting one.** `dotnet/CLAUDE.md` and `NumberFormatCodeTests`
both recorded that BIFF and OOXML *deliberately* use different built-in tables, BIFF reading
`spBuiltInFormats_DONTKNOW` (`sc/source/filter/excel/xlstyle.cxx`:819). **That is the wrong
axis.** Neither table is chosen by the file:

- `XclNumFmtBuffer::InsertBuiltinFormats` walks from `meSysLang` up through the parent tables to
  `DONTKNOW`, and `meSysLang` is `rRoot.GetSysLanguage()` —
  `sc/source/filter/inc/xlstyle.hxx`:469 calls it *"Current system language"*
  (`xlstyle.cxx`:1437-1470).
- `NumberFormatsBuffer::insertBuiltinFormats` does the same with
  `officecfg::Setup::L10N::ooSetupSystemLocale` (`numberformatsbuffer.cxx`:1865, :1919-1975).

So **both readers land on the same row, and here that row is en-US**:
`spBuiltInFormats_ENGLISH_US` (`xlstyle.cxx`:937-953) over `spBuiltInFormats_ENGLISH` (:911-919)
over `DONTKNOW`. Measured on `codes.xlsx`, seventeen built-in ids in a workbook declaring no
`<numFmt>` at all, both binaries agreeing on every one:

| id | `DONTKNOW` spells | both binaries draw |
|---:|---|---|
| 14 | `DD/MM/YYYY` | **`8/21/2022`** |
| 20 | `hh:mm` | **`2:20`**, `0:00` |
| 21 | `hh:mm:ss` | `2:20:00` |
| 22 | `DD/MM/YYYY hh:mm` | `8/21/2022 2:20` |
| 37–40 | `#,##0;-#,##0` … | **`(100)`**, `(100.00)` |

The tree already held the en-US answers — in `XlsxStyles.BuiltinCode`, a private duplicate of the
Core table. The two disagreed about the same id in the same workbook depending on which reader
opened it.

**Seat.** `XRay.Office.Core/Numbers/BuiltInNumberFormats.cs` is now the en-US table and
`XlsxStyles.BuiltinCode` delegates to it; there is one table.

**One thing the two filters genuinely do not share, and merging them broke it for an hour.**
The *locale row* is shared; the covered *set* of ids is not. Both BIFF tables say so in as many
words — `// 5...8 contained in file` (`xlstyle.cxx`:826) and `// 41...44 contained in file`
(:862) — and carry no entry for either run, while the OOXML tables state all eight
(`numberformatsbuffer.cxx`:294-320, :802). The first cut of this change gave the BIFF reader all
eight; `BuiltInNumberFormats.BiffCode` takes them back. 63-66 are built in on both sides with the
same four codes. Corpus reach of the difference is **nil** — every BIFF workbook of the 947 that
uses one of the eight writes its own `FORMAT` record for it.

**Reach.** BIFF documents using a built-in id that their `FORMAT` records do not define, censused
over all 947: **14 → 13 documents**, 15 → 2, 20 → 1, 22 → 1, 38 → 1, 40 → 1. So the day/month
transposition reaches thirteen documents and the padded hour one. Ids 5–8 and 41–44 are stated by
every corpus file that uses them, so adding them to the table is inert on the corpus and is there
for a file that omits one.

**Before / after.** `00:hh` strings 142 → **0**, against the reference's 0; `h:mm` strings 309 →
**453**, exactly the reference's 453. `PA28 10/11/2003` → `11/10/2003`, matching.

---

## 4. `053_Personal_asset_inventory_5446d84b.xlsx` and
## `070_Equipment_inventory_list_…xlsx` — the theme colour, and it is 102 documents

**What the files say.** 053's 48 pt heading is `fonts[5]`, `<color theme="4"/>`; its theme part is
`xl/theme/**theme11**.xml`, whose `clrScheme` names `accent1` = `177185`. 070's title names
`theme="9"` — `accent6` = `639FCC` against the stock `70AD47`.

**What 26.2 paints.** `#177185` and `#255172`. **What we painted.** `#4472C4` and an olive green.

**Mechanism, and it is *not* `CLAUDE.md`'s risk area 2.** No `lumMod`/`shade`/`tint` chain is
involved and nothing was mis-composed. `XlsxCellFormats` carried a **hard-coded twelve-colour
standard Office palette** for a font's `<color theme="n"/>`, with a comment saying so —
*"`theme` needs `theme1.xml`, which is not read yet"*. The correct resolver already existed in
the same library and was already reading the workbook's own theme for **fills, borders and
conditional formats**: `XlsxPalette`, with the SpreadsheetML slot order (`lt1` first, `dk1`
second — swapped against the scheme's element order) and the HSL luminance tint. Only fonts were
on the stock table. The theme part was already being loaded correctly; nothing was passing it in.

**Seat.** `XlsxCellFormats.Read` takes the theme root and resolves through `XlsxPalette`;
`ColourOf`, `ThemeColour`, `Tint` and `DefaultPalette` are deleted, so there is one colour
resolver in the library rather than two.

**Reach: 102 of 947 documents** re-theme a slot their fonts actually name (`census-theme-fonts.py`
lists each with its slot, the stock colour and the file's). 219 name a theme slot at all, and 64
keep their theme somewhere other than `theme1.xml`. **This is the largest thing in the brief and
neither of its two witnesses is a chart.**

**Before / after.** 053's heading `#4472C4` → `#177185`, the reference's exactly. 070's title →
`#255172`, the reference's exactly.

The only behaviour that could have moved beside it is `indexed` 64/65/81, which `XlsxPalette`
resolves and the old table did not: censused, **only 81 occurs** on a font, in 10 documents, and
both routes give black.

---

## 5. `053`'s value axis stepping 100k against the reference's 50k — **not a defect**

**Refused on measurement.** The chart is a **pivot** chart whose cached `c:cat`/`c:val` hold six
categories with a maximum of 250 000 and **no Grand Total**. 26.2 plots those six and steps
50 000 to 300 000. We plot a seventh category, `Grand Total` = 363 500, read from the pivot
table's output range rather than from the cache — so our data maximum is higher and the axis
follows.

The decisive instrument is one attribute: `053-grandtotal.xlsx` adds that seventh point to the
chart's own cache and changes nothing else. **26.2.4.2 then draws `$0 $100,000 $200,000 $300,000
$400,000` — our axis, exactly.** So `ChartScale` agrees with 26.2 and there is nothing to fix in
the scale.

What is left is *which categories the chart has*, which is a pivot-chart data-range question and
outside this round's lane. The page-count half of that document's gate row (4 against 2) is
almost certainly the same cause.

---

## The sheets track, re-swept

`batch-check.sh /home/user/sample-files 'sheets/*'` at `cef0cd958`, scored against the banked gate
at `2f4709c08` with `score-sheets.py`, which refuses to print unless every banked path found a
row. `sheets-after.tsv` is the result.

```
TOTAL 307   MATCH 260   MISMATCH 47   REF-CANNOT-RENDER 0
banked      MATCH 257   MISMATCH 50
```

**Four verdicts moved and none moved backwards. Two of the four are this round's.**

| document | banked | now | whose |
|---|---|---|---|
| `Template Pilot Logbook JAR-FCL V3.0.xls` | `words` | **`match`** | **this round** — defect 3 |
| `042_Business_monthly_budget…xlsx` | `words` | **`match`** | **this round** — defect 1 |
| `062_Run_chart_cb7476ea.xlsx` | `pages,words` | **`match`** | the chart-sheet round, already in this round's base |
| `057_Simple_balance_sheet…xlsx` | `pages,words` | `words` | the same |

**The banked gate is twenty commits behind this round's base, so a banked figure is not a
"before".** Fifteen further rows moved their numbers without moving a verdict, and attributing
them to this round would have been wrong for nine of the fifteen. The instrument that settles it
is a binary built at the base commit: `git checkout 0fc357beb -- dotnet/src`, build
`tools/XRay.Office.Cli` alone, render, then `git checkout HEAD -- dotnet/src`, `touch`, and
`rm -rf` the four projects' `obj`/`bin` before rebuilding.

| document | base `0fc357beb` | now | |
|---|---:|---:|---|
| `Liste-Zertifizierung-ChemKlimaschutzV-RPT.xls` | 8783 | **8638** | the reference's 8638 exactly |
| `6f9e605c-fded-11e3…xls` | 32400 | **32397** | the reference's 32397 exactly |
| `042_Business_monthly_budget…xlsx` | 1453 | **1567** | the reference's 1567 exactly |
| `UASEventsNov2014-Aug2015.xls` | 323358 | 322292 | 1066 closer to the reference's 320886 |
| `laufende-nip-vorhaben-hyland.xls` | 49179 | 49113 | +57 over the reference to −9 |
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 5504 | 5396 | verdict `words` → `match` |
| `065_Weight_loss_tracker…xlsx` | 334 | 340 | `aaaa` → `Sunday`, and the chart's date labels |
| `environment-edb…xls` | 235720 | 235512 | **see below** |
| `capa-liste-nse-1.xls` | 128298 | 128296 | **see below** |
| `List of EQS securities_0.xls` | 154907 | 154906 | the reference's count exactly |
| the other **nine** rows | — | unchanged | not this round |

**The two rows whose character count moved *away* from the reference are both improvements, and
the character count is the wrong instrument for them.** On `environment-edb…xls` the change is
`03-Jun-85` → `3-Jun-85` on 208 tokens — built-in 15, `D-MMM-YY` in en-US against `DD-MMM-YY` in
the fallback table — and **all 208 of the gained tokens appear in the reference while none of the
208 lost ones do**. `capa-liste-nse-1.xls` is the same in miniature: one `01/04/2021` becomes
`4/1/2021`, which the reference draws and the old spelling did not. Splitting a sweep diff by
*which tokens* rather than by *how many characters* is what tells those apart from a regression.

Nine of the fifteen did not move at all under this round. The clearest is
`18-02RD301_ILS_components_Master_9-13-18.xls`, which loses 626 numbers to `###` and gains 142
U+2007 against the banked gate — that is the figure-space commit `8f87b8278`, four commits before
this round's base, and reading it as a regression here would have sent a round after nothing.

## What the gate cannot see: the theme colour

The colour change moves no gate column at all — it adds no page, no word and no font. The right
instrument is the drawn colour of each text span, and the right *before* is the banked gate's own
`ours` PDF, because `XlsxCellFormats`, `XlsxPalette` and `XlsxTint` are byte-identical between
`2f4709c08` and this round's base. `colourcheck.py` counts, per document, how many drawn
characters carry a colour the 26.2.4.2 reference also uses.

| document | before | after |
|---|---:|---:|
| `061_Regional_sales_chart` | 158/546 | **546/546** |
| `089_Vintage_inventory_list` | 208/255 | **255/255** |
| `052_Manufacturing_output_chart` | 272/576 | **576/576** |
| `046_Cost_analysis_with_Pareto_chart` | 367/777 | **776/777** |
| `083_Project_tracker` | 369/1072 | **1050/1072** |
| `074_Idea_planner` | 201/643 | **624/643** |
| `050_Financial_vision` | 278/467 | **457/467** |
| `040_Blood_pressure_tracker` | 789/845 | **811/845** |
| `070_Equipment_inventory_list` | 1215/1362 | **1239/1362** |
| `053_Personal_asset_inventory` | 142/304 | **176/304** |
| `063_Sales_pipeline` | 18/191 | **62/191** |
| `058_Social_media_engagement_data` | 548/576 | 548/576 |
| **total** | **4565/7614 = 60.0 %** | **7120/7614 = 93.5 %** |

**Eleven improve, one is unchanged, none is worse.** The control that says this is ours rather
than the version gap: the two reference binaries draw **identical colour sets on 12 of 12**.

## What this contradicts in the record

1. **`dotnet/CLAUDE.md` and `NumberFormatCodeTests.TheBiffAndOoxmlBuiltInTablesDisagreeAndAreMeantTo`:
   "the one place the two readers deliberately do not share a table."** They do share one. The
   table is chosen by the *running application's* locale, not by the file or by the filter, and
   both readers resolve to en-US here. The old reading cost 126 cells on one document and
   transposed the dates of thirteen more. The test is replaced by
   `TheBiffAndOoxmlReadersShareOneBuiltInTable`, which carries the citation and the measurement.

2. **`probes/mismatch-classify-01/classification.tsv`, on `053`: "our value axis steps 100k where
   the reference steps 50k."** True as an observation and wrong as a defect: given the same
   categories 26.2 steps 100k too. The classification's own note pairs it with the colour, and the
   two have nothing to do with each other.

3. **`dotnet/CLAUDE.md` risk area 2, applied to this document.** The heading's wrong hue was not a
   `lumMod`/`shade`/`tint` chain at all — it was a palette that never consulted the theme. Worth
   saying because the brief expected the harder thing and the reach census (102 documents) would
   have justified either.

## Verification

| | |
|---|---|
| `dotnet build XRay.Office.slnx -v q -nologo` | 0 warnings, 0 errors |
| the ten non-fidelity projects, run individually and totalled by hand | **5890 passed, 0 failed, 0 skipped**, against a baseline of 5841 and 49 tests added here |
| `XRay.Office.Fidelity.Tests` | **542 passed, 10 failed, 0 skipped** of 552 — the baseline exactly, and the same four classes: `PageDrawing` x4, `TabStop` x4, `SheetDrawing`, `JustificationShrink` |
| sheets track | `TOTAL 307  MATCH 260  MISMATCH 47  REF-CANNOT-RENDER 0` |

One document, `sectors-defense-and-aerospace.xlsx`, was rendered in the nine seconds the sweep
overlapped a rebuild. Re-rendered afterwards it gives the same 449 pages and the same 139 439
alphanumeric characters as the sweep's own copy, so the sweep stands; the three corrections made
after it reach nothing in the corpus, and re-rendering all nineteen movers with the final binary
reproduces the sweep's counts on 19 of 19.

## What is left

- The Jewish-calendar month and day an `A` day-name key drags in (1 document).
- The date scale a `c:dateAx` should give a scatter group's ticks (1 document).
- `053`'s pivot-chart category set (1 document, not a formatting question).
- `XlsxSheetFormats` still takes `cellXfs[0]` as the *rendering* sheet default — fonts and
  alignment — where the number format now takes the Default cell style. No corpus document
  separates the two, and changing the rendering default is a much wider blast radius.
