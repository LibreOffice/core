# words-r63 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r63`, base
`43142b73ccf`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`; every sweep's `TMPDIR` on the host mount at
`/c/sandbox/workdir/scratch-r63-words/tmp`, and `/` never rose above 72 %. `prediction.md` was
committed at `0a0fc107ae3`, **before** the first behavioural commit `7cbff417b86`.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 340 MISMATCH 15 REF-CANNOT-RENDER 0`, scored
against `MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries. **323 of 337, zero disagreements with the manifest's status
column, document for document.** The scorer refuses to print unless every manifest path found a row.

## Result

**323 → 323 of 337. Zero verdicts gained, zero lost, and that was the prediction.** Both changes are
draw-only by construction and the round is a fidelity round, not a gate round.

| | base | after change 1 | after change 2 |
|---|---:|---:|---:|
| words verdicts | 323 | 323 | **323** |
| renderings whose normalised bytes changed (against the previous leg) | — | **42** | **9** |
| verdicts gained / **lost** | — | 0 / **0** | 0 / **0** |
| page counts changed | — | 0 | **0** |
| extractable word counts changed | — | 0 | **0** |
| font lists changed | — | 0 | **0** |
| documents whose fill agreement with the reference improved / **worsened** | — | 42 / **0** | — |
| glyphs we draw white the reference draws black (round 59's LONG column) | 34 in 2 | 34 in 2 | **34 in 2** |
| glyphs the reference draws white and we do not (SHORT) | 2728 in 38 | 2728 in 38 | **2571 in 37** |

Prediction against measurement, per direction, as `COMMON.md` now requires:

| quantity | predicted | measured |
|---|---|---|
| change 1, renderings changed | 38–46 | **42** ✓ |
| change 1, verdicts gained / lost | 0 / 0 | **0 / 0** ✓ |
| change 1, pages / words / fonts moved | 0 / 0 / 0 | **0 / 0 / 0** ✓ |
| change 1, `012` page 1 fills | 19 → 75, colour for colour | **19 → 75**, `#F2F2F2` 0→48, `#FFFFFF` 0→8, the rest unmoved ✓ |
| change 1, `012` page 1 strokes | 2, unchanged | **2** ✓ |
| change 2, renderings changed | 2–9 | **9** ✓ (top of the band) |
| change 2, verdicts gained / lost | 0 / 0 | **0 / 0** ✓ |
| change 2, LONG column | **34, unchanged** | **34, unchanged — same two documents, same counts** ✓ |
| change 2, `069` text shows | 87 black / 0 white, unchanged | **87 / 0** ✓ |

The census that sized change 1 said 733 cells in 42 documents; corrected before implementation to
**749 in 42** when the unconditional half was found to live on `w:style/w:tcPr` and not under
`w:tblPr`. The document count — which is what the band was stated in — did not move, and the 42
documents that changed are **exactly** the 42 the census named.

---

## 1. A table style's conditional `w:tcPr`, and the bands

Round 62 §3 read the rule out of `012` and named the seat, and both halves of it were wrong in the
same place. `WordStyle`'s constructor **discarded any `w:tblStylePr` that carried no `w:rPr`**, and
`WordTableStyleConditions.Names` never offered the four band layers at all. `PlainTable5`'s
`firstCol`, `band1Horz` and `band2Horz` layers are all `w:tcPr`-only, so a conditional cell shade was
read by nothing anywhere in this reader.

### `012`, operator for operator

| | reference | ours before | ours after |
|---|---|---|---|
| page 1 fills | 75 — `#F2F2F2` 48, `#000000` 12, `#FFFFFF` 8, `#ED7D31` 4, `#002060` 3 | 19 — `#000000` 12, `#ED7D31` 4, `#002060` 3 | **75, identical to the reference's histogram** |
| page 1 strokes | 10 — `#000000` 9, `#7F7F7F` 1 | 2 | 2, unchanged |
| page 2 fills | 1 — `#FFFFFF` | 0 | **1 — `#FFFFFF`** |
| page 2 strokes | 1 — `#7F7F7F` | 0 | 0 |

Not only the counts: every one of the 48 band rectangles and all 8 `firstCol` rectangles matches the
reference's own rectangle to **≤ 0.10 pt on every edge**, which is the same sub-tenth rounding the
twelve black fills already carried before this round.

