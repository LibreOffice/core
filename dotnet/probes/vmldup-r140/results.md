# Round 140 — O90: a `w:object`'s replacement picture drawn twice

**Measured against LibreOffice 26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**
(`/opt/libreoffice26.2/program/soffice`), with the tarball's duplicate, Noto Latin, Narrow and
Condensed faces moved aside, `fc-match "DejaVu Sans"` answering `DejaVuSans.ttf`. Every sweep
below is pinned with `SOURCE_DATE_EPOCH=0`.

## 1. The mechanism

`DocxVmlFrames.TopLevel` selected with

```csharp
IsShape(child) is not false
```

against

```csharp
private static bool? IsShape(XElement child)
    => … ? true : null;
```

A predicate that answers `true` or `null` and **never `false`** makes `is not false`
unconditionally true, so the only condition doing any work in that filter was the group test
beside it: **every descendant** of the `w:pict`/`w:object` was offered to `One` — a `v:imagedata`,
a `v:f`, a `v:path`, a `v:shapetype` among them. The class' own remark said `v:shapetype` "is
excluded deliberately" and described a filter that did not exist.

Inside a `w:pict` that costs nothing, because `One` wants a size and a property element states
none. Inside a `w:object` it costs a duplicate of everything, through two independent fallbacks:

* `One` takes the **object's** `w:dxaOrig`/`w:dyaOrig` when the element it is handed states no box
  of its own, and that is the same pair for every descendant;
* `DocxPictures.ReadVml` searches `DescendantsAndSelf`, so the `v:imagedata` element resolves
  **itself**.

## 2. The fixture reproduces both rectangles exactly

`features/words-object-duplicate.docx` (2761 B), built by `make-fixture.py`: one `w:object`
carrying `w:dxaOrig="1540" w:dyaOrig="997"`, holding a `v:shapetype`, a `v:shape` whose own
`style` states `width:77.25pt;height:49.5pt`, and a `v:imagedata` inside the shape. It carries an
empty `word/settings.xml` with `compatibilityMode` 15, per the `office-corpus` skill — a
hand-built DOCX without one answers a different question.

Image placements read out of each PDF's inflated content streams (`fixture-placements.tsv`):

| renderer | placements | matrices |
|---|---:|---|
| 26.2.4.2 | **1** | `77.3 0 0 49.5 56.7 722.239` |
| ours, broken predicate | **2** | `77.25 0 0 49.5 56.7 721.9` and `77 0 0 49.85 56.7 721.9` |
| ours, fixed | **1** | `77.25 0 0 49.5 56.7 722.25` |

The rejected duplicate is `1540/20 × 997/20` — the object's original size — which is what
identifies the second fallback rather than merely counting one picture too many. Against the
reference the surviving placement agrees to **0.05 pt** on the extent and **0.011 pt** on the
origin.

## 3. The fix, and what is *not* the fix

`IsShape` is now a plain `bool` naming VML's ten real shape elements, and `TopLevel` filters on it
directly; `Group`, which used `IsShape(child) is not null`, takes the same predicate.

**Narrowing to the original five would have worked equally well on this corpus, and an earlier
draft of the source remark claimed otherwise on a figure that is wrong.** That draft said 72 of
the corpus's 150 top-level `v:line` state a sized `style`. Re-censused (`census-vml.py`,
`census-vml.tsv`):

| element | top-level in a `w:pict`/`w:object` | of those, sized `style` | direct child of a `v:group` | kept by `Group`'s guard |
|---|---:|---:|---:|---:|
| `line` | 150 | **0** | 715 | **0** |
| `polyline`, `curve`, `arc`, `image` | 0 | 0 | 0 | 0 |

All 150 state `from`/`to` **attributes** and no `style` width or height, and all 150 are
`position:absolute`, so `Floating` returns null for every one of them; none is inside a
`w:object`, so the `dxaOrig` fallback cannot reach them either. Rendering the **53** corpus
documents that hold one of the five added elements (`vline-docs.txt`) once with each predicate
gives **53 byte-identical PDFs**. So the widening is a measured no-op and the ten are named
because they are what the method claims to answer; **the reach is entirely in answering `false`
for the property elements.**

**And that census leaves a gap of its own**: this tree draws none of those 150 rules, because a
`v:line` states its extent in `from`/`to` and nothing reads them. Seated separately, not closed
here.

## 4. Reach on the words track

The whole 337-document words track rendered twice under `SOURCE_DATE_EPOCH=0`
(`fingerprints-before.txt`, `fingerprints-after.txt`):

