# words-e1-01 — results

Round `words-e1-01`, 2026-08-15, worktree `wt-w-e1`, branch `wt-w-e1`, base `886bcde7091`.
Reference LibreOffice **26.2.4.2**, `check-env.sh` green (Carlito, Caladea, Liberation, DejaVu,
Liberation Mono all resolving; `pdftotext`/`pdftoppm` 26.01.0). References reused from
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, never re-rendered.
`SOURCE_DATE_EPOCH=1700000000` on every render that is diffed.

Prediction written and committed before any effect of a change was measured: `prediction.md`,
commit `413215679e8`.

**Two seats, both closed, and the group's verdict count did not move.** That is the result rather
than a caveat: the two defects fixed here are invisible to all three gate columns, and one of them
rendered a document as five blank sheets while its word column read 1298 of 1302.

---

## 1. The group, re-measured first

The whole 200-document words track rendered once with the tree as merged and verdicted against the
banked references (`lineheight-01/verdict.py`, which is `batch-check.sh`'s three checks column for
column). Cross-checked on this group with `batch-check.sh` itself, re-rendering the reference: **the
two instruments agree on all seven documents, every column.**

```
words track          173 of 200 match
words/done-*         158 of 159
words/extra-001        2 of 7          — the briefed 5 failing, exactly
```

| document | verdict | pages | words | whose defect |
|---|---|---|---|---|
| `762.doc` | pages | 22/23 | 4120/4122 | ours, and **not** what the brief says (§6) |
| `info-bulletin-601.doc` | pages | 5/6 | 1298/1302 | ours — **§2**, and the severe one |
| `ABCD-FE-01-00 Flight Envelope` | pages,words | 14/15 | 3844/3720 | the reference's, mostly (§6) |
| `ABCD-SDE-23-00 Avionic System Description` | **match** | 29/29 | 8419/8402 | — |
| `ABCD-WB-08-00 Weight and Balance` | **match** | 12/12 | 2639/2612 | — |
| `UG.CAO.00006 …User Guide for Applicants` | pages,words | 30/29 | 8011/7399 | LibreOffice's (§6) |
| `UG.CAO.00133 …Language` | words | 18/18 | 3899/3667 | LibreOffice's (§6) |

`ABCD-WB-08-00` has crossed into `match` since `words-extra-01` left it at +57 against a 55.2 band;
nothing in this round moved it.

## 2. `info-bulletin-601.doc` rendered as five blank pages

**Found in the operators before anything was looked at.** On all five of our page content streams
the single `/Im … Do` sat *after* the last `BT`, preceded by a full-page white `re f`; the
reference's sits at byte ~195, before every `BT`:

```
ours    stream 0: BT×34  lastBT=5589   Im1 at 5738      ← after all the text
        stream 1: BT×58  lastBT=7682   Im2 at 7485
reference  stream 0: BT×38 lastBT=10804  Im4  at 195    ← before all the text
           stream 2: BT×12 lastBT=2102   Im51 at 199
```

The image is 2480×3508 DeviceRGB DCTDecode — an A4 letterhead scan at 300 dpi, opaque, covering the
sheet. A blind reviewer given only the composed pair and forbidden the repo described our half as
*"a solid black band, then blank white all the way down… not a single glyph, rule, bullet or image"*
against a reference half carrying three headings and eleven bullets.

The footer survived on our side and that detail is the confirmation, not a curiosity:
`PageDrawing.Draw` emitted the footer *after* the frames, so the page numbers were the one thing the
raster did not bury. The reviewer read `Page 3 of 5` off our half and `Page 3 of 6` off the
reference's without being told either.

### The rule, and it is not the field that names it

Writer decides paint order with one item — `SvxOpaqueItem`, false meaning the *hell* layer, which is
painted before the text (`sw/source/core/layout/fly.cxx`:1129-1138). The two importers derive it
differently and neither reads what a reader would reach for first.

**WW8** (`sw/source/filter/ww8/ww8graf.cxx`:2831-2836):

```
bMoveToBackground = pRecord->bDrawHell || ((m_bIsHeader || m_bIsFooter) && aFSFA.nwr == 3)
```

- `bDrawHell` is the Escher `DFF_Prop_fPrint` group's `fBehindDocument` bit —
  `GetPropertyValue(DFF_Prop_fPrint, 0) & 0x20` (`filter/source/msfilter/msdffimp.cxx`:5546-5549),
  which in the group-relative numbering `EscherPropertyTable.Boolean` uses is property **954**.
