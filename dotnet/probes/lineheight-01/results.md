# lineheight-01 — the one twip, reconstructed

Round `lineheight-01`, 2026-08-15, worktree `wt-lineheight`. Reference LibreOffice **26.2.4.2**,
Carlito / Caladea / Liberation / DejaVu / OpenSymbol / IPAGothic / WenQuanYi all resolving.
References reused from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`, never re-rendered;
`SOURCE_DATE_EPOCH=1700000000` on every render that is diffed.

Prediction written and committed before any probe of my own was run: `prediction.md`, commit
`443694df511`.

**The rule was found and it is not a rounding rule at all — it is a device.** It reproduces
**195 of 195** measured (face, size) pairs exactly, ascent and line height both, and a further
**234 of 234** on eight faces neither prior round touched. The 39 pairs it does not reproduce are
all one face and all one *other*, separately identified rule, stated in §7.

Reach: **159 of 200 words renderings changed**, **2 verdicts won and 0 lost**, slides and sheets
**byte-identical, 0 of 334**. `Sample_SQMS_Program.docx` is 61 pages and matches;
`1447.doc` is 4 pages, 959 words against 959, and matches.

---

## 1. The rule

Writer never scales a face's design units to the point size. It formats against a **reference
device**, and every vertical metric is quantised onto that device's pixel grid on the way in and
back onto twips on the way out. Two earlier rounds swept device resolutions from 72 to 6000 dpi and
found nothing because the device is **8640 dpi**.

```
H  = size_twips * 6                                    the em, in whole device pixels
a  = round( hheaAscender  * H / upem )                 whole device pixels, separately
d  = round(-hheaDescender * H / upem )
g  = round( lineGap       * H / upem )

textHeight = round( (a + d) / 6 )                      the SUM converted, once
extLeading = round(  g      / 6 )                      the gap converted, on its own

