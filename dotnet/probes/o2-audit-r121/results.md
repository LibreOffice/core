# Round 121 — the O2 audit: four seat numbers with no row, and what is left of them

The register's opening sentence is a contract: *every entry here ends in one of two states.*
Four early seat numbers break it by appearing in no row of any section — **O1, O2, O4, O5**.
This round closes all four and measures the two live pieces inside O2.

Everything below was measured at HEAD `90b5b2207` with the CLI gate r119 itself built
(`/home/user/gate-r119-cli/XRay.Office.Cli`, 05:05 UTC today) — no rebuild, so our leg is the
same binary the banked gate used, and `TICAPCapability_Final.pdf` re-renders to the gate's
own 256 856 bytes exactly. Reference is `/opt/libreoffice26.2/program/soffice`, 26.2.4.2.

---

## 0. What the four seats were, and where each ends

| seat | round 94's sentence | ends as |
|---|---|---|
| **O1** | head of the sheets ink ranking, `TK-Syllabus-Comparison-Document-v2.xlsx`, 304.57 summed `\|ink\|%` over 1235 pages | **fixed** — `probes/sheet-ink-r94`, the 413 unread `cfRule type="expression"` blocks |
| **O2** | `FAA_Form_337` p67 and `pres_ioc_phuket` p26 carry ink missing from ours; `NAS-…-Weather` 9.58 uncharacterised | **fixed**, in five pieces; §1–§3 below are the two that had no row |
| **O4** | BIFF `CONDFMT` unread | **fixed** — re-seated as O25, closed in round 98; `~~N14~~` says so and did not say it was O4 |
| **O5** | `017_Timeline_Templates` blank page from a print-area extent | **fixed** — `probes/ods-residue-r95` §3, a `draw:connector`'s rectangle is its two endpoints |

O2 dissolved into five sub-seats in `probes/slides-ink-r94` §8. Three were already adjudicated
— the `.ppt` WordArt path became **O13**, WMF nibble case `0x7` became **N8**, and
`NAS-…-Weather.pptx` *as an ink defect* became **N7** (refuted). The two with no terminal row
are §1 and §2 here; §3 is r94 §8's fifth line, the transform, which is a different claim from
N7's and was never adjudicated either.

---

## 1. `pres_ioc_phuket.ppt` page 26's dark blue banner — **already fixed, by O14**

### What r94 measured, and what it is today

r94: *"the reference fills `#00007E` at (−18.03, 436.65)–(719.97, 540.03) and this tree at
(−18.00, 475.12)–(720.00, 540.00) — same top, same width, and 38.5 pt short at the bottom …
Not worked."*

`banner-geometry.py` reads the rectangle and **the paint operator** out of four renders —
ours, two fresh reference renders made minutes apart under separate user profiles, and the
reference leg gate r119 banked at 05:12 (`banner-geometry.tsv`):

| render | x0 | y0 | x1 | y1 | width | height | colour | op |
|---|---:|---:|---:|---:|---:|---:|---|---|
| ours | −18.00 | 436.63 | 720.00 | 540.00 | 738.00 | **103.37** | `#00007E` | `f` |
| ref 1 | −18.03 | 436.65 | 719.97 | 540.03 | 738.00 | **103.38** | `#00007E` | `f*` |
| ref 2 | −18.03 | 436.65 | 719.97 | 540.03 | 738.00 | **103.38** | `#00007E` | `f*` |
| gate r119's ref | −18.03 | 436.65 | 719.97 | 540.03 | 738.00 | **103.38** | `#00007E` | `f*` |

**64.88 is gone.** The height agrees to 0.01 pt, the width to 0.00, the origin to 0.03.

**C16 is answered before the size is believed**: both sides paint a *fill*, not a stroke —
`f` here against `f*` there, which on a rectangle with no self-intersection is the same
region under either winding rule. There is no primitive mismatch hiding a size claim.

