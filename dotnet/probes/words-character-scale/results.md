# `w:rPr/w:w`: the character width, and the twip it lands on

*Measured 2026-09-03 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`, reference `soffice` the
distro's **24.2.7.2**. Every figure below is a fresh measurement in that environment.*

## Where this came from

`renderer-parity-sweep-01` grepped deliberately for the pattern `dotnet/CLAUDE.md` records as the
cause four times over — a property read into a model and consumed by nothing — and listed `w:w`
character scaling as one of five it found. Four of the five have since landed. This is the last,
and it was not merely unconsumed: it was never read at all, so `<w:w>` appears nowhere in
`dotnet/src`.

## Reach

**1677 `<w:w>` across 20 corpus DOCX.** 237 of them say 100 and mean nothing; of the 1440 that do
not, **1226 say 99**:

```
value  99: 1226   95: 105   105: 39   103: 14   102: 10   104: 10   107: 8   106: 7
       98: 6    101: 4     96: 3    108: 2    130: 2     90: 1     86: 1   131: 1   112: 1
```

The documents carrying it are the ones this project's notes reach for whenever they discuss reflow:

```
  1100  Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule
   126  f445896eb008d14c1746fc37d412dc22.docx
   112  ESPN-R - MCF - RA - Ed1.docx
    97  ESPN-R - MCF - Manual - Ed1.0 - For Publication.docx
    59  091_Business_Case_Template_Complete_Guide_cb91a140.docx
    48  OM template for non-complex NCC operators_August 2016.docx
    25  SWDD-template.docx
    25  AWR OPS-AOC 044 Statement of Compliance RVSM  Rev 04 01 Feb 2024.docx
    ... 12 more
```

`AWR OPS-AOC 044` is the document `dotnet/CLAUDE.md`'s cascade section works through at length.

## The probe

`scale.py` writes two documents. `S_scale` sets `Hamburgefonstiv 12345` at 12 pt Liberation Serif
eight times — unscaled, then at 100, 99, 95, 90, 50, 150 and 200 per cent — each followed by a tab
to a fixed stop, so a run's width is read straight off the PDF text layer with `pdftotext -bbox`.
`S_both` crosses the width with a `w:spacing` of 40 twips.

| `w:w` | 24.2.7.2 width | ratio | ours, before | ours, after |
|---|---:|---:|---:|---:|
| *(absent)* | 83.928 | 1.00000 | 83.982 | 83.982 |
| 100 | 83.928 | 1.00000 | 83.982 | 83.982 |
| 99 | 82.879 | **0.98750** | 83.982 | 82.933 |
| 95 | 79.732 | 0.94999 | 83.982 | 79.783 |
| 90 | 75.535 | 0.90000 | 83.982 | 75.584 |
| 50 | 41.958 | 0.49993 | 83.982 | 41.991 |
| 150 | 125.892 | 1.50000 | 83.982 | 125.974 |
| 200 | 167.856 | 2.00000 | 83.982 | 167.965 |

Every row after the change is the reference's, plus the 0.054 pt this reader is already out by on
the unscaled control — scaled by the same factor, which is what says the scale itself is exact.

## The one finding that is not the obvious one

**99 per cent is 0.98750, not 0.99.** VCL's font width is a `tools::Long` in the map mode's own
unit and Writer's map mode is twips, so a 12 pt run is set at `trunc(240 × 99 / 100) = 237` twips
and drawn at 237/240. Every other value the corpus states divides 240 exactly, which is why the
error shows on that one and hides on the rest — and that one is 1226 of the 1440.

`TextWidthScale.Of` is that rule, and it takes the em size because the grid is the em's: 99 per
cent of a 10 pt run is `trunc(200 × 99 / 100) = 198` of 200, which *is* 0.99.

## Tracking is added after the squeeze, not squeezed with it

From `S_both`, all four combinations of 100/50 per cent against 0/40 twips:

| | 24.2.7.2 | ours, after |
|---|---:|---:|
| 100 %, no spacing | 83.928 | 83.982 |
| 50 %, no spacing | 41.958 | 41.991 |
| 100 %, 40 twips | 111.804 | 111.982 |
| 50 %, 40 twips | 69.900 | 69.991 |

69.900 − 41.958 = 27.942 and 111.804 − 83.928 = 27.876: the tracking costs the same whether the
glyphs are squeezed or not. So the width multiplies the face's own advance and nothing else — not
the gap a `w:spacing` puts between glyphs, and not the share a justified line hands out.

## The glyphs are squeezed, not merely moved closer

Read off the reference's own 200 dpi raster: at 50 per cent the glyphs are the same height and half
the width, and at 200 per cent twice the width. Advancing the pen by half and leaving the glyphs
their own shape would overlap every one of them with the next.

Both backends spell it directly, which is why `GlyphRun` carries the factor rather than a backend
guessing it from the advances: a PDF text matrix's `a` term, and Skia's `SKFont.ScaleX`. The
positions inside the run are already squeezed, so a consumer that only reads positions — extraction,
the extent the pen advances by — needs to know nothing about it.

Whole-page ink between the two renderings of `S_scale`, at 200 dpi: **0.117**.

## Reproducing

```sh
python3 scale.py <dir>
for f in <dir>/*.docx; do
  "$XRAY_OFFICE_CLI" render "$f" --outdir ours
  soffice --headless --convert-to pdf --outdir ref "$f"
done
pdftotext -bbox -f 1 -l 1 <pdf> - | grep '<word'
```
