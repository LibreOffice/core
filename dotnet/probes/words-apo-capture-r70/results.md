# `omrIMInterpretiveGuideLine.doc` — the APO 11.35 pt too low, and the two measurements it was carried with

Measured 2026-09-07 in `/home/user/wt-wordsgap`, branch `agent/wordsgap`, base `f4b8c3825`.
Reference `/opt/libreoffice26.2/program/soffice` **26.2.4.2**, with its eight Latin metric duplicates,
its Latin Noto **and its four `LiberationSansNarrow`** in `.duplicates-aside/`.

**26.2 is usable on this document now, and this round re-measured rather than inheriting the previous
round's workaround.** `pdffonts` gives the identical four faces on all three stacks —
`LiberationSans`, `LiberationSans-Bold`, `DejaVuSans`, `DejaVuSans-Bold` — for ours, for 26.2.4.2 and
for 24.2.7.2, and all three paginate to 2 pages. The 36% narrower layout the earlier round saw is
gone with the face.

---

## The seat named on the brief is refuted

*"`FrameLayout` places it at `line.Top + 100.9 pt` where Writer measures from the anchor paragraph's
own top — the `PlacedLine.UpperSpace` distinction."*

The wiring is already right, and there is no upper space to lose. `FrameResolution.Of` builds
`Placement(… Top: BodyArea.Y + line.Top, ParagraphTop: BodyArea.Y + line.ParagraphTop)` and passes
`anchorTop: placement.ParagraphTop`, `anchorLineTop: placement.Top` — the paragraph's top for the
paragraph origin and the line's for the line origin, which is the way round the type documents. And
on this document **every one of page 1's 39 placed lines has `UpperSpace = 0`**, so the two are the
same number wherever they are read.

The anchor is block 38, `On August 27, 2001, the Office of Mental…`, whose line top is 603.45 in
body coordinates over a body starting at 63.00 — **666.45 pt** down the page. The frame's own
`sprmPDyaAbs` is **100.90 pt** and its stated height 486.00 × 36.00 pt. 666.45 + 100.90 = 767.35,
which is exactly where we drew its top; its bottom is then 803.35 on a 792 pt sheet, **11.35 pt below
the paper**, and its second line was drawn at a baseline of −0.70 and lost from the text layer.

## The mechanism

`SwAnchoredObjectPosition::ImplAdjustVertRelPos`
(`sw/source/core/objectpositioning/anchoredobjectposition.cxx`:504-667), whose own comment is
*"adjust calculated vertical in order to keep object inside 'page' alignment layout frame"*:

```cpp
if ( bCheckBottom && nTopOfAnch + nAdjustedRelPosY + aObjSize.Height()
                       > aPgAlignArea.Top() + aPgAlignArea.Height() )
    nAdjustedRelPosY = aPgAlignArea.Top() + aPgAlignArea.Height() - nTopOfAnch - aObjSize.Height();
if ( nTopOfAnch + nAdjustedRelPosY < aPgAlignArea.Top() )
    nAdjustedRelPosY = aPgAlignArea.Top() - nTopOfAnch;
```

Bottom first, then top — so a frame taller than the page ends flush with the top and overflows the
bottom, which is the order and not an accident of it. The area is `rPageFrame.getFrameArea()`, the
whole sheet, whenever `CONSIDER_WRAP_ON_OBJECT_POSITION` is set, which every Word import sets. Beside
it is a narrowing to the **body** frame for DOCX `compatibilityMode` 15 and non-wrap-through objects;
that is deliberately not reproduced, because it can only clamp *further* and so cannot put a frame
outside the page.

Three things decide whether it runs at all, and one of them is the whole story:

- **`SwToLayoutAnchoredObjectPosition` never calls it.** A `FLY_AT_PAGE` fly sets its position and
  then only grows the page in browse mode (`tolayoutanchoredobjectposition.cxx`:100-131), so a
  page-anchored frame is left hanging off the sheet. It is the *anchor type* that decides, not the
  vertical origin.
- **`SwAnchoredObject::IsDraggingOffPageAllowed`** (`sw/source/core/layout/anchoredobject.cxx`
  :790-801) needs `DisableOffPagePositioning`, an ODF settings flag defaulting to false
  (`DocumentSettingManager.cxx`:100) that no DOC or DOCX import sets. It does not arise here.
- **`mbDoNotCaptureAnchoredObj`** (`anchoredobjectposition.cxx`:122-144) makes `AdjustVertRelPos`
  skip the whole thing. It needs `DoNotCaptureDrawObjsOnPage`, and **which importers set that is the
  distinction that matters**: `sw/source/writerfilter/filter/WriterFilter.cxx`:332 sets it for every
  writerfilter import — DOCX *and* RTF — while the WW8 binary filter never sets it and the ODF one
  sets it only for a document written before SO8 (`sw/source/filter/xml/xmlimp.cxx`:1554-1557).

