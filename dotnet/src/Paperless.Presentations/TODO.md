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

Extraction resolves the half it can observe: bullets per level, and emphasis, baseline and language
per level through the `a:defRPr` chain. The half it cannot — a run's colour, size and typefaces —
is resolved too, but into `DrawingCharacterStyle` rather than into the content tree, which carries
none of them; it is measured against the colours LibreOffice's own PDF draws with. Not yet: a
shape's fill and line, charts, and everything below.

**Done: PPT extraction** (`ppt`/`pot`/`pps`), via `MsBinary/` here and the Escher reader in
`Paperless.MsBinary`. Produces the same content tree as the ODF path for the same deck —
`ppt-features.ppt` is `slides-features.odp` converted, and a test asserts the two extract to
identical sections, so a divergence names which reader is wrong. Metadata comes from the OLE
property sets, not from anything PowerPoint-specific.

**Done: the master style sheet** (`MsBinary/PptStyleSheet.cs`). A slide states only what differs
from its master, so the eight-level `TxMasterStyleAtom` set is not a rendering nicety — without it
a PowerPoint title comes out unemphasised. `ppt-features.ppt` now agrees with `slides-features.odp`
on emphasis as well as on text, order and the hidden flag.

Not yet: the *shape* half of the placeholder relationship — `SlideAtom`'s eight layout placeholder
ids, which say which master shape a slide placeholder stands in for. Extraction does not need it
because a PPT text run names its master style directly, in its `TextHeaderAtom`; rendering will,
for position and fill.

**Done: slide layout, for PPTX and ODP.** A deck is an `IPaginatedDocument` and each slide is an
`IPage` of the deck's own size; shapes are placed, filled, outlined and their text laid out
through `Paperless.Text`. Verified against LibreOffice's own PDF for the same deck — placements to
a twentieth of a point, text pens and baselines to a tenth, fill colours exactly. The whole of
what is done and what is not is in **Rendering** below. PPT is not laid out at all.

## Document model

- [ ] Slides, layouts, masters, notes pages, handouts. **Slides are done**; a notes page and a
      handout are separate page kinds and neither is produced.
- [x] Shape tree: rectangles, paths, groups, placeholders and pictures, in document order, which
      is z-order in both vocabularies. Tables and connectors have their own entries below.
- [x] Shape properties: transform (with flip **before** rotation), solid fill, line, text body.
      Not effects: no shadow, glow, soft edge or reflection is drawn.
- [x] Text bodies via the shared text layout, with insets and anchoring. `normAutofit`'s
      `fontScale` and `lnSpcReduction` are applied **as stated in the file** rather than resolved
      again; `spAutoFit` is not.
- [x] Slide size (`p:sldSz`, in EMUs already — the one office measurement needing no conversion)
- [x] Background fill with inheritance: the slide's `p:bg`, then its layout's, then its master's,
      solid only. ODF resolves the same thing through the drawing-page style's parent chain.
- [ ] Presentation styles and outline-level styles — see the inheritance chain below, which
      extraction resolves and rendering does not yet consult for size, face and colour.
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
- [x] Per-level *character* inheritance, as far as extraction can observe it: `defRPr`'s `b`,
      `i`, `u`, `strike`, `baseline` and `lang`, resolved attribute by attribute over the same
      chain the bullet uses. `DrawingTextBody.EmphasisOf` walks it.
- [x] Per-level character inheritance of `sz`, the fill colour and the `a:latin`/`a:ea`/`a:cs`
      typefaces, in `Paperless.Ooxml/DrawingML/DrawingCharacterStyle.cs`. The recorded reason for
      leaving it — that nothing reports these, so no measurement could tell a correct
      implementation from a plausible one — turned out to be true of the *content tree* and false
      of the reference: LibreOffice's PDF states a character colour as the fill colour in the
      graphics state, and `PdfTextRuns` now reads it. So the colour is measured end to end and
      `ContentRun` is still left alone.
      The resolved style is deliberately not merged into the content tree. It is what a renderer
      wants and what an index does not, and adding a colour to `ContentRun` that only one of the
      three families could fill in would be worse than not having one.
- [x] **The shape's own text style**, which sits between the master's list style and the body's
      (`oox/source/drawingml/textparagraph.cxx`:52-67). `DrawingCharacterStyle.FromShapeStyle`
      reads it and `DrawingCharacterStyle.Resolve` puts it in place.
      **Where it goes is the entire content of the item.** Both extremes give the right answer on a
      shape that states the colour once, which is nearly every shape, so the ordering is only
      visible on a shape that states it twice — hence `deck-text-style.pptx`, whose seven text
      boxes each state it at a different rung. Measured against LibreOffice's own render, strongest
      first: the run's `a:rPr`, the paragraph's `a:defRPr`, the body's `a:lstStyle`, **the shape
      style**, then everything inherited. The five that LibreOffice and Paperless can both answer
      agree exactly — #4F81BD, #00B050, #9BBB59, #FF7F00 and #953735, the last being accent2 under a
      `lumMod` of 75%, resolved by the shared chain rather than by a second implementation of it.
      It carries a typeface as well as a colour: `a:fontRef idx="minor"` is the theme's minor Latin
      face, which needed the theme's `a:fontScheme` (`DrawingFontScheme`) as well as its colour
      scheme. That also brings the `+mn-lt` indirection, where a stated typeface is a *reference*
      rather than a name — a reader taking it literally reports a font family called `+mn-lt` and
      every run of a PowerPoint-authored deck silently falls back to a substitute
      (`oox/source/drawingml/theme.cxx`:71).
      **The named trap, and it looked obviously right: a placeholder does not inherit its shape
      style.** A slide placeholder takes its geometry, fill, line, effects and master list style
      from the layout placeholder it stands in for, so taking the `a:fontRef` too seemed free —
      `applyShapeReference` was already the citation for the rest of it. It does not:
      `oox/source/drawingml/shape.cxx`:565-587 copies the *resolved* line, fill and effect
      properties and never touches `maShapeStyleRefs`. Measured on the deck's seventh shape, a
      placeholder with no style of its own whose layout placeholder states an `a:fontRef` of
      accent 5 — LibreOffice draws it in plain black in its default face. Inheriting it would have
      recoloured a placeholder on every deck whose layouts style their placeholders, which is most
      PowerPoint-authored decks. Half an hour, and the render is what settled it.
      **A second finding, free: LibreOffice never applies a master's `p:otherStyle` at all.**
      `SlidePersist::createXShapes` pushes the master text styles into Impress's style families
      with `for (int i = 0; i < 4; i++)` over a switch whose `case 4` *is* the standard style that
      `p:otherStyle` parses into (`oox/source/ppt/slidepersist.cxx`:315). The loop stops one short,
      so the style is read, stored and never used; `case 5`, the subtitle style, is unreachable for
      the same reason. Paperless does apply it — the deck's sixth shape resolves to the magenta the
      master states where LibreOffice draws black — and that divergence is not new: the same
      fallback has always decided the bullet of every non-placeholder shape. It had simply never
      been measured, and it now is.
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

**A master placeholder's *character* defaults are somewhere else again, and LibreOffice does not
read them either.** The same exporter writes the master title's boldness on the demonstration
paragraph's `a:rPr` and `a:endParaRPr`, not on an `a:pPr/a:defRPr` — and `ApplyMasterTextStyle`
takes only `TextParagraphProperties::getTextCharacterProperties`, which is the `defRPr`
(`oox/source/drawingml/textbody.cxx:183`). So a LibreOffice-authored deck's master character
defaults reach nothing on either side. That is not a loss, because the same exporter also states
every property on every slide run, but it does mean the corpus cannot test character inheritance:
`deck-features.pptx`'s master says `b="1"` on its title placeholder's run and the inheritance is
never consulted. The measurement had to come from PowerPoint-authored files instead.

