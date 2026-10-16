# `words-close-r95` — the frame capture's missing half, and a pool parent that is a *reference*

Measured 2026-09-11 in `/home/user/wt-wordsclose` (branch `agent/wordsclose`, base `4ac492420`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`**, printed by every script that renders one. The banked
reference renderings were reused rather than re-rendered: `/home/user/gate-orig-r83/ref` for the
947 originals and `/home/user/gate-odf-r80/ref` for the converted corpus.

**There is no `Task`/subagent tool in this container**, so nothing here rests on a page this agent
read. Every claim below is a number an instrument produced, and the two that could have been read
off an image — *the reference does not clamp that chart* and *the two genogram templates got
worse* — are settled by arithmetic instead: a saturating clamp and a byte comparison.

The box was under load throughout and the load moved: `uptime` reported a one-minute average of
**3.1** while the base sweep ran and **7.3** while the head sweep did, which took that sweep from
about 100 renders a minute to 20 and back to 100 when the other rounds finished. Nothing here is a
gate verdict, so the *contention undercounts on the reference side* trap cannot reach it — and the
two sweeps that are compared byte for byte were both taken with `SOURCE_DATE_EPOCH` fixed, so a
slow leg and a fast one produce the same bytes.

---

## 1. O8 — the frame capture, and the two halves that were missing

### 1.1 What was open

`frame-area-r85` §3 applied the DOCX capture to a frame stated against one of the two *margin
bands* and nowhere else, because the wide form — every non-wrap-through content anchor — moved 10
of the 338 words renderings and was net worse: three genogram templates improved and `b053-19`
went 11.254 → 19.508 of mean page ink, `023_Unit_Circle_Chart_Circular_Percentage` 10.820 →
16.524. `words-seat-r94` §2 then **refuted** the suspect that round had named (`bCheckBottom =
!DoesObjFollowsTextFlow()`, whose pool default makes the term drop out) and named two others from
the source: the `bConsidered` asymmetry, and *the area*.

### 1.2 The rule, as three facts rather than one

`SwAnchoredObjectPosition`'s constructor
(`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:125-144):

```cpp
if (mbIsObjFly)  bConsidered = bWrapThrough && !bTextBox;   // a picture, an OLE object, a text frame
else             bConsidered = bWrapThrough || !bTextBox;   // a drawing shape or a group
mbDoNotCaptureAnchoredObj = bConsidered && !mbFollowTextFlow && DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE;
```

* **(a) A shape with no text box is never captured, whatever its wrap.** This tree's remark said
  the escape was wrap-through and nothing else, which is wrong for a third of the object kinds.
* **(b) The area is the page *body*, not the sheet**, under `compatibilityMode` 15 and for every
  vertical relation but `PAGE_FRAME` and `PAGE_PRINT_AREA` (`:552-573`).
* **(c) …and only where there is a body to narrow to.** `mpAnchorFrame->FindBodyFrame()` walked up
  to a page body frame whose upper is *this* page (`:568-573`). **A header, a footer and a footnote
  anchor have none.** That is the half neither of the two previous rounds had, and it is what the
  wide rule cost the most on.

`mbFollowTextFlow` is deliberately still not modelled, for `words-seat-r94`'s own reason: its pool
default is false and every writerfilter write of it is gated on the anchor being inside a table, so
outside a table the term drops out — and inside one the object is captured in its **cell**
(`:576-591`), which is a different area this tree does not have. 552 of 6055, 40 of 272 documents.

### 1.3 Measured at the reference, on 93 one-attribute fixtures

`make-capture.py` writes one A4 page per fixture carrying a single red band — the only coloured
ink on it, so the band's own rectangle separates from the text with no assumption about the text.
Three object kinds (`shape` a bare `wps:wsp`, `text` a `wps:wsp` with a `wps:txbx`, `pic` a
`pic:pic`) × five wraps × six positions, plus a `compatibilityMode` 14 twin of the position that
tests the narrowing. `measure-capture.py` renders both sides and reads the band off the raster at
288 dpi.

