# `rtf-holdover-r87` — the holdover pair is two defects, one of them general

## Environment

    ours    XRay.Office.Cli built in /home/user/wt-rtfhold, at 3466245b1 (base) and at this
            round's HEAD. Every figure names which. The base binary was made by copying the
            two changed source files aside and `git checkout`-ing them, with
            `XRay.Office.WordProcessing`'s and `XRay.Office.Cli`'s `obj`/`bin` deleted before each
            rebuild; it reproduces `Sample_SQMS_Program` at 60 pages and the changed one at 77,
            so neither half is a stale build.
    ref     /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2
            0229ac93fcf0d7cbc6376066c6f35021cef002dc, its version line read at the start of the
            round. Probe renderings were made fresh, one `soffice` profile per document keyed on
            an md5 of its own path, every call under `timeout -k 30 240`. For the column, the
            banked reference half /home/user/gate-odf-r80/ref (336 `__rtf.pdf`, rendered
            2026-09-08 02:56–03:40). The diff is confined to `XRay.Office.WordProcessing/Rtf`,
            which `soffice` cannot reach, so the banked reference is the same bytes throughout.
    corpus  /home/user/corpus-odf/words/**/rtf — 338 files, 26.2.4.2's own RTF export of the
            corpus's words track; 336 have a reference rendering and are scoreable.
            /home/user/sample-files/words — the 338 original `.doc`/`.docx`.
    rule    batch-check.sh of 2026-09-05, re-read rather than quoted: page count, then
            **alphanumeric characters** (the ninth column) within `max(2%, 15)`, then unembedded
            fonts. `score.py` applies that rule to a pair of banks and is round 80's, unchanged,
            so this column is comparable with rounds 80 and 85.

## The column

| | base `3466245b1` | after | of |
|---|---:|---:|---:|
| gate `match` | **259** | **260** | 336 |
| page-exact | 279 | **280** | 336 |
| total \|Δ pages\| | 319 | **296** | |
| total \|Δ alphanumeric characters\| | 164 119 | **162 886** | |

**The brief's table was stale in both directions and the base was re-measured rather than
taken from it.** `probes/odf-gate-r80/rows.tsv` reads `166/233` and `162/223` for the holdover
pair; at `3466245b1` they are **169/233** and **156/223**. The column itself is 259 at HEAD,
not the 257 the brief carried, and it is unchanged by the two merges that landed after round
85 (`c044d251f`, `3466245b1`) — the `\super` round's 259 is still the number.

**Four rows of 338 move and none loses a verdict:**

    Sample_SQMS_Program                       60/77   ->  77/77    pages,words -> match
    24-25_FAA_Holdover_Tables                156/223  -> 158/223   pages,words (unchanged)
    FAA 2025-26 Holdover Tables              169/233  -> 171/233   pages,words (unchanged)
    PES-Technical-Report-Template_Jan_2019    13/17   ->  15/17    pages -> pages,words

`PES-Technical-Report-Template_Jan_2019` is the one row that pays: two pages closer on the
count and 1 197 alphanumeric characters further away (91 short, then 1 288 over). It fails on
`pages` before and after, so no verdict moves, and the excess is not the change inventing text
— word-for-word against the reference it is the *bibliography*, which this tree draws more of
than 26.2.4.2 does at either binary, plus a two-page front matter (`iv`–`viii`, three
`THIS PAGE LEFT BLANK` leaves) that the reference has and this tree has never had. The
document is a stack of defects and this round moved one of them.

## Why the holdover pair is 65 pages short, and it is two things

The pair was the brief's target and it is worth being precise about what it turned out to be:
**two independent mechanisms, of which one is a two-document quirk and the other is general.**
Only the second is fixed here.

The instrument that separates them is the one the brief named — the `.odt` twin of the same
document — plus one the brief did not: **converting the `.rtf` to flat ODF with 26.2.4.2
itself**, which prints the reference's own answer for every bookmark and style in eight
seconds and needs no rendering at all (`rt/src.fodt` in the working directory; the command is
`soffice --convert-to fodt`).

### 1. `REF` expands from a bookmark whose ends the RTF import swaps — 30 of the 67 pages

