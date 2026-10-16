# An anchored frame's horizontal origin is right — a refutation

**Measured 2026-09-03**, this container: LibreOffice 24.2.7.2, `fc-match "DejaVu Sans"`
resolving to DejaVu. Read the environment note in `dotnet/CLAUDE.md` before quoting any
figure here.

## What was suspected

`004_Free_Genogram_Diagram_Template_Editable_Format` draws its family tree about **72 pt
left** of where the reference draws it — the shapes span an identical 639 pt but start at
62 pt against the reference's 134. Seventy-two points is exactly the section's left margin,
and twenty of the document's twenty-five anchors state
`<wp:positionH relativeFrom="column">`. The obvious reading is that a `column` origin is
being resolved from the page's edge rather than from the text column's, and it would be a
broad defect: `column` is the commonest `relativeFrom` in the corpus.

## What the probe says

`origins.docx` is landscape A4 with 72 pt margins and three anchored squares — red, green,
blue — at `column`, `margin` and `page`, each with `posOffset` of nought. Rendered both
ways and read off the raster by colour:

| origin | XRay.Office | LibreOffice |
|---|---:|---:|
| `column` | 72.00 pt | 72.00 pt |
| `margin` | 72.00 pt | 72.00 pt |
| `page` | 0.00 pt | 0.00 pt |

**Exact on all three.** The horizontal origin is not the defect, and a round that sees the
genogram family shifted should not spend its time here.

## What is still open

The genogram shift is real and has eight witnesses — the `00N_Free_Genogram_*` family —
and it is *not* a uniform translation: the span matches to the point while the interior
positions do not, so it is per-shape rather than per-page. Those documents nest groups up
to twelve deep and place most of their shapes inside them, so the remaining suspect is the
composition of nested `wpg:grpSp` transforms, which `DocxFrames.GroupTransform.Around`
already has one recorded defect in.

## The one that did mislead

A first cut of this probe put the square in a paragraph that also held the word `anchor`,
and read the reference's placement from the first `m` operator in its content stream. That
gave 108 pt against our 72 and looked like a clean 36 pt error. The operator was not the
square. **Read a shape's position off the raster by its own colour**, not off the first
path in a stream that also holds text.
