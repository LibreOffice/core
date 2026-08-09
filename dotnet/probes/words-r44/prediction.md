# Words round 44 — predicted reach and verdict movement

Committed **before anything was rendered post-change**. Baseline at `7f7c5fbb7`, reproduced
exactly: 154/200, absolute page error 78, 164 exactly-correct page counts, absolute word error
6512 (`baseline.tsv`).

Three changes are in the tree. Each is predicted separately, and what each census can and cannot
see is stated with it.

## 1. A DOC shape group is flattened into its members

`Ww8Frames.Build` made one `PageFrame` per `FSPA`; a group's members have no `FSPA`, so a Word 97
letterhead drew as an empty rectangle. The group envelope also stops painting: an `SdrObjGroup`
has no fill and no line, and ours had an opaque white one that covered whatever was under it.

**Reach — no census can bound this one.** The shape is a group container inside the Escher blob of
a `.doc`, which is not readable at zip level and not readable by grep. 66 of the 200 documents are
`.doc`. My estimate is **4 to 15 renderings changed**, from the one document examined plus the
prior that a masthead-as-group is ordinary house style in this corpus; I expect to be wrong about
this number and am recording it so the miss is measurable.

**Verdicts: +1.** `omrIMInterpretiveGuideLine.doc` goes 355 words against 382 → measured 376
against 382 on the document alone, inside the 2% band, so `words` → `match`.

**The risk is over-drawing.** A group whose members were previously invisible now contributes their
text to `wc -w` on every page it appears on, exactly as round 42's furniture frames did. A document
whose word error grows for that reason is not a regression, and the token multiset is the
instrument that says so.

## 2. A greyscale JPEG is no longer announced to PDF as three-channel colour

`PdfImages` wrote `/ColorSpace/DeviceRGB` on every passed-through JPEG. A one-component JPEG then
draws as several squashed copies across the top of its box.

**Reach: every document on every track that embeds a greyscale JPEG**, which I have not counted.
This is a pixel change and nothing else — it cannot move a page count or a word count, so the
predicted verdict movement on words is **0** and the predicted movement in absolute page and word
error is **0**.

It cannot make a correct image incorrect: the branch it changes was already drawing 1-component
and 4-component JPEGs against the wrong colour space, and 3-component JPEGs are untouched.

## 3. The empty paragraph OOXML makes mandatory after a nested table

Established from the reference by mutating one real cell seven ways and reading the drawn cell
edges back out of both PDFs (`header-row-mutations.py`): LibreOffice does not lay out a cell's
**last** paragraph when it is empty and the block before it is a **table**.

**Reach: at most the 8 of 134 DOCX that carry the shape** (`nested-filler-census.py`), and the
census is exact for DOCX because the shape is syntactic — a `w:tc` whose last two children are a
`w:tbl` and an empty `w:p`. It is applied in the DOCX reader only, so **no `.doc` can change**, and
the 66 `.doc` are outside it by construction rather than merely unmeasured.

The eight, with their baseline verdicts:

| document | occurrences | pages | words | verdict |
|---|---:|---:|---:|---|
| `May 25 bulletin focus on carers in the workplace.docx` | 13 | 5/4 | 441/528 | pages,words |
| `docs-quality-MA.IMS.00001…manual.docx` | 8 | 43/44 | 12174/12397 | pages |
| `OM template for non-complex NCC operators_August 2016.docx` | 5 | 164/165 | 51556/51986 | pages |
| `UG.CAO.00133 … Language.docx` | 4 | 18/18 | 3944/3700 | words |
| `UG.CAO.00006 … Approval Holders.docx` | 2 | 30/29 | 8124/7458 | pages,words |
| `FAA 2025-26 Holdover Tables.docx` | 1 | 154/154 | 78946/78905 | **match** |
| `24-25_FAA_Holdover_Tables.docx` | 1 | 142/141 | 70693/70663 | pages |
| `part-145-approval list (1).docx` | 1 | 8/8 | 1550/1552 | **match** |

**Verdicts: +1, and the direction is not uniform.** Every cell loses about a line, so we get
shorter. Three of the eight over-paginate and are helped; two under-paginate and are made worse in
page error while staying failures. The one I expect to flip is `24-25_FAA_Holdover_Tables.docx`,
whose words are already within 0.04%. **Two documents that currently match are in the population**
and are the change's real risk.

## The whole tree

| | baseline | predicted |
|---|---:|---|
| documents matching | 154 | **155–156** |
| absolute page error | 78 | 74–80 |
| exactly-correct page counts | 164 | 164–166 |
| absolute word error | 6512 | 6400–6560, and it may rise for change 1 |
| renderings changed | — | 10–30 |

I expect the reach estimate for change 1 to be the one that misses, because it is the only one with
no census behind it at all.
