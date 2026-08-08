# Sheets round twenty-three — probe data

Base commit `9c5bef08c`. Reference renderer LibreOffice 24.2.7.2.

## The drawn-font-size class, measured rather than guessed

`mkfods.py <out.fods> <scale-pct>` writes a flat-ODS probe: one column of cells, one font
size each, the sheet printed at a stated scale. `ref-font-sizes.tsv` holds every `Tf`
operand LibreOffice's own PDF export writes for the probes below, in page order.

| probe | sizes | print scale |
|---|---|---|
| `scale100 … scale25` | 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 36, 48 pt | 100, 90, 75, 60, 50, 33, 25 % |
| `fine100`, `fine75` | 8.00 → 10.00 pt in 0.05 pt steps | 100, 75 % |

### What the reference actually computes

`178 of 178` observations across eight documents are reproduced exactly by

```
h    = round(twips * 127 / 72)          # 1/100 mm — this is SheetDeviceUnits.SnapFontSize
p    = round(h * scale * 720 / 2540)    # integer device pixels, PDF reference device at 720 dpi
L    = round(p * 2540 / (720 * scale))  # back to logical 1/100 mm *through the scaled map mode*
size = L * scale                        # what reaches the page
```

every `round` being half-away-from-zero. The 720 dpi is measured, not assumed: at 100 % the
emitted sizes step in units of 2540/720 = 3.5278 hundredths of a millimetre and nothing else.

Two consequences worth stating separately because they are easy to conflate:

- **At scale 100 % the round trip is the identity for every whole-point size from 6 to 48**,
  which is why this class only ever showed up on sheets printed at a zoom. It is *not* the
  identity in general — 8.25 pt (291 hundredths) comes back as 289.
- **Under a zoom it is the identity for even whole-point sizes and not for odd ones.** At
  75 %, 6/8/10/12/14 pt land exactly on `h * scale`; 7/9/11/13 pt land 1.5 hundredths away.
  9 pt and 11 pt being the two commonest sizes in a spreadsheet is the whole of why a
  "constant per-document ratio" was the shape this was first seen in.

The predecessor's two established facts both survive and one of its framings does not: the
reference's 6.803 pt is 240 hundredths, and 240 is *not* reachable by quantising 9 pt to the
1/100 mm grid in either order. It is `round(round(238.5 * 720/2540) * 2540/540) * 0.75`
= `round(68 * 4.7037) * 0.75` = `320 * 0.75`. The em never sits on the 1/100 mm grid at all
under a zoom; the *unscaled* logical value does.
