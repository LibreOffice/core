# Round 130 — a `.doc`'s contents list keeps a style the reference throws away, and two pictures on one line had no character between them

Both of this round's seats were in the WW8 reader and both are **fixed**. Neither is the rule its
DOCX sibling turned out to be, and one of them cost a third change in a shared layer to keep a
fixture right.

| | |
|---|---|
| worktree | `/home/user/wt-ww8toc`, branch `agent/ww8toc`, base `fb9bf3b47`, 2026-09-14 |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r129, `/home/user/gate-r129/ref`, 947 PDFs |
| our base leg | built in this worktree from `fb9bf3b47` with both files reverted, saved as `/home/user/r130-base-cli` |
| C++ tree | `/home/user/libreoffice-core`, read only. **C8**: it declares `27.2.0.0.alpha0+` and is a bulk import, so it is not 26.2's source; every arm below is confirmed a second time against 26.2.4.2's own output |
| C11 | no arm rests on a per-document reference delta of a handful of characters, and no gate column moves in either direction, so the reference's run-to-run instability cannot reach a conclusion here |

**Headline.**

| | before | after | 26.2.4.2 |
|---|--:|--:|--:|
| O81 — Σ\|ours − reference\| of rule cover on the 10 contents pages of the 5 `.doc` that have one | **34 650 pt** | **677 pt** | — |
| `150_5335_5a.doc`, 3 contents pages | 29 251 pt | **0 pt** | **0 pt** |
| `361400CSLegislation1RF01PUBLIC1.doc`, 1 contents page | 4 960 pt | **238 pt** | 146 pt |
| O79 — pictures drawn on one baseline, one over the other, over the 66 `.doc` | **1 of 66** | **0 of 66** | 0 of 66 |
| `RMI_…GettingOffOil.doc` p2, the two pictures | y 115.10–373.95 and 99.60–373.95 | **99.60–358.45 and 358.45–632.80** | 99.70–358.35 and 358.50–632.70 |
| summed \|ink\|% over every `.doc` rendering that moves | **34.15**, 10 MAJOR pages | **22.04**, 7 MAJOR | — |
| `.doc` renderings that move | — | **6 of 66** | — |
| page counts and alphanumeric counts that move, `.doc` | — | **0 of 66** | — |

---

## 0. What the two seats turned out to be, in one paragraph each

**O81.** A `.doc`'s contents entries name the built-in `Hyperlink` character style — blue and
single-underlined — and 26.2.4.2 refuses that one style, and only that one, while it is reading a
`TOC` field's cached result. **The rule is not round 126's DOCX rule and does not transfer from
it**: the OOXML reader drops *every* character style inside a TOC, and the WW8 reader drops exactly
one, in the sprm handler itself.

**O79.** The WW8 walk dropped the anchor character of every frame it made. Two adjacent inline
pictures therefore arrived at the layout with **one offset between them** — and an inline object's
offset is a *boundary* — so there was nothing for the measurer to break at and the second picture
was drawn on the first's line, resting on the same baseline. Round 128 was right that the line
breaker is not at fault: it never had two boundaries to break between.

---

## 1. O81 — the rule, read out of the sprm handler and confirmed against the binary

`SwWW8ImplReader::Read_CColl` is the `sprmCIstd` handler, and it has a TOX branch
(`sw/source/filter/ww8/ww8par6.cxx`:4135-4160):

```cpp
    // if current on loading a TOX field, and current trying to apply a hyperlink character style,
    // just ignore. For the hyperlinks inside TOX in MS Word is not same with a common hyperlink
    // Character styles: without underline and blue font color. And such type style will be applied in others
    // processes.
    if (m_bLoadingTOXCache && m_vColl[nId].GetWWStyleId() == ww::stiHyperlink)
    {
        return;
    }
```

Three things in that hunk are the whole of the difference from the DOCX side, and each was worth
finding before writing anything:

- **It is one style, not every style.** `DomainMapper.cxx`:3037-3047 declines to insert
  `PROP_CHAR_STYLE_NAME` for any run inside a TOC, which is what
  `WordParagraphFormats.ResolveRun`'s `ignoreCharacterStyle` models. This declines
  `ww::stiHyperlink` and nothing else — `stiHyperlinkFollowed` (86) is kept, and so is any style of
  the document's own.
