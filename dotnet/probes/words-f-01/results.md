# Words-F round 1 — the WMF over-crop was not a crop rule, and the rule I found is refuted by the corpus that suggested it

Baseline `80633bfbd36`. Worktree `/c/sandbox/workdir/wt-words-f`, branch `wt-words-f`,
`XRAY_OFFICE_CLI` set explicitly to this tree's binary on every sweep. Reference
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, 200 PDFs from LibreOffice 26.2.4.2
620(Build:2). `check-env.sh` green before anything was measured — Calibri→Carlito,
Cambria→Caladea, Arial→Liberation Sans, Times New Roman→Liberation Serif, Courier
New→Liberation Mono, DejaVu Sans→DejaVu Sans, pdftoppm and pdftotext 26.01.0,
"**Environment is good**". `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every render of
ours.

---

## 1. Headline

> **`crop-wiring-01`'s named debit is closed, and not by the mechanism it named.** The two
> WMF pictures in `150_5300_13_chg10` that we cropped where the reference crops nothing were
> never a crop defect: **we were drawing the wrong picture**. An inline `.doc` picture's
> `pib` is numbered inside its own container and we were resolving it against the document's
> shared blip store, so four of that document's figures were each drawn as **the same
> 197 × 77 grayscale JPEG belonging to a floating shape elsewhere in the file**, at exactly
> the right place and the right size. Fixing the lookup takes the paired crop comparison from
> **6 agreeing / 2 over-cropped / 4 unpaired** to **7 agreeing / 0 over-cropped / 3
> unpaired**, and gains a seventh agreement (`chg10`'s 462.9 × 551.4 frame, ours 1.010
> against the reference's 1.010) that we previously did not draw at all.
>
> **The rule I set out to establish is stated, measured, and then refuted — by the corpus, in
> this round.** "26.2.4.2 ignores an Escher crop when the blip is a WMF" reproduces on
> **eighteen** corpus pictures across three documents and is confirmed by three in-place
> experiments that separate it cleanly from crop magnitude and from metafile-versus-bitmap.
> It is then **contradicted** by a WMF in `150_5335_5a.doc` whose crop 26.2.4.2 *does* apply,
> and by an authored fixture. Six candidate discriminators are ruled out by measurement and
> **none of them is the answer**. §4 is the whole of it and it is the most useful thing here.
>
> **Words is 155 / 200 before and 155 / 200 after.** Page error 115 → 115, exact page counts
> 165 → 165, render failures 0 → 0, absolute word error **6869 → 6840**. **Six of 200
> renderings change bytes, no page count moves, no verdict moves.**

---

## 2. The prediction, scored

`prediction.md` in this directory, committed as `771a71e797a` **before** the target document
was opened, before any census, and before anything was rendered. Its blind-spot section is §E
there.

| # | prediction | outcome |
|---|---|---|
| P1 | the discriminator is not the graphic's kind | **wrong for the wrong reason, then right.** Kind separates the corpus *perfectly* on `chg10` and `chg12` — and then fails on `5335_5a`. So the prediction lands, and the measurement that lands it is not the one it predicted |
| P2 | the crop dies because `SetAttributesAtGrfNode` is not reached — a group, or an OLE node | **refuted.** `chg10` has **no `ObjectPool` storage at all**, so no OLE object; all 25 inline containers are single `SpContainer`s of type 75 with `FSP` flags `0xa00` (`HaveAnchor\|HaveSpt`), so no group and no `OLEShape` bit |
| P3 | both over-cropped pictures are OLE; band 1–2 of 2 | **refuted, 0 of 2** |
| P4 | the fallback: the graphic's stated extent is zero | **refuted.** Every WMF in play states a non-zero `ptSize` in its `OfficeArtMetafileHeader`, and for all four documents that `ptSize` equals the `PICF`'s `dxaGoal` to the twip |
| P5 | `lcl_ConvertCrop`'s `abs(nCrop >> 16) >= 50` heuristic is not the cause, 0 of 2 | **held** — every fraction in the corpus has integral part 0 |
| P6 | `dxaCrop*` stays zero on both | **held** — 0/0/0/0 on all 25 of `chg10`'s inline pictures |
| P7 | the metafile's own frame is not the discriminator | **held as far as measured** — `rcBounds`, `ptSize`, the map mode and the `SetWindowExt` are structurally identical between the WMFs whose crop is ignored and the one whose crop is applied (§4.4) |
| P8 | the fix is a suppression; reach 1–3 of 200, point estimate 1 | **wrong, and not a suppression.** The fix is a lookup-order correction and reach is **6 of 200** |
| P9 | the 8 crop frames go to 8 agreeing / 0 over-cropped, band 7–8 | **inside the band at the bottom: 7 agreeing, 0 over-cropped, 3 the reference crops and we do not draw** |
| P10 | verdicts: 0, band 0–1; if anything moves it moves *away* on `chg10`'s words | **held on the point estimate (0), and the direction call was right in miniature** — `chg10`'s word error goes 499 → 537 further, while `5335_5a`'s goes 116 → 49 closer and the track total goes 6869 → **6840**, better |
| P11 | nothing on sheets or slides | **held**, structurally (the diff is `XRay.Office.WordProcessing` and the test kit) and corroborated: Spreadsheets 663 and Presentations 613 reproduce to the digit |
| P12/P13 | the page cluster | **void** — task 1 did not close early and task 2 was not started. Said plainly rather than padded |

**Two refuted outright, one void, and the round's whole thesis (P1/P2/P3) wrong.** The
prediction named OLE objects and groups as the mechanism; the mechanism was that we were
reading the wrong table.

---

## 3. The defect that actually caused the over-crops

### 3.1 What was measured

`inline-pics.py` walks `150_5300_13_chg10.doc`'s `Data` stream and reports, per inline
picture, the `PICF`, the `FSP`, the `OPT` and the record that follows the container.
**Twenty-five inline pictures**, `pib` running 1 to 22, and a shared blip store of **twelve**
entries reached through `fcDggInfo`.

The frames come out of the `PICF` as `dxaGoal × mx / 1000`, and every one of the reference's
26 image placements pairs with exactly one of those containers. That is what makes the
attribution safe: `crop-wiring-01` §6 named the first over-cropped picture as the one stating
`left 0.0049 right 0.5761 top 0.0198 bottom 0.5366`, and **that attribution is wrong** — that
container's frame is 528.3 × 250.4 pt. The 466.6 × 545.8 pt frame belongs to the container
stating `left 0.2720 right 0.3105`, whose 1/(1−0.2720−0.3105) = **2.395** is the measured
growth to three places, and 416.9 × 227.4 belongs to `right 0.1489`, whose 1/(1−0.1489) =
**1.175** is the other.

Our own PDF then said what was really wrong. Six image placements, and **four of them are the
same object**:

```
3 0 obj <</Subtype/Image/Width 197/Height 77/ColorSpace/DeviceGray/Filter/DCTDecode ...
4 0 obj <</Subtype/Image/Width 197/Height 77/ColorSpace/DeviceGray/Filter/DCTDecode ...
5 0 obj <</Subtype/Image/Width 197/Height 77/ColorSpace/DeviceGray/Filter/DCTDecode ...
6 0 obj <</Subtype/Image/Width 197/Height 77/ColorSpace/DeviceGray/Filter/DCTDecode ...
```

197 × 77 grayscale JPEG, four times, at four different frames of 147 × 45, 466 × 545,
416 × 227 and 436 × 587 pt. It is the picture the document's **floating** shape carries at
store index 3 — and `chg10`'s inline containers state `pib` 2, 3, 4, 3 at those positions.

### 3.2 Why the lookup is wrong

An inline picture in a `.doc` is an `OfficeArtInlineSpContainer`: the `SpContainer`, then its
**own** `OfficeArtFBSE`. Its `pib` is numbered from one *inside that container*, so the same
small number appears on every inline picture in the file and collides with the shared store
whenever the document also has floating shapes.

`Ww8DocumentReader.InlineFrame` tried the store first and fell through to the container's own
blip only when the store answered nothing. Whenever `pib ≤ store size` — which is most of
`chg10` — the store answered, wrongly, and the fall-through never ran.

LibreOffice reaches the right order by a different route and says why in the code, twenty-odd
years ago:

```cpp
/* ##835##
 * Disable use of main stream as fallback stream for inline direct
 * blips as it is known that they are directly after the record
 * header, testing for existence in main stream may lead to an
 * incorrect fallback graphic being found
 */