The eight remaining strokes are the other half of the same element — `firstRow`'s
`w:tcBorders/w:bottom` gives the `#7F7F7F` rule on both pages, and the seven black ones are the bar
shapes' `a:ln`. Neither is in this change.

### The band rule, and what fixes it

Band 1 is the *first* band, so a zero-based band index that is even takes `band1Horz`; the count
excludes the rows and columns an edge layer claims. `012` fixes both: its bands land on table rows
**2, 4, 6 and 8** with `w:tblStyleRowBandSize="1"` and `firstRow` on, and counting the heading row
would put them on 3, 5, 7 and 9.

`w:tblLook`'s two band bits are stated **the other way up** — the attribute is `noHBand`, not
`hBand` — so reading them like the other four bands every table that switches banding off and none
that switches it on. `012`'s own look is `noHBand="0" noVBand="1"`.

### The one route to a moved line, closed by measurement before the change

Adding the bands to `Names` hands them to `TableStyleRunProperties` too, and a band layer carrying a
`w:rPr` **would** change how text is measured. Two documents declare such layers
(`te.iors.00048-002 SUP Questionnaire.docx`, `EHEST-SMS-Safety-Management-Manual-V2.docx`) and in
both the styles are latent: **no table in the corpus names a style whose `w:basedOn` chain reaches
one — 0 of 271**, checked through the full chain. That is why 0 page counts and 0 word counts moved,
and it was written down before the sweep rather than discovered by it.

### The fixture is authored and its answer is the reference's

`probes/words-r63/make-band-fixture.py` writes `table-style-bands.docx`: six rows by three columns,
one style stating a **distinct** fill on `firstRow`, `firstCol`, `band1Horz` and `band2Horz` at once
with `w:tblStyleRowBandSize="2"`, and one cell stating its own `w:shd`. Distinct colours mean the
reference's own fill operators name which layer won, cell by cell. Predicted from the rule and
measured on 26.2.4.2:

```
reference:  #FFF2CC 10   #D9E2F3 10   #FBE4D5 8   #4472C4 6   #00B0F0 2   #EDEDED 1
ours:       #FFF2CC  5   #D9E2F3  5   #FBE4D5 4   #4472C4 3   #00B0F0 1
```

Exactly half, cell for cell, plus one. **The reference paints every cell fill twice** — a cell
background and a table background over one another — and the odd `#EDEDED` is the style's
unconditional `w:style/w:tcPr` painted once behind the table, which we do not draw. That is a
one-operator divergence recorded rather than chased; what the fixture pins is which layer each of
the eighteen cells resolves to, and on that the two agree 18 of 18.

---

## 2. Round 59's counter-witnesses, re-measured — and they are Writer text boxes after all

The brief's question was *"find out whether those shapes are Writer text boxes at all"*. **They are,
their fills are consulted, and the discriminator is neither the shape kind nor the drawing layer.
It is the fill's own transparency, and both witnesses state one.**

- `docs-quality-MA.IMS.00001-…docx` page 9: a `wps` text box filled
  `<a:solidFill><a:srgbClr val="0070C0"><a:alpha val="52941"/></a:srgbClr></a:solidFill>`.
- `069_Work_Breakdown_Structure_Template_Professional_Format`: **no `wps:wsp` at all** — the whole
  document is VML — and the witness is `<v:rect … fillcolor="#8496b0 [1951]"><v:fill opacity="26214f"/>`.

`SwDrawTextInfo::ApplyAutoColor` does not ask the fill for its colour. It asks
`SdrAllFillAttributesHelper::getAverageColor(aGlobalRetoucheColor)`, which interpolates the fill
toward the application's retouche colour — white — by the transparency, and only then is
`Color::IsDark` asked. Blended, `#0070C0` at 47.059 % is WCAG **105** and `#8496B0` at 60 % is
**168**. Both bright. Both black. **Round 59's measurement and round 62's rule are both correct and
the missing term is alpha.**

### The arms, chosen so the two hypotheses answer differently

`alphaauto.py`, one substitution per arm on the corpus documents themselves. H-alpha is the blend;
H-shape is round 59's reading, that these shapes' text is drawn by editeng and never reaches
`ApplyAutoColor`.

