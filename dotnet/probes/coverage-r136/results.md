# A third of the corpus has still never been looked at, and ranking it on anything but ink found six seats

Round 136, `/home/user/wt-coverage`, branch `agent/coverage`, base `9a90bd663`, 2026-09-15.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r133, `/home/user/gate-r133/ref`, 947 PDFs |
| our renderings | `/home/user/gate-r133-cli/XRay.Office.Cli`, the binary gate r133 itself ran; `9a90bd663` adds probe files only to `a185ec79a`, which that gate was taken at, and touches no `dotnet/src` |
| population | **all 947**, not the 915 that pass — see §1.1 |
| C++ tree | `/home/user/libreoffice-core`, read only (C8: it declares `27.2.0.0.alpha0+` and is not the reference's own source, so every arm below is confirmed a second time against 26.2.4.2's own output) |
| `/usr/bin/soffice` | 24.2.7.2 — not used for anything |
| `soffice` spend | **7 invocations, on 3 documents** (one `--convert-to fods`, and the C11 control's two fresh renders of three documents). No sweep drove the reference binary. |
| disk | started 7.9 GB free, ended 5.8 GB, and none of the fall is this round's: our PDFs are measured and deleted one document at a time, so the sweep's peak cost is three documents |

**Seat numbers.** The brief named **O88** as the next free seat. While this round was working, the
coordinator dispatched `agent/pagefield` and gave it O88 provisionally, so **every row proposed here
is numbered from O89 up** and the section headings below carry the renumbered ids — O89 to O94.
There is no lost seat in the gap.

**Why this round exists.** Round 124 ranked the 915 gate-passing documents on summed `|ink|%`
and its two most valuable documents scored **0.71 and 1.67**. They were found by a different
instrument entirely — a census of which documents no write-up had ever named — and they are
pale, so no ink ranking could have reached them. This round rebuilds that census at today's
head and then ranks the corpus on five things that are not ink.

---

## 1. The coverage census

`mention-census.py` reads every `*.md` under `dotnet/probes` (and, as a sensitivity, every
`*.md` under `dotnet/`) and asks, for each of the 947 corpus stems, whether any of them names
it. Three rules, each of which exists because without it a document a round *did* open reads as
never examined — the one direction this census must not err in:

- the stem as a whole token, bounded by something that is not a letter, digit or underscore;
- **`__ext` is this project's own rendering-name suffix**, so a single trailing underscore must
  not end the token and a doubled one must. Without this, `048_Expense_trends_budget_18d1e8ba`
  reads as never named although `chart-fit-r97/results.md` writes
  `048_Expense_trends_budget_18d1e8ba__xlsx` on its line 296;
- **an elided citation.** Write-ups routinely shorten a long name — `RMI_…GettingOffOil`,
  `ws_prod-…-European-Safety-Strategy-Initiative`. Every inline-code span holding an ellipsis is
  split on it and matched against each stem in order.

A stem shorter than nine characters must additionally be followed by its own extension: there are
31 of them (`09`, `003`, `135`, `762`, `1447`, `Reid`…) and `09` occurs inside every `2026-09-14`.

### 1.1 The figure, and the correction to the brief's version of it

**The brief's "406 of 947" is not what round 124 measured.** Its own §0 reads *"509 of the 915
passing documents have been named somewhere before; 406 never have"* — the denominator is the
**915** rows gate r122 called `match`, not the corpus. 406 of 915 is 44.4 %.

So that figure and this round's are not comparable, and the honest comparison is one instrument
run at both revisions:

| scope | at r124's base `d98c8be24` | at this round's base `9a90bd663` |
|---|--:|--:|
| `dotnet/probes/**/*.md` (443 → 454 write-ups) | **338 of 947 never named** | **328 of 947** |
| `dotnet/**/*.md`, which adds `TODO.batches.md`'s 18 900-line batch log (480 → 491) | 305 of 947 | **297 of 947** |

Restricted to round 124's own 915 stems and run at its own base, this instrument says 419 never
named where r124's said 406; the 24 it counts as named and r124 did not are all in
`TODO.batches.md`, which r124's scan included and this one's primary scope does not, and the 11
the other way are the `__ext` and hash-suffix rules above. **Take the 947-denominator figures as
this round's; the r124 number is quoted only so the two are not confused.**

### 1.2 Nine rounds moved the never-examined set by six documents

Rounds 125 through 133 added **11 write-ups** and took the never-examined set from 338 to 328.
Ten of those are net; the six documents newly named by a `probes` write-up are
`LENTOBUSSIAIKATAULU.-31.10.-31.12.2022`, `private-hire-operators-licensed`,
`License App Instructions 2-22`, `2ca950374241722b8ea7c132dfce63ae`,
`eTAR_External_Web_tool_Tip_Sheet_mh` and `PK_FlugzeugeStricken` — and four of those six were
named by round 124 itself, in the write-up that closed after its census was taken.

**So the hole is not closing, and that is the first result of this round.** A third of the corpus
has never been named by any probe write-up, and nine rounds of work moved that third by 1 %.

### 1.3 What is over-represented in it

`census-summary.txt` is the whole table, every row carrying its own base (C9). The three that
matter:

| | never named | of | share | vs the corpus' 34.6 % |
|---|--:|--:|--:|--:|
| documents the reference prints on **2–3 pages** | 126 | 214 | 58.9 % | **1.70×** |
| documents the reference prints on **1 page** | 106 | 238 | 44.5 % | 1.29× |
| documents the reference prints on **50+ pages** | 7 | 92 | 7.6 % | **0.22×** |
| the `chartset` batch family | 208 | 412 | 50.5 % | **1.46×** |
| `pptx` | 128 | 251 | 51.0 % | 1.47× |
| `ppt` | 7 | 51 | 13.7 % | 0.40× |
| manifest kind `ceiling` | 55 | 69 | 79.7 % | **2.30×** |
| manifest status `open` | 84 | 144 | 58.3 % | 1.68 × |

**The hole has one shape and it is length.** A document of two or three pages is nearly eight
times likelier never to have been named than one of fifty or more, and 208 of the 328 are
`chartset` templates — the numbered one- and two-page template sets. That is exactly where round
124's two Storyboards were, and it is exactly what an ink ranking cannot reach: a two-page
template has two pages of ink to sum and a 700-page advisory circular has seven hundred.

---

## 2. The instruments, and why none of them is ink

`shape-sweep.py` renders our half of all 947, measures both sides page by page with PyMuPDF and
**deletes our PDF as soon as it is measured**, so the sweep's peak disk cost is three documents
rather than a corpus. Per page, on both sides: the media box to a tenth of a point, the text
layer's character count with whitespace removed, `page.get_drawings()` split into fills and
strokes, and the **count** of `page.get_image_info()` placements.

`rank-shape.py` normalises five disagreements to `|a−b| / max(a,b,1)` and ranks on the **worst**
of them, never their sum — a document wrong in one respect and right in four is the whole point,
and a sum buries it.

Four instrument cautions were applied rather than discovered:

- **`fills` and `strokes` are carried and not scored.** C16: this tree fills every text rule and
  26.2.4.2 strokes every one, so those two columns disagree by construction on any document with
  an underline in it.
- **The image *box* is never read, only the count.** Round 128 established that
  `get_image_info()` reports the image XObject's declared placement rectangle, which for a
  cropped picture is deliberately far larger than the frame, and that this cost round 124 its
  "8.85× too wide" headline. §5 shows the same instrument telling a second lie, of a different
  kind, and that lie is what the brief sent this round to check.
- **`draws` is not a clean column either** and is used only as a filter. `CLAUDE.md` records that
  26.2.4.2 writes a themed gradient as a Form XObject full of band fills where this tree writes
  one `sh`, so a path count is not comparable wherever a gradient is involved.
- **The page-*size* column exists because `pdf-image-diff.py` prints a non-numeric row for such a
  page and every ink-summing tool silently drops it** — r124 §5. It is measured here from the
  two PDFs' page boxes with no rendering at all.

947 of 947 scored, none unscoreable. **843 disagree in at least one column.**

---

## 3. SEAT O89 — a worksheet shape whose own fill is a picture is drawn with no fill at all

### What the ranking said

The head of the never-examined set on the worst-of-five score is a family of ten: every
`0NN_Volunteer_Sign_Up_Sheet_Template_*` workbook in the corpus, **all ten with a prior-mention
count of zero**, each missing exactly one image placement the reference draws.

### What the file says

`091_Volunteer_Sign_Up_Sheet_Template_Colored_Background_2cb58d01.xlsx`, `xl/drawings/drawing1.xml`
is one `xdr:twoCellAnchor` holding an **`xdr:sp`** — a shape, not an `xdr:pic` — whose `xdr:spPr`
carries

```xml
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:blipFill dpi="0" rotWithShape="1">
  <a:blip r:embed="rId1"/><a:srcRect/><a:stretch><a:fillRect/></a:stretch>
</a:blipFill>
<a:ln><a:noFill/></a:ln>
```

`rId1` is `xl/media/image1.png`. The shape's `a:ext` is 1898650 × 323850 EMU = 149.5 × 25.5 pt.

### What each side draws

26.2.4.2 draws it, at the stated box:

```
213.25 682.186 147.912 25.483 re W* n
q 147.912 0 0 25.483 213.25 682.213 cm /Im100 Do
```

This tree draws **nothing whatever** in that rectangle — no image, no fill, no stroke.

### The rule, confirmed twice

**Against 26.2.4.2's own resolved view.** `--convert-to fods` on that workbook writes the shape
as a `draw:custom-shape` at `svg:width="2.0543in" svg:height="0.3539in"` carrying
`draw:fill="bitmap"` and a `<draw:fill-image draw:name="msFillBitmap_20_1">`.

**In source** (`/home/user/libreoffice-core`, C8): `FillProperties::pushToPropMap`
(`oox/source/drawingml/fillproperties.cxx`:427, `case XML_blipFill:` at :597) sets
`eFillStyle = FillStyle_BITMAP` and pushes the graphic as the shape's own fill. There is no
spreadsheet-specific branch; a blip fill is a fill like any other.

### Where it is missing here

`XlsxDrawings.cs`:303 — *"A shape carries no image and no chart, so it reaches the print area and
stops there"* — returns early for an `xdr:sp`, because `picture` is the `xdr:pic` element and
there is none. The shape's ink then comes from `XlsxShapeInk.Read`, which resolves
`a:solidFill`, `a:gradFill`, `a:grpFill` and the style matrix's fill and **has no `a:blipFill`
branch at all**. Worse, its `StatesFill` guard (`XlsxShapeInk.cs`:175-177) lists `blipFill` — so
a blip-filled shape is correctly held back from the theme fallback and then given nothing. The
class' own remarks carry the census that briefed it and it names the number: *"**10
`a:blipFill`**"*. They were counted in round 84 and skipped.

### Reach, with its base rate

`blipfill-census.py` counts `a:blipFill` under a shape's own `spPr` — not under a picture
wrapper, which every reader here already draws — in all three OOXML formats:

| | documents | shapes |
|---|--:|--:|
| `xlsx` | **10** | **10** |
| `pptx` | 15 | 57 |
| `docx` | 20 | 36 |

**Only the spreadsheet reader has the hole.** `PptxSlideLayout.cs`:1638 is *"An `a:blipFill` used
as a shape's fill: a tiled or stretched bitmap"*, and `DocxPictures.Read`'s own remarks name
`a:blipFill` as one of the three wrappers it resolves; neither the `pptx` nor the `docx` column
shows a missing placement traceable to this.

Drawn cost, `blipfill-drawn.py`: **10 of 10 workbooks are missing exactly one image placement,
37 460 pt² of picture in all**, 3 671–3 885 pt² each. On four of the ten this tree draws nothing
at all inside that rectangle; on the other six the only marks inside it are cell rules and text
that pass through it.

**The base rate is the rest of the corpus and it is zero.** `image-gap-census.py` finds 106
documents where the reference places more images than we do; 26 of them place none at all, and of
those, 7 hold no media part (C7, §7) and the remaining 17 are these ten, six `pptx` whose media is
SVG-only (§7), and one `.docx` whose one media part is a WMF. No workbook without an
`a:blipFill` shape is missing a placement.

**No gate column can see it.** All ten score `match`, and on eight of the ten our character count
equals the reference's exactly.

---

## 4. SEAT O90 — a `w:object`'s replacement picture and its caption are drawn twice, and the reason is a filter that can never be false

### The mechanism

`DocxVmlFrames.TopLevel` selects the shapes of a `w:pict` or `w:object` with

```csharp
element.Descendants().Where(child => IsShape(child) is not false && …)
```

and `IsShape` returns `true` for `shape`/`rect`/`roundrect`/`oval`/`group` and **`null` for
everything else — it never returns `false`.** So the filter admits every descendant, and the
class' own remarks, which say `v:shapetype` *"is excluded deliberately"*, describe a filter that
does not exist.

Inside a `w:pict` that costs nothing: a `v:path`, an `o:lock` or a `v:imagedata` carries no CSS
`style`, `One` finds no width and returns null. Inside a `w:object` it costs, because `One` then
falls back to the object's own size:

```csharp
width  ??= Twips(element, "dxaOrig");
height ??= Twips(element, "dyaOrig");
```

which gives **every** descendant a box. And `DocxPictures.ReadVml` looks for its blip with
`DescendantsAndSelf()`, so the `v:imagedata` element — one of those descendants — resolves
*itself* and the object's replacement picture is built a second time.

### On the page

`A1. EASA Form 2.docx` page 7. The `w:object` states
`w:dxaOrig="1540" w:dyaOrig="997"` and its `v:shape` states
`style="width:77.25pt;height:49.5pt"`. Our content stream holds both boxes, one after the other:

```
q 394.35 305.625 77.25 49.5  re W n   0.9778 0 0 0.968  394.35 -459.824  cm  … /Im3 Do …
q 394.35 305.625 77   49.85 re W n   0.9747 0 0 0.9748 394.35 -465.2362 cm  … /Im3 Do …
```

`77.25 × 49.5` is the `style`; `77 × 49.85` is `1540/20 × 997/20`. Each replay also draws the
metafile's caption, so `WI.CAO.00113.docx` is set twice, 0.3 pt apart.

### Confirmed by a one-attribute variant

Removing `w:dxaOrig` and `w:dyaOrig` from that one element and changing nothing else: the second
`/Im3 Do` disappears, the duplicated `WI.CAO.00113.docx` disappears, page count unchanged at 7,
character count 2166 → 2149.

### Reach, with its base rate

`object-dup-census.py` over all 272 corpus `docx`: **8 documents hold a `w:object`, 26 objects in
all, and all 26 state both a `v:imagedata` and a `w:dxaOrig`/`w:dyaOrig` pair — so all 26 are
drawn twice.** There is no `w:object` in this corpus that escapes it, and `w:dxaOrig` exists on no
other element, so nothing outside `w:object` is affected.

`dxaorig-variant.py` renders each of the eight as authored and with that attribute pair stripped
from every `w:object` in every story, and differences **our two renderings** — the reference is
nowhere in the instrument:

| document | objects | Δ characters | Δ image placements | Δ path constructions |
|---|--:|--:|--:|--:|
| `f2_registro_de_aprovacao_com_pbcs_EN` | 18 | 0 | 0 | 162 |
| `UG.CAO.00006 Foreign Part 145 approvals …` | 1 | 580 | 0 | 38 |
| `5709.16 ch.40_mgfinal` | 1 | 384 | 1 | 0 |
| `EHEST-SMS-Safety-Management-Manual-V2` | 1 | 159 | 5 | 19 |
| `FO.FCTOA_.000129 …` | 2 | 45 | 2 | 0 |
| `eTAR_External_Web_tool_Tip_Sheet_mh` | 1 | 29 | 1 | 0 |
| `A1. EASA Form 2` | 1 | 17 | 1 | 0 |
| `airbus-pdf-information-package_v1-4` | 1 | 0 | 2 | 0 |
| **total** | **26** | **1 214** | **12** | **219** |

**Page count moves on 0 of the 8**, so it is entirely a paint defect: 1 214 characters and 219
paths of ink drawn twice, slightly out of register.

Four of these eight were named by round 124 (§5 below) and four — `f2_registro…` with 18 objects,
`UG.CAO.00006`, `airbus-pdf-information-package` and `EHEST-SMS-Safety-Management-Manual-V2` —
were not named for it by anything.

### What it does NOT license

Not `IsShape(child) is true`. That would also drop `v:line`, `v:polyline`, `v:curve` and
`v:arc`, which are VML shapes this filter currently admits by accident and `One` currently sizes
from their `style`. The narrow change is to stop the `dxaOrig` fallback, or the picture
resolution, reaching an element that is not one of the shape kinds; which of those is right is
not measured here.

---

## 5. r124's "an image drawn at about a third of its size, and drawn twice" — retired: half is §4 and half is the instrument

Round 124 §8 recorded, unattributed, *"an image drawn at about a third of its size, and drawn
twice"* in four documents, `A1. EASA Form 2.docx` p7 among them, with *"reference one image
442,376-520,426 (78 × 50); ours two identical 421,487-445,512 (24 × 25)"*. The brief asked
whether that is round 128's `get_image_info` artefact before it is treated as a defect. **It is
both, and the two halves separate cleanly.**

**"Drawn twice" is real and is §4.** The one-attribute variant above removes the second draw.

**"A third of its size" is the instrument, and it is a second and different lie from round 128's.**
The object is `DrawAspect="Icon"`, and its replacement `word/media/image1.emf` is 5 148 bytes
holding one `EMR_ALPHABLEND` of a **32 × 32** icon, an `EMR_EXTTEXTOUTA` of the caption and a
`EMR_POLYBEZIER16`, inside a frame of 2786 × 1803 hundredths of a millimetre = 78.97 × 51.11 pt.
26.2.4.2 rasterises the whole metafile and emits **one** image XObject of 322 × 206 px at
77.3 × 49.5 pt, which is 300 dpi. This tree replays the metafile and emits the **icon** as its own
image XObject at 24.8 × 24.6 pt — the size the icon actually is inside a 79 pt frame. So r124's
"78 × 50 against 24 × 25" compares *the reference's raster of a whole metafile* against *our
bitmap of one record inside it*, and 24.8/77.3 = 0.32 is the icon's share of the frame, not an
error.

Our frame is right: our replay's composite scale on that object is 0.9778 across against the
metafile's own 78.97 pt frame in a 77.25 pt box, and the reference's placement is 77.3 × 49.5.

**The general form, which round 128 stated for cropped pictures and which this extends:** an
image placement list is not comparable between two renderers wherever one replays a metafile and
the other rasterises it. Round 128's rule was *the box is not where the image is drawn*; this one
is *the list is not the same objects*. Both are reasons this round scores the image **count** and
never the box, and reads even the count only through the media-part discriminator of §7.

---

## 6. SEAT O91 — a `<w:br/>` line in a shape's text takes the paragraph mark's size, and when the body then exceeds the box this tree draws none of it

### What the ranking said

`086_Printable_Graph_Paper_Template_Gray_Theme_7300e5d7.docx`, one page, **prior mentions zero**:
this tree draws **10 characters** where 26.2.4.2 draws **95**. It is the largest relative
character disagreement in the whole corpus, in either direction.

### What is missing

Both sides draw the page's grid and the footer's `Graph Paper`. What ours does not draw is the
text of the two floating text boxes — `Title: ___…` at y 42.5–55.3 and `Date: ___…` at
y 42.4–55.2. **Both sides draw the boxes themselves**, at `fs [49.0, 27.1, 319.9, 59.8]` with the
same grey fill and the same 1 pt grey stroke, to a tenth of a point. Only the text inside them is
absent, and it is absent entirely — not clipped.

### The two faults, separated by variants

Each box is a `wps:wsp` of `cy="414655"` EMU = 32.65 pt, `wps:bodyPr` with `anchor="ctr"` and
`tIns = bIns = 45720` EMU = 3.6 pt, so its inner height is 25.45 pt. Its `w:txbxContent` is four
runs holding nothing but `<w:br/>` at `w:sz="4"` — two points — and then one run of text at the
paragraph default.

`br-threshold.txt` is the sweep. Keeping k of the four breaks and changing nothing else:

| breaks kept | this tree draws the text | our `Title:` ink top |
|--:|---|--:|
| 0 | yes | 37.1 |
| 1 | yes | 44.5 |
| 2 | **no** | — |
| 3 | **no** | — |
| 4 (as authored) | **no** | — |

A break added above a vertically centred single line moves that line down by half the line it
adds, so **this tree gives an empty `<w:br/>` line 2 × 7.4 = 14.8 pt**. 26.2.4.2 spends 10.8 pt on
all four — **2.7 pt each**, which is the run's own 2 pt at the paragraph's 1.35 spacing. 14.8 pt is
11 pt at the same spacing, and 11 pt is what the paragraph *mark* states; the break run's own
`w:sz="4"` is not consulted.

**And the drop is the height and nothing else.** With all four breaks kept and the shape's `cy`
raised from 414655 to 1414655 EMU, one attribute, this tree draws the text (ink top 104.0). So the
text reaches the layout and the layout discards it when the body exceeds the box — where
26.2.4.2 draws it, and where this shape's own `wps:bodyPr` says `vertOverflow="overflow"`.

So the row is two rules, and the second is the general one:

- **O91a** — an empty `<w:br/>` line in a shape's text body takes its height from the paragraph
  mark rather than from the run carrying the break;
- **O91b** — a shape's text body taller than the shape is drawn not at all rather than
  overflowing.

### Reach, with its base rate

`txbx-br-census.py` over all 272 `docx`: **13 documents hold a `<w:br/>` inside a
`w:txbxContent`, 353 breaks in all; 7 documents and 278 breaks state a `w:sz` smaller than their
own paragraph mark**, which is the population where the two rules differ. On the other 6
documents and 75 breaks the two readings necessarily agree.

**Text actually lost is narrower than that and is the figure to quote: 2 documents, 86
characters** — `086` 10 against 95, `084_Printable_Graph_Paper_Template_Editable_Layout` 10
against 21. The five `Work_Breakdown_Structure` templates carry 263 of the 278 disagreeing breaks
and lose nothing; they draw 0 to 22 characters *more* than the reference, so their boxes absorb
the difference. O91b's own reach outside this family is **not censused** — a static census cannot
say whether a body overflows — and that is the largest thing this round leaves open.

---

## 7. Measured and discarded — recorded so they are not re-derived

- **The page-size census over all 947 finds nothing round 124 did not already have.** r124 ran it
  over the 915 passing documents and found 2 documents and 20 pages, all orientation flips.
  Extended to 947 at a 1 pt tolerance it finds the same two — `FAA 2025-26 Holdover Tables` and
  `24-25_FAA_Holdover_Tables`, 10 pages each, which is **O78** — plus one page of
  `047_Date_tracker_Gantt_chart`, which is a C11 oscillator and a C6 volatile-date workbook and
  differs in page *count* (5 against 8). **There is no third document.** The instrument the brief
  pointed at is therefore now exhausted on this corpus, and that is worth recording as a negative.
- **Four `.ppt` differ in page size by 0.094 pt and it is a unit conversion, not a defect.**
  `Inducement-to-Insurance-Business`, `RESPA_-_Section_8_Webinar`,
  `ws_prod-…-industrymeeting18112004-Aercap` and `undp_presentation_revised_17_may`: the reference
  gives 960.094482 × 540 where we give 960 × 540, i.e. it converts the slide size through
  hundredths of a millimetre and we do not. 91 pages, 0.094 pt, all four `match` on the gate.
  Reported here only so the next round's size census does not open it.
- **"The reference draws image placements and we draw none" is at least half C7, the raster
  ceiling, and the discriminator is the package.** Of the 26 documents where we place no image and
  the reference places some, 7 hold no media part at all. Six of the rest are `pptx` whose media is
  **SVG only** — `036_Chevron_Process_Cards`, `023_5-Step_Progress_Cards`,
  `050_Five-Point_Radial_Hub`, `064_Four-Step_Success_Cycle`, `003_` and `008_…Venn_Diagram`, all
  prior 0 or 1. Checked on `036` page 1: the reference's placements inside the first icon's
  rectangle are 449 × 82 pt and 140 × 88 pt — they are the **chevrons**, rasterised whole — while
  this tree draws 11 path constructions there. We draw as vectors what the reference rasterises,
  which is C7 with the sign reversed, and it is not a defect of ours.
- **`071_Four-week_project_timeline` is C6 and C13, not a pagination defect.** It ranks high on
  both `dpages` (1 against 2) and `dchars` (405 against 594) and its manifest kind is
  `pagination` — and the reference's page 1 reads `9/1/2026` where ours reads the cached
  `2/6/2023`. It is a `TODAY()` workbook, the extra page is the recalculated date extending the
  sheet, and neither column is ours. It also has prior mentions 0, which is exactly the trap: a
  never-examined document at the head of a ranking is not thereby a finding.
- **The never-examined set is enriched at the head of this ranking, but not in every column, and
  the column that most looks like a defect is the one where it is depleted.** Share of
  never-examined among the top N on the worst-of-five score: 64 % at N=25, 42 % at 50, 39 % at 100,
  36.5 % at 200, against the corpus' 34.6 %. But broken out: **top 50 on `dimages` is 50 %
  never-examined and top 50 on `dchars` is 12 %.** The documents whose text layers disagree most
  with the reference are documents rounds have already opened. So "coverage finds what magnitude
  hides" is supported here for drawn objects and **refuted for text**, and the 64 % at N=25 is
  largely one family of ten (§3) rather than a general property.
- **`dpages` and `dsize` are not enriched at all.** 11 of 947 documents disagree on page count and
  2 of those 11 have never been named (18 %, against 34.6 %); 3 disagree on page size and none of
  those has never been named.

---

## 8. The C11 control

For each of the three documents a finding rests on, the reference was rendered **twice more**,
fresh, same binary, same UTC day, into separate user profiles, and the quantity the finding
rests on compared — not the character count, which C11 records as a lower bound on the
reference's instability, but the image placements *with their boxes* and the extracted text
*itself*:

| document | placements a / b / the r133 bank | boxes identical | text identical |
|---|---|---|---|
| `091_Volunteer_Sign_Up_Sheet_Template_Colored_Background` | 1 / 1 / 1 | yes | yes |
| `A1. EASA Form 2` | 15 / 15 / 15 | yes | yes |
| `086_Printable_Graph_Paper_Template_Gray_Theme` | 1 / 1 / 1 | yes | yes |

## 9. The ten blind readings, what they found, and the three of their own headlines that measurement then knocked down

This container has no subagent tool; the coordinating session spawned **ten readers, one image
each, in parallel**, given the composed pair and nothing else — no document name, no numbers, no
statement of what was suspected, and an instruction not to read any project file or run any
command. The pairs were built by `pair-banked.sh` at 150 dpi from our render and the **banked**
reference rather than by `page-vision`'s own `pair.sh`, which re-renders the reference through
`soffice`.

### 9.1 The compositor was cleared, and three headline claims died with it

Four readers could not decide whether a size or pitch difference was theirs or an artefact of the
two halves being composed at different scales, and each named the separating test: measure a
shared landmark in pixels. The coordinator ran it on all ten composites without viewing them.
**Every pair's two page rasters are the same size to within one pixel**, so there is no scale
artefact anywhere in this set and `compose.py` did not mis-scale. With that explanation gone,
three claims fail on the ink extents:

| reading | claimed | measured ink rows | verdict |
|---|---|---|---|
| `072` Gantt | rows 488 px ours against 509 ref, *"~4 % taller row pitch, ~20 px cumulative"* | ours 64…651, ref 64…653 | **refuted**, two pixels |
| `086` graph paper | *"~30 px cumulative grid height difference"* | ours 43…923, ref 43…924 | **refuted**, one pixel |
| `concepts-…-cloud` p9 | reference content sits lower | ours 0…877, ref 0…875 | self-voided, **confirmed void** |
| `429 BLISc` p20 | page-height difference, left-edge green wedge | 0…967 both | self-voided, **confirmed void** |

Two vertical differences survive: `Regulations` p1 (ink ends 1066 ours / 1095 ref) and
`A1. EASA Form 2` p7 (1353 / 1369).

**A reader who hands you the tool to knock their own finding down is doing the job**, and on `072`
and `086` that is exactly what happened — and on both, the finding the reader is *remembered* for
is a different one that stands (§3, §6, §10.2).

### 9.2 `A1. EASA Form 2` p7 is not comparable, and the reason is a frozen field — not pagination

The reader reported that the printed footers read `Page 7 of 7` on ours and `Page 6 of 7` on the
reference, and listed five candidates. The coordinator settled it: **both sides render 7 pages**
(gate r133: 7/7, glyphs 11 547/11 527, `match`), and the reference prints `Page 6 of 7` on **all
seven**, because the footer's `PAGE`/`NUMPAGES` fields carry a cached result of 6 and 26.2.4.2
emits the cache instead of evaluating. The reader's candidate (3) was the right one; its three
pagination candidates are out. A second document has the identical signature —
`B11. TE.CAO.00129  Experience  logbook.docx`, 6 pages, reference `Page 3 of 6` on all six — and
the discriminator that survives the coordinator's census (272 `docx`, 96 with such a field in a
header or footer, 9 of them inside a text box, exactly 2 freezing) is that the drawing is anchored
**inside a table cell in the footer**. A `wpg` group was tested and refuted. That is
`agent/pagefield`'s round and nothing here was spent on it.

