# A SmartArt diagram in a word-processing document, and where the shared half of it belongs

**Measured 2026-09-06 at `559c86998`.** Environment, stated once because a stored figure is
evidence about an environment and not about the code:

| | |
|---|---|
| ours | `XRay.Office.Cli` built from this worktree at `559c86998` plus this round's change |
| ref24 | `/usr/bin/soffice` — **LibreOffice 24.2.7.2 420(Build:2)**, which is what `batch-check.sh` measures against |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2 0229ac93fcf0d7cb**, eight Latin duplicate faces aside |
| fonts | system fontconfig: Carlito, Caladea, Liberation, DejaVu, WenQuanYi, IPAGothic |
| corpus | `/home/user/sample-files`, 947 documents; gate at the base commit banked at `/home/user/gate-2f47/` |
| rule | `batch-check.sh` of 2026-09-05: page count, then max(2 %, 15) **alphanumeric characters** |

---

## The reach, per family

`census.py` walks the corpus by `git ls-files` and reads each package's zip directory, so it
cannot see a case-variant alias and cannot double-count.

| family | documents with a `dgm:` data part | data parts | drawing parts | of which hold at least one `dsp:sp` | baked `dsp:sp` |
|---|---:|---:|---:|---:|---:|
| slides | 15 | 33 | 33 | 28 | 226 |
| **words** | **3** | **5** | **5** | **5** | **89** |
| sheets | **0** | 0 | 0 | 0 | 0 |
| total | **18** | 38 | 38 | 33 | 315 |

Three things this settles that the brief left open:

- **Spreadsheets carry none.** Not one of the 307 corpus workbooks has a `xl/diagrams/` part or
  an `a:graphicData` naming the diagram namespace, so the third family is a hypothetical here.
  The code this round moved is reachable from a workbook if one ever appears, but nothing in the
  corpus exercises it.
- **Every anchor has a data part and every data part has a drawing part** — 38 of 38 in both
  directions — so the corpus's diagrams are all Office-2008-or-later files. There is no corpus
  document that needs the layout-atom evaluator.
- **All five of the words track's drawing parts are usable**, 89 shapes between them. The five
  emptied ones are all in two decks (`ghgp-supply-chain-initiative`, `County ACHS`), which is the
  hand-stripped shape LibreOffice's own corpus is full of and which the reader already handles by
  counting `dsp:sp` rather than trusting the relationship.

---

## What the PPTX path actually does

**Both.** `PptxDiagram.Baked` reads the pre-baked `dsp:spTree` and `PptxDiagram.Evaluated` runs
the layout-atom program — a real evaluator, 4 065 lines over ten files, with the constraint
solver, the iterators and the four geometric algorithms. `PptxSlideLayout.Diagram` prefers the
baked one and falls through to the evaluator exactly as `diagram.cxx:701` does.

So the answer to the question the brief asked early is **plumbing, not layout**: every corpus
document that could benefit has a usable baked drawing, and the baked path is a part lookup and a
namespace rename.

The rename is the trick worth naming. `dsp:sp`, `dsp:spPr` and `dsp:txBody` are the *same
elements* as `p:sp`, `p:spPr` and `p:txBody` under another namespace — LibreOffice makes the
substitution in one line (`pptshapegroupcontext.cxx:60-61`) — so the slides path renames the tree
into PresentationML and runs its ordinary slide walker over it, buying 187 preset geometries,
gradients, dashes and text layout for free.

---

## Where the shared code went, and which test decided it

Nine of the ten files import **`System.Xml.Linq`, `System.Globalization`, `XRay.Office.Core.Units`,
`XRay.Office.Core.Graphics` and `XRay.Office.Ooxml.DrawingML`, and nothing else.** Not one of them
mentions `PptxFile`, `PptxSlide`, `SlideTheme` or any other presentation type. By `CLAUDE.md`'s
rule — *a thing belongs lower when it depends on nothing above* — they were already in the wrong
library, and had been since they were written.

They now live in **`XRay.Office.Ooxml/DrawingML`** and not in `XRay.Office.Core`, and the second half
of that is the same distinction `Core/Charts` draws: a chart's *model and layout* came down to
Core while *the readers that parse markup* stayed in `XRay.Office.Ooxml`. Every one of these ten
files parses `dgm:`/`dsp:` markup and emits an `XElement` tree. They are readers. `XRay.Office.Ooxml`
is where readers of OOXML markup that serve more than one family live, and it is exactly one layer
above where they were reachable from only one.