m_xMSDffManager->DisableFallbackStream();
```
`sw/source/filter/ww8/ww8graf2.cxx`:531-537. **That is this defect, described in 2003.**
Attribution is from the 27.2.0.0.alpha0+ tree in this checkout and is therefore *read*, not
measured; the behaviour it explains is measured.

### 3.3 The change

One method, `Ww8DocumentReader.Drawings.cs`. `FramePicture image = PictureOf(shape);` plus a
conditional fall-through becomes `FramePicture image = InlinePictureAt(shapeAt, shape) ??
PictureOf(shape);`, with the container's blip authoritative **whenever the container has
one** — including when its bytes cannot be decoded. That last part is deliberate and is
argued in the code: the store's answer for that `pib` would be a different picture, and an
empty frame in the right place is better than the right size holding someone else's figure.

Nothing else changed. `XRay.Office.MsBinary`, `XRay.Office.Core` and every other library are
byte-identical to the baseline.

### 3.4 The crop comparison, re-measured

`crop-wiring-01`'s `crop-vs-reference.py`, unmodified, pairing by frame rectangle — never by
page index, because our `chg10` page 50 is the reference's page 47.

| paired cropped picture | base | **after** |
|---|---:|---:|
| agrees with the reference (growth within 2%) | 6 | **7** |
| drawn uncropped where the reference crops it | 0 | 0 |
| **cropped where the reference does not crop** | **2** | **0** |
| the reference crops a frame we do not draw at all | 4 | **3** |

The seven that agree:

| document | frame | ours | reference |
|---|---|---:|---:|
| `150_5300_13_chg10` | 147.8 × 45.0 | 1.000 / 1.283 | 1.000 / 1.287 |
| `150_5300_13_chg10` | **462.9 × 551.4** | **1.010 / 1.000** | **1.010 / 1.000** |
| `150_5300_13_chg10` | 503.4 × 586.7 | 1.000 / 1.043 | 1.000 / 1.043 |
| `150_5300_13_chg10` | 503.3 × 586.5 | 1.000 / 1.043 | 1.000 / 1.043 |
| `150_5300_13_chg12` | 503.4 × 586.7 | 1.000 / 1.043 | 1.000 / 1.043 |
| `RMI…GettingOffOil` | 468.0 × 274.4 | 1.000 / 1.074 | 1.000 / 1.074 |
| `absrc-pac-01-info-note-en` | 64.8 × 45.9 | 1.099 / 1.000 | 1.101 / 1.002 |

The 462.9 × 551.4 row is new: that PNG was previously drawn as the wrong JPEG at a frame the
tool could not pair, and it now agrees with the reference to three places.

**"0 over-cropped" is not the same as "correct", and the difference is stated rather than
buried.** The two frames that were over-cropped are now **not drawn at all**: their pictures
are WMFs and our `.doc` path renders no WMF (§5). So `chg10` goes from 6 image placements to
7 — three gained, two lost, one previously-wrong one corrected — and the crop comparison
stops seeing those two frames because neither side crops anything there. That is an
improvement (a wrong figure removed, three right ones added) and it is **not** the same as
drawing them.

---

## 4. The WMF rule: measured, and then refuted

This is the part worth reading. The rule is stated the way it was believed at each stage,
with what refuted it.

### 4.1 The census, walking records

`blip-kind-census.py` adds one column to `crop-wiring-01`'s census — the **kind of blip the
`pib` resolves to** — by two different lookups, because a `.doc` stores pictures in two
places: an inline container's own `FBSE` follows it, a floating shape's `pib` indexes the
`DggContainer`'s store. Both are walked, never regexed, and `SpContainer` headers are found
by the validated scan `crop-wiring-01` §5 established, because of the one-byte `dgglbl`.

**It reproduces `crop-wiring-01`'s answer exactly — 9 documents / 40 cropped shapes** — with
control columns of 66 `.doc` read, 575 Escher shapes, 135 carrying a `pib`, all three
identical to that round's figures.

| | floating | inline |
|---|---:|---:|
| EMF | 5 | 5 |
| WMF | 1 | 20 |
| PNG | 1 | 4 |
| JPEG | 1 | 1 |
| no picture | — | 2 |

### 4.2 What 26.2.4.2 does, read out of the PDFs

All 26 of the reference's image placements in `chg10` pair with a container by frame. The
split is total:

- **cropped by the reference**: the PNG stating `left 0.0095` (growth 1.010), the EMF stating
  `right 0.0091` (1.010/1.004), the PNG stating `bottom 0.0409` (1.043), the PNG stating
  `right 0.0533` (1.056).
- **not cropped by the reference**: **all sixteen WMFs**, whose stated crops run from 0.0002
  to **0.8599** and every one of which is drawn at its frame with growth 1.000.

`150_5300_13_chg12` repeats it inside one document: its inline PNG (`bottom 0.0409`) is drawn
at 1.043 and its inline WMF (`top 0.1483 bottom 0.2177 left 0.1528 right 0.2014`) at
476.30 × 289.35 with growth **1.000 / 1.000**.

### 4.3 Three in-place experiments that separate kind from magnitude

`patch-crop.py` rewrites one 4-byte fixed-point crop value inside an `OfficeArtFOPT` — same
field width, no structural change, the discipline `picture-crop-goal.doc` was built with —
and the result is rendered by the installed 26.2.4.2 and compared page-raster by page-raster
at 60 dpi against an unpatched control rendered the same way.

| edit | pages of 77 that change | growth on that frame |
|---|---:|---|
| **WMF** `right` 0.1489 → **0.0409** (small, like the PNG that *is* cropped) | **0** | 1.000 → 1.000 |
| **PNG** `bottom` 0.0409 → **0.3105** (large, like the WMF that is *not*) | **1** | 1.043 → **1.451** |
| **EMF** `right` 0.0091 → **0.3105** | **1** | 1.010 → **1.452** |

So **magnitude is refuted from both sides**, and **"metafile against bitmap" is refuted** —
an EMF is a metafile and is cropped. The patch is verified to have taken (re-reading
`expC.doc` reports `right=0.0409`), and the WMF's image XObject is **byte-identical** between
the two renderings — same object number, same 1737 × 947 JPEG, same 10014 bytes — so the
crop is not being baked into the raster either.

A fourth experiment extends it to a second document: **`150_5300_13_chg8`'s inline WMF**,
`left` 0.1433 → 0.5, **0 of 18 pages change**.

> **The rule, as it stood at that point.** On the inline `.doc` path 26.2.4.2 applies an
> Escher `cropFrom*` to a PNG, a JPEG, a DIB and an EMF, and ignores it entirely on a WMF.
> Eighteen corpus pictures across three documents, four in-place experiments, six
> non-WMF pictures cropped exactly as stated.

### 4.4 And it is wrong

`150_5335_5a.doc` container 3 is an `msofbtBlip_WMF` (`0xF01B`, `btWin32` 3) stating
`top 0.3439 bottom 0.2653`. Patching its `top` to 0 **changes page 22 of 64**. Its EMF
neighbour, patched the same way, changes page 51. A determinism control — the same file
rendered twice through `soffice` — differs on **0 of 64** pages, so the single-page change is
real.

An authored fixture agrees with the counterexample rather than the rule. `picture-crop-wmf.doc`
is a hand-assembled WMF inline at 288 × 216 pt with `a:srcRect l=10% t=20% r=30% b=40%`,
converted by `soffice` and then patched into the shape Word writes — `dxaCrop*` zeroed,
`dxaGoal` shrunk to the visible extent, exactly the edit `picture-crop-goal.doc` records.
26.2.4.2 draws it **cropped**: the ink fills 287.3 × 215.3 pt and the boundary between two
quadrants sits at **0.667** of the width and **0.75** of the height, which are
(0.5−0.1)/0.6 and (0.5−0.2)/0.4 — the stated fractions, applied.

**Six candidate discriminators, each ruled out by measurement:**

| candidate | ruled out by |
|---|---|
| the crop's magnitude | a 0.0409 WMF crop ignored; a 0.3105 PNG crop applied |
| metafile against bitmap | the EMF beside them is cropped |
| `pibFlags` | the cropped EMF has `pibFlags=0x0`, as every ignored WMF does |
| an OLE object or a group | no `ObjectPool`; all containers single, type 75, flags `0xa00` |
| `PICF` `xExt`/`yExt`, and `mx`/`my` | zeroing `xExt`/`yExt` and shrinking `mx`/`my` to `chg10`'s values in the fixture leaves it cropped, in three variants |
| the presence of a shared blip store | the collision fixture has one and is cropped |
| the metafile's own shape | `MM_ANISOTROPIC` + `SetWindowOrg(0,0)` + `SetWindowExt`, and `ptSize` equal to `dxaGoal` to the twip, on **both** the ignored and the applied WMFs |

**So the honest statement is: the reference ignores the Escher crop on every WMF in the
`150/5300-13` family and applies it on a WMF in `150/5335-5A` and on an authored one, and I
did not find what separates them.** The candidate the C++ points at — `lcl_ConvertCrop`
returning 0 because `pGrfNd->GetTwipSize()` is 0
(`sw/source/filter/ww8/ww8graf.cxx`:2164-2220, with `pF == nullptr` on the inline path so the
`FSPA` fallback cannot fire) — is *consistent* with every "ignored" case and is **not
established**, because it predicts the `PICF` crop would be clobbered too and the
round-tripped fixture shows a `PICF` crop surviving.

**No rule was shipped.** A version of this round that shipped "WMF ⇒ no crop" is sitting in
this branch's history at `6722b2550fc` and was removed after `5335_5a` refuted it. It is also
worth recording that it **bought nothing**: the crop comparison is `MATCH 7 / OVER 0 /
NO PAIR 3` with the lookup fix alone, identical to the two-fix result, because the pictures it
would have acted on are not drawn at all.

**And the fixture is the round's second instance of `crop-wiring-01` §4's lesson, arriving by
a new route.** A file round-tripped through `soffice` is a statement about `soffice`; here it
is a statement about `soffice`'s *WMF exporter*, and on the one question that mattered it
disagrees with the metafiles Word wrote. This time the corpus refuted the fixture *and* the
fixture refuted the corpus rule, which is why neither is trusted alone.

---

## 5. What this uncovered and did not fix

**Our `.doc` path draws no WMF at all.** The collision fixture was first built with a
hand-assembled WMF inline, and after the lookup fix the reader drew **one** picture where two
were expected — the floating PNG. The same thing accounts for `chg10`: with the correct blip
in hand, its two formerly over-cropped WMF frames become no image at all, and the document
goes from 6 placements to 7 rather than to 9. An EMF in the same document renders fine
(1049 × 307), so it is specific to WMF on this path.

The likely seat is that `EscherBlips` hands on a **non-placeable** WMF — the
`OfficeArtMetafileHeader` is stripped and what remains begins `01 00 09 00 00 03`, with no
`0x9AC6CDD7` key — and something downstream of it will not recognise that as a WMF. **That is
inferred from the bytes and not measured**; no test was written and no fix attempted. It is
the strongest lead this round leaves, it is one format on one path, and it would put real ink
back on `chg10`, `chg8`, `chg12` and `5335_5a`.

---

## 6. Reach, verdicts and direction

Both legs are full 200-document sweeps through `words-d-01/gate.py` against the canonical
26.2.4.2 references, with the build's exit status checked before each.

**The baseline reproduces to the digit: 155 match, 115 absolute page error, 165 exact page
counts, 0 render failures.**

| | base | after |
|---|---:|---:|
| match | **155** | **155** |
| absolute page error | 115 | 115 |
| exact page counts | 165 | 165 |
| absolute word error | 6869 | **6840** |
| render failures | 0 | 0 |
| renderings differing byte for byte | — | **6 of 200** |

The six: `150_5300_13_chg10`, `150_5300_13_chg12`, `150_5335_5a`, `SFSP_2013-02_Bulletin`,
`absrc-pac-01-info-note-en`, `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX` — all `.doc`, as
the change requires. **No page count moves anywhere and no verdict moves**, and the whole gate
TSV differs on exactly two lines:

| document | words ours/ref, base → after | direction |
|---|---|---|
| `150_5335_5a.doc` | 19392 → **19325** / 19276 | **closer**, 116 → 49 |
| `150_5300_13_chg10.doc` | 24052 → **24090** / 23553 | further, 499 → 537 |

Net **−29** on the track's absolute word error. `chg10` moving away is the expected sign and
was predicted: the four figures that were the wrong JPEG are now three correct pictures whose
embedded labels are extractable text, and the reference rasterises its metafiles and loses
that text — `TODO.raster-ceiling.md`'s phenomenon, on the side where ours is the better
output and `wc -w` scores it as worse.

**Sheets and slides: zero, argued structurally and corroborated rather than swept.** The diff
is two files in `XRay.Office.WordProcessing` and one in the test kit; `XRay.Office.Spreadsheets`
and `XRay.Office.Presentations` reference neither, and their suites reproduce at 663 and 613
exactly. No render sweep was run on those tracks and this is labelled as the weaker evidence
it is.

---

## 7. Tests

Three new tests, `XRay.Office.WordProcessing.Tests.InlineBlipLookupTests`, and one test-kit
addition. `DrawnPage` gains `Pictures` — the placements paired with the `RasterImage` that
went into each — because `Images` keeps only rectangles, and **a rectangle cannot see this
defect at all**: the wrong picture and the right one are drawn at the same place and the same
size, since the frame comes from the `PICF` and the crop from the shape. That is why it
survived a whole crop round wearing a crop defect's clothes.

### Verified by reintroduction

| mutation | detected by |
|---|---|
| the store consulted first and the container's own blip used only as a fall-through — the exact code that was there | `AnInlinePicturesPibDoesNotIndexTheDocumentBlipStore` |

`verify-test.sh XRay.Office.WordProcessing '<mutation>' InlineBlipLookup`, exit 0, on a tree
that built clean with the mutation applied: **Failed 1, Passed 2**, naming that test.

### Drift guards only — kept deliberately, and labelled

- `TheFloatingPictureInTheSameDocumentStillComesFromTheStore` — the control. It passes under
  the mutation, because the mutation does not touch floating shapes; it is here so that a
  future change which stopped drawing the store's pictures entirely — a much larger
  regression wearing this fix's clothes — cannot pass the detector above.
- `TheInlinePictureKeepsItsFrameAndItsCrop` — repeats `FramePictureCropTests`' 480 × 540 pt
  arithmetic on a second fixture, so that changing *where an inline picture's bytes come
  from* cannot quietly change *where it lands*. Every mutation that breaks it breaks a
  detector too.

### Fixture provenance

`picture-blip-collision.doc` is **newly authored** by `make-wmf-fixtures.py` in this
directory — no corpus document is copied or excerpted, and the CV is not touched. It is a
minimal `.docx` holding one **anchored** PNG (100 × 100 px, which is what puts an entry in the
shared store) and one **inline** PNG (64 × 64 px, cropped 10/20/30/40) through `soffice
--convert-to doc`, then patched in place into the shape Word writes: `dxaCrop*` zeroed and
`dxaGoal` shrunk from 960 × 960 to 576 × 384 twips, same field widths, no structural change.
Verified after conversion by walking its records: inline blip PNG with `pib=1`, store
`['PNG']` — **the collision is real, not arranged.**

**The two pixel sizes are the instrument.** Both pictures are drawn at the same place
whichever way the `pib` is read, so the pixel count is the only thing that says which one
arrived.

`picture-crop-wmf.docx` is produced by the same script and is **used by no test**, for the
reason in §4.4.

---

## 8. Final state

```
dotnet build XRay.Office.slnx -v q -nologo     0 Warning(s)   0 Error(s)
```

| project | before | after |
|---|---:|---:|
| Core | 305 | 305 |
| Containers | 109 | 109 |
| Text | 289 | 289 |
| Vector | 295 | 295 |
| Rendering | 149 (1 skipped) | 149 (1 skipped) |
| Markup | 259 | 259 |
| OpenDocument | 125 | 125 |
| **WordProcessing** | **789** | **792** |
| Spreadsheets | 663 | 663 |
| Presentations | 613 | 613 |
| **total** | **3596** | **3599**, 0 failed, 1 skipped |

The briefed per-project baseline reproduced exactly, project for project, before any
addition. Each project was run on its own with `--no-build` after a build whose exit status
was checked, never more than four at a time. `XRay.Office.Fidelity.Tests` was not run.

---

## 9. Measured, inferred, not established

**Measured:**

- Both 200-document sweeps, complete, with the base reproducing 155 / 115 / 165 / 0 to the
  digit; the byte comparison over all 200; every gate column on both legs.
- The paired crop comparison on all seven crop-carrying documents, base and after.
- The record walk of `150_5300_13_chg10.doc` — 25 inline containers, their `PICF`, `FSP`,
  `OPT` and following `FBSE` — and the pairing of all 26 reference image placements to them
  by frame.
- Our own PDF's image XObjects showing one 197 × 77 JPEG placed four times.
- Four in-place crop experiments against the installed 26.2.4.2, page-raster compared, with a
  determinism control at 0 of 64 pages.
- The authored WMF fixture's rendering by 26.2.4.2, and three single-variable variants of it.
- The blip-kind census, reproducing `crop-wiring-01`'s 9 documents / 40 shapes and all three
  of its control columns.
- The reintroduction result in §7 and every project's test count.

**Inferred, and flagged:**

- That `##835##`/`DisableFallbackStream` is the *mechanism* by which LibreOffice avoids the
  collision. Read from a 27.2.0.0.alpha0+ tree that made none of the references; the
  behaviour it explains is measured.
- That our `.doc` path fails on WMF because the blip arrives non-placeable (§5). Read from the
  bytes, not measured, and no test written.
- That sheets and slides are unreachable by this change. From the reference graph plus two
  suites reproducing exactly, not from a render sweep.

**Not established:**

- **What separates a WMF whose Escher crop 26.2.4.2 applies from one it ignores** (§4.4).
  Eighteen ignored, two applied, six candidates ruled out. **Start here.**
- Whether the same question has a floating arm: `chg10` carries a floating WMF with a crop of
  0.2944/0.3596 and no image in the reference grows by the 2.891 that would imply, but that
  shape may simply be drawn as vector, so it is consistent and not evidence.
- Why our `.doc` path draws no WMF (§5).
- Three frames the reference crops and we do not draw at all — `chg10`'s 483.7 × 141.6 and
  256.1 × 369.5, and `A_320`'s 168.8 × 19.1. Pre-existing, untouched, and *not* counted as
  wins.
- Everything task 2 was to look at. The page cluster was not started: **37 documents still
  fail check 1**, `EHEST-SMS-Safety-Management-Manual-V2.docx`'s second defect is still
  visible and still unattributed, and this round contributes nothing to either.
