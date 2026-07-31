# Paperless.OpenDocument — TODO

Shared ODF infrastructure: style families and resolution, common attributes, flat XML.

**Done: extraction for `odt`/`ods`/`odp` and their template and flat variants.** The three
family readers sit on top of this library and are thin — the style resolution, the text walk
and the table walk all live here, because ODF's text content model is common to all three
applications, exactly as LibreOffice shares `XMLTextImportHelper` across Writer, Calc and
Impress.

Reference: `research/02-writer.md` section D; `research/05-infrastructure.md` section E for
the item-set semantics being reproduced.

## Style resolution

The core semantic, and the thing to get exactly right.

- [x] Parse the three style containers, all of which participate:
      `office:styles` (named), `office:automatic-styles` (generated, standing in for direct
      formatting), `office:master-styles` (page and slide masters). Merged across
      `styles.xml` and `content.xml` into one `OdfStyles`.
- [x] Walk `style:parent-style-name` upwards, then fall back to `style:default-style` for
      the family. Cycle-guarded — illegal but present in converter output.
- [x] **Keep "set here", "inherited" and "defaulted" distinguishable.** `OdfPropertyOrigin`
      does this, and `OdfProperty.SourceStyleName` records *which* style supplied the value.
- [x] All style families (`OdfStyleFamily`), plus `PageLayout`, which is written as
      `style:page-layout` rather than `style:style` but resolves like any other style.
- [x] `style:*-properties` child elements per family, keyed by `OdfPropertyKind` as well as
      by name — `fo:background-color` means different things in text, paragraph and cell
      properties, and one style carries several sets at once.
- [x] Cascade resolution for a run: paragraph style, then spans, innermost wins, with the
      family defaults applying only where nothing in the cascade sets a value. The two-pass
      precedence is the subtle part — resolving each reference independently would let an
      inner style's *default* beat an outer style's explicit value.
- [x] Character defaults fall back to the paragraph family default, because ODF declares no
      `style:default-style` for the text family.
- [x] List styles and the outline style, with generated labels (`OdfListStyle.FormatLabel`).
- [x] Data styles (`number:`) parsed and kept.

Still open here:

- [ ] **Apply** data styles to format a value. Not needed for extraction — an ODF cell
      carries its formatted appearance as `text:p` alongside the raw value, and re-deriving
      it would risk disagreeing with what the authoring application showed — but rendering
      will need it for cells whose cached text is absent or stale.
- [ ] Presentation placeholder inheritance: `presentation:class` against the master slide's
      matching placeholder. Extraction does not need it; slide rendering will.
- [ ] `style:*-properties` for the families extraction does not read yet (chart, ruby).

## Common parsing

- [x] ODF length values with units (`cm`, `mm`, `in`, `pt`, `pc`, `px`) → `Length`, plus the
      unitless 1/100 mm form converters emit
- [x] Percentages, kept separate from absolute lengths where they resolve against a parent
- [x] Colours (`#rrggbb`, `transparent`)
- [x] Booleans (both the keyword and XSD numeric spellings), dates, ISO 8601 durations
- [x] Style-name decoding (`Text_20_body` → `Text body`)
- [ ] `fo:` box-model attributes: borders, padding, margins, backgrounds. Parsed as raw
      property values today; no typed border/box model yet, which layout will want.
- [ ] `svg:` position and size attributes — same: available, not yet typed.

## Flat XML

- [x] `.fodt`/`.fods`/`.fodp`: one document with `office:body` inline, no package. `OdfFile`
      presents both forms identically, and the reader is chosen by looking at the bytes
      rather than the extension. A fidelity test asserts a packaged document and its flat
      form extract to identical text.

## Metadata

- [x] `meta.xml`: Dublin Core plus `meta:*`, including `meta:document-statistic` and
      `meta:user-defined` with its declared value types. The statistics counters are
      interpreted per family, since ODF reuses `meta:table-count` for a spreadsheet's sheet
      count and `meta:page-count` for a deck's slide count.
- [ ] `settings.xml` where it affects rendering. Parsed and exposed as
      `OdfFile.Settings`; nothing reads it yet. View settings mostly do not matter; some
      compatibility flags do.

## Content extraction

- [x] Paragraphs, headings (both `text:h` and a paragraph style with
      `style:default-outline-level`), spans with resolved emphasis and language, hyperlinks
- [x] ODF white-space collapsing, `text:s`, `text:tab`, `text:line-break`
- [x] Lists: XML-nesting depth plus generated markers; continuation paragraphs; list headers;
      `text:numbered-paragraph`
- [x] Tables: header rows, row groups, column and row repeats, spans, covered cells; and for
      spreadsheets typed values, formulas and error results
- [x] Frames: images recorded with their part name and alternative text; text-box flows
      hoisted into their own section rather than spliced into the anchoring paragraph
- [x] Shapes, groups and shape text; shape styles joining the cascade so slide text is
      formatted
- [x] Notes and comments hoisted, with the note citation left inline where a reader sees it
- [x] Fields, by recursing into unknown inline elements to pick up the cached result
- [x] Indexes and tables of contents, via `text:index-body`
- [x] Tracked changes, bookmarks and fields — as *positions*, through the `IOdfMarkSink` hook.
      The walk still does not recurse into `text:tracked-changes`, which holds *deleted* text no
      reader displays, and still emits none of it. What it does now is tell a sink where each
      `text:change-start`, `text:bookmark-start` and field element sat, at an offset into the
      paragraph's own text. The sink is an interface rather than a collection because what a mark
      *means* is a Writer concept — a bookmark's range, a redline's author — and this library sits
      below `Paperless.WordProcessing`; `OdtMarkSink` there resolves the ids against the regions.
      Null for Calc and Impress, and null for a text document too until someone asks, so the cost of
      the hook on an unmarked document is one null check per paragraph.
- [ ] Embedded objects (`draw:object`): recorded as a graphic, not opened. An embedded
      spreadsheet inside a text document is a whole nested document.
- [ ] `text:ruby` annotation text — the base is read, the gloss is dropped.

## Legacy OpenOffice.org 1.x

- [ ] `sxw`/`sxc`/`sxi`: same shape, older namespaces. Low priority — rare in practice. The
      readers currently report them as unimplemented rather than mis-reading them.
