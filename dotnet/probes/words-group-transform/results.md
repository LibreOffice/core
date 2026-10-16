# A group's transform: what it covers, and which way up it is

*Measured 2026-09-03 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`, reference `soffice` the
distro's **24.2.7.2**. Every figure below is a fresh measurement in that environment, not a stored
one.*

## The question

`004_Free_Genogram_*` and its seven siblings place their shapes wrongly, and three earlier rounds
narrowed the fault without closing it: the drawing's span matches the reference to the point while
the interior positions do not, so it is per-shape rather than a translation, and
`dotnet/probes/words-frame-origins/` refuted the horizontal-origin hypothesis outright. What was
left was the composition of nested `wpg:grpSp` transforms.

It turned out to be two defects, and the first is what exposed the second.

## The instrument

Small `docx`, each one anchored group, each varying one thing, every size a round number so the
answer is checkable by hand. `mk.py` writes the extent probes and `orientation.py` the orientation
ones; `measure.py` reads the drawn rectangles off a 100 dpi raster by colour, which is the
instrument the origins probe had to switch to after reading a PDF content stream produced a
fictional 36 pt error. `ink.py` scores a whole sweep's first pages against the reference's.

Every probe is an anchor at page (100 pt, 50 pt) reserving **400 × 200 pt**, holding a group whose
`a:ext` is 400 × 200 pt.

---

## Defect 1 — a group is as big as what is in it

Extent probes, group `a:chExt` **800 × 400 pt** — a uniform 0.5 — unless the row says otherwise.

| probe | what is in the group | ours, before | 24.2.7.2 |
|---|---|---|---|
| `A_small` | one 100 × 50 pt member at the child origin | 49.7 × 25.2 | **399.6 × 199.4** |
| `B_exact` | one member filling `chExt` | 399.6 × 200.2 | 399.6 × 199.4 |
| `C_offset` | one 100 × 50 pt member at (200, 100) in child space | 49.7 × 25.2 at (200, 100) | **399.6 × 199.4 at (200, 100)** |
| `D_two` | two members covering half of `chExt` each way | 49.7 × 25.2 | **79.9 × 39.6** |
| `E_aniso` | two covering half across and a quarter down | 49.7 × 25.2 | **79.9 × 99.4** |
| `F_nested` | one member plus a nested group holding another | blue at (349.9, 175.0) | **blue at (433.4, 216.7)** |
| `G_extdiff` | the group's own `a:ext` halved | 200.2 × 100.1 | **399.6 × 199.4** |
| `H_big` | one member twice the size of `chExt` | 742.3 × 400.3 | **399.6 × 199.4** |
| `I_nochext` | no `a:chExt` at all | 100.1 × 50.4 | **399.6 × 199.4** |
| `K_rot` | one member filling `chExt`, `rot="5400000"` | 200.2 × 349.9 | **399.6 × 149.8** |
| `L_norot` | the same member upright | 399.6 × 200.2 | 399.6 × 199.4 |

The 0.7 pt discrepancy in the last column is the raster's edge convention and is present in the two
control rows as well.

**A group has no size of its own — its rectangle is whatever its members happen to cover — and
Writer sizes the imported group by resizing it to the anchor's `wp:extent`**, which is the one size
a `w:drawing` actually declares. Read off the rows:

- `A_small`, `H_big`, `I_nochext`: it grows and shrinks alike, and needs no `chExt` to fire.
- `B_exact`, `L_norot`: where the members already fill their child space it is the identity, which
  is nearly every file in the corpus and is why this survived so long.
- `G_extdiff`: the target is `wp:extent`, **not** the group's own `a:ext`. Halving the latter
  changes nothing in the reference; `a:ext` decides the child scale and nothing else.
- `E_aniso`: two independent factors, 1.6 across and 4.0 down.
- `C_offset`: the corner it grows from is the members' own top-left, so the drawn content can end
  up outside the rectangle the anchor reserved — an `SdrObjGroup` resized about its own snap
  rectangle.
- `K_rot`: what a turned member covers is its **rotated** box. `P_rotout` isolates that: a group
  holding one member that fills it and one turned member sticking out top and bottom comes back
  with the filler at **399.6 × 133.2**, which is the fit to the turned union and not to the stated
  rectangles.

Predicted from that model, `F_nested`'s blue square lands at **(433.33, 216.67) pt**; measured
**(433.4, 216.7)**. Every other row is exact to the raster.

### What it is not

`oox`'s own composition is not the difference. `Shape::createAndInsert`
(`oox/source/drawingml/shape.cxx:1167-1205`) maps a child by `aParentScale / maChSize`, where
`aParentScale` is the cumulated scale from the decomposed parent matrix — which is exactly what
`DocxFrames.GroupTransform` composes, and which reproduces **our** answer for `F_nested`, not the
reference's. Tracing that first is what made it clear the difference has to live above the import,
in what Writer then does to the group.

### Reach

`census.py` walks the corpus, composes each group anchor's children and compares what they cover
with the anchor's `wp:extent`. **13 group anchors across 10 `docx`** are out by more than 2 per
cent (rotation not modelled, so this is a floor):

```
   1.565     1/5     docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx
   0.674     2/2     059_Disease_Concept_Map_Template_4acf63f2.docx
   0.355     1/8     003_Free_Genogram_Diagram_Template_Easy_Format_60eb6e42.docx
   0.230     3/13    001_Free_Genogram_Diagram_Template_Blue_and_Gray_Theme_7c9e01a7.docx
   0.146     1/1     026_Unit_Circle_Chart_Four_Quadrants_09bf3ed1.docx
   0.097     1/4     010_Free_Genogram_Diagram_Template_Yellow_Theme_6ee4f818.docx
   0.078     1/15    009_Free_Genogram_Diagram_Template_Handy_Format_4bb900f0.docx
   0.075     1/5     008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme_9017c1a8.docx
   0.067     1/1     071_Storyboard_Template_Cartoon_Theme_ae113de2.docx
   0.037     1/5     002_Free_Genogram_Diagram_Template_Customizable_Format_2219584b.docx
