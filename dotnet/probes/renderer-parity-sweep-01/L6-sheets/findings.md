# L6-sheets — findings, by root cause

24 documents; 11 root causes; 4 patches, one of which is marked **DO NOT APPLY**.

Everything below was measured in this container. Where a figure came from a probe I built, the
probe's inputs are stated so it can be rebuilt. Probe sources and outputs are in the session
scratchpad (`.../scratchpad/probe/`), not in the checkout — **nothing in
`/home/user/libreoffice-core` was modified**.

---

## RC-0 (context) · This sweep's reference is a different LibreOffice from the tree's

`soffice --version` here is **24.2.7.2**, and every `/data/bench/lo/*/out.pdf` carries producer
`LibreOffice 24.2`. `dotnet/CLAUDE.md` § *This container* records that the project moved to
**26.2.4.2** and that **16 of 171 sheets documents changed reference page count** across the
move, naming three of my documents among the largest movers:

| document | reference 24.2.7.2 | reference 26.2.4.2 | **ours** |
|---|---:|---:|---:|
| `sectors-defense-and-aerospace.xlsx` (#163) | 227 | **449** | **449** |
| `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls` (#183) | 109 | **88** | **88** |
| `grants-2005.xls` (#184) | 220 | **201** | **201** |

Three for three. **#163, #183 and #184 are version divergences, not defects.** #183 and #184
are also the two cases `cases.md` describes as "pixels match on the compared pages; only the
page count differs", which is the shape a pagination-rule change takes.

For #163 the mechanism is known and is RC-1 below. For #183 and #184 I did not isolate the
mechanism and did not try: our answer already equals the binary the tree is developed against.

**The probe that would refute this:** re-render those three references with 26.2.4.2 and check
that the page counts become 449 / 88 / 201. I cannot run it — the LibreOffice download hosts
are unreachable from here and only 24.2.7.2 is installed.

---

## RC-1 · The digit width per em is calibrated to 26.2.4.2

**Documents:** #163 `sectors-defense-and-aerospace.xlsx` — 449 pages against 227, SSIM 0.765,
the reference's page holding six questions ours does not.

**What the pages show.** `/data/bench/pairs-view/163.jpg`: the reference sets the guidance
questions in two columns (C.1…C.6 left, C.1.1…C.6.1 right); we draw only the left column. Our
449 pages is 2 × 227 less the sheets that were one column wide anyway. The text, the green and
yellow bands and the column origin are identical — measured on page 12, both sides start their
first column's text at **x = 54.822 pt**.

**What the document contains.** `xl/worksheets/sheet1.xml` opens
`<cols><col min="1" max="300" width="40" customWidth="1"/></cols>` and states **no
`pageSetup`, no `pageMargins` and no `printOptions`**, so nothing in the file selects one
column per page. `xl/styles.xml` `fonts[0]` is `<sz val="12"/><name val="Calibri"/>`.

**Where it lives in the source.** `dotnet/src/XRay.Office.Spreadsheets/Layout/SheetFonts.cs:237`

```
    private const double DigitWidthCarry = 0.57;
```

and its own remarks (`:170-236`) say the rest: the constant is "fitted, with no mechanism
behind it", the corpus constrains it to `0.5039 <= c < 0.6406`, **"This constant was 0.67, and
that was right for LibreOffice 24.2.7.2"**, and 26.2.4.2 answers 122 for Carlito 12 pt where
24.2.7.2 answers 121. Carlito 12 pt is exactly `1038/2048 × 12 × 20 = 121.6406` twips, whose
fraction `0.6406` is the boundary the two constants sit either side of.

**Measured here, on the installed 24.2.7.2**, with a one-cell workbook holding a solid red fill
and one `<col>`, rasterised at 288 dpi and the fill's width read off:

| face / size | stated width | reference (24.2.7.2) | ours |
|---|---:|---:|---:|
| Calibri 12 pt | 40 | **4840 twips** (d = 121) | **4880** (d = 122) |
| Calibri 12 pt | 80 | **9680** | **9760** |
| Calibri 11 pt | 40 | 4440 (d = 111) | 4440 |
| Arial 10 pt | 9.109375 | 1010 | 1010 |

Two columns of 4840 are 9680 twips against a printable width of 9752 (A4 less the OOXML
default 0.748 in margins) — they fit. Two of ours are 9760 — they do not. That is the whole
defect.

**Reach inside the lane.** I resolved the default font of all 19 `.xlsx` cases, computed the
exact digit twips from each face's `hmtx`, and compared the answers at `c = 0.57` and
`c = 0.67`: **#163 is the only one that differs.** Carlito 11 pt (0.5039) and Century Gothic
12 pt, Helvetica→DejaVu 10 pt, Arial 10/11 pt all land outside the window.

**The proposed change.** `patches/DO-NOT-APPLY-digit-width-carry-24.2.diff` — one constant,
`0.57` → `0.67`. **It is written for the record, not for application.** Applying it re-tunes
the tree to the older reference and would move the seven corpus workbooks whose default font is
Carlito 12 pt in the wrong direction for the binary the project develops against.

**The probe that would refute me.** The one above, inverted: render the same one-cell workbook
through 26.2.4.2 and read the fill width. If it comes back 4840 there too, the constant is
simply wrong and the patch should be applied. `dotnet/probes/sheets-r53-totalsrow/audit_colwidth.py`
is the tree's own version of this measurement.

**Confidence: certain** on the measurement and the arithmetic. What I did not establish: whether
the other six Carlito-12 workbooks in the corpus are currently passing against 26.2.4.2.

---

## RC-2 · The print range of a sheet that declares none stops at the cells

**Documents:** #001 `CSJU List of Recipients of funds 2013-2020.xlsx` (97 pages against 96,
SSIM 0.304, every page from the fifth carrying different rows) — confirmed by probe.
**Suspected** on #066 `atspp_pay_tables.xlsx`.

**What the pages show.** #001's page 5 holds a different block of recipients on each side, and
the cell rendering is faithful. The measurable difference is the **print scale**: reading the
x of column D's text against the known width of columns A+B+C (14 794 twips = 739.7 pt) on
page 1 gives **zoom 46 for the reference and 52 for us**; the row pitch agrees (6.90 pt against
7.80 pt, both being a 15 pt row at those zooms). At 52 we fit fewer rows per page — 824
extractable words against the reference's 948 — so the extra page appears early and everything
after it is displaced.

**What the document contains.** `sheetPr/pageSetUpPr/@fitToPage="1"` with
`<pageSetup scale="58" fitToHeight="11" …/>` and **no `fitToWidth`**, so both engines bisect for
the largest zoom that fits one page wide and eleven tall. `dimension` is `A7:D778`; the cells
reach column D; and the `<cols>` list continues past the data:

```
<col min="5" max="5" width="9.140625" style="1"/>
<col min="6" max="6" width="9.85546875" style="1" bestFit="1" customWidth="1"/>
<col min="7" max="16384" width="9.140625" style="1"/>
```

**The probe, run on the document itself** (copied to scratch, `xl/worksheets/sheet1.xml`'s
`<cols>` rewritten, repackaged, rendered through the installed 24.2.7.2):

| `<cols>` after column D | reference pages | reference zoom |
|---|---:|---:|
| original (E, F, G:XFD) | 96 | 46 |
| nothing | **97** | **52** — our exact output |
| E only | 97 | — |
| E + F | 96 | 46 |
| E + F + G:XFD | 96 | 46 |
| G:XFD only | 97 | — |
| E with `customWidth`, no F | 97 | 47 |
| **F widened to `width="30"`** | 96 | **42** |

The last row is the load-bearing one: 42 is what the fit-to-width arithmetic predicts if
column F's width is inside the sum. So **the reference's print range reaches column F and ours
reaches column D**, and F's width is a pagination input.

**Where it lives in the source.** `Layout/SheetLayout.cs:340` `PrintedRange` widens `UsedRange`
by `SheetDecorationArea.Extend` (cell fills and borders, Calc's attribute pass),
`SheetDrawingArea.Extend` and `SheetTextOverflow.ExtendedLastColumn`. `UsedRange`
(`SheetLayout.cs:229-259`) is built from the content tree and explicitly skips "a blank cell
carrying only a style". `SheetDecorationArea.Extend` (`:97`) returns immediately when
`formatting.IsEmpty` and otherwise walks `formatting.Cells` — **per-cell** decoration. A column
whose only claim is a `<col>` entry therefore never widens the range, and that is the gap.
This is not a read-but-never-used property: the attribute pass exists and is consumed, it is
fed the wrong evidence.

**Why there is no patch.** I could not isolate Calc's predicate. The documented mechanism is
`ScTable::GetPrintArea`'s second loop (`sc/source/core/data/table1.cxx`, `// Test attribute`
+ the `SC_COLUMNS_STOP` walk-back), but traced by hand over this file's column styles that walk
lands on column D, and the measured discriminator is the *presence of the `<col>` element* for
column F, not its `style` — a variant with the `style` attribute removed still gives 46, and a
variant with `bestFit` removed still gives 46. Writing a rule that reproduces the table above
without knowing which C++ line produces it would be exactly the sample-tuned heuristic
`AGENTS.md` forbids. The next step is a one-line instrumentation of `ScTable::GetPrintArea` —
or an `ScPrintFunc` trace — on this file, which needs a C++ build this environment must not do.

**#066 is probably the same fault in the other direction.** `atspp_pay_tables.xlsx` sheet
`Albuquerque` renders at **0.83× the reference's scale** — measured three ways that agree to
0.2 %: glyph heights (8.27 pt against 9.94), column pitch (40.80 pt against 49.07 for a
1011-twip column) and inter-row text pitch (18.83/15.09 against 22.66/18.19). The row
*structure* is identical, so this is the zoom alone. The sheet is `fitToPage="1"` with neither
`fitToWidth` nor `fitToHeight` stated (so 1 × 1) and its `dimension` is `A1:P40` while its
content ends at row 26 — 14 declared-but-empty trailing rows we count and Calc's print-area
search would not. It also states `zeroHeight="1"`, which `XlsxPrintSetup.ReadGrid` does not
read at all (`SheetGrid.cs:48` names the attribute in a comment; nothing consumes it). A
reduced probe reproducing that sheet's grid, margins and footer gave **the same answer on both
engines**, so the extra height comes from the sheet's own content and I did not isolate it.
`cases.md` describes #066 as missing borders and grey bands; at 200 dpi **both engines draw
them** — the pair image is too small to show it. The real difference is the scale.

**Confidence: high** that #001's cause is the print range's last column, on a probe run against
the document itself. **None** on the exact predicate, and **medium** that #066 is the same
fault.

---

## RC-3 · Printed row and column headings are drawn at a fixed ten point

**Documents:** #136 `Application_Compliance_Checklist_5_Apr_2021.xlsx`.

**What the pages show.** `cases.md` reads this as "X-Ray.Office prints three things the reference
leaves off a printed sheet: the column letters, the row numbers, and the cell comments". At
200 dpi that reading is wrong in an instructive way: **both engines print the headings.** The
reference's are 8 px tall and sit inside their strip; ours are 40 px tall and cover the first
three columns of the table. The heading *strip* is the same width on both sides — the vertical
rule separating it from column A is at the same x — so only the text is out.

**What the document contains.** `xl/worksheets/sheet3.xml` states `<printOptions headings="1"/>`
and `<pageSetup scale="28" …/>`. Two more of its sheets do the same at 50 and 59 per cent. The
file is right and we honour it; the size is the defect.

**Where it lives in the source.**
`Layout/SheetPageDecoration.cs:333-334` scales the strip —

```
Length headingWidth  = setup.PrintsHeadings ? HeadingWidth  * _scale : Length.Zero;
Length headingHeight = setup.PrintsHeadings ? HeadingHeight * _scale : Length.Zero;
```

— and `:863` does not scale the label:

```
    private static void Box(DocRect area, string label, IDrawingSink sink)
    {
        Outline(area, sink);
        Length size = SheetBandText.DefaultSize;      // always 10 pt
```

Calc sets the heading font on a device whose map mode is `aOffsetMode`
(`printfun.cxx:2350-2357`), and `InitModes` builds that map mode with the zoom as its scale
fraction (`:2642`), so `PrintColHdr`'s `DrawText` (`:1417`) is scaled with the cells.
`XRay.Office.Spreadsheets/TODO.md:2920` already notes that the two constants are placed unscaled
by pagination and scaled by drawing — the drawing half is the one that was not done.

**Probe, on 24.2.7.2.** A 20-row sheet with `printOptions headings="1"`:

| | reference `A` height | ours |
|---|---:|---:|
| `pageSetup scale="30"` | **3.35 pt** | **11.17 pt** |
| `pageSetup scale="100"` | 11.17 pt | 11.17 pt |

The 100 % row is the control: the two agree there, so this is not a font or a metric
difference, and it is not version-sensitive.

**The proposed change.** `patches/heading-label-scale.diff` — thread `_scale` into `Box` and
use `SheetBandText.DefaultSize * scale`.

**The probe that would refute me.** The same probe at `scale="30"` after the patch: the
reference and ours must both report 3.35 pt, and the `scale="100"` control must not move.

**Confidence: certain.** What I did not establish: whether the heading label should also take
the workbook's default cell *font* rather than `SheetBandText`'s ten-point default — Calc fills
it from a default `ScPatternAttr` (`printfun.cxx:2354-2356`), so a workbook whose default font
is not ten point may still differ. That is a separate, smaller question and this patch does not
touch it.

**Two more faults on the same page, both separate root causes:** the yellow note captions we
draw beside the repeated title row (not isolated — the sheet's four `<x:Visible/>` VML notes
are anchored at row 1, which is the repeated print title, and they appear on every page of
ours), and the missing DRAFT watermark (RC-10).

---

## RC-4 · Header and footer field codes are read case-sensitively

**Documents:** #012 `airports_6.xlsx` — the reference footer prints `Page 2`, ours prints
`Page` with the number missing.

**What the document contains.** `xl/worksheets/sheet1.xml`:

```
<headerFooter alignWithMargins="0">
  <oddHeader>&amp;CPFC Approved Locations (as of 5/31/2020)</oddHeader>
  <oddFooter>Page &amp;p</oddFooter>
</headerFooter>
```

A **lower-case `&p`**. A census of every `oddHeader`/`evenHeader`/`firstHeader` and the three
footers across all 307 sheets documents found lower-case codes in exactly one workbook — this
one — using exactly one code, `p`.

**Where it lives in the source.**
`Layout/SheetHeaderFooter.cs:388-391` takes `char code = text[at + 1]` and switches on it. Of
the seventeen arms only two — `case 'B' or 'b'` (`:530`) and `case 'I' or 'i'` (`:535`) — spell
both cases; `'P'`, `'N'`, `'D'`, `'T'`, `'A'`, `'F'`, `'Z'`, `'L'`, `'C'`, `'R'`, `'K'` do not,
so `&p` reaches `default: break` (`:545`) and is consumed silently. Calc folds the case first:

```
// ignore case of token codes
if( ('a' <= cChar) && (cChar <= 'z') ) cChar = (cChar - 'a') + 'A';
```

(`HeaderFooterParser::parse`, `sc/source/filter/oox/pagesettings.cxx:565-567`; the BIFF parser
in `xihelper.cxx` does the same.)

**Probe, on 24.2.7.2.** Two one-sheet workbooks differing only in the case of the footer codes:

| footer | reference | ours |
|---|---|---|
| `Page &P of &N` | `Page 1 of 1` | `Page 1 of 1` |
| `Page &p of &n` | `Page 1 of 1` | **`Page of`** |

**The proposed change.** `patches/header-footer-code-case.diff` — fold `a`–`z` to `A`–`Z`
before the switch, and accept either case of the `F` in Calc's `&Z&F` look-ahead, which is how
Calc spells the same test. The fold cannot disturb the non-letter arms: `&"`, `&&`, `&\n` and
the digit run that states a point size are all outside `a`–`z`, and the digit arm reads its
span out of `text` rather than out of the folded variable.

**The probe that would refute me.** The table above after the patch. A stronger one: a footer
holding `&z&f` and one holding `&k00FF00text`, which exercise the two arms that consume
following characters.

**Confidence: certain.** Reach is one document in the corpus, and it is the sharper half of
#012's defect — the other half is the row height where a cell's text wraps, which I did not
isolate (see RC-11).

---

## RC-5 · A BIFF sheet's custom-view blocks are read as the sheet's own settings

**Documents:** #109 `programs contact list as of 07-01-10.xls` — the reference heads the sheet
`PROGRAMS CONTACTS`, we head it `APF-100 PROGRAM CONTACTS`.

**What the pages show.** Every contact row, the rotated column headers, the phone numbers and
the X/A markers match line for line; the centred title above the table differs, and it is a
`&C` header rather than a cell — neither engine's *extracted text* contains either string.

**What the document contains.** The `Workbook` stream holds **one** `BOUNDSHEET`
(`CONTACTS`, offset 8998) and **six** `HEADER` records inside its single substream:

| stream offset | preceding record | text |
|---:|---|---|
| 9378 | `HORZPAGEBREAKS` | `&C&"Arial,Bold"&16 PROGRAMS CONTACTS…` |
| 26336 | `0x01AA` `USERSVIEWBEGIN` … | the same |
| 27980 | inside a view | the same |
| 32446 | inside a view | the same |
| 36852 | inside a view | `&C&"Arial,Bold"&16APF-100 PROGRAM CONTACTS…` |
| 39290 | inside a view | the same |

Five of the six sit between `USERSVIEWBEGIN` (`0x01AA`) and `USERSVIEWEND` (`0x01AB`), and each
of those blocks also repeats `FOOTER`, `SETUP`, all four margins, `HCENTER`, `VCENTER` and
`HORZPAGEBREAKS` — so this is a pagination input, not only a string.

**Where it lives in the source.** `MsBinary/XlsWorkbookReader.cs:1625` `ReadSheetRecords` walks
every record in the substream and routes it; `MsBinary/BiffRecords.cs` defines no constant for
either id and nothing anywhere in `dotnet/` mentions `0x01AA`. LibreOffice skips the block
wholesale:

```
/*  #i39464# Ignore records between USERSVIEWBEGIN and USERSVIEWEND
    completely (user specific view settings). … */
```

(`sc/source/filter/excel/read.cxx:952-966`.) Its own reader overwrites on each `HEADER`
(`XclImpPageSettings::ReadHeaderFooter`, `xipage.cxx:113-121`), so without the skip the last
view would win for it too — which is precisely what happens to us.

**Reach.** Scanning every `.xls`/`.xlt` in the corpus for `USERSVIEWBEGIN` blocks: **three
workbooks** — this one (5 views), `done-007/xls/CSA_CCM_v1.2.xls` (4) and
`done-013/xls/ECA Sinters.xls` (2). All three repeat `HEADER`, `FOOTER`, `SETUP` and the
margins inside the views.

**The proposed change.** `patches/biff-custom-views.diff` — two record constants and a
`inCustomView` flag in `ReadSheetRecords` that skips every record between them, before the
`BOF`/`EOF` depth counter sees them (as `read.cxx` does).

**The probe that would refute me.** Render #109 after the patch and read the page-1 title: it
must become `PROGRAMS CONTACTS`. And the risk to state: `CSA_CCM_v1.2.xls` and `ECA Sinters.xls`
currently pass the gate; if either passes *because* a custom view's `SETUP` happens to match
the sheet's, this will move it. Both should be re-checked in the same pass.

**Confidence: certain** on the mechanism and the file's contents; **the reach on the two other
workbooks is untested.** Note also that LibreOffice applies the skip to the whole record walk,
globals included; this patch applies it to the sheet loop only, which is where the corpus
evidence is (`USERSVIEWBEGIN` is a sheet-level record; the globals carry `USERBVIEW`, `0x01A9`).

---

## RC-6 · Cell text that spills past a horizontal page break is not drawn on the next page

**Documents:** #174 `essd-16-3433-2024-t02.xlsx` — our page 2 is completely blank
(ink ×0.00) where the reference draws the middle of an oversized table.

**What the pages show.** The reference's page 2 is the second *column band* of a sheet whose
cells are far wider than the page: every line on it is the continuation of a string that starts
in a column on page 1. Ours is white.

**Where it lives in the source.** This is already diagnosed, in the module's own TODO
(`dotnet/src/XRay.Office.Spreadsheets/TODO.md`, the item beginning "**A cell's overflow stops at
a horizontal page break**"), measured on `xls-features.xls`: page 4 is "3 words in ours against
1011 in the reference". `SpreadsheetPages.DrawCell` is driven by the placed columns of the
page, so a cell in column A is drawn only on the page whose column band contains A;
`SheetTextContext` already measures the spill against the document grid, which is why the
clipped string on the *first* band is right. The missing half is Calc's lead-in —
`ScOutputData::LayoutStringsImpl` walks back over the left neighbours before deciding an output
area (`sc/source/ui/view/output2.cxx:1595-2290`). `SheetEmptyPages.IsPrintEmpty`'s
`ReachedFromTheLeft` test is why the page is kept at all, which is consistent.

**No patch.** The change is a per-page lead-in of the columns left of the band, drawn for their
overflow alone, in `SpreadsheetPages` — a real piece of work that should be built and tested
rather than written blind.

**Confidence: certain** (the diagnosis is the project's own, and this document matches its
signature exactly).

---

## RC-7 · Volatile date formulas print their cached value

**Documents:** #146 `065_Weight_loss_tracker_ff1c89af.xlsx`, #187 `062_Run_chart_cb7476ea.xlsx`.

**What the documents contain.** #146's `xl/worksheets/sheet12.xml` holds
`<f>TODAY()-77</f><v>44794</v>`, `TODAY()-70 / 44801`, `TODAY()-63 / 44808` … and sheet21 holds
`IFERROR(IF(LEN(…)=0,TODAY(),…),TODAY())`. #187 holds three `TODAY()`. The cached `<v>`s are
from 2022 and 2023; the reference prints recalculated dates.

**Why this is the reference behaving correctly.** Excel and LibreOffice both recalculate
*volatile* functions when a workbook is opened, whatever the recalculation setting; the cached
value is a hint for a reader that cannot calculate. So the reference is right and we are wrong.

**The trap the lane brief flags, resolved.** This is **not** the `SOURCE_DATE_EPOCH` question.
That one is about `&D`/`&T` header fields, where `xray-office render` honours the
reproducible-builds convention and `batch-check.sh` renders the reference unpinned — a genuine
asymmetry, and the reason `dotnet/CLAUDE.md` says a stored verdict on a date-bearing sheet goes
stale on its own. Here the divergence is in *cell* values driven by a formula, and it would be
there on any day: 44794 is 2022-08-21 and no epoch makes it today. If the volatile functions
were ever evaluated, `SOURCE_DATE_EPOCH` would be the correct clock to seed `TODAY()` from.

**No patch.** Evaluating `TODAY()-77` needs a formula evaluator, and `XlsWorkbookReader`'s own
header says "Nothing here decodes formula tokens". Out of scope for this round; worth recording
as the reason two chart-set workbooks cannot match.

**Confidence: certain** on the cause.

---

## RC-8 · The `AAA`/`AAAA` day-of-week format codes are not recognised

**Documents:** #146 — the reference prints `Tuesday`, we print the literal `aaaa`.

**What the document contains.** `xl/styles.xml`:
`<numFmt numFmtId="166" formatCode="mm/dd/yy\ aaaa"/>`.

**Where it lives in the source.** `dotnet/src/XRay.Office.Core/Numbers/NumberFormatter.cs:790-797`
handles the `d`/`dd`/`ddd`/`dddd` family and nothing else names a day; there is no `a` arm, so
the run is emitted as a literal. LibreOffice's scanner has `NF_KEY_AAA` and `NF_KEY_AAAA`
beside `NNN`/`NNNN` in its date-keyword set (`svl/source/numbers/zforscan.cxx:75-77` and the
three date-keyword switches at `:1345`, `:2514`, `:2835`), so `AAA` is the short day name and
`AAAA` the long one.

**Cross-lane.** The fix is in `XRay.Office.Core/Numbers`, which this lane does not own — see the
section at the end.

**Confidence: certain** on cause and location.

---

## RC-9 · A pivot table's output range gets none of Calc's pivot cell styles

**Documents:** #098 `alle einzeln.xlsx` — ink ×0.34, SSIM 0.638.

**What the pages show.** At 200 dpi the reference draws a complete ruled grid around every cell
of the three-column name list, and a heavy vertical bar down the left margin at the print
margin. We draw the same text with no rule anywhere. Every name, number and column position is
correct.

**What the document contains.** `xl/styles.xml` has `<borders count="1">` and that one border
is empty on all four sides; no `cellXfs` entry references anything else. **So the rules are not
in the cell formats and could not have been dropped by the format reader** — the sheet is a
pivot table (`xl/pivotTables/pivotTable1.xml`, `<pivotTableStyleInfo name="PivotStyleLight16"
showRowHeaders="1" showColHeaders="1" …/>`, and `cellXfs` carries a `pivotButton="1"` entry).

**Where it lives.** LibreOffice does not honour `PivotStyleLight16`; it re-renders the
DataPilot with its own built-in cell styles, applying `STR_PIVOT_STYLENAME_TITLE`,
`_CATEGORY`, `_FIELDNAME`, `_RESULT`, `_TOP` and `_INNER` over the output range and
`lcl_SetFrame` around parts of it (`sc/source/core/data/dpoutput.cxx:783-790, 811, 825, 998,
1052, 1245-1246`). Those built-in styles carry the box border that draws the grid. Nothing in
`XRay.Office.Spreadsheets` reads `xl/pivotTables/` for OOXML at all (`MsBinary/XlsPivotCache.cs`
is the BIFF pivot-cache reader and is a different thing).

**No patch.** The scoped version — take `pivotTableDefinition/location/@ref` and apply a box
border over it — would be a guess at which of the six styles covers which sub-range, and would
be wrong on any pivot with column fields. This needs the output geometry.

**Confidence: high** on the cause (the borders are provably not in the file's own formats, and
LibreOffice provably synthesises them); **low** on the size of the work.

---

## RC-10 · A sheet's `<picture>` background is not drawn

**Documents:** #136 (the tiled DRAFT watermark under the whole page).

**What the document contains.** `xl/worksheets/sheet3.xml` ends
`<drawing r:id="rId2"/><legacyDrawing r:id="rId3"/><picture r:id="rId4"/>`, and `rId4` resolves
to `xl/media/image1.png`. `<picture>` is the sheet background.

**Where it lives.** LibreOffice imports it and prints it tiled —
`aPropMap.setProperty(PROP_BackGraphic, rModel.mxGraphic); aPropMap.setProperty(
PROP_BackGraphicLocation, css::style::GraphicLocation_TILED);`
(`sc/source/filter/oox/pagesettings.cxx:991-995`). `git grep '"picture"'` over
`dotnet/src/XRay.Office.Spreadsheets` and `dotnet/src/XRay.Office.Ooxml` returns nothing.
`XRay.Office.Spreadsheets/TODO.md` already lists "**A page's own background and border**"
(`ATTR_PAGE_BACKGROUND`) as unread by all three readers.

**Reach.** Two corpus workbooks state a sheet `<picture>`: this one (4 sheets) and
`chartset-013/xlsx/019_Free_Blood_Sugar_Chart…xlsx`. Separately, three workbooks use
`legacyDrawingHF` (a header/footer picture, a different mechanism):
`done-004/xlsx/UAE Type Accepted Aircraft Models.xlsx`,
`done-009/xlsx/PBN Matrix NAAs (V01).xlsx` (23 sheets) and this one.

**No patch.** It needs a raster drawn behind the whole printed block, tiled, before every other
layer — a new drawing pass in `SheetPageDecoration`, plus the reading in `XlsxPrintSetup`.

**Confidence: certain** on cause; untested on how the tile origin is placed.

---

## RC-11 · The residue: marginal drift I did not isolate

Twelve documents whose difference is small, diffuse, or not reducible to any of the above.
Recording them honestly rather than attaching them to a cause they may not share.

- **#082, #105, #127, #134, #144, #170** — column widths a fraction narrower or wider than the
  reference's, line spacing looser, the block shifted a few millimetres. Every value, row order
  and footer matches. These are the shape of the advance-width divergence `dotnet/CLAUDE.md`
  records as a known open defect with a known seat (grid-fitted against unhinted advances), and
  I did not re-derive it.
- **#131, #135** — the two lease-return checklists. Both engines overrun rows whose fixed height
  is smaller than their text; the collisions land on different rows because the line breaks
  differ. Fills, codes and section bands correct.
- **#180 `FY2023-AIP-grants.xlsx`** — `cases.md` reads this as the title rows colliding with the
  table header "where the reference keeps them on separate lines". At 220 dpi **both engines
  collide**: the sheet's `oddHeader` holds *three* lines
  (`&CFY-2023 …\nCumulative …\nAs of 10/20/2023`) in a band of `top - header` = 0.45 in, and
  both clip the third line across the table's header row. The reference clips it about **2 pt
  higher** than we do. A band-clip depth difference, not a collision; not isolated.
- **#177 `Hazard Analysis Template.xls`** — the reference sets every hazard-field description
  bold and we set them regular; the centred `&C&"Arial,Bold"&12Hazard Analysis Template` page
  header is also nearly absent from ours. The obvious BIFF trap is **refuted**: the hole at
  `FONT` index 4 is handled (`MsBinary/XlsCellFormats.cs:87-98` duplicates the entry so the XF's
  `ifnt` lines up), and the workbook's only bold face is `FONT` record 4 = `ifnt` 5, which seven
  XFs use. Cause not isolated.
- **#153 `SIL_TDB648.xlsx`** — the diagonal Honeywell watermark is two `<xdr:pic>` elements in
  `xl/drawings/drawing1.xml` (`xl/media/image1.jpeg`) inside `twoCellAnchor`s, not a sheet
  `<picture>` and not a `legacyDrawingHF`; we draw neither. Also one extra row fitted per page
  and light cell borders where the reference draws none. Not isolated.
- **#189 `7-memento-2015-transports-aeriens-b.xls`** — 190 pages against 191, compared pages
  pixel-identical. A single page-boundary decision; not isolated. Worth noting that
  `dotnet/CLAUDE.md` records a *different* document (`ans_mappings_of_eccairs_terms.xlsx`)
  rendering 191 pages eight times and 190 once through the reference itself, so a one-page
  difference at this size is within the reference's own instability for at least one workbook.

**On the lane brief's second cluster.** `SheetGrid.IsOptimalSize` is **not** a read-but-never-
used property any more: `Layout/SheetOptimalRowHeights.cs:257, 261, 300` consumes it and
`SheetGrid.cs:119` is its accessor. #012's row-height half is therefore *not* "the property is
never read" — 262 of `airports_6.xlsx`'s 1024 rows state `ht` without `customHeight` and are
recomputed by that code. Whether the recomputation wraps the cell the way Calc does I did not
establish.

---

## Cross-lane dependencies

- **`dotnet/src/XRay.Office.Core/Numbers/NumberFormatter.cs`** — RC-8. The date-format renderer
  needs `AAA` and `AAAA` arms beside its `ddd`/`dddd` arms (short and long day-of-week names),
  matching `NF_KEY_AAA`/`NF_KEY_AAAA` in `svl/source/numbers/zforscan.cxx:75-77`. The format
  code that exercises it is `mm/dd/yy\ aaaa` in
  `sheets/chartset-010/xlsx/065_Weight_loss_tracker_ff1c89af.xlsx`. Whatever parses the format
  code into sections (`NumberFormatCode.cs`) probably needs the letter added to its date-token
  set at the same time.

No other lane's files are needed. RC-9 and RC-10 would both be new code inside
`XRay.Office.Spreadsheets`, and RC-2 and RC-6 are changes to files this lane owns.

---

## What I did not do

- I did not build, test or modify anything in `/home/user/libreoffice-core`. The four patches
  were verified only with `git apply --check`; none has been compiled.
- I did not re-render any corpus reference. The four probe workbooks I rendered through
  `soffice` are ones I constructed, plus two rewritten *copies* of `CSJU …xlsx` and its
  `<cols>` element, made in the scratchpad.
- I did not establish the mechanism behind #183 and #184 beyond identifying them as the
  documented reference-version movers.
