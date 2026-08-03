# Paperless.MsBinary — TODO

Shared legacy binary infrastructure: Escher drawings, OLE property sets, record streams,
codepages. Used by the DOC, XLS and PPT readers.

Reference: `research/04-impress.md` section C (Escher);
`research/05-infrastructure.md` sections F and G.

## Record streams

- [x] The eight-byte drawing-layer header and a random-access walk over it
      (`Records/DffRecord.cs`). Random access rather than forward-only because nothing in
      these formats can be read in one pass: PowerPoint's persist directory turns an object id
      into a byte offset, and Escher's property table stores its complex values in a block
      *after* the fixed entries. PowerPoint's own `PPT_PST_*` records share this header, so one
      walk serves both vocabularies where they interleave in the same stream.
- [x] **Clamp overlong records rather than throwing.** A record whose declared length runs
      past the end of the stream is common in thirty-year-old files written by dozens of
      applications; rejecting them makes a large fraction of a real corpus unreadable. A
      zero-length atom advances by the header alone, which is what stops the run of zero bytes
      at the end of a truncated stream from looping forever.
- [ ] `IRecordReader` over a CFB stream. Still only the interface: every caller so far reads
      the whole stream into memory first, because every one of them needs random access.
- [ ] Continuation records (BIFF `CONTINUE`), where a logical record spans several physical
      ones

## Escher / MS-ODRAW

High leverage: DOC, XLS and PPT all delegate their drawings to Escher, so implementing it
once buys shape support in all three — the same leverage LibreOffice gets from a single
`msdffimp.cxx`.

Written to the format rather than to one host's use of it. The `ClientAnchor`, `ClientData`
and `ClientTextbox` records are reported as *unparsed* record headers, because their payloads
are by definition whatever the host chose: PowerPoint puts its text records in the client
textbox, Word an `FSPA` index in the client anchor, and Excel a cell reference. PPT was the
first consumer and DOC the second, and the second needed no change to the traversal at all —
only the additions below, all of them things the format states and no host owns.

What the DOC reader had to supply for itself, so that XLS's `NOTE` comments know where the
line falls: the `PlcSpa` table that maps a character position to a shape id; the **one label
byte** Word writes before each `DgContainer` (`msdffimp.cxx:5997` advances one byte and
re-reads the header when the first attempt is not a drawing, which is what that byte is); and
which of Word's property ids mean what. None of that belongs here.

- [x] Record header: version, instance, type, length (shared with the record walk above)
- [x] Container traversal (`DrawingContainer` → `ShapeGroupContainer` → `ShapeContainer`).
      **A group's first child shape container is the group itself**, carrying its flags and
      its `msofbtSpgr` coordinate space rather than describing anything drawn; treating it as
      content puts a phantom empty shape in front of every group.
- [x] The sparse shape-property table (`Escher/EscherPropertyTable.cs`). The entry count is in
      the record header's *instance*, not derivable from the length, and the complex block
      starts exactly `6 × count` bytes in — so a miscounted table reads every complex value
      six bytes per entry out of place and yields plausible strings rather than an error.
- [x] Group shapes, nested, with a depth cap: an eight-byte container costs a stack frame, so
      a small hostile file can otherwise ask for tens of thousands of them.
- [x] **Boolean properties are not stored under their own identifier.** Each group of them is
      one thirty-two-bit entry written under the group's highest id — `id | 31` — with the value
      in bit `1 << (base - id)` of the low half and a "was this stated" bit sixteen places
      higher (`DffPropSet::GetPropertyBool`, `dffpropset.cxx:1308`). So `fLine` is bit 3 of
      property 511 and `fFilled` bit 4 of property 447, and asking for 508 or 443 finds nothing
      whatever the shape said. The old constants named the *group* ids, which made
      `Value(Filled)` compile and return the whole word — non-zero for any shape stating any
      fill property at all, so every shape came back filled. `Boolean` and `StatesBoolean` now
      do the arithmetic, and the second is needed as much as the first: "stated false" and "said
      nothing" take different defaults in every host. `fShadow` is the same trap once more and
      the least obvious of the three: it is **bit 1 of property 575**, whose own name is
      `fshadowObscured`, so the shadow switch is read as `Has(575) && Boolean(574)` —
      exactly LibreOffice's `IsProperty(DFF_Prop_fshadowObscured) && (GetPropertyValue(…) & 2)`
      (`msdffimp.cxx:2665-2668`).
- [x] **The tertiary table is kept apart from the secondary.** They share a layout and mean
      different things: `msofbtSecondaryOPT` holds a master's values for the shape's own
      properties, `msofbtUDefProp` is where a host writes properties of its own. Word puts a
      floating shape's position origins — `posh`, `posrelh`, `posv`, `posrelv`, ids 0x038F to
      0x0392 — there and nowhere else (`msdffimp.cxx:5216`, which reads them as raw six-byte
      entries rather than through the property-set machinery). Merging the two, as this did, took
      whichever table came first in the file.
- [x] `ReadShape` is public, because a shape does not always live in a drawing: Word stores an
      *inline* shape alone in the picture stream, immediately after the `PICF` header that
      locates it, with no `DgContainer` and no group around it.
- [ ] Blip store; picture extraction, including the metafile blip types. A shape is currently
      told to *be* a picture by its `pib` property being set, which is all a reader classifying
      an anchor needs and is not enough to get the bytes out.
- [ ] Shape anchors: the child anchor and the group's coordinate space are decoded, but
      **mapping a child's coordinates through its group's space is not**. A child anchor is in
      the parent's `msofbtSpgr` units and needs the two-rectangle scale
      `msdffimp.cxx:4329` applies; extraction never asks, rendering will.
- [ ] Map the built-in shape types onto the same preset-geometry evaluator DrawingML uses —
      do not build a second one. Only the type number is read so far.
- [ ] The `hspMaster` inheritance chain: a shape with the `HaveMaster` flag takes unstated
      properties from the master shape that property names. The secondary and tertiary
      property tables are read; following the reference is not.

## OLE property sets

- [ ] The summary-information and document-summary-information streams (names begin with
      U+0005)
- [ ] Property-set structure: format id, section, property id/offset table
- [ ] The `VT_*` value types actually used
- [ ] **Property ids mean different things in each of the two property sets** — do not share
      one lookup table between them
- [ ] Custom properties from the user-defined section

## Codepages

Getting this wrong corrupts every non-ASCII character while leaving ASCII perfect — the
signature to look for when text comes out mangled.

- [ ] `Encoding.RegisterProvider(CodePagesEncodingProvider.Instance)` for the Windows
      codepages, which .NET does not include by default
- [ ] DOC/WW8: codepage derived from the language id in the FIB
- [ ] XLS/BIFF: the `CODEPAGE` record; BIFF8 is mostly UTF-16 but not universally
- [x] PPT: a `TextBytesAtom` is decoded as Windows-1252, not as the document's codepage and
      not as ISO-8859-1. The specification calls each byte the low half of a UTF-16 unit, which
      would make it Latin-1; LibreOffice decodes Windows-1252 (`svdfppt.cxx:5273`) and that is
      what makes a curly quotation mark written by PowerPoint read back as one. The two agree
      everywhere except 0x80–0x9F, where Latin-1 has controls and no real file means them.
- [ ] PPT: per-run character properties can override the above — a run naming a symbol
      typeface reinterprets its bytes in that font's own encoding.
- [ ] Map LCIDs to BCP 47 (`research/05-infrastructure.md` section F)

## Legacy crypto glue

- [ ] Shared RC4 and XOR-obfuscation key derivation used by all three formats
