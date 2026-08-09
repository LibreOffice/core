# Words round 42 — measured results against the predictions

Predictions in `prediction.md`, committed at `51c2c473b` before anything was rendered post-change.

## The scoreboard

| | baseline `4efef703e` | after | predicted |
|---|---:|---:|---|
| documents matching | 154 | **154** | 154 |
| absolute page error | 78 | **78** | 78 |
| exactly-correct page counts | 164 | **164** | 164 |
| absolute word error | 6427 | **6512** | 6427 ± a few |

**Zero verdicts moved**, which was said in advance and is the headline. Page error and exact page
counts held exactly. Word error is 85 worse, and that prediction was wrong in a way worth reading
rather than rounding off — see below.

## Reach: 37 of 200 renderings, against a predicted 18–30

Measured by rendering the whole track before and after and byte-comparing with `/CreationDate`
masked. **The prediction was low, and low for the reason the skill names.** The census could see
only the 134 DOCX; **11 of the 37 documents that changed are `.doc`**, whose header stories live in
the WW8 text stream where no zip-level census can look. On the half the census *could* see it was
accurate: 26 DOCX changed against a ceiling of 29 distinct DOCX declaring either shape.

| | census ceiling (134 DOCX) | measured |
|---|---:|---:|
| header/footer table holding a drawing | 26 | — |
| `PAGE` field inside a `txbxContent` | 11 | — |
| distinct DOCX in either population | 29 | **26** |
| `.doc`, invisible to the census | 0 | **11** |
| **total renderings changed** | | **37 of 200** |

## The word gate reads a regression, and the token multiset says otherwise

Two documents moved the word count, both in the direction the gate scores as worse. Both are
running heads that were **not being drawn at all** and now are:

| document | | under-draw | over-draw | total mismatch |
|---|---|---:|---:|---:|
| `A1. EASA Form 2.docx` | before | 275 | 129 | 404 |
| | **after** | **29** | 225 | **254** |
| `B11. TE.CAO.00129  Experience  logbook.docx` | before | 233 | 130 | 363 |
| | **after** | **46** | 174 | **220** |
| `UG.CAO.00133 … Language.docx` | before | 100 | 344 | 444 |
| | **after** | **89** | 333 | **422** |

Under-draw — text the reference draws and we do not — falls from 608 to 164 across the three, and
total mismatch from 1211 to 896. The residual over-draw is a *third* defect, described below: on
these documents we draw the running head on more pages than the reference does, so drawing its
text-box content correctly makes the surplus larger while making the page closer.

## What the round set out to do: the `box` cluster

### 1. Round 39's table reproduces cell for cell

`divergence-from-pdfs.py` re-runs `first-divergence.py`'s analysis over the PDFs the baseline sweep
already rendered — same build, no second render. Every cell of round 39's whole-track table comes
back: `box` 8 failing / 5 matching, `glyphs` 29/54, `one-sided` 5/13, `face` 0/5, `size` 3/4, no
divergent page 0/71.

### 2. Two thirds of the class is the instrument

`box-note-anatomy.py` asks the question the brief asked: **could a `size WxH vs WxH` note be
produced by something other than a rule-and-fill defect?** Two things produce it.

- **Rectangle corners.** `pdf-ops.py` anchors a non-text record at `(x0, y1)` — its top-left corner
  — and the left edge and the top edge of any rectangle share that corner exactly. Greedy
  nearest-neighbour therefore pairs our *vertical* rule against the reference's *horizontal* one at
  every table corner, and the note then reports one rule's width against another's height.
  On `UG.CAO.00133 … Language.docx` page 1 the four box notes include
  `size 0.0x36.6 vs 486.9x0.0`, `size 0.0x36.6 vs 0.6x0.0` and `size 487.1x0.0 vs 0.0x26.4`.
- **Flattened curves.** A logo arrives as dozens of hairline segments a few points long, and two
  renderers flatten a curve at slightly different sub-point positions, so every segment mismatches.

Over the 44 documents carrying any box note on their first divergent page: **439 notes, 142
cross-orientation, 146 hairline, 151 pairing two same-direction records with at least one of rule
scale.**

Re-scoring the dominant class with only the 151 survivors counted as `box`:

| dominant kind | matching, as published | failing, as published | matching, cleaned | failing, cleaned |
|---|---:|---:|---:|---:|
| `box` | 5 | 8 | **3** | **5** |
| `glyphs` | 54 | 29 | 54 | 32 |
| `one-sided` | 13 | 5 | 14 | 5 |

**The three documents the brief names as the heart of the cluster all leave it.**

- `150-5370-10H.docx` (711/721) has **236** box notes on page 1 and **not one** of them is a rule:
  90 cross-orientation and 146 hairline, all of them inside a single 12 pt × 12 pt graphic around
  (84–100, 715–717) that both renderers draw. Its real dominant note is `glyphs`.
- `UG.CAO.00133 … Language.docx` — the user's document — has four, three of them cross pairs. Its
  real dominant note is `glyphs`, and what is actually wrong with the page is a missing logo and a
  whole-page displacement.
- `UG.CAO.00006 … .docx` is the same shape: four notes, three cross.

### 3. The `box` cluster and the "rule-and-fill" cluster are one measurement, not two

The brief asked whether they are the same documents. They are the same *instrument column*, read in
two rounds: round 34's nine came from `first-divergence.py` over the 46 failures and round 39's
eight from the same tool over all 200. What the brief adds is that the cluster contains
`AC-150-5370-10G-updated-201604.docx` beside `150-5370-10H.docx`, "the same document in two
revisions" — and it does not. `AC-150-5370-10G` is `glyphs`-dominant on page 5 in round 39's own
committed TSV and in this round's re-measurement. **Two revisions of one document land in two
different classes**, which on its own says the class is a property of what the instrument happened
to count rather than of the document.

### 4. What the cluster's documents are actually wrong about

Taking the largest one apart is what produced both of this round's fixes. Neither is geometry.

- The reference draws a 42.75 pt image on page 1 of `UG.CAO.00133` and on its three landscape
  pages; we drew it nowhere. `FrameLayout.FlowsOn` walked the body's tables and not a furniture
  flow's own, so a picture anchored in a *header table cell* never had a rectangle resolved.
- The reference's footer counts 1 to 18 and ours printed `Page 1 of 18` on every page —
  **the defect the user reported**. The footer is a `wpg` group of text boxes and the `PAGE` field
  is inside one; `PageFields` descended into table cells and not into a paragraph's frames, so the
  running head also answered "does not vary" and was cached across all eighteen pages.

## Refuted: the header does not stop being inherited at a section break

`UG.CAO.00133` draws its running head on all 18 pages where the reference draws it on 5, which is
244 of its 244-word surplus and looks exactly like a link-to-previous rule we have wrong: its
sections 1, 2 and 4 each name an *even* and a *first* header and no *default* one, and
LibreOffice's flat-ODF export of the document gives all three an **empty** `<style:header>`.

`header-link-to-previous.py` authors the shapes the corpus cannot separate and **refutes it**. In
all six — a second section naming nothing, naming only a first header, only an even header, both,
both *empty* exactly as Word writes the unused slots, and a three-section document whose middle
section names two empty ones — LibreOffice puts the **first section's header on every page**. Our
rule is right and this is not why that document over-draws.

So the cause is something specific to that file that six authored shapes do not reproduce, and it
is left open rather than guessed at. The probe is committed so the next round can extend it rather
than re-derive the refutation.
