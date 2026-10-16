# odt-frame-r75 — a `draw:frame` whose stated height is a floor grows to its text

## Environment

Measured 2026-09-07, in the container the round ran in.

| | |
|---|---|
| Repository | `/home/user/libreoffice-core` (read-only C++ reference), worktree `/home/user/wt-odtframe` on `agent/odtframe` |
| Base commit | `0e54dba0a` |
| Reference binary | **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2**, TDF tarball, with the 38 duplicate/Narrow faces and the 8 Latin Noto moved aside |
| Corpus | `/home/user/corpus-odf/words/*/odt/*.odt` — 338 documents, 26.2.4.2's own `--convert-to odt` of the original words track |
| Original track | `/home/user/sample-files/words` — 338 documents, the `.docx`/`.doc` originals |
| Gate rule | `batch-check.sh`'s, reimplemented in `sweep.py`: page count, alphanumeric characters within `max(2%, 15)`, unembedded fonts |
| Workers | 2 throughout; another session's whole-corpus sweep and two other agents' sweeps were live, load average 20–28 |

`/usr/bin/soffice` (24.2.7.2) was **not** used for anything.

## What was measured

1. A census of the converted `.odt` corpus for the frames the change can reach
   (`census.py`, `growing-frames.tsv`).
2. Eighteen authored flat-ODF probes rendered through 26.2.4.2, one attribute apart, to
   establish the growth rule by measurement rather than only from the source
   (`gen.py`, `read.py`, `probe-heights.txt`).
3. The whole `.odt` column, before and after, against a reference half rendered once
   (`sweep.py`, `rows-before.tsv`, `rows-after.tsv`).
4. The original `.docx`/`.doc` track rendered with both binaries and compared byte for byte
   (`original.py`).
5. How much of a growing frame's text never reaches the page, as the reach of the thing this
   round did **not** do (`overflow.py`, `overflow.tsv`).

## The rule, established at the seat

### Which attribute decides the height *kind*

`XMLTextFrameContext_Impl` reads a frame's geometry from two attribute lists — the content
element's (`draw:text-box`, `draw:image`, …) first and the `draw:frame`'s second
(`xmloff/source/text/XMLTextFrameContext.cxx`:1113-1116).

* `svg:height` on `draw:frame` writes `nHeight` (:965-978).
* `fo:min-height` on `draw:text-box` writes the **same** `nHeight` and additionally sets
  `bMinHeight` (:997-1010).
* `Create()` sets `Height` when `nHeight > 0` (:643-646) and then
  `SizeType = (bMinHeight && XML_TEXT_FRAME_TEXTBOX == nType) ? SizeType::MIN : SizeType::FIX`
  (:655-661).

So the stated height is a **floor** exactly when the text box carries `fo:min-height`; and
because the box's attributes are processed *first*, a frame stating both keeps `svg:height` as
the floor and still grows. A frame stating neither leaves `SizeType` at the text frame's own
default, `VARIABLE` — grow with no floor, which is the same behaviour as a floor of nought.

`SizeType::MIN` becomes `SwFrameSize::Minimum`, which sets `m_bMinHeight` and leaves
`mbFixSize` false (`SwFlyFrame::SwFlyFrame`, `sw/source/core/layout/fly.cxx`:243-247;
`SwFlyFrame::FrameSizeChg`, :752-771).

### The growth

`SwFlyFrame::Format` (`sw/source/core/layout/fly.cxx`:1549-1570):

```
nRemaining = CalcContentHeight(pAttrs, nMinHeight, nUL);
if (IsMinHeight() && (nRemaining + nUL) < nMinHeight) nRemaining = nMinHeight - nUL;
if (nRemaining < MINFLY)                              nRemaining = MINFLY;
… SetHeight(aPrt, nRemaining); … AddBottom(aFrm, nRemaining + nUL);
```

* `CalcContentHeight` (`fly.cxx`:3538-3588) is the sum of the fly's lower frames' heights.
* `nUL = pAttrs->CalcTopLine() + pAttrs->CalcBottomLine()` — each edge's padding plus its
  border width plus its shadow space (`SwBorderAttrs::CalcTopLine_`,
  `sw/source/core/layout/frmtool.cxx`:2474-2486, over `SvxBoxItem::CalcLineSpace`,
  `editeng/source/items/frmitems.cxx`:3717-3755).
* `MINFLY` is **23 twips** (`sw/inc/swtypes.hxx`:59).
* The stated minimum is compared against `nRemaining + nUL`, so **it is a floor for the whole
  frame, insets included**, while the content height is the print area's.

### The same rule, measured

