# Every variant this round ran, and what 26.2.4.2 answered

All heights are `style:row-height` in twips, read out of 26.2.4.2's own `--convert-to fods` by
`fodsrows.py`. Nothing here was rendered. `variant.sh <in.fods> <tag> <sheet>` is the driver; it
converts a flat ODF back through the reference and prints the resulting row heights.

## The document — `sistem-rekod-markah-srm-_-rekod-master.ods`, sheet `6A`

The base is 26.2.4.2's own `fods` of the packaged `.ods`, so every variant differs from the
reference's own view of the file by exactly the edit named.

| tag | the one edit | rows 0–1 | rows 3–42 | rows 43–51 |
|---|---|---:|---:|---:|
| `p0` | none (the round-trip control) | 276.0 | **298.2** | 276.0 |
| `p1` | `style:parent-style-name="Default"` deleted from `ConditionalStyle_2` | 276.0 | 298.2 | 276.0 |
| `p2` | `fo:font-size="9pt"` added to `ConditionalStyle_2`'s own text properties | 276.0 | **276.0** | 276.0 |
| `p3` | `ConditionalStyle_2` re-parented to a new empty parentless style | 276.0 | 298.2 | 276.0 |
| `p4` | the `Default` cell style's three font sizes 11 pt → 20 pt | 489.3 | **522.1** | 489.3 |
| `p5` | `p3`'s intermediate style given `fo:font-size="7pt"` | 276.0 | **276.0** | 276.0 |
| `p6` | every `cell-content()<40` and `calcext:value="<40"` → `-1000` | 276.0 | **276.0** | 276.0 |
| `p7` | the same conditions → `>-1000` | 276.0 | 298.2 | 276.0 |

`p1` and `p3` both come back from the round trip with `style:parent-style-name="Default"` written
onto the style again, which is the importer re-parenting a parentless cell style to `Default`; that
is why neither moves. `p5` is the variant that shows the search really is a walk — the size comes
from an *ancestor* the file did not have before.

`p6` and `p7` are the pair that decide the seat. Under `<-1000` nothing in the range satisfies the
condition and the rows fall to the arithmetic; under `>-1000` everything numeric does and they stay
measured. The cells that satisfy `<40` are the six `#N/A` columns per row: an error formula result
is the number zero.

## The minimal probe — `mkprobe.py`, sheet `Probe`

Two rows, one plain cell and one carrying a `style:map`, in the same cell style otherwise.
`mkprobe.py <out.fods> <default pt> <cell pt> <conditional pt or -> <cell face> <default face>`.

| tag | Default | cell | the conditional cell holds | condition | plain row | conditional row |
|---|---|---|---|---|---:|---:|
| `q1` | Calibri 20 | Calibri 11 | `"E"` | `<40` | 276.0 | **298.2** |
| `q2` | Calibri 40 | Arial 9 | `"E"` | `<40` | 256.3 | **256.3** |
| `q3` | Calibri 40 | Arial 9 | `"E"`, plus a `calcext:conditional-formats` block | `<40` | 256.3 | 256.3 |
| `q4` | Calibri 40 | Arial 9 | the number `5` | `<40` | 256.3 | **984.8** |
| `q5` | Calibri 40 | Arial 9 | a formula returning `#N/A` | `<40` | 256.3 | **984.8** |
| `q6` | Calibri 40 | Arial 9 | the number `5` | `>1000` | 256.3 | 256.3 |

`q1` is round 95's `fontsource.fods` reproduced: 276 against 298.2 under a 20 pt `Default` with an
11 pt cell, which that round read as *the conditional cell is measured in its own font*. `q2`
separates the two candidate answers by 31 points and shows that reading is right **only because
the condition does not hold** — a string cell against a numeric operand is false outright
(`IsValidStr`, `conditio.cxx`:1156-1183). `q4` and `q5` fire and answer one line of the
**default's** 40 pt in a cell whose own font is 9 pt. `q6` is the control: the same numeric cell
under a condition that cannot hold.

`q3` also settles a candidate that looked plausible and is not the discriminator: adding the
extension-namespace block beside the `style:map` changes nothing, because both spellings import
into one `ScConditionalFormat`.

## The fixture

`tests/corpus/features/sheet-conditional-font-source.fods` is `q2`, `q4` and `q5` in one file with
a plain control row, and 26.2.4.2 answers **256.3, 256.3, 984.8, 984.8**. Those four numbers were
read before the test was written.