| arm | H-alpha | H-shape | measured |
|---|---|---|---|
| `069` as found (`#8496B0`, opacity 0.4) | black | black | 87 black / 0 white |
| `069` with the opacity removed | **WHITE** | black | **1 white** — H-alpha |
| `069` filled opaque black | **WHITE** | black | **1 white** — H-alpha |
| `069` filled opaque white | black | black | black — the control |
| `069` filled black at opacity 0.2 | black | black | black |
| `ims` as found | black | black | 45 black / 0 white |
| `ims` with `<a:alpha>` removed | **WHITE** | black | **1 white** — H-alpha |
| `012` as found | WHITE | — | 23 white / 12 black |
| `012` title box filled **opaque** black | WHITE | — | 23 / 12, unchanged — round 62's arm `t` reproduced |
| `012` title box filled black at `a:alpha val="20000"` | **black** | — | **20 white / 15 black — three shows move white → black** |

The last row is the term measured **in the other direction, on a third document, whose anchor is
dark** — so it refutes "the anchor decides" and confirms the blend in one arm. **H-shape is refuted
on both witnesses, in both formats.**

### The blend is pinned on three colours at once, with no free parameter

The blend predicts a *different* flip transparency for every fill colour. `threshold.py` straddles
each one on `069` with nothing else changed:

| fill | predicted flip | arms | result |
|---|---:|---|---|
| `#8496B0` | 9.571 % | 8.4 / 9.2 / 9.4 / 10.0 % | white, white, white, **black** |
| `#0070C0` | 37.454 % | 36.2 / 37.0 / 37.8 / 37.9 % | white, white, **black**, **black** |
| `#000000` | 62.222 % | 61.0 / 62.0 / 63.0 % | white, white, **black** |

**11 of 11.** A constant threshold, a threshold that ignores the fill colour, and no blend at all are
all refuted by the same eleven renderings, because the three flips are in three different places.

**And the probe's first cut mispredicted one arm, which is how the last digit was found.** It
bisected on the *continuous* luminance ≤ 87 and put the flips at 8.796 / 36.882 / 61.900 %.
`Color::GetWCAGLuminance` returns a `sal_uInt8`, so the comparison is against the **truncated**
value and the flip is where the blend first reaches 88.0. `#8496B0` at 9.4 %, predicted black under
the continuous reading and white under the truncating one, is the arm that separates them, and it
measured white. 5 of 6 became 11 of 11.

### What shipped, and the control it is gated on

A text box's own fill is now the background an automatic font colour resolves against, blended by its
transparency; and `v:fill/@opacity` is read, in all three of its spellings, which the VML reader did
not read at all. **Round 59's LONG column is the gate and it did not move: 34 glyphs in 2 documents,
the same two, the same counts.** That is the column that matters, because painting text out of a page
moves no page count, no word count and no font list — nothing in the gate would have said a word.

Nine renderings changed. Eight moved toward the reference and one — `069`, the counter-witness — kept
its text colour exactly as it was, which is the whole point:

| document | white glyphs before | after | reference | |
|---|---:|---:|---:|---|
| `067_Work_Breakdown_Structure_Template_Gray_Theme` | 0 | **12** | 12 | closes |
| `065_Work_Breakdown_Structure_Template_Blue_Theme` | 0 | 11 | 12 | closer |
| `048_Visual_Product_Roadmap_Template_Quality_Layout` | 239 | 270 | 272 | closer |
| `ABCD-WB-08-00 Weight and Balance Report` | 288 | 303 | 309 | closer |
| `016_Project_Timeline_Template_Complete_Guide` | 342 | 366 | 380 | closer |
| `ABCD-FE-01-00 Flight Envelope` | 336 | 355 | 389 | closer |
| `059_Disease_Concept_Map_Template` | 109 | 127 | 132 | closer |
| `012_Project_Timeline_Template_Black_and_Brown_Theme` | 49 | 76 | 134 | closer |
| `069_Work_Breakdown_Structure_Template_Professional_Format` | 0 | **0** | 0 | unchanged, as required |

**8 improved, 1 unchanged, 0 worsened.**

### The fill itself, confirmed by a second instrument

Reading `v:fill/@opacity` also makes the box translucent where we painted it at full strength. At
100 dpi, `069`'s three fill colours:

| region | ours before | ours after | reference |
|---|---|---|---|
| row bands | `#F2F2F2` | **`#FAFAFA`** | **`#FAFAFA`** |
| category boxes | `#D5DCE4` | **`#EEF1F4`** | **`#EEF1F4`** |
| title box | `#8496B0` | **`#CED5DF`** | **`#CED5DF`** |

