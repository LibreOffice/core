# fonts-r64 — glyph fallback asks fontconfig first, and one generic's list decides the answer

Three defects, in the brief's order. The first is the probe committed at `de7955510` as its own
round; the second and third were handed on by the last fonts round.

Everything below is measured against **LibreOffice 26.2.4.2** from the TDF tarball with all three
font confounds moved aside — the metric-compatible duplicates, the Latin `NotoSans-*`/`NotoSerif-*`
and `opens___.ttf`. `/usr/bin/soffice` (24.2.7.2) is not used anywhere in this round.

---

## 1. Our glyph fallback asked the two sources in the reverse order to LibreOffice's

### The mechanism

`PhysicalFontCollection::GetGlyphFallbackFont` calls the fontconfig hook and reaches
`ImplInitGenericGlyphFallback`'s fixed list only `if (!pFallbackData)`
(`vcl/source/font/PhysicalFontCollection.cxx`:231-291). `SystemFontResolver.FallbackFor` did the
opposite, and the fixed list heads with `starsymbol, opensymbol` — so **every character OpenSymbol
covers was drawn from OpenSymbol**. Only three of that list's forty-eight families are installed
here (`OpenSymbol`, `DejaVu Sans`, `Liberation Sans`), and OpenSymbol comes first, so in practice the
list *was* OpenSymbol.

Reversing the two stages is necessary and not sufficient, because the second stage ranked candidates
by **one merged preference order across every generic**, and that is not what fontconfig matches
against.

### What actually decides it, measured

`FontConfigManager::Substitute` is one function and the glyph-fallback hook goes through it
(`vcl/unx/generic/font/fontsubst.cxx`:173-184), so the same switch that decides *family*
substitution decides *glyph* fallback: `serif` is appended as a second `FC_FAMILY` for
`FAMILY_ROMAN` and `sans` for `FAMILY_SWISS` (`fontconfig.cxx`:1075-1088). `FC_CHARSET` outranks
`FC_FAMILY`, so the answer is **the first face on that one generic's `<prefer>` list that covers the
character**.

`gen-generic.py` builds one DOCX per (declared class, character) — 72 of them, six classes by the
twelve characters in its `CHARS`, the family always `Calibri` so that everything actually falls back.
Faces read out of the PDFs both binaries produced. `U+2011` is the thirteenth row and was taken by
the same generator with `CHARS` narrowed to it, because it is the character the corpus documents in
the reach table below fall back on:

| character | roman · modern · script · decorative · undeclared | swiss |
|---|---|---|
| `U+2713` ✓ | **FreeSerif** | **DejaVu Sans** |
| `U+27A2` ➢ | **FreeSerif** | **DejaVu Sans** |
| `U+2011` non-breaking hyphen | **DejaVu Serif** | **DejaVu Sans** |
| `U+4E00` 一 | WenQuanYi Zen Hei | WenQuanYi Zen Hei |
| `U+2714` ✔ · `U+2611` ☑ · `U+263A` ☺ | Noto Color Emoji | Noto Color Emoji |
| `U+05D0` א | FreeSans | FreeSans |
| `U+0E01` ก | FreeSerif | FreeSerif |

Two things fall out of that table and both are in the fix:

- **Undeclared behaves as roman**, because Writer's own pool default is roman. So a word-processing
  document lands on the *serif* list unless its font table says otherwise, and a slide or a sheet —
  which declare nothing LibreOffice acts on — lands on the sans-serif one through
  `49-sansserif.conf`.
- **The emoji row is a language rule, not a family one.** `getExemplarLangTagForCodePoint` answers
  `und-zsye` for a character with the Unicode `Emoji` property (`fontconfig.cxx`:1026-1029) and
  fontconfig scores `PRI_LANG` above `PRI_FAMILY_WEAK`, so `U+2714` answers Noto Color Emoji under
  all six classes although FreeSerif holds it and is on the serif list. `U+2713`, which the property
  excludes, does not. Modelled as a coverage question over the `emoji` preference list rather than as
  a Unicode property, because the two are the same set by construction.

### `fc-match ":charset=XXXX"` is not the question LibreOffice asks

Asked bare it answers **DejaVu Sans** for every character in the table, because `49-sansserif.conf`
appends `sans-serif` to a pattern that named no generic — which is the *swiss* row, not the common
one. `fc-match "Calibri,serif:charset=2713"` answers FreeSerif and `fc-match
"Calibri,sans:charset=2713"` answers DejaVu Sans. The previous round's probe comment read
`fc-match ":charset=25cf"`, concluded DejaVu Sans, and was right only because its witness was a
`.pptx`.

