# `rtf-htmautsp-r85` — what closes RTF's settings window, and why `150-5370-10H` was the only document it cost

## Environment

    ours    XRay.Office.Cli built in /home/user/wt-rtfhtmautsp, at 6a6f18d9a (base) and at this
            round's HEAD. Every figure below names which. The base binary was made by copying
            the two changed files aside and `git checkout`-ing them, with the project's
            `obj`/`bin` deleted before each rebuild; it reproduces `150-5370-10H` at 702 pages
            and the changed one at 746, so neither half is a stale build.
    ref     /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2
            0229ac93fcf0d7cbc6376066c6f35021cef002dc, checked at the start of the round.
            Every probe rendering was made fresh, one `soffice` profile per document keyed on
            an md5 of its own path. For the column, the banked reference half
            /home/user/gate-odf-r80/ref (336 `__rtf.pdf`, rendered 2026-09-08 02:56–03:40).
            The diff is confined to `XRay.Office.WordProcessing/Rtf`, which `soffice` cannot
            reach, so the banked reference is the same bytes throughout.
    fonts   All five tarball confounds aside: /opt/libreoffice26.2/share/fonts/truetype holds
            82 files with .duplicates-aside, .noto-aside and .condensed-aside beside it;
            `fc-list | grep -ci condensed` is 0 and `fc-match "DejaVu Sans"` answers DejaVu Sans.
            Nothing was moved during the round.
    corpus  /home/user/corpus-odf/words/**/rtf — 338 files, 26.2.4.2's own RTF export of the
            corpus's words track; 336 have a reference rendering and are scoreable.
            /home/user/sample-files/words — the 338 original `.doc`/`.docx`.
    rule    batch-check.sh of 2026-09-05, re-read at `:285-293` rather than quoted: page count,
            then **alphanumeric characters** (`$og`/`$rg`, the ninth column) within
            `max(2%, 15)`, then unembedded fonts. `score.py` applies that rule to a pair of
            banks and is round 80's, unchanged, so the two rounds' columns are comparable.

## The column

| | base `6a6f18d9a` | after | of |
|---|---:|---:|---:|
| gate `match` | **258** | **259** | 336 |
| page-exact | 278 | **279** | 336 |
| total \|Δ pages\| | 363 | **319** | |
| total \|Δ alphanumeric characters\| | 161 396 | 164 119 | |

The base was re-measured rather than taken from the brief and reproduces it exactly:
**258 match, 34 `pages`, 24 `pages,words`, 20 `words`** of 336.

**One document gains the verdict and none loses it**, and **one rendering of the 338 changes
by a byte** (`bytediff.py`, dates masked; 337 identical):

    150-5370-10H    702 / 746  ->  746 / 746      pages -> match

That is the brief's largest row, and it is now page-exact against 26.2.4.2. The alphanumeric
total rises because that document draws 44 more pages of running head and foot: its own glyph
distance from the reference goes 89 → 2812 on a base of 1 504 839, which is a twentieth of the
2 % band.

**The original `.doc`/`.docx` words track does not move.** All **338** of it, rendered with a
binary built at this round's base and again at its HEAD under `SOURCE_DATE_EPOCH`, md5-compared:
**338 identical, 0 different, 0 failed** (`sweep-orig-hash.sh`, `orig85-base.tsv` against
`orig85-after.tsv`). The variable is what makes a plain md5 mean anything — `xray-office render`
honours it in both `/CreationDate` and a header's date fields — and it is what let the check run
without banking 338 PDFs twice on a container with 3.5 GB free.

## The rule, and the C++ that states it

Every citation below was opened and read in `/home/user/libreoffice-core` at the line quoted.

### 1. `\super` sends the settings table, and a list level is not exempt

`RTFDocumentImpl::dispatchFlag`'s `SUPER` case
(`sw/source/writerfilter/rtftok/rtfdispatchflag.cxx`:887-895):

