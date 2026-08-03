# Paperless.Ooxml — TODO

Shared OOXML infrastructure: DrawingML, themes, VML, part/relationship resolution.
Everything here is used by all three format libraries.

Reference: `research/04-impress.md` section B (DrawingML in most depth);
`research/01-formats-and-detection.md` for content types.

## Foundations

- [x] Namespace handling for **both** ECMA-376 1st edition and ISO/IEC 29500 strict. Done by
      rewriting strict names to their transitional equivalents once, at load
      (`OoxmlXml.Normalise`), rather than checking two URIs at every comparison — which is the
      version of this that gets forgotten in one place and yields a silently empty document.
- [x] `mc:AlternateContent`: pick the highest-fidelity branch understood, fall back
      otherwise. Done, and resolved in place at load so nothing downstream sees the element. A
      choice wins only when every namespace its `Requires` names is one Paperless can read,
      because the fallback exists precisely for the case where the choice cannot be. Walking
      both branches is the bug this prevents: a text box's text extracted twice.
- [x] Metadata from all three `docProps` parts (`OoxmlMetadata`), shared by all three families.
- [ ] A streaming XML reader over parts — `XmlReader`, not a DOM. Some parts are very large.
      Currently every part is loaded as an `XDocument`, which is what makes the
      namespace-and-compatibility normalisation above a single pass; revisit if a real file
      makes it hurt.

## Theme and colour

**The single most visible failure mode in PPTX rendering.** A wrong transform chain makes
every themed shape on every slide the wrong colour at once.

Colour resolution is in `DrawingML/` and is done: `DrawingColour` is the reference,
`DrawingColourTransforms` the chain, `DrawingTheme` the scheme and the map. All three families
can call it — DOCX does today, through `WordThemeColour`; XLSX's `theme=` index and PPTX's
`a:schemeClr` are unblocked and need only their own readers, not a second chain.

- [x] Parse `theme1.xml`'s colour scheme, and locate the part **by relationship** rather than by
      name. The trap is inside the scheme rather than around it: `dk1` and `lt1` are an
      `a:sysClr` with a `lastClr` in every theme Word ships, not an `a:srgbClr`, so a reader that
      handles only `a:srgbClr` silently loses the two slots "text 1" and "background 1" refer to.
      `lastClr` is preferred to asking the host, since a document must not change colour under a
      dark desktop theme.
- [x] Resolve colour references: `srgbClr`, `schemeClr`, `sysClr`, `prstClr`, `hslClr`,
      `scrgbClr`. The last two go back through the transform chain rather than being converted
      directly, because `scrgbClr`'s components *are* the chain's CRGB space and `hslClr`'s are
      its HSL — a second conversion would be a second place to get the gamma wrong.
      `PresetColours` is transcribed from `color.cxx`'s `constDmlColors`, not from the X11 list it
      resembles: DrawingML abbreviates (`dkBlue`, `ltGray`, `medPurple`) and at least one value
      differs, `ltGoldenrodYellow` being `FAFA78` here and `FAFAD2` in X11.
- [x] Apply the transform chain **in document order and in the right colour space**, ported from
      `oox/source/drawingml/color.cxx`:723 — the authority because it is what LibreOffice renders
      with, and every comparison in this project is against LibreOffice's rendering.
      Two facts carry the whole item and neither is guessable from the element names.
      **It is not commutative.** `lumMod` then `shade` is a different colour from `shade` then
      `lumMod`, because the two work in different spaces. Measured on one scheme colour with
      `lumMod 50%` and `shade 60%` both ways round, LibreOffice paints #3F4E20 and #3E4A23.
      **`shade` and `tint` carry a gamma round trip.** They act in "CRGB" — thousandths of a
      percent per component, gamma-decoded with an exponent of **2.3** — rather than on the 8-bit
      components, so a 50% shade of accent 1 (`4F81BD`) is `3A5F8B` and not the `284560` a plain
      multiply gives. The specification describes the plain multiply; following it leaves every
      shaded fill perceptibly too dark while still looking like a plausible colour, which is the
      failure mode this whole area is about.
      Verified against LibreOffice's own PDF on `theme-colours.docx`, twelve themed shape fills,
      every one matching to the byte: both orderings above, `lumMod`/`lumOff`, `satMod`,
      `a:gray`'s 22/72/6 weights, and the colour map.
- [x] The `a:clrMap`, which is why "background 1" is not simply the scheme's first colour. Only
      the twelve document-facing names (`bg1 tx1 bg2 tx2 accent1…6 hlink folHlink`) are mapped;
      `dk1`/`lt1` address the theme's storage and never move, which is the whole reason both
      spellings exist — a slide master that swaps light for dark still wants `lt1` to be the light
      colour. WordprocessingML's `w:clrSchemeMapping` is the same map under different attribute
      names and normalises onto the same keys.
- [ ] `phClr` placeholder-colour substitution inside style matrix references. The kind exists
      (`DrawingColourKind.Placeholder`) and `Resolve` takes the substitute, so what is missing is
      a caller: nothing reads `a:lnRef`/`a:fillRef`/`a:effectRef` yet, and until something does
      there is no colour to pass in.
