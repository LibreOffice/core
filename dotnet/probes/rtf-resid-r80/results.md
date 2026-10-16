# `rtf-resid-r80` — what a paragraph takes from the style it names, and when a setting is still readable

## Environment

    ours    XRay.Office.Cli built in /home/user/wt-rtfresid2, at 5c003d58c (base) and at this
            round's HEAD. Every figure below names which.
    ref     /opt/libreoffice26.2/program/soffice — LibreOffice 26.2.4.2
            0229ac93fcf0d7cbc6376066c6f35021cef002dc. For the column, the banked reference half
            /home/user/gate-odf-r78/ref, rendered 2026-09-07 20:34–21:16; every probe rendering
            below was made fresh, one profile per document keyed on an md5 of its own path.
    fonts   All five tarball confounds aside: /opt/libreoffice26.2/share/fonts/truetype holds 82
            files with .duplicates-aside (38), .noto-aside (8) and .condensed-aside (8) beside it;
            `fc-list | grep -ci condensed` is 0 and `fc-match "DejaVu Sans"` answers DejaVu Sans.
            Checked at the start of the round and unchanged throughout — nothing was moved.
    corpus  /home/user/corpus-odf/words/**/rtf — 338 files, 26.2.4.2's own RTF export of the
            corpus's words track; 336 have a reference rendering and are scoreable.
    rule    batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
            max(2%, 15), then unembedded fonts. `score.py` applies that rule to a pair of banks.

Only our half was re-rendered. The diff is confined to `XRay.Office.WordProcessing/Rtf`, which
`soffice` cannot reach, so the banked reference is the same bytes throughout.

**The base was re-measured rather than taken from the brief**, because `873c766f9`..`5c003d58c`
touches `XRay.Office.Rendering/Pdf/FontSubsetter.cs`, which every rendering goes through. It
reproduces the brief exactly: **257 match, 36 `pages`, 24 `pages,words`, 19 `words`** of 336.

## The column

| | base `5c003d58c` | after | of |
|---|---:|---:|---:|
| gate `match` | **257** | **258** | 336 |
| page-exact | 276 | **278** | 336 |
| total \|Δ pages\| | 436 | **363** | |
| total \|Δ alphanumeric characters\| | 164675 | **161396** | |

Three documents gain the verdict (`02_mcar_part-2_and_IS_v2.10` 317 → 318 of 318,
`231164_SystemDesignDocument` 20 → 21 of 21, `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX`
22 → 21 of 21) and two lose it, each by one page (`120509coss` 25 → 26 of 25,
`SPA-11_mcar_part-11_v2.9` 47 → 46 of 47). **14 renderings move at all, 9 closer and 5 further**,
and the aggregate is what this round actually bought: a sixth of the column's page error and the
brief's largest single row, `150-5370-10H`, from **651 to 702 against 746**.

`column.txt` is that table with every mover; `base-rtf.tsv` and `after-rtf.tsv` are the two scored
columns. `after1`/`after2`/`after3`/`after4-rtf.tsv` are the four intermediate states, which is how
the per-rule table in *What moved* below is measured rather than argued.

**The original `.doc`/`.docx` words track does not move.** All **338** of it, rendered with a
binary built at this round's base and again at its HEAD, byte-compared with the PDF dates masked:
**338 identical, 0 different.** `sweep-orig-words.sh` renders it and `bytediff.py` compares;
the base binary was made by copying the three changed files aside and `git checkout`-ing them, and
the restored binary reproduces three swept `.rtf` renderings byte for byte, so neither half is a
stale build.

## The rule, and the C++ that states it

Every citation below was opened and read in `/home/user/libreoffice-core` at the line quoted.

### 1. A paragraph takes nothing from the paragraph properties of the style it *names*

The seat is `cloneAndDeduplicateSprm`
(`sw/source/writerfilter/rtftok/rtfsprm.cxx`:290-340). `RTFDocumentImpl::getProperties`
(`rtfdocumentimpl.cxx`:534-637) ends with

```cpp
RTFSprms sprms(aSprms.cloneAndDeduplicate(aStyleSprms, nStyleType, true, &aSprms));
```

where `aSprms` is the paragraph's own properties and `aStyleSprms` is the **named** style entry's,
flattened by `lcl_copyFlatten` (:490-514). For every property the style states and the paragraph
does not, that function reaches its

```cpp
// not found - try to override style with default
RTFValue::Pointer_t const pDefault(getDefaultSPRM(rSprm.first, nStyleType));
if (pDefault) { ret.set(rSprm.first, pDefault); }
```