- **It is keyed on the `sti`, not on the name.** `sti` is the low twelve bits of the STD's first
  word (`sw/source/filter/inc/wwstyles.hxx`:126, `ww8par2.cxx`:3902 `rSI.SetOrgWWIdent(sName,
  xStd->sti)`), and `Ww8StyleSheet` was throwing that word away.
- **The extent is the field's cached result, and it spans every entry's paragraph.**
  `m_bLoadingTOXCache` is set in `Read_F_Tox` (`ww8par5.cxx`:3053-3055) and cleared at that same
  field's end (`:596-608`), with `m_nEmbeddedTOXLevel` counting a nested index rather than clearing
  the flag; the `PAGEREF` and `HYPERLINK` fields inside each entry open and close well inside it.

The three other places the same round 126 register row cites — `ww8par5.cxx`:2334, 3362-3363 and
3653 — are the `Index Link` *pool style* being put on the links the reference **builds**, and they
are not what suppresses the file's own style. Following them alone would have modelled the wrong
half.

### Confirmed a second time, against 26.2.4.2's own output

`150_5335_5a.doc`'s stylesheet, read with `TocProbe`: style **26** is `Hyperlink`, kind 2, CHPX
`3E2A01 422A02 70680000FF00` — `sprmCKul` 1 (single underline) and `sprmCCv` `0000FF` (blue). The
document holds **one** `TOC` field, its cached result running cp 1429–14841, and inside it **5161
drawn characters name istd 26** against 443 naming none.

And the drawn page says the same. Page 3 of the two renderings, spans grouped by face and colour:

| | ours, at the base | 26.2.4.2 |
|---|---|---|
| `LiberationSerif` colour `0x0000FF` | **4286 characters** | — |
| `LiberationSerif` colour `0x000000` | 140 | 3863 |
| `LiberationSerif-Bold` colour `0x000000` | — | 523 |

## 2. O81 — the fix, and the before/after on the named documents

Three pieces, all in the WW8 reader:

- `Ww8Style.Sti`, the low twelve bits of the STD's first word, with `Ww8Style.HyperlinkStyle` (85)
  and `Ww8Style.UserStyle` (0x0FFE) named beside it. The four bits above `sti` are `fScratch`,
  `fInvalHeight`, `fHasUpe` and `fMassCopy`; taking the whole word answers 0x8055 for a style whose
  `fMassCopy` is set and matches nothing.
- `Ww8DocumentReader.IndexResultRanges`, which pairs a story's field markers ahead of the character
  walk and returns the cp ranges of its index fields' cached results, and `_indexResults`, which
  holds them for the story being walked and is **saved and restored around a nested story** exactly
  as `_pendingNotes` is — a note or a text box read from inside a contents entry is a story of its
  own with its own field table.
- `ApplyCharacterException` takes an `inIndexResult` flag and skips the *style* half of a CHPX —
  keeping every direct sprm — when the style it names is the built-in `Hyperlink` one.

A pre-pass rather than the walk's own state, because a run's formatting is resolved from a
*position* and the walk reaches a paragraph's runs only once the mark that ends it has been read.

### Before and after, on the 5 `.doc` with a drawn contents page

`toc-cover.py` is round 126's `toc-pages.py` cover function verbatim — a rule is a filled or
stroked rectangle at most 2 pt tall and at least 4 pt wide, or a horizontal line, **merged per y
band**; a contents page is one carrying five or more runs of five or more leader dots — so these
figures are comparable with `wordsdec-r126/toc-pages-{base,after}.tsv` column for column.

| document | contents pages | ours, base | ours, after | 26.2.4.2 |
|---|--:|--:|--:|--:|
| `150_5335_5a.doc` | 3 | **29 251 pt** | **0 pt** | **0 pt** |
| `361400CSLegislation1RF01PUBLIC1.doc` | 1 | **4 960 pt** | **238 pt** | 146 pt |
| *control* `A320SimNotes.doc` | 2 | 1 698 | 1 698 | 1 698 |
| *control* `absrc-pac-01-info-note-en.doc` | 1 | 1 911 | 1 911 | 2 458 |
| *control* `150_5300_13_chg10.doc` | 3 | 0 | 0 | 38 |

Σ|ours − reference| on those ten pages: **34 650 → 677 pt**. The 34 065 pt the seat named is
`150_5335_5a`'s 29 251 plus `361400`'s 4 814, and it is now **92 pt** — §6 characterises the 92.