- [ ] Format scheme: `fillStyleLst`, `lnStyleLst`, `effectStyleLst`, `bgFillStyleLst`
- [x] The font scheme, in `DrawingFontScheme`, hung off `DrawingTheme.Fonts` as a non-positional
      member so that every caller constructing a theme from a colour scheme alone keeps compiling.
      Six typefaces, major and minor across Latin, East Asian and complex script — and it exists for
      the *indirection* rather than for the names. `<a:latin typeface="+mn-lt"/>` is a reference,
      not a family, and a reader taking it literally reports a font called `+mn-lt`, which resolves
      to nothing: every run of a PowerPoint-authored deck then silently gets a substitute face and a
      different set of advance widths. `Theme::resolveFont` (`oox/source/drawingml/theme.cxx`:71)
      recognises exactly the six-character `+mj-lt` shape and nothing else.
      The smaller trap beside it: every theme Word ships writes `<a:ea typeface=""/>` rather than
      omitting the element, so testing for the attribute's presence yields two typefaces named ""
      and hands them to font resolution.
      What is still open is Word's own spelling of the same idea — `majorHAnsi` and `majorAscii`
      both mean the Latin face, and a supplemental typeface for the run's script beats it
      (`writerfilter/dmapper/ThemeHandler.cxx`). That is a `w:rFonts` reader rather than a scheme
      one, and it needs the per-script supplemental lists this deliberately does not hold.
- [x] The character properties a run resolves to that the content tree does not carry — colour,
      size and the three typefaces — in `DrawingCharacterStyle`. Kept apart from the emphasis flags
      `DrawingTextBody` resolves, because these are the three that need the *theme* to answer and
      because they are what a renderer wants rather than what an index does.
      `Resolve` is the whole chain in one place, strongest first: the run's `a:rPr`, the paragraph's
      `a:defRPr`, the body's `a:lstStyle` for the level, the **shape's own text style**, then each
      inherited source. That middle rung is the one with no element inside the text body — it comes
      from `p:style/a:fontRef` — and both extremes give the right answer on every shape that states
      the colour once, so only a document stating it twice can tell them apart. See
      `Paperless.Presentations/TODO.md` for the deck that does, and what it measured.
      `sz` is hundredths of a point, not `w:sz`'s half-points, and the two vocabularies share a
      package — a DOCX shape holds DrawingML text inside WordprocessingML — so applying one unit to
      the other is a factor of fifty away.
      Only `a:solidFill` is read as a colour. A gradient text fill is a real thing DrawingML can
      express and not one a single colour stands for, so it reads as unstated and falls through
      rather than being flattened to its first stop, which would be a colour the file never asked
      for.

## DrawingML shapes

- [ ] `spPr`: transform (`xfrm`) with flip and rotation; **flip is applied before
      rotation** — the other order gives mirrored output
- [ ] Preset geometries: ~190 of them. Port LibreOffice's generated data tables
      mechanically, then write one evaluator for the guide-formula language. The same
      evaluator must serve the ODF and legacy binary syntaxes — that is how LibreOffice
      structures it, and duplicating it three ways would be a mistake.
- [ ] Custom geometry (`custGeom`): explicit path lists
- [ ] Fills: `solidFill`, `gradFill`, `blipFill`, `pattFill`, `noFill`, `grpFill`
- [ ] Lines: width, dash, caps, joins, head/tail arrows
- [x] Effects: **`a:outerShdw`**, inline and through the theme's `a:effectRef`
      (`DrawingML/DrawingEffects.cs`). Glow, soft edge, reflection and `a:innerShdw` are still
      not read, which matches the reference: `EffectProperties::pushToPropMap` acts on
      `outerShdw` alone and drops the rest.
- [ ] `txBody`: paragraphs, runs, `lvlXpPr` list styles, insets, anchoring, autofit
      (`normAutofit` `fontScale` and `lnSpcReduction`)
- [ ] Groups, with child coordinate-space mapping (`chOff`/`chExt`)
- [ ] Tables (`a:tbl`)
- [ ] Units: EMU natively; rotation in 60000ths of a degree

## VML

Legacy but still needed: older files, and comment/annotation shapes in current ones.

- [ ] Enough shape and path support to render comment anchors and simple shapes

## Other

- [ ] Image parts: media types, and the `blip` `embed`/`link` distinction
- [ ] SmartArt (`dgm:`) — **decide** whether to use the pre-rendered drawing fallback the
      file carries (cheap, probably sufficient) or implement diagram layout (large)
- [x] Charts (`c:`) — `DrawingML/DrawingChart.cs`, reading `c:chartSpace` into a section of
      paragraphs and one table. Family-blind on purpose: the chart part is identical whether the
      graphic frame pointing at it is on a slide, a worksheet or a page of a Word document, so the
      three family readers supply only the relationship hop and none of the vocabulary. Values
      come from `c:numCache`/`c:strCache` and never from the workbook `c:f` names — the reasoning,
      the LibreOffice citations and the measurements are in the master TODO's Phase 3.5
- [ ] Charts (`cx:`) — the 2014 "chartex" vocabulary funnel, waterfall, treemap and histogram
      charts use, whose data is `cx:chartData/cx:data` rather than `c:ser`. Exactly one file in
      LibreOffice's own 192-document OOXML chart corpus uses it, which is why it waits
