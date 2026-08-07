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