**Character properties inherit attribute by attribute, not element by element.** A run stating
`b="1"` and nothing else has not cancelled the italic its level's `defRPr` gives it —
LibreOffice's `assignUsed` applies the master's list style, then the shape's text style, then the
body's list style, then the paragraph's `defRPr`, then the run, each overwriting only what it sets
(`oox/source/drawingml/textparagraph.cxx:51-67`, `textrun.cxx:80`). Merging whole property sets
gives the right answer on every run that states everything, which is every run LibreOffice writes,
and the wrong answer on the PowerPoint-authored files where it matters.

Measured over all 389 decks in `sd/qa/unit/data/pptx/`: resolving the chain changed **eight** of
them, always by gaining emphasis, never by losing text. Every change was checked against
LibreOffice's own PDF text layer and every one agrees — `bnc904423.pptx`'s three lines, whose
`p:txStyles` states the boldness once for the whole deck; `ShapeLineProperties.pptx`'s two bold
lines, where the third line is *not* bold in the reference and Paperless correctly leaves it
alone; `tdf95932.pptx`, `tdf120028.pptx`, `tdf132282.pptx`, `tdf114848.pptx`,
`slide-sections.pptx` and `bnc870233_1.pptx`, whose test text is literally labelled "Red, bold"
and "Blue, italic".

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
- [ ] Master pages; `style:presentation-page-layout`; `presentation:*` attributes. The master's
      `style:page-layout` **is** read, for the slide size, and it must be the one the page's own
      master names rather than the first in the file: a deck carries at least two and the notes
      one is A4 portrait in everything LibreOffice writes. The master's own *shapes* are not
      drawn, which is the same open question the PPTX side has about `showMasterSp`.
- [ ] **The list style an outline paragraph's indents, spacing and bullet come from.** A
      `text:list` names a `text:style-name`, and the level is the nesting depth; `OdfListStyle`
      already parses the levels. Nothing reads them, so `slides-features.odp`'s outline lands
      17 pt left of the reference with no bullet, and drifts to 9.3 pt low by its third
      paragraph. The three quantities needed are `fo:margin-left`, `fo:text-indent` and
      `text:bullet-char` — exactly what the OOXML side gets from `marL`, `indent` and `a:buChar`,
      through a different resolution.
- [x] Simpler two-level inheritance than PPTX, which made it the right place to build first

### PPTX
- [x] `presentation.xml`; slide, layout and master parts by relationship. The master hangs off
      the **layout**, not the slide (`presentationfragmenthandler.cxx:589-600`) — a deck with
      several masters has no other way to say which slide belongs to which. Conventional names
      are a last-ditch fallback only.
- [x] `sldSz`; slide order from `p:sldIdLst`; hidden slides via `p:sld/@show` (absent means
      shown, so reading it as a presence test hides every slide in every deck)
- [x] The theme part, reached from the **master** and not from the presentation, with the master's
      `p:clrMap` applied to it before anyone sees it. PowerPoint writes a theme relationship on the
      presentation as well, and taking that one gives every slide in the deck the *first* master's
      colours — right for the single-master deck and wrong for exactly the decks that bothered to
      have two. The map is joined on here for the same reason `DocxFile` joins `w:clrSchemeMapping`:
      a scheme without its map answers the wrong question, and a dark master is where that shows.
      Not read: `p:clrMapOvr/a:overrideClrMapping` on a layout or slide, which would make the map
      per-slide rather than per-master; nothing measured carries one.
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
- [x] Comments — `p:cmLst` in a part reached by relationship **from the slide**, and the author
      names in a second part reached from the *presentation*. `Ooxml/PptxComments.cs`. It was
      cheap once there was a deck to measure against, and building that deck
      (`comment-deck.pptx`) is what refuted the guess: a comment does not carry its author's
      name, it carries an id. It also carries no runs — a two-paragraph comment is one `p:text`
      with a newline in it. An id resolving to nobody leaves the name null; LibreOffice invents
      "Anonymous" (`oox/source/ppt/comments.cxx:70`), which would claim the file said something
      it did not. Measured against the three decks in `sd/qa/unit/data/pptx/` that carry a
      comment list — `pres-with-notes.pptx`, `tdf89064.pptx`, `tdf91060.pptx`: authors resolve on
      all three, including `tdf89064.pptx`, whose author really is called "Anonymous".
- [ ] PowerPoint 365's "modern" comments, in `ppt/comments/modernComment_*.xml` under a `p188:`
      vocabulary with the authors in `ppt/authors.xml`. LibreOffice 24.2 does not read them
      either, and a deck carrying them also carries the legacy `p:cmLst` for compatibility, so
      nothing observable is lost today. Worth revisiting when a deck appears that has only the
      modern form.
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
- [x] Master style sheets (`MsBinary/PptStyleSheet.cs`). A slide's `SlideAtom` names a master by
      **slide** id — masters number themselves from `0x80000000`, so the field is matched against
      the master list's persist atoms and not resolved through the persist directory
      (`svdfppt.cxx:2520`) — and that master's `TxMasterStyleAtom` records hold the per-level
      character and paragraph defaults. `ppt-features.ppt`'s title states a mask of `0x040000`,
      its colour and nothing else; its boldness is instance 0's level-0 flags word. The equality
      test against the ODF deck now compares emphasis as well as text, order and the hidden flag.
      What is *not* done here: the eight layout placeholder ids in the same atom, which rendering
      needs for position and fill.
- [ ] The rest of what the style sheet already parses. `PptCharacterLevel` carries the font index,
      size, colour and escapement because the record cannot be walked without decoding them; only
      the flags word is used. Colour is a raw `0xTTBBGGRR` where a non-zero top byte means a
      **colour-scheme index** rather than a literal, so it needs the page's `ColorSchemeAtom` —
      which is per slide, not per master, and is the reason it was not resolved here.
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

### What the master style sheet cost, and what it bought

**The first level of a `TxMasterStyleAtom` uses a different field order from every later level.**
`PPTParaSheet::Read` has two branches on a `bFirst` flag (`svdfppt.cxx:3925-4010`): on the first
level, alignment is mask bit `0x0F00` and the tab-stop array is `0x200000`; on every later one
they are `0x0800` and `0x100000`, with `0x200000` becoming a two-byte text direction. So the two
layouts differ in **size**, not merely in meaning, and reading every level the later way consumes
ten bytes too few on a level-0 record that states tab stops — after which the character mask is
read out of the middle of the tab-stop array and the boldness that comes back is whatever the
bytes happened to say. That cost the most time here. The check that catches it in one line: the
levels must consume the record's declared length exactly, and a Python walk of
`ppt-features.ppt`'s eight atoms consuming 270, 270, 270, 270, 222, 222, 222, 222 of 270×4 and
222×4 was what confirmed the layout before a line of C# was written.

**Two more, cheaper but equally silent.** The four instances above `TextInShape` prefix *every*
level with an unexplained word and never take the first-level layout (`svdfppt.cxx:4253-4260`).
And a level inherits from the level above it only for the five low instances
(`svdfppt.cxx:4247`); the four high ones are initialised as a copy of the body or title sheet at
the moment their atom is met, which is why the atoms have to be read in the order the file writes
them rather than gathered into a map first.

**Two bugs in the existing reader fell out of measuring the result** over the 33 binary decks in
`sd/qa/unit/data/`:

