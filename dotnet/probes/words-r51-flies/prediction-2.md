# words-r51-flies — second prediction, for a third fix found during the round

Committed **before** the sweep that measures it, and after the first sweep had already banked
309 → 311. Same environment as `prediction.md`.

## How this one was found, in order

1. `056_Organogram_Template_Square_Theme` (24 words against 56) was handed to a blind reviewer
   with the image and nothing else. It reported: *"only 5 blue leaf boxes survive, and they are
   piled into the left edge of the page, a single vertical stack … the remaining 20 leaves are
   absent as boxes; 4 of them survive as naked text with no rectangle."*
2. The round-50 reviewer's hypothesis for the same document was *"the group's child-offset /
   child-extent transform not being mapped onto the parent's"*. I tested the **other** standing
   hypothesis first, because it was cheaper: that `DocxFrames.MaxGroupNesting = 8` cuts the tree
   off, `056`'s deepest chain being `wgp > grpSp ×6 > grpSp > grpSp > wsp`. **Refuted**: raising
   the bound from 8 to 64 changes the word count of `056`, `057`, `025`, `030`, `008` and `071`
   by **zero**, every one of them.
3. Read the text layer's own coordinates instead. Ours draws its four surviving leaves at
   x = 134.8 pt, which is the drawing's left edge exactly; 26.2.4.2 draws twenty-five in a
   5 × 5 lattice at x = 146.0, 262.7, 386.8, 509.7, 627.7.
4. The markup says why. `056`'s five text-bearing `a:grpSp` are byte-identical apart from
   `a:off/@x` — 141890, 1623848, 3200400, 4761186, 6258911 EMU — and each states `chOff="0,0"`
   with a `chExt` equal to its `ext`.
5. `GroupTransform.Composed(inner)` sets `ShiftX = ShiftX + inner.ShiftX * ScaleX`, and
   `TransformOf` **never sets a shift** — it returns `0, 0` on every path. So a nested group's own
   `a:off` was dropped and its members were laid out at the parent's origin. The *scale* composed
   correctly throughout, which is why the members came out the right size in the wrong place.

## The change

`Around(group, inner)` replaces `Composed(inner)`: it reads the nested group's own
`a:grpSpPr/a:xfrm/a:off` and maps it through the enclosing transform, exactly as a leaf's offset is
mapped, so `ShiftX = ShiftX + (off.x − OriginX) × ScaleX`.

## Documents expected to change, measured one at a time before the sweep

| document | before | after | reference |
|---|---:|---:|---:|
| `056_Organogram_Template_Square_Theme` | 24 | **56** | 56 |
| `057_Organogram_Template_Vertical_Colorful_Theme` | 21 | **36** | 36 |
| `025_Unit_Circle_Chart_Cos_and_Sin_Model` | 126 | **141** | 141 |
| `071_Storyboard_Template_Cartoon_Theme` | 11 | **41** | 41 |
| `030_Unit_Circle_Chart_Points_System` | 107 | **116** | 118 |
| `008_Free_Genogram_Diagram_Template_Green_and_Yellow` | 57 | **66** | 70 |

(raw `pdftotext | wc -w`, which is not the gate's metric; the gate counts only tokens carrying a
letter or a digit.)

**Predicted verdict movement: +5** — `056`, `057`, `025`, `071` to exact agreement, and `030` to
within the band (`max(2% × 118, 3) = 3`, and it lands 2 short). `008` is expected to stay open at 4
short of a band of 3. Predicted track total: **316 of 337**.

## The regression surface, named in advance

**35 words documents hold a nested DrawingML group with a non-zero `a:off` that this reader
reaches, and 29 of them currently match.** Every one of them moves shapes. The ones close enough to
the band for that to cost a verdict:

| document | words | band | slack |
|---|---:|---:|---:|
| `078_Storyboard_Template_Pink_and_Gray_Theme` | 51/54 | 3 | **0** |
| `002_Free_Genogram_Diagram_Template_Customizable_Format` | 88/86 | 3 | 1 |
| `026_Unit_Circle_Chart_Four_Quadrants` | 98/99 | 3 | 2 |
| `Press release_EUREKA labels ITEA 3 Cluster` | 827/813 | 16 | 2 |
| `003_Free_Genogram_Diagram_Template_Easy_Format` | 27/29 | 3 | 1 |
| `TE.CAO.00125 … OJT Logbook` | 2790/2793 | 55 | 52 |

The other 23 are at ±0 or have double-figure slack.

## What this census cannot see

- **Page counts.** A group's *envelope* carries the anchor's wrap and a member carries
  `TextWrap.Through`, so moving members should not move a single line of body text — but that is a
  reading of the code, not a measurement, and `docs-quality-MA.IMS.00001` (26 nested groups, 155
  pages) is where it would show. The sweep is the test.
- **The other two families.** `DocxFrames` is `XRay.Office.WordProcessing`, so nothing outside words
  can be reached — but `PageFrame`, `FrameLayout` and `GroupOffset` are consumed by the shared
  layout, and this changes only what is put into them.
- **Whether the missing shapes were dropped or merely coincident.** `056` draws 4 of its 20 nested
  leaves and the other 16 land on identical rectangles; if they are being coalesced somewhere
  rather than overdrawn, the count after the fix will overshoot rather than land on 56. It lands on
  56 exactly, which is evidence but not proof, since a coincidence at four documents would be
  remarkable.
- **`.doc` and `.rtf`.** Escher groups take an entirely different path.
