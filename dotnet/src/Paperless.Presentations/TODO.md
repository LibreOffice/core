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

**Done: PPTX extraction** (`pptx`/`pptm`/`potx`/`potm`/`ppsx`/`ppsm`), producing the same
content-tree shape. Every part is located by relationship; slide order is `p:sldIdLst`'s order
of `r:id`s. Text bodies go through a reader shared with the other two OOXML families
(`Paperless.Ooxml/DrawingML/DrawingTextBody.cs`), because a text body is identical in a deck, a
spreadsheet drawing and a Word shape — only the element wrapping it is namespaced per family.

Not yet: the *formatting* half of the inheritance chain (extraction needs only enough of it to
attribute text and resolve bullets; rendering needs fill, line and font), charts, and everything
below.

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
- [x] Slide size (`p:sldSz`, in EMUs already — the one office measurement needing no conversion)
- [ ] Background fill with inheritance
- [ ] Presentation styles and outline-level styles
- [ ] Animations and transitions — extract only; do not attempt to render

## The inheritance chain

**Get this exactly right; it is the most common cause of wrong-looking slides.** A shape's
effective formatting resolves through:

```
shape → layout placeholder → master placeholder → theme defaults (spDef/lnDef/txDef)
```

resolved **per text level** for list styles (`lvlXpPr`).

- [x] Placeholder matching between slide, layout and master, by type and index —
      `Ooxml/PptxPlaceholders.cs`, ported rung for rung from `oox/source/ppt/pptshape.cxx:715-820`.
- [x] Per-level paragraph-property inheritance, as far as extraction can observe it (the bullet).
      `Ooxml/PptxTextStyles.cs` builds the chain; `DrawingTextBody` walks it per level.
- [ ] Per-level *character* inheritance — `defRPr` size, colour, typeface. Needed by rendering,
      invisible to extraction, so untouched.
- [ ] Theme default shape/line/text properties (`a:objectDefaults`)
- [ ] Background inheritance, including `showMasterSp`

### What was learned building it

**The layout beats the master because of a reversed walk, not because it is searched first.**
LibreOffice imports a layout *into* the master's `SlidePersist` — `LayoutFragmentHandler` is a
`SlideFragmentHandler` over the master's persist with `ShapeLocation::Layout`
(`oox/source/ppt/presentationfragmenthandler.cxx:287`) — so one shape list holds the master's
shapes then the layout's, and `PPTShape::findPlaceholder` iterates it **backwards**
(`pptshape.cxx:791`). Search the two trees in the obvious order and the master silently wins
every tie. `PptxTextStyles.For` concatenates in the same order, for the same reason.

**`<p:ph>` with no `type` is `obj`, not `body`.** ECMA-376 gives `CT_Placeholder/@type` a default
of `body`; LibreOffice reads `obj` (`oox/source/ppt/pptshapecontext.cxx:68`). Not a hypothetical
divergence: LibreOffice's own PPTX export writes a bare `<p:ph/>` for every outline placeholder
it emits, so it is in the first deck anyone round-trips through Impress. Paperless follows
LibreOffice — the point of the exercise is agreeing with the reference — and the two mostly land
in the same place anyway, since `obj`'s fallback match type *is* `body`. The related trap: `idx`
may be `4294967295` (`SAL_MAX_UINT32`, an unsigned −1), meaning "inherit nothing"; read into a
signed `int` it overflows, and read as an unsigned and kept it matches nothing by accident.

**Placeholder text is never inherited, only formatting.** A master's placeholders carry prompt
text — "Click to edit the title text format", then one sample paragraph per outline level — and a
reader that fills an empty slide placeholder from its layout puts that on every slide of every
deck. LibreOffice does not: `Shape::applyShapeReference` copies geometry, fill, line and list
style and then **resets** the text body (`oox/source/drawingml/shape.cxx:570-573`), and
`pptgraphicshapecontext.cxx:135-155` passes `bUseText=false` for every placeholder type there is.
Measured across all 389 PPTX files in `sd/qa/unit/data/pptx/`: zero occurrences of "Click to
edit" in the extracted text.

