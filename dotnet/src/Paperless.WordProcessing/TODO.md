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

### DOCX
- [ ] `document.xml` body; `styles.xml`; `numbering.xml`; `settings.xml`; `fontTable.xml`
- [ ] Headers/footers, footnotes/endnotes, comments
- [ ] Compatibility flags from `settings.xml` — these genuinely change layout, and ignoring
      them makes documents authored by older Word versions lay out wrongly
- [ ] `w:drawing` and legacy `w:pict` via `Paperless.Ooxml`

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