- The `FSPA`'s own `fBelowText` bit is **not** part of the test. We already parse it into
  `Ww8ShapeAnchor.IsBelowText` and using it would have been the obvious move; the comment above the
  C++ says in terms that "#i46794 — it reveals that value of flag `<bBelowText>` can be neglected".
  It is the second clause that carries the corpus: a Word letterhead is a header-story shape with
  wrap 3 and states no `fBehindDocument` at all.

**DOCX** (`sw/source/writerfilter/dmapper/GraphicImport.cxx`), in the order that file assigns
`m_bOpaque`, because the order is the rule:

1. `m_bOpaque = !IsInHeaderFooter()` (:342) — so **every drawing in a header or footer is behind the
   text** whether or not it says so;
2. `behindDoc="1"` clears it (:698-702);
3. for `wrapSquare`, `wrapThrough`, `wrapTight` and `wrapTopAndBottom`, a file whose
   `compatibilityMode` is 15 or more puts it back (:1589, :1697; tdf#137850, *"Word >= 2013 seems to
   ignore bBehindDoc except for wrapNone, but older versions honour it"*).

So under a modern compatibility mode `behindDoc` is honoured for `wrapNone` alone — and a header
drawing is behind the text regardless. Only anchored drawings are asked; a `wp:inline` takes room on
its own line rather than floating over anything, and following LibreOffice's z-order push for those
(:242-246) would move as-character pictures in every header in the corpus to buy nothing visible.

### After

Every page of `info-bulletin-601.doc` now emits its image at byte 71–77, before the first `BT`. A
**second** blind reviewer — a fresh subagent, given three post-fix pairs and no numbers — described
our page 3 as carrying the bullets, the headings and the two black bands, and reported *"no
difference at all in non-text ink: both black panels are present, same size, same position, in both
halves."*

It does not flip to `match`, which P3 predicted. The reviewer named the reason without being asked:
our bullet-to-text gap is roughly twice the reference's and we set a blank line between bullets
where the reference sets them solid. That is a list-indent and paragraph-spacing defect, and it is
what costs the page — recorded in §7 as the next thing to measure on this document, not fixed here.

## 3. We drew no fill and no outline on a DrawingML shape

`words-extra-01` recorded this and deliberately did not fix it. It is real and it is now fixed.

A **third** blind reviewer, on a document neither of the first two saw (`ABCD-FE-01-00` page 13),
independently reported the reference's *"light-grey band spanning the full page width"*, its
*"dark grey filled bar containing 'Document reference.'"* and its *"filled boxes behind the footer
page number and the Date/Revision/Doc.-No. fields"* as absent from ours, with ours *"plain white"*
and *"bare text on white"* in each place.

`PageFrame` has carried `Fill`, `BorderColour` and `BorderWidth` all along — the WW8 reader sets all
three from the Escher properties — and `DocxFrames` set none of them. `Appearance` now reads
`wps:spPr`'s `a:solidFill` and `a:ln` through `DrawingColour`, which resolves `a:alpha` and the
theme's `a:schemeClr` chain; the theme reaches it through a new `DocxFrameContext` carrying the
three things a drawing's surroundings decide about it and the drawing itself does not state.

Only `a:solidFill` is read, on the area and on the line. A gradient, pattern or picture fill is a
real fill this cannot yet draw, and painting its first stop flat would be a confident wrong answer
rather than an absent one. A shape stating **no** fill element is left unfilled rather than given
the fill its `wps:style/a:fillRef` names out of the theme's format scheme — that is a whole style
matrix (`oox/source/drawingml/shape.cxx`) and reading only what the shape itself states never
invents ink. `a:noFill` is honoured explicitly, so "stated none" and "said nothing" already differ.

### Why a grep of the reference's content stream finds no fill

The header panel is `<a:solidFill><a:srgbClr val="000000"><a:alpha val="50000"/></a:srgbClr>`, and
LibreOffice's PDF export writes an alpha fill as a **transparency-group XObject** rather than as a
plain `re f`. That is why `words-extra-01` had to count XObjects to see it at all, and it is
asserted in a test rather than left as a note.

Measured on `ABCD-WB-08-00`'s page 2, in the operators: **6 fills and 4 strokes before, 12 fills, 5
strokes and 6 `/GS` alpha states after**, against the reference's 9 transparency groups and 6
strokes.

The blind reviewer's second pass, on the post-fix pairs of `ABCD-WB-08-00` page 2 and
`ABCD-SDE-23-00` page 4, lists the *"pale grey header band"*, the *"grey-outlined 'Company logo'
box"* and the *"medium-grey 'Document reference:' bar (position, width, height, right-aligned
label)"* under **identical** on both documents. That is the exact set of three things two reviewers
reported missing two rounds ago.

## 4. Reach, all 200 words documents, both directions

Rendered twice with `SOURCE_DATE_EPOCH` set — once with the fixed binary, once with a
behaviour-reverted one built from files copied aside (never `git stash`) — and diffed byte for byte.

- **64 of 200 renderings changed**; 136 byte-identical.
- **No page count and no word count moved on any of the 200.** Not one column of the gate.
- **0 verdicts won, 0 lost.** Track total stays 173 of 200; `words/done-*` stays **158 of 159**.

What did move, counted in the operators over the 64:

| | |
|---|---:|
| documents that gained a page whose ink now precedes its first text | **39** |
| documents whose fill / stroke / alpha counts changed | **25** |
| net fill operators | **+460** |
| net stroke operators | **+190** |
| net constant-alpha graphics states | **+378** |

| document | fills | strokes | alpha | pages with ink before text |
|---|---|---|---|---|
| `ABCD-SDE-23-00 … Avionic System Description` | 3405 → 3589 | 659 → 704 | 0 → 175 | 0 → 17 |
| `ABCD-FE-01-00 … Flight Envelope` | 277 → 362 | 303 → 317 | 0 → 84 | 1 → 14 |
| `ABCD-WB-08-00 … Weight and Balance` | 288 → 360 | 181 → 193 | 0 → 72 | 0 → 12 |
| `done-014/Regulations Governing the Status…` | 31 → 48 | 16 → 16 | 0 → 0 | 0 → 16 |
| `pagination-002/docs-quality-MA.IMS.00001…` | 1996 → 2029 | 1273 → 1306 | 0 → 33 | 1 → 2 |

### The refutation criterion, and the one document that tripped it

`prediction.md` said: *if the shape-fill change puts an opaque box over text on any document, the
fill is being read from the wrong element.* Swept for it — any fill covering more than 40% of a page
emitted after that page's first `BT` — and **exactly one document gained one**:
`metrics-001/FRE-03_mcar_part-3_and_IS_v2.9.docx`, page 50, a white 469.8 × 556.35 pt box.

It is correct. The box is a front-of-text shape with a white fill, and the text drawn *over* it
follows it in the stream; the only text preceding it is a footer at y = 70.35, below the box's lower
edge at y = 121.75. Page 50's ink is **7.10% before, 7.61% after, 7.41% in the reference** — the
change moves it towards the reference rather than away. The criterion fired and the inspection
cleared it, which is the criterion working.

## 5. Tests

11 new tests in one file, `tests/XRay.Office.WordProcessing.Tests/FramePaintOrderTests.cs`, over
`tests/corpus/features/header-behind-text.docx` — authored by
`probes/words-e1-01/make-fixture.py`, one body line and a header holding two anchored shapes: a
full-page 50%-alpha panel with `wrapNone`, and a 2 cm box with `wrapSquare`, an opaque `777777` fill
and a 1 pt red outline.

**Every expectation is read off LibreOffice's own rendering of that fixture**, not restated from the
markup it was written from. Its content stream is, in order:

```
q /EGS6 gs /Tr5 Do Q                                  ← the alpha panel, a transparency group
0.4666666667 0.4666666667 0.4666666667 rg … f*        ← 0x777777
1 0 0 RG q 1 w … S Q                                  ← FF0000, 12700 EMU
BT 72.1 759.389 Td /F1 11 Tf [ … ] TJ ET              ← BODYLINE, last
```

The reference draws **both** shapes before the text, which is the `!IsInHeaderFooter()` default of
§2 and is why the fixture is a header rather than a body.

The two end-to-end tests go through the real reader and a sink that keeps **one flat log** rather
than `RecordingDrawingSink`'s per-kind lists — that recorder answers "what was drawn" and cannot
answer "in what order", which is the only question here. The other nine are the two readers' rules,
each with its discriminating opposite: header-with-wrap-3 against body-with-wrap-3 and
header-with-wrap-2; `fBehindDocument` against the `fHidden` bit four places away in the same word;
`behindDoc` under compatibility 14 against 15; and a `wp:inline` in a header, which must not inherit
the header default.

### Verified failing against the unfixed behaviour, in two separate reverts

| reverted | result |
|---|---|
| the paint order only (`PageDrawing.Draw` back to one loop after the body) | **1 failed**, 10 passed — the ordering test reports `["glyph", "fill", "fill", "stroke"]` |
| the appearance only (`Appearance` returning nothing) | **2 failed**, 9 passed |

The second revert fails two rather than one, and the reason is worth stating: with no fill and no
outline, the fixture's frames draw *nothing at all*, so the ordering test's log is `["glyph"]` —
there is no ink left to be out of order. The two halves are still proved separately, because the
first revert leaves the appearance test passing.

### Counts, every project run individually

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 337 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 563 | 0 | 0 |
| Vector | 295 | 0 | 0 |
| Rendering | 150 | 0 | 1 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 125 | 0 | 0 |
| WordProcessing | **914** | 0 | 0 |
| Spreadsheets | 853 | 0 | 0 |
| Presentations | 717 | 0 | 0 |
| **Fidelity** | **520** | **30** | **0** |

WordProcessing is 903 + the 11 new. Build is 0 warnings, 0 errors. No flaky run was seen; every
count is from a single pass with each project named on its own.

**Fidelity is 30 failed of 550 — checked by name, not only by count.** The reverted-behaviour build
and the fixed one produce the *identical* 30 names, `diff` clean, captured in
`fidelity-before-names.txt` and `fidelity-after-names.txt`. The list includes
`FrameComparisonTests` and `PageDrawingComparisonTests`, which are the two that could plausibly have
moved, and neither did.

### The mtime trap, guarded rather than avoided

Four builds across two revert/restore cycles. Every restore is `cp` followed by `touch`, with
`rm -rf src/XRay.Office.WordProcessing/{obj,bin}` before the rebuild, and each restore is checked by
re-rendering a subset and comparing **byte for byte** against the run being claimed: **47 of 47**
after the first cycle (`words/done-00[1-4]` plus the group), **76 of 76** after the second
(`words/done-01*` plus the group). A `grep` for the neutering markers returns 0 in both files.

## 6. Contradicting the brief

- **"`762.doc` and `info-bulletin-601.doc` … both share an opaque full-page raster emitted AFTER the
  text."** True of `info-bulletin-601`, **false of `762.doc`**, and checked exactly where the brief
  says to check it. `762.doc` has images on **one** page of 23, and on that page our ordering
  already matches the reference's: one image early and one late on both sides (`Im5` at 249 and
  `Im39` after the last `BT` in the reference; `Im1` at 485 and `Im2` after the last `BT` in ours).
  Its rendering changed by not one byte this round. Its real defect is a flow difference spread over
  the whole document — ±2 to ±17 words a page with no single site — which loses one page in 23.
