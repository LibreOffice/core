# Shared Cross-Application Infrastructure — Reference for a C# Reimplementation

This document catalogs the LibreOffice core infrastructure that every
application-level filter (Writer/Calc/Impress, DOC/XLS/PPT, DOCX/XLSX/PPTX,
ODF) sits on top of. Everything here is genuinely shared: the container
formats, the encryption layers, the rich-text engine used for every Calc
cell and every Draw/Impress shape, the generic attribute/item system that
underlies every object model, the text-encoding/locale machinery, and the
document-metadata property sets. A C# port should build these layers
*first*, since almost nothing else can be tested without them.

All paths are relative to the repository root
(`/home/user/libreoffice-core`) and citations use `path/file.cxx:line`.

---

## Table of contents

- A. OLE2 / Compound File Binary (CFB) container
- B. ZIP-based packages (ODF package + OOXML OPC)
- C. Encryption / password protection (ODF, OOXML, legacy binary)
- D. EditEngine (rich text engine shared by Calc/Draw/Impress)
- E. SfxItemSet / SfxPoolItem property system
- F. Text encodings & i18n (codepages, LCIDs, break iteration)
- G. Metadata (ODF meta.xml, OOXML docProps, OLE property sets)
- H. Recommended C# equivalents

---

## A. OLE2 / Compound File Binary (CFB) container

Location: `sot/source/sdstor/`. This is LibreOffice's own from-scratch
implementation of the Microsoft Compound File Binary Format (the same
format documented by Microsoft as `[MS-CFB]`). It is *not* a wrapper
around an external library — every structure below is hand-rolled, which
is good news for porting: the whole format is contained in a handful of
files.

Key files:

| File | Responsibility |
|---|---|
| `sot/source/sdstor/stgelem.cxx/hxx` | Binary layout of the file header (`StgHeader`) and directory entries (`StgEntry`) — the exact byte-for-byte struct definitions. |
| `sot/source/sdstor/stgcache.cxx/hxx` | Sector cache: `StgPage` (one sector), `StgCache` (LRU + dirty-page tracking, raw sector I/O). |
| `sot/source/sdstor/stgstrms.cxx/hxx` | `StgFAT` (FAT chain walking/allocation), `StgStrm`/`StgFATStrm`/`StgDataStrm`/`StgSmallStrm` (stream abstractions over sector chains, including the mini-stream/short-stream variant). |
| `sot/source/sdstor/stgdir.cxx/hxx` | `StgDirEntry`/`StgDirStrm`: the directory tree, wrapping `StgAvlNode` (see `stgavl.cxx/hxx`) — LibreOffice keeps the on-disk red/black tree info (`m_cFlags`) but manages it in memory as an **AVL tree**, not literally a red/black tree; semantics (sorted children per storage, case-insensitive uppercase name compare) are what must be preserved, not the specific self-balancing algorithm. |
| `sot/source/sdstor/stgio.cxx/hxx` | `StgIo`: ties header + FAT stream + mini-FAT + mini-stream + directory stream together; `SetupStreams()` is the top-level "open a CFB file" entry point; also contains a FAT consistency validator (`Validator`/`EasyFat`, debug-only). |
| `sot/source/sdstor/storage.cxx` | `SotStorage`/`SotStorageStream`: the public, ref-counted, UNO-friendly façade apps use. |
| `sot/source/sdstor/ucbstorage.cxx` | `UCBStorage`: alternate storage implementation via UCB content providers (used mostly for encrypted temp files / non-local access); not needed for a straightforward reimplementation. |
| `sot/source/sdstor/stgole.cxx/hxx` | OLE `\1CompObj`/`\1Ole` stream helpers for embedded-object metadata (class id, user type name, "Ole" stream flags). |

### A.1 On-disk structure overview

A CFB file is a flat array of fixed-size **sectors** (LibreOffice calls
them "pages"). Sector 0 conceptually starts *after* the 512-byte header
(the header itself occupies file offset 0..511 and is treated specially:
`StgCache::Page2Pos()` returns `nPage * nPageSize + nPageSize`, i.e. it
reserves exactly one page-size worth of header space regardless of the
configured sector size — `sot/source/sdstor/stgcache.cxx:424`). Almost
all real-world files use 512-byte sectors (`nPageSize == 9` i.e. `1<<9`);
LibreOffice's header `Check()` in fact **requires** `m_nPageSize == 9`
(`sot/source/sdstor/stgelem.cxx:198`), i.e. it only accepts 512-byte
major sectors on read (4096-byte-sector CFB files, version 4, are
rejected by this exact check — a C# port that wants broader compatibility
should relax this and honor the real header value).

### A.2 Header layout (first 512 bytes), from `StgHeader` (`stgelem.hxx:34`)

All multi-byte integers are little-endian.

| Offset (hex) | Size | Field | Notes |
|---|---|---|---|
| `0x00` | 8 | Signature | Fixed magic: `D0 CF 11 E0 A1 B1 1A E1` (`cStgSignature`, `stgelem.cxx:33`). |
| `0x08` | 16 | `ClsId` | Root/whole-file CLSID (GUID), usually all-zero for user documents; set for some embedded objects. |
| `0x18` | 4 | Version | `0x0003003B` written by LO (`stgelem.cxx:92`); high 16 bits must be `3` on read (`Check()`, major version 3 = 512-byte sectors). |
| `0x1C` | 2 | Byte order mark | `0xFFFE` (little-endian indicator). |
| `0x1E` | 2 (signed) | `nPageSize` | Sector size exponent; sector size = `1 << nPageSize`. LO requires this to equal `9` (512 bytes) to accept the file. |
| `0x20` | 2 (signed) | `nDataPageSize` | Mini-sector ("short sector"/mini-stream sector) size exponent; LO default `6` → 64 bytes. |
| `0x22` | 1 | dirty flag | Internal only, not really part of the spec; not meaningful on read. |
| `0x23` | 9 | reserved | Zero. |
| `0x2C` | 4 | `nFATSize` | Total number of FAT sectors. |
| `0x30` | 4 | `nTOCstrm` | Starting sector of the directory stream. |
| `0x34` | 4 | reserved | — |
| `0x38` | 4 | `nThreshold` | Mini-stream cutoff size in bytes; LO default `4096`. Any stream whose size is `< nThreshold` is stored in the **mini-stream** instead of full sectors. |
| `0x3C` | 4 | `nDataFAT` (a.k.a. `nMiniFatStart`) | Starting sector of the mini-FAT. |
| `0x40` | 4 | `nDataFATSize` (a.k.a. `nMiniFatSize`) | Number of mini-FAT sectors. |
| `0x44` | 4 | `nMasterChain` (a.k.a. `nDIFATStart`) | Starting sector of the **DIFAT** (double-indirect FAT) chain, or `STG_EOF` if none. |
| `0x48` | 4 | `nMaster` (a.k.a. `nDIFATCount`) | Number of DIFAT sectors beyond the header's inline 109 entries. |
| `0x4C` | 109 × 4 = 436 | `nMasterFAT[109]` | First 109 FAT sector numbers, stored directly in the header (`cFATPagesInHeader = 109`, `stgelem.hxx:36`). |

Total header size: `0x4C + 436 = 512` bytes, matching the fixed first
sector. This matches `[MS-CFB]`'s header layout exactly (LibreOffice's
comments even preserve the original field semantics).

Special sentinel values used throughout (`stgelem.hxx:139`):
`STG_FREE = -1`, `STG_EOF = -2`, `STG_FAT = -3` (marks a sector that
itself holds FAT data), `STG_MASTER = -4` (marks a sector holding a
DIFAT/master-FAT page).

### A.3 FAT / DIFAT / mini-FAT chain resolution

- **FAT** ("File Allocation Table"): an array of `int32` next-pointers, one
  per sector in the file, stored in dedicated FAT sectors. Given a
  starting sector, walk `FAT[sector]` repeatedly until `STG_EOF`
  (`StgFAT::GetNextPage`, `stgstrms.cxx:71`). Each FAT sector holds
  `sectorSize/4` entries (128 entries for 512-byte sectors).
- **DIFAT** (double-indirect FAT): needed only when the FAT itself grows
  beyond the 109 sectors listed inline in the header. Each DIFAT sector
  holds `(sectorSize/4)-1` FAT-sector-number entries plus one trailing
  pointer to the next DIFAT sector (`STG_EOF` terminates); see
  `StgFATStrm::GetPage`/`SetPage` (`stgstrms.cxx:601`, `:679`) which
  implement exactly this two-level indirection.
