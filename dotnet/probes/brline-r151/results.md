# Round 151 — a break-only line takes the break run's face, not the neighbour's (seat O91)

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
Claims are marked **[src]** (the C++ checkout, 27.2.0.0.alpha0+, *not* the reference binary's
source) or **[bin]** (26.2.4.2's own output). The measurement-only round that built these arms is
`probes/brline-r149/`; it could measure nothing on our side, because the parent's build was live
throughout, and it said so instead of reporting unattributable figures. **This is the render it
specified, with the tree quiescent.**

---

## 1. What the discriminator answered

`probes/brline-r149`'s banked `measured3.txt` / `measured4.txt` against our own output, read by
that round's own `read3.py` so one instrument reads both halves (`discriminate.py`,
`discriminate.txt`). Before the change:

| family | what varies | 26.2.4.2 | ours |
|---|---|---|---|
| `b02 … b80` | **the break run's size** | 1.150 … 46.550, a 1.164-em slope | **flat 12.800** |
| `m002 … m120` | the paragraph **mark's** size | flat 2.350 | flat 12.800 |
| `n0 … n6` | 0..6 breaks | 2.350 each | 12.800 each |
| `pr259-n0 … n4` | the same at 108 % spacing | 2.500–3.350 each | 13.800 each |

Three things fall out, and two of them correct the seat:

- **Ours agreed at exactly one arm, `b22` — which is 11 pt, the paragraph's own size.** So the break
  line was taking *the paragraph's* face and size, not the run's.
- **The paragraph mark is not consulted, by either side.** The seat said *"a `<w:br/>` line takes the
  PARAGRAPH MARK's size"*; `m*` is flat at every one of six sizes on both sides, so the mark half of
  that is refuted twice over.
- **The proportional surplus is applied once, correctly.** `pr259-n*` gives 13.800 = 12.800 × 1.08,
  not 14.800. Round 149's hypothesis (b) — *the 108 % surplus applied twice* — is **refuted**.

## 2. The seat, found by instrumenting rather than by reading

Round 149 read the DOCX path statically and concluded it already carried the break run's size —
`DocxLayoutSource.Emit` appends the U+2028 under `_runProperties`, `size != paragraph.Size` sets
`varies`, `HoldsNothingButAnchors` tests U+0001 only. **All of that is correct**, and a scratch
probe confirms it: for `b04` the frame paragraph's `PageRun`s are `[0,1) em=2.00` and
`[1,4) em=11.00`, and for `b40` `[0,1) em=20.00`. The reader is right.

**The run is lost one layer down, in `MeasuredParagraph.Measure`.** `TextItemiser.IsFormatControl`
cuts U+2028 out of every sub-run, so a `FormattedRun` whose every character is a break yields **no
sub-run at all** and never becomes a `MeasuredRun`. For `b04` the measured run list is one entry,
`[1,4) em=11.00` — the neighbour.