**C11 is answered too**: the two fresh reference renders are identical to each other
record for record on page 26 (`pdf-ops.py diff ref1 ref2 --page 26` → 0 / 0 / 0), and both
equal the gate's third render on this rectangle. This document is not one of C11's movers.

### Which round fixed it, and why the register already knew

It is **O14**, closed in round 97: *"A `.ppt` shape resizes itself around its own text …
Witness `pres_ioc_phuket.ppt` p26: 64.88 → 103.37 against 26.2.4.2's 103.38."* Escher's
`fFitShapeToText` becomes `SdrTextAutoGrowHeightItem` and the height is the outliner's plus
one unit of tolerance. The number in O14's own row is the number measured again here from a
render made today. What was missing was a row saying that this closes an O2 sub-seat; O14's
row names the document and the page but not the seat it came from.

### The font-class confound, and why it does not reach this

`pres_ioc_phuket.ppt` is one of L1/O57's three movers — it states `Times` at `FF_ROMAN`, and
round 115 measured that class reaching drawn text on exactly this document. That is an
**advance-width** defect: the reference measures a DX array with the family class on and
rebuilds the face at `FAMILY_DONTKNOW` to draw it. The banner's height is not an advance
width; it is the outliner's *line* height for two 40 pt lines, and it agrees to 0.01 pt on a
103 pt box, which is 1 part in 10 000. O57 does reach this page — it is why several text
origins on it differ by ~1 pt — but it cannot be what set the banner's height, and the
banner's height is not evidence for or against it.

### What is left on page 26, and it is O13's, not this seat's

