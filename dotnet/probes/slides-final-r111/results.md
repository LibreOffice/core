# slides-final-r111 — the `.ppt` picture bullet, and what the rest of O15's residual actually is

Round 111, slides track, one seat with two rows: **O46**, the binary PowerPoint picture bullet,
and what is left of **O15**, the per-page dominant drawn text size over the 51-document `.ppt`
column.

Four results.

1. **The picture bullet is implemented and it lands where the reference puts it.** A `.ppt`
   states a whole outline level's picture bullet in the *main master's* extended paragraph
   sheet, and the graphic in a store under the document's `List` record; neither was read.
   Reading both, merging them into the paragraph the way `ImplGetExtNumberFormat` does, and
   giving the label a graphic's box puts this deck's six page-8 bullets within **0.1 pt of
   26.2.4.2's own placement, at exactly its 17.86 × 17.86 pt**, with the size rule confirmed
   against **eight** stated values in the reference's own flat ODP of the file.
2. **O46 is 4 pages → 2, and the other two are shown not to be picture-bullet defects.** All
   four legs of round 108's instrument were measured this round rather than inherited: pages 8
   and 15 are fixed; page 14 is a **LAYOUT** page — both readers hand this tree the same wrong
   answer on identical input — and page 16 is still a `.ppt` **IMPORT** defect, but a
   label-width one rather than a bullet one. The register is **11 → 10 pages and
   54.10 → 52.12 pt, 2 fixed and 1 newly wrong**, and the one newly wrong is a page round 110
   had fixed *by accident of the same unread bullet*; §2.3 measures it and names what is left
   unresolved.
3. **`RRM-training-syllabus-…` 16 is not ours and leaves the statistic.** Round 110's
   "separate portions accumulating rounding" is refuted: the line is one text object, one font
   and one size, and the gap after each capital M is a `-162` TJ adjustment. Measured
   character by character, **the reference draws DejaVu Sans at DejaVu Serif's advances** — the
   seventh confound `dotnet/CLAUDE.md` already records, arriving on the `.ppt` column through a
   `Book Antiqua` run filed as family class *roman*. Reproducing it would mean laying text out
   in a face we do not draw.
4. **The four "not size questions" split three-and-one, and the one is a live seat.** Measured
   at whole-page level rather than through `sizes.py`'s dominant-size bucket: three are the
   **raster ceiling** — the reference draws a picture where we draw the picture's own text —
   and `Thailand17` 11 draws **595 alphanumeric characters on both sides** and differs only in
   the size 358 of them are drawn at. That is a table-cell autofit question, and it keeps a
   seat.

