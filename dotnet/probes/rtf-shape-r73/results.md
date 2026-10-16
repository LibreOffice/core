# `rtf-shape-r73` — a shape's wrap, its origin, its inset, its page, and the table inside it

## Environment

    ours   = XRay.Office.Cli @ 97628b7fe (base) and @ ff61dd5ad (after), Debug
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 (TDF tarball),
             all four font confounds aside — 38 faces in .duplicates-aside, 8 in .noto-aside,
             no LiberationSansNarrow anywhere
    gate   = /home/user/gate-odf-rows.tsv (26.2.4.2) for the .rtf column,
             /home/user/gate-2f47 (24.2.7.2) for the original words track
    rule   = batch-check.sh of 2026-09-05: pages, then alphanumeric characters within
             max(2%, 15), then unembedded fonts

Both gates' reference halves were reused rather than re-rendered: the whole diff is confined
to `dotnet/src` and cannot reach `soffice`. Every 26.2.4.2 rendering below was made fresh,
one `soffice` profile per document keyed on an md5 of the document's own path.

---

## The result

| | match | of | |
|---|---:|---:|---|
| `.rtf`, at `97628b7fe` | 216 | 328 | 65.9% |
| `.rtf`, after the wrap/origin/inset/capture commit | 233 | 328 | 71.0% |
| `.rtf`, after the frame-table commit | **239** | 328 | **72.9%** |
| the original words track, before and after | 315 | 338 | **not one column of one row moved** |

**23 rows to `match`, none away from it**, and six more improved without crossing: five went
`pages,words` → `words` and `info-bulletin-601` went `pages,words` 63 pages → `pages` 7, against
the reference's 6.

---

## The brief's premise was wrong at the seat, and the probe that refutes it is the same probe

The brief named `RTFSdrImport::resolve` mapping only `posrelh == 1`
(`sw/source/writerfilter/rtftok/rtfsdrimport.cxx`:696-717) as the reason `{\shptxt}` is clipped,
and asked which frame of reference each value names. Nine files — the property absent and every
value 0 to 7 — on a page with a **2 inch left margin and a 1 inch paragraph indent**, so the page,
the body area and the indented column are three different origins:

| `posrelh` | 26.2.4.2 draws the shape's text at | ours, at the round's base |
|---|---|---|
| 1 | **50.1 pt** — the page's own left edge plus the shape's 1000 twips | 50.0 |
| absent, 0, 2, 3, 4, 5, 6, 7 | **194.1 pt** — the *body area's* left edge plus the same 1000 | 194.0 |

**We already agreed with 26.2.4.2 on nine of nine before this round touched anything.** The
mapping the reader had — 0 the margin, 1 the page, everything else the column — gives the
reference's answer at every value, because a single-column section's column *is* the body area.
So `posrelh` was not the fifteen documents' defect and mapping "the rest" was not the fix.

What `posrelh`'s values actually mean, from LibreOffice rather than from the enumeration's names:

- **`RTFSdrImport::resolve` has a case for 1 and no other**, and 1 is
  `text::RelOrientation::PAGE_FRAME` — the sheet, not its print area.
- **Every other value keeps the relation the shape was *created* with.** A top-level `shapeType`
  1 (rectangle) or 202 (text box) becomes a `com.sun.star.text.TextFrame`
  (`rtfsdrimport.cxx`:323-334), and `getTextFrameDefaults` (:111-124) sets its
  `HoriOrientRelation` and `VertOrientRelation` to `RelOrientation::FRAME`. `FRAME` is the
  *anchor's own frame*: horizontally the body column, vertically the anchor paragraph.
- So **0 is not the page margin**, although MS-ODRAW's own names for these values invite it, and
  0 and 3 place a shape identically. Five vertical probes with the shape anchored in the *second*
  paragraph confirm the other axis: `posrelv` 1 puts the text at 50.4 pt and absent, 0, 2 and 3
  all put it at 134.0, which is that paragraph's own top plus the same 1000 twips.

The reader now maps 1 and folds every other value into the frame, which is what the import does.
**Corpus reach of that correction is nil and the census says so**: the `.rtf` column states
`posrelh` 3 3536 times, 2 201 times, 1 33 times and **0 not once**.

---

## What the fifteen actually were: three defects, and none of them the origin

`xray-office extract` against our own rendering had already separated *drawn short, read whole*
from the rest; what it could not say is why. Rendering all fifteen through 26.2.4.2 and
differencing the two word *sets* — rather than the counts — splits them immediately.

### 1. `\shpwr` — three of its five values were read from the wrong table