branch (:319-327) and writes RTF's default onto the paragraph as **direct** formatting. The
ancestors' statements are not in `aStyleSprms` — `lcl_copyFlatten` is given the named entry alone —
so they are never visited and reach the paragraph through Writer's own style chain.

**Measured over 53 one-page probes, thirteen paragraph properties × four positions in the chain**
(`genmatrix.py`, `matrix-after.txt`; the tree reproduces 26.2.4.2 on **53 of 53**):

| the property is stated | `\qc \qr \qj` | `\sb` | `\sa` | `\li` | `\ri` | `\fi` | `\sl` | `\keepn` |
|---|---|---|---|---|---|---|---|---|
| by the style the paragraph names | — | — | — | — | — | — | — | **applies** |
| by an ancestor of it | **applies** | **applies** | — | — | — | — | — | **applies** |
| by `\s0`, paragraph naming a child of it | **applies** | **applies** | — | — | — | — | — | **applies** |
| by `\s0`, paragraph naming no style at all | — | — | — | — | — | — | — | *(unmeasured)* |

Which properties survive follows from the same function's table and the probes agree with it row
for row. `getDefaultSPRM` (`rtfsprm.cxx`:154-224) answers:

- **`LN_CT_PPrBase_spacing` → a value carrying `after = 0` and nothing else**, taken before the
  per-attribute recursion — so a style's space *after* is written over and its space *before* is
  never visited. That one line is the whole of the `\sa`/`\sb` asymmetry, which no reading of the
  specification would predict.
- `LN_CT_Ind_left`, `_right`, `_firstLine` → 0, reached through the recursion on `ind`, so every
  indent is zeroed.
- `LN_CT_Spacing_line` → 240 and `_lineRule` → auto, so a style's `\sl` is single spacing.
- **nothing at all for `jc`, `keepNext` or `tabs`**, which is why an alignment survives from an
  ancestor and a keep-with-next survives even from the named style: a plain value with no default
  and no children leaves the branch with nothing to write.

`\keepn` is measured separately, because a three-paragraph probe cannot see it: a page filled to
its last line, with `\keepn` direct, from the named style and from its parent, moves the paragraph
to the next page in all three (`genextra.py`'s `keep45-*`, 12 probes at three fill levels).
Tab stops are measured the same way and behave as `jc` does — an inherited `\tx4320` puts the word
after the tab at x = 288.1 and the named style's own leaves it at the default 108.1. **Tab stops
are read and the reader does not carry them yet; see *What is left*.**

### 2. The same split governs the *character* half, and the face is not in it at all

The character arm of `getDefaultSPRM` (:156-176) answers `24` for `sz` and `szCs`, `0` for `b`,
`i` and the colour, `none` for the underline and `"Times New Roman"` for the three `rFonts` slots —
so the same branch replaces those too. **Measured over 29 probes, seven character properties ×
four positions** (`genchar.py`, `char-after.txt`, 29 of 29):

| | own | inherited |
|---|---|---|
| `\fs` | 12 pt, the RTF default | **applies** |
| `\b`, `\i`, `\ul`, `\cf` | — | **applies** |
| `\caps` (and `\scaps`, `\strike`, `\lang`: no default) | **applies** | **applies** |
| `\f` | — | **—** |

**The font face reaches a run from nowhere**, and that is a separate mechanism rather than the
same one: `\deff`'s name is put into the importer's default character state when the font table
closes (`RTFDocumentImpl::beforePopState`, `rtfdocumentimpl.cxx`:2494-2500) and every group state
is copied from that state, so a run carries the face as *direct* formatting no style can beat.
Measured on a document whose `\deff2`, whose style's `\f1` and whose `Times New Roman` default are
three distinguishable faces — Liberation Mono, Liberation Sans and Liberation Serif —
26.2.4.2 answers `\deff`'s in all four arms, with `\plain` and without (`genextra.py`'s `deff-*`).

### 3. A `\sbasedon` naming a style the sheet has not reached yet is not a parent

`\sbasedon` is resolved to a style **name** where it stands —
`pIntValue = new RTFValue(getStyleName(nParam))`, `rtfdispatchvalue.cxx`:131-134 — and a style not
yet read has no name to give, so `StyleSheetTable` never calls `setParentStyle`
(`StyleSheetTable.cxx`:1156-1170, guarded on `!pEntry->m_sBaseStyleIdentifier.isEmpty()`).
Measured: a child declared before its parent takes none of its centring
(`chain-childfirst`), while an unrelated entry between the two changes nothing (`chain-gap`).
**Reach 20 of the 338**, censused by `census.py`.