- **A paragraph property run is not one paragraph.** Its count is a *character* count, and a
  writer may cover several paragraphs with one run; LibreOffice clones the property set at every
  carriage return inside the count (`svdfppt.cxx:5081-5090`). Pairing the *n*th run with the
  *n*th paragraph lost the depth and the bullet of every paragraph after such a run.
  `hanging-indent.ppt` and `tdf166030.ppt` each write two runs for three paragraphs, and their
  third paragraph came out unindented and unbulleted where LibreOffice renders it at level two
  with a bullet. Both now match.
- **A bullet the paragraph does not state falls through to the master.** The mask's low four bits
  are the *bullet flags*; a paragraph whose mask does not include bit 0 has said nothing about
  whether it draws one. `fdo68594.ppt` gained the four bullets it was missing (the reference PDF
  shows five, Paperless had one) and `indent_multiple_spacings.ppt` its three.

Net effect over those 33 decks: 172 changed lines, all of them gains, none a loss of text. The
emphasis changes were checked against LibreOffice's own PDF — `tdf168786.ppt`'s rows of
underscores are `LiberationSans-Bold` in the reference and were being reported unemphasised.

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

- [x] Slide as a page: background, then shapes in z-order. `Layout/SlidePages.cs` and
      `Layout/SlideDrawing.cs`. A deck that states no background anywhere gets white, because
      that is what LibreOffice paints — a full-sheet rectangle on every slide of every deck — and
      a slide that painted nothing would rasterise transparent where a viewer shows paper.
- [x] Shape geometry: **all 187 presets**, evaluated rather than transcribed, and `a:custGeom`
      through the same path builder. A name that is not a preset still falls back to its bounding
      rectangle. See below for what the evaluator had to get right that six hand-written shapes
      never showed.
- [x] Solid fills, including themed ones, and lines with width, cap and join. Not shadows.
- [x] **Gradient fills**, for both formats: `a:gradFill` and `draw:gradient`, linear, axial,
      radial, elliptical and rectangular. `Layout/SlideGradients.cs` holds the geometry, which is
      LibreOffice's rather than either format's — both importers converge on `basegfx::BGradient`
      and everything that decides where the ends land happens after that. See **Two gradient
      conventions that are invisible except in colour** below.
- [x] **Bitmap fills**, tiled or stretched: `a:blipFill` and `draw:fill="bitmap"`. A tile's size
      is the picture's *natural* size scaled by `a:tile/@sx`, so the reader has to know how large
      a picture is without decoding it — twenty bytes of header, in `Layout/SlideImages.cs`.
- [x] **Pictures**, with crop: `p:pic` and `draw:frame`/`draw:image`. `a:srcRect` becomes a
      larger destination rectangle clipped to the shape rather than a crop, because the drawing
      model has clipping and no crop and the two are the same thing. Nothing decodes: a reader
      emits `RasterImage.Encoded` and a backend decodes when it wants pixels, which is what keeps
      this library free of a `ProjectReference` on `Paperless.Rendering` and therefore keeps
      `paperless extract` free of a codec it never uses.
- [x] Text bodies with anchoring, insets and the stated autofit scale.
- [x] Groups with nested transforms, including a child coordinate space that scales.
- [ ] Picture *effects*: `a:effectLst` (shadow, glow, reflection, soft edge), `a:duotone`,
      `a:grayscl`, `a:biLevel` and the brightness/contrast pair. All are per-pixel work on a
      decoded bitmap, which is the one thing this library must not do — they belong beside the
      decoder in `Paperless.Rendering`, as a transform the display list names rather than
      performs. `a:alphaModFix` is the exception and is honoured, because a uniform opacity is
      already a parameter of `DrawImage`.
- [ ] **A rotated picture.** `IDrawingSink.DrawImage` takes a rectangle and not a matrix, so a
      `p:pic` with a non-zero `rot` is drawn upright inside its rotated clip. The clip is right
      and the pixels are not turned. Fixing it means either a matrix on `DrawImage` or a
      `Save`/`Transform` pair around it in `SlideDrawing`; the second is a two-line change and is
      not made yet only because no corpus deck rotates a picture, so it would be unmeasured.
- [x] Tables, for PPTX. A `p:graphicFrame` holding an `a:tbl` becomes a run of ordinary placed
      shapes — one per visible cell with its fill and its text, then one per consolidated grid
      line with only a pen — so nothing in the display list knows a table happened and the binary
      PPT path can reuse every line of it. Measured stroke for stroke against the reference; see
      **Tables on a slide** below. Not for ODP, whose `table:table` inside a `draw:frame` is a
      different vocabulary and is not read at all yet.
- [ ] Notes pages as separate output pages (optional)
- [ ] PPT (binary). Nothing lays out. The Escher reader already produces the shape tree and the
      anchors, but `SlideAtom`'s eight layout placeholder ids — the shape half of the placeholder
      relationship, recorded as missing in the PPT section above — are what a placeholder's
      position and fill come from, and without them a PPT title has no rectangle to be in.

### What renders, and what it was measured against

Three corpus decks, each hand-written so that every offset is a round number of inches and a
disagreement is a bug rather than a rounding, and each with two tests: one in
`Paperless.Presentations.Tests` asserting against numbers transcribed from LibreOffice's PDF once
and quoted in place, needing no LibreOffice at all, and one in `Paperless.Fidelity.Tests` that
re-derives them.

| Deck | What it pins | Comparison |
|---|---|---|
| `shape-geometry.pptx`, `shape-geometry.odp` | placement, groups, flip before rotation, text insets, anchoring | `SlideRenderComparisonTests` |
| `slide-table-grid.pptx` | the grid, cell fills, cell text, border priority, a grown row | `SlideTableComparisonTests` |
| `slide-shape-features.pptx` | twelve presets, three dash patterns, two arrowheads, a two-level numbered list | `SlideShapeGeometryComparisonTests` |

`Paperless.TestKit`'s readers grew one for this: `PdfPaths` reads a filled path of *any* shape,
where `PdfFills` reads only axis-parallel rectangles. It compares **on-curve points only** — the
destinations of `m`, `l` and `c` — because neither writer chooses the same number of cubics per
arc and the control points are therefore incomparable, while the points a curve passes through
are. So a straight-edged preset compares vertex for vertex and a curved one by those points and
its bounding box. `PdfStrokes` grew a dash array for the same reason: a preset dash states no
lengths anywhere else.

**What the richer decks measure at.** `deck-features.pptx` now draws every run the reference
does — 21 against 21, on three pages against three — so its table slide and its page count are in
the comparison. What keeps the *whole* of it out is one thing rather than four: its bullet markers
sit 8.19 pt below where LibreOffice draws them, which is the label-placement item below.

| | Reference runs | Ours | Agreement, and what is missing |
|---|---|---|---|
| `deck-features.pptx` | 21 | 21 | Titles, bullets and outline text land exactly — pens to a hundredth of a point, baselines to 0.04 — and the group and text-box slide to 0.04 across, 0.55 down inside the ellipse. The table slide's six runs now agree to 0.015. What is still out is the **marker baseline**, 8.19 pt high on every bulleted line. |
| `slides-features.odp` | 14 | 12 | The same slides through ODF. The title and the outline's first line agree to 0.04; the outline then **drifts to 9.3 pt by its third paragraph**, and its text sits at 56.69 where the reference puts it at 73.70. Both have the same cause: an outline paragraph's indents, spacing and bullet come from the `text:list`'s **list style**, and nothing reads it. |

The ODF list style is therefore the single largest remaining item on that path, and it is a
different resolution from the paragraph cascade rather than more of it: `OdfListStyle` already
parses the levels, and what is missing is choosing the level from the `text:list` nesting depth
and reading its `fo:margin-left`/`fo:text-indent` and `text:bullet-char` — the same three
quantities the OOXML side gets from `marL`, `indent` and `a:buChar`.

