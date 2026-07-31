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
textbox, Word an `FSPA` index in the client anchor, and Excel a cell reference. The PPT reader
is the first consumer; the DOC reader's open shape item should be able to use the same code.

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
- [ ] Blip store; picture extraction, including the metafile blip types
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