### 4. `\htmautsp` is opt-in for RTF, and only until the document's first run

`SettingsTable`'s constructor sets `m_bDoNotUseHTMLParagraphAutoSpacing` **true** for an RTF
import outright — *"HTML paragraph auto-spacing is opt-in for RTF, opt-out for OOXML"*,
`sw/source/writerfilter/dmapper/SettingsTable.cxx`:118-126 — so an RTF that states nothing **adds**
two adjacent spacings and `\htmautsp` (`rtfdispatchflag.cxx`:1354-1357) clears the flag so the
larger wins. Both halves of that were already right here: `sum-*`, four probes, 4 of 4 before and
after.

What was not right is **when the word can still be read**. `RTFDocumentImpl::checkFirstRun` calls
`outputSettingsTable` (`rtfdocumentimpl.cxx`:414-435), once, at the document's first run, so a
`\htmautsp` written after that is stored in an `m_aSettingsTableSprms` nobody reads again — which
`rtfsprm.cxx`:238-241 records in a comment of its own (*"\htmautsp arrives after the style table,
so only the non-style value is correct"*). Measured, four probes, `htm-*`:

| where `\htmautsp` is written | 26.2.4.2 |
|---|---|
| absent | the spacings add, 61.80 pt |
| before any content | the larger wins, 37.80 pt |
| after a body paragraph | **the spacings add** — the word is too late |
| after a `{\header}` group | the larger wins — a substream is not the first run |

**`checkFirstRun` has nine callers and one of them is `resolvePict`** (`rtfdocumentimpl.cxx`:1296),
so a picture closes the window as a word of text does — but only a body one. `genpicture.py` puts
`150-5370-10H.rtf`'s own `{\*\listtable{\*\listpicture{\*\shppict{\pict …}}}}` before a
`\htmautsp` and 26.2.4.2 **still honours the word**, while a bare `{\pict}` in a paragraph in the
same position makes it too late. 4 of 4.

**Reach: 308 of the 338 state `\htmautsp`, exactly once each** (`census.py`).

## What moved, and what each rule was worth

Measured by sweeping the column at four intermediate states rather than reasoning about them:

| | `match` | page-exact | Σ\|Δ pages\| | `150-5370-10H` |
|---|---:|---:|---:|---|
| base | 257 | 276 | 436 | 651 / 746 |
| + the paragraph half, inherited only | 256 | 276 | 372 | 700 |
| + the `\htmautsp` window, + the named style's statement replaced by the default | 256 | 276 | 372 | 700 |
| + the character half, + the forward `\sbasedon` | **259** | 278 | 363 | 702 |
| + a body picture ends the window | **258** | **278** | **363** | 702 |

Two things in that table are worth carrying. **The second row and the third are byte-identical over
all 338 renderings**, which is not a mistake: on this column the `\htmautsp` window fires nowhere
(every document that states the word states it before its first body content) and the default-replaces
rule and the inherited-only rule differ only where a style states a property its parent also states.
And **the last row costs a verdict and is kept anyway**, because `p-bodypict` measures it and the
document it costs, `120509coss`, is one page over rather than one page short — the collapse was
masking a different defect there rather than curing one.

`150-5370-10H` is the row the brief named first and it is the round's clearest witness. Its
`\s3155 Centered` is `\qc\sb480\sa120\keepn\caps\b` and its `\s3274 Centered bold KWN` is
`\sbasedon3155` and states nothing, so every one of its section headings was drawn flush left with
6 pt above it where 26.2.4.2 centres it with 24. Its `\s1 heading 1` is `\qc\sb720\sa720\keepn` and
its `\s2 heading 2` is `\sbasedon1` and states nothing, so every `Item P-nnn` heading lost 36 pt.
Page 240 of the reference and the matching page of ours now agree line for line from `Part 5` down
to `table below.`, at 87.92 pt between the first two where we drew 51.92.

## What this brief got wrong

**"The worst rows are all short in the same direction … something makes our pages hold more than
the reference's."** The direction is right and the singular is not. The 60 pagination rows are at
least three things, and the two largest are not the same thing at all:

- **`150-5370-10H` and `AC-150-5370-10G` are ours**, and they are this round: dropped paragraph
  spacing and a dropped alignment, worth 51 and 21 pages.
- **The two Holdover Tables are the reference's own RTF import, and our `.rtf` agrees with its
  `.odt`.** See below. Together they are 128 of the column's 363 remaining pages of error.
- **`195584360` (22 / 51) is a third thing again** and this round does not move it.

**"The `.odt` spelling is a control … both spellings go through the same XRay.Office layout engine
and only the reader differs."** True, and the brief's own caution against assuming a shared cause
is the more useful half. On `FAA 2025-26 Holdover Tables` the reference draws its `.odt` in 167
pages and its `.rtf` in 233 — a 40% difference on bytes it exported itself — while our two
spellings are 168 and 169. The `.odt` column is a control on *our* engine and not on the
reference's.

**And this round's own first probe was wrong in the same way a brief can be.** `gen.py`'s `\s0`
states `\fs22`, so every paragraph naming a style was drawn at 11 pt and every bare one at 12, and
the resulting table read as "the space before is 6 pt bigger in five cases and not in six others".
It is not: the six were a line-height difference. **A probe whose control differs from its cases in
a second property measures the sum of the two**, and the fix was to give `\s0` nothing but `\ql`
and to score every case against *its own side's* control rather than against the other side's
(`measurematrix.py`'s comment says so, because the 2.8 pt ragged-right constant would otherwise
read as a defect on all 53).

## What round 77 got wrong, and it is the cause rather than the effect

`RtfStyleFormatting`'s own remarks said a style's paragraph formatting *cannot* be resolved,
because `StyleSheetTable::ConvertStyleName` (`StyleSheetTable.cxx`:1620-1660) maps an RTF style
**name** onto Writer's built-in style of that name and a pool style brings its own vertical
spacing. That mapping is real and it is not what suppresses the spacing:

- A style called `Centered` maps onto no pool style at all, and its own `\sb480` is dropped just
  the same as `heading 5`'s.
- The identical `\sb480` one level up is honoured, **including for a pool-named style**:
  `{\s10 …\qc\sb480\sa480 Centered;}{\s5\sbasedon10\snext0 heading 5;}` draws centred with 24 pt
  above it. The pool name does not break the RTF chain (`gen.py`'s `pool-h5-parent`).

The measured consequence of the wrong cause was the right decision for the wrong reason — the
round refuted "carry the whole paragraph half" and concluded "carry none of it", where the
distinction it needed was between the named style and its ancestors. **328 of the 338 hold a
paragraph style that inherits an alignment or a space before it does not state itself**
(`census.py`), so the half that was dropped is the half nearly every document uses.

The same round's *character* rule is also only half right: it resolves the character half over the
whole chain, and the named style's own `\fs`, `\b`, `\i`, `\ul` and `\cf` do not reach the
paragraph, while its `\f` reaches nothing anywhere. `RtfStyleFormattingTests` asserted the face
rule in three tests and they are rewritten here.

## What is left, and where its seat is

**`150-5370-10H` is 702 against 746 and the whole of the residue is `\htmautsp`.** Deleting the
word from the file — one `bytes.replace` — makes this tree draw **746 pages, the reference's count
exactly**. So the reference is *not* honouring the word on that document and we are. What we do
not have is why, and the measurement is sharper than the guess:

- The word is at byte 266199 of 5159891, and **every one of the document's body paragraphs comes
  after it**. Rendering `head + document[178110:266208] + two spaced paragraphs` — the document's
  own bytes up to and including the word — 26.2.4.2 **collapses**, exactly as we do. So the
  trigger is not in the 88 KB before the word.
- On the whole document 26.2.4.2 sums: its page 240 puts `Part 5` at 72.02 and `Item P-304` at
  161.13, which is one 14 pt line plus `\sa720` plus `\sb720`, not plus their maximum.
- The list-table picture that opens that 88 KB is *not* the trigger (`p-listpicture`).

So something after the word makes the reference behave as though the settings table had already
been sent, and `checkFirstRun`'s eight non-picture callers — `rtfdispatchdestination.cxx`:264 and
:405, `rtfdispatchflag.cxx`:893 and :1103, `rtfdispatchsymbol.cxx`:133, :250, :518 and :574 — are
the place to look. Reach is 308 of 338 documents, so this is the column's largest single lever.

**The two Holdover Tables are 128 pages of the residue and the defect is the reference's, not
ours — but there is a 6 pt of ours underneath it.** `FAA 2025-26 Holdover Tables` is 62 holdover
tables, each started by a `\pagebb` heading. Counting pages between successive `TABLE n:`
headings:

| | pages | 1 page per table | 2 pages | 3+ |
|---|---:|---:|---:|---:|
| 26.2.4.2, `.rtf` | 233 | 21 | **35** | 5 |
| 26.2.4.2, `.odt` | 167 | 50 | 8 | 3 |
| ours, `.rtf` | 169 | 50 | 7 | 4 |

The reference's second page carries **no table at all**: its only content is one bulleted paragraph
with no text — `` at x = 75.7 — and its `get_drawings()` is empty. All four renderings draw
the same **270** bullets; what differs is how many of them land on a page of their own: 70 in the
reference's `.rtf` against 12 in its own `.odt` and 13 in ours. So the reference's RTF table is
just tall enough to leave no room for the empty list item that follows it, and ours is not: on the
matching pages our last text line sits at 532.8 and the reference's at 538.1. **6 pt of table
height, repeated 35 times**, is the whole of 128 pages, and it is below the table rather than in
it — the cell-border rectangles agree to 0.0 pt down to y = 325.5 and the divergence is in the
notes underneath.

**Tab stops are measured and not carried.** `tab-inh` puts the word after a tab at 288.1 pt where
this tree puts it at 108.1: an inherited `\tx` reaches the paragraph and a named style's own does
not, exactly as the alignment does. It is left because it is horizontal rather than paginating,
and because the reader's tab list is built from `\tx` plus a pending alignment and leader rather
than from a single control word, so recording it in `RtfStyleFormatting` is a larger change than
the three properties this round added.

**And `\keepn` from `\s0` to a paragraph that names no style is unmeasured.** The matrix's
`zero-keepn` cannot see a keep-with-next and the `keep45-*` family does not have a
paragraph-names-nothing arm. The tree treats `\keepn` as resolving over the whole chain including
style zero, which is what the three arms that *are* measured show; a document where `\s0` states
`\keepn` would decide it and none of the 338 does.

## Files

| | |
|---|---|
| `gen.py`, `measure.py` | the round's first probe family — `direct-*`, `chain-*`, `pool-*` — which refuted round 77's stated cause and whose own `\s0` confound is described above |
| `genmatrix.py`, `measurematrix.py`, `matrix-after.txt` | 53 probes: thirteen paragraph properties × four positions in the chain |
| `genchar.py`, `measurechar.py`, `char-after.txt` | 29 probes: seven character properties × four positions |
| `genextra.py`, `measureextra.py`, `extra-after.txt` | 34 probes: the chain, `\keepn`, the summing control, the `\htmautsp` window, and the `\deff` discriminator |
| `genpicture.py`, `picture-after.txt` | 4 probes: which pictures end the settings window |
| `render.sh` | renders a probe directory both ways, one `soffice` profile per document |
| `sweep-ours.sh`, `score.py`, `compare.py` | our half of one extension of the converted corpus, scored by `batch-check.sh`'s rule, and two columns compared |
| `sweep-orig-words.sh`, `bytediff.py` | the original words track, and a date-masked byte comparison of two banks |
| `base-rtf.tsv`, `after-rtf.tsv`, `column.txt` | the column before and after |
| `after1`–`after4-rtf.tsv` | the four intermediate states the per-rule table rests on |
| `pagegeom.py`, `lines.py`, `ypos.py`, `align.py` | per-page geometry, line and character totals, one page's line origins, and the drift curve |
| `census.py`, `census.txt` | the reach of the three rules over the 338 |

## Reproducing

```sh
export TMPDIR=/abs/tmp                      # absolute: a relative one makes soffice hang forever
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
R=dotnet/probes/rtf-resid-r80

python3 $R/genmatrix.py /abs/matrix && $R/render.sh /abs/matrix /abs/matrixout
python3 $R/measurematrix.py /abs/matrixout          # 53 of 53
python3 $R/genchar.py    /abs/char   && $R/render.sh /abs/char   /abs/charout
python3 $R/measurechar.py /abs/charout              # 29 of 29
python3 $R/genextra.py   /abs/extra  && $R/render.sh /abs/extra  /abs/extraout
python3 $R/measureextra.py /abs/extraout            # 34 of 34
python3 $R/genpicture.py /abs/pic    && $R/render.sh /abs/pic    /abs/picout
python3 $R/measure.py     /abs/picout               # 4 of 4

$R/sweep-ours.sh rtf /abs/rtf-after 2
python3 $R/score.py /abs/rtf-after /home/user/gate-odf-r78/ref rtf > /abs/after.tsv
python3 $R/compare.py $R/base-rtf.tsv /abs/after.tsv
python3 $R/census.py /home/user/corpus-odf/words
```
