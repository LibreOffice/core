# Paperless.WordProcessing — TODO

The Writer equivalent: DOCX/DOC/RTF/ODT and variants. Extraction plus paginated layout.

All four front ends build **one** document model consumed by **one** layout engine — the
same arrangement LibreOffice uses, where the DOCX, DOC and RTF importers all build the same
`SwDoc`. That keeps layout, the expensive half, written once.

Reference: `research/02-writer.md` — model, layout engine, and all four importers.

## Document model

In `Model/`, distinct from the extraction tree in `Paperless.Core` and built by a separate pass —
extraction is the common case and must not pay for interval-tagged formatting, document-order
indexes or resolvable style chains.

- [x] A **tree** rather than `SwNodes`' flat array of paired start and end markers. The research
      notes say the markers exist because LibreOffice's node array is also its edit buffer and its
      undo unit, and that a conventional tree is semantically equivalent and the natural C# shape
      (`research/02-writer.md` section A.2).
- [x] **Document order** kept anyway, since that is what the flat array is really for: layout,
      bookmarks, cross-references and tracked changes all ask "before", "after" and "inside". Each
      node carries its own index and its subtree's end, so containment is two integer comparisons.
      Assigned once when the tree is finished, not maintained per append.
- [x] Paragraph text as one string with the character formatting as **interval-tagged hints**, not
      as a split-run list. Overlapping formatting is the normal case — a bold range, a language
      range and a hyperlink each start where the user put them — and runs force every overlap to
      split every run that crosses it, so three pairwise overlaps become seven fragments that
      correspond to nothing the document said.
- [x] The resolution order between hint kinds, which is load-bearing: direct formatting must beat a
      character style and a link's own formatting must beat the style too, or a hyperlink inside a
      styled run loses its colour. `TextHintKind`'s declaration order *is* that order.
- [x] Placeholder characters for what occupies a position without being text — a field, a note or
      comment anchor, an as-character frame. They count towards a paragraph's length, because every
      offset in the document is counted against the same string; they are not in its text.
- [x] What an edit does to the hints across it: text typed inside a bold range is bold, text typed
      immediately before it is not, and an anchor never stretches because its width is the one
      character it stands for.
- [x] Positions and ranges as node-plus-offset, never an offset into a whole-document string.
      Ranges normalise on construction, so a selection made backwards is the same range as one made
      forwards.
- [x] Style model with parent-chain resolution, one pool per family — ODF documents routinely have a
      paragraph style and a character style both called `Standard`, and resolving across families
      would take one style's parent from the other's chain. Cycle-guarded; the family's defaults lead
      a chain that does not reach them.
- [x] Tables: rows, cells, spans, nested tables, and the column-edge grid the spans index into
- [ ] Lists and numbering with multi-level definitions and restart semantics. The four readers each
      compute labels already; the model holds the label but not yet the definitions that produced it.
- [x] **Page geometry**, in `Model/PageGeometry.cs`, read from all four formats and verified against
      LibreOffice's own rendering. The interesting part is that the formats do not mean the same thing
      by "top margin": Word's `w:top` is the distance to the first line of *body* text with the header
      living above it, while ODF's `fo:margin-top` is the distance to the top of the *header*. The model
      stores the body's, because that is the number pagination needs, and each reader converts — so the
      ODF reader adds the header's extent where the Word readers take the value as given. Reading ODF's
      value as the body's margin puts every line on the page too high by the height of a header.
- [x] The ODF header height, whose two spellings behave differently in a way the attribute names do not
      suggest: `svg:height` is fixed, honoured exactly, and *absorbs* the spacing below it, while
      `fo:min-height` makes the height dynamic — LibreOffice maps it to `HeaderIsDynamicHeight` and then
      sizes the frame to its content, so a header declaring 6 mm around one 12 pt line renders 4.9 mm
      and the spacing is added on top. Settled by rendering both and measuring, not by reading the
      specification.
- [ ] The dynamic-height case exactly. It needs the header's content laid out before the page it sits
      on is known, so the declared minimum plus spacing is used instead — the same approximation
      LibreOffice's own DOC exporter falls back to and calls "totally nonoptimum, but the best we can
      do". It errs towards leaving too much room, so body text starts slightly low rather than
      overlapping the header.
- [ ] Which section each paragraph belongs to. DOCX, DOC and RTF all delimit sections by position and
      the geometry is read in document order; ODF has no section list at all, only master pages and the
      paragraph styles that reach them, so its sections are the geometries the document *defines*
      rather than one per page break. Resolving either into "this paragraph is on that page
      description" needs the page-break chain, which needs layout.
- [ ] Headers and footers as page furniture rather than as flows. `WritingSection` has the slots and
      the fallback rules — first page, even page, default, with a default header appearing on a first
      page that asked for nothing else — but nothing populates them, because the flows are built by the
      extraction pass and the model pass that would connect them is not written.