**Nothing in §4 or §5 rests on that pair**: both are read out of the content streams of page 7 of
each side and out of a one-attribute variant, and §5's finding is precisely that the two sides'
*image lists* are not the same objects. What the reader adds that is real is the list of things
identical on that page — section 13's line breaks word for word, the header, the grey note row,
the address block, the footer's construction — and its separate observation that ours carries an
extra content block at the top, which the field does not explain and which is **left open**.

This round's contribution to that other round is a wider denominator, which the coordinator names
as its weakest part: `odf-pagefield-candidates.txt` lists the **18 of 338 converted `.odt`** whose
`styles.xml` puts a `text:page-number` or `text:page-count` inside a `draw:frame` in a
`style:header` or `style:footer`. It is a candidate list, not a positive one.

### 9.3 What the readings found, and how it lands against the arithmetic

Four independent readers, on four unrelated documents, each landed on *a whole object the
reference draws and ours does not*. **That is a class, not four incidents**, and three of the four
are the seats of §3, §6 and §10.2 — arrived at from the other end, from pixels, by readers who had
not seen a number.

| pair | the reading | disposition |
|---|---|---|
| `091_Volunteer_Sign_Up_Sheet…` p1 | the reference draws a ~250 px logo in the white entry row and ours leaves it blank; **the row is the same height in both and every band below sits at the same y** | **§3, O89.** The reader's three candidates were "not decoded", "decoded and not painted", "anchor dropped"; the markup says a fourth — the picture is a *fill*, on a shape, and the shape is drawn. The reader's own caution that equal row height may just be a specified height is right and the fods view settles it instead. |
| `086_Printable_Graph_Paper…` p1 | both header boxes are **completely empty** in ours; the reference draws `Title:` and `Date:` and a ~330 px leader rule in each; grid, caption and logo identical | **§6, O91.** The reader's discriminator — *"if the leader is a drawn border rather than an underscore run and it is absent too, a font explanation is ruled out"* — is answered the other way: the leader is a run of underscores inside the same text body, and the whole body is discarded. |
| `072_Gantt_project_planner` p1 | **the entire chart region is blank white in ours**: all 60 period columns × 26 rows, no striping, no amber highlight column, no 26 stacked bars. And the discriminator: the amber `Period Highlight: 1` **input box IS filled in ours**, so it is not a blanket fill failure | **§10.2, O93.** |
| `069_Blue_modern_balance_sheet` p1 | a colour, not an object: ours draws `(100)`, `(85)`, `(185)` in **black** where the reference draws them **red** — and the same row draws `500` blue and `700` red identically on both sides, and both draw the parentheses | **§10.1, O92.** The sharpest reading of the ten: it named the mechanism (a negative section's colour clause applied on one side and not the other), the control that rules out a general colour failure, and the evidence that the section itself is selected. |
| `Regulations Governing the Status…` p1 | ours draws the UN emblem **inline, over the caption text**; the reference puts it on its own line above with the caption moved down. Separately the title rewraps — the reference pulls `Duties` up to line 2 | **§10.3, O94** for the wrap; the emblem is §7. The reader gave the separating test — measure the two *identical* lines — and it answers **unequal**: `Experts on Mission`, same face, same start x, **227.56 pt ours against 218.06 pt**. |
| `f2_registro_de_aprovacao` p1 | ours draws a light-grey four-sided box round each of ~18 fill-in fields; the reference a dark mark of which only the bottom edge resolves. Field extents and the three recurring widths agree, so the geometry is not shifted | **Not seated.** The reader **voided** its own "the reference omits three of four borders" (at 1 px it can only say they are unresolvable) and its own "ours is heavier" (colour and width are confounded at this density). This is C16 territory and wants `pdf-ops`, not pixels. The document is also §4's largest, with 18 duplicated `w:object` replays, which may be the whole of the "darker" impression. |
| `078_Modern_inventory_list` p1 | 25 rows, 10 columns, the full 25-row fill-band sequence, the three struck-through rows and the same red-flagged rows all agree. One residual: the comment marker beside each red flag is full text height in ours and a tiny superscript mark in the reference | **Not chased.** The reader **voided** its own "hash/crosshatch" identification. Settling it is a 300 dpi crop of the left margin strip. |
| `429 BLISc` p20, `concepts-…-cloud` p9 | read as matches: all 15 line breaks, bullet glyphs, hanging indents and the anomalous extra indent on item 4 agree on the first; identical line breaks in all 17 body lines, bullets, gradient band, navy bar and a teal underlined hyperlink on the second | **Nulls, and useful ones** — both are in the `imgs 0/N` population §7 attributes to C7, and a reader seeing no difference on a page where the reference places 160 image placements and we place none is what "we draw as vectors what it rasterises" looks like from the pixels. |

### 9.4 A C13 flag the readers turned up

`069` renders **FY-2022 / FY-2023** in ours and **FY-2025 / FY-2026** in the reference, a uniform
three-year offset in both columns at the same position in the same face. That is date-derived
content and the pair is not comparable on those two cells; it is C6/C13 and not a defect. The
three-year gap does not fit a wall-clock difference, so it is more likely the cached-versus-live
distinction than the date. **It does not touch O92**, which is about three other cells on the same
page whose values are identical on both sides.

### 9.5 How the readers bounded themselves

All ten stated a resolution limit (≈100–160 px/inch, hairlines at or under 1 px) and voided their
own thin-mark claims unprompted, several of them their own headline observations — `f2`'s "the
reference omits three borders", `072`'s chart-frame rule, `091`'s entry-row divider, `429`'s grey
diagonal, `Regulations`' notdef-box identification. **Treat every present/absent claim about a
hairline in this set as unresolved, in both halves and in both directions.** No two readers saw the
same page, so there are no direct disagreements to report; the agreement that matters here is
structural rather than per-page.

## 10. Three more seats, each of which a blind reader found first

### 10.1 SEAT O92 — a number format's colour clause is discarded

`069_Blue_modern_balance_sheet_Use_this_template_6b4d70d3.xlsx` states one custom `numFmt`
(`0_);\-0_)`, no colour) and gives **ten of its 39 `cellXfs`** the built-in `numFmtId="38"`, whose
ECMA-376 17.4.7 code is `#,##0 ;[Red](#,##0)`. `NumberFormatSection`
(`src/XRay.Office.Core/Numbers/NumberFormatSection.cs`:204) discards every bracketed body it does not
recognise as a currency symbol, an elapsed unit or a calendar directive, under the comment
*"Anything else — a colour name, `[ENG]` — changes appearance rather than the text this extracts"*.
True of the text, false of the ink. `git grep '\[Red\]'` over `dotnet/src` finds exactly one hit and
it is the built-in table's own `[38] = "#,##0_);[RED](#,##0)"` — **nothing applies a format colour
in any format, in any reader here.**

Reach, `numfmt-colour.tsv`: **23 of 241 `xlsx` state a colour clause; 20 use one on at least one
numeric cell (13 771 cells); and 8 documents and 137 cells hold a value that actually selects the
coloured section** — that last is the figure to quote, and the drop from 13 771 to 137 is the
reason to count what selects the section rather than what states it. Two of the eight have never
been named by any write-up. The `.xls` and `.ods` halves are not censused. No gate column can see
it.

### 10.2 SEAT O93 — a `cfRule type="expression"` this parser refuses paints nothing

`XlsxConditionalStyles.ConditionOf` sends an `expression` rule to `Comparison.Parse`, which scans
for one of `<> <= >= = < >` and requires each side to be a cell reference or a literal
(`SheetConditions.cs`:207-230). `072_Gantt_project_planner.xlsx` holds **no drawing part at all** —
its Gantt chart is conditional formatting over `H5:BO30` — and its ten expression rules are
`Plan`, `Actual`, `ActualBeyond`, `PercentComplete`, `PercentCompleteBeyond` (bare defined names),
`H$4=period_selected` (a comparison against a name), `MOD(COLUMN(),2)` and `MOD(COLUMN(),2)=0`
(function calls). Every one returns null.

Measured: **7 filled paths against the reference's 1 453**, 30 path constructions against 1 532,
with the character count identical at 815 on both sides — which is why no gate column moves.

Reach, `cfexpr.tsv`: **23 of 241 `xlsx` hold at least one refused rule, 118 rules over 60 341
cells, against 483 rules the parser accepts** across 34 workbooks. The drawn consequence shows as a
large fill deficit on **2** of the 23, and both have never been named: `072` and
`VOR Candidate Discontinuance List 2026-01-08` (9 fills against 152, refused rule `MOD(ROW(),2)=0`
over 1 419 cells). On several of the other 21 we draw *more* fills than the reference; C16 warns
that a fill count is not a weight and those were not chased.

**This is the round's one large seat.** Honouring it needs a small formula evaluator — defined
names, `COLUMN()`, `ROW()`, `MOD`, and relative-reference shifting from the `sqref`'s anchor —
and it should not be attempted as part of anything else.

### 10.3 SEAT O94 — `w:w` is read from a run and not from a paragraph style

`Regulations Governing the Status, Basic Rights and Duties…docx` styles its title `SL`, which
states `<w:sz w:val="57"/>` (28.5 pt), `<w:spacing w:val="-8"/>` and **`<w:w w:val="96"/>**.
`WordParagraphFormats.WidthOf` reads `w:w` and `TextWidthScale.Of` quantises it as VCL does; what
reaches neither is a *style's* value.

Two one-attribute variants:

| variant | `Regulations Governing the` |
|---|--:|
| as authored | 316.58 pt |
| the **style**'s `w:w` 96 → **60** | **316.58 pt — no change at all** |
| `w:w="95"` on the **run** | 299.98 pt |
| `w:w="90"` on the **run** | 283.96 pt |

And the reference honours it: `Experts on Mission`, `LiberationSerif-Bold` on both sides, same
start x, is **218.06 pt against our 227.56**, and `227.56 × (570×96/100)/570 = 218.36` — within
0.3 pt of what the style asks for. The title then wraps differently, which is the reader's
observation and is a symptom rather than the fault.

**A trap for whoever implements it, and it nearly went into this write-up as a finding.** A run
stating the *same* percentage as its style also comes out unscaled: `w:w="96"` on the run of a
paragraph styled `w:w="96"` gives 316.58, exactly the unscaled width, while 93, 94, 95, 97 and 99
on the same run give 293.68, 296.55, 299.98, 306.28 and 313.16. So a variant that sets the run to
the style's own value is a **false negative**, and reading it at face value says "`w:w` is applied
nowhere", which is wrong.

Reach, `stylescale.tsv`: **2 of 272 `docx` state a non-100 `w:w` in `styles.xml`, 11 statements**
— this document's 10 and one in `mde087077~283.docx` — against **11 documents and 1 391 statements
on a run**, which are read correctly and are the base rate. Narrow, and the whole of one
document's title block.

## 11. Tests and confinement

**Nothing in `dotnet/src` was changed.** The round was asked to find and characterise; none of
the three seats is a one-line change that is both obviously correct and fully measured — O89
needs the sheet drawing to carry a bitmap fill through to the painter, O90's narrow form is a
choice between two guards that is not measured here, O91 is two rules of which the second is
uncensused, and O93 needs a formula evaluator. So there is no confinement leg and no test to move.

## 12. The scripts and the banked data

| | |
|---|---|
| `mention-census.py` | §1, the coverage census; runs against a git revision as well as the tree |
| `mentions.tsv` | all 947 at HEAD, `dotnet/probes` scope — the primary figure |
| `mentions-at-r124-base.tsv` | the same instrument at `d98c8be24`, so §1.1's delta is like for like |
| `mentions-dotnet-scope.tsv` | the sensitivity that includes `TODO.batches.md` |
| `census-summary.py`, `census-summary.txt` | §1.3, every row with its own base rate |
| `shape-sweep.py` | our half of all 947, measured and deleted one document at a time |
| `docs.tsv`, `pages.tsv` | all 947 documents and all 28 616 pages, both sides |
| `rank-shape.py`, `ranking.tsv` | §2, the worst-of-five ranking with each document's prior-mention count |
| `image-gap-census.py`, `image-gap.tsv` | §3, §7; the media-part discriminator |
| `blipfill-census.py`, `blipfill.tsv` | §3, `a:blipFill` under a shape's own `spPr`, all three formats |
| `blipfill-drawn.py`, `blipfill-drawn.tsv` | §3, what the unread fill costs on the page |
| `object-dup-census.py`, `object-dup.tsv` | §4, every `w:object` and whether it duplicates |
| `dxaorig-variant.py`, `dxaorig-variant.tsv` | §4, the one-attribute variant, ours against ours |
| `dup-draw-measure.py`, `object-dup-drawn.tsv` | §4, the same eight against the reference |
| `txbx-br-census.py`, `txbx-br.tsv` | §6, a `<w:br/>` in a shape text body and its own size |
| `br-threshold.txt` | §6, the break sweep and the taller-box control |
| `c11-control.py`, `c11-control.txt` | §8 |
| `pair-banked.sh` | §9, a composed pair with no `soffice` in it |
| `numfmt-colour-census.py`, `numfmt-colour.tsv` | §10.1, and why the figure to quote is 137 and not 13 771 |
| `cfexpr-census.py`, `cfexpr.tsv` | §10.2, expression rules this parser accepts and refuses |
| `stylescale-census.py`, `stylescale.tsv` | §10.3, `w:w` split by the part that states it |
| `odf-pagefield-candidates.txt` | §9.2, 18 converted `.odt` for the page-field round's denominator |
| `pairs/` | the four composed pairs that produced a seat |
