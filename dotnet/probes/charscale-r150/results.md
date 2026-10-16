# Round 150 — `\charscalex` and `sprmCCharScale`: the last two thirds of O101

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
Claims are marked **[src]** (the C++ checkout, 27.2.0.0.alpha0+, *not* the reference binary's
source) or **[bin]** (26.2.4.2's own output). The measurement-only round that diagnosed these is
banked at `probes/charscale-r149/`; this is the fix, its pin and its confinement.

O101 seated: *three of the four word-processing readers do not read a character width at all*.
Round 143 closed DOCX, round 148 closed ODF, and this closes **WW8 and RTF**, so the seat is done.

---

## 1. One item under four names

`RES_CHRATR_SCALEW` / `PROP_CharScaleWidth`, applied by VCL as `Font::SetAverageFontWidth`, spelt
`w:rPr/w:w`, `style:text-scale`, `\charscalex` and `sprmCCharScale`. The face is built at a **whole
number of twips**, so `TextWidthScale`'s truncation applies to all four and is not re-derived here.

**[src] WW8** — `sprmChr<0x52, 0, SPRA::operand_2b_2>` (`sw/source/filter/ww8/sprmids.hxx`:335), a
two-byte *unsigned* operand, handled by `SwWW8ImplReader::Read_ScaleWidth`
(`ww8par6.cxx`:4985-4997): `if (nVal < 1 || nVal > 600) nVal = 100;`. A one-byte operand is not a
value — it pops the attribute off the control stack and *ends* the scaled stretch.

**[src] RTF** — `{ "charscalex", { VALUE, CHARSCALEX, **100** } }` (`rtftokenizer.cxx`:253)
dispatched to `NS_ooxml::LN_EG_RPrBase_w` (`rtfdispatchvalue.cxx`:193), which is the very sprm
`w:w` uses, so out of range resets to 100 in the same place. **A bare `\charscalex` is 100, not 0**
— the opposite of `\kerning` beside it, whose bare form is zero by RTF's default-of-zero rule, and
the one place the two differ.

## 2. [bin] The reference, on a fixture that is the same document twice

`words-char-scale.rtf` carries the six arms of `odt-text-scale.fodt` — absent, 100, 99, 60, 130 and
an unscaled paragraph holding a scaled span — and `words-char-scale.doc` is **26.2.4.2's own
conversion of it**, with the sprm verified present in the CHPX before anything was measured
(`probes/charscale-r149/dump-chpx.py`: five CHPX carrying 100, 99, 60, 130 and 60). A fixture
converted and not checked measures nothing and reports agreement.

| arm | `.rtf` at 26.2.4.2 | `.doc` at 26.2.4.2 | this tree |
|---|---:|---:|---:|
| absent | 1.00000 | 1.00000 | 1.00000 |
| `100` | 1.00000 | 1.00000 | 1.00000 |
| `99` | **0.98665** | **0.98665** | 0.98750 |
| `60` | 0.59974 | 0.59974 | 0.60000 |
| `130` | 1.29981 | 1.29981 | 1.30000 |
| scaled span | 0.59974 | 0.59974 | 0.60000 |

Identical to five places to each other **and to round 148's ODF arms**, because it is one item in
VCL. The 99 row is the one worth having: 0.98665 is nearer the truncated 0.98750 than the stated
0.99, and 99 is the corpus's commonest value by a factor of ten.

## 3. Three seats per reader, not two

The DOCX and ODF paths each needed the width in two places: on the run, and in the
**uniform-paragraph varies-predicate**. These two need a third, and it is the one a one-line probe
cannot see — an **adjacent-run merge predicate**:

- **WW8 builds its runs one character at a time** and merges each into its predecessor when
  `MatchesFormatting` finds the formatting identical. A property missing from that comparison does
  not merely fail to vary the paragraph: the scaled characters are **absorbed into the unscaled run
  beside them and take its width**, so the file's own boundary is gone before the shortcut is ever
  asked.
- **RTF is the same shape for the same reason** — a producer restates `\f0\fs22` before every run
  whether or not anything changed, and the merge is what keeps those restatements from breaking the
  shaping context.

RTF needs a fourth: **the stylesheet**. [bin] a paragraph style's and a character style's
`\charscalex` each scale the run at 26.2.4.2, both at 0.59974, so `RtfStyleFormatting` carries the
property and `ApplyCharacters`/`WithdrawCharacters` move it. WW8 needs no style path of its own
because `ApplyCharacterException` resolves a style's character UPX through the same sprm switch.

## 4. Census, re-verified rather than quoted

| | non-identity | documents | base rate |
|---|---:|---:|---|
| `.rtf` column | **1520** | **17 of 337** | 256 927 `\fN` |
| `.doc` | **8** | **1 of 66** | 124 737 CHPX (all stories), 0 style-level |

Non-identity, because a run stating 100 costs nothing: gross is 1965 in 20 documents and counting it
overstates the reach by a quarter. **1265 of the 1520 say 99.** The `.doc` witness is
`AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX`, which is also one of round 148's ODF movers — the
same document stating the same item under two names.

**The `.rtf` column had to be built before it could be censused.** `dotnet/CLAUDE.md` describes four
converted columns and this container had two, so a glob over the absent directory returned 0
occurrences *and* 0 base-rate tokens — which is the tell that a census has measured nothing rather
than found nothing. `probes/charscale-r149/convert-rtf.sh` is 26.2.4.2's own `--convert-to rtf` of
the words track, 337 of 337. It is LibreOffice-written RTF, not native RTF; the corpus holds none.

## 5. Confinement: 13 of 1620, and the five non-movers are the reference agreeing

Our half of the whole corpus and of the `.rtf` and `.odt` columns rendered twice under
`SOURCE_DATE_EPOCH=0`, one output directory per document. **1620 renderings a leg, 0 failures**, each
leg's renders wholly after its own build (head 02:31:02–02:39:55 against 02:31:00; base
02:40:54–02:49:39 against 02:40:52).