- **"`info-bulletin-601` renders as five blank pages while its word column reads 1299 of 1299."**
  The mechanism is right and the number is not: this container's baseline is **1298 of 1302**, and
  the 1299-of-1299 figure belongs to `airbus-pdf-information-package_v1-4.docx`, which is the one
  `words/done-*` failure and a different document entirely.
- **"`words/extra-001`, currently 5 failing."** Reproduced exactly, on two independent instruments.
- **"One document in your group holds 33 `m:oMathPara`."** Confirmed —`ABCD-FE-01-00`. Its residual
  is not only that: the same blind reviewer who read its page 13 reported a large diagonal
  *"EASA Example Documents"* watermark on the reference and none on ours, which is a VML WordArt
  shape (`_x0000_t136`, `PowerPlusWaterMarkObject…`) in `header1.xml` and `header3.xml`. A preset
  text path with its own fill is a feature, not a paint-order fix; §7.
- **"the reference merely draws horizontally scaled text"** — the brief's warning about refuted
  italics claims. A reviewer did report the reference slanting `UG.CAO.00006`'s numbered headings
  where ours draws them upright. It is **not** chased here and is **not** claimed as a defect: the
  document's whole per-page surplus is accounted for by §6's header defect, and the observation is
  recorded in §7 as unverified rather than acted on.

