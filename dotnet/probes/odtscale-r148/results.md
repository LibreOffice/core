# Round 148 — `style:text-scale`: the ODF half of O101

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
**Corpus** the converted ODF column, `/home/user/corpus-odf/odt`, **337** documents.
**Tree** `/home/user/libreoffice-core`, branch `claude/renderer-comparison-artifact-m1g0wy`.
Every claim below is marked **[src]** (a reading of the C++ checkout, which is 27.2.0.0.alpha0+
and *not* the reference binary's source) or **[bin]** (a measurement of 26.2.4.2's own output).

O101 seats *"three of the four word-processing readers do not read a character width at all"*.
This closes the ODF third of it. The WW8 third is censused at 8 CHPX runs in 1 of 66 `.doc` and
is left; the RTF third cannot be censused in this container, and a zero must not be quoted for
it — `/home/user/corpus-odf` holds `odt` and `ods` and no `rtf` column, so a glob over the
absent directory returns 0 occurrences **and** 0 base-rate tokens, which is the tell.

---

## 1. The item is the one the DOCX path already uses

**[src]** `xmloff/source/text/txtprmap.cxx`:253 and :641 —

```
MT_E( PROP_CharScaleWidth, XML_NAMESPACE_STYLE, XML_TEXT_SCALE, XML_TYPE_PERCENT16, 0 )
```

`PROP_CharScaleWidth` is `RES_CHRATR_SCALEW`, which `DomainMapper` fills from
`LN_EG_RPrBase_w` and the RTF tokeniser from `\charscalex`. So the ODF attribute is not a
second mechanism: it is the same item, and `TextWidthScale`'s twip truncation — the face is
built at a whole number of twips, `trunc(240 × 99 / 100) = 237` at 12 pt — applies here
unchanged and is **not re-derived**. `CharacterScaleTests` pins the arithmetic; this round pins
that the ODF reader reaches it.

## 2. The reference, on five one-attribute arms

**[bin]** `make-probe.py` builds five flat-ODF files differing in one attribute, each a single
right-aligned line of `Hamburgefonstiv` at 12 pt Liberation Serif. The width is read from the
line's own origin (`reference.txt`, `readline.py`) rather than from glyph boxes, because a PDF's
glyph positioning is quantised to whole thousandths of an em and `dotnet/CLAUDE.md` records four
rounds lost to measuring a sub-thousandth effect through that channel.

| arm | 26.2.4.2 | ratio | this tree | agreement |
|---|---:|---:|---:|---:|
| absent | 83.712 pt | 1.00000 | 1.00000 | — |
| `100%` | 83.712 pt | 1.00000 | 1.00000 | exact |
| `130%` | 108.810 pt | 1.29981 | 1.30000 | 0.02 % |
| `60%` | 50.205 pt | 0.59974 | 0.60000 | 0.04 % |
| `99%` | 82.594 pt | **0.98665** | **0.98750** | 0.09 % |

The 99 % row is the one worth having: **0.98665 is nearer the truncated 0.98750 than the stated
0.99000**, so the grid is the reference's here as it is for DOCX, and a reader using the
percentage itself would be wrong by a quarter of a per cent on the corpus's commonest value.
An absent attribute and a stated `100%` are byte-identical.

## 3. The namespace is the specification's, which is worth stating

**[bin]** `census.py`, over both `content.xml` and `styles.xml` of all 337:

```
occurrences by namespace : {'style': 350}
base rate, text:span     : 210828
documents off the identity: 18
```

**350 as `style:text-scale` and not once as `loext:text-scale`.** `dotnet/CLAUDE.md` records
six ODF attributes in this reader that are the other way round — `drawooo:display`,
`loext:shadow-blur`, `chartext:coordinate-region`, `text:line-break`, `drawooo:sub-view-size`
and `loext:writing-mode` — each of which cost a round. This one is not among them, and the
grep that establishes that is the same two-spelling grep those rounds should have run.

**The reach is 111 occurrences in 18 documents, not 350 in 22.** A run stating `100%` costs
nothing, so counting it overstates the reach here by a factor of three. The values are Word's
condense/expand micro-adjustments — 99, 98, 105, 130 — which move a line break rather than a
glyph.

## 4. Confinement: 13 movers of 337, every one predicted, every non-mover explained

