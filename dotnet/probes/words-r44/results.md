# Words round 44 — measured results

Predictions in `prediction.md`, committed at `9d4a8873c` before anything was rendered post-change.

## Baseline

Reproduced **exactly** at `7f7c5fbb7`, over `words/batch-*`, 200 rows, no duplicate paths
(`baseline.tsv`):

| | brief | measured |
|---|---:|---:|
| documents matching | 154 | **154** |
| absolute page error | 78 | **78** |
| exactly-correct page counts | 164 | **164** |
| absolute word error | 6512 | **6512** |

## The scoreboard after

`after.tsv`, same harness, same corpus, 200 rows, no duplicates.

| | baseline | after | predicted |
|---|---:|---:|---|
| documents matching | 154 | **155** | 155–156 |
| absolute page error | 78 | **77** | 74–80 |
| exactly-correct page counts | 164 | **165** | 164–166 |
| absolute word error | 6512 | **6605** | 6400–6560 — **missed** |
| renderings changed | — | **14** | 10–30 |

**The word error rose and the prediction's band did not allow for it, though its prose did.**
The rise is one document: `150_5335_5a.doc`, +143, whose newly-drawn shape group contributes text
to `wc -w` on the pages it appears on. Its under-draw *falls* and its pixel metric improves — see
below.

Three documents moved on the gate:

| document | before | after |
|---|---|---|
| `omrIMInterpretiveGuideLine.doc` | `words` 355/382 | **`match`** 376/382 |
| `May 25 bulletin focus on carers…docx` | `pages,words` 5/4 | `words` **4/4** |
| `150_5335_5a.doc` | `match` 19185/19195 | `match` 19328/19195 |

## Reach, and how it was measured

**Byte-comparing our two renderings does not work on this project's PDFs.** Every one of the 200
differs, because the writer stamps `/CreationDate`. Normalising that one string first gives
**14 renderings changed**; without it the answer is 200 and looks like a corpus-wide change.
Recorded here because the skill recommends the byte comparison and it silently gives the wrong
answer.

Attributed by what changed inside each PDF:

| change | reach | predicted |
|---|---:|---|
| the nested-table filler paragraph | **8** DOCX | at most 8, from an exact syntactic census |
| a DOC shape group flattened | **2** `.doc` | 4–15, **too high** |
| a greyscale JPEG as `/DeviceGray` | **3** (`.doc`) | uncounted |
| a CMYK JPEG decoded rather than passed through | **2** (`.docx`) | uncounted |

The filler census was exact because the shape is syntactic — a `w:tc` whose last two children are a
`w:tbl` and an empty `w:p` — and all eight of the eight it named changed. **The group estimate was
the one with no census behind it, and it is the one that missed**, which is what `prediction.md`
said would happen.

## Task one — the row-height law is refuted, and one rule survives under it

Two independent measurements, either of which is a refutation on its own.

**1. The whole track.** `row-height-census.py` reads the vertical strokes of the 400 PDFs the
baseline sweep had already written. A vertical stroke inside a table is a cell edge and its length
is that cell's drawn height, so the sorted multiset of them on a page is the multiset of drawn row
heights — translation invariant, which matters because one wrong row displaces every rule below it.

Over the 164 page-exact documents: **4264 paired rules, 3410 of them (80%) agree within 1 pt.** Of
the 854 that differ, **383 are ours taller and 471 ours shorter**, median −1.68. There is no
systematic direction, so there is no law of the kind the sheets track found.

**2. The control.** Of the 76 *matching* page-exact documents that draw any pairable cell edge,
**46 have a row height off by more than 1 pt and 35 by 10 pt or more** — 61% and 46%, against 6 of
8 on the documents failing on words alone. The observable does not separate broken documents from
correct ones.

What it can and cannot see, stated because a census that does not say so has misled this project
twice: it sees only rows whose cell edges are *stroked*, it is blind to documents whose page counts
differ, and — unlike a markup census — it reads `.doc` and `.docx` identically, because it reads
neither.

### The rule that is really there

`header-row-mutations.py` varies what one cell of `UG.CAO.00133`'s header table holds and reads the
drawn geometry back out of both renderers:

| cell content | ref | ours (before) | diff |
|---|---:|---:|---:|
| table (2 rows) + empty paragraph | 26.35 | 36.65 | +10.30 |
| table (1 row) + empty paragraph | 14.55 | 24.90 | +10.35 |
| table (3 rows) + empty paragraph | 38.05 | 48.40 | +10.35 |
| table + a paragraph with text | 37.75 | 36.50 | −1.25 |
| table + **two** empty paragraphs | 49.55 | 48.29 | −1.26 |
| empty paragraph + table + empty paragraph | 37.95 | 48.29 | +10.34 |
| an empty paragraph, no table | 14.45 | 14.40 | −0.05 |