The tenth file — `PptxDiagram.cs`, now `DiagramParts.cs` — was the only one with a real dependency
on `PptxFile`, and it was two method calls: *resolve a relationship id stated on a part* and *load
a part by name*. Those are `DiagramPartSource`, two delegates. `PptxFile` answers them from its own
relationship table and part cache; `DocxPictures` answers them from `OpcPackage.GetRelationships`
and `OpcXml`, which is what it already did for a chart.

**Visibility.** Only `DiagramParts` and `DiagramPartSource` became public; the other fifteen types
stay `internal` with an `InternalsVisibleTo` for the two test projects that reach them, so the
move costs two types of public API rather than seventeen.

**This is the proper move and not a narrower reachable path.** Nothing was left behind in
`XRay.Office.Presentations`, and `XRay.Office.WordProcessing` gained no dependency it did not have —
it has referenced `XRay.Office.Ooxml` since it was created.

---

## What reaching it cost on the DOCX side

One new file, `DocxDiagram.cs`, and it is smaller than expected for a reason worth recording:
**`DocxFrames` matches every element it reads on its *local* name.** `spPr`, `xfrm`, `prstGeom`,
`gradFill`, `style`, `bodyPr`, `sp` — all of them. So a `p:sp` from the diagram tree is already a
shape Writer's frame reader can place, fill, outline and give a preset geometry to. Only the text
is family-specific: a Word shape states its text as `w:txbxContent` full of `w:p`, and DrawingML
states it as `a:txBody` full of `a:p`.

So the whole translation is one `w:txbxContent` hung off each shape, carrying the run's size
(hundredths of a point into half-points), its colour (its own `a:solidFill`, else the shape's
`a:fontRef` resolved against the theme — which is what makes a node's text white rather than black
on its own accent colour), its face, and the paragraph's alignment, indent and spacing. Every
property is stated explicitly rather than left to inherit, because Word writes 8 pt after and 1.08
line spacing into `docDefaults` for nearly every file and a diagram node is a fixed circle with two
words in it.

**One decision could not be taken by analogy, and the probe below is why it is what it is.**
`DocxFrames` refits a `wpg:wgp` whose members do not fill their child space — measured behaviour,
right for a group Word wrote — and leaves a `wpc` canvas alone. A diagram's baked shapes are stated
in the *frame's* coordinates (`pParentShape->setChildSize(pParentShape->getSize())`,
`diagram.cxx:131`), so the mapping is the identity and refitting would stretch 024's diagram — a
5 804 749 EMU square — across a 6 998 335 × 5 848 350 frame. The synthesised container is therefore
named `wpc`.

---

## The reference was measured before any of it was written

`024_Unit_Circle_Chart_Colorful_Circles…docx`, rendered through 26.2.4.2:

- its text layer holds **five** `YOUR TEXT`, one above, three in a row, one below;
- its content stream holds **five** filled circles, at
  `(230.24, 557.06)`, `(65.11, 393.88)`, `(230.24, 393.88)`, `(395.37, 393.88)`, `(230.24, 230.74)`,
  each about 126.8 × 125.3 pt;
- each carries **white** text — `1 1 1 rg` — at `/F1 17.804 Tf`.

The baked `drawing1.xml` holds nine `dsp:sp`: five `ellipse` of 1 610 468 EMU (126.809 pt) with
`YOUR TEXT` at `sz="1800"`, `algn="ctr"`, `anchor="ctr"` and `a:fontRef idx="minor"` over
`a:schemeClr val="lt1"`, plus four `custGeom` connectors. Their `a:off` differ by **2 097 140 EMU
= 165.129 pt** in each direction.

So: five nodes, that text, that geometry — a measurement, before implementing.

### The residual, and two hypotheses refuted

The reference's **horizontal** node spacing is 165.13 and 165.14 pt against the stated 165.129 —
the identity, to a fiftieth of a point. Its **vertical** spacing is 163.14 and 163.18, and its
circles are 125.28 tall against 126.81 wide. **26.2.4.2 squashes that diagram vertically by
0.9880 and does not squash it horizontally**, and the drawn font size follows (17.804 for a stated
18.000, ratio 0.9891, which is 628 of 635 hundredths of a millimetre — the same scale after
quantisation).

Two structural explanations were tested by probe and **both are refuted**
(`effextent.py`, `frameshape.py`, `probe26.tsv`, twelve hand-built DOCX rendered through 26.2.4.2):

- **It is not the `effectExtent`.** Four documents identical but for `wp:effectExtent` — none,
  024's own `t=19050 b=57150`, `b=114300`, and `l=r=114300` — render to **identical node positions
  at every one of eight measurements**. The idea that the shadow's extent enters the group's
  bounding box and the group is then refitted to `wp:extent` predicts a vertical scale of 0.994 and
  is simply wrong.
