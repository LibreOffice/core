# Words-B round 1 — list labels and the line's two sides

Reference: **LibreOffice 26.2.4.2 620(Build:2)**, the installed binary, with
`fonts-dejavu-core` present. Every render pinned `SOURCE_DATE_EPOCH=1700000000` and
`TZ=UTC`, and every concurrent `soffice` got its own `-env:UserInstallation` profile.
Worktree `/c/sandbox/workdir/wt-words-b`, branch `wt-words-b`, from `4cbaeb41c3b`.

**Headline.** Round 47's law reproduces on 26.2.4.2 exactly — I predicted it would not, and
it does. The sentence attached to it is nevertheless wrong in one word: the base is not a
maximum of **boxes** but a maximum on **each side of the baseline**, and our gate for
letting a label into the line was blind to the descent side entirely. Fixing that moves
**37 of 200 renderings and 0 verdicts**, which is what a spacing law is worth and is
reported as such. Two further laws are measured and pinned and deliberately **not** acted
on, because their corpus reach in DOCX is nought.

---

## 1. The prediction, and how it fared

`prediction.md` in this directory was written and its addendum appended **before the first
`soffice` or CLI invocation of the round**. Its blind-spot section is reproduced there in
full and is not restated here. Scoring it honestly:

| # | predicted | confidence | outcome |
|---|---|---:|---|
| 1 | Round 47's label law does **not** reproduce on 26.2.4.2; labelled rows collapse onto the unlabelled control | 0.70 | **REFUTED.** It reproduces to the digit — see §2. |
| 2 | The proportional gap has moved from above the line to below it (`LINE_SPACING_AS_GAP_BELOW`) | 0.60 | **REFUTED.** The gap is above, exactly as round 45 had it — see §2.1. |
| 3 | The base is the tallest single portion's whole box, not max-ascent + max-descent | 0.55 | **REFUTED.** It is per side — see §2.3. This is the round's finding. |
| 4 | **0 verdicts moved** (pages, ±2% words, unembedded fonts) | — | **CONFIRMED.** 0 of 200. |
| 5 | 45–75 of 134 DOCX resolve to a label reaching past its item | — | **47 of 134.** Inside the band. |
| 6 | 12–25 of 134 at proportional spacing above 100% | — | **9 of 134.** Below the band. |

Predictions 1 and 2 were both taken from reading the C++ tree in this checkout, which is
**27.2.0.0.alpha0+**. `SwLinePortion::IsUsedToCalcLineSpacingHeight`
(`sw/source/core/text/porlin.cxx`:324) excludes `InNumberGrp()` by name in both its
branches, and `SwTextFormatter` (`itrform2.cxx`:2443-2448) takes the base from `GetPrev()`
under `LINE_SPACING_AS_GAP_BELOW` — *"Like Microsoft Word, apply the line spacing gap after
the line"*, commented in the source as the *"new (2027)"* implementation. **Neither is in
26.2.4.2.** That is the whole lesson of two wrong predictions at 0.7 and 0.6: the dev tree
is a description of a future binary, and the binary that made the references is the one
that decides. It cost nothing here because the probe was written before the reading was
believed.

---

## 2. The law for labels, measured

All rows: a 12 pt Liberation Serif item, faces named explicitly in `w:docDefaults` and in
the level so no font change can reach them (`pdffonts` on every load-bearing probe PDF
shows only Liberation and Caladea faces; all four probes were re-run after
`fonts-dejavu-core` landed mid-round and returned **identical** numbers).

### 2.1 Where the gap goes — `pitch.py`

One paragraph of five lines broken by explicit `<w:br/>`, each line starting with a 12 pt
marker word so a pitch is the difference of two identical glyph boxes. A 28 pt word is put
on line 1, line 3 or line 5, or nowhere.

