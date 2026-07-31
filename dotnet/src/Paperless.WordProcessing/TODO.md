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
- [x] Which section each paragraph belongs to, resolved by the layout pass in all four formats — see the
      layout section below for how differently the four say it. The *model* still records only the
      geometries, since the extraction tree has no place to hang a section index on.
- [ ] Headers and footers as page furniture in the *document model*. `WritingSection` has the slots and
      the fallback rules — first page, even page, default, with a default header appearing on a first
      page that asked for nothing else — but nothing populates them, because the flows are built by the
      extraction pass and the model pass that would connect them is not written. Layout does not wait for
      this: it reads the furniture itself, through the same walk the body uses.
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
- [x] Which slot each header story fills, for the layout pass: the six stories per section, in DOC's
      own order — even header, odd header, even footer, odd footer, first header, first footer, after
      the six separator stories that are not furniture at all. The *odd* one is the default slot, since
      a first page is a right-hand page. Word writes all six whether the section uses them or not, so an
      empty story is what distinguishes "no header" from "an empty header"; filling the slot anyway
      draws a blank line on every page.
- [ ] Connecting those stories to the section that names them in the *model*, which needs the
      six-per-section indexing carried through `WritingSection.Headers` rather than only into layout.
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
- [x] Header and footer frames beside the body, laid out per slot and cached — most pages of a document
      share one header, and shaping its text again per page would be the largest single cost of
      paginating a long one. `PageFurnitureSet` holds them, `PageGeometry.HeaderArea`/`FooterArea` say
      where they go, and `Paginator` places one of each on every page by the section's slot rules. All
      four formats reach it, each through the walk its body already uses — and each names its slots
      differently: DOCX by `w:type`, RTF by the suffix on `\header` (where `\headerr` is the *default*
      slot rather than a third one), DOC by the position of a story in the header subdocument, ODF by
      `style:header-left` for what everything else calls even.
- [x] Where a footer's first line goes, which the two families genuinely disagree about — five points
      apart on A4, so it is not a rounding matter. Word bottom-aligns the footer: its *last* line sits
      at `pageHeight - w:footer`, and a second line grows upwards. ODF top-aligns it below the body: its
      *first* line sits the footer style's own spacing below where body text stops, and a second line
      grows downwards. `PageGeometry.FooterOffset` carries the ODF answer and null means the Word rule;
      both were measured against LibreOffice's rendering of the same one-line footer in each format.
- [ ] Section and floating frames inside the body.
- [x] Several sections in one document. `PageBlock.SectionIndex` says which section a block belongs to and
      the paginator switches paper size, margins, breaking width and furniture at the boundary — and lays
      each block out at *its own* section's width, since a paragraph in a landscape section breaks where
      landscape says it does. `WritingSection.Break` decides whether the change costs a page: three of the
      four kinds start one and continuous deliberately does not.
- [x] Which section each block belongs to, per format. Three of the four delimit sections by position:
      DOCX by the `w:sectPr` inside the properties of the paragraph that *ends* a section, so the counter
      advances after the paragraph rather than before. ODF delimits nothing at all — a paragraph reaches its
      page description through its style's `style:master-page-name`, followed up the parent chain, and a
      paragraph naming one *starts a page* on that master. An empty name is not an absent one: ODF writes
      `style:master-page-name=""` to cancel a master a parent style named.
- [x] DOC's and RTF's section boundaries, which are as different from each other as either is from ODF's.
      RTF marks only section *ends* with `\sect` and its last section has none at all, so counting the
      marks is the whole of it. DOC delimits by position: the `PlcfSed`'s own positions are the section
      ends in character space, so which section a paragraph is in is a lookup rather than a count. Their
      break kinds are `\sbk*` and `sprmSBkc`, whose numbering is Word's own — 0 column, 1 continuous, 2
      page, 3 even, 4 odd, which is not the order the concepts are usually listed in.
- [x] Furniture per section in all four formats. DOC keeps six header stories *per section*, so a section's
      own six start six further along for each section before it. RTF writes a header in the preamble of the
      section it belongs to and a section stating none inherits the previous one's, since `\sectd` resets
      the geometry and leaves the running heads alone.
- [x] **A column section break**, `SectionBreak.NewColumn`: RTF's `\sbkcol`, OOXML's `nextColumn` and
      `sprmSBkc` of 0, which previously all read as continuous. The rule is Word's rather than the obvious one,
      and LibreOffice's own importer states it outright: *"Word 2013+ seems to treat a section column break as
      a page break all the time. It always acts like a page break if there are no columns, or a different
      number of columns"* (`dmapper/PropertyMap.cxx`). So the kind becomes a page break unless the section has
      two or more columns and the same count as the one before it. Measured: a three-section document whose
      middle section is `\sbkcol` renders as three pages, where reading the kind as continuous gives one.
      The one case that stays a true column break follows **Word rather than LibreOffice**, deliberately:
      LibreOffice's own code marks that branch broken (tdf#135343, "completely broken, producing a no-column
      section that starts on a new page"), so it cannot be compared against and matching it would mean
      reproducing a bug it has a ticket for.
      Two pre-existing bugs surfaced with it, both about ending a page in a multi-column section rather than
      ending a column — the difference is invisible until a section is only part way through its columns:
      a section *break* would put the next section beside the last one instead of after it, and the final
      flush would advance to column two and never write the last page at all. A document ending part way
      through column one of a two-column section silently lost its last page.
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
- [x] Tables as grids. `PageTable` is a block beside `PageParagraph`, and a cell is a `PlacedFlow` — the
      same type a header is, because a cell is the same thing: paragraphs stacked in a rectangle whose
      width decides their line breaks. `TableLayouter` resolves the circularity in two passes, since a
      cell's height needs its width, a row's height needs its cells, and a cell's rectangle needs the row
      heights. A cell spanning rows charges its height to the *last* row it covers, so a merge does not
      make its first row as tall as itself.
