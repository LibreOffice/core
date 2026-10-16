# An as-character frame's own width is not on the line that aligns it

**Measured 2026-09-06 at `ffda5d02e` plus this round's change**, in `/home/user/wt-words67`.
Environment, stated once because a stored figure is evidence about an environment and not about the
code:

| | |
|---|---|
| ours | `XRay.Office.Cli` built from this worktree |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 0229ac93fcf0d7cb**, its eight Latin duplicate faces aside |
| ref24 | `/usr/bin/soffice` — **24.2.7.2 420(Build:2)**, which is what `batch-check.sh` measures against |
| fonts | system fontconfig: Carlito, Caladea, Liberation, DejaVu, WenQuanYi, IPAGothic |
| corpus | `/home/user/sample-files`, 947 documents |

---

## What the brief said, and what the probe found instead

The brief carried two candidate mechanisms for the logo on
`words/done-011/doc/AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc` sitting 501 pt out
horizontally, and named the second as the seat: *"`FrameObstacles.SpaceFor` already computes that
band for text; an as-character frame's line does not reach it."*

**The second half is refuted and the first half is a general defect in four of the five formats.**
`gen.py` builds four hand-made DOCX — one right-aligned picture-only paragraph, the same left
aligned, and the same with a square-wrapped floating shape beside it at two wrap distances — and
our placement already agreed with 26.2.4.2 on every one of them **before any change**:

| probe | ref26 left edge | ours before | ours after |
|---|---:|---:|---:|
| `align-only.docx` right-aligned, no frame | 394.55 | 394.50 | 394.50 |
| `left-align.docx` | 72.00 | 72.00 | 72.00 |
| `band.docx` right-aligned beside a frame at 250 | 104.55 | 104.45 | 104.45 |
| `band-dist.docx` the same with a 9 pt wrap distance | 95.55 | 95.45 | 95.45 |

So an as-character frame's line **does** reach the obstacle band, and it is narrowed correctly by
both the frame and its wrap distance. That is not the AAC defect.

## The defect is the boundary model's last boundary, and it is invisible in a DOCX

An `InlineObject` is a **boundary** in `MeasuredParagraph`'s prefix table: it widens every prefix
*past* the position it occupies and none at or before it
(`XRay.Office.Text/Layout/MeasuredParagraph.cs`:453-457). That rule is what makes a picture sit
between two characters rather than replace one, and what lets a picture too wide for the room left
on a line move to the next line whole.

**The last boundary has no prefix past it.** An object at `Offset == text.Length` therefore widens
nothing, so every width the table answers for that line is short by the object — and
`ParagraphLayouter.AlignmentOffset` is `available - line.Width`
(`XRay.Office.Text/Layout/ParagraphLayouter.cs`:952-964), so the line has *full* slack and a
right-aligned picture is drawn from the right margin **rightwards**, its own width out.

**It is invisible in a DOCX because the readers disagree about whether a picture is a character.**
OOXML puts a `U+0001` where one stands, so the object sits at boundary 0 of a one-character
paragraph and the table carries it. ODF, RTF and — for a paragraph holding nothing else — WW8 put
nothing there, so the object sits at boundary 0 of an *empty* paragraph, which is the last
boundary.

Measured: `align-only.docx` converted by 26.2.4.2 to four other formats, one right-aligned
picture-only paragraph, picture 145.5 pt wide on a text area ending at 540 pt.

| format | ref26 left edge | ours before | ours after |
|---|---:|---:|---:|
| `.docx` | 394.55 | **394.50** | 394.50 |
| `.odt` | 394.55 | **540.00** | **394.50** |
| `.doc` | 394.60 | **540.00** | **394.50** |
| `.rtf` | 394.60 | **540.00** | **394.50** |
| `.fodt` | 394.55 | **540.00** | **394.50** |

Four of five wrong by exactly the picture's width, and the fifth right by the accident of an anchor
character.

Writer has no such gap because it has no prefix table: an as-character fly is a `SwFlyCntPortion`
in the line and its width is the line's like any other portion's.

## The seat

`MeasuredParagraph` gains `_trailingObjectsEmu`, the width of the objects standing at the text's
last boundary, and `WidthBetween(start, end)` adds it for a range that *ends* at the text end and
does not *start* there — so the empty line a trailing manual break opens does not pay for the
picture on the line above it. `TextMeasurer.Fill`'s `text.Length == 0` short circuit stops
returning a hard-coded zero width and asks `widthBetween(0, 0)` instead, which is the whole of the
picture-only case.

Because the width goes through `widthBetween`, the *break candidates* carry it too, which is
Writer's behaviour: a picture that no longer fits the room left on a line moves to the next line.