| | agrees with 26.2.4.2 |
|---|---:|
| at `4ac492420` | **79** of 93 |
| after | **91** of 93 |

The two that are neither are `pic-*-hdr`, where **both sides draw no band at all**: the fixture
puts a `pic:pic` in `word/header1.xml` and writes no `word/_rels/header1.xml.rels`, so the
`r:embed` resolves for nobody. They are reported as unscoreable rather than as agreement; the
`shape` and `text` kinds cover the same position.

The twelve rows that moved are exactly the twelve where a **fly or a text-box shape is not
wrap-through**, and the table reads as the rule:

| fixture | 26.2.4.2 | before | after | what it says |
|---|---:|---:|---:|---|
| `pic-square-hright` | x 495.25 | 555.25 | **495.25** | a fly is captured across the sheet |
| `text-square-hright` | 495.25 | 555.25 | **495.25** | and so is a shape with a text box |
| `shape-square-hright` | **555.25** | 555.25 | 555.25 | and a shape with none never is — (a) |
| `pic-square-vbelow` | y 801.75 | 821.75 | **801.75** | the same rule down the sheet |
| `shape-square-vbelow` | **822.00** | 822.00 | 822.00 | (a) again, in the other axis |
| `pic-square-vbody-c15` | 71.75 | 20.00 | **71.75** | the area is the body — (b) |
| `pic-square-vbody-c14` | **20.00** | 20.00 | 20.00 | …under `compatibilityMode` 15 alone |
| `shape-square-vbody-c15` | **20.00** | 20.00 | 20.00 | (a) beats (b) |
| `text-square-hdr` | **36.00** | 36.00 | 36.00 | a header anchor is not narrowed — (c) |
| `text-none-*` | unmoved | | | wrap-through is exempt, as it always was |

`hleft` and `vabove` — a band hanging off the sheet's *left* or *top* — are in the fixture set and
carry **no information**, because the page clips the part that would have moved and the drawn
corner is 0.00 either way. The first cut of this probe read twenty-five such rows as agreement.
`hright` and `vbelow` are the discriminating form and were added for that reason.

**Two instrument notes, both of which cost this round time.** A fresh
`-env:UserInstallation` per document costs more than the conversion, and under contention more
than a 240 s bound: the first cut timed out on document one and reported *no band*, which reads
exactly like the reference declining to draw the object. And
`-env:UserInstallation=file://relative/path` is a URL whose **host** is the first segment, so
`soffice` silently falls back to the shared profile — where another round holds it, the process
sleeps for ever at 0 % CPU with no output and no error.

### 1.4 What moved on the corpus

Our half of the whole 947-document corpus and of the 676 converted words documents, rendered at
`4ac492420` and again at this round's HEAD, `obj`/`bin` cleared per leg, the restore done with
`cp` and an explicit `touch`, both legs under `SOURCE_DATE_EPOCH` — so a byte difference is a real
difference. `movers.py`, with the mover list and the per-document ink in `reach.txt`.

| corpus | renderings compared | differ |
|---|---:|---:|
| `/home/user/sample-files` — words 338, slides 302, sheets 307 | 947 | **4**, every one a `.docx` |
| `/home/user/corpus-odf/words` — `.odt` 338, `.rtf` 338 | 676 | **0** |
| total | **1623** | **4** |

Scored against the banked 26.2.4.2, mean per-page absolute grey difference at 150 dpi — a
*distance*, so it can fall as well as rise:

| document | before | after | |
|---|---:|---:|---|
| `027_Unit_Circle_Chart_Graphical_Chart` | 12.396 | **11.265** | better |
| `ABCD-SDE-23-00 — Avionic System Description` | 4.979 | 4.978 | level to three decimals |
| `021_Unit_Circle_Chart_3D_Pie_Chart` | 12.089 | 12.663 | worse |
| `023_Unit_Circle_Chart_Circular_Percentage` | 10.718 | **16.424** | much worse |
| **total over the four** | **40.182** | **45.331** | **+5.149** |

