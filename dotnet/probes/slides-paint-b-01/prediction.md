# slides-paint-b-01 — prediction, committed before measuring

Written and committed before running the metric probe, before any census, before any render,
and before reading any of the implementation sites named in the brief beyond the line numbers
the brief itself quotes. Nothing here is edited afterwards; the results file records what
actually happened beside it.

Baseline for this round: `wt-paint-b` off `75fe89e67cb`, built **0 warnings, 0 errors**.
Reference `soffice` **26.2.4.2** 620(Build:2), `check-env.sh` green on all five checks
(Calibri→Carlito, Cambria→Caladea, Arial→Liberation Sans, Times→Liberation Serif,
Courier→Liberation Mono, DejaVu Sans→DejaVu Sans; pdftoppm and pdftotext 26.01.0).

---

## 1. The blocker — will one probe separate the two line-height sums?

**Prediction: yes, and I do not need to author a `.ppt` to do it.**

The rule at `vcl/source/outdev/text.cxx:394-407` lives in **vcl at draw time**, not in any
importer. An importer's only contribution is setting the `Shadowed` flag on the `vcl::Font`.
So any format that can state "this run is shadowed" exercises the identical code. ODF states it
as `fo:text-shadow`, which `xmloff` maps to `CharShadowed`. **I predict a flat ODP/FODP
authored with `fo:text-shadow` and Liberation Sans reproduces the same `nOff` ladder as the two
`.ppt` decks**, and that this is a legitimate substitute for authoring a PPT binary because the
divergence point is downstream of both readers. If the ladder does *not* match the three known
`.ppt` offsets at 32.00 / 33.99 / 38.01 pt, the substitution is invalid and I abandon it and say
so rather than fitting to it.

**Prediction of the separating size.** `nOff = 1 + ((L−24)/24)` with C truncation steps at
`L = 24k`. The two candidate factors are 1.1172 (hhea sum) and 1.088 (the OS/2 sum the previous
round names); their spread is `0.292·S` device units at 10 units per point. A multiple of 24
falls inside that spread first at **S = 48 pt**, where hhea gives L≈536 → `nOff` 22 → **2.2 pt**
and the OS/2 candidate gives L≈522 → `nOff` 21 → **2.1 pt**. I will sweep a ladder of sizes
rather than trusting one, and require the winning factor to reproduce **all three** previously
measured `.ppt` offsets as well as the probe's.

**Prediction of the winner: hhea, the 1.1172 sum.** vcl's `ImplFontMetricData` prefers the
horizontal header's ascent/descent and only falls back to OS/2, and Liberation Sans does not set
the `USE_TYPO_METRICS` bit in `fsSelection`. So `mnLineHeight = mnAscent + mnDescent` off hhea.

**What the probe cannot see, named in advance.** It cannot tell me whether `mnLineHeight` is
`ascent + descent` rounded once or each rounded separately, because for these sizes the two
agree; and it cannot see whether a font that *does* set `USE_TYPO_METRICS` takes a different
branch, because Liberation Sans does not. Both stay inferred unless a second font separates
them, which I will attempt with DejaVu Sans.

**If it does not separate them I ship nothing on item 1** and the results file says so in one
sentence with the measurements that failed to separate.

## 2. Escher 263 and `a:clrChange`

**Prediction: 263 is one fix with two visible halves, exactly as briefed — and I predict the
brief's second half is the part most likely to be wrong.** "`DrawShadow` declines because a
PNG's alpha is not visible at that layer" is a claim about a guard I have not read. I predict
that supplying alpha is *necessary* and I am **not** confident it is *sufficient*: the guard may
key on the container format (`is this blip a JPEG`) rather than on measured opacity, in which
case a PNG stays excluded no matter what alpha it carries and the shadow needs a second, smaller
change. I will read the guard before claiming the halves are one fix, and I expect to have to
amend the brief here.

**Prediction on `a:clrChange`: it is NOT the same shape underneath, and I will say so.** Escher
263 is a *knockout* — one stated colour becomes transparent. `a:clrChange` is a colour
*substitution*, `clrFrom` → `clrTo`, and only degenerates to a knockout when `clrTo` carries
`a:alpha val="0"`. They share a "match a colour in the decoded raster, with tolerance" core and
differ in what they write back. I predict the shared core is worth factoring and the `clrChange`
half is **not** landed this round unless the census shows the alpha-0 degenerate case dominating
its 19 decks. I predict it does not dominate.