- [x] Tables spanning page breaks, with header-row repetition. A row moves whole rather than being cut,
      which is Writer's behaviour and not Word's, and the heading rows are placed again at the top of each
      continuation. Because the cells are laid out once relative to the table's own top-left, both are a
      shift rather than a re-layout — a long table is not shaped once per page it touches. Verified in all
      four formats against a sixty-row table that crosses a page break: same page count, same words on each
      page, heading row repeated, every line start within a tenth of a point.
- [x] DOCX's grid, which expresses the same two facts differently from ODF's and so is a separate set of
      mistakes to avoid. A horizontal merge is `w:gridSpan` with **no placeholder** for the columns it
      swallows, so the next cell starts at column plus span — the opposite of ODF's covered cells. A
      vertical merge is `w:vMerge`, a state rather than a count: `restart` opens one and a bare `w:vMerge`
      continues it, so a row span has to be counted by looking down the following rows and the rows must
      all be read before any span is known. And cell padding is stated twice, `w:tblCellMar` for the table
      and `w:tcMar` per cell, overriding **per side** — LibreOffice writes a `w:tcMar` holding only the
      side that differs, so taking the element as all four zeroes the other three.
- [x] RTF's `\cellx` edges, which the extraction pass already resolves into a grid — including the merges
      LibreOffice writes with no flag at all. What was missing was the per-cell *layout* paragraph lists,
      and the reason they were empty is worth recording: `\cell` closes a paragraph through a call that
      passed no group state, and without the state a paragraph reaches the content tree but never the
      layout pass. So a cell's text extracted and did not draw.
- [x] **`\clpadl` is the top margin and `\clpadt` the left.** Top and left are swapped; bottom and right
      are not. LibreOffice's own importer does this deliberately — "Top and left is swapped, that's what
      Word does", `rtfdispatchvalue.cxx`, `RTFKeyword::CLPADL` — so it is the specification's reading of
      those two words that is wrong, Word being what defines RTF in practice. `\trpadd*` maps straight
      through, so the two spellings of one quantity disagree about which side is which. Caught by
      measurement: LibreOffice exports 0.6 cm of left padding as `\clpadt340` and then renders it on the
      left.
- [x] DOC's `sprmTDefTable` edges, reusing the extraction pass's own `AssignColumns` and
      `ResolveVerticalMerges` — a cell draft now carries both a content list and a layout list, so the hard
      part is resolved once and whichever walk built the draft fills its own half.
- [x] **A PAPX whose `cb` is zero is `2 × cb'` bytes, not `2 × cb' − 1`.** A non-zero `cb` means
      `2 × cb − 1`, which is always odd, so a grpprl of *even* length must use the second form — which is
      half of them. Subtracting one there as well loses the PAPX's last byte, and a sprm walk that is one
      byte short at the end silently drops whichever sprm was last. Nothing looks corrupt: every sprm
      before it decodes perfectly and the document merely lacks one property. It cost a table cell its
      padding, and was found only because that cell's text was 14 points from where LibreOffice put it.
- [x] `sprmTCellPadding` and `sprmTCellPaddingDefault`, whose six-byte operands look identical and are
      not. The fourth byte is an `Fts` size type in the specific form, which must be 3, and nothing at all
      in the default form — LibreOffice's `ProcessSpacing` skips it as "unknown" and never tests it, so
      requiring 3 there rejects every default a real document writes. Each entry states one value and the
      sides it applies to, so a uniform table carries **four** of them; keeping only the last leaves three
      sides at Word's 108 twips.
- [x] **Cell shading**, for ODF, DOCX and RTF, drawn and compared against LibreOffice's own fills. It covers
      the whole cell rectangle, padding included — measured: LibreOffice fills 56.7 to 141.75 pt for a cell
      whose column runs 56.7 to 141.8, so the fill covers the cell and stops half a border short of the next.
      `fo:background-color`, `w:shd`'s `w:fill` and `\clcbpat`; null rather than white for "no shading", because
      the two differ. Three details worth keeping:
      - **`w:fill`, not `w:color`.** The latter is the *pattern's* foreground and only shows through a `w:val`
        that is not `clear` or `nil`; reading it gives the wrong colour on every ordinary shaded cell. RTF has
        the same pair, `\clcbpat` and `\clcfpat`, and the same trap.
      - **A reference can paint one shade twice.** LibreOffice's DOCX render fills each shaded cell in the
        corpus document at identical coordinates twice, where its ODF render fills it once — so the comparison
        is of *distinct* rectangles, or the same document passes in one format and fails in the other for a
        reason unrelated to where anything went.
      - Every shade before any text, rather than each cell's shade before its own text: a shade is opaque, so a
        cell whose content overflows would otherwise have that overflow painted over by its neighbour's fill.