## 7. Deliberately not done, named plainly

Three seats were opened and two were closed. What was left, and why:

1. **`762.doc`** — one page short over 23, no single site. Not the brief's raster defect (§6), and a
   flow difference of that shape is a round of its own.
2. **`UG.CAO.00006` and `UG.CAO.00133`** — the same LibreOffice table-only-header import defect, now
   established for *both* rather than only the second. `UG.CAO.00006`'s `word/header1.xml` has
   `['tbl']` as its only top-level child, exactly as `UG.CAO.00133`'s does, and its per-page surplus
   is a flat **+20 words on pages 2 through 13** — the header's own word count, drawn by us on every
   page and by the reference on page 1 alone. Over 29 pages that is the whole of its +612. Following
   the reference here means deleting `SectionInheritedHeaderTests`, which exists to say why not to.
   **Two verdicts, knowingly left.**
3. **`ABCD-FE-01-00`** — its reference is measuring a LibreOffice with no Math module, so 33
   `m:oMathPara` draw nothing on that side (`MISSING_PACKAGES.md`; not installed here, deliberately,
   because re-banking is a decision for whoever owns that artefact). On top of that, the VML WordArt
   watermark above.
4. **`info-bulletin-601`'s remaining page.** The strongest new lead in the group and the cheapest:
   a blind reviewer reading the post-fix pair reported our bullet-to-text gap at roughly twice the
   reference's and a blank line between bullets where the reference sets them solid. Both push
   content down and both are list-indent/paragraph-spacing questions, not paint order.
