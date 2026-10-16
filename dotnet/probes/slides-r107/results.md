# slides-r107 — the Escher WordArt reader (O13), and one row of O15's residual

This round resumed a session whose container was restarted mid-edit. The salvage commit
`95629cbd3` was treated as unvalidated throughout: everything below was rebuilt, re-measured
and re-cited from scratch, and two of its claims did not survive.

**Where the citations come from.** The C++ read below is `/home/user/libreoffice-core`, whose
`configure.ac` says `27.2.0.0.alpha0+`. The reference binary is
`/opt/libreoffice26.2/program/soffice`, **26.2.4.2**, whose build is not an object in that tree.
So every mechanism here has two legs: a source leg, which is a *later version's* explanation,
and a measurement leg against the actual 26.2.4.2 binary or its banked renderings. Where only
the source leg exists it is said so.

---

## 1. O13 — an Escher WordArt shape drawn as Fontwork

### 1.1 What states it

A shape is WordArt on one bit and not on its type:
`bIsFontwork = ( GetPropertyValue( DFF_Prop_gtextFStrikethrough, 0 ) & 0x4000 ) != 0`,
`filter/source/msfilter/msdffimp.cxx`:4424-4425, and the same test again in
`DffPropertyReader::ApplyAttributes` at :2517. The type only chooses which preset is bent:
`svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx`:171 maps `fontwork-plain-text` to
`mso_sptTextPlainText` and :179 maps `fontwork-arch-up-curve` to `mso_sptTextArchUpCurve`;
`include/svx/msdffdef.hxx`:412-451 numbers the family contiguously from 136 to 175.

> The brief was right to correct an earlier round here. `msdffimp.cxx`:2516-2600 is the
> `TextPath` property set, not the type mapping; it is cited below only for what it is.

### 1.2 Census — two shapes, and the second instrument agrees

`fgtext.py` walks the record tree of each `.ppt` from offset 0. A tree walk can desync on one
bad length and silently under-count, so `fg2.py` was written with a different failure mode: it
does not walk at all, it scans every byte offset for a plausible `msofbtOPT` header
(`ver == 3`, `recType == 0xF00B`, `len >= 6 × instance`) and reads the property array there.

    ## files 51   candidate OPT 15032   fGtext shapes 2      (fg2.txt)
    # shapes with fGtext: 2   in 136..175: 2   outside: 0     (fgtext.txt)

Both find the same two, at the same offsets (`fgtext.py` reports the `msofbtSp`, `fg2.py` the
`msofbtOPT` sixteen bytes later). The denominator matters: 15 030 of 15 032 candidate property
records do **not** set the bit, so this is a sparse signal rather than a saturated one.

| document | type | preset | prop 255 | adjustValue | fill |
|---|---|---|---|---|---|
| `pres_ioc_phuket.ppt` | 136 | `fontwork-plain-text` | `0xFFFF5700` | 10707 | `fillType` 5, `fillColor` 0x00FFFF, `fillBackColor` 0x3399FF |
| `8.16_AOD_FINAL_…ppt` | 144 | `fontwork-arch-up-curve` | `0xF2BB5200` | `0xFF4C000B` | `fillColor` 0, **no `fillType`** |

Neither shape is outside the 136–175 band, so on this corpus nothing reaches the "WordArt bit on
a non-text-path type" case.

### 1.3 Why the box is not drawn, and where the fill comes from

`EnhancedCustomShapeEngine::render2` builds the custom shape's line geometry, then
**replaces it outright**:

    rtl::Reference<SdrObject> xRenderedFontWork( EnhancedCustomShapeFontWork::CreateFontWork(...) );
    if (xRenderedFontWork) { xRenderedShape = std::move(xRenderedFontWork); }

— `svx/source/customshapes/EnhancedCustomShapeEngine.cxx`:250-259. So the shape's own outline,
its preset geometry and its shadow are not drawn at all, which is what the `PptSlideLayout` hunk
returns early for.

The glyphs are filled with the **shape's** fill, because
`CreateSdrObjectFromParagraphOutlines` copies the whole item set onto the path object with only
the shadow and the text direction cleared —
`SfxItemSet aSet(rSdrObjCustomShape.GetMergedItemSet())`,
`EnhancedCustomShapeFontWork.cxx`:1123-1126. An Escher WordArt has no runs to have copied a
character fill from, so `fillType`/`fillColor` — `PptFills`'s ordinary answer — is what fills
them. That is why the caller passes the shape's resolved fill and no new paint kind was needed:
the phuket shape's `fillType` 5 already resolves to a `GradientPaint`.

### 1.4 Details of the decode, each read out and each checked against the two witnesses