- [ ] Cell shading for **DOC**, which is a bigger read than the other three: a per-band `WW8_SHD` array from
      `sprmTDefTableShd`, indexed by cell, with a newer three-sprm form (`sprmTDefTableNewShd` and its 2nd and
      3rd parts) carrying full RGB.
- [x] **Cell borders**, for ODF: read, drawn *consolidated*, and compared against LibreOffice's own strokes —
      axis, position, both ends and thickness, every one within a tenth of a point. What had to be right:
      - **One stroke per grid line, not four per cell.** Five horizontals for a four-row table and one vertical
        per column boundary, built by grouping the cells' edges onto the line they sit on and merging the runs
        that touch. Drawing twelve short segments would be right on the page and incomparable against the
        reference — which is the whole reason this needed a `PdfStrokes` reader before it could be attempted.
      - **A vertical stops where its boundary stops.** The corpus table's last row spans two columns, so that
        column's stroke is shorter than the rest — and it falls out of the merge rather than being special-cased.
      - **A grid line runs through the centre of its border**, and each stroke **overshoots by half its width at
        both ends**, which is what makes two perpendicular borders meet rather than leaving a notch.
      - **A border takes space**, and the arithmetic is per row rather than per table: a row owns *half* of each
        of the two grid lines it sits between, so a row 18.95 pt tall gains 0.5 pt from two 0.5 pt borders. And
        the table's **first** grid line sits half a border below its top edge, which is one shift of the whole
        table and was the last 0.25 pt of disagreement.
      - Width and colour, no style: a style is a rasteriser's problem and no two formats agree on the set, while
        a dashed border at the right width in the right place is far closer than none — and the width is the half
        that moves text.
- [x] Cell borders for **DOCX and RTF** as well, each with its own trap:
      - **DOCX writes `w:start` and `w:end`, not `w:left` and `w:right`.** OOXML has both — the logical pair is
        the ISO spelling and the physical pair the legacy one — and LibreOffice's export uses the logical names,
        so a reader that knew only `w:left` finds *no vertical borders at all* and draws five strokes where the
        reference draws nine. Both are read, logical first.
      - **`w:sz` is in eighths of a point**, the one OOXML measurement that is neither twips nor half-points.
      - **RTF states a border in two halves**: `\clbrdrl` and friends *select* a side, and the `\brdrw`,
        `\brdrcf` and `\brdrnone` words that follow describe whichever was last selected. So the selection is
        state between control words rather than an argument to one. `\brdrw` is in twips — three formats, three
        units for one quantity.
      - ODF's own trap, for the record: `none` is a *value* meaning there is no border, so it beats the
        `fo:border` shorthand rather than falling through to it.
- [x] The **table's own left edge** in DOCX and RTF, which is what had kept those two out of the border
      comparison, and — found on the way — a second rule that had nothing to do with the origin. Both are now
      in the comparison, every stroke within 0.05 pt of LibreOffice's.
      - **Writer positions a table by the centre of its left border; Word positions it by the border's outer
        edge.** So a Word-family reader has to add half the first cell's left border to whatever indent the
        file states. `DomainMapperTableHandler::endTableGetTableStyle`, in the block commented "Table position
        in Office is computed in 2 different ways" — "Writer starts a table in the middle of the border, Word
        starts a table at the left edge of the border, so emulate that by adding the half the width". Measured
        on the corpus table's first vertical: DOCX states `w:tblInd w:w="-5"` with a 0.5 pt border and renders
        at 56.70 pt where taking the indent literally laid it out at 56.45; RTF states `\trleft0` and renders
        at 57.00 where it had been laid out at 56.70. A quarter and a third of a point, and the same rule
        underneath both — RTF's `\trleft` becomes `w:tblInd` in `rtfdocumentimpl.cxx` and the two share
        dmapper from there. The **third** of a point is the trap: it looks like a different constant and is
        the same half-border, differing only because RTF's number was 0 where DOCX's was −5.
      - **Word 2007 to 2010 measured the indent to the cell's *text* instead**, and subtracted rather than
        added: `tblInd − max(halfBorder, first cell's left padding)`. The same function picks between the two
        on `compatibilityMode`, which is a `w:compatSetting` in Microsoft's namespace rather than `w:`; 15 and
        above is the modern rule, and **an absent setting means the old one** (`GetWordCompatibilityMode`
        returns −1, and every caller asks whether it is below 15). Nested tables always take the modern rule
        whatever the mode, with the indent floored at zero first. Not academic: the corpus table rewritten to
        mode 12 renders 3.00 pt further left, because its cells are padded 55 twips.
      - **A Word table joins its grid lines differently, and that is a *drawing* rule rather than a reading
        one.** Everywhere else a line overshoots the line it meets by half a width, which is what makes a
        corner solid. In a Word table an interior line meeting the *outer frame* stops at that frame's inner
        edge instead and lets the frame cover the join — `SwTabFramePainter::FindStylesForLine`,
        `paintfrm.cxx`, guarded by `bWordTableCell`, which is the document setting `TableRowKeep` that every
        Word-family importer switches on and the ODF one never does. Measured: the same table renders its
        middle horizontals 56.95 to 538.35 from DOCX and 56.45 to 538.85 from ODF. Half a point, invisible on
        paper and four failing assertions in the stroke comparison — and it is why `PageTable` carries
        `HasWordBorderJoins`, the one place a table's original format still shows after it has been read.
        **DOC needs the flag too** when its borders arrive: `ww8par.cxx` sets `TableRowKeep` as well.
      - Checked past the corpus, since a fix that was really "subtract a quarter point" would pass one
        document and fail the next: a table indented 567 twips and a bordered table nested inside a cell, both
        converted to DOCX and RTF, agree with LibreOffice to 0.05 pt. Worth knowing that the corpus does not
        cover this by itself — every other table document carries an *empty* `w:tcBorders`, so its border
        width is zero and the origin rule moves nothing. `table-borders.*` is the only file that would notice
        a regression.
