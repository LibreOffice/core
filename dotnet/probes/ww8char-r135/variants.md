# The six one-field variants of `361400CSLegislation1RF01PUBLIC1.doc`

Each is a byte-for-byte copy of
`/home/user/sample-files/words/done-010/doc/361400CSLegislation1RF01PUBLIC1.doc` with **one** field
of the file changed, converted with

```sh
/opt/libreoffice26.2/program/soffice -env:UserInstallation=file://$PROFILE \
    --headless --convert-to fodt --outdir out <variant>.doc
```

and read by looking for the automatic paragraph style of the `Table of Contents` paragraph (`P14`,
parent `Block_20_Text`) in the result. The variants are not committed: they are patched copies of a
corpus document, and the offsets below reproduce them in three lines of Python.

## Where the three offsets come from

The document's `1Table` stream holds `PlcfFldMom` at offset **13972**, length **490** — 81 field
characters, so 82 four-byte CPs then 81 two-byte `FLD` records. The record for cp **159**, the
`TOC` field's `U+0013`, is index 0, so its type byte is at table offset **14301** and reads 13.

`PlcfBteChpx` is at table offset 13792; the CHPX FKP for this region is page **162** of
`WordDocument`, whose `rgfc` entry number 11 is **2207** — the end of the run covering
fc 2189-2207 — and whose `rgb[10]` puts that run's CHPX at page offset 396, length 9,
`1668E3156C00 3E2A01`.

Mapped into the file on disk (the OLE sector mapping is not a constant offset, so each was located
by searching for a unique window rather than by arithmetic):

| what | offset in the `.doc` | value as authored |
|---|--:|---|
| the `TOC` field's `flt` | **113 117** | `13` |
| the `rgfc` entry ending the underlined run | **83 500** (4 bytes, little-endian) | `2207` |
| the `sprmCKul` inside that run's grpprl | **83 859** (3 bytes) | `3E 2A 01` |

The CHPX grpprl is found by searching for `09 1668E3156C00 3E2A01` — its length prefix included —
which occurs twice in the file and once inside page 162.

## The arms

| arm | patch | 26.2.4.2's `P14` |
|---|---|---|
| — | none | **no underline** |
| A | `[113117] = 9` — a field type with no handler in `aWW8FieldTab` | `style:text-underline-style="solid"` |
| B | `[83500..83504] = 2206` — the run closes at the paragraph mark instead | `style:text-underline-style="solid"` |
| C | `[83859..83862] = 42 2A 06` — `sprmCIco` 6, red, in place of `sprmCKul` 1 | **no `fo:color`** |
| D | arm C **and** arm A | `fo:color="#ff0000"` |
| E | `[113117] = 8` — `INDEX`, the other `Read_F_Tox` slot | **no underline** |
| F | `[113117] = 3` — `REF` | `style:text-underline-style="solid"` |
| G | `[113117] = 88` — `HYPERLINK` | `style:text-underline-style="solid"` |

## The two things the arms are controls for

**D is the control on C.** A colour sprm that appears nowhere is indistinguishable from a colour
sprm that was patched in wrongly; D is the same three bytes with the field retyped, and the colour
appears, so the patch is read.

**F and G are the control on A.** A is one byte away from the original in the same field as E, F
and G, so "the field type decides it" and "any change to that byte decides it" are separated by
the two arms that keep the underline.
