# Slides, round eighteen — working notes

## The base

The worktree opened **465 commits behind** `claude/paperless-odf-phase-1-rnyzcu` — the eighth
agent in a row on a stale base, and the largest gap yet. Fast-forwarded to `b7950ffd5` before
measuring anything.

## The master's `p:otherStyle` is not the rung a plain text box lands on

The round-seventeen census hands on **537 pages disagreeing on the dominant `/Tf` size by more
than one per cent**, and its cleanest cluster is `171128IPAP.pptx`: thirteen pages at
**exactly −25.00%**, ours 13.50 against the reference's 18.00.

The deck's master states `<p:otherStyle>` at `sz="1350"` on every level and its
`presentation.xml` states `<p:defaultTextStyle>` at `sz="1800"`. Slide 3 carries one plain
`p:sp` text box — not a placeholder — whose runs state no size at all. We take 13.50 from the
master's `otherStyle`; the reference takes 18.00.

### The mechanism, cited and then measured

`oox/source/ppt/pptshape.cxx:424-429` at tag **`libreoffice-24.2.7.2`** — the release that made
every reference PDF, fetched rather than read out of this checkout's 27.2-alpha, and
byte-identical between the two:

```cpp
bool isOther = !getTextBody() && sServiceName != "com.sun.star.drawing.GroupShape";
TextListStylePtr aSlideStyle = isOther ? rSlidePersist.getOtherTextStyle()
                                       : rSlidePersist.getDefaultTextStyle();
```

`isOther` requires the shape to have **no text body**. So a shape that has text never reaches
`getOtherTextStyle()` — it reaches `getDefaultTextStyle()`, which is `p:defaultTextStyle` from
`presentation.xml` (`presentationfragmenthandler.cxx:115` builds `mpTextListStyle` and passes it
into every `SlidePersist` as its default text style, lines 242, 584 and 639).

That is a *second* route to the same answer as the one already recorded in
`src/Paperless.Presentations/TODO.md`: `SlidePersist::createXShapes` pushes master text styles
into Impress's style families with `for (int i = 0; i < 4; i++)` over a switch whose `case 4` is
the style `p:otherStyle` parses into, so the loop stops one short and never applies it. The
record noted the divergence and left it unmeasured.

### The fixture that separates the three candidate answers

The existing `deck-text-style.pptx` cannot settle it: its `otherStyle` states `sz="1800"` and
LibreOffice's own fallback for a slide run is *also* 18 pt, so "otherStyle applied" and
"otherStyle ignored" predict the same number. Confirmed — the reference draws its seven boxes at
exactly 18.0000.

`tests/corpus/features/slide-other-style.pptx` separates them. Master `otherStyle` states
`sz="1200"` magenta at level 1 and `sz="1000"` magenta at level 2; `p:defaultTextStyle` states
`sz="2400"` green at level 1 and `sz="3200"` blue at level 2; the slide holds two plain
non-placeholder text boxes, one paragraph each, at levels 1 and 2, stating neither size nor
colour. Three outcomes were possible and they are distinguishable:

| If the reference draws | then |
|---|---|
| 12 pt / 10 pt magenta | the master's `otherStyle` wins — our behaviour is right |
| 24 pt / 32 pt green, blue | `p:defaultTextStyle` wins |
| 18 pt black | neither is consulted |

LibreOffice 24.2.7.2 draws, out of the PDF content stream:

```
q 0 0.5019607843 0 rg   BT 46.46 473.074 Td /F1 24.009 Tf …    (#008000)
q 0 0 1 rg              BT 46.46 315.468 Td /F1 32.003 Tf …    (#0000FF)
```

and we drew `1 0 1 rg` at 11.9906 and 10.0063 — magenta, the master's `otherStyle`, on both
counts. The size *and* the colour move together, which is what says the whole rung is wrong
rather than one property of it.

### Reach

`count-otherstyle.py` over the slides track: of **112** pptx-family decks, **110** state a
`p:otherStyle` and **110** a `p:defaultTextStyle`, and on **33** the two disagree about level
one's `sz`. That is an upper bound on the documents this can move and not an estimate of it —
the rung only decides a run that states no size anywhere nearer, and most text boxes state
their own. Reach is reported below as documents whose rendering actually changed.

## The round, swept whole twice

163 documents each time, both against checksummed CLI snapshots of this worktree, with the
snapshot verified to move a document I knew the change moved before the run started
(`171128IPAP.pptx`, 19 differing pages to 7). 163 rows, no path twice, 0 `ref-failed`, in both.

| | baseline `b7950ffd5` | after |
|---|---|---|
| word gate | 151 / 163 | **151 / 163** |
| signed `ink%` | 1406.70 | **1360.08** |
| unsigned `\|ink\|%` | 1752.00 | **1680.10** |
| major pages | 462 | **438** |

**The baseline reproduced all four of round seventeen's post-fix figures to the digit** —
151/163, 1406.70, 1752.00, 462 — and its per-batch split with them: full parity in 001–007,
011, 013 and 015, with 008 9/10, 009 9/10, 010 8/10, 012 8/10, 014 7/10, 016 8/10 and 017 4/5.
No batch moved.

27 documents moved, every one of them a `.pptx` — which is the change's own scope, since
`PptxTextStyles` serves no other reader. On `|ink|%`: **25 better, 2 worse, 136 unchanged;
72.26 won against 0.36 lost.** The two against are `NAS-Infrastructure-Roadmaps-HSI.pptx`
(3.28 → 3.50) and `Sean Monogue.pptx` (1.54 → 1.68).

