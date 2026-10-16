# A lone SRCAND is a bitmap masking itself, and `@fontScale` moves nothing at all

Round 94, slides track. Base `725bbbfd2`, worktree `/home/user/wt-slidesink`, branch
`agent/slidesink`, 2026-09-11.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, the five tarball font confounds aside |
| `/usr/bin/soffice` | 24.2.7.2 — **not used for anything in this round** |
| reference bank | `/home/user/gate-orig-r83/ref`, 947 PDFs; nothing re-rendered through `soffice` except the twenty autofit variants of §4 |
| our half | rendered twice from this worktree, `SOURCE_DATE_EPOCH=1700000000`, one output directory per *document* |
| C++ tree | `/home/user/libreoffice-core`, read only — this worktree is a sparse checkout and has no `emfio/` |

**No build overlapped a sweep.** The base leg was built by `cp`-ing the three changed files aside,
`git checkout --`, `touch`, and `rm -rf obj bin` across every `src/` and `tools/` project; the head
leg is the binary the fixed tree builds. **The check that the two legs are what they claim**: the
final rebuilt binary re-renders `FAA_Form_337.ppt` byte-identically to the head bank, and
`171128IPAP.pptx` — a non-mover — byte-identically to *both* banks.

**There is no `Task`/subagent tool in this container**, and `create_session` spawns a sibling that
cannot open `/home/user/...`. So every reading below is mine and is contaminated. Each is
corroborated by arithmetic that does not depend on it: the extracted image planes in §1, the
content-stream rectangles in §2 and §3, the drawn `Tf` sizes in §4, and the two-leg sweep in §5.
One reading of mine was wrong and §1 says so.

---

## 0. Two corrections to the brief's own figures, before anything else

The brief names *"`FAA_Form_337.ppt` — page 67 (8.97 summed |ink| over 49 pages)"*,
*"`pres_ioc_phuket.ppt` — page 26 (6.79 over 21 pages)"* and
*"`NAS-Infrastructure-Roadmaps-Weather.pptx` — 9.58 over 28 pages"*.

**The page counts are not those documents'.** Measured with `pdfinfo` on both sides of the banked
gate: FAA_Form_337 is **67 pages** on both, `pres_ioc_phuket` **26**, and
`NAS-Infrastructure-Roadmaps-Weather` **11**. So "page 67" and "page 26" are each the *last* page of
their deck, not a page 67 of 49. And the summed figures do not reproduce either: scored the same way
against the same banked reference, over exactly the renderings the brief points at
(`/home/user/ink90/ours`, at `1184c6316`), this round measures **13.81**, **7.78** and **11.30**
against the brief's 8.97, 6.79 and 9.58. The ranking's *order* is
intact and its two named pages are real; its denominators are not, and I could not find the run that
produced them.

---

## 1. FAA_Form_337 page 67: one `SRCAND` blit with nobody to pair with

### What the picture said

`pdf-image-diff.py` on the banked pair puts page 67 at **|ink| 2.73, MAJOR**, with one region read as
*ink missing from ours* at x 0.05–0.29, y 0.10–0.34 — the top-left quarter, where the deck draws the
Department of Transportation triskelion. A 300 dpi crop (`faa67-before.png`) shows it outright: the
reference draws the black disc straight onto the slide's blue, and this tree draws it inside a
**white rectangle** 168 × 121 pt.

### What the picture could not decide, and what settled it

Three causes fit that image — a picture frame we fill white, a transparency we drop, or a crop we do
not apply. `pdfimages -list` separates them in one line:

| | page 67 image |
|---|---|
| 26.2.4.2 | `199 × 144 gray` **plus a `199 × 144` `/SMask`** |
| this tree | `199 × 144 rgb`, no mask |

and decoding the three planes settles it with no tolerance anywhere: the reference's colour plane is
**28 656 pixels of pure black**, its mask is **20 299 zeros and 8357 opaque**, and our RGB plane is
**20 299 white and 8357 black** — the same two counts. The reference is drawing our bitmap with its
white knocked out.

### The seat, and it is not `pictureTransparent`