### Reach, with its base rate

`TocProbe --census` over all **66** corpus `.doc`, from the file rather than from the rendering
(`toc-census.tsv`):

| | |
|---|--:|
| `.doc` carrying a `TOC`/`INDEX` field with a cached result | **4 of 66** |
| of those, with runs inside the result naming the built-in `Hyperlink` style | **2** (5161 and 859 characters) |
| of those, with runs inside the result naming **any** character style | **2** — the same two |
| **base rate**: TOC-bearing `.doc` where the change could fire and correctly does not | **2 of 4** (`CP-ETSO template_v2020.DOC` 267 characters, `absrc-pac-01-info-note-en.doc` 256, both naming no style at all) |
| `.doc` carrying no such field, which cannot move | 62 of 66 |

Both of the two move and neither of the two controls does, which is the measurement rather than the
census.

---

## 3. O79 — what the WW8 reader was handing the layout

`RMI_Document_Repository_Public-Reprts_GettingOffOil.doc` carries **two `U+0001` anchors adjacent
in one paragraph** — cp 3326 and 3327, each with its own `sprmCPicLocation`, the paragraph mark at
3328. Each picture is essentially the whole measure: 467.60 pt and 468.00 pt on 468.00.

Our layout at the base, read out of the engine rather than off the page:

```
page 2: 3 frame(s)
  AsCharacter size=467.60x258.85  area=(72.00,115.10)
  AsCharacter size=468.00x274.35  area=(72.00, 99.60)
  line p17#0 top=27.60 h=274.35 base=274.35 w=935.60
```

**One line, 935.60 pt wide on a 468 pt measure** — 467.60 + 468.00, both objects on it. The seat is
`CollectFrame`, which returned "a frame was made" and the walk then `continue`d without emitting the
anchor character, for **every** frame — floating and as-character alike. Both frames were therefore
recorded at `Offset = current.Length` = the same offset, and an `InlineObject`'s offset is a
*boundary*: two objects on one boundary cannot be split, whatever the breaker does.

Writer inserts an anchor character for `FLY_AS_CHAR` and for no other anchor (`SwFormatFlyCnt` and
`GetCharOfTextAttr`, `sw/source/core/txtnode/thints.cxx`:3633-3652, put in by
`SwDoc::SetFlyFrameAnchor`, `docfly.cxx`:337-348) — the same rule `CLAUDE.md` already records from
the DOCX cover-art fix, and the WW8 walk did not make the distinction. `CollectFrame` now reports
whether the frame it made is as-character, and the walk emits the anchor for that case alone.

It costs no width: `ShapingControls.IsRemovedBeforeShaping` drops the whole C0 range before the
shaper is given the run. Measured on `word-features.doc` with a binary built either way, *"After the
box."* is drawn at **304.06 pt on both**.

### After

| | upper picture | lower picture |
|---|---|---|
| 26.2.4.2 | y 99.70–358.35 | y 358.50–632.70 |
| this tree, base | y 115.10–373.95 | y 99.60–373.95 |
| this tree, after | **y 99.60–358.45** | **y 358.45–632.80** |

Both within 0.10 pt of the reference. `|ink|%` on that document **10.85 → 0.83**, MAJOR pages
**2 → 0**.

### Reach, with its base rate

`baseline-pairs.py` measures the defect's own signature on the drawn page: two images on one page
whose **bottom edges agree to a tenth of a point** and whose horizontal extents overlap by more than
half the narrower one.

| population | documents with such a pair |
|---|--:|
| ours, the 66 `.doc`, at the base | **1 of 66** (1 pair) |
| ours, the 66 `.doc`, after | **0 of 66** |
| 26.2.4.2, the same 66 `.doc` | **0 of 66** |
| 26.2.4.2, the whole words track, 338 documents | **2 of 338, 10 pairs** — and both are `.docx` |

*Round 124's census reported a base rate of 0 and this instrument reports 2 of 338.* The two are
`EHEST-SMS-Safety-Management-Manual-V2.docx` (9 pairs) and
`Press release_EUREKA labels ITEA 3 Cluster.docx` (1), where the reference itself draws overlapping
artwork; so the signature is **0 on `.doc`**, which is the population the change reaches, and not
zero across the track. A census of a rendering signature needs the reference's own count beside it
(C9), and this is the direction that matters: had the reference shown the signature on `.doc` too,
the seat would have been a defect of the instrument.