All three now match the reference **exactly**, and `#CED5DF` is what the blend predicts for
`#8496B0` at alpha 102. Our output carries 22 `/GS1 gs` ExtGState references where the reference
carries 22 of its own.

### What did *not* ship, and why

The other limb of round 62's rule — a `noFill` box continues to its **anchor's** background — is
**not** in this change. `012`'s white title needs it, and the anchor is not reachable from
`PageDrawing.DrawFrame`, which draws frames from a per-page list built by `FrameLayout`. That is the
next round's plumbing job, and it is why `012` is at 76 white glyphs against the reference's 134.

---

## 3. An instrument that reported two regressions that do not exist

The fill-agreement measurement first came back **40 improved, 2 worsened**, and the two were
`005_advanced_word_chart_report` and `007_advanced_word_chart_report` going from a perfect 0 to 13
— on documents whose markup is byte-for-byte the same shape as the 31 that improved.

They were not regressions. `textcolour.py`'s `page_streams` — round 62's, inherited here — extracts a
stream with `re.search(rb'stream\r?\n(.*?)\s*endstream')`, and `\s*` eats any whitespace byte at the
end of the **compressed** data. `0x0A` is an ordinary deflate byte. Two of the 337 reference PDFs end
that way; zlib then raised and the code `continue`d, so the page came back with **no operators at
all** and every count on it read as zero.

Two things are worth carrying:

- **A second instrument caught it in one step.** Rasterising at 100 dpi, `005`'s reference has
  35 944 `#D3DFEE` pixels and 14 006 `#4F81BD` — identical to `001`'s reference — and ours went from
  0 and 2 136 to 36 688 and 14 079. The corrected count is **42 improved, 0 worsened**.
- **The fix has a trap of its own and the first cut hit it.** `/Length` is an *indirect* reference in
  every PDF LibreOffice writes (`/Length 3 0 R`), so reading it as a literal integer returned the
  object number and every stream in the corpus came back empty — a change that turns two false
  regressions into 337. It is resolved through the object table now, with the old search as a
  fallback, and `012`'s 75/10 reproduces as the control.

All of the round's earlier probe PDFs were re-read with the corrected reader and **not one number
moved**, so the eleven-arm bracket and the ten-arm quadruple stand as first measured.

---

## 4. The vision reading

Three blind readings, each handed one composed image and nothing else, each forbidden from reading
any other file or running any command, each asked to describe the halves separately before comparing
and to give the direction. None was chosen by `--worst`.

### `012_Project_Timeline_Template_Black_and_Brown_Theme` page 1 — the round's own item

Chosen because it is the document round 62 named and the one both changes land on. Round 61's and
round 62's readers, on this same page **before** this round, both reported *"the reference draws
alternating grey row bands on odd task rows; ours draws none"*.

This round's reader, who had seen neither, lists the row banding among the things that are
**identical**: *"the alternating light-grey/white row shading pattern"*, in a paragraph of features
that match. It reports the bands on both halves and does not report them as a difference. **Three
readers over three rounds, and the sentence has changed side.**

Confirmed by a second instrument and more sharply: 48 `#F2F2F2` fills on both halves, at rectangles
agreeing to 0.10 pt.

What it *does* still report, in the right direction both times:

* *"the top half displays a large italic serif title … the bottom half does not display this title
  … that entire area is blank"* — ours draws the title black on black paper where the reference
  draws it white; still open, and it is the anchor limb §2 did not ship.
* *"the corresponding bars in the bottom half have a thin dark outline around each bar"* — the seven
  `a:ln` outlines. Confirmed by the dump: the reference strokes 9 black paths on that page and we
  stroke 2. Unchanged this round and correctly reported as unchanged.

### `069_Work_Breakdown_Structure_Template_Professional_Format` page 1 — change 2's control

Chosen because it is round 59's counter-witness: its text **must not** change colour and its boxes
**must** become paler.

The reader was asked specifically about text colour inside coloured boxes and about saturation, and
answered: *"the pale blue-grey boxes … appear similarly pale/desaturated in both halves — this looks
identical, not different"*, and *"no other differences in box fill colour, text colour inside boxes,
or saturation were detected"*, at low confidence on the last point by its own account.

