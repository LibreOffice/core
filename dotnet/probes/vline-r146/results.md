# Round 146 — a `v:line` states its extent in `from`/`to` (seat O97)

Reference binary: `/opt/libreoffice26.2/program/soffice` → **LibreOffice 26.2.4.2**,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. The C++ tree read here is
`/home/user/libreoffice-core` at **27.2.0.0.alpha0+**, which is *not* that binary's source;
**[src]** marks a reading of it and **[bin]** a measurement against 26.2.4.2's own output.

Round 145's `probes/vline-r145/` is where this round's evidence comes from: the corrected census,
the eighteen authored one-attribute probes and their reference renderings, and the two witnesses.
This directory holds only what the *fix* needed.

## 1. The defect

**[src]** `DocxVmlFrames.Floating` required a `style` width and a `style` height, and returned
null without them. Every top-level `v:line` states neither, so every one was dropped.
`LineShape::getAbsRectangle` (`oox/source/vml/vmlshape.cxx`) builds the rectangle out of the two
endpoints instead and **does not read the style rectangle at all**:

```
X = from.x     Y = from.y     Width = to.x − X     Height = to.y − Y
```

`getRelRectangle` beside it does the same inside a `v:group`, with `o3tl::toInt32` on each token —
the leading integer, **suffix dropped** — because there the numbers are coordinate units.

**[bin]** All eighteen of round 145's probes now reproduce 26.2.4.2: the twelve top-level cases
exactly, the three group cases within **0.05 pt**, and the three that draw nothing still draw
nothing. The eighteenth, `thirdpara`, puts the line on its anchor paragraph's own top, which is
the case a fixed page offset would pass by accident.

## 2. Four things the rule is, each measured rather than assumed

- **The `style` rectangle is ignored.** Three probes add `left:100pt;top:50pt`,
  `width:200pt;height:100pt` and both, and the reference draws all three exactly where the bare
  shape put it. So the seat's *"states no `style` width or height"* was describing the symptom,
  not the rule: even when a width **is** stated it is not the one used.
- **A bare number is a pixel at 96 dpi, not a point.** `decodeMeasureToHmm(…, bDefaultAsPixel:
  true)`; `to="96,48"` is drawn 72 × 36 pt. This is why `LineOf` parses the pair itself instead of
  going through `Css`, whose bare number is a point — `Css` serves the `style` properties, which
  this round did not measure and did not change.
- **A line whose x decreases is not drawn; one whose y decreases is.** `getAbsRectangle` subtracts
  without normalising, so a backwards line has a negative width, and the probes stating
  `from="144pt,0" to="0,0"` and `from="144pt,72pt" to="0,0"` come back with **no ink at all**
  while `from="0,72pt" to="144pt,0"` is drawn as the box's anti-diagonal. **Normalising the two
  points into a rectangle would draw three lines where the reference draws two**, so the
  asymmetry is reproduced rather than tidied away.
- **Inside a group the numbers are units and a suffix is dropped.** `to="1000,1000"` spans a
  200 × 100 pt group whole; `to="100pt,50pt"` is drawn 20 × 5 pt, which is a hundred units by
  fifty. Round 145 left this ambiguous between "the suffix is parsed to points and the point
  count used as the unit count" and "the suffix is dropped"; `o3tl::toInt32` settles it as the
  second.

## 3. A second defect the witness exposed, which is not about lines

`JEMIT_Template.docx`'s footer rule is the corpus's **only** `strokecolor="windowText"` — against
1015 `black`, 78 `red`, 41 `white`, 34 `blue`, 4 `gray` and 3 `none`. `VmlColour` knew the twenty
preset names and nothing else, so the name resolved to null, `PaintOf` returned no stroke, and
the shape was built and painted with nothing.

**[src]** `ConversionHelper::decodeColor` (`oox/source/vml/vmlformatting.cxx`) tries
`Color::getVmlPresetColor` and then falls through to `GraphicHelper::getSystemColor`, whose
palette is **a fixed table of Windows XP's defaults** compiled into the filter
(`oox/source/helper/graphichelper.cxx`:63-100) rather than anything the running desktop decides.
So these are constants, and a reader that treats them as a desktop theme and declines to resolve
them draws nothing. `DocxVmlFrames.SystemColours` is that table's twenty-three useful entries.