### The seat

- `FontconfigPreferences` now keeps **each generic's preference order as well as the merged one**
  (`RankOf(family, generic)`, `InOrderFor(generic)`); the merged order survives as the tie-break, so
  a face on no list at all is ordered exactly as it was.
- `SystemFontResolver.FallbackFor` asks the emoji list, then the request's own generic list, then —
  only if both answer nothing — LibreOffice's fixed list. With **no** fontconfig the fixed list still
  comes first, because there is then no preference order for the other stage to read.
- The generic comes from the *request*, so `SystemFontResolver` records it against the face it chose
  and `FontItemiser` passes the primary face through a new
  `IGlyphFallbackResolver.FallbackFor(codePoint, weight, isItalic, primary)`. First writer wins, and
  that is a stability requirement rather than a preference: measurement and drawing itemise the same
  paragraph separately and must choose the same fallback face for it.
- **`SymbolFallbackFor` is untouched**, so the previous round's rule holds: a pi face is never handed
  to fontconfig and the fixed list is still the whole of its answer.

### Reach

On the 72-cell probe, agreement with 26.2.4.2 goes from **30/72 to 65/72**. The seven that remain are
the two complex-script rows — Hebrew under all six classes and Thai under swiss — and neither is a
glyph fallback: a CTL run takes Writer's own CTL font item, which has its own family and its own
class. Recorded in `dotnet/CLAUDE.md`, not fixed here.

The probe's own witness closes exactly. `slides/done-010/pptx/Tax factsheet 2022 (1).pptx`, whose
`a:buChar char="●"` bullets the reference draws in DejaVu Sans:

| | faces |
|---|---|
| 26.2.4.2 | `Carlito-Bold, Carlito-Regular, DejaVuSans, LiberationSans, LiberationSans-Bold` |
| before | `Carlito-Bold, Carlito-Regular, LiberationSans, LiberationSans-Bold, OpenSymbol` |
| after | `Carlito-Bold, Carlito-Regular, DejaVuSans, LiberationSans, LiberationSans-Bold` |

### Reach, corpus

`ourfaces.sh` renders all 947 corpus documents through our own binary and records the face set of
each; `facediff.py` reports which sets differ between two such sweeps, and only the documents that
moved are worth a `soffice` run. **Seven of 947 moved for this defect** — one sheet, two decks, four
words documents — and screened against 26.2.4.2 with `reffaces.sh`:

| document | before | after |
|---|---:|---:|
| `Tax factsheet 2022 (1).pptx` | 2 | **0** |
| `sectors-defense-and-aerospace.xlsx` | 1 | 1 |
| `PI-doc.-no.-2E-Technical-Review-Report.docx` | 2 | 2 |
| `ESPN-R - MCF - Manual…docx` | 1 | 1 |
| `2015-Civil-Rights-Website-training.ppt` | 2 | **3** |
| `AWR OPS-AOC 044…docx` | 1 | **2** |
| `150-5370-10H.docx` | 0 | **2** |
| total (this defect) | 9 | 11 |

(The figure is the symmetric difference between our face set and the reference's, so 0 is an exact
match and every face named on one side and not the other counts 1.)

**Three of the seven got worse and each has one named cause, none of them the ordering.**

- `150-5370-10H.docx` and `AWR OPS-AOC 044…docx` draw `U+2610` ☐ in runs whose font comes from
  `w:rFonts w:eastAsia="MS Gothic" w:hint="eastAsia"`. Writer puts such a run on
  `RES_CHRATR_CJK_FONT`, a *different item* from the western one, and that item does not carry the
  roman default `WordFallbackClass.ForDeclared` applies — so the reference lands on the sans-serif
  list and answers DejaVu Sans while we take the serif list and answer FreeSerif. Both documents
  drew DejaVu Sans before, by the accident of `dejavusans` being on the fixed list. This is the same
  residual as the Hebrew and Thai probe rows and it is the named next step.
- `2015-Civil-Rights-Website-training.ppt` is mostly the environment: the reference draws
  `DejaVuSansCondensed-Oblique` and `-BoldOblique`, which the tarball ships and the system does not,
  so they were already two faces we could never match; we now add a `DejaVuSansMono` it does not
  draw.

Against that, the probe's own witness closes exactly and the controlled probe moves from 30/72 to
65/72.

### What this makes worse, and it is worth knowing

**We cannot paint a colour bitmap face.** `roman__2714.docx` through 26.2.4.2 has ink at
(80,80)-(119,118) at 100 dpi; ours now embeds Noto Color Emoji, names it in the PDF with the right
advance, and paints **nothing** — the face is CBDT/CBLC and our rasteriser reads outlines. Before this
round the same character was drawn from OpenSymbol: the wrong face, at the wrong advance, but
visible. The advance is the thing that moves line breaks and pagination, so the face is still the
right answer; the missing half is colour-font support, which is its own body of work.