| case | MZZ→A | A→B | B→C | C→D | D→E | E→MYY |
|---|---:|---:|---:|---:|---:|---:|
| `flat` 200% − `flat` 100% | +0.00 | +13.80 | +13.80 | +13.80 | +13.80 | +13.80 |
| `tall3` 200% − `tall3` 100% | +0.00 | +13.80 | **+32.20** | +13.80 | +13.80 | +13.80 |
| `tall5` 200% − `tall5` 100% | +0.00 | +13.80 | +13.80 | +13.80 | **+32.20** | **+32.20** |

`B→C` is the pitch **into** line 3. The extension for line 3's height lands there, not in
`C→D`, so the gap is applied **above** the line and sized by **that line's own** content —
round 45's law, unmoved at 26.2.4.2. The paragraph's own first line never gets one
(`MZZ→A` is +0.00 in every row), which is `IsParaLine()`; its share arrives at the
paragraph boundary instead, which is why `E→MYY` moves with line 5.

`box(12 pt Liberation Serif) = 13.80` and `box(28 pt) = 32.20`, both to the hundredth from
`hhea` — ascender 1825, descender −443, lineGap 87 over 2048 upem. **Ours matched
LibreOffice on all 126 pitches in this probe, before and after the change.**

### 2.2 A label is a portion, and takes its share — `lastline.py`

A label only ever sits on line 1, so its share is only visible where line 1 is also the
last line: a **one-line** paragraph, whose height feeds the next paragraph's upper space.
That is round 47's geometry. The control it did not have is `tallL` — the same paragraph
with an extra *run* at L pt instead of a label, which produces the identical line box.

`gap(p) − gap(100)`, LibreOffice 26.2.4.2:

| L | tall run, 150% | label, 150% | tall run, 200% | label, 200% |
|---:|---:|---:|---:|---:|
| 14 | +8.05 | +8.05 | +16.10 | +16.10 |
| 20 | +11.50 | +11.50 | +23.00 | +23.00 |
| 28 | +16.10 | +16.10 | **+32.20** | **+32.20** |
| — (none) | +6.90 | — | +13.80 | — |

A label and a run of the same box give the same answer in all twelve cells. **Round 47's
figures reproduce exactly** (+16.10 / +23.00 / +32.20 at 200%), on a different binary from
the one it measured. Our tree matched every row to ±0.05.

### 2.3 The base is per side, not per box — `labelshape.py`

Round 47 varied only the level's **size** within one family, where a taller label is taller
above *and* below at once and the two candidate rules are identical by construction. A
level naming a different **face** at the *same* size separates them. Metrics per 12 pt, from
`hhea`:

| face | ascent | descent | box |
|---|---:|---:|---:|
| Liberation Serif (the item) | 11.20 | 2.60 | 13.80 |
| Liberation Sans | 11.26 | 2.54 | 13.80 |
| Carlito | 11.43 | 3.22 | 14.65 |
| Caladea | 10.80 | 3.00 | 13.80 |
| Liberation Mono | 9.99 | **3.60** | **13.59** |

Baseline gap from the item to the paragraph below, 12 pt level over 12 pt item:

| level's face | LibreOffice | ours **before** | ours **after** | what each rule predicts |
|---|---:|---:|---:|---|
| Liberation Serif | 13.80 | 13.80 | 13.80 | no change either way — control |
| Liberation Sans | 13.80 | 13.80 | 13.80 | ascent term already fired — control |
| Carlito | 14.40 | 14.40 | 14.40 | box term already fired — control |
| **Caladea** | **14.20** | 13.80 | **14.20** | per side 11.20+3.00 = 14.20; box rule 13.80 |
| **Liberation Mono** | **14.80** | 13.80 | **14.80** | per side 11.20+3.60 = 14.80; box rule 13.59 |

And the same two rows at 200%, where the *proportional base* is read rather than the raw
line:

| level's face | LO share | ours before | ours after |
|---|---:|---:|---:|
| Caladea | 14.20 | 13.80 | **14.20** |
| Liberation Mono | 14.80 | 13.80 | **14.80** |

Both are `max(ascent) + max(descent)` and neither is any portion's own box. Two independent
faces, each predicted to the hundredth from its font tables before the render, with three
controls that must not move and did not.

