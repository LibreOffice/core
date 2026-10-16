# `rtf-bookmark-r88` — a `REF` field expands from a bookmark the RTF import misnames

## Environment

    ours    XRay.Office.Cli built in /home/user/wt-rtfbook, at 494ad1a8a (base) and at this round's
            HEAD. Every figure names which. The base binary was made by copying the changed source
            files aside and `git checkout`-ing them, with `XRay.Office.WordProcessing`'s and
            `XRay.Office.Cli`'s `obj`/`bin` deleted before each rebuild; it reproduces
            `24-25_FAA_Holdover_Tables` at 158 pages and the changed one at 221, so neither half is
            a stale build. The base column additionally reproduces round 87's own `after-rtf.tsv`
            **336 rows of 336, column for column**, which is the control on the instrument.
    ref     /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2
            0229ac93fcf0d7cbc6376066c6f35021cef002dc, version line read at the start of the round.
            Probe renderings were made fresh, under `timeout -k 30 240`, one `soffice` profile per
            probe family. For the column, the banked reference half /home/user/gate-odf-r80/ref
            (336 `__rtf.pdf`, rendered 2026-09-08 02:56–03:40). The diff is confined to
            `XRay.Office.WordProcessing`, which `soffice` cannot reach, so the banked reference is the
            same bytes throughout.
    corpus  /home/user/corpus-odf/words/**/rtf — 338 files, 26.2.4.2's own RTF export of the
            corpus's words track; 336 have a reference rendering and are scoreable.
            /home/user/sample-files/words — the 338 original `.doc`/`.docx`.
            /home/user/corpus-odf/words/**/odt — the 338 converted `.odt`.
    rule    batch-check.sh of 2026-09-05, re-read rather than quoted: page count, then
            **alphanumeric characters** (the ninth column) within `max(2%, 15)`, then unembedded
            fonts. `score.py` applies that rule to a pair of banks and is round 80's, unchanged, so
            this column is comparable with rounds 80, 85 and 87.

## What moved

### The column

| | base `494ad1a8a` | after | of |
|---|---:|---:|---:|
| gate `match` | **260** | **260** | 336 |
| page-exact | 280 | **280** | 336 |
| total \|Δ pages\| | 296 | **232** | |
| total \|Δ alphanumeric characters\| | 162 886 | **125 668** | |

### The documents the seat is on

**The gate does not see this and cannot be asked to.** The two documents it is worth most on fail
on their page count before and after, so no verdict moves on them; what moves is the page count
itself and the characters drawn.

| document | pages | alphanumeric characters | verdict |
|---|---|---|---|
| `24-25_FAA_Holdover_Tables` | **158 → 221** of 223 | 299 852 → **320 241** of 321 294, 6.71 % → **0.33 %** | `pages,words` → `pages` |
| `FAA 2025-26 Holdover Tables` | 171 → **172** of 233 | 335 933 → **352 762** of 358 857, 6.39 % → **1.70 %** | `pages,words` → `pages` |

On the first of the two the long caption — `Table 48: Snowfall Intensities as a Function of
Prevailing Visibility` — is now drawn **253 times against the reference's 253**, and on the second
`Snowfall Intensities` **578 against 578**. Round 87's hand-patched estimate for the first was 219
pages and 0.80 %; the implementation is two pages and half a percent better than that, because it
expands every `REF` rather than the twelve bookmarks that round could reconstruct by hand.

**And exactly two of the 338 renderings change at all**, byte for byte with the dates masked
(`pdfdiff.py`, `rtf-movers.txt`) — the same two. The other nine documents that state a `REF` are
unmoved because their bookmarks do not overlap, so the rotation is the identity on them and every
expansion equals the result the file cached. `extractdiff.sh` says the same thing through a second
channel: extracting with each binary, those two documents' text changes and the other nine do not.

## 1. The mechanism, verified here rather than taken from the brief

Three links, each re-derived from the C++ and then measured at the reference.