26.2.4.2 draws `(Table 48: Snowfall Intensities as a Function of Prevailing Visibility)` where
the file states `{\field{\*\fldinst  REF _Ref107225632 \\h }{\fldrslt Table 48}}` and the
bookmark covers `Table 48` alone. The `.odt` twin of the same document draws `Table 48`, and so
do we. Counted over the two renderings, the long form appears **422 times against the `.odt`'s
171** — 251 extra copies of a 51-character string, which is most of the 21 602 alphanumeric
characters the reference draws and this tree does not.

**It is a bookmark-naming rotation in `writerfilter`, reproduced in three lines of RTF.**
`RTFDocumentImpl::popState` sends a bookmark half through `lcl_getBookmarkProperties`
(`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:224-236 and :2735-2764) as
`LN_CT_Bookmark_name` **and then** `LN_CT_MarkupRangeBookmark_id` — the comment there says *"If
present, this should be sent first"*. `DomainMapper::lcl_attribute` (`DomainMapper.cxx`:340-347)
routes the first to `SetBookmarkName` and the second to `StartOrEndBookmark`. But
`SetBookmarkName` (`DomainMapper_Impl.cxx`:9426-9447) is written for OOXML, where
`w:bookmarkStart` states `w:id` before `w:name`: it looks up `m_sCurrentBkmkId` — *the previous
start* — and where that start is still open it writes the incoming name **onto it**, reaching
`m_sCurrentBkmkName` only when the map misses. RTF's order is the other way round, so **every
name lands on the bookmark before it**, and a paragraph opening five bookmarks hands out five
names one place shifted.

`probe2/H_five.rtf` is that paragraph and nothing else — five `\bkmkstart`, three collapsed
`\bkmkend`, a `{\field}`, two more `\bkmkend` — and 26.2.4.2 draws `See Table 1: Caption Words
Here end.` for a `REF` whose bookmark ends after the number. Its flat-ODF round trip shows the
permutation exactly as the corpus document's does: the three collapsed marks come out named
`T2 R2 T3` for starts written `T1 T2 R2`, and the two ranges come out `T1` (the number) and
`R1` (the whole paragraph). Written the other way — `probe2/I_two.rtf`, two starts — the
importer emits **two bookmarks with the same name**, `R1` and `R1 Copy 1`, which is the same
fault with nowhere to hide.

What turns a mis-ended bookmark into a longer expansion is `SwGetRefFieldType::FindAnchor`
(`sw/source/core/fields/reffld.cxx`:1560-1592): a bookmark whose two ends are in different text
nodes answers `*pEnd = -1` (`:1588`), and `SwGetRefField::UpdateField`'s `Bookmark` case
(`:604-607`) reads that as *to the end of the paragraph* — `nEnd = nNumEnd<0 ? nLen : nNumEnd`.

**Measured worth, and why it is not implemented.** Substituting every `REF` result in
`24-25_FAA_Holdover_Tables.rtf` with what the reference expands it to — `expand.py` computes
them off the flat-ODF round trip, `patchref.py` writes them into the file, 332 replacements over
12 distinct bookmarks — takes this tree from **156 pages to 186** against the reference's 223,
and its alphanumeric count from 299 692 to 316 289 against 321 294. So it is 30 of the 67 pages
and 16 597 of the 21 602 characters, and it is **real** — with §2's fix in as well the same
patched file reaches **219 pages of 223**. Its reach is not: **11 of the 338
`.rtf` state a `REF` field at all**, and two of them are this pair (332 and 365 occurrences);
the other nine state between 1 and 32. Closing it moves no gate verdict, because 186 is not 223
either. It is written down here rather than built.

### 2. A `heading N` whose `\sbasedon` does not resolve keeps Writer's pool parent — the other 37

With the `REF` results patched in, our page 17 and the reference's page 17 hold the same words —
and the reference then spends a **whole extra page** on the empty bulleted paragraph that
follows, and we do not. It does that after every one of these table pages; we did it after some.
The residual is sub-line, and it is measurable at the bottom of the page: by the last body line
of page 17 the reference sits **5.5 pt** lower than we do.

The 5.5 pt is two headings. `NOTES` and `CAUTIONS` are drawn by 26.2.4.2 at **14 pt** and by
this tree at **12 pt** — same face, widths in the same 1.1667 ratio as the heights. The style is
`{\s4\sbasedon1392\snext0\sb120\sa120\lang1033 heading 4;}` and style 1392, *Notes/Cautions
Heading*, states `\fs18`. **Neither renderer used it**, and the reason is that style 1392 is
declared **after** style 4 in the same `{\stylesheet}`.

`\sbasedon` is converted from an index to a *name* where it is dispatched —
`pIntValue = new RTFValue(getStyleName(nParam))`,
`sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx`:131-134 — and `getStyleName`
(`rtfdocumentimpl.cxx`:873-885) reads `m_aStyleNames`, which is filled one entry at a time as
each entry *ends* (`:1568-1600`, `m_aStyleNames[m_nCurrentStyleIndex] = aName.trim()` at
`:1594`). A forward reference therefore resolves to the **empty string**, and
`lcl_findParentStyle` (`:275-298`), which matches `LN_CT_Style_basedOn` against other entries'
`LN_CT_Style_name`, finds nothing. Round 80 already found that half and `RtfStyles.Add` already
drops such a parent.

**What round 80 did not have is what happens next, and it is not "nothing".**
`StyleSheetTable::ApplyStyleSheets` (`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:1099-1121)
converts the entry's name through `ConvertStyleName` (`:1620-1660`, whose map holds
`heading 1`…`heading 9` and `Heading 1`…`Heading 9`), and where `xStyles->hasByName` answers yes
it **reuses Writer's own style of that name**, resets its properties and leaves its parent
alone. The one branch that would clear the parent needs `m_bHasImportedDefaultParaProps`, which
only an OOXML `w:docDefaults` sets (`:653-667`) — never RTF. A `\sbasedon` that *does* resolve
calls `setParentStyle` and takes that parent's place instead (`:1156-1169`), which is what makes
this a fallback rather than an override.