- [ ] Cell borders and shading for **DOC**. Borders are a `BRC` structure through `sprmTSetBrc`, applied to a
      *range* of cells rather than to one; shading is the per-band `WW8_SHD` array noted above. Both are bigger
      reads than the other three formats needed.
      The grid places text correctly and draws nothing round it, which is the
      half a word-box comparison can check; borders need the sink's line and rectangle primitives and a
      resolved border model, and a border's width also eats into the cell's text area.
- [x] A table inside a cell, in all four formats. A cell holds *blocks* rather than paragraphs and
      `FlowLayouter` places a table among its lines, so a cell's content is exactly what a header's is —
      which is what made this a small change rather than a second layout path. The subtle part is that a
      nested table's cells carry page coordinates while a flow's lines carry flow-relative ones, so moving a
      cell has to move the tables inside it explicitly; forgetting it leaves the inner table near the page's
      top-left corner, where the pre-layout pass built it.
- [x] The two binary formats' nesting, which they express through the *paragraph* rather than the markup:
      `sprmPItap` and `\itap` give a depth, and an inner table's cells end at paragraph marks rather than
      at U+0007. So the depth cannot be read off the character — U+0007 always means the outermost table,
      while a paragraph mark means whichever level its own sprms name, and `sprmPFInnerTableCell` implies at
      least level two whatever the depth sprm says. Both assemblers now keep one open table per level and
      append a finished inner one to the cell of the level enclosing it, innermost first.
- [x] A table inside a header, in **all four formats**. `PageFurnitureSet` holds blocks rather than paragraphs
      and `ReadFlow` takes the same walk a cell does, so the layouter — which could always place one — is
      finally given one. What it buys is the two-part running head: one cell hard left and another hard right,
      on one line, which is how a running head is usually built. Dropping the table is not the harmless
      simplification it looks like, and that is the measured part: its paragraphs would stack as loose lines,
      giving the header a height no table has and pushing the body text down by the difference on **every**
      page. Verified against LibreOffice in all five files, positions and body offset alike.
      The two binary-family readers needed their *recording* changed rather than their layout, and RTF needed
      one thing more that is worth remembering: a header flow now has somewhere to put blocks, so the test for
      "is this flow laid out at all" had to stop meaning "is this the body". It had been guarding the *cell*
      path too, which meant a table in a running head closed normally and simply held nothing — a table with
      no cells draws no text and reports no error.
- [x] **Exact row heights**, in all four formats — the one row height that really is a height rather than a
      floor. `PageTableRow.HasExactHeight` says which, and the layouter then takes the stated height as the
      row's whatever its content, so the rows below it move up. Measured: a 0.8 cm exact row whose content
      needs 32.6 pt renders 22.70 pt, which moves every row after it.
      Three of the four formats say it by a **sign** rather than by a word: RTF's `\trrh` and WW8's
      `sprmTDyaRowHeight` are both "at least" when positive and exact when negative, so the parameter has to
      be kept signed — taking its magnitude, which both readers did, turns every exact row into a minimum.
      ODF distinguishes `style:row-height` from `style:min-row-height` by the attribute name, and a row stating
      both means the floor: a minimum the content may exceed is the weaker claim.
      This also uncovered a bug that had been silent since row heights were first read: DOCX's `w:trHeight`
      carries `w:val`, not the `w:w` a `w:tblWidth` carries, so it was being read with the width helper and
      coming back as nothing. For an "at least" height that is invisible — a zero floor is no floor — and it
      only surfaced when the same value became an exact height and produced a zero-height row.
      `sprmTDyaRowHeight` (0x9407) is newly read; the WW8 reader had not been reading row heights at all.
- [ ] Fitting a table to the page when its columns state no widths, and this is a **much bigger item than it
      reads as** — worth saying so, because the obvious guess is wrong. It is not "divide the text width
      equally": measured on a three-column table with every column style stripped, LibreOffice produces columns
      of 160.6, 107.1 and 214.1 pt, sized to their *content*. That is Writer's table auto-layout, the same
      class of algorithm as CSS's — minimum and maximum content widths per column, then distribution — and it
      needs the cells' text measured before the grid exists. The readers currently take the declared grid and
      nothing else, so a width-less table comes out with zero-width columns.