Reach: over the whole slide corpus at both legs — 51 `.ppt`, 251 `.pptx`, 302 `.odp`, 604
documents, 0 failures — **8 renderings change and 596 are byte-identical**, all eight `.ppt`,
all eight predicted in advance by a record census, and **0 page counts and 0 alphanumeric
counts move anywhere**.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidesfinal`, `agent/slidesfinal`, base **`a362d41f9`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, used for the four pages of §4 |
| corpora | `/home/user/sample-files` (51 `.ppt`, 251 `.pptx`) and `/home/user/corpus-odf` (302 `.odp`) |
| working directory | `/home/user/r111-work` |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is **not** the reference binary's source |
| fonts | the five tarball confounds were already aside when this round started — `.duplicates-aside`, `.noto-aside` and `.condensed-aside` all present, `fc-list \| grep -c 'DejaVu.*Condensed'` = 0 |

**Both legs, as this track requires.** Every `file:line` below is the `27.2.0.0.alpha0+`
checkout and is the *explanation*. The evidence is 26.2.4.2's own output: its flat ODP of the
deck, its PDF of the same file, its PDFs of its own ODPs, and its banked renderings of the
corpus.

---

# 1. Where a `.ppt` states a picture bullet, and why round 110 could not find one

Round 110 established that gating on the paragraph's own `BulletBlip` is inert and said the
blip lives in the master's `aExtParaLevel`. That is right, and the census puts a number on it.

`pptdump.py` walks the record tree of `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt`:

* **Six `ExtendedParagraphMasterAtom` records** (type 4013), all under a `MainMaster` container's
  own `___PPT9` tag — two main masters × the `Body`, `HalfBody` and `QuarterBody` instances —
  each of depth five, whose levels 0 and 1 carry `mask=0x03800000 buBlip=0/1` and `2/3` and whose
  levels 2 to 4 are unwritten.
* **Four `ExtendedBuGraAtom` records** (type 2041) under `Document → List → ___PPT9 →
  ExtendedBuGraContainer`, instances 0 to 3, holding two PNGs (79 × 79 and 25 × 25) and two
  JPEGs as bare blip records behind a two-byte type word.
* **Five `ExtendedParagraphAtom` records** (type 4012) in the whole file, of which **three**
  entries state a blip — and all three are in the document's own presentation-rules container,
  not on a shape.

So a reader that consults only a shape's `ExtendedParagraphAtom`, as this one did, finds a
picture bullet essentially nowhere. Over the whole column (`census.py`): **8 of the 51 `.ppt`
carry a master level stating a blip, 120 levels in all**, and **0 master levels anywhere state
`hasAnm`** — which is what makes merging the numbering fields as well safe on this corpus.

## 1.1 The size rule, confirmed against eight of the reference's own numbers

`ImplGetExtNumberFormat` computes the graphic's box as

```cpp
sal_uInt32 nHeight = static_cast<sal_uInt32>( static_cast<double>(nFontHeight) * 0.2540 * nBulletHeight + 0.5 );
nWidth = ( nHeight * aPrefSize.Width() ) / aPrefSize.Height();
```

(`filter/source/msfilter/svdfppt.cxx`:3455-3461), where `nFontHeight` is the paragraph's *first
portion's* size in points, defaulted to 24, and `nBulletHeight` the level's raw percentage.

26.2.4.2's own flat ODP of the deck states its answer for every level it draws, as
`text:list-level-style-image/style:list-level-properties/@fo:height`. Pairing each with the
font size of the text beside it gives 155 % at level 1 and 110 % at level 2, and then the
expression is exact at eight of eight:

| level | font | percentage | expression | 26.2.4.2 states |
|---|---:|---:|---:|---|
| 1 | 20 pt | 155 | 787.4 → 787 | `0.787cm` |
| 1 | 16 pt | 155 | 629.9 → 630 | `0.63cm` |
| 1 | 18 pt | 155 | 708.7 → 709 | `0.709cm` |
| 1 | 28 pt | 155 | 1102.4 → 1102 | `1.102cm` |
| 1 | 5 pt | 155 | 196.9 → 197 | `0.197cm` |
| 2 | 24 pt | 110 | 670.6 → 671 | `0.671cm` |
| 2 | 18 pt | 110 | 503.0 → 503 | `0.503cm` |
| 2 | 16 pt | 110 | 447.0 → 447 | `0.447cm` |
| 2 | 20 pt | 110 | 558.8 → 559 | `0.559cm` |

All four graphics are square, and the reference's `fo:width` equals its `fo:height` on every
one of the twenty-two image levels in the file, which is the aspect-ratio half of the rule.

## 1.2 The merge, which is not symmetrical

`ImplGetExtNumberFormat` (`svdfppt.cxx`:3407-3446) takes each of the three fields from the
master's level only where the paragraph's own mask does not name it, skips the merge entirely
when the paragraph's mask names all three, and puts one extra condition on the blip alone:

```cpp
if ( (!( nBuFlags & 0x00800000)) && ( nMaBuFlags & 0x00800000 ) )
{
    if (!( nBuFlags & 0x02000000))          // if there is a BuStart without BuInstance,
        nBuBlip = rLev.mnBuBlip;            // then there is no graphical Bullet possible
}
```

So a paragraph that states `hasAnm` and nothing else **suppresses** its level's picture rather
than merging it, while the scheme still merges. `PptTextBody.MergedExtension` is that function
and `PptPictureBulletTests` has one test per arm.

**And a master's level is not the same structure as a shape's entry, though it looks like one.**
`ReadPPTExtParaLevel` (`:3185-3201`) and `StyleTextProp9::Read` (`:4812-4830`) agree field for
field as far as the character mask — the paragraph mask, `buBlip`, `hasAnm`, `anmScheme`, a PP10
extension, then the character mask with one optional word — and the paragraph's entry then reads
a **third** mask, the special-info one, with two more optional fields behind it. A level has
none. Every field is optional, so reading a level with the paragraph's reader consumes four
bytes too many and takes the *next* level from the wrong offset rather than losing one value;
`ALevelEndsAtItsCharacterMaskWhereAShapesEntryHasAThirdMaskAfterIt` pins it by asserting the
*second* level's blip. *This paragraph said the two differ in field order until the source was
read a second time; they do not.*

## 1.3 What the change is

Six source files and one new one.

* **`PptBulletPictures.cs`** (new) — `PptExtendedParagraphLevel`, one level of a master's sheet;
  `PptExtendedParagraphSheet`, the sheet itself, read off the `MainMaster`'s `___PPT9` tag;
  `PptBulletPictures`, the document's graphic store with a decode cache; and `PptProgTags`, the
  three-container walk to a `___PPT9` payload, which `PptTextReader` now shares instead of
  keeping its own copy.
* **`EscherBlips.Direct`** — a blip record with no `msofbtBSE` in front of it, which is how a
  `ExtendedBuGraAtom` stores one. The existing `Blip` tail is factored into `Decode` and both
  use it, so the UID-count and DIB-header rules cannot drift apart.
* **`PptStyleSheet.Extended`** — the sheet hangs off the style sheet because
  `PPTExtParaProv` does, so a title master inherits it exactly as it inherits the text styles.
* **`PptTextBody`** — `MergedExtension`, `BulletPicture`, and a picture branch in `Marker` that
  outranks the automatic number, as the reference's `if (nBuBlip != 0xffff) … else if (nHasAnm)`
  does.
* **`SlideMarker.Picture`** and **`SlideMarkerPicture`** — the label as a graphic with an
  absolute box. `Text` is empty when it is set, which is what keeps the character path — the
  recode table, the face resolution, the fallback search — from being reached at all.
* **`SlideTextLayout`** — the box in `BulletFloored`, the width in `MarkerReach`, and
  `EmitMarkerPicture`, which places the graphic at `ImpCalcBulletArea`'s top rather than lifting
  it to a baseline.
* **`PlacedText.MarkerPictures`** and **`SlideDrawing.DrawMarkerPictures`** — a picture bullet is
  not a glyph run and cannot travel in one, which is the same split `Outliner::PaintBullet`
  makes between `processDrawBulletInfo` and `processDrawPortionInfo`.

**One size trap, found by measuring the output rather than by reading anything.** A fresh
`RasterImage` per paragraph is a distinct picture to the PDF writer: the first cut embedded the
same four PNGs 186 times and listed all 186 in every page's resource dictionary, taking the
rendering from 190 154 bytes to **277 550** and page 8's resources from 1 image to 186, of which
seven are drawn. `PptBulletPictures.Decoded` caches one decode per blip and takes it back to
**191 247 bytes and 3 resources**, with every drawn size unchanged.

---

# 2. What it fixes, page by page, through all four legs

Every leg measured this round; `fourleg.txt` is the record.

| leg | page 8 | page 14 | page 15 | page 16 |
|---|---:|---:|---:|---:|
| 26.2.4.2 reading the `.ppt` | **15.99** | **18.99** | **18.99** | **14.99** |
| 26.2.4.2 reading its own flat ODP | 15.99 | 18.99 | 18.99 | 14.99 |
| this tree reading that flat ODP | 15.99 | 20.01 | 18.99 | 15.00 |
| this tree reading the `.ppt`, base | 15.00 | 20.01 | 17.01 | 17.01 |
| this tree reading the `.ppt`, head | **15.99** | 20.01 | **18.99** | 17.01 |

The round trip is **faithful** on all four pages, so the instrument is usable here — unlike on
the two pages of §5.

* **Page 8 and page 15: fixed**, and the reference's answer exactly.
* **Page 14 is LAYOUT.** Both readers hand this tree 20.01 on inputs the reference reads as
  18.99, so it is not an import defect and not the picture bullet.
* **Page 16 is IMPORT and still open.** This tree's ODP answer is the reference's (15.00 against
  14.99, inside the 0.15 pt band) and its `.ppt` answer is 17.01.

## 2.1 The bullets land where the reference puts them

Read out of the two PDFs with `pymupdf.Page.get_image_info`, page 8, top-left corner and drawn
size in points:

| | corners | size |
|---|---|---|
| 26.2.4.2 | 43.1,128.4 · 43.1,164.7 · 43.1,317.5 · 43.1,348.7 · 43.1,379.8 · 43.1,432.5 | 17.86 × 17.86 |
| this tree | 43.1,128.5 · 43.1,164.7 · 43.1,317.5 · 43.1,348.7 · 43.1,379.9 · 43.1,432.5 | 17.86 × 17.86 |

Six of six, within a tenth of a point, with no free parameter anywhere in the chain.

## 2.2 The residual on the same page is a label width, and it predates this round

The reference starts the bulleted body text at **x = 85.10** and this tree at **70.09** — on the
head leg and on the base leg alike, so nothing here moved it. The bullet itself is at 43.09 on
both sides, and

```
43.09 + 0.953 cm = 70.09     43.09 + 1.482 cm = 85.10
```

are both `text:min-label-width` values the deck's own list styles state. So it is *which* level's
label width is resolved, not the picture's width. Page 16 shows the same 15 pt, and it is the
prime candidate for that page's remaining `constScaleLevels` row. **Named, measured, not
worked.**

## 2.3 The page this cost, and the question it opens

`ws_prod-g-doc-Events-Part-M-presentation` page 21 goes **17.01 → 18.0** against a reference of
17.01, and round 110 had fixed it. Its four legs, measured:

| leg | page 21 |
|---|---|
| 26.2.4.2 reading the `.ppt` | **17.01** |
| 26.2.4.2 reading its own flat ODP | 18.0 |
| this tree, base | 17.01 |
| this tree, head | 18.0 |

Two things are true at once and both belong in the record.

**Round 110's fix of this page was an accident of an unread picture bullet.** That page's
bullets *are* pictures — the reference draws one 31.24 pt graphic and five 19.02 pt ones on it —
and the base floored its empty line at the box of a Wingdings 2 character no face holds. The
right box happened to push the fit onto the reference's row. With the picture read, the box is
the graphic's and the floor no longer fires.

**And the size we give the graphic is wrong on this deck, which is a real and separate finding.**
Reference against head, read out of the two PDFs:

| | level 1 | level 2 |
|---|---:|---:|
| 26.2.4.2 | 31.24 pt | 19.02 pt |
| this tree | 20.10 pt | 14.26 pt |

31.24 pt is 1102 hundredths of a millimetre — **28 pt** at the level's 155 % — and 19.02 is 671,
**24 pt** at 110 %. Those two are the deck's **master char levels'** heights at depths 0 and 1.
This tree uses the paragraph's own first portion, 18 pt, which is what
`GetNumberFormat(…, pParaObj, …)` reads — and on the France deck the paragraph's portion is what
the reference uses, exactly: 17.86 pt is 630, **16 pt** at 155 %, where that deck's master level
is 28 and would give 31.24.

So the two decks want different sources for the same quantity, and the mechanism that would
separate them is `nHardCount`: a paragraph gets a numbering rule of its own only when at least
one of seven attributes is hard, and otherwise keeps the master's rule, which
`PPTStyleSheet`'s constructor builds from `rCharLevel.mnFontHeight` (`svdfppt.cxx`:4409-4419,
:6151). **Implementing that test does not separate them** — measured, as a variant: France's
page-8 paragraphs state none of the seven (their masks are `0x1800`, `0x1801` and `0x1000`) and
still take their portion's height, and the variant drew France's bullets at 31.24 and reverted
both of its fixed pages. A second variant using the master's height everywhere gets page 21
exactly right — 31.24, 19.02 and 17.01 — and gives back both France pages.

What is left over is one of the two terms of `PPTParagraphObj::GetAttrib` that this reader
cannot evaluate: the destination instance, and the comparison of the source instance's master
level against the destination's. `PptTextBody`'s `LineSpacingStated` remark already records the
same pair as unevaluable for a different question, and this is the second time they decide one.
**The portion is kept** — it is what the paragraph overload literally reads, and it is exact on
the deck this seat is about — and the cost is this page.

---

# 3. `RRM-training-syllabus-…` 16 — the reference measures in one face and draws in another

`rrm.txt` is the record. Round 110's reading — *"the reference is drawing those stretches as
separate portions and accumulating each portion's rounding"* — is **refuted**: the reference's
line is a single text object at a single size in a single font,

```
BT 34.214 455.216 Td /F1 25.994 Tf [<3B>-162<02>-67<210733>-40<18>15 … ] TJ ET
```

and the gap `pymupdf` reports after each capital M is the `-162`, which is 0.162 em and 4.211 pt
at that size. There are no portions to accumulate anything.

Measured character by character out of the reference's own PDF, the cell each glyph was given is
**DejaVu Serif's advance** while the glyph drawn is **DejaVu Sans'**:

| char | drawn (em) | cell (em) | DejaVu Serif |
|---|---:|---:|---:|
| M | 0.862 | 1.024 | 1.024 |
| r | 0.411 | 0.478 | 0.478 |
| a | 0.612 | 0.597 | 0.596 |
| e | 0.615 | 0.593 | 0.592 |
| n | 0.633 | 0.643 | 0.644 |
| i | 0.277 | 0.320 | 0.320 |
| s | 0.520 | 0.515 | 0.513 |
| W | 0.988 | 1.028 | 1.028 |
| . | 0.318 | 0.318 | 0.318 |

Searched over every installed face, DejaVu Serif's total error is **0.0051 em** and the
next-best face's is 0.3235.

The run states `Book Antiqua` at 28 pt and the deck files Book Antiqua as family class *roman*;
on this machine `fc-match "Book Antiqua"` answers **DejaVu Sans** and
`fc-match "Book Antiqua,serif"` answers **DejaVu Serif**. That is the seventh confound
`dotnet/CLAUDE.md` already records — the declared class reaches editeng's measurement and
`FontAttribute` has no field for it, so `getVclFontFromFontAttribute` rebuilds the font at
`FAMILY_DONTKNOW` before the draw layer paints it
(`drawinglayer/source/primitive2d/textlayoutdevice.cxx`:416-448).

This tree measures and draws DejaVu Sans, so its line is narrower, wraps later, and its fit
lands one row higher: 20.01 against 18.99. **Ours is the self-consistent output**, and
reproducing the reference would mean laying text out in a face it does not draw. The page
leaves the statistic.

**Reach on this column, upper bound** (`fontclass.py`, a named face rather than a used one):
**22 of the 51 `.ppt`** name at least one face whose family class changes fontconfig's answer;
`Wingdings 2` in 9 documents, `Times` in 4, `Book Antiqua` in 3, `Helvetica` in 2.

---

# 4. The four "not size questions", reclassified with a measurement

`reclassified.txt` is the record. Round 110's figures were `sizes.py`'s dominant-*size* bucket,
which is why one of the four reads as text presence and is not. Whole-page counts against the
banked 26.2.4.2 renderings:

| page | ref alnum | ours | ref images | ours | ref draws | ours |
|---|---:|---:|---:|---:|---:|---:|
| `Thailand17` 8 | 92 | 592 | 2 | 1 | 2 | 180 |
| `W3_Case_Study…` 10 | 92 | 592 | 1 | 0 | 3 | 181 |
| `Fundamentals_Module_1_basics` 6 | 92 | 210 | 6 | 565 | 6 | 165 |
| `Thailand17` 11 | **595** | **595** | 3 | 3 | 72 | 71 |

**Three of the four are the raster ceiling**, with the sign that register describes: the
reference draws a *picture* where we draw the picture's own text. On the first two — the same
slide in two decks — the reference's whole page is its title drawn twice plus two images, and
this tree decodes the pasted metafile and draws its table as 592 real alphanumeric characters.
Neither deck has an `ObjectPool`, so it is a pasted metafile rather than an embedded object.
**Ours is the searchable output** and the gate scores it as failure; they belong in
`dotnet/TODO.raster-ceiling.md`'s class and not in a size statistic.

**The fourth is a size question after all, and a live one.** `Thailand17` 11 draws every one of
595 alphanumeric characters on both sides, in the same table cells, and differs only in the size
a subset is drawn at — 26.2.4.2 draws 548 at 11.99 pt where this tree draws 358 at **11.00** and
190 at 11.99. That is a per-cell autofit in a `.ppt` table. It **keeps a seat**; round 110's
reading of it as *"text presence the other way, 548 against 358"* is withdrawn.

---

# 5. The two pages the instrument cannot resolve — re-verified, and terminal

`unresolvable.txt` is the record. Three renders per document with 26.2.4.2: the `.ppt` to PDF,
the `.ppt` to flat ODP, that ODP to PDF.

| document | page | 26.2.4.2 reading the `.ppt` | 26.2.4.2 reading its own ODP |
|---|---:|---|---|
| `2015-Civil-Rights-Website-training` | 22 | 14.0 / 92 alnum | **14.99** / 90 alnum |
| `gfopportunitiesforlinkagespres_2010_en` | 27 | 25.99 / 262 alnum | **28.01** / 262 alnum |

Both move by a full `constScaleLevels` row, so the claim holds and is not inherited: on these
two pages the reference does not agree with itself across its own round trip, and every leg of
the instrument that runs through the ODP measures a different document from the one under test.

**The control matters and is what makes this specific rather than a blanket doubt**: on the four
O46 pages of the France deck the same three renders agree exactly, both ways. The round trip is
faithful where it is faithful.

Recorded as an **established instrument limit**. For the record, on
`gfopportunitiesforlinkagespres_2010_en` 27 this tree answers 28.01 — the reference's own ODP
answer exactly.

---

# 6. Reach — measured, over the whole slide corpus, base against head

`confine.py` renders a list with one CLI and records pages (from the real page tree, via
PyMuPDF — **not** `slides-r99/tfy.py`, which miscounts 45 of 51 banked reference PDFs),
alphanumeric characters and the PDF's md5, one directory per *document*, deleting each render as
it goes. Both binaries were built before either sweep started and neither was rebuilt while one
was running.

```sh
probes/slides-final-r111/confine.py <cli> /home/user/sample-files  ppt.list  out.tsv 3
probes/slides-final-r111/confine.py <cli> /home/user/corpus-odf    odp.list  out.tsv 3
```

| column | documents | rendered both legs | **renderings that move** | page counts differing | alphanumeric counts differing |
|---|---:|---:|---:|---:|---:|
| `.ppt` | 51 | 51 | **8** | 0 | 0 |
| `.pptx` | 251 | 251 | **0** | 0 | 0 |
| `.odp` | 302 | 302 | **0** | 0 | 0 |
| **total** | **604** | **604** | **8** | **0** | **0** |

## 6.1 Ink, over the eight renderings that move

`inksweep.sh` scores **every** differing page against the banked 26.2.4.2 renderings rather than
only the MAJOR ones, at both legs.

| | base | head |
|---|---:|---:|
| sum \|ink\|% over the eight | 47.94 | **38.66** |
| MAJOR pages | 11 | **12** |
| improve / worsen / level | — | **7 / 1 / 0** |

`ws_prod-…-M.017-(French)-France` is the largest single movement, **20.08 → 17.50**, and the one
that worsens is `ws_prod-g-doc-Events-Part-M-presentation`, 1.86 → 2.09 — §2.3.

## 6.2 The registered statistic

`sizesweep.py` over the 51 `.ppt`, scored with `slides-r107/sizescore.py` against the banked
reference table (`size-summary.txt`).

| | base = `a362d41f9` | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **11** | **10** |
| documents holding one | 7 | 8 |
| total \|size error\| over 1534 pages | **54.10** | **52.12** |
| **fixed / newly wrong** | — | **2 / 1** |
| pages whose dominant size moved at all | — | **3 of 1534** |

Fixed: `ws_prod-…-M.017-(French)-France` **8** (15.0 → 15.99) and **15** (17.01 → 18.99), both
the reference's own answer. Newly wrong: `ws_prod-g-doc-Events-Part-M-presentation` **21**
(17.01 → 18.0) — §2.3. The base column was measured on the base binary this round and
reproduces round 110's head column exactly.

The eight are **exactly the eight documents `census.py` named in advance** as carrying a master
level that states a blip, which is the reader split measured on the corpus rather than argued.
A count of markup would have said something much larger: 120 master levels carry one.


---

# 7. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then each project alone
with `--no-build`, individually rather than as a solution.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 573 / 573 |
| Markup | 259 / 259 |
| OpenDocument | 160 / 160 |
| Rendering | 164 / 164 |
| Spreadsheets | 1346 / 1346 |
| Text | 728 / 728 |
| Vector | 309 / 309 |
| WordProcessing | 1938 / 1938 |
| **Presentations** | **1136 / 1136** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else, confirmed **by name** in the log:
`TabStopComparisonTests` ×4 (`list-label-overrun.doc/.docx/.fodt/.odt`),
`PageDrawingComparisonTests` ×4 (`paginated.doc/.docx/.fodt/.rtf`),
`SheetDrawingComparisonTests` ×1 (`sheet-rich-text.xlsx`) and
`JustificationShrinkComparisonTests` ×1 (`justify-shrink-2013.docx`). **No eleventh.** Every
project reported its full total and none was aborted. Presentations' 1136 is round 110's 1114
plus the **22** new `PptPictureBulletTests`; Core's 573 and Spreadsheets' 1346 are other seats'
merges into the base.

**The new tests could not have been red at the base**: every one of the 22 calls a type or an
overload the base does not have, so "measured failing before the change" is not available for
them and is not claimed. What is claimed instead is that each merge test isolates one arm of
`ImplGetExtNumberFormat`'s merge, and that the nine size cases are 26.2.4.2's own stated
numbers rather than this tree's output.

---

# 8. Looking at the pages — and the control that does not exist here

**The blind reading the brief asked for could not be delegated, and this round checked twice
rather than assuming.** `ToolSearch` over the whole deferred-tool set for a subagent-spawning
tool returns `TaskStop`, `SendMessage`, `EnterWorktree` and the GitHub tools and **no `Task` and
no `ListAgents`**; `mcp__Claude_Code_Remote__create_session` spawns a sibling in its own
container, which cannot open a local PNG and has no channel back. `page-vision`'s own warning
says to check once and stop looking, and that is what this is.

**So every reading below is the seat's own and contaminated**, and each is corroborated by
arithmetic that does not depend on it:

* the picture bullet's placement and size — corroborated by `get_image_info`'s six corners and
  by the reference's own eight `fo:height` values, neither of which needs a reading;
* the label-width residual — corroborated by the two text left edges, 85.10 and 70.09, and by
  the deck's own two `text:min-label-width` values;
* the `RRM` face — corroborated by the TJ array and by an exhaustive search over installed
  faces, and the reading contributed nothing to it at all.

The one page looked at as an image is page 8 of the France deck, both halves rendered at 110 dpi
and composed with `page-vision/scripts/compose.py` (1100 x 1724, stacked, shown at 100 %; deleted with the
rest of the round's renders, and reproducible in about twenty seconds from the two PDFs). What it shows: the same six bullets in the same places at the same size on both
halves, and our body text starting visibly further left. Both of those were then measured, and
the measurement is what is quoted above.

---

# 9. What remains open, and why

| page | what it is | seat |
|---|---|---|
| `ws_prod-…-France` 14 | LAYOUT: both readers give this tree 20.01 against 18.99 on identical input | O15 |
| `ws_prod-…-France` 16 | IMPORT: the `.ppt` reader's label width, 70.09 against 85.10 | O46 |
| `ws_prod-…-Part-M-presentation` 21 | which font height sizes a picture bullet's box — the paragraph's portion or its master level's (§2.3) | O46 |
| `Thailand17` 11 | a `.ppt` table's per-cell autofit: 358 of 595 characters drawn a point small | new |
| `Thailand17` 8, `W3_Case_Study` 10, `Fundamentals_Module_1_basics` 6 | raster ceiling: the reference draws a picture, we draw its text | leaves O15 |
| `RRM-training-syllabus-…` 16 | the reference measures in DejaVu Serif and draws DejaVu Sans | leaves O15 |
| `2015-Civil-Rights-Website-training` 22, `gfopportunitiesforlinkagespres` 27 | the instrument cannot be run: the reference disagrees with itself across its own round trip | terminal |

---

# 10. The files

| | |
|---|---|
| `pptdump.py`, `census.py` | the record walk of the deck, and the whole-column census of master blips and `hasAnm` |
| `fourleg.txt` | the four-leg table, the six bullet placements, and the label-width residual |
| `rrm.txt` | the TJ array, the per-character cells, the face search and the family-class census |
| `fontclass.py` | that census: every `FontEntityAtom` of the 51 `.ppt` against `fc-match` with and without its class's generic |
| `reclassified.txt` | the four pages at whole-page level, against the banked 26.2.4.2 renderings |
| `unresolvable.txt` | the two pages' round trip, re-verified, with the France deck as the control |
| `confine.py`, `confine.txt`, `base-*.tsv`, `head-*.tsv` | the whole-corpus confine: pages, alphanumerics and md5 for 604 documents at both legs |
| `sizesweep.py`, `base-sizes.tsv`, `head-sizes.tsv`, `size-summary.txt` | O15's statistic at both legs, scored against `slides-r107/sizes-ref.tsv` |
| `inksweep.sh`, `ink-ppt.txt`, `moved-ppt.list` | ink over every differing page of the eight renderings that move |
| `ppt.list`, `pptx.list`, `odp.list` | the three columns |
| `tests.log` | the suite |