Page 26 is now `|ink|` **0.23 %**, *shifted*, against r94's **4.28 MAJOR**
(`pdf-image-diff.py ours ref1`; the whole deck is 26 pages with 1 MAJOR, page 24, which is
neither of this seat's questions). The residue on 26 is the WordArt band: the reference paints
a gradient bitmap `Im675` through a glyph-outline clip at (12.10, 491.22)–(693.27, 528.07)
and we paint a 105-band gradient over the same rectangle. That is **O13**'s row, which
records the same page going 1.97 → 0.23 `|ink|%`, and it is closed there.

**Terminal state: fixed in this tree (round 97, O14).** No code change this round.

---

## 2. `TICAPCapability_Final.xls`'s paired `SRCAND` blits — **they reach the reader, and the pair fires**

### The question, and why the r94 census could not answer it

r94 §8: *"`TICAPCapability_Final.xls`'s eight paired `SRCAND` blits do not reach our WMF
reader at all: the document has eight of them and did not change by a byte. Whether those
metafiles are read on a path this fix cannot see is not established."*

The document not changing by a byte is not evidence either way: r94's change added the **lone**
`SRCAND` arm (`IsSelfMasked`), and every one of TICAP's blits is a **pair**, which
`PendingBlit.PairsWith` had already handled before that round. So the census could not move
and the path question stayed open.

### First correction: they are EMF, and there are nine

Walking the workbook stream's `MSODRAWINGGROUP` record and its `CONTINUE`s into the Escher
blip store (`xls-blip-reach.py`, banked as `ticap-blip-reach.txt`) finds **11 `msofbtBSE`
entries, every one of them `msofbtBlipEMF` (`0xF01A`)** — not one WMF in the file. So *"do not
reach our WMF reader"* is true and says nothing: they are `EmfReader`'s, not `WmfReader`'s.

Nine of the eleven hold exactly one `EMR_BITBLT` at `SRCAND` immediately followed by an
`EMR_STRETCHDIBITS` at `SRCPAINT` over the same 13 × 13 destination. **Nine, not eight** —
r94's `census-srcand.py` reaches blips by scanning raw bytes for a record header and finds 9
metafiles where the store holds 11, and classifies 8 pairs. The structured walk finds 9 pairs,
and the render below draws 9.

### Second: the bytes reaching the reader, shown

`pdfimages -list` on our render of the document lists **9 image XObjects, each with its own
`/SMask`**, 13 × 13, three on each of pages 4, 10 and 15 — and the reference's own PDF holds
**zero images on all 17 pages**. An `/SMask` is only there if the merge fired: an unmerged
blit is drawn opaque with no mask at all.

`ticap-blip-merge.py` closes it with no tolerance. It decodes the SRCAND DIB and the SRCPAINT
DIB straight out of the blip, merges them the way `RasterOperations.Merge` does — colour where
the mask is black, transparent where it is white — and compares against the extracted image
and its `/SMask`:

| blip | page-4 image | colour-plane mismatches | alpha-plane mismatches | opaque pixels |
|---|---|---:|---:|---:|
| 5 | `p4-000` / `p4-001` | **0 of 169** | **0 of 169** | 112 of 169 |
| 1 | `p4-002` / `p4-003` | **0 of 169** | **0 of 169** | 112 of 169 |
| 9 | `p4-004` / `p4-005` | **0 of 169** | **0 of 169** | 112 of 169 |

*Weakness stated:* the nine icons are the same artwork, so this is one comparison confirmed
three times rather than three independent ones. The 112-of-169 opaque count is the load-bearing
part: 57 pixels are knocked out, which only the merge can do.

**The metafiles are read, on the ordinary Escher-blip path, and the paired idiom resolves.
r94's suspicion is refuted.**

### Base rate (C9)

`metafile-base-rate.py` over the whole manifest, so r94's "eight documents" has a denominator:

```
manifest rows           : 947
documents scanned       : 947
documents with metafiles: 123
metafiles found         : 453
blit records found      : 4149
documents, paired SRCAND: 8  (blits 200)
documents, lone   SRCAND: 3  (blits 6)
metafile-holding docs by extension: doc 8, docx 29, ppt 27, pptx 52, xls 3, xlsx 4
```

TICAP is 1 of the 3 `.xls` that hold a metafile at all, and the only `.xls` with a pair.

### What the difference on the page actually is, and it is not a metafile question

The nine blips belong to nine `OBJ` records of type `PICTURE` whose `ftPioGrbit` states
`EXC_OBJ_PIC_CONTROL | EXC_OBJ_PIC_CTLSSTREAM` (`0x30`/`0x31`), and whose Escher shapes are
type **201 `hostControl`** with `fOleShape` set and `pictureId` present. They are **ActiveX
Forms 2.0 check boxes** persisted in the file's `_VBA_PROJECT_CUR/Ctls` stream (2 004 bytes),
and the EMF is Excel's cached picture of each one.

Both legs agree on what that means. *Source* (this checkout, 27.2.0.0.alpha0+, C8):
`IsOcxControl()` is `mbEmbedded && mbControl && mbUseCtlsStrm`
(`sc/source/filter/inc/xiescher.hxx`:824); `XclImpPictureObj::DoCreateSdrObj`
(`xiescher.cxx`:3038-3068) asks `rDffConv.CreateSdrObject` for a control first and only falls
back to `SdrGrafObj` on the cached graphic when that returns nothing, and
`DoPreProcessSdrObj` (`:3082`) sends an OCX control to `ProcessControl` and nowhere near the
blip. *Binary*: 26.2.4.2 draws each control as two nested rectangles — `#000000`
3.00 × 3.00 pt at (76.60, 538.10) and `#FFFFFF` 2.60 × 2.60 pt inset 0.20 pt inside it — an
empty native check box, with **no raster anywhere in the document**. We draw the cached EMF:
a `#FFFFFF` 3.60 × 3.40 pt panel at (76.94, 537.94) and the 13 × 13 masked icon on it, which
is also an empty check box.

So both sides draw an empty check box with the same label at the same place; the box differs by
0.34 pt in origin and 0.6 × 0.4 pt in extent, and the label is drawn at an effective
3.82 × 3.40 pt here against 4.00 pt there. Over nine controls that is ~110 pt² of our ink
against ~81 pt² of theirs, on 3 of 17 pages of 1 of 947 documents, on a document the gate
scores `match` (17/17 pages, 27 834/27 777 glyphs).

**Terminal state: the path question is answered — reached, decoded, pair resolved. The residue
is a separate thing and is NOT WORK; it is seated as O66 below.**

---

## 3. `NAS-…-Weather.pptx`'s transform — **alive as a mechanism, dead as a defect, and misdiagnosed**

### The frame carrying the transform is not a table — but the file does hold three

r94 §8: *"we lay a `p:graphicFrame` table out nominally and stretch it by 1.3151 × 1.3134;
the reference lays it out at its final size. Costs 0.13 % of anisotropy here and could cost
more where the factor is further from unity."*

`ppt/slides/slide11.xml` holds one `p:graphicFrame`, and its `graphicData` uri is
`http://schemas.openxmlformats.org/presentationml/2006/ole`. Inside it is
`<p:oleObj name="Worksheet" progId="Excel.Sheet.12" imgW="11277718" imgH="3192990">` with a
`mc:Fallback` picture. **The frame carrying the transform is an embedded Excel worksheet, not a
DrawingML table**, and any future round looking for the table layout code would look in the wrong
place for *this* frame.

> **Correction, made at merge and not by this round.** The sentence that stood here — *"There is
> no `a:tbl` in the file at all — no `a:gridCol`, no `a:tr`"* — is **false**, and it mattered,
> because it was the whole reason this round never looked further. `NAS-…-Weather.pptx` holds
> **three** DrawingML tables, one each on `slide8.xml`, `slide9.xml` and `slide10.xml`
> (`unzip -p … | grep -c '<a:tbl>'` → 1, 1, 1; five `<a:tbl>` across all parts). Only
> `slide11.xml` is the OLE frame. r94 §3's heading is *"the coarse instrument overstates page
> 11"*, so page 11 is indeed the page it discussed — but r94 §8 called that frame a *table*, and
> the file does contain three real ones that this round dismissed without measuring.
>
> **Measured at merge, and the verdict survives — for a reason this round did not give.**
> Rendering the document with the r119 CLI and 26.2.4.2 and diffing every page with
> `pdf-ops.py diff`, classified by the *kind* of difference reported:
>
> | page | frame | `shows` | `glyphs` | **position** | **size** |
> |---|---|---:|---:|---:|---:|
> | 8 | `a:tbl` | 35 | 14 | **0** | **0** |
> | 9 | `a:tbl` | 33 | 15 | **0** | **0** |
> | 10 | `a:tbl` | 30 | 15 | **0** | **0** |
> | 11 | `p:oleObj` | 3 | 0 | **0** | 87 |
>
> On all three real table pages the *only* differences are how text is split into show operators
> and the glyph counts that follow from it — **not one position and not one size divergence**. So
> the three table frames take no nominal-layout-plus-stretch at all, and r94's claim is dead on
> the tables as well as on the OLE frame. Page 11's 87 size differences are the 9.00 vs 9.01
> hundredth-of-a-point already tabulated below, and the count of 87 corroborates this round's
> 87 paired records independently.

### The mechanism is unchanged at HEAD

`ole-transform.py` reads the page's own content stream and pairs every text record
(`ole-transform.txt`):

| | NAS-…-Weather p11 | RPA P4 - Advanced Material p3 |
|---|---|---|
| frame | 888.0 × 251.4 pt, `Excel.Sheet.12` | 641.7 × 300.6 pt, `Excel.Sheet.8` |
| our non-unit `cm` | `1.3151 0 0 1.3134` | `0.379 0 0 0.3791` |
| reference's | **none** | **none** |
| text records paired | 87 / 87 | 64 / 64 |
| drawn size ours / ref | 8.9973 / 9.01 | 6.8238 / 6.802 |
| origin divergence max | **0.053 pt** | **0.030 pt** |
| origin divergence mean | 0.030 pt | 0.022 pt |

So the claim is **alive**: we still lay the embedded object out at its own natural size and
scale it onto the frame, and the reference still lays it out at the frame's size with no
transform at all. r94's `1.3151 0 0 1.3134 36 -240.726 cm` is in the file today, byte for byte.

And it is **dead as a defect**. r94's worry was that the cost grows with distance from unity.
`RPA P4` is the corpus's most extreme case in the other direction — a factor of 0.379, three
and a half times further from unity than NAS's 1.3151 — and its worst origin divergence is
**smaller**, 0.030 pt against 0.053. The anisotropy itself falls with it: 1.3151/1.3134 is
0.129 %, 0.379/0.3791 is 0.026 %. The residual size difference (−0.14 % on NAS, **+0.32 %** on
RPA — opposite signs) is therefore not the transform's doing at all; it is ordinary layout
rounding, and calling it "0.13 % of anisotropy" conflated two things.

*Refutation of a figure this round could have quoted:* r94's "0.13 % anisotropy" is arithmetically
right for NAS and is **not** a general property of the path. Do not carry it forward as the
transform's cost.

### Reach, with the base rate

`oleobj-census.py` over the manifest (`oleobj-census.tsv`):

```
corpus documents          : 947
presentations of any kind : 302
zip presentations scanned : 251
p:graphicFrame seen       : 721
p:oleObj seen             : 45 in 10 documents
```

Of the 721 graphic frames on corpus slides, **45 are embedded OLE objects, in 10 of 251 zip
presentations** — 36 `Excel.Sheet.12`/`Excel.Sheet.8`, 3 `Visio.Drawing.11`, 3
`Photohse.Document`, and one each of `Equation.DSMT4`, `TCLayout.ActiveDocument.1` and
`WPDraw30.Drawing`.
The scale factor is *not* readable from the file: `imgW`/`imgH` is the cached presentation's
size, which for NAS equals the frame to 118 EMU while our emitted factor is 1.3151. It takes a
render, which is why the two measured here are the two rendered.

**Terminal state: NOT WORK, seated as O67 below with the number that sizes it.**

---

## 4. Rows added to the register

Four to *Fixed in this tree* (O1, O2, O5 and the O4 pointer inside `~~N14~~`), two to the
NOT WORK / nil-reach table (O66, O67). Nothing was deleted; `~~N14~~` gained a clause.

## 5. The scripts and the banked data

| | |
|---|---|
| `banner-geometry.py` | the `#00007E` rectangle on phuket p26 with its paint operator, over any number of renders → `banner-geometry.tsv` |
| `xls-blip-reach.py` | a BIFF workbook's Escher blip store and every shape's `pib`/`pictureId`/flags → `ticap-blip-reach.txt` |
| `ticap-blip-merge.py` | decodes one blip's SRCAND + SRCPAINT DIBs, merges them outside X-Ray.Office, compares against our own image and `/SMask` |
| `metafile-base-rate.py` | C9's denominator behind r94's `srcand-census.tsv` → `metafile-base-rate.txt` |
| `oleobj-census.py` | `p:oleObj` on corpus slides, with the frame and the stated `imgW`/`imgH` → `oleobj-census.tsv` |
| `ole-transform.py` | the non-unit `cm` a page emits and the origin divergence of every shared text record → `ole-transform.txt` |
| `ole.py`, `esch.py` | r94's OLE2 reader and Escher walker, copied so this directory runs standalone |

## 6. What this round did not settle

- **Whether our cached-EMF check box or 26.2.4.2's native one is the better rendering.** Ours
  is Excel's own picture of the control and theirs is LibreOffice's redraw of it; neither is
  Excel-on-screen and no reading of a 3 pt box can separate them. O66 declines the question
  rather than answering it.
- **The scale factor's distribution over the 45 corpus `p:oleObj`.** Only two were rendered.
  A sweep would give the other 43, and the two measured say the answer would not change a
  verdict.
