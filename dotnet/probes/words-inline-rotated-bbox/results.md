# A *turned* inline drawing's box on its line

*Measured 2026-09-06 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used and they agree on every figure below to the raster quantum: the distro's
**24.2.7.2** at `/usr/bin/soffice` and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with
its bundled Latin faces moved aside. X-Ray.Office at `agent/draw-inline`, base `6bf527227`.*

## Why this exists

`probes/words-inline-effectextent/results.md` closed with an open defect: a rotated inline picture
grows its line by **+46.25 pt with a `137160` extent and by +46.25 pt with no extent at all**, so the
growth is "the rotated bounding box, which we do not yet size a rotated inline drawing by". It was
deliberately separated from the extent rule and left for its own round. This is that round, and the
bounding box turns out to be only half of the rule.

## The rule, from the source

A rotation takes the other branch of `nOOXAngle == 0` in `GraphicImport.cxx`:1055-1090, and that
branch keeps **two** rectangles:

- **Word's box** — the stated `wp:extent`, with `lcl_doMSOWidthHeightSwap` applied
  (`GraphicImport.cxx`:533-548: swap width and height about the centre when the angle, truncated to
  whole degrees and taken modulo 180, lands in `[45, 135)`), then expanded on each side by the
  `wp:effectExtent`;
- **LibreOffice's snap rectangle** — the bounding box of the turned drawing.

Each margin is set to the signed gap between the two:

```cpp
m_nLeftMargin  += aLOSnapRect.X - aMSOBaseLeftTop.X;
m_nRightMargin += aMSOBaseLeftTop.X + aMSOBaseSize.Width - (aLOSnapRect.X + aLOSnapRect.Width);
m_nTopMargin   += aLOSnapRect.Y - aMSOBaseLeftTop.Y;
m_nBottomMargin += aMSOBaseLeftTop.Y + aMSOBaseSize.Height - (aLOSnapRect.Y + aLOSnapRect.Height);
```

