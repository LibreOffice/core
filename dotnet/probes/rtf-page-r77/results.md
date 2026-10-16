# The `.rtf` column at `584ba83f9`: where a run's face comes from

## Environment

    ours    XRay.Office.Cli @ 584ba83f9 (base) and @ this commit, built in this worktree
    ref     /opt/libreoffice26.2/program/soffice -- LibreOffice 26.2.4.2
            reused from the banked gate /home/user/gate-odf-r76/ref, which was rendered by
            batch-check.sh with REF_SOFFICE pointed at that binary. The bank is at c6e730d90,
            and `git diff c6e730d90 584ba83f9 -- dotnet/src dotnet/tools` is empty, so it is a
            bank at this round's base tree.
    corpus  /home/user/corpus-odf/words/**/rtf -- 338 files, 26.2.4.2's own RTF export of the
            corpus's words track; 336 have a reference rendering and are scoreable.
    fonts   system fontconfig. `fc-list | grep -ci condensed` is 0 and
            /opt/libreoffice26.2/share/fonts/truetype still holds eight DejaVu*Condensed faces
            -- see "What the reference can draw and we cannot" below.
    rule    batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
            max(2%, 15), then unembedded fonts. `score.py` here applies that rule to a pair of
            rendered PDFs; run against the banked halves it reproduces batch-check.sh's own
            `.rtf` verdict on 336 of 336.

Only our half was re-rendered. The diff is confined to `XRay.Office.WordProcessing/Rtf`, which
`soffice` cannot reach, so the reference half of the bank is the same bytes throughout.

## The column

| | base | after | of |
|---|---:|---:|---:|
| gate `match` | **243** | **257** | 336 |
| page-exact | 265 | **276** | 336 |
| total \|Δ pages\| | 510 | **436** | |
| total \|Δ alphanumeric characters\| | 169334 | **164623** | |

18 documents gain the verdict and 4 lose it. The four losses are all one page, and two of them
(`092_Business_Case_Template_Convenient_Format`, `02_mcar_part-2_and_IS_v2.10`) are one page
*short* on a document whose text is now much closer to the reference's — see "The four losses".

**The original `.doc`/`.docx` words track does not move.** All **338** of it, re-rendered with
this binary and compared byte for byte against `/home/user/gate-orig-r76/ours` with the PDF
creation date masked: **338 identical, 0 different.** `sweep-orig-words.sh` is the script. The
corpus holds no `.rtf` at all, so this is what the confinement of the diff predicts, and it is
measured rather than argued.

## What this brief got wrong

**"The +1 bucket is the sharpest thing on the board."** It is not one thing. Its 19 documents
(the brief says 16, counting only those whose `.odt` twin is exact) carry at least six unrelated
causes, and `diverge2.py` -- which compares the two renderings page by page as a *multiset of
words*, so that a header drawn first does not read as a divergence -- separates them:

| what `diverge2.py` shows on the first differing page | documents |
|---|---:|
| becomes page-exact after this round | **8** |
| a trailing page carrying no text at all, and only ours has it | 3 (`003`, `042_Visual_Product_Roadmap`, `May 25 bulletin`) |
| the reference embeds a face this machine has not got | 4 (`1447`, `EHEST-SMS`, `UG.CAO.00006`, `template---tpr`) |
| a volatile `DATE` field, the two halves rendered a day apart | 1 (`system_design__technical_architecture_template`) |
| hyphenation: ours `pre-engineered`, the reference `pre-` + `engineered` | 1 (`JEMIT_Template`) |
| the only difference on the page is a `NUMPAGES` field or a filename field | 3 (`AWR OPS-AOC 044`, `EHEST-SMS`, `361400CSLegislation1RF01PUBLIC1`) |
| a TOC leader run, a table split, a wide first-page frame | the rest |

The classes overlap -- `EHEST-SMS` is in three of them -- which is the point: `plus-one-after.txt`
lists all 19 rows and where each went, and eight of them became page-exact for one reason while the
other eleven did not move for eleven others.

