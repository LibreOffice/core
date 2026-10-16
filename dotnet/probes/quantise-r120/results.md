# Round 120 — where the quantisation is, and which unit it is in

Seat `agent/quantise`, based on `90b5b2207`. Two rows of the register: **O64**, the whole
hundredth of a millimetre every rule at the reference is a multiple of, and **O65**, the one
workbook whose hyperlink cells keep their own colour.

Every arm below is confirmed twice, once in the 27.2 tree and once against the installed
26.2.4.2's own output, per `dotnet/CLAUDE.md`'s rule 1.

---

## 0. The headline

**A rule reproduces all seven of O64's integers, and it has no free parameter.** It also
reproduces **810 of 810** thicknesses on a purpose-built sweep of six faces × fifteen sizes ×
three kinds authored three times over, as a Writer, a Calc and an Impress document.

The chain has three steps and the third is the one the register was missing:

1. **The font is instantiated on the PDF writer's own reference device, 720 dpi.**
   `PDFWriterImpl` is a `VirtualDevice` whose constructor asks for `RefDevMode::PDF1`
   (`vcl/source/pdf/pdfwriter_impl.cxx`:424-428, `vcl/source/gdi/virdev.cxx`:409-411).
2. **`FontMetricData::ImplInitTextLineSize` answers a whole number of those pixels**
   (`vcl/source/font/fontmetric.cxx`:200-330). Round 118 read both of its branches and
   rejected them; what it was missing is that they answer in device pixels rather than in
   points, so `post.underlineThickness` at 10 pt is not 0.7324 pt but `ceil(150 × 100/2048)`
   = 8 *pixels*.
3. **`PDFWriterImpl::drawStraightTextLine` rounds that pixel count to a whole LOGICAL unit**
   — `nLineHeight = HCONV(nLineHeight)` at `pdfwriter_impl.cxx`:6751, where
   `HCONV` is `DevicePixelToLogicHeight`, which is `CoordinateMapper::ViewToLogicDistanceY`,
   which is `std::llround(n / fMapResolutionScale / GetDPIY())`
   (`vcl/source/outdev/CoordinateMapper.cxx`:278-283).

So a rule is **a whole number of 720 dpi pixels rounded to a whole unit of the map mode the
page is being written in**, and the register's "whole hundredth of a millimetre" is what that
comes to for a spreadsheet.

**Which unit it is has to be measured, and the source alone gets it wrong.** Reading step 3 out
of `PDFExport::ExportSelection` — `const MapMode aMapMode(MapUnit::Map100thMM)`,
`filter/source/pdf/pdfexport.cxx`:168-179 — says *1/100 mm for every module*. It is not: the
recording device's map mode is whatever the application sets while it paints, and **Writer
paints in twips**. The sweep separates them outright:

| module | whole 1/100 mm | whole twip |
|---|---:|---:|
| Calc (`.fods`) | **270 of 270** | — |
| Impress (`.fodp`) | **270 of 270** | — |
| Writer (`.fodt`) | **0 of 270** | **270 of 270** |

A twip is exactly two of these pixels, so a Writer rule is a whole tenth of a point and carries
no rounding at all; a Calc or Impress rule is a whole hundredth of a millimetre and the only
widths that exist are 4, 7, 11, 14, 18, 21, 25, 28, 32, … .

**O65 is a different mechanism from the one round 118 hypothesised, and the discriminator is the
importer.** Identical content in an `.xlsx` and in the `.xls` 26.2.4.2 itself converts it to is
drawn in two different colours: every hyperlink cell of the `.xlsx` is the application's navy
whatever the file states, and every hyperlink cell of the `.xls` is the colour the file states.
It is not the rich/plain distinction that round 118 named.

---

## 1. O64 in source, end to end

### 1.1 The two branches of `ImplInitTextLineSize`, in device pixels