---

## 2. A `Symbol` body run in a `.doc` was not recoded

### The mechanism, which is not a recode rule

`Read_FontCode` opens with `if (m_bSymbol) return;` — *"if bSymbol, the symbol's font (see
sprmCSymbol) is valid!"* (`sw/source/filter/ww8/ww8par6.cxx`:3963-3966). Word writes both sprms into
one CHPX, `sprmCSymbol` first, and only the first of them is the run's font. The CHPX behind
`150_5300_13_chg12.doc`'s greater-or-equal sign is

```
096A 0100 B3F0   sprmCSymbol: font 1 (Symbol), U+F0B3
1668 D2550B00    sprmCPicLocation
434A 1200        sprmCHps
4F4A 0000        sprmCRgFtc0: font 0 (Times New Roman)
514A 0000        sprmCRgFtc1
```

Applying that grpprl left to right — which is what we did — hands the run back to Times New Roman.
The face is then not OpenSymbol, so nothing recodes the slot, and the reference's twenty-one
OpenSymbol characters in that document were drawn either as the Latin letter the slot spells —
`<`, `>`, `³` — or, for the eight that state a Private Use slot outright, as a code point nothing
installed holds.

A second half of the same defect: `MatchesFormatting` did not compare `SymbolSlot`, so once the
symbol run had lost its face it also matched the ordinary run beside it and was merged into it —
which is why only one of that document's nine `sprmCSymbol` runs survived to layout at all. The slot
is not formatting: `SwWW8ImplReader::ReadChars` inserts it once per position the sprm covers
(`ww8par.cxx`:3410-3413), so merging either loses the symbol or spreads it over its neighbour's text.

### The seat

`Ww8DocumentReader.ApplyLayoutSprms` — a font code after a `sprmCSymbol` in the same grpprl is
dropped — and `MatchesFormatting`, which now compares the slot first. The sprm walk was made
`internal static` and takes the document's properties as an argument, which is what makes the
ordering rules checkable against a hand-built grpprl.

### Reach

**5 of the corpus's 66 `.doc` files** carry a `sprmCSymbol` with a `sprmCRgFtc0` behind it in the
same CHPX — a byte scan of the FKP pages for `09 6A` followed by `4F 4A` within the next forty
bytes: `150_5300_13_chg12.doc`, `150_5300_13_chg8.doc`, `150_5300_13_chg10.doc`,
`1257259179492_2007_TPPT_102_Supporting_Doc_2-434003080.doc` and
`300502-Mecenat-Airbus-Simusante_CHU-Amiens-Picardie.doc`. Six more carry a `sprmCSymbol` with no
font code after it and are unaffected. Three of the five move a *face set*; the two `150_5300_13`
siblings draw OpenSymbol elsewhere already, so their symbol runs move glyph for glyph without the
face-set instrument being able to see it.

`150_5300_13_chg12.doc`'s face-set distance from 26.2.4.2 goes from **2 to 1**: OpenSymbol arrives,
and the only remaining difference is a `LiberationSans` we draw and the reference does not, which is
not a fallback question.

---

## 3. Calibri in legacy binaries did not reach Carlito

### The mechanism

Nothing is wrong with the substitution: `Resolve("Calibri")` answers Carlito under every declared
class. **`Calibri` never reached the resolver at all.** A WW8 document's default font is not in any
style's CHPX and not in the DOP — it is `Stshi.ftcAsci`, a bare font-table index twelve bytes into
the stylesheet's own header, which `WW8Style` reads as `m_ftcAsci`
(`sw/source/filter/ww8/ww8scan.cxx`:6919-6921) and `WW8RStyle::Set1StyleDefaults` applies to every
paragraph style based on nothing that set no font of its own (`ww8par2.cxx`:3714-3725, called from
`PostStyle` at :3862). We never read it, so every run stating no font of its own arrived with a null
family and took the resolver's own no-family default, Liberation Serif.

On `AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc`, `Normal`'s whole CHPX is
`5f48 0104 6d48 0904 7348 0904 7448 0904` — no `sprmCRgFtc0` anywhere — while
`rgftcStandardChpStsh` is `(4, 4, 4)` and font 4 is `Calibri`. Traced through `LayoutFonts`, the
document asked for `Times New Roman`, `Arial`, `Cambria Math` and the empty string, and never for
Calibri.

### The seat

