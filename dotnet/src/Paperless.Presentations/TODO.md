# Paperless.Presentations — TODO

The Impress equivalent: PPTX/PPT/ODP and variants.

The most rendering-dominated family — almost nothing flows, almost everything is a
positioned shape. So the shape model and theme resolution carry nearly all the fidelity
burden.

Reference: `research/04-impress.md`.

**Done: ODP extraction** (`odp`/`otp`/`fodp`), via `Paperless.OpenDocument`. Each slide
becomes a section, with its speaker notes as a section beside it; shapes are read in document
order, including grouped shapes and shapes with text bodies, both of which LibreOffice's own
HTML export loses. Hidden slides are extracted and flagged. A shape's own style joins the
character cascade, which is where nearly all of a slide's formatting lives.

**Done: PPT extraction** (`ppt`/`pot`/`pps`), via `MsBinary/` here and the Escher reader in
`Paperless.MsBinary`. Produces the same content tree as the ODF path for the same deck —
`ppt-features.ppt` is `slides-features.odp` converted, and a test asserts the two extract to
identical sections, so a divergence names which reader is wrong. Metadata comes from the OLE
property sets, not from anything PowerPoint-specific.

Not yet: the placeholder inheritance chain against the master slide (extraction does not need
it, slide rendering does), and everything below.

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
- [x] The persist directory and `UserEditAtom` chain (`MsBinary/PptPersistDirectory.cs`). The
      chain is walked newest-first from the offset the `Current User` stream names, and **the
      first offset written for an id wins** — later blocks are the superseded copies
      (`filter/source/msfilter/svdfppt.cxx:1379`). Both corpus decks have a single edit
      session, so a reader that ignored the chain entirely would pass on every file
      LibreOffice can write and then read a stale slide out of the first real PowerPoint file
      it met; the chain tests build their streams by hand for that reason. The walk's cycle
      guard is that the chain must strictly decrease, which is also how the format is written.
- [x] Atom/container records (`Paperless.MsBinary/Records/DffRecord.cs`, shared with Escher —
      the eight-byte header is the same one, and the two vocabularies interleave in one stream)
- [x] Text: `TextHeaderAtom`, `TextCharsAtom`, `TextBytesAtom`, `StyleTextPropAtom`
- [x] Escher shapes via `Paperless.MsBinary` — groups walked through, the group's own shape
      record recognised as the group rather than as a phantom empty shape in front of it
- [ ] `TextSpecInfoAtom` — per-run language and spelling state. Read only far enough to be
      skipped. Extraction does not need it; `ContentRun.Language` would.
- [ ] Master/slide relationships. `SlideAtom` states a master persist id and the layout's eight
      placeholder ids, and the masters' `TxMasterStyleAtom` records hold the per-outline-level
      character and paragraph defaults. Everything a slide does *not* state falls through to
      them, which is why `ppt-features.ppt` reports its title as unemphasised where the ODF
      deck reports it bold: the title's own `StyleTextPropAtom` states a mask of `0x040000`,
      colour alone, and the boldness lives in the master's `TxMasterStyleAtom` instance 0. The
      equality test against the ODF deck compares text, order and the hidden flag, and passes;
      it deliberately does not compare emphasis for this reason. Building the eight-level style
      sheet is the next piece of work here and is what rendering will need anyway.
- [ ] The `Environment` container's `FontCollection`, so a run's `cfTypeface` index resolves to
      a face name rather than to nothing.
- [ ] `OutlineTextRefAtom` is implemented but has **no corpus coverage**, and cannot get any
      from a file LibreOffice writes: its PPT exporter never emits the record —
      `grep OutlineTextRefAtom sd/source/filter/eppt/` returns nothing, while the importer at
      `svdfppt.cxx:6570` handles it. PowerPoint itself writes it for autolayout placeholders,
      whose characters then live in the slide's own entry in the document's `SlideListWithText`
      rather than in the shape. A reader without it loses every title and body of a
      PowerPoint-authored deck while reading a LibreOffice-authored one perfectly, so the code
      was written from the C++ and is waiting on a genuinely PowerPoint-written file to confirm.
- [ ] Encrypted files. The `CurrentUserAtom`'s header token distinguishes them — the corpus
      decks both carry `0xE391C05F`, and MS-PPT gives `0xF3D1C4DF` for an encrypted one — and
      nothing here checks it yet, so an encrypted deck reads as a tree of garbage rather than
      raising `PasswordRequiredException`. LibreOffice does not check it either; its PPT filter
      simply fails later, which is why there is no C++ line to cite for the second value.

### What PPT extraction was measured against

LibreOffice's `impress_html_Export` is close to useless as an oracle here: it emits only the
title and outline *placeholder* text plus notes, and drops every ordinary text box. Run over
`slides-ppt.ppt` — whose shapes are all plain text boxes — it produces a stylesheet and no
text at all. The PDF text layer is the usable reference.

| File | Similarity | Every difference, by name |
|---|---|---|
| `slides-ppt.ppt` | 1.0000 | Blank-line padding the PDF layer puts between text frames |
| `ppt-features.ppt` | 0.7000 | Four things, all of them the reference being narrower |

For `ppt-features.ppt` the differences are: the `• ` markers Paperless emits because a bullet
is text a reader sees and it exists nowhere in the file's runs (the ODF path does the same);
the speaker notes, which are not on a printed slide; the hidden slide's text, which PDF export
excludes by design and Paperless extracts and flags; and line breaks — the reference wraps
"Text in a custom shape" across two rendered lines and "A plain text box with an emphasised
word." across three, and orders two side-by-side frames by position where Paperless uses
document order. None is a defect.

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