**A LibreOffice-written deck puts its per-level list style somewhere the specification does not
point at.** The exporter writes no `<p:txStyles>` at all. Instead the master placeholder's own
demonstration paragraphs — one per level, `lvl="0".."6"` — carry the per-level `a:pPr`, and
`PPTShape::setTextMasterStyles` (`oox/source/ppt/pptshape.cxx:162-233`) pushes them into the
presentation style for that outline level on import. So the chain has to consult a layout or
master placeholder's *paragraphs* as well as its `a:lstStyle`, or a LibreOffice-authored deck
inherits nothing while a PowerPoint-authored one inherits correctly.
`PptxTextStyles.FromPlaceholder` does both, `a:lstStyle` first.

**An empty paragraph draws no bullet and consumes no number.** The blank line an author leaves
between two items is still an `a:p` and still inherits the level's bullet. Emitting it produced a
stray `a.` after the last real item of `sd/qa/unit/data/pptx/NumberedList-12ab-ab-34.pptx`, whose
expected rendering LibreOffice's own layout test spells out
(`sd/qa/unit/layout-tests.cxx:270-292`): `1 2 a b — a b — 3 4`. With the rule in place Paperless
reproduces that sequence exactly, **including** the outer list resuming at 3 — which
LibreOffice's HTML *export* gets wrong, restarting at 1. The clearest case in the corpus of the
reference filter, rather than Paperless, being the lossy party.

**A merged table cell is positioned by ordinal, not by accumulated span.** A row always holds one
`a:tc` per grid column; the covered ones are written out carrying `hMerge`/`vMerge` rather than
omitted. Advancing the column by `gridSpan` *and* stepping over the marker counts the covered
columns twice and shifts every cell after a merge one column right. Caught by the first merged
table asserted, not by the corpus.

## Importers

### ODP — first
- [x] `draw:page`, `draw:frame`, `draw:custom-shape` (text; `draw:enhanced-geometry` is rendering)
- [ ] Master pages; `style:presentation-page-layout`; `presentation:*` attributes
- [x] Simpler two-level inheritance than PPTX, which made it the right place to build first

### PPTX
- [x] `presentation.xml`; slide, layout and master parts by relationship. The master hangs off
      the **layout**, not the slide (`presentationfragmenthandler.cxx:589-600`) — a deck with
      several masters has no other way to say which slide belongs to which. Conventional names
      are a last-ditch fallback only.
- [x] `sldSz`; slide order from `p:sldIdLst`; hidden slides via `p:sld/@show` (absent means
      shown, so reading it as a presence test hides every slide in every deck)
- [x] Shape tree in document order: `p:sp`, `p:cxnSp`, `p:grpSp` (descended into),
      `p:graphicFrame`, `p:pic`
- [x] Text bodies, `a:br`, `a:fld` (cached value, not recomputed), tables inside `a:tbl`
- [x] Placeholder resolution for extraction: type, index, per-level bullets
- [x] Notes slides, from the `notesSlide` relationship — never by assuming `notesSlide3.xml`
      pairs with `slide3.xml`, which stops being true the moment one slide in the middle has
      notes and its neighbours do not. A notes body resolves against the **notes** master's
      `p:notesStyle`, not the slide master's `p:bodyStyle` (`pptshape.cxx:126-136`); get that
      wrong and every line of every speaker note comes out bulleted.
- [x] SmartArt **text**, from `dgm:pt/dgm:t` in the diagram's data part. The open question
      "fallback or implement" is answered "extract the text, decline the layout": the words are
      typed by the author and sit in the data model as ordinary DrawingML text bodies, and only
      the *shapes* need the declarative layout-atom program in `layout1.xml` that
      `oox/source/drawingml/diagram/` executes. Measured: 64 of the 66 decks in
      `sd/qa/unit/data/pptx/` carrying a diagram data part now yield text, and the number of
      decks in that corpus extracting to nothing at all fell from 179 to 121.
