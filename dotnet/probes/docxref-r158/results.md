# Round 158 — a DOCX `REF` field draws its bookmark, not the result Word cached

Reference binary: `/opt/libreoffice26.2/program/soffice`, **LibreOffice 26.2.4.2**
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). Corpus `/home/user/sample-files`, 271 DOCX.
Every rendering under `SOURCE_DATE_EPOCH=0`.

## Where this came from

The user, looking at the published parity artefact, asked about a page pair in which *"one page
is completely empty and the other rendered by xray-office has content"*, and proposed that a page
break — or another of Word's break kinds — was being dropped.

**It is not a break.** The witness is `FAA 2025-26 Holdover Tables.docx` and the answer is
measured three ways:

- **The document's breaks are all honoured.** It states **72 `w:sectPr`** (all untyped, so all
  `nextPage`), **112 `w:br w:type="page"`**, and **zero** `evenPage`, `oddPage`, `continuous`,
  `nextColumn` or `w:pageBreakBefore`. So the classic "the reference emits a blank filler page"
  mechanism cannot apply to it at all.
- **The two renderings agree page for page up to the divergence.** Every near-empty page the
  reference emits — 8, 99, 101, 104, 106, 108, 110, 112, 114, 116, 118, 120 — this tree emits too,
  at the same index. The whole of the difference is **one** page: the reference has a near-empty
  page 127 and this tree does not, after which it is one page behind (ref 164/166 against ours
  163/165). 166 pages against 167.
- **On the page before it, the two agree to 0.18 pt for 104 of 105 matched lines and then part by
  10.35 pt at the last one.** Page 126 holds `TABLE ADJ-19` and its notes; every line down to
  y = 438.90 is within 0.18 pt, and the `CAUTIONS` heading below them sits at 535.75 in the
  reference and 525.40 here. That 10.35 pt is one line, and it is what lets the trailing caution
  bullet fit on our page 126 where the reference pushes it onto a page of its own.

The missing line is in note 11. The reference draws it over three lines and this tree over two:

```
ref  11 Heavy snow, ice pellets, … and hail (Table Adj-51 provides adjusted allowance times for
        Type IV EG fluids and Table Adj-52: Adjusted Allowance Times for SAE Type IV Propylene
        Glycol (PG) Fluids1,2 provides adjusted allowance times for Type IV PG fluids …).
ours    … and Table Adj-52 provides adjusted allowance times for Type IV PG fluids …).
```

`Adj-52` is a `REF _Ref199241802 \h` field. Its cached result is `Table Adj-52`; the bookmark of
that name covers the caption's **whole paragraph** — `Table Adj-` + the `SEQ` that numbers it +
`: Adjusted Allowance Times for SAE Type IV` + a `w:br` + `Propylene Glycol (PG) Fluids` + the
superscript `1` and `,2`. Writer recomputes the field from the bookmark on load, so it draws all
of that; we drew the cache.

The control that settles it beyond argument is on page 7 of the same document, where four `REF`
fields quote bookmarks covering the word `Table` as well as the number:

```
ref   The list of fluids (Tables Table 55, Table 56, Table 57 and Table 58) has been updated …
ours  The list of fluids (Tables 55, 56, 57 and 58) has been updated …
```

The reference's line is nonsense and it is what 26.2.4.2 draws. Both are reproduced exactly after
the fix.

## The rule