`FontMetricData::ImplInitTextLineSizeHarfBuzz` (`fontmetric.cxx`:200-260) is taken unless the
family is in `Office::Common::Misc::FontsDontUseUnderlineMetrics`, which in the installed
26.2.4.2's `share/registry/main.xcd` is exactly *Liberation Serif, Liberation Sans, Liberation
Mono* (checked in that file, not only in the tree). It scales the face's `post` and `OS/2`
values by `fScale`, which `LogicalFontInstance::GetScale` computes as
`m_aFontSelData.mnHeight / upem` — and `mnHeight` is already **whole device pixels** — and
takes `std::ceil`:

```
underline        ceil(post.underlineThickness × em / upem)
double underline ceil(2/3 × the same)                     // nBSize = nSize*2; n2Size = nBSize/3
strikethrough    ceil(OS/2.yStrikeoutSize × em / upem)
```

The other branch (`:261-330`) derives everything from the device's own rounded descent, with
#i55341's clamp on the way:

```
descent  = round(descent_units × em / upem)          // ImplCalcLineSpacing's own round()
if 3×descent > ascent: descent = ascent / 3          // integer division
underline = strikethrough = ((descent × 25) + 50) / 100      // integer division, min 1
double                    = ((descent × 16) + 50) / 100      // integer division, min 1
```

`FontsUseWinMetrics` in the same registry holds four entries — Celticmd, DIN Light and two
B Nazanin — so no face in this corpus takes the Windows-metrics precedence, and only Caladea of
the six swept asks for its typographic metrics by `fsSelection` bit 7.

### 1.2 Why round 118's arithmetic missed

Its three rejections are each right about the number and wrong about the unit.

* *"Liberation Sans' `post.underlineThickness` 150/2048 is 0.7324 pt at 10 pt against a measured
  0.51"* — the face is on the blacklist, so `post` is not read at all; and had it been, the
  answer would be `ceil(150 × 100 / 2048)` = **8 pixels**, 28 hundredths of a millimetre.
* *"its descent fallback `((descent*25)+50)/100` at the 1/100 mm device gives 0.5386"* — the
  arithmetic is right and the device is wrong. At 720 dpi Liberation Sans' descent at 10 pt is
  `round(434 × 100 / 2048)` = 21 pixels, `((21×25)+50)/100` = **5 pixels**, and
  `llround(5 × 2540/720)` = **18**.
* *"`RefDevMode::PDF1` is 720 dpi, which cannot produce a whole hundredth of a millimetre at
  all"* — it is the device the *count* is in, and the hundredth of a millimetre is the unit the
  count is converted to one step later, in `drawStraightTextLine`.

### 1.3 The last step of the chain is visible in the register's own numbers

`PDFPage::appendMappedLength` writes thousandths of a point (`pdfwriter_utils.hxx`:40,
`nLog10Divisor = 3`) and `PDFPage::convert` is an integer conversion, so a thickness of *n*
hundredths of a millimetre is printed as `llround(n × 72000/2540)/1000`. Round 118 recorded five
distinct values and all five round-trip exactly, `llround` and not `trunc` on three of them:

| 1/100 mm | exact pt | printed | r118 read |
|---:|---:|---:|---:|
| 18 | 0.5102362 | 0.510 | 0.51 |
| 21 | 0.5952756 | 0.595 | 0.595 |
| 49 | 1.3889764 | **1.389** (trunc 1.388) | 1.389 |
| 28 | 0.7937008 | **0.794** (trunc 0.793) | 0.794 |
| 14 | 0.3968504 | **0.397** (trunc 0.396) | 0.397 |

---

## 2. O64 against the binary

### 2.1 The sweep

`make-sweep.py` authors one line per (face, size, kind) over **six faces × fifteen sizes ×
three kinds = 270 rows**, three times over — `sweep-fodt.fodt`, `sweep-fods.fods` and
`sweep-fodp.fodp` — with the same styles, so the module is the only variable. Flat ODF, because
each is one file and nothing in these fixtures is a shape, a frame or an automatic row height.

