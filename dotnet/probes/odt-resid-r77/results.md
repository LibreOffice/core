# odt-resid-r77 — five ODF reader defects, and the `.odt` column 264 → 282

## Environment

Measured 2026-09-07, in the container the round ran in.

| | |
|---|---|
| Repository | `/home/user/libreoffice-core` (read-only C++ reference), worktree `/home/user/wt-odtresid` on `agent/odtresid` |
| Base commit | `584ba83f9` |
| Reference binary | **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2**, TDF tarball, with the 38 duplicate/Narrow faces in `.duplicates-aside/` and the 8 Latin Noto in `.noto-aside/` (checked: 90 faces remain, `ls LiberationSansNarrow*` empty) |
| Converted corpus | `/home/user/corpus-odf/words/*/odt/*.odt` — 338 documents, 26.2.4.2's own `--convert-to odt` of the words track; `…/rtf/*.rtf` the same 338 as RTF |
| Original track | `/home/user/sample-files/words` — 338 documents in their original `.docx`/`.doc` spellings |
| Reference halves | `/home/user/gate-odf-r76/ref` (26.2.4.2) for the ODF columns, `/home/user/gate-orig-r76/ref` (24.2.7.2, deliberately, because that bank is what the original track's baseline was scored against) for the original track |
| Gate rule | `batch-check.sh`'s of 2026-09-05, reimplemented column for column in `sweep-ours.sh` |
| Workers | 2 throughout |

`/usr/bin/soffice` (24.2.7.2) was used for nothing except the original track's banked
reference, which was taken against it.

**The banked base was verified rather than assumed.** Twelve `.odt` spanning every verdict
class were re-rendered at `584ba83f9` and scored against the same reference bytes: **12 of 12
reproduce the banked page and glyph counts exactly**, so `probes/odf-gate-r76/rows.tsv` is a
sound "before" and only our half needed rendering.

## What moved

`.odt` column, 338 documents, same reference bytes on both sides:

| | match | pages | words | pages,words |
|---|---:|---:|---:|---:|
| before (`584ba83f9`) | **264** | 28 | 32 | 14 |
| after | **282** | 26 | 19 | 11 |

**264 → 282 of 338.** Forty-four renderings differ at all; **19 gain a verdict and 1 loses
one**. The sum of `|page delta|` over the column goes **257 → 98** and the sum of
`|glyph delta|` **79 935 → 56 526**.

The two worst rows in the column are all but closed: `FAA 2025-26 Holdover Tables` **249 → 168**
against the reference's 167, and `24-25_FAA_Holdover_Tables` **225 → 153** against 155.

### Nothing else moved

| track | rows | moved |
|---|---:|---:|
| original `.docx`/`.doc` words, against `/home/user/gate-orig-r76` | 338 | **0** |
| converted `.rtf`, against `/home/user/gate-odf-r76` | 338 | **0** |

Both are compared on page count, glyph count **and** verdict, row by row, and every row is
identical. The original track stands at 315 match / 20 pages / 2 pages,words / 1 words, which is
its banked baseline exactly.

That is what the code predicts. Four of the five changes are in the ODF word-processing reader,
which no other format reaches. The fifth — nested-frame placement in `FrameResolution` — is
generic layout, which is why it was measured rather than reasoned about: on this corpus no
`.rtf`, `.docx` or `.doc` document anchors a frame inside another frame's text.

## The five rules, at the seat

### 1. `JustifyLinesWithShrinking` is an ODF document setting, and it was never read

`SwTextPortion::Format_` guesses a full justified line's break a second time with its blanks at
their minimum spacing and takes the longer guess (`sw/source/core/text/portxt.cxx`:532-562). The
DOCX spelling of the switch is `compatibilityMode` 15 and `WordParagraphFormats` has read it
since `JustificationShrink` was written; the ODF spelling is the document setting
`JustifyLinesWithShrinking`, read as `bInteropSmartJustify` at `portxt.cxx`:543-545.

`SwXMLImport::SetConfigurationSettings` (`sw/source/filter/xml/xmlimp.cxx`:1238) sets it straight
through as a document property — it is in none of that function's exclusion lists and none of its
*"use the old behaviour when the item is missing"* cases — so an absent item leaves the member at
its own default, and **that default is `false`** (`mbJustifyLinesWithShrinking = false`,
`sw/source/core/inc/DocumentSettingManager.hxx`:180). It is therefore the one compatibility flag
in `OdtLayoutSource` whose absent case is *off* rather than on.

Reach: **161 of the 338 converted `.odt` state it true**, and 51 of those also hold a justified
paragraph, which is what it takes for the flag to decide anything.

**The paragraph-level spelling is a second and independent way in, and is deliberately not read.**
`portxt.cxx`:546-562 shrinks when the document setting is on *and* the paragraph states no
word-spacing bounds of its own — its `bOldInterop`, the *"support old ODT documents"* case — *or*
when the paragraph states bounds that are not all 100%, whatever the document setting says. On the
converted corpus the second clause adds nothing: all 51 documents stating
`loext:word-spacing-minimum` state it as `75%`, which is the floor
`JustificationShrink.MinimumBlankProportion` already applies, and all 51 also state the document
setting.

Seat: `OdtLayoutSource.ShrinksJustifiedBlanks`, `OdfParagraphFormats.Resolve`.

### 2. A frame anchored inside another frame's text was placed nowhere

An anchored object belongs to the *page* however deeply its anchor is nested, which is why
`FrameResolution.Of` already walked the body's blocks, the running heads, the footnotes and every
table cell. What it did not walk was a placed frame's own content, on the reasoning — written into
the code — that the outer frame's rectangle has to exist before an inner frame can be placed. It
does; but it exists by the time the outer frame has been placed, so the walk belongs *after* that
loop rather than beside it. It is now a worklist, one nesting level per round, bounded by
`FlowLayouter.MaxNesting`.

The failure was silent in the way a missing anchored object always is: the outer frame is drawn,
filled and stroked, and only the inner shape's text is absent. LibreOffice's exporter writes a
Word text box holding a table as exactly this shape — a `draw:frame`, a `table:table` inside it,
and a `draw:custom-shape` anchored in a cell's paragraph.

Reach: **8 documents hold 857 alphanumeric characters in a shape nested inside a frame**, six of
them were failing the gate for exactly that many, and on three the loss was the document's whole
shortfall to the character — `020_Project_Timeline_Template_Modern_Theme` 109 → **316** of 316,
`012` 75 → **201** of 201, `015` 116 → **211** of 211.

Seat: `FrameResolution.Of`'s nested pass and `FrameResolution.Inside`.

### 3. `style:rel-width` and `style:rel-column-width` were both unread

A table's width and its columns can each be stated as a proportion, and neither spelling was read.

* `style:rel-width` on `style:table-properties` maps to `RES_FRM_SIZE`'s `MID_FRMSIZE_REL_WIDTH`
  (`sw/source/filter/xml/xmlitemm.cxx`:47) and becomes `SwFormatFrameSize::SetWidthPercent`,
  **clamped to 1..100** (`sw/source/filter/xml/xmlimpit.cxx`:936-949) — the same clamp OOXML's
  `w:tblW` percentage gets, so `PageTable.RelativeWidth` means one thing for both formats and was
  already wired end to end.
* `style:rel-column-width="2677*"` is the column's own proportion: `xmlimpit.cxx`:971-986 stores
  the number and marks the size type `Variable`, and `SwXMLTableColContext` passes that to
  `InsertColumn` as `bRelWidth = true` (`sw/source/filter/xml/xmltbli.cxx`:694-714).
  `TableColumnFit.ResolveOpenDocument` already ported both of `MakeTable_`'s distributions; what it
  did not have was a way to say *relative with a stated number* as against *relative because the
  file stated nothing*, which arrives as `MINLAY`. `TableColumnFit.IsRelative` is that, and it
  defaults to empty so no other reader can be changed by it.

**The percentage counts only for a table with a real horizontal orientation.**
`SwXMLTableContext::MakeTable_` reads the size in the `default:` arm of its orientation switch
alone; under `FULL` and `NONE` — `table:align="margins"` or no `table:align` at all — it sets
`m_nWidth = MAX_WIDTH` and never looks at it (`xmltbli.cxx`:2540-2582). That is the same condition
the reader already applied to a stated `style:width`, and `table:align` also had to start deciding
where a narrower table *sits*: `aXMLTableAlignMap` (`sw/source/filter/xml/xmlithlp.cxx`:307-316)
makes `center` and `right` real orientations. `left` is deliberately answered as null rather than
as `Left`, because a left-aligned table that also states an `fo:margin-left` is `LEFT_AND_WIDTH`
rather than `LEFT` (`xmltbli.cxx`:2521-2527) and keeps the margin.

Reach: **29 of the 338 `.odt` hold an oriented `style:rel-width` table** (424 such table styles),
and **2399 columns in 24 documents state `style:rel-column-width` and no `style:column-width`** —
1069 of them in `FAA 2025-26 Holdover Tables.odt` alone, whose columns therefore all came out
equal.

**The port is confirmed against the reference to within 0.2 pt on a real document.**
`AWR OPS-AOC 044`'s page 3 holds a table whose every column is proportional and whose own width is
absolute, so it exercises the *other* branch of the distribution — the one with the accumulator
that makes three equal columns come out 3:2:4. Its vertical rules, read out of both PDFs by
`rules.py`:

```
ref    136.3 203.1 204.4 223.2 268.7 295.5 302.9 333.1 345.5 369.6 380.3 397.2 421.5 436.5 440.6 465.7 471.2 490.3 548.3 575.1
after  136.3 203.1 204.4 223.2 268.7 295.2 302.8 333.0 341.1 345.4 369.6 380.2 383.4 397.1 421.5 436.4 440.5 465.6 471.1 490.1 548.3 575.1
before 180.2 204.7 239.1 253.8 290.6 327.4 342.1 376.4 401.0 421.5 445.2 462.2 471.1 474.6 548.3 575.1
```

Every one of the reference's boundaries is reproduced to a fifth of a point where before not one
of them was.

Seat: `OdtLayoutSource.Tables.RelativeWidth`, `.HorizontalPosition`, `.ColumnWidth`,
`TableColumnFit.IsRelative`.

### 4. `loext:table` is a table

ODF 1.3 does not allow a table everywhere LibreOffice can put one — inside a drawing shape's text,
most of all — so LibreOffice writes such a table in its own extension namespace and reads the two
spellings as one thing: `XMLTextImportHelper::CreateTextChildContext` falls
`case XML_ELEMENT(TABLE, XML_TABLE): case XML_ELEMENT(LO_EXT, XML_TABLE):` through to the same
`CreateTableChildContext` (`xmloff/source/text/txtimp.cxx`:1787-1795). The *attributes* stay in
`table:` — a `loext:table-cell` carries `table:style-name` — so only the element names move, which
is why `OdfNamespaces.IsTable` asks about a namespace rather than about a name.

This is the fourth instance of a rule this project already records for *attributes* — an ODF name
LibreOffice's own exporter writes is very often not in the namespace the specification puts it in —
and the first where the name is an **element**. Both the layout walk and the extraction walk
dispatched on `table:` and were equally blind.

Reach: **7 of the 338 `.odt` hold one, 14 tables in all**, and
`043_Visual_Product_Roadmap_Template_Customizable_Format` drew **41 of its 1099 characters**
without it, because every table of its roadmap diagram is a `loext:table`. Five documents gained a
verdict on this one change.

A second half fell out of writing the fixture and reaches nothing on this corpus but is the rule:
`OdfFrames.ShapeTextBody` recognised a shape as carrying text only when it held a `text:p`, `h` or
`list`, so **a shape whose whole content is a table has no text at all** as far as the reader is
concerned. It now accepts a table child in either spelling.

Seat: `OdfNamespaces.IsTable`, `OdtLayoutSource`, `OdtLayoutSource.Tables`, `OdfContentReader`,
`OdfContentReader.Tables`, `OdfFrames.ShapeTextBody`.

### 5. A dynamic header or footer is as tall as its content, not as tall as its floor

`fo:min-height` on `style:header-style` is `SwFrameSize::Minimum`, and
`SwHeadFootFrame::FormatPrt` takes `nHeight = lcl_CalcContentHeight(*this)` whenever
`!HasFixSize()`, raising it to the stated minimum only if it falls short
(`sw/source/core/layout/hffrm.cxx`:114-145). With `style:dynamic-spacing` set the frame eats the
gap below it rather than adding it, so the whole rule is

```
total = max(min-height, content + (dynamic-spacing ? 0 : gap))
```

`OdfPageGeometry` has stated that formula since round 61 and could not evaluate it: it reads the
file and nothing else, and its own remarks said the content *"cannot be had before the page the
header sits on is known"*. **That is not true of a header.** Its blocks are read by the same walk
the body's are, its width is the body's text width whatever the pagination does, and nothing about
its height depends on which page it lands on. `OdtWordDocument.Layout` now lays each master's
header and footer out once at that width and re-reads the geometry with the answer; extraction,
which must not pay for a layout, passes nothing and gets exactly the readings the file gave before.

It is not a refinement. LibreOffice's exporter writes `fo:min-height="0.0398in"` — two and a half
points — for a running head of any height, so a four-line header was given three points of room and
everything below it sat a whole header too high on every page. Reach: **68 of the 338 `.odt`
declare a dynamic header or footer whose content plainly outruns its stated floor**, 30 of them
among the round's remaining failures at the time. Seven documents gained a verdict on this change
alone and none lost one.

Two limits of the model, both deliberate and both written at the seat. The height is one number per
*section* where Writer sizes each page's header to its own content, so a document whose first-page
header differs in height from its default one is laid out on the default one's. And the growth is
refused if it would leave the body less than a line of room, which is a guard against a malformed
file rather than a rule.

Seat: `OdfPageGeometry.Read`/`.FurnitureExtent`, `OdtWordDocument.GrownTo`/`.Furnished`.

## What this brief got wrong, and what this round refuted

* **The 16 `.rtf`-exact control documents are not one question.** The brief says the ten `.odt`-long
  and six `.odt`-short documents whose `.rtf` is exact are *"the cleanest questions in the
  column"*. They are clean, but they are not comparable to each other, because **the reference
  itself paginates the two spellings differently on eleven of the sixteen**. Of the 42 pagination rows
  only **five** are the shape the phrase implies — reference agrees across both spellings, our
  `.rtf` is exact, our `.odt` is not — and they were five different causes. `BID_ACKNOWLEDGEMENT`
  is 2 pages as `.odt` and 3 as `.rtf` **in the reference**; a round that treated the two spellings
  as one document with two encodings would have chased that difference as a defect of ours.

* **The two Holdover Tables are not a mystery and share no cause with their `.rtf` twins.** They
  are `style:rel-column-width`: 1069 of `FAA 2025-26`'s 1121 column styles state a proportion and no
  length, so every one of its 115 tables came out with equal columns, every header cell wrapped a
  line too many, and the `CAUTIONS` block under each table was pushed to a page of its own — the
  82 extra pages are 82 near-empty ones, 145 to 276 glyphs each. Fixing the columns takes it to
  **168 against 167**. The opposite-sign `.rtf` behaviour the brief was careful not to build on is
  indeed unrelated: the `.rtf` column did not move by one row.

* **`WordArt_Shapes_Arrows_Catalog1` and `Case-Study-Heathrow-Airport` are not "text is missing".**
  Both draw their text; it lands *below the sheet*, where `pdftotext` cannot see it. Read out of the
  content stream, Heathrow's single page runs from `y` 827.8 down to **−945.6** on an 841.9 pt page.
  So a glyph count says *missing* where the defect is *unsplit* — the fly-splitting defect, which
  is still open. Any round working a glyph-count row on this column should read the `Tm` range
  before believing the count.

* **Fly splitting is still not the cheapest path, and the reason has changed.** The brief's
  measurement — 13 rows — stands, but 6 of the 13 were the *nested-frame* defect above and closed
  for a few lines. What is left needs the two things round 75 named (pages that no block created,
  obstacles keyed by page) *plus* a third it did not: **the flow has to be sliced, and on the
  documents that matter the flow is a table.** Heathrow's frame holds one 13-row table and nothing
  else; `012` and `015` the same. Splitting them is row-level table splitting inside a fly, not a
  `FlowLayouter.Truncated` with two bounds. Twenty-four of the failing rows still touch a
  `loext:may-break-between-pages` frame.

* **A hand-authored minimal flat ODF's `office:settings` is ignored by 26.2.4.2, and this is now
  demonstrated rather than suspected.** `CLAUDE.md` records it as a trap found once against
  `AddParaTableSpacing`. Measured again here on `JustifyLinesWithShrinking`: **forty probes over
  twenty page widths, one config item, byte-identical renderings with it `true` and `false`**. The
  same item inside a LibreOffice-written file's full `ooo:configuration-settings` set decides the
  break at **nine of thirty-six** widths. The fixtures in this round are therefore derived from a
  LibreOffice-written file with one item changed, and any future settings probe must be.

* **`OdfHeaderDynamicSpacingTests`'s second assertion was pinning an approximation and is now
  exact.** It read `add − eat > 1 cm` and its own remarks said the unflagged branch overshot the
  reference — 56.70 pt against 26.2.4.2's 41.80. With the content measured the tree draws **41.80**,
  and the two readings are 13.45 pt apart rather than 28.35, so the old assertion could not survive
  its own fix. It now asserts the reference's number.

## What is left, with its seat

* **Fly splitting**, above: 24 of the remaining failing rows hold a
  `loext:may-break-between-pages` frame, and the blocker is now stated as three pieces rather than
  two.
* **A shape that draws more text than the reference does.** Seven `chartset` templates over-draw by
  3% to 43% — `051_Organogram_Template_Basic_Theme` 86 characters against 60 — and the shapes
  carry `draw:auto-grow-height="false"` with a `fo:min-height`. That is the `draw:custom-shape`
  autofit/clip rule round 75 also left; it is worth about seven rows.
* **`A_320.odt`, 134 pages against 118**, and the `150_5300_13_*` family with it. Its tables state
  their columns as lengths, so rule 3 does not reach them; what it does is insert sixteen pages
  holding nothing but a running head. The first is page 24, whose only content is the header.
* **`AWR OPS-AOC 044`, the round's one lost row.** Its column grid now reproduces the reference's
  to a fifth of a point (above) and its pagination went 15 → 10 against the reference's 15. The
  reference alternates full pages with 80-to-170-glyph ones, which is a row splitting across a page
  break; we fit more per page and split nothing. It was a *lucky* 15/15 before — its page 2 held
  different content from the reference's page 2 at the base commit too.
* **`loext:word-spacing-minimum`**, rule 1's second clause: no reach on this corpus, cited at the
  seat.

## Verification

Full build `dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.**

The ten non-fidelity projects, run individually:

| project | passed | failed |
|---|---:|---:|
| Containers | 109 | 0 |
| Core | 516 | 0 |
| Markup | 259 | 0 |
| OpenDocument | 139 | 0 |
| Presentations | 982 | 0 |
| Rendering | 162 | 0 |
| Spreadsheets | 1175 | 0 |
| Text | 723 | 0 |
| Vector | 302 | 0 |
| WordProcessing | **1746** | 0 |
| **total** | **6113** | **0** |

6101 before, plus this round's twelve new assertions.

`XRay.Office.Fidelity.Tests` with `/opt/libreoffice26.2/program` first on `PATH`: **542 passed, 10
failed, 0 skipped of 552** — the ten known-open, named, and no others:

```
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt   paginated.{doc,docx,fodt,rtf}
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop      list-label-overrun.{doc,docx,fodt,odt}
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt   sheet-rich-text.xlsx
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt  justify-shrink-2013.docx
```

## Files

| file | what it is |
|---|---|
| `sweep-ours.sh` | renders our half of one extension of the converted corpus and scores it against a banked reference |
| `sweep-track.sh` | the same for a corpus laid out by family rather than by extension — the original words track |
| `render.sh` | one reference render, in a profile keyed on a hex digest of the path, under `timeout -k` |
| `census.py` | the five reach censuses; every figure above comes from it |
| `lines.py` | groups a PDF page's words into lines with their bounding boxes |
| `rules.py` | a page's vertical rules, which are a table's column boundaries |
| `rows-before.tsv` | the `.odt` column at `584ba83f9`, taken from `probes/odf-gate-r76/rows.tsv` |
| `rows-after.tsv` | the `.odt` column with all five changes |
| `rows-rtf-after.tsv` | the converted `.rtf` column, which does not move |
| `rows-original-after.tsv` | the original `.docx`/`.doc` words track, which does not move |

Fixtures and tests added:

| fixture | test |
|---|---|
| `odt-justify-shrink.fodt`, `odt-justify-noshrink.fodt` | `OdtJustifyShrinkTests` |
| `odt-nested-frame.fodt` | `OdtNestedFrameTests` |
| `odt-table-relative.fodt` | `OdtTableRelativeWidthTests` |
| `odt-shape-loext-table.fodt` | `OdtShapeTableTests` |
| `odt-header-grows.fodt` | `OdtHeaderGrowthTests` |

Every one of the five was rendered through 26.2.4.2 and its assertions are that rendering's own
numbers, quoted per assertion.
