# odt-split-r82 — a floating table is a table, and the `.odt` column 281 → 290

## Environment

Measured 2026-09-08, in the container the round ran in.

| | |
|---|---|
| Repository | `/home/user/libreoffice-core` (read-only C++ reference), worktree `/home/user/wt-odtsplit` on `agent/odtsplit` |
| Base commit | `e6be864e4` |
| Reference binary | **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2**, TDF tarball, with the duplicate/Narrow faces in `.duplicates-aside/` and the Latin Noto in `.noto-aside/` |
| Converted corpus | `/home/user/corpus-odf/words/*/odt/*.odt` — 338 documents, 26.2.4.2's own `--convert-to odt` of the words track |
| Original track | `/home/user/sample-files/words` — the same 338 in their original `.docx`/`.doc` spellings |
| Reference halves | `/home/user/gate-odf-r78/ref` (26.2.4.2, all 338 `.odt`) for the ODF column, `/home/user/gate-orig-r81/ref` (24.2.7.2, deliberately — that bank is what the original track's baseline was scored against) for the original track |
| Gate rule | `batch-check.sh`'s of 2026-09-05, reimplemented column for column in `probes/odt-resid-r77/sweep-ours.sh` |
| Workers | 2 throughout |

`/usr/bin/soffice` (24.2.7.2) was used for nothing but the original track's banked reference.

**The banked base was verified rather than assumed.** A fresh render of our half at `e6be864e4`
against `gate-odf-r78/ref` gives **281 match / 27 pages / 11 pages,words / 19 words**, which is the
brief's figure column for column. The sweep's oldest render is 05:58:28 and the binary it measured
was built at 05:56:44, so no rebuild landed inside it.

## What moved

`.odt` column, 338 documents, the same reference bytes on both sides:

| | match | pages | words | pages,words |
|---|---:|---:|---:|---:|
| before (`e6be864e4`) | **281** | 27 | 19 | 11 |
| after the cell writing mode alone | 285 | 24 | 19 | 10 |
| after both changes | **290** | 22 | 18 | 8 |

**281 → 290 of 338.** Twenty renderings differ at all; **ten gain a verdict and one loses one**.
The sum of `|page delta|` over the column goes **99 → 73** and the sum of `|glyph delta|`
**56 626 → 28 687**, which is halved.

Gained:

| document | before | after |
|---|---|---|
| `Case-Study-Heathrow-Airport` | 1/3 pages, 2159/6461 | **3/3, 6461/6461** |
| `ESPN-R - MCF - RA - Ed1` | 41/58, 42993/65952 | **58/58, 65615/65952** |
| `A1. EASA Form 2` | 9/7, 12128/11534 | **7/7, 11541/11534** |
| `AFS-050-004-F2_0i` | 7/8, 13175/13442 | **8/8, 13442/13442** |
| `part-147_approval list_20230119` | 2/2, 3161/3570 | **2/2, 3563/3570** |
| `012_Project_Timeline_Template_Black_and_Brown_Theme` | 1/2, 201/201 | **2/2, 201/201** |
| `015_Project_Timeline_Template_Colored_Background` | 1/2, 211/211 | **2/2, 211/211** |
| `045_Visual_Product_Roadmap_Template_Excellent_Format` | 2/1, 242/235 | **1/1, 242/235** |
| `047_Visual_Product_Roadmap_Template_Professional_Layout` | 2/1, 221/225 | **1/1, 224/225** |
| `33004` | 48/47, 60562/60488 | **47/47, 60522/60488** |

Lost:

| document | before | after |
|---|---|---|
| `slcc-architecture-uu-architecture` | 4/4 pages, 6328/6328 | 3/4, 6328/6328 |

`movers.txt` is the full table, the nine that move without changing verdict included.

---

## 1. A cell's writing mode: the namespace is chosen by the *value*

### The rule, at the seat

Writer's own ODF filter maps **both** spellings of the attribute onto one item. Its table-cell item
map holds `style:writing-mode` and `loext:writing-mode` side by side against `RES_FRAMEDIR`
(`sw/source/filter/xml/xmlitemm.cxx`:281-282), and the value handler takes `bt-lr` as
`SvxFrameDirection::Vertical_LR_BT` and `tb-rl90` as `Vertical_RL_TB90` whichever namespace carried
them, leaving every other value to the ordinary `XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT` handler
(`sw/source/filter/xml/xmlimpit.cxx`:1008-1030).

The *exporter* is what makes the namespace look like part of the property, and it does it **by
value**:

* `sw/source/filter/xml/xmlexpit.cxx`:193-220 writes `bt-lr` and `tb-rl90` as `loext:writing-mode`
  and lets every other value through to the ordinary path, under a comment that says exactly that.
* The generic property-set path does the same for every other item:
  `SvXMLExportPropertyMapper::_exportXML` calls `CheckExtendedNamespace`
  (`xmloff/source/style/xmlexppr.cxx`:947-952) and, when it answers 1, re-qualifies the attribute
  into `XML_NAMESPACE_LO_EXT` (:1108-1118) — *"We don't seem to have a generic mechanism to write an
  attribute in the extension namespace in case of certain attribute values only, so do this
  manually."*

**This is a sixth instance of the ODF namespace rule this project records, and the first that is a
value rather than a name.** The five already recorded (`drawooo:display`, `loext:shadow-blur`,
`chartext:coordinate-region`, `text:line-break`, `drawooo:sub-view-size`) are properties that live
in an unexpected namespace always. `writing-mode` lives in `style:` for every value ODF 1.3 defines
and in `loext:` for the two it does not — so **grepping the corpus for the attribute name finds it
and tells you nothing**. The converted `.odt` state `style:writing-mode` on 14 264 table cells, of
which 14 261 say `lr-tb`; every one of the 86 `bt-lr` cells is `loext:`. A reader that consults the
specification's spelling therefore finds every horizontal cell and no turned one.

### Measured, one cell per spelling

`gen.py` writes one table whose six cells differ in exactly the spelling and the value of
`writing-mode` on their `style:table-cell-properties`; `probe-reading.txt` is 26.2.4.2's rendering
of it, read as each line's direction vector and box.

| cell | stated | 26.2.4.2 draws |
|---|---|---|
| `c0` | nothing | `dir = (1, 0)`, upright at x 73.55 |
| `c1` | `loext:writing-mode="bt-lr"` | `dir = (0, −1)`, glyphs up, lines stacking rightwards from x 145.96 |
| `c2` | `style:writing-mode="bt-lr"` | **identical**, one 72 pt column along, from x 217.96 |
| `c3` | `style:writing-mode="tb-rl"` | `dir = (0, +1)`, glyphs down, lines stacking **leftwards** |
| `c4` | `loext:writing-mode="tb-rl90"` | identical to `c3` |
| `c5` | `style:writing-mode="tb-lr"` | `dir = (0, +1)`, glyphs down, lines stacking **rightwards** |

So the extension namespace is an **export convention and not an import requirement**: 26.2.4.2 draws
`style:writing-mode="bt-lr"` exactly as it draws the `loext:` spelling. Both are therefore read, and
resolved together — the *level* before the spelling — through
`OdfStyles.ResolveWithoutDefaults(…, candidates, out matched)`, which round 72 added for
`style:font-name` against `fo:font-family`.

`tb-lr` is a **third** direction and the enum has two. It is answered as
`TopToBottomRightToLeft`, which is right about the glyphs and about where the line breaks and wrong
only about which way a second line goes; no `.odt` of the converted corpus states it on a cell.
Upright — what LibreOffice's own DOCX importer does with the corresponding `tbLrV`, *"we can't
handle these"* — would be wrong about all three.

### The layout law was already right, and the probes reproduce it on ODF

`TurnedCellTests` established the law on DOCX in an earlier round. Two of its four facts were
re-measured here on ODF, because the ODF path had never exercised them:

* **A turned cell contributes nothing to its row's height, and a row of only turned cells collapses
  to nothing at all.** `d-alone.fodt` — one row, one `bt-lr` cell holding `Strategy` — comes back
  from 26.2.4.2 with **no text and no borders**: the table is absent from the rendering entirely.
  The C++ shape of it is `lcl_CalcMinCellHeight`'s `SwRectFnSet aRectFnSet(*_pCell)`
  (`sw/source/core/layout/tabfrm.cxx`:5009-5054), which measures a vertical cell across the axis the
  row does not grow on.
* **The line breaks at the cell's height and the stack runs across its width.** `f-rowheight-*.fodt`
  at 0.5, 1 and 2 in: `Strategy` comes back as two lines, then one, then one, and in the two tall
  cases it is drawn from the cell's **bottom** upwards, 142.45 and 214.45 — the text starts at the
  end it runs *from*.

### Reach, and whether the fix touches the sheets and slides readers

Censused over the converted corpus by spelling, value and the element the attribute sits on
(`census_wm` in this round's working notes; the numbers are reproduced in
`OdtLayoutSource.Tables.TextDirection`'s remarks):

| column | on a table cell | elsewhere |
|---|---|---|
| `.odt` (338) | **86 × `loext:…="bt-lr"` in 11 documents**, 3 × `style:…="tb-rl"` in 1 | 1 `loext:bt-lr` paragraph, 1 graphic, 1 `style:tb-rl` paragraph |
| `.ods` (307) | **none** | 1 `style:…="tb-rl"` on a paragraph |
| `.odp` (302) | **none** | 63 `loext:bt-lr` + 3 `tb-rl90` on `style:graphic-properties` in 5 and 3 documents; 414 `style:tb-rl` on paragraphs in 80 |

**So this fix reaches the `.odt` column alone, and that is a fact about where the attribute sits
rather than about which reader was changed.** `.ods` states no extension spelling anywhere and no
turned cell at all. `.odp` states 82 extension-namespace writing modes but **not one of them is on a
table cell** — they are on a *shape's* `style:graphic-properties`, which is a different object with a
different seat (`OdfFrames`/`OdpSlideLayout` feeding `SlideTextBody`, whose OOXML twin
`PptxTextBody` already models `WritingMode2::TB_RL` against `TB_RL90`). Closing the slides half is a
separate change and is left, named.

### What it was worth

**281 → 285 of 338 on its own**, four rows, eleven renderings moved and none lost: `047` (the row
frame growth cost round 75), `045`, `33004` and `A1. EASA Form 2`. The seven that keep their verdict
all hold a turned cell; `Sample_SQMS_Program` moves 85 960 → 85 730 glyphs against 86 734, which is
the one movement away from the reference and stays inside the band.

---

## 2. Fly splitting — and why the brief's account of the blocker is wrong

### What splitting is actually worth: six rows, not thirteen and not twenty-four

The brief carries round 75's *"13 further rows"* and round 77's *"24 failing rows"*. Both are
censuses of documents that **hold** a splittable fly. What matters is how many the reference
actually splits, and that is measurable at the reference with no layout reasoning at all: strip
`loext:may-break-between-pages` from the file, render the patched document through 26.2.4.2, and
compare its page and glyph counts against the reference's own rendering of the original. `unsplit.py`
does it; `unsplit.tsv` is the result.

Of the **20 failing rows that hold a splittable fly** at this round's writing-mode base, 26.2.4.2
splits **six**:

| document | reference | reference, unsplit | ours before | ours after |
|---|---:|---:|---:|---:|
| `012_Project_Timeline…Black_and_Brown` | 2p/201 | 1p/201 | **1p/201** | 2p/201 |
| `015_Project_Timeline…Colored_Background` | 2p/211 | 1p/211 | **1p/211** | 2p/211 |
| `part-147_approval list_20230119` | 2p/3570 | 2p/**3161** | **2p/3161** | 2p/3563 |
| `Case-Study-Heathrow-Airport` | 3p/6461 | 1p/**2159** | **1p/2159** | 3p/6461 |
| `OM template for non-complex NCC operators` | 165p/268604 | 165p/268550 | 166p/268604 | 166p/268658 |
| `ESPN-R - MCF - RA - Ed1` | 58p/65952 | 40p/**42875** | 41p/42993 | 58p/65615 |

**On four of the six the reference with splitting turned off reproduces this tree's own output to
the page and to the glyph**, and on `ESPN-R` to one page and 118 glyphs of 42 875. That is as clean
a statement as this corpus offers that splitting was the *whole* of the gap on those documents and
nothing else was wrong with them.

The other fourteen are documents that state the attribute on a fly that fits; they were failing for
other reasons and still are, except `AFS-050-004-F2_0i` — see below.

### The blocker the brief names does not exist: this engine already splits a floating table

The brief describes three coupled pieces, of which the third — *"pages that no block created, and
obstacles keyed by page"* — is called architectural. **It is already built, and has been since the
round that closed `Case-Study-Heathrow-Airport.docx`.**

`Paginator.PlaceFloatedTable` (`Paginator.cs`:3388) places a **positioned block table** — OOXML's
`w:tblpPr`, which LibreOffice's DOCX importer turns into a fly holding a table — carries the rows the
page cannot take in `pendingFloats`, and `ContinueFloatedTables` (`:1972`) puts them at the top of
the next page's text area. When the body's flow has run out and rows are still waiting,
`while (tables.Count > 0 && …) FinishPage();` (`:1739`) **starts pages that no block created**. Its
own remarks name `012_Project_Timeline_Template_Black_and_Brown_Theme`, whose ninth row is the whole
of the reference's page 2. The comment at `:1734` reads *"The rest of a split fly, when the flow ran
out before it did."*

So the three pieces are not three:

1. **Read `loext:may-break-between-pages`** — one attribute, and the `draw:` spelling beside it
   (`xmloff/source/text/XMLTextFrameContext.cxx`:1104-1107 reads both into `IsSplitAllowed`;
   `xmloff/source/text/txtparae.cxx`:3115-3119 writes the extension one, and only when true).
2. **Slice the flow at the deadline** — *already done*, and done at row level, because the thing
   `PlaceFloatedTable` splits is a table.
3. **Put the continuation on the following pages, creating them if need be** — *already done*.

What was missing was none of those. **It was the reader**: `OdtLayoutSource` routed the object
through the frame path, where a `PageFrame` is placed whole by `FrameResolution` and its tail is
drawn off the bottom of the sheet. The fix is to read a `draw:frame` that holds nothing but a table
and says it may break as what it is — the same object a `.docx` spells `w:tblpPr`. In the reference's
own words, `sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx`:1765: *"A text frame created
for floating tables is always allowed to split."* One object, two spellings; this tree already had
the model and the ODF reader was not using it.

`OdfFrames.FloatingTable` is the recogniser and `OdtLayoutSource.Floated` builds the positioned
table; the whole of it is about 120 lines of reader.

### Round 77's third piece is stronger than it was stated, and it is free

Round 77 added *"on the documents that actually matter the flow to be sliced **is a table**"*. That
is an understatement: censused over the whole column by `census.py`, **all 51 splittable flies in the
converted corpus hold a table and nothing else** — not one holds a paragraph. So a paragraph-flow
slicer would have reached nothing, and the row-level table splitting the engine already had is
exactly and only what the corpus needs. The `FloatingTable` recogniser therefore requires a
table-only text box and has no paragraph arm at all.

### The refusals, and the two document settings that decide the rest

`SwFlyFrame::IsFlySplitAllowed` (`sw/source/core/layout/fly.cxx`:689-737 — round 75's correction to
:730 stands) refuses for a fly that is not at-content, is in a header or footer, is in a
multi-column section, is in a footnote, or grows upwards from the body's bottom, and refuses
outright while `DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES` holds (:696-700). The reader applies
the reachable ones: the anchor must be `paragraph` or `char`, and only the body walk lifts a fly out
— a header, a footer and a cell are read by the other two walks and are left alone.

**Two document settings decide more of this column than the attribute does, and neither was read.**

* **`TabOverMargin`** picks the *deadline* a split fly is cut at. `GetFlyAnchorBottom`
  (`fly.cxx`:113-162) has two arms and `isLegacyBehavior` (:101-110) chooses between them on
  `DocumentSettingId::TAB_OVER_MARGIN` together with the fly being page-relative: the Word ≤ 2010 arm
  is the **page's** bottom, limited so the fly still fits the body's height, and the Word ≥ 2013 arm
  is the body frame's print bottom. Absent means false (`mbTabOverMargin(false)`,
  `DocumentSettingManager.cxx`:92). **177 of the 338 `.odt` state it true and 161 false.** Reading it
  as absent split the three graph-paper templates onto a second page each: `080` draws its 49th grid
  rule at 779.4 pt on a body whose print bottom is 769.9, which is the overlap the legacy arm allows
  and the modern one does not. `PaginationOptions.FliesMayOverlapTheBottomMargin` already modelled
  the rule for DOCX (`compatibilityMode < 15`); the ODT path was taking `PaginationOptions.Default`'s
  false.
* **`DoNotBreakWrappedTables`** is a document-level veto over every `may-break-between-pages` the
  file states, tested before the fly is looked at at all. **30 of the 338 state it.**
  `SwXMLImport::SetConfigurationSettings` sets the property only when they do
  (`sw/source/filter/xml/xmlimp.cxx`:1627-1630), so absent is permissive.

### Where the fly is put

Censused over the 51 (`poscensus.py`): every one is anchored to a paragraph, every one states
`svg:y` with `style:vertical-pos="from-top"`, and none states `style:run-through`. The vertical
relation is `paragraph` 29, `page` 17, `page-content` 5 — which is exactly OOXML's
`w:vertAnchor` triple and maps onto `FrameVerticalOrigin.Paragraph`, `.Page` and `.PageMargin`. The
horizontal position is `center` 30, `from-left` 20, `right` 1, and `from-left` is the only value that
carries an `svg:x` — 20 of 20 do, and 31 of 31 of the others do not.

`style:horizontal-rel="page"` (8 of the 51) measures that offset from the **sheet's own left edge**
where every other length on the way to `PageTable` is measured from the text area's. That is the
same correction `DocxLayoutSource.PositionedLeftEdge` makes for `w:horzAnchor="page"`, and it needs
one piece of page geometry the ODF reader did not carry — the section's left margin, now passed in.
`Case-Study-Heathrow-Airport` is the witness on both sides: its `.docx` states `w:tblpX="705"` and
its `.odt` `svg:x="0.4862in"`, which are the same 35.0 pt from the sheet.

### What it cost, and the one loss

Six of the ten gains are this change: `Heathrow`, `ESPN-R`, `part-147`, `012`, `015` and
`AFS-050-004-F2_0i`. **`AFS` is not a split** — the unsplit experiment says the reference does not
split it — and it comes right because the positioned-table path places the fly where the file says
while the frame path did not; that is worth knowing, because it means the reader change is worth a
row beyond the splitting it was written for.

`slcc-architecture-uu-architecture` is the loss, 4/4 → 3/4 pages at an unchanged glyph count, and it
is understood rather than merely observed. Instrumenting `PlaceFloatedTable` on it gives
`used=656.60 top=656.65 height=275.05 origin=Paragraph runsInto=True fills=True`, so the paginator's
own guard fires — *"a `w:vertAnchor="text"` fly sits where the flow already is"*, measured by the
DOCX round on this very document — and **refuses to float it**, leaving the table in the flow. In the
flow it comes to 3 pages; floated (tested by bypassing the guard) it also comes to 3; as a frame it
came to 4. So neither arm of the positioned-table model reproduces it and the frame path did, by
accident of the wrap: a frame is an obstacle and pushes the flow down, and a positioned table never
does that for a paragraph-anchored fly. `ESPN-R` states **the identical** geometry —
`svg:y="0.0008in"`, `vrel=paragraph`, `svg:x="-0.0035in"`, `hrel=paragraph` — and gains 17 pages, so
no reader-side rule separates the two. Left, named, with its instrumented numbers.

### The one accommodation, stated as one

A fly whose offset from its own anchor paragraph is **negative** starts above the paragraph it hangs
on — a place the flow has already passed — and a positioned block table is placed at
`used + VerticalOffset`, so the model would put it behind the text rather than over it. Such a fly is
left in its frame. This is a limit of the model stated where it bites rather than a rule of Writer's,
which simply draws the fly there. Measured on `HC-Bulletin-template`, whose second fly states
`svg:y="-0.078in"`: hoisted it is cut 18.9 pt above the body's bottom and the document comes to six
pages, and left in its frame it is **five against the reference's five**, glyph-exact. Two of the 51
state a negative offset against their paragraph, and the reference splits neither.

---

## What this brief got wrong, and what this round refuted

* **"That is three coupled changes… the seat is `FrameResolution.ObstaclesFor(int block)` and
  `Paginator._obstacles`, which have to become per *page*."** No part of that was needed. The
  engine has split a floating table across pages — and started pages of its own to do it — since the
  round that closed `Case-Study-Heathrow-Airport.docx`; `Paginator.cs`:1734-1739 and :1972-1999 are
  the seat, and its own comments name `012`. What was missing was a reader that recognised the ODF
  spelling of the object. **Neither obstacle keying nor page creation was touched by this round.**

* **The reach was overstated by both prior rounds, and the instrument that settles it is the
  reference itself.** Round 75 measured 13 rows by comparing held text against drawn text; round 77
  measured 24 rows that *touch* a splittable fly (this round measures 20 at its own base). The number
  that matters is **6** — the rows 26.2.4.2 actually splits — established by stripping the attribute
  and re-rendering. A document that states `may-break-between-pages` on a fly that fits is not a
  splitting document.

* **"A glyph count reads *missing* where the defect is *unsplit*" is right, and the obvious
  instrument cannot see it.** Heathrow's page-one text does reach `y = −945.6` and `ESPN-R`'s page 41
  reaches −6732.1. But **PyMuPDF's `get_text` clips to the page** — asked for text outside the sheet
  it reports a handful of descender-sized overhangs and none of this — and a content-stream scan that
  matches only `Tm` finds **one** document in 338, because Writer's PDF writer positions most text
  objects with `Td`. `tdrange.py` matches both and finds 30; `offsheet.py` and `tmrange.py` are kept
  as the record of the two wrong instruments. This round very nearly published *"round 77's
  observation no longer holds"* on the strength of the `Tm`-only scan.

* **`fly.cxx`:689-737 is right** (round 75's correction to the brief's :730 stands, re-checked line
  by line here), and so is `DomainMapperTableHandler.cxx`:1765.

* **`loext:may-break-between-pages` is on the `draw:frame` element, not on its graphic style.**
  Round 75's census phrasing — *"51 of the 59 declare `loext:may-break-between-pages="true"`"* —
  counts frames correctly but reads as a style property; it is an attribute of the element, which is
  why `OdfFrames.FloatingTable` takes it off the element and the graphic style is consulted only for
  the position.

* **`047`'s turned cells were not a frame-growth defect and the row was not lost to growth.** Round
  75 recorded the lost row as the cost of frame growth exposed by a cell-rotation defect. The cell
  reading alone closes it — 2/1 pages → **1/1**, 221 → 224 of 225 — with no change to growth at all.

* **The `bt-lr` cell reading is not an `.odp` or `.ods` fix in disguise.** The brief asks whether it
  reaches the sheets and slides readers. It does not, and the reason is not that they were skipped:
  `.ods` states no extension writing mode anywhere and no turned cell, and every one of `.odp`'s 82
  extension writing modes is on a *shape's* graphic properties rather than on a cell.

## What is left, with its seat

* **`slcc-architecture-uu-architecture`**, above: a positioned table cannot push the flow down the
  way a frame's obstacle does, and on this one document that is worth a page. Seat
  `Paginator.PlaceFloatedTable`'s `RunsIntoTheFly` guard and `FrameObstacles`.
* **A shape that "over-draws" — and the brief's framing of that group is at least partly wrong.**
  The eight `chartset` templates in the column draw 3 % to 43 % more alphanumeric characters than
  the reference by the gate's own count. Not touched here, but screened, because the screen is one
  measurement and it changes what the next round should look for: counting each rendering's text
  characters **and its glyph-sized filled paths** side by side (PyMuPDF, first page onwards) gives

  | document | ours text | ours outlined | ref text | ref outlined |
  |---|---:|---:|---:|---:|
  | `051_Organogram_Template_Basic_Theme` | 86 | 0 | 49 | **29** |
  | `055_Organogram_Template_Horizontal_Structure` | 470 | 0 | 300 | **37** |
  | `016_Project_Timeline_Template_Complete_Guide` | 653 | 28 | 625 | 32 |
  | `031_Venn_Diagram_Template_Blue_Theme` | 304 | 0 | 240 | 3 |
  | `040_Venn_Diagram_Template_Three_Circle` | 294 | 0 | 231 | 3 |
  | `011_Project_Timeline_Template_Beautiful_Theme` | 1065 | 0 | 832 | 0 |
  | `018_Project_Timeline_Template_Editable_Format` | 453 | 0 | 432 | 0 |
  | `024_Unit_Circle_Chart_Colorful_Circles` | 517 | 0 | **552** | 0 |

  On `051` the reference draws `Te` where the shape's text is `Text`, and on four of its shapes a
  bare `t` on the second line — and it **outlines 29 glyph-sized paths** that `pdftotext` cannot
  see, which is most of the 37-character "excess". That is the raster ceiling and the shear rule
  this file already records (`VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`,
  `drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141), not an autofit defect, and
  **`024` draws 35 characters *fewer* than the reference**, so the group is not one sign let alone
  one cause. The `draw:custom-shape` autofit and clip rule round 75 left is still open and the four
  documents that carry no outlines at either side — `011`, `018`, `031`, `040` — are the ones to
  work it on. Count the reference's outlines before calling one of these an over-draw.
* **`A_320.odt`, 134 pages against 118**, and the `150_5300_13_*` family with it. Untouched.
* **The slides half of the vertical writing mode**: 63 `loext:writing-mode="bt-lr"` and 3
  `tb-rl90` on `style:graphic-properties` in 5 and 3 of the 302 `.odp`, plus 414 `style:tb-rl` on
  paragraph properties in 80. `SlideTextBody`; `PptxTextBody`:273-284 is the OOXML model of the same
  distinction.
* **`tb-lr` as a third cell direction**, measured here and not modelled: 26.2.4.2 turns it clockwise
  and stacks the lines rightwards. Zero corpus reach.

## Nothing else moved

| track | rows | differing on pages, glyphs, fonts or verdict |
|---|---:|---:|
| original `.docx`/`.doc` words, against `/home/user/gate-orig-r81` | 338 | **0** |
| converted `.rtf`, against `/home/user/gate-odf-r78` | 338 | **0** |

The original track stands at 315 match / 20 pages / 2 pages,words / 1 words, which is its banked
baseline exactly, and the `.rtf` column reproduces its banked rows row for row.

That is what the code predicts. Both changes are in the ODF word-processing reader —
`OdtLayoutSource`, `OdtLayoutSource.Tables`, `OdfFrames` and the two options `OdtWordDocument` passes
— which only `odt`, `ott` and `fodt` construct; and the one shared file that was touched,
`Paginator`, was touched only by reverting the instrumentation this round added to it (`git diff`
over it is empty). The measurement is the check on that reasoning rather than a substitute for it.

## Verification

Full build `dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.**

The ten non-fidelity projects, run individually:

| project | passed | failed |
|---|---:|---:|
| Containers | 109 | 0 |
| Core | 516 | 0 |
| Markup | 259 | 0 |
| OpenDocument | 143 | 0 |
| Presentations | 1005 | 0 |
| Rendering | 164 | 0 |
| Spreadsheets | 1198 | 0 |
| Text | 727 | 0 |
| Vector | 302 | 0 |
| WordProcessing | **1787** | 0 |
| **total** | **6210** | **0** |

6194 before, plus this round's sixteen assertions — five in `OdtTurnedCellTests`, five in
`OdtFloatingTableTests` and six theory cases in `OdtFlySettingsTests`. Every other project reports
its briefed count unchanged.

`XRay.Office.Fidelity.Tests` with `/opt/libreoffice26.2/program` first on `PATH`: **542 passed, 10
failed, 0 skipped of 552** — the ten known-open, named, and no others:

```
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt   paginated.{doc,docx,fodt,rtf}
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop      list-label-overrun.{doc,docx,fodt,odt}
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt   sheet-rich-text.xlsx
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt  justify-shrink-2013.docx
```

Run twice; the same ten both times.

Fixtures and tests added:

| fixture | test |
|---|---|
| `odt-turned-cell.fodt` | `OdtTurnedCellTests` |
| `odt-floating-table.fodt`, `odt-floating-table-whole.fodt` | `OdtFloatingTableTests` |
| — | `OdtFlySettingsTests` |

Each fixture was rendered through 26.2.4.2 and its assertions are that rendering's own numbers,
quoted per assertion. `OdtFlySettingsTests` deliberately has no fixture: a minimal hand-authored flat
ODF's `office:settings` is ignored outright by 26.2.4.2, so it asserts the reader against the C++
defaults instead, and the reference's evidence for the rule is the corpus documents in §2.

## Files

| file | what it is |
|---|---|
| `gen.py` | the eleven authored flat-ODF writing-mode probes |
| `probes/*.fodt` | those probes as generated |
| `render.sh` | one reference render, in a profile keyed on a hex digest of the path, under `timeout -k` |
| `read.py`, `probe-reading.txt` | reads each probe's runs with their direction vectors; its output |
| `census.py`, `split-census.txt` | every growing frame, whether it may break, and what it holds |
| `poscensus.py` | the positioning every splittable floating table states |
| `unsplit.py`, `unsplit.tsv` | the reference rendered with the attribute stripped — what splitting is worth |
| `unsplit-control.txt` | the six the reference splits, against this tree before and after |
| `tdrange.py`, `td-base.tsv` | text drawn below the sheet, read off `Td` **and** `Tm` |
| `belowsheet.py`, `below-base.tsv` | how many glyphs of it there are, per document |
| `tmrange.py`, `offsheet.py` | the two instruments that answered wrongly; kept as the record |
| `overdraw.py`, `overdraw.tsv` | text against outlined glyphs over the eight templates the gate says we over-draw |
| `census-writingmode.py`, `census-writingmode-all.py` | the writing-mode censuses, `.odt` and then `.ods`/`.odp` |
| `rows-before.tsv` | the `.odt` column at `e6be864e4` |
| `rows-writingmode.tsv` | with the cell writing mode alone |
| `rows-after.tsv` | with both changes |
| `rows-split-firstcut.tsv` | the intermediate cut, before the two document settings were read |
| `movers.txt` | every row that moved, with its verdict either side |
