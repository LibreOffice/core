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
- [x] Tracked changes: a change region is hoisted out of the body by ODF itself, so a deletion is
      absent from the text without the reader having to skip it — which is why all four formats
      agree. Reading the regions themselves, to report who changed what, is still to do; see
      `Paperless.OpenDocument/TODO.md`.
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
      in the file and emitting it invents content. All four readers agree on this;
      `TrackedChangeTests` pins it over one document converted to all four formats.
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

### DOC (WW8) — extraction done
- [x] FIB; the text streams; `0Table` vs `1Table` chosen by `fWhichTblStm`. The FIB's
      offset-and-length array is positional and its indexes are easy to get wrong by three:
      `fcPlcfLst` is 73, not 70 — verified against LibreOffice's own `Ww8Fib::Ww8Fib`, which
      seeks to `0x2DA` for the field at index 72.
- [x] **The piece table**: a position is turned into a byte offset through it, never by
      arithmetic, so a fast-saved document reads in logical order rather than scrambled.
      Compressed pieces decode in the document's code page with WW8's sixteen fixed overrides.
- [x] PLCF structures, with the record size passed explicitly — a PLCF read as though it had no
      data records takes its record bytes for positions, which point anywhere and yield stories
      full of unrelated text
- [x] CHPX/PAPX sprm decoding, including the `0xD608` two-byte-length exception; FKP pages with
      the paragraph index's thirteen-byte stride
- [x] The `STSH` stylesheet, with **both** UPXs per style: a paragraph style carries a CHPX half
      as well as a PAPX half, and that half is how a heading style makes its runs bold. The
      style name's length prefix is two bytes; read as one, every name comes back shifted eight
      bits and arrives as CJK rather than as nothing.
- [x] Character formatting resolved paragraph style → character style → direct, with the
      character style found in a first pass over the exception: the sprm naming it sits in the
      same grpprl as the direct formatting it must not override.
- [x] List tables (`LSTF`/`LVL`/`LFO`): labels are computed, since the file stores none. The
      level a placeholder stands for comes from the *character value* at its offset, not from
      the offset's slot — producers pack `rgbxchNums` from zero rather than indexing it by
      level, and LibreOffice reads the character for the same reason.
      The `LVL` array also lies **outside** `lcbPlfLst`, so honouring the declared length finds
      every list's header and none of its levels.
- [x] Subdocuments: the eight ranges in CP space, the header PLCF's six leading separator
      stories, and the terminating paragraph mark that ends every story rather than opening a
      paragraph in it (LibreOffice reads `nLen - 1` for the same reason)
- [x] Comments with their author: `GrpXstAtnOwners` is a bare array of strings rather than a
      string table, and `ibst` follows a fixed 22 bytes of initials
- [x] Metadata from the OLE property sets, via the shared reader in `Paperless.MsBinary` —
      including the `FILETIME` that carries an elapsed time rather than an instant
- [x] Codepage handling from the FIB language id
- [x] Tables: `sprmTDefTable`'s column edges and cell descriptors, and `sprmTTableHeader` for
      repeated header rows. A horizontal merge is expressed *either* by a cell's flags or by
      geometry alone — Word writes the flags, LibreOffice writes neither — so the span comes from
      the table's column grid as well as from the cell, exactly as in RTF. The geometry lives on
      the row-*end* paragraph, whose properties are cleared as soon as it is finished, so it has
      to be handed to the row rather than read back from the walk.
- [x] Hyperlink targets from the `HYPERLINK` field instruction, through the parser the RTF reader
      shares. The target is in the instruction and nowhere else; the cached result says only what
      the link looked like.
- [x] Tracked changes: `sprmCFRMarkDel` text is skipped, so the extraction says what the changes
      leave rather than what they removed. `TrackedChangeTests` pins all four readers agreeing.
- [x] Word 95 and earlier: a different FIB and a different sprm numbering, so it is rejected
      rather than misread
- [ ] Escher drawings via `Paperless.MsBinary`. Until then a drawing anchor is not reported as
      an image: telling an embedded picture from a shape needs the record stream, and counting
      every `U+0001` reports a picture for every text box.
- [ ] Section descriptors (`PlcfSed`): page setup and which header slots a section uses. The
      stories are read by position today, which is right for extraction and not enough for
      layout.
- [x] Nested tables: `sprmPItap` for the depth, and the detail that makes them readable at all —
      only the *outermost* table separates cells with U+0007. A nested table reuses the paragraph
      mark and says what it means with `sprmPFInnerTableCell` and `sprmPFInnerTtp`, so a reader that
      treats every carriage return as merely a paragraph puts a nested table's whole contents into
      one cell.
- [x] `sprmTDefTable`'s length field is the operand's length **plus one**, not plus two: it counts
      itself as one byte rather than the two it occupies. Subtracting two loses the operand's last
      byte and desynchronises the rest of the grpprl, so every sprm after a table definition decodes
      as something else — which for a table means the repeat-header flag and the shading that follow
      it. LibreOffice notes the same quirk at `ww8scan.cxx`'s `L_VAR2`.

### RTF — extraction done
- [x] Byte-level tokeniser: groups, control words with parameters, control symbols, `\'hh`
      escapes, and the three details that bite — one space after a control word is its
      delimiter, a bare newline is *ignored*, and a backslash before a newline is `\par`
- [x] Destination handling, including skipping an unknown `\*` destination whole. RTF puts
      private extensions in the same syntax as content, so recursing hopefully emits binary
      picture data as text.
- [x] Character encoding: `\ansicpg`, per-font `\fcharset`, and `\uN` with its code-page
      fallback skipped per `\ucN`. Not skipping the fallback doubles every non-ASCII character.
- [x] Flows: footnotes with counted marks, annotations with their author, headers, footers and
      shape text
- [x] Tables, including the merge LibreOffice writes with **no flag at all** — the span has to
      be derived from the table's column grid of `\cellx` edges, because `\clmgf`/`\clmrg` are
      absent from LibreOffice's output
- [x] Fields: `\fldinst` skipped, `\fldrslt` kept, and `HYPERLINK "…"` parsed out of the
      instruction, since RTF has no hyperlink markup
- [x] Metadata from `{\info}`, whose timestamps are groups of numeric control words
- [x] Embedded pictures recorded as graphics without decoding the bytes
- [x] Nested tables: `\itap` for the depth, `\nestcell`/`\nestrow` for the inner ends, and
      `{\*\nesttableprops}` — the one ignorable destination that must **not** be skipped, since it
      holds the inner row's geometry and its end. `{\nonesttables …}` beside it is a plain-text
      approximation for readers that cannot nest, and reading both duplicates every nested cell.
      A nested row's definition arrives *after* its cells, so the declarations are matched to cells
      when the row closes rather than as each cell ends.
- [x] `\trhdr` header rows. LibreOffice's own export omits it, so this is covered by a hand-written
      document rather than by the corpus.
- [x] The full LCID table, now generated into `Paperless.Core.Globalization.WindowsLanguages` from
      LibreOffice's `i18nlangtag` data and shared with the DOC reader

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
