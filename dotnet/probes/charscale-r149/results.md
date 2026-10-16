# Round 149 — the WW8 and RTF thirds of O101 (`sprmCCharScale`, `\charscalex`)

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2**
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. `/usr/bin/soffice` is 24.2.7.2 and is not the target;
every invocation below names the full path.
**Tree read for source** `/home/user/libreoffice-core`, `configure.ac` 27.2.0.0.alpha0+ — **not**
the reference binary's source.
Every claim is marked **[src]** (a reading of that checkout) or **[bin]** (a measurement of
26.2.4.2's own output).

O101: `RES_CHRATR_SCALEW` / `PROP_CharScaleWidth` — `w:rPr/w:w` in DOCX, `style:text-scale` in ODF,
`sprmCCharScale` in WW8, `\charscalex` in RTF, applied by VCL as `Font::SetAverageFontWidth`. The
DOCX third closed in round 143 and the ODF third in round 148; this round measures the other two.

*(written incrementally; container restarts have killed agents in this session)*

---

## 0. What is new here as an instrument: the `.rtf` column exists now

`/home/user/corpus-odf` held `odt` and `ods` and no `rtf`, so round 145's `\charscalex` census
returned **0 occurrences and 0 base-rate tokens** — the tell that a census has measured nothing.
`convert-rtf.sh` builds the missing column: 26.2.4.2's own `--convert-to rtf` of every
words-track document named by `MANIFEST.tsv`, into `/home/user/corpus-odf/rtf`, **outside the
repository**.

```
337 of 337 ok, 0 failed, 104 s wall at 6 workers, 259 MB
```

`convert-rtf.log` is the per-document record. Three things it has to get right and does:

- **a unique stem per output.** `soffice --convert-to` names its output after the input stem
  alone, so two inputs sharing a stem silently become one output. Each input is copied to a work
  directory as `<md5-12-of-absolute-path>-<stem>.<ext>`, which is the rule
  `/home/user/corpus-odf/odt` was built with — so the two columns line up document for document
  and `crosstab.py` can join them.
- **a bounded `timeout -k 30 300`.** `soffice` execs `oosplash`, which ignores SIGTERM; a plain
  `timeout` waits forever.
- **one profile per worker** (here one per document, inside the work directory, removed with it).

---

## A. The WW8 third — `sprmCCharScale`

### A.1 [src] The sprm, its operand and its semantics

| | |
|---|---|
| id | **0x4852** — `using CCharScale = sprmChr<0x52, 0, SPRA::operand_2b_2>;` (`sw/source/filter/ww8/sprmids.hxx`:335) |
| operand | `SPRA::operand_2b_2` — **two bytes**, read unsigned |
| where it is dispatched | `{NS_sprm::CCharScale::val, &SwWW8ImplReader::Read_ScaleWidth}` (`ww8par6.cxx`:6131); the scanner's own table row is `InfoRow<NS_sprm::CCharScale>()` (`ww8scan.cxx`:593) |
| handler | `SwWW8ImplReader::Read_ScaleWidth` (`ww8par6.cxx`:4985-4997) |
| item | `SvxCharScaleWidthItem(nVal, RES_CHRATR_SCALEW)` — the same item `w:w`, `\charscalex` and `style:text-scale` all land in |

```cpp
void SwWW8ImplReader::Read_ScaleWidth( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 2)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_SCALEW );
    else
    {
        sal_uInt16 nVal = SVBT16ToUInt16( pData );
        //The number must be between 1 and 600
        if (nVal < 1 || nVal > 600)
            nVal = 100;
        NewAttr( SvxCharScaleWidthItem( nVal, RES_CHRATR_SCALEW ) );
    }
}
```

Three answers come out of that:

- **The default is 100.** Nothing states it and the item's own default is 100 —
  `SvxCharScaleWidthItem::CreateDefault` returns `SvxCharScaleWidthItem(100, …)`
  (`editeng/source/items/textitem.cxx`:112), and the pool row is the same
  (`eerdll.cxx`:131).
- **Out of range is 100, not the stated value and not "ignore the sprm".** `< 1 || > 600 → 100`.
  **So WW8 agrees with the DOCX path exactly**, and the agreement is closer than the seat's
  wording suggests: `DomainMapper`'s `LN_EG_RPrBase_w` (`DomainMapper.cxx`:2485-2497) does not
  ignore an out-of-range value either — it *inserts 100*, under the comment *"ST_TextScale must
  fall between 1% and 600% according to spec, otherwise resets to 100% according to experience"*.
  Inserting 100 and declining to read are the same drawn result, which is what
  `WordParagraphFormats.WidthOf` already does (`>= 1 and <= 600 ? percent : Natural`). A zero
  would collapse the run to nothing and neither filter allows it.
- **A one-byte (`nLen < 2`) operand is not a value at all**: it pops the attribute off the control
  stack, i.e. it *ends* the scaled stretch. That is the sprm-stack close, not a percentage.

### A.2 [bin] The reference, measured on authored `.doc`

`.doc` cannot be authored directly here, so each arm is `26.2.4.2`'s own `--convert-to doc` of the
one-control-word `.rtf` probes in §B.4 — and **the sprm is verified to be in the file before
anything is measured from it**, which is the difference between a measurement and an absence
mistaken for agreement. `dump-chpx.py` (the FKP walk of `probes/charscale-r145/census-ww8.py`,
reused) prints the CHPX of every probe:

```
none.doc    chpx 1   sprmCCharScale NONE
s60.doc     chpx 1   sprmCCharScale fc2048..2078=60(2b)
s99.doc     chpx 1   sprmCCharScale fc2048..2078=99(2b)
s130.doc    chpx 1   sprmCCharScale fc2048..2078=130(2b)
```

**The out-of-range arms cannot be authored that way** — the RTF *import* has already replaced them
with 100, so a probe built that route measures the importer twice and the WW8 reader not at all.
`patch-chpx.py` rewrites the two operand bytes of the one CHPX in place (`olefile` `write_stream`,
every other byte of the file untouched) and `dump-chpx.py` confirms each patch landed.

Each arm is one right-aligned line of `Hamburgefonstiv` at 12 pt Liberation Serif, and the drawn
width is read from **the line's own origin** (`readline.py`, reused from round 148) rather than
from glyph boxes, which are quantised to whole thousandths of an em.

| arm | operand | 26.2.4.2 width | ratio to unscaled |
|---|---|---:|---:|
| `none` | — | 83.712 pt | 1.00000 |
| `s100` | 100 | 83.712 | 1.00000 |
| `s60` | 60 | 50.205 | **0.59974** |
| `s99` | 99 | 82.594 | **0.98665** |
| `s130` | 130 | 108.810 | **1.29981** |
| `p1` | 1 (patched) | 0.650 | 0.00776 |
| `p600` | 600 (patched, short text) | 139.824 / 23.280 | **6.00621** |
| `p601` | 601 (patched) | 83.712 | 1.00000 |
| `p0` | 0 (patched) | 83.712 | 1.00000 |
| `p900` | 900 (patched) | 83.712 | 1.00000 |
| `p65535` | 65535 (patched) | 83.712 | 1.00000 |

`doc-reference.txt`, `short-doc-reference.txt`, `doc-sprm.txt`.

- **The clamp is exact at the boundary**: 600 scales and **601 does not**, so the source's
  `nVal < 1 || nVal > 600` is the reference's behaviour and not merely this checkout's.
- **0, 900 and 65535 draw the unscaled width**, so an out-of-range operand is the identity — the
  DOCX rule, unchanged.
- **The 99 arm is 0.98665, the same number to five places as the RTF arm below and as round 148's
  ODF arm**, and nearer the truncated 0.98750 than the stated 0.99000. So `TextWidthScale`'s twip
  grid transfers to WW8 unchanged and must not be re-derived; the item is applied once, in the
  layout, and the four filters differ only in how they spell it.
- `p600` had to be measured on a three-letter probe: six times `Hamburgefonstiv` is wider than the
  measure and wraps, and a wrapped arm has two spans, which `readline.py` refuses rather than
  averaging. An instrument that refuses is the point of it.

### A.3 [bin] The census re-verified, and what the sprm paints

`census-ww8.py` re-run here over the corpus's 66 `.doc` (`census-ww8.tsv`) reproduces round 145
exactly:

```
documents 66     base CHPX runs 61366     base styles 1736
CHPX sprmCCharScale        8   (all != 100, values {99: 8}, story {body: 8})
style-level sprmCCharScale 0
documents stating it       1
raw 0x4852 byte pairs      185      <- the byte scan's upper bound, 23x the truth
```

The witness is `words/done-011/doc/AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc`.

**Cross-checked at the reference rather than trusted**: `--convert-to fodt` of that `.doc` through
26.2.4.2 states `style:text-scale="99%"` on **5** automatic styles and on nothing else — so the
reference does import the sprm, and the eight CHPX runs it covers reduce to five distinct text
styles.

**And what it paints, measured rather than censused** (`sensitivity-ww8.py`, `ww8-pagediff.txt`):
the witness rendered through 26.2.4.2 as it stands and with all eight operands rewritten from 99
to 100 and no other byte touched —

```
pages 20 vs 20
page 1   spans 53 vs 54, 34 differ
pages that differ at all: 1 of 20
```

One paragraph on page 1 rewraps — `dated 20 November 2020,` fits on its line at 99 % and does not
at 100 % — and everything below it on that page shifts. No page count moves and the other 19 pages
are identical span for span. **So the WW8 reach is one document of 66, one page of 20, and one
rewrap**, which is a real but small consequence, and it is the same *kind* of consequence round
143 measured for DOCX: a width that moves a break rather than a glyph.

---

## B. The RTF third — `\charscalex`

### B.1 The column that had to be built first

Round 145 could not census this and said so rather than quoting the zero it got. The column now
exists (§0): **337 `.rtf`, 26.2.4.2's own export of the same 337 words-track originals the `.odt`
column was made from**, 104 s, 0 failures.

**What that column is and is not.** It is a *LibreOffice-written* RTF, so what it measures is which
documents carry a character width and what the reference does with it — not what a Word-written or
hand-written RTF looks like. The corpus holds no native `.rtf` at all (`MANIFEST.tsv`: docx 272,
pptx 251, xlsx 241, doc 66, xls 64, ppt 51, xlsm 2), so this is the only instrument available and
the caveat travels with every figure below.

### B.2 [bin] The census, gross and net, with its base rate

`census-rtf.py`, `census-rtf.txt`:

```
files                         : 337
base rate, \fN run tokens     : 256927
occurrences, gross            : 1965 in 20 documents
occurrences, NON-IDENTITY     : 1520 in 17 documents
outside {\stylesheet}, gross  : 1680
outside {\stylesheet}, net    : 1467 in 12 documents
values (all) : {86:1, 90:1, 95:87, 96:4, 98:6, 99:1265, 100:445, 101:4, 102:10, 103:77,
                104:13, 105:27, 106:7, 107:6, 108:2, 112:1, 130:4, 131:5}
```

**The base rate is the point of the row.** 256 927 `\fN` tokens over 337 files is what a real RTF
corpus looks like; round 145's pass over the absent directory reported 0 occurrences *and* 0 base
rate, and only the second number tells you which of the two findings you have.

**1265 of the 1520 non-identity occurrences say 99** — the same Word condense habit the DOCX and
ODF censuses found, and the value the twip grid makes 0.98750 rather than 0.99.

**Cross-checked against round 148's `.odt` column** (`crosstab.py`, `crosstab.txt`). Both columns
are 26.2.4.2's own conversion of the same originals and both stems are the md5 of the original's
absolute path, so they join exactly:

```
documents stating a non-identity scale   rtf 17   odt 18   in both 17   rtf only 0   odt only 1
```

The one document that states it in ODF and not in RTF is
`AW-104D-RVSM-Aircraft-Approval-Checklist.pdf`, whose two `style:text-scale="105%"` sit on an
automatic text style applied to **nothing** in `content.xml` and on one used only inside
`styles.xml`. That is an exporter difference — ODF writes an unused automatic style, RTF does not —
and not a corpus difference.

### B.3 [src] The semantics, and they are the DOCX ones by construction

| | |
|---|---|
| keyword | `{ "charscalex"_ostr, { RTFControlType::VALUE, RTFKeyword::CHARSCALEX, **100** } }` (`sw/source/writerfilter/rtftok/rtftokenizer.cxx`:253) |
| dispatch | `case RTFKeyword::CHARSCALEX: nSprm = NS_ooxml::LN_EG_RPrBase_w;` (`rtfdispatchvalue.cxx`:193-195) |
| then | the same `DomainMapper` case as `w:w` — `1 <= n <= 600 ? PROP_CHAR_SCALE_WIDTH = n : 100` (`DomainMapper.cxx`:2485-2497) |

- **A bare `\charscalex` with no parameter is 100**, because the tokeniser's `VALUE` branch
  substitutes the symbol's own `defValue` when `!bParam` (`rtftokenizer.cxx`:2204-2207). That is the
  *opposite* of `\kerning`, whose bare form is RTF's default-of-zero and turns kerning off — this
  keyword carries an explicit 100 in its table row.
- **Out of range is 100**, by the same line of `DomainMapper` the DOCX path uses. So RTF is not a
  second rule: it is the DOCX rule reached through a different tokeniser, which is the round-80 and
  round-87 warning discharged rather than assumed — the importer does here do exactly what the
  specification implies, and the measurement in §B.4 is what establishes it.
- **`\plain` clears it**: `dispatchFlag`'s `PLAIN` case restores the default state's character sprms
  wholesale (`rtfdispatchflag.cxx`:575-583), so the item falls back to its own 100.
- **Inside a list level the sprm goes to the level's table sprms instead of to the run**
  (`rtfdispatchvalue.cxx`, the `Destination::LISTLEVEL` branch) — which is where the corpus's
  `{\*\listtable}` and `{\listtext}` occurrences land, and §B.5 measures that they paint nothing.
- **It is not in `getDefaultSPRM`** (`rtfsprm.cxx`:154-200), so a stylesheet entry stating it is not
  overwritten by the deduplication default the way `\sa` is (`probes/rtf-holdover-r87`).

### B.4 [bin] The reference, on one-control-word probes

`make-rtf-probe.py` writes one minimal `.rtf` per arm — `\pard\qr\plain\f0\fs24<arm>
Hamburgefonstiv\par`, one right-aligned line, one control word between the arms and nothing else —
and `render.sh` runs each through 26.2.4.2 into its own profile, refusing to report an arm whose
PDF does not exist. Widths from the line's own origin (`readline.py`), `rtf-reference.txt`:

| arm | 26.2.4.2 width | ratio | what it establishes |
|---|---:|---:|---|
| absent | 83.712 pt | 1.00000 | — |
| `\charscalex100` | 83.712 | 1.00000 | the identity is free |
| `\charscalex60` | 50.205 | **0.59974** | |
| `\charscalex99` | 82.594 | **0.98665** | the corpus's commonest value |
| `\charscalex130` | 108.810 | **1.29981** | |
| `\charscalex0` | 83.712 | 1.00000 | out of range → 100 |
| `\charscalex900` | 83.712 | 1.00000 | out of range → 100 |
| `\charscalex-50` | 83.712 | 1.00000 | negative → 100 |
| `\charscalex` (bare) | 83.712 | 1.00000 | the tokeniser's `defValue` of 100 |

**Every ratio is identical, to five places, to the WW8 arms of §A.2 and to round 148's ODF arms.**
That is the useful result and it is not a coincidence: all four filters set one item, and the
truncation happens once, in VCL, at `Font::SetAverageFontWidth` on a whole number of twips. So the
two remaining readers need `TextWidthScale` and nothing of their own.

The 99 arm again: stated 0.99000, truncated model 0.98750, **measured 0.98665** — nearer the
truncated model by a factor of four, and a reader using the percentage itself is wrong by a quarter
of a per cent on 1265 of the column's 1520 statements.

### B.5 [bin] What it paints: 11 documents of 17, 31 pages of 590

A census counts statements; this counts consequences. `sensitivity-rtf.py` renders each of the 17
documents through 26.2.4.2 twice — as it stands, and with every non-identity `\charscalex` rewritten
to `\charscalex100`, one substitution and no other byte — and `pagediff.py` compares them page by
page (`sensitivity-rtf.txt`, `rtf-pagediff.txt`).

| document | net | pages that differ |
|---|---:|---|
| `Annex-10-…-GCAA` | 1100 | 3 of 148 |
| `Regulations Governing the Status…` | 80 | 3 of 4 |
| `ESPN-R - MCF - RA - Ed1` | 94 | 6 of 46 |
| `091_Business_Case_Template…` | 54 | 3 of 5 |
| `OM template for non-complex NCC operators` | 39 | 4 of 171 |
| `AWR OPS-AOC 044 …RVSM` | 36 | 1 of 10 |
| `BID_ACKNOWLEDGEMENT_FORM_FOR_A320` | 19 | 1 of 3 |
| `ESPN-R - MCF - Manual - Ed1.0` | 14 | 4 of 42 |
| `19-06 Assistive Technology TAB Final - 508` | 10 | 1 of 8 |
| `AAC-AD-No-2021-01-Boeing-737-8/9-MAX` | 9 | 1 of 21 |
| `t_TEMPforInvProgs` | 8 | 4 of 25 |
| `SPA-06_mcar_part-6_and_IS_v2.9` | 12 | **0 of 88** |
| `SWDD-template` | 35 | **0 of 6** |
| `DRX-Ascend System Course Description` | 7 | **0 of 2** |
| `JEMIT_Template` | 1 | **0 of 4** |
| `Allegiant_Company_Profile_with_History` | 1 | **0 of 3** |
| `mde087077~283` | 1 | **0 of 4** |

```
documents whose rendering moves: 11 of 17        pages that move: 31 of 590
page counts: identical on all 17
```

- **The six that do not move are the same class round 148 found in ODF, and five are literally the
  same documents.** In those five every non-identity statement is inside `{\stylesheet}` and
  nowhere else — four on `ListLabel`/bullet **character styles** and the fifth
  (`mde087077~283`) on the paragraph style `Title (Title and Subtitles)`, which the body applies to
  nothing. A list label takes no character scale at all (`SwFont::SetDiffFnt` reads no
  `RES_CHRATR_SCALEW`; round 148 §5 measured that with a control that *does* move), so this is
  agreement rather than a gap. `SWDD-template` is the sixth and is new: its 30 body occurrences are
  `{\*\listtable}` level definitions, `{\listtext}` label groups and table-of-contents number
  groups, and none of them moves a span at 0.01 pt.
- **No page count moves anywhere.** The consequence is a break position and a line's width, which
  is exactly what the gate cannot see — the same argument as `w:pgBorders` and the worksheet fills.
- `AAC-AD-No-2021-01` is the WW8 witness of §A seen through the other filter, and the two
  measurements agree: 1 page of 21 here, 1 page of 20 there.

---

## C. The second seat both readers have — and it is a *third* one, not a second

The DOCX and ODF paths each needed the scale in two places: on the run, and in the
**varies-predicate** that decides whether a paragraph's run list survives the uniform-paragraph
shortcut. A scaled run folded into an unscaled paragraph is otherwise measured and broken at the
paragraph's width.

**Both remaining readers have that fold, and both have a second one the two closed readers do not.**
`git grep MatchesFormatting -- dotnet/src` finds exactly two definitions and both are here:

| fold | DOCX | ODF | WW8 | RTF |
|---|---|---|---|---|
| uniform-paragraph varies-predicate | `DocxLayoutSource.RunsOf` ✅ done r143 | `OdtLayoutSource` ✅ done r148 | **`DocReader.RunsOf`** (`DocReader.cs`:1189-1275, the predicate ending at `:1255`) | **`RtfReader.RunsOf`** (`RtfReader.cs`:668-732, the predicate ending at `:713`) |
| adjacent-run **merge** predicate | — none — | — none — | **`Ww8DocumentReader.MatchesFormatting`** (`Ww8DocumentReader.Layout.cs`:1609-1623) | **`RtfLayoutRun.MatchesFormatting`** (`RtfLayoutParagraph.cs`:64-76) |

The merge fold is the sharper of the two and it is invisible in a one-line probe:

- **WW8 builds its runs one character at a time** and merges each into its predecessor when
  `MatchesFormatting` says the formatting is identical (`:1586-1594`,
  `runs[^1] with { Length = … + 1 }`). A property missing from that comparison does not merely
  fail to vary the paragraph — the scaled characters are **absorbed into the unscaled run beside
  them and take its scale**, so the file's own boundary is gone before `RunsOf` is ever asked. The
  field's own remarks already say this about `Tracking` (`:257-262`, *"Unlike the two rules it
  changes the run's width, so `MatchesFormatting` compares it"*); the same sentence has to be true
  of the width.
- **RTF is the same shape for the same reason** — a producer restates `\f0\fs22` before every run
  whether or not anything changed, so unmerged restatements would break the shaping context, and
  the merge predicate is what prevents that. `RtfLayoutRun.MatchesFormatting` compares eleven
  properties and would have to compare a twelfth.

So each of the two readers needs the width in **three** places, not two: read it, keep it across
the merge, and vary the paragraph with it.

### C.1 The WW8 seat, exactly

1. **`Ww8DocumentReader.Layout.cs`**, `LayoutSprms`: a constant
   `internal const ushort CharacterScale = 0x4852;` beside `CharacterSpacing = 0x8840`, with the
   citation `sprmids.hxx`:335 / `ww8par6.cxx`:4985.
2. **the sprm switch** (`:2765-2790`, beside `case LayoutSprms.CharacterSpacing`):
   `case LayoutSprms.CharacterScale: format = format with { CharacterScale = sprm.Word };` —
   `Word`, not `SignedWord`: the operand is unsigned and the negative arm is out of range anyway.
3. **`Ww8LayoutFormat.cs`**: `public int? CharacterScale { get; init; }` beside `CharacterSpacing`,
   documenting `Read_ScaleWidth`'s 1..600-or-100 rule.
4. **a `ScaleOf(Ww8LayoutFormat)` helper** beside `TrackingOf` (`:1633`):
   `format.CharacterScale is { } n and >= 1 and <= 600 ? n : TextWidthScale.Natural` — which is the
   reference's own rule (§A.1, §A.2) and not a guard we invent, and which matches
   `WordParagraphFormats.WidthOf` word for word.
5. **`Ww8LayoutRun`** gains `int WidthPerCent = 100` and **`Ww8LayoutParagraph`** gains
   `public int WidthPerCent { get; init; } = 100` — the *mark's*, filled at `:1399` beside
   `Tracking = TrackingOf(character)`, because a paragraph set end to end in one scaled style
   carries no runs at all.
6. **`Ww8DocumentReader.MatchesFormatting`** (`:1623`): `&& a.WidthPerCent == b.WidthPerCent`.
7. **`DocReader.Convert`** (`DocReader.cs`:662-711, `Tracking = paragraph.Tracking` at `:695`): `WidthPerCent = paragraph.WidthPerCent,`.
8. **`DocReader.RunsOf`**: `|| run.WidthPerCent != paragraph.WidthPerCent` in the varies predicate
   and `WidthPerCent: run.WidthPerCent` on the `PageRun` (`:1258-1272`).

### C.2 The RTF seat, exactly

1. **`RtfDocumentReader.cs`**, the character-word switch beside `case "kerning":` (`:948-954`):
   `case "charscalex": state.WidthPerCent = Scale(token.Parameter);` where `Scale` is
   `p is { } n and >= 1 and <= 600 ? n : TextWidthScale.Natural` **and a null parameter is 100**,
   not zero — the tokeniser's `defValue` (`rtftokenizer.cxx`:253), measured in §B.4's `sbare` arm.
   This is the one place where `\charscalex` and `\kerning` differ and it is easy to copy wrongly.
2. **`RtfDocumentReader.State.cs`**, `GroupState`: `public int WidthPerCent { get; set; } = 100;`
   beside `AutoKerning`, copied in the group clone (`:315`) and reset to 100 in the `\plain` reset
   (`:371`) — which is `dispatchFlag`'s `PLAIN` restoring the default state's character sprms.
3. **`RtfLayoutRun`** gains `int WidthPerCent = 100`, and **`RtfLayoutRun.MatchesFormatting`**
   (`RtfLayoutParagraph.cs`:76) compares it.
4. **`RecordLayoutRun`** (`State.cs`:1209-1226) and **`CitationRun`** (`:1403-1419`) pass
   `state.WidthPerCent`; **`RecordLayoutParagraph`** (`:1258`, the argument list ending
   `state.AutoKerning` at `:1368`) carries the mark's.
5. **`RtfReader.Convert`** (`RtfReader.cs`:390-428): `WidthPerCent = paragraph.WidthPerCent,` on the
   `PageParagraph`.
6. **`RtfReader.RunsOf`** (`:668-731`): `|| run.WidthPerCent != paragraph.WidthPerCent` in the
   varies predicate and `WidthPerCent: run.WidthPerCent` on the `PageRun`.
7. **`RtfStyles.cs`'s `RtfStyleFormatting`** gains `int? WidthPerCent`, applied and withdrawn in
   `ApplyCharacters`/`WithdrawCharacters` (`State.cs`:2092, :2111) — **needed for the stylesheet
   half**: 53 of the column's 1520 non-identity occurrences are declared in `{\stylesheet}` and
   nowhere else, and the census's `ListLabel` character styles are exactly that shape.

### C.3 One thing found on the way that is not O101

**The RTF reader reads no character tracking at all.** `\expndtw` appears nowhere in
`dotnet/src/XRay.Office.WordProcessing/Rtf`, `RtfLayoutRun` has no `Tracking`, and `RtfReader.RunsOf`
builds its `PageRun` without one — while the DOCX, WW8 and ODF readers all do. The converted column
states `\expndtw` beside almost every `\charscalex` (`\charscalex99\expnd0\expndtw-1` is
LibreOffice's own export shape), so the two are the same runs — and the tracking's reach is an
order of magnitude larger: **13 829 non-zero `\expndtw` in 109 of the 337**, against 1520
non-identity `\charscalex` in 17 (`census-expndtw.py`). `\expndtw` is dispatched to
`NS_ooxml::LN_EG_RPrBase_spacing` (`rtfdispatchvalue.cxx`:187-189) — the very sprm `w:spacing`
uses, in twips — so `RtfLayoutRun` would need the same `Tracking` field the other three readers
carry, in the same three places. Censused here and **not** measured at the reference; it is an
observation with a number on it, not a claim about what it paints.

---

## D. [bin] The style levels, both formats, because two of the seats depend on them

Three more arms, one control word apart, rendered the same way (`style-rtf/`,
`style-rtf-reference.txt`; converted to `.doc` and rendered again, `style-doc/`,
`style-doc-reference.txt`):

| arm | RTF at 26.2.4.2 | the same file as `.doc` | CHPX / style UPX in the `.doc` |
|---|---:|---:|---|
| no stylesheet | 1.00000 | 1.00000 | — |
| `{\stylesheet{\s1\charscalex60 …}}` applied with `\s1` | **0.59974** | **0.59974** | CHPX **none**, style hits **1** |
| `{\stylesheet{\*\cs2\charscalex60 …}}` applied with `\cs2` | **0.59974** | **0.59974** | CHPX **none**, style hits **1** |

Two consequences, and each decides a seat:

- **RTF needs the stylesheet half** (seat C.2.7). A paragraph style's and a character style's
  `\charscalex` both scale the run at the reference, and `RtfStyleFormatting` carries neither
  today.
- **WW8 does not need a separate style path**, because `ApplyCharacterException` resolves a style's
  character UPX through **the same `ApplyLayoutSprms` switch** as a CHPX (`:2316`, `:2333`, `:2375`; the switch itself is `ApplyLayoutSprms`, `:2480`) — so the one `case` of seat C.1.2 covers both. The `.doc` written from these arms proves
  the case is live: 26.2.4.2's own DOC export puts the sprm in the **style UPX and in no CHPX at
  all**, and still draws 0.59974.
- **And it is a warning about censusing WW8**: a CHPX-only walk cannot see a style-level sprm.
  Round 145's walk did read the style UPX (its `style_hits` column) and found **0** in the corpus,
  so the corpus figure stands — but the instrument, not the number, is what makes it safe.

---

## E. What this round did not establish

- **Nothing was built and nothing was measured on our own side.** The brief forbade a build (the
  parent session is building in this tree), so every figure here is 26.2.4.2's. There is no
  before/after, no confinement sweep and no claim that the change would improve any document; what
  is established is what the reference does, where the corpus states it, and what it paints.
- **The 1 % arm does not confirm the twip model.** `p1` draws at 0.00776 where
  `trunc(240 × 1 / 100) / 240` predicts 0.00833. At 1 % of a 12 pt em the face is built at two
  twips and the instrument is measuring a 0.65 pt line, so the arm is not discriminating; it is
  reported as measured and is not evidence against the grid, which the 60 / 99 / 130 / 600 arms
  fit.
- **A one-byte `sprmCCharScale` operand (`nLen < 2`, the control-stack close) was not measured.**
  Authoring one means rewriting an FKP's grpprl lengths, not two operand bytes.
- **The `.rtf` column is 26.2.4.2's own export, not native RTF.** No Word-written or hand-written
  RTF exists in the corpus, so nothing here says how often a *real-world* RTF states
  `\charscalex`, only how often these 337 documents' content does and what the reference does with
  it. A Word-written RTF states `\charscalex100` on nearly every run (see the `sw/qa` fixtures),
  which would raise the gross count and not the net one.
- **The values' provenance is not established.** 95, 103, 105 and 131 appear beside the dominant
  99, and whether they are Word's condense/expand or an author's typography is not something this
  instrument can say.
- **`\expndtw` is censused and not measured** (§C.3).
- **`PROVENANCE.tsv` and `OPEN-ISSUES.md` were not touched**, per the brief. This directory needs a
  `PROVENANCE.tsv` row added **by hand** — the index must not be regenerated in this container,
  because `provenance-index.py` collapses the era column.

---

## F. Files

| file | what it is |
|---|---|
| `convert-rtf.sh`, `convert-rtf.log`, `convert-rtf.start`/`.end` | builds `/home/user/corpus-odf/rtf`; 337 of 337 ok, 104 s |
| `census-rtf.py`, `census-rtf.txt` | `\charscalex` gross/net/stylesheet split, with the `\fN` base rate |
| `census-expndtw.py`, `census-expndtw.txt` | the adjacent tracking census of §C.3 |
| `crosstab.py`, `crosstab.txt` | the `.rtf` census joined to round 148's `.odt` census by stem |
| `census-ww8.py`, `census-ww8.tsv` | round 145's CHPX/style-UPX walk, re-run here; reproduces 8-in-1 |
| `make-rtf-probe.py`, `rtf-fixtures/`, `rtf-out/`, `rtf-reference.txt` | the nine one-control-word RTF arms |
| `doc-fixtures/`, `doc-out/`, `doc-reference.txt`, `doc-sprm.txt` | the same arms as `.doc`, with the CHPX dump that verifies the sprm survived |
| `short-rtf/`, `short-doc/`, `short-doc-out/`, `short-doc-reference.txt` | the 600/601 boundary, on a three-letter line that does not wrap |
| `dump-chpx.py` | every CHPX `sprmCCharScale` of a `.doc`, the guard before any measurement |
| `patch-chpx.py` | rewrites one operand in place, so the out-of-range arms can exist at all |
| `style-rtf/`, `style-doc/`, `style-*-reference.txt` | §D's paragraph-style and character-style arms |
| `render.sh` | one arm at a time, one profile each, refuses to report an arm that produced nothing |
| `readline.py` | round 148's instrument, reused unchanged: the width from the line's own origin |
| `sensitivity-rtf.py`, `sensitivity-rtf.txt` | each stating document rendered as-is and flattened to 100 |
| `sensitivity-ww8.py`, `sensitivity-ww8.txt`, `ww8-pagediff.txt` | the same for the one corpus `.doc` |
| `pagediff.py`, `rtf-pagediff.txt` | per-page comparison; the index pairing in the two `sensitivity-*` scripts overstates a rewrap by the length of the document, and this is the correction |

## G. Summary of the change to make

| | WW8 | RTF |
|---|---|---|
| read it | `LayoutSprms.CharacterScale = 0x4852` + one `case` in `ApplyLayoutSprms` (covers CHPX **and** styles) | `case "charscalex"` beside `"kerning"`, bare = 100 |
| clamp | `>= 1 and <= 600 ? n : TextWidthScale.Natural` | the same |
| keep it across the run merge | `Ww8DocumentReader.MatchesFormatting` | `RtfLayoutRun.MatchesFormatting` |
| carry it | `Ww8LayoutRun`, `Ww8LayoutParagraph` (the **mark's**) | `GroupState` (+ clone, + `\plain` reset), `RtfLayoutRun`, `RtfLayoutParagraph`, `RtfStyleFormatting` |
| vary the paragraph with it | `DocReader.RunsOf` | `RtfReader.RunsOf` |
| set it on the layout | `DocReader.Convert` → `PageParagraph.WidthPerCent`, `PageRun.WidthPerCent` | `RtfReader.Convert` → the same two |
| reach | 8 CHPX in **1 of 66** `.doc`, all 99; 1 page of 20 moves | 1520 non-identity in **17 of 337**; **11 of 17** documents move, 31 pages of 590, no page count |