One rule fits all seven to a tenth of a point:

> LibreOffice does not lay out a cell's **last** paragraph when it is empty and the block before it
> is a **table**.

Both conditions are load-bearing. With two trailing empty paragraphs *both* are laid out, because
the last one follows a paragraph; with a leading empty paragraph and a trailing one, only the
trailing one goes. That paragraph is the one OOXML makes mandatory, since a `w:tc` may not end with
a `w:tbl`.

Implemented in `DocxLayoutSource.ReadCell` and pinned by five tests. `UG.CAO.00133`'s header row is
now 25.0 pt against the reference's 26.35, from 36.65.

## Task two — two objects the user could see were missing

### `omrIMInterpretiveGuideLine.doc`: read, never drawn

`paperless extract` has every word of the masthead — "MENTAL RETARDATION BULLETIN", the date, the
number, "Nancy R. Thaler" — so this was never a reading fault. Two defects were stacked on it:

- **A group is one `FSPA` and many shapes.** The members have no anchor of their own and state
  their rectangles in the group's `msofbtSpgr` space. One frame per anchor drew the group's outline
  and nothing else.
- **The envelope painted an opaque white box** over the body text underneath, because `fFilled` and
  `fLine` default true and a group states neither. An `SdrObjGroup` paints nothing.
- **The seal is a 635×638 one-component JPEG** and every passed-through JPEG was announced as
  `/DeviceRGB`, so it drew as three squashed copies across the top of its box.

All five inner boxes and the seal now land within 0.05 pt of the reference. 355 words against 382
before, **376 against 382** after; under-draw 27 → **6**, over-draw 0 either way.

### `FO.FCTOA.00010`: 249 form checkboxes, established and not fixed

Not a missing picture. The document holds exactly one image and both renderers draw it on every
page. What is missing is **249 legacy `FORMCHECKBOX` form fields**, which the reference draws as a
stroked square in the run of text and we draw as nothing at all — every "Mr. ☐ Ms. ☐", every "☐
Same as Applicant Data in section 1.1".

`formfield-census.py`: **16 of 200 documents** carry one — 13 DOCX and 3 `.doc`, and the `.doc`
half is found by searching the text stream for the UTF-16 instruction, so unlike a zip census this
one sees both formats.

**Not implemented, and the reason is a constant that would not pin.** The drawn square is 9.0,
9.05, 9.7, 10.95, 11.3 and 15.9 pt across the corpus, and it does not follow `w:checkBox/w:size`
(which is 22, 24 or `sizeAuto`). LibreOffice's own arithmetic is
`FormControlHelper::createCheckbox` — `16 × size` in 1/100 mm, or `floor(CharHeight × 35.3)` when
the size is automatic — which gives 3.53 mm for a 10 pt run where the drawn square is 3.175 mm, so
the control's rectangle is not the square. Twelve of the sixteen documents currently **match**, and
adding an advance to every checkbox line without the right constant would put them at risk for a
change the word gate cannot see. Left as a specified lead rather than a guessed one.

## The cross-track change

`PdfImages` is in `Paperless.Rendering`, which all three tracks share. The branch it changes was
already wrong for every 1-component and 4-component JPEG and is untouched for 3-component ones, so
it cannot make a correct image incorrect. Measured on words: 3 documents gained `/DeviceGray`, and
two DOCX carrying a CMYK JPEG stopped being passed through — `PES-Technical-Report-Template`'s
cover picture goes from a washed-out ghost to the reference's crisp blue-and-green artwork.

**The change's ceiling is an exact census this time**, because the component count is a property
of the image bytes rather than of anything that has to resolve, and it reads a zip container and an
OLE2 container alike (`jpeg-component-census.py`):

| track | documents carrying a 1- or 4-component JPEG | of |
|---|---:|---:|
| words | **5** | 200 |
| slides | **7** | 163 |
| sheets | **0** | 171 |

Words is the check on the instrument: it says 5 and **exactly 5 words documents changed for this
reason** in the sweep — three gaining `/DeviceGray`, two ceasing to be passed through. So the
slides figure of 7 is a reach and not merely a ceiling, and sheets cannot be touched at all.

No slides sweep was run for it. The parent session owes one; 7 documents is what it should expect
to move, and none of them can move a page count or a word count.

## `150_5335_5a.doc`, where the word gate reads a regression

| | under-draw | over-draw | total | sum \|ink\|% |
|---|---:|---:|---:|---:|
| before | 236 | 227 | 463 | 45.94 |
| after | **191** | 325 | 516 | **39.23** |

45 words that were missing are now drawn; the surplus grows by 98 and the scalar rises by 143. The
pixel metric moves the other way — total `|ink|%` over its 63 pages falls 45.94 → 39.23, and the
major-page count is 14 either side. The document still matches.