**LibreOffice's PDF export is one hundredth of a millimetre small, and one up and left.** Its page
clip opens `0 0.028 719.971 539.971 re` on a 720 × 540 pt page, its page background is 719.971 pt
wide, and a rectangle the file puts at 72 pt comes out at 71.972 with a width of 143.971 instead
of 144. It applies to the sheet as much as to the shapes, so it is the export rounding into the
drawing layer's own unit rather than a placement difference — 0.0283 pt, comfortably inside the
tenth of a point everything here is held to. Worth knowing before chasing it: it is present in
every single number the reference reports.

### The transform, and the measurement that pins its order

`Shape::createAndInsert` (`oox/source/drawingml/shape.cxx:1098-1224`) builds one cumulative matrix
per shape: scale the unit square to the extent, **mirror at the shape's centre**
(`lcl_mirrorAtCenter`, `shape.cxx:882`), translate to the offset, map through the parent group's
child coordinate space (`shape.cxx:1174-1198`), and only then **rotate at the shape's centre**
(`lcl_RotateAtCenter`, `shape.cxx:910`). `Layout/ShapeTransform.cs` composes exactly that.

**Flip before rotation is observable, and only on a shape that is not symmetric.** A rectangle
looks identical under either order, which is why the rule is easy to get wrong and hard to notice
— and why the corpus deck carries an `rtTriangle` with `flipH="1"` and `rot="5400000"` at
(288 pt, 216 pt) sized 144 × 72. LibreOffice's PDF draws its vertices at (324, 324), (396, 324)
and (324, 180). Flipping first gives exactly those; rotating first gives (396, 180), (324, 180)
and (396, 324) — a different triangle in a different corner.

**A group's child space is the thing that makes a group a group.** `a:chOff`/`a:chExt` say what
coordinates the children are written in, and when they differ from `a:off`/`a:ext` the children
scale. The deck's group is at 72,72 sized 288 × 144 with a child space of 144 × 72, so its
children double: a child stated at 0,0 sized 72 × 36 comes out at 71.972–215.943 × 71.972–143.943
in the reference, and one stated at 72,36 comes out at 215.972–359.943 × 143.972–215.943. A shape
beside the group is unaffected, which is what says the scale reached the children and not the
slide.

The formulation here composes the group's own placement on top of the child mapping, rather than
decomposing the group's matrix into scale, rotation and translation as LibreOffice does. The two
are the same matrix — the group's placement *is* that decomposition, reassembled — and this way
round needs no decomposition and no shear term.

### The line height on a slide is not the font's

**The single largest thing that would be wrong without reading the C++.** The PPTX importer sets
`FontIndependentLineSpacing` on every text body it reads
(`oox/source/ppt/pptshapecontext.cxx:186`), and EditEngine then takes the line's ascent to be the
font height outright and its descent to be
`ImplCalculateFontIndependentLineSpacing(height) − ascent`
(`editeng/source/editeng/impedit3.cxx:3138-3141`), where that function is
`fround(height × 12 / 10)` (`impedit3.cxx:501-505`). So the first baseline is **one em** below the
top of the text area and the next line is **1.2 em** further down, whatever face the text is set
in.

Measured on slide 3 of the corpus deck, four text boxes of 18 pt Liberation Sans:

| What | Reference | Rule | Font's own metrics |
|---|---|---|---|
| First baseline, no insets | 89.972 pt on a shape at 71.972 | 18.000 below the top | 16.295 below |
| With `lIns=91440 tIns=45720` | 79.172, 237.572 | 72 + 7.2, 216 + 3.6 + 18 | 1.7 pt high |
| Centred paragraph's pen | 435.969 | 468 − w/2, w from our own shaping | — |
| `anchor="ctr"` baseline | 259.172 | 216 + (72 − 21.6)/2 + 18 = 259.2 | 1.4 pt high |

Liberation Sans reports an ascent of 0.905 em, so a reader using the font's metrics is 1.705 pt
high on **every line of every shape** — a cascade, and one that a word-processing comparison would
never have found because Writer does not set the flag. `SlideTextLayout` therefore takes the line
*breaks* and the horizontal placement from the shared `ParagraphLayouter` and recomputes only the
vertical, so the presentation-specific rule stays out of the engine the three families share.

ODF states the same thing per paragraph style as `style:font-independent-line-spacing`, and
`SlideTextBody.FontIndependentLineSpacing` carries it. A natively authored ODP usually does not set
it, in which case the face's own metrics decide — which is why the same deck can legitimately lay
out slightly differently through the two paths.

**And when the face's metrics do decide, they are its ascent and descent and *not* its line gap.**
EditEngine adds the external leading only under `IsAddExtLeading()`, which is a Writer
compatibility flag and off in Impress (`impedit3.cxx:3131-3136`). Liberation Sans declares a line
gap of 67/2048, so keeping it makes an 18 pt line 20.70 pt where LibreOffice draws 20.15 — half a
point per line, which the shared `ParagraphLayouter` legitimately includes because a word
processor wants it. `SlideTextLayout` therefore computes the height itself in this branch too,
from `LineSpacing.Resolve`, and rounds the ascent and the descent to whole hundredths of a
millimetre before adding them, because `FontMetricData::ImplCalcLineSpacing` ends
`mnAscent = round(fAscent)` in the device's own unit (`vcl/source/font/fontmetric.cxx:538-540`)
and Impress's reference device is in 1/100 mm.

- [ ] **A residual 0.028 pt per line.** With both rules applied an 18 pt Liberation Sans line is
      20.126 pt here and 20.154 in the reference — one unit of 1/100 mm, every line. It is under
      the tolerance for one line and reaches it at four, so a cell wrapping onto four lines has
      its last baseline 0.08 pt out and the table's bottom rule 0.11. Where the extra unit comes
      from is not settled: 575 + 135 is what `round(1854/2048 × 635)` and `round(434/2048 × 635)`
      give, and the reference behaves as though it had 576 + 135. Worth one more look at how VCL
      scales an `hb_position_t` before it rounds.

### Text insets have a default, and it is not zero

`a:bodyPr`'s `lIns` and `rIns` default to 91440 EMU and `tIns`/`bIns` to 45720 — a tenth and a
twentieth of an inch. Defaulting them to zero moves every line of every text box that states none
7.2 pt left and 3.6 pt up. ODF has no such implied default and states all four, which is how a
converted deck agrees with its original: LibreOffice writes the OOXML defaults out explicitly as
`fo:padding-*`.

### Two ODF traps, both silent

**`draw:transform`'s rotation runs the opposite way from `a:xfrm/@rot`.** The corpus deck's
30°-clockwise rectangle comes out of LibreOffice's ODF export as
`rotate (-0.523598775598299) translate (3.515cm 10.33cm)` — the angle negated, because ODF's is
counter-clockwise in a y-up reading. In the y-down space everything here works in the matrix is
`[cos, sin; −sin, cos]`, and only that reading puts the shape's centre back at the 5.0795 cm,
12.6995 cm the OOXML original states.

**A bare number in a `draw:transform` is not a length.** `OdfValue.ParseLength` takes a unitless
value for hundredths of a millimetre, which is right for ODF's lengths and catastrophic for an
angle: putting `rotate (-0.5236)` through it rounds to −1 and then means −360 radians, which wraps
to about −106° and lands the shape in a *plausible-looking* wrong place rather than an obviously
wrong one. Arguments are therefore parsed per operation — lengths for `translate`, plain numbers
for `rotate`, `scale` and `skewX`.