The horizontal margins keep their sign; the vertical ones are clamped at nought twenty lines further
on (`GraphicImport.cxx`:1245-1249, *"FixMe: tdf#141880 LibreOffice cannot handle negative vertical
margins"*). The object plus its margins is what `SwFlyCntPortion` hangs on the line, so:

> **across, Word's box; down, the larger of Word's box and the turned one** — and the snap rectangle
> is centred in whichever it is, in both axes.

## The measurement

`makeprobe.py` builds a landscape page reading `LEFT` + drawing + `RIGHT` between a `TOPLINE` and a
`BOTLINE`, holding one **black rectangle** 144 × 50.4 pt at a stated `a:xfrm/@rot`. A rectangle
rather than a picture because its ink *is* its snap rectangle. Every text run is set to `0x909090`
so `measure.py` can separate the drawing's ink from the line's by threshold alone — the first cut of
that instrument thresholded at black and returned the whole line's width as the drawing's.

`gap` is `BOTLINE` less `TOPLINE`, so the room the drawing takes down the page is `gap` less the
66.85 of the zero-degree control plus its own 50.4. `adv` is `RIGHT` less `LEFT`, so the room across
is `adv` less the 5.87 the two spaces take.

Both references, in PDF points. `ours` before this round in brackets:

| fixture | gap | room down | inkL | inkW | adv | room across |
|---|---:|---:|---:|---:|---:|---:|
| `rot000` — control | 66.85 | 50.40 | 103.50 | 144.00 | 149.87 | 144.00 |
| `rot020` | 113.05 *(66.80)* | **96.60** | 99.25 *(99.50)* | 152.25 | 149.87 *(150.00)* | 144.00 |
| `rot020-ee` — plus `137160` | 113.05 *(66.80)* | **96.60** | 110.00 *(99.50)* | 152.50 | 171.47 *(150.00)* | **165.60** |
| `rot045` | 160.45 *(66.80)* | **144.00** | 60.00 *(107.00)* | 137.25 | 56.27 *(150.00)* | **50.40** |
| `rot090` | 160.45 *(66.80)* | **144.00** | 103.50 *(150.50)* | 50.25 | 56.27 *(150.00)* | **50.40** |
| `rot135` | 153.90 *(66.80)* | **137.45** | 106.75 *(107.00)* | 137.25 | 149.87 *(150.00)* | 144.00 |
| `rot315` | 153.90 *(66.80)* | **137.45** | 106.75 *(107.00)* | 137.25 | 149.87 *(150.00)* | 144.00 |
| `sq-rot020` — 144 × 144 | 201.00 *(160.40)* | **184.55** | 83.25 *(83.50)* | 184.25 | 149.87 *(150.00)* | 144.00 |

Five things it settles:

1. **The height is not simply the turned bounding box.** At 20° it is (96.61 turned against 50.4
   stated). At **45° it is not**: the turned box is 137.46 square and both references take
   **144.00**, which is the *swapped* stated height and nothing else. Only a rule that keeps both
   rectangles produces that, and it is why the 45° row is the one to keep.
2. **45° swaps and 135° does not.** `[45, 135)` is half-open, so an oblong behaves differently at the
   two ends: 45° reads 50.40 × 144.00 and 135° reads 144.00 × 137.45, from the same drawing.
   315° behaves as 135° (315 mod 180 = 135).
3. **The effect extent is *not* inert on a turned drawing** — which is what the earlier probe
   concluded, from a fixture that varied only the vertical gap. `rot020-ee` has the same gap as
   `rot020` to the hundredth **and an advance 21.60 pt larger**: the extent grows Word's box in both
   axes, and downwards the turned height simply happens to be the larger of the two and swallows it.
   The vertical-only fixture could not have seen this.
4. **The drawing is centred in the box, not corner-aligned.** `rot045`'s ink starts at **60.00 pt** on
   a line that starts at 103.50 and inside a page margin at 72: its 137.25 pt turned box centred in a
   50.40 pt reservation hangs 43 pt into the margin, and both references draw it there.
5. **`sq-rot020` is the control on the swap.** A square has no width/height to swap, so its turned
   box (184.57) is the answer at every angle, and it is.

## After

The same table, ours against both references:

| fixture | gap | inkTop | inkBot | inkL | inkW | adv |
|---|---:|---:|---:|---:|---:|---:|
| `rot020` | 113.00 / 113.05 | 86.00 / 86.00 | 182.25 / 182.25 | 99.50 / 99.25 | 152.25 / 152.25 | 150.00 / 149.87 |
| `rot020-ee` | 113.00 / 113.05 | 86.00 / 86.00 | 182.25 / 182.25 | 110.25 / 110.00 | 152.50 / 152.50 | 171.60 / 171.47 |
| `rot045` | 160.40 / 160.45 | 89.25 / 89.25 | 226.50 / 226.50 | 60.25 / 60.00 | 137.25 / 137.25 | 56.40 / 56.27 |
| `rot090` | 160.40 / 160.45 | 85.75 / 85.75 | 229.75 / 229.75 | 103.75 / 103.50 | 50.25 / 50.25 | 56.40 / 56.27 |
| `rot135` | 153.85 / 153.90 | 86.00 / 85.75 | 223.25 / 223.00 | 107.00 / 106.75 | 137.25 / 137.25 | 150.00 / 149.87 |
| `rot315` | 153.85 / 153.90 | 86.00 / 86.00 | 223.25 / 223.25 | 107.00 / 106.75 | 137.25 / 137.25 | 150.00 / 149.87 |
| `sq-rot020` | 200.95 / 201.00 | 86.00 / 85.75 | 270.25 / 270.25 | 83.50 / 83.25 | 184.25 / 184.25 | 150.00 / 149.87 |

Every row agrees to the 0.25 pt raster quantum and the 0.05–0.13 pt advance divergence that the
zero-degree control already carries — see `dotnet/CLAUDE.md` on that divergence, which is not this.

## Reproducing

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 makeprobe.py /abs/scratch/rot
python3 measure.py   /abs/scratch/rot /abs/scratch/rotout
```

`measure.py` needs Pillow and `pdftotext`, and renders each fixture through `/usr/bin/soffice`,
`/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`.