| family | ext | moved | of |
|---|---|---:|---:|
| **rtf column** | rtf | **12** | 337 |
| **words** | doc | **1** | 66 |
| words | docx | 0 | 271 |
| odt column | odt | 0 | 337 |
| sheets | xlsx / xls / xlsm | 0 | 307 |
| slides | pptx / ppt | 0 | 302 |

**13 movers, 1607 byte-identical.** The one `.doc` is the census's one `.doc`. Of the 17 `.rtf` that
state a non-identity value, **the twelve that move are exactly those stating one in the body, and the
five that do not are exactly those stating it only inside `{\stylesheet}`** — and those five are the
same five documents round 148 found not moving in ODF, on the same `ListLabel` character styles named
by a `text:list-level-style-*`. **26.2.4.2 does not scale a list label either**: round 148 measured
that with a 24 pt control on the same style and seated it at `SwFont::SetDiffFnt`
(`sw/source/core/txtnode/swfont.cxx`:508 onward), which has arms for seventeen character items and no
`RES_CHRATR_SCALEW`. So the five are agreement, not a gap.

## 6. Do the movers get closer? 1173 spans of 1178

The 13 movers rendered through 26.2.4.2 as well, and both legs scored against it.

**The first instrument said nothing and said it loudly.** `score.py`, which scores a span's left
*edge* and is what `probes/odscentre-r150` needed, returns **exactly 0.100 pt for all thirteen
documents on both legs** — that is the two PDF writers' own constant text origin and nothing else. A
character width does not move where a span starts.

**The second was diluted.** `score-width.py` scores the drawn *width*, averaged over every paired
span, and gives 3 closer, 2 further, 8 level — because a scaled run is a handful of spans among
hundreds and eight documents come out identical to four decimal places. A metric that cannot see the
change is not evidence that the change did nothing.

**The third is the measurement.** `score-changed.py` restricts to the spans whose drawn width
actually differs between the legs and asks whether each moved towards the reference:

```
spans whose drawn width this change moved: 1178
  closer to 26.2.4.2 1173   further 5   neither 0
```

`Annex-10` alone accounts for 1099 of them (1097 closer, 2 further). Four of the thirteen show no
moved *paired* span at all while their bytes differ — the change moved a line break there, so the
span text differs and `difflib` drops the pair; that is a limit of the instrument and is not counted
either way.

## 7. Mutation pin, six arms

`mutate.sh`, restoring with `cp` + `touch` — never `mv` and never `git checkout` alone, either of
which leaves the source older than the assembly so MSBuild skips the project while reporting
`0 Error(s)`. Round 150 lost two builds to exactly that.

| arm | reverted | tests red |
|---|---|---:|
| M1 | WW8 never reads `sprmCCharScale` | 4 of 10 |
| M2 | WW8's adjacent-run **merge** does not compare the width | 1 |
| M3 | WW8's varies-predicate does not list the width | 1 |
| M4 | RTF never reads `\charscalex` | 4 |
| M5 | RTF's adjacent-run **merge** does not compare the width | 1 |
| M6 | RTF's varies-predicate does not list the width | 1 |

**A mutation arm that fails to compile prints nothing and reads exactly like an arm that is not
pinned.** M4's first cut used `and >= 601 and <= 600`, an impossible relational pattern; the compiler
warns, `TreatWarningsAsErrors` makes that a build failure, and the arm came out silent. `run` now
greps for compiler errors as well, so a broken arm can never look like a green one again.

## 8. What this does not establish

- **No gate verdict can move** and none is claimed: a character width adds no alphanumeric character
  and no page.
- **The `.rtf` column is LibreOffice-written RTF.** Nothing here says how often *native* RTF states
  the control word; the corpus holds no `.rtf` of its own.
- **The four movers with no moved paired span are unexplained.** Their bytes differ and their line
  breaks moved; which breaks and whether towards the reference was not measured.
- **The five spans that moved further are not explained**, only counted.
- **`\expndtw` is not read by the RTF reader at all** — an adjacent gap, not O101. The converted
  column states **13 829 non-zero `\expndtw` in 109 of the 337**, an order of magnitude more reach
  than the scale and on the same runs; it is `NS_ooxml::LN_EG_RPrBase_spacing`, the sprm `w:spacing`
  uses, and the other three readers all carry it. Censused, not measured at the reference.