- [ ] Floating objects and text wrap, including contour wrap
- [x] Footnote **placement**, which is the half that changes pagination rather than appearance. The note
      area takes its room out of the body's, so a page with notes holds less text — and adding a note can
      push the line that cites it onto the next page, which removes the note again. That is a feedback loop
      and not a second pass, and the paginator resolves it by trying the unconstrained line count and
      shortening until it holds: each step removes a line and so can only remove notes, which is why it
      terminates.

      The geometry is measured rather than guessed, from `footnotes.fodt` rendered at A4 with 2 cm margins:

      - **The note area is bottom-aligned inside the body text area.** The last note line's box bottom
        coincides with the body area's bottom to a fiftieth of a point — 785.23 against 785.2. That is
        exactly what `FlowLayouter` does with a null `offsetFromTop`, the rule a Word footer follows, so
        placing the area needed no new machinery.
      - Note lines break at the **full body width**, not a column's and not an indented one.
      - A note paragraph's line height is its own style's: 12.20 pt for the 10 pt note against 13.42 pt for
        the 11 pt body in the corpus document.

- [x] Footnote **reading**, for ODF. A note is a body of blocks plus the character offset of its anchor.
      The bodies are read *after* the citing paragraph's own walk rather than during it, because reading one
      recurses into the paragraph reader — doing it mid-walk numbers a note cited inside note one as note two.
- [x] The **citation**, which is two things and neither is in the file. LibreOffice draws the number twice:
      as a superscript run at the anchor in the sentence, and again at the start of the note's first line,
      where it fuses with the first word — its rendering reads `1Note 2 text alpha bravo charlie.`. And it
      **renumbers**, ignoring `text:note-citation` and counting in document order, which turns a document's
      stated citations of 2 and 5 into 1 and 2. Both are now produced, and the corpus document states 2 and
      5 deliberately so that a reader taking the file at its word fails.
- [x] `style:text-position`, which is what makes the citation superscript, and `PageRun.Rise` to carry it.
      The rise moves the run's origin rather than its glyphs, so the pen along the baseline carries on
      unchanged — a raised run advances the line as if it were not raised. `super` and `sub` are ±33% of the
      font size, LibreOffice's automatic values; the size half of the attribute is deliberately left to
      `SizeIn`, which already walks the cascade multiplying percentages, so that a span stating both does not
      have it applied twice.
- [x] Footnote reading for **DOCX**, whose note store is `footnotes.xml`/`endnotes.xml`, keyed by the `w:id`
      a `w:footnoteReference` carries. Two things differ from ODF and both were measured:
      - The note body **marks where its own citation goes**, with a `<w:footnoteRef/>` in its first
        paragraph, so the number is emitted there rather than prepended. Anywhere else in that paragraph and
        it would be, since a note can begin with a tab.
      - That citation is nonetheless **not formatted by the file**. LibreOffice exports the run around it
        with the character style `FootnoteCharacters` and an *empty* `w:rPr`, leaving the superscript to a
        built-in style the file never defines. So the citation falls back to the same 58%-size, 33%-rise
        default the ODF reader applies; a reader taking the file at its word draws the number full size on
        the baseline, where it fuses with the note's first word and the comparison reads
        `2Note 5` against LibreOffice's `2 Note 5`.
      `w:vertAlign` supplies `PageRun.Rise` for the anchor's own citation, which *is* stated — the run in
      the sentence carries `superscript` properly.