- [ ] `normAutofit` `fontScale` and `lnSpcReduction` — rendering only; they change where the text
      sits, not what it says
- [ ] **Charts.** Recorded as a graphic, not read. A chart's user text — title, axis titles,
      series names, category labels — is real content, but it lives in a separate vocabulary in
      its own part, split between `c:tx/c:rich` (a DrawingML text body, so easy) and
      `c:strRef/c:strCache` (a cached copy of a spreadsheet range, which is the part needing
      judgement: the cache can be stale, and the live values are in an embedded workbook that
      would have to be opened through `Paperless.Spreadsheets` — a dependency this library does
      not have and should not acquire for extraction). Decide when the chart renderer forces it.
- [ ] Comments (`p:cmLst` in a `comments` part, authors in `commentAuthors.xml`). Cheap, and the
      content tree already has `SectionKind.Comment` for them; not done because no corpus deck
      has one and an unmeasured reader is a guess.
- [ ] Embedded fonts (`p:embeddedFontLst`) — rendering only.
- [ ] `p:custShowLst`; header/footer visibility (`p:hf`) — neither adds text.

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

## Measuring PPTX extraction

The reference is `impress_html_Export`, and it is lossy in ways that make a raw diff alarming.
Measured over the 45 richest of the 389 decks in `sd/qa/unit/data/pptx/` (normalising the marker
spelling, non-breaking spaces and cell separators described below), **37 contain every line the
reference produces**; the eight that do not reduce to four named causes, none of them a Paperless
defect:

- **Five decks: `<number>`.** The reference emits its own unresolved page-number placeholder where
  the deck has an `a:fld` slide-number field. Paperless emits the cached `a:t` — `1`, `2`, `3` —
  which is what a reader actually saw. The one place the reference is further from the file than
  Paperless is.
- **One deck: a Private Use Area bullet.** `tdf169524.pptx`'s markers come out of the reference as
  U+F06C and U+F02D — Wingdings code points, meaningless outside that font.
  `OutlineNumbers.NormaliseBullet` substitutes U+2022, as the ODP path already does.
- **One deck: marker spelling.** LibreOffice's HTML export writes the marker glued to the text and
  doubles the suffix (`1..Outer, one`); Paperless writes `1. `.
- **One deck: a literal tab inside a run.** `tdf120028.pptx` has a `\t` inside an `a:t`; the export
  renders it as a run of non-breaking spaces. Paperless keeps the tab.

Going the other way, Paperless finds text the reference drops on **30 of the 45** — grouped shapes,
plain text boxes, WordArt, SmartArt — which is the same deliberate improvement the ODP path
makes, not a difference to reconcile. `group.pptx` is the extreme: the reference emits nothing at
all, Paperless 35 lines, every one of them from the slide's own `p:spTree`.

Two reference artefacts worth knowing before chasing them:

- **The HTML export escapes runs of spaces as `&nbsp;`.** Four decks differ only in this.
  Verified against the source — `bnc862510_7.pptx`'s notes part holds two ordinary spaces.
  `compare-text.py --fold-spaces` exists for exactly this, and should stay off by default:
  NBSP versus space *is* a real difference in other formats.
- **Table cells arrive one per `<td>`.** `ContentTableRow` tab-joins a row, matching Writer's
  text filter and CSV export; a line-per-cell comparison reports every table as a mismatch.

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
- [ ] Should a master's *non-placeholder* shapes — a logo, a running strapline — be extracted?
      They are genuinely visible on every slide, and neither the ODP path nor this one reads
      them, on the grounds that repeating a master's text once per slide is worse than losing it
      once. `showMasterSp` on the slide and on the layout decides visibility, so the machinery to
      do it properly exists; what is missing is a decision about what a caller indexing a mixed
      corpus actually wants.