Five documents moved on the parity row and no verdict changed. `3492.pptx` goes 1273 words to
**1274 against the reference's 1274**, and `Wildlife for REDAC September 11.pptx` goes 9
embedded faces to **11 against the reference's 11**.

The biggest single movers, `|ink|%` before → after:

| document | | major |
|---|---|---|
| `171128IPAP.pptx` | 50.58 → **17.40** | 18 → 6 |
| `ghgp-supply-chain-initiative_20100323_wri.pptx` | 28.79 → **20.92** | 10 → 7 |
| `Reporting_responsibilities_matrix.pptx` | 74.55 → **69.80** | 6 → 6 |
| `iris07.12.12.pptx` | 5.30 → **0.96** | 1 → 0 |
| `Wildlife for REDAC September 11.pptx` | 59.58 → **56.29** | 10 → 6 |
| `BasicMed_AME_Presentation.pptx` | 4.25 → **1.08** | 2 → 0 |
| `ANG C-1 ATC-TO Core Program…pptx` | 4.10 → **1.43** | 0 → 0 |

`Reporting_responsibilities_matrix` is the round's illustration of why the two ink columns are
both reported: its **signed** figure *rises*, 49.45 → 55.03, while its unsigned falls. Filling a
deficit uncancels a surplus elsewhere in the signed sum and has nowhere to hide in the unsigned
one.

## The size census, before and after — and it needed two corrections first

Re-run whole on both sweeps (`base-size-census.txt` is the round-seventeen tool's output on the
baseline; `base-census2.txt` and `after-census2.txt` are the refined one's).

**The em fix of round seventeen did what it was predicted to do.** Its census ran over 129 of
163 documents and reported 1642 differing pages of which 1043 were the ≤1% quantisation band.
Run whole on this round's baseline — 163 documents, so a *larger* sample — the total is **711**
and the ≤1% band is **25**. The class is gone.

That left 686 pages over 1%, and round seventeen's summary counts 537 of them as "both sides
drawing text". Two of those are not a resolution defect at all, and together they are nearly
half the total.

### `census-split.py`: a reference size off the 1/100 mm grid is metafile text

A slide's character height lives in an `SvxFontHeightItem` in the model's map unit, so every
size the reference resolved *through the shape model* is `n × 72/2540`. One off that grid was
played out of a metafile or rasterised. **135 of the 686 are that**, over 45 documents — and it
re-ranks the census: `NAS-Infrastructure-Roadmaps-v16.0`, first at 42 pages, is 26 metafile
against 16 model, and `2014BSA_Sunday_Killion`, second at 29, is 18 against 11. `2014BSA`'s
reference draws **208 operators at 11.400 pt** on its page 6 against our 45 at 14.980, and
11.400 pt is not a size the draw layer can hold.

### `size-census-2.py`: the dominant size is decided by an operator count

The census weights each size by show operators, which is right in a wall of body text and wrong
on a slide whose body text is sparse. `ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt`
page 3 is reported as *ours 20.010 x7 against ref 10.010 x8, +99.90%* — the largest single ratio
class in the whole run — and the two distributions are:

```
ours  [(31.02, 2), (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 5)]
ref   [           (28.01, 1), (20.01, 7), (15.99, 3), (10.01, 8)]
```

Every size agrees. The reference splits the slide-number footer into four portions at
x = 635.6, 639.1, 645.8 and 649.3 where we write one at 638.7, and those three operators move
its dominant size from 20 pt to 10 pt. On that deck 15 census pages collapse to **5**. Five
decks of that family carry the class.

### What the residue actually is

| | baseline | after |
|---|---|---|
| pages compared | 4199 | 4199 |
| dominant size agrees | 3488 | **3519** |
| differs by ≤1% — the mm100 grid | 25 | 25 |
| differs, same sizes cut up differently | 128 | 133 |
| differs, the reference draws far less — the raster ceiling | 65 | 62 |
| differs, its size is off the mm100 grid — metafile text | 135 | 135 |
| **differs, and none of those explains it** | **358** over 99 docs | **325** over 96 docs |

So the track's size residue is **325 pages, not 537**, and `171128IPAP` has left the list
outright (it was 16). What remains is startlingly homogeneous: of the twenty commonest
(ours, ref) pairs, thirteen are **one point apart** — 18.99↔20.01, 24.01↔22.99, 27.01↔28.01,
17.01↔18.00, 20.01↔18.99, 15.00↔15.99, 18.00↔18.99, 25.99↔25.00, 22.00↔22.99 — about 92 pages
between them. That is the autofit search landing one grid step away, which round seventeen
pinned on `2015-Civil-Rights-Website-training.ppt` page 21 to a **16 mm100 window, 0.45 pt,
0.14% of the block**, and attributed to the *heights measured* rather than to the search.

## Test counts

Per project, each run redirected to its own file, on the final tree: Core 243, Text 237,
Containers 109, Vector 291, Rendering 104, Markup 259, OpenDocument 125, WordProcessing 608,
Spreadsheets 432, Presentations 516, Fidelity 542. **Zero failed and zero skipped throughout.**

Nothing outside `Paperless.Presentations/Ooxml` was touched, so the words and sheets tracks are
not owed a sweep.
