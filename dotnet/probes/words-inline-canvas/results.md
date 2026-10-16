# An inline drawing canvas, and where its members go

*Measured 2026-09-06 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used and they agree on every figure below to the last digit: the distro's **24.2.7.2**
at `/usr/bin/soffice` and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with its bundled
Latin faces moved aside. XRay.Office at `agent/words-draw2`, base `e152bc0b2`.*

## Why this exists

`wpc:wpc` is a drawing **canvas** — what Word writes when a user draws several shapes on a canvas
rather than grouping them. `DocxFrames.Group` has taken one since it was written, and
`DocxFrames.Members` flattens it into an envelope plus one frame per leaf, each carrying
`PageFrame.GroupSize` (the envelope's rectangle) and `PageFrame.GroupOffset` (where the member sits
inside it).

`FrameLayout.Placed`, which positions an **anchored** drawing, adds both. `FrameLayout.HangInline`,
which hangs an **as-character** one on its line, added neither — so every member of an inline group
or canvas was drawn at the drawing's own top-left corner, all of them on the same spot, and only the
last one painted was visible.

Censused over the words corpus by walking every `word/*.xml` and asking which of `wp:inline` /
`wp:anchor` most recently opened before each element:

| | count | documents |
|---|---:|---|
| `wpc:wpc`, all of them inline | **9** | `docs-quality-MA.IMS.00001…` (6), `ABCD-FE-01-00 Flight Envelope` (1), `ABCD-WB-08-00 Weight and Balance Report` (1), `DOA_Template_Form_Type_Certification_Programme` (1) |
| `wpg:wgp` inline | **28** | `docs-quality-MA.IMS.00001…` (27), `OM template for non-complex NCC operators` (1) |

## The measurement

`makeprobe.py` builds two one-page fixtures, each a `TOPLINE`, an inline drawing 4 × 2 in, and a
`BOTLINE`. The drawing holds three 1 × 0.5 in rectangles stepped diagonally — red at (0, 0), green at
(1.5 in, 0.5 in), blue at (3 in, 1 in) — so no two share a row or a column. `canvas.docx` wraps them
in a `wpc:wpc`, which states no child space of its own; `group.docx` in a `wpg:wgp` stating
`a:chOff`/`a:chExt` equal to its `a:ext`.

Each member has its own primary colour, so `measure.py` reads its box straight out of the raster and
nothing has to be paired. Boxes in PDF points at 150 dpi:

| fixture | member | 24.2 / 26.2 | ours, before | ours, after |
|---|---|---|---|---|
| `canvas` | red | (72.00, 85.92)–(144.00, 121.44) | **absent — overdrawn** | (72.00, 85.92)–(144.00, **121.92**) |
| | green | (180.00, 121.92)–(252.00, 157.44) | **absent — overdrawn** | (180.00, 121.92)–(252.00, **157.92**) |
| | blue | (288.00, 157.92)–(360.00, 193.44) | **(72.00, 193.92)–(144.00, 229.92)** | (288.00, 157.92)–(360.00, **193.92**) |
| `group` | red | (72.00, 85.92)–(144.00, 133.44) | **absent — overdrawn** | (72.00, 85.92)–(144.00, **133.92**) |
| | green | (180.00, 133.92)–(252.00, 181.92) | **absent — overdrawn** | (180.00, 133.92)–(252.00, 181.92) |
| | blue | (288.00, 181.92)–(360.00, 229.44) | **(72.00, 181.92)–(144.00, 229.92)** | (288.00, 181.92)–(360.00, **229.92**) |

Three things it settles:

1. **The members were piled**, and the pile is visible as two of the three colours being absent from
   the page altogether: they are under the third. The one that survives is the last painted.
2. **Every member is now within one 150 dpi pixel (0.48 pt) of both references**, and the residual is
   on the bottom edge only, which is the antialiased last row of the fill.
3. **A canvas is not scaled and a group is.** The canvas's members keep their stated 0.5 in height
   (35.52 pt in the raster); the group's come out 47.52 pt, because its members cover only three
   quarters of its `a:chExt` down the page and `DocxFrames.Fit` resizes them onto the anchor's extent
   — which both references do too, to the pixel. That is `Members`' existing
   `if (group.Name.LocalName is "wgp")` guard, and the fixtures confirm it rather than change it.

## On the corpus document

`words/pagination-002/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx`, six
canvases and twenty-seven inline groups, 44 pages against both references' 44. One-sided ink per
page at 150 dpi — the share of a page where exactly one of the two has ink:

| page | vs 24.2 before → after | vs 26.2 before → after |
|---:|---|---|
| 9 | 14.5226 → **8.2223** | 14.3555 → **8.0563** |
| 17 | 4.0171 → **3.4706** | 5.5282 → **5.0238** |
| 21 | 5.4819 → **5.2762** | 4.4216 → **4.2165** |
| 23 | 12.6432 → **7.5943** | 12.4985 → **7.4498** |
| 42 | 6.3275 → **5.2650** | 5.8914 → **4.8282** |
| 11 | 12.1148 → **12.8843** | 11.9644 → **12.7342** |
| all 44 | 290.94 → **278.54** | 285.70 → **273.34** |

**Page 11 is worse by 0.77 and it is not a regression** — it is `words-inline-effectextent`'s warning
about one-sided ink arriving again. Pairing every fill on that page by size and colour:

| | dx | dy |
|---|---:|---:|
| before | −0.65, −138.05, −215.15, −282.15 | −3.09, −67.24, −201.99 |
| after | **+0.05 on every one** | **+17.56 to +17.66 on every one** |

So after the change the drawing's members are in the right places relative to each other and to the
page across, and the whole drawing sits 17.6 pt high. That 17.6 pt is the page's, not the drawing's:
`pdftotext -bbox` puts page 11's *body text* at the same −17.65 pt from the third paragraph onwards,
and the page carries 286 words against the reference's 290 — one line short, which is the standing
advance divergence at the top of `dotnet/CLAUDE.md`. A correctly-shaped drawing on a page displaced
by a line scores more one-sided ink than a degenerate pile did, and the pile is not the better
rendering.

## Reproducing

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 makeprobe.py /abs/scratch/cvfx
python3 measure.py   /abs/scratch/cvfx /abs/scratch/cvout
```

`measure.py` needs Pillow, and renders every fixture through `/usr/bin/soffice`,
`/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`, so the version question is re-checked on
every run rather than taken from this file.