* **`ScaleX` is literal on this path.** `ApplyAttributes` writes it from bit `0x40` of property
  255 (`msdffimp.cxx`:2557). The DrawingML derivation that turns the four arch presets on lives
  in `oox/source/drawingml/fontworkhelpers.cxx`:172-178 and is explicitly gated on
  `!bFromWordArt`, so it must not run here. Both witnesses have the bit clear.
* **Only `ScaleX` and `SameLetterHeights` are read back.** `EnhancedCustomShapeFontWork.cxx`
  reads exactly those two off the `TextPath` set (:107-111, :521-523). `TextPathMode`, which
  `ApplyAttributes` computes at :2528-2554 and which differs between the two witnesses
  (the arch resolves to `NORMAL`, the plain text to `SHAPE`), is read back by nothing in this
  tree except the exporter, `filter/source/msfilter/escherex.cxx`:3312 — so not modelling it
  costs nothing.
* **A polar adjustment is a 16.16 degree.** `msdffimp.cxx`:2163-2174 sets a conversion bit for
  each default handle that is `POLAR` with `nPositionY >= 0x256 || <= 0x107`, and :2594-2599
  divides that adjustment by 65536. Across the whole WordArt vocabulary exactly seven handle
  tables carry `POLAR` — `mso_sptTextArchUpCurveHandle`, `…ArchDownCurve…`, `…CircleCurve…`,
  `…ButtonCurve…`, `mso_sptTextArchPourHandle` (shared by 148 and 149), `…CirclePour…` and
  `…ButtonPour…` — covering shape types 144 to 151, each with one handle at `nPositionY = 0x100`.
  The other twenty-seven text handle tables are `RANGE` and are converted by nothing. A pour's
  second adjustment, its radius, is not converted, because the bit is set per handle and a pour
  declares one.
* **Line breaking.** `SvxMSDffManager::ReadObjText` (`msdffimp.cxx`:3698-3717) breaks on `0x0a`
  and on `0x0d` alike and swallows the other when it follows, so `CRLF` and `LFCR` are one break
  and a lone `CR` is a break of its own.
* **Alignment.** `msdffimp.cxx`:4467-4484; the default for an absent `gtextAlign` is
  `mso_alignTextCenter`, and the three justifying modes become `SDRTEXTHORZADJUST_BLOCK`, which
  the layouter treats as left (`EnhancedCustomShapeFontWork.cxx`:621,
  `case SDRTEXTHORZADJUST_BLOCK : break;   // don't know`). Neither witness states property 194.
* **Size.** `gtextSize` is a point in 16.16 fixed point (`msdffimp.cxx`:2626-2627 through
  `ScalePt`).

**What is not modelled, and why the corpus cannot see it.** `IsHardAttribute` gating on bold and
italic (`msdffimp.cxx`:4447-4451) — both witnesses state the attribute *and* clear the value bit,
so the answers coincide; `SameLetterHeights` (bit `0x80`) — clear on both; `gtextSpacing`'s
`SvxCharScaleWidthItem` — stated by neither; vertical writing (bit `0x2000`) — the value bit is
clear on both. An adjustment *gap* (a stated adjustment `n` with an unstated `m < n`) becomes
zero here where the reference leaves the preset default; both witnesses state only
`adjustValue`, so no gap exists. These are latent, not reachable on this corpus.

### 1.5 Reach — measured, base against head, nothing else differing

`base` is `95629cbd3` with the `PptSlideLayout` hunk reverted and rebuilt; `head` is the hunk in
place. Both compared against the banked 26.2.4.2 renderings in `/home/user/gate-orig-r83/ref`
with `pdf-image-diff.py`. Full table in `ink.txt`.

| document | pages | sum \|ink\|% base → head | MAJOR base → head |
|---|---|---|---|
| `8.16_AOD_FINAL_…ppt` | 94 | 20.34 → **4.56** | 2 → 1 |
| `pres_ioc_phuket.ppt` | 26 | 4.70 → **2.96** | 2 → 1 |

Page by page, exactly two pages moved in the two documents and **no page is worse**:

* AOD page 59: `16.04 → 0.26`, MAJOR cleared.
* phuket page 26: `1.97 → 0.23`, MAJOR cleared.

**The brief was wrong about the second half.** It placed the phuket shape on page 1 and priced it
at 0.01. Page 1 measures 0.01 in *both* legs and does not move; the shape is on page 26 and is
worth 1.97. The consequence is the opposite of the brief's caution: the gradient half is not a
rounding-error page, it is a MAJOR page of its own.

Read on the rendered pages — **not a blind reading, this session has no second reader** — the
arch on AOD 59 and the gradient banner on phuket 26 both match the reference. The ink figure is
the ranked evidence; the look was a check that we were not drawing differently-shaped ink of the
same quantity.

