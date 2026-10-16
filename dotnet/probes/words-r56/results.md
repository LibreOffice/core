# words-r56 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r56`, base
`e64f743dbff`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Two predictions, each committed before the
change it covers: `prediction.md` at `3c9619f3bd1` before `224354e5403`, and
`prediction-checkbox.md` at `c7325332de9` before `a7a64900a7f`.

## Baseline, reproduced exactly

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 336 MISMATCH 19`. Scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **319 match, 18 open, zero disagreements with the
manifest's status column, document for document.** The brief's figure exactly.

## One defect, both directions, and the brief expected only one of them

**Synthetic oblique did not survive the uniform-paragraph shortcut.** All four word-processing
readers build a paragraph's `PageRun` list only when its formatting *varies*, and each of the four
sites writes the list of properties that count out longhand — face, size, colour, language,
escapement, case map, highlight, underline, strike-through, kerning, tracking. Each of the last four
carries a sentence saying why it is there. Slant was not on the list.

For nearly every family it did not need to be: an italic run of `Arial` resolves to
`LiberationSans-Italic`, a **different `OpenTypeFace`**, so `face != paragraphFace` already fires.
The families with **no italic installed at all** are exactly the fallback faces — DejaVu Sans and
DejaVu Serif ship Book and Bold and nothing else here — so an italic run that falls back resolves to
the *same* face as its upright neighbour, passes every other test, and is drawn upright.

**And the same fold, read the other way, drew whole paragraphs of upright prose leaning**, because a
paragraph mark that is italic donates its own font to every run folded into it. That is the
over-shear the brief posed as a separate problem. It is the same line of code.

### Seated by a discriminating pair, not by argument

`oblique-uniform.py`, ten authored packages of **one paragraph and two runs**, so the second run's
sheared-glyph count is the only quantity that can move.

| case | reference leans | we leaned | after |
|---|---:|---:|---:|
| `nonesuch/i` — run 2 states `w:i` and nothing else | 23 | **0** | 22 |
| `nonesuch/i+sz` — the same run plus a `w:sz` the predicate already tests | 23 | **22** | 22 |
| `nonesuch-swiss/i` | 23 | 0 | 22 |
| `nonesuch/para-i` | 23 | 0 | 22 |
| `nonesuch/style-i` — the whole paragraph italic, no run differs | 47 | 46 | 46 |
| `arial/i`, `courier/i`, `nonesuch/sz-only`, `nonesuch/b`, `nonesuch/iCs` | 0 | 0 | 0 |

The only difference between the first two rows is a property already on the `varies` list. **No
hypothesis about how we read `w:i` predicts that; the shortcut does.** Five negative controls are
nought on both sides before and after, so the predicate did not become "always vary".

The worked corpus case is `review-welsh-government-communications-mister-peter-mandelson.docx`:
both sides embed the **identical six faces**, both set its page-5 table body in DejaVu Sans, the
reference leans 105 of those glyphs and we leaned none. The runs are
`<w:rPr><w:rFonts w:eastAsia="Aptos"/><w:i/></w:rPr>` inside paragraphs whose other runs differ by
nothing else. **Same face lists, per-run divergence** — the shape the slides round named.

### The change

Four sites, one clause each, plus a shared predicate on `PageRun` carrying the reasoning:
`DocxLayoutSource.RunsOf`, `OdtLayoutSource.RunsOf`, `RtfReader.RunsOf`, `DocReader.RunsOf`.
**The whole round's diff is confined to `XRay.Office.WordProcessing`.** No shared layer is touched;
slides and sheets cannot be reached and no cross-track sweep is owed.

## Prediction against measurement — most columns missed, and the miss is the finding

| quantity | instrument | baseline | predicted | measured |
|---|---|---:|---|---:|
| sheared glyphs, ours (reference 154 501) | `shear-chars.py` | 158 673 | 164 000 – 166 500 | **153 806** |
| documents the reference shears more of | `shear-split.py` | 38 | 6 – 14 | **39** |
| glyphs in that direction | `shear-split.py` | 6 819 | 300 – 1 400 | **1 611** |
| documents where we shear **none** and it shears some | `shear-split.py` | 15 | 0 – 3 | **11** |
| pages where the reference shears and we draw none | `shear-split.py` | 162 | 0 – 25 | **148** |
| documents **we** shear more of | `shear-split.py` | 8 | 8 – 12 | **5** |
| glyphs in *that* direction | `shear-split.py` | 10 991 | 10 991 – 11 600 | **916** |
| pages that agree outright | `shear-split.py` | 4 382 | — | **4 394** |
| **verdict movement** | `batch-check.sh` vs `MANIFEST.tsv` | 319 of 337 | **0** | **0** ✓ |
| page counts changed | `parity.tsv` | — | **0** | **0** ✓ |
| font-list disagreements | `pdffonts` | 52 | — | **52** |

**The prediction was wrong about seven columns and right about the two its reasoning staked on**, and
it was wrong in the direction it had explicitly written down as the thing it could not see. Blind
spot 2 said: *"it cannot see the over-shear at all, and cannot rule out that this fix makes it
worse."* It was exactly backwards — the fix **cured** the over-shear, because over-shear and
under-shear are the same fold. Predicting a number for a defect while modelling only half of it
produces a confident band that misses on both edges.

| document | ours before | ours after | reference |
|---|---:|---:|---:|
| `644730BRI0mna000BOX361539B00public0.doc` | 6 643 | **2 124** | 2 171 |
| `SPA-02_mcar_part-2_and_IS_v2.9.docx` | 58 011 | **54 175** | 54 694 |
| `02_mcar_part-2_and_IS_v2.10.docx` | 49 900 | **46 430** | 46 856 |
| `EHEST-SMS-Safety-Management-Manual-V2.docx` | 8 470 | **13 629** | 13 473 |
| `review-welsh-…-mandelson.docx` | 0 | **105** | 105 |

## What is left of the shear gap, and it is a different seat

**1 611 glyphs short on 39 documents, 916 long on 5**, from 6 819 and 10 991. Summed per face over
the whole track, the residual short side is **DejaVu Sans 1 348, WenQuanYi Zen Hei 177, OpenSymbol
112, DejaVu Serif 86, DejaVu Sans Bold 46, DejaVu Serif Bold 15**.

The two middle rows name the next seat outright. `手机免提系统TSB.doc` leans 82 glyphs of **WenQuanYi
Zen Hei** on the reference and none on ours; `A320SimNotes.doc` 75 of **OpenSymbol**. No document
declares either family — they are reached by **glyph fallback**, at draw time, through
`FontItemiser.Split`. And the reference that names that face is built by
`SystemFontResolver.ReferenceFor`, whose own remark says what is wrong with it: it is *"a reverse
lookup from a face"* with **no request to compare against**, so it cannot set `SyntheticOblique` and
never has. An italic run whose glyph comes from a fallback face loses its lean at a second, separate
place.

That fix is in `XRay.Office.Text` and owes a measured cross-track sweep.

`2024-12_Comlux_opens_Maintenance_and_Service_Center_in_Dubai.docx` is the one document whose lean
got further from the reference (18 → 652 against 0), and it is **not a shear defect**: the reference
draws that text in `LiberationSans-Italic` and we draw it in `DejaVuSans`, a pre-existing font-list
disagreement. Our 652 leaning glyphs are the right *kind* in the wrong face. This is precisely the
class the slides round predicted — a font-resolution divergence that was invisible while both stacks
drew upright — and it is now visible.

## The legacy `FORMCHECKBOX`: the standing record's premise is false

`HANDOVER.md` §8 has said since round 38 that these are *"established, deliberately not implemented:
the drawn square's size would not pin (9.0…15.9 pt, not following `w:checkBox/w:size`)"*.

**It pins exactly.** `SwFieldFormCheckboxPortion::Format` (`portxt.cxx`:1492) sets the portion's
width and height to `rInf.GetTextHeight()` and its ascent to `rInf.GetAscent()`;
`SwTextPaintInfo::DrawCheckBox` (`inftxt.cxx`:1247) strokes that rectangle deflated by a hard
`delta = 25` twips a side, black, unfilled, crossed corner to corner when ticked. So the square
follows the **line's text height**, and **9.0…15.9 pt was a range of font sizes read as a failure to
pin.**

`formcheckbox.py`, nineteen authored packages, **duplicate-input control first and it agreed to the
digit**:

| | text height | drawn square | difference |
|---|---:|---:|---:|
| Liberation Serif 8 pt | 184 tw | 134 tw (6.700 pt) | 50 tw |
| 12 pt | 276 | 226 (11.300) | 50 |
| 24 pt | 552 | 502 (25.100) | 50 |
| 40 pt | 920 | 870 (43.500) | 50 |
| Liberation Mono 12 pt | 272 | 222 (11.100) | 50 |
| DejaVu Sans 12 pt | 280 | 230 (11.500) | 50 |
| Carlito 12 pt | 293 | 243 (12.150) | 50 |

Four fixtures stating `w:size` of 5, 10, 20 and 40 pt all draw the run's own 11.300. **`w:size` is
inert**, and 109 of the corpus's 675 boxes state one.

**The census is larger than the record too.** `checkbox-census.py` over all 337 manifest paths,
reading every `.xml` part of every package rather than `document.xml` alone: **675 boxes in 12
documents**, all `.docx`, against the record's "249 across 16". 566 state `w:sizeAuto`, 10 are ticked.

### Implemented, and the width is the half that matters

675 positions were reserving **nothing** where the reference reserves a square of the line's whole
text height, so every line holding one was laid out narrower than the reference lays it out.
`PageFrame` gains `BorderInset` — the outer square is what the line is charged and the inner one is
what the page shows, and the two cannot be folded into one number — and `IsCrossed` for the ten
ticked boxes.

| | predicted | measured |
|---|---|---|
| renderings whose bytes change | **12, exactly the census** | **12, exactly the census** ✓ |
| documents outside the 12 that move | 0 | **0** ✓ |
| **verdict movement** | 0 | **0** ✓ |
| downside risk | −1 to −4 | **0** ✓ |
| page counts changed | 0 to 3 | **0** ✓ |
| extractable words changed | 0 | **0** ✓ |
| font lists changed | 0 | **0** ✓ |

Against the reference on the three densest documents, counting small stroked squares:
`FO.FCTOA.00010` **249 to 249**, `Form-SM-76A` **152 to 152**, `te.iors.00048-002` **48 to 48**,
with side lengths identical to **0.000 pt** and x positions within 0.05 pt.

## The vision round — three pages, three fresh readers, none of whom read anything else

Each page was chosen for a stated reason rather than by `--worst`, and each reviewer was forbidden
from reading documentation, source or running any command, and asked to describe the halves
separately before comparing.

1. **`644730BRI0mna000BOX361539B00public0.doc` page 2**, chosen because it was the round's largest
   *over*-shear. The reviewer reported an entire lead paragraph set slanted on our side and upright
   on the reference's, in a two-column newsletter. That is the reading that turned the round's
   hypothesis from "we lose leans" into "the fold loses the disagreement in both directions", and it
   arrived before the mechanism was understood.
2. **`AFS-050-004-F2_0i` page 2**, the brief's item 3. Confirmed: five black banner rows carrying
   white reversed-out text on the reference and nothing on ours. The reviewer adds a fact the earlier
   record has backwards — **our rows are the *looser* ones**; the reference packs its rows tighter
   and so runs out of page a row earlier, where ours fits the final row and both footer lines. It
   also separates two causes this round did not settle: text present but unpainted, versus text
   never extracted.
3. **`FO.FCTOA.00010` page 3**, chosen for the checkbox item. Nine squares on the reference, none on
   ours, each *"roughly the same height as the cap-height of the adjacent bold label text"* — an
   independent reading of the size rule this round then pinned to the twip, from a reader who had
   seen no part of it.

## Refutations

1. **"The words track over-shears by 4 172 glyphs" and "162 pages still have none" are one defect,
   not two.** The brief posed the over-shear as a font-resolution divergence and the under-shear as
   its likely companion. Both are the uniform-paragraph fold, and fixing it moved the over-shear from
   10 991 glyphs to 916 and the under-shear from 6 819 to 1 611 in one change.
2. **"The FORMCHECKBOX square's size would not pin (9.0…15.9 pt)" is false.** It is
   `rInf.GetTextHeight()` less 50 twips at every size and on every face, measured on seven sizes and
   five faces with a control. The range was font sizes.
3. **"249 fields in 16 documents" is not the census.** 675 in 12, all `.docx`, counted over every
   part of every package.
4. **`w:checkBox/w:size` does nothing**, on four stated values from 5 to 40 pt.
5. **`w:iCs` does not lean Latin text on 26.2.4.2** — nought sheared glyphs on both sides. That was
   a live candidate for the over-shear and is now closed.
6. **`WriterPoolSpacing`'s lower-case `body text` row is wrong**, and 27 of the other 28 names are
   right. See the audit below.
7. **My own probe's first big result was an artefact and is recorded as one.** `audit_poolspacing.py`
   reported **nine** rows of the pool table wrong. It named the two case variants of a heading
   `heading-5` and `Heading-5`, which are **one file** on this mount, so 28 of 58 conversions were
   silently missing — and a missing file reads as nought, which reads as a finding. Caught by the
   rule that says to run a census on a case whose answer is already known: the *shape* of the
   failures (`heading 5` and `heading 8` wrong, `heading 4` and `heading 6` right) is not a rule any
   binary could implement. The probe now numbers its packages and **refuses to print anything unless
   every conversion produced output**.

## The 24.2.7.2 audit

`XRay.Office.WordProcessing/Ooxml/WriterPoolSpacing.cs` — **VERIFIED 2026-08-21, round words-r56**,
with one row corrected. The site says outright what it is (*"Each row is one rendered probe against
LibreOffice 24.2.7.2"*) and already carried a later round's warning about one row, left standing
because correcting one row belongs to whichever round re-measures the table. This is that round.

Re-measured whole: 28 names, **both halves of every row**, a custom child based on a parent carrying
the built-in name and declared after it, stating 480 twips — a value in no pool row, so "mirror the
stated value" is refuted by every case rather than assumed away. Two controls ran first and both
answered correctly. **27 of 28 agree**, including the three that claim nothing (`Quote`, `Normal`,
`List Paragraph`). Lower-case `body text` answers nought on both sides where the table claims 0/140,
and is removed: **zero corpus documents name a parent that way and 80 name `Body Text`.**

Counts re-derived with the file's own commands, not quoted: **40 open sites in 21 files; markers 16 →
17 (14 verified, 3 wrong, 0 undecided).** The open count is unchanged, which is the file's stated
intent — the sentence naming the superseded binary stays.

## Tests

```
Core 337   Containers 109   Text 617   Vector 295   Rendering 153(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1180   Spreadsheets 940   Presentations 819     = 4834
0 failed, 1 skipped
```

**4809 → 4834, delta +25**, all in `WordProcessing`: 10 in a new `SyntheticObliqueRunTests.cs` and
15 in a new `FormCheckBoxTests.cs`. Re-derived rather than quoted: every other project's count is
unchanged and 1180 − 1155 = 25. `dotnet build -v q -nologo`: **0 warnings, 0 errors.**

Run through `verify-test.sh`, tree clean before each and restored after — **seven mutations, seven
detected**:

| mutation | detected by |
|---|---|
| `LeansDifferently` answers false | 5 of the 10 oblique tests |
| the clause removed from **`DocxLayoutSource`** alone | 3 |
| from **`RtfReader`** alone | `TheRtfReaderKeepsALeaningRunToo` |
| from **`OdtLayoutSource`** alone | `TheOdfReaderKeepsALeaningRunToo` |
| from **`DocReader`** alone | `TheWw8ReaderKeepsALeaningRunToo` |
| the walker never recognises `w:ffData/w:checkBox` | all 15 checkbox tests |
| the 25-twip inset is dropped, at the reader or at the drawing pass | 12 of 15, twice |
| a ticked box is not crossed | 2 |

**All four reader arms are individually detected**, and the fourth only became so inside this round:
`verify-test.sh` first reported the `DocReader` site **not detected by any of the 1164
WordProcessing tests** — and it is the arm with the largest measured corpus effect. A `.doc` cannot
be authored from a string, so `features/synthetic-oblique-run.doc` is LibreOffice's own Word 97
export of a flat ODF whose one font face declares no generic; that leaves the family at
`FAMILY_DONTKNOW` and `wwFont::Write` puts `ff = 0` in the table, which a `.docx` round trip cannot
reach.

**Labelled honestly:** `AStatedCheckBoxSizeIsInert` is a **drift guard**, not a detector of its own
claim — nothing in the reader reads `w:size`, so "we started honouring it" is not a mutation the
current code can express. It does detect both inset mutations. `TheBoxReservesItsWholeSquareOnTheLine`
detects only the walker mutation.

## Files

- `prediction.md`, `prediction-checkbox.md` — each committed before the change it covers.
- `shear-chars.py` — round 55 slides' instrument, re-pointed; reproduces its words figures exactly.
- `shear-faces.py` — the face drawn under a sheared matrix, on each side.
- `shear-split.py` — the two directions separated, which the signed total hides.
- `oblique-uniform.py` — ten authored two-run packages, built to discriminate.
- `formcheckbox.py` — nineteen authored packages, duplicate-input control first.
- `checkbox-census.py` — 675 boxes in 12 documents, with its blind spots in the docstring.
- `audit_poolspacing.py` — 28 names, both halves, two controls, and an output assertion it now has
  because its first run did not.

## What the next round does first

1. **Synthetic oblique is lost a second time, on the glyph-fallback face.**
   `SystemFontResolver.ReferenceFor` is a reverse lookup from a face with no request to compare
   against, so a run whose glyph comes from a fallback face is drawn upright however italic it is.
   289 of the residual 1 611 short glyphs are in faces no document names — WenQuanYi Zen Hei 177,
   OpenSymbol 112 — led by `手机免提系统TSB.doc` 82 and `A320SimNotes.doc` 75. **This one is in
   `XRay.Office.Text` and owes a measured cross-track sweep.**
2. **`AFS-050-004-F2_0i` page 2's five banner rows.** Confirmed again by a fresh reader, with the
   direction corrected: *our* rows are looser, not shorter. Separate the two causes the image cannot
   — extract our text layer for the strings `0.000 General Information…`, `CE-1 …` and see whether
   they are present-but-unpainted or never read.
3. **`2024-12_Comlux…docx`** — the reference draws `LiberationSans-Italic` where we draw `DejaVuSans`
   on 652 glyphs. A font-resolution divergence with the same face *count* on both sides, now visible.
4. **The `ascii` slot fallback** (four documents), untouched this round.
5. **`097`'s 1.7 pt boundary case** — 11.50 pt against 12.65 per empty paragraph, untouched for five
   rounds.
6. The `.doc` and `.rtf` arms of the form checkbox: WW8 spells it as a `PLCF` of field characters
   and RTF as `\*\formfield`, and neither is censused, let alone implemented. "675 in 12" is exact
   for OOXML and a floor for the corpus.