The faces are chosen to exercise both branches and both metric precedences: the three Liberation
faces take the descent branch (and **Liberation Mono's descent trips #i55341's clamp** — 615/2048
against an ascent of 1705/2048, so `3 × descent > ascent` at every size), while Carlito, Caladea
and DejaVu Sans take the HarfBuzz branch and Caladea is 1000 upem with `USE_TYPO_METRICS` set.

```sh
python3 make-sweep.py $OUT
for e in fodt fods fodp; do
  /opt/libreoffice26.2/program/soffice -env:UserInstallation=file://$OUT/ref1/profile \
      --headless --norestore --convert-to pdf --outdir $OUT/ref1 $OUT/sweep-$e.$e
done
python3 read-rules.py ref1/sweep-$e.pdf manifest.tsv > ref-$e.tsv
python3 score.py manifest.tsv ref-$e.tsv $e
```

**C11**: every leg was rendered **twice**, into two separate profiles, and the two readings are
byte-identical for all three modules — so nothing here rests on a single run of a binary that is
known not to be reproducible on some spreadsheets.

### 2.2 The result

**810 of 810 exact, with no free parameter.**

| leg | rows | exact |
|---|---:|---:|
| Calc, whole 1/100 mm | 270 | **270** |
| Impress, whole 1/100 mm | 270 | **270** |
| Writer, whole twips | 270 | **270** |

And the units are not interchangeable: the Writer leg scores **0 of 270** against 1/100 mm, and
the Calc leg 0 of 270 against twips.

### 2.3 The seven

The four documents behind O64's seven pairs are all spreadsheets, so all seven are on the Calc
unit. Their faces and sizes come from the files themselves — `084_Service_invoice`'s two
underlined fonts are 10 pt Arial (→ Liberation Sans), `002_Contextures`' ten `<u/>` cells are
14 pt Calibri (→ Carlito), and `TK-Syllabus`' strikethroughs are 11 pt Calibri:

| | face, size, kind | px | 1/100 mm | pt | r118 measured |
|---|---|---:|---:|---:|---:|
| 1 | Liberation Sans 10, single | 5 | **18** | 0.510 | 0.51 |
| 2 | Liberation Sans 12, single | 6 | **21** | 0.595 | 0.595 |
| 3 | Liberation Sans 10, single | 5 | **18** | 0.510 | 0.51 |
| 4 | Liberation Sans 12, single | 6 | **21** | 0.595 | 0.595 |
| 5 | Carlito 14, single | 14 | **49** | 1.389 | 1.389 |
| 6 | Carlito 11, strikethrough | 8 | **28** | 0.794 | 0.794 |
| 7 | Liberation Sans 12, double | 4 | **14** | 0.397 | 0.397 |

**Seven of seven.** Rows 5 and 6 are the strongest of them, because they are the HarfBuzz branch
and neither is a round number of anything: `ceil(194 × 140/2048)` = 14 and
`ceil(134 × 110/2048)` = 8, from the face's own `post` and `OS/2`.

*What is inference rather than measurement in that table*: rows 2 and 3 are attributed to a face
and a size by reading the invoice's `styles.xml`, and the invoice's only underlined **fonts** are
10 pt Arial — the 12 pt row is a hyperlink field on a cell whose own font is 12 pt, which round
118 recorded as "invoice p3, 12 pt" without naming the face. Any 12 pt face on the descent branch
gives 21, so the row is reproduced either way; the *face* is not established.

---

## 3. What changed

`LineSpacing.ResolveRuleWidths(face, line, size, grid)` is the whole of it: the two branches
above, on a `MetricGrid`, answering three `Length`s that are already whole logical units.
`MetricGrid.TextLine` is 720 dpi in 1/100 mm and `MetricGrid.WriterTextLine` is 720 dpi in twips;
`MetricGrid.ToPixelEm` is the em rounding the two share, factored out of `ToPixels`, `ToAdvance`
and `ToEmSize` where it was written three times.

The three drawing sites take the thickness from it and keep taking the **offsets** from
`ResolveDecorations`' design units, which are unchanged and are not this seat:

| site | grid |
|---|---|
| `SheetTextLayout.Rules` | `MetricGrid.TextLine` |
| `SlideTextLayout.Rules` | `MetricGrid.TextLine` |
| `PageDrawing.Rules` | `MetricGrid.WriterTextLine` |

`SheetTextLayout` also takes the *double* underline's own thickness, which it did not have: both
lines of a double underline were drawn at the single thickness, and both branches make a double
line thinner.

**One deliberate divergence from the C++, stated rather than defended.** HarfBuzz answers a
present-but-zero `post.underlineThickness` successfully, so the reference would take that branch
and `if (!nLineHeight) return;` would draw nothing at all; this treats a face declaring no
underline metric as one whose tables cannot be read and sends it down the descent branch, which
is what `ResolveDecorations` already did. **No installed face on this machine declares one** —
0 of 137 across `/usr/share/fonts` and the reference's own bundle have a zero
`underlineThickness` or `yStrikeoutSize` — so the divergence is unmeasurable here.

**Not changed.** The rule's *position*. `drawStraightTextLine` runs the offset through the same
`HCONV`, so it is quantised too, and it is measured as agreeing already: C16 records the two
sides' top edges within **0.02 pt** on 7449 rules of `TK-Syllabus`. Moving it would be a change
with no measurement asking for it.

---

## 4. O64 — reach, measured honestly

`sweep.py` renders the whole corpus with one binary and banks path, status, page count,
alphanumeric characters and the PDF's sha256, deleting each render as soon as the four numbers
are taken. Two legs, on **2026-09-14 UTC**, both with `SOURCE_DATE_EPOCH=1700000000`, one
temporary directory per *document*:

| leg | binary | rows | failed |
|---|---|---:|---:|
| `base.tsv` | this tree at `90b5b2207` | 947 | 0 |
| `after.tsv` | + O64 | 947 | 0 |

**368 of 947 renderings change. 0 page counts. 0 alphanumeric characters.**

| track | moved | of |
|---|---:|---:|
| words | 147 | 338 |
| slides | 133 | 302 |
| sheets | 88 | 307 |

By extension: docx 106, pptx 94, doc 41, ppt 39, xlsx 73, xls 13, xlsm 2.

**A rule thickness is ink and no gate column can see one**, which is stated rather than dressed
up: the gate's three checks are page count, alphanumeric characters and font embedding, and a
rule under a word changes none of them. The same is true of the fidelity suite —
`XRay.Office.Fidelity.Tests` is **10 failed of 552 before and 10 of 552 after, the same ten tests
by name**.

**C13 is not a hazard on this pair.** Both legs are ours, both pinned by `SOURCE_DATE_EPOCH`,
and this tree evaluates no volatile function — so the two legs are byte-comparable whatever day
they ran on, and the reference takes no part in the comparison at all. The after leg's binary was
rebuilt afterwards and reproduces one of its own renders byte for byte, which is the check that
it was not swapped mid-sweep.

### 4.1 Did it get closer

The sweep's own fixtures, scored against 26.2.4.2's readings of them:

| | exact | mean \|error\| | worst |
|---|---:|---:|---:|
| single underline and strikethrough, **before** | 74 of 540 | 0.0323 pt | 0.1090 pt |
| single underline and strikethrough, **after** | **540 of 540** | **0.0002 pt** | 0.0005 pt |
| double underline, before | 0 of 270 | 0.3545 pt | 1.4560 pt |
| double underline, after | 90 of 270 | 0.2546 pt | 1.5021 pt |

0.0002 pt is the instrument's own floor — the reference's PDF states a width in thousandths of a
point — so the single and strikethrough columns are exact to the last digit the channel carries.

**The 180 double rows that are still out are a different defect and it is not this seat.**
`PageDrawing` and `SlideTextLayout` model a run's underline as a `bool`, so a double underline is
drawn as **one line at the single thickness** in a word-processing document and on a slide; the
90 Calc rows, where `SheetUnderline` has the state, are exact. That is seated below as **O68**.

---

## 5. O65 — the colour half, and round 118's hypothesis is refuted