*The preset table's own remark already warned about this shape of failure — "a missing name draws
nothing at all, which is the one failure mode that is invisible" — and the table was still one
family short.*

## 4. Reach and confinement, measured

The words track — 337 documents from `MANIFEST.tsv` — rendered twice under `SOURCE_DATE_EPOCH=0`
with nothing but the binary changing, one output directory per document
(`par-sweep-words.sh`, `diff-legs.py`, `sweep-words.txt`):

```
documents scored: 337      failed on either leg: 0
byte-identical:   335      moved: 2
```

**The two that move are the two the census named**, and nothing else did:

```
syscolour+vline-live   words/done-015/docx/JEMIT_Template.docx
vline-live             words/missing-001/docx/33004.docx
```

`diff-legs.py`'s tags are deliberately narrow — a `v:line` *not* inside an `mc:Fallback`, such a
line inside a `v:group`, and a `strokecolor`/`fillcolor` naming a system colour — because the
round's whole claim is that nothing outside them moved. Its `live_lines` walks the tag stream
with a depth counter rather than parsing the part, for the reason round 145 recorded: a regular
expression over `word/document.xml` sees **both** branches of every `mc:AlternateContent`, and
859 of the corpus's 865 `v:line` are in the branch nobody reads.

Scored against 26.2.4.2 (`pdf-image-diff.py`, both legs and the reference on the same day):

| | pages | base | after |
| --- | ---: | ---: | ---: |
| `33004.docx` | 47 of 47 | 9.65 | **3.53** |
| `JEMIT_Template.docx` | 4 of 4 | 0.91 | **0.85** |

**No page count moves** and the MAJOR count is unchanged on both. On `33004` the header rule is
now drawn at `x 72.00, y 63.35, w 468.00` against the reference's **identical** figures, on every
page; the count of long thin horizontal rules goes 335 → 383 against the reference's 409.

## 5. The residual on `JEMIT_Template`, which is not this defect

Its footer rule is now drawn, 180 pt wide at the right x — and **11.5 pt too high**. That is not
the line rule: the footer paragraph it is anchored in is itself 11.5 pt high, measured on its own
text (`Corresponding author: MoizMohammed` sits at y 778.4 in ours and 789.9 in the reference),
and the line sits exactly 3.5 pt above its anchor on both sides, which is what the file says.

So this document has a second, independent footer-position difference, and it is why the
tolerant rule matcher still reports the same missing count for it while the rule count goes
21 → 22. **A count moving to the reference's while the matched set does not is the signature of a
mark that is now drawn and displaced**, and reading either number alone would have got it wrong —
the count alone would have called it fixed, the matcher alone would have called it untouched.

## 6. Tests, and what pins them

`VmlLineExtentTests`, 12 assertions, every expected value taken from a 26.2.4.2 probe: the box
from the endpoints, the three ignored `style` rectangles, the pixel, the unit suffix, the
x-decreasing asymmetry with its mirrored sibling, the zero-height line, the unstroked line, the
system colour, and the two group cases.

Pinned by three mutations (`mutations.txt`), each caught by **exactly one** test:

| mutation | caught by |
| --- | --- |
| the endpoints are normalised into a rectangle | `ALineWhoseXDecreasesIsNotDrawnAndOneWhoseYDecreasesIs` |
| a bare endpoint number is read as a point | `ABareNumberIsAPixelAtNinetySixDotsPerInchAndNotAPoint` |
| a system colour name is not resolved | `ASystemColourNameResolvesRatherThanPaintingNothing` |

One test per mutation is the right shape here: each is a separate rule with a separate witness,
and a mutation that tripped half the file would mean the assertions were not independent.

## 7. Suite state

Every project run separately: Containers 109, Core 591, Markup 259, OpenDocument 194,
Rendering 164, Spreadsheets 1479, Text 750, Vector 309, **WordProcessing 2025** — all green.
Fidelity 542 passed, **10 failed of 552, 0 skipped**, the same ten as rounds 144 and 145 by name.
Four of them are `PageDrawingComparisonTests` on `paginated.*`, which is the one family a
drawing change could plausibly have reached; it is the advance-reconstruction family this project
leaves failing on purpose, and none of the four `paginated` fixtures holds a `v:line`.