A `REF` whose first argument is not a `SET`/`SEQ` variable becomes a `SwGetRefField` with
`ReferenceFieldSource::BOOKMARK` and part `TEXT`
(`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:8541-8635) — the same reading
`FieldInstructions.ReferenceBookmark` already had for the RTF reader, which has expanded these
since round 88. `SwGetRefField::UpdateField` recomputes it; `SwGetRefFieldType::FindAnchor`
(`sw/source/core/fields/reffld.cxx`:1559-1591) gives the range, and its **−1** for two halves in
different nodes is read at `:603-607` as *to the end of the paragraph*. The text taken is the
node's **expanded** text, so a field inside the bookmark contributes its own value.

`FilterText` (`:461-489`) then drops a soft hyphen, turns U+2011 into `-`, and replaces every
character below U+0020 with a space — which is why a caption written over two lines comes back
as one.

## What 26.2.4.2 actually answers — `tests/corpus/features/words-reference-field.docx`

One arm per paragraph, each field caching a deliberately wrong result so that whatever the
reference draws is its own answer. `make-probe.py` builds it; `reference-arms.txt` is
26.2.4.2's own rendering of it.

| arm | the field quotes | 26.2.4.2 draws | this tree |
|---|---|---|---|
| A | a bookmark covering `Table 7`, cache `7` | `Table 7` | `Table 7` |
| B | a bookmark ending in the next paragraph | `half one` | `half one` |
| C | a collapsed bookmark | *(nothing)* | the cache — **declined, see below** |
| D | a collapsed `__RefHeading__5` | *(nothing)* | the cache — **declined** |
| E | a `w:br` inside the bookmark | `Line one Line two` | same |
| F | a `SEQ` inside the bookmark | `Figure 1` | same (from the cache — see below) |
| G | a soft and a non-breaking hyphen | `softhyphen and non-breaking` | same |
| H | a `w:tab` inside the bookmark | `before after` | same |
| I | a name no bookmark holds | `Error: Reference source not found` | the cache — **declined** |
| J | `REF … \p` | `below` | the cache — not computed |
| K | `PAGEREF` | `1` | the cache — not computed |
| L | the `w:fldSimple` spelling | `Table 7` | `Table 7` |

**The `#i81002#` cross-reference branch is unreachable from a DOCX, and that is measured rather
than assumed.** `make-crossref.py` puts a collapsed bookmark named `__RefHeading__1234_567890`,
`__RefNumPara__1234_567890`, `_Toc12345` and `_Ref999` behind four `REF` fields:
**all four draw nothing** (`reference-crossref.txt`). Writer promotes a name to a
`CrossRefBookmark` only where the caller asks for that mark type, and
`DomainMapper_Impl::StartOrEndBookmark` asks for an ordinary one. So the prefix rule the RTF path
needs is right there and wrong here, and `DocxReferenceFields` deliberately has no such branch.

### Three divergences kept on purpose

- **A collapsed bookmark, and a name no bookmark holds** (arms C, D, I). Both would *erase* what
  the file cached, and the lookup that missed is our own bookmark table. This reader's standing
  policy — *a wrong substitution is worse than a stale one* — applies. **Corpus reach nil:** of
  the **725** resolvable `REF` fields in the corpus's 271 DOCX, **0** name a collapsed bookmark.
- **A stale `SEQ` inside the bookmark** (arm F). The expansion is built from the walked text, so
  a `SEQ`'s **cached** value is what it contributes; the reference recomputes it. Right for a file
  Word last saved — the FAA caption's cached `52` is the number the reference computes too — and
  wrong for one whose caption numbers are stale. The fixture caches the current value so the arm
  tests the inclusion rather than the recomputation.
- **`\p`, `\r`, `\n`, `\w` and `PAGEREF`** (arms J, K), which ask for something other than the
  text. Unchanged from `FieldInstructions.ReferenceBookmark`'s existing reading.

### And a nested field's result is left alone

`Agile_Arc_SysDes.docx` holds a `REF` whose **whole cached result is a second `REF`**:

```
fldChar begin | REF _Ref450299582 | separate | fldChar begin | REF _Ref450299849 | separate
   | "Exhibit 2" | end | end
```

26.2.4.2 draws **`Exhibit 2Exhibit 1`** — it drops the cached *text* and keeps the nested
*field*, computing both and placing the outer field's value after the inner one's. This walker
cannot say where the inner value lands, so a name quoted that way is left to its cache: replacing
the pair with the outer value alone is a different wrong answer, not a better one. The first cut
of this round did substitute it and turned `Exhibit 2` into `Exhibit 1`, which is why the
`Quotation.Nested` flag exists. Reach: **1 field in 1 document.**

## The decision was reversed in round 161: the expansion ships switched OFF

**This project wants Word parity, and Word draws the cache.** A `REF` is not updated when a
document is opened or printed — only F9, or *update fields before printing*, changes what the
reader sees — so the disagreement below is about *when* a field is evaluated rather than about what
it evaluates to, and the reader's answer is their word processor's. `DocxReferenceFields` is gated
on `XRAY_OFFICE_LIBREOFFICE_QUIRKS`, off by default, and everything from here on describes the
switched-on behaviour.

What it costs is one gate row and is written down in `dotnet/TODO.word-parity.md`:
`FAA 2025-26 Holdover Tables.docx` prints **166 pages against the reference's 167**, deliberately.
Read the table below as *what the reference does*.

## Reach and confinement

`census.py` walks every corpus DOCX, resolves each `REF`'s bookmark against the document's own
text, and compares with the cached result (`census.txt`):

```
271 documents, 7 with a resolvable REF, 2 where a bookmark expansion differs from the cache
5  365  FAA 2025-26 Holdover Tables.docx
1    5  Agile_Arc_SysDes.docx
0  332  24-25_FAA_Holdover_Tables.docx      <- the sibling document, and it agrees throughout
0   13  ABCD-SDE-23-00 - Avionic System Description …
0    7  report-template.docx
0    2  ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx
0    1  ECSS-E-ST-50-16C-Annex-A(30September2021).docx
```

`confine.sh` renders all **271** DOCX with the binary under test and compares each byte for byte
against the bank this round started from (`confine-rows.tsv`):

```
270 same   1 MOVED   0 failures
MOVED  FAA 2025-26 Holdover Tables
```

`Agile_Arc_SysDes` is byte-identical once the nested case is declined, and the five documents
whose expansions match their caches are byte-identical because a name whose expansion equals
every quoting field's cache is never recorded at all.

**The witness:**

| | pages | alphanumeric | distance |
|---|---|---|---|
| 26.2.4.2 | 167 | 335 603 | — |
| before | 166 | 336 680 | 0.32 % |
| after | **167** | 336 847 | 0.37 % |

and **0 of its 167 pages** are outside the gate's `max(2 %, 15)` band, against a page-count
mismatch before. `words/pagination-001`'s row should move to `match`.

## What is left on this document, and it is not a break either

Our note markers on that page read **516, 517, … 522** where the reference reads **6 … 12**, and
the wider label pushes the first line's text from x 71.4 to 93.6. The document's notes are split
between paragraphs carrying `<w:numId w:val="54"/>` — one of **125 `w:num` entries over
`abstractNumId` 21, 122 of which carry `<w:startOverride w:val="1"/>`** — and paragraphs carrying
no `w:numPr` at all, which take the `ListNotes` style's `numId 118`, one of the three entries over
that abstract with **no** override. The reference counts all of them in one sequence, so a table's
notes run 1..12 and the next table's restart at 1; this tree gives 118 a document-wide counter of
its own and reaches 516. Seated as **O109**; it costs no page here, which is why no gate column
has ever shown it.