`Heading 4`'s parent is `Heading`, and `SwPoolFormatId::COLL_HEADLINE_BASE`
(`sw/source/core/doc/DocumentStylePoolManager.cxx`:768-819) states exactly four things a
paragraph can take: `SvxFontHeightItem aFntSize(PT_14, …)` at `:809`,
`SvxULSpaceItem aUL(PT_12, PT_6, RES_UL_SPACE)` at `:810`, and
`SvxFormatKeepItem(true, RES_KEEP)` at `:814`. The per-level percentages in `aHeadlineSizes`
(`:107-115`) do **not** arrive with them — the import resets the level style's own properties —
which is why every level answers 14 pt and not 18.2, 16.1, 14.1, …

All four were measured against 26.2.4.2 before anything was written, `genstyles.py` and
`readstyles.py`:

| probe | what it varies | 26.2.4.2 | this tree, before | after |
|---|---|---|---|---|
| `ord_before` | `\sbasedon` target declared **first** | 9 pt | 9 pt | 9 pt |
| `ord_after` | the same two entries, other order | **14 pt** | 12 pt | **14 pt** |
| `q_h4_nobase` | `heading 4`, no `\sbasedon` at all | **14 pt** | 12 pt | **14 pt** |
| `q_h4_own` | `heading 4` stating its own `\fs20` | 12 pt | 12 pt | 12 pt |
| `fw_h1`…`fw_h5` | five heading levels, forward parent | **14 pt** ×5 | 12 pt ×5 | **14 pt** ×5 |
| `fw_widget`, `fw_list`, `fw_quote` | a name Writer answers nothing for | 12 pt | 12 pt | 12 pt |
| `sp_h4` | the heading's own spacing | +12 pt above, +6 below | none | +12 / +6 |
| `q_ownsp` | a parentless style stating `\sb480\sa480\fs28` | none, 12 pt | same | same |
| `n_h4_50…59`, `n_widget_50…59` | 20 files sweeping a page boundary | keep-with-next on the heading and not on the control | — | 20 of 20 |

`q_h4_own` is the control that keeps this inside the rule round 80 measured: a named style's own
character size never reaches its paragraphs, so a heading that states one gets neither its own
ten points nor the pool's fourteen but `\pard\plain`'s twelve. The pool sits in the *inherited*
half, which is the half that does reach the text.