**Corpus-wide:** the reader reaches 2 documents of the 51-document `.ppt` column and 0 of the
251 `.pptx` (Escher WordArt is a binary-only construct). Nothing else in the corpus states the
bit, by the two independent censuses above.

### 1.6 The OOXML sibling is nil-reach, separately

`warpfill.py` walks every slide, layout and master of the 251 `.pptx`, finds each body whose
`a:bodyPr/a:prstTxWarp` states a warp other than `textNoShape`, and reports the fill of the run
the reference would copy character properties from (`oox/source/drawingml/shape.cxx`:721-905):

    # warped bodies: 21 in 3 documents
    #   solidFill 15   (none stated) 6   gradFill/blipFill/pattFill/grpFill 0
    # documents with a non-solid first-run fill: 0

Re-run this round; identical to the salvaged output. So `SlideFontwork.Read`'s
`Paint.Solid(stated.Colour)` has **nil reach** on this corpus: 0 of 251 documents state a warped
body whose first run carries a fill a solid colour cannot draw.

**O13 ends: fixed in this tree.**

---

## 2. O15 — the escapement line box, two of the nine

### 2.1 What was recovered, and the two claims that did not survive

The killed session left an uncommitted edit to `SlideTextLayout.cs` in its scratch directory. It
is not part of `95629cbd3`. Recovered, re-cited, corrected in two places, given a test, and
re-measured from scratch.

### 2.2 The rule

`ImpEditEngine::CreateLines` finishes a line by walking every non-`LINEBREAK` portion through
`RecalcFormatterFontMetrics` and taking the largest ascent and the largest descent **separately**
(`editeng/source/editeng/impedit3.cxx`:1496-1519). Under `IsFixedCellHeight()` that function
answers `ascent = fontHeight` and
`descent = ImplCalculateFontIndependentLineSpacing(fontHeight) − fontHeight` (:3138-3142), and
`ImplCalculateFontIndependentLineSpacing` is `basegfx::fround(h × 12.0/10.0)` (:501-505). So an
unescaped line is `fround(1.2 em)` and that is the rule this tree already had.

An escaped portion is measured **twice** (:3164-3183). The function first forces the proportion
back to 100 (:3121-3128, *"for line height at high / low first without Propr!"*) and takes the
metric, so an escaped portion contributes the *full* ascent like any other; then

    nDiff = fontSize.Height() * escapement / 100;
    if (escapement > 0) nAscent  = nAscent  * nPropr / 100 + nDiff;   // and max against the line
    else                nDescent = nDescent * nPropr / 100 - nDiff;

For a subscript the second term is a subtraction of a negative, so the descent grows by the whole
of the drop; a superscript does the same to the ascent. Neither ever shrinks the line, because
both answers are compared against the unescaped one the line already carries.

At 24 pt (847 hundredths of a millimetre) a plain line is `fround(847 × 1.2) = 1016`, and a
`-25% 58%` subscript's descent is `169 × 58/100 + 847 × 25/100 = 98 + 211 = 309` against the
plain 169, so the line is 1156 — 3.97 pt taller. On an autofitted body that is enough to change
which `constScaleLevels` row fits.

### 2.3 The variant experiment at 26.2.4.2 — and where the salvaged comment was wrong

Full record in `variant.txt`. `pods05.ppt` was flattened to ODP by 26.2.4.2, edited in one
place, and rendered back through the same binary.

Page 32 is the page the round trip is faithful on: the unmodified re-render gives 9.01 pt over
30 alphanumerics, exactly the direct `.ppt → pdf` reference. (Page 33 is *not* faithful under the
round trip — 11.0 against 9.01 — so it carries no part of this.)

| variant | page 32 dominant size |
|---|---|
| unchanged (`-25% 58%`) | 9.01 |
| offset removed (`0% 58%`) | **11.0** — exactly what this tree drew before the change |
| shrink removed (`-25% 100%`) | 9.01 — unchanged |

**It is the drop that costs a row, not the shrink.** The salvaged comment asserted that setting
the proportion to 100 "makes it shrink further still, which is the other half of the formula".
It does not: the extra descent proportion 100 produces (445 hundredths against 361 at 28 pt) does
not cross a row boundary here. That sentence has been replaced by what was measured.

Three other repairs to the salvaged text: the new method had been pasted **inside**
`LargestSize`'s doc comment, leaving that method undocumented and two `<summary>` elements on
one declaration; `impedit3.cxx:500-504` is really 501-505, `:1495-1519` is 1496-1519,
`:3160-3182` is 3164-3183, `:1478-1491` is 1480-1491; and a comment referred to a symbol
`Escaped` that does not exist.