## The corpus measured that distinction rather than the brief asserting it

Applying the clamp to every format first, then gating it on the format, over the whole words track
(338 of 338, our half re-rendered each time, scored against `/home/user/gate-2f47/parity.tsv`):

| | renderings changed | gate |
|---|---:|---|
| clamp everywhere | **30** | MATCH 315 |
| clamp only where the importer captures (`.doc`, ODF) | **3** | MATCH 315 |

Both reach `MATCH 315`, and the difference is entirely in the ink. Clamping DOCX as well moves 27
more documents and moves several of them *away* from 26.2.4.2 — the templates full of anchored
DrawingML shapes, which is exactly the population `DoNotCaptureDrawObjsOnPage` exempts:

| document | clamp everywhere, before → after |
|---|---|
| `050_Visual_Product_Roadmap_Template_Yellow_and_Blue_Theme.docx` | 0.46 → **2.19** |
| `047_Visual_Product_Roadmap_Template_Professional_Layout.docx` | 0.57 → **1.15** |
| `062_Vocabulary_Concept_Map_Template.docx` | 0.32 → 0.45 |
| `038_Venn_Diagram_Template_Off_White_and_Blue_Theme.docx` | 0.01 → 0.17 |
| `016_Project_Timeline_Template_Complete_Guide.docx` | 4.69 → **1.21** |

— a mixture in both directions, summing to a net gain of only 0.62 over 30 documents, against a clean
gain over 3 with the gate on the format. The two per-object conditions the C++ also applies (a fly is
exempt only when it is wrap-through and not a textbox; and only when it does not follow the text
flow) are not modelled: they can only make a DOCX frame *captured* where this leaves it alone, which
is where the tree already was.

## What moved

`.doc` only, 3 of 338 renderings, `|ink|%` against 26.2.4.2 summed per document:

| document | pages b/a/ref | before | after |
|---|---|---:|---:|
| `omrIMInterpretiveGuideLine.doc` | 2/2/2 | 0.17 | 0.16 |
| `AAC-AD-No-2021-01-…MAX.doc` | 20/20/20 | 2.38 | 2.37 |
| `150_5300_13_chg10.doc` | 77/77/78 | — | — |

The ink is small because the frame is small; the verdict is not. On the witness the frame's two lines
now sit at **(91.20, 24.05)** and **(91.20, 10.65)** against the reference's **(91.70, 24.00)** and
**(91.70, 10.60)** — within **0.05 pt** vertically, which is the tolerance page 1's other 30 lines
already met — and the gate goes

```
words/done-013/doc/omrIMInterpretiveGuideLine.doc   words -> match   2370/2370 alphanumerics
```

where the bank had 2319 of ours: the 51 characters are the second line, which used to be drawn below
the paper edge and never reached the text layer. Track total **MATCH 314 → 315**, `WORDS 2 → 1`,
nothing else moving.

## The second measurement, and whether the two are one defect

They are **two**, and the second does not reproduce at this base.

The brief carried *"a separate measurement found the same document's APO reaching layout **1.15 pt
tall** (`MINFLY`, `Ww8TextFrame.cs:126-128`), so it obstructs nothing"*. Dumping every frame this
document produces, the APO reaches layout at **486.00 × 36.00 pt** — its stated `sprmPWHeightAbs` of
720 twips. `MINFLY` is 23 twips and the guard `if (height <= MinimumExtent) height = MinimumExtent`
never fires here; no frame in the document is 1.15 pt in either dimension. So the height was never
what put the frame off the page, and closing the position closed the document.

What *is* real, and is left: the reference's frame box is **36.65 pt** tall against our 36.00, and its
text sits 0.75 pt lower inside it — the two cancel to the 0.05 pt agreement above, so nothing but the
border stroke shows it. That does contradict the comment at `Ww8TextFrame.cs`:126-128 — *"layout gives
a frame its content's height in either case"* — but in the opposite direction to the one recorded:
layout **does** grow the frame, by 13 twips here, and we take the stated height.

## Reproducing

```sh
CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
SOURCE_DATE_EPOCH=1700000000 $CLI render --outdir /abs/ours \
  /home/user/sample-files/words/done-013/doc/omrIMInterpretiveGuideLine.doc
/opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir /abs/ref26 <same>
python3 .claude/skills/render-comparison/scripts/pdf-ops.py dump /abs/ours/omr*.pdf --page 1
../words-firstpage-r70/render-ours.sh $CLI /abs/track /abs/words-paths.txt 6
python3 ../words-ink-r67/score.py /abs/track /home/user/gate-2f47 words/
```