**Prediction on the ±9 tolerance.** Inferred from a 27.2 tree that is not the reference binary.
I predict the ODP round trip's 51 361 / 67 332 count is reproducible with an exact match as well
as with ±9, i.e. the count alone does not discriminate, and that I will have to authorise ±9 by a
probe image carrying near-white pixels or leave it inferred. I predict I leave it inferred and
implement ±9 anyway, because being wrong costs a fringe pixel and being right costs nothing.

**Prediction on direction: ink added to ours on `1-secretariat` page 1**, both halves — the globe
showing through where we paint white, and a grey silhouette that we currently do not draw at all.
This agrees with the previous round and against its own predecessor.

## 3. The two underline causes

**Prediction: the hyperlink fix is the larger risk of the round and it is a regression risk, not
a correctness risk.** The importer snippet sets the `hlink` scheme colour on the character
properties **unconditionally**, guarded only by whether the *hyperlink property map* — not the
run — already carries `CharColor`. If that reading is right, a run that states its own
`a:solidFill` has that colour **overridden** by the theme's `hlink`. `Stakeholders` page 13 is
one confirming instance (file `#0070C0`, reference `#0000FF`). I predict this generalises, and I
predict it is the claim in the brief most likely to cost a regression if I take it on trust: I
will verify it on a second deck with a differently coloured hyperlink run before landing it.

**Prediction of net effect across the 41 decks: improvement, but not uniformly.** Underlines and
`hlink` colour arriving on 297 runs will move some pages toward the reference and some away,
because a deck whose `a:hlink` scheme colour happens to match what we already draw gains only the
rule. I predict at least one deck gets visibly *worse* by ink percentage while being *more*
correct, and I predict that is the gradient-line deck rather than a hyperlink deck.

**Prediction on `PptxSlideLayout.Pen`: the fix is to stop discarding the whole `a:ln` when its
paint is not a `solidFill`, and to resolve a `gradFill` line to its first stop.** The reference
draws `#C60C30`, which is both `pos=0` and `pos=59000`. **My census cannot distinguish "first
stop" from "the stop covering the majority of the run" on this deck**, because they are the same
colour here. I will find a second deck among the eight whose gradient stops differ at pos 0 to
decide it, and if none exists I will implement first-stop and record the ambiguity rather than
hide it.

## 4. Verdict movement — and I am deliberately *not* predicting a flat zero

The brief says to expect zero and to say so plainly. I predict **zero movement on page
exactness** — slides is 163/163, a shadow, an alpha and a stroke add no pages, and nothing here
touches pagination. That half I am confident in.

**The word column is a different matter and I predict it can move.** The reference draws the
`.ppt` character shadow as a *second real text record*, not as a raster — the previous round
measured two text records where we emit one. `pdftotext` has no reason to suppress a duplicated
run, so **the reference's extractable word count on a shadowed deck plausibly counts every
shadowed run twice**, and ours counts it once. If so, 36 decks and 843 runs are sitting under the
gate's 2%+3 band from below for a reason nobody has attributed, and drawing the shadow moves them
up. I predict **between 0 and +3 decks move on the word check, in our favour, and none move
against us**. This is the prediction I most expect to be wrong, and it is worth being wrong in
public because no round has checked it.

## 5. What my census cannot see, named before it runs

- **A stated feature is not a drawn feature.** A shadowed run in an empty master placeholder, a
  263 whose colour never occurs in the artwork, a hyperlink on an off-slide shape and a
  `gradFill` line on a hidden connector all count and draw nothing. Every count is an upper
  bound; only the deck-level counts are sound.
- **`.ppt` incremental save keeps superseded `OfficeArtFOPT` records**, so an instance count over
  a record walk over-counts shapes. Named by the previous round; it still applies to mine.
- **A record walk cannot see z-order**, so it cannot tell me whether a knocked-out logo reveals a
  photograph or reveals the slide background, which is what decides the sign of the ink change.
- **Neither census nor ink percentage can tell a missing shadow from a shadow drawn in the wrong
  place.** Only looking at the page can, which is why every fix below gets a before-and-after
  read, not a before-and-after number.

## 6. Explicitly out of scope, recorded not fixed

`pres_ioc_phuket`'s gradient-filled WordArt band, which the reference clips to glyph outlines and
we paint as a solid rectangle. Its transparency does not reproduce and is not a defect. I predict
I touch neither.
