# r178 — the ink ranking was measuring colour quantisation, and could not see displacement

`wordsink-r174`'s scorer compared each page's **mean** darkness: `|mean(ours) − mean(reference)|`,
summed over pages. Two things are wrong with that, and both were found by following its own
ranking to the top.

## It ranked a document first for one part in 255

`Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.docx` was the **worst
passing document on the words track** at 59.56 over 148 pages, 0 MAJOR pages, `aligned` 1.00, and
**189 432 alphanumerics against the reference's 189 432 — exact**. Its page 5 has 112 spans on
both sides in the same faces at the same sizes.

The whole of it is the cell shading, one grey level:

| | dark cells | light cells |
|---|---:|---:|
| ours | **128** | **217** |
| 26.2.4.2 | **127** | **216** |

And neither renderer is wrong about the colour. The operators are
`0.502 0.502 0.502 rg` here and `0.5019607843 0.5019607843 0.5019607843 rg` there — the same byte,
written to four decimals and to ten. **The rasteriser resolves a literal at or below `v/255` to
`v − 1`**, measured on six one-literal PDFs (`quantise.py`):

```
0.502                  x255 = 128.0100000000  ->  128
0.5019607843           x255 = 127.9999999965  ->  127
0.50196078431372549    x255 = 128.0000000000  ->  127
0.851                  x255 = 217.0050000000  ->  217
0.8509803922           x255 = 217.0000000110  ->  216
0.5                    x255 = 127.5000000000  ->  127
```

So `PdfSyntax.Component`'s four decimals happen to land above the value and 26.2.4.2's ten land
at or below it, and **every shaded area the two render differs by one level**. Writing more digits
would reproduce the reference's raster by reproducing an artefact of its own decimal output — that
is making the output worse to make a metric agree, so the change is in the **metric**.

## And it was blind to ink that moved

A block drawn in the wrong place leaves a page's mean darkness untouched, so the old measure could
not see displacement at all — which is most of what a layout defect is.

## What the scorer does now

`rank.py` compares the two rasters **pixel for pixel** with a one-level dead band and reports it as
`pixelD`, keeping `meanD` beside it for continuity with the stored figures. The dead band is the
quantisation above; the pixel comparison is what sees displacement.

It re-sorts the table completely, and `Annex-10` leaves the top ten outright:

| | `meanD` | `pixelD` | per page | MAJOR | aligned |
|---|---:|---:|---:|---:|---:|
| `SPA-02_mcar_part-2_and_IS_v2.9.docx` | 34.90 | **1128.68** | 4.243 | 165 of 266 | 0.81 |
| `150-5370-10H.docx` | 45.65 | 552.63 | 0.760 | 45 of 727 | 0.91 |
| `AC-150-5370-10G-updated-201604.docx` | 49.16 | 516.50 | 0.742 | 42 of 696 | 0.92 |
| `02_mcar_part-2_and_IS_v2.10.docx` | 25.57 | 223.28 | 0.716 | 64 of 312 | 0.99 |
| `FRE-03_mcar_part-3_and_IS_v2.9.docx` | 8.17 | 208.56 | 2.744 | 29 of 76 | 0.92 |
| `Annex-10-…-GCAA.docx` | **59.56** | **out of the top ten** | | | |

`ranking-pixelD.tsv` is the whole table on the new measure; `ranking-meanD.tsv` is the old one, so
the two are comparable row by row.

**The new head is not another offset**, checked before trusting it: `SPA-02_mcar` is 266 pages on
both sides and each of its 51 low-scoring pages best-matches the reference's page of the *same*
index, at ratios of 0.65 to 0.87. The difference is inside the pages.
