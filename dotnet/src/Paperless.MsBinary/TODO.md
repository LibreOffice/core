# Paperless.MsBinary — TODO

Shared legacy binary infrastructure: Escher drawings, OLE property sets, record streams,
codepages. Used by the DOC, XLS and PPT readers.

Reference: `research/04-impress.md` section C (Escher);
`research/05-infrastructure.md` sections F and G.

## Record streams

- [ ] `IRecordReader` over a CFB stream
- [ ] **Clamp overlong records rather than throwing.** A record whose declared length runs
      past the end of the stream is common in thirty-year-old files written by dozens of
      applications; rejecting them makes a large fraction of a real corpus unreadable.
      Clamp, continue, and record a `Diagnostic`.
- [ ] Continuation records (BIFF `CONTINUE`), where a logical record spans several physical
      ones
- [ ] Nested container traversal

## Escher / MS-ODRAW

High leverage: DOC, XLS and PPT all delegate their drawings to Escher, so implementing it
once buys shape support in all three — the same leverage LibreOffice gets from a single
`msdffimp.cxx`.

- [ ] Record header: version, instance, type, length
- [ ] Container traversal (`DrawingGroupContainer` → `DrawingContainer` → shape containers)
- [ ] The sparse shape-property table (`DFF_Prop_*`): properties arrive by id, not as fixed
      fields, so the decoder is a property-id lookup
- [ ] Blip store; picture extraction, including the metafile blip types
- [ ] Shape anchors: client and child, and their different coordinate spaces
- [ ] Group shapes with coordinate mapping
- [ ] Map the built-in shape types onto the same preset-geometry evaluator DrawingML uses —
      do not build a second one

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
- [ ] PPT: per-run character properties can override
- [ ] Map LCIDs to BCP 47 (`research/05-infrastructure.md` section F)

## Legacy crypto glue

- [ ] Shared RC4 and XOR-obfuscation key derivation used by all three formats