`RtfLayoutFrame.Wrap`'s own remark gave RTF's specification numbering (*1 around, 2 tight,
3 through, 4 top and bottom, 5 none*). LibreOffice's dispatch is a different function
(`rtfdispatchvalue.cxx`:1222-1247) and the corpus is LibreOffice's own export, so the dispatch
is the only reading that can be right about it:

| `\shpwr` | LibreOffice sets | this reader had | 26.2.4.2's first prose line, measured |
|---|---|---|---|
| 1 | `WrapTextMode_NONE` | parallel | x = 72.1, **y = 132.4** — below the box |
| 2 | `PARALLEL` | parallel | x = 272.1, y = 72.4 — beside it |
| **3** | **`THROUGH`** + `wrapNone` | **parallel** | x = 72.1, y = 72.4 — over it |
| 4 | `PARALLEL` + `wrapTight` | top and bottom | x = 272.1, y = 72.4 |
| 5 | `THROUGH` | through | x = 72.1, y = 72.4 |
| *absent* | nothing — the fly default | **top and bottom** | x = 272.1, y = 72.4 — beside it |

**3 is the expensive one: 1341 occurrences in 178 of the 338 `.rtf`.** Reading a through shape as
an obstacle narrows every line beside it, which is a pagination difference on a third of the
column. The *absent* row cost as much per document and reached fewer: the field defaulted to 1,
so a shape stating no `\shpwr` took the one value that puts no text beside it at all.
`rtfsdrimport.cxx`:1091 is the guard that makes absent mean the fly default — it sets `Surround`
only when the shape's wrap is not still the `WrapTextMode_MAKE_FIXED_SIZE` sentinel.

### 2. The shape is captured on its page, in both axes

`PaginationOptions.CapturesAnchoredObjectsOnPage` was **off** for RTF, on the same grounds as for
DOCX: `sw/source/writerfilter/filter/WriterFilter.cxx`:332 sets `DoNotCaptureDrawObjsOnPage` for
every writerfilter import. It does — and the shapes are captured anyway, because they are not
drawing objects. `createShape` makes them Writer **flies**, and a fly is clipped onto its page by
`SwFlyFreeFrame::CheckClip` (`sw/source/core/layout/flylay.cxx`:471-545), which that flag does not
reach.

Twenty-one probes settle the rule, and it has no exceptions in any of them:

| case | shape offered | 26.2.4.2 draws it at | rule |
|---|---:|---:|---|
| overflows right, `\shpwr` 1–5 | 3730 twips, 10723 wide | **1183** | `paperw − width` |
| begins entirely off the sheet | 15139 twips, 3000 wide | **8906** | the same, from off-page |
| overflows left, `\shpwr` 1–5 | −861 twips | **0** | the page's own left edge |
| below the sheet | 19440 twips down | **16238** | `paperh − height` |
| above it | −560 twips | **0** | the page's own top |
| fits, `\shpwr` 1–5 | 3730 twips | 3730 | untouched |

**The wrap does not gate it and neither does the text.** Two of the five values are wrap-through,
and `SwAnchoredObject::IsDraggingOffPageAllowed`
(`sw/source/core/layout/anchoredobject.cxx`:790-801) would let a wrap-through object off the page
— it fires on none of them. A box with `{\shptxt}` and one without are clamped alike, read off the
PDF's path operators.

