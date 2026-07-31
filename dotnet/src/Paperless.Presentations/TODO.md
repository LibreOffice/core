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

Not yet: the *rendering* half of the inheritance chain — fill, line, size, colour and typeface.
Extraction resolves the half it can observe: bullets per level, and now emphasis, baseline and
language per level through the `a:defRPr` chain. Also not yet: charts, and everything below.

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
- [x] Shape tree: rectangles, paths, groups and placeholders, in document order, which is z-order
      in both vocabularies. Not pictures (no decoder yet), tables or connectors — each has its own
      entry below.
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
- [ ] Per-level character inheritance of `sz`, the fill colour and the `a:latin`/`a:ea`/`a:cs`
      typefaces. Deliberately left: nothing in the content tree reports any of them, so there is
      no measurement that would tell a correct implementation from a plausible one. They change
      how the text looks and where it sits, not what it says. The walk is already there — adding
      them is reading three more attributes off the same elements — so this is waiting on the
      renderer, not on research.
- [ ] The shape's own text style, which sits between the master's list style and the body's
      (`oox/source/drawingml/textparagraph.cxx:63`). It comes from the theme's `txDef` and the
      shape style's `a:fontRef`, so it needs theme resolution; neither property it carries
      (typeface, colour) is one extraction reports.
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
- [x] Shape geometry: `rect`, `roundRect`, `ellipse`, `triangle`, `rtTriangle` and `diamond`,
      transcribed from the preset file. Everything else falls back to its bounding rectangle —
      see below for why that is the right failure and what the real evaluator costs.
- [x] Solid fills, including themed ones, and lines with width, cap and join. Not shadows, and
      not gradients: nothing here emits a `GradientPaint`, deliberately.
- [x] Text bodies with anchoring, insets and the stated autofit scale.
- [x] Groups with nested transforms, including a child coordinate space that scales.
- [ ] Pictures, including crop and the picture effects worth having. A `p:pic` is placed and its
      frame drawn — outline and line, so a missing image is a hole rather than nothing — but no
      raster is decoded, because nothing in the project decodes one yet
      (`src/Paperless.Rendering/TODO.md`, "Raster image decode").
- [ ] Tables. `a:tbl` extracts as a `ContentTable` and is not drawn: a table is a grid of text
      bodies with per-cell fills and per-edge borders, which is the word processor's
      `TableLayouter` problem again in a different vocabulary rather than a shape.
- [ ] Notes pages as separate output pages (optional)
- [ ] PPT (binary). Nothing lays out. The Escher reader already produces the shape tree and the
      anchors, but `SlideAtom`'s eight layout placeholder ids — the shape half of the placeholder
      relationship, recorded as missing in the PPT section above — are what a placeholder's
      position and fill come from, and without them a PPT title has no rectangle to be in.

### What renders, and what it was measured against

The corpus file is `shape-geometry.pptx` and its converted twin `shape-geometry.odp`, written for
this and described in `tests/corpus/README.md`. Every offset in it is a round number of inches, so
a disagreement is a bug rather than a rounding. `Paperless.Presentations.Tests` asserts against
numbers transcribed from LibreOffice's PDF once and quoted in each test, and needs no LibreOffice
to run; `Paperless.Fidelity.Tests/SlideRenderComparisonTests` re-derives them, rendering our own
PDF and comparing sheet sizes, rectangular fills with their colours, and every text run's pen,
baseline, size and glyph count against LibreOffice's, in both formats.

**What the richer decks measure at, which no test asserts yet.** `deck-features.pptx` and
`slides-features.odp` are not in the fidelity comparison, because two of their slides render
incomplete and pinning that would pin the gaps as though they were correct. What they do measure
is worth writing down, since it is where the next work is:

| | Reference runs | Ours | Agreement, and what is missing |
|---|---|---|---|
| `deck-features.pptx` | 21 | 16 | Titles, bullets and outline text land exactly — pens to a hundredth of a point, baselines to 0.04 — and the group and text-box slide to 0.04 across, 0.55 down inside the ellipse. Six of the reference's runs are the **table** slide, which draws nothing; one of ours is the **hidden** slide, which we lay out and its PDF export omits. |
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

### Preset geometry: six of a hundred and eighty-seven

`Layout/SlidePresetGeometry.cs` expands `rect` (and the aliases that are literally a rectangle in
the preset file), `roundRect`, `ellipse`, `triangle`, `rtTriangle` and `diamond`, each transcribed
from `oox/source/drawingml/customshapes/presetShapeDefinitions.xml`. Anything else draws its
bounding rectangle: in the right place, in the right colour, with the wrong outline — which is a
far better failure than drawing nothing, because it is *visible* in a comparison rather than
silently absent.

- [ ] **The real evaluator.** Each preset is a small program: guide formulas over the bounding box
      and the adjustment handles, then a path built from the results, with `arcTo`, `quadBezTo` and
      a dozen operators. LibreOffice compiles the file into data tables and runs one shared engine
      (`EnhancedCustomShape2d`) that also serves ODF's `draw:enhanced-path` and the legacy binary
      syntax, which is the right shape for a port: the data is mechanical and the engine is one
      piece of work that buys all three front ends. `a:custGeom` — a shape whose path the file
      states outright — needs the same path builder and none of the formula evaluation, so it is
      the cheaper half and should come first.

The **text** rectangle is carried beside the outline, because for two of the six it is not the
bounding box: an ellipse's is the box inscribed at 45°, which is
`ConstructPresetTextRectangle` (`oox/source/drawingml/transform2dcontext.cxx:66-73`) and the reason
a caption inside a circle does not touch its edge.

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
- [ ] **Gradients.** Nothing here emits a `GradientPaint`, and that is deliberate rather than
      pending: both backends currently draw one as its middle stop or not at all
      (`src/Paperless.Rendering/TODO.md`), so emitting one would make a wrong picture look like a
      right one and would be unverifiable besides. `a:gradFill` and ODF's `draw:gradient` should
      land together with the shading dictionaries and Skia shaders that consume them — the
      backend TODO names slide fills as exactly the feature that unblocks it, and the producer
      should not go first.
- [ ] **Pattern, hatch and picture fills**, for the same reason: `BitmapPaint` draws nothing in
      either backend, and a hatch has to be resolved into stroked lines by whoever reads it.
- [ ] **Dash patterns.** `a:prstDash` and `a:custDash` are read as far as being ignored, so a
      dashed outline draws solid. The `Stroke` record already carries a dash array and both
      backends honour it, so this is a table of seven preset patterns and nothing else.
- [ ] **Compound lines and arrowheads.** `cmpd="dbl"`, `a:headEnd`/`a:tailEnd`. A connector with
      no arrowhead is a visibly different picture, and connectors are the shapes that carry them.

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
