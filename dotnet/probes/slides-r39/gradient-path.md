# What LibreOffice does with `a:gradFill/a:path`

Measured against the installed reference binary, **LibreOffice 24.2.7.2 420(Build:2)**, by
authoring `gradient-path-probe.py`'s deck and reading soffice's *own* flat-ODF export of it —
not inferred from `oox/source/drawingml/fillproperties.cxx` in this checkout, which is a
development branch and **disagrees with the binary on the whole of rule 2 below**.

Reproduce:

```sh
python3 dotnet/probes/slides-r39/gradient-path-probe.py <any deck with >= 18 slides> /abs/gp.pptx
soffice --headless --convert-to fodp --outdir /abs/rt /abs/gp.pptx
# then read draw:style / draw:angle / draw:cx / draw:cy off each page's drawing-page style
```

## The rule

1. **The focus is the centre of `a:fillToRect`, in whole per cent, clamped to `[0, 100]`.**

   ```
   cx% = clamp(trunc(((100000 + l - r) / 2) / 1000), 0, 100)
   cy% = clamp(trunc(((100000 + t - b) / 2) / 1000), 0, 100)
   ```

   The clamp is `getLimitedValue<sal_Int16, sal_Int32>` in `fillproperties.cxx`:532-537 and it
   is the half of this the checkout and the binary agree on. The truncation to whole per cent
   is real and observable: `l="0" t="0" r="99000" b="99000"` is a centre of 0.5%, and the
   binary treats it as **0**, which crosses the corner test below; `r="98000" b="98000"` is
   1% and does not.

2. **`path="circle"` with the focus on a corner is imported as a *linear* gradient**, not a
   radial one — where "on a corner" means `cx ∈ {0, 100}` **and** `cy ∈ {0, 100}` after the
   clamp. One axis at a limit is not enough: `(50, 0)`, `(0, 50)`, `(100, 50)` and `(100, 99)`
   are all radial.

   | focus | `draw:style` | `draw:angle` |
   |---|---|---|
   | (100, 100) bottom-right | linear | 45deg |
   | (100, 0) top-right | linear | 135deg |
   | (0, 0) top-left | linear | 225deg |
   | (0, 100) bottom-left | linear | 315deg |
   | anything else | radial, `cx`/`cy` as computed | — |

   The ramp runs along the box's diagonal with **DrawingML stop 0 at the focus corner** and the
   last stop at the opposite one, which is the same stop convention the radial case already
   has. A linear gradient's extent is `w·|cos| + h·|sin|`, and at 45° the two corners project
   exactly onto its two ends, so the diagonal is spanned exactly on a non-square box too.

3. **`path="rect"` and `path="shape"`** are `rectangular` at `(cx%, cy%)` with `draw:angle="90deg"`,
   corner or not.

## Why it matters

`(50000, -80000, 50000, 180000)` — the gradient fill in the **stock Office theme**, carried by
79 of the corpus's 114 zip-container decks — has an unclamped centre 80% of the box *above* its
top edge. Clamped it is `(50, 0)`: a glow at the top edge fading down. Unclamped, the whole box
falls past the last stop and the fill comes out **flat**. That is one line of arithmetic and it
is the difference between a gradient and no gradient.

## What this does *not* settle

The rectangular gradient's own geometry. LibreOffice's `initRectGradientInfo`
(`basegfx/source/tools/gradienttools.cxx`) scales the texture by the box's width *and* height
and then applies the 90° rotation of rule 3, which transposes the two;
`SlideGradients.Centred` uses `width / 2` for both axes and no rotation. Probe slide 15 of the
first case list (`path="rect"`, bottom-right focus) differs 58.81% of pixels on that account
alone. Left unfixed: three corpus decks state a `rect` path and none of them is near the top of
the ink ranking.