**An outline placeholder's paragraphs are not its children.** A `draw:frame` wraps its text in a
`draw:text-box`, a `draw:custom-shape` holds its `text:p` children directly, and an *outline*
placeholder wraps every paragraph in a `text:list`/`text:list-item` pair — one level of nesting per
outline level. Taking only the direct children lost all three lines of `slides-features.odp`'s
outline while its title read perfectly, which is the shape of failure that looks like a placement
bug and is not.

**And a shape's paragraph formatting is not on its paragraphs.** LibreOffice's ODF export writes
it on the shape as `draw:text-style-name` and leaves each `text:p` pointing at an almost empty
automatic style. A reader consulting only `text:style-name` finds no alignment, no line-height rule
and no `style:font-independent-line-spacing` — which on `shape-geometry.odp` put every baseline
1.7 pt high, exactly the font-metrics error above, arrived at by a completely different route.

### Preset geometry: all hundred and eighty-seven, evaluated

`Paperless.Ooxml/DrawingML/CustomShapeGeometry.cs` runs the program each preset is: the
adjustment handles take their defaults from `a:avLst`, the `a:gdLst` formulas are evaluated in
order over the bounding box and those handles, and `a:pathLst` is walked into a `GraphicsPath`.
`a:custGeom` — a shape whose guides and paths the file states itself — goes through the same
evaluator, which is why it cost nothing extra. `Layout/SlidePresetGeometry.cs` is now an adapter.

**The definitions are data, and are kept as data.** `PresetShapeGeometry.txt` is LibreOffice's own
`presetShapeDefinitions.xml` reduced to the adjustments, the guides, the text rectangle and the
paths — one line per element, 110 kB against the source's 539 kB, embedded as a resource. Ported
into C# literals it would be unreviewable and undiffable against the file it came from; as a text
table a change to either is a diff against the other.

**The named trap, and it cost an hour: an `a:arcTo` angle is a direction, not an ellipse
parameter.** The command states two radii and two angles and no end point, so the arc starts at
the current point and the centre is wherever the start angle puts it. What the angle *names* is
the ray from that centre; the point drawn is where the ray crosses the ellipse, and for
`x = a·cos t, y = b·sin t` that is `tan t = (a/b)·tan θ` — `lcl_getNormalizedCircleAngleRad`
(`svx/source/customshapes/EnhancedCustomShape2d.cxx:2040-2057`). It is invisible on every circular
arc, which is every rounded corner in the preset file, and invisible again at multiples of 90°,
which LibreOffice special-cases outright. So all six of the hand-transcribed shapes agreed without
it and the seventh did not: `pie` with a 240° sweep on a 3:2 box ends its arc at 249° in parameter
terms, and the reference draws that end 7.6 pt from where the stated angle alone puts it.

**A second one, cheaper: split an arc at the quadrant boundaries, not into equal parts.** Three
eighty-degree cubics cover a 240° sweep perfectly well as a curve and never pass through 180°, so
the shape's own leftmost point is never on the path and its bounding box comes out narrower than
the shape. `createPolygonFromEllipseSegment` splits at the quadrants, and so does this.

Measured on `slide-shape-features.pptx`, against LibreOffice's own PDF:

| | Reference | Ours |
|---|---|---|
| `hexagon`, `chevron`, `parallelogram`, `plus` | six to twelve vertices each | identical to 0.03 pt |
| `pentagon`, `trapezoid`, `star5` | guides needing `sin`, `cos` and `at2` | within 0.06 pt |
| `roundRect`, `ellipse`, `donut`, `moon` | arcs | on-curve points and box within 0.06 pt |
| `pie`, 240° on a 3:2 box | arc ends at 394.583, 465.619 | 394.603, 465.597 |

- [ ] **A subpath's own fill modifier.** `a:path/@fill` takes `lighten`, `lightenLess`, `darken`,
      `darkenLess` and `none`, and the reduced table carries all of them; nothing acts on them.
      What that costs is exactly the shapes that are drawn as a solid with a shaded face — `can`,
      `cube`, `bevel`. Measured on a `can` filled `4F81BD`: LibreOffice draws the body in that
      colour and the lid as a *second* filled path in `95B3D7`, which is the fill blended 40%
      towards white. Reproducing it means a preset producing more than one `PlacedShape`, which
      is a change to the shape of the output rather than to the evaluator, and `can` was taken
      back out of the corpus deck rather than pinned as though the single-colour answer were
      right.
- [ ] **`a:path/@stroke="false"`** — a subpath that is filled and not outlined. Same table entry,
      same reason, and it matters for the callout presets, whose tail is a separate unstroked
      subpath.
- [ ] **Preset text rectangles beyond the shape's own.** `a:rect` is honoured, which covers the
      presets that state one. The ones that do not get the bounding box, which is what LibreOffice
      falls back to as well.

The **text** rectangle is carried beside the outline because for many presets it is not the
bounding box: an ellipse's is the box inscribed at 45°, a rounded rectangle's is inset by the
corner radius, and a callout's excludes its tail. That is why a caption inside a circle does not
touch its edge.

### Tables on a slide

A table is not a shape with a table inside it. LibreOffice decomposes an `SdrTableObj` into one
filled-and-texted primitive per cell and then, *separately*, the grid's border lines — cells
first and borders after, "to get the correct overlapping"
(`svx/source/table/viewcontactoftableobj.cxx:202-204`). `Layout/SlideTable.cs` produces exactly
that: a run of `PlacedShape`, one per visible cell and one per consolidated grid line. Nothing in
the display list knows a table happened, which is what keeps `SlideDrawing` unchanged and what
lets the binary PPT path reuse all of it.

**One stroke per grid line, and the order is not "all horizontals then all verticals".** The
cells are walked row-major and each contributes its top edge, its bottom if it is on the last row,
its left, and its right if it is on the last column
(`svx/source/dialog/framelinkarray.cxx:1487-1530`); collinear neighbours that agree merge into
whichever appeared first. So a three-row table emits its top rule, then the verticals crossing its
first row, then the next horizontal, and so on. Measured on `slide-table-grid.pptx`: twelve
strokes on the first slide and nine on the second, in that order on both sides, every coordinate
within 0.045 pt.

**A grid line runs past its own end by half the width of the line *crossing* it, not half its
own.** That is the rule the word processor's `PageDrawing` states as "overshoots by half its own
width", which is true only because a Writer table's borders are usually all the same width. Here
they are not: `slide-table-grid.pptx`'s one-point grey horizontals run 71.121 to 612.879 on a
table spanning 72 to 612 — 0.879 at each end, half of the *two-point* outer verticals they meet —
while its three-point orange vertical is extended by 0.425, half of the one-point horizontals
crossing it. The general rule is `getExtends`
(`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:310-395`), which intersects the
border's two offset edges with every style at the junction and takes the nearest crossing; for
single lines meeting at a right angle that reduces to half the perpendicular width, which is every
junction a table has.

**A grid position takes one pen, and which one is `TableLayouter::HasPriority`**
(`svx/source/table/tablelayouter.cxx:944-978`): the wider wins, a tie goes to whichever cell was
written later — under a row-major walk, the one below or to the right — and a cell stating no edge
never displaces one that does. All three are in the corpus deck and all three are drawn: a red
right edge loses to the later cell's grey left edge of the same width, a grey loses to a green
the same way, and a one-point grey loses to a three-point orange.

**The named trap, and it is a 15% error that looks like a rounding: a border's width is not the
width the file states.** It travels EMU → 1/100 mm → a `BorderLine2.LineWidth` that is *halved*
(`oox/source/drawingml/table/tablecell.cxx:99-101`) → an `SvxBorderLine` that the table's view
contact then rescales as though it were in *twips*
(`svx/source/table/viewcontactoftableobj.cxx:176-180`). Both conversions are integer and they do
not cancel: a stated `w="12700"` — one point — comes out of the reference's PDF as a pen of
**0.85009 pt**, a stated two points as 1.75008 and three as 2.65006. `DrawingTableGeometry.BorderWidth`
reproduces the chain rather than correcting it, because the reference is what the comparison is
against.