**Confirmed at pixel level and much more sharply than the reading can be:** all three fill colours
match the reference to the byte after the change and none of them did before (`#F2F2F2`→`#FAFAFA`,
`#D5DCE4`→`#EEF1F4`, `#8496B0`→`#CED5DF`). A blind reader cannot tell `#8496B0` from `#CED5DF` at
150 dpi through a 76 % downscale, and it correctly declined to claim it could.

**And it found something neither change touches, unprompted and at high confidence:** *"the top-level
pale blue-grey box is rendered empty, while the 'PROJECT NAME' text is displaced downward and to the
right, overlapping the 'DEVELOPMENT' label of the row below"*. That is a **new lead** — the vertical
placement of text inside a VML text box on a document that fails the gate on words, 108 against 117.
Not checked by a second instrument this round.

### `airbus-pdf-information-package_v1-4` page 5 — the largest reach of change 1

Chosen because it is the document change 1 reaches furthest into (136 of the 749 cells) **and it
already passes the gate**, so it is where a draw-only change is most likely to break something the
gate cannot see. Page 5 chosen because it carries the largest pre-change fill gap of the document's
nine, not because of ink.

The reader, asked directly about row shading, header shading and text colour in shaded cells:
*"the alternating row shading pattern (white, then light grey, then white, then light grey…) in the
data rows appears the same in both halves — **I do not see a case where the top shades a row grey
that the bottom leaves white, or vice versa**"*, and *"text colour inside shaded (grey and teal)
cells is black in both halves"*.

Second instrument: on that page the reference draws 60 `#F2F2F2` and 4 `#00CC99`; we drew 0 and 2
before and 30 and 2 after — the reference's doubling again, so cell for cell we now draw what it
draws. Over the whole document the fill distance falls from 314 to 178.

---

## 5. The 24.2.7.2 audit — one site, VERIFIED, and the list's over-read quantified

`XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs`:777 states three things about `w:pPr/w:rPr`
being the *paragraph mark's* formatting and says all three were measured against the superseded
binary. `probes/words-r63/audit_markstyle.py` re-measures all three on 26.2.4.2, **on the reference
alone** — our renderer never runs — with a control for each whose answer was known first:

| arm | claim | measured |
|---|---|---|
| `Heavy` paragraph, mark says `<w:b w:val="0"/>` | text stays bold | **LiberationSans-Bold, 10.00 pt** |
| the same style, the **run** says it — the control | text goes regular | LiberationSans, 10.00 pt |
| plain paragraph, mark says `<w:b/><w:sz w:val="48"/>` | text stays 10 pt upright | **LiberationSans, 10.00 pt** |
| the same, the **run** says it — the control | bold and 24 pt | LiberationSans-Bold, 24.00 pt |
| empty paragraph whose mark states `w:sz w:val="72"` | its height is the mark's | baseline pitch **52.90 pt** against a control's **23.00**, so the paragraph is 11.50 + 29.90 = **41.40 pt = 36 × 1.15 exactly** |

**VERIFIED.** The mark/run pairs are what make this an experiment rather than a confirmation: under
"the mark formats the text" the two members of each pair answer the same, and they do not.

**And the more useful half of this entry is a classification the list has never had.**
`XRay.Office.WordProcessing` shows **11** open hits, and they are not 11 open sites: five sit inside an
existing marker (`WordCompatibility.cs`, `WriterPoolSpacing.cs` twice, `WordStyles.cs`:316, and the
`Paginator.cs` note that explicitly *supersedes* a 24.2.7.2 claim), and five more state a 26.2.4.2
measurement in the same comment as the older one they replaced — two of those in the ODT and RTF
readers, whose formats have **no witness in this corpus at all** (271 `.docx`, 66 `.doc`, 0 `.rtf`,
0 `.odt`). That left exactly one reachable open site and it is now closed. The header has warned
three times that the string over-reads; this is the first time it has been counted: **11 hits, 1
reachable site.**

Counters re-derived at both commits with the file's own commands, never quoted:

| | base `43142b73ccf` | this tree |
|---|---:|---:|
| open hits (lines with the string, less marker lines) | 37 | **37** |
| marker lines | 34 | **35** |
| VERIFIED / FIXED / WRONG / UNDECIDED | 29 / 4 / 1 / 0 | **30 / 4 / 1 / 0** |

The open count does not fall, because a marked site keeps its original prose line — which is exactly
the self-corruption the file's header describes, and the marker's own text was written to avoid
spelling the version and adding to it.

---