Eighteen probes, one page each, a paragraph-anchored `draw:frame` of stated width and no
`svg:height` whose `style:wrap="none"` pushes the body paragraph below it. The frame's height is
read as the movement of that body paragraph against the control `a-2para`
(`probe-heights.txt`); 12 pt Liberation Serif, so a line is 13.80 pt.

| probe | one attribute changed | body moves | what it establishes |
|---|---|---:|---|
| `b-4para` | four paragraphs, not two | **+27.600** | the height is the content's, 2 × 13.80 |
| `j-1para` | one paragraph | **−13.800** | and linearly so |
| `k-0para` | an empty `draw:text-box` | −13.800 | an empty box still holds one empty paragraph |
| `q-2para-before12pt` | 12 pt space-*before* on each of two | **+24.000** | **both** are counted, the first included |
| `r-4para-before12pt` | the same on each of four | +75.600 (= b + 48) | all four |
| `c-2para-after12pt` | 12 pt space-*after* on each of two | **+12.000** | only **one** — the last is dropped |
| `d-4para-after12pt` | the same on each of four | +63.600 (= b + 36) | three of four |
| `e-2para-pad10pt` | `fo:padding="10pt"` | **+20.000** | padding is added at both edges |
| `f-2para-border3pt` | `fo:border="3pt …"` | **+5.900** | so is the border (2 × 3 − 2 × 0.06) |
| `g-2para-pad10-bord3` | both | +25.900 | and they are additive |
| `l-2para-noborder` | `fo:border="none"` | −0.100 | the control's own 0.06 pt edges |
| `h-2para-minh3in` | `fo:min-height="3in"` | **+188.300** | the floor binds: 216 − 27.72 |
| `i-2para-minh3in-pad10` | the floor **and** 10 pt padding | +188.300 | the floor is the **whole frame**, insets included |
| `m-2para-minh0.2in` | a floor of 14.4 pt | +0.000 | a floor under the content does not bind |

The space-before / space-after asymmetry is Writer's and not an artefact: a text frame's own
area carries its upper space, and its lower space is only ever realised as the upper space of
whatever follows it (`SwFlowFrame::CalcUpperSpace`), which for the last frame in a fly is
nothing. That is why the measure implemented is `FlowLayouter.Extent` and not
`PlacedFlow.Advance`.

### The trap that cost the first probe set