---

## 4. The third change: an anchor character is not a glyph, and a fixture said so

Emitting the anchor broke one fidelity comparison —
`FurnitureComparisonTests.APictureOnlyHeadAndFootTakeTheirRoomFromTheBody`, ours 455 words on page 1
against the reference's 476 — and the failure was real rather than an assertion out of date.

`picture-furniture.doc`'s running head is one 79.2 pt logo, written as a `SHAPE` field, so its frame
hangs **below** the baseline: `Ww8Frames.InlineAscent` is nought for that case
(`ww8graf.cxx`:2436-2439). With no text in the paragraph the line had no runs at all, its ascent was
nought, and the head came out 79.2 pt tall with the logo at the top. Give the paragraph one anchor
character and it acquires a run — the paragraph font's 11.2 pt ascent — so the head became
**90.4 pt**, the body started 11.2 pt lower, and page 1 lost two lines and 21 words.

**The anchor character *is* the object in Writer.** `SwTextFormatter::NewFlyCntPortion` replaces it
with a `SwFlyCntPortion` rather than shaping it beside one, so a line holding nothing but anchors has
no text portion to be tall for and `SwLineLayout::CalcLine` builds its ascent out of the fly portion
alone. `MeasuredParagraph.MeasureLine` already zeroed the **descent** for exactly that case, with
`HoldsNoText` as the test; it now zeroes the **ascent** under the same test, which is the symmetric
half of the same rule.

**Its blast radius is bounded by who sets `InlineAscent`, and that is a three-line census.** The
change can only move a line whose objects' ascents are all *below* the paragraph font's, and
`InlineAscent` is non-null in exactly three places: a DOCX checkbox and a WW8 checkbox, both the
font's own ascent, and a WW8 `SHAPE`-field frame, which is nought. Everywhere else an as-character
object's ascent is its whole height. **Slides and sheets cannot reach it at all**: `objects` is an
optional parameter of `MeasuredParagraph.Measure` and only `PageContent` and `DocxLayoutSource` pass
one, so `_objects.Length > 0` is false for every `SlideTextLayout` and `SheetTextLayout` measurement.

With it, `picture-furniture.doc` is **byte-identical to the base** and the fidelity set is back to
its standing ten.

---

## 5. Reach, confinement and the gate columns

Our half of the `.doc` and `.docx` tracks rendered twice, at the base and at the head, one output
directory per **document**, `SOURCE_DATE_EPOCH` pinned on both legs (`sweep-doc.py`; 66 of 66 and
272 of 272 on each leg, 0 failures):

| | `.doc` | `.docx` |
|---|--:|--:|
| renderings that move | **6 of 66** | **3 of 272** |
| byte-identical | 60 | 269 |
| page counts that move | **0** | **0** |
| alphanumeric counts that move | **0** | **0** |

Summed `|ink|%` against 26.2.4.2 over the movers (`pdf-image-diff.py` at 512 px, the instrument this
project scores an invisible-to-the-gate change on):

| document | base | head |
|---|--:|--:|
| `150_5335_5a.doc` | 21.46, 7 MAJOR | **19.52, 6 MAJOR** |
| `RMI_…GettingOffOil.doc` | 10.85, 2 MAJOR | **0.83, 0 MAJOR** |
| `361400CSLegislation1RF01PUBLIC1.doc` | 1.27, 1 MAJOR | **1.12, 1 MAJOR** |
| `Reid.doc` | 0.28 | 0.28 |
| `f111.doc` | 0.29 | 0.29 |
| `150_5300_13_chg8.doc` | *not scoreable — 18 pages ours against the reference's 17* | |
| **`.doc` total** | **34.15, 10 MAJOR** | **22.04, 7 MAJOR** |
| `A1. EASA Form 2.docx` | 2.80, 1 MAJOR | 2.82, 1 MAJOR |
| `FO.FCTOA_.000129 … FSTD.docx` | 0.90 | 0.83 |
| `May 25 bulletin focus on carers in the workplace.docx` | 6.80, 3 MAJOR | 6.79, 3 MAJOR |
| **`.docx` total** | **10.50, 4 MAJOR** | **10.44, 4 MAJOR** |