```

### A canvas is not a group

`wpc:wpc` is left alone. It states no child space at all — its members are in its own coordinates —
and it is a fixed rectangle the author drew into rather than a group taking its size from its
contents. This is a decision rather than a measurement: `soffice` 24.2.7.2 **crashes** on a
hand-written minimal canvas (`Unspecified Application Error`, with and without a
`graphicData/@uri`), so the probe that would have settled it could not be run. The corpus holds 9
`wpc:wpc` in 4 documents against 190 `wpg:wgp`, and the one of those four in the census above
mis-fits on a `wgp`, so the restriction costs it nothing.

---

## Defect 2 — a group's own `rot`, `flipH` and `flipV` were never read

**The fit is what exposed it.** Applied on its own it made two documents much worse —
`055_Organogram_Template_Horizontal_Structure` 7.94 → 31.02 of first-page ink,
`008_Free_Genogram` 8.96 → 19.05 — because the fit takes the union of what the members cover, and
members put in the wrong place take the whole drawing with them. The wrong place came from
`<a:xfrm rot="5400000">` on a `wpg:grpSp`, which this reader ignored: 055's four rows of connectors
each sit in a group turned 90°, and unturned they run down the page through the boxes instead of
across between them, as one black rule and sixteen arrows pointing the wrong way.

Censused over the corpus, **74 groups across 15 `docx`** state an orientation:

| | groups |
|---|---:|
| `flipH` alone | 28 |
| `flipV` alone | 14 |
| both flips | 2 |
| `rot` 90° | 19 |
| `rot` 270° | 6 |
| `rot` 180° | 6 |
| ... of which also flipped | 6 |

**Every rotation in the corpus is a multiple of ninety degrees**, which is what makes an
axis-aligned frame able to hold the result at all.

`orientation.py` writes one probe per case: a nested group stating the orientation, a grey member
filling it so the fit is the identity, and a gold text-bearing mark in the middle. After the fix,
all nine agree with 24.2.7.2 to the raster's 0.7 pt:

```
O_plain            ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_rot90            ours  left= 349.9 top=   0.0 w= 100.1 h= 100.1   ref  350.6   0.0  99.4 100.1
O_rot180           ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_rot270           ours  left= 349.9 top=   0.0 w= 100.1 h= 100.1   ref  349.9   0.0 100.1 100.1
O_flipH            ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_flipV            ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_rot180_flipH     ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_flipH_in_flipV   ours  left= 200.2 top= 125.3 w= 200.2 h=  49.7   ref  200.2 125.3 199.4  49.7
O_rot90_at_top     ours  left= 275.0 top=  49.7 w=  49.7 h= 200.2   ref  275.0  50.4  49.7 199.4
```

Three things in there are not what the arithmetic alone would give, and each cost a round:

1. **A half turn and a horizontal flip is a plain vertical mirror, and turns nothing.**
   `051_Organogram_Template_Basic_Theme` states exactly that, and adding 180° to its members stood
   every box in the diagram on its head. A mirroring map factors as `R(φ) ∘ Fh`, which leaves φ free
   by 180° — the same map is also `R(φ+180) ∘ Fv` — so the smaller of the two is taken. Nothing here
   can mirror a frame, and dropping the mirror while keeping the turn is the closer of the two
   available answers.

2. **A group's turn does not reach its members' text.** In `O_rot180` the reference moves the box to
   the opposite corner — which this reader now reproduces to the point — and draws its "ABC" upright
   at the top left. `oox`'s `lcl_mirrorAtCenter` is why: a parent's negative scale becomes the
   child's own `flipH`/`flipV`, and a half turn decomposes into exactly that pair, two mirrors,
   which move a rectangle and leave its text alone. At a quarter turn the reference draws the text
   nowhere at all, so there is nothing there to match either way.

3. **The outermost group's orientation is applied after the fit, a nested group's before it.**
   `O_rot90` and `O_rot90_at_top` state the same 90° on the same geometry and the reference answers
   350.6 and 275.0 — LibreOffice turns the outermost group as an *object*, once it has been sized to
   the anchor, while a nested one's turn is part of the child transform and is inside what the fit
   measures. Ten of the 74 oriented groups are outermost ones, across five documents.

---

## What the two together are worth

Words gate, all 337 `words` paths, against the same sweep before the change:

```
TOTAL 337  MATCH 310  MISMATCH 27  REF-CANNOT-RENDER 0      (309 / 28 before)
```

One document moves from failing to matching — `008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme`,
66 of 70 extractable words to **70 of 70** — and three other rows change without changing
verdict, one of them (`003_Free_Genogram`, 27 of 29 words to 29 of 29) to exact.
Nothing regresses.

First-page ink against the reference, whole track, `ink.py`:

| | before | after |
|---|---:|---:|
| `052_Organogram_Template_Colorful_Flow_Chart` | 21.493 | **1.716** |
| `004_Free_Genogram_Diagram_Template_Editable_Format` | 23.716 | **3.844** |
| `009_Free_Genogram_Diagram_Template_Handy_Format` | 16.251 | **1.921** |
| `010_Free_Genogram_Diagram_Template_Yellow_Theme` | 10.384 | 3.722 |
| `051_Organogram_Template_Basic_Theme` | 9.955 | 4.670 |
| `008_Free_Genogram_Diagram_Template_Green_and_Yellow_Theme` | 8.963 | 5.536 |
| `002_Free_Genogram_Diagram_Template_Customizable_Format` | 8.704 | 6.027 |
| `055_Organogram_Template_Horizontal_Structure` | 7.935 | 5.835 |
| `007_Free_Genogram_Diagram_Template_Green_and_Purple_Theme` | 5.235 | 1.593 |
| `059_Disease_Concept_Map_Template` | 5.322 | 3.333 |
| `030_Unit_Circle_Chart_Points_System` | 4.550 | 4.119 |
| `056_Organogram_Template_Square_Theme` | 4.595 | 3.425 |
| `001_Free_Genogram_Diagram_Template_Blue_and_Gray_Theme` | 3.681 | 2.279 |
| `006_Free_Genogram_Diagram_Template_Fillable_Format` | 3.777 | 2.503 |
| `025_Unit_Circle_Chart_Cos_and_Sin_Model` | 3.668 | 2.671 |
| `003_Free_Genogram_Diagram_Template_Easy_Format` | 2.185 | 1.965 |
| `053_Organogram_Template_Creative_Theme` | 2.162 | 2.048 |
| `058_Organogram_Template_With_Picture_Theme` | 1.328 | 0.876 |
| **337 documents, mean** | **8.367** | **8.112** |

**Eighteen documents improve and none regresses.** All eight `Free_Genogram` templates are in the
list, which is the family this started from.

## A trap in the instrument, worth keeping

`ink.py`'s first cut assumed `pdftoppm` writes `a-1.png`. It pads the page number to the width of
the document's page count, so a hundred-page file writes `a-001.png` — and the measurement silently
covered **267 of 337** documents, all of them the short ones, while reporting a confident mean. The
figure it gave was not wrong for what it measured; it just was not measuring the corpus. Assert
that your instrument produced output for everything you are about to average.

## Reproducing

```sh
python3 mk.py <dir>                      # the extent probes
python3 orientation.py <dir>             # the orientation probes
for f in <dir>/*.docx; do
  "$XRAY_OFFICE_CLI" render "$f" --outdir ours
  soffice --headless --convert-to pdf --outdir ref "$f"
done
pdftoppm -r 100 -f 1 -l 1 -png <pdf> <stem>
python3 measure.py <png>...
python3 census.py /home/user/sample-files
python3 ink.py <before-sweep-dir> <after-sweep-dir>
```