**A `draw:frame` whose `draw:style-name` names an automatic graphic style with *no parent style*
is not a Writer text frame at all.** `XMLTextFrameContext`'s constructor sets
`m_HasAutomaticStyleWithoutParentStyle` (`XMLTextFrameContext.cxx`:1374-1394 — *"New distinguish
attribute between Writer objects and Draw objects is: Draw objects have an automatic style
without a parent style (#i51726#)"*) and `createFastChildContext` then routes the element to
`XMLShapeImportHelper::CreateFrameChildContext` (:1500-1507). A drawing shape fits itself to its
text by a different rule and ignores `fo:min-height` outright.

The first sixteen probes were written without a parent style and 26.2.4.2 drew
`fo:min-height="3in"` on a two-line box as **27.65 pt** — which reads exactly like "the reference
ignores `fo:min-height`" and is not that. Confirmed three ways before the cause was found: on the
authored flat ODF, on the same file zipped as `.odt`, and by patching a corpus document's own
`fo:min-height` from `0.2598in` to `3in` and getting a byte-identical text layer back. Every
height-less `draw:frame` LibreOffice's own exporter writes names `Frame` as its parent, so the
corpus is unaffected — but a probe has to name one before it measures anything.

## Census of what the change can reach

`census.py` over the 338 converted `.odt`, counting a `draw:frame` as *growing* when the
`draw:text-box` that follows it states `fo:min-height`:

* **59 growing frames in 50 documents.**
* **59 of 59 state no `svg:height`**, and **0 of 59** state both — so on this corpus "grows" and
  "states no height" coincide, although the rule is the first and that is what is implemented.
* Anchors: 53 `paragraph`, 5 `char`, 1 `as-char`.
* `fo:padding`: 52 × `0in`, 5 × `0.0591in`, 1 × `0.0555in`, 1 unresolved.
* `fo:border`: 53 × `none`, 3 × `0.06pt`, 2 × `0.74pt`.
* `fo:min-height`: 49 × `0in`, and 10 with a real floor, the largest `5.9701in`.
* **51 of the 59 declare `loext:may-break-between-pages="true"`**, in 43 of the 50 documents —
  which reproduces the figure the brief carried (51 of 58 in 43 of 49; the difference is that
  this census pairs each frame with the box that follows it rather than testing the frame's own
  attributes, and so finds one more).

## What moved

`.odt` column, 338 documents, same reference bytes on both sides, gate rule unchanged:

| | match | pages | words | pages,words |
|---|---:|---:|---:|---:|
| before (`0e54dba0a`) | **261** | 28 | 31 | 18 |
| after | **264** | 28 | 32 | 14 |

**261 → 264 of 338.** Twelve renderings differ at all; four gain a verdict and one loses one.

Gained:

| document | before | after |
|---|---|---|
| `087_Printable_Graph_Paper_Template_Green_Theme` | 1/2 pages, 27/37 | **2/2, 37/37** |
| `461249.odt` | 4/5, 8643/8911 | **5/5, 8911/8911** |
| `PAT-047 - Architecture and Detailed Design Assessment` | 3/4, 5992/6381 | **4/4, 6396/6381** |
| `slcc-architecture-uu-architecture` | 3/4, 5679/6328 | **4/4, 6328/6328** |

Lost:

| document | before | after |
|---|---|---|
| `047_Visual_Product_Roadmap_Template_Professional_Layout` | 1/1 pages, 213/225 | 2/1, **221**/225 |

The other seven movers keep their verdict and all but one move towards the reference on
characters — `OM template for non-complex NCC operators` goes 268507 → **268604 of 268604**,
`AFS-050-004-F2_0i` 11146 → 13175 of 13442 and 4 → 7 of 8 pages, `Case-Study-Heathrow-Airport`
2052 → 2159 of 6461.

### The one loss is not the growth's defect, and it has a name

`047_Visual_Product_Roadmap_Template_Professional_Layout` holds one growing frame containing a
table whose first column states **`loext:writing-mode="bt-lr"`** — bottom-to-top cell text. We
draw such a cell one glyph per line: the reference draws `Strategy` as a single run at
x = 32.8, and we draw `S`, `t`, `r`, `a`, `t`, `e`, `g`, `y` at eight successive baselines on
one x. That makes the cell, and therefore the frame, far taller than it is; grown to that height
the frame no longer fits its page and the body goes to a second one. On characters the same
change moves us **towards** the reference, 213 → 221 of 225.

Censused: **13 of the 338 converted `.odt` state a vertical writing mode, 92 occurrences**, and
exactly one of them is among the twelve movers. Closing it is a cell-rotation defect and not a
frame one; it is left, named.

## The original track does not move

`original.py` renders all 338 `/home/user/sample-files/words` documents with the before binary
and with the after binary under `SOURCE_DATE_EPOCH=0`, which makes `xray-office render`
byte-deterministic, and compares the two PDFs with nothing masked.

**338 documents, 338 identical, 0 moved, 0 unrendered by either binary.** The 272 `.docx`,
66 `.doc` and everything else on that track produce the same PDF, byte for byte, before and
after. `original.tsv` carries the two hashes per document.

This is what the code predicts: `PageFrame.GrowsToContent` is set in exactly one place —
`OdfFrames.Read` — and `FrameLayout.Grown` returns its argument unchanged for every frame that
does not carry it, so no `.docx`, `.doc` or `.rtf` document can reach a line of the new
arithmetic. The measurement is the check on that reasoning rather than a substitute for it.

## Fly splitting: measured, and deliberately not attempted

### What the reference does, measured

Two probes differing in one attribute, 60 twelve-point paragraphs in a paragraph-anchored frame
on an A4 page (`n-60para-split`, `o-60para-nosplit`):

| `loext:may-break-between-pages` | pages | frame lines on page 1 | first frame line | body resumes |
|---|---:|---:|---:|---|
| `true` | 2 | **52** | 57.26 | page 2, y 181.51 — below the fly's second fragment |
| `false` | 2 | **60** | **14.36** | page 2, y 71.01 |

With splitting allowed the fly is cut at the body's bottom and 8 lines continue on page two.
With it refused the fly stays whole, runs from y 14.36 to 828.56 on an 841.89 pt sheet — outside
both margins, captured onto the **page** rather than the body — and the whole body moves to
page two.

The seat is `SwFlyFrame::IsFlySplitAllowed` (`sw/source/core/layout/fly.cxx`:689-737), whose last
line is `pFormat->GetFlySplit().GetValue()` and which refuses outright for a fly that is not
at-content, in a header or footer, in a multi-column section, in a footnote, or growing upwards
from the body's bottom. The height a splittable fly is cut to is `GetFlyAnchorBottom`
(`fly.cxx`:114-162), read at `Format`'s :1576-1596. The continuation is a `SwFlowFrame` follow
chain: `SwFrame::GetNextFlyLeaf` (`sw/source/core/layout/flycnt.cxx`:1576) and
`SwFlyAtContentFrame::GetFollow` (:1566-1574). `IsFlySplitAllowed` is consulted from **46 places
in `sw/source/core`** — the layout, the text formatter, object positioning and the object
formatter.

*The brief cited `fly.cxx`:689-730 for `IsFlySplitAllowed`; the function runs to 737 in this
tree. Everything else the brief said about it is right.*

### The reach, measured

`overflow.py` compares, for each of the 50 documents holding a growing frame, what
`xray-office extract` finds (a walk of the content tree, no layout at all) against what the render
draws. A frame taller than the room left on its page draws its tail outside the sheet, where
`pdftotext` cannot see it, so *held ≫ drawn* is the signature.

**13 of the 50 draw materially less than they hold**, and every one of the 13 is a currently
failing row:

| document | drawn | held | reference |
|---|---:|---:|---:|
| `ESPN-R - MCF - RA - Ed1` | 43028 | 58948 | 65952 |
| `Case-Study-Heathrow-Airport` | 2159 | 6445 | 6461 |
| `part-147_approval list_20230119` | 3161 | 3384 | 3570 |
| `May 25 bulletin focus on carers in the workplace` | 2384 | 2721 | 2721 |
| `042_Visual_Product_Roadmap_Template_Colored_Background` | 235 | 499 | 487 |
| `020_Project_Timeline_Template_Modern_Theme` | 109 | 316 | 316 |
| `015_Project_Timeline_Template_Colored_Background` | 116 | 211 | 211 |
| `012_Project_Timeline_Template_Black_and_Brown_Theme` | 75 | 201 | 201 |
| `018_Project_Timeline_Template_Editable_Format` | 379 | 453 | 421 |
| `016_Project_Timeline_Template_Complete_Guide` | 587 | 653 | 633 |
| `011_Project_Timeline_Template_Beautiful_Theme` | 993 | 4611 | 885 |
| `AW-104D-RVSM-Aircraft-Approval-Checklist.pdf` | 660 | 772 | 764 |
| `0c662f5c948a7d81d32ffd833c5cc59421a5f522` | 1953 | 2012 | 2004 |

So splitting is worth about **13 rows** of the `.odt` column against growth's 3 — four times the
prize. It is still not attempted, and the reason is a specific architectural gap rather than a
budget.

### Why it is not attempted, and where the next round should start

Three pieces are needed and only the first two are local:

1. **`PageFrame.MayBreakBetweenPages`**, from `loext:may-break-between-pages`, plus
   `IsFlySplitAllowed`'s refusals — at-content anchor, not in a header, footer, footnote or
   multi-column section. Local to `OdfFrames` and `PageFrame`.
2. **Cutting the frame's flow at a deadline**, which is `GetFlyAnchorBottom` — the body frame's
   print bottom — and is the rule `FlowLayouter.Truncated` already implements for a
   fixed-height shape (keep an item whose top is strictly less than the deadline). Local to
   `FrameResolution.Content`.
3. **Putting the continuation on the following pages, and this is the gap.** In this engine a
   page exists because a *block* overflowed onto it: `Paginator.EmitPage` is reached only from
   the block loop. A fly's obstacles are keyed by the page its **anchor paragraph starts on** —
   `FrameResolution.Of` builds one `FrameObstacles` per block index from
   `obstaclesByPage[placement.Index]`, and `FrameObstacles.SpaceFor` works in that one page's
   coordinates. So a paragraph that spills onto page N+1 never sees page N+1's obstacles, and a
   fly fragment on a page no body text has reached has nothing to be an obstacle *to*.

   The seat for (3) is therefore **`FrameResolution.ObstaclesFor(int block)` and its consumer
   `Paginator`'s `_obstacles`**: the obstacle set has to become per *page* and be consulted for
   whichever page each line lands on, rather than per anchoring block in that block's own page
   frame. That is a change to the line filler's contract, on the one track whose gate is page
   counts, and it is what makes this architectural rather than large.

**One thing the brief supposed that this round found to be true and worth keeping:** growth does
not need `FrameLayout`'s order inverted. `FrameLayout.Place` is a pure function of the frame's
stated size and the page and anchor geometry, and the content's own layout needs the frame's
*width*, which the file states — so the height is measured in a pass that runs first
(`FrameLayout.Grown`, called from `FrameResolution.Of` before `Place`). Splitting does not need
that inversion either; it needs (3).

## What this round got wrong, or found the brief had

* **`fo:min-height` is not "the floor a height-less frame starts out at" — it is what makes the
  frame grow at all.** A `draw:frame` with `svg:height` *and* a `fo:min-height` box grows too,
  because the two attributes write the same variable and the box's is read first. No corpus
  document does it, but the rule implemented is the rule, not the corpus's coincidence.
* **`fo:min-height` is a floor for the *whole frame*, insets included**, not for its content
  height. `i-2para-minh3in-pad10` settles it: adding 10 pt of padding to a frame already held at
  its 3 in floor moves nothing.
* **The last paragraph's space-after is not part of the frame's height and the first
  paragraph's space-before is.** A round that had reached for `PlacedFlow.Advance` — the obvious
  choice, and the one a table cell wants — would have been out by that space on every frame that
  has one.
* **A parentless automatic graphic style makes a `draw:frame` a drawing shape**, and until that
  was found the probes reported that 26.2.4.2 ignores `fo:min-height`. Three independent
  measurements agreed on a wrong conclusion because they shared the defect.
* **`IsFlySplitAllowed` runs to `fly.cxx`:737, not :730.** A small correction to the brief's
  citation; the rest of what it said about splitting is accurate, including that the census
  figure ("51 of 58 in 43 of 49") is about what a *file declares* rather than what a frame needs.
* **The banked baseline was stale, as the brief warned.** The brief carried 257; a fresh render
  at this round's base commit `0e54dba0a` gives **261**. Every figure here is against that.

## What is deliberately left

* **Fly splitting**, with its reach measured at 13 rows and its seat named above.
* **The border and shadow halves of `nUL`.** `FrameLayout.Grown` takes `PageFrame.Padding`
  alone, because `FrameResolution.Content` insets a frame's text by the padding alone; taking a
  wider inset for the height than for the text would leave the frame taller than what was
  measured. Of the 59 growing frames, 53 declare `fo:border="none"` and five declare 0.06 pt or
  0.74 pt, so the whole of the omission is under a point and a half on five frames.
* **The as-character line box.** One of the 59 frames is `as-char`. `FrameResolution` grows it
  for placement and drawing, but `PageContent.InlineObjects` — which decides how tall the *line*
  carrying it is — still reads the stated size. One frame in 338 documents.
* **`loext:writing-mode="bt-lr"` cell text**, drawn a glyph per line: 13 documents, 92
  occurrences, and the cause of this round's single lost row.
* **The `draw:custom-shape` / `draw:rect` autofit rule.** Those hold their paragraphs directly
  and are `SdrTextObj`s rather than flies; `GrowsToContent` is set only for a `draw:text-box`,
  and how a drawing shape fits itself to its text is a different seat.

## Verification

Full build `dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.**

The ten non-fidelity projects, run individually:

| project | passed | failed |
|---|---:|---:|
| Containers | 109 | 0 |
| Core | 507 | 0 |
| Markup | 259 | 0 |
| OpenDocument | 139 | 0 |
| Presentations | 982 | 0 |
| Rendering | 162 | 0 |
| Spreadsheets | 1153 | 0 |
| Text | 723 | 0 |
| Vector | 302 | 0 |
| WordProcessing | **1734** | 0 |
| **total** | **6070** | **0** |

6067 before, plus this round's three. `XRay.Office.Vector.Tests` was re-run alone because the
loop that ran five projects in one shell printed no verdict line for it; alone it reports
302/0, and the missing line was the loop's own output handling rather than the project.

`XRay.Office.Fidelity.Tests` against 26.2.4.2 on `PATH`: **542 passed, 10 failed, 0 skipped of
552** — the ten known-open, named, and no others:

```
PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt   paginated.{doc,docx,fodt,rtf}
TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop      list-label-overrun.{doc,docx,fodt,odt}
SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt   sheet-rich-text.xlsx
JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt  justify-shrink-2013.docx
```

Run twice; the same ten both times.

## Files

| file | what it is |
|---|---|
| `census.py` | the growing-frame census over the converted `.odt` corpus |
| `growing-frames.tsv` | its output: 50 documents, per-frame anchors, floors and split flags |
| `gen.py` | the eighteen authored flat-ODF probes |
| `probes/*.fodt` | those probes as generated |
| `render.sh` | one reference render, in a profile keyed on a hex digest of the path |
| `read.py` | reads the probe PDFs and reports each one's movement against the control |
| `probe-heights.txt` | its output — the table quoted above |
| `sweep.py` | the corpus sweep: bank the reference once, render ours per binary, score |
| `rows-before.tsv`, `rows-after.tsv` | the `.odt` column at `0e54dba0a` and with the change |
| `original.py` | the original words track rendered with both binaries and byte-compared |
| `overflow.py`, `overflow.tsv` | held-against-drawn, which is the split's measured reach |
| `rule.md` | the working notes the rule section was written from |
