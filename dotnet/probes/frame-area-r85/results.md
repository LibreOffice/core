# What a frame's `relativeFrom` measures from — and two defects that were already closed

*Measured 2026-09-08 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used throughout and they agree on every figure below: the distro's **24.2.7.2** at
`/usr/bin/soffice`, and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with its duplicate
metric-compatible faces, its Latin Noto, its DejaVu Condensed and its `LiberationSansNarrow` all in
the `.duplicates-aside/`, `.noto-aside/` and `.condensed-aside/` directories the rulebook describes.
`fc-match "DejaVu Sans"` resolves to DejaVu, `fc-match Calibri` to Carlito and `fc-match Cambria` to
Caladea. XRay.Office at `agent/framearea`, base `6a6f18d9a`; corpus `/home/user/sample-files`, 947
paths in `MANIFEST.tsv`.*

---

## 0. What this round was briefed to do, and what it found instead

The brief named two open defects and made the first one the priority, as the diagnosed cause of a
live regression. **Both were already closed at the round's own base commit, and the two `TODO.md`
entries that record them are stale.** The evidence is in this directory and is summarised first,
because a round that repeats work already done is worse than one that does nothing.

| briefed as open | actually | closed by |
|---|---|---|
| `RelOrientation::PAGE_PRINT_AREA` starts below the header, not at `w:top` | **closed** | `dccc5be0a`, an ancestor of `6a6f18d9a` |
| `wp:effectExtent` is not added to an inline drawing's horizontal position | **closed** | the round that wrote `probes/words-inline-effectextent/` §*Horizontally* |

Both were re-measured at `6a6f18d9a` before anything was changed, with the probes' own fixtures:

* **`words-margin-print-area`, 8 fixtures**: this tree agrees with both references on all eight, the
  largest disagreement being 0.13 pt — under the 0.25 pt raster quantum at 288 dpi. `FrameLayout`
  places a `FrameVerticalOrigin.PageMargin` frame in `LaidOutPage.BodyArea`, and the doc-comment at
  that seat already carries the measurement and the C++ citation.
* **`words-inline-effectextent`, 8 horizontal fixtures**: `x-l-only`'s ink band reads **114.50**
  against both references' 114.25 and `x-r-only`'s stays at the control's 103.75; a text box's
  `INSIDE` run tracks. `PageFrame.InlineOffset` is `new DocPoint(EffectExtent.Left, Length.Zero)`.
* **The corpus witness**: on `WordArt_Shapes_Arrows_Catalog1.docx`, 52 pages against 26.2.4.2's 52,
  and the page's ink columns agree with 26.2.4.2 on **52 of 52 pages** within the raster quantum
  (`catalogue-x.py`, `catalogue-x.txt`). The TODO's quoted x span of 229.68…359.64 does not
  reproduce.

**The regression the brief made the priority is closed, and by more than the regression was worth.**
Re-running `probes/words-vml-fontwork/measure.py` at `6a6f18d9a` (`vml-fontwork-at-head.txt`), mean
absolute grey difference at 100 dpi over the whole document, against 26.2.4.2:

| document | before the watermark landed | after it landed (the regression) | at `6a6f18d9a` |
|---|---:|---:|---:|
| `ABCD-FE-01-00 Flight Envelope` | 15.210 | 14.855 | **14.706** |
| `ABCD-SDE-23-00 Avionic System Description` | 6.050 | 5.694 | **5.211** |
| `ABCD-WB-08-00 Weight and Balance` | 8.515 | 8.197 | **8.142** |
| **`DOA_Template_Form_Type_Certification_Programme`** | 10.960 | *11.239* | **9.891** |
| `technical-architecture` | 5.884 | 5.499 | **4.021** |

`DOA_Template` is **9.891**, which is 1.348 below the regressed figure and **1.069 below the
pre-watermark baseline the regression was measured against**. All five rows are below both stored
figures. The row is un-regressed and the two stored figures are now history rather than targets.

**So the round's own work is the third defect it found on the way**, which is in the same seat, was
never recorded anywhere, and is described from §2 on.

---

## 1. What the brief got wrong

1. **Both defects were closed.** `XRay.Office.WordProcessing/TODO.md`:2232 and :2242 are stale. The
   brief's own warning — *"several module TODO entries are stale and some are wrong … check before
   believing any TODO, including the two above"* — was the operative instruction, and both of the two
   it named turned out to be instances of it. The entries are rewritten in this round's commit.
