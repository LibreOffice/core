# Paperless.Vector — TODO

Importers for embedded vector graphics: WMF, EMF, EMF+ and SVG.

**The highest-risk area in the project.** Not optional: office documents embed these
constantly — pasted clip art, chart and Visio snapshots, equation images, and the fallback
rendering that accompanies SmartArt and OLE objects. A renderer that skips them leaves
large blank areas on visually important pages. And there is no C# library to build on.

Reference: `research/06-rendering.md` section D.

## Decide the strategy first

Before writing code, resolve this — it changes the plan and the timeline:

- [ ] **Spike EMF+ scope.** Roughly fifty record types. Options: implement it; implement
      only EMF and use the EMF fallback that many EMF+ files also carry (dual-mode files
      are common); or accept a placeholder. Measure how much of a real corpus each option
      actually covers before committing.

## WMF

- [ ] Placeable header (`D7 CD C6 9A`) and the bare metafile header
- [ ] The core records: object selection, pen/brush/font creation, `TextOut`,
      `ExtTextOut`, `Polygon`, `Polyline`, `Rectangle`, `Ellipse`, `StretchDIBits`
- [ ] Mapping modes and window/viewport transforms — the usual source of
      wrongly-scaled output
- [ ] The graphics-object table, with correct handle reuse semantics

## EMF

- [ ] Header, bounds and frame; the reference-device fields
- [ ] Path construction records; `EMR_*` drawing records
- [ ] World transform records
- [ ] Text with `EMR_EXTTEXTOUTW`, including per-glyph DX arrays
- [ ] Bitmap records
- [ ] Clipping records

## EMF+

Only after the spike above.

- [ ] `EmfPlusRecord` dispatch inside `EMR_COMMENT`
- [ ] Dual-mode files: decide correctly between the EMF+ and EMF interpretations rather
      than replaying both
- [ ] Object table: pens, brushes, paths, images, fonts, string formats
- [ ] Anti-aliasing and colour-matrix effects

## SVG

- [ ] Basic shapes, paths, transforms, gradients
- [ ] Text — needs `Paperless.Text` for shaping
- [ ] Explicitly **do not** support external references, scripting or animation. SVG comes
      from untrusted documents; network access from a document parser would be a
      vulnerability, not a feature.

## Open questions

- [ ] Is a hand-rolled EMF+ implementation worth it versus rasterising via LibreOffice at
      corpus-build time for reference purposes only? The latter is not viable for shipped
      output but might be acceptable for tests.
- [ ] Should decoding be lazy? Documents often embed images that are never drawn.
