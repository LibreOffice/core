# The touching line: a frame whose top edge is exactly a line's bottom

Measured 2026-09-06, worktree `wt-frames` at `531e9a1f3`, against
`/opt/libreoffice26.2/program/soffice` (**26.2.4.2**, TDF tarball with the metric-compatible
duplicates, the Latin `NotoSans-*`/`NotoSerif-*` and `opens___.ttf` moved aside) and
`/usr/bin/soffice` (**24.2.7.2**). Corpus documents
`dotnet/tests/corpus/features/frame-{wrap,parallel}.{fodt,odt}`.

## The question

`frame-parallel.{fodt,odt}` failed `FrameComparisonTests.TextFillsBothSidesOfAFrameThatTouchesNeitherMargin`
on one line only — **line 4, the last line of the paragraph *above* the frame**, whose box bottom is
exactly the frame's top. We divided it; 26.2.4.2 does not. The frame is anchored at the top of
paragraph 2 with `svg:y="0cm"` and `style:vertical-rel="paragraph"`, so in twips the line's bottom and
the frame's top are both **2210** — page margin 1134 plus four lines of 269.

The round before this one recorded that the reference's answer at that tie turns on the frame's
**horizontal** position, flipping between 0.9 and 1.0 cm, and stopped there because a threshold near
1 cm with no mechanism behind it is not shippable. That is reproduced here and then explained: there
is no threshold. **The reference has no rule at the tie at all.**

## 1. Away from the tie the rule is a strict overlap, in both references

Six documents — the two fixtures plus four cut from `frame-parallel` at
(width, x) = (2 cm, 0.5 cm), (2 cm, 0.8 cm), (4 cm, 0.95 cm), (4 cm, 1 cm) — each rendered at
`svg:y` = −0.01 cm, 0 and +0.01 cm (one hundredth of a millimetre is 0.567 twips; ±0.01 cm is
±6 twips, well clear of any rounding).

| `svg:y` | frame top vs line bottom | touching line wrapped, 26.2.4.2 |
|---|---|---|
| −0.01 cm | 6 twips of genuine overlap | **6 of 6** |
| +0.01 cm | 6 twips of genuine clearance | **0 of 6** |
| 0 | exactly equal | **3 of 6** |

The same three-way sweep at ±0.001 cm (one 1/100 mm unit, one twip) on both fixtures under **both**
binaries gives the same answer: one twip up wraps it, one twip down does not, 24.2.7.2 and 26.2.4.2
agreeing in all eight renderings. So the rule either binary implements, wherever the geometry decides
it, is `SwRect::Overlaps`' plain inclusive-rectangle test: `frameTop <= lineBottom`, with
`Bottom() == Top() + Height() − 1`, which means **an exactly touching frame does not obstruct**.

## 2. At the tie the answer moves with quantities that cannot change the vertical relation

26.2.4.2, `frame-parallel` re-cut, line 4 wrapped (`A`) or left at the margin (`N`), frame top
110.500 pt in **all twenty-one** renderings:

| width | x = 0.5 | 0.8 | 0.9 | 0.95 | 1.0 | 1.1 | 1.5 cm |
|---|---|---|---|---|---|---|---|
| 2 cm | N | A | A | A | A | A | A |
| 4 cm | A | A | A | A | N | N | N |
| 6 cm | N | N | N | N | N | N | N |

Not monotone in x, not monotone in width, and the previous round's "flip between 0.9 and 1.0 cm" is
the 4 cm row read on its own. Height does the same thing: at (4 cm, 0.95 cm) the frame's **height**
decides it — 2 cm `N`, 3 cm `A`, 5 cm `N` — and a frame's height cannot move its top edge.
`frame-wrap` (x = 0) is `N` at heights 1 and 2 cm and `A` at 3, 4, 5 and 6.

All twenty-seven documents render on one page, so no pagination is involved. The conclusion is that
the outcome at the tie is a residue of Writer's incremental layout — which pass formatted paragraph 1,
and against what the fly's rectangle then was — and not a geometric rule that can be ported.

## 3. The four spellings of `frame-wrap` disagree with each other, and the recorded reason was wrong

26.2.4.2, line 4's pens, same document in five forms:

| spelling | line 4 | why |
|---|---|---|
| `.fodt` | wrapped, 170.25 | exact tie |
| `.odt` | wrapped, 170.25 | exact tie |
| `.rtf` | wrapped, 175.80 | the RTF import supplies 0.2 cm of wrap spacing the file does not state, so the overlap is real |
| `.docx` | **at the margin, 56.80** | `wp:positionV/wp:posOffset` is **635 EMU = 1 twip**, so there is no overlap |
| `.doc` | at the margin, 56.80 | as the DOCX |

`FrameComparisonTests` recorded the DOCX difference as `ADD_VERTICAL_FLY_OFFSETS` changing the
rectangle `CalcFlyWidth` intersects. **That is refuted**: rebuilding the same DOCX with
`wp:posOffset` set to 0 and nothing else changed makes 26.2.4.2 wrap line 4 at 170.30, exactly as the
ODF forms do. The compatibility flag is still set by the OOXML import and still does other things;
it is not what separates these two renderings. One twip of stated offset is.

## What was done

* `FrameObstacles.Inflation` is applied **horizontally only**. The horizontal twip is unchanged and
  still measured — text resumes 3402 twips along from a frame 2268 wide at 1134. The vertical twip is
  removed: it was the whole of the `frame-parallel` failure and it encodes a rule the reference does
  not have.
* The four ODF fixtures are moved one hundredth of a centimetre off the tie, in the direction that
  keeps what each was written to show: `frame-wrap` to `svg:y="-0.01cm"` so its frame genuinely
  overlaps the line above and still narrows it, `frame-parallel` to `svg:y="0.01cm"` so its frame
  genuinely clears it. After the move **all four render identically under 24.2.7.2 and 26.2.4.2**,
  8 renderings of 8, where before the two references disagreed about `frame-parallel`.

A fixture sitting on that tie is asserting a value its own reference decides by layout order. The
documents were built to measure a wrap; they now measure it on both sides of a boundary instead of on
it.

## Reach

Forty corpus documents carrying an anchored frame — every `words` document with a `<wp:anchor>` or a
paragraph-anchored `draw:frame`, smallest first — rendered before and after the change and compared
line break for line break against 26.2.4.2. **None of the forty moved.** The rule only differs from
the one it replaces when a frame's edge is within a twip of a line's, and no corpus document among
those forty is on that tie. So this is a rule correction with no corpus movement behind it, which is
also why the two synthetic fixtures are the only place it can be seen.

`reach-set.txt` is the list.
