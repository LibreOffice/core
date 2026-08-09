# Slides round 39 — measured

Base commit `545550952`, verified before measuring. `SOURCE_DATE_EPOCH=1700000000` throughout.
Reference PDFs are round twenty's kept set, re-verified last round; nothing in this round runs
`soffice` over the corpus, so the reference cannot have moved under the comparison.

## The two sweeps

Both sides rendered with a snapshotted CLI (checksummed against the tree), compared against the
same reference PDFs with the *same* copy of `pdf-image-diff.py`.

| | base (`545550952`) | after the gradient fix |
|---|---:|---:|
| match | **151 / 163** | **151 / 163** |
| `unembedded` verdicts | 0 | 0 |
| `\|ink\|%` | 1493.00 | **1448.71** |
| signed `ink%` | 1181.39 | 1132.20 |
| major pages | 395 | **392** |
| renderings byte-changed | — | **2 of 163** |

`|signed| ≤ |ink|` holds on both, which is the free check on any table built from these columns.

**The inherited headline does not reproduce as what it was labelled.** The brief carried
"`|ink|%` 1185.07, 407 major pages". Measured here at the same commit: the *signed* column is
1181.39 and the unsigned one is 1493.00, with 395 major pages. 1185.07 is within four of the
signed column and 308 below the unsigned one, so the figure the track has been quoting as
`|ink|%` is the signed sum — and the ranking committed under `probes/slides-r22` ranks on the
signed column too. Two different measurements have been circulating under one name. This
round's tables state which column every number is.

## What changed, and what did not

The whole movement is one document:

| document | `\|ink\|` before | after | major |
|---|---:|---:|---|
| `Wildlife for REDAC September 11.pptx` | 56.25 | **11.97** | 6 → 3 |
| `3492.pptx` | 6.56 | 6.55 | 3 → 3 |

No verdict moved, and **no document's word count changed by a single word** — which is the
expected shape: a slide background is invisible to page count, word count and font embedding
alike.

## The reach prediction, and how badly one half of it did

Committed before the sweep: *35–55 of 163 renderings change, `|ink|%` improves by 40–70 of
which about 47 is Wildlife's three whole-page backgrounds, and 0 verdicts move.*

| | predicted | measured |
|---|---|---:|
| renderings byte-changed | 35–55 | **2** |
| `\|ink\|` improvement | 40–70 | **44.29** |
| of which Wildlife | ~47 | **44.28** |
| verdicts moved | 0 | **0** |

The magnitude was right and the breadth was wrong by more than an order of magnitude, for the
reason the skill names: **the census counted the property, not the drawing.**

- 87 of the corpus's 114 zip decks state a circle path gradient in a **theme** part, 81 of them
  with a focus outside the box. **None of those 81 changed a pixel** — the theme's third fill
  style is present in almost every Office-authored deck and is essentially never the fill a
  drawn shape resolves to here.
- 6 decks state one in a **content** part (slide, layout or master).
- 3 decks state one whose focus is a corner, and one of those three states it only in its theme.

So the clamp — the half predicted to be wide — has **no measured corpus reach at all**, and the
corner branch is the whole of the movement. The clamp is still right: probe slide 11 goes from
56.94% of pixels differing to 0.15%, and it is the difference between drawing the stock theme
gradient and drawing a flat fill. It is correct code waiting for a document, and it is pinned by
`SlideGradientPathTests` so it cannot be silently undone.

## Files

`slides-parity.tsv` is the after sweep's rows; `slides-ink-ranking.tsv` is the ranking
regenerated from it by `ink-ranking.py`, with the `mc:AlternateContent` and rasterisation
ceilings subtracted per page.

## A note on the sweep that produced these

Twenty-five documents came back `ref-failed` and one comparison came back empty because the
sweep's copy of the reference directory was deleted underneath it during a disk emergency
(the container reached 426 MB free with three agents running). Their renderings were already
on disk, so the rows and the comparisons were **spliced** — recomputed for those documents
alone against the kept reference set — rather than the sweep being repeated. The verdict
`ref-failed` on a document that renders fine when re-run is exactly the infrastructure
signature the skill describes, and it arrived here from disk rather than from code.
