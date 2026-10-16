# `words-seat-r94` — a page's one text area, and what the wide capture rule is *not* missing

Measured 2026-09-11 in `/home/user/wt-wordsseat` (branch `agent/wordsseat`, base `725bbbfd2`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2**. The banked reference
renderings were reused rather than re-rendered: `/home/user/gate-odf-r80/ref` for the converted
`.odt` corpus and `/home/user/gate-orig-r83/ref` for the 947 originals.

**There is no `Task`/subagent tool in this container**, so every page this round looked at was read
by the same agent that wrote the change. Those readings are contaminated and are used only for
direction; every claim below rests on arithmetic that does not depend on one, and the crop in
`plain-margins-crop.png` is banked so that a later reader can disagree with it.

---

## 1. Seat one: a page carried one text area and it was the last section's

`odt-sectable-r92` §5 left this with its seat named: `LaidOutPage.BodyArea` is `geometry.TextArea`
at the moment the page is *emitted* (`Paginator.cs`:2649), and a page whose columned, indented
`text:section` ends part way down it is emitted with the section that restores the master. A line
carried its own column count, gap and ruler — but the rectangle those divide came from the page.

The line breaking was already right. What was wrong was the *drawing*.

### The change

Three files, and none of them is the ODF reader:

* **`PlacedLine`** gains `BodyLeft` and `BodyWidth` — the text area in force where the line was laid
  out. `BodyWidth` is the discriminator: zero means *the page's*, which is what every line of a
  document with no text section in it leaves it at, and what a header's, a footer's and a cell's
  lines leave it at.
* **`LaidOutPage.BodyAreaOf(PlacedLine)`** answers that rectangle, and `ColumnArea(PlacedLine)` and
  the private `Area` divide *it* rather than `BodyArea`. **Horizontal only** — a text section is
  inset from the body's sides and begins wherever the flow had reached, so its top and its height
  are the page's and `PlacedLine.Top` still means what it did.
* **`PageDrawing.DrawBody`** groups lines by their own band as well as by their column, and its
  single-column fast path now also requires that every line's band *is* the page's. Without the
  second half a page of one-column text drawn from two different left edges takes the first
  branch and is drawn from one.

### Measured at the reference, on `odt-sectable-r92/nested.py`'s own fixtures

`variant-columnx.txt`, produced by `columnx.py`, which clusters the left edge of **every text span**
and merges nothing. That matters here: `odt-startx-r88/startx.py` merges every span of a baseline
and keeps the leftmost, so on a two-column page it samples only the first column — and
`odt-sectable-r89/xhist.py`'s docstring records the same trap the other way, a merged histogram
reading a two-column page as one column. A wrong *second* column is invisible to both.

`plain-margins` is the base fixture's two-column section indented `fo:margin-left="0.5in"` and
`fo:margin-right="0.75in"`, so its measure is 108…486 and, at the file's own 36 pt gap, its columns
are 171 pt wide at 108 and 315:

| left edge, with the number of spans on it | 26.2.4.2 | before | after |
|---|---|---|---|
| the one-column text above and below | 72.10 (6) | 72.00 (46) | 72.00 (6) |
| the section's first column | **108.10** (40) | *(none)* | **108.00** (40) |
| the section's second column | **315.10** (39) | *(none)* | **315.00** (39) |
| where this tree drew them instead | | 72.00, **324.00** (39) | |

The 0.10 pt is the constant offset between the two writers' text origins and is present on the
one-column lines too. Every span count matches the reference's exactly. The same table holds for
`nested-margins` (a nested `text:section`) and `toc-margins` (a `text:table-of-content`), and the
two controls — `base` and `plain`, whose section states no indent — do not move at all.

Scored with `odt-startx-r88/startx.py` over the five, which is the weaker instrument but the one
the record already quotes:

```
  matched b/a mean|dx| before     after  within .1pt  unique mean|dx| unique   document
    14/14               0.100     0.100    14/14          12           0.100   base.pdf
    13/13               0.100     0.100    13/13           4           0.100   plain.pdf
    14/14              20.671     0.100     6/14           6           0.100   plain-margins.pdf
    17/17              23.394     0.100     6/17          10           0.100   nested-margins.pdf
    17/17              23.394     0.100     6/17          10           0.100   toc-margins.pdf
TOTAL matched 75/75  mean |dx| before 14.500 pt after 0.100 pt  within 0.1 pt 45 -> 75
```

### What moved on the corpus

`odt-rows-before.tsv` / `odt-rows-after.tsv`: our half of the `.odt` track rendered by a binary
built with the three source files reverted to the round's base and again by the tree as it stands,
both scored against the same banked reference bytes.

| | `.odt` track |
|---|---|
| before | **293** of 338 match |
| after | **293** of 338 match |

**No gate verdict moves in either direction**, and no row's verdict string differs.

**Five renderings change** (`odt-movers.txt`) and they are exactly the five documents the section
census finds with an indented `text:section` — `odt-startx-r88/census-sections.py` answers *10
margined sections in 5 documents*, and the movers are those five and nothing else.

`selfmove.py` pairs spans between our own two legs by (page, baseline, text), which is what the
three `150_5300_13` revisions need: they sit a page later than 26.2.4.2 does and fail the gate on
pages for reasons of their own, so a page-for-page comparison with the reference compares different
content on them.

```
  spans   moved  mean dx of movers      min      max   document
   6608       6               7.20     7.20     7.20   150_5300_13_chg10__odt.pdf
   3123      14               7.20     7.20     7.20   150_5300_13_chg12__odt.pdf
   2112       1              22.50    22.50    22.50   150_5300_13_chg8__odt.pdf
    675      10             -42.00   -42.00   -42.00   644730BRI0mna000BOX361539B00public0__odt.pdf
    108      40             -38.15   -38.15   -38.15   手机免提系统TSB__odt.pdf
```

**71 spans move in the whole track and each document moves them by exactly one constant, which is
its own section's indent.** Nothing else in 338 documents moves by so much as a hundredth of a
point — which is the shape a correction of this kind should have, and is why the fixture rather
than the corpus is where the 36 pt is visible.

Against the reference, with `colscore.py` — per page, every span's left edge paired with the
nearest edge the reference actually uses, weighted by how many spans sit on it:

```
mean|dx| before     after  off-edge before   after   spans   document
         20.106    20.113             4208    4208    6279   150_5300_13_chg10__odt.pdf
         26.483    26.455             1774    1773    2946   150_5300_13_chg12__odt.pdf
         17.717    17.721             1066    1067    1974   150_5300_13_chg8__odt.pdf
          2.931     2.885              149     147     675   644730BRI0mna000BOX361539B00public0__odt.pdf
          2.462     1.347               22      16     111   手机免提系统TSB__odt.pdf
```

The two documents whose pagination agrees with the reference improve; the three that are a page out
move by less than 0.03 pt in a comparison that is not measuring them. `odt-startx-r88/startx.py`
agrees on the two (`10.023 → 7.729` and `16.374 → 3.967`) and reports the three as *unchanged*,
which is the merged-baseline instrument doing exactly what its docstring warns of.

### What it is not

* **It is not vertical.** A `text:section` states `fo:margin-left`/`fo:margin-right` and takes its
  top from wherever the flow reached; `SwSectionFrame::Init` insets the print area by the format's
  `SvxLRSpaceItem` (`sectfrm.cxx`:129-166). `PlacedLine.Top` is still measured from the page's body.
* **It does not reach an anchored frame.** `FrameLayout` places a frame against `page.BodyArea`, so
  a frame anchored inside an indented text section is still positioned from the page's text area.
  No corpus document has one; **left**.
* **It does not reach a table.** A table is placed with an absolute rectangle taken from
  `body.ColumnArea(column)` at layout time, which already carries the section's geometry —
  `odt-sectable-r92` §0.1 measured all three corpus tables-in-columned-sections at the reference
  and they agree to 0.35 pt.

### Confinement

`confine.list` is 1305 documents: the **338 words originals**, their **338 `.rtf` twins**, 10
`.ods`, 10 `.odp`, and — because the rulebook's test is that a words change moves no sheet and no
slide — **every one of the corpus's 302 slides and 307 sheets**. Both legs rendered with
`SOURCE_DATE_EPOCH` fixed and `obj`/`bin` cleared per leg, the restore done with `cp` and an
explicit `touch`:

```
1305 compared byte for byte, 0 differ
```

**Zero, and zero is the right answer here** where `odt-sectable-r92`'s four was: the band a line
records is `page.TextArea.X`/`TextWidth` at the moment it was placed, and `page.Margins` changes
part way down a sheet for a `WritingSection.IsTextSection` and for nothing else, so outside ODF
`BodyAreaOf(line)` is `BodyArea` by construction. `PushedDownBy` and `PulledUpBy` move only
`Margins.Top` and `Margins.Bottom`, which is why the equality is exact rather than approximate.

Both legs, and both `.odt` sweeps, were checked for truncation before being compared — the
container was down to 2.7 GB free with three rounds rendering, and on this filesystem a write that
runs out of room does not fail loudly. **1305/1305, 1305/1305, 338/338 and 338/338 PDFs end in
`%%EOF`.** The restored binary was then checked against the sweep it is claimed to have produced:
`plain-margins` re-rendered after the rebuild is byte-identical with the date masked.

The suite is at its documented baseline: ten non-fidelity projects green (6382 tests), and
`XRay.Office.Fidelity.Tests` **542 passed / 10 failed** with exactly the ten the rulebook names —
`PageDrawingComparisonTests.EveryLineIsDrawn` ×4, `TabStopComparisonTests.AListLabelsTabAdvance`
×4, `SheetDrawingComparisonTests.APictureIsDrawn`, `JustificationShrinkComparisonTests`. **The four
TabStop cases were checked rather than assumed**: they are `list-label-overrun` in four formats,
none of which holds a text section, and all four fail identically before and after.

---

## 2. Seat two: `bCheckBottom` is real, has reach, and is **not** what the wide capture rule is missing

`frame-area-r85` §3 left the wide form of the frame capture rule with a named suspect:

> The likely missing half is named rather than guessed at … `SwToContentAnchoredObjectPosition::CalcPosition`
> passes `bCheckBottom = !DoesObjFollowsTextFlow()` into the clamp
> (`tocntntanchoredobjectposition.cxx`:457) … and `PROP_FOLLOW_TEXT_FLOW` is written only for an
> anchor *inside a table*, so the pool default decides it everywhere else.

Every clause of that is true. The conclusion does not follow, and one census settles it.

### The pool default is `false`, so `bCheckBottom` is `true` unless the anchor is in a table

`init.cxx`:437 is `{ RES_FOLLOW_TEXT_FLOW, new SwFormatFollowTextFlow(false), … }`, and
`SwFormatFollowTextFlow`'s own constructor default is `false` too (`fmtfollowtextflow.hxx`:34).
The only three `SwDoc::SetDefault`/`SetFormatAttr` writes of it in the tree are the HTML filter
(`swhtml.cxx`:370, `true`) and WW8 (`ww8graf.cxx`:2430, `ww8par2.cxx`:3454) — **no writerfilter
path changes the default at all**, and writerfilter's three per-object writes are each gated on
being in a table (`GraphicImport.cxx`:1316-1318 and :1859-1861 on `IsInTable()`,
`OOXMLFastContextHandler.cxx`:1879-1883 on `mnTableDepth > 0`).

So `!DoesObjFollowsTextFlow()` is **`true`** — bottom check on, exactly what the wide rule already
does — for every DOCX object that is not anchored inside a table.

### None of the ten documents the wide rule moved has one

`anchor-census.py` walks `word/document.xml` *and every header and footer* of `MANIFEST.tsv`'s 272
DOCX-family files, counting each `wp:anchor` and each absolutely positioned VML shape and whether
it is inside a `w:tbl` (`anchor-census.txt`):

```
DOCX-family documents scanned: 272, carrying an absolutely positioned object: 170
objects: 6055   of them inside a w:tbl: 552   documents with one: 40
```

**552 of 6055, in 40 of 272 documents** — the rule is real and has reach. And **not one of those 40
is among the ten documents r85's wide rule moved**: `anchor-kind.py` reports every anchor of all
ten as sitting in the body, so `bCheckBottom` is `true` on every object in every one of them.
Adding the bottom-check half changes nothing about any of them, and it cannot be what makes the
wide form net worse. **Refuted.**

### Two halves that are missing, both named from the source and one of them measured

**(a) `mbDoNotCaptureAnchoredObj` is not "wrap-through escapes"; it asks a different question of a
fly than of a draw object.** `SwAnchoredObjectPosition`'s constructor
(`anchoredobjectposition.cxx`:125-144):

```cpp
if (mbIsObjFly)  bConsidered = bWrapThrough && !bTextBox;      // a picture, a frame, an OLE object
else             bConsidered = bWrapThrough || !bTextBox;      // a drawing shape
mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE;
```

With DOCX's `DoNotCaptureDrawObjsOnPage` set, that comes to: a **picture** and a **shape that has a
text box** are captured unless they are wrap-through; a **shape with no text box is never captured,
whatever its wrap**. The tree's own remark says the escape is wrap-through *and nothing else*, and
that is wrong for the third of those.

**It is not the discriminator here either, and saying so is the point of checking.**
`anchor-kind.py` classifies every anchor of the ten by what it holds, and the only bare `wps:wsp`
shapes among them — 18 in `004_Free_Genogram`, 2 in `008` — are all `wp:wrapNone`, which *is*
wrap-through, so they escape under either reading. Correct the rule because it is wrong, not
because it will move these ten.

**(b) Two of the ten are header-anchored, and the reference draws them where they are stated.**
`b053-19` and `Case-Study-Heathrow-Airport` carry exactly one anchored object each, both
`wrapTight` pictures in `word/header1.xml` with a *negative* `posOffset` against `paragraph`
(−276225 and −121876 EMU). Read off the banked reference and this tree's own rendering:

| | 26.2.4.2 | this tree at head |
|---|---|---|
| `b053-19` page 1, the header picture's box | y 14.20…101.95 | y **14.25**…102.00 |

They already agree to 0.05 pt with no capture at all, so any clamp that moves that picture is a
regression by construction. `ImplAdjustVertRelPos` has a header/footer-specific escape of its own
for this family (tdf#123002, :641-651), and the `compatibilityMode` 15 narrowing to the body cannot
apply to a header anchor because `mpAnchorFrame->FindBodyFrame()` finds none.

**(c) And what is left is the *area*, which r85 stated and did not apply.** Under
`compatibilityMode` 15 the vertical area is the page **body** for every vertical relation except
`PAGE_FRAME` and `PAGE_PRINT_AREA` (:562-573); r85's wide rule clamped to the **sheet** at every
origin, which is a different rule from the C++'s at every origin but those two. That is the one of
the three that can move the remaining eight, and it needs its own measurement rather than another
reading: on the three `Unit_Circle` documents the objects in question are a chart frame and a text
box both stated against `margin` — so `PAGE_PRINT_AREA`, the *excluded* relation — with
`posOffset` −221.35 pt horizontally on a 595.3 pt page, and their drawn extent is confounded by
`ChartLayout`'s own fit, so the capture cannot be scored off them without separating the two.

**Left, with a better seat than it had.** `PaginationOptions.CapturesMarginBandObjects`' remark
carried the refuted suspect and now carries these three.

### And the other thing r85 left is still true and still not worth a round

`wp14:sizeRelH`/`sizeRelV`: 1873 elements across 146 of the 272 DOCX, of which three carry a
non-zero percentage. Not re-derived here; recorded so that it is not.

---

## 3. Files

| file | what it is |
|---|---|
| `columnx.py` | the left edges a page's text is drawn from, clustered, merging nothing |
| `colscore.py` | those clusters scored per page against the reference's, weighted by span count |
| `selfmove.py` | spans paired between our own two legs, for a document that sits a page out |
| `anchor-census.py`, `anchor-census.txt` | every absolutely positioned object in the 272 DOCX and whether it is inside a `w:tbl` |
| `anchor-kind.py`, `anchor-kind.txt` | what each anchor of the ten wide-rule movers holds, which is what decides whether it is captured |
| `variant-columnx.txt` | the five `nested.py` fixtures, three renderings each |
| `odt-startx.txt`, `odt-colscore.txt`, `odt-selfmove.txt`, `odt-movers.txt` | the five `.odt` movers on three instruments |
| `odt-rows-before.tsv`, `odt-rows-after.tsv` | the two `.odt` sweeps, both against `/home/user/gate-odf-r80/ref` |
| `confine.list`, `confinement.txt` | the 1305-document no-reach check and its result |
| `plain-margins-crop.png` | 26.2.4.2, before and after, the same crop of the same page |

Reused rather than rebuilt: `odt-sectable-r92/nested.py` and `confine.sh`,
`odt-startx-r88/startx.py`, `sweep-odt-ours.sh` and `census-sections.py`, and the banked references
`/home/user/gate-odf-r80/ref` and `/home/user/gate-orig-r83/ref`.

## 4. Reproducing

```sh
export XRAY_OFFICE_CLI=/abs/tree/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

# the fixtures, rendered through 26.2.4.2 and through this tree
python3 ../odt-sectable-r92/nested.py tests/corpus/features/odt-section-columns.odt /abs/fx \
        base plain plain-margins nested-margins toc-margins
for v in base plain plain-margins nested-margins toc-margins; do
  "$XRAY_OFFICE_CLI" render /abs/fx/$v.odt --outdir /abs/ours; done
python3 columnx.py /abs/fx/plain-margins.pdf /abs/ours/plain-margins.pdf

# the .odt track, scored against the banked reference
../odt-startx-r88/sweep-odt-ours.sh /home/user/corpus-odf /home/user/gate-odf-r80 /abs/after 3
python3 ../odt-startx-r88/movers.py /abs/before/ours /abs/after/ours
python3 colscore.py /abs/before/ours /abs/after/ours /home/user/gate-odf-r80/ref /abs/movers.list
python3 selfmove.py /abs/before/ours /abs/after/ours /abs/movers.list

# confinement
../odt-sectable-r92/confine.sh confine.list /abs/conf-after 3

# seat two
python3 anchor-census.py /home/user/sample-files/MANIFEST.tsv
python3 anchor-kind.py <the ten documents>
```

`columnx.py`, `colscore.py` and `selfmove.py` need PyMuPDF; nothing here renders a reference except
`nested.py`, which bounds its own `soffice` call.