**The three genogram templates do not move at all**, because r85's narrow rule already reached
them: they are the only corpus documents stating `topMargin`, and their capture is unchanged.
**`b053-19` and `Case-Study-Heathrow-Airport` do not move either** — which is the whole of §1.2(c),
since the wide rule cost them 8.25 and 1.09 of ink. Neither does
`1603642410-MoM-CASCOM-06-2020-draft04`, the fourth of r85's ten.

**No gate verdict can move on the 943 that are byte-identical**, and the four that moved add no
glyph and no page — measured rather than assumed, alphanumeric characters through `pdftotext` and
pages through `pdfinfo`:

| document | glyphs before / after / reference | pages |
|---|---|---|
| `021_Unit_Circle…3D_Pie_Chart` | 549 / 549 / **549** | 1 / 1 / 1 |
| `023_Unit_Circle…Circular_Percentage` | 550 / 550 / **550** | 1 / 1 / 1 |
| `027_Unit_Circle…Graphical_Chart` | 1557 / 1557 / **1557** | 1 / 1 / 1 |
| `ABCD-SDE-23-00` | 43746 / 43746 / 43748 | 29 / 29 / 29 |

So the gate is blind to this in both directions. That is the `w:pgBorders` shape again, and it is
why the round is scored on pixels.

### 1.5 The three `Unit_Circle` documents, and why the residual is not the capture

Two of the three get **worse**, and `words-seat-r94` warned that they could not be scored off the
capture alone because `ChartLayout`'s own fit confounds them. They can now be scored, and the
finding is sharper than the warning: **the frame lands in exactly the same place on both sides,
and the residual is entirely inside the chart.**

`chart-variants.py` changes one attribute of `023`'s chart anchor at a time and renders each
through 26.2.4.2 (`chart-variants.txt`):

| variant | 26.2.4.2, words drawn | this tree |
|---|---|---|
| `base` — `positionH relativeFrom="margin" posOffset="-221.35pt"` | 107 words, mean x 262.76 | 107, 267.65 |
| `offset` — the offset alone, −221.35 pt → −100 pt | **identical to `base`** | **identical** |
| `page` — `relativeFrom` alone, `margin` → `page` | **identical to `base`** | **identical** |
| `wrapnone` — the wrap alone, `wrapSquare` → `wrapNone` | mean x 260.26 | 260.42 |

The chart is **682.1 pt wide on a 595.3 pt page**, so `ImplAdjustHoriRelPos` corrects the right
edge to `595.3 − 682.1 = −86.8` and then the left edge to `0`: the clamp **saturates**, and the
stated offset cannot reach the drawn position at all. Three different stated positions giving one
rendering is what says the reference is clamping — an unclamped chart would have moved 121.35 pt
between `base` and `offset` — and this tree now saturates at the same value. `wrapnone` moves on
both sides, which is the control that says the instrument is live.

So the frames coincide and the ink that is left is the chart's own layout inside it: the mean word
x differs by 4.9 pt, and the eight labels that move are the pie's data labels, which the fit
places. It is the class `CLAUDE.md` already records against `probes/chart-fit` — *the fit makes
our own label-arrangement errors visible as a global squeeze instead of as local overflow* —
arriving at a frame position that is now right. Recorded as its own register entry rather than
worked here.

### 1.6 What this does *not* claim

* **It is not the whole of `ImplAdjustVertRelPos`.** A split floating table (`GetFlySplit()`) and a
  `w:framePr` text box (`TextBoxIsFramePr`) are two more exclusions from the narrowing, and this
  tree builds a `PageFrame` for neither, so neither can be reached. `w:framePr` appears nowhere in
  `dotnet/src`.