- [x] Footnote reading for **RTF**, whose note is a `{\*\footnote …}` group sitting *inside* the sentence,
      immediately after the `\chftn` that cites it. So the note is finished before the paragraph carrying it
      is, which is the opposite of every other format: the body is collected onto the flow and the paragraph
      claims it when it closes. Three details are RTF's own:
      - The anchor's offset has to be taken at the `\chftn`, before the group opens — by then the reader has
        pushed a flow of its own and the citing paragraph is no longer the current one.
      - The note body opens with a **second** `\chftn` repeating the same number. Extraction deliberately
        drops it (the note's section already carries the number as its name), so layout takes it from a
        separate `LayoutPrefix` on the flow — the one place the layout text differs from the extracted text
        by more than a character's identity.
      - An endnote is a `\footnote` group whose *first* control word is `\ftnalt`. LibreOffice peeks seven
        bytes ahead in the stream to see it; reading the flag when the tokeniser reaches it says the same.
- [x] **`Escapement`**, and it is worth reading before touching a superscript. The rise is a percentage of
      the font's **height** — ascent plus descent plus line gap — not of its em size, which for a Latin face
      is about 22% smaller. Reading it as the em size puts a citation 0.7 pt low at eleven point, seven times
      the tolerance a comparison runs at, and no word-box test can see it: a box top carries the font's
      ascent, so a rise and a size change are indistinguishable from boxes alone. Measured off the PDF
      content stream at two sizes, where it comes out exact both times — an eleven-point citation is raised
      4.40 pt and a ten-point one 4.00 pt, which is 33% of Carlito's height truncated to whole twips. The
      size is 58% of the em, also snapped to twips, which is why LibreOffice draws 6.4 pt and not 6.38.
      Both halves now live in one place and each reader hands it the face, because the face is what the
      percentage is of. Two bugs fell out of writing it down: ODF's `style:text-position="super 58%"` and
      DOCX's `w:vertAlign` were shrinking nothing at all, so both drew their anchors full size.
- [x] The **asymmetry between the two citations**, which is measured rather than assumed. Writer has two
      built-in character styles and they do not agree: `Footnote Anchor` carries an automatic superscript and
      `Footnote Characters` carries *nothing* — `DocumentStylePoolManager.cxx` falls straight through for the
      latter. So the number in the sentence is raised and shrunk, and the number at the head of the note is
      full size on the note's own baseline. LibreOffice renders ODF exactly that way. A DOCX gets a raised one
      anyway, because its file names `FootnoteCharacters` on the run and LibreOffice's import gives that style
      the shift; an RTF gets one because the note-body `\chftn` inherits the `\super` still in force from the
      anchor's group. So all three are right for three different reasons.
- [x] Footnote reading for **DOC**, which turned out the cleanest of the four once the tables were found. Two
      PLCFs make a note: `PlcffndRef` gives the body positions of the references and `PlcffndTxt` the extents
      of their texts in the footnote subdocument, with the *n*th reference owning the *n*th text. The endnote
      pair is the same arrangement over a different subdocument — and that pairing is the *only* thing that
      tells the two kinds apart, because the reference character in the body is the same U+0002 for both.
      Three things fell out pleasingly:
      - The citation is emitted **at the reference's own position** rather than as a synthetic run, so the
        CHPX covering that character governs it — and Word writes the mark with a character style carrying
        `sprmCIss`, which is what makes it superscript. No default needed anywhere.
      - The mark at the head of a note is the same U+0002, so the note body's number needs no separate
        mechanism: reading a note's range with the citing number in hand is enough, and only a reference in
        the *body* advances the counter.
      - A note's text lives in a different subdocument than the body, so a note cannot contain its own
        reference and the recursion is bounded by the ranges themselves.
      `sprmCIss` (0x2A48) is now read into `Ww8LayoutFormat.Escapement`; its companion `sprmCHpsPos` (0x4845)
      states a half-point offset outright and is still unread, so a document using one gets no shift rather
      than a wrong one.
- [ ] RTF footnotes cannot be compared against LibreOffice's *rendering*, and the reason is upstream rather
      than here: LibreOffice's RTF import drops the character and paragraph formatting stated inside a
      `{\*\footnote …}` group and falls back to the document's defaults — a note the file sets in Carlito at
      10 pt with no indent renders in Liberation Serif with a 340-twip hanging indent. Reproduced on a
      hand-written three-line RTF, so it is not an artefact of the corpus export. Paperless reads what the
      file says; `FootnoteReadingTests` checks the notes structurally instead, and `tests/corpus/README.md`
      records the measurement. DOC has no such problem: its notes are compared word for word and pen for pen.
- [x] **Note numbering the document states**, in `NoteNumbering`: the sequence and the start value, per class,
      read from all four formats. The defaults stay what LibreOffice does when a file says nothing — footnotes
      1, 2, 3 and endnotes i, ii, iii. What each format says and how it lies:
      - **ODF**'s `text:notes-configuration` states the format *by example* — `style:num-format="I"` is the
        literal numeral, not a name — and its `text:start-value` is an **offset, not the first number**. A
        document stating 7 renders VIII and IX, because LibreOffice maps the attribute to
        `SwFootnoteInfo::nFootnoteOffset` and adds one. Measured; a reader taking it at face value is out by
        one on every citation.
      - **OOXML** names the format (`w:numFmt w:val="upperRoman"`) and its `w:numStart` *is* the first number.
      - **RTF** puts the sequence in the control word itself — one word per format, `\ftnnar` through
        `\ftnnchi`, doubled with an `a` prefix for the endnotes — and `\ftnstart` is one-based.
      - **DOC** packs it into the DOP three ways: `nFootnote` at 0x02 and `nEdn` at 0x34 above two bits of
        restart rule, and both sequence codes four bits each in one word at 0x36. The `MSONFC` order has a
        trap in it: **upper** roman is 1 and lower roman is 2, so assuming the lower-case form comes first
        gives I where the document says i.
      Word's `chicago` sequence is modelled too, since it is the one format that is not an arithmetic
      progression — the fifth mark is `**` rather than a fifth symbol.
- [x] Note numbering **restarts**, the *rule*: read from all four formats into `NoteRestart` and carried on
      `PageNote` beside `Placement`, which is the same kind of thing — a property of the note's class that only
      pagination can act on. `note-restart.*` is the corpus case, and building it first settled the shape of the
      problem: all four formats keep the rule through LibreOffice's own export and all four then render
      *identically*, page one citing 1, 2, 3, 4 and page two citing 1, 2, 3, 4 again.
      Three findings, each of which would otherwise have been a wrong guess:
      - **There are three restart kinds, not four.** Writer's `SwFootnoteNum` is exactly
        `FTNNUM_DOC`/`FTNNUM_CHAPTER`/`FTNNUM_PAGE` (`ftninfo.hxx:92`), and every importer folds a
        per-*section* restart onto the chapter one — WW8 through
        `{ FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC }` (`ww8par.cxx:5064`), OOXML's `eachSect`
        (`docxattributeoutput.cxx:9329`) and RTF's `\ftnrestart` (`rtfexport.cxx:970`). A fourth member would
        model something no reference implementation round-trips.
      - **WW8's own comment contradicts its own code.** `ww8scan.hxx:1624` documents `rncFootnote` as
        "1 section, 2 page"; the mapping array reads 1 as *chapter*. The comment describes Word's field, the
        array what Writer does with it. The bits cost nothing to read, incidentally — they are the two the DOP
        reader was already shifting past to reach the start value, in the same word at 0x02 and at 0x34.
      - **RTF's endnote set is one word short rather than a mirror.** `\ftnrstcont`, `\ftnrestart` and
        `\ftnrstpg` have `a`-prefixed twins for only the first two: there is no `\aftnrstpg`, because an endnote
        does not restart per page. A reader assuming the mirror would be inventing a keyword.
- [ ] Note numbering restarts, the *application* — assigning the numbers, which is pagination's half and is
      genuinely circular: a note's number under a restart is its position within its page, the citation's width
      depends on that number, and where the citing line breaks depends on the width. **Writer has an answer and
      it is not a fixed point**, which is the correction to the note that used to stand here:
      - It numbers per page as a pass over pages that already exist — `SwRootFrame::UpdateFootnoteNums`
        (`ftnfrm.cxx:971`) runs only for `FTNNUM_PAGE` and walks the finished page frames, and
        `SwPageFrame::UpdateFootnoteNum` (`ftnfrm.cxx:2564`) counts one page's notes and assigns 1..n.
      - The loop is real: `SwTextFootnote::SetNumber` (`atrftn.cxx:362`) invalidates the citing text node *and*
        every text node of the note's own body, so a renumber can rebreak the line that cites it.
      - And Writer **damps rather than iterating**. `flowfrm.cxx:2268` renumbers both pages the moment a note
        moves between them, and `txtftn.cxx:560` then validates the frame under the comment
        *"We break the oscillation"* — accepting a layout one iteration stale rather than chasing a fixed point
        that need not exist.
      So the shape is: paginate with the document-order numbering, renumber per page over the finished pages,
      re-lay out once, and stop. The corpus document deliberately holds four notes to a page so that every
      citation stays one digit wide — the numbering can be got right before the width feedback is, and a second
      document with ten notes on a page is what would exercise the rest.
- [x] **The separator rule above the notes**, drawn and compared — and the interesting part is that it turned
      out to be comparable at all. The old note here said it could not be, that it needed the rasteriser first
      because a text comparison sees no lines. That was the wrong conclusion from a true premise: `pdftotext`
      sees no lines, but the PDF's **content stream** does, and LibreOffice writes the rule as an explicit
      closed rectangle. `PdfFills` reads them, so a rule is now compared at the same tenth of a point as
      everything else — and every constant in it is measured rather than assumed:
      - a quarter of the text width (56.7 to 177.15 pt on an A4 page with 2 cm margins, so 120.45 of 481.89),
      - half a point thick,
      - left aligned at the text area's left edge,
      - and 0.1 cm above the first note's line box.
      **Filled rather than stroked**, which matters: a stroke is centred on its path, so the same coordinates
      stroked would put half the thickness on the wrong side of the line. LibreOffice fills a closed rectangle
      and so does this.
      The same reasoning applies to everything else drawn that is not text, so this unblocks more than itself —
      see the cell-borders item, which no longer has to wait for a rasteriser either.
- [x] **Endnotes**, which are the same thing to read as a footnote and a different thing to place: a footnote
      takes its room out of the page that cites it, and an endnote takes none at all. Measured — LibreOffice
      leaves the citing page holding every body paragraph and puts the notes at the top of a fresh page after
      the last of them, in the body's own text area. So it is an ordinary pagination of an ordinary flow, done
      by recursion rather than a second implementation: the notes get page breaks, headers and footers exactly
      as body text does, because on those pages they *are* body text.
      Three things had to change with it:
      - `LaidOutPage.Blocks`, because an endnote page's lines index into the endnote flow rather than the
        body's block list. Without it an endnote page draws the body's first paragraphs at the endnotes' line
        lengths, which looks like a layout bug and is an indexing one.
      - **Two numbering sequences, formatted differently.** LibreOffice cites footnotes 1, 2, 3 and endnotes
        i, ii, iii, and the corpus document renders "i" and "ii" both in the sentence and at the head of the
        note. A single counter gets both the numbers and the numerals wrong.
      - In RTF the class is only knowable by **peeking**: the `\ftnalt` that marks an endnote is inside the
        `{\*\footnote …}` group, which comes *after* the `\chftn` where the citation has to be numbered and
        appended. LibreOffice's own importer peeks seven bytes ahead for exactly this; the reader now looks in
        the same short window.
- [x] **Note placement read apart from note class**, in `NotePlacement`. An endnote is not always at the end
      of the document and the exception is not exotic: LibreOffice's own WW8 export writes the DOP's `epc` as
      zero, "collect at the end of the section", so a document round-tripped through DOC has its endnotes in
      the page-bottom note area instead of on pages of their own. Read from all four — RTF's
      `\aenddoc`/`\aendsec`, OOXML's `w:pos`, the DOP's `epc`, and ODF's `text:footnotes-position` (which
      exists for footnotes only; ODF cannot move its endnotes off the end of the document at all). The
      paginator now places by position rather than by class, which turned `endnotes.doc` from a documented
      divergence into a compared case: one page rather than two, notes at the foot.
      One trap worth naming, because it produced the only failure: a reader building a fresh `NoteNumbering`
      from a document's stated format and start value silently loses the class's *default placement*, so a
      document declaring a configuration purely to set a start value has its endnotes moved to the foot of the
      page. The fallback's placement has to be carried through explicitly.