**"A trailing empty page … would look like this."** Measured across the whole column with
`lastpage.py`: 17 of the 336 `.rtf` renderings end in a textless page and so do 17 of the
reference's, and **ours exceeds the reference's on 3 documents**, of which all three are in the
+1 bucket. It is a real class and it is 3 of 19, not the mechanism.

**"Our body starts 12 pt higher on every page — reference median 93.0 pt, ours 81.0, which is
`\margt1620` exactly … Unexplained, and it is a good place to start."** Two thirds of that was
an instrument artefact and the residue is not a body origin. On `Annex-10` page 3 the *cell
background rectangles* -- the table states `\clcbpat15`, so every cell paints one and the row
boundaries are readable off the PDF's path operators -- give:

    ours (base)   first row 81.00 .. 104.45   (23.45 pt: two lines)
    reference     first row 93.70 .. 105.35   (11.65 pt: one line)
    both          second row starts 104.45 / 105.40

The repeated header row reads `GROUP 1 AEROPLANES`, centred in a 109.45 pt measure. 26.2.4.2
draws it 95.23 pt wide in **Carlito-Bold** and fits it; this tree drew it in
**LiberationSerif-Bold**, 114.7 pt wide, and wrapped it. So *most* of the apparent 12 pt was our
own extra line, and the two effects nearly cancelled, which is why a first-text-y census reads a
clean constant. **The 12.70 pt that is left is not a body origin either**: it is an empty
paragraph at the head of each of that document's 156 sections, which we lost because we were
swallowing the section break as well. Closed by the second defect below.

## The rule, and the C++ that states it

Every citation below was opened and read in `/home/user/libreoffice-core` at the line quoted.

**1. `\deffN` is the default character state's face, not font zero.**
`RTFDocumentImpl::beforePopState` (`sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx`:2494-2500)
puts `m_aFontNames[getFontIndex(m_nDefaultFontIndex)]` into `m_aDefaultState`'s character sprms as
`LN_CT_Fonts_ascii` when the font table closes. Nothing else consumes `\deff`.

**Reach: 215 of the 338 converted `.rtf` declare a `\deff` naming a different face from their
`\f0`** (`deffcensus.py`), and `\f0` is `Times New Roman` in every file LibreOffice writes, so the
wrong answer was Liberation Serif almost everywhere.

**2. `\plain` restores that default state, including the face and the size.**
`rtfdispatchflag.cxx`:575-583:
`m_aStates.top().getCharacterSprms() = getDefaultState().getCharacterSprms()` plus
`setCurrentEncoding(getEncoding(getFontIndex(m_nDefaultFontIndex)))`. Our `ResetCharacter` cleared
the toggles and left `FontIndex` and `FontSizeHalfPoints` where the last run put them. The witness
is a table row whose empty cells are written `\pard\plain …\f0\fs18 \cell`: that `\f0\fs18` then
set the face and the size of every paragraph after it, which on
`092_Business_Case_Template_Convenient_Format` was **3255 characters drawn at 14 pt that the
reference draws at 10**.

**3. A paragraph style's character formatting reaches a run that names none, through `\sbasedon`.**
`RTFDocumentImpl::getProperties` (`rtfdocumentimpl.cxx`:534-637) merges the style entry's
properties under the paragraph's own; `lcl_copyFlatten` (:490-514) flattens a style's `rPr` into
that list, and the comment at :616-618 is *"Take paragraph style into account for character
properties as well, as paragraph style may contain character properties."*

**4. `\pard` selects style zero.** `rtfdispatchflag.cxx`:600-614 -- *"Reset currently selected
paragraph style as well. By default the style with index 0 is applied"* -- and `getProperties`
falls back to `EnsureDefaultStyle` (:518-533) when `getCurrentStyleIndex()` is 0. So a paragraph
that names no style is not unstyled: it is in `\s0`, and takes `\s0`'s face. `\deff` is the floor
under *that*.

