# words-r58 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r58`, base
`32f946bf612`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Two predictions, each committed before the
change it covers: `prediction.md` at `9acf69c23ac` before `d08e9aff103`, and `prediction-hyphen.md`
at `b0632378ddc` before `91172e3b99c`.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`. Scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **319 match, 18 open, zero disagreements with the
manifest's status column, document for document.** The residual shear reproduces round 56's to the
glyph: 39 documents / 1 611 short, 5 / 916 long, 148 pages, and the per-face short column
DejaVuSans 1 348 / WenQuanYiZenHei 177 / OpenSymbol 112 / DejaVuSerif 86 / DejaVuSans-Bold 46 /
DejaVuSerif-Bold 15.

Slides and sheets were swept too, because the round's primary change is shared: **slides 200 of
302, sheets 276 of 307**, both the brief's figures.

## 1. The primary: synthetic oblique lost a second time, at glyph fallback

### The rule came out clean, over six filters, with a discriminator that could have refused it

`fallback-oblique.py` and `fallback-oblique-ooxml.py`: **41 authored packages**, each one paragraph
of two runs so the second run's sheared-glyph count is the only quantity that can move, over
`.docx`, `.fodt`, `.fodp`, `.fods`, `.pptx` and `.xlsx`.

**The format is a varied axis, deliberately.** `GenericFallbacks` was recorded `WRONG` by round 53
and `VERIFIED` by round 54 because round 53's probe was DOCX-only — it held the document format
fixed without noticing the format *was* the variable. Here the answer is identical in all six.

| case | ref | ours before | ours after | face |
|---|---:|---:|---:|---|
| `cjk-italic` — italic Arial + CJK | 6 | **0** | 6 | WenQuanYi Zen Hei |
| `sym-italic` | 4 | 0 | 4 | DejaVu Sans |
| `heb-italic-opensym` | 4 | 0 | 4 | DejaVu Sans |
| `heb-italic-carlito` | 4 | 0 | 4 | DejaVu Sans |
| `cjk-italic-none` — the primary is *itself* synthetic | 6 | 0 | 6 | WenQuanYi Zen Hei |
| `cjk-bold-italic` | 6 | 0 | 6 | WenQuanYi Zen Hei |
| `cjk-italic-eastasia` (docx) | 6 | 0 | 6 | WenQuanYi Zen Hei |
| `latin-italic`, `cjk-upright`, `heb-upright-*` — **four controls** | 0 | 0 | 0 | — |
| `latin-italic-none` — round 56's fix, as a regression control | 12 | 12 | 12 | DejaVu Serif / Sans |

**The discriminator is the part that could have stopped this shipping.** Hebrew from an italic
`Carlito` run is covered by DejaVu Sans, which has no italic here, *and* by Liberation Sans, which
does. If 26.2.4.2 preferred the italic-bearing family it would draw the run upright in
`LiberationSans-Italic`, and setting `SyntheticOblique` on a fallback reference would then be wrong.
It draws **DejaVu Sans, sheared** — its fallback order wins over the slant, exactly as our own
`FallbackFor` already ranks family above slant. So the face was already right and only the lean was
missing.

A second thing fell out of it: **`w:iCs` is the other half of round 56's finding.** The Hebrew
`.docx` arm read 0 on both sides until `w:iCs` was stated beside `w:i`, because OOXML files Hebrew
under the complex-script slot. Round 56 established that `w:iCs` does not lean *Latin* text; this is
the same rule from the other side, and it makes all six filters agree.

### The change, and it is `XRay.Office.Text`

`IGlyphFallbackResolver` gains a **default interface method**
`ReferenceFor(OpenTypeFace, bool isItalicRequested)` — the one-argument reverse lookup plus
`SyntheticOblique = isItalicRequested && !face.IsItalic`. Neither implementer changes. Three call
sites pass the request: `PageDrawing.ByFace` (words), `SlideTextLayout.Block.FontFor` (slides),
`SheetFonts.ForFallback` (sheets), whose cache key gains the slant because one substituted face
answers both an upright cell and an italic one.

The request is **two states, not one** — the primary face is italic, *or* the primary run was itself
being sheared. `cjk-italic` and `cjk-italic-none` differ by exactly that and the reference shears
both.

### Prediction against measurement — and the census was wrong in a way worth publishing

| quantity | baseline | predicted | measured |
|---|---:|---|---:|
| words: sheared glyphs, ours (ref 154 501) | 153 806 | 154 000 – 154 700 | **153 848** |
| words: documents the reference shears more of | 39 | 28 – 37 | **38** |
| words: glyphs in that direction | 1 611 | 900 – 1 350 | **1 569** |
| words: pages the reference shears and we draw none | 148 | 132 – 145 | **142** |
| words: documents **we** shear more of | 5 | 5 – 8 | **5** (916 glyphs, unchanged) |
| words: our leans in pure-fallback faces | 0 | 190 – 210 | **42** |
| slides: our leans in pure-fallback faces | 4 | 340 – 350 | **4** |
| sheets: our leans in pure-fallback faces | 0 | 4 | **4** ✓ |
| **words verdict** | 319 of 337 | 0 movement | **319, 0 movement** ✓ |
| **slides verdict** | 200 of 302 | 0 movement | **200, 0 movement** ✓ |
| **sheets verdict** | 276 of 307 | 0 movement | **276, 0 movement** ✓ |
| page counts changed, any track | — | 0 | **0** ✓ |
| extractable words changed, any track | — | 0 | **0** ✓ |
| font lists changed, any track | — | 0 | **0** ✓ |
| words renderings whose bytes change | — | 14 – 40 | **2** |
| slides renderings whose bytes change | — | 1 – 20 | **0** |
| sheets renderings whose bytes change | — | 1 – 3 | **1** ✓ |

Verdicts are per document, not netted: **zero gains and zero regressions on all three tracks**, over
337 + 302 + 307 paths, every one compared by name against its own baseline row.

**The three big misses have one cause, and it is the census, not the fix.** `fallbackfaces.py`
summed WenQuanYi Zen Hei, OpenSymbol and IPA Gothic together per document. Per *face* the picture is
different and the second version of the probe prints it:

| track | reference leans | **reachable** (we draw the same face, upright) | **unreachable** (we do not draw that face at all) |
|---|---:|---:|---:|
| words | 289 | **206** in 12 document/face pairs | **83** in 2 |
| slides | 345 | **0** | **341** in 1 |
| sheets | 4 | **4** in 1 | 0 |

`outlook_of_nigerian_pension_sector.ppt` — the single largest item in the whole census, 341 glyphs —
draws **355 WenQuanYi Zen Hei glyphs on the reference and none at all on ours**; we use DejaVu Sans
Bold there. That is a fallback-*order* divergence and no slant fix can reach it. The summing census
read it as a lean we had failed to draw, and predicted 345 glyphs of movement where the true answer
was **nought**. Blind spot 2 of the prediction named exactly this shape — "the two words documents
where we draw *none* of the fallback face at all" — and I applied it to words and did not check
slides.

### Of the 206 reachable words glyphs, 42 moved. The other 164 are a different seat.

`手机免提系统TSB.doc` 0 → 30 of 82, `P200904290238_0238_51880.doc` 0 → 12 of 12. Those are the two
documents whose bytes changed.

**The 164 that did not move are, to the glyph, the OpenSymbol column** — 112 across ten documents,
plus 52 of `手机免提系统TSB.doc`'s WenQuanYi Zen Hei. The OpenSymbol glyphs are single `<01>` draws
one per line at x ≈ 104.5: **list bullets**, which reach the page through `PageDrawing`'s label
branch and never through `ByFace`. `A320SimNotes.doc` is the largest at 75, and its body text leans
on **neither** side — so it is the list level's own character formatting that is italic, not the
paragraph's.

Pinned, so the next round need not: `label-and-autocolour.py`, five authored packages with a control
that is nought on both sides.

| what states `w:i` | reference leans the bullet | ours |
|---|---:|---:|
| nothing — the control | 0 | 0 |
| the level's own `w:rPr` | **1** | 0 |
| the paragraph mark's `w:rPr` | **1** | 0 |
| the run's `w:rPr` | **0** | 0 |
| all three | **1** | 0 |

**The level's slant reaches the bullet, the paragraph mark's reaches it, a run's does not.** We lean
it in none of the three. `Ww8DocumentReader.Layout` carries the level's `HalfPointSize`, `FontIndex`
and `IsBullet` and no slant at all, so this is a reader change in all four readers, not a layout one.

## 2. `AFS-050-004-F2_0i` page 2 — the answer is neither of the two the record offered

The brief asked to separate *present-but-unpainted* from *never-read*. It is **neither**: the text is
present, painted, and **black on black**.

- Both strings are in our text layer at the reference's own positions —
  `0.000 General Information and Air Operator Complexity` and `CE-1 Primary Aviation Legislation`,
  and the CE-2, CE-3 and CE-4 banners with them.
- We draw all **five** black banner rectangles, at the same x, width and height to a twip.
- The reference draws **305 glyphs `1 1 1 rg`** on that page. **We draw none** — no `1 1 1` appears
  anywhere in our page-2 content stream. Ours: 2 332 black + 178 blue. Reference: 2 133 black +
  34 blue + 305 white.
- The cell is `<w:shd w:val="clear" w:color="auto" w:fill="000000"/>` and the run states **no
  colour**, so the colour is `COL_AUTO` and `SwTextPaintInfo`'s automatic-colour resolution
  (`fntcache.cxx`:2369) answers it against the frame's background brush.

**The rule is pinned exactly**, 22 authored fills, and it needed no fitting:

| fill | reference | | fill | reference |
|---|---|---|---|---|
| `000000` … `909090` | white | | `FF0000` | white |
| `9C9C9C`, `9D9D9D`, **`9E9E9E`** | white | | `00FF00` | black |
| **`9F9F9F`**, `A0A0A0`, `C0C0C0`, `FFFFFF` | black | | `0000FF`, `008000`, `000080` | white |
| | | | `FFFF00`, `00FFFF` | black |

Every one of the 22 is predicted by `Color::IsDark()` = `GetWCAGLuminance() <= 87`, and the boundary
holds **to the single sRGB step**: grey 158 has WCAG luminance 87.2 → 87 → white; grey 159 has 88.4
→ 88 → black. A first cut of the prediction put the step at `0x9D`/`0x9E` from a hand calculation and
the measurement corrected it by one, which is what the two extra fills were added for.

**Not implemented, and the reason is structural rather than doubt**: the rule needs *the background
behind a run* at the drawing pass, and the cell fill is drawn by the table renderer where
`PageDrawing.RunsIn` cannot see it. An unstated colour is already distinguishable — `PageRun.Colour`
is transparent and `EffectiveColour` turns it black — so the missing half is the background, not the
"automatic" state.

Beside it, on the same page: the reference fills **8** rectangles and we fill **5**. The three we
miss are the shaded header row's cells (`Checklist Sections` / `DESCRIPTION` / `Responsible Team
Member`), 25.75 pt tall against the banners' 11.55.

## 3. The `FORMCHECKBOX` census is now closed: 778 in 16, not "675 in 12, a floor"

`doc-checkbox-census.py`, over the manifest's 66 `.doc` documents, with **two independent
instruments that agree to the digit**:

| document | `FORMCHECKBOX` in the bytes | `w:checkBox` after LibreOffice's own reader |
|---|---:|---:|
| `f111.doc` | 58 | 58 |
| `1528364855.doc` | 37 | 37 |
| `foca_form_1.doc` | 3 + 1 (8-bit and UTF-16 pieces) | 4 |
| `LHD-230-application-for-the-approval-of-an-aircraftr.doc` | 1 + 3 | 4 |
| **total** | **103 in 4** | **103 in 4** |

**The `.rtf` arm needs no probe at all.** `MANIFEST.tsv`'s words family is **271 `.docx` and 66
`.doc`** and nothing else, so `\*\formfield` has zero witnesses in this corpus. Round 56's floor is
therefore exact: **675 + 103 = 778 boxes in 16 documents**, and the `.doc` arm is the only one left
to implement.

## 4. The 24.2.7.2 audit — a shared-layer site, taken and found wrong

`XRay.Office.Text/Layout/LineBreaker.cs`:473 said LibreOffice never lets a hyphen open a number, that
"dropping HY here is the whole of the rule", and gave five worked examples. **Three of the five are
false on 26.2.4.2, the code implemented the false version, and a test asserted it.**

`audit_hyphenbreak.py`: ten authored packages, **no width tuning** — each token is longer than its
line in a six-character column, so *where* it breaks separates the two rules outright — and two
controls with known answers.

| token | reference | ours before | after |
|---|---|---|---|
| `abcd-efghijklmnop` — **CONTROL** | `abcd-` | `abcd-` | `abcd-` |
| `(222222222222222` — **CONTROL** | `(22222` | `(22222` | `(22222` |
| `E-222222222222` | `E-2222` | **`E-`** | `E-2222` |
| `$-222222222222` | `$-2222` | **`$-`** | `$-2222` |
| `abc-222222222222` | `abc-22` | **`abc-`** | `abc-22` |
| `-2222222222222` | `-22222` | **`-`** | `-22222` |
| `A -222222222222` | `A` \| `-22222` | **`A-`** | `A` \| `-22222` |
| `10-1922222222222` | `10-` | `10-` | `10-` |
| `5-2222222222222` | `5-` | `5-` | `5-` |
| `222-abcdefghijkl` | `222-` | `222-` | `222-` |

**7 of 10 agreed before, 10 of 10 after, character for character.** One rule covers all ten: **a
hyphen opens a number unless a digit precedes it.** That exception is LibreOffice's own i#83229
number-range customisation — which the old comment named and then called the instance rather than the
exception.

Counts re-derived with the file's own commands rather than quoted: at the base commit **39 open,
17 VERIFIED / 2 FIXED / 1 WRONG / 0 UNDECIDED**, which reproduces the round-56/57 merge note exactly.
At this tree **38 open** (`XRay.Office.Text` 6 → 5) and **17 / 3 / 1 / 0**. The open count falls here,
against the file's usual convention, because the site's prose was not merely stale — its worked
examples were wrong, so there was nothing to preserve as a record of what the code was fitted to.

### What the line-breaking change did to the corpus

| quantity | predicted | measured |
|---|---|---|
| the ten probe cases agreeing with the reference | 10 of 10 | **10 of 10** ✓ |
| words renderings whose bytes change | 15 – 30 | **15** ✓ |
| **words verdict** | 319, downside risk −1 to −3 | **319, zero movement, zero regressions** ✓ |
| words page counts changed | — | **0** |
| words font lists changed | — | **0** |
| words extractable-word counts changed | — | **4, every one toward the reference** |

`review-welsh-government-communications-mister-peter-mandelson.docx` **3015 → 3017 against the
reference's 3017 — exact**; `xx_SETIS_PWS_template_10.19.22.docx` 4920 → 4922 of 4923;
`absrc-pac-01-info-note-en.doc` 1301 → 1302 of 1303; `report-template.docx` 3012 → 3013 of 3026.
Every movement is in the right direction, which is what a change that stops splitting tokens the
reference does not split should produce.

**The census's sheets figure was noise and the prediction said so in advance.** `hyphcensus.py`
counted 2 071 candidate line ends on sheets against 109 on words, and the prediction wrote down,
before the measurement, that `pdftotext -layout` reconstructs a spreadsheet row as one line so
"a line ending in a hyphen whose next line starts with a digit" is satisfied by two unrelated cells.

| track | renderings changed | predicted | verdicts | pages | fonts | extractable words |
|---|---:|---|---|---:|---:|---|
| words | **15** | 15 – 30 ✓ | 319 → **319** ✓ | 0 | 0 | 4 rows, all toward the reference |
| slides | **11** | 8 – 20 ✓ | 200 → **200** ✓ | 0 | 0 | 2 rows, both toward the reference |
| sheets | **22** | 3 – 40 ✓ | 276 → **276** ✓ | 0 | 0 | 13 rows, see below |

Slides: `429_BLISc 2019 IInd Sem. PAPER-7 UNIT 3.pptx` **3126 → 3125, the reference's figure
exactly**; `Curriculum-Schematic-E22-11052025.pptx` 1119 → 1131 of 1134.

Sheets: of the thirteen, **three are the reference moving, not us** — `PBN Matrix NAAs (V01)`,
`ans_mappings_of_eccairs_terms` and `SIL_TDB648` all hold our count fixed while the reference's
changes by one between renderings. That is the date-volatility trap the round-57 sheets work named,
arriving again, and it is why the columns are read as a pair. Of the remaining ten, **eight move
toward the reference and two away**:

| document | ours before → after | reference | |
|---|---|---:|---|
| `STC_WebList.xlsx` | 1 286 028 → **1 284 972** | 1 284 926 | error 1 102 → **46** |
| `MinCh-Digital-Certificate-Publication-Report.xlsx` | 112 059 → **111 971** | 111 969 | 90 → **2** |
| `fm-provider-service-measures.xlsx` | 21 358 → **21 352** | 21 348 | 10 → **4** |
| `AMOC-Digital-Certificate-Publication-Report.xlsx` | 36 609 → **36 605** | 36 603 | 6 → **2** |
| `State-Medicaid-…-Cost-Sharing.xlsx` | 40 276 → **40 280** | 40 411 | 135 → **131** |
| `commander-authorisation-tasks-…-2025.xlsx` | 13 077 → **13 078** | 13 078 | 1 → **0** |
| `afn-afn-20250801-fy25-jan25-mar25.xlsx` | 72 796 → **72 797** | 72 843 | 47 → **46** |
| `SLSA_Directory_031423.xlsx` | 5 769 → 5 768 | 5 786 | 17 → **18**, worse by one |
| `MajCh-Digital-Certificate-Publication-Report.xlsx` | 66 902 → 66 906 | 66 901 | 1 → **5**, worse by four |

**This is a shared-layer change and the other two tracks were measured, not reasoned about.** It is
one commit — `91172e3b99c` — and separable from the round's other change if the parent wants it out.

## Refutations

1. **`LineBreaker.cs`'s "a hyphen never opens a number" is false on 26.2.4.2, in three of its own
   five worked examples**, and the code and its test implemented the false version. The true rule is
   the reverse with one exception: a hyphen opens a number *unless a digit precedes it*.
2. **The list label's slant is not the run's.** The level's own `w:rPr` leans the bullet and so does
   the paragraph mark's; a run's does not. Five authored packages with a control.
3. **`AFS-050-004-F2_0i` page 2 is not "text present but unpainted" and not "text never read".** It
   is painted, in the wrong colour, black on black. 305 white glyphs on the reference against our
   nought, with our five banner rectangles matching the reference's five to a twip.
4. **`Color::IsDark()`'s WCAG threshold survives the binary change**, confirmed to the single sRGB
   step over 22 fills — a claim I could have assumed from the 27.2 tree in this checkout and did not,
   because that tree is not the reference.
5. **"675 in 12" is no longer a floor: the corpus total is 778 in 16.** The `.rtf` arm has zero
   witnesses because the words corpus holds no `.rtf` at all, which is a census a probe was not
   needed for and which two earlier rounds nevertheless deferred.
6. **My own census was wrong and its shape is the finding.** Summing the pure-fallback faces per
   document instead of matching them per face turned a face-*selection* divergence into an apparent
   lean defect, and produced a confident prediction of 340–350 glyphs of slides movement where the
   answer was nought. The probe now prints REACHABLE and UNREACHABLE separately and says in its
   docstring why.
7. **`batch-check.sh`'s own TOTAL is not a scoreboard, and this round nearly read one.** The slides
   sweep enumerated **311** rows before the change and **315** after; the sheets sweep **325** and
   **363**. Read naively that is "+4 slides, +10 sheets". Every extra row is an upper-case alias
   entry of a document already counted — which aliases a shell glob enumerates is not stable between
   runs on this mount. Scored against `MANIFEST.tsv`'s own path list, every one of the six sweeps
   gives 337 / 302 / 307 paths with exactly one row each, and the scores are 319 / 200 / 276 before
   and after both changes.

## Tests

```
Core 337   Containers 109   Text 624   Vector 295   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1188   Spreadsheets 967   Presentations 824     = 4881
0 failed, 1 skipped
```

**4855 → 4881, delta +26**, re-derived rather than quoted: Text 617 → 624 (+7, one of them replacing
a test that asserted the inverted hyphen rule and so is a net +7 on a file that also lost an
assertion), WordProcessing 1180 → 1188 (+8), Spreadsheets 961 → 967 (+6), Presentations 819 → 824
(+5). `dotnet build -v q -nologo`: **0 warnings, 0 errors.**

**Two of the new tests failed on their first run and both failures were the test being wrong in a
way worth recording.** `TheLeanMovesNoAdvance` compared whole paragraphs whose Latin runs resolve to
`LiberationSerif-Italic` in one arm and `LiberationSerif-Regular` in the other — genuinely different
advances, and the assertion said so. `ASubstituteThatIsItselfItalicIsNotLeanedTwice` asked the system
resolver for a reference to a face loaded through `LoadOpenType`, which fills `_loaded` and **not**
`_keys`, so the reverse lookup answered null; and no family installed here has both an italic and a
character LibreOffice's fallback list reaches later than Liberation Sans, so that arm is asked
through a stub — with a positive case beside it, so the stub cannot pass by answering false to
everything.

Run through `verify-test.sh`, tree clean before each and restored after — **seven mutations, seven
detected, across all four projects**:

| mutation | detected by |
|---|---|
| the default method never leans | 2 Text, 3 WordProcessing, 1 Presentations, 3 Spreadsheets |
| the default method leans *whatever* the substitute is (drops `!face.IsItalic`) | `ASubstituteThatIsItselfItalicIsNotLeanedTwice` |
| the **words** call site passes `false` | 3 |
| the **words** call site reads the face only, dropping the synthetic arm | `AFallbackFaceInheritsALeanTheRunOnlyHasSynthetically` |
| the **slides** call site passes `false` | `AFallbackFaceInAnItalicRunIsDrawnLeaning` |
| the **sheets** call site passes `false` | 3 |
| the sheets cache keyed on the face alone | `AFallbackFaceInAnUprightCellIsNotDrawnLeaning`, `TheTwoAnswersDoNotOverwriteEachOtherInTheCache` |

The hyphen rule needs no separate mutation run: `AHyphenOpensANumberUnlessADigitPrecedesIt`
**replaced a test that asserted the opposite**, so the old code fails the new test by construction —
which is the strongest form of detection this harness can produce.

## Files

- `prediction.md`, `prediction-hyphen.md` — each committed before the change it covers.
- `fallback-oblique.py` — 25 packages over four filters, four controls, the Carlito discriminator.
- `fallback-oblique-ooxml.py` — the same claim in `.pptx` and `.xlsx`, the formats the other two
  corpora are actually made of.
- `fallbackfaces.py` — the per-face REACHABLE / UNREACHABLE census, with the reason for the split in
  its docstring because the first version did not have it.
- `facegap.py` — per-face short/long by document, all three tracks.
- `score.py` — scores a `rows.tsv` against `MANIFEST.tsv`'s own path list, for any family, and
  refuses to print unless every manifest path found exactly one row.
- `label-and-autocolour.py` — the list label's slant and the automatic font colour, 23 packages.
- `doc-checkbox-census.py` — the `.doc` `FORMCHECKBOX` census, two independent instruments.
- `audit_hyphenbreak.py` — ten packages, two controls, no width tuning.
- `hyphcensus.py` — the reach census, whose sheets column is noise and says so.

## What the next round does first

1. **The list label's slant.** 164 of the 206 reachable glyphs, the whole OpenSymbol column, ten
   documents. The rule is pinned: the level's `w:rPr` and the paragraph mark's `w:rPr` lean the
   bullet, a run's does not. `Ww8DocumentReader.Layout` carries the level's size, font index and
   bullet flag and no slant, so it is a change in all four readers.
2. **The automatic font colour on a dark background.** `AFS-050-004-F2_0i.docx` alone is 305 glyphs
   drawn black on black. The rule is exact — white when `GetWCAGLuminance() <= 87`. The work is
   getting *the background behind a run* to the drawing pass, which the table renderer owns.
3. **`outlook_of_nigerian_pension_sector.ppt`, and the fallback *order*.** 355 WenQuanYi Zen Hei
   glyphs on the reference and none on ours, plus `1228841571067…doc` 74 and `1257259179492…doc` 9
   on words. This is `FallbackFor`'s ordering, not the lean.
4. **`2024-12_Comlux…docx`** — `LiberationSans-Italic` against our `DejaVuSans` on 652 glyphs, and
   the same class shows in this round's own probe: `sym-italic` draws ☒ and ➢ from **OpenSymbol**
   where the reference draws all four symbols from **DejaVu Sans**, though LibreOffice's own
   fallback list has `opensymbol` *before* `dejavusans`. That is a second, smaller witness for the
   same question and it is authored rather than found.
5. The `ascii` slot fallback (four documents); `097`'s 1.7 pt line-height deficit, untouched for six
   rounds now; and the `.doc` arm of the form checkbox, whose census is now exact at 103 in 4.