lineHeight = textHeight + extLeading
ascent     = round( a / 6 ) + extLeading               Writer charges the leading to the ascent
```

`round` is half **away from zero** throughout.

| step | citation |
|---|---|
| the device is a `VirtualDevice` in `RefDevMode::MSO1`, `MapUnit::MapTwip` | `sw/source/core/doc/DocumentDeviceManager.cxx`:259 |
| `MSO1` is `6*1440` = **8640** dpi | `vcl/source/gdi/virdev.cxx`:407 |
| twips → pixels is `llround(n * (1/1440) * dpi)`, so ×6 exactly | `vcl/source/outdev/CoordinateMapper.cxx`:230 |
| the three metrics are rounded to whole pixels **separately** | `vcl/source/font/fontmetric.cxx`:538-540 |
| `mnLineHeight` is `ascent + descent`, still in pixels | `vcl/source/outdev/font.cxx`:910 |
| `GetTextHeight` converts that sum back in one step | `vcl/source/outdev/text.cxx`:640-651 |
| `GetFontMetric` converts the external leading separately | `vcl/source/outdev/font.cxx`:355 |
| pixels → twips is `llround(n / (1/1440) / dpi)` | `vcl/source/outdev/CoordinateMapper.cxx`:279 |
| Writer's line height is `GetTextHeight() + GetFontLeading()` | `sw/source/core/txtnode/fntcache.cxx`:336-376 |
| Writer's ascent is `GetFontMetric().GetAscent() + GetFontLeading()` | `sw/source/core/txtnode/fntcache.cxx`:297-329 |

### Why every previous candidate failed, in one case

Liberation Serif at 10 pt. The em is 1200 device pixels; the three metrics land on 1069.336,
259.570 and 50.977 pixels, so `a, d, g = 1069, 260, 51`.

| candidate | arithmetic | twips | |
|---|---|---:|---|
| scale the 2355-unit total once (**the tree, before**) | `2355 × 200 / 2048` | 230 | ✗ |
| round the three separately, straight to twips | `178 + 43 + 8` | 229 | ✗ |
| round three to pixels, convert the **sum** once (3 + 1) | `1380 / 6` | 230 | ✗ |
| the same but with .NET's default midpoint rule | `221.5 → 222`, `8.5 → 8` | 230 | ✗ |
| **round three to pixels, convert 2 + 1, halves away from zero** | `1329/6 = 221.5 → 222` plus `51/6 = 8.5 → 9` | **231** | ✓ |

LibreOffice draws **231**. That single case refutes all four alternatives at once, and it is kept
as a test — `ReferenceGridTests.OneCaseRefutesEveryRuleThatWasTriedBeforeThisOne` — so that
re-proposing any of them fails rather than being re-derived a fourth time.

### And why the sum could not decide

`words-pages-01` §4's refutation stands and is now explained. Liberation Serif (1825 + 443 + 87) and
Liberation Sans (1854 + 434 + 67) state the *identical* 2355 units over the same 2048-unit em, so
any rule that is a function of the total predicts one number for both — and LibreOffice draws two,
**differing in both directions**:

| pt | Liberation Sans | Liberation Serif |
|---:|---:|---:|
| 10 | 230 | **231** |
| 13 | **300** | 299 |
| 16 | **369** | 368 |

The two faces part company because their ascents and descents round to different whole *pixels*
before anything is added up. The split decides because the split is what is rounded.

## 2. The fit: 195 of 195, and 234 more

`probe-grid.py` authors one page per (face, size) — five faces, every half point from 5.0 to 24.0 —
and reads the two baselines off the reference PDF's own text matrices, so the first baseline's
distance below the top margin *is* the ascent and the gap between the two *is* the line height.

```
MODEL: line height 195/195 exact, ascent 195/195 exact
TREE:  line height 195/195 exact, ascent 195/195 exact      (149 and 164 before)
```

The same 195 pairs measured through the **ODT** path — the DOCX converted to ODT by the reference
and re-exported — are identical on all 195, so the reference device is the same for the ODF importer
and this is a Writer rule rather than a writerfilter one.

`PROBE_SET=extra` repeats it on eight faces no prior round measured — Liberation Mono, Liberation
Serif Bold, Liberation Sans Italic, Caladea Bold, DejaVu Serif, DejaVu Sans Mono, OpenSymbol and
IPAGothic. **234 of 273** exact; every one of the 39 misses is IPAGothic and every one of the 39 is
reproduced exactly by a *second* rule, §7.

### The briefed "22 of 195" was really 22 of 81

Worth recording, because it is the silent-truncation shape `dotnet/CLAUDE.md` warns about wearing a
new hat. `words-regress-01`'s `probe-ascent.py` keys its measurements off the label text in the
record `pdf-ops.py dump` prints — and above 14 pt the subset font's ToUnicode map defeats that
script's literal decoder, so the record carries a glyph count and no string. The parse silently
dropped **114 of the 195 pages** and the summary line still said 195.

Run as written it gives `line height 59/81` — 22 misses, the briefed number exactly. Keyed off the
**page number** instead, which is authored and always present, the same instrument gives
`line height 149/195` and `ascent 164/195`: **46 and 31 misses, not 22**. The rule found here is
exact on all 195 either way, so the correction does not change the conclusion — but the prior
round's "we agree on 173" understated the defect by half.

## 3. What was changed

Five source files, and the type that does the arithmetic already existed and was already right.

`MetricGrid` was written for the handful of documents setting `w:usePrinterMetrics`, and it already
grouped the conversion 2 + 1 — `TextHeightOn` converts ascent-plus-descent together and `LeadingOn`
converts the gap alone, both citing the C++. What was wrong was **which documents got a grid at
all**: every other document was scaled exactly, which is a thing Writer never does.

| file | change |
|---|---|
| `src/XRay.Office.Text/Fonts/LineSpacing.cs` | `MetricGrid.Reference` = 8640 dpi; all four roundings take a half away from zero rather than to even; a `QuantisesAdvances` component |
| `src/XRay.Office.Text/Layout/MeasuredParagraph.cs` | advances go through the grid only when it quantises them |
| `src/XRay.Office.WordProcessing/Layout/PageContent.cs` | `Metrics` defaults to `MetricGrid.Reference`, so DOCX, DOC, RTF and ODT all get it |
| `src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs` | `usePrinterMetrics ? Printer : Reference` |
| `src/XRay.Office.WordProcessing/Ww8/DocReader.cs` | the same |

### The midpoint rule is a second, independent defect

.NET's `Math.Round` takes a half **to even**; C++'s `std::round` and `llround` take it **away from
zero**. On a 300 dpi printer grid the halves essentially never arise; at six pixels to the twip they
are ordinary. **58 of the 195 pairs land on a midpoint** in at least one of the two conversions, and
**banker's rounding gets 30 of the 195 wrong** — so the grid alone would have fixed roughly half the
residue and left the rest looking like the rule was still not quite right.

### Advances deliberately do not go through this grid

The vertical metrics are quantised and the horizontal ones are not, and the asymmetry is measured
rather than assumed. `probes/printer-metric-advance.py`'s control half — the same authored body with
`fUsePrinterMetrics` **clear** — has unquantised scaling exact on 96 of 96 rows and the quantised
rule out by 6.73 pt. The dominant term in the advance rule is the em being rounded to whole pixels,
which a 300 dpi device makes worth 1.33% of every advance and an 8640 dpi device makes worth exactly
nothing; what is left is a floor worth a sixth of a twip. `MetricGrid.QuantisesAdvances` carries the
distinction so the reference device cannot silently acquire the printer's advance rule.

This is the one place the prediction was wrong: P5 said `MetricGrid` would need no structural
change, and it needed this field.

## 4. Reach, all 534 documents, both directions

Rendered twice — once with the fixed tree, once with a binary built from the same five files
reverted — with `SOURCE_DATE_EPOCH` set, so the two runs are byte-comparable with nothing masked.
Verdicts against the banked references, `batch-check.sh`'s three checks column for column.

| track | renderings changed | before | after | won | lost | Σ\|page error\| | Σ\|word error\| |
|---|---:|---:|---:|---:|---:|---|---|
| words | **159 of 200** | 167 | **169** | 2 | **0** | 96 → **94** | 5386 → 5391 |
| slides | **0 of 163** | 147 | 147 | 0 | 0 | 0 → 0 | 3880 → 3880 |
| sheets | **0 of 171** | 163 | 163 | 0 | 0 | 43 → 43 | 21033 → 21033 |

The two that moved:

| document | group | before | after |
|---|---|---|---|
| `Sample_SQMS_Program.docx` | `done-015` | `pages` 60/61 | **`match`**, 61/61 |
| `1447.doc` | `pagination-001` | `pages` 3/4 | **`match`**, 4/4, words 959/959 |

> **Correction, 2026-08-15, from `probes/words-metrics-01` §4: `words-after.tsv` is wrong on four
> rows and its 169 should be 171.** `TE.CAO.00125 … OJT Logbook.docx`,
> `xx_SETIS_PWS_template_10.19.22.docx`, `FO.FCTOA.00010 …docx` and `EHEST-SMS-…docx` are all
> recorded here with worse word counts than the tree at this commit actually produces; the first two
> are recorded as failing and in fact match. Two independent measurements agree against this file —
> `refdev-01`'s own baseline, and a 200-document render made with a binary built from this commit's
> behaviour. The cause is not established from here, but the shape fits a sweep that overlapped a
> rebuild. **Measure a baseline; do not inherit this one.**

**159 renderings changed and only six documents' error changed at all.** That is the shape a
correct sub-line refinement makes: every page moves a fraction of a twip and almost nothing crosses
a boundary.

| document | Δ page error | Δ word error |
|---|---:|---:|
| `done-015/docx/Sample_SQMS_Program.docx` | −1 | −1 |
| `pagination-001/doc/1447.doc` | −1 | 0 |
| `table-001/doc/150_5300_13_chg10.doc` | 0 | −3 |
| `done-015/docx/JEMIT_Template.docx` | 0 | −1 |
| `pagination-002/doc/150_5300_13_chg12.doc` | 0 | +1 |
| `pagination-002/docx/150-5370-10H.docx` | 0 | +9 |

The +5 net on words is those last two, on documents of 12 698 and 292 647 words. No verdict is near
its band because of it.

**Slides and sheets did not move a byte**, which is P12 and is the falsification test that mattered
most: the grid is scoped to Writer's reference device, and Calc's and Impress's are different
devices in a different map unit (§7). Had either track moved, the scoping would have been wrong.

## 5. The three `done-*` tracks

Verdicts taken from the same whole-track renders against the banked references, which is what
`dotnet/CLAUDE.md` asks for — `verdict.py` is `batch-check.sh`'s three checks column for column with
the reference read off disk instead of re-run.

| track | documents | before | after | renderings changed |
|---|---:|---:|---:|---:|
| `words/done-*` | 159 | 157 | **158** | 121 |
| `slides/done-*` | 144 | 144 | 144 | 0 |
| `sheets/done-*` | 156 | 156 | 156 | 0 |

**No `done-*` document lost its verdict, in any track.** The one remaining words mismatch is
`airbus-pdf-information-package_v1-4.docx` at 1272 words against 1299 — unchanged, and
`words-regress-01` §2 established it is a missing repeat of a header row worth about thirty words,
not a metric.

## 6. Tests

48 new tests in two files.

`tests/XRay.Office.Text.Tests/ReferenceGridTests.cs` — 45. Every expectation is a distance LibreOffice
itself drew: 30 line heights and 8 ascents across five faces from 5 to 24 pt, the
identical-2355-units refutation at the three sizes where the two faces part company, the four-way
refutation of §1's table, the grid's own arithmetic, and the two pins that keep the reference and
printer grids from collapsing into one. The design-unit metrics are stated rather than read from the
installed files, so the arithmetic is tested without the tests depending on a font being present.

`tests/XRay.Office.WordProcessing.Tests/ReferenceDeviceWiringTests.cs` — 3. Asserted through a read
package rather than a constructed paragraph, because a constructed one inherits the default and
would pass however the readers behave: a DOCX carries `MetricGrid.Reference`, one setting
`w:usePrinterMetrics` carries `MetricGrid.Printer`, and a 10 pt Liberation Serif paragraph measures
231 twips end to end.

One existing test changed: `HighlightTests.TheBandStraddlesTheBaselineByTheRunsOwnMetrics` computed
its expectation with an ungridded `LineSpacing.Resolve` and now uses the reference grid — a stale
expectation, not a behaviour change; the band is the line's own height and 13.799 pt became 13.80.

### Verified failing against the unfixed behaviour

Two separate reverts, so each half is proved on its own:

| reverted | result |
|---|---|
| the wiring only (`PageContent`, `DocxLayoutSource`, `DocReader` back to `null`) | `ReferenceDeviceWiringTests` **2 failed**, 1 passed; `ReferenceGridTests` 45 passed |
| `MidpointRounding.AwayFromZero` in `ToLength` only | `ReferenceGridTests` **12 failed**, 33 passed |

The one wiring test that still passes is deliberately the control — the `usePrinterMetrics`
document, which reached a grid before this round and still reaches the same one. `ReferenceGridTests`
passing under the first revert is by design: it names the grid explicitly and tests arithmetic, so
only the second revert can move it.

### Counts, every project run individually

| project | passed | failed | skipped | baseline |
|---|---:|---:|---:|---|
| Core | 337 | 0 | 0 | 337 |
| Containers | 109 | 0 | 0 | 109 |
| Text | **412** | 0 | 0 | 367 + 45 |
| Vector | 295 | 0 | 0 | 295 |
| Rendering | 150 | 0 | 1 | 150 |
| Markup | 259 | 0 | 0 | 259 |
| OpenDocument | 125 | 0 | 0 | 125 |
| WordProcessing | **883** | 0 | 0 | 880 + 3 |
| Spreadsheets | 847 | 0 | 0 | 847 |
| Presentations | 710 | 0 | 0 | 710 |
| **Fidelity** | **520** | **30** | **0** | **30 of 550** |
| total | 4647 | 30 | 1 | |

Fidelity is **30 failed of 550** — the briefed baseline, measured on the unmodified tree before
anything was changed and again at the end, the same 30. Build is 0 warnings, 0 errors. No flaky run
was seen this round; every count above is from a single pass and each project was run alone.

### The mtime trap, guarded rather than avoided

This round built the tree six times across four revert/restore cycles, which is exactly the shape
that contaminated `words-regress-01`. Two guards, both cheap:

- every restore is `cp` or `git checkout` **followed by `touch`**, and `rm -rf src/<project>/{obj,bin}`
  before each rebuild;
- after each restore, a subset of the corpus is re-rendered and compared **byte for byte** against
  the run being claimed — 10 of 10 on `words/done-001` after the first cycle, **69 of 69** on
  `words/done-01*` after the last. The "before" binary was checked the other way: `QuantisesAdvances`
  is absent from its `XRay.Office.Text.dll` and present in the final one.

## 7. Two things found and deliberately not done

### (a) The CJK 127% scale — measured, bounded, and riskier than it looks

The 39 IPAGothic misses in §2 are not the grid failing. They are `lcl_ApplyCjkHeightAdjustment`
(`sw/source/core/txtnode/fntcache.cxx`:272-293, tdf#129808): when `MS_WORD_COMP_GRID_METRICS` is set
and the font's `OS/2` declares coverage of CP932, CP936, CP949 or CP950, Writer multiplies both the
ascent and the line height by `127/100` — **integer division**, applied to the gridded value.

> Oddly, Word scales up ascent and line height for *fonts* that self-report coverage for certain
> specific CJK code pages, even when that font isn't used for CJK text.

`(gridded * 127) / 100` reproduces **all 39** IPAGothic ascents and **all 39** line heights exactly,
so the rule is not in doubt. What is in doubt is whether implementing it helps:

> **Superseded 2026-08-15 by `probes/words-metrics-01`, and the correction is instructive: this
> statement of the rule is wrong, and the 39-row fit could not have shown it.** The scale reaches the
> device's ascent and its ascent-plus-descent; the leading is added *afterwards*, unscaled —
> `GetFontHeight` is `lcl_ApplyCjkHeightAdjustment(m_nPrtHeight, …) + GetFontLeading(…)`. IPAGothic's
> `hhea` lineGap is **0**, so on that one face the two readings are the same number at every size.
> WenQuanYi Zen Hei's gap of 92/1024 separates them: at 12 pt this rule gives 412 twips, the correct
> one gives 406, and LibreOffice draws 406. Scored over 117 pairs on three faces, this rule is
> 78/117 and the corrected one 117/117. *A perfect fit on a sample that cannot discriminate is not
> evidence for the rule — only for the sample.*
>
> The reach estimate below was also wrong, and cheaply checkable: the three "at risk" documents were
> never at risk. Only one document of the 200 has East Asian letters adjacent to Latin ones.

- Across all 534 documents, **7** embed a CJK face in the reference rendering: 4 in words,
  1 in slides, 2 in sheets. Only the 4 words ones are in Writer's path at all.
- Of those 4, **three currently pass the gate** and one (`手机免提系统TSB.doc`, `metrics-001`,
  2 pages against 3) does not.

So the honest reach is *one document to gain and three to risk*, and it needs its own before/after
measurement on those four rather than being bolted onto this round — where it would have made the
159 renderings that moved impossible to attribute. `probe-grid.py` with `PROBE_SET=extra` reproduces
the whole 39-row table in one command.

### (b) Calc and Impress have their own reference devices

This grid is Writer's, and it is the twip because that is Writer's map mode. The other two are
different and neither is implemented:

| application | device | map unit | pixels per logical unit |
|---|---|---|---|
| Writer | `RefDevMode::MSO1`, 8640 dpi | twip | 6, exact |
| Calc | `RefDevMode::MSO1`, 8640 dpi (`sc/source/core/data/documen8.cxx`:191) | 1/100 mm | 3.4016, never exact |
| Impress / Draw | `RefDevMode::Dpi600`, 600 dpi (`sd/source/ui/app/sdmod.cxx`:85) | 1/100 mm | 0.2362, very coarse |

Impress's is the striking one: at 600 dpi in hundredths of a millimetre a device pixel is 2.4 twips,
so the quantisation there is worth up to **28 times** what it is worth in Writer. Both tracks are
byte-identical under this round's change, which is correct — but it also means both are still
measured with unquantised scaling, and if either has a systematic line-height residue this is where
to look first. `MetricGrid` would need a logical unit as well as a resolution.

## 8. Predictions, scored

Twelve right, one wrong. The wrong one is P5 and it is a detail of the implementation rather than of
the rule.

| | claim | conf | outcome |
|---|---|---:|---|
| P1 | 8640 dpi twip grid, per-component pixel rounding, 2 + 1 conversion | 85% | **right**, and from the source rather than a fit |
| P2 | reproduces 195 of 195 line heights | 80% | **right** — 195/195 |
| P3 | reproduces the ascents too | 70% | **right** — 195/195 |
| P4 | banker's rounding is a second defect worth at least one of the 195 | 90% | **right**, and much more than one: 30 of 195 |
| P5 | `MetricGrid` needs no structural change | 70% | **wrong** — the 2 + 1 grouping was already right, but advances had to be kept off the grid, which needed a new component |
| P6 | `Sample_SQMS_Program.docx` flips to 61 pages and `match` | 70% | **right** |
| P7 | more than 100 of the 200 words renderings change | 60% | **right** — 159 |
| P8 | the words `match` count does not fall below 163 | 55% | **right** — 167 → 169 |
| P9 | `1447.doc` improves its page count | 35% | **right** — 3 → 4, and it matches outright, 959 words against 959 |
| P10 | Fidelity no worse than 30 of 550 | 50% | **right** — exactly 30, twice |
| P11 | our `hhea`/`OS/2` precedence is not the defect | 85% | **right** — untouched, and exact on eight further faces including one that asks for typo metrics |
| P12 | slides and sheets byte-identical | 80% | **right** — 0 of 334 |
| P13 | Calc and Impress have their own, different grids | 75% | **right** — §7(b) |

## 9. Contradicting the brief

- **"we agree on 173 and differ on 22"** of 195. The instrument that produced it measured **81**
  pairs and called them 195 — see §2. The real figure for the tree as briefed is 149 of 195 on line
  height and 164 of 195 on ascent. Every miss is still exactly one twip and still in both
  directions, so the *characterisation* was right; only the count was.
- **"No device resolution from 72 to 6000 dpi with any per-component rounding fits."** True as
  stated and it is why the answer was missed: the resolution is 8640, and no per-component rounding
  fits at *any* resolution because the conversion back is grouped 2 + 1 rather than 3.
- **"`words/ceiling-001/doc/1447.doc`"** is `words/pagination-001/doc/1447.doc` since the regrouping.
  The brief's account of it is otherwise exactly right — "every line break on page 1 already matches
  word for word and 0.05 pt per line accumulates to one page by line 35" is one twip per line, and
  the document now matches on all four pages and all 959 words.
- **"Six documents in `words/metrics-001` are classified as metrics defects and some are likely
  this."** None of them was. All six still fail, none changed verdict, and only one changed its
  error at all. Whatever is wrong with `metrics-001` is not the line height.
- **"the `hhea` versus `OS/2` precedence rules themselves — check that assumption."** Checked and the
  assumption holds. The precedence in `LineSpacing.Resolve` is untouched by this round and is exact
  on all 13 faces measured, including Caladea, which sets `USE_TYPO_METRICS`, and IPAGothic, whose
  `hhea` and Windows metrics differ by 7.6% of the em.
- **"Reading the value out of LibreOffice's own flat ODF export … the single most promising untried
  instrument."** Not used, and it would not have helped: `--convert-to fodt` writes the *document*,
  not the layout, so a computed line height is not in it. The ODF path was used the other way round
  instead — the same probe converted to ODT and re-exported, to prove the reference device is not a
  writerfilter artefact (§2).

## Files

```
src/XRay.Office.Text/Fonts/LineSpacing.cs                  MetricGrid.Reference, the midpoint rule
src/XRay.Office.Text/Layout/MeasuredParagraph.cs           advances stay off the reference grid
src/XRay.Office.WordProcessing/Layout/PageContent.cs       the default every reader inherits
src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs   printer or reference, never neither
src/XRay.Office.WordProcessing/Ww8/DocReader.cs            the same
tests/XRay.Office.Text.Tests/ReferenceGridTests.cs         45 tests, all against LibreOffice's own PDFs
tests/XRay.Office.WordProcessing.Tests/ReferenceDeviceWiringTests.cs   3, through a read package
tests/XRay.Office.WordProcessing.Tests/HighlightTests.cs   one stale expectation
probes/lineheight-01/probe-grid.py                       the 195-pair table, and PROBE_SET=extra
probes/lineheight-01/render-track.py                     a whole track, one build, with a manifest
probes/lineheight-01/verdict.py                          batch-check.sh's checks, banked references
```