**No document is materially worse.** The three `.docx` and the three small `.doc` movers are level
inside the instrument's own resolution — 512 px on the long edge, where ±0.02 is one region's
antialiasing — and they move because the anchor character changes a paragraph's offsets, not its ink.

**The third change is in a shared layer, so the other two word-processing formats were swept too**,
from the converted ODF corpus at `/home/user/corpus-odf/words` (`sweep-tree.py`, 338 of 338 on each
leg of each column, 0 failures):

| column | documents | renderings that move |
|---|--:|--:|
| `.odt`, 26.2.4.2's own conversion of the words track | 338 | **0** |
| `.rtf`, the same | 338 | **0** |

Byte-identical, both columns, both legs — which is what the `InlineAscent` census predicts: an
as-character object's ascent is its whole height in every reader but the WW8 `SHAPE`-field one, so
the new guard changes nothing where no object's ascent is below the paragraph font's.

**Slides and sheets cannot move at all**, and that is structural rather than measured: `objects` is
an optional parameter of `MeasuredParagraph.Measure` and the only callers that pass one are
`PageContent` and `DocxLayoutSource`, so `_objects.Length > 0` is false for every `SlideTextLayout`
and `SheetTextLayout` measurement. The two reader files are in `XRay.Office.WordProcessing/Ww8`, and
`git grep` says the only type any other reader borrows from that folder is `Ww8DateTime`.

---

## 6. What is left, with the measurement that sizes it

### SEAT O83 — a toggle sprm is resolved against the character style the same CHPX names, not against what the paragraph style resolved to

**This is the whole of what is left on `150_5335_5a.doc`'s contents pages, and it is a second WW8
defect that O81 uncovered rather than caused.** After the fix the reference still draws **523
characters of page 3 in `LiberationSerif-Bold`** and this tree draws the page entirely regular; the
bold entries are the `TOC 3` and `TOC 7` lines.

The run's CHPX at `Definition of ACN` (cp 1747) is `304A1A00 350881` — `sprmCIstd` 26 (`Hyperlink`)
and `sprmCFBold` with operand **0x81**, *"the opposite of the style's value"*. Its paragraph style
`TOC 3` states `sprmCFBold 0x81` too, over a `Normal` that is not bold, so the paragraph style
resolves to **bold**. This tree layers the character style over the paragraph style's resolved
format and then applies the run's own sprms, so the 0x81 inverts **true** and the run comes out
regular.

**The reference resolves it against a different style.** `SwWW8ImplReader::Read_BoldUsw`
(`ww8par6.cxx`:4193-4265) starts at `pSI = GetStyle(m_nCurrentColl)` — the paragraph style — and
then replaces it outright when the same CHPX carries a `sprmCIstd`:

```cpp
        SprmResult aCharIstd = m_xPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::CIstd::val);
        if (aCharIstd.pSprm && aCharIstd.nRemainingData >= 2)
            pSI = GetStyle(SVBT16ToUInt16(aCharIstd.pSprm));
```

and then `if (*pData & 0x80) { if (pSI->m_n81Flags & nMask) bOn = !bOn; }`. `Hyperlink`'s own CHPX
states no weight at all, so its `m_n81Flags` bold bit is clear, the value is **not** inverted, and
the run is bold. So the toggle's base is the style the CHPX *names*, resolved on its own — never the
accumulated value, and never the paragraph style when a character style is named.

**Reach, with its base rate**, over all 66 corpus `.doc` (`toggle-census.tsv`, one row per document,
counting distinct CHPX entries): **14 documents hold at least one CHPX carrying both a character
style and a toggle sprm stated relative to the style — 583 CHPX covering 6765 drawn characters**,
which is the population on which the two readings can differ at all. The base rate is the rest of
the same sprm: **65 of 66 documents and 13 570 CHPX** state such a toggle with **no** character
style beside it, where the two readings necessarily agree. So it is a 4 % subpopulation of the
toggles and it is concentrated — `150_5335_5a.doc` alone holds 506 of the 583 CHPX and 5105 of the
6765 characters, and `135.doc` (35), `f111.doc` (12), `003.doc` (5) and
`LENTOBUSSIAIKATAULU.-31.10.-31.12.2022.doc` (5) are the rest of the head.