**The law, as it now stands measured:**

> A list label is a portion in its line. The line's ascent is the maximum over every
> portion's ascent including the label's, and the line's descent the maximum over every
> portion's descent including the label's. Proportional line spacing above 100% extends the
> line by `(p − 100)%` of `max(ascent) + max(descent)` taken over the portions that are not
> flies-in-content — the label among them, an as-character picture not.

### 2.4 Two further laws, measured and pinned, not acted on — `followup.py`

**`w:lineRule="atLeast"`: the extra room goes above the baseline.** All of it.

| minimum | LO gap above | our gap above | LO line total | our line total |
|---:|---:|---:|---:|---:|
| 300 tw (15.00 pt) | 15.00 | 13.80 | 28.80 | 28.80 |
| 360 tw (18.00 pt) | 18.00 | 13.80 | 31.80 | 31.80 |
| 480 tw (24.00 pt) | 24.00 | 13.80 | 37.80 | 37.80 |
| 600 tw (30.00 pt) | 30.00 | 13.80 | 43.80 | 43.80 |

Four points, exact: LibreOffice's ascent is `natural ascent + (minimum − natural height)`
and ours is the natural ascent, with the whole difference taken off the descent instead.

**Proportional spacing below 100%: the shrunk line's ascent is four fifths of its height.**

| p | line height | LO ascent | LO ascent ÷ height | our ascent |
|---:|---:|---:|---:|---:|
| 50 | 6.90 | 5.50 | 0.797 | 11.20 |
| 60 | 8.28 | 6.60 | 0.797 | 11.20 |
| 75 | 10.35 | 8.25 | 0.797 | 11.20 |
| 90 | 12.42 | 9.90 | 0.797 | 11.20 |
| 50, with a 28 pt run | 16.09 | 12.85 | 0.799 | 26.13 |

Ten points across five percentages and two line heights. This is Writer's
`nAsc = (4 * nLineHeight) / 5` under `PROP_LINE_SPACING_SHRINKS_FIRST_LINE`.

**Both leave the line's total height identical** — every `total` column above agrees to
0.05 — so neither can move a page, a word count or a font. Both misplace drawn text
vertically, by up to 16.20 pt (`atLeast` at 600 twips) and 13.30 pt (50% over a 28 pt run).
Neither is fixed here: the census below measures their DOCX corpus reach at **nought
paragraphs**, and a change that can only cost regressions is not worth making blind. They
are recorded so the next round does not have to re-derive them.

### 2.5 The control that killed an instrument

`labelshape.py` also put IPAGothic and WenQuanYi Zen Hei levels over Latin items, and both
showed large divergences — 2.15 and 3.10 pt above the baseline. **They are not label
findings.** `followup.py`'s `run-F` family renders an *unlabelled* paragraph whose own run is
that face, and the same divergence is already there:

| face | plain run, ours − LO (total line) |
|---|---:|
| Liberation Serif / Sans / Carlito | +0.00 |
| Liberation Mono | +0.00 (±0.01 either side) |
| Caladea | +0.00 total, +1.80 in the ascent/descent split |
| OpenSymbol | **+3.05** |
| IPAGothic | **−3.20** |
| WenQuanYi Zen Hei | **−4.10** |

So our line metrics for those three faces disagree with LibreOffice's before any label is
involved, and any label row measured on them measures the font metrics. Only the faces
whose plain run already agrees — Liberation Mono to 0.01, Caladea to 0.00 in total — carry
the label conclusion in §2.3, which is why those two are the ones the fix is pinned on.
The face-metrics divergence is real, off this round's subject, and left where it is.

One further by-product, recorded and not chased: a 30 pt **subscript** run
(`w:vertAlign="subscript"` with `w:sz="60"`) contributes 28.05 pt of ascent to LibreOffice's
line and 16.25 to ours — an 11.80 pt gap on a plain unlabelled paragraph. It is a bigger
number than anything in this round and it belongs to whoever takes vertical alignment.

