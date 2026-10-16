# The last eight warps — measured, not transcribed

Round `agent/fontwork2`. Environment: this container, `soffice` = 24.2.7.2,
`/opt/libreoffice26.2/program/soffice` = 26.2.4.2.

## Why they were left out, and what changed

The previous round implemented thirty-two of `ST_TextShapeType`'s forty warps and left eight,
on a rule worth keeping: *a table transcribed for a preset no document states is a transcription
nothing checks.* The corpus states none of the eight — censused over every OOXML container, the
`docx` side holds 2103 `textNoShape`, 5 `textPlain` and the catalogue's 24, and the two decks that
bend anything hold ten arches.

What changed is not the reach. It is that a fixture now exists that checks them:
`fontwork-presets-default.docx` and `fontwork-presets-adjusted.docx`, one shape per
`ST_TextShapeType` value authored into `WordArt_Shapes_Arrows_Catalog1.docx`'s own container, with
reference PDFs from both binaries. The generator is `gen40b.py` beside this file.

## What the eight actually needed, which is not what was expected

The standing note said the four `*Pour` and two `textRing*` shapes "are drawn with `ANGLEELLIPSE`
and a radius handle rather than the four opcodes `FontworkGeometry` decodes, so they would need a
second path builder". Reading the tables, that is true of **one** of the six.

| preset | segment programme | needed |
|---|---|---|
| `fontwork-arch-up-pour` | `0xA504 0x8000 0xA504 0x8000` | its table |
| `fontwork-arch-down-pour` | `0xA304 0x8000 0xA304 0x8000` | its table |
| `fontwork-circle-pour` | `0xA504 0x8000 0xA504 0x8000` | its table |
| `fontwork-open-circle-pour` | `0xA504 … 0x4000 0x0001 … 0xA304 …` | its table |
| `mso-spt142` (`textRingInside`) | `0xa604 0xa504 0x8000` ×2 | its table |
| `mso-spt143` (`textRingOutside`) | `0xA203 0x8000` ×2 | **`ANGLEELLIPSE`** |
| `mso-spt166` / `mso-spt167` | `0x4000 0x0001 / 0x2002` | their tables |

A pour is two concentric arcs with the text fitted into the ring between them — an ordinary
even-rail envelope, drawn with the same `0xA304`/`0xA504` the arch family already used. The radius
handle is an *adjustment*, `adj2`, and `Fontwork.Adjustments` already halved it
(`oox/source/drawingml/fontworkhelpers.cxx:135-141`) for presets that could not use it yet.

`mso-spt143` genuinely needed the new opcode. `ANGLEELLIPSE` is `0xA2` with a count in thirds
(`svx/source/svdraw/svdoashp.cxx:124-133`) and three vertex pairs — centre, radii, angles — and the
arm of `EnhancedCustomShape2d.cxx:2178-2286` it takes is the `bIsFromBinaryImport` one, where the
second angle is a **swing** rather than an end and both are negated to convert the orientation.
`mso-spt143` is also the shape the reference names explicitly at line 2255 as the one whose angles
are plain degrees where every other binary user of the opcode states 1/65536ths of one.

## Measured

Nine pages, 100 dpi mean absolute grey difference against both references. The two arms of the
fixture score identically, for a reason worth recording: **all 24 adjustment values the catalogue
states are equal to the preset's own default** — `textArchUp` states `10800000` in 1/60000 degree,
which is 180, and the `mso_sptTextArchUpCurve` default is 180; `textCanUp` states 85648, which is
18500 at 0.216, and the default is 18500; and so on for the other 22. So the "adjusted" arm is the
"default" arm with the defaults written out, and both references agree with that to 0.01 per page.

| | mean, 9 pages | p1 | p2 | p3 | p4 | p5 | p6 | p7 | p8 | p9 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| before, against 26.2.4.2 | 2.584 | 0.70 | 7.47 | 4.01 | 5.38 | 0.61 | 0.57 | 3.88 | 0.57 | 0.06 |
| + `mso-spt166`/`167` | 2.211 | 0.70 | 7.47 | 4.01 | 5.38 | 0.61 | 0.57 | **0.52** | 0.57 | 0.06 |
| + the four pours and `mso-spt142` | 0.965 | 0.70 | 4.18 | **0.62** | **0.84** | 0.61 | 0.57 | 0.52 | 0.57 | 0.06 |
| + `mso-spt143` | **0.603** | 0.70 | **0.93** | 0.62 | 0.84 | 0.61 | 0.57 | 0.52 | 0.57 | 0.06 |

Against 24.2.7.2 the final mean is **0.603** as well, page for page within 0.06.

Extracted words are 228 on both sides throughout, and 10 `WORDART` tokens against the reference's
10 — nine of them the inherited page header and the tenth `textNoShape`, which is not a fontwork
and correctly keeps its text. So no warp draws text on either side, before or after.

No page is now above **0.93**, and the residual is spread evenly across all nine rather than
concentrated on the unimplemented ones: page 1 has been 0.70 throughout and holds nothing that
changed.

## What this did not move

`WordArt_Shapes_Arrows_Catalog1.docx` renders **byte-identically** to the build before this change
— 52 pages, 2468 raw extracted words, and a maximum per-page grey difference of **0.0000** across
all 52 pages against the previous binary. It states 24 of the 40 presets and none of the eight, so
that is the expected answer and the check that the shared formula evaluator and the new
`ANGLEELLIPSE` arm did not disturb it.