- **Mini-FAT** ("short-sector FAT" / `nDataFAT` in the header): a second,
  independent FAT-like chain, but over 64-byte mini-sectors instead of
  512-byte sectors. It is itself stored as a *normal* stream allocated
  through the regular FAT (`StgDataStrm` with `m_pFat` pointing at the
  main FAT, `stgio.cxx:113`). The mini-stream's actual bytes are
  concatenated in the **root directory entry's own data stream**
  (`pRoot`'s `STG_DATA` leaf, `stgio.cxx:114–117`) — i.e. the mini-stream
  is one giant stream owned by the root entry, sliced into 64-byte
  mini-sectors addressed via the mini-FAT.
- **Mini-stream cutoff**: header field `nThreshold`, default and
  practically-always `4096` bytes. Any non-storage entry whose `m_nSize <
  nThreshold` is read/written through `StgSmallStrm` (mini-FAT + root's
  mini-stream); everything else goes through `StgDataStrm` (main FAT).
  See `StgDirEntry::OpenStream` decision logic in `sot/source/sdstor/stgdir.cxx`
  (dispatches on `nSize < rIo.m_aHdr.GetThreshold()`, mirrored in the
  validator at `stgio.cxx:317`).

### A.4 Directory entries (128 bytes each), from `StgEntry` (`stgelem.hxx:99`, layout in `Load`/`Store`, `stgelem.cxx:386`/`:462`)

Directory entries are packed 4 per 512-byte sector and, like the FAT,
form a stream (the "directory stream", `StgDirStrm`) starting at header
field `nTOCstrm`.

| Offset (hex) | Size | Field | Notes |
|---|---|---|---|
| `0x00` | 64 | Name | UTF-16LE, fixed 32 UTF-16 code units, NUL-padded. |
| `0x40` | 2 | `nNameLen` | Length **in bytes including the terminating NUL** (so `(nNameLen>>1)-1` UTF-16 code units of actual name); max legal name length enforced by LO is 31 characters (`nMaxLegalStr = 31`, `stgelem.cxx:31`). |
| `0x42` | 1 | `cType` | Entry type: `0` = empty/unused, `1` = storage (subfolder), `2` = stream, `5` = root storage (`StgEntryType`, `stgelem.hxx:85`). (The spec also defines `3`/`4` = user-defined LockBytes types, unused by LO.) |
| `0x43` | 1 | `cFlags` | Red/black tree color bit (0 or 1) in the on-disk directory tree; LibreOffice records but does not use this for its own in-memory AVL tree — a C# port targeting write-compatibility with MS Office should still compute/maintain proper red-black coloring, but a *reader* can ignore it entirely. |
| `0x44` | 4 | `nLeft` | Left sibling entry index (`-1` = none). |
| `0x48` | 4 | `nRight` | Right sibling entry index (`-1` = none). |
| `0x4C` | 4 | `nChild` | First child entry index for storages (`-1` = empty storage). |
| `0x50` | 16 | `ClsId` | CLSID/GUID — this is where per-storage OLE class identifiers live (embedded objects, and see §A.5 for whole-document CLSIDs). |
| `0x60` | 4 | `nFlags` | "State bits" (user flags), largely unused by LO on read. |
| `0x64` | 8 | `nMtime[2]` | Modify time, Windows `FILETIME` (two `int32`s). |
| `0x6C` | 8 | `nAtime[2]` | Create time, Windows `FILETIME`. |
| `0x74` | 4 | `nPage1` | Starting sector (main FAT) or mini-sector (mini-FAT) of this entry's data, depending on type/size. |
| `0x78` | 4 | `nSize` | Stream size in bytes (32-bit only — LO's implementation is limited to files where individual streams fit in a signed 32-bit int; `[MS-CFB]` technically defines a 64-bit size field for version-4 files, unused here since only version-3/512-byte files are accepted). |
| `0x7C` | 4 | `nUnknown` | Padding / reserved. |

Total: 128 bytes exactly (`STGENTRY_SIZE`, `stgelem.hxx:99`).

**Directory tree shape**: each storage's `nChild` points at exactly one
child entry, which is the root of a binary search tree (via `nLeft`/
`nRight`) over the *other* siblings in that storage, ordered by
`StgEntry::Compare()` (`stgelem.cxx:375`) — compare by name length first,
then by case-insensitive uppercased name (`ToUpperUnicode`,
`stgelem.cxx:308`, using an `en-US` `CharClass` for uppercasing — a C#
port should use ordinal-uppercase-then-compare on the UTF-16 name to
match this, not culture-aware comparison). A reader can ignore the
sibling tree shape entirely and simply flatten it (in-order traversal via
`StgAvlIterator`, `sot/source/sdstor/stgavl.cxx`) to get "all entries in
this storage".

### A.5 CLSIDs identifying Word/Excel/PowerPoint documents

These are the exact Microsoft-assigned class IDs LibreOffice matches
against (`include/filter/msfilter/classids.hxx:1`):

| Constant | GUID | Meaning |
|---|---|---|
| `MSO_WW8_CLASSID` | `00020906-0000-0000-C000-000000000046` | Word 97-2003 binary document (also used generically for "is this a Word OLE object") |
| `MSO_EXCEL5_CLASSID` | `00020810-0000-0000-C000-000000000046` | Excel 5.0 binary workbook |
| `MSO_EXCEL8_CLASSID` | `00020820-0000-0000-C000-000000000046` | Excel 97-2003 binary workbook |
| `MSO_EXCEL8_CHART_CLASSID` | `00020821-0000-0000-C000-000000000046` | Excel 97-2003 chart |
| `MSO_PPT8_CLASSID` | `64818D10-4F9B-11CF-86EA-00AA00B929E8` | PowerPoint 97-2003 presentation |
| `MSO_PPT8_SLIDE_CLASSID` | `64818D11-4F9B-11CF-86EA-00AA00B929E8` | PowerPoint 97-2003 single slide (OLE embedding) |
| `MSO_EQUATION2_CLASSID` | `00021700-0000-0000-C000-000000000046` | Equation Editor 2.x object |
| `MSO_EQUATION3_CLASSID` | `0002CE02-0000-0000-C000-000000000046` | Equation Editor 3.x (MathType) object |

**Important**: in practice LibreOffice (and MS Office itself) identifies
the *type* of a standalone `.doc`/`.xls`/`.ppt` file primarily by
**well-known stream names inside the root storage**, not by the root
entry's CLSID (which is frequently zero for top-level documents):
`WordDocument` (`sw/source/filter/basflt/iodetect.cxx:66`,
`sfx2/source/doc/docfilt.cxx:124`), `Workbook` (BIFF8) or `Book`
(BIFF3/4/5) for Excel (`sfx2/source/doc/docfilt.cxx:131,135`), and
`PowerPoint Document` for PowerPoint
(`sd/source/ui/unoidl/sddetect.cxx:82`). The CLSIDs above matter mainly
for **embedded OLE objects** (an embedded Word table inside a PowerPoint
slide, etc.), where the root entry's CLSID plus a `\1CompObj` stream
identify the embedded application (`filter/source/msfilter/msdffimp.cxx:7002-7098`
dispatches on exactly these GUIDs when importing OLE objects from
Escher/DFF streams).

LibreOffice's *own* native storage-format CLSIDs (relevant only if
targeting old StarOffice binary compatibility, not MS formats) live in
`include/comphelper/classids.hxx` as `SO3_SW_CLASSID`, `SO3_SC_CLASSID`,
`SO3_SIMPRESS_CLASSID`, `SO3_SDRAW_CLASSID`, `SO3_SCH_CLASSID`,
`SO3_SM_CLASSID`, each versioned (`_30`/`_40`/`_50`/`_60`) plus separate
"OLE embed" variants (`_OLE_EMBED_CLASSID_60`/`_8`) used specifically
when such a document is embedded inside an MS-format container — these
are irrelevant to reading MS Office documents but are needed if the port
ever needs to write ODF-native OLE embeddings.

### A.6 Byte order

Everything is little-endian (matching the `0xFFFE` byte-order mark in
the header); LibreOffice explicitly byte-swaps on big-endian hosts
(`OSL_BIGENDIAN` branches in `stgcache.hxx:117-127`), which a C# port can
simply ignore (all supported .NET runtimes are little-endian on the
platforms MS Office files matter for, and `BinaryReader`/`Span` binary
APIs are already little-endian by convention).

---

## B. ZIP-based packages (ODF package + OOXML OPC)

Location: `package/source/`. LibreOffice ships its own ZIP reader/writer
(`package/source/zipapi/`) rather than depending on an external ZIP
library, plus two layers built on top of it: `zippackage/` implements the
UNO `XPackage` storage API (used for both ODF and, in "PACKAGE"/"ZIP"
mode, ad-hoc zip access) and `xstor/` implements `com.sun.star.embed.XStorage`
(the higher-level storage abstraction Writer/Calc/Impress actually talk
to, which can be backed by either a ZIP package or an OLE storage
transparently).

Key files:

| File | Responsibility |
|---|---|
| `package/source/zipapi/ZipFile.cxx` | Central-directory ZIP reader, per-entry decryption (see §C), CRC/digest verification, recovery mode for corrupt zips. |
| `package/source/zipapi/ZipOutputStream.cxx`, `ZipOutputEntry.cxx` | ZIP writer (local file headers, data descriptors, central directory). |
| `package/source/zipapi/Deflater.cxx`, `InflateZlib.cxx`, `InflaterBytesZlib.cxx`, `InflaterBytesZstd.cxx` | Compression backends: zlib for normal DEFLATE, and a zstd-backed inflater (`InflaterBytesZstd.cxx`) used for a LibreOffice-specific optional stronger-compression variant of some internal streams — not relevant to interoperability with MS Office or standard ODF, safe to ignore for a reader. |
| `package/source/zippackage/ZipPackage.cxx` | Top-level `XPackage`: decides the package "flavor" (`m_nFormat` — ODF vs OOXML/plain-ZIP vs "OFOPXMLFormat") and writes the format-specific bookkeeping files (`mimetype`, `META-INF/manifest.xml`, or `[Content_Types].xml`). |
| `package/source/zippackage/ZipPackageFolder.cxx`, `ZipPackageStream.cxx`, `ZipPackageEntry.cxx` | In-memory tree mirroring the ZIP's folder/file structure, each entry carrying media-type + (if encrypted) `EncryptionData`. |
| `package/source/manifest/ManifestImport.cxx`, `ManifestExport.cxx`, `ManifestReader.cxx`, `ManifestWriter.cxx` | ODF `META-INF/manifest.xml` parse/serialize, including `manifest:encryption-data`. |
| `package/source/xstor/xstorage.cxx` | `XStorage` implementation gluing OLE (`SotStorage`) or ZIP (`ZipPackage`) backends behind one API used by document code; also implements transactional commit semantics (temp-stream-then-swap). |
| `oox/source/helper/zipstorage.cxx` | Thin `oox::StorageBase` adapter wrapping a `ZipFile`/`Package`, used by the OOXML import filters. |
| `comphelper/source/misc/storagehelper.cxx` | Format-detection helpers (`GetStorageFormatFromMediaType`, `CreatePackageEncryptionData`, etc.) and the SHA-256/SHA-1 ODF "start key" derivation (see §C.1). |

### B.1 ODF package rules (verified against `ZipPackage.cxx`)

- **`mimetype` is the first entry, stored uncompressed (`STORED`, not
  `DEFLATED`)**, with CRC32 precomputed
  (`ZipPackage::WriteMimetypeMagicFile`, `zippackage/ZipPackage.cxx:1125-1161`).
  This lets OS-level "file" detection read the media type from the first
  ~30-40 bytes of the ZIP without inflating anything, and it is what
  distinguishes an ODF zip from a generic zip/xlsx/docx (an OOXML file's
  first entry is `[Content_Types].xml`, DEFLATED).
- `META-INF/manifest.xml` — DEFLATED, written after all content parts
  (`ZipPackage::WriteManifest`, `:1163-1189`); **never encrypted** even
  when the document itself is password-protected (comment at
  `ZipPackage.cxx:1182`: "the manifest.xml is never encrypted").
- Content parts: `content.xml`, `styles.xml`, `meta.xml`, `settings.xml`
  at the package root; embedded objects/pictures under
  subfolder-per-object (`ObjectN/`, containing their own nested
  `content.xml` etc., recursively an ODF-shaped package) and
  `Pictures/imageNNN.ext`; `Thumbnails/thumbnail.png` for the preview
  image. `META-INF` is a reserved folder name enforced on import
  (`ZipPackage.cxx:215`, `:555`).
- Each manifest `<manifest:file-entry>` records `full-path`,
  `media-type`, and (ODF ≥1.2) optionally `manifest:version` per part —
  needed because different embedded sub-documents can be different ODF
  versions.
- Package-wide "wholesome" encryption (used for OOXML mode when storing
  an *encrypted OOXML package inside an ODF-shaped container*, i.e. the
  `encrypted-package` special path) is a distinct, less common mode; see
  §C.1.

### B.2 OOXML OPC (Open Packaging Conventions) rules

Implemented largely in `oox/source/core/` rather than `package/source/`
(the `package/` ZIP engine is reused, but part/relationship semantics are
OOXML-specific and live in `oox`):

- **`[Content_Types].xml`** at the package root declares `<Default
  Extension="..." ContentType="..."/>` (extension-based fallback, e.g.
  `xml`, `rels`, `png`) and `<Override PartName="/..." ContentType="..."/>`
  (exact-part overrides, used for the well-known parts like
  `/word/document.xml`). LO's writer seeds a baseline set of `Default`
  entries (`fntdata`, `jpeg`, `png`, `rels`, …,
  `zippackage/ZipPackage.cxx:1192-1209`) then adds `Override`s per part.
- **`_rels/.rels`** at the root and a **`_rels/<partname>.rels`** file
  alongside every part that has outgoing relationships. LibreOffice's
  relationship graph is modeled by `oox::core::Relations`
  (`oox/source/core/relations.cxx`): a map from relationship id (`rId#`)
  to `{Type, Target, TargetMode(Internal/External)}`. Fragment-path
  resolution (`getFragmentPathFromRelation`, `relations.cxx:105-136`)
  correctly handles absolute targets (leading `/`), relative targets, and
  `../` segments relative to the *referencing* part's own folder.
- Relationship *type* URIs come in three flavors LO explicitly probes, in
  order: transitional
  (`http://schemas.openxmlformats.org/officeDocument/2006/relationships/…`),
  strict (`http://purl.oclc.org/ooxml/officeDocument/relationships/…`),
  and Microsoft-proprietary
  (`http://schemas.microsoft.com/office/2006/relationships/…`) — see
  `createOfficeDocRelationTypeTransitional/Strict`, `createMSOfficeRelationType`
  and `getFragmentPathFromFirstTypeFromOfficeDoc`
  (`oox/source/core/relations.cxx:46-165`). A reader must fall back
  through all three to be robust against Strict-OOXML-produced files and
  older Microsoft tooling.
- Part naming: no compression requirement (parts are ordinary DEFLATEd
  zip entries; no first-entry-uncompressed rule like ODF's `mimetype`).
  Encryption for the whole package, when present, replaces the *entire*
  ZIP with a single OLE/CFB container holding `EncryptionInfo` +
  `EncryptedPackage` streams — see §C.2.
- `oox/source/core/xmlfilterbase.cxx` is the top-level driver:
  `getFragmentPathFromFirstTypeFromOfficeDoc` (`:343`) resolves e.g. "the
  main document part" from the package-level `_rels/.rels`; content-types
  are read back in on `readRelationInfo`-adjacent code
  (`xmlfilterbase.cxx:1205-1207` opens `[Content_Types].xml`).
- `comphelper/source/misc/storagehelper.cxx` (`OStorageHelper`) contains
  the format-agnostic glue UNO code uses to pick ODF-vs-OOXML-vs-OLE
  storage implementations based on a `StorageFormat` string
  (`"PackageFormat"`, `"OFOPXMLFormat"`, `"ZipFormat"`).

---

## C. Encryption / password protection

Three essentially unrelated systems exist side by side, one per era of
Microsoft/OASIS format. All are readable without owning
Microsoft/vendor code — algorithms are standard AES/RC4/Blowfish with
publicly documented key-derivation.

### C.1 ODF encryption

Implementation: `package/source/zipapi/ZipFile.cxx` (per-entry cipher
selection + PBKDF2/Argon2), `comphelper/source/misc/storagehelper.cxx`
(start-key derivation), `package/source/manifest/ManifestExport.cxx` /
`ManifestImport.cxx` (manifest schema), constants in
`package/source/manifest/ManifestDefines.hxx` and
`package/inc/EncryptedDataHeader.hxx`.

**Algorithms supported** (`ZipFile::StaticGetCipher`, `ZipFile.cxx:151-232`):
- `BLOWFISH_CFB_8` — legacy default (ODF 1.0/1.1 era): Blowfish in 8-bit
  CFB mode, via `BlowfishCFB8CipherContext` (`package/source/zipapi/blowfishcontext.cxx`).
- `AES_CBC_W3C_PADDING` — AES-256-CBC (ODF ≥1.2 default), via NSS
  (`XNSSInitializer`/`XCipherContext`).
  ODF manifest algorithm URI: `http://www.w3.org/2001/04/xmlenc#aes256-cbc`
  (`AES256_URL`, `ManifestDefines.hxx:94`); 128/192-bit CBC URIs also
  defined (`AES128_URL`/`AES192_URL`) though 256-bit is what LO writes.
- `AES_GCM_W3C` — AES-256-GCM (newer ODF, provides integrity/authentication;
  no separate digest needed — see `ZipFile.cxx:743,770-772`, which skips
  the standalone digest check for GCM since GCM's tag already authenticates).
  URI: `http://www.w3.org/2009/xmlenc11#aes256-gcm` (`AESGCM256_URL`).

**Key derivation**:
1. **Start key**: derived from the user's password text, *not* the raw
   password bytes directly.
   - ODF ≥1.2 (current default): SHA-256 of the **UTF-8** encoded
     password (`OStorageHelper::CreatePackageEncryptionData`,
     `comphelper/source/misc/storagehelper.cxx:358-382`, property name
     `PACKAGE_ENCRYPTIONDATA_SHA256UTF8`).
   - Legacy StarOffice-era compatibility keys are also generated
     alongside: SHA-1 of UTF-8 password, and SHA-1 of the password
     re-encoded as `MS_1252` (Windows-1252) — because SO6-generation
     documents used that encoding — plus a third "correct SHA-1" variant
     to work around a historical SHA-1 padding bug in LibreOffice's own
     digest implementation (`m_bTryWrongSHA1` /
     `StarOfficeSHA1DigestContext` vs `CorrectSHA1DigestContext`,
     `ZipFile.cxx:136-146`) — i.e. **two different, both-wrong-by-a-bit
     SHA-1 implementations may be in the wild** and a reader trying old
     files may need to try both.
2. **Per-stream derived key**: `PBKDF2(startKey, salt, iterationCount) →
   derivedKey` via `rtl_digest_PBKDF2`
   (`ZipFile::StaticGetCipher`, `ZipFile.cxx:200-209`), OR, for newer ODF
   1.4+ options, **Argon2id** via `argon2id_ctx` with `(t_cost, m_cost,
   lanes)` parameters (`ZipFile.cxx:168-198`). Manifest attribute names:
   `manifest:key-derivation-name` = `PBKDF2` (URI
   `urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#pbkdf2`) or
   `argon2id` (URI
   `urn:oasis:names:tc:opendocument:xmlns:manifest:1.5#argon2id`, or the
   LO-experimental `loext:` variant while still in draft,
   `ManifestDefines.hxx:99-103`).
3. Each stream stores its own `salt`, `iv`, `iteration-count` (or Argon2
   params), `checksum` (`SHA1/1K` or `SHA256/1K` — i.e. hash of the first
   1024 bytes of the *decrypted* plaintext, used as a lightweight
   password-check before fully decoding — attribute values
   `urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k` /
   `#sha256-1k`, `ManifestDefines.hxx:86-88`) inside its
   `<manifest:encryption-data>` element (written per-file-entry in
   `ManifestExport::ManifestExport`, `package/source/manifest/ManifestExport.cxx:41-460`).
4. Additionally, and *independent* of the manifest.xml encryption-data,
   every encrypted stream's ZIP payload is prefixed with a **fixed
   50-byte-plus binary header** (`package/inc/EncryptedDataHeader.hxx:24-49`,
   magic `0x05024d4d` = `"MM\x02\x05"`, version currently `2`) laid out as:

   ```
   offset  size  field
   0       4     header signature ("MM\002\005" = 0x05024d4dL)
   4       2     version (currently 2)
   6       4     PBKDF2 iteration count
   10      4     Argon2 t_cost
   14      4     Argon2 m_cost
   18      4     Argon2 lanes
   22      4     plaintext size
   26      4     encryption algorithm id (xml::crypto::CipherID)
   30      4     digest/checksum algorithm id (xml::crypto::DigestID, 0 if GCM)
   34      4     derived key size (bytes)
   38      4     start-key generation algorithm id
   42      2     salt length
   44      2     IV length
   46      2     digest length
   48      2     media-type string length (UTF-16 byte length)
   50      X     salt bytes
   50+X    Y     IV bytes
   ...     Z     digest bytes
   ...     W     media-type UTF-16LE bytes
   ```
   (assembled field-by-field in `ZipFile::StaticFillHeader`,
   `package/source/zipapi/ZipFile.cxx:234-345`). This inline header is
   what a reader actually parses from the raw ZIP entry bytes *before*
   the AES/Blowfish ciphertext — the manifest.xml's
   `<manifest:encryption-data>` element is largely a redundant/mirrored
   copy of the same salt/IV/iteration-count/algorithm values, present so
   tools can inspect encryption parameters without decompressing entries.
5. **Whole-package ("wholesome") encryption**: an alternative mode where
   the entire OOXML (or any) package is encrypted as one blob named
   `encrypted-package` inside an otherwise-normal ODF-shaped zip/manifest,
   with a `<manifest:key-info>`/`<manifest:encrypted-key>` structure
   holding one PGP-wrapped key per recipient (`gpg4libre`,
   `ManifestExport.cxx:176-290`) — this is LibreOffice's GPG/OpenPGP
   public-key encryption feature layered on top of the ODF container;
   safe to skip unless GPG-encrypted ODF support is a goal.

### C.2 OOXML encryption ([MS-OFFCRYPTO])

Implementation: `oox/source/crypto/` — `AgileEngine.cxx` (current/"Agile"
scheme, the default since Office 2010), `Standard2007Engine.cxx`
("Standard"/ECMA-376 scheme, Office 2007), `StrongEncryptionDataSpace.cxx`
(the "strong encryption" data-space dispatcher for the ODF-inside-CFB
Excel-XML predecessor and older RC4-CryptoAPI paths),
`DocumentDecryption.cxx`/`DocumentEncryption.cxx` (top-level driver that
inspects `\006DataSpaces/DataSpaceMap` to decide which engine to
instantiate).

Container: an encrypted OOXML file is itself an **OLE/CFB file** (§A)
containing (per `[MS-OFFCRYPTO]`):
- `EncryptionInfo` stream — algorithm parameters (parsed by
  `readEncryptionInfo`).
- `EncryptedPackage` stream — the AES/RC4-encrypted bytes of the *actual*
  OOXML zip (`DocumentDecryption::decrypt`, `oox/source/crypto/DocumentDecryption.cxx:193-219`,
  opens `EncryptedPackage` and writes the decrypted zip bytes out).
- Optionally `\006DataSpaces/DataSpaceMap` + related streams describing
  which "data space" handler applies (`readEncryptionInfo`,
  `DocumentDecryption.cxx:95-183`); LO falls back to assuming
  `"StrongEncryptionDataSpace"` if this is missing (some LO-written files
  omit it — `DocumentDecryption.cxx:164-168`).

**Standard (2007) encryption** (`Standard2007Engine.cxx`):
- Always AES-128-ECB in LO's implementation (`generateVerifier`,
  `calculateEncryptionKey`) though the on-disk header
  (`EncryptionStandardHeader`, `include/filter/msfilter/mscodec.hxx:446`)
  supports RC4/AES128/192/256 in general per spec.
- Key derivation: `SHA1(salt ++ password_as_UTF16LE)`, then **50,000
  rounds** of `SHA1(iterationIndex_LE32 ++ previousHash)`
  (`Standard2007Engine::calculateEncryptionKey`, `Standard2007Engine.cxx:71-117`),
  then `hash = SHA1(finalHash XOR-padded-into-64-byte-0x36-buffer)`
  ("hash of the ipad-like buffer" — this is effectively HMAC-flavored key
  stretching but not literally HMAC), then the encryption key is the
  first N bytes of that hash.
- Verifier check: decrypt the stored 16-byte encrypted verifier and its
  encrypted SHA-1 hash with AES-128-ECB and compare
  (`generateEncryptionKey`, `:119-159`).
- Bulk decryption: 4096-byte blocks, AES-128-ECB, no per-block IV
  (`Standard2007Engine::decrypt`, `:161-183`) — note ECB mode (no
  chaining) is what the *2007 standard scheme* actually uses.

**Agile encryption** (Office 2010+, `AgileEngine.cxx`) — parameters come
from an inline XML blob (not a fixed C struct) inside `EncryptionInfo`,
parsed with a minimal hand-rolled fast-parser
(`AgileDocumentHandler`/`AgileTokenHandler`, `AgileEngine.cxx:58-192`):
- Supported combinations, all AES-CBC (`AgileEngine::readEncryptionInfo`,
  `AgileEngine.cxx:519-613`): AES-128 + SHA1, AES-128 + SHA384,
  AES-192 + SHA384, **AES-256 + SHA512 (the default LO writes,**
  `setupEncryption`, `AgileEngine.cxx:731-741`, 100,000 spin count).
- Password → key: `spinCount` rounds (typically 100,000) of
  `hash = H(salt ++ password)` then `H(iterationIndex_LE32 ++ hash)`
  repeated (`comphelper::DocPasswordHelper::GetOoxHashAsVector`,
  `comphelper/source/misc/docpasswordhelper.cxx:367-389`), i.e. the same
  "prepend the counter" style as Standard2007 but with a
  configurable hash (SHA-1/256/384/512) and spin count, matching
  `[MS-OFFCRYPTO] 2.3.4.11` `ECMA-376 Agile` `hashPassword`.
- Distinct sub-keys are derived from that password hash by hashing it
  together with fixed 8-byte "block" constants and truncating/XOR-padding
  to the cipher's block size (`AgileEngine::calculateBlock`/`encryptBlock`,
  `:264-307`, and IV derivation `calculateIV`, `:251-262`) — three
  constants for verifier-hash-input, verifier-hash-value, and the actual
  encryption key (`constBlock1/2/3`, `:196-198`), and two more for the
  HMAC integrity key/value (`constBlockHmac1/2`, `:199-200`). This
  "block constant" scheme is exactly `[MS-OFFCRYPTO]`'s Agile key
  derivation and must be reproduced byte-for-byte.
- Bulk decryption: AES-CBC, 4096-byte segments, **each segment has its
  own IV** = `H(keyDataSalt ++ segmentIndex_LE32)` truncated to key size
  (`AgileEngine::decrypt`, `:452-517`) — unlike Standard2007's ECB, Agile
  changes the IV every 4KB segment.
- Integrity: an HMAC-like construction (`hmacKey`/`hmacHash`, computed via
  `comphelper::CryptoHash` over ciphertext, `:454,506,514,888,892`) allows
  the whole decrypted stream to be validated
  (`checkDataIntegrity`, `AgileEngine.cxx:444-450`).

**"VelvetSweatshop" default password**: Excel's "workbook protection" /
worksheet-protection-with-password feature (distinct from real
document-open encryption, but implemented via the *same* OOXML/CFB
encryption machinery) uses a hardcoded, well-known password,
`"VelvetSweatshop"`, that LO tries automatically before prompting the
user (`oox/source/core/filterdetect.cxx:423-438`). A reader can simply
always attempt this password first, silently, on any encrypted `.xlsx`.

**Write-protection vs encryption**: important distinction — MS Office
"mark as final" / "protect document" (editing restrictions) and Excel's
worksheet/workbook "protect sheet" (with or without password) are *not*
the same as document-open encryption. Simple write-protection is just a
flag (and, for worksheet protection, a weak legacy password hash — see
below) stored in plaintext inside the (unencrypted) file; VelvetSweatshop
workbook-protection-with-password is the one case that actually invokes
real OOXML encryption with a fixed password. Do not conflate "protected"
with "encrypted" when reading files — most "protected" documents need no
decryption at all.

### C.3 Legacy binary formats (DOC/XLS/PPT 97 and earlier)

Implementation: `filter/source/msfilter/mscodec.cxx` /
`include/filter/msfilter/mscodec.hxx`.

- **Pre-97 (Word/Excel ≤95) "XOR obfuscation"**: `MSCodec_Xor95` base
  class with `MSCodec_XorWord95`/`MSCodec_XorXLS95` subclasses
  (different bit-rotation distances, 7 vs 2). Key/hash derived from the
  password via a bit-rotating XOR scramble (`lclGetKey`/`lclGetHash`,
  `mscodec.cxx:73-115`) — this is trivially reversible (it's obfuscation,
  not real encryption, matching `[MS-DOC]`/`[MS-XLS]` "XOR Obfuscation").
- **97+ "Standard" RC4 (`MSCodec_Std97`)**: password + 16-byte random
  `DocId` → `GenerateStd97Key` (`comphelper/source/misc/docpasswordhelper.cxx:441-484`,
  same style key-mixing as the OOXML Standard2007 predecessor) produces a
  digest; `InitCipher(counter)` re-keys an RC4 stream cipher every block
  (512 bytes for Word, 1024 for Excel) by hashing
  `digest[0..5] ++ counter_LE32 ++ 0x80 ++ zero-padding ++ 0x48` with MD5
  (`MSCodec_Std97::InitCipher`, `mscodec.cxx:435-466`) — i.e. RC4 keyed
  from a rolling MD5 hash, **not** a single static RC4 key for the whole
  file (this per-block rekeying is essential to reproduce correctly or
  decryption silently produces garbage after the first block).
- **RC4 CryptoAPI (`MSCodec_CryptoAPI`)**: same outer RC4 structure but
  the per-block key is `SHA1(digest ++ counter_LE32)` truncated to
  128 bits AES-key-size-equivalent (despite the class name, it's
  documented in `[MS-OFFCRYPTO] 2.3.6` as "RC4 CryptoAPI Encryption", RC4
  with a CryptoAPI/CSP-flavored key schedule) — `MSCodec_CryptoAPI::InitCipher`,
  `mscodec.cxx:468-492`.
- Password verification for both 97+ schemes: `VerifyKey` decodes a
  stored 16-byte salt-derived digest and compares against a freshly
  computed one (`MSCodec97::VerifyKey`, `mscodec.cxx:393-422`).
- On-disk header structs mirror `[MS-OFFCRYPTO]` almost verbatim:
  `EncryptionStandardHeader`, `EncryptionVerifierAES`/`RC4`,
  `StandardEncryptionInfo`/`RC4EncryptionInfo`
  (`mscodec.hxx:446-492`) — version markers `VERSION_INFO_1997_FORMAT =
  0x00010001`, `VERSION_INFO_2007_FORMAT = 0x00020003`,
  `VERSION_INFO_2007_FORMAT_SP2 = 0x00020004`, `VERSION_INFO_AGILE =
  0x00040004` (`mscodec.hxx:431-439`) are the discriminator a reader
  checks first (at the start of the `EncryptionInfo`/`0x6DataSpaces`-style
  stream, or, for the pure-binary 97 format, at the start of the
  encryption header inside the document stream itself) to pick which of
  §C.2/§C.3's engines to use.

---

## D. EditEngine — the shared rich-text engine

Location: `editeng/`. `EditEngine` is not Writer-specific — it is the
text engine used for **every Calc cell's rich text**, **every
Draw/Impress shape's text**, form-control text, the formula bar, and
comment/annotation text. A C# port needs exactly one implementation of
this, shared across all "applications".

### D.1 Core classes

| Class | File | Role |
|---|---|---|
| `EditEngine` | `editeng/source/editeng/editeng.cxx` (public façade) | Owns document model + view(s); the public API apps call. |
| `ImpEditEngine` | `editeng/source/editeng/impedit*.cxx` (`impedit.hxx`, `impedit2/3/4/5.cxx`) | The actual implementation — formatting, layout, editing operations, undo. `impedit3.cxx` alone is ~5000 lines and holds `CreateLines`/`FormatParas`/`CreateTextPortions` (the layout engine). |
| `EditDoc` / `ContentNode` | `editeng/inc/ContentNode.hxx` | `ContentNode` = one paragraph: a `OUString` of raw text, a `ContentAttribs` (paragraph-level `SfxItemSet` + style pointer), and a `CharAttribList` (sorted list of `EditCharAttrib` character-range attributes). `EditDoc` (not shown above but the array-of-`ContentNode`) is the whole multi-paragraph document. |
| `CharAttribList` | `editeng/inc/ContentNode.hxx:61` | Vector of `unique_ptr<EditCharAttrib>`, each an (item, start, end) triple; supports range queries (`FindAttrib`, `FindAttribRightOpen`, `HasAttrib`) and range-optimization/merging (`OptimizeRanges`). |
| `ParaPortion` | `editeng/inc/ParagraphPortion.hxx:58` | Per-paragraph *layout* cache: `EditLineList` (the wrapped visual lines), `TextPortionList` (contiguous runs of identical formatting/kind within the paragraph), script-type and writing-direction position tables (`ScriptTypePosInfos`, `WritingDirectionInfos`), plus dirty/invalidation bookkeeping (`mnInvalidPosStart`, `IsInvalid()`/`SetValid()`) that drives incremental reformatting. |
| `EditLine` | `editeng/inc/EditLine.hxx:29` | One visual (wrapped) line: `[mnStart,mnEnd)` character range, `[mnStartPortion,mnEndPortion)` portion range, per-character X positions (`KernArray maPositions`), line height/ascent, hanging-punctuation flag. |
| `TextPortion` | `editeng/inc/TextPortion.hxx:92` | One formatting-homogeneous run: length, measured size, `PortionKind` (`TEXT`/`TAB`/`LINEBREAK`/`FIELD`/`HYPHENATOR`), RTL level, plus optional `ExtraPortionInfo` (CJK compression data, field line-break points) and `RubyPortionInfo`. |
| `EditTextObject` / `EditTextObjectImpl` | `editeng/source/editeng/editobj.cxx` (`editobj2.hxx`) | The serializable/clipboard/undo snapshot of edited rich text — an ODF-like "paragraphs + per-paragraph items + per-run character items" structure independent of any live `EditEngine` instance; this is what Calc stores per rich-text cell and what Draw/Impress store per shape when not actively being edited. |
| `SvxFont` | `editeng/source/items/svxfont.cxx` (used pervasively) | A concrete, resolved font (family, size, weight, posture, CJK/CTL variants, kerning, case-mapping, relief, etc.) computed by collapsing an `SfxItemSet` down to renderable font attributes — this is the bridge between the item system (§E) and an actual `vcl::Font`/text-shaping call. |

### D.2 Which-ID ranges (`include/editeng/eeitem.hxx`)

EditEngine registers its own contiguous which-ID range on top of
whatever range the host application's item pool already occupies
(`EE_ITEMS_START = OWN_ATTR_VALUE_END + 1`, `eeitem.hxx:75`):

- **Paragraph attributes** `EE_PARA_START..EE_PARA_END` (21 items):
  writing direction, XML-attribute grab-bag, hanging punctuation,
  forbidden-rules (kinsoku), CJK/CTL spacing, numbering/bullet state and
  glyph, hyphenation (+ two sub-flags for "no all-caps" / "no last word"),
  outline left/right-spacing and level, left/right spacing, top/bottom
  spacing, line spacing, paragraph adjustment (justify), tab stops,
  justify-method, vertical-justify, auto writing-direction.
- **Character attributes** `EE_CHAR_START..EE_CHAR_END` (36 items):
  color, font, font height, char-width-scale, weight, underline,
  strikeout, posture (italic), outline, shadow, escapement
  (super/subscript), pair-kerning, kerning, word-line-mode, three
  parallel language items (Western/CJK/CTL), three parallel font items,
  three parallel font-height items, two parallel weight items, two
  parallel posture items, emphasis mark, relief, XML-attribute grab-bag,
  overline, case-map, generic grab-bag, background color, ruby text,
  script-hint, optical sizing, font variations. **The CJK/CTL
  parallel-item pattern (three parallel items per Western property) is
  pervasive across LibreOffice's whole text-attribute model** — a C# port
  should design its font-property model with this three-way split from
  the start rather than bolting it on later.
- **Feature "items"** `EE_FEATURE_START..EE_FEATURE_END`: these are not
  really formatting attributes but in-text markers occupying one
  character position each — tab (`EE_FEATURE_TAB`), forced line break
  (`EE_FEATURE_LINEBR`), "not convertible" marker (`EE_FEATURE_NOTCONV`),
  and fields (`EE_FEATURE_FIELD`, carrying a `SvxFieldItem` — date, page
  number, URL, etc.). Each occupies exactly one `CH_FEATURE` placeholder
  character in the paragraph's text buffer.

### D.3 Formatting & painting pipeline

Driven by `ImpEditEngine::FormatParas()` (paragraph loop) →
`ImpEditEngine::CreateLines()` (`editeng/source/editeng/impedit3.cxx:653`)
per paragraph → `CreateTextPortions()` (`impedit3.cxx:2586`, builds the
`TextPortionList` by walking character attribute boundaries + script/CTL
runs) → line-breaking within `CreateLines` itself, which:

1. Special-cases empty paragraphs (`createLinesForEmptyParagraph`).
2. Chooses between a fast incremental "quick format" path
   (`bQuickFormat`, only valid when the edit was a simple insert/delete
   that didn't cross portion boundaries and involves no complex-script
   text, `impedit3.cxx:716-745`) and a full
   `CreateTextPortions`+re-line-break pass.
3. Walks portions left-to-right accumulating pixel width; when adding a
   portion would exceed the paragraph's available width, decides the
   break point using `css::i18n::XBreakIterator` (ICU-backed; see §F.3)
   for ordinary word-wrap, or — for fields wider than the line
   (`impedit3.cxx:1100-1180`) — walks *cell* boundaries via
   `nextCharacters(..., CharacterIteratorMode::SKIPCELL, ...)` to avoid
   breaking inside a grapheme cluster/surrogate pair.
4. Hyphenation, when enabled per-paragraph (`EE_PARA_HYPHENATE`) and the
   word doesn't fit, calls a pluggable `XHyphenator` service
   (`impedit3.cxx:2143-2195`) to find a language-specific hyphenation
   point — this is a distinct, swappable component from the break
   iterator (word/line breaking is mandatory and built-in; hyphenation is
   an optional linguistic add-on, typically backed by Hunspell-style
   dictionaries in real LO builds).
5. Records final per-character X offsets into `EditLine::maPositions`
   (a `KernArray`, effectively a `double[]` of per-glyph advance/position
   values) so painting later is a simple positioned-glyph-run blit with
   no re-shaping.

Painting is a separate, much simpler pass: `ImpEditEngine::Paint`/
`StripPortions`-family functions (see `StripPortionsHelper.cxx`) replay
the already-computed `ParaPortion`/`EditLine`/`TextPortion` structures
against a `vcl::OutputDevice`, resolving each portion's `SvxFont` via
`SeekCursor` and drawing text runs at the precomputed positions — no
layout decisions happen during painting, which is the key architectural
point to preserve: **layout (measurement/line-breaking) and paint
(drawing) are fully decoupled**, and Calc/Draw/Impress query the layout
results (line count, paragraph height, portion boxes) without needing to
paint at all (e.g. for autosizing a shape to its text).

### D.4 Item pool / `SfxItemSet` usage

Every `ContentNode` owns one `ContentAttribs`, which wraps one
`SfxItemSet` over the `EE_PARA_*`/`EE_CHAR_*` (+ host application's own)
which-ranges, with `GetStyleSheet()`/`SetStyleSheet()` establishing the
paragraph-style parent link. Character-run attributes are *not* stored as
item sets — they are individual `EditCharAttrib` items with explicit
`[start,end)` ranges layered on top of the paragraph's item set,
resolved at format/paint time by "what attribute is active at this
character position", falling back to the paragraph's `SfxItemSet` (which
itself falls back to its style-sheet parent, per §E) when no run-level
override exists. Understanding this two-tier model (item-set fallback
chain *plus* an orthogonal position-range override list) is essential —
naively modeling EditEngine attributes as "one `SfxItemSet` per
character" would be both wrong and far too slow; naively modeling them as
"one item set for the whole paragraph" would lose all per-character
formatting.

---

## E. SfxItemSet / SfxPoolItem property system

Location: `svl/source/items/` (implementation), `include/svl/`
(interfaces: `poolitem.hxx`, `itemset.hxx`, `itempool.hxx`,
`style.hxx`). This is the single generic attribute/property mechanism
underlying *every* formatting model in LibreOffice — paragraph/character
attributes (§D), paragraph/cell/graphic-object styles, page styles, and
huge swaths of Draw/Impress shape properties. A C# port does not need to
reproduce the memory-pooling optimizations, but must reproduce the
*resolution semantics* exactly, because document-format import/export
code everywhere assumes them.

### E.1 Which-IDs and ranges

Every distinct property has a stable, small-integer "which-ID". An
`SfxItemPool` owns a contiguous `[mnStart, mnEnd]` which-ID range
(`include/svl/itempool.hxx:165-166`) and, for each which-ID, a **pool
default item** (the value used when nothing overrides it — think
"factory default", e.g. "not bold", "black text", "no border"). An
`SfxItemSet` is constructed against one or more which-ID *ranges*
(`WhichRangesContainer`, possibly non-contiguous, e.g. "1000-1050 plus
5000-5030") and internally stores only the properties actually **set**,
as a sparse `std::unordered_map<sal_uInt16, const SfxPoolItem*>`
(`m_aPoolItemMap`, `include/svl/itemset.hxx:104`) — a which-ID being *in
range* but absent from that map is the "default" state, not an error.

### E.2 `SfxItemState`: the four-valued resolution result

`SfxItemState` (`include/svl/poolitem.hxx:78-105`) is the key concept a
port must reproduce precisely:

| Value | Meaning |
|---|---|
| `DEFAULT` (0x20) | Which-ID is within this set's registered ranges, but not explicitly stored here (or in any ancestor with an explicit value) — falls through to the pool's default item. |
| `SET` (0x40) | This exact set (or, when searching with `bSrchInParent`, some ancestor) has an explicit value stored for this which-ID. |
| `DONTCARE` / `INVALID` (0x10) | Used specifically when *merging* several item sets (e.g. "what's the bold-state of this multi-paragraph selection") and they disagree — meaning "over-defined, no single valid answer" (e.g., toolbar checkbox shows tri-state/indeterminate). Internally represented by a magic sentinel pointer `INVALID_POOL_ITEM` stored in the map rather than a real item (`svl/source/items/poolitem.cxx`, tested via `IsInvalidItem()`). |
| `DISABLED` (0x01) | The property is explicitly turned off/unavailable for this set (e.g., a UI-only "this control is disabled" state); represented by another sentinel, `DISABLED_POOL_ITEM`. |
| `UNKNOWN` (0) | Which-ID isn't even in this set's registered ranges — programming error / not-applicable, not a legitimate style-resolution outcome. |

### E.3 Parent chaining (style inheritance) — the core algorithm

Every `SfxItemSet` may have a `m_pParent` (another `SfxItemSet`, often
belonging to a *paragraph/character/cell style sheet*, itself possibly
chained to a parent style). Resolution walks up this chain **only while
the current level's answer is "not explicitly set here"**:

`GetItemState_ForWhichID` (`svl/source/items/itemset.cxx:497-524`):
1. If this set's own map has an entry for the which-ID: return
   immediately, using that entry's concrete state (`SET`, or `INVALID`/
   `DISABLED` if it's a sentinel) — **the search stops here regardless of
   parent**, i.e. a value explicitly set at a lower level always wins,
   full stop, no matter what any ancestor style says.
2. Else, if the which-ID is within this set's registered ranges, the
   *provisional* result is `DEFAULT`.
3. Only if `bSrchInParent` is true *and* a parent exists *and* the
   provisional result is `UNKNOWN` or `DEFAULT` (i.e., **not** already
   `SET`/`INVALID`/`DISABLED`), recurse into the parent with the
   provisional state carried forward, and return the parent's answer.
4. Otherwise return the provisional result as-is.

`Get()` (`svl/source/items/itemset.cxx:934-965`) implements value
retrieval with the same shape: own map hit (deref, or pool-default if the
hit is the `INVALID` sentinel) → else recurse into parent if present →
else fetch the pool's own default item for that which-ID
(`GetUserOrPoolDefaultItem`). **The critical invariant for a correct
style-inheritance port**: parent lookup happens only on a *miss* at the
current level; an explicit value anywhere below in the chain always
shadows everything above it, and only the *pool* (not any style) supplies
the ultimate fallback if the entire chain has nothing set.

This is exactly "CSS-style single-property inheritance with `!important`
semantics at each level" — but note it is **not** cascading in the CSS
sense of "more specific selector wins"; it is strictly "first non-empty
value found walking from the object outward to its style ancestors,
then to the pool default" — a simple linked-list-of-dictionaries lookup,
which is straightforward to reimplement as, e.g., a chain of
`Dictionary<int, object>` with a `TryGetValueWalkingParents` helper in
C#, without needing any of LibreOffice's reference-counted item pooling.

### E.4 Merging / comparing sets

`SfxItemSet::Put(const SfxItemSet&, bool bInvalidAsDefault)`,
`MergeValues`, `Intersect`, `Differentiate` implement set-algebra used
for multi-selection formatting queries (Writer/Calc/Impress all use this
to answer "what's the common formatting of this selection" and "apply
this one change to every paragraph in the selection"); a port only needs
these if it intends to support an *editing UI* rather than pure
read/import — for pure format-reading/conversion purposes, only §E.2/E.3
(state + inheritance resolution) matter.

### E.5 Style sheets

`svl/source/items/style.cxx` (`SfxStyleSheet`) wraps exactly one
`SfxItemSet` plus a "parent style name" — style inheritance *is*
`SfxItemSet` parent chaining, just with the parent `SfxItemSet` belonging
to another named `SfxStyleSheet` object rather than being an anonymous
ancestor. Multi-level style hierarchies (e.g. ODF's paragraph-style →
parent-style → ... chain, or DOCX's `basedOn`) are simply multiple linked
`SfxItemSet`s, each `SfxStyleSheet::GetItemSet()` chained via `SetParent`.

---

## F. Text encodings & i18n

### F.1 `rtl_TextEncoding` and legacy MS codepages

`rtl_TextEncoding` (defined in `sal/`, tables in `sal/textenc/tables.cxx`
and the numerous `convert*.cxx`/`.tab` single-byte and multi-byte
converter tables) is LibreOffice's central "which 8-bit/legacy charset is
this text in" enum, covering: all `RTL_TEXTENCODING_MS_12xx` Windows code
pages (874 Thai, 932 Shift-JIS, 936 GBK, 949 Wansung, 950 Big5, 1250
Central European, 1251 Cyrillic, 1252 Western/Latin-1-ish, 1253 Greek,
1254 Turkish, 1255 Hebrew, 1256 Arabic, 1257 Baltic, 1258 Vietnamese,
1361 Johab), all `RTL_TEXTENCODING_IBM_*` OEM/DOS code pages (437, 737,
775, 850, 852, 855, 857, 860-866, 869), Apple Roman, and of course
UTF-8/UTF-16/UTF-7/ISO-8859-*/etc. **These `MS_12xx`/`IBM_*` values are
exactly the ones that matter for legacy DOC/XLS/PPT** — everything else
in the encoding table is for other import/export filters (HTML, plain
text, RTF from non-Windows sources) and doesn't need duplicating for a
pure MS-format-focused port, though `MS_1252` in particular is also the
ODF-legacy-password encoding noted in §C.1.

### F.2 How DOC/XLS/PPT pick a codepage

- **XLS (BIFF)**: an explicit `CODEPAGE` record (opcode-driven,
  `ImportExcel::Codepage`, `sc/source/filter/excel/impop.cxx:631-634`)
  carries a 16-bit numeric Windows/IBM codepage identifier, translated to
  `rtl_TextEncoding` via a straightforward lookup table
  (`XclTools::GetTextEncoding`, `sc/source/filter/excel/xltools.cxx:365-430`):
  a flat array of `{codepage_number, rtl_TextEncoding}` pairs (437→IBM_437,
  ... 1200→`DONTKNOW`/Unicode-native for BIFF8, 1252→MS_1252, ...
  10000/32768→Apple Roman, 32769→MS_1252 for BIFF2/3). **Codepage 1200
  ("Unicode") signals that BIFF8's native UTF-16 string encoding is in
  effect and legacy single-byte reencoding should not be applied** — this
  is the single most important special case to get right, since BIFF8
  (Excel 97+) strings are natively UTF-16 with a per-string
  compressed/uncompressed flag byte, and the codepage only matters for
  *older* BIFF versions' 8-bit strings.
- **DOC (`[MS-DOC]`)**: the `FIB` (File Information Block) carries an
  `lid`/language-ID (LCID) field rather than a direct codepage number;
  the codepage is derived indirectly from the LCID via the same
  LCID→locale mapping used everywhere else (§F.3) plus, for older Word
  versions writing 8-bit ANSI text, `utl_getWinTextEncodingFromLangStr`
  (`unotools/source/misc/wincodepage.cxx:134-139`) which maps a
  BCP-47-ish language string to the Windows ANSI or OEM codepage that
  version of Windows would have used for that language (two parallel
  tables, ANSI `impl_getWinTextEncodingFromLangStrANSI` and OEM
  `impl_getWinTextEncodingFromLangStrOEM`, `wincodepage.cxx:26-130`,
  covering the same `MS_12xx`/`IBM_*` set as above, keyed by ISO language
  prefix e.g. `"ja"`→MS_932, `"ru"`→MS_1251, default fallback MS_1252
  ANSI / IBM_850 OEM).
- **PPT (`[MS-PPT]`)**: similarly LCID-driven for legacy text runs;
  modern PPT/PPTX text is UTF-16/UTF-8 and codepage detection is largely
  moot except for very old (PowerPoint 4/95) binary records.

### F.3 Language tags / LCIDs

`i18nlangtag/source/isolang/isolang.cxx` (1541 lines) is the canonical
LCID (Microsoft `LANGID`, e.g. `0x0409` = en-US) ↔ BCP-47 language tag
mapping table used for every MS format's language fields (DOC/XLS/PPT
`lid` fields, OOXML `w:lang`/`xml:lang` attributes, etc.). Structure:
several parallel tables (`IsoLanguageCountryEntry` — plain
language+country; `IsoLanguageScriptCountryEntry` — language+script+country
for tags needing an explicit script subtag like `sr-Latn-RS`;
`Bcp47CountryEntry` — entries that need special-cased BCP-47 strings)
plus an **override mechanism** (`mnOverride` field,
`isolang.cxx:41-58`): some LCIDs Microsoft assigned don't correspond to
the "canonical" BCP-47 tag LO wants to normalize to (documented example:
`LANGUAGE_ROMANIAN_MOLDOVA` was actually assigned by Microsoft to the
`ro-MD` LCID slot but should canonicalize differently; `LANGUAGE_TIBETAN_BHUTAN`
was reserved by MS for `bo-BT` but is used in practice for Dzongkha
`dz-BT`). A port needs this full table (it's large but static data, not
logic) to correctly round-trip language attributes in every MS format —
getting it only "mostly right" will silently corrupt spell-check-language
and locale-dependent-number-formatting metadata on round-trip.

### F.4 Break iterator / line breaking

`i18npool/source/breakiterator/breakiterator_unicode.cxx` wraps **ICU's
`icu::RuleBasedBreakIterator`** (`OOoRuleBasedBreakIterator` subclass,
`:67-72`) loaded from LibreOffice's own compiled rule data
(`udata_open("OpenOffice", "brk", ...)`, `:171`) for character, word,
sentence, and line-break iteration
(`icu::BreakIterator::createCharacterInstance/createWordInstance/
createSentenceInstance/createLineInstance`, `:264-278`) — i.e. **ICU is
load-bearing for line breaking**, not merely used as a convenience;
LibreOffice does not implement its own UAX #14 line-breaking algorithm
from scratch, it drives ICU's, only supplying custom `.brk` rule files
where Unicode's default line-breaking rules need locale-specific
overrides. `breakiterator_cjk.cxx` (`BreakIterator_CJK`) layers
CJK-specific logic *on top of* (not instead of) this — e.g. explicit
Hangul-range detection (`isHangul`, `:41-44`) and forbidden
begin/end-of-line character sets for East Asian kinsoku rules — for
scripts where pure UAX #14 defaults are insufficient.
`breakiteratorImpl.cxx` is the dispatcher choosing which underlying
implementation (`_unicode` generic vs `_cjk` vs script-specific) to
invoke per detected script run.

**Implication for a C# port**: a from-scratch reimplementation should
plan on an ICU binding (see §H) for line/word/sentence/character
breaking rather than hand-rolling UAX #14 — LibreOffice's own choice
confirms this is the pragматic path, and the custom `.brk` rule
overrides are a relatively small, documentable layer on top rather than
a replacement.

---

## G. Metadata

### G.1 ODF (`meta.xml`)

`sfx2/source/doc/SfxDocumentMetaData.cxx` (2346 lines) implements
`com.sun.star.document.XDocumentProperties` reading/writing ODF's
`meta.xml`. Standard elements handled include `meta:generator`,
`dc:title`, `dc:description` (comments/description), `dc:subject`,
`meta:keyword` (repeated), `meta:initial-creator`, `dc:creator` (last
modified-by), `meta:creation-date`, `dc:date` (modification date),
`meta:print-date`, `meta:printed-by`, `meta:template` (an `xlink:href`
XLink to the template used), `meta:editing-cycles`,
`meta:editing-duration` (an ISO-8601 duration), `meta:document-statistic`
(child attributes `meta:page-count`, `table-count`, `draw-count`,
`image-count`, `object-count`, `ole-object-count`, `paragraph-count`,
`word-count`, `character-count`, `row-count`, `frame-count`,
`sentence-count`, `syllable-count`, `non-whitespace-character-count`,
`cell-count` — see `SfxDocumentMetaData.cxx:363-377`), and
`meta:user-defined` (repeated, arbitrary name/typed-value custom
properties, `sMetaUserDefined`, `:445`).

### G.2 OOXML (`docProps/core.xml` + `docProps/app.xml`)

Handled in `oox/source/docprop/` (`docprophandler.cxx`,
`ooxmldocpropimport.cxx`): `core.xml` carries the Dublin-Core-ish
properties (`dc:title`, `dc:creator`, `cp:lastModifiedBy`,
`dcterms:created`/`dcterms:modified` (with `xsi:type="dcterms:W3CDTF"`),
`cp:revision`, `cp:category`, `cp:contentStatus`, `dc:language`) while
`app.xml` carries application-specific statistics (`Application`,
`AppVersion`, `Company`, `Manager`, page/word/character/paragraph/line
counts, `TitlesOfParts`/`HeadingPairs` for outline structure). These map
close to 1:1 onto the same `XDocumentProperties` UNO interface ODF uses,
which is why both filters share one in-memory document-properties model
in LibreOffice (`SfxDocumentMetaData` is format-agnostic; only the
XML-serialization layer differs per format) — a C# port should copy this
shape: one canonical in-memory metadata object, two independent
serializers.

### G.3 OLE property sets (`\005SummaryInformation` /
`\005DocumentSummaryInformation`) — legacy DOC/XLS/PPT

Implementation: `sfx2/source/doc/docinf.cxx` (the
`LoadOlePropertySet`/`SaveOlePropertySet` glue mapping to
`XDocumentProperties`) and `sfx2/source/doc/oleprops.cxx`/`.hxx` (the
actual binary property-set codec, `SfxOlePropertySet`/`SfxOleSection`).
These are two CFB streams (see §A) named literally `\005SummaryInformation`
and `\005DocumentSummaryInformation` (the leading `\005` — byte `0x05` —
is a Microsoft convention marking "special"/non-user-visible streams)
sitting alongside `WordDocument`/`Workbook`/`PowerPoint Document` in the
root storage.

Binary format (`SfxOlePropertySet::ImplLoad`, `oleprops.cxx:1159-1192`,
matching `[MS-OLEPS]`'s `PropertySetStream` structure):

```
offset  size  field
0       2     byte order (0xFFFE)
2       2     version
4       2     OS minor version
6       2     OS type (2 = Windows in LO's writer)
8       16    format CLSID (unused/zero in practice)
24      4     section count
28      20*N  array of {SectionFMTID(16 bytes GUID), sectionOffset(4 bytes)} pairs
...           one PropertySet per section, at the given offsets
```

Each section identified by its FMTID GUID: the "global"/summary-info
section (`SECTION_GLOBAL`) holds `PROPID_TITLE`, `PROPID_SUBJECT`,
`PROPID_AUTHOR`, `PROPID_KEYWORDS`, `PROPID_COMMENTS`, `PROPID_TEMPLATE`,
`PROPID_LASTAUTHOR`, `PROPID_REVNUMBER`, `PROPID_EDITTIME` (a `FILETIME`
*duration*, i.e. editing time encoded as an absolute-looking date
relative to the 1601-01-01 epoch — LO subtracts that epoch back out on
read, `docinf.cxx:113-136`), `PROPID_LASTPRINTED`, `PROPID_CREATED`,
`PROPID_LASTSAVED`; the "document summary info"/custom section
(`SECTION_CUSTOM`) holds arbitrary named user-defined properties (each
with a property ID, a name string, and a typed value — string, `int32`,
bool, `FILETIME`, etc.) plus a `SECTION_BUILTIN` section for
`PROPID_MANAGER`/`PROPID_CATEGORY`/`PROPID_COMPANY`. Each section's own
internal layout is itself a small `[MS-OLEPS]` `PropertySet` structure
(property-id → typed-value dictionary, string values length-prefixed and
encoded per that property set's codepage property, ID `0x1` /
`PROPID_CODEPAGE`, typically not shown above but present in
`oleprops.cxx`). A from-scratch reader only needs: the two-level
section-dictionary structure above, the standard `[MS-OLEPS]` typed-value
tag encoding (`VT_LPSTR`, `VT_LPWSTR`, `VT_FILETIME`, `VT_I4`, `VT_BOOL`,
`VT_BLOB`, etc. — LO's variant handling is visible throughout
`oleprops.cxx`, e.g. its `VT_FILETIME`-instead-of-`VT_DATE` note at
`oleprops.cxx:838`), and the specific `PROPID_*` well-known IDs for the
summary/doc-summary sections (`[MS-OLEPS] 2.15`/`2.16`).

---

## H. Recommended C# equivalents

| Area | Recommendation | Reasoning |
|---|---|---|
| **OLE2/CFB container (§A)** | **OpenMcdf** (NuGet) as a starting point; expect to patch or partially hand-roll for edge cases. | OpenMcdf implements the same `[MS-CFB]` format LO hand-rolls here and covers header/FAT/DIFAT/mini-FAT/directory-tree correctly for the common case. Verify it handles: files with `nThreshold != 4096`, malformed/truncated FAT chains (LO's `Validator`/recovery-mode logic exists because real-world files are sometimes broken), and very old Word/Excel 95 files with unusual sector layouts. For a from-scratch stream-level reader, the tables in §A.2/A.4 are complete enough to hand-roll a minimal read-only CFB parser in well under 1000 lines if OpenMcdf proves inadequate for edge cases. |
| **ZIP / ODF package / OOXML OPC (§B)** | **`System.IO.Compression.ZipArchive`** (BCL) for the raw ZIP layer; **hand-roll** the ODF-manifest and OOXML-OPC semantic layers (part relationships, content-types, the uncompressed-`mimetype`-first-entry rule). `System.IO.Packaging` covers OPC relationships/content-types natively for OOXML but is Windows-only-flavored historically (now cross-platform in modern .NET) and does **not** understand ODF manifests at all. | The ZIP container itself is a solved problem in the BCL; SharpCompress is unnecessary extra weight unless zstd-compressed entries are required (LO's `InflaterBytesZstd` is a LO-internal optimization, not needed for MS-format or standard-ODF interop). `System.IO.Packaging` can be used *specifically* for the OOXML side (it literally implements OPC) but do not expect it to help at all with ODF; budget for a small hand-rolled ODF-manifest reader/writer (§B.1) regardless. |
| **Encryption — ODF (§C.1)** | **`System.Security.Cryptography`** (AES-CBC/GCM built in, PBKDF2 via `Rfc2898DeriveBytes`) + a small hand-rolled **Blowfish-CFB8** implementation (not in BCL) + **BouncyCastle** for Argon2id (not in BCL; `Org.BouncyCastle.Crypto.Generators.Argon2BytesGenerator`) if ODF 1.4+ Argon2-encrypted files must be supported. | AES/PBKDF2/SHA-1/256 are all native .NET; Blowfish is legacy/small enough to port directly from LO's usage pattern (only 8-bit CFB mode needed, no need for a general Blowfish library); Argon2 is the only piece requiring an external dependency. |
| **Encryption — OOXML (§C.2)** | **`System.Security.Cryptography`** entirely suffices: `Aes` (CBC and ECB — note ECB requires manually using `Aes.CreateEncryptor` with `Mode = CipherMode.ECB`, which .NET does support despite ECB being generally discouraged), `SHA1`/`SHA384`/`SHA512`, `Rfc2898DeriveBytes`-style manual iteration (the Agile/Standard2007 key stretching in §C.2 is a custom iterated-hash construction, not literally PBKDF2, so implement the loop by hand per the pseudocode given). | No external dependency needed; this is the easiest of the three encryption systems to port, being pure "documented algorithm + BCL primitives." |
| **Encryption — legacy binary (§C.3)** | **`System.Security.Cryptography.RC4`** does not exist in modern .NET (RC4 was removed as insecure) — hand-roll a ~20-line RC4 implementation (trivial, well-specified), or pull in **BouncyCastle**'s `RC4Engine`. MD5/SHA1 for the rekeying are native. | RC4 itself is so simple (a 256-byte S-box + swap loop) that a dependency is arguably not worth it; BouncyCastle is a reasonable alternative if already a dependency for Argon2/ODF. |
| **Rich text engine (§D, EditEngine)** | **Hand-roll.** No NuGet package replicates EditEngine's paragraph/portion/line model plus its specific incremental-reformatting and CJK/BiDi-aware layout semantics. | This is bespoke application logic, not a wrapped standard. Plan a C# `ContentNode`/`ParaPortion`/`EditLine`/`TextPortion` model directly mirroring §D.1, and rely on the recommendations below (HarfBuzzSharp + ICU) for the *shaping/measurement* primitives EditEngine's layout loop calls into, while keeping the paragraph/line/portion bookkeeping itself hand-written. |
| **Text shaping / font metrics (used by EditEngine layout)** | **HarfBuzzSharp** for complex-script shaping (Arabic/Indic/etc. glyph shaping, ligatures, kerning) + a hand-rolled OpenType table reader for font metrics, with **SkiaSharp** for rasterization. | LibreOffice itself uses HarfBuzz internally for shaping; matching that choice minimizes shaping-behavior divergence versus real Office/LO output. Metrics are read directly from the font tables rather than via a library, because matching LO's line heights requires raw `hhea`/`OS/2` access and LO's own precedence rules — see `06-rendering.md` section B. |
| **Item/property system (§E)** | **Hand-roll**, but this is *small*: a `WhichId`-keyed dictionary-based `ItemSet` class with a `Parent` reference and the exact `Get`/`GetItemState` fallback algorithm from §E.3 is maybe 100-200 lines of C#. Do **not** attempt to replicate LibreOffice's reference-counted item-pooling (`SfxItemPool` interning/refcounting) — that's a C++ allocation optimization irrelevant in a garbage-collected runtime; a plain `Dictionary<int, object>` per item-set plus normal GC is both simpler and sufficient. | The *semantics* (§E.2/E.3) are what document-format fidelity depends on; the memory-layout optimizations are pure C++-era performance engineering with no bearing on correctness and no natural analog needed in .NET. |
| **Text encodings (§F.1/F.2)** | **`System.Text.Encoding.CodePages`** (NuGet, from Microsoft) registers exactly the `Encoding.GetEncoding(1252)`-style Windows/IBM codepages needed (874, 932, 936, 949, 950, 1250-1258, 1361, 437, 850, 852, 855, 857, 860-866, 869, etc.) via ICU-backed or platform codepage tables — call `Encoding.GetEncoding(codepageNumber)` after registering the provider. | This is a solved, first-party-supported problem; do not hand-roll codepage tables. Reuse LO's `XclTools`-style `{BIFF codepage number → .NET codepage number}` map (§F.2, essentially identity for most entries) as a thin translation layer, since .NET's `Encoding.GetEncoding` already expects the raw Windows codepage numbers. |
| **LCID / language tags (§F.3)** | **`System.Globalization.CultureInfo`** covers modern LCID↔BCP-47 for the vast majority of cases (`CultureInfo(lcid)` and `.Name`/`.IetfLanguageTag`); however it will **not** reproduce LibreOffice's specific historical override table (the Moldova/Bhutan-style special cases in §F.3) or some legacy/deprecated Microsoft LCIDs `CultureInfo` no longer recognizes. Port LO's `isolang.cxx` override table (small, static data) as a supplementary lookup layered *in front of* `CultureInfo` for the handful of LCIDs where they'd disagree, falling back to `CultureInfo` for everything else. | `CultureInfo` handles the 95% common case for free; the LO override table is small enough (a few dozen special-cased entries) to port directly rather than trying to make `CultureInfo` match LO's specific historical choices everywhere. |
| **Break iteration / line breaking (§F.4)** | **ICU4N** (NuGet port of ICU to .NET) is the closest match to what LibreOffice itself uses — gives `RuleBasedBreakIterator` equivalents for character/word/sentence/line breaking directly. Alternative: .NET's built-in `System.Globalization.StringInfo`/`TextElementEnumerator` only covers grapheme-cluster (character) breaking, not word/line/sentence breaking, so is insufficient alone. | Given LibreOffice's own line-breaking is ICU-backed (§F.4), using ICU4N maximizes behavioral fidelity (identical UAX #14 line-break classes, identical default word/sentence boundaries) versus reimplementing Unicode's line-breaking algorithm from the UAX #14 spec by hand, which is a substantial, error-prone undertaking. Budget for porting LibreOffice's small custom `.brk`-rule locale overrides and the CJK-specific kinsoku logic (`breakiterator_cjk.cxx`) as a thin layer on top of ICU4N's defaults. |
| **Metadata / OLE property sets (§G.3)** | **Hand-roll** the `[MS-OLEPS]` `PropertySetStream` codec (small — a section-dictionary of typed values, §G.3's format table is essentially complete) on top of whichever CFB library is chosen for §A. For ODF `meta.xml`/OOXML `core.xml`+`app.xml`, plain **`System.Xml`** (`XmlReader`/`XDocument`) suffices — these are simple, well-documented flat XML schemas (§G.1/G.2) with no need for a specialized library. | `[MS-OLEPS]` is a small, fully-specified binary format (a few hundred lines to implement fully); no NuGet package specifically targets it, but it's not worth searching for one given the size. XML metadata parsing needs nothing beyond the standard library. |
| **PDF, images (out of scope here but adjacent)** | Not covered by this document's areas A-G, but noted for completeness per the requester's candidate list: **PdfPig** for PDF *reading*; **SkiaSharp** for raster image decode/encode. | Genuinely out of scope for the container/encryption/text-engine/item-system infrastructure this document covers. |

### H.1 Summary recommendation matrix

| Layer | Hand-roll | Off-the-shelf package |
|---|---|---|
| CFB container | Fallback plan / edge cases | OpenMcdf (primary) |
| ZIP container | — | `System.IO.Compression` (BCL) |
| ODF manifest / OOXML OPC semantics | **Yes** (both) | `System.IO.Packaging` (OOXML only, partial help) |
| ODF encryption (AES/Blowfish/PBKDF2) | Blowfish-CFB8 only | `System.Security.Cryptography` + BouncyCastle (Argon2) |
| OOXML encryption (Agile/Standard2007) | Key-stretching loop (small) | `System.Security.Cryptography` |
| Legacy binary encryption (RC4/XOR) | **Yes** (trivial) | BouncyCastle (optional) |
| EditEngine (rich text layout) | **Yes** (bespoke) | HarfBuzzSharp for shaping + a hand-rolled OpenType reader for metrics |
| Item/property system | **Yes** (small, ~100-200 LOC) | — |
| Codepages | — | `System.Text.Encoding.CodePages` |
| LCID/BCP-47 | Override table only (small) | `System.Globalization.CultureInfo` |
| Break iteration | Custom `.brk` overrides + CJK kinsoku | ICU4N |
| OLE property sets (SummaryInformation) | **Yes** (small, well-specified) | — |
| ODF/OOXML metadata XML | **Yes** (trivial with BCL XML) | `System.Xml` (BCL) |