That mechanism was already known and already half-repaired: the `measured.Count == 0` fallback
handles a paragraph whose *whole* text is breaks (round 52's `words-r52` probe). What it did not
handle is a break run **beside** ordinary text, which is the ordinary case. `MeasureLine` then found
no run covering the break-only line, fell through to its last resort — *"the empty-paragraph rule,
which takes the first run's metrics"* — and took `_runs[0]`, the neighbour. Hence a flat 12.800.

## 3. The change

One branch in `MeasuredParagraph.Measure`: a `FormattedRun` that produced no sub-run is kept as a
`MeasuredRun` **with its own range** and `ShapedText.Empty`. It then reaches `Fold` on `touches` or
`contains`, adds no advance, and draws no glyph.

**This overturns a deliberate choice of an earlier round, and the earlier round named this exact
change as the hazard.** `ControlOnlyParagraphHeightTests.TheKeptRunIsInvisibleToDrawingAndToTheFold`
asserted the kept run is zero length, remarking that giving it a range *"would put it in the fold —
where it would be right by accident here and wrong the moment a real run sat beside it"*. A real run
beside it is the ordinary case, and the measurement says the fold is where it belongs. The test is
rewritten to the measured rule and keeps the two properties the old form was really protecting: no
shaped glyphs, no advance.

## 4. [bin] 70 of 70 arms

After the change, every arm round 149 banked agrees within 0.05 pt — **70 of 70**, with no arm
unrendered. That covers the thirteen break sizes, the six mark sizes, four faces (DejaVu Sans,
FreeSerif, Liberation Sans, Liberation Mono, each with its own `hhea`), the four line-spacing rules
(`auto` at 240/259/480, `exact`, `atLeast`), the descender arms, the tab arms, and the **page body**
as well as a shape — so the rule is not a shape rule and the 132 breaks in 28 documents round 149
censused in page bodies are in scope too.

## 5. O91b does not exist, and that is round 149's finding carried forward

*"When the body then exceeds the shape this tree draws none of it"* is not a defect. 26.2.4.2 lays
an oversized body out at the positions a tall shape would give, paints it under a clip equal to the
shape's inner rectangle, and omits only lines whose **top** is already at or below the inner bottom;
`vertOverflow` and `anchor` are inert once it overflows, on eight byte-identical arms.
`FlowLayouter.Truncated` already implements exactly that. **The witness's body does not even
overflow at the reference** — 23.65 pt against 25.45 — so O91a is what created the overflow O91b was
invented to explain.

What is left of it is narrower and is recorded rather than fixed: we do not emit the reference's
clip, so a kept line straddling the shape's bottom is drawn whole where the reference cuts it.
Uncensused.

## 6. Instrument notes

**`readraw.py` does not read our PDFs.** Round 149's clip-blind content-stream reader matches
`x y Td /Fn size Tf [ … ] TJ`, which is LibreOffice's shape; ours emits `<hex>Tj` with no array, so
the reader returned nothing at all for every one of our 70 renders. `read3.py`, the `get_text`
reader, works on both and is what this round used — safe here because none of the discriminating
arms is clipped. **A reader written against one producer's output is not a reader.**

**PyMuPDF's text extraction honours clip paths**, which is why `readraw.py` exists at all: an
overflow arm read with `get_text` reports text the reference did draw as missing, and round 149's
first §3 was written from that read and said the opposite of the truth.

## 7. Confinement and reach: 33 of 1927, all four word-processing formats

Our half of the corpus and of all three converted ODF columns rendered twice under
`SOURCE_DATE_EPOCH=0`, one output directory per document. **1927 renderings a leg, 0 failures**, each
leg wholly after its own build.

| family | ext | moved | of |
|---|---|---:|---:|
| words | docx | **10** | 271 |
| words | doc | **1** | 66 |
| rtf column | rtf | **11** | 337 |
| odt column | odt | **11** | 337 |
| ods column | ods | 0 | 307 |
| sheets | xlsx / xls / xlsm | 0 | 307 |
| slides | pptx / ppt | 0 | 302 |

**33 movers, 1894 byte-identical.** This is a `XRay.Office.Text` change, so slides and sheets *could*
have moved and did not: no deck or workbook in the corpus holds a break-only run at a size of its
own. The same documents move in all four word-processing formats, which is what a shared-layer fix
should look like.

## 8. Do they get closer? 19 of 33, and the median error falls by four fifths

The 33 movers rendered through 26.2.4.2 as well, and both legs scored on **baseline position** —
vertical, because that is the axis a line height moves. Round 150's lesson applied: a metric aimed at
the wrong axis returns the same number for every document on both legs and reads as "no effect".

| | base | head |
|---|---:|---:|
| mean \|dy\| per span, median over the documents | **13.9080 pt** | **3.4348 pt** |
| closer / further / level | — | **19 / 4 / 10** |
| page counts closer / further | — | **0 / 0** |

**The seat's own witness is among the largest**: `086_Printable_Graph_Paper_Template_Gray_Theme`
54.849 → 13.049 and 42.449 → 1.901 in its two converted columns. `084_Printable_Graph_Paper_Template`
goes 19.800 → **0.0008** and 15.650 → **0.0008**, which is exact.

**The four that worsen are named rather than averaged away**, and all four are Work Breakdown or
Business Case templates — the same family as the largest winners, so a second defect in those
documents is interacting: `066_Work_Breakdown…` 13.908 → 22.243, `068669ebccb5-068_…` 21.351 →
22.539, `068_Work_Breakdown…` 2.368 → 3.055, `17ddb50d9338-097_Business_Case…` 3.253 → 3.435. None
is explained here.

**No page count moves in either direction**, so the gate cannot see this change either.
