# Where an inline drawing's ink sits, and why it is not where its text sits

*Measured 2026-09-06 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used throughout and they agree on every figure below except where the table says
otherwise: the distro's **24.2.7.2** at `/usr/bin/soffice`, and the TDF tarball's **26.2.4.2** at
`/opt/libreoffice26.2` with its bundled Latin faces moved aside (`fc-match "DejaVu Sans"` resolves to
DejaVu and `fc-match Calibri` to Carlito). X-Ray.Office at `agent/words-draw2`, base `e152bc0b2`.*

## Why this exists

`probes/words-inline-effectextent/results.md` closed the horizontal half of `wp:effectExtent` and
left the vertical half open, with the reason stated plainly: *"a frame here is one object and cannot
be in two places"*. The reference paints a shape's **fill** at the outer top plus the top extent and
lays its text box's **text** out at the outer top; one rectangle can satisfy one of those.

That round's fixtures vary the *gap* between two text lines, which measures the line box, and its
horizontal fixtures read ink *columns*. Neither reads the drawing's own band **and** the run inside
its text box down the page at once, which is the one measurement the disagreement needs. This probe
does, on four kinds of drawing rather than one.

## The measurement

`makeprobe.py` builds one-page fixtures: a 12 pt `TOPLINE`, a paragraph holding one 144 × 50.4 pt
inline drawing, a 12 pt `BOTLINE`. `measure.py` reports, in PDF points from the page top, the two
text lines, the rows carrying the drawing's fill, and the `INSIDE` run of a `wps:txbx` where the
fixture has one.

`pl-*` is a plain black `wps:wsp`; `tb-*` the same shape carrying a centred `INSIDE` run in a
`wps:txbx`; `ov-*` an `ellipse` preset, so the *geometry* is read rather than the frame rectangle;
`px-*` a `pic:pic` that keeps its shape by declaring an `a:outerShdw`, which
`words-inline-effectextent` establishes is the one picture case that takes the extent at all.

| fixture | `wp:effectExtent` | ref fill top | ref `INSIDE` | ours before | ours after |
|---|---|---:|---:|---:|---:|
| `pl-ee0` | — | 85.75 | — | 85.75 | 85.75 |
| `pl-ee27432` | 2.16 pt, all four | **88.00** | — | 85.75 | **88.00** |
| `pl-ee91440` | 7.2 pt, all four | **93.00** | — | 85.75 | **93.00** |
| `pl-ee137160` | 10.8 pt, all four | **96.50** | — | 85.75 | **96.50** |
| `pl-t-only` | `t` 10.8 pt | **96.50** | — | 85.75 | **96.50** |
| `pl-b-only` | `b` 10.8 pt | 85.75 | — | 85.75 | 85.75 |
| `tb-ee0` | — | 85.75 | 104.66 | 85.75 / 104.61 | 85.75 / 104.61 |
| `tb-ee137160` | 10.8 pt, all four | **96.75** | **104.66** | 85.75 / 104.61 | **96.50** / 104.61 |
| `tb-t-only` | `t` 10.8 pt | **96.75** | **104.66** | 85.75 / 104.61 | **96.50** / 104.61 |
| `tb-b-only` | `b` 10.8 pt | 85.75 | 104.66 | 85.75 / 104.61 | 85.75 / 104.61 |
| `ov-ee0` | — | 92.00 | — | 92.00 | 92.00 |
| `ov-t-only` | `t` 10.8 pt | **102.75** | — | 92.00 | **102.75** |
| `px-ee0` | — | 85.75 | — | 85.75 | 85.75 |
| `px-t-only` | `t` 10.8 pt | **96.50** | — | 85.75 | **96.50** |

Five things it settles:

1. **Only the top edge moves the ink.** `pl-b-only` states a 10.8 pt *bottom* extent and its fill
   band starts at 85.75, exactly where the zero-extent control's does. The line still grows by the
   bottom edge — `BOTLINE` moves from 136.76 to 147.56 — so the two are independent, as the earlier
   probe already showed for the line box.
2. **The move is the top extent, to the raster's own quantum.** 2.16 → 88.00 (85.75 + 2.16 = 87.91,
   read at 288 dpi in 0.25 pt steps), 7.2 → 93.00, 10.8 → 96.50.
3. **The text of a `wps:txbx` does not move at all**, at any extent, top or bottom: 104.66 on all
   four `tb-*` rows on both references. That is the disagreement, and it is *only* vertical —
   `words-inline-effectextent/make-x-fixture.py` shows a left extent moving the fill band and the
   `INSIDE` run together.
4. **It is the drawing and not the rectangle.** An `ellipse` preset's curves move by the same
   amount, so a fix that moved the frame's box and left the geometry behind would not reproduce it.
5. **A picture that keeps its shape takes it too.** `px-t-only` moves by the same 10.75; the plain
   picture case never reaches the margin code at all and is unaffected either way.

The two references differ on exactly two cells and neither is the quantity under test: `px-ee0`'s
fill *bottom* reads 138.50 on 24.2 against 136.25 on 26.2, and `px-t-only`'s 149.25 against 147.00.
That is the `a:outerShdw` the fixture declares, which 24.2 draws two and a quarter points further
down; the band's **top** — the measurement — agrees to the hundredth on both rows.