- [ ] Fields — store both the definition and the cached result. The cached result is what a
      reference renderer shows, so prefer it by default. The hint kind exists; the definitions do not.
- [ ] Bookmarks and cross-references
- [ ] Floating frames with anchoring (paragraph, character, as-character, page) and wrap mode. An
      as-character anchor is a hint over a placeholder; the other three need a per-page anchor model.
- [ ] Tracked changes (redlines) with their author and timestamp. The hint kinds exist and the
      readers currently resolve changes rather than recording them.
- [ ] Importers that build this model. All four build the extraction tree today, and all four now also
      produce the flat paragraph-plus-format sequence the paginator takes — which turned out to be what
      layout actually needed, and is much less than this model holds. The tree itself still has no
      importer, and what would use it is the run-level formatting that drawing needs.

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
- [x] Layout: `OdtLayoutSource` walks `content.xml` a second time and resolves each paragraph's format
      through the style chain that was already built. The translations that needed care: `fo:line-height`
      carries a percentage *or* a length and the length means exact; `fo:font-family` is CSS syntax, so a
      real document says `'Liberation Serif', 'Times New Roman', serif` and passing that whole string to
      a resolver matches nothing; `style:font-name` names a font-face declaration rather than a family
      and is what LibreOffice writes most of the time; `fo:keep-with-next` is `always` against `auto`
      rather than a boolean; and the default size is ten points, not twelve.
- [x] ODF's own text-for-layout problem: runs of spaces, tabs and line breaks are elements, so taking the
      descendant text nodes alone loses every one of them — `text:s` above all, which is how any run of
      two or more spaces is written.

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
- [x] Layout: `DocxLayoutSource` walks `document.xml` a second time. OOXML's unit traps, each written
      down where it is read: `w:sz` is half-points, so reading it as points halves every document;
      `w:spacing`'s `auto` rule counts two-hundred-and-fortieths of a line rather than a percentage;
      `w:hanging` is a positive number meaning a negative indent; `w:start` and `w:left` are the same
      attribute under two names; and `w:rFonts` names four families at once, of which Latin text wants
      the ASCII one. Deleted text and field instructions are skipped, since both are in the file and
      neither is on the page.
- [ ] `fontTable.xml`. Not needed for layout after all — `w:rFonts` names the family directly — but it
      carries the embedded-font relationships and the panose data a substitution could use.
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
- [x] Layout: a second walk over the body range, resolving the layout sprms through the same style chain
      the content pass uses. The paragraph's properties are found from its **mark's** position, not its
      first character's — looking up the first character finds the *previous* paragraph's properties and
      formats the whole document one paragraph out of step.
- [x] The font table (`SttbfFfn`), which `sprmCRgFtc0` indexes. Two offsets decide whether it reads at
      all: the first entry starts **four** bytes in, not two, because the count is followed by the string
      table's extra-data length; and each name sits after PANOSE *and* the twenty-four-byte font
      signature, so skipping only PANOSE reads the signature as UTF-16 and yields a plausible-looking
      string of CJK. Starting two bytes early finds no fonts at all, which shows up as every paragraph
      laid out in a substituted face.
- [ ] The `Dop`, for `fDontUseHTMLAutoSpacing` — which is what LibreOffice's importer reads into
      `PARA_SPACE_MAX` and therefore decides whether two paragraphs' spacings add or the larger wins. The
      DOC path defaults to adding, which matches every document LibreOffice itself wrote.
- [ ] Escher drawings via `Paperless.MsBinary`. Until then a drawing anchor is not reported as
      an image: telling an embedded picture from a shape needs the record stream, and counting
      every `U+0001` reports a picture for every text box.
- [x] Section descriptors (`PlcfSed`): the page setup. Two levels of indirection, both easy to get
      wrong — the PLCF's twelve-byte records hold an offset that points into the *WordDocument* stream
      rather than the table stream it was read from, and it points at a length-prefixed grpprl rather
      than a structure. The sprm ids come from LibreOffice's `sprmids.hxx`, which states each operand's
      size on the same line; two of them invite an off-by-one, since `sprmSCcolumns` holds the column
      count *minus one* and `sprmSBOrientation` uses Word's `DM_*` numbering where 1 is portrait, so a
      plain truth test makes every portrait page landscape.
- [ ] Which header slots a section uses. The stories are read by position today, which is right for
      extraction; connecting them to the section that names them needs the six-per-section slot
      indexing the reader already computes to be carried into the model.
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
- [x] Layout: the formatting is recorded **as the content walk closes each paragraph**, not by a second
      pass. RTF is a token stream with nothing to revisit — re-reading it would mean running the whole
      state machine again, encoding and destinations included, and the two runs could then disagree. The
      properties themselves translate through the DOC's own `Ww8LayoutFormat`, because RTF states them
      the same way: twips, a size in half-points, and a `\sl` whose sign and `\slmult` companion choose
      between a multiple, a minimum and a fixed height.