### 5.1 The mechanism, in source

Two hunks decide it and neither is in the file round 118 was reading.

**(a) A hard character colour beats a field's colour, by name.** `ImpEditEngine::SeekCursor`
applies each character attribute in turn and then, under the comment
*"`#i1550#` hard color attrib should win over text color from field"*, re-applies any
`EE_CHAR_COLOR` covering the field's own position
(`editeng/source/editeng/impedit3.cxx`:2947-2957). So `GetCellFieldValue`'s LINKS colour is
applied and then overwritten wherever the edit text states a colour of its own.

**(b) The two spreadsheet importers put different things in that edit text.**

* `WorksheetGlobals::insertHyperlink` — the **OOXML** path
  (`sc/source/filter/oox/worksheethelper.cxx`) — takes the cell's plain string, calls
  `rEE.Clear()`, and inserts **one bare field with no attributes at all**. The cell's rich runs
  and its stated colour are both discarded, so nothing can beat the LINKS colour.
* `lclInsertUrl` — the **BIFF** path (`sc/source/filter/excel/xicontent.cxx`:155-215) — keeps
  what the cell had. Its `CELLTYPE_EDIT` branch does `SetTextCurrentDefaults(*pEditObj)` and
  replaces the text with a field, so the run's own attributes survive; its `CELLTYPE_STRING`
  branch explicitly applies the cell pattern's item set through
  `ScPatternAttr::FillEditItemSet`, and **that puts a hard `EE_CHAR_COLOR` unless the colour is
  automatic** — `if (oColorItem->GetValue() == COL_AUTO) rEditSet.ClearItem(EE_CHAR_COLOR);`
  (`sc/source/core/data/patattr.cxx`:1196-1209). A BIFF font's automatic colour is index
  `0x7FFF` and nothing else: `XclDefaultPalette::GetDefColor` maps `EXC_COLOR_FONTAUTO` to
  `COL_AUTO` and `EXC_COLOR_WINDOWTEXT` (64) to a hard `COL_BLACK`
  (`sc/source/filter/excel/xlstyle.cxx`:140-163).

**So the discriminator is the importer, not the rich/plain distinction round 118 named.** Its
`CELLTYPE_EDIT`-branch hypothesis is right that the witness's cells are rich and wrong that
being rich is the reason: a *plain* BIFF hyperlink cell stating a colour keeps it too, and a
*rich* OOXML one does not.

### 5.2 The mechanism, against the binary

`make-linkcolour.py` authors `link-colour.xlsx` — six cells, four of them linked, covering a
plain cell with no stated colour, a plain cell stated `#FF0000`, a rich cell with a run stated
`#00B050`, a rich cell whose runs state nothing, and two unlinked controls. **26.2.4.2 itself
converts that file to `.xls`**, so both importers see content it produced from one source, and
both are rendered **twice** (C11); the two runs agree cell for cell.

| cell | linked | the file states | `.xlsx` drawn | `.xls` drawn |
|---|---|---|---|---|
| plainauto | yes | nothing (automatic) | **#000080** | #0000FF |
| plainred | yes | `#FF0000` | **#000080** | **#FF0000** |
| richgreen | yes | run `#00B050` | **#000080** | #0000FF |
| richplainrun | yes | nothing | **#000080** | #0000FF |
| barered | no | `#FF0000` | #FF0000 | #FF0000 |
| bargreen | no | run `#00B050` | #00B050 | #00B050 |

**Every hyperlink cell of the `.xlsx` is navy whatever the file says, and every hyperlink cell
of the `.xls` is what the file says.** That is (a) and (b) exactly.

**The `.xls` leg's own content is not the `.xlsx`'s, and the reason is the finding restated.**
26.2.4.2's BIFF *export* bakes the drawn colour into the cell's font, so all four hyperlink
cells of the converted file state a colour — `--convert-to fods` on it gives `ce1`
`fo:color="#0000ff"` and `ce2` `#ff0000`, and the two rich cells collapsed to `ce1` because the
OOXML *import* had already thrown their runs away. So the `.xls` leg establishes that a stated
colour wins there and **cannot**, from this file, exercise the automatic case; the source says
that case falls through to the LINKS colour and that arm is not measured.