`Ww8StyleSheet` reads `DefaultFontIndex` from the `Stshi` and `ResolveCharacterChain` prepends a
synthesised `sprmCRgFtc0` for it whenever the chain's root is a paragraph style based on nothing.
Prepending rather than testing whether the chain set a font is what makes it a *default*: a style
that states one states it later in the list and wins, which is `mbFontChanged` expressed as ordering.
A character style based on nothing does not get it, matching `Set1StyleDefaults`' guard on
`rSI.m_bColl`.

### Reach

**11 of the corpus's 66 `.doc` files** declare a non-zero `Stshi.ftcAsci`, and the brief's "three
documents" undercounts: seven of the eleven name **Calibri**, two name `Times` and two name
`Century`.

| `ftcAsci` names | documents |
|---|---|
| Calibri | `FMRBullletinB-28.doc`, `chapter 12.doc`, `2013-op12-Annex III - Curriculum Vitae.doc`, `A380MaenPressRel.doc`, `f111.doc`, `AAC-AD-No-2021-01…doc`, `300502-Mecenat-Airbus-Simusante…doc` |
| Times | `10795.doc`, `RMI_…GettingOffOil.doc` |
| Century | `1257259179492_2007_TPPT_102…doc`, `1228841571067_2009_TPPT_13…doc` |

The other 55 declare 0, which names the font table's first entry; the change makes those runs ask for
that entry by name instead of asking for nothing, which is what LibreOffice does and which usually
resolves to the same face.

`AAC-AD-No-2021-01…doc`'s face-set distance from 26.2.4.2 goes from **4 to 1**: Carlito Regular and
Bold arrive, the spurious `LiberationSerif-Bold` goes, and the one remaining difference is an
OpenSymbol the reference draws for its list bullets — Writer's own default bullet font, which is a
fourth defect and not one of these three.

---

## Corpus reach, both sweeps ours

`faces-before.tsv` and `faces-after.tsv` beside this are the two whole-corpus sweeps,
`faces-ref26-moved.tsv` the reference half of the fourteen that moved, and `moved.txt` the list.
`ourfaces.sh` over all 947 documents, before and after, both through our own binary; `reffaces.sh`
for the reference half of the documents that moved. **14 of 947 moved** — sheets 1, slides 2, words
11 — and the symmetric difference against 26.2.4.2 over those 14 goes from **26 to 19**: seven
closer, four unchanged, three further.

| defect | documents moved | before | after |
|---|---:|---:|---:|
| 1 · fallback order and the generic | 7 | 9 | 11 |
| 2 · a `Symbol` body run in a `.doc` | 3 | 5 | 2 |
| 3 · `Stshi.ftcAsci` | 4 | 12 | 6 |
| **total** | **14** | **26** | **19** |

Two of the fourteen reach an exact match they did not have before —
`Tax factsheet 2022 (1).pptx` and `300502-Mecenat-Airbus-Simusante…doc`.

Nothing else in the corpus draws a different face at all, which is the useful half of the figure: a
change to glyph fallback that "moves every track at once" turns out to move fourteen documents,
because on this machine only three of the fixed list's forty-eight families are installed and the
overwhelming majority of the corpus's OpenSymbol is a *recode* — a run whose family really is
`Symbol` or `Wingdings` — and not a fallback. **237 of the 947 draw OpenSymbol and 235 of them still
do**: only `sectors-defense-and-aerospace.xlsx` and `Tax factsheet 2022 (1).pptx` stopped, and three
`.doc` gained it through defect 2.

## What this round did not close

- **The script-specific font item.** Hebrew and Thai answer FreeSans and FreeSerif under every
  declared class, which no generic's preference list explains: a complex-script run takes Writer's
  own CTL font item, with its own family and its own class — seven of the 72 probe cells. The CJK
  side of the same thing is what makes two corpus documents worse here, and the fix is in the
  readers rather than in the resolver: `WordFallbackClass.ForDeclared`'s roman default is the
  *western* item's, and a run taken from the `w:eastAsia` or `w:cs` slot must not be given it.
- **`rMissingCodes` is a string.** LibreOffice asks fontconfig for a face covering *all* the
  characters a run is missing at once; we ask one code point at a time. That is why
  `AAC-AD-No-2021-01…doc` draws `U+2011` in FreeSerif where a one-character probe of the same request
  draws it in DejaVu Serif.
- **Colour bitmap faces do not paint.** See the note under defect 1.
- **Writer's default list-bullet font.** `AAC-AD-No-2021-01…doc` and others draw their bullets in
  OpenSymbol in the reference and in the paragraph's own face here. Found while measuring defect 2,
  and it is not defect 2.
