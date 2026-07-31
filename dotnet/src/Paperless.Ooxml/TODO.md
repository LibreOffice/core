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

- [ ] Parse `theme1.xml`: colour scheme, font scheme, format scheme
- [ ] Resolve colour references: `srgbClr`, `schemeClr`, `sysClr`, `prstClr`, `hslClr`,
      `scrgbClr`
- [ ] Apply the transform chain **in document order and in the right colour space**:
      `lumMod`, `lumOff`, `shade`, `tint`, `satMod`, `alpha`, `hueMod`, `inv`, `gray`.
      Arithmetic is in `research/04-impress.md` section B.
- [ ] `phClr` placeholder-colour substitution inside style matrix references
- [ ] Format scheme: `fillStyleLst`, `lnStyleLst`, `effectStyleLst`, `bgFillStyleLst`

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
- [ ] Effects: shadow, glow, soft edge, reflection. Shadow first; the rest are lower value.
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
- [ ] Charts (`c:`) — open question in the master TODO; embedded charts are common
