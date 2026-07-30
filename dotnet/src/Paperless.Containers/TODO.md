# Paperless.Containers — TODO

The container layer: OLE2/CFB, ZIP, OPC, ODF packages, and decryption. Everything above
this is blocked on it, so it comes first.

Reference: `research/05-infrastructure.md` sections A, B and C — it has exact byte offsets.

## OLE2 / Compound File Binary

Unlocks DOC, XLS and PPT simultaneously, plus encrypted OOXML (which wraps its payload in
a CFB container).

- [ ] Header parse: signature `D0 CF 11 E0 A1 B1 1A E1`, sector size, FAT/DIFAT/mini-FAT
      locations, mini-stream cutoff (normally 4096)
- [ ] FAT and DIFAT chain walking, with cycle detection — a malformed file must not hang
- [ ] Directory as a red-black tree; tolerate an inconsistent tree by falling back to a
      linear scan of entries
- [ ] Mini-stream for entries below the cutoff
- [ ] Stream names beginning with U+0005 (the summary-information streams) — the character
      is part of the name
- [ ] Root CLSID exposed, but **not** relied on for format detection: LibreOffice detects by
      stream name (`WordDocument`, `Workbook`, `Book`, `PowerPoint Document`) precisely
      because real files often have wrong or missing CLSIDs. Do the same.
- [ ] Decide `OpenMcdf` vs hand-rolled. Judge on malformed input, not clean files: test
      against `sw/qa/`, `sc/qa/`, `sd/qa/` in this repository.

## ZIP

- [ ] `System.IO.Compression.ZipArchive` for the common case
- [ ] Tolerate the deviations real packages show: a wrong central directory, duplicate
      entry names, entries with no compression
- [ ] Guard against zip bombs — cap the uncompressed size and the entry count. These
      readers consume untrusted input.
- [ ] Path traversal: reject or normalise `..` in entry names before they reach the
      filesystem

## OPC (OOXML)

- [ ] `[Content_Types].xml`: defaults by extension, then per-part overrides
- [ ] `_rels/` relationship parts; resolve relative targets to normalised part names
- [ ] Locate the main document part by **following a relationship from the package root**,
      never by assuming a path
- [ ] Accept both the ECMA-376 1st-edition and ISO/IEC 29500 strict namespace URIs — real
      files use both, sometimes mixed within one package

## ODF

- [ ] `mimetype` as the first, uncompressed entry — the authoritative statement of what the
      document is
- [ ] `META-INF/manifest.xml`
- [ ] Cross-check `mimetype` against the manifest's root media type. LibreOffice throws on
      mismatch; Paperless records a `Diagnostic` and continues, so slightly malformed files
      still read.
- [ ] Flat XML (`.fodt`/`.fods`/`.fodp`) — no package at all, so it must present as one
      synthetic part for uniformity above this layer

## Encryption

One scheme at a time. Read support only.

- [ ] Detect encryption without a password, and report it via `FormatIdentification`
- [ ] OOXML standard (2007): AES-128, SHA-1 key derivation
- [ ] OOXML agile: from `EncryptionInfo`, per-segment IVs
- [ ] ODF: PBKDF2 + Blowfish CFB (older) and AES-256 GCM (newer). Blowfish needs
      BouncyCastle; the BCL has no Blowfish.
- [ ] Legacy XLS/DOC/PPT: RC4, RC4 CryptoAPI, XOR obfuscation
- [ ] The `VelvetSweatshop` default password for XLS — files encrypted with it must open
      with no password supplied
- [ ] **Distinguish write-protection from encryption.** Several formats carry a
      write-protect password that encrypts nothing; those documents read without a password
      and must not raise `PasswordRequiredException`.

## Open questions

- [ ] Is `SharpCompress` needed at all? The BCL handles ZIP. Drop the dependency if nothing
      requires it.
- [ ] Should decryption stream lazily or decrypt whole parts up front? Lazy is better for
      large files but complicates the `IPackagePart` contract.