### 5.3 Reach

`xls-linkcolour-census.py` converts every `.xls` of the sheets track with 26.2.4.2 and counts
the hyperlink cells whose text states a colour of its own. **C15 is what makes the flat export
the right instrument here rather than the wrong one**: `ScXMLExport` resolves the field with
both out-parameters null, so what it prints is the cell's *own* colour — which is exactly the
quantity that decides the question — rather than the LINKS colour the PDF draws. Automatic is
distinguishable in it: 26.2.4.2 writes `style:use-window-font-color="true"` and no `fo:color`,
which the fixture's own `plainauto` cell confirms.

**The `.ods` arm is a third answer and it is the sharpest confirmation of the mechanism.**
`link-colour-odf.fods` puts a colour on a *cell* style and on a *text span*, and 26.2.4.2 draws
the first navy and the second in the span's own colour:

| cell | where the colour is stated | drawn |
|---|---|---|
| odsplainauto | nowhere | **#000080** |
| odsplainred | the cell style | **#000080** |
| odsrichgreen | a `text:span` around the `text:a` | **#00B050** |
| odsbarered (control) | the cell style, no link | #FF0000 |
| odsbargreen (control) | a span, no link | #00B050 |

A cell-level colour reaches the EditEngine as a *default* and a span's as a **hard character
attribute**, and only the second one beats the field. So the discriminator is `#i1550`'s exactly,
and the three importers differ only in what they leave hard: SpreadsheetML nothing, ODF a span's
own colour, BIFF the pattern's too (unless automatic).

**The witness fits it without being fitted to.**
`Praktikastellen_-_chinesischsprachiger_Kulturraum.xls`'s 53 hyperlink cells are `ce14`, which
states **no colour at all**, inside a `T3` span which states **`#0000ff`** — so it is the span's
colour that survives, exactly as the ODF fixture's does, and the reference draws blue.

### 5.4 Reach

`xls-linkcolour-census.py` over the 64 `.xls` of the sheets track:

| document | hyperlink cells | stating a colour | of which non-black |
|---|---:|---:|---:|
| `AIM_OPR_LIST.xls` | 2232 | 0 | 0 |
| `7-memento-2015-transports-aeriens-b.xls` | 62 | 62 | 62 |
| `Praktikastellen_-_chinesischsprachiger_Kulturraum.xls` | 53 | 52 | 52 |
| `CSA_CCM_v1.2.xls` | 10 | 10 | 10 |
| `011_Contextures_chart_sample_599b4392.xls` | 5 | 5 | 5 |
| `014_Contextures_chart_sample_991ecfc5.xls` | 5 | 5 | **0** (hard black) |
| `capa-liste-nse-1.xls` | 2 | 2 | 2 |
| `orbus_togaf_tool_csq.xls` | 2 | 2 | 2 |
| `SIL_TDB605.xls`, `SIL_TDB609.xls` | 1 each | 1 each | 1 each |
| `RMP 2011-2014 and Inventory.xls` | 1 | 0 | 0 |

**11 of the 64 hold a hyperlink cell at all, 2374 cells; 140 of those cells state a colour, in
9 documents.** The other 2234 state none and are navy on both sides. `014_Contextures` is the
one that goes the other way — its five cells state a hard **black**, which beats the field, so
the reference draws black where this tree drew navy.

**That census is a screen and not a measurement, and one of its rows is wrong.** Its cell-to-style
join is a regex over the flat file and it mis-attributes a style where a self-closing
`table:table-cell` sits between the two — `014_Contextures`' five cells read as stating black and
are drawn `#0000FF` by both renderers. Read the sweep below for the reach and the census only for
the shape.