**It is often said that RTF's stylesheet is decorative**, on the grounds that a writer restates a
paragraph's effective formatting inline after the `\s`. `RtfStyles`'s own doc comment said exactly
that. **LibreOffice's export does not**: it writes the *difference* from the style. `Annex-10`'s
table header cell is `\pard\plain \s24\li85\lin85\intbl\sl-234\slmult0\li379\lin379{\cf8\fs20\b
TC Holder}` -- the indent, the spacing, the colour, the size and the weight, and no font, because
`\s24 Table Paragraph` is `\sbasedon0` and `\s0 Normal` ends `\loch\f5\fs22`, where `\f5` is
Calibri.

## What that moved

`Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.rtf`, the brief's own
witness, goes from **169 pages to 148 against the reference's 148**, at 189432 alphanumeric
characters on both sides before and after. Its embedded font set goes from
`DejaVuSerif-Bold, LiberationSerif-Bold, Carlito-Bold, Carlito-Regular, LiberationSerif,
LiberationSerif-Italic, Carlito-Italic` to
`DejaVuSerif-Bold, Carlito-Bold, Carlito-Regular, Carlito-Italic` -- which is the reference's set
exactly.

The rest of the column moves with it: page-exact 265 → 273 and gate `match` 243 → **254** for this
rule alone.

## The paragraph half of the same cascade is refuted, and that is measured

The first cut carried the style's *paragraph* properties as well -- indents, spacing, alignment,
keeps -- resolved through the same `\sbasedon` chain. Over the same 336 documents that scores
**247** where the character half alone scores **254**, against a base of 243.