## What it did to the AAC document, and what is left

`words/done-011/doc/AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc`, page 1, the logo:

| | x left | x right | y top | y bottom |
|---|---:|---:|---:|---:|
| ours before | **546.00** | 691.60 | 697.85 | 766.45 |
| ours after | **400.40** | 546.00 | 697.85 | 766.45 |
| ref26 | **44.90** | 190.40 | 697.94 | 766.44 |

Exactly the 145.60 the arithmetic predicts, and the page count is unchanged at 20/20.

**The residual 355.50 is a second, different defect, and this probe identifies it rather than
fixing it.** `aac-page1-layout.txt` is the laid-out page 1 (lines and frames) dumped from the
tree. The masthead's Word APO reaches the layout as

```
frame anchor=Paragraph off=0 area=(199.40,64.60)-(738.40,65.75) inlineExtent=539.00x1.15 wrap=Both
```

— **1.15 pt tall**. That is `MINFLY`, Word's 23-twip minimum, which
`Ww8TextFrames.Build` (`XRay.Office.WordProcessing/Ww8/Ww8TextFrame.cs`:126-128) writes into
`PageFrame.Size` with the comment *"layout gives a frame its content's height in either case"* —
and layout does not. `FrameLayout.PlaceFrames` builds the obstacle straight from
`FrameLayout.Place`'s rectangle (`XRay.Office.WordProcessing/Layout/FrameLayout.cs`:265-298), so the
masthead obstructs 1.15 pt of page instead of the ~72 pt its content occupies, and the logo's line
— at y 75.45 — sees no obstacle at all.

The arithmetic that says this is the whole of the residual: the frame's own left edge is 199.40 and
its wrap spacing is 9.00 pt, so the band left of it ends at **190.40**; a right-aligned 145.50 pt
picture in that band starts at **44.90**, which is the reference's figure to the hundredth. Our
frame is at the right x already — the height is the only thing wrong.

It is left open deliberately. Growing an auto-height frame's obstacle to its content's height
changes what every such frame narrows, and this round had no budget to sweep the corpus for the
line breaks that would move. The measurement above is what the next round should start from.

## What it moved on the corpus

The words track rendered twice, once with each binary, with `SOURCE_DATE_EPOCH` set so the two
runs are byte-comparable: **10 of 338 renderings changed, every one of them a `.doc`**, and no page
count moved on any of them. `|ink|%` against 26.2.4.2, summed over the pages
(`movers-ink.txt`; two of the ten paginate differently from 26.2 and `pdf-image-diff` rightly
refuses them):

| document | before | after |
|---|---:|---:|
| `120509coss.doc` | 8.58 | **0.38** |
| `150_5300_13_chg12.doc` | 7.35 | 8.16 |
| `150_5335_5a.doc` | 26.45 | **21.46** |
| `AAC-AD-No-2021-01….doc` | 2.40 | 2.38 |
| `PUR-0012-UTAS-GRAMS-SPA-Template….doc` | 0.28 | **0.06** |
| `RobertQ_Service.doc` | 3.78 | **2.29** |
| `SFSP_2013-02_Bulletin.doc` | 0.91 | **0.46** |
| `chapter 12.doc` | 0.06 | **0.01** |
| **total** | **49.81** | **35.20** |
| `150_5300_13_chg10.doc`, `absrc-pac-01-info-note-en.doc` | — | — |

Seven of the eight move towards the reference and one against it, and the one that moves against it
does so **entirely on its page 25** (1.04 → 1.85, every other page identical to the twip); that page
is not read here.

**The gate cannot see any of this and very nearly says so.** Scored against
`/home/user/gate-2f47/parity.tsv` with `batch-check.sh`'s own rule — page count, then max(2 %, 15)
alphanumeric characters — the words track is **MATCH 314 before and MATCH 314 after**, and exactly
one verdict string moves: `words/table-001/doc/150_5300_13_chg10.doc` goes `pages` → `pages,words`.
That document is **already on the raster ceiling** (`TODO.raster-ceiling.md`: the reference draws
full-page 300 dpi JPEGs on its pages 27, 29, 30, 31 and 32 where we play the embedded metafile and
emit its labels as real text). This change puts *more* of that metafile on the page — pages 27, 29,
30, 34, 35, 39, 40, 46 and 50 each gain words — so its glyph count moves further from the
reference's while its output gets better, which is exactly what that list exists to say.

## Reproducing

```sh
python3 gen.py /abs/out                       # four DOCX
# convert align-only.docx to odt/doc/rtf/fodt through 26.2.4.2, render each both ways,
# and read the image rectangle with
#   .claude/skills/render-comparison/scripts/pdf-ops.py dump <pdf> --page 1 --only image
```