**Not attempted this round**, because it is a change to how every `.doc` run resolves its weight,
slant, caps and hidden bits and it needs its own before/after over the track. `|ink|%` on the
witness is **19.52** with it, against 21.46 before O81.

### SEAT O84 — a character attribute open where a `TOC` field begins is not drawn by 26.2.4.2

`361400CSLegislation1RF01PUBLIC1.doc` is 238 pt of rule on its contents page against the reference's
146, and the whole of the 92 is **one black underline, 91.33 pt wide at y 104.4, under the heading
`Table of Contents`** — the line immediately above the `TOC` field. The reference draws it with no
underline.

It is stated in the file: the CHPX covering cp 141–158, fc 2189–2207, is `1668E3156C00 3E2A01` —
`sprmCRsidText` and `sprmCKul` **1**. And the reference does read underlines in this document:
`--convert-to fodt` gives it six `style:text-underline-style="solid"`, two of them on `Block Text`
paragraphs exactly like this one — while this paragraph's own automatic style `P14` carries **no
`style:text-properties` at all**.

**The mechanism is a candidate and is NOT confirmed.** `Read_F_Tox` moves the insertion point
backwards into the index section it has just inserted — `m_oPosAfterTOC.emplace(*m_pPaM, m_pPaM);
(*m_pPaM).Move(fnMoveBackward);` (`ww8par5.cxx`:3531-3533) — while the underline opened at cp 141 is
still on `m_xCtrlStck`, so the attribute is closed at a position in a different node from its start.
Nothing was measured against that reading; a one-attribute variant series on a hand-built `.doc` is
what would settle it, and authoring a `.doc` by hand is the part that was not affordable here.

**Reach**: `toc-census.tsv`'s last column counts the documents whose run ending where a `TOC` field
begins states an underline — **1 of the 4 TOC-bearing `.doc`, and 0 of the other 62**. **Cost of
declining**: 92 pt of rule on 1 page of 1 document, whose `|ink|%` is 1.12 with it.

### Named and not seated

- **`150_5335_5a.doc` is still 19.52 `|ink|%` with 6 MAJOR pages**, and O83 is the largest
  identified part of it but has not been shown to be all of it.
- **`150_5300_13_chg8.doc` cannot be scored on ink at all** — 18 pages ours against the reference's
  17, so `pdf-image-diff.py` refuses the pair. Its rendering moved this round and its rule cover did
  not; what moved is not characterised.
- **`Reid.doc` and `f111.doc`** move by the anchor character and are unchanged on ink to two decimal
  places; neither was examined further.

---

## 7. Tests

| file | cases | what it pins |
|---|--:|---|
| `Ww8IndexLinkStyleTests` (new) | 7 | O81: the `sti`, which style the rule names, and the extent of an index field's result |
| `DocShapeFieldTests.AnAsCharacterFramesAnchorIsOneCharacterThatCostsNoWidth` (rewritten) | 1 | O79: one anchor character per as-character frame, at the frame's own offset, costing no width |
| `InlineObjectPairTests.TwoWideInlineObjectsTakeTwoLines` (a `.doc` case added) | 2 | O79 end to end, on 26.2.4.2's own DOC export of two adjacent inline pictures |
| `PictureFurnitureTests.APictureOnlyHeaderIsNotMistakenForAnEmptyOne` (corrected) | 1 | that a picture-only running head now reads back as one anchor character and still survives |

**`Ww8IndexLinkStyleTests` is a unit file and says so in its own remarks**: the corpus holds the only
`.doc` that exercise O81 and the repository holds no fixture with a hyperlinked contents list, so
each arm is pinned by **mutation** instead of by failing at the base. Five mutations were run and all
five fail exactly one test:

| mutation | test that fails |
|---|---|
| drop the `IsCharacterStyle` guard | `OnlyTheBuiltInHyperlinkCharacterStyleIsTheIndexLinkStyle` |
| read the whole first word as the `sti` rather than its low twelve bits | `AStylesBuiltInIdentityIsTheLowTwelveBitsOfItsFirstWord` |
| close the result on any field end rather than on an index field's | `ANestedFieldInsideAnEntryDoesNotEndTheResult` |
| look the field type up at the walk's own position rather than at the story's base | `AStoryThatDoesNotStartAtNoughtPairsAgainstItsOwnBase` |
| drop the range for a field the story ends inside | `AnIndexFieldLeftOpenReachesTheEndOfTheStory` |