- [ ] Section-end endnotes on a section spanning **more than one page**. Writer collects them on the section's
      last page; this places them at the foot of the page that cites them, which is the same thing for a
      one-page section — the case LibreOffice's own DOC export produces and the one the corpus verifies — and
      not for a longer one. Doing it properly is the footnote feedback loop at section granularity: which page
      is last is not known until the section is full, and reserving the notes' room changes which page that is.
- [ ] Vertical and RTL writing modes
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

- [x] **Justification**, which stretches a line rather than shifting it: the slack divided over the line's
      blanks, in hundredths of a twip and truncated, exactly as `SwTextAdjuster::CalcNewBlock` computes
      `nGluePortionWidth / nGluePortion`. It lands on each blank's own advance, where Writer's kern array
      puts it, so a run stays one draw call and its glyph positions stay self-consistent.
- [x] **Mandatory breaks**, which the line filler previously ignored — it treated every UAX #14 opportunity
      as optional, so a manual line break in a line that still fitted did nothing at all and two of the
      document's lines shared one of the page's. `ILineBreaker` now answers which breaks are required, and
      a required break also has no width: the separator is trimmed from the line's visible text, or the
      line would stretch by one glyph too little and draw a `.notdef` box at its end.
- [x] **Tab stops**, all four alignments. A tab is the one character whose width is not a property of the
      font — it is the distance to the next stop — so one walk (`TabRuler`) serves both the measuring and
      the drawing, and a line is split at its tabs when it is drawn. A left stop puts the following text's
      start on it, a right stop its end, a centre stop its middle and a decimal stop its separator; a stop
      that cannot hold what follows is skipped rather than drawn backwards over the column before it.
