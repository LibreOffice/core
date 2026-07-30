# Paperless.WordProcessing — TODO

The Writer equivalent: DOCX/DOC/RTF/ODT and variants. Extraction plus paginated layout.

All four front ends build **one** document model consumed by **one** layout engine — the
same arrangement LibreOffice uses, where the DOCX, DOC and RTF importers all build the same
`SwDoc`. That keeps layout, the expensive half, written once.

Reference: `research/02-writer.md` — model, layout engine, and all four importers.

## Document model

- [ ] Node sequence with paired start/end markers for nesting (tables, sections, frames) —
      the `SwNodes` arrangement. A flat array with markers handles arbitrary nesting without
      a deep tree.
- [ ] Paragraph nodes with text plus character-formatting runs stored as interval-tagged
      attributes over the text, not as a split-run list. Overlapping formatting is common and
      intervals handle it without exploding into fragments.
- [ ] Style model with parent-chain resolution; paragraph, character, frame, list, page styles
- [ ] Tables: rows, cells, spans, nested tables
- [ ] Lists and numbering with multi-level definitions and restart semantics
- [ ] Sections; page styles; headers and footers, including first-page and even/odd variants
- [ ] Fields — store both the definition and the cached result. The cached result is what a
      reference renderer shows, so prefer it by default.
- [ ] Bookmarks, hyperlinks, comments, footnotes and endnotes
- [ ] Floating frames with anchoring (paragraph, character, as-character, page) and wrap mode
- [ ] Tracked changes (redlines)

## Importers

Order chosen so each is verifiable before the next gets harder.

### ODT — extraction done
- [x] `content.xml` text content; automatic and named styles via `Paperless.OpenDocument`
- [x] Tables, lists, frames, sections, fields, notes, comments, and the master pages'
      headers and footers — the last four are things LibreOffice's own text filter drops, so
      extracting them is a deliberate improvement rather than a difference to reconcile
- [x] `ott` templates and flat `fodt`, through the same reader
- [ ] Tracked changes (redlines) — see `Paperless.OpenDocument/TODO.md`
- [ ] Layout, which needs the document model below rather than the extraction tree

`OdtReader` builds the extraction tree directly today. The full `SwDoc`-shaped model below is
for layout, and ODT will need a second pass through it once that exists; the extraction path
does not need it and must not be made to pay for it.

### DOCX — extraction done
- [x] `document.xml` body; `styles.xml`; `numbering.xml`; parts located by relationship with
      the conventional name only as a fallback
- [x] Style resolution across every layer, with **ECMA-376 §17.7.3's toggle rule**: bold set by
      both the paragraph style and the character style comes out *off*, while direct formatting
      is absolute. This is the single most common way a DOCX reader gets emphasis wrong; the
      four cases are pinned by `WordToggleTests` and were verified against LibreOffice's HTML
      export, where bold is visible.
- [x] Numbering: `w:abstractNum` definitions against `w:num` instances, level overrides,
      `w:lvlText` templates, `w:isLgl`, style-linked lists, and the `numId` of zero that means
      "not numbered"
- [x] Fields: the `w:fldChar` state machine, so the instruction is skipped and the cached result
      kept. `w:fldSimple` too.
- [x] Tracked changes: `w:ins` content read, `w:del`/`w:delText` skipped — deleted text is still
      in the file and emitting it invents content
- [x] Headers/footers (only the parts a section names), footnotes/endnotes with computed
      citations, comments with their author
- [x] `w:drawing` and legacy `w:pict`: images recorded, text boxes hoisted into their own
      section, and the DrawingML/VML pair read once rather than twice
- [x] Tables: `w:gridSpan`, and `w:vMerge`'s top-and-continuation encoding turned into a row
      span, which needs the rows drafted before they are materialised
- [ ] `settings.xml` compatibility flags — parsed and exposed, nothing reads them yet. They
      genuinely change layout maths, so layout will need a handful of them.
- [ ] `w:altChunk`: an embedded foreign document, reported as a diagnostic rather than read
- [ ] `fontTable.xml`, needed for font resolution rather than extraction
- [ ] Theme colours for `w:color w:themeColor` references

### DOC (WW8) — hardest
- [ ] FIB; the text streams; `0Table`/`1Table`
- [ ] **The piece table** for complex files: text is not contiguous, so a naive read produces
      scrambled output. This is the first thing to get right.
- [ ] PLCF structures
- [ ] CHPX/PAPX sprm decoding; the `STSH` stylesheet
- [ ] List tables (`LST`/`LFO`)
- [ ] Escher drawings via `Paperless.MsBinary`
- [ ] Codepage handling from the FIB language id

### RTF
- [ ] Group/control-word tokeniser; destination handling
- [ ] Character encoding: `\ansicpg`, `\uN` with fallback characters
- [ ] Embedded pictures; nested tables (`\itap`)

## Layout engine

Only after extraction is solid and `Paperless.Text` breaks lines correctly.

- [ ] Frame hierarchy: page → body/header/footer → text/table/section/floating frames
- [ ] Pagination: fill a page, split what does not fit, continue
- [ ] Paragraph splitting across pages; keep-with-next; widow/orphan control
- [ ] Tables spanning page breaks, with header-row repetition
- [ ] Floating objects and text wrap, including contour wrap
- [ ] Footnote placement — footnote area growth changes how much body text fits, which
      changes pagination, so it must feed back into the page-filling loop
- [ ] Columns; vertical and RTL writing modes
- [ ] Emit to `IDrawingSink`

## Open questions

- [ ] Tracked changes: render as accepted, or show change marks? LibreOffice shows marks by
      default; `LayoutOptions.AcceptTrackedChanges` exists to choose. Confirm the default
      matches the reference.
- [ ] How much compatibility-flag behaviour is worth implementing? There are dozens; only
      some matter visually.