**`inline-object-pair.doc` fails at the base and it is the round's one end-to-end regression case.**
It is 26.2.4.2's own `--convert-to doc` of a flat ODT holding two 4.4 in as-character pictures in
one paragraph (`inline-object-pair.fodt`, banked beside this file). At the base this tree draws them
at y 84.9–171.3 and 70.5–171.3, sharing a bottom edge; at the head, 70.5–156.9 and 156.9–257.7
against the reference's 70.6–157.0 and 157.0–257.6. Removing the one line that emits the anchor
character fails the `.doc` case and leaves the `.docx` case passing, which is the control that says
the two formats reach the split by different routes.

**Two existing tests asserted the behaviour this round changed, and both were corrected rather than
deleted** — see §8.

**The full run**, projects individually, listed against passed (`CLAUDE.md`: a drop with zero
failures is a truncated run, and this project has lost a round to it twice):

| project | listed | passed | failed | skipped |
|---|--:|--:|--:|--:|
| `XRay.Office.Containers.Tests` | 109 | 109 | 0 | 0 |
| `XRay.Office.Core.Tests` | 591 | 591 | 0 | 0 |
| `XRay.Office.Markup.Tests` | 249 | 259 | 0 | 0 |
| `XRay.Office.OpenDocument.Tests` | 169 | 169 | 0 | 0 |
| `XRay.Office.Presentations.Tests` | 1205 | 1205 | 0 | 0 |
| `XRay.Office.Rendering.Tests` | 164 | 164 | 0 | 0 |
| `XRay.Office.Spreadsheets.Tests` | 1387 | 1387 | 0 | 0 |
| `XRay.Office.Text.Tests` | 744 | 744 | 0 | 0 |
| `XRay.Office.Vector.Tests` | 309 | 309 | 0 | 0 |
| `XRay.Office.WordProcessing.Tests` | 1988 | 1988 | 0 | 0 |
| **total, excluding fidelity** | **6915** | **6925** | **0** | **0** |
| `XRay.Office.Fidelity.Tests` | 552 | 542 | **10** | 0 |

**Passed equals Total on every project and no project reports fewer than it listed**, which is the
check round 128's table failed. The listing collapses some theory rows, so a run *above* the listing
is normal (Markup 259 against 249 listed) and a run below it is the signal. **0 skipped everywhere**,
so no project covered nothing.

The ten fidelity failures are the standing set: `diff` against `probes/wordsdec-r126/fidelity-failures.txt`
is empty. `XRay.Office.WordProcessing.Tests` was **1974** at round 126 and is 1988 here: +7
`Ww8IndexLinkStyleTests`, +1 the `.doc` case of `InlineObjectPairTests`, and +6 from earlier rounds
between the two.

**An eleventh failure appeared during the round and was a real regression, not an assertion out of
date** — `FurnitureComparisonTests.APictureOnlyHeadAndFootTakeTheirRoomFromTheBody`, ours 455 words
on page 1 against the reference's 476. §4 is the rule that closed it; the fidelity project is back to
ten.

`dotnet build XRay.Office.slnx`: **0 warnings, 0 errors**. The run is banked as `test-run.txt` and the
failure names as `fidelity-failures.txt`.

## 8. Refutations and corrections of this round's own findings, and of earlier ones

- **"Round 126's DOCX mechanism transfers."** It does not, and the brief was right to say so. The
  DOCX rule drops every character style inside a TOC; the WW8 rule drops the one built-in
  `Hyperlink` style, keyed on its `sti`. Modelling the DOCX rule here would also have dropped
  `FollowedHyperlink` and every style of a document's own.
- **The three `ww8par5.cxx` line numbers the register row carries are not the suppression.** 2334,
  3362-3363 and 3653 put the `Index Link` pool style on the links the reference *builds*; the file's
  own style is refused in `Read_CColl`, in `ww8par6.cxx`. Both halves are real and only the second
  is what a reader has to reproduce.
- **"O79's second picture is 294.55 pt tall and starts at y 79.40."** The register row's figures for
  our side do not reproduce. Measured on the base binary's own rendering, the image boxes are
  115.10–373.95 (**258.85**) and 99.60–373.95 (**274.35**), against the reference's 99.70–358.35
  (258.65) and 358.50–632.70 (274.20). The finding — one baseline, one picture over the other,
  heights right — is exactly as recorded; two of its four numbers are not.
