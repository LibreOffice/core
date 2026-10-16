# slides-bullet2-r112 — a `.ppt` table's cells, and the ruler that was never applied

Round 112, slides track, seat `agent/slidebullet2`, three questions handed over by round 111:
**O48** (a `.ppt` table's cells autofitted one by one), **O46**'s residual (`France` 16's label
width, and which font height sizes a picture bullet's box), and the one page of **O15** still
seated on this track.

Four results.

1. **A `.ppt` table's cells never autofit, and this tree shrank 358 characters of one.**
   `CreateTable` copies each rectangle's `OutlinerParaObject` into a cell and *nothing else of its
   item set* — `ApplyCellAttributes` carries the text distances, the two adjusts, the writing mode
   and the fill, and `SDRATTR_TEXT_FITTOSIZE` is not among them. `Thailand17` page 11 now draws
   **548 alphanumerics at 11.99 pt against the reference's 548**, exactly. **O48 closes.**
2. **A shape whose text is an `OutlineTextRefAtom` keeps its own `TextRulerAtom`, and this tree
   dropped it.** The reference remembers the ruler's file offset *before* it patches the client
   textbox header over to the referenced text. **Every ruler in the two decks that decides is in
   that position** — 54 of 54 in the France deck and 3 of 3 in Part-M — and over the corpus's 51
   `.ppt`, 121 rulers in 17 documents were never applied. It closes `France` 16's label width
   (70.09 → **85.09** against 85.10) *and*, through `nHardCount`, the picture-bullet size question.
3. **The two decks' bullet sizes are separated by `nHardCount` after all, and round 111's reading
   of it was one bit out.** `PPT_ParaAttr_BulletOn` is bit zero and is the first of the seven; the
   `0x1801` masks round 111 traced state it. But the mask is not what fires here — the **ruler**
   is, because `ReadParaProps`' tail sets `PPT_ParaAttr_TextOfs`'s mask bit when the shape's ruler
   speaks for a level. With the ruler read, France's paragraphs are hard and take their portion's
   height (17.86 pt) and Part-M's page-21 paragraphs are soft and keep their master's (31.24 and
   19.02 pt), which is the reference on both. **`Part-M` 21 is recovered.**
4. **`France` 14 was not a layout page and O15's residual now holds no live seat.** Round 111
   classified it LAYOUT because both readers hand this tree 20.01 on inputs the reference reads as
   18.99 — and the reason the flat-ODP leg agreed with the `.ppt` leg is that the ODP carries the
   ruler's own `text:min-label-width` and this tree was ignoring the ruler in the `.ppt` and
   *mis-fitting* the ODP for the same reason the `.ppt` was wrong. It is 18.99 at head.

O15's registered statistic goes **10 pages → 6, 52.12 → 47.11 pt, 4 fixed and 0 newly wrong**, and
the six that remain are the raster ceiling (3), the seventh confound (1) and the two pages the
instrument cannot reach — every one of them already classified *not ours* or *terminal* by round
111.

Reach: over the whole slide corpus at both legs — 51 `.ppt`, 251 `.pptx`, 302 `.odp`, 604
documents, 0 failures, `SOURCE_DATE_EPOCH` pinned — **20 renderings change and 584 are
byte-identical**, all twenty `.ppt`, **every one of the twenty predicted in advance by a record
census**, and **0 page counts and 0 alphanumeric counts move anywhere**.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidebullet2`, `agent/slidebullet2`, base **`4f4d0ddc8`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used only by `XRay.Office.Fidelity.Tests`, as it always is |
| reference bank | `/home/user/gate-orig-r83/ref`, plus two fresh renders of `Thailand17` for the C11 check below |
| corpora | `/home/user/sample-files` (51 `.ppt`, 251 `.pptx`) and `/home/user/corpus-odf` (302 `.odp`) |
| working directory | `/home/user/r112-work` |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is **not** the reference binary's source |
| fonts | the five tarball confounds were already aside — `.duplicates-aside`, `.noto-aside`, `.condensed-aside` all present |

**Both legs, as this track requires.** Every `file:line` below is the `27.2.0.0.alpha0+` checkout
and is the *explanation*; the evidence is 26.2.4.2's own output — its flat ODP of three decks, its
PDFs, and its banked renderings of the corpus.

**Confound C11.** `Thailand17` 11 and `France` 16 are single-page claims, so the reference was
rendered **twice** for `Thailand17` and both fresh renders agree with the bank on the dominant size
of all 54 pages (`pagesizes.py` prints no differing row across the three). The two PDFs differ in
md5 and are the same length — that is the XMP date.

---

# 1. O48 — a `.ppt` table's cells are autofitted one by one

## 1.1 What the page actually shows

Round 111 left this as *"which cells the fit reaches and how far"*, which is right and is
answerable exactly. Page 11's spans, read out of both PDFs:

| | spans | sizes |
|---|---:|---|
| 26.2.4.2 | 78 | 77 at 11.99, 1 at 24.01 |
| this tree, base | 73 | 37 at 11.99, **35 at 11.00**, 1 at 24.01 |

Sorted by position, the 35 are exactly the spans of the cells whose text **wraps to two lines** —
columns 1, 5 and 6 — and the 37 are the single-line cells of columns 2, 3 and 4. The line pitch
inside a wrapped cell is 13.3 pt against the reference's 14.4, which is the same 11/11.99.

`11.00` is `round(12 × 0.925)` — `constScaleLevels`' second row. So the whole of it is that this
tree ran the shrink-to-fit on each table cell separately, and the reference ran it on none of them.

## 1.2 The reference does not autofit a table cell — source

A `.ppt` writes a table as a plain **group of rectangles**, each an ordinary Body-kind text shape,
and `PptSlideLayout.Autofits` gives every one of those the fit that `svdfppt.cxx`:1030-1099 gives
them. The reference then throws the group away:

* `ProcessObj` reads `DFF_Prop_tableProperties` and `DFF_Prop_tableRowProperties` out of the
  **tertiary** property table of a shape holding no text of its own (`svdfppt.cxx`:1202-1240) and
  records the row array on the process data;
* the caller replaces the whole object with one `sdr::table::SdrTableObj`
  (`:2913` → `CreateTable`, `:7569`);
* `CreateTable` copies each member's `OutlinerParaObject` into a cell —
  `pSdrText->SetOutlinerParaObject(*pParaObject)`, `:7654-7658` — and calls `ApplyCellAttributes`
  (`:7412`) for everything else it wants: the four `SDRATTR_TEXT_*DIST`, `SDRATTR_TEXT_VERTADJUST`,
  `SDRATTR_TEXT_HORZADJUST`, `EE_PARA_WRITINGDIR` and the fill. **`SDRATTR_TEXT_FITTOSIZE` is not
  in that list**, and `git grep -n 'AUTOFIT\|FITTOSIZE' -- svx/source/table/` returns nothing at
  all.

So the fit does not survive the move into the cell, and a fresh `SdrTableObj`'s own item set
carries the pool default, which is `TextFitToSizeType_NONE`.

## 1.3 And it does not — binary

26.2.4.2's own flat ODP of `slides/ceiling-001/ppt/Thailand17.ppt`
(`soffice --headless --convert-to fodp`, two seconds):

* the deck's autofitted bodies export **`style:shrink-to-fit="true"`** — one style does, on
  `Default-outline1` — so the attribute is present in the file when it applies;
* page 11 is one `table:table` of 9 rows × 6 columns, 54 `table:table-cell`, and **not one of the
  54 cell styles states `style:shrink-to-fit` or `draw:fit-to-size`**, nor does `gr15`, the
  graphic style of the frame holding the table;
* all 42 of its spans state a flat **`fo:font-size="12pt"`**.

And in its PDF of the same file, every one of page 11's 77 body spans is drawn at 11.99 pt.

## 1.4 The change, and its census

`PptShapeGeometry.TableProperties` (927) and `.TableRowProperties` (928);
`PptSlideLayout.IsTableGroup`, which is the reference's `nTableProperties & 3` beside a non-empty
complex row array; a `Context.InTable` flag set when `Add` descends into such a group; and one arm
at the top of `Autofits`.

**The row heights are deliberately not touched**, and §5 measures what that leaves.

`tablecensus.py` over the 51 `.ppt` (`tablecensus.txt`):

| | |
|---|---:|
| documents holding a table group | **10** |
| table groups | 34 |
| members of those groups | 1089 |
| members holding Body/HalfBody/QuarterBody text | **391, in 5 documents** |

The five are `Thailand17`, `pres_ioc_phuket`, `outlook_of_nigerian_pension_sector`,
`undp_presentation_revised_17_may` and `pods05`, and all five are in §6's mover list.

---

# 2. The ruler a shape keeps when its text is somewhere else

## 2.1 Where it is, and why nothing had read it

A `.ppt` shape may hold its text by reference — an `OutlineTextRefAtom` naming an entry of the
document's slide list — while keeping its own `TextRulerAtom` in its client textbox. The reference
reads the two from those two places, and the order is the whole of it: `PPTTextObj`'s constructor
seeks `PPT_PST_OutlineTextRefAtom`, then seeks `PPT_PST_TextRulerAtom` **inside the same
`aClientTextBoxHd`** and remembers its file offset (`svdfppt.cxx`:6572-6580, the `nTextRulerAtomOfs`
block), *then* patches that header over to the referenced text, and only afterwards builds
`PPTTextRulerInterpreter aTextRulerInterpreter(nTextRulerAtomOfs, aClientTextBoxHd, rIn)`
(`:6713`).

`PptSlideLayout.TextOf` returned the slide-list run as soon as it saw the reference atom, so the
ruler was never looked for on that branch. **`PptTextReader.RulerIn`** is that seek and `TextOf`
now carries the result onto the run.

## 2.2 Every ruler that matters is in exactly that position

`rulercensus.py` walks the record tree, groups the records of each `msofbtClientTextbox`, and asks
whether the box holding a ruler also holds a reference atom (`rulercensus.txt`):

| | rulers | in a textbox that refers out | of those, stating a level-0 text offset |
|---|---:|---:|---:|
| `ws_prod-…-M.017-(French)-France.ppt` | 54 | **54** | 54 |
| `ws_prod-g-doc-Events-Part-M-presentation.ppt` | 3 | **3** | 2 |
| the 51 `.ppt` | 4025 | **121, in 17 documents** | 106 |

## 2.3 What it is worth, measured

The France deck's rulers state flags `0x000000F8` — five text offsets, no bullet offsets — and
`textOfs[0] = 336` master units, which is **1.482 cm**. Its Body master level states 216, which is
**0.953 cm**. Round 111 had already measured the reference starting that deck's bulleted text at
`43.09 + 1.482 cm` and this tree at `43.09 + 0.953 cm`, and named the pair of stated lengths without
finding which structure carried the larger one.

| | first six span x, France page 8 |
|---|---|
| 26.2.4.2 | 43.06 **85.10** 111.03 115.43 152.19 282.05 |
| base | 43.09 **70.09** 111.97 115.46 151.17 282.71 |
| head | 43.09 **85.09** 111.97 115.46 151.17 282.71 |

Page 16 the same, 70.09 → 85.09 against 85.10. `fourleg.txt` holds both rows.

---

# 3. Which font height sizes a picture bullet's box

## 3.1 The rule, in one sentence

`PPTParagraphObj::ApplyTo` copies the **style sheet's** `SvxNumBulletItem` for the instance and
replaces the single level the paragraph sits at, **and only when `GetNumberFormat` returns
non-zero** (`svdfppt.cxx`:6146-6199). A paragraph summing zero has no item put on it at all and
keeps the master's rule at every level. The two rules are built from different heights:

* the master's, `GetNumberFormat(…, rParaLevel, rCharLevel, nInstance)` at `:3642-3661`, passes
  **`rCharLevel.mnFontHeight`** — the master char level at that depth;
* the paragraph's, `:3690-3712`, passes **`pParaObj->First()->GetAttrib(PPT_CharAttr_FontHeight,
  …)`** — the first portion's stated height, or that same master char height where the portion
  states none (`PPTPortionObj::GetAttrib`, `:5448-5553`).

`nFontHeight` then goes into `ImplGetExtNumberFormat`'s
`round(nFontHeight × 0.2540 × nBulletHeight)` at `:3455`.

So V1 and V2 of round 111's `size-source.txt` are not two candidate rules: they are the same rule
under the two branches of `nHardCount`.

## 3.2 Why round 111 could not separate the decks with it

Two reasons, and the first is a one-bit reading error.

**`PPT_ParaAttr_BulletOn` is bit 0 and is the first of the seven** (`svdfppt.cxx`:3694,
`include/filter/msfilter/svdfppt.hxx`:1396). `size-source.txt` lists France's traced masks as
`0x1800`, `0x1801` and `0x1000` and reads them as *"none of the seven"* while naming BulletOn in the
same sentence; `0x1801` states it.

**But that is not what fires here.** Traced this round with the same instrument (a throwaway build,
not committed), France's page-8 *bulleted* paragraphs are the `0x1800` ones, and Part-M's page-21
paragraphs are `0x0000` and `0x1000` — so the mask alone still answers *soft* for both decks, and
V3 still behaves as V2 everywhere. What separates them is the **eighth** input to the same sum:
`ReadParaProps`' tail writes the shape ruler's value into the property set *and sets
`PPT_ParaAttr_TextOfs`'s mask bit with it* (`svdfppt.cxx`:5062-5068). France's shapes carry a
ruler; Part-M's page-21 shape does not.

## 3.3 Confirmed at the reference, on the deck that costs the most

26.2.4.2's own flat ODP of `ws_prod-g-doc-Events-Part-M-presentation.ppt` states 25
`text:list-level-style-image` heights:

* **level 1** takes four values — 1.102 cm, 0.945, 0.787, 0.709 — which are 28, 24, 20 and 18 pt at
  the level's 155 %;
* **level 2** takes **one**, 0.671 cm, on every one of the thirteen list styles that has one, which
  is 24 pt at 110 %.

Its depth-1 paragraphs state a hard 18 pt portion throughout. If the portion sized the box
unconditionally, 0.709 cm would appear at level 2 somewhere; it never does. And the two list styles
page 21 uses that override a level at all — `L18` at level 2 and `L7` at level 1 — replace it with
a **number**, not a picture, which is the same rule seen from the other side.

Read out of the two PDFs, page 21's drawn bullet graphics:

| | level 1 | level 2 |
|---|---:|---:|
| 26.2.4.2 | 31.24 pt | 19.02 pt |
| base (V1, the portion) | 20.10 | 14.26 |
| head | **31.24** | **19.02** |

And on France, where the ruler makes every paragraph hard, head reproduces the reference's
17.86/22.31/15.85/20.10/14.26 on pages 8, 14, 15, 16 and 44 — five pages, ten sizes, exact.

## 3.4 What is implemented

`PptTextBody.NumberingIsOwn` is `nHardCount != 0`, with the two arms that are not plain mask bits:

* **`BulletHeight` is hard only behind `BuHardHeight`**, in the mask *and* in the bullet-flags word
  (`ReadParaProps`, `:4903-4912`, the only one of the three hard-flags that gates the mask rather
  than the value);
* **`TextOfs` and `BulletOfs` are hard when the shape's ruler speaks for them** as well as when the
  paragraph does.

The eighth term is `ImplGetExtNumberFormat`'s own return for a bulleted paragraph — true when the
destination instance is `TSS_Type::Unknown`, which no shape path reaches (`ProcessObj` resolves one
at `:999-1028`), or when the paragraph carries an extended entry stating anything (`:3409-3419`).

**What is still not evaluated is one of the two terms round 111 named**, and it is now narrower:
where the text's own instance differs from the destination, the seven are also hard whenever the
source instance's master level differs from the destination's (`:5954-5958`). A Body text resolves
to the Body destination and never reaches it; a **HalfBody** or **QuarterBody** does, since both map
to Body at `:1021-1024`. No page in this corpus's picture-bullet set is one, so the arm is left
rather than guessed. The other term — a destination instance of `TSS_Type::Unknown` — is now
**resolved rather than unevaluable**: the shape path never produces one.

---

# 4. `France` 14, which round 111 filed as a layout page

Round 111's four-leg instrument reported this tree answering 20.01 both from the `.ppt` and from
26.2.4.2's own flat ODP of it, against the reference's 18.99, and concluded *"not an import
defect"*. The conclusion does not follow from the premise here, because **the ODP carries the same
information**: the reference exports the ruler's own answer as the list level's
`text:min-label-width`, and this tree's ODF reader was resolving that page's label width by the
same route it resolves everything else. The page is 18.99 at head, from the `.ppt`.

The lesson is about the instrument rather than about the page: **a round trip through the
reference's own ODF is only a discriminator where the two formats state the quantity in different
places.** For a label width they do not.

---

# 5. What remains on the three decks, and what this round did not do

**`Thailand17` 11's row heights.** The reference lays the table's rows out through
`TableLayouter`, which re-derives them; the drawn row boundaries are 112.76, 151.96, 188.16 …
441.55 — a 39.20 pt header and eight rows of exactly 36.20, filling the frame. This tree draws the
group's rectangles at their own anchors: 112.75, 151.50, 187.37 … 438.50, a 38.75 pt header and
eight rows of 35.87, leaving **3.05 pt unused at the bottom**. The error is 0.33 pt per row and
cumulative, and it moves no character's size and no page. The reference's numbers are its own
minimum-height arithmetic — 1277 hundredths of a millimetre is two 12 pt lines at fixed cell height
plus 0.13 cm of padding each way, and 1383 is that plus one 0.106 cm paragraph margin — so closing
it means porting `TableLayouter::LayoutTableHeight`, not reading a record. **Named, measured, not
worked.**

**`Thailand17` 8 and `W3_Case_Study` 10** are the raster ceiling and are unchanged: the reference
draws a picture where this tree decodes the pasted metafile and draws 592 real characters.

**Nothing on the France deck.** All eight of its previously differing pages are inside the band.

---

# 6. Reach — the whole slide corpus, base against head

`confine.py` renders a list with one CLI and records pages (from the real page tree, via PyMuPDF —
**not** `slides-r99/tfy.py`, which miscounts 45 of 51 banked reference PDFs), alphanumeric
characters and the PDF's md5, one directory per *document*, deleting each render as it goes.
**`SOURCE_DATE_EPOCH=1700000000` is pinned inside the script** rather than left to the environment.
Both binaries were built before either sweep started and neither was rebuilt while one was running.

```sh
probes/slides-bullet2-r112/confine.py <cli> /home/user/sample-files  ppt.list   out.tsv 3
probes/slides-bullet2-r112/confine.py <cli> /home/user/sample-files  pptx.list  out.tsv 3
probes/slides-bullet2-r112/confine.py <cli> /home/user/corpus-odf    odp.list   out.tsv 3
```

| column | documents | rendered both legs | **renderings that move** | page counts differing | alphanumeric counts differing |
|---|---:|---:|---:|---:|---:|
| `.ppt` | 51 | 51 | **20** | 0 | 0 |
| `.pptx` | 251 | 251 | **0** | 0 | 0 |
| `.odp` | 302 | 302 | **0** | 0 | 0 |
| **total** | **604** | **604** | **20** | **0** | **0** |

## 6.1 Every mover was predicted, and two predictions did not move

Three record censuses, all run before the sweep:

| set | documents |
|---|---:|
| **T** a table group holding Body-kind text (`tablecensus.py`) | 5 |
| **R** at least one ruler in a textbox that refers out (`rulercensus.py`) | 17 |
| **N** a master extended-paragraph level stating a blip (`slides-final-r111/census.py`) | 8 |

`T ∪ R ∪ N` is **22 documents**; the 20 movers are a subset of it, with nothing outside. The two
that did not move are `010605Vul` (one lost ruler, stating no level-0 text offset) and
`EG1_dsrc tech` (five lost rulers stating one) — for the second, why the ruler's values make no
difference to the drawn page is **not** established here.

## 6.2 Ink, over the twenty renderings that move

`inksweep.sh` scores **every** differing page against the banked 26.2.4.2 renderings rather than
only the MAJOR ones, at both legs (`ink-ppt.txt`).

| | base | head |
|---|---:|---:|
| sum \|ink\|% over the twenty | 103.94 | **79.49** |
| MAJOR pages | 30 | **25** |
| improve / worsen / level | — | **18 / 1 / 1** |

The largest single movement is `ws_prod-…-M.017-(French)-France`, **17.50 → 4.42**; the one that
worsens is `pods05`, 6.78 → 6.99.

## 6.3 The registered statistic

`sizesweep.py` over the 51 `.ppt`, scored with `slides-r107/sizescore.py` against the banked
reference table (`size-summary.txt`).

| | base = `4f4d0ddc8` | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **10** | **6** |
| documents holding one | 8 | 6 |
| total \|size error\| over 1534 pages | **52.12** | **47.11** |
| **fixed / newly wrong** | — | **4 / 0** |
| pages whose dominant size moved at all | — | **4 of 1534** |

Fixed: `Thailand17` **11** (11.0 → 11.99), `ws_prod-…-France` **14** (20.01 → 18.99) and **16**
(17.01 → 15.0, against 14.99), `ws_prod-…-Part-M-presentation` **21** (18.0 → 17.01). The base
column reproduces round 111's head column exactly.

**The six that remain hold no live seat**, and each was classified by round 111 rather than by this
one:

| page | what it is |
|---|---|
| `Thailand17` 8, `W3_Case_Study` 10, `Fundamentals_Module_1_basics` 6 | raster ceiling — the reference draws a picture, we draw its text |
| `RRM-training-syllabus-…` 16 | the seventh confound: 26.2.4.2 measures in DejaVu Serif and draws DejaVu Sans |
| `2015-Civil-Rights-Website-training` 22, `gfopportunitiesforlinkagespres` 27 | terminal: the reference disagrees with itself across its own round trip |

---

# 7. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Debug` at **0 warnings, 0 errors**, then each project alone with
`--no-build`, individually rather than as a solution.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 573 / 573 |
| Markup | 259 / 259 |
| OpenDocument | 160 / 160 |
| Rendering | 164 / 164 |
| Spreadsheets | 1352 / 1352 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1938 / 1938 |
| **Presentations** | **1180 / 1180** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else, confirmed **by name** in the log:
`TabStopComparisonTests` ×4 (`list-label-overrun.doc/.docx/.fodt/.odt`),
`PageDrawingComparisonTests` ×4 (`paginated.doc/.docx/.fodt/.rtf`),
`SheetDrawingComparisonTests` ×1 (`sheet-rich-text.xlsx`) and
`JustificationShrinkComparisonTests` ×1 (`justify-shrink-2013.docx`). **No eleventh.** Every project
reported its full total and none was aborted. Presentations' 1180 is the base's 1142 plus the
**38** new tests: `PptTableCellFitTests` 16 and `PptOutlineRulerTests` 22.

**Whether the new tests would have been red at the base**: the sixteen table tests call
`IsTableGroup` and an `Autofits` overload the base does not have, so "measured failing before" is
not available for them and is not claimed. Of the ruler tests, the four `RulerIn` ones call a method
the base does not have; the eighteen `NumberingIsOwn` cases likewise. What is claimed instead is
that each isolates one arm of a rule whose *output* is measured in §1–§3 against 26.2.4.2's own
numbers.

---

# 8. Looking at the pages, and the control that does not exist here

**The blind reading the brief asks for could not be delegated, and this round did not spend time
rediscovering that** — round 111 checked twice and `page-vision`'s own warning says to check once
and stop. There is no `Task` tool in this container and
`mcp__Claude_Code_Remote__create_session` spawns a sibling that cannot open a local PNG.

**So the one reading below is the seat's own and contaminated**, and every conclusion in this
write-up rests on arithmetic that does not depend on it:

* the table-cell fit — corroborated by the reference's own 54 cell styles, its 42 spans' stated
  `fo:font-size`, and the 548/548 alphanumeric bucket;
* the ruler and the label width — corroborated by the ruler's own `textOfs[0]` of 336 master units
  and the two text left edges, 85.10 and 85.09;
* the bullet-size source — corroborated by the reference's 25 stated `fo:height` values on the
  Part-M deck and its six drawn graphics on page 21.

The page looked at is `Thailand17` 11, both halves at 110 dpi, composed with
`page-vision/scripts/compose.py` (1100 × 1724, stacked, shown at 100 %). What it shows: the same
table with the same text at the same size in the same cells on both halves — the defect this round
closed is gone — and our grid's bottom rule sitting slightly high, which is §5's 3.05 pt. Both of
those were then measured, and the measurements are what is quoted.

---

# 9. What remains open on this seat

| page | what it is | seat |
|---|---|---|
| `Thailand17` 11 | the *row heights* — the reference re-derives them through `TableLayouter`, we draw the group's own rectangles; 0.33 pt per row, cumulative 3.05 pt, no character and no page | new, §5 |
| `Thailand17` 8, `W3_Case_Study` 10, `Fundamentals_Module_1_basics` 6 | raster ceiling | `TODO.raster-ceiling.md` |
| `RRM-training-syllabus-…` 16 | the seventh confound | not ours |
| `2015-Civil-Rights-Website-training` 22, `gfopportunitiesforlinkagespres` 27 | terminal instrument limit | — |
| `EG1_dsrc tech` | five lost rulers stating a level-0 text offset, and the rendering does not move; not established why | minor |
| — | `GetAttrib`'s source-against-destination master-level comparison, for a HalfBody or QuarterBody text | unimplemented, no corpus witness |

---

# 10. The files

| | |
|---|---|
| `tablecensus.py`, `tablecensus.txt` | `.ppt` table groups and the text kinds inside them |
| `rulercensus.py`, `rulercensus.txt` | rulers in a client textbox that refers out — the ones that were never applied |
| `fourleg.txt` | the three decks page by page, the bullet sizes, the label-width edges, and page 11's size buckets |
| `pagesizes.py` | the per-page dominant-size reader, three legs side by side |
| `confine.py`, `confine.txt`, `base-*.tsv`, `head-*.tsv` | the whole-corpus confine: pages, alphanumerics and md5 for 604 documents at both legs, epoch pinned |
| `sizesweep.py`, `base-sizes.tsv`, `head-sizes.tsv`, `size-summary.txt` | O15's statistic at both legs, scored against `slides-r107/sizes-ref.tsv` |
| `inksweep.sh`, `ink-ppt.txt`, `moved-ppt.list` | ink over every differing page of the twenty renderings that move |
| `ppt.list`, `pptx.list`, `odp.list` | the three columns, copied from `slides-final-r111` |
| `tests.log` | the suite |