The residual 0.05 pt on `INSIDE` and on `BOTLINE`, and the 0.25 pt on `tb-*`'s fill top, are the
pre-existing one-twip text offset and the raster quantum. Both are on the zero-extent controls and
neither moved.

## The change

`PlacedFrame` now carries **two** rectangles. `Area` is where the frame's *text* was laid out and is
what `FrameLayout.HangInline` computes exactly as before; `Ink` is where its fill, outline, preset
geometry, picture and chart are painted, and is `Area` moved down by `PageFrame.InlineInkOffset` —
the top effect extent, or nothing for a turned drawing, whose two rectangles are centred in one
another instead.

`SwAsCharAnchoredObjectPosition::CalcPosition`
(`sw/source/core/objectpositioning/ascharanchoredobjectposition.cxx`:129-133) moves the anchor point
by both the left and the upper spacing; `SwTextBoxHelper` then fails to carry the vertical move to
the TextBox fly, which is what the `INSIDE` column above is measuring. **We reproduce that rather
than correcting it**: our text already lands where the reference's does, and moving it would create
a divergence rather than close one. Recorded so that a later round reading this as an oversight has
the reason. What is genuinely unmodelled is Word's own placement, where both halves would move.

`DocxFontwork.Inset` is gone, and its removal is part of the same change rather than a tidy-up. A
warped body has no text box left — the importer clears it at `WpsContext.cxx:985` — so the previous
round moved *its* curves by the top extent at read time, as a special case for the one drawing kind
that needed it. Those curves become the frame's `FillOutline`, which is now placed against `Ink`, so
keeping the pre-shift would double it.

## On the corpus document

`words/drawingset-001/docx/WordArt_Shapes_Arrows_Catalog1.docx`, 340 inline `wps:wsp` in three
extent groups — 182 at 2.16 pt, 59 at 7.2 pt and 99 at 10.8 pt, the last of these the warped WordArt
that already carried the offset. `verdict.py` at 150 dpi, against **both** references, which give
the same figures to within one shifted tile:

| | before | after |
|---|---:|---:|
| pages / words | 52 / 1972 | 52 / 1972 (references 52 / 1972) |
| pages reading `dy != 0` | **27** | **1** |
| total \|dy\|, px | **164** | **1** |
| pages reading `dx != 0` | 0 | 0 |
| pages verdict `match` | 6 | **9** |
| total worst-tile error | 7.93 / 7.96 | **4.00 / 3.99** |
| total shifted tiles | 4748 / 4754 | **1451 / 1449** |

The `dy` column is the one this closes, and the group sizes are visible in it: pages 23-45 read 4-5
px before (2.16 pt is 4.5 px at 150 dpi) and pages 47-52 read 15-16 px (7.2 pt is 15 px). Page 48
keeps a single pixel. Page 7's `dx` stays 0, and page 46 is now `localised` with no shifted tile at
all, which is a different and smaller thing than it was.

## What this does *not* explain, and was tested first

The brief that opened this round proposed that the catalogue's `dy` was the **empty
running-head paragraph** filed at `probes/words-margin-print-area/results.md` §4 — an empty header
paragraph 1.90 pt shorter than Writer's, and 4 px at 150 dpi is 1.92 pt. It is not, and the document
settles it three ways:

- `word/header1.xml` is the only blank header, and `word/document.xml`'s single `w:sectPr` references
  it as `w:type="even"`. `word/settings.xml` states no `w:evenAndOddHeaders`, so **no page in the
  document uses it** — nor does the reference.
- Measured off both PDFs with `pdftotext -bbox` on pages 3, 7, 23, 24 and 30, the header run
  `EDITABLE` sits at `y` **35.38** in ours and in the reference, and the first body word at
  **72.01**. The header frame is the right height on every page checked.
- The `dy`-bearing pages' own label text is at most **0.35 pt** out and accumulates 0.05 pt per row,
  which is a fifth of the quantity and the wrong shape for a header defect.

The 1.90 pt shortfall is real and is still open; its reach is a document with an empty paragraph in
a *running head*, which this one has not got. See the report for the classification.

## Reproducing

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 makeprobe.py /abs/scratch/sifx
python3 measure.py   /abs/scratch/sifx /abs/scratch/siout
```

`measure.py` needs Pillow and `pdftotext`, and renders every fixture through `/usr/bin/soffice`,
`/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`, so the version question is re-checked on
every run rather than taken from this file. `makeprobe.py` writes its own 8×8 PNG with `struct` and
`zlib`, so the fixture half needs no image library.

**The band detector's threshold is a measurement of the page, not a constant.** The first cut
required a quarter of the page's width of dark pixels in a row, copied from
`words-margin-print-area/measure.py`, whose band is 200 pt wide; this fixture's drawing is 144 pt on
A4's 595, which is 24.2%, and every row came back `0.00`. A threshold that finds nothing looks
exactly like a shape that is not drawn.