- **"Round 124's baseline-pair census has a base rate of 0."** On the population this round's change
  reaches, the 66 `.doc`, that is right: 0 in the reference and 0 in ours after. Over the whole words
  track the same signature appears in the reference on **2 of 338 documents and 10 pairs**, both
  `.docx`. State the population with the base rate.
- **`DocShapeFieldTests`' "the anchor characters leave nothing in the line" is withdrawn.** Its cost
  argument — the pair shaping to 18.67 pt of `.notdef` — was true when it was measured and is not
  reachable now, because `ShapingControls.IsRemovedBeforeShaping` drops the C0 range before shaping.
  Measured either way, `word-features.doc` draws *"After the box."* at 304.06 pt on both.
- **And the first cut of the anchor-character change was wrong in a way no corpus document could
  show.** It broke `FurnitureComparisonTests` by 21 words on page 1 of `picture-furniture.doc`, and
  the cause was that a line holding nothing but anchors was taking the paragraph font's ascent
  *above* an object that hangs below the baseline. §4 has the rule and the census that bounds it.
  The `.doc` corpus sweep saw none of it: 0 page counts and 0 alphanumeric counts moved, and the two
  documents it made marginally worse on ink moved by 0.01. **A fixture caught what 66 corpus
  documents did not.**
- **An instrument note.** `dotnet build` of the CLI does not rebuild a probe project's own copy of
  `XRay.Office.WordProcessing.dll`, so the first run of `TocProbe` after the O79 change reported the
  base behaviour and read as the change not firing. This is `CLAUDE.md`'s stale-binary trap through
  the build graph, and the same guard applies: re-render one document and byte-compare it against
  the run you are claiming to have reproduced. Done at the end of this round — the final tree's
  rendering of `150_5335_5a.doc` is byte-identical to the swept copy.

## 9. The scripts and the banked data

| file | what it is |
|---|---|
| `TocProbe/` | the probe: a stylesheet dump, a field-and-character-style census (`--census`), a toggle census (`--toggles`), a CHPX range dump, and a layout dump of a page's frames and lines (`--frames`) |
| `sweep-doc.py` | renders one extension of the sample corpus, one directory per document, `SOURCE_DATE_EPOCH` pinned |
| `sweep-tree.py` | the same for a directory tree, which is how the converted ODF columns were swept |
| `toc-cover.py` | rule cover on contents pages against a banked reference — round 126's `toc-pages.py` cover function verbatim |
| `gate-columns.py` | pages and alphanumeric characters for both legs and the reference |
| `ink.py` | summed `\|ink\|%` and MAJOR pages over a sweep, driving `pdf-image-diff.py` |
| `baseline-pairs.py` | the O79 signature: two images sharing a bottom edge and overlapping |
| `toc-cover-base.tsv`, `toc-cover-final.tsv` | the five contents-page `.doc`, before and after |
| `toc-census.tsv` | TOC fields, characters inside their results, and the styles those name, per `.doc` |
| `toggle-census.tsv` | O83's population and its base rate, per `.doc` |
| `gate-columns.tsv`, `gate-columns-docx.tsv` | the two gate columns over both tracks |
| `ink-base.tsv`, `ink-final.tsv`, `ink-docx-base.tsv`, `ink-docx-final.tsv` | the ink scores |
| `pairs-base.tsv`, `pairs-final.tsv`, `pairs-ref-doc.tsv`, `pairs-ref-words.tsv` | the baseline-pair census and its base rate |
| `confinement.tsv` | the four columns swept and what moved in each |
| `inline-object-pair.fodt` | the source the `.doc` fixture was exported from by 26.2.4.2 |

**Reproducing.** Build the CLI, then:

```sh
python3 probes/ww8toc-r130/sweep-doc.py <cli> /home/user/r130-doc-final 3 doc
python3 probes/ww8toc-r130/toc-cover.py /home/user/r130-doc-final /home/user/gate-r129/ref out.tsv
python3 probes/ww8toc-r130/baseline-pairs.py /home/user/r130-doc-final pairs.tsv
probes/ww8toc-r130/TocProbe/bin/Debug/net10.0/linux-x64/TocProbe x --census <every .doc>
```
