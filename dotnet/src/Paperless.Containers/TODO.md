# Paperless.Containers — TODO

The container layer: OLE2/CFB, ZIP, OPC, ODF packages, and decryption.

**Status: the readers are done; decryption is not started.** Everything above this layer is
now unblocked for unencrypted documents.

Reference: `research/05-infrastructure.md` sections A, B and C — it has exact byte offsets.

## Done

### OLE2 / Compound File Binary (`Ole2/CompoundFile.cs`)

Hand-rolled. `OpenMcdf` was evaluated and dropped: tolerance of malformed real-world files
was the deciding requirement, and the reader needs control over chain-walking and directory
traversal that a library does not expose.

- [x] Header parse, deriving sector sizes from the shifts rather than the version field,
      since some writers set an inconsistent version but a correct shift
- [x] DIFAT (header entries plus the sector chain) and FAT
- [x] Mini-FAT and the root-entry mini-stream, for streams below the 4096-byte cutoff
- [x] Directory walk that ignores the red-black ordering invariants malformed files violate,
      with a cycle guard on child/sibling pointers
- [x] `RootStreamNames` — what format identification needs
- [x] `RootClassId` exposed but documented as unreliable for identification, matching why
      LibreOffice detects by stream name instead
- [x] Robustness, each with a test: out-of-range sectors truncate and report; cyclic chains
      hit a guard instead of hanging; duplicate entries keep the first; a stream declaring
      more bytes than the file holds yields what was readable. Truncation past the directory
      is the one unrecoverable case and throws.
- [x] Stream names beginning with U+0005 (the summary-information streams)

### ZIP (`Zip/ZipPackageBase.cs`)

- [x] `System.IO.Compression.ZipArchive`, with entries indexed by normalised name
- [x] Part-name normalisation resolving `.` and `..`, so traversal cannot climb above the
      package root
- [x] Duplicate entry names tolerated, first wins, diagnostic recorded
- [x] Zip-bomb guards: per-part size, total extraction budget, and compression ratio.
      **Read the remarks on `MaxCompressionRatio` before touching it** — DEFLATE cannot
      exceed ~1028:1, so a threshold above that can never fire, while real office XML
      reaches ~342:1. The size caps are the actual defence.

### OPC (`Ooxml/OpcPackage.cs`, `Ooxml/OpcXml.cs`)

- [x] `[Content_Types].xml`: defaults by extension, then per-part overrides
- [x] `_rels/` relationships, with relative targets resolved against the source part's
      directory and absolute ones against the package root
- [x] Main document part located by **following the officeDocument relationship**, matching
      both the full URI and the type's final segment so strict and transitional packages
      both work
- [x] Malformed XML yields whatever parsed rather than throwing
- [x] One safe-reader factory with `DtdProcessing.Prohibit` and a null resolver (XXE/SSRF),
      used by every XML read in the library
- [x] Shared with `FormatIdentifier`, so the two cannot drift on content-type resolution

### ODF (`OpenDocument/OdfPackage.cs`)

- [x] `mimetype` as the authoritative document type
- [x] `META-INF/manifest.xml` for per-part media types
- [x] Manifest cross-check recorded as a diagnostic rather than thrown, with `mimetype`
      winning — a document whose two self-descriptions disagree is still readable
- [x] Per-entry encryption detection

### Format identification (`FormatIdentifier.cs`)

- [x] Content-based across all four container kinds; verified on all 17 corpus formats, with
      deliberately wrong extensions and with no file name at all
- [x] Encrypted OOXML recognised from its `EncryptedPackage` stream
- [x] Encryption reported through `FormatIdentification` without needing a password

## To do

### Encryption

The only remaining feature work in this library. One scheme at a time, read support only.
See `research/05-infrastructure.md` section C for the algorithms and key derivations.

- [ ] **Distinguish write-protection from encryption first.** Several formats carry a
      write-protect password that encrypts nothing; those documents read without a password
      and must not raise `PasswordRequiredException`. Getting this wrong makes perfectly
      readable documents look locked.
- [ ] OOXML standard (2007): AES-128, SHA-1 key derivation
- [ ] OOXML agile: parameters from `EncryptionInfo`, per-segment IVs
- [ ] ODF: PBKDF2 + Blowfish CFB (older) and AES-256 GCM (newer). Blowfish needs
      BouncyCastle; the BCL has none.
- [ ] Legacy XLS/DOC/PPT: RC4, RC4 CryptoAPI, XOR obfuscation
- [ ] The `VelvetSweatshop` default password for XLS — such files must open with no password
      supplied
- [ ] Decide whether decryption streams lazily or decrypts whole parts up front. Lazy is
      better for large files but complicates the `IPackagePart` contract.

### Hardening

- [ ] Run the readers against LibreOffice's own test corpus (`sw/qa/`, `sc/qa/`, `sd/qa/` in
      this repository — thousands of real files, many deliberately damaged). The cheapest
      available source of adversarial input, and it will find more than hand-built fixtures
      can.
- [ ] Fuzz `CompoundFile.Open` and the package readers. Invariant: never hang, never crash,
      never allocate unboundedly; `MalformedDocumentException` is a correct outcome.
- [ ] Benchmark: identification should stay well under a millisecond for a typical file,
      since callers use it to filter whole corpora.

### Open questions

- [ ] Is `SharpCompress` needed at all? The BCL's ZIP support has handled every corpus file
      so far, and the package is currently pinned but unreferenced. Drop it unless a real
      document turns up using a compression method the BCL lacks.
- [ ] Should `IPackage` expose a way to enumerate a storage's children, for the legacy
      formats' nested object pools? `Parts` currently flattens everything, which is right for
      OPC and ODF but loses structure that DOC/XLS/PPT embedded objects rely on.