---

## 3. Corpus reach

### 3.1 The census, and the method

`census.py`, over the **134 DOCX** of `words/`. It resolves rather than declares. For every
`w:p` it walks `w:docDefaults` → the `w:style` chain through `w:basedOn` → `w:pPr/w:rPr`
(the paragraph **mark**, which is what a label inherits, not the first run) → the level's
`w:lvl/w:rPr`; resolves numbering through `w:numPr`, the paragraph style's own `w:numPr`,
`w:numStyleLink`, `w:styleLink` and `w:lvlOverride`; and then turns each side into a real
ascent and descent by loading the font `fc-match` returns for the family — the same
substitution LibreOffice performs — so a face-driven label is **scored** rather than left in
a band that cannot be scored. That last step is what round 47 named as the thing it could
not do.

| population | documents | paragraphs |
|---|---:|---:|
| resolves to a drawn label at all | 103 | 17851 |
| label's box or ascent exceeds the item's — the old gate already fired | 24 | 2742 |
| …and at proportional spacing above 100% (round 47's population) | 9 | 208 |
| **label's descent alone exceeds the item's — the population the old gate dropped** | **33** | **1943** |
| …of which the label differs by **face** rather than size | 33 | 1943 |
| either band (the prediction's "reaches past its item") | **47** | — |
| proportional spacing **below** 100% | **0** | **0** |
| `w:lineRule="atLeast"` whose minimum actually binds | **0** | **0** |

Every one of the 33 is a face difference and none is a size difference, which is the shape
the law predicts: a level that changes *size* is almost always bigger on both sides at once
and lands in the first band.

### 3.2 The measured reach

Rendered all 200 words documents with the CLI at `HEAD` and with this worktree's, and scored
both against the stored 26.2.4.2 references at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` using `batch-check.sh`'s three checks in its
order and its 2% word band (`sweep.py`).

| | before | after |
|---|---:|---:|
| documents matching | 154 | **154** |
| absolute page error | 117 | **117** |
| exactly-correct page counts | 163 | **163** |
| absolute word error | 7023 | **7023** |
| renderings failed | 0 | 0 |
| **renderings changed, byte for byte** | — | **37 of 200** |
| of those, page count moved | — | **0** |

Of the 37: **29 DOCX and 8 `.doc`**. Against the census:

| | |
|---|---:|
| census predicted (DOCX) | 33 |
| DOCX renderings that changed | 29 |
| in both | 28 |
| census predicted, did not change | 5 |
| changed, census did not predict | 1 |
| `.doc` renderings that changed, none of them censused | **8** |

The census is well calibrated on the half it can see and **completely blind to the half it
cannot**, exactly as `prediction.md` said in advance: the 66 `.doc` carry their levels in
WW8 `LSTF`/`LVLF`, no zip-level census reads them, and the fix is in `XRay.Office.Text` and
`XRay.Office.WordProcessing` where both readers meet. Eight of them moved.

`OM template for non-complex NCC operators_August 2016.docx` — **the one document round 47
measured as its entire reach** — holds 691 deeper-only paragraphs, the largest count in the
corpus, and is in the changed set with the largest content delta (+426 bytes).

### 3.3 Cross-track

`XRay.Office.Text` is shared, so this owes a measurement rather than an argument. All 163
slides and 171 sheets rendered at both trees with `SOURCE_DATE_EPOCH` pinned:
**0 of 334 renderings differ.** The argument that predicted it — `RaisesTextHeight` is set
only by `PageParagraph.MeasurementObjects` in `XRay.Office.WordProcessing`, and the new
`textAscent`/`textDescent` differ from the old `textHeight` only when such an object is
present — is confirmed rather than relied on.

---

## 4. The implementation gap, with file and line

Two places, both in the tree as it stood at `4cbaeb41c3b`.

**`dotnet/src/XRay.Office.WordProcessing/Layout/PageContent.cs`:402** — the gate that decides
whether a label reaches measurement at all:

```csharp
return height > own || ascent > ownAscent ? (height, ascent) : null;
```

`OwnExtent()` at **:412** returned `(Height, Ascent)` and **carried no descent at all**, so
there was nothing to compare a descent against. This is the structural gap the handover
pointed at — the same shape as `MeasureLine` lacking a line list for the ODF case: a
quantity the rule needs was never plumbed to where the rule is applied. A label deeper than
its item but no taller overall was dropped on the floor and never reached
`MeasuredParagraph`, which would have folded it in correctly.

**`dotnet/src/XRay.Office.Text/Layout/MeasuredParagraph.cs`:661-664** — the fold, for the
labels that did get through:

```csharp
if (one.RaisesTextHeight)
{
    textHeight = Length.Max(textHeight, one.AboveBaseline + one.BelowBaseline);
}
```

The label's own box, where the two lines above it in the same loop already do the right
thing for the *line* (`ascent = Max(ascent, …); descent = Max(descent, …)`). The
proportional base was the one place composed by box rather than by side.

### What changed

`OwnExtent` now accumulates a per-face descent and returns `(Height, Ascent, Descent)`; the
gate gains `|| height - ascent > ownDescent`. `MeasureLine` keeps `textAscent`/`textDescent`
beside the line's own and folds a flagged object into both, taking `textHeight` from their
sum. Four files, 236 insertions, 74 deletions. `dotnet build -v q -nologo`: **0 warnings,
0 errors**.

Three of round 47's own tests had to be changed, and that is worth stating plainly rather
than burying. They built their label as `new InlineObject(0, Length.Zero, Label, Ascent:
null, …)` — `Ascent: null` means *all of it above the baseline*, which is the ordinary
inline picture and is a shape a label can never have, since
`PageParagraph.MeasurementObjects` always supplies a real ascent from `PageLabel.LineExtent`.
Under a box rule that fixture is harmless; under the per-side rule it asks a different
question. They now state a real 28 pt ascent (`28 × 1911/2048`) and assert the same
measured numbers they always did. One assertion in
`APictureBesideATallerLabelTakesNoShareOfThePercentage` gained a one-twip tolerance, because
`LineSpacingRule.Apply` snaps to whole twips and a real fractional ascent does not.

### Tests

| test | project | verified how |
|---|---|---|
| `ALabelDeeperThanItsTextRaisesTheFirstLineEvenWhenItsBoxIsShorter` | WordProcessing | **by reintroduction** — removing `|| height - ascent > ownDescent` fails this test and only this test (1 of 763) |
| `ALabelTallerAboveOrEqualOnBothSidesIsUnaffectedByTheDescentRule` | WordProcessing | **drift guard** — the two controls, which the mutation does not move |
| `ALabelDeeperThanItsTextWidensTheBaseByItsDescentAlone` | Text | **by reintroduction** — restoring the box fold fails this test and only this test (1 of 289) |
| `ALabelDeeperThanItsTextDoesNotRaiseTheBaseAboveTheBaseline` | Text | **drift guard** — pins that the ascent side is still a maximum |

`verify-test.sh` refuses to start on a dirty tree and I am instructed not to commit, so the
reintroduction was done by hand with backups of both source files and an explicit `touch`
before each rebuild — the stale-mtime trap the script's own header warns about. Both
mutations were applied one at a time, built, run, and restored; the restored tree rebuilds
clean and `git diff --stat` shows only the four intended files.

**Full suite, ten non-Fidelity projects, run one at a time in the foreground:**

| project | passed | project | passed |
|---|---:|---|---:|
| Containers | 109 | Rendering | 120 (1 skipped) |
| Core | 284 | Spreadsheets | 621 |
| Markup | 259 | Text | 289 |
| OpenDocument | 125 | Vector | 295 |
| Presentations | 592 | WordProcessing | 763 |

**3457 passed, 1 skipped, 0 failed.** Fidelity was not run, per instruction.

---

## 5. Measured, inferred, and not established

**Measured**, each from a render of an authored document against the installed 26.2.4.2 and
read out of the PDF with `pdftotext -bbox`:

- The gap is applied above each line and sized by that line's own content; the paragraph's
  first line gets none (§2.1, 126 pitches).
- A label takes the same share of proportional spacing as a run of the same box, at three
  sizes and two percentages (§2.2, 12 cells plus 4 controls). Round 47 reproduces.
- The line and the base are composed per side of the baseline, pinned on two faces
  predicted to the hundredth with three controls (§2.3).
- `atLeast` puts all its extra above the baseline, four points (§2.4).
- A shrunk line's ascent is 4/5 of its height, ten points (§2.4).
- Our metrics for OpenSymbol, IPAGothic and WenQuanYi Zen Hei disagree with LibreOffice's
  on a plain unlabelled run (§2.5).
- 37 of 200 renderings changed, 0 page counts, 0 verdicts; 0 of 334 cross-track (§3.2, §3.3).

**Inferred**, and marked as such:

- That `SwLineLayout::CalcLine`'s running maxima are the mechanism behind §2.3. The
  behaviour is measured; the attribution is a reading of a 27.2-alpha tree and is offered as
  explanation, not evidence. Two of my predictions from exactly that kind of reading were
  refuted this round.
- That the census's 33 documents and the sweep's 29 are the same phenomenon. 28 overlap;
  the 5 non-movers and the 1 surprise were not individually diagnosed.
- That the 8 changed `.doc` are the same rule reached through `Ww8DocumentReader`. They
  changed, and nothing else in the diff could have changed them, but no WW8 document was
  read at level detail.

**Not established:**

- **Why 5 censused documents did not move.** Most likely the label's extra descent is
  smaller than a twip after rounding, or the paragraph's own runs already supply a deeper
  descent than the paragraph mark does — the census resolves the *mark*, and the line takes
  the maximum over the actual runs. That is a real over-count in the census and the
  direction is safe (it over-predicts), but it is a guess.
- **The `.doc` half's population.** 8 of 66 moved; nothing says how many could.
- **The ODF and RTF readers.** `OdtLayoutSource.Lists.cs` and `RtfReader.cs` build the same
  `PageLabel` through the same `LabelExtent`, so both are reached; `words/` holds no `.odt`
  or `.rtf`, so the measured reach there is zero and the real reach is unknown.
- **`w:rStyle` inside `w:lvl/w:rPr`.** Neither `DocxLayoutSource.Lists.cs`'s `LabelSize` nor
  my census follows a character style named by the level, so both agree for the wrong
  reason. Population unmeasured.
- **Whether `atLeast` and the sub-100% shrink matter outside DOCX.** Their DOCX reach is
  nought; the `.doc`, `.odt` and `.rtf` populations were not censused.

**On the CLI.** My brief said there was none and the firewall opened mid-round, before the
first render. Everything above therefore has a real *ours* column and a real corpus sweep;
nothing in this document is a hand-computed stand-in for a measurement. The only trace left
of the constraint is `prediction.md`'s original text, which is kept as written.

---

## 6. What this round is worth

A spacing law is invisible to all three gates unless it moves a page, and this one does not:
**0 verdicts, 0 page counts, on a change that alters 37 of 200 renderings.** That was
predicted before measuring and it is the result, said plainly rather than dressed up. What
the round buys is that the label rule is now composed the way Writer composes it, on
evidence from two faces and five controls rather than from one family scaled up; that
1943 paragraphs across 33 DOCX and 8 `.doc` are drawn at the right line height where they
were not; and that two further line-height laws are pinned with slopes so the next round can
act on them if it finds a population that cares.

### Files

`prediction.md` (written first), `probelib.py` (builder and read-back), `pitch.py`,
`lastline.py`, `labelshape.py`, `followup.py` (the four probes), `census.py` (the resolving
census), `sweep.py` (the before/after corpus sweep). Every fixture is newly authored and
minimal; nothing was copied or excerpted from the corpus.