The reason is in the same file. `StyleSheetTable::ConvertStyleName`
(`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:1620-1660) maps an RTF style *name* onto
Writer's built-in style of that name -- `{ "heading 5", "Heading 5" }` -- and `setParentStyle`
(:1156-1170) only sets the parent, so a pool style's own vertical spacing is not the RTF chain's.
Measured on `DEP2008-1900.rtf`, whose `\s5 heading 5` is `\sbasedon0` and whose `\s0` states
`\sa160`: 26.2.4.2 draws **12.60 pt** after `Cyclists at Hanslope Park`, which is one line pitch
and no space at all, and carrying the inherited `\sa160` drew 20.90.

So `RtfStyleFormatting` is character-only on purpose, and the type's remarks say so.

## And a second defect, found on the way

**A `\sect` written straight after a table does not break the page.** RTF marks no end to a
table -- a paragraph back at the enclosing level closes one -- so `…\row\pard \sect\sectd…\sbkpage`
arrives while the table is still open, and `FinishTable` stamps the finished table with the
section index it reads at *that* moment, which is the next one. Every block on both sides of the
break then claims one section and the page break never happens.

Measured on a two-section probe (`t4.rtf` in the write-up's own words: two identical one-row
tables with `\pard \sect…\sbkpage` between them): 26.2.4.2 draws **two** pages with the second
table's first cell at y = 94.0 pt, and this tree drew **one**, with the second table at 105.5.
`Annex-10` is 156 sections written in exactly that shape.

`\sect` now closes any open table before it increments the section index. Column 254 → **257**,
five gained and two lost.

## What the reference can draw and we cannot, and it is the environment

`/opt/libreoffice26.2/share/fonts/truetype` still holds `DejaVuSansCondensed-{,Bold,Oblique,
BoldOblique}.ttf` and `DejaVuSerifCondensed-{,Bold,Italic,BoldItalic}.ttf`. **This system has no
condensed face at all** (`fc-list | grep -ci condensed` → 0), and `fonts-dejavu-core` carries no
DejaVu Serif italic, so where a fallback run wants italic DejaVu Serif the tarball answers
`DejaVuSerifCondensed-Italic` and a stock machine -- and XRay.Office -- synthesise an oblique of
`DejaVuSerif`, which is about **15% wider**.

Censused with `faces.py` over the four converted columns, counting documents whose *reference*
PDF embeds a `Condensed` face that ours does not:

| column | documents | of |
|---|---:|---:|
| `.rtf` | 39 | 336 |
| `.odt` | 45 | 338 |
| `.odp` | 38 | 312 |
| `.ods` | 8 | 307 |

**33 of the 39 `.rtf` name no condensed or narrow family anywhere in their own font table**, so
they reach the face purely through fallback. This is the `LiberationSansNarrow` trap the working
notes already record, one directory over and one recipe short: the `mv $D/{Carlito,Caladea,
Liberation,DejaVu}*.ttf` in the notes does match these files, and they are present anyway.

`1447.rtf` is the clean witness and it is **not** a defect of ours: its `\f5` is `Times`, which
nothing here has; 26.2.4.2 sets `THE (as appropriate) MEA/MVA/MOCA/MIA IN YOUR AREA IS
(altitude), ..."` in 422.00 pt of DejaVuSerifCondensed-Italic and fits it on one line, and this
tree sets the same string in DejaVuSerif, where the part that fits the 432 pt measure is
390.61 pt and `(altitude), ..."` goes to a second line -- one line, one page.

**Nothing was moved aside.** Two other rounds are live against this reference bank, and changing
the shared font set mid-round would invalidate their measurements as well as this one's. It is
reported rather than acted on.

## The four losses

| document | base | after | ref | |
|---|---:|---:|---:|---|
| `092_Business_Case_Template_Convenient_Format` | 8 | 7 | 8 | its text is now right: 3255 characters move from 14 pt to 10, and its size histogram becomes the reference's (10 pt: 9892 against 9946; 14 pt: 17 against 17). One page short. |
| `02_mcar_part-2_and_IS_v2.10` | 318 | 317 | 318 | the reference draws it partly in `DejaVuSerifCondensed-Italic` and `Carlito-Bold`, neither of which we can resolve here. Condensed class. |
| `150_5300_13_chg12` | 33 | 34 | 33 | the section-break fix; one page over. |
| `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX` | 21 | 22 | 21 | the same. |

## What is left, and what is closed

**Nothing of the 12.70 pt offset is left, and the second defect is what closed it.** With the face
right but the section break still swallowed, our table began at y = 81.00 on every page -- exactly
`\margtsxn1620` -- against the reference's 93.70. Each of that document's 156 sections begins
`…{\footer…}\pard\plain \s19\f0 {\f0\par}\trowd…`, an empty paragraph in `\s19 Body Text`
(`\sbasedon0`, `\fs22` -- 11 pt, whose line is 12.7) before the table; with the section break not
firing, that paragraph stayed on the previous page and the table began at the margin. Once `\sect`
closes the open table, our origin is **93.65** against the reference's **93.70** on pages 3, 4 and 5
alike.

**`SwTabFrame::Split` is not the seat, and the brief's own suggestion should be dropped.** The
suggestion came from a round that measured page *boundaries*; the `.rtf` column's largest single
mover turned out to be a font, and the second a section break. Nothing in this round needed the
table splitter, and `probes/rtf-shape-r73` had already refuted the row-splitting hypothesis by
synthetic.

**The heavily short documents are barely touched and are the next thing.** `150-5370-10H` is
642 pages against 746 at base and **651** after; `24-25_FAA_Holdover_Tables` 143 → **162** against
223; `FAA 2025-26 Holdover Tables` 156 → **166** against 233. All three move in the right
direction and all three are still tens of pages short; they lose *lines*, not pages.

## Files

| | |
|---|---|
| `score.py` | batch-check.sh's verdict rule over a pair of rendered banks |
| `sweep-ours.sh` | renders our half of one extension of `/home/user/corpus-odf` |
| `sweep-orig-words.sh` | renders the original corpus's words track, named as `gate-orig-r76` names it |
| `pagegeom.py`, `diverge.py`, `diverge2.py` | per-page geometry and the first divergent page |
| `lastpage.py`, `lines.py`, `extent.py` | trailing-empty-page, line-count and text-extent censuses |
| `faces.py`, `deffcensus.py` | the face census, and the `\deff`-against-`\f0` census |
| `fresh-base-rtf.tsv`, `after2-rtf.tsv` | the scored column before and after |
| `charonly-rtf.tsv`, `after-rtf.tsv` | the two intermediate states the paragraph-half measurement rests on |
| `plus-one-after.txt` | where each of the base's 19 `+1` rows went |