- [x] Font family names from the font table, which the reader previously discarded because extraction
      never needs them — a run's font does not change its text. A name can arrive in several text chunks
      and ends at a semicolon, so it is accumulated rather than assigned.

## Layout engine

Only after extraction is solid and `Paperless.Text` breaks lines correctly. Line breaking now agrees
with Writer's — see `Paperless.Text/TODO.md` — and the page geometry every break is measured against
is read and verified, so what remains is the filling of pages rather than the measuring of lines.

- [x] **Pagination**: fill a page, split what does not fit, continue. Verified against LibreOffice on a
      sixty-paragraph document at three line spacings, comparing which paragraph starts each page — the
      assertion that catches every upstream measurement error, since a wrong advance width moves a line
      break, a moved line break changes a line count, and a changed line count moves a page break.
- [x] Paragraph splitting across pages, keep-together, keep-with-next (as a chain, so three headings in
      a row all move together), and orphan and widow counts. Each turns "fill until full" into "fill
      until full, then reconsider", and the reconsidering is what moves whole paragraphs.
- [x] The constraint that cannot be honoured is overridden rather than obeyed: a keep-together
      paragraph taller than a page splits, because placing nothing page after page is a hang and not a
      layout.
- [x] **The first line on a page loses the space above its text.** Found by this test and not by the
      four line-spacing tests, which all pass either way — the space line spacing adds sits above the
      text, so putting it below leaves every baseline pitch identical and every paragraph height wrong.
      A 200%-spaced A4 page holds twenty-five lines with the rule and twenty-four without, and every
      break after the first then falls somewhere else.
- [x] `PaginationOptions` for the two places Word and Writer disagree and the file says which by a
      compatibility flag rather than by a property: whether a paragraph keeps its space-before at the
      top of a page, and whether space-before collapses against the previous space-after or adds to it.
- [ ] Frame hierarchy: header and footer frames beside the body, and section/floating frames inside it.
      The page's body area is computed and filled; the furniture areas are known
      (`HeaderDistance`/`HeaderHeight`) but nothing lays anything out in them, because the flows that
      would go there are built by the extraction pass.
- [ ] Several sections in one document. The paginator takes one section's geometry; carrying a section
      change mid-document needs each paragraph to know which section it is in, which is the gap recorded
      under the document model above.
- [x] **All four formats reach the layout engine**, and all four paginate a five-page document the way
      LibreOffice does — same page count, same words on every page, verified against its own rendering of
      each format. `DocumentPaginationTests` is the test that proves the links are connected rather than
      each one working alone.
- [x] The one place the formats genuinely disagree, found by measuring rather than by reading a
      specification: whether two paragraphs' spacings add or the larger wins. LibreOffice adds for ODF,
      DOC and RTF, and takes the larger for DOCX — the same source document exported four ways puts its
      41st line 5.65 pt lower in three of them, which is exactly one paragraph space-after. For DOCX the
      answer is read from the file (`w:doNotUseHTMLParagraphAutoSpacing`); for DOC it needs the `Dop`.
- [x] **Per-run formatting**, for ODF. A paragraph carries `PageRun`s when its formatting varies and none
      when it does not, which keeps plain prose on the single-face path — a paragraph split into runs it
      does not need loses the shaping context at each boundary and measures very slightly wide. The line
      filler measures across the runs, the line's height and ascent come from the runs on it, and each run
      draws with its own face, size and colour.
- [x] The line's height is the maxima taken three ways, not the tallest run's: `SwLineLayout::CalcLine`
      accumulates height, ascent and descent independently and finishes with
      `if (nMaxDescent > Height() - mnAscent) Height(nMaxDescent + mnAscent)`. Taking the tallest run's
      ascent agrees whenever one run is tallest in every direction — two sizes of one family — and
      disagrees the moment a line mixes families, since one face can own the ascent and another the descent.
- [x] ODF measures are rounded to whole twips as they are read, because Writer's core unit is twips and
      `SvXMLUnitConverter` converts straight into it: 2.5 cm is 1417.32 twips and LibreOffice keeps 1417.
      The same file read into Calc or Draw keeps 1/100 mm instead, which is why the rounding lives beside
      the Writer readers rather than in `Paperless.OpenDocument`.