- [x] The stops themselves, for ODF and DOCX. Both state them as a list that *replaces* the style's rather
      than adding to it, so the innermost layer declaring one wins whole. Two per-format traps: ODF's
      default interval is **1.25 cm**, not the half inch Word uses — measured, a tab in a paragraph with no
      stops lands 709 twips along — and its `style:type="char"` is only a decimal stop when `style:char`
      names a separator, behaving as a right stop otherwise, which is what LibreOffice renders.
- [x] Tab stops for DOC and RTF too, so all four formats now advance a tab to the same place. WW8's
      `sprmPChgTabsPapx` states a *change* rather than a list — a count of positions to delete, a count to
      add, and then one descriptor byte each — so applying the style chain and then the direct formatting in
      order is what turns it into a list. RTF states the kind *before* the position (`\tqr\tx5000`), so the
      kind is held and consumed by the `\tx` that follows.
- [x] The `Dop`, at last: its `dxaTab` is where a DOC's default tab interval lives, and reading it was the
      only way the DOC agreed with the other three — LibreOffice writes 709 twips into a DOC it exported
      from an ODF document, not Word's 720, so assuming either constant is wrong. It is FIB entry **31**,
      two before the piece table's 33. The same record's `fDontUseHTMLAutoSpacing` now answers whether the
      two paragraph spacings collapse, which was the last thing the DOC path had hard-coded.
- [ ] A justified line that also holds a tab is left ragged. Writer stops justifying at a centre, right or
      decimal tab and gives each stretch between tabs its own space-add, which is a per-stretch answer where
      the engine has one per line; stretching the blanks anyway would move text out of the columns the tabs
      put it in, which is the more visible error.
- [ ] A decimal stop aligns on the last `.` or `,` in the stretch rather than on the character the document
      named. `TabStop` would need to carry it; ODF states it as `style:char` and OOXML does not state it at
      all, so the two would disagree about a `1.234,56`.

## Known deviations, measured

- Two of LibreOffice's numbers are reproduced by construction rather than derived, and both are recorded
  here so a future comparison does not mistake them for bugs:
  - Its PDF export adds **two twips** to every pen position horizontally and nothing vertically. With left
    margins of 1 cm, 2.5 cm and 5 cm it lays the body out at 567, 1417 and 2835 twips — its own RTF export
    says so — and its PDF puts the first pen at 28.45, 70.95 and 141.85 pt. Additive, not a scale.
  - It breaks a **justified** paragraph's lines differently from the same text left ragged. Measured on a
    corpus document holding both: a line that takes twenty-one words ragged takes nineteen justified,
    though the ragged line ends well inside the margin. The greedy break the two agree on everywhere else
    is evidently not what 24.2 does for justified text, and the C++ tree here is a newer version than the
    installed binary — its word-spacing and shrinking machinery does not describe what 24.2.7 did. So the
    justification comparison uses lines fixed by a manual break, where the words cannot move, and the
    break rule itself is left as it is: agreeing with Writer on ragged text and on where justified text
    *would* break by the same rule.
  - It measures a line about **0.15%** wider than HarfBuzz does — twelve twips on a 395 pt line. Invisible
    to a direct comparison, because `pdftotext`'s own quantisation is larger, and visible in justification
    only because the slack is divided over the blanks: 0.7 twips of stretch each.
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