`EscherPicture.TransparentColour` already models MS-ODRAW property 263, and **no shape in this deck
states it** (`census-gtext.py`'s walker over every OPT record: 0). The transparency is inside the
picture. The blip is `msofbtBlipWMF` (`0xF01B`), and the whole metafile is 71 records of which
**exactly one draws**: a `META_DIBSTRETCHBLT` of a 199 × 144 **1-bpp** DIB at
`rop = 0x008800C6`, `SRCAND`.

`RasterOperations.IsTransparentPair` knows `SRCAND`, but only as the *first half* of the two-record
idiom — a monochrome mask blitted `SRCAND` and then the colour image `SRCPAINT`. Here there is no
second record, because **the artwork is its own mask**: it is black on white, and ANDing with white
is the identity. `PendingBlit.PairsWith` therefore never fired, `FlushBlit` warned `PL6033` and drew
the source opaquely, and the white ground of the bitmap became a white panel on a blue slide.

LibreOffice resolves it in the same function as the pair, one branch further down.
`MtfTools::ResolveBitmapActions` switches on the **low nibble of the operation's middle byte**
(`emfio/source/reader/mtftools.cxx`:2662-2799), and case `0x8` — which `SRCAND`'s `0x88` reduces to —
is three lines:

```cpp
Bitmap  aMask( aBitmap );
…
Bitmap aBmpEx( aBitmap, aMask );
ImplDrawBitmap( aPos, aSize, aBmpEx );
```

with the bitmap as its own mask. The pattern branch above it (`:2691-2699`) only fires when `nUsed`'s
pattern bit is set, and for `0x88` it is not: `(0x88 & 0xf) == (0x88 >> 4)`, so the source and the
destination are used and the brush is not.

### The change

`RasterOperations.IsSelfMasked(uint)` is `((operation >> 16) & 0x0F) == 0x08`, and `WmfReader` and
`EmfReader` each gain a four-line `SelfMask` that merges the pending blit's pixels with themselves —
`RasterOperations.Merge(pixels, pixels, invertMask: false)`, the same merge the pair already used.
**Case `0x7` is deliberately left to the warning**: it is the identical construction followed by an
inversion of the destination, which a display list cannot read back, so drawing the mask without the
inversion would be a different picture where drawing the source without the mask is at least the
right artwork.

### What it is worth on that document

|ink|% against the banked reference, per page:

| | page 2 | page 67 | whole document (67 pages) | MAJOR pages |
|---|---:|---:|---:|---:|
| before | 2.73 | 2.73 | **13.81** | 7 |
| after | 0.07 | 0.07 | **8.49** | 5 |

The residual 0.07 is agreement: the drawn disc measures **97.44 × 96.00 pt at (66.72, 64.56)**
against the reference's **97.68 × 96.48 at (66.72, 64.32)**, read as the near-black bounding box of a
300 dpi raster of each side — within a quarter of a point in origin and extent, which is one pixel of
the source bitmap.

### The reading of mine that was wrong, and the arithmetic that caught it

Looking at `faa67-after.png` I recorded *"the reference's disc is noticeably larger than ours,
perhaps six per cent"*. It is 0.25% larger. The composed pair is 300 dpi and the two halves are the
same crop, so there was no excuse; the bounding-box measurement above is what corrected it, and it is
the reason that measurement is in the write-up at all. **A size read off a picture is a guess even at
300 dpi.**

---

## 2. `pres_ioc_phuket` page 26: a WordArt title drawn as a blank bar — diagnosed and left

|ink| **4.28**, MAJOR, and two independent defects on one band. A 150 dpi crop of the top 144 pt
(`phuket26-wordart.png`) shows both.

**(a) The title.** 26.2.4.2 draws `INTERNATIONAL TSUNAMI HAZARD MITIGATION` in Impact italic filled
with a yellow-to-orange gradient; this tree draws **the gradient rectangle and no glyphs**. Neither
side puts the string in the text layer — `pdftotext -f 26 -l 26` returns it from neither — because
the reference *outlines* it: its content stream builds a long path of glyph curves, closes it with
`W* n`, and paints `q 681.165 0 0 36.85 12.104 491.216 cm /Im675 Do Q` through that clip. We paint
the same gradient over the same rectangle, (12.00, 491.12)–(693.38, 528.00) against the reference's
(12.10, 491.22)–(693.27, 528.07), with no clip.

The shape is Escher WordArt: its OPT record states **property 192 `gtextUNICODE`** =
`INTERNATIONAL TSUNAMI HAZARD MITIGATION`, **197 `gtextFont`** = `Impact`, 384 fill type 5, 385
`0x00FFFF` and 387 `0x3399FF`. `XRay.Office.Ooxml/DrawingML/Fontwork*.cs` implements the geometry for
DrawingML, VML and ODF; **no MS-binary reader reaches it** — there is no `PptFontwork` and
`git grep Fontwork` finds no hit under `XRay.Office.Presentations/MsBinary`.

**Left, on reach.** `census-gtext.py` walks every OLE2 stream of every `.ppt`, `.doc` and `.xls` for
an OPT record carrying property 192: **2 shapes in 2 of the 181 MS-binary documents** —
`pres_ioc_phuket.ppt` and `8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt`. A whole
Fontwork path for two shapes is the wrong trade while §1's three-line rule was unclaimed.

**(b) A second defect on the same band, measured and not characterised.** Both sides draw the white
panel behind `WARNING CENTER OPERATORS` at exactly (84, 369.6)–(654, 462). What differs is the dark
blue banner above it: the reference fills `#00007E` at **(−18.03, 436.65)–(719.97, 540.03)** and this
tree at **(−18.00, 475.12)–(720.00, 540.00)** — same top, same width, and **38.5 pt short at the
bottom**. Sampled at y = 100 pt from the top, the reference is `#00007E` at x = 10, 50, 660 and 700
and white between; we are white at all seven sampled columns. That is a shape height, not the
WordArt, and it is the larger of the two regions the diff reports. Not worked.

---

## 3. `NAS-Infrastructure-Roadmaps-Weather.pptx`: the coarse instrument overstates page 11

The third document in the brief's ranking, *"not yet characterised at all"*. It is 11 pages, four of
them MAJOR. Page 11 is the largest single contributor at **|ink| 3.52** and it is **not a layout
difference at all**:

- `pdf-ops.py diff` pairs **90 text records drawn by both at identical positions**, reports
  **0 records only in ours** and one only in the reference (the page-level white background fill).
- Every one of the 90 differs in exactly two ways: `size 9.00 vs 9.01` and the number of `TJ` shows.

Read out of the raw content streams, the mechanism is a transform rather than a size: the reference
draws the table's 87 runs at `/F 9.01 Tf` with **no `cm`**, and this tree draws them at
`/F 6.8504 Tf` inside `1.3151 0 0 1.3134 … cm` — an effective **9.009 wide × 8.998 tall** against
9.01 both ways. So we lay the table out at a nominal size and stretch it onto the frame where the
reference lays it out at its final size, and the residual is a 0.13% anisotropy. At
`pdf-image-diff.py`'s 512 px a 9 pt em is under five pixels, so dense small text at a sub-pixel
offset lights up the whole block; the region it calls *marks displaced or reshaped, 22.62% of the
page* is that.

Pages 4, 5 and 7 are genuinely reflowed — 12 text records only in ours and 115 only in the reference
on page 4, most of the reference's being one-glyph shows down the left margin of a roadmap chart.
That is a different question and is not this one. **Nothing changed for this document; it is
characterised and left.**

---

## 4. Seat two: `a:normAutofit/@fontScale` is not honoured, and the round that read it said to check

`probes/ppt-fit-r85/results.md` closes with a section read from the C++ and censused but explicitly
**not measured** — *"whoever takes it should measure before implementing"*. Measured, and the answer
is no.

`fontscale-variants.py` rewrites **one attribute** of one slide of a corpus deck, renders the whole
deck through 26.2.4.2, and reads the drawn `Tf` sizes of that page out of the reference's own PDF.
Two decks, ten variants each; `fontscale-variants.tsv` is the table.

**Witness A, `171128IPAP.pptx` slide 31** — states `fontScale="90000"` and the deck states no
`lnSpcReduction` anywhere:

| variant | drawn sizes |
|---|---|
| the file's own `fontScale="90000"` | 14.003×2 18×14 **29.991**×1 |
| `fontScale` removed (bare `<a:normAutofit/>`) | 14.003×2 18×14 **29.991**×1 |
| `fontScale="50000"` | identical |
| `fontScale="25000"` | identical |
| `fontScale="90000" lnSpcReduction="10000"` | identical |
| `fontScale="50000"` / `"25000"` / `"100000"` with `lnSpcReduction="10000"` | identical |
| `fontScale="25000" lnSpcReduction="50000"` | identical |
| **`<a:normAutofit>` removed outright** | 14.003×2 18×14 **32.003**×2 |

**Witness B, `NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` slide 5**, patching the element that
already states both attributes, `fontScale="25000" lnSpcReduction="20000"`: nine variants including
`fontScale="100000" lnSpcReduction="10000"` all draw `13.011×8 14.995×3`, and removing the element
draws `51.987×35 60.009×8`.

**So the stated pair moves nothing.** Only the presence of the element does. And the sizes the
reference does draw are the *search's* answer, not the file's: 51.987 × 0.250 = 12.997 and
60.009 × 0.250 = 15.002 against the drawn 13.011 and 14.995, which is `constScaleLevels`' last row,
`{0.250, 0.250, 1.0, 0.8}`, reached because the deck's own value happens to be near it.
**That coincidence is why this needs an absurd variant to test**: PowerPoint stores the scale
PowerPoint computed, LibreOffice's search converges near it, and a witness measured only at its own
value cannot tell the two apart.

**What survives of r85's section.** The census — *326 `a:normAutofit` state `fontScale`, 209 state
`lnSpcReduction`, the latter in 40 of the 251 `.pptx`* — reproduces (50 decks state a slide
`fontScale`, 326 elements). The source chain is really there. What does not survive is the
conclusion drawn from it, and one further reading of the C++ explains why a chain that exists still
decides nothing: `ScaleContentToFitWindow` (`editeng/source/editeng/impedit3.cxx`:304-333) seeds
`maScalingParameters` from the custom pair, formats **once**, and then — if that format overflows —
walks `constScaleLevels` **from row 0** regardless. The stored pair is at best a first guess that a
single overflow discards. The mechanism by which it fails to reach the outliner even as a guess is
**not established here**: `textbodypropertiescontext.cxx`:242-243 stores
`mnFontScale / 100000.0` and `unoshape.cxx`:2343-2352 then divides by 100 again, so a stated 90000
would reach `setupAutoFitText` as `0.009` and draw text at a hundredth of its size, which no variant
does. Whatever intervenes, **nothing in this tree should be built on the stored pair.**

`SlideAutofit`'s remark still says the stated scale is thrown away. That sentence was flagged as
stale by r85 and is **correct after all**, for a different reason than it gives; I have not rewritten
it, because the reason matters and belongs in one place, which is this file.

---

## 5. Reach and cost, measured on two full renders of the whole corpus

Our half of all **947** documents rendered at the round's base and again with the fix, byte-compared:

| | |
|---|---:|
| renderings byte-identical | **940** |
| renderings changed | **7** |
| pages changed on any of the 7 | **0** |
| alphanumeric characters changed on any of the 7 | **0** |

so **no gate verdict can move in either direction**, which is the `w:pgBorders` shape again.

`ink.tsv` has the seven; summed |ink|% against the banked 26.2.4.2 reference goes **48.67 → 42.84**,
and every one of the seven is level or better:

| document | before | after |
|---|---:|---:|
| `FAA_Form_337__ppt` | 13.81 | **8.49** |
| `3492__pptx` | 3.59 | **3.08** |
| the other five | 32.27 | 32.27 |

**The five that do not move on ink are not noise and are worth the paragraph.** Measured as the
maximum per-channel difference over a 100 dpi RGB raster of every page: `EG1_dsrc tech__ppt` is
**pixel-identical** (a byte change only), `airbus-pdf-information-package_v1-4__docx` differs by at
most **2** levels and `outlook_of_nigerian_pension_sector__ppt` by **1** — antialiasing, because
their blits sit on white paper where knocking white out changes nothing visible.
`EHEST-SMS…__docx` page 34 and `JEMIT_Template__docx` page 3 do change (255 and 64 levels) but over
115 and **42** sampled pixels; page 34's |ink| goes 0.02 → 0.02 and page 3's 0.35 → 0.35.

`3492__pptx` page 19 is the second real one and it is worth looking at, because the *right* answer
looks wrong: the fix turns the band behind an `A380` wordmark from white to **black**, and
`3492-page19-after-vs-ref.png` shows 26.2.4.2 drawing that band black too. |ink| 0.69 → 0.18 and the
page goes MAJOR → ok. **A self-mask can darken a picture as easily as it can lighten one**, and a
reviewer told only "transparency was added" would file this as a regression.

---

## 6. The census that briefed the reach was wrong twice, and the sweep is the authority

`census-srcand.py` reaches every metafile it can — zip parts for OOXML, Escher BLIP records inside
every OLE2 stream for the MS binaries — and classifies each `SRCAND` as *lone* or *paired*. Its first
cut answered **3 lone in 3 documents and 4 paired**, and the sweep then moved **7**. Both halves of
that gap are instrument defects, and they point opposite ways:

- **The raster operation is at a different offset in `EMR_STRETCHDIBITS` than in `EMR_BITBLT`** —
  `+68` against `+40`, because the DIB's five offsets and the usage sit between. Reading the BitBlt
  offset for all three record types made the census miss `3492.pptx`, `EHEST-SMS…docx` and two more
  outright and gave a nonsense `0xf3` for the ones it half-read. Corrected, the census answers
  **3 documents with a lone SRCAND — `FAA_Form_337.ppt` 1, `3492.pptx` 1, `JEMIT_Template.docx` 4 —
  and 8 with a paired one**, and `TICAPCapability_Final.xls`'s 8 move from the lone column to the
  paired one, which is why that document does not appear among the movers.
- **A census's idea of "paired" is not the reader's.** `PendingBlit.PairsWith` also requires the two
  bitmaps to have the same pixel dimensions and the two destinations to be the same *mapped*
  `DocRect`; where either differs the pair does not merge and the `SRCAND` half now self-masks. That
  is why four of the eight paired documents moved at all, and it is exactly the ≤2-level class above.

**Rank on the sweep, not on the census.** The census is worth keeping for what it says about *where*
to look; the seven movers came from rendering 1894 documents and diffing them.

---

## 7. Tests

Ten non-fidelity projects run individually at the round's head, **0 failed and 0 skipped
everywhere**: Containers 109, Core 521, Markup 259, OpenDocument 146, Presentations 1044,
Rendering 164, Spreadsheets 1245, Text 728, **Vector 309**, WordProcessing 1864 — 6389 in all.
Vector is 302 at r85's baseline and gains seven.

`XRay.Office.Fidelity.Tests`: **542 passed / 10 failed of 552, 0 skipped** — the briefed baseline
exactly, and the same ten (`PageDrawingComparisonTests.EveryLineIsDrawn` ×4,
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`,
`JustificationShrinkComparisonTests`).

**One existing test asserted the opposite of the fix and its premise was the right one.**
`MetafileTransparencyTests.AnUnpairedMaskIsStillDrawnRatherThanSwallowed` held that an unpaired
`SRCAND` must still reach the page — true — and asserted that it reaches it *undecoded, with a
`PL6033` diagnostic* — false. It is now `AnUnpairedAndBlitIsItsOwnTransparencyMask`, carries the
citation and the FAA measurement in its remarks, and is joined by
`AnOperationThatIsNotTheSelfMaskKeepsItsWarning` (a lone `SRCINVERT`, which must keep the opaque
fallback because it needs the destination back) and a six-row `[Theory]` pinning the nibble rule.

---

## 8. Left, with its seat

- **The `.ppt` WordArt path**, §2(a). `XRay.Office.Ooxml/DrawingML/Fontwork*` is the model; the reader
  side is Escher properties 192, 194–197 and 240; reach 2 of 181.
- **`pres_ioc_phuket` page 26's dark blue banner**, §2(b): 103.38 pt tall in the reference and 64.88
  in ours, same top and same width. Not characterised.
- **`NAS-…-Weather.pptx`'s table transform**, §3: we lay a `p:graphicFrame` table out nominally and
  stretch it by 1.3151 × 1.3134; the reference lays it out at its final size. Costs 0.13% of
  anisotropy here and could cost more where the factor is further from unity.
- **Case `0x7` of the nibble switch**, §1 — the self-mask plus an inversion of the destination. No
  corpus document reaches it (`srcand-census.tsv` lists none).
- **`TICAPCapability_Final.xls`'s eight paired `SRCAND` blits do not reach our WMF reader at all**:
  the document has eight of them and did not change by a byte. Whether those metafiles are read on a
  path this fix cannot see is not established.

## The scripts and the banked data

| | |
|---|---|
| `sweep-ours.sh` | our half of the whole 947-document corpus, one directory per document |
| `score-ink.py` | summed |ink|% per document for two of our banks against one banked reference (round 92's, repointed at the primary checkout's skills) |
| `census-srcand.py` | metafiles blitting with a lone `SRCAND`, with the corrected EMF rop offsets |
| `census-gtext.py` | MS-binary shapes carrying Escher WordArt (property 192) |
| `fontscale-variants.py` | one-attribute variants of `a:normAutofit`, drawn `Tf` sizes read out of 26.2.4.2's PDF |
| `ole.py`, `esch.py` | a minimal OLE2 reader and an Escher record walker, shared by the two censuses |

`movers.txt`, `ink.tsv`, `srcand-census.tsv`, `gtext-census.tsv`, `fontscale-variants.tsv`, and the
four crops actually read: `faa67-before.png`, `faa67-after.png`, `phuket26-wordart.png`,
`nas11-table.png`, `3492-page19-after-vs-ref.png`.