`.xlsx` cannot move by construction and neither can the words or slides tracks — the flag lives
on `SheetCellFormat` and only the BIFF and ODF readers ever set it — and the corpus holds no
`.ods` at all, so the ODF arm is exercised by the fixture and by `/home/user/corpus-odf` and not
by the gate.

**Measured: 8 of the 307 sheets renderings change, 0 page counts, 0 alphanumeric characters.**
The leg is `sweep.py` over the sheets track with the O64-only binary and again with this one,
`SOURCE_DATE_EPOCH` pinned, and the binary was confirmed not to have been rebuilt under either
run. The ninth census candidate, `CSA_CCM_v1.2.xls`, does not move and the reason is measured
rather than assumed: its ten hyperlink cells are not printed at all — neither rendering holds a
single `#0000ff` span.

**Did it get closer? Every one of the eight now matches the reference's own colour census.**
`o65-check.py` renders each mover with 26.2.4.2 **twice** and with this tree, and counts the
text spans by colour:

| document | navy, reference | navy, ours | other-coloured, reference | other-coloured, ours |
|---|---:|---:|---:|---:|
| `011_Contextures_chart_sample_599b4392.xls` | 0 | **0** | 15 | 15 |
| `014_Contextures_chart_sample_991ecfc5.xls` | 0 | **0** | 5 | 5 |
| `Praktikastellen_-_…_Kulturraum.xls` | 1 | **1** | 57 | 57 |
| `SIL_TDB605.xls` | 0 | **0** | 1 | 1 |
| `SIL_TDB609.xls` | 0 | **0** | 1 | 1 |
| `capa-liste-nse-1.xls` | 0 | **0** | 2 | 2 |
| `7-memento-2015-transports-aeriens-b.xls` | 0 | **0** | 21 180 | 21 178 |
| `orbus_togaf_tool_csq.xls` | 0 | **0** | 1 792 | 1 792 |

The two reference renders agree cell for cell on all eight. **Before the change every one of
those cells was navy**, because `Ink` returned the link colour for any field cell unconditionally;
round 118 measured the witness at 10 navy spans a page and 52 navy rules where the reference draws
blue. On the witness the whole-document census is now **identical**: 566 `#000000`, 54 `#0000FF`,
3 `#333333`, 1 `#000080` on both sides, and the same twice over at the reference.

**And the automatic arm is measured too, on a witness rather than on the fixture.**
`AIM_OPR_LIST.xls` holds **2232 hyperlink cells that state no colour**, and the reference and this
tree both draw **2262 navy spans** over its 229 pages — identical, twice at the reference. So all
three BIFF arms are measured: automatic → the LINKS navy, stated → the file's colour, and the
converted fixture's `plainred` → `#FF0000` where the same content as `.xlsx` is navy.

### 5.5 What changed for O65

`SheetCellFormat.ColourIsHard` — "this format's colour reaches the EditEngine as a hard character
attribute" — set by `XlsCellFormats.Resolve` and `.ApplyFont` from `font.ColourIndex != 0x7FFF`,
and by `OdsCellFormats.TextStyle` from whether the *span* states an `fo:color`. The
SpreadsheetML reader never sets it, which is the measured answer and not an omission.

`SheetTextLayout.FieldInk` answers **one colour for the whole cell**, because the reference
collapses a hyperlink cell to one field character and the attribute that decides the colour is
whichever `EE_CHAR_COLOR` survives at position zero — the *first* portion's where the cell is
rich, the cell's own where it is not. `Ink` takes that colour instead of the `bool`.

---

## 6. Tests

### 6.1 New

`XRay.Office.Text.Tests.RuleWidthTests`, twelve cases, of which two carry the whole measurement:
the Calc and Writer tables of all 90 (face, size) pairs each, asserted against the reference's
own drawn value in its own unit. Beside them: that the two grids disagree, the seven O64 pairs in
the form the register states them, that a double underline is thinner than a single one on both
branches, that the thickness *steps* rather than scaling, and that the blacklist is checked by
name.

