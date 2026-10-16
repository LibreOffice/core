# words-vision-01 — prediction, committed before any measurement

Written after reading the brief and `check-env.sh` (LibreOffice 26.2.4.2 620(Build:2),
metric-compatible fonts all present, pdftoppm/pdftotext 26.01.0), and after reading
`look.py` and the render-comparison skill, but **before rendering a single page**.

Corpus for this track: 200 documents, **134 `.docx` and 66 `.doc`**.

## What my census cannot see

1. **The 66 `.doc` files are invisible to any XML census.** Nothing I can grep out of an
   OOXML part exists for them. Any statement of the form "N documents use feature X"
   derived from unzipping parts covers at most 134/200 and I must say so every time.
   Four previous rounds have had to say this; I am saying it in advance.
2. **A per-page ink percentage cannot be paired across a pagination difference.** For any
   document where our page count differs from the reference, the "worst page" that
   `look.py --worst` selects is comparing page *i* of two different documents. The ranking
   will therefore be contaminated at the top by pagination mismatches masquerading as
   drawing defects. I expect to have to segregate the ranking into
   (a) same-page-count documents, where the ink figure means something, and
   (b) different-page-count documents, where it does not.
3. **Ink fraction is blind to colour.** A shape filled the wrong colour, an axis with the
   right ticks in the wrong hue, a gradient flattened to a flat fill — all score near zero
   on an ink-mask comparison as long as both sides are dark-ish or both light-ish. The
   radial-gradient story in the skill is exactly this. So the ranking will systematically
   under-rank colour defects, and I will only find those by looking at pages the ranking
   says are fine.
4. **Ink fraction is blind to a shift smaller than a glyph.** A uniform sub-point drift is
   invisible; a 1 pt drift lights up every glyph edge and scores like a catastrophe.

## What I predict I will find

- **P1.** The ranking will be dominated by text-position/reflow, not by missing drawing.
  Concretely: on the top 10 pages by `|ink|%`, I predict **at least 6** will show text in
  both renderings with different line breaks or different vertical positions, rather than
  an element present on one side and absent on the other. Confidence: moderate.
- **P2.** The signed ink figure across the whole track will be **net negative but small**
  (we draw slightly less), and the negative tail will be a handful of documents, not a
  broad bias. Confidence: moderate.
- **P3.** I predict I *will* find the slides text-metrics class on words — specifically,
  pages where every line in a paragraph breaks at a different word than the reference, with
  the reference breaking **earlier** (LibreOffice's line being wider / our glyphs narrower),
  consistent with the recorded ~0.1% advance divergence and "LibreOffice kerns 19% harder".
  I predict this appears in **at least 5 of the top 20** pages. Confidence: moderate-high,
  and this is the prediction most worth being wrong about, since a shared cause across the
  slides and words tracks is the highest-value finding available.
  **Counter-prediction I must test:** if LibreOffice kerns *harder*, its lines are
  *narrower* per unit text and it should break **later**, not earlier. If I observe the
  reference breaking earlier, the "we are narrower" reading and the "LO kerns 19% harder"
  measurement point in opposite directions and one of them is wrong. I commit now to
  reporting that contradiction rather than smoothing it.
- **P4.** At least one `.doc` document will show a raster/vector image present in the
  reference and blank in ours, consistent with the standing "our `.doc` path draws no WMF
  at all" lead. Confidence: high, because that lead was inferred from bytes and has never
  been confirmed by looking at a page. If I see it, that is the confirmation the lead
  lacks. If I see no such page at all, the lead is weaker than recorded.
- **P5.** I predict **at least 3 findings on documents that currently PASS the gate**, i.e.
  they are not in the 45 failures. The brief says the passing 155 are unexamined; the
  slides precedent was 3 findings from the first 3 passing documents opened.

## What would refute the round's method

If the top of the ranking is entirely documents whose page counts already differ, the
ranking is measuring pagination and not rendering, and I must rebuild it paired by content
rather than by page index before drawing any conclusion from it.