- [x] Two ODF font-size quirks, both measured rather than read: nothing stating a size renders at **12 pt**
      (the item pool's default), and `fo:font-size="150%"` is 150% of *that* rather than of the enclosing
      text or of the style's parent — a 150% span inside an 11 pt paragraph renders at 18 pt, and a 150%
      style parented on a 20 pt one still renders at 18. A percentage arrives as `CharPropHeight` and
      `SvxFontHeightItem` resolves it against the height the item set holds at that moment, which for a
      style being built up from nothing is the pool default; so nested percentages do not compound either.
- [x] **Per-run formatting for all four formats.** Each reader states runs its own way and each produces
      the same `PageRun` list: an ODF `text:span` cascade, a `w:rPr` on a `w:r` with the toggle rule, a WW8
      CHPX with its character style, and RTF's loose control words snapshotted as the text is appended.
      Verified against LibreOffice run by run for the same document exported four ways.
- [x] Every format's run colour, which meant new readers in two of them: RTF's `\colortbl` and `\cf`, and
      WW8's `sprmCIco` palette and `sprmCCv` COLORREF. Both had a trap. The RTF table is zero-based and its
      conventional first entry is the *automatic* colour rather than black, so an off-by-one draws a
      red word silver. And a COLORREF is `0x00bbggrr`, so the outer bytes have to be swapped —
      LibreOffice does it with `BGRToRGB`.
- [x] Two WW8 bugs the run walk exposed, both in code the content pass shares. A run's character style is
      named by a sprm *inside* its CHPX, so a reader that decodes only the exception finds no emphasis at
      all in a document LibreOffice wrote — its DOC export states emphasis as a character style. And
      `istd` 0 is *Normal*, not "no character style": WW8 keeps paragraph and character styles in one
      table, so resolving index zero as a run's style lays the document's default font size over the
      paragraph's own and turns every run of an 11 pt paragraph into a 12 pt one.
- [ ] Tables. Skipped by every layout source rather than flattened, because a table is laid out as a grid
      and stacking its cells would give the page a height no table has.
- [ ] Tables spanning page breaks, with header-row repetition
- [ ] Floating objects and text wrap, including contour wrap
- [ ] Footnote placement — footnote area growth changes how much body text fits, which
      changes pagination, so it must feed back into the page-filling loop
- [ ] Columns; vertical and RTL writing modes
- [x] **Emit to `IDrawingSink`**: one glyph run per line, positioned at its baseline, with glyph ids and
      per-glyph advances rather than characters — a backend must not re-shape, since layout already
      committed to these advances when it decided where the lines broke. The run carries the resolved
      `FontReference` so a PDF backend embeds the face the document was laid out against rather than the
      one it asked for, and the text plus cluster map so extracted text and a `ToUnicode` map are correct.
- [x] Verified against LibreOffice's own rendering for all four formats, comparing a display list rather
      than pixels: each line's drawn origin against the first word's left edge, its summed advances
      against the last word's right edge, and the baseline pitch against the rendered line pitch. An image
      comparison answers "does it look the same", which is weaker and much harder to attribute — a
      baseline a point out and a wrong glyph look equally different.
- [x] And, for a mixed-formatting document, against the PDF's own text operators rather than against
      `pdftotext`'s word boxes. LibreOffice writes one `BT … ET` block per portion — per line, split again
      at every formatting change — which is exactly what a glyph run is, so the comparison needs no
      grouping and reads a *baseline* rather than a box top. Word boxes cannot do this: poppler groups
      them by vertical position, which puts a 22 pt word on an 11 pt line into a line of its own and
      scrambles the reading order of precisely the documents worth checking.

## Known deviations, measured

- Two of LibreOffice's numbers are reproduced by construction rather than derived, and both are recorded
  here so a future comparison does not mistake them for bugs:
  - Its PDF export adds **two twips** to every pen position horizontally and nothing vertically. With left
    margins of 1 cm, 2.5 cm and 5 cm it lays the body out at 567, 1417 and 2835 twips — its own RTF export
    says so — and its PDF puts the first pen at 28.45, 70.95 and 141.85 pt. Additive, not a scale.
  - Its font ascents and line heights differ from scaling the design metrics by **up to one twip**, because
    VCL rounds them through the reference device. For 11 pt Carlito the design metrics give an ascent of
    209.47 twips and it uses 210; at 9 pt they give 171.39 and it uses 171; at 18 pt the line height is
    439.45 and it uses 440. Eight of twelve sizes tested agree exactly, four differ by one twip, and no
    single scale factor reproduces the set — so this is left as a per-size constant of at most a twip. It
    shifts every baseline of one size equally and does not accumulate, which is what makes it tolerable.
- [ ] A rasteriser and a PDF writer. `Paperless.Rendering`'s two backends are still stubs; the display
      list they consume is now real, which is the half that had to come first.

## Open questions

- [ ] Tracked changes: render as accepted, or show change marks? LibreOffice shows marks by
      default; `LayoutOptions.AcceptTrackedChanges` exists to choose. Confirm the default
      matches the reference.
- [ ] How much compatibility-flag behaviour is worth implementing? There are dozens; only
      some matter visually.