5. **A shape's `wps:style/a:fillRef`.** A shape stating no fill takes one from the theme's format
   scheme; only what the shape itself states is read (§3).
6. **Gradient, pattern and picture fills** on DrawingML shapes.

## 8. Predictions, scored

Nine right, one wrong, one half.

| # | claim | conf | outcome |
|---|---|---:|---|
| P1 | the raster is behind-text by `bDrawHell \|\| (header && nwr == 3)` | 80% | **right** — and it is the second clause |
| P2 | the paint order makes the five pages legible | 85% | **right**, confirmed by a blind reviewer who was told nothing |
| P3 | it does not flip to `match` | 80% | **right** — still 5 pages against 6 |
| P4 | `762.doc` is not the same defect and the brief is wrong about it | 85% | **right** — §6 |
| P5 | `UG.CAO.00006` is the table-only-header defect, not ours | 85% | **right** — `['tbl']`, and a flat +20 a page |
| P6 | the shape fill moves no verdict at all | 75% | **right** — 0 won, 0 lost, no column moved |
| P7 | 10–40 of the 200 renderings change | 50% | **wrong** — 64. The header rule reaches far more documents than `behindDoc` alone would |
| P8 | no `done-*` document loses its verdict; 158 of 159 | 70% | **right** |
| P9 | Fidelity stays 30 of 550, by name | 75% | **right** — the identical 30, `diff` clean |
| P10 | one `PageFrame` property, one branch in `PageDrawing`, one rule per reader, no layouter change | 80% | **half** — no layouter changed, but the DOCX rule needed the theme, the header flag and the compatibility mode threaded in, which is a new context type and a field on `DocxLayoutSource` |
| P11 | `ABCD-FE-01-00` does not flip | 85% | **right** |

P7 is the instructive miss and it is the same shape as the last round's: I sized the reach from the
defect I had in hand — one letterhead — rather than from the rule I was about to implement, and
`!IsInHeaderFooter()` applies to every anchored drawing in every header in the corpus.

## 9. Files

```
src/XRay.Office.MsBinary/Escher/EscherRecordTypes.cs        EscherPropertyIds.BehindDocument = 954
src/XRay.Office.WordProcessing/Layout/PageFrame.cs          BehindText, with both rules on it
src/XRay.Office.WordProcessing/Layout/PageDrawing.cs        the two loops round the text
src/XRay.Office.WordProcessing/Ww8/Ww8Frames.cs             bMoveToBackground
src/XRay.Office.WordProcessing/Ooxml/DocxFrames.cs          m_bOpaque; Appearance; DocxFrameContext
src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs    InHeaderFooter, and the context it builds
src/XRay.Office.WordProcessing/Ooxml/DocxReader.cs          sets it round the furniture walk
tests/XRay.Office.WordProcessing.Tests/FramePaintOrderTests.cs   11 tests
tests/corpus/features/header-behind-text.docx             the fixture
probes/words-e1-01/make-fixture.py                        which authors it
probes/words-e1-01/prediction.md                          written first, commit 413215679e8
probes/words-e1-01/fidelity-{before,after}-names.txt      the 30, twice, identical
```