`FrameLayout` had only the vertical half of the capture. The horizontal half is
`SwAnchoredObjectPosition::ImplAdjustHoriRelPos`
(`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:674-721), which sits beside
`ImplAdjustVertRelPos`, is reached through the same `mbDoNotCaptureAnchoredObj` guard
(`anchoredobjectposition.hxx`:268-273), and corrects **the right edge first and the left
afterwards** — so a frame wider than the page ends flush with the *left*, the mirror of the
vertical order.

**This is what the seven `Unit_Circle_Chart` documents were.**
`026_Unit_Circle_Chart_Four_Quadrants` states `\shpleft591\shpright11314` against a 3139-twip left
margin, so its 10723-twip box is offered 3730 and would end 2547 twips past an 11906-twip sheet.
26.2.4.2 draws it at 1183 and the paragraph inside it fits; we drew it at 3730 and every one of its
lines ran off the sheet and lost its last word — `Lorem … aut p`, `… cupiditate`, `… inventor`.
Read as a word count that is *drawn short, read whole*; read as a page it is one paragraph 120.1 pt
too far right.

### 3. A shape's text is inset, and the default is not zero

`dxTextLeft`, `dyTextTop`, `dxTextRight` and `dyTextBottom` are EMUs divided by 360 into the
frame's four `*BorderDistance` properties (`rtfsdrimport.cxx`:600-625); the reader read none of
them. **And the default when a shape states none is not zero** — `getTextFrameDefaults` gives
`91440 / 360` = 254 (1/100 mm) across and `45720 / 360` = 127 down, 0.1 inch and 0.05 inch. On the
wrap probe, which states none, the reference draws `ZZQ` at x = 79.3, y = 86.04 and we drew it at
72.0, 82.44 — exactly 7.2 and 3.6 pt out. Zero and absent are two different answers, as they
already are for the wrap distance, and LibreOffice's own export writes all four on every shape it
emits.

### After the three: nine of the fifteen draw every word the reference draws

| document | ref words | ours before | ours after |
|---|---:|---:|---:|
| `037_Venn_Diagram_Template_Four_Circle` | 20 | 16 (2 pages) | **20** |
| `038_Venn_Diagram_…Off_White_and_Blue` | 40 | 33 (2 pages) | **40** |
| `025_Unit_Circle_Chart_Cos_and_Sin_Model` | 105 | 95 | **105** |
| `030_Unit_Circle_Chart_Points_System` | 80 | 72 | **80** |
| `026_Unit_Circle_Chart_Four_Quadrants` | 52 | 45 | 53 |
| `022_Unit_Circle_Chart_Circle_A_and_B` | 195 | 147 | **195** |
| `023`, `021`, `024` (Unit Circle) | 94, 95, 94 | 89, 91, 77 | 105, 104, 95 |

The three that end *above* the reference are the `drawn-long` class the previous round already
separated out and are not this defect.

---

## The other six: a table inside `{\shptxt}` was dropped whole

`FinishTable` (`Rtf/RtfDocumentReader.Tables.cs`) named three destinations for a finished table —
the body's block list, a note's, and a running head's or foot's — and **not the frame's**, while
`RecordLayoutParagraph` twenty lines away already named all four. A `{\shp}` whose text is a table
therefore put every cell's paragraphs on the table level, found nowhere to hand the table when
`\row` closed it, and drew nothing at all in its place.

LibreOffice's own RTF export is where these come from: it writes a Writer text frame's content
verbatim, so a boxed table comes back as `{\shptxt\trowd…\cellx…\intbl…\row}` — with the shape's
own geometry words replayed into each cell paragraph's `\pard`, which is why `\shpleft9404`
appears eight times in a document with one such shape.

**20 such groups in 11 of the 338 `.rtf`** (`census.py shptbl`). The six that were failing on it:

| document | ref words | ours before | ours after |
|---|---:|---:|---:|
| `043_Visual_Product_Roadmap_Template_Customizable_Format` | 184 | **6** | 186 |
| `019_Project_Timeline_Template_Excellent_Layout` | 98 | **14** | 101 |
| `093_Business_Case_Template_Customizable_Layout` | 34 | **2** | **34** |
| `090_Business_Case_Template_Blue_Theme` | 47 | 24 | **47** |
| `095_Business_Case_Template_Easy_Format` | 74 | 55 | **74** |
| `067_Work_Breakdown_Structure_Template_Gray_Theme` | 17 | 7 | **17** |

Invisible to extraction, which walks the content tree and reads every cell: all six reported the
reference's character count *exactly* while drawing a fraction of it. That is the whole reason the
previous round's `drawn-short-read-whole` split was the right instrument — and it is also why the
brief's single-construct reading of the fifteen was too tidy. They were three constructs and a
fourth.

---

## The nine pagination rows, and the three `mcar` documents

**Characterised, not closed.** The brief asked whether they share a cause with the `mcar`
documents. They share a *symptom* and I could not reduce them to one mechanism.

### What is true of `Annex-10` (169 pages against 148, 189432 glyphs against 189432)

- **The pagination is not drifting; it is offset.** Counting the third column's lines per page —
  one per table row — gives ref page 4 = 38 rows and ours page 5 = 38; ref 5 = 21 and ours 6 = 21;
  ref 6 = 25 and ours 7 = 25; ref 7 = 20 and ours 9 = 20. **Our page *N*+1 holds the reference's
  page *N*** for long stretches, so the excess is inserted at page boundaries rather than
  accumulated across them.
- **Eight of our 169 pages carry fewer than 12 text lines and none of the reference's 148 do.**
  Our page 8 holds one line, `Ltd` — the third line of a row whose first two are on page 7 — and
  the table's repeated header row is not drawn on it. Our page 7 runs to y = 736.6 where the
  reference's stops at 660.0.
- **Our body starts 12 pt higher on every page**: the reference's topmost line is at a median
  93.0 pt and ours at 81.0, which is `\margt1620` exactly. The document declares **no `{\header}`
  group at all**, so the usual header-height explanation does not apply and this is unexplained.
- Within a row the line pitch agrees to 0.00 pt (11.50 both sides). Row *heights* differ by
  ±0.6 pt with no consistent sign — over two rows the reference spends 47.75 pt and we spend
  48.16.

### One hypothesis raised and refuted, so no later round need re-derive it

*"The reference keeps a row whole where we split it."* **It does not.** A twenty-file synthetic
(`rowsplit`) puts a three-line row after 44 to 63 one-line rows on a letter page and asks where
`AAAA`, `CCCC` and the following row land. **Both renderers split the tall row in the same place
at every one of the twenty**, and the two disagree on two files only — `n51` and `n52`, where the
reference breaks the page and we fit one more row on it. So the difference at a page bottom is a
**one-row capacity difference**, and its sign is the wrong way round to explain `Annex-10`: fitting
*more* rows per page would give us *fewer* pages.

### The `mcar` documents

They do share the near-empty-page symptom: 7 thin pages of 50 on `SPA-11`, 7 of 88 on `SPA-06`,
7 of 76 on `FRE-03` and **26 of 318** on `02_mcar_part-2`, against 0 in the reference on every
page of `Annex-10` that could be checked. What they do *not* share with `Annex-10` is the setup —
all five `mcar` declare eleven `{\header}` groups and `\headery432` against `\margt1080`, and
`Annex-10` declares none — so a header-height rule cannot be the common cause. The two
`text-agrees` rows with no header and no table depth, `prison-population-bulletin-june` (6/4) and
`II.1.2_Hypoxia` (12/13), carry **no thin page at all**, so they are a third thing again.

**Where a follow-up round should start**: `SwTabFrame::Split`
(`sw/source/core/layout/tabfrm.cxx`:1103-1510) and the two-attempt structure that calls it
(`:2963-2968`, *"1. Try: bTryToSplit = true => Try to split the row. 2. Try: bTryToSplit = false
=> Split the table between the rows"*), with `lcl_RecalcSplitLine` (`:678`) as the thing that
rejects a proposed split and sends the whole row on. The synthetic above is the instrument to
extend; it agrees 18 of 20 today and the two it does not agree on are the page-bottom capacity.

---

## The four raster-ceiling rows: confirmed, and filed

`clustered-column-result`, `clustered-column-template`, `pie-chart-result` and
`pie-chart-template` each carry **two `{\pict}` and no `{\shp}`**: LibreOffice's export writes the
chart as `{\*\shppict{\pict\wmetafile8 …}}{\nonshppict{\pict\pngblip …}}`. It reads the metafile
and rasterises it, so its PDF holds a picture with the chart's labels *inside* it; we play the
metafile and its text records stay real text. Both sides carry the raster; only ours carries the
words.

The excess is exactly the chart's furniture — on `clustered-column-result` it is `Production in
2015/2016/2017` and the six category names, on `pie-chart-result` the six names plus
`30% 16% 8% 6% 5% 35%` — and **no word the reference draws is missing from ours** on any of the
four. Added to `dotnet/TODO.raster-ceiling.md` so no later round re-derives them.

---

## What `dotnet/CLAUDE.md` says that this contradicts

- *"`SwAnchoredObject::IsDraggingOffPageAllowed` … needs `DisableOffPagePositioning` — an ODF
  settings flag defaulting to false that no DOC or DOCX import sets. So it does not arise for the
  formats this reader reads."* (`FrameLayout.CapturedOnPage`'s remark.) **`WriterFilter.cxx`:333
  sets it `true` for every writerfilter import**, DOCX and RTF both, one line below the
  `DoNotCaptureDrawObjsOnPage` the same paragraph cites. It is therefore live for two of the
  formats, gated on the object being wrap-through — and it still fires on none of the twenty-one
  RTF probes here, two of which are wrap-through, which is a separate thing a later round may want
  to run down.
- The same file's *"a content-anchored frame is captured on its page … which formats do it is the
  DOC/DOCX distinction"* is right about the flag and incomplete about the consequence: **an RTF
  `{\shp}` is captured whatever the flag says**, because it is a fly rather than a drawing object.
  DOCX is untouched by this round and its capture stays off.

---

## Reproducing

```sh
export TMPDIR=/home/user/wt-rtfshape/.work/tmp
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
R=dotnet/probes/rtf-shape-r73

python3 $R/gen.py /abs/probes && $R/render.sh /abs/probes /abs/probeout
python3 $R/measure.py /abs/probeout            # 41 of 41 agree
python3 $R/census.py /home/user/corpus-odf/words

dotnet/probes/rtf-gate-r71/sweep-ours.sh /home/user/corpus-odf /abs/rtf-after 4
python3 dotnet/probes/rtf-gate-r71/score.py /abs/rtf-after/ours.tsv > /abs/rtf-rows.tsv
dotnet/probes/words-continuous-r72/sweep-ours.sh \
    /home/user/sample-files words /home/user/gate-2f47 /abs/words-after 4
```