**RTF sends a bookmark half's name before its id.** `lcl_getBookmarkProperties`
(`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:224-236) sets `LN_CT_Bookmark_name` and then
`LN_CT_MarkupRangeBookmark_id`, under a comment that says the name *"should be sent first"*; the two
halves are emitted at `:2735-2764`, a start taking `m_aBookmarks.size()` for its id and an end
taking `m_aBookmarks[name]`, which default-constructs **0** for a name no start ever used.
`DomainMapper::lcl_attribute` (`dmapper/DomainMapper.cxx`:340-347) routes the two to
`SetBookmarkName` and `StartOrEndBookmark` in that order.

**`SetBookmarkName` is written for the opposite order.** (`DomainMapper_Impl.cxx`:9426-9447.) It
looks `m_sCurrentBkmkId` up — *the previously opened start* — and where that start is still open
writes the incoming name onto **it**, reaching `m_sCurrentBkmkName` only when the map misses. OOXML
states `w:id` before `w:name`, so the fallback is OOXML's path and the overwrite is RTF's.
`StartOrEndBookmark` (`:9450-9543`) then either inserts the bookmark under whatever name the entry
is holding — clearing `m_sCurrentBkmkId` — or registers a new entry under `m_sCurrentBkmkName`.

**A `REF` expands from that bookmark rather than from the file.** `FIELD_REF` with a first argument
that is not a `SetExpression` master becomes `ReferenceFieldSource::BOOKMARK` with
`ReferenceFieldPart::TEXT` (`DomainMapper_Impl.cxx`:8541-8635), and `SwGetRefField::UpdateField`
(`sw/source/core/fields/reffld.cxx`:594-670) recomputes it: `FindAnchor` (`:1559-1591`) answers the
start's node and offset, and an end which is the end offset in one node, the node's **length** for a
collapsed cross-reference bookmark (#i81002#), the start for any other collapsed one, and **−1** for
two ends in different nodes — which `:604-607` reads as *to the end of the paragraph*. A name no
mark holds draws `STR_GETREFFLD_REFITEMNOTFOUND` (`:594-598`).

### The simulator agrees with 26.2.4.2 on 27 of 28

`rotate.py` is those three functions written out in the order a bookmark half goes through them;
`genprobes.py` writes ten RTF files that put the two halves of the mechanism under the reference,
and `check.py` scores the prediction against 26.2.4.2's own flat ODF (where the bookmarks landed)
and its own PDF (what each `REF` drew). `marks-vs-reference.txt` is that run.

| probe | what it varies | predicted marks | predicted expansions |
|---|---|---|---|
| `a_single` | one bookmark | ✓ | ✓ |
| `b_sequential` | two that do not overlap | ✓ | ✓ ×2 |
| `c_nested` | B inside A | ✓ `B`, `B Copy 1` | ✓ ×2 |
| `d_overlap` | A opens, B opens, A closes, B closes | ✓ | ✓ ×2 |
| `e_two` | two starts then two ends | ✓ `R1`, `R1 Copy 1` | ✓ ×2 |
| `f_five` | the corpus's own shape, five starts | ✓ | ✓ ×5 |
| `g_crosspara` | two ends in different paragraphs | ✓ | ✓ |
| `h_point` | a collapsed bookmark | ✓ | ✓ |
| `i_missing` | a `REF` naming a name nobody holds | ✓ | ✓ |
| `j_crossref` | a collapsed `__RefHeading__` | **✗** | ✓ |
| `k_nested` | a group inside the end half's name | ✓ `xA` | ✓ (not found) |
| `l_control` | the same without it | ✓ `A` | ✓ |

**The one miss is the flat-ODF *shape* of a cross-reference bookmark and not what it expands to.**
26.2.4.2 writes that mark out spanning its whole paragraph where the simulator predicts a collapsed
one; the `REF` drew the whole paragraph on both sides, which is the behaviour the rule is about.

**Two controls bound the reach.** A single bookmark and two that do not overlap are named exactly as
the file states them — `SetBookmarkName` only overwrites while a start is still *open* — so the
rotation reaches documents with overlapping or nested bookmarks and no others.

### The fourth link, which the brief did not name and which the rotation makes expensive

**A group nested inside a bookmark's name is part of the name and not a half of its own.**
`RTFDocumentImpl::popState` guards both bookmark destinations with
`if (&getDestinationText() != getCurrentDestinationText()) break; // not for nested group`
(`rtfdocumentimpl.cxx`:2736-2740, :2751-2755), and that one test does two things: it skips the
nested group's own half, and — because the two states share one destination buffer — it leaves the
nested group's *text* inside the outer name.

Under a reader that pairs bookmarks by name this is harmless; under the rotation it is not. A
spurious half takes an id, and a spurious **end** naming nothing takes `m_aBookmarks[""]`, which
`std::map` value-initialises to **0** — so it closes the document's *first* bookmark under the wrong
name and leaves the real end to open a second one that never closes.

Measured, `gennested.py`, and the answer is not what reading the guard alone suggests: 26.2.4.2's
flat ODF for `{\*\bkmkstart A}first{\*\bkmkend {x}A}` holds **one bookmark called `xA`** — both
halves, because the end's name rotated onto the start — and its `REF A` therefore finds nothing.
The control without the nested group holds `A` and draws `first`. This tree now answers `xA` and
`A`; before the guard it answered a bookmark called `x` and lost the name altogether.
`AGroupInsideABookmarksNameIsPartOfTheName` is that pair, and it fails on a binary built without
the guard.

## 2. What the tree does with it

`RtfBookmarkRotation` is the naming, `RtfReferenceFields` the expansion. Two things about the shape
are worth writing down.

**The bookmark's key is the importer's id and its name is settled when it closes**, because that is
when the rotation stops moving it. `WritingMarkBuilder.CloseBookmark` gained an optional name for
that, which no other reader passes.

**A document that states a `REF` is read twice.** A `REF` may name a bookmark the walk has not
reached, so the expansions cannot be computed during the walk that draws them — and rewriting a
paragraph after it has closed would rebase every offset counted against it: its runs, its notes, its
frames, its page fields and its own marks. `RtfReader.Read` therefore computes the expansions from
the first read's marks and hands them to a second read, which substitutes them where the cached
result would have been appended. Only a document stating a `REF` whose expansion differs from its
cache pays for it, which is **2 of the 338** — measured by extracting every
one of the eleven that state a `REF` with each binary (`extractdiff.sh`).

**One deliberate divergence.** Where the rotation leaves a name on nobody, 26.2.4.2 draws
*"Error: Reference source not found"* and this tree keeps the producer's cached result. Reproducing
the error would mean drawing an error message wherever *our* bookmark table is the incomplete one —
a table built by a different reader over destinations LibreOffice's buffers reorder. The cost of not
reproducing it is measured rather than assumed: **two occurrences across the 13 converted `.rtf`
that state a `REF` at all**, one in `FAA 2025-26 Holdover Tables` and one in `ABCD-SDE-23-00`.

## 3. Reach

**11 of the 338 converted `.rtf` state a `REF` field**: 365, 332, 32, 19, 15, 10, 5, 4, 4, 1 and 1
occurrences (`refcensus.py`, `refcensus.txt`). Two of them are the holdover pair, and those two are
the only ones the change moves. **Count the `fldinst` group and not the string** — `REF ` also
matches `PAGEREF` and any heading called `REFERENCES`, which is how this round's first census said
13 and named two documents that state no `REF` at all.

## 4. Seat two: four more pool names are two mechanisms, and only one of them is here

Round 87 left `Body Text`, `caption`, `Title` and `Subtitle` as "26.2.4.2 answers 10, 10, 14 and 14
where this tree answers 12". The numbers are right and **the reading of them was not**: the four are
not one case, and neither of the two 10s is a pool value at all.

Read out of the pool rather than out of the brief:

| name | `ConvertStyleName` | pool style states | pool parent |
|---|---|---|---|
| `heading 1`…`heading 9` | `Heading N` | `aHeadlineSizes` percentages | `COLL_HEADLINE_BASE` |
| `Title` | `Title` | 28 pt, bold, centred (`:1365-1374`) | `COLL_HEADLINE_BASE` |
| `Subtitle` | `Subtitle` | 18 pt, centred, 3/6 pt (`:1376-1387`) | `COLL_HEADLINE_BASE` |
| `caption` | `Caption` | 10 pt, italic, 6/6 pt (`:971-983`) | `COLL_STANDARD` |
| `Body Text` | `Text body` | no size, 0/7 pt, 115 % (`:695-704`) | `COLL_STANDARD` |

`GetPoolParent` (`sw/source/core/doc/poolfmt.cxx`:279-289) is what settles the last column: every id
in `COLL_DOC_BITS` that is not `COLL_HEADLINE_BASE` itself has that style for a parent, and
`COLL_LABEL` and `COLL_TEXT` have `COLL_STANDARD` (`:201-204`, `:229-235`). Since the import resets
the entry's *own* properties, what a paragraph gets is the parent's — so `Title` and `Subtitle`
should answer `Heading`'s fourteen points and nothing of their own, and `caption` and `Body Text`
should answer whatever the document's own `Normal` entry says.

**Measured, and it is exactly that.** `genpool.py` writes each name twice, with `Normal` at `\fs20`
and at `\fs28`, which is the discriminator between a pool constant and inheritance from *Standard*;
`readpool.py` reads the size, the face and the two gaps out of the reference's PDF
(`pool-reference.txt`):

| probe | `Normal \fs20` | `Normal \fs28` |
|---|---|---|
| `heading 4` | 14 pt, 28.16 above, 17.49 below | 14 pt, 28.16, 17.49 |
| `Title` | **14 pt, 28.16, 17.49** | **14 pt, 28.16, 17.49** |
| `Subtitle` | **14 pt, 28.16, 17.49** | **14 pt, 28.16, 17.49** |
| `Body Text` | 10 pt, 11.55, 11.55 | **14 pt**, 16.16, 11.49 |
| `caption` | 10 pt, 11.55, 11.55 | **14 pt**, 16.16, 11.49 |
| `Quote` (control) | 12 pt, 13.83, 11.52 | 12 pt, 13.83, 11.52 |
| `Widget Heading` (control) | 12 pt, 13.83, 11.52 | 12 pt, 13.83, 11.52 |

So `Title` and `Subtitle` are the heading family under two other names and are **implemented** — one
branch in `RtfStyles.PoolFormattingOf`. `Body Text` and `caption` track `Normal` and are **left**:
that is inheritance from *Standard* rather than a pool value, it would need the whole of
`ConvertStyleName`'s two hundred names to be safe, and neither `COLL_TEXT`'s 0/7 pt nor
`COLL_LABEL`'s italic 6/6 survives the reset — so the pool entry a reader would be tempted to copy
is the wrong half of it. `APoolStyleUnderStandardIsNotModelledYet` pins the gap.

**Reach is nil for the half that is implemented, and the census that says otherwise counts the
stylesheet.** Round 87's `poolcensus.py` takes its *used* set from the whole file, so each entry's
own `\sN` counts as a paragraph using it and every declared style looks used. Excluding the
stylesheet group changes the answer: `Technical_Issue_Report_Form.rtf` declares `Title` at `\s667`
and **no paragraph names it**, and `ECSS-E-ST-50-16C-Annex-A`'s `Subtitle` is the same case. So over
the 338:

| name | documents, counting the stylesheet | documents, excluding it |
|---|---:|---:|
| `Title` | 1 | **0** |
| `Subtitle` | 1 | **0** |
| `Body Text` | 2 | **2** |
| `caption` | 0 | **0** |
| `heading 1`…`heading 9` | 17 | **12** |

The last row is round 87's own reach figure and it is **17 → 12** under the same correction; the
fix it measured is unaffected, the census it quoted is not. `poolcensus.py` here is the corrected
one and `poolcensus.txt` its run.

So `Title` and `Subtitle` are implemented and change **no corpus rendering at all** — which is
exactly what the column shows, two movers and both of them §1's. They are in because they are
right and free, and the write-up says so rather than claiming a win.

## 5. Confinement

The diff is six files. Four are under `XRay.Office.WordProcessing/Rtf`; the other two are shared and
neither changes an existing answer — `FieldInstructions` gains a method nothing else calls, and
`WritingMarkBuilder.CloseBookmark` gains an optional parameter only the RTF reader passes.
`XRay.Office.Spreadsheets` and `XRay.Office.Presentations` do not reference `XRay.Office.WordProcessing`
at all, so no sheets or slides row can be reached by construction.

Shown rather than asserted: rendered with a binary built at this round's base and again at its HEAD,
`obj`/`bin` cleared on each leg, under `SOURCE_DATE_EPOCH`, and md5-compared (`sweep-hash.sh`).

| track | files | rendered | identical | different |
|---|---:|---:|---:|---:|
| `/home/user/sample-files/words` — the original `.doc`/`.docx` | 338 | 338 | **338** | 338D |
| `/home/user/corpus-odf/words/**/odt` — the converted `.odt` | 338 | 338 | **338** | 338D |

Neither track loses a document to a render failure on either binary, so the comparison is over all
676 renderings and not over a surviving subset.

## 6. The tests

`RtfReferenceFieldTests` is nine assertions over the two mechanisms, each with the probe it was
measured on named in its own remarks; `RtfStyleFormattingTests` gains
`TheDocumentTitleStylesTakeTheSamePoolHeading` (two names) and
`APoolStyleUnderStandardIsNotModelledYet` (two more), the second pinning a gap rather than a rule.

The ten non-fidelity projects were run one at a time: 521, 109, 259, 728, 302, 143, 1044, 164,
1226, 1851 — all green, and the last of those is 1838 before this round's thirteen new tests.
`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed**, and the ten are exactly the expected names:
`PageDrawingComparisonTests.EveryLineIsDrawn` ×4, `TabStopComparisonTests.AListLabelsTabAdvance` ×4,
`SheetDrawingComparisonTests.APictureIsDrawn` and `JustificationShrinkComparisonTests`.

## 7. What is left with its seat

- **`Body Text` and `caption` inherit *Standard*** — §4. Measured, pinned by a test, not built.
- **The error string for a `REF` whose name nobody holds** — §2. Measured at two occurrences.
- **`FAA 2025-26 Holdover Tables` is still 172 pages against 233** while its glyph distance is now
  1.70 %, so whatever is left there adds pages without adding text. It is not the `REF` mechanism:
  the long expansion appears 578 times on both sides.
- **A bookmark half inside a table cell or a shape is *buffered*** (`bufferProperties`,
  `rtfdocumentimpl.cxx`:2744-2764) and replayed later, so the order the mapper sees is not always the
  order the file states. The rotation here runs in file order. No probe separates the two yet.