### 2.4 Test

`SlideEscapementTests.ASubscriptMakesItsLineTallerAndASuperscriptDoesNot`. Two paragraphs at
24 pt; the pitch between the two full-size baselines is 28.80 pt plain, must stay 28.80 pt with a
`+30%` superscript, and must become 32.77 pt with a `-25%` subscript. The asymmetry is the
assertion — a rule that added the rise to both sides, or to neither, fails one of the two.
Measured at the base commit it answers 28.80 for all three and fails; after the change it passes.

### 2.5 Reach

The registered statistic is the per-page dominant drawn text size (`sizes.py`, `sizescore.py`).
The reference table was **re-derived** this round from the 51 banked reference PDFs and is
byte-identical to the committed `probes/slides-r97/sizes-ref.tsv` — 1534 pages.

    base   pages differing >0.15pt   14   documents  11   total |size error|  62.06   same-alnum  9
    head   pages differing >0.15pt   12   documents  10   total |size error|  58.08   same-alnum  7
    fixed: 2   newly wrong: 0   pages whose dominant size moved at all: 2 of 1534

The two are `pods05__ppt` pages 32 and 33, both from the identical-text group — so the brief's
nine pages that are one row apart with identical alphanumeric counts are now **seven**. The
remaining twelve are listed in `size-summary.txt`.

On ink, `pods05.ppt` goes `7.20 → 6.78` sum |ink|% with its MAJOR count unchanged at 3: the
change moves the size statistic and a little ink but clears no MAJOR page.

**The `.pptx` column is neutral, and it was checked rather than assumed.** `FixedCellBox` can
only change an answer for a line carrying a non-zero escapement, so `esccensus.py` bounded the
reachable set: 33 of 251 documents state one, a 13.1 % base rate. All 33 were rendered at base
and at head. Seven renderings changed at all; **zero** changed page count and **zero** changed
alphanumeric count — the gate's verdict column is untouched. Over those seven the net ink change
is −0.17 |ink|% with no MAJOR page gained or lost (`pptx-esc.txt`).

**O15 ends: still seated.** Two of its fourteen pages are closed and the mechanism for those two
is read out and measured; twelve remain, of which seven are one row apart with identical text and
five differ in alphanumeric count too and are a different question.

---

## 3. What could not be settled

* **Page 33 of `pods05.ppt`** is fixed by the change and the size score says so, but the flat-ODP
  round trip is not faithful on that page, so the single-variable experiment could only be run on
  page 32. The mechanism is the same one; the *evidence* is page 32's.
* **The source leg is a later version.** Every `file:line` above is
  `/home/user/libreoffice-core` at `27.2.0.0.alpha0+`. The measurement legs — the ink diffs
  against the banked 26.2.4.2 renderings, and the `--convert-to` variant run — are of the real
  reference and stand on their own.
* **The remaining twelve O15 pages.** `fitedge.py`'s bisection agreed 84 of 84 on height and
  164 of 164 on width in an earlier round, so the block arithmetic and the line breaking are
  exact; the escapement term was a third input to the same block height and it accounted for two
  pages. What accounts for the other seven identical-text pages is not established here.
* **The latent Escher gaps in §1.4** are unreachable on this corpus and therefore untested. They
  are named so a future round with a witness has a list rather than a search.

## 4. Files

| file | what it is |
|---|---|
| `fgtext.py` / `fgtext.txt` | fGtext census by record-tree walk |
| `fg2.py` / `fg2.txt` | the same census by brute-force OPT scan — a different failure mode |
| `wordart-props.py` / `wordart-props.txt` | the two witnesses' property tables, decoded |
| `warpfill.py` / `warpfill.txt` | the DrawingML warped-body fill census, 251 `.pptx` |
| `ink.txt` | O13's reach, base against head, per document and per page |
| `sizes.py` / `sizescore.py` | the registered O15 statistic and its scorer |
| `sizesweep.sh` | renders a list of `.ppt` and records `sizes.py` for each, deleting as it goes |
| `sizes-ref.tsv` / `sizes-base.tsv` / `sizes-head.tsv` / `size-summary.txt` | the three legs and the score |
| `variant.txt` | the `pods05` escapement variant at 26.2.4.2 |
| `esccensus.py` / `esc-pptx.txt` / `pptx-esc.txt` | the `.pptx` reachable set and the regression check |
| `sweep-ours.sh` | pages / alphanumerics / md5 for a list of documents |
| `ppt.list` / `pptx.list` | the two corpus columns |
| `tfz.py` | page counter that walks the real page tree (slides-r100's; `tfy.py` miscounts) |