2. **"`DOA_Template` … the watermark lands 34.7 pt high"** does not reproduce and had already been
   withdrawn: `words-margin-print-area/results.md` records the gap as 16.56 pt at `6bf527227` and
   then as 0.24 pt after the fix. This round measures the document itself at 9.891 against 26.2.4.2.
3. **"`RelOrientation::PAGE_PRINT_AREA` starts below the header"** is right about the mechanism and
   the C++ line the brief declined to name is `anchoredobjectposition.cxx`:336-364 — but it is not
   the only seat, and the second one matters for §2: `SwToContentAnchoredObjectPosition::CalcPosition`
   resolves the same relation independently at :597-616, through
   `SwPageFrame::PrtWithoutHeaderAndFooter()`, which is the body rectangle by another name.
4. **"a gate row's verdict is decided on column 9 (`glyphs`)"** is right and is checked here:
   `batch-check.sh`:290-293 is `awk -v a="$og" -v b="$rg" … d > b*0.02 && d > 15`, with `$og`/`$rg`
   the alphanumeric character counts. `score-gate.py` transcribes that rule rather than improving on
   it.
5. **"Both of these are position defects and may move no gate row at all"** is right, and it is right
   about this round's own work too: **no gate verdict moves in either direction.**

---

## 2. The third defect: `relativeFrom` has nine values and four of them name a *band*