`XRay.Office.Spreadsheets.Tests.SheetLinkColourByFormatTests`, three cases over three new fixtures
— `sheet-link-colour-ooxml.xlsx`, `sheet-link-colour-biff.xls` and `sheet-link-colour-odf.fods` —
one per importer, every expectation the reference's own drawn colour.

### 6.2 Mutation control

Five, each applied to a committed tree and then reverted with `git checkout` and a `touch`:

| mutation | caught by |
|---|---|
| drop the logical-unit rounding, scale the pixel count exactly | 8 of 12 `RuleWidthTests` |
| give the words path Calc's map unit | `UnderlineTests.TheRuleIgnoresAPostTableLibreOfficeRefusesToBelieve` |
| make a double underline as thick as a single one | 5 of 12 `RuleWidthTests` |
| ignore `ColourIsHard` — always take the link colour | the BIFF and ODF colour tests |
| make an ODF span's colour always hard | the ODF colour test |

**The second and the fifth each caught nothing on the first attempt and both are recorded because
of it.** Removing the logical-unit rounding is *invisible* on a Writer page — a twip is exactly
two 720 dpi pixels, so exact scaling and the rounding agree there — which is why the Calc table is
the discriminating one and the Writer one alone would not have been enough. And the ODF mutation
was a no-op until the fixture gained a **span that states a weight and no colour**: every other
span in it states one, so "the portion has a colour" and "the portion's colour is hard" could not
be told apart.

### 6.3 The full run, to completion

Every project individually, on the built tree, with the discovered count checked against the run
count for the one that shells out to `soffice`:

| project | failed | passed |
|---|---:|---:|
| Core | 0 | 591 |
| Text | 0 | **740** (728 before) |
| Containers | 0 | 109 |
| Vector | 0 | 309 |
| Rendering | 0 | 164 |
| Markup | 0 | 259 |
| OpenDocument | 0 | 160 |
| WordProcessing | 0 | **1940** (1939 before) |
| Spreadsheets | 0 | **1387** (1384 before) |
| Presentations | 0 | 1203 |
| Fidelity | **10** | 542 of **552 discovered, 0 skipped** |

**7404 passed, 10 failed, 7414 run.** The ten are `TabStopComparisonTests`'
`list-label-overrun` ×4, `PageDrawingComparisonTests`' `paginated` ×4,
`JustificationShrinkComparisonTests` and `SheetDrawingComparisonTests` — the families
`dotnet/CLAUDE.md` records as left failing on purpose. **The same ten by name at the round's
base**, checked by running the fidelity project against a binary built at `90b5b2207` and
diffing the failing set: identical.

---

## 7. What this round did not do

**O68 — a double underline is one line at the single thickness on a word-processing page and on a
slide.** `PageRun.IsUnderlined` and `SlideTextLayout`'s `(bool Underline, bool Strikethrough)` have
no state for it, so `w:u w:val="double"`, `\\uldb`, `style:text-underline-type="double"` and
`a:u="dbl"` all draw one rule of the wrong weight. Measured on the sweep: **180 of the 810 rows**,
mean 0.2546 pt out where the Calc path is exact, worst 1.502 pt at 48 pt. Reach not censused.
Seated as **O68**; renumbered from O66 at merge, where round 121 had already taken 66 and 67.

**The rule's *position* is quantised by the same `HCONV` and is not changed here.** It is measured
as agreeing already — C16 records the two sides' top edges within 0.02 pt over 7449 rules — so
moving it would be a change with no measurement asking for it.

**The `.fodp` and `.fodt` sweep fixtures show one thing that is not this seat and is worth a
line**: a hand-authored `.fods` with no `office:master-styles` gets Calc's default header and
footer at the reference and none here. It is a property of the fixture rather than of the corpus
— a real `.ods` always names a master page — so it is recorded and not seated.

**A blind reading was not needed and was not asked for.** Every claim here is a number read out of
a content stream, and the smallest of them — 18 hundredths of a millimetre, 0.51 pt — is under the
`dpi ≈ 216 / width_in_points` rule's threshold for a full page and would have needed a crop to
judge. The arithmetic settles it without one.
