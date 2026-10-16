# Prediction — `slides/batch-008` round 1

Written and committed **before** rendering our own side of
`slides/batch-008/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`.

## What I already knew when writing this

The brief's gate row: `pages 26/26  words 2120/2010  verdict: words` — we draw **+110**.

From the banked reference alone (no X-Ray.Office render yet):

- `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/8_P-Pavese_…__pptx.pdf`, 26 pages,
  2010 letter-or-digit words / **2199 raw** `pdftotext | wc -w`.
- `pdfimages -list` on the reference shows rasters on pages 1, 2, 3, **5, 6**, **8, 9**, 11.
  The interesting ones are the repeats: object **207** (692×240 JPEG + soft mask) drawn on
  **pages 5 and 6**, and object **566** (859×234, no mask) drawn on **pages 8 and 9**.
- Reference per-page raw words: p5 = 70, p6 = 169, p8 = 32, p9 = 54, p16 = 200.
- `dotnet/raster-ceiling-pages.tsv` flags exactly **one** page of this document — page 5,
  ours 114 raw against ref 70 raw.

**Correction to the brief up front.** The brief says the document "appears in the table at
line ~123 with 5 flagged pages". It does not: the `5` in that row is the **page number**
column. There is one flagged page, page 5. I expect this to matter, because it means the
under-count the ceiling file warns about is proportionally larger here than the brief assumes.

## Predictions

1. **Page 5 and page 6 are both the rasterisation ceiling**, on the same object 207.
   Combined raw excess **80–95** (the ceiling file records +44 and +44). In the gate's
   letter-or-digit metric I expect somewhat less, **70–85**, because a rasterised diagram's
   text is label-heavy and some tokens are punctuation-only. — *confidence: high*

2. **Pages 8 and 9 will also prove to be the same class** — object 566 is drawn twice, on two
   consecutive pages, exactly the signature of pages 5/6, and the reference extracts only
   32 and 54 raw words there. I expect us to draw text and not the raster, for a further
   **+20 to +60** combined. If so, the flagged-page list for this document should grow from
   1 to 4, not to 2. — *confidence: medium*. The alternative is that we draw the same
   859×234 raster ourselves (condition 2 fails) and these pages are neutral.

3. **Page 16 is not this class.** The ceiling file names it at "+23%" and calls it "a
   different defect entirely", then never says what. Reference p16 is 200 raw words and
   carries **no raster at all**. I predict a residue of **+20 to +50** here that is *not*
   raster and is a genuine candidate defect. — *confidence: medium-low on the size, high on
   "not raster"*.

4. **The split.** Of the 110 letter-or-digit word excess I predict:
   - **75–95 raster-ceiling** (pages 5, 6, and probably 8, 9)
   - **15–35 residue**, most of it on page 16, the rest single-digit noise spread over
     pages the gate cannot resolve.
   - *confidence: medium*.

5. **The residue will NOT be the `words/batch-008` ligature shape.** That defect needed a
   run carrying `w:spacing`/tracking and a ligature-forming Latin face; it showed as our
   one-character token count exploding. I predict our one-character token count on this
   document is **within 1.5× of the reference's**, and our multi-character `ToUnicode`
   entry count is **≤ 2 more** than the reference's. — *confidence: medium-high*.

6. **No code change this round.** I predict I will not find a fix worth making, and the
   round ends with rows added to `TODO.raster-ceiling.md`. — *confidence: medium-high*.

7. **Even excusing every raster page, the document stays outside the 2% band.** The band is
   ±(2% of 2010 + 3) = ±43.2. If the raster ceiling is 75–95, the residue 15–35 is *inside*
   the band, so the document would pass on a corrected gate. I predict the corrected figure
   lands **inside** the band — which contradicts the ceiling file's own worked example, which
   says excusing the listed page leaves it "still outside". That sentence was written when only
   page 5 was excused. — *confidence: medium*.

## Baselines I predict I will reproduce

- `XRay.Office.Fidelity.Tests`: **30 failed of 550**.
- `slides/batch-001`–`007`: 66/68, failures `solog_orientation_august_2019` and
  `architecture6`.
- `slides/batch-008`: 4 of 5 matching, this document the only failure.
  — *confidence: high on the first two (briefed), medium on the third (not briefed)*.

## How I will score this

Each numbered item gets hit / partial / miss, with the measured number beside it, in
`results.md`. Item 2 is the one that decides whether this round adds one row or three.