**8 renderings move, 329 byte-identical, and no page count moves anywhere.**

`movers.tsv`, alphanumeric characters counted with PyMuPDF:

| document | pages before/after/ref | alnum before | after | 26.2.4.2 |
|---|---|---:|---:|---:|
| `A1. EASA Form 2` | 7 / 7 / 7 | 11561 | **11547** | 11527 |
| `EHEST-SMS-Safety-Management-Manual-V2` | 82 / 82 / 82 | 105466 | **105317** | 105180 |
| `UG.CAO.00006 … User Guide for Applicants` | 29 / 29 / 29 | 42116 | **41545** | 40732 |
| `FO.FCTOA_.000129 Application … FSTD` | 6 / 6 / — | 9067 | 9023 | — |
| `eTAR_External_Web_tool_Tip_Sheet_mh` | 4 / 4 / — | 4629 | 4604 | — |
| `5709.16 ch.40_mgfinal` | 32 / 32 / — | 59010 | 59010 | — |
| `airbus-pdf-information-package_v1-4` | 9 / 9 / — | 6448 | 6448 | — |
| `f2_registro_de_aprovacao_com_pbcs_EN` | 2 / 2 / — | 2293 | 2293 | — |

Five of the eight lose characters and **every one of the five moves towards the reference**, so
what went was drawn twice. Three move ink only.

On the witness `A1. EASA Form 2.docx` the image placements go **16 → 15** against the reference's
**15**, counted with `get_image_info(xrefs=True)` over all seven pages.

**No gate verdict can move**, and the reason is worth stating rather than assuming: the gate
scores page count, alphanumeric characters within `max(2%, 15)`, and unembedded fonts. No page
count moves; the three documents whose reference is banked here were inside the band before and
are closer to it after.

## 5. Pinned by mutation

`git checkout --` the source (which is the broken predicate at HEAD), rebuild, run the three new
tests:

```
Failed!  - Failed: 2, Passed: 1, Skipped: 0, Total: 3
  ItIsDrawnAtTheShapesOwnBoxRatherThanTheObjectsOriginalSize [FAIL]
  AnObjectsReplacementPictureIsDrawnOnce [FAIL]
```

The third, `TheSurroundingTextIsStillDrawn`, is the control against a filter that rejects too
much and passes on both legs, which is what it is for. Restored with `cp` + `touch` (never `mv` —
the up-to-date check skips a project whose source looks older than its assembly) and rebuilt:
3 of 3 pass.

A second mutation is not available here: narrowing `IsShape` to the original five also fixes the
duplicate, and §3 measures that arm at the corpus instead.

## 6. Suite

Each project run on its own and totalled by hand, on an already-built tree.

| project | discovered | passed | failed | skipped |
|---|---:|---:|---:|---:|
| Core | 591 | 591 | 0 | 0 |
| Containers | 109 | 109 | 0 | 0 |
| Text | 750 | 750 | 0 | 0 |
| Vector | 309 | 309 | 0 | 0 |
| Rendering | 164 | 164 | 0 | 0 |
| Markup | — | 259 | 0 | 0 |
| OpenDocument | 169 | 169 | 0 | 0 |
| WordProcessing | 2010 | 2010 | 0 | 0 |
| Spreadsheets | 1412 | 1412 | 0 | 0 |
| Presentations | 1205 | 1205 | 0 | 0 |
| Fidelity | 552 | 542 | **10** | 0 |

The ten fidelity failures are the documented set and are unchanged by this round — four
`PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` (`paginated.doc/.docx/.fodt/
.rtf`), four `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop`
(`list-label-overrun.doc/.docx/.fodt/.odt`), one
`JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt`
(`justify-shrink-2013.docx`) and one
`SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt` (`sheet-rich-text.xlsx`).
**0 skipped** on Fidelity, which is what says the reference binary was actually reachable.

## 7. Files

| file | what it is |
|---|---|
| `make-fixture.py` | builds `features/words-object-duplicate.docx` |
| `census-vml.py`, `census-vml.tsv` | the top-level / group-member census of the five added elements |
| `vline-docs.txt` | the 53 corpus documents holding one, rendered with each predicate |
| `fingerprints-before.txt`, `fingerprints-after.txt` | md5 of all 337 words-track renderings, each leg |
| `movers.tsv` | the 8 movers, pages and alphanumerics, with the reference where it was rendered |
| `fixture-placements.tsv` | the image placement matrices of the three renderings in §2 |