`wp:positionH`/`wp:positionV`'s `relativeFrom` is mapped onto a `RelOrientation` by
`PositionHandler::lcl_attribute` (`sw/source/writerfilter/dmapper/GraphicHelpers.cxx`:57-135), and
the layout resolves each in `SwAnchoredObjectPosition::GetVertAlignmentValues` /
`GetHoriAlignmentValues` (`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:280-390 and
:735-875). This tree read six of the nine values as *the page* or *the margin area*. They are not.

| `relativeFrom` | `RelOrientation` | the rectangle | seat |
|---|---|---|---|
| `topMargin` | `PAGE_PRINT_AREA_TOP` | sheet top … **body** top | :327-335 + `tocntnt…`:305-306, :367-370, :407-410 |
| `bottomMargin` | `PAGE_PRINT_AREA_BOTTOM` | **body** bottom … sheet bottom | :365-389, `tocntnt…`:617-628 |
| `leftMargin` | `PAGE_LEFT` | sheet left … text area left | :769-778 |
| `rightMargin` | `PAGE_RIGHT` | text area right … sheet right | :779-788 |
| `insideMargin` | `PAGE_FRAME` + page toggle | the sheet | `GraphicHelpers.cxx`:109-112 |
| `outsideMargin` | *(no case)* → `FRAME` | the text **column** | `GraphicHelpers.cxx`:130-132 |

Three of those are counter-intuitive enough that only a measurement settles them, and one of them
cannot be read off the layout switch at all:

* **`topMargin` shares its `case` label with `PAGE_FRAME`** (:327-335) and therefore answers *the
  whole page's height* as its alignment area. Read that way, a 20 pt band centred against
  `topMargin` on A4 lands at 410.95 pt. It does not:
  `SwToContentAnchoredObjectPosition::CalcPosition` overrides the height for exactly this relation,
  substituting `nHeightBetweenOffsetAndMargin` — the offset plus
  `page->GetTopMargin() + headerFrame->GetPaintArea().Height()`, which is the body's own top — under
  both `CENTER` (:367-370) and `BOTTOM` (:407-410). Both references draw it at **26.00**.
* **`outsideMargin` reaches no case**: the switch warns and leaves `m_nRelation` at its
  `RelOrientation::FRAME` default. It is the text column, not a margin, and not the sheet.
* **VML crosses the horizontal pair over.** `lcl_SetAnchorType`
  (`oox/source/vml/vmlshape.cxx`:616-700) is a second, independent mapping:
  `inner-margin-area` joins `right-margin-area` under `PAGE_RIGHT` (:687-689) and
  `outer-margin-area` joins `left-margin-area` under `PAGE_LEFT` (:690-692) — the opposite way round
  from DrawingML's `insideMargin`/`outsideMargin`. The vertical pair does *not* cross over (:633-640).
  So the two branches of one `mc:AlternateContent` genuinely mean different things, and a reader that
  translated VML into the DrawingML vocabulary would put those two 523 pt apart.

### The measurement

`make-relfrom.py` builds **34 one-page A4 fixtures**: a red band on a page with 72 pt margins and a
one-line running head and foot, both fitting the room their margins reserve, so the body is exactly
`w:top`…`pageHeight − w:bottom` and the header-overflow rule of `probes/words-margin-print-area/`
cannot confound this one. Twenty-five state a `wp:anchor`, nine a `v:rect`. `measure-relfrom.py`
reads the band's rectangle off the raster at 288 dpi — the band is the only red ink on the page, so
it separates from the body line and the running heads with no assumption about either.

**24.2.7.2 and 26.2.4.2 are identical on all 34 rows.** Agreement with 26.2.4.2, within the 0.25 pt
raster quantum: **10 of 34 before, 34 of 34 after.** The full table is `relfrom-before-after.txt`;
the rows that moved:

| fixture | reference | ours before | ours after |
|---|---:|---:|---:|
| `v-topmargin-0` | y 0.00 | 72.00 | **0.00** |
| `v-topmargin-36` | 36.00 | 108.00 | **36.00** |
| `v-topmargin-centre` | 26.00 | 411.00 | **26.00** |
| `v-topmargin-bottom` | 52.00 | 750.00 | **52.00** |
| `v-bottommargin-0` | 770.00 | 72.00 | **770.00** |
| `v-bottommargin-centre` | 796.00 | 411.00 | **796.00** |
| `v-bottommargin-bottom` | 821.75 | 750.00 | **822.00** |
| `h-leftmargin-centre` | x 16.00 | 277.75 | **16.00** |
| `h-leftmargin-right` | 32.00 | 555.25 | **32.00** |
| `h-rightmargin-0` | 523.25 | 0.00 | **523.25** |
| `h-rightmargin-centre` | 539.25 | 277.75 | **539.25** |
| `h-outsidemargin-0` | 72.00 | 0.00 | **72.00** |
| `h-outsidemargin-right` | 483.25 | 555.25 | **483.25** |
| `m-h-innermargin-0` | 523.25 | 72.00 | **523.25** |
| `m-h-outermargin-0` | 0.00 | 72.00 | **0.00** |
| `m-v-topmargin-0` | y 0.00 | 72.00 | **0.00** |
| `m-v-bottommargin-0` | 770.00 | 72.00 | **770.00** |

**`h-leftmargin-0` is the row that hid this for as long as it was hidden**: the left margin band and
the sheet share a left edge, so a stated offset against `leftMargin` has always been right and only
an *alignment* inside the band shows the difference. `h-rightmargin-right` agrees by the mirror
accident. Reading offsets alone, this tree looked correct on 2 of the 4 horizontal band rows.

---

## 3. The corpus reach is three documents, and getting them right needed a second rule

**Census, `MANIFEST.tsv`'s 272 DOCX, both spellings:** `topMargin` appears **3 times in 3
documents** and `top-margin-area` in the same three, in the VML fallback of the same
`mc:AlternateContent`. `bottomMargin`, `leftMargin`, `rightMargin`, `insideMargin` and
`outsideMargin` appear **nowhere in the corpus at all**. The three are
`003`, `004` and `008 Free_Genogram_Diagram_Template`, each with one `wp:wrapSquare` title text box
stating `topMargin` and a 18.6 to 20.7 pt `wp:posOffset`.

**Landing §2 alone made all three worse**, which is what sent this round after the second rule
rather than shipping. One-attribute variants of `008`, rendered through 26.2.4.2 — the only
instrument that settles it — with the text box's text read off the PDF (`variants/`):

| the file says | 26.2.4.2 draws its title at | which is |
|---|---:|---|
| `relativeFrom="page"` | y 83.56 | page top + the 20.71 pt offset |
| `relativeFrom="margin"` | 155.56 | body top + the offset |
| `relativeFrom="topMargin"` *(as authored)* | **134.86** | **neither** |

134.86 is the page-relative placement **clamped to the body's own top**, and the clamp is
`SwAnchoredObjectPosition::ImplAdjustVertRelPos` (`anchoredobjectposition.cxx`:504-667). Two facts
about it were missing from this tree:

1. **`DisableOffPagePositioning` exempts a *wrap-through* object and nothing else.**
   `SwAnchoredObject::IsDraggingOffPageAllowed` (`sw/source/core/layout/anchoredobject.cxx`:790-801)
   returns `bDisablePositioning && bIsWrapThrough` — a conjunction. `PaginationOptions`'
   `CapturesAnchoredObjectsOnPage` had modelled it as the flag alone, so a DOCX frame stating
   `wp:wrapSquare` escaped a clamp the reference applies. That is also why the fixtures in §2, which
   are all `wp:wrapNone`, agree without it.
2. **Under `compatibilityMode` 15 the area is the body, not the sheet.** `ImplAdjustVertRelPos`
   :562-573 narrows `aPgAlignArea` from `rPageFrame.getFrameArea()` to the page body frame's, under
   its own comment: *"Instead of using the top of the page as the vertical limit, DOCX
   compatibilityMode 15 started to use the text body as the vertical limit for most paragraph or
   line-oriented anchored non-wrapthrough objects."* The guard is `bCompat15`
   (`!TAB_OVER_MARGIN && TAB_OVER_SPACING`), not wrap-through, not a split fly, and **a vertical
   relation that is neither `PAGE_FRAME` nor `PAGE_PRINT_AREA`** (:564-565) — which is exactly why
   `as-page` and `as-margin` above are unclamped and the authored `topMargin` is not.
   The horizontal half has no such branch: `ImplAdjustHoriRelPos` (:674-722) takes the page frame's
   rectangle unconditionally.

All three genograms state `compatibilityMode` 15. After both rules this tree answers **83.556,
155.556 and 134.856** against the reference's 83.557, 155.557 and 134.857 — 0.001 pt on all three
variants.

### The clamp is applied only to the margin bands, and that narrowing is measured

The C++ clamps *every* non-wrap-through content-anchored frame, whatever its origin. Applied that
widely to this tree it moves **10 of the 338** words renderings and is net **worse** against
26.2.4.2 (mean page ink at 100 dpi, before → after):

| document | before | wide rule | verdict |
|---|---:|---:|---|
| `003_Free_Genogram…` | 1.347 | **0.588** | better |
| `004_Free_Genogram…` | 2.302 | **0.868** | better |
| `008_Free_Genogram…` | 3.065 | **2.441** | better |
| `027_Unit_Circle_Chart_Graphical_Chart` | 12.652 | 11.540 | better |
| `1603642410-MoM-CASCOM-06-2020-draft04` | 9.347 | 9.446 | worse |
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 12.458 | 13.036 | worse |
| `Case-Study-Heathrow-Airport` | 18.181 | 19.186 | worse |
| **`023_Unit_Circle_Chart_Circular_Percentage`** | 10.820 | **16.524** | much worse |
| **`b053-19`** | 11.254 | **19.508** | much worse |

**The likely missing half is named rather than guessed at, and it is left with its seat.**
`SwToContentAnchoredObjectPosition::CalcPosition` passes `bCheckBottom = !DoesObjFollowsTextFlow()`
into the clamp (`tocntntanchoredobjectposition.cxx`:457), so a frame that follows the text flow has
its **bottom** correction skipped and only its top clamped — and `PROP_FOLLOW_TEXT_FLOW` is written
only for an anchor *inside a table* (`GraphicImport.cxx`:1316-1318, :1859-1861), so the pool default
decides it everywhere else. Both of the two documents that lose badly are ones a bottom clamp would
pull upwards. Establishing that is its own round.

Until then the capture is applied where this round measured it — a frame whose vertical origin is one
of the two margin bands — and nowhere else. **That regresses nothing by construction**, because those
two origins reached no frame that was placed before this round.

---

## 4. What moved

**Reach, byte for byte over every one of `MANIFEST.tsv`'s 947 paths**, rendered at the base binary
and at the changed one, `/CreationDate` masked (`sweep-ours.py`, `reach.txt`):

| track | renderings changed |
|---|---|
| words | **3** of 338 |
| slides | **0** of 302 |
| sheets | 1 of 307 |
| total | 4 of 947 |

The three words rows are exactly the three documents the census predicts. **The sheets row is the
wall clock and not the tree**: `PBN Matrix NAAs (V01).xlsx` prints a `&T` field in a page header and
the two sweeps ran sixteen minutes apart — its only difference is `20:14` against `20:30`, confirmed
by diffing the extracted text. It is the documented volatile class, and it is also why the first cut
of this comparison reported *947 of 947 changed*: without masking `/CreationDate` every PDF differs
by four bytes.

Slides and sheets could not have moved: the changed files are all in `XRay.Office.WordProcessing`, and
neither `XRay.Office.Presentations.csproj` nor `XRay.Office.Spreadsheets.csproj` references it. The
sweep is the measurement that agrees with that.

**Against 26.2.4.2 on the three movers** (`score-ink.py`, 150 dpi, `genogram-ink.txt`):

| document | before | after |
|---|---:|---:|
| `003_Free_Genogram_Diagram_Template_Easy_Format` | 1.371 | **0.521** |
| `004_Free_Genogram_Diagram_Template_Editable_Format` | 2.237 | **0.800** |
| `008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme` | 3.021 | **2.378** |

**No gate verdict moves.** The reference half was rendered at 26.2.4.2 for the three batches the
movers sit in and for all ten documents the wide rule touched, and both our sweeps were scored
against it with `batch-check.sh`'s own rule (`score-gate.py`): `MATCH 30 of 30` and `MATCH 10 of 10`
before and after, **0 verdicts moved**. The other 917 renderings are byte-identical, so no verdict
outside those can move either — which is why no whole-corpus reference sweep was rendered, and is
worth more than one would have been: a contended reference sweep undercounts on the reference side
and reads exactly like our regression.

---

## 5. Reproducing

```sh
export XRAY_OFFICE_CLI=/abs/tree/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli

# the 34 fixtures
python3 make-relfrom.py    /abs/scratch/rf
python3 measure-relfrom.py /abs/scratch/rf /abs/scratch/rfout

# the two briefed defects, at their own probes' fixtures
python3 ../words-margin-print-area/makeprobe.py /abs/scratch/mpa
python3 ../words-margin-print-area/measure.py   /abs/scratch/mpa /abs/scratch/mpaout
python3 ../words-inline-effectextent/make-x-fixture.py /abs/scratch/xfx
python3 ../words-inline-effectextent/measure-x.py      /abs/scratch/xfx /abs/scratch/xout
python3 catalogue-x.py /abs/scratch/cat all

# the corpus witness the brief named
TAG=head XRAY_OFFICE_CLI=$XRAY_OFFICE_CLI python3 ../words-vml-fontwork/measure.py /abs/scratch/vml

# reach and score
python3 sweep-ours.py /home/user/sample-files 'words/*' /abs/scratch/after 3
REF_SOFFICE=/opt/libreoffice26.2/program/soffice \
  python3 sweep-ref.py /home/user/sample-files 'words/chartset-005' /abs/scratch/ref 2
python3 score-gate.py /abs/scratch/ref/ref /abs/scratch/before/ours /abs/scratch/after/ours
python3 score-ink.py  ref.pdf before.pdf after.pdf 150
```

`measure-relfrom.py`, `catalogue-x.py` and `score-ink.py` need Pillow and poppler; every script that
renders a reference resolves `$REF_SOFFICE` and prints the binary's own `--version` before its table,
so a stored run says which of the two installed LibreOffices produced it.

## 6. Instrument notes

* **A byte comparison of two of our own sweeps needs `/CreationDate` masked.** Without it the answer
  is *everything changed*, which is not obviously wrong until you diff one file. `SOURCE_DATE_EPOCH`
  on both runs is the other way to get it, and is what to use when the comparison is the point.
* **The ink metric is a distance, so it is only meaningful against a fixed reference render.**
  Scoring `before` and `after` against the same reference PDF is what makes `+5.704` mean *worse*
  rather than *different*; comparing `before` with `after` directly cannot say which is closer.
* **A raster band is a better instrument than `pdftotext -bbox` for a frame's position**, because the
  band has no font. Where the text had to be read instead — the corpus variants in §3, whose text box
  has no fill — the three variants share a substituted face, so the *differences* between them are
  sound even though the absolute `yMin` comes from the font descriptor.
* **`wp14:sizeRelH`/`sizeRelV` is unread and its reach is three documents.** 1873 `sizeRel*`
  elements across 146 of the 272 DOCX, of which exactly **three carry a non-zero percentage** —
  `ABCD-FE-01-00 Flight Envelope` a width, `HC-Bulletin-template` and `fleetfastfacts16nov2023` a
  height. `GraphicImport.cxx`:1455-1480 is the seat, and it maps `topMargin` there to
  `PAGE_PRINT_AREA_BOTTOM` rather than to `_TOP`, which is worth a second look before anyone
  implements it. Left.