**The space below is why `RtfStyleFormatting` gained a member it had refused before.** Its own
remark says `\sa` is deliberately absent, because `getDefaultSPRM` writes `after = 0` over any
style's space after. That is a statement about **RTF sprms**, and the pool's six points is not
one — nothing in `cloneAndDeduplicateSprm`'s table can reach it. `SpaceAfterTwips` carries the
pool's value and no `\sa` ever writes to it; a paragraph's own `\sa0` still beats it, which is
its own assertion in the tests.

**Reach, censused two ways.** `fwcensus.py`: **54 of the 338 `.rtf` hold at least one forward
`\sbasedon`**. `poolcensus.py`, which is the one that matters — a `heading N` entry whose
`\sbasedon` does not resolve *and* which some paragraph names: **17 of the 338**, spread over all
nine levels. Six of the seventeen were failing rows.

## Confinement

The diff is two files, both under `src/XRay.Office.WordProcessing/Rtf`. `XRay.Office.Spreadsheets`
and `XRay.Office.Presentations` do not reference `XRay.Office.WordProcessing` at all — their
`csproj` name Core, Containers, Text, Vector, Ooxml, OpenDocument and MsBinary and nothing else
— so no sheets or slides row can be reached by construction, and the words family is what was
measured. `git grep` for the changed symbols (`RtfStyleFormatting`, `RtfStyles`,
`ApplyParagraph`, `WithdrawParagraph`) finds them in three files under `Rtf/` and one test.

Rendered with a binary built at this round's base and again at its HEAD under
`SOURCE_DATE_EPOCH`, md5-compared (`sweep-hash.sh`):

| track | files | rendered | identical | different |
|---|---:|---:|---:|---:|
| `/home/user/sample-files/words` — the original `.doc`/`.docx` | 338 | 338 | **338** | 0 |
| `/home/user/corpus-odf/words/**/odt` — the converted `.odt` | 338 | 338 | **338** | 0 |

Neither track loses a document to a render failure on either binary, so the comparison is over
all 676 renderings and not over a surviving subset. Sheets and slides are not measured because
they cannot be reached: neither `XRay.Office.Spreadsheets` nor `XRay.Office.Presentations` names
`XRay.Office.WordProcessing` in its `csproj`.

## The tests

`RtfStyleFormattingTests` gains four:
`AHeadingWhoseParentDoesNotResolveTakesWritersPoolHeadingSize` (both spellings of *does not
resolve*), `AResolvableParentReplacesThePoolHeading`,
`APoolHeadingNeitherRenamedNorRestatedIsTheOnlyOneThatTakesIt` and
`ThePoolHeadingBringsTwelvePointsAboveSixBelowAndKeepWithNext`. The ten non-fidelity projects
were run one at a time: 109, 519, 259, 143, 1044, 164, 1226, 728, 302, 1831 — all green.
`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed**, and the ten are exactly the expected
names: `PageDrawingComparisonTests.EveryLineIsDrawn` ×4, `TabStopComparisonTests.AListLabelsTabAdvance` ×4,
`SheetDrawingComparisonTests.APictureIsDrawn`, `JustificationShrinkComparisonTests`.

## What is left with its seat

- **The `REF` rotation above.** Mechanism established and cited, worth 30 pages on one document
  and 11 documents' reach; not built.
- **Four more pool names.** `fw_bodytext`, `fw_caption`, `fw_title` and `fw_subtitle` still
  disagree — 26.2.4.2 answers 10, 10, 14 and 14 where this tree answers 12 — because only the
  heading family is modelled. `ConvertStyleName`'s map is two hundred names long and each needs
  its pool style's own items; the heading family is the one the corpus is short on.
- **The holdover pair's remaining 65 and 62 pages.** With both mechanisms in
  (the fix, plus the `REF` results patched into the file by hand) `24-25` reaches
  PATCHED_AFTER pages against 223. What is left after that has not been looked at.
- **`PES-Technical-Report-Template_Jan_2019`'s missing front matter** — the reference opens with
  a title page and a five-leaf roman-numbered preface this tree has never drawn, which is worth
  two pages of the four it is short.
