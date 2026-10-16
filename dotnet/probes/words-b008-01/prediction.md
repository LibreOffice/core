# words/batch-008 round 1 — prediction, written before any sweep

Written 2026-08-14, on branch `wt-words-b008`, before the fix was compiled and before any
corpus measurement. Scored afterwards in `results.md`.

## What the round found before predicting

`words/batch-008/docx/FAA-2017-0628-0002_attachment_1.docx` — `pages 4/4 words 666/638`.

The two checks the brief demanded first, both run before anything else:

1. **Determinism.** Six independent `soffice 26.2.4.2` conversions, each with its own profile:
   `638 638 638 638 638 638` (raw `641` each). The banked reference is the same. So this is
   **not** the `fse_identification_form` shape — the reference is a fixed answer here.
2. **Whitespace-stripped character streams.** `3750` non-space characters on each side,
   `difflib` reports **zero** diff blocks — the streams are byte-identical. So no content is
   missing or surplus in either direction; the whole 28 is tokenisation.

Diffing the token streams localises it to two runs, both ours fragmenting:

```
REF : ['PADM', '533:', 'Policy', 'Formation']        OURS: ['P','A','D','M','5','3','3',':', … 'ti', …]
REF : ['Dr.', 'Marcia', 'Godwin']                    OURS: ['D','r','.','M','a','r','c','i','a', …]
```

One line, the cover page's footer, drawn from a `w:txbxContent` whose run carries
`<w:spacing w:val="60"/>` — 3 pt of tracking per character.

## The mechanism, measured rather than assumed

Both renderers write that line as **one show operator per glyph inside a single `TJ` array**
with ~`-300` adjustments (ours 45 glyphs, the reference 46). Same geometry: our run spans
338.64 pt, the reference's 341.21 pt. So `pdf-ops.py`'s show counts do not separate them.

Byte-surgery on both PDFs (content stream re-deflated, file offsets preserved) finds poppler's
word-break threshold to be **0.400 em in the reference PDF and 0.100 em in ours** — same font,
same size, same reader. Mutating our PDF one property at a time:

| mutation | result |
|---|---|
| baseline | SPLIT |
| page stream reduced to that one `BT…ET` | SPLIT |
| `/Widths` integerised to match the reference's | SPLIT |
| `/StemV`, `/Descent`, `/FontBBox` set to the reference's | SPLIT |
| **`ToUnicode <15>` changed from `<00740069>` to `<0074>`** | **JOINED** |

So the trigger is the **multi-character `ToUnicode` entry**: we form the `t`+`i` ligature
(Carlito-Bold `liga` lookup 37, `t ['i'] -> glyph02210`, confirmed in the font's GSUB), one
glyph mapping to two characters, and poppler drops its intra-word gap tolerance from 0.4 em to
0.1 em for the whole line, shattering all 45 glyphs.

The reference does not form that ligature. `vcl/source/outdev/text.cxx:996-998` —

```cpp
if( maFont.IsFixKerning() || … PITCH_FIXED )
    nLayoutFlags |= SalLayoutFlags::DisableLigatures;
```

— and `Font::IsFixKerning()` is `mnSpacing != 0`, fed from `RES_CHRATR_KERNING`
(`sw/source/core/text/atrstck.cxx:619`), which is exactly `w:spacing`.
`CommonSalLayout.cxx:453` turns the flag into `liga=0, clig=0`. **Non-zero tracking disables
the optional ligatures.** We have `ShapingOptions.DisableLigatures` already and no caller ever
sets it.

*The tree is 27.2-alpha and the binary 26.2.4.2, so the tree is cited for the mechanism only.
The behaviour is measured: the reference's PDF holds 46 glyphs with no ligature.*

## The fix I intend

State the rule once, where tracking and shaping meet — an `EffectiveShaping` on `FormattedRun`
and `PageRun` that ORs in `DisableLigatures` when `Tracking != 0` — and use it at every site
that shapes.

## Predictions

Scored honestly in `results.md`. Numbers stated now, not adjusted later.

1. **`FAA-2017-0628-0002_attachment_1.docx` reaches `words 638/638` and verdict `match`.**
   Confidence **high**. The character streams already agree; removing the ligature removes the
   only multi-char `ToUnicode` entry on that line.
2. **Our footer run widens by about 2.6 pt** (one extra tracking gap of 3 pt, less the
   difference between `ti` and `t`+`i`), landing within ~0.5 pt of the reference's 341.21.
   Confidence **medium-high** — it is arithmetic on measured spans, but the twip snapping could
   move it.
3. **Reach: 1 to 3 of the 200 words documents change verdict, net positive.** 61 of the 200
   carry a non-zero run `w:spacing` (a grep prior, not a resolution count), but a verdict only
   moves where a tracked run also contains a `liga` pair *and* the document is within reach of
   the 2% band. Confidence **medium**.
4. **This is a cascade risk and I expect it to cost something.** Disabling `liga`/`clig` widens
   every tracked run holding `fi`, `fl`, `ff`, `ti`, `ct`… so line breaks can move on any of
   those 61. **I predict at least one currently-passing words document regresses**, most likely
   on page count, and put it at **40%**. If it happens the fix is still right — it is what the
   reference does — and the regression is a different defect made visible.
5. **`XRay.Office.Fidelity.Tests` moves by at most 2 in either direction** from its 30-of-550
   baseline. Confidence **medium**.
6. **`words/batch-001..008` ends at 79 of 80**, the one failure being `1447.doc`, which is the
   line-height law and deliberately not chased. Confidence **medium** — this is prediction 4
   with the sign assumed favourable.

## A second finding I am recording and NOT fixing this round

The blind page-vision reviewer (given the image alone, no numbers) reported that the reference
breaks the cover title over three lines where we break it over two, and sets everything below it
lower. Independently, our footer starts at x=33.85 and the reference's at x=41.15 — a difference
of **7.30 pt** against the `wps:bodyPr/@lIns` of `91440` EMU = **7.2 pt**. So we appear to
ignore a Word text box's `lIns`/`rIns`/`tIns`/`bIns` insets, which both displaces the text and
widens the measure by `lIns+rIns` = 14.4 pt — enough to explain the title wrap the reviewer saw.

**It moves no word and no page here**, it is a separate defect with a separate blast radius, and
mixing it into this commit would make prediction 4 unscoreable. Recorded for its own round.
