# words-f-01 — prediction, committed before any measurement of the target document

Written after reading the C++ tree (27.2.0.0.alpha0+, **not** the reference binary) and the
three predecessor probes, and **before** opening `150_5300_13_chg10.doc`, before running any
census over the corpus, and before re-rendering anything. Nothing below has been checked.

Baseline commit: `80633bfbd36`. Worktree `/c/sandbox/workdir/wt-words-f`, branch `wt-words-f`.
`dotnet build XRay.Office.slnx -v q -nologo` on this baseline: **0 warnings, 0 errors** (checked,
exit 0 — that is the one fact here that is already measured, and it is measured because a green
test run after a failed build is a known trap on this project).

---

## A. What I am predicting about

`crop-wiring-01` §6 leaves two WMF pictures in `150_5300_13_chg10.doc` cropped by us at growth
**2.395** and **1.175** where the reference crops nothing, on frames 466.6 × 545.8 pt and
416.9 × 227.4 pt. Six other cropped pictures across the words corpus now agree with the
reference. The rule that separates them is not established. That is the question.

## B. The mechanism, predicted

**P1 (primary). The discriminator is not the graphic's kind.** `crop-wiring-01` already
noticed that `chg10` has metafiles among the six that agree, so "WMF" cannot be the rule, and
I predict a census confirms that from the other side: at least one *bitmap* somewhere in the
corpus will also be in the no-crop class, or the two WMFs will be shown to differ from the
agreeing metafiles by something that is not their kind.

**P2 (primary mechanism). LibreOffice's DOC reader applies the Escher crop in exactly one
place, `SwWW8ImplReader::SetAttributesAtGrfNode` (`sw/source/filter/ww8/ww8graf.cxx`:2178),
and that function returns before doing anything when the node it is handed is not a
`SwGrfNode`.** For an inline picture the call is `ww8graf2.cxx`:685, and it is reached only
when two conditions hold in `ImportGraf`:

1. `SvxMSDffImportRec const*const pRecord = (1 == aData.size()) ? … : nullptr;`
   (`ww8graf2.cxx`:556) — **an inline Escher object that imports as more than one shape (a
   group) has no record, so no crop is applied at all**; and
2. the insert went through `InsertGraphic` rather than `InsertOle`/`ImportOle`
   (`ww8graf2.cxx`:650-670) — **an OLE object becomes an `SwOLENode`, `GetGrfNode()` returns
   null, and `SetAttributesAtGrfNode` returns at its third line with no crop applied**.

So my predicted rule is: **the Escher crop on a `.doc` picture is applied only to a picture
that lands as a plain graphic — not to an OLE object and not to a group.**

**P3. Concretely, for the two pictures in `150_5300_13_chg10`:** I predict at least one of the
two is an **embedded OLE object** — `sprmCObjLocation` present on the run, or the `SpContainer`
carrying `mso_sptPictureFrame` with an OLE `pib`/`ShapeFlag::OLEShape` — and that whichever of
the two mechanisms applies, it applies to **both** of them and to **neither** of the six that
agree. Point estimate: **both are OLE**. Band: 1 or 2 of 2 OLE, 0–1 of 2 a group.

**P4 (the fallback, if P3 is refuted).** The next candidate in order is
`SetAttributesAtGrfNode`'s own arithmetic: it converts the 16.16 fractions against
`pGrfNd->GetTwipSize()`, **the graphic's own natural size**, and for an inline picture it is
handed `pF == nullptr` so the `if (!nWidth && pF)` fallback cannot fire. A graphic whose
preferred size reads as zero therefore gets `lcl_ConvertCrop(…, 0) == 0` on all four edges —
no crop. Predict: if P3 fails, the two WMFs have a degenerate stated extent (zero or absent
`PrefSize`) and the six that agree do not.

**P5 (ruled out in advance, and I will still check it).** `lcl_ConvertCrop`'s fdo#77454
heuristic — `if (abs(nCrop >> 16) >= 50) return 0` — **cannot** be what fires here: the stated
fractions are 0.0049/0.5761/0.0198/0.5366, whose integral parts are all 0. I predict this is
not the cause, 0 of 2.

**P6. `dxaCrop*` stays zero on both**, consistent with `crop-wiring-01`'s 32 of 32.

**P7. The metafile's own frame is not the discriminator.** The brief names "whether a WMF's
stated extent already accounts for the crop" as a candidate. I predict **refuted**: LibreOffice
scales the visible sub-rectangle of the graphic onto the frame regardless of the graphic's
extent, so a self-accounting extent would show as a *scale* error, not as an all-or-nothing
crop, and the reference draws these two with growth exactly 1.000/1.000 rather than something
near it.