```cpp
case RTFKeyword::SUPER:
{
    // Make sure character properties are not lost if the document
    // starts with a footnote.
    if (!isStyleSheetImport())
    {
        checkFirstRun();
        checkNeedPap();
    }
```

`isStyleSheetImport()` is true for `Destination::STYLESHEET` and `STYLEENTRY` and for nothing
else (`rtfdocumentimpl.cxx`:911-916). `checkFirstRun` calls `outputSettingsTable` once and then
clears `m_bFirstRun` (`:428-435`), so every `\htmautsp` after it is set on an
`m_aSettingsTableSprms` nobody reads again.

**`\sub` and `\nosupersub` beside it do not call it** (`:904` and `:910`), which is the kind of
asymmetry no reading of the specification would predict and which the tests now pin.

**The reach is a list level, and that is the whole of it.** LibreOffice's own RTF export writes
`\super` into a numbering level whose text is superscript, and writes `\htmautsp` after the list
table. `150-5370-10H.rtf`'s 39th `{\list}` opens

    {\listlevel\levelnfc0\leveljc0\levelstartat1\levelfollow0
     {\leveltext \'01\'00;}{\levelnumbers\'01;}\super\rtlch\af28\ltrch\fi-360\li540}

at byte 222775, and the document states `\htmautsp` at 266199.

### 2. A list table is dispatched; a header, a footer and a private extension are not

`RTFTokenizer::dispatchKeyword` returns **before it looks the keyword up at all** when the
destination is `Destination::SKIP` (`rtftokenizer.cxx`:2156-2164), and `RTFDocumentImpl::resolveChars` (`:1314`) does the
same for text at `:1424-1425`. So nothing inside a `{\header}`, a `{\footer}`,
an `{\info}` or a `{\*\anything-unknown}` has any effect whatsoever — which is the real reason a
header does not close the window, rather than round 80's "a substream is re-parsed when its
section ends".

A list table is `Destination::LISTTABLE` rather than `SKIP`, so its control words **are**
dispatched. That one distinction is the whole of this round.

**The second half of it was measured the hard way.** The first cut of the fix asked only
"the destination is not `Skip` and not `StyleSheet`", and `FRE-03_mcar_part-3_and_IS_v2.9.rtf`
went 71 pages to 74 against the reference's 70 — it writes `\super` at byte 330901 inside a
`{\header}` and `\htmautsp` at 331546. This tree routes a header to a flow of its own rather
than to a skipped destination, so the predicate also has to ask that the word is in the
document's own flow, which is what `NoteSettingsWindowClosed` does.

### 3. What the flag does once it is read

`\htmautsp` clears `LN_CT_Compat_doNotUseHTMLParagraphAutoSpacing`
(`rtfdispatchflag.cxx`:1354-1357); `SettingsTable`'s constructor sets it **true** for an RTF
import (`dmapper/SettingsTable.cxx`:118-126, *"HTML paragraph auto-spacing is opt-in for RTF,
opt-out for OOXML"*); `DomainMapper_Impl::ApplySettingsTable` writes it to
`AddParaTableSpacing` (`DomainMapper_Impl.cxx`:10179); `SwXDocumentSettings` maps that name onto
`DocumentSettingId::PARA_SPACE_MAX` (`SwXDocumentSettings.cxx`:189, :445-450); and
`SwFlowFrame::CalcUpperSpace` reads it as **add**, not as max —
`nUpper = nPrevLowerSpace + pAttrs->GetULSpace().GetUpper()` under
`if( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX) )` (`sw/source/core/layout/flowfrm.cxx`:1652-1654).
The name says the opposite of what the flag does, which is worth knowing before reading any of
the other eight sites that consult it.

## What was measured, and in what order

Every probe family ends in the same three paragraphs — `\sa480`, then `\sb480`, then nothing —
so the distance from `AAAA` to `BBBB` is one line plus **24 pt** where the larger of two
adjacent spacings wins and one line plus **48 pt** where they add. The two answers are 24 pt
apart whatever the face is, so the midpoint separates them without depending on the line height.

| family | probes | what it settles |
|---|---:|---|
| `genmove.py` → `move-ref.txt` | 3 | the witness with the word where it stands, deleted, and moved to byte 31 |
| `genhead.py` → `head-ref.txt` | 16 | which part of the 266 KB preamble costs the word |
| `genlist.py` → `list-ref.txt` | 6 | the list table's picture and its levels, separately |
| `genlistn.py` → `listn-ref.txt` | 21 | which of the 70 `{\list}` entries |
| `genwindow.py` → `window-after.txt` | 10 | the window's shape on hand-built documents, both sides |
| `genrow.py` → `row-after.txt` | 7 | a row, a cell, a field and a picture after the word, both sides |
| `gensuper.py` → `super-after.txt` | 22 | `\super` isolated, each case against its own no-word control, both sides |
| `genbodypict.py` → `bodypict-after.txt` | 6 | a body `{\pict}` separated from the `\par` beside it, both sides |
| `genresidual.py` → `residual-ref.txt` | 8 | where the one unreproduced arm comes from |
| `census.py`, `census-all.py` | 338 | the reach of all thirteen callers over the column |

**The witness, three ways** (`move-ref.txt`): 26.2.4.2 draws `150-5370-10H.rtf` in **746** pages
as it stands, **746** with the nine bytes of `\htmautsp` deleted — the two renderings are
text-identical on all 746 pages — and **705** with the same nine bytes moved to byte 31. So the
word is inert where the author wrote it and is honoured before the font table, and the
difference is made somewhere between.

**The preamble bisect** (`head-ref.txt`): the flip is between byte **178110**, which is the end
of the style sheet, and **260222**, which is the end of `{\*\listtable}`. Not one of the eleven
cut points after the list table changes the answer.

**Inside the list table** (`list-ref.txt`, `listn-ref.txt`): the `{\*\listpicture}` alone honours
the word and one `{\list}` alone honours it; 32 lists honour it, 39 do not, and the 39th is the
one that states `\super`.

**`\super` isolated** (`super-after.txt`, 22 probes, this tree agrees with 26.2.4.2 on **20**):
on a hand-built preamble, one list level with `\super` loses the word and the identical level
without it keeps it; a `\super` in a `{\header}` keeps it; a `\super` in a style-sheet entry
keeps it. Each of the eleven cases carries a twin with the word deleted, and **that is the
control the round turned on** — see below.

## What this brief got wrong

**"Something after the word fires `checkFirstRun` and closes the window."** It cannot: nothing
parsed after `\htmautsp` can un-send a table that has not been sent yet, and the settings sprms
are never cleared — `m_aSettingsTableSprms` takes **twenty** `set` calls across `rtftok` and there
is no `erase` or `clear` of it anywhere. The
inference does not follow from its own premise, and **the premise was a confounded
measurement**. Round 80 rendered `head + witness[178110:266208]` — the document's own bytes from
the list table up to and including the word, on a *synthetic* preamble — watched the reference
collapse, and concluded the trigger was later. The 178 KB it replaced is where the trigger is.

**"The round left eight named non-picture callers to walk."** The eight line numbers are all
correct, verified one by one:

| | |
|---|---|
| `rtfdispatchdestination.cxx`:264 | `\footnote` |
| `rtfdispatchdestination.cxx`:405 | `\shptxt`, `\dptxbxtext`, when the shape is not a picture frame |
| `rtfdispatchflag.cxx`:893 | `\super`, when not a style-sheet entry |
| `rtfdispatchflag.cxx`:1103 | `\dptxbx` |
| `rtfdispatchsymbol.cxx`:133 | `\par`, except in a `{\*\ftnsep}` |
| `rtfdispatchsymbol.cxx`:250 | `\cell`, `\nestcell` |
| `rtfdispatchsymbol.cxx`:518 | `\column` |
| `rtfdispatchsymbol.cxx`:574 | `\page` |

But **`checkFirstRun` has thirteen call sites, not nine**: the five above them are
`tableBreak` (`rtfdocumentimpl.cxx`:724), `parBreak` (:732), `resolvePict` (:1297 — round 80 and
this tree both said **1296**), `text` (:1706) and `RTFFrame::setSprm` (:4137, the only one
guarded on `getFirstRun()` itself). And the eight are not "non-picture callers to walk" in the
sense the brief meant: **seven of the eight can only stand in a document's body**, where the
word is already too late by every other rule, so `\super` was the only one that could ever have
mattered.

**"Reach: 308 of 338 `.rtf` state the word — the largest single lever in that column."** 308 do
state it, and the *lever* is **1 of 338**: `census-all.py` walks all thirteen callers over the
column, honouring `Destination::SKIP` and the style-sheet guard, and finds exactly one document
with any caller before its `\htmautsp` — the witness, with `\super`. The other 307 already read
the word correctly on both sides. A rule that reached 308 documents would have been a large
regression risk; this one changes **one rendering of 338** and the byte comparison proves it.

**"`\htmautsp` is honoured by us and not by the reference on the `.rtf` column's largest row."**
That part is exactly right, and it is what this round closed.

## What round 80 got wrong, and the instrument that hid it

**A probe of this shape needs a no-word twin, and without one it measures the wrong thing.**
With the witness's own 178 KB preamble, a list level stating `\super`, and no table in the body,
26.2.4.2 collapses two adjacent spacings **with no `\htmautsp` in the file at all**
(`super-after.txt`, `x-super-norow-noword` at 38.65 against `x-nosuper-norow-noword` at 62.65).
Read without its control, that arm says *the word was honoured*. It says nothing of the kind.

Four of this round's own intermediate families are confounded by exactly that and are kept as
the record of it rather than as evidence: `genprefix.py`, `genown.py`, `genpict.py` and
`gendel.py` all measure the witness with no body table, so their "the word survives at byte
266229" readings are the same artefact. `genhead.py` — the family with one table row in it — is
the sound instrument, and `gensuper.py`'s twins are what proved which was which.

**Round 80's `p-bodypict` cannot separate a picture from the `\par` beside it**, because it
writes `\pard\plain{\pict …}\par\htmautsp` and `\par` is `checkFirstRun`'s caller at
`rtfdispatchsymbol.cxx`:133. Separated here (`bodypict-after.txt`, six probes with their own
no-word twins): **a body `{\pict}` alone does close the window**, `\par` alone closes it, and
both close it together. So round 80's conclusion stands and only its argument did not; this tree
agrees with 26.2.4.2 on all six, and nothing about the picture rule changed.

**And round 80's own `htm-afterheader` row was right for the wrong reason.** A `{\header}` group
does not close the window because `dispatchKeyword` never dispatches inside a `SKIP`
destination, not because "a substream is re-parsed when its section ends". The distinction is
not academic: it is what tells you that a *list table* does close it.

## What is left, and where its seat is

**One arm of the probe families is not reproduced, and it is not a question about the word.**
With the witness's own style sheet, a list-level `\super`, and no table in the body, 26.2.4.2
uses the **maximum** where every other arrangement adds — and it does so with the word deleted,
so `\htmautsp` is not what it is about. `genresidual.py` localises it: the font table and the
colour table do not produce it, the 175 KB style sheet does, and **adding one table row makes
the reference add the spacings again** (`residual-ref.txt`, `r-styles` 38.65 against
`r-styles-row` 62.65).

The seat to start from is `DomainMapper_Impl::ApplySettingsTable`
(`DomainMapper_Impl.cxx`:10122-10125), which returns silently when `m_xTextDocument` is not yet
there and wraps everything after it in one `try`, with `AddParaTableSpacing` set late in that
block at `:10179` — so a settings table sent very early can leave the flag at Writer's own
default, which is *max*. That is a hypothesis and not a measurement; what is measured is the
four-way table above. **It has no reach on this corpus**: the witness opens its body with a
table, which is the arrangement the reference and this tree agree on.

**The other twelve callers are read and not implemented**, deliberately. `census-all.py` says
none of them stands before a `\htmautsp` in any of the 338, so implementing them would be
unmeasurable here and a regression risk in the 308 documents that state the word. The census is
the instrument to re-run before adding one.

**`w-beforerowdef` has no reference marks** and is 1 of the 10 in `window-after.txt` that cannot
be scored: a `\trowd` with no `\row` buffers the rest of the document into a row that never
closes, so the reference draws a blank page. It is kept because that is also why every cut of
the witness inside its first table row is unreadable, which is what forced the `genhead.py`
design.

## Files

| | |
|---|---|
| `genmove.py`, `move-ref.txt` | the witness with its word in place, deleted, and moved to byte 31 |
| `scan.py`, `snap.py`, `rowcuts.py` | the witness's group structure, and where a prefix may be cut |
| `genhead.py`, `head-ref.txt` | 16 preamble cuts with a table row — the sound bisect |
| `genlist.py`, `list-ref.txt`; `genlistn.py`, `listn-ref.txt` | the list table's parts, then its 70 entries |
| `gensuper.py`, `super-after.txt` | `\super` isolated, 11 cases and 11 no-word twins, both sides |
| `genwindow.py`, `window-after.txt`; `genrow.py`, `row-after.txt` | the window's shape on hand-built documents, both sides |
| `genbodypict.py`, `bodypict-after.txt` | a body `{\pict}` separated from its `\par`, both sides |
| `genresidual.py`, `residual-ref.txt` | where the one unreproduced arm comes from |
| `genprefix.py`, `prefix-ref.txt`; `genown.py`, `own-ref.txt`; `genpict.py`, `pict-ref.txt`; `gendel.py`, `del-ref.txt`; `gensuffix.py`, `pages.py` | **the confounded families**, kept as the record of how the wrong instrument reads |
| `census.py`, `census.txt`; `census-all.py`, `census-all.txt` | the reach of `\super` and of all thirteen callers over the 338 |
| `refrender.sh`, `oursrender.sh` | render a probe directory through the reference and through this tree |
| `measureab.py`, `compareab.py` | the A→B distance, one side and both |
| `sweep-ours.sh`, `score.py`, `compare.py`, `bytediff.py` | round 80's, unchanged, so the columns are comparable |
| `sweep-orig-hash.sh` | the original words track, hashed and unlinked as it goes |
| `base-rtf.tsv`, `after-rtf.tsv`, `column.txt`, `confinement.txt` | the column before and after, and the byte comparison |

## Reproducing

```sh
export TMPDIR=/abs/tmp                      # absolute: a relative one makes soffice hang forever
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
R=dotnet/probes/rtf-htmautsp-r85

python3 $R/census-all.py /home/user/corpus-odf/words          # 1 of 338

python3 $R/genmove.py     /abs/move && $R/refrender.sh /abs/move /abs/moveout 3
python3 $R/genhead.py     /abs/head && $R/refrender.sh /abs/head /abs/headout 5
python3 $R/measureab.py   /abs/headout                        # flips at 260222

for f in gensuper genwindow genrow genbodypict; do
  python3 $R/$f.py /abs/$f && $R/refrender.sh /abs/$f /abs/${f}out 6 \
      && $R/oursrender.sh /abs/$f /abs/${f}out && python3 $R/compareab.py /abs/${f}out
done

$R/sweep-ours.sh rtf /abs/rtf-after 3
python3 $R/score.py /abs/rtf-after /home/user/gate-odf-r80/ref rtf > /abs/after.tsv
python3 $R/compare.py $R/base-rtf.tsv /abs/after.tsv
python3 $R/bytediff.py /abs/rtf-base /abs/rtf-after

$R/sweep-orig-hash.sh /abs/orig-after.tsv 3                   # 338 md5s, no PDFs kept
```