**[bin]** Our half of the whole converted `.odt` column rendered twice — once at the round's
base, once with the fix — under `SOURCE_DATE_EPOCH=0`, **one output directory per document**
rather than per worker slot (`sweep.sh`; a thread pool does not work consecutive indices and two
live renders in one directory silently destroy each other's output). 337 documents, one PDF
each, both legs.

```
movers 13   byte-identical 324
```

`census.py` cross-tabs the movers against the census (`census.txt`):

- **every one of the 13 movers states a non-identity `style:text-scale`** — `movers the census
  did not predict: []`;
- **no document outside the 18 moved**, so nothing else in the tree was reached.

The five documents that state one and did **not** move are explained individually
(`nonmovers.py`, `nonmovers.txt`), and four of them are one thing:

| document | states | why it does not move |
|---|---|---|
| `0126226bc58c-SPA-06_mcar_part-6_and_IS_v2.9` | 12 × 99 % | only on `ListLabel_20_*`, named by a `text:list-level-style-number` |
| `1ef46e226af0-JEMIT_Template` | 1 × 86 % | same |
| `2df5a30320b3-Allegiant_Company_Profile…` | 1 × 99 % | same, on a `text:list-level-style-bullet` |
| `bc17eefecac9-DRX-Ascend System Course Description` | 7 × 99 % | same |
| `f30b3e8d2d1e-mde087077~283` | 1 × 90 % | a paragraph style **applied to nothing** — `content.xml` names neither it nor the `Subtitle` style that derives from it |

## 5. A list label takes no character scale, and that is the reference's own answer

The four above are a **list label**, not text, so the question is whether we are missing
something. We are not.

**[bin]** `labels/` — three arms, all with the level's own character style `LL` and
`style:num-suffix=""` with `text:label-followed-by="nothing"`, so the paragraph's `X` begins
exactly at the label's right edge (`labels/reference.txt`):

```
lab-none       '1X'   size 12.00  width  14.664
lab-50         '1X'   size 12.00  width  14.664     <- style:text-scale="50%" on LL
lab-ctl24      '1'    size 24.00  width  12.000     <- fo:font-size="24pt" on LL
lab-ctl24      'X'    size 12.00  width   8.664
```

**`lab-ctl24` is the control and is not optional.** An arm that does not move is evidence only
once a *different* attribute on the *same* style is shown to move; without it, "the reference
ignores the scale" and "the probe never referenced the style" are the same picture. The control
moves — the label is drawn at 24 pt and 12.000 pt wide against 6.000 — so `LL` **is** reached
and the scale is nonetheless ignored.

**[src]** The seat, and it is an absence rather than a branch. `SwTextFormatter::NewNumberPortion`
builds the label's font by `SwFont::SetDiffFnt(&rNumFormat.GetCharFormat()->GetAttrSet(), …)`
(`sw/source/core/text/txtfld.cxx`:567-568 and the `SetDiffFnt` at :95), and
`SwFont::SetDiffFnt` (`sw/source/core/txtnode/swfont.cxx`:508 onward) reads **no
`RES_CHRATR_SCALEW`**: it has arms for the three scripts' font, size, posture, weight and
language, for underline, overline, strikeout, colour, emphasis, contour, relief, autokern,
word-line-mode, escapement, case map and kerning, and the only `SetPropWidth` in it is
`RES_CHRATR_SHADOWED`'s, which sets 50 or 100. The one place the item reaches a font is
`SwAttrHandler`'s text-attribute stack, `sw/source/core/text/atrstck.cxx`:763-764 — and a
numbering portion does not go through it.

So the four `ListLabel` non-movers are **correct as they stand**, and this tree agreeing with
26.2.4.2 on them is the measurement rather than a gap.

## 6. The change

- **`OdfParagraphFormats`** — `OdfTextStyle` gains `int WidthPerCent = 100`; `ResolveText`
  fills it from `ScaleIn`, which reads the cascaded `style:text-scale`, tolerates the `%`
  and a decimal, and falls back to `TextWidthScale.Natural` for anything unparsable or
  non-positive. A zero would collapse the run to nothing, which is worse than reading none —
  the same guard `WordParagraphFormats` makes for `w:w`.
- **`OdtLayoutSource`** — `WidthPerCent` on the `PageParagraph`, on each `PageRun`, and in the
  varies-predicate that decides whether the run list survives the uniform-paragraph shortcut.
  That last is the half a reader loses silently: the measurement fallbacks rebuild a run from
  the paragraph and carry no scale, so a scaled run folded into an unscaled paragraph is
  measured and broken at the paragraph's width.

**Fixture** `tests/corpus/features/odt-text-scale.fodt`, six paragraphs — absent, 100, 99, 60,
130, and an unscaled paragraph holding a 60 % span. **Every arm draws the same word**, so the
ratios in `OdtCharacterScaleTests` are exact rather than a per-character proxy; the first cut
used five different words and had to assert at 6 % where these assert at 0.05 %.

## 7. Mutation pin

`mutate.sh` reverts one line of the round's diff at a time, rebuilds, runs the five tests, and
restores with `cp` + `touch` — never `mv`, which keeps the old mtime and lets MSBuild skip the
project while reporting `0 Error(s)`.

| arm | reverted | tests red |
|---|---|---:|
| M1 | `ScaleIn` never reads the attribute | **4 of 5** |
| M2 | the scale is the stated percentage, not the twip grid | **1** (the 99 % test alone) |
| M3 | the varies-predicate does not list `WidthPerCent` | **1** (the span test) |
| M4 | `PageRun` is built without the run's own `WidthPerCent` | **1** (the span test) |

M2 is the arm that matters most and is the one an obvious implementation would fail: it leaves
60 % and 130 % right and 99 % wrong by a quarter of a per cent, which is the corpus's commonest
value.

## 8. What this does not establish

- **No gate verdict can move on this, and none was claimed.** A character width adds no
  alphanumeric character and no page, so the column the gate scores is blind to it — the same
  argument as `w:pgBorders` and the worksheet fills. The evidence here is the confined reach
  and the five-arm agreement, not a scoreboard.
- **The 13 movers were not scored against the reference.** Confinement says the change reaches
  exactly the documents that state the attribute and nothing else; it does not say each of the
  13 is now *closer* to 26.2.4.2. The five-arm agreement is the argument that it is, and a
  per-document ink comparison over the 13 is the measurement that would settle it.
- **`ScaleIn` rounds a fractional percentage** (`98.5%` → 99) because the item is a
  `sal_Int16`. No corpus document states one, so that arm is unmeasured at the reference.
- **The RTF and WW8 thirds of O101 are untouched.**
