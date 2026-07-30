# Paperless.Presentations — TODO

The Impress equivalent: PPTX/PPT/ODP and variants.

The most rendering-dominated family — almost nothing flows, almost everything is a
positioned shape. So the shape model and theme resolution carry nearly all the fidelity
burden.

Reference: `research/04-impress.md`.

## Document model

- [ ] Slides, layouts, masters, notes pages, handouts
- [ ] Shape tree: rectangles, paths, pictures, groups, tables, custom shapes, connectors,
      placeholders
- [ ] Shape properties: transform (with flip **before** rotation), fill, line, effects, text
      body
- [ ] Text bodies via the shared text layout, with insets, anchoring and autofit
- [ ] Slide size; background fill with inheritance
- [ ] Presentation styles and outline-level styles
- [ ] Animations and transitions — extract only; do not attempt to render

## The inheritance chain

**Get this exactly right; it is the most common cause of wrong-looking slides.** A shape's
effective formatting resolves through:

```
shape → layout placeholder → master placeholder → theme defaults (spDef/lnDef/txDef)
```

resolved **per text level** for list styles (`lvlXpPr`).

- [ ] Placeholder matching between slide, layout and master (by type and index — and the
      matching rules have edge cases)
- [ ] Per-level text property inheritance
- [ ] Theme default shape/line/text properties
- [ ] Background inheritance, including `showMasterSp`

## Importers

### ODP — first
- [ ] `draw:page`, `draw:frame`, `draw:custom-shape` with `draw:enhanced-geometry`
- [ ] Master pages; `style:presentation-page-layout`; `presentation:*` attributes
- [ ] Simpler two-level inheritance than PPTX, which makes it the right place to build the
      resolution machinery first

### PPTX
- [ ] `presentation.xml`; slide, layout and master parts and their relationships
- [ ] `sldSz`; slide order; hidden slides
- [ ] Shapes via `Paperless.Ooxml` DrawingML
- [ ] Placeholder resolution (above)
- [ ] Notes slides
- [ ] `normAutofit` `fontScale` and `lnSpcReduction`
- [ ] SmartArt and charts — decide fallback-vs-implement (open question in master TODO)

### PPT (binary)
- [ ] The persist directory and `UserEditAtom` chain — **this must be walked correctly to
      find the current version of each record**; a file edited repeatedly contains stale
      copies, and reading the wrong one yields an old version of the slide
- [ ] Atom/container records
- [ ] Text: `TextHeaderAtom`, `TextCharsAtom`, `TextBytesAtom`, `StyleTextPropAtom`,
      `TextSpecInfoAtom`
- [ ] Master/slide relationships
- [ ] Escher shapes via `Paperless.MsBinary`

## Rendering

- [ ] Slide as a page: background, then shapes in z-order
- [ ] Shape geometry through the shared preset evaluator
- [ ] Fills, lines, shadows
- [ ] Text bodies with anchoring and autofit
- [ ] Groups with nested transforms
- [ ] Pictures, including crop and the picture effects worth having
- [ ] Tables
- [ ] Notes pages as separate output pages (optional)

## Open questions

- [ ] Render animations' final state or initial state? Initial matches what a static export
      shows; confirm against the reference.
- [ ] Are connectors worth routing properly, or is a straight line acceptable initially?
