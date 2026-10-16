# font-class-01 — the shape a family name implies comes from fontconfig, not from `VCL.xcu`

Round brief: `fc-match "Century Schoolbook"` answers DejaVu **Sans** and our `ClassOf` answers DejaVu
**Serif**, recorded at `TODO.batches.md:2361` and deliberately left for a round of its own because
the fix reaches all three tracks.

**The brief is right and the defect is bigger than the one name.** Over the 296 font families the
whole 534-document corpus mentions, our resolver disagreed with the installed 26.2.4.2 on **22**.
Thirteen of those are now fixed and none regressed: the resolver agrees with the reference binary on
**287 of 296**, against 274 before. The remaining nine are four pi faces the instrument cannot judge,
one unmeasurable, and four that are two named leads.

`prediction.md` was committed at `d02605c60f9`, before the change was written and before any effect
of it was measured.

---

## 1. What fontconfig actually does

Three rules, in this order, and only the first is what LibreOffice's own configuration also holds:

1. **`<alias><family>X</family><default><family>G</family></default></alias>` files a family under a
   generic.** `45-latin.conf` does this for 46 Latin families, `40-nonlatin.conf` for 75 more,
   `45-generic.conf` for the emoji, maths and system-UI faces.
2. **The `<default>` chain runs through *concrete* families, and `<accept>` and `<prefer>` extend
   it.** `30-metric-aliases.conf` — 63 of the 206 classifications on this machine — defaults a family
   to the metric group's canonical name rather than to a generic. `Century Schoolbook` defaults to
   `New Century Schoolbook`, which is filed under nothing at all. `Palatino` is filed under nothing
   either, but *accepts* `Palatino Linotype`, which `45-latin.conf` files under `serif` — the
   accepted family joins the pattern and brings its generic with it.
3. **A family reached by none of that is a grotesque.** `49-sansserif.conf` appends `sans-serif` to
   any pattern that has not already named a generic. **fontconfig has no "unknown".**

Which is why `fc-match "Century Schoolbook"` answers DejaVu Sans, and `fc-match Palatino` answers
DejaVu Serif, and the two look inconsistent until you follow the chain.

### What we did

`FontSubstitutions.ClassOf` reads `FontType` out of `VCL.xcu` — `Normal,Serif` for Century
Schoolbook, `Normal,Fixed` for Lucida Console, nothing at all for Palatino Linotype. That is
LibreOffice's own classification and **it is not what the running binary uses on Linux**, because
`PhysicalFontCollection::FindFontFamily` asks the fontconfig pre-match hook at
`PhysicalFontCollection.cxx:1142` and returns its answer at `:1151`, while `ImplFontSubstitute` — the
`SubstFonts` chain and this classification — is only reached in the second loop at `:1180`.

### Where they diverge, measured on the binary rather than on the source

The 296-row flat-ODS face probe from `research/probes/sheets-r17`, re-taken on the installed
**26.2.4.2** (`lo-faces-26.tsv` here). It is worth recording that the gold table barely moved from
24.2.7.2 to 26.2.4.2: **1 family of 296** changed answer, `Cambria Math`. Font resolution is far more
stable across the version bump than page counts were.

`fc-answers.tsv` is `fc-match` asked directly for the same 296 names, bare and with each generic
appended. **`fc-match` and LibreOffice 26.2.4.2 agree on 288 of 296**, and every one of the eight
disagreements is an artefact of the `fc-match` *command*, not of fontconfig: `FcNameParse` reads `-`
as introducing a size and `,` as separating families, so `Arial-BoldMT`, `Calibri-Bold`,
`Helvetica-Narrow`, `Times-Italic`, `Times-Roman` and `TimesNewRoman,Bold` are not asked as written.
**Anything measured with `fc-match` on a name containing punctuation is measuring the CLI's parser.**
LibreOffice passes the family through `FcPatternAddString` and does no such parsing.

Our 22 disagreements with 26.2.4.2, by cause:

| cause | families | acted on |
|---|---|---|
| the classification: `VCL.xcu` says roman or fixed, fontconfig files it under nothing → sans | `Century Schoolbook`, `Century`, `NewCenturySchlbk`, `Book Antiqua`, `Bookman Old Style`, `CG Times`, `Times-Roman`, `Lucida Console` | **yes** |
| the classification, the other way: unknown to the table, filed `serif` by fontconfig | `Palatino Linotype`, `SimSun`, `ＭＳ 明朝` | **yes** |
| the chain reaching a face fontconfig never names | `MS Gothic`, `MS PGothic` | **yes** |
| pi faces — the probe is the wrong instrument | `Wingdings`, `Wingdings 2`, `Wingdings 3`, `Webdings` | no, and must not be |
| the probe could not name the face | `Symbol` | no |
| a metric alias we do not reach | `Nimbus Sans L` | **lead** |
| name canonicalisation | `Times New Roman CE`, `Times New Roman CYR`, `TimesNewRoman,Bold` | **lead** |

---

## 2. The chain was already dead, and the measurement says so cleanly

Before deciding anything, each of the 296 was classified by *which branch of the resolver answered
it*. The split is the round's sharpest result:

| branch | families |
|---|---:|
| the family is installed | 2 |
| **LibreOffice's `SubstFonts` chain** | **18** |
| the generic shape fallback | 276 |

Eighteen. And they partition exactly:

* **Eight the chain gets right** — `Arial`, `Calibri`, `Cambria`, `Courier`, `Courier New`,
  `Helvetica`, `Times`, `Times New Roman`. fontconfig has an `<alias>` naming every one of them, and
  its own alias expansion reaches the same installed face our chain does. The chain is a faithful
  stand-in for an expansion this resolver does not implement.
* **Six pi faces** — `Symbol`, `Wingdings` ×3, `Webdings`. A symbol-encoded request makes the hook
  bail at `fontsubst.cxx:101` before fontconfig is consulted, so for these the chain is what
  LibreOffice runs too.
* **Four defects** — `CG Times`, `Times-Roman`, `MS Gothic`, `MS PGothic`. fontconfig has no
  `<alias>` naming any of them, and the chain sends all four to a face 26.2.4.2 does not use.

So the rule is derivable rather than hardcoded: **a family fontconfig names nowhere does not reach
the chain.** It reproduces the existing `{helv, sansserif}` carve-out — neither is named by any
`<alias>` in a stock configuration — and it fixes `CG Times` and `Times-Roman`, whose chains name
`liberationserif` and would otherwise have survived the classification change untouched.

### `MS Gothic` is the objection, and it does not hold

The natural exemption is that fontconfig answers by *character* and our request carries none, so an
East Asian family might deserve its chain entry. It does not. Measured on 26.2.4.2 with one authored
FODT:

| declared family | text | LibreOffice draws |
|---|---|---|
| MS Gothic | `Hamburgefonstiv` | DejaVu Sans |
| MS Gothic | 日本語のテキストです | **WenQuanYi Zen Hei** |
| SimSun | 中文文本测试 | WenQuanYi Zen Hei (through DejaVu Serif's fallback) |

The chain's answer — IPAGothic — is neither. And the last row is this architecture working as
designed: `SimSun`'s primary face is DejaVu **Serif**, which is fontconfig's classification of the
name, and the Han characters arrive through glyph fallback, which reads the same configuration and
ranks WenQuanYi Zen Hei first. Nothing needed a CJK carve-out.

---

## 3. What changed

**`FontconfigPreferences` learns the classification half of the file it was already parsing.**
`GenericClassOf` walks `<default>`/`<accept>`/`<prefer>` edges of concrete subjects breadth-first to
the first generic and answers `SansSerif` when it reaches none, which is `49-sansserif.conf`;
`Names` says whether the configuration mentions the family at all; `IsConfigured` says whether there
is a configuration to ask. The generics with no face list in this resolver — `math`, `system-ui`,
`cursive`, `fantasy`, `emoji` — map to `SansSerif`, which is not a claim about their shape but about
where a pattern filed under them lands: `Cambria Math` is filed `math` and 26.2.4.2 draws it in
DejaVu Sans.

**`SystemFontResolver` takes its shape from that**, through `ShapeOf`, and consults the chain only
through `ConsultsTheChain`.

**`FontSubstitutions.ClassOf` is kept, and its remarks now say what it is for.** Two things, both
real: it is the whole answer on a machine with no fontconfig, where the pre-match hook does not exist
and `ImplFontSubstitute` really is what runs; and it is the only source of `FontFamilyClass.Symbol`,
which fontconfig has no generic for and which decides the pi-face carve-outs on both sides of the
resolver. `FontconfigOverridesTheChain`, the hardcoded `{helv, sansserif}`, is gone — derived, not
deleted.

### The declared-family logic is not made redundant, and the words track proves it

The brief warned about this and it is the round's most useful negative result. **The declared class
outranks fontconfig's classification of the name, and on the words track it dominates.**
`195584360.docx` declares `Book Antiqua` as `<w:family w:val="roman"/>` in its font table; the
declared-roman rule routes it to DejaVu Serif and the classification change never gets a look in —
which is correct, and is why the words track moved 2 renderings where slides moved 12. No
presentation reader sets a declared class, so there the name's classification decides everything.

Three new rows pin the composition directly (`ADeclaredShapeStillBeatsFontconfigsClassificationOfTheName`):
Century Schoolbook declared roman is DejaVu Serif although fontconfig files the bare name under
nothing, and Palatino Linotype declared swiss is DejaVu Sans although `45-latin.conf` files it under
serif. Nothing was deleted and the four names that fix the ordering — `Times`, `Thorndale`,
`Helvetica`, `Albany` — keep their own theory unchanged.

### A second change the first one made necessary: the declared *pitch* on a slide

Two decks got measurably **worse** on the first pass, both naming `Lucida Console`:
`introduction_to_bea_tuxedo.ppt` went from an exact face-set match to four faces wrong, and
`airbus-powerpoint-presentation-2019-20…pptx` to one. The reference draws DejaVu Sans **Mono** in
both, and the `VCL.xcu` `Fixed` classification had been supplying that answer by accident.

The cause is not the classification. Both decks *declare* the pitch — PPTX as
`pitchFamily="49"` on `<a:latin>`, PPT as `lfPitchAndFamily` `0x31` at the end of the
`FontEntityAtom` — and no presentation reader read it. `words-pages-01` §5 named this exact gap as a
lead. It is settled by an isolated A/B on the real binary (`pitchprobe.sh`, committed): re-zipping
the deck with that one attribute removed and nothing else changed, LibreOffice draws DejaVu **Sans**
instead of DejaVu Sans Mono. Control included, because re-packing a corpus file is itself a change.

`SlideFonts.DeclaredPitches`, fed by `PptxSlideLayout.ReadDeclaredPitches` and by
`PptFontTable.PitchOf`. **The pitch and not the family class**, deliberately: the family bits are in
the same byte, the word processor's equivalent leaves them alone for the same reason, and a declared
family class on a slide has never been measured. With it, both decks return to exact and the round
has no regression anywhere.

---

## 4. Reach, by both instruments, on all three tracks

The seat is shared: **words (200), slides (163) and sheets (171) were all swept, twice each**, before
and after, against the banked 26.2.4.2 references with `SOURCE_DATE_EPOCH` set. `sweep.sh` here is
`words-pages-01`'s with the track parameterised and the embedded *face names* recorded rather than
counted. Its baseline reproduces `words-pages-01`'s recorded whole-track figures exactly — 157 match,
166 page-exact, 113 total page error — which is the instrument validating itself against an
independent run.

### The gate sees nothing at all

| track | | before | after |
|---|---|---:|---:|
| words (200) | match / page-exact / page error | 157 / 166 / 113 | 157 / 166 / 113 |
| slides (163) | match / page-exact / page error | 144 / 163 / 0 | 144 / 163 / 0 |
| sheets (171) | match / page-exact / page error | 155 / 162 / 329 | 155 / 162 / 329 |

**Not one verdict moved and not one page count moved, on any of the 534 documents.** That is the
expected result for this class of change and it is why the brief asked for the other instrument.

### Face-set distance to the reference

Symmetric difference between the faces `pdffonts` reports for our PDF and for the reference's, subset
prefixes stripped, over every rendering whose face set changed:

| track | renderings moved | closer | unchanged | further |
|---|---:|---:|---:|---:|
| words | 2 | **2** | 0 | 0 |
| slides | 10 | **10** | 0 | 0 |
| sheets | 1 | **1** | 0 | 0 |
| **total** | **13** | **13** | **0** | **0** |

Five go to an **exact** match with the reference's face set: `AWR OPS-AOC 044…docx`,
`Fundamentals_Module_1_basics.ppt`, `RRM-training-syllabus…Dec-2009.ppt`, `Architecture.ppt`,
`template-ECSPR-notifications.xlsx`.

**`solog_orientation_august_2019.pptx`, the document the defect was found on, gains the
`DejaVuSans-Bold` it was missing** — its Century Schoolbook runs, exactly as the brief said. Its
residue is a `LiberationSans-Bold` we draw that the reference does not and an `OpenSymbol` it draws
that we do not; the second is a separate class visible on four of the moved decks and is a lead.

### Why the words track barely moves, stated as reach rather than as a grep

Roughly 70 corpus documents *name* an affected family. Only 13 renderings changed, and the reason is
what a request **resolves** to rather than what a file mentions:

* on **words**, a DOC or DOCX font table declares a family class for nearly every entry, and the
  declared class outranks the name's classification — so the change is invisible on the 24 word
  documents naming these families except the two that reach `MS Gothic`, which no font table declares;
* on **slides**, nothing declares a class, so the classification decides — 12 of 163;
* on **sheets**, the declared class is wired too, and the one document that moved reaches `MS Gothic`.

---

## 5. Regression

`batch-check.sh` against the banked references, batches 001–006 of every track touched — all three.

```
words/batch-00[1-6]     TOTAL 60  MATCH 59  MISMATCH 1
slides/batch-00[1-6]    TOTAL 58  MATCH 57  MISMATCH 1
sheets/batch-00[1-6]    TOTAL 60  MATCH 57  MISMATCH 3
```

Unchanged from the state the round was briefed with. The remaining failures are the documented
ceilings — `solog` and the Lease-Transition twins in `TODO.raster-ceiling.md`, whose word deltas are
`pdftotext` reading LibreOffice's own positioning as word breaks — plus `1447.doc` at 3 pages against
4, whose residue is the line-height law and which this round deliberately did not chase.

### Tests

Every project run individually, counts read rather than colours.

| project | passed | failed | skipped | total |
|---|---:|---:|---:|---:|
| XRay.Office.Core.Tests | 313 | 0 | 0 | 313 |
| XRay.Office.Containers.Tests | 109 | 0 | 0 | 109 |
| XRay.Office.Markup.Tests | 259 | 0 | 0 | 259 |
| XRay.Office.OpenDocument.Tests | 125 | 0 | 0 | 125 |
| XRay.Office.Presentations.Tests | **653** | 0 | 0 | 653 |
| XRay.Office.Rendering.Tests | 150 | 0 | 1 | 151 |
| XRay.Office.Spreadsheets.Tests | 758 | 0 | 0 | 758 |
| XRay.Office.Text.Tests | **339** | 0 | 0 | 339 |
| XRay.Office.Vector.Tests | 295 | 0 | 0 | 295 |
| XRay.Office.WordProcessing.Tests | 818 | 0 | 0 | 818 |
| XRay.Office.Fidelity.Tests | 520 | **30** | 0 | 550 |

`XRay.Office.Fidelity.Tests` was **30 failed of 550** on the branch point, measured before anything was
changed, and is 30 of 550 now. `XRay.Office.Text.Tests` gains 29 (310 → 339) and
`XRay.Office.Presentations.Tests` 7 (646 → 653); both are this round's. `Rendering`'s single skip is on
every run measured this session and predates the branch. The build is warning-free.

**No committed fixture can carry the PPTX pitch case.** Every `.pptx` in `tests/corpus/features`
writes `pitchFamily="0"` on every typeface — scanned, 16 of 16, because LibreOffice's own exporter
never states a pitch — so a fixture test would pin the no-op. The same trap `PptSymbolBulletTests`
documents for `.ppt` bullets. The binary half *is* built by hand and tested end to end, the shared
decoding is pinned directly, and the PPTX plumbing rests on the corpus measurement and on
`pitchprobe.sh`.

---

## 6. Scoring `prediction.md`

| # | claim | conf | outcome |
|---|---|---:|---|
| P1 | ≥ 288 of 296 agree with the gold table, against 274. | 0.70 | **wrong, by one** — 287. 13 moved, all 13 correct, 0 regressions. Called too finely; the honest claim was "13 of the 22, and the other 9 are two named leads plus the pi faces". |
| P2 | The pi faces keep OpenSymbol. | 0.90 | **right** |
| P3 | `MS Gothic`/`MS PGothic` move to DejaVu Sans and cost no verdict. | 0.60 | **right**, and better founded than predicted — the Japanese probe showed IPAGothic is wrong for CJK too. |
| P4 | Renderings changed: words 20–50, slides 15–45, sheets 8–25. | 0.50 | **wrong, badly, on all three** — 2, 10, 1. The mistake is precisely the one the rules of evidence warn about: I estimated from the ~70 documents that *name* an affected family instead of from what those requests resolve to. On words and sheets the declared class already decided, so the family being named changed nothing. |
| P5 | Net closer on every track; words ≥ 8 closer and ≤ 3 further; slides ≥ 10 closer. | 0.55 | **half right** — every track is net closer and nothing anywhere is further, which is the stronger half; but words is 2, not ≥ 8, for P4's reason. Slides is exactly 10. |
| P6 | `solog_orientation_august_2019.pptx` gets closer. | 0.60 | **right** — 3 → 2, gaining the `DejaVuSans-Bold` the brief named. |
| P7 | `ABCD-FE-01-00` and `ABCD-WB-08-00` lose their extra DejaVu Serif. | 0.70 | **wrong** — neither moved at all. `Times-Roman` *does* now resolve to DejaVu Sans, and I checked the resolution and not the document: both declare their `Times-Roman` roman in the font table, so the declared-class rule answers first and the name's classification is never consulted. The same mistake as P4, on two named documents. |
| P8 | Fidelity stays at 30 of 550. | 0.60 | **right** |
| P9 | No batch verdict lost. | 0.50 | **right** — 59/60, 57/58, 57/60, all unchanged. |
| P10 | The declared-family logic is not made redundant. | 0.85 | **right**, and by a wider margin than expected: it is what stops the change reaching the words track at all. |

Five and a half of ten. **Every miss is the same mistake** — P1 excepted — and it is worth naming
because it is the one the brief explicitly warned about: I estimated reach from which documents
*mention* a family rather than from what those requests *resolve* to, and forgot that a rule landed
earlier the same day already answers first for the entire word-processing and spreadsheet corpus. The
one prediction I made about a mechanism rather than a count, P10, is the one that turned out to
understate the case.

What no prediction covered at all is the regression the change *created*: two decks whose declared
fixed pitch had been arriving by accident through `VCL.xcu`'s classification. Removing a wrong
answer's accidental cover is a failure mode worth predicting next time.

---

## 7. Leads

* **`Nimbus Sans L`** — 26.2.4.2 draws Liberation Sans, we draw DejaVu Sans. fontconfig files it
  `sans-serif` *and* metric-aliases it into the Helvetica group, and the alias is reaching an
  installed face our chain does not name. One family; the general shape is that we do not implement
  fontconfig's alias expansion, only approximate it with `VCL.xcu`'s chain.
* **Name canonicalisation** — `Times New Roman CE`, `Times New Roman CYR` and `TimesNewRoman,Bold`
  all answer Liberation Serif in 26.2.4.2 and DejaVu Sans here. LibreOffice's
  `GetEnglishSearchFontName` strips the charset suffix and the style suffix; `Normalise` does not.
  Three of the 296, and the third is drawn by a corpus document.
* **`OpenSymbol` the reference draws and we do not** — visible on four of the twelve decks that moved
  (`solog`, `8_P-Pavese_AIRBUS-ATB`, `30-04-2021 merged NDoH`, `PRM_training`). A bullet class, not a
  substitution class, and unrelated to this round.
* **A declared family class on a slide or a sheet.** `pitchFamily`'s high four bits carry it and this
  round reads only the low two. On words it was worth 29 renderings; nothing has measured it on the
  other two tracks, and the instrument to do it with is `sweep.sh`'s face column.
* **The classification is a property of the machine.** `GenericClassOf` reads `/etc/fonts`, so a
  figure in this file is reproducible on a box with the same configuration and not elsewhere. That is
  already true of `FontconfigPreferences` and is the right trade for the same reason — the reference
  renderer asks the same files — but it is now true of the *primary* face and not only of glyph
  fallback, which is a wider surface. A machine with no fontconfig falls back to `VCL.xcu` and is
  pinned by `WithNoFontconfigTheTablesShapeIsStillTheAnswer`.
