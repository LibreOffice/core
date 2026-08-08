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

## Verifying the tests by reintroducing the bug

`SheetFontSizeDeviceTests` has 24 cases. Replacing the body of
`SnapFontSize(Length, double)` with the pre-fix `SnapFontSize(value) * scale` fails **11** of
them — every `OddPointSizesUnderAZoom` and every `FractionalSizesAreQuantisedWithoutAZoom`.
The other 13 (`Unscaled`, `EvenPointSizesAreUnmovedByTheRoundTrip`) pass under the mutation and
are kept as **drift guards**: they are what says the common case did not move.

## Batch 010 triage — it is a chart batch

The four failures at `9c5bef08c`, each rendered by both renderers and diffed page by page:

| document | pages | words | what is actually wrong |
|---|---|---|---|
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 46/46 | 4695/4808 | an OOXML chart. The reference draws its labels in **Carlito**, we draw them in **Liberation Sans** — the theme's `minorFont` is Calibri and no chart label carries a family at all |
| `EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` | 24/24 | 7825/8382 | `.xls` charts. Ours plot an axis of 0…12 where the reference plots 0…90 — the default scale of a plot with no series |
| `Template Pilot Logbook JAR-FCL V3.0.xls` | 38/38 | 1305/1610 | the same, on pages 16–18: 0…12 against 0…1400 |
| `INDEX_Digital_Transformation_Toolkits.xls` | 20/24 | 1982/1982 | the previous round's open item, not re-derived here |

So three of the four are the `.xls`/OOXML chart work, and the deficits are localised to the
pages carrying a chart rather than spread over the batch. **Batch 010 cannot be closed without
it**, which is worth saying plainly: the round that takes it should budget for the chart series
rather than expect to find a layout bug.

### The chart-label font gap, and why it is not this round's fix

`ChartLabel` (`Paperless.Core/Charts/ChartLayout.cs`) carries `Text`, `At`, `Anchor`, `Size`,
`Colour`, `Rotation`, `Stretch` — **and no family**. `SheetChart.Text` therefore calls
`SheetBandText.Shape(text, size)` with no family and gets `SheetBandText.DefaultFamily`,
"Liberation Sans", for every chart label in every workbook. `SlideChart` and `FrameChart` sit on
the same record, so this is one gap across all three families, and closing it means widening a
`Paperless.Core` type plus the measurer that sizes the labels.

**Measured reach on this track: one document.** Of the 171 sheets documents exactly one is a
zip container holding an `xl/charts/chart*.xml` part, and its theme minor face is Calibri. That
is a reach of one, which is a special case by this project's own rule, so it is recorded rather
than fixed. The reach on the slides and words tracks is not measured here and is likely larger —
whoever picks it up should measure it there before sizing the work.