* **tdf#123002 is modelled and is not measured.** A header- or footer-anchored object whose top has
  already passed the area's bottom is returned unadjusted (`:637-647`). No fixture here reaches it
  — the `hdr` band is above the body, not below the page — and no corpus census can see it, because
  it is a property of a computed position rather than of the markup. It is in because leaving it
  out would clamp an object the reference leaves alone, which is the direction that regresses.
* **The RTF reader is untouched by it.** `PaginationOptions.CapturesAnchoredObjectsOnPage` is true
  there, so RTF captures everything and never reaches the new branch — which keeps
  `probes/rtf-shape-r73`'s 21 measured probes exactly where they were.

---

## 2. O9 — `Body Text` and `caption` track the document's own `Normal`

### 2.1 What was open

`rtf-bookmark-r88` §4 measured it and left it: `COLL_TEXT` and `COLL_LABEL` have `COLL_STANDARD`
for a pool parent (`sw/source/core/doc/poolfmt.cxx`:201-204, :229-235), so a style named
`Body Text` or `caption` whose `\sbasedon` does not resolve inherits **the document's own `Normal`
entry** rather than a constant — 10 pt under `{\s0 … \fs20 Normal;}` and 14 under `\fs28`, where
`heading 4`, `Title` and `Subtitle` answer 14 to both. It was left because *"it needs the whole of
`ConvertStyleName`'s two hundred names to be safe"*.

### 2.2 The census says where the boundary is, and it is three names

`standard-census.py` asks r88's question of *all* the names rather than a list of five: every
paragraph style the 338 converted `.rtf` apply, with no resolvable `\sbasedon`, counted outside the
`{\stylesheet}` group so that an entry's own `\sN` does not make every declared style look used.

**84 distinct names**, of which the ones whose `ConvertStyleName` name reaches an existing Writer
style with `COLL_STANDARD` somewhere above it are:

| name | documents | pool style | pool parent |
|---|---:|---|---|
| `Body Text` | **2** | `Text body`, `COLL_TEXT` | `COLL_STANDARD` — **done** |
| `caption` / `Caption` | **0** | `Caption`, `COLL_LABEL` | `COLL_STANDARD` — **done, nil reach** |
| `header`, `footer` | 5 each | `Header`, `Footer` | `COLL_HEADERFOOTER`, then Standard |
| `toc 1`, `toc 2`, `toc 3` | 2, 1, 1 | `Contents N` | `COLL_REGISTER_BASE`, then Standard |
| `Heading` (bare) | 1 | `Heading`, `COLL_HEADLINE_BASE` | `COLL_STANDARD`, but see below |
| `Figure` | 1 | `Figure`, `COLL_LABEL_FIGURE` | `COLL_LABEL`, then Standard |

**The six that are left out are left out for two stated reasons, not for caution.** `header`,
`footer`, `toc 1`…`toc 3` and `Figure` reach `COLL_STANDARD` only through an *intermediate* pool
style — `COLL_HEADERFOOTER`, `COLL_REGISTER_BASE`, `COLL_LABEL` — and unlike the entry's own
properties those intermediates' are **not** reset by the import, so each needs its own measurement
rather than this branch. A bare `Heading` is a different gap again: it is not in
`ConvertStyleName`'s map at all, and what makes Writer reuse its style is the second lookup,
`xStyles->hasByName` on the *unconverted* name (`StyleSheetTable.cxx`:1099-1121) — a set this round
did not census and whose members are Writer's UI names rather than Word's.

The control family is every name the map answers with an **empty** Writer name — `Quote`,
`List Paragraph` and `Normal (Web)` (`StyleSheetTable.cxx`:1794, :1883-1884) — which keeps
`\pard\plain`'s twelve points and must not move. `ANameWriterHasNoStyleForKeepsTheResetSize` is
that control as a test, and `standard-own-size.txt` measures `Quote` at the reference.

### 2.3 The fix, and why it is not a constant