## Refutations, collected

1. **Round 59's counter-witnesses are not evidence that a shape's text bypasses `ApplyAutoColor`.**
   Both state a fill transparency; blended toward white they are luminance 105 and 168. Seven arms
   over the two documents, each chosen so the two hypotheses answer differently, and the rival
   hypothesis is refuted on both — including on `069`, which holds **no DrawingML shape at all**.
2. **The discriminator is not the shape kind either.** VML and DrawingML behave identically once the
   transparency is accounted for: `069`'s `v:rect` and `ims`'s `wps:wsp` both flip to white when
   their transparency is removed.
3. **`Color::IsDark`'s threshold is not the continuous 87.** `GetWCAGLuminance` returns a
   `sal_uInt8`, so the flip is where the blend first reaches 88.0. One arm of eleven separates the
   two readings and it went the truncating way.
4. **`012`'s missing fills are not `w:shd`** (round 62's finding, now implemented) **and they are not
   the run half of a conditional layer either**: `WordStyle` discarded any `w:tblStylePr` with no
   `w:rPr` inside it, which is every band and column layer of `PlainTable5`.
5. **Adding the band layers cannot move a line in this corpus.** Two documents declare a band layer
   carrying a `w:rPr` and **0 of 271** name such a style through any `w:basedOn` chain. Measured
   before the change; 0 page counts and 0 word counts moved.
6. **A table style's unconditional cell half is `w:style/w:tcPr`, not `w:tblPr/w:shd`.** The census
   read the wrong slot first and under-counted by 16 cells; the document count was unaffected.
7. **Two "regressions" that never existed.** `page_streams` ate a whitespace byte off the compressed
   data of two of the 337 reference PDFs and reported their pages as empty. A 100 dpi raster refutes
   it in one step, and the corrected figure is 42 improved / 0 worsened.
8. **And the fix's own first cut was worse than the bug.** `/Length` is an indirect reference in
   every PDF LibreOffice writes; reading it as a literal returned the object number and emptied
   every stream in the corpus.
9. **A test named after a rule can pass under that rule's own mutation.** `verify-test.sh` found
   `TheHeadingRowIsNotCountedAsABand` detecting nothing: at a band size of two, counting the heading
   row leaves the *first* body row in band 0 either way. It now asks rows 2 and 4.

## Tests

```
Core 407   Containers 109   Text 625   Vector 302   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1251   Spreadsheets 1035   Presentations 878     = 5144
0 failed, 1 skipped
```

Re-derived project by project rather than quoted. The whole delta is
`XRay.Office.WordProcessing` **1231 → 1251, +20**: 8 `ConditionalCellShadingTests`, 7
`TranslucentAutoColourTests`, and 5 added to `VmlShapePaintTests`.
`dotnet build -v q -nologo`: **0 warnings, 0 errors.**

Through `verify-test.sh`, tree clean before each and restored after — **eight mutations, all eight
detected**, and the attribution is the point:

| mutation | detected by |
|---|---|
| the horizontal bands removed from `Names` | four `ConditionalCellShadingTests`, including the fixture |
| the heading row counted in the band index | `TheHeadingRowIsNotCountedAsABand` **and** the fixture |
| `w:tblStyleRowBandSize` ignored | the fixture |
| `noHBand` read the right way up instead of inverted | `TheBandBitsAreStatedAsProhibitions` and two others |
| the conditional shading never consulted | the fixture and the band test |
| the blend ignores the transparency | `EachFillFlipsAtItsOwnAlpha` and `TheTransparencyTheWitnessesStateMakesThemBright` |
| the `26214f` suffix read as a plain number | `AFillOpacityBecomesTheFillsAlpha` |
| the opacity never reaches the fill's alpha | `AFillOpacityBecomesTheFillsAlpha` |

Two of these are worth reading rather than counting. The **second** was a *failure* at first —
`TheHeadingRowIsNotCountedAsABand` did not fire, because the fixture's band size of two makes body
row 1 band 0 under both readings; the test now asks the two rows the shift actually moves, and fires
alone. The **sixth and seventh** are the pair that separates "the blend exists" from "the fixed-point
suffix is read", which are two different ways to get `069` wrong and would otherwise both hide behind
one green test.

`verify-test.sh` was run only with **no sweep in flight**. The binary was re-rendered afterwards and
`012`, `069` and `airbus` all came back byte-identical to the final sweep's own renderings — 26 167,
20 060 and 265 501 bytes, 0 differing — so the double rebuild left the shipped state.