**A cell's text is inset by `a:tcPr`'s margins and not by its body's `a:bodyPr`.** Measurable
rather than arguable: `deck-features.pptx`'s table states `lIns="0" tIns="0"` on every cell body,
and LibreOffice still draws the first column's text 7.2 pt in from the table's left edge — the
default `marL` of 91440 EMU.

**And a cell's line height is the face's, where a slide shape's is the em.** This is the one place
the current C++ and the installed LibreOffice disagree: `tablecellcontext.cxx:61` sets
`FontIndependentLineSpacing` on every cell text body it creates, and LibreOffice 24.2.7.2 does not
draw as though it had. Measured on `deck-features.pptx`'s first cell — 18 pt Arial, substituted by
Liberation Sans, in a cell whose top edge its own PDF puts at 170.079 pt — the baseline is 19.93 pt
below that edge. Take off the 3.6 pt top margin and the ascent is 16.33 pt, which is 0.907 em: the
font's, where the em rule would give 18.00. So the cell body carries
`FontIndependentLineSpacing = false` and says why.

- [ ] **Table styles.** `a:tblPr/@firstRow` and the `tableStyles.xml` part it bands against are
      not read, so a PowerPoint-authored table styled from the gallery draws unfilled and
      unbordered. It costs nothing on a LibreOffice-written file, whose export states an explicit
      `a:lnL`…`a:lnB` and an explicit fill on every cell — which is also why no corpus deck can
      measure it, and why the corpus deck for tables is hand-written.
- [ ] **Diagonal cell borders**, `a:lnTlToBr`/`a:lnBlToTr`. Read as far as being ignored. The
      border array carries them (`GetCellStyleTLBR`) and nothing in the corpus has one.
- [ ] **ODP tables**, which are `table:table` inside a `draw:frame` and share none of this
      vocabulary. `SlideTable.Place` takes the grid and a delegate for the text body, so an ODF
      reader would need the geometry model and nothing else.
- [ ] **A row taller than its frame.** Rows grow to their text and columns are stretched to the
      frame's width, which is the asymmetry `LayoutTableWidth`/`LayoutTableHeight` has; a table
      whose rows overflow the frame overflows it here too, which is what a viewer shows and what
      the reference draws.

### What a shape's fill and line resolve through, and what they do not

Resolved: `a:solidFill` and `a:noFill` on the shape, then on the layout or master placeholder it
stands in for, with the colour going through the existing `DrawingColour`/`DrawingColourTransforms`
in `Paperless.Ooxml/DrawingML` — which is what makes `schemeClr accent1` come out `18a303` and
`accent2` under `lumMod val="60000"`/`lumOff val="40000"` come out `34b3fb`, both confirmed
against the reference PDF's own `rg` operators. The colour map comes from the slide master's
`a:clrMap`, so a dark master inverts correctly.

- [ ] **`p:style`'s style-matrix references.** `a:fillRef`, `a:lnRef` and `a:effectRef` index the
      theme's `a:fmtScheme` — three fill styles, three line styles, three effect styles — with the
      `phClr` inside each substituted by the colour the reference carries. `DrawingColour` already
      resolves `phClr` when it is given a placeholder colour, so what is missing is only reading
      `a:fmtScheme` and indexing it. This is how the whole "Shape Styles" gallery in PowerPoint
      works, so a deck authored with it renders unfilled today. Not done because no corpus deck
      uses one: `shape-geometry.pptx` states its fills outright and `deck-features.pptx` writes
      `<a:fillRef idx="0"/>`, which means none.
- [x] **Gradients**, both formats, five geometries. See the section below for the two mappings
      that had to be measured and the one unit that cost the time.
- [x] **Picture fills**, tiled and stretched.
- [ ] **`a:pattFill` and `draw:fill="hatch"`.** Both resolve into something the reader would have
      to *synthesise* rather than read — a pattern is an 8×8 monochrome bitmap named by a preset
      token and a hatch is a set of stroked lines at an angle — so neither is a parse, and both
      want a table of the fifty-four `ST_PresetPatternVal` bitmaps beside the code that expands
      them. `Paint` already has the tiled `BitmapPaint` to receive a pattern, which is the
      deliberate design note at the top of `Paint.cs`; nothing in the corpus carries one.
- [ ] **`style:repeat="no-repeat"`.** ODF's third bitmap mode places the picture once at a
      reference point at a stated size and leaves the rest of the shape empty. `BitmapPaint`
      either tiles or stretches, and neither is that; a `PlacedPicture` anchored inside the shape
      would express it exactly, and the reason it is not done is that it needs a shape to be able
      to carry a picture *and* a fill at the same origin, which the corpus gives no way to check.
- [ ] **`a:srcRect` and `a:fillRect` on a shape's fill**, as opposed to on a `p:pic`. Honoured on
      the picture and ignored on the fill, because a cropped *fill* is a crop of the tile rather
      than of the shape and `BitmapPaint` names one image with no window into it. LibreOffice's
      own handling is three nested cases spanning sixty lines (`fillproperties.cxx:652-737`) and
      ends by physically cropping the bitmap, which is a codec operation this library will not do.
- [ ] **A gradient on a rotated shape with `rotWithShape="0"`.** The fill is computed in the
      shape's own box and travels with its placement, so a gradient always turns with the shape
      here. DrawingML's flag says it should not, and LibreOffice honours it by zeroing the shape's
      contribution to the shade angle (`fillproperties.cxx:524-527`). Expressing it needs the
      gradient's own transform to be the *inverse* rotation composed with the placement, which is
      three lines — unmeasured, because the corpus states `rotWithShape="0"` only on upright
      shapes, where it makes no difference.
- [ ] **`a:lin/@scaled`.** Read and carried, never acted on, which is also what LibreOffice does
      with it: `moShadeScaled` is assigned in `fillproperties.cxx:356` and consulted nowhere.
      Honouring it would skew a gradient's axis by the shape's aspect ratio.
- [ ] **A focal radial**, `a:path` with an off-centre `a:fillToRect`. The centre is read and
      honoured; the *focus* — PDF's `/ShadingType 3` takes two circles with different centres —
      is not, because `GradientPaint` has a single `Start`. A Core change, recorded in
      `src/Paperless.Rendering/TODO.md` from the other side and not worth making until a corpus
      document needs it.
- [ ] **`fo:clip` on an ODF picture frame.** ODF states a crop as four absolute lengths off the
      *original* image, so converting it to the fractions `SlideImages.Uncropped` wants needs the
      picture's natural size — which the header reader already gives. Not done because the corpus
      has none and an unverified crop is worse than an uncropped picture, which is at least
      visibly wrong in a way an author would notice.
- [x] **Preset dash patterns.** All ten of `a:prstDash`, in `Layout/SlideDashes.cs`. They are
      not a table of lengths: a preset states a *count* of dots and dashes with each length as a
      percentage of the pen, and `XDash::CreateDotDashArray` lays them out dots first then
      dashes, each followed by one gap — so `dashDot` draws long, gap, short, gap, which is the
      opposite order from its name. Measured on `slide-shape-features.pptx`: `dash` on a stated
      three-point pen comes out `[12.0187 9.01402]` in the reference and `[12.0189 9.0142]` here.
- [ ] **`a:custDash`.** The stop list is read as far as being ignored. LibreOffice cannot
      represent more than two distinct lengths either — `lclConvertCustomDash` folds the list into
      one "dots" length and one "dashes" length and then guesses which named ODF style it was
      (`lineproperties.cxx:91-140`) — so a faithful port has to reproduce the guessing as well as
      the folding, and nothing in the corpus carries one.