`RtfStyles.PoolFormattingOf` answered a `RtfStyleFormatting?` — a constant or nothing — which
cannot express *Standard*. It is now `PoolParentOf`, answering `RtfPoolParent.{None, Heading,
Standard}`, and the `\sbasedon` walk **continues into style 0** for `Standard` instead of folding a
constant in. RTF fixes the default style at index 0 (`rtfdispatchflag.cxx`:600-614, *"By default
the style with index 0 is applied"*), and `Normal` itself answers `None`, so nothing can inherit
from itself.

`RtfStyleFormattingTests.APoolStyleUnderStandardIsNotModelledYet` — the test r88 wrote to pin the
gap — becomes `APoolStyleUnderStandardTakesTheDocumentsOwnNormal`, five cases over the two
`Normal` sizes, because **one `Normal` cannot tell a pool constant from inheritance** and that is
what round 87 read the other way. `ANameWriterHasNoStyleForKeepsTheResetSize` is the control.

### 2.4 Reach is nil, and the reason is a second rule meeting the first

`caption` is **0 of 338**: no `.rtf` in the converted corpus applies it. It is implemented anyway,
because it is the same branch as `Body Text` and certain from the map — in, as round 88 put it of
`Title`, because it is right and free.

`Body Text` is 2 of 338 and **neither of the two moves**, which is not luck. Both entries state an
`\fs` on themselves:

```
{\s0 …\fs20 Normal;}  {\s415 …\fs20 Body Text;}      Press release_EUREKA labels ITEA 3 Cluster
{\s0 …\fs24 Normal;}  {\s… …\fs24 Body Text;}        CRIF - Spécification technique - Socle applicatif
```

and round 87's trap is that **a style's own `\fs` reaches no paragraph** — `getDefaultSPRM` writes
the reset back over it — so a style that states one gets neither its own size nor its parent's.
That was measured for a heading; `genstandard.py` measures it here, four arms × the two `Normal`
sizes, rendered through 26.2.4.2 and read with `rtf-bookmark-r88/readpool.py`
(`standard-own-size.txt`):

| the `Body Text` entry states | `Normal \fs20` | `Normal \fs28` |
|---|---:|---:|
| nothing | **10 pt** | **14 pt** |
| `\fs28` of its own | **12 pt** | **12 pt** |
| `caption` stating `\fs28` | 12 pt | 12 pt |
| `Quote` stating `\fs28` (control) | 12 pt | 12 pt |

**8 of 8, this tree and the reference.** So the rule is implemented, verified on ten probes across
the two rounds, pinned by five test cases — and its corpus reach is **nil**, for a reason that is
itself measured rather than assumed. The `.rtf` column of §1.4's sweep is the corroboration:
338 renderings, 0 differ.

---

## 3. Confinement

The diff is six source files, all under `XRay.Office.WordProcessing`. Neither
`XRay.Office.Spreadsheets.csproj` nor `XRay.Office.Presentations.csproj` references that project, so
no sheet and no slide can be reached by construction — and the sweep is the measurement that
agrees with it: **302 of 302 slides and 307 of 307 sheets byte-identical**, and 334 of the 338
words originals.

Shown rather than asserted, and with the traps the rulebook names taken: `obj`/`bin` cleared on
each leg, the restore done with `cp` and an explicit `touch` (never `mv`, which leaves MSBuild's
up-to-date check skipping the project), both legs under `SOURCE_DATE_EPOCH`, every one of the
3246 PDFs checked for `%%EOF`, and the newest render of each leg checked against the binary's own
mtime — the base sweeps end at 08:14 and 08:20 and the changed binary was built at 08:21, so no
row of either sweep predates its own binary.

## 4. The suite

The ten non-fidelity projects were run one at a time: 521, 109, 728, 309, 164, 259, 146,
1885, 1245, 1044 — all green, and `XRay.Office.WordProcessing.Tests` is 1885 against 1870 before
this round's fifteen new cases.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed**, and the ten are exactly the names the
rulebook lists: `PageDrawingComparisonTests.EveryLineIsDrawn` ×4,
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`
and `JustificationShrinkComparisonTests`. **The four TabStop cases were checked rather than
assumed**: they are `list-label-overrun` in four formats and not one of the four holds an anchored
object or an unresolved style name, so nothing in this diff can reach them; they fail identically
before and after.

`FrameMarginBandTests.OnlyAWrappedBandIsPulledIntoTheBody` **did** fail, and it was the test that
was wrong. It asserted r85's `008` genogram answer on a bare `wps:wsp`, where that document's box
carries a `wps:txbx` — so it had the right number for an object the reference does not capture at
all. It is now `OnlyAWrappedTextBoxIsPulledIntoTheBody`, seven cases over both object kinds, with
the fixture writing the text box.

## 5. Files

| file | what it is |
|---|---|
| `make-capture.py` | the 93 one-attribute DOCX fixtures: three object kinds × five wraps × six positions |
| `measure-capture.py` | both sides rendered and the red band read off the raster; `REUSE_REF=1` scores a second binary against a reference half already rendered |
| `capture-before.txt`, `capture-after.txt` | those 93 at `4ac492420` and at HEAD |
| `capture-census.py`, `capture-census.txt` | every positioned object in the 272 DOCX by kind and wrap, and the four the `bConsidered` asymmetry decides |
| `chart-variants.py`, `chart-variants.txt` | one attribute of `023`'s chart anchor at a time, which is what shows the clamp saturating |
| `standard-census.py`, `standard-census.txt` | every paragraph style the 338 `.rtf` apply without a resolvable `\sbasedon` |
| `genstandard.py`, `standard-own-size.txt` | eight RTF probes: what a *Standard*-parented name answers when the entry states its own `\fs` |
| `movers.py`, `reach.txt` | the two sweeps compared byte for byte, and each mover's distance from the banked reference |

Reused rather than rebuilt: `probes/frame-area-r85/sweep-ours.py` and `score-ink.py`, and the
banked references `/home/user/gate-orig-r83/ref` and `/home/user/gate-odf-r80/ref`.

## 6. Reproducing

```sh
export XRAY_OFFICE_CLI=/abs/tree/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export REF_SOFFICE=/opt/libreoffice26.2/program/soffice

# the 93 fixtures, both sides
python3 make-capture.py    /abs/fx
python3 measure-capture.py /abs/fx /abs/capwork 288

# the same fixtures against a second binary, reusing the reference half
REUSE_REF=1 python3 measure-capture.py /abs/fx /abs/capwork2 288

# the chart that looked like a counter-example
python3 chart-variants.py /home/user/sample-files/words/chartset-010/docx/023_*.docx /abs/cv023

# reach and confinement
python3 ../frame-area-r85/sweep-ours.py /home/user/sample-files '*' /abs/base 3
python3 ../frame-area-r85/sweep-ours.py /home/user/corpus-odf/words '*' /abs/base-odf 3
python3 movers.py /abs/base/ours /abs/head/ours /home/user/gate-orig-r83/ref 150

# the censuses
python3 capture-census.py  /home/user/sample-files/MANIFEST.tsv
python3 standard-census.py /home/user/corpus-odf/words

# the eight RTF pool probes
python3 genstandard.py /abs/std
timeout -k 30 900 "$REF_SOFFICE" --headless -env:UserInstallation=file:///abs/std/prof \
    --convert-to pdf --outdir /abs/std/ref /abs/std/*.rtf
python3 ../rtf-bookmark-r88/readpool.py /abs/std/ref
```

`measure-capture.py`, `movers.py` and `chart-variants.py` need PyMuPDF's neighbours rather than
PyMuPDF: NumPy, Pillow and poppler. Nothing here renders a reference except `make-capture.py`'s
measurement pass and `chart-variants.py`, and both bound their own `soffice` with `timeout -k`.