## Shared layers

**None.** `git diff 43142b73ccf..HEAD --name-only` over `dotnet/src` is seven files, every one under
`XRay.Office.WordProcessing`. The same command over `XRay.Office.Core`, `Containers`, `Text`, `Vector`,
`Rendering`, `Markup`, `Ooxml`, `OpenDocument`, `Spreadsheets` and `Presentations` prints **nothing**.
Slides and sheets cannot move **by construction**; that is a falsifiable claim for the parent's sweep.

The blend was deliberately **not** put on `XRay.Office.Core`'s `Colour`, where it would have been a
natural home and a shared-layer diff, but a new `AutomaticColour` in the words layout instead.

For the round that extends this: `v:fill/@opacity` is read in exactly one other place in the tree,
`XRay.Office.Spreadsheets/Ooxml/XlsxNoteCaptions.cs`, untouched here. A census of the other two
corpora finds **5 `v:fill/@opacity` attributes in 3 slides documents and none in sheets**, so that
is the whole cross-track reach of this half whenever someone wants it.

## Files

- `prediction.md` — committed at `0a0fc107ae3`, before `7cbff417b86`, with both changes predicted
  per direction and five named blind spots for the first and four for the second.
- `alphaauto.py` — the ten-arm quadruple over three corpus documents, with each arm's prediction
  under both hypotheses written into the table so a non-discriminating arm cannot be added by
  accident.
- `threshold.py` — the eleven-arm bracket over three fill colours, with its own mispredicted first
  cut recorded in the docstring.
- `tblstylepr-census.py` — what the *styles* declare (34 977 layers, which over-reads by two orders
  of magnitude) and the band-`w:rPr` risk.
- `tblstyle-reach.py` — what the *tables* resolve to: 749 cells in 42 documents, with four blind
  spots in the docstring.
- `shapealpha-census.py` — 48 `v:fill/@opacity` in 7 documents, 138 `a:alpha` solid fills in 9, and
  the 49 fills in 7 documents that are dark opaque and bright blended.
- `fillcount.py` — fill and stroke operators by colour, with both of its own tokenising bugs in the
  docstring.
- `filldelta.py` — per-document fill agreement with the reference, never netted.
- `textcolour.py` — round 62's, with the `/Length` fix above.
- `audit_markstyle.py` — the paragraph-mark audit, reference-only, four arms with four controls.
- `make-band-fixture.py` — authors `dotnet/tests/corpus/features/table-style-bands.docx` with a fixed
  timestamp on every entry, so re-running it reproduces the committed package byte for byte.

## What the next round does first

1. **The anchor limb.** A `noFill` text box continues to its *anchor's* background — round 62 proved
   it on four arms and this round shipped the other limb around it. `012`'s title is 76 white glyphs
   against 134 because of it. The work is plumbing, not measurement: `PageDrawing.DrawFrame` draws
   frames from `page.Frames`, and the anchor's cell background has to reach `FrameLayout`. **The
   control already exists and is the one this round used** — round 59's LONG column, which must stay
   at 34.
2. **`012`'s remaining 8 strokes**, which are two different things: one `#7F7F7F` rule per page from
   `firstRow`'s `w:tcBorders/w:bottom` — the *border* half of the element this round implemented the
   shading half of, and the same `ConditionalCellProperties` dictionary already holds it — and seven
   bar outlines from the shapes' `a:ln`.
3. **`069`'s displaced text**, a new lead from a blind reader at high confidence and unchecked by a
   second instrument: `PROJECT NAME` is drawn below and right of the box that should hold it,
   overlapping the row beneath. The document fails the gate on words, 108 against 117.
4. **The tall-table guard**, untouched again, with its two protected documents named and passing:
   `ESPN-R - MCF - RA - Ed1.docx` and `part-147_approval list_20230119.docx`. Writer floats and
   splits those too and its height term is the **page's** print area, not the body's.
5. **`097`'s remaining 1.65 pt**, in the height of a body paragraph holding an inline image —
   untouched three rounds now.
6. Then the `.doc` label slant at `Ww8DocumentReader.Describe` — still 80 of the 81 remaining
   OpenSymbol glyphs — and the Carlito-versus-serif class, `AAC-AD-…-MAX.doc` alone at 46 637.