- [x] **Arrowheads.** `a:headEnd`/`a:tailEnd`, all five marker types, in
      `Layout/SlideLineEnds.cs`. A marker is a *filled polygon* beside the shaft rather than a
      property of the stroke, which is why the display list needed no new record.
- [ ] **Compound lines.** `cmpd="dbl"`, `"thickThin"`, `"tri"`. A double line is two strokes with
      a gap, and the widths are fractions of the stated one; nothing in the corpus carries one.

### Two gradient conventions that are invisible except in colour

A gradient's geometry is easy to check and tells you almost nothing: a red-to-blue ramp drawn
blue-to-red is the right shape, in the right place, at the right size, and is the wrong picture.
Both of the mappings below were found by comparing rendered colours against LibreOffice's, and
both are the kind of thing a reader gets backwards and then confirms with a test that measures
the axis.

**ODF's `draw:start-color` paints the outer edge of a centred gradient, not its centre.**
`getRadialGradientAlpha` returns `1 - hypot(x, y)` (`basegfx/source/tools/gradienttools.cxx:641`),
so the ramp is measured *inwards* and alpha reaches 1 — the end of the stop list — in the middle.
The corpus states a `#00c0c0`-to-`#101010` radial and LibreOffice renders it with a black centre.
The same applies to `ellipsoid`, `square` and `rectangular`, which all use the same `1 - …` form.
So `OdpSlideLayout.Gradient` swaps the ends, and the backends keep one convention: stop 0 sits at
`GradientPaint.Start`, which for a centred gradient is the centre.

**DrawingML's first `a:gs` is already the centre, and it looks like it should not be.** LibreOffice
*reverses* the OOXML stop list for a path gradient (`fillproperties.cxx:544`) before handing it to
the model that then reads it outside-in, so the two reversals cancel. The corpus proves it: the
flat ODF says start `#00c0c0`, end `#101010`, and LibreOffice's own PPTX export of the same slide
says `a:gs pos="0"` is `101010`. Two files, opposite orders, one picture — which is why the
`SlidePaintTests` are a `[Theory]` over both formats rather than two files of assertions.

**The trap that cost the time: `draw:angle="900deg"` is 900 degrees, not 90.** ODF 1.1 left the
unit undefined and OpenOffice wrote tenths of a degree; ODF 1.2 says degrees and LibreOffice now
writes the suffix. `Converter::convert10thDegAngle` (`sax/source/tools/converter.cxx:878`)
multiplies a bare number by ten for a 1.2-or-later document and leaves it alone otherwise, and
multiplies a `deg`-suffixed one by ten in *both*. Read the old way, a 1.3 file's 90-degree
gradient becomes 9 degrees — which is not obviously wrong on a rectangle, because a 9-degree ramp
still looks like a ramp. It shows up as a mean absolute error of about 0.02 and nowhere else.

**And the one the backend agent had already measured, restated because a reader has to reproduce
it: a radial's outer radius is half the shape's *diagonal*.** `Gradient::GetBoundRect` builds a
square of side `hypot(w, h)` for `GradientStyle_RADIAL` (`vcl/source/gdi/gradient.cxx:246-251`);
`ELLIPTICAL` instead scales each axis by √2. Half the width instead moves the page's mean absolute
error from 0.0016 to 0.0054.

Two more, smaller, both from `initEllipticalGradientInfo` and `init1DGradientInfo`:

- A linear gradient's axis spans `w·|dx| + h·|dy|`, the *rotated* extent of the box
  (`gradienttools.cxx:75-81`), not the box's own height. Using the height leaves a diagonal
  gradient's corners flat.
- `draw:border` shortens the ramp rather than shifting it, and which end it holds depends on which
  end the format put first — so after the ODF swap a centred gradient's border is at the far end
  of the stop list. `SlideGradients.WithBorder` takes that as a parameter for exactly that reason.

### A tile's size needs the picture's size, and the picture must not be decoded

`a:tile/@sx` is a percentage of the picture's *natural* size, so a reader cannot place a tiled
fill without knowing how large the picture is — and it must not decode one, because that would
put a codec on the extraction path. `Layout/SlideImages.cs` reads the header instead: PNG's
`IHDR` and `pHYs`, JPEG's `SOFn` and JFIF density, GIF's screen descriptor, BMP's `BITMAPINFOHEADER`.

The physical size, not the pixel count, is what the scale multiplies. LibreOffice asks the graphic
for its `Size100thMM` and converts the pixel size at the *screen's* resolution when the format
states none (`GraphicHelper::getOriginalSize`, `oox/source/helper/graphichelper.cxx:302`), which is
96 dpi. Measured: LibreOffice's own export of a one-centimetre checkerboard writes
`sx="471698"` over an 8-pixel image, and 8 px × 25.4/96 mm × 4.71698 is 9.984 mm. Assuming 72 dpi
instead would give 13.3 mm and a visibly coarser grid.

The grid is anchored on the *middle* of the shape. `a:tile/@algn` and `draw:fill-image-ref-point`
both default to a corner in their schemas and are both written centred by LibreOffice, and
anchoring top-left instead shifts every tile by up to half a tile — small, uniform, and exactly
the kind of difference a shape-by-shape comparison does not notice.

### What the fills were measured against

`tests/corpus/features/paint-fills.fodp` — one slide with a linear, an axial and a radial gradient
and a one-centimetre tiled checkerboard, and a second slide with an embedded picture — plus
`paint-fills-pptx.pptx`, which is LibreOffice's own export of it and keeps all three gradients as
`a:gradFill` and the tile as `a:blipFill`/`a:tile`. Both documents survive a round trip through
LibreOffice, which is what makes the pair usable as one reference for two readers.

**The comparison is picture for picture, and it cannot be anything else.** Impress decomposes
every shape gradient into flat bands before its PDF writer sees one — tdf#150551, in
`VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D` — so LibreOffice's PDF of this
slide holds *no shading dictionary at all* against the three ours states, and its page-one content
stream is 91602 bytes against our 2570. At 150 dpi, per channel:

| Comparison | Page 1 (three gradients, one tiled fill) | Page 2 (one picture) |
|---|---|---|
| Our raster against LibreOffice's rendering, flat ODF | mae **0.0016**, ink ratio 1.003 | mae **0.0020** |
| Our PDF against its PDF, poppler reading both, flat ODF | mae **0.0007** | mae **0.0001** |
| The same, from the PPTX | mae **0.0009** | mae **0.0001** |
| Our ODF rendering against our own OOXML rendering | mae **0.0002** | mae **0.0000** |

The last row is the sharpest and only the pair of documents can ask it: the two files are the same
slide written twice, so any difference is one of the readers having a convention backwards, with
no antialiasing, no band decomposition and no image filtering in between to hide behind.
`SlidePaintComparisonTests` holds the reference rows under 0.002 and the cross-format row under
0.001; `SlidePaintTests` pins the numbers those rows depend on, so a regression says which mapping
broke rather than only that the page changed.

`deck-features.pptx` was re-checked end to end and is unchanged: 3/3 pages and 43/43 words against
`soffice --convert-to pdf`.

**A bug in `Paperless.Rendering` had to be fixed to see any of this.** Four guards asked
`image.Width <= 0` before drawing a `RasterImage`, which is the right question only for an image
that has already been decoded — and a reader emits `RasterImage.Encoded`, whose dimensions are
zero until a codec has seen the bytes. Every picture every reader emits was therefore discarded,
silently and only in the backends. The first `p:pic` laid out perfectly and rendered a blank page.

### Text: what the runs know, and the rung of the chain that is missing

