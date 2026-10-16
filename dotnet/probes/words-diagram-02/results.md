# `xray-office extract` is blind to a SmartArt diagram in a word-processing document

**Measured 2026-09-06 at `ffda5d02e` plus this round's change**, in `/home/user/wt-words67`.

| | |
|---|---|
| ours | `XRay.Office.Cli` built from this worktree; the "before" figures are the same tree with this round's three files reverted, `obj`/`bin` removed and rebuilt |
| ref26 | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, its eight Latin duplicate faces aside |
| corpus | `/home/user/sample-files`, 947 documents |

---

## What was wrong

The round before this one made a DOCX SmartArt **render**. Extraction was untouched, and the two
are separate paths on purpose: `IDocument` gives content, `IPaginatedDocument.Layout()` is a
distinct deferred step, and extraction must not pay for fonts or layout. So
`024_Unit_Circle_Chart_Colorful_Circles…docx` drew its five nodes and still extracted to the body
text alone — `xray-office extract | grep -c "YOUR TEXT"` was **0**.

The PPTX side had read them from the data model since the slides track needed it
(`PptxShapeReader.ReadDiagram`). `DocxContentReader.ReadChart` is the sibling seat, and there was
no `ReadDiagram` beside it.

## The reach, measured over the whole corpus

`census.py` walks by `git ls-files` and reads each package's zip directory, so it cannot see a
case-variant alias and cannot double-count. It counts the *authored* points — every `dgm:pt` that
is not `doc`, `pres`, `parTrans` or `sibTrans` and whose `dgm:t` carries text.

| family | documents | data parts | authored points | paragraphs | words |
|---|---:|---:|---:|---:|---:|
| slides | 15 | 33 | 182 | 237 | 1 720 |
| **words** | **3** | **5** | **47** | **48** | **124** |
| sheets | 0 | 0 | 0 | 0 | 0 |

The words track's 48 paragraphs are the whole of what extraction could not see.

## The seat, and where the reading went

`DocxContentReader.ReadDiagram` is a sibling of `ReadChart`: it finds the `a:graphicData`, checks
the diagram URI, and hoists the text to a `SectionKind.Frame` section named from `wp:docPr` — for
the same reason a text box is hoisted, since splicing five circle labels into the sentence that
happens to anchor them would join two unrelated pieces of prose and split that paragraph in two.

**The walk itself did not need writing twice.** It parses `dgm:` markup and emits content nodes, so
by `CLAUDE.md`'s own rule it belongs one layer above Core with the other readers of OOXML that
serve more than one family: it is now `DiagramParts.AuthoredText` in `XRay.Office.Ooxml/DrawingML`,
beside the part resolution the previous round moved there, and `PptxShapeReader.ReadDiagram` is
three lines that call it.

**It reads the data model and not the baked shape tree, and the two disagree on purpose.** The
baked `dsp:spTree` is what the author *sees*, so it repeats a node's text wherever the layout drew
it and adds text the layout generated; the data model is what the author *typed*, once each. An
index wants the second. `DocxDiagramPackage`'s data model now carries a third node the baked
drawing never draws, plus a `pres` and a `sibTrans` point, so the two possible sources are
distinguishable by the output rather than by inspection.

## Before and after

Extraction, `xray-office extract`, both binaries built from this worktree:

| document | | before | after | authored (census) |
|---|---|---:|---:|---:|
| `024_Unit_Circle_Chart…docx` | words | 95 | **105** | +10 |
| | characters | 621 | **671** | |
| | diagram paragraphs | 0 | **5** | 5 |
| `t_TEMPforInvProgs.docx` | words | 4 674 | **4 734** | +60 |
| | characters | 32 285 | **32 563** | |
| | diagram paragraphs | 0 | **24** | 24 |
| `SPA-06_mcar_part-6…docx` | words | 25 967 | **26 023** | +54 |
| | characters | 169 096 | **169 537** | |
| | diagram paragraphs | 0 | **19** | 19 |

**48 of 48 authored paragraphs, in all three documents.** `SPA-06` carries two diagrams and the
second is named `Organization Chart 14` rather than `Diagram N`, which is worth saying because a
first pass at counting them filtered on the name and reported 15 of 19.

## The blast radius, measured rather than argued

`xray-office extract` was run over **every** words and slides document in the corpus with both
binaries and the output md5'd:

- **slides: 0 of 302 changed.** The `PptxShapeReader` refactor is behaviour-preserving to the byte.
- **words: 3 of 338 changed**, and they are exactly the three documents above.

## ODF and the legacy binaries: there is nothing to seat

**The corpus holds no ODF document at all.** Counted by `git ls-files` over
`/home/user/sample-files`: 272 `docx`, 251 `pptx`, 241 `xlsx`, 66 `doc`, 64 `xls`, 51 `ppt`, 2
`xlsm` — 947, and not one `odt`, `ods`, `odp`, `rtf`, `sxw` or flat-ODF file. Any ODF claim in this
project is therefore about hand-built or converted documents and never about corpus reach.

**ODF has no SmartArt vocabulary and LibreOffice does not invent one.** Converting
`024_Unit_Circle_Chart…docx` to `.odt` through 26.2.4.2 gives a `content.xml` holding **zero**
occurrences of `YOUR TEXT`; the diagram becomes a single `draw:image` pointing at a 61 445-byte
`.svm` StarView metafile, and the text is inside it as metafile text actions. Rendering that ODT
through 26.2.4.2 does put five `YOUR TEXT` in the PDF's text layer, which is the metafile being
replayed, not markup being read. So there is no data model for an ODF reader to reach, and reading
one would mean decoding a metafile during extraction — which is the one thing extraction is
defined not to do.

Where a diagram *does* arrive as ordinary shapes, `OdfContentReader` already reads it:
`ReadShape`'s `"g" or "a"` case recurses into a group
(`XRay.Office.OpenDocument/OdfContentReader.cs`:971-973) and its `default` case reads a custom
shape's paragraphs directly.

**The legacy binaries are the same story with a different container.** The same document saved as
`.doc` by 26.2.4.2 contains the string `YOUR TEXT` **zero** times anywhere in its 122 368 bytes —
the diagram is an Escher blip holding a deflated metafile. Both 26.2.4.2 and *our own renderer*
draw its five nodes as real text from that metafile (5 of 5, 105 words each), while our extraction
gives 95: the two text boxes on the page extract, the picture does not. That is the same
extraction/rendering boundary as ODF, not a missing reader — and MS-DOC has no diagram part for one
to read.

## Reproducing

```sh
python3 census.py /home/user/sample-files            # the reach, per document and per family
xray-office extract <doc> --json                       # the Frame sections a diagram becomes
```