## C. The fix, predicted

**P8.** The fix is a **suppression**, not new arithmetic: the crop is dropped on the inline
`.doc` path when the picture is an OLE object (and/or a group). Reach: **1 to 3 of 200**
renderings change bytes — `150_5300_13_chg10` certainly, and `chg8`/`chg12` are the same
document family and may carry the same figures. Point estimate **1**, band **1–3**.

**P9. Direction.** The re-measured 8 crop frames go to **8 agreeing / 0 over-cropped /
0 missing**, from 6/2/0. Point estimate 8 of 8; band 7–8. Anything less than 7 means the rule
is wrong rather than incomplete.

**P10. Verdicts: zero movement.** Point estimate 0, band 0–1. A crop is not a page count and
not a font, and `crop-wiring-01` measured 0 of 7 for exactly this reason. The one way it could
move is check 2: the clip currently deletes words that live inside the two metafiles, so
*undoing* the crop on `chg10` puts extractable words **back**, moving 24 052 further from the
reference's 23 553 rather than closer. So if a verdict moves at all I predict it moves
**away**, on `chg10`'s word column, and `chg10` already fails both checks so it cannot get
worse than `pages,words`.

**P11.** Nothing on the sheets or slides tracks changes: the fix is inside the WW8 reader,
which neither family references. 0 of 171 and 0 of 163, argued structurally and, if the change
touches anything above `XRay.Office.WordProcessing`, measured instead.

## D. If task 1 closes early — the page cluster

**P12.** `EHEST-SMS-Safety-Management-Manual-V2.docx` at 79/82: I predict its remaining
3-page deficit is **not** one defect at one place. Point estimate: **at least two distinct
first-divergence sites**, and I predict the deficit is a *content-loss or line-fit* defect
rather than a section/break defect, because words-e already swept 72 authored break variants
and closed every shape but the column-break one. Band on "one localisable cause found and
fixed": **20%**.

**P13.** I predict I do **not** move the whole 37-document cluster this round, and that any
movement is 0 to +2 verdicts. The ±1 cluster having no single shared cause has been refuted
three times; I am not predicting a fourth attempt succeeds.

## E. Blind spots — what my instruments cannot see, named in advance

1. **Any census over `word/document.xml` is blind to the 66 `.doc` files**, which is the whole
   population this round is about. Three rounds have said this; it is stated again because the
   fix here is *only* on the `.doc` path, so an XML census is not merely incomplete here, it is
   **entirely inapplicable**. Every census in this round must walk binary records.
2. **A record walker that trusts record lengths finds no floating shape in any `.doc`** — the
   one-byte `dgglbl` before each `DgContainer` in `OfficeArtWordDrawing`. I am reusing
   `crop-wiring-01`'s corrected scanner rather than writing a new one, and I will re-run its
   known-answer control (16 decks / 100 cropped shapes on `.ppt`) before believing any figure
   it produces.
3. **An inline picture's `SpContainer` is in the `Data` stream**, not in `fcDggInfo`. A
   scanner that reads only `fcDggInfo` reaches zero.
4. **A page-level pixel tally is uninterpretable on `chg10`**, whose page 50 is the reference's
   page 47. Direction must be read out of the PDF's image-placement operators paired by frame
   rectangle, as `crop-vs-reference.py` does, never by page index.
5. **`first-divergence.py` cannot resolve geometry below ~1.6 pt on A4** (512 px raster). It is
   not an instrument for any crop question and will not be used for one.
6. **The C++ tree here is 27.2.0.0.alpha0+ and did not make the references.** Every claim in §B
   above is *read from source* and therefore unproven until the installed 26.2.4.2 confirms it
   on an authored document. **Three rounds have burned predictions on this tree**; if the
   authored probe disagrees with §B, §B is wrong and the probe is right.
7. **An authored fixture that has been round-tripped through `soffice` is a statement about
   `soffice`'s exporter.** `crop-wiring-01` §4 is the standing example. Any fixture I author
   for the OLE/group case must be checked against the real `chg10` records before it is
   believed, and the corpus sweep is the arbiter.
8. **I cannot see whether the four frames the reference crops and we do not draw at all**
   (`chg10` emits 6 images against the reference's 26) hide a ninth and tenth case of this same
   rule. Those frames are a pre-existing gap; if the rule fixes the two known cases the other
   four remain unmeasured and I will say so rather than counting them.
9. **Disk and memory.** ~8.8 GB free with two other worktrees live. If a build dies under
   memory pressure a following `--no-build` test run measures stale binaries and reports green.
   Every build's exit status is checked before the tests that follow it.