A run's own `a:rPr`, the paragraph's `a:defRPr` and the body's own `a:lstStyle` entry for the
paragraph's level are read, attribute by attribute rather than element by element — the same
`assignUsed` rule extraction already follows (`oox/source/drawingml/textparagraph.cxx:51-67`).

- [ ] **The rest of the chain, for size, typeface and colour.** The layout placeholder's list
      style, the master's, `p:txStyles` and the theme's `txDef` are *not* consulted.
      `PptxTextStyles` already walks precisely that chain for bullets and emphasis, so this is
      reading three more attributes off elements the walk already visits — the reason it is not
      done is that no deck in the corpus can tell a correct implementation from a plausible one.
      Every run in `shape-geometry.pptx` and in `deck-features.pptx` states its size, face and
      colour outright, because LibreOffice's exporter writes them on every run. The measurement
      that would settle it has to come from a PowerPoint-authored deck in
      `sd/qa/unit/data/pptx/`, the same place the emphasis inheritance was settled.
- [x] **`a:buChar` bullets are drawn, and the hanging indent goes to the marker.** A marker is a
      run of its own at its own pen, in its own face at its own size — LibreOffice writes it as a
      separate `/Lbl` block — so it is on `SlideParagraph.Marker` rather than prefixed to the
      text, which would shift every character offset the runs index by. **The rule that matters:
      a hanging indent is the room the marker occupies, not a first-line indent.** LibreOffice
      draws the bullet at `marL + indent` and the paragraph's own first line at `marL`; applying
      the indent to the text as well puts every bulleted line a whole hanging indent to the left.
      Measured on `deck-features.pptx`, whose outline states `marL="216000" indent="-216000"` —
      17.008 pt: the reference draws bullet and text at 56.693/73.701 for the first level and
      73.701/90.709 for the second, and so do we, to a hundredth of a point.
- [ ] **`a:buAutoNum` yields no marker at all**, deliberately: numbering needs a counter per
      outline level carried across the body and restarted where the level rises, and inventing
      "1." for every item of every list is a worse answer than none. The paragraph still gets its
      hanging indent, so its text lands where the reference puts it and only the number is
      absent. `OutlineNumbers` already has every numeral format extraction needs.
- [ ] **A marker's baseline is 8.2 pt higher in the reference than here.** Measured on
      `deck-features.pptx`: LibreOffice draws the first outline bullet at 175.72 and its text at
      183.91, and the third at 254.30 against 262.49 — the same 8.19 pt each time, so it is a
      rule rather than drift, and the horizontal pens agree exactly. We put the marker on the
      text's own baseline. Whatever EditEngine does with a 12.6 pt label on a 28 pt line, it is
      not "same baseline" and it is not "same line top" either, which would be 15.4 pt under the
      font-independent rule. Worth chasing in `impedit3.cxx`'s label placement when markers get
      their own comparison.
- [ ] **`a:spcPct` paragraph spacing.** `a:spcBef`/`a:spcAft` are honoured in points
      (`a:spcPts`) and ignored as a percentage, because the percentage is of the line height and
      the line height is not known until the paragraph's runs are. It belongs with the line
      heights rather than with the reader.
- [ ] **Character spacing (`spc`) and kerning (`kern`).** `spc` is in hundredths of a point and is
      not applied; on the corpus deck it is −1, which is a hundredth of a point per character and
      inside every tolerance here, but a deck that tracks a title tightly would drift visibly.
- [ ] **`spAutoFit`.** `normAutofit`'s stated `fontScale` and `lnSpcReduction` are applied, which
      is what LibreOffice does — it honours the value the authoring application arrived at rather
      than solving the fit again (`oox/source/drawingml/textbodypropertiescontext.cxx:240-243`),
      and a reader that recomputed would disagree with the reference on every autofitted shape.
      `spAutoFit` is the other direction — grow the shape to fit the text — and is not applied at
      all, so a shape sized to its text in the file is drawn at the size the file states.
- [ ] **Vertical text** (`a:bodyPr/@vert`) and text rotation (`a:txXfrm`, `moTextAreaRotation`).
- [ ] **Right-to-left text.** Bidi levels are resolved and carried by `MeasuredParagraph` and
      nothing consumes them here, which is the same open item word processing has.

### Small differences that are measured and not yet closed

- [ ] **A wrapped line is one glyph shorter here than in the reference.** LibreOffice draws the
      space a line broke at as part of that line's run; the shared layouter stops at the last
      *visible* character (`LineBox.VisibleEnd`). Nothing is visibly missing — it is a space at
      the end of a line — but a glyph-count comparison sees it, and
      `SlideTableComparisonTests` accepts one fewer for exactly this reason rather than comparing
      sets and hiding it.
- [ ] **A shape's line join.** We write a mitre where LibreOffice writes a round join on a
      connector (`0 J 1 j` against our `0 J 0 j`). Invisible on a straight line, visible on a
      bent connector's corner.
- [ ] **The fill rule.** Our PDF writer fills with `f` where LibreOffice uses `f*`. It agrees on
      every preset with a hole — `donut`, `frame` — because the preset file winds the inner
      subpath the other way precisely so that both rules give the same answer, but a `custGeom`
      whose author did not would differ. That belongs to `Paperless.Rendering`.

### Two things the layout deliberately keeps out of the display list

**A shape's outline is emitted already in slide coordinates**, point by point, rather than as a
path plus a matrix for a backend to apply. An affine map takes a cubic Bezier's control points to
the control points of the mapped curve exactly, so nothing is lost — and what is gained is that a
fill's coordinates in our PDF are directly comparable with a fill's coordinates in LibreOffice's,
which is the whole basis of the comparison. A rotated shape emits rotated coordinates and looks
like a general path, which is correct and is also why `PdfFills` cannot read it.

**Text is the exception, and has to be.** A `GlyphRun` carries an origin and advances rather than a
matrix, so a rotated shape's text cannot be baked into slide coordinates. An upright shape's runs
therefore go into slide coordinates with no matrix at all — the common case, and the one where a
backend's pens compare directly against the reference's — and a rotated or mirrored one's stay in
the shape's own space and travel with `PlacedText.Transform`.

## Open questions

- [ ] Render animations' final state or initial state? Initial matches what a static export
      shows; confirm against the reference.
- [ ] Are connectors worth routing properly, or is a straight line acceptable initially?
- [ ] Should a master's *non-placeholder* shapes — a logo, a running strapline — be extracted?
      Half of this is now measured, and it is the half that was in doubt. **They are visible.**
      LibreOffice's own PDF export renders `master-slides.pptx`'s "Copyright © SUSE",
      `cshapes.pptx`'s "© Novell, Inc. All rights reserved." and `tdf149865.pptx`'s
      "Copyright © SUSE 2021" onto the slide, and Paperless reports none of the three: that text
      is simply lost, not deferred.
      **And they are rare.** Scanning all 389 decks in `sd/qa/unit/data/pptx/` for a master
      `p:sp` with no `p:ph` and non-empty `a:t`: six decks match, and three of those carry only
      the `‹#›` slide-number glyph. Four have real strapline text — the three above plus
      `slide-sections.pptx`, whose master strapline LibreOffice renders on exactly one of its
      seven pages, so even "visible on every slide" is not reliably true.
      So the trade is now numbers rather than intuition: extracting costs a repeated line on
      ~1% of decks, not extracting loses a line on the same ~1%. That is small enough either way
      that the deciding factor should be what a caller wants, and the remaining question is only
      that. `showMasterSp` on the slide and on the layout decides visibility, and neither of the
      four decks states it, so it is not the discriminator it looks like — the machinery is there
      but real files leave it at its default and rely on the shape being off-slide or invisible
      instead.