- **It is not the frame's shape or the diagram's position in it.** A probe with 024's *own* frame
  (6 998 335 × 5 848 350), its own node diameter and its own five offsets renders with node spacing
  **165.136 pt in both directions** — the identity to 0.007 pt. The same nodes in a frame exactly
  the size of their span render identically. So a frame the diagram does not fill is mapped
  one-to-one, both ways, which is what this round implements.

What is left in the real document and not in the probe: the four rotated `custGeom` connectors, the
gradient fills and `a:effectRef idx="3"`, `wrapSquare` with `relativeFrom="margin"`, the `dgm:bg`,
and three other anchored drawings on the same page. Their union bounding box is exactly the shapes'
own 5 804 749 EMU square — computed in `census.py`'s sibling arithmetic, connector rotations
included — so a refit-to-extent would give 1.2056 × 1.0075 and not 1.0000 × 0.9880. **The mechanism
is not identified.** It is worth about 1.2 % of one document's vertical geometry and it is recorded
rather than guessed at.

---

## Before and after, all three DOCX, screened against 26.2

Glyphs are alphanumeric characters, which is what the gate compares.

| document | | ours before | ours after | ref24 | ref26 |
|---|---|---:|---:|---:|---:|
| `024_Unit_Circle_Chart…docx` | pages | 1 | 1 | 1 | 1 |
| | words | 95 | **105** | 105 | 105 |
| | glyphs | 517 | **557** | 557 | 557 |
| | verdict | `words` | **`match`** | | |
| `t_TEMPforInvProgs.docx` | pages | 26 | 26 | 26 | 26 |
| | words | 4 962 | **5 020** | 5 047 | 5 020 |
| | glyphs | 27 525 | **27 737** | 27 737 | 27 737 |
| | verdict | `match` | `match` | | |
| `SPA-06_mcar_part-6…docx` | pages | 85 | 85 | 85 | **64** |
| | words | 27 357 | 27 413 | 27 422 | 26 950 |
| | glyphs | 142 741 | 143 116 | 142 938 | 140 686 |
| | verdict | `match` | `match` | | |

**024 is now exact against both references**, on words and on characters. The deficit was
`YOUR TEXT` × 5 — ten words, forty characters — and it is closed to the character.

**`t_TEMPforInvProgs` is now exact against 26.2.4.2** on characters (27 737 of 27 737) and on
words (5 020 of 5 020). It was −212 characters and inside the band, which is the brief's point
about the gate being blind: nothing in the gate would ever have found it.

**`SPA-06` is a version-gap document and should not be scored.** 26.2.4.2 paginates it at **64
pages** where both 24.2.7.2 and we give 85, and its character count moves with the pagination. Its
diagram nodes do now draw — the change adds 375 characters to our output — but the −197 the brief
quotes was measured against 24.2 and the two references disagree by 21 pages on this file. It
belongs in `mismatch-classify-01`'s *"the two references disagree with each other — read, do not
score"* row, and that is a correction to the classification round, which listed all three of these
as one finding.

---

## The words track re-swept

Whole track, all 338 banked paths, scored against `/home/user/gate-2f47/parity.tsv` by
`score.py`, which refuses to print a figure unless every banked path found a row (338 of 338 did,
with no extra rows — so no case-variant alias inflated the denominator).

```
TOTAL 338  MATCH 313  MISMATCH 25  REF-CANNOT-RENDER 0     (banked: MATCH 312  MISMATCH 26)
```

**Exactly one verdict moved, and it moved the right way.**

| document | before | after |
|---|---|---|
| `024_Unit_Circle_Chart…docx` | `words` — 95 of 105 | **`match`** — 105 of 105 |

**Nothing moved the other way**, and only two other rows' numbers moved at all — the other two
diagram documents, `SPA-06` (27 357 → 27 413 words) and `t_TEMPforInvProgs` (4 962 → 5 020). The
remaining 335 rows are identical, page count and word count, to the banked gate.

The sweep is sound against `CLAUDE.md`'s two traps: the binary's mtime (16:35) predates the
sweep's first document (16:42) and nothing rebuilt during it, and re-rendering `024` afterwards
gives a file byte-identical to the sweep's own copy once `/CreationDate` is masked.

`words-after.tsv` is the run, with the binary, the reference and the font set in its header.

---

## Reproducing

```sh
python3 census.py                                  # the reach, per family
python3 effextent.py  /abs/out                     # four DOCX differing only in wp:effectExtent
python3 frameshape.py /abs/out24                   # three DOCX with 024's own frame and offsets
# render each through 26.2.4.2 and read the node positions out with `pdftotext -bbox`
```

`probe26.tsv` holds what those renderings measured, with the binary and the expected spacing in
its header.
