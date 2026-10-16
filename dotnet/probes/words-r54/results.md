# words-r54 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r54`, base
`4aea606ac78`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. `prediction.md` beside this was committed
at `f8ca11486d0`, **before** the change at `bcf8958f0a2`.

## Baseline, reproduced

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 335  MISMATCH 20`. Scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **318 match, 19 open, 0 disagreements with the manifest's
status column, document for document.** Round 53's claim that it ended byte-equal to baseline
holds.

Both other tracks were swept at the same commit, for the cross-track census: **slides 199 of 302,
sheets 271 of 307**, each with 0 disagreements against the manifest.

## The rule, established before anything was changed

126 authored files through the installed `soffice`, five known-answer controls agreeing
(`Liberation Serif` → itself, `Calibri` → Carlito, `Cambria` → Caladea, `Arial` → Liberation Sans,
`Courier New` → Liberation Mono), the drawn face read out of each PDF with `pdffonts`.

**The question round 53 could not answer was never about the font resolver. It is about which
filter read the file.**

| filter | an unrecognised family, nothing declared | with a class declared |
|---|---|---|
| **DOCX** | **DejaVu Serif**, always | only `w:family="swiss"` moves it, to DejaVu Sans |
| **DOC** | **DejaVu Serif** | the `FFN`'s swiss code moves it the same way |
| **RTF** | **DejaVu Serif** | nothing moves it — `\fnil`, `\froman`, `\fswiss`, `\fmodern` are all Serif |
| **ODF text** | fontconfig's own generic | `style:font-family-generic` and `style:font-pitch` both honoured |
| **XLSX, PPTX, flat ODS** | fontconfig's own generic, `fc-match`'s column exactly | — |

### The experiment that separates "a fixed default" from "fontconfig's generic"

Round 53's ten families were all names `45-latin.conf` files under **no** generic, so its result
could not tell those two hypotheses apart. `/etc/fonts/conf.d/45-latin.conf` files 60 families
under one, and **none of them is installed here** (`fc-list` gives 22 families: Caladea, Carlito,
DejaVu ×3, Liberation ×3, OpenSymbol, IPA*, WenQuanYi*), so there are names whose fontconfig
generic is known and whose face is absent. That is the discriminator, and it is stated rather than
assumed — `fc-match` is printed beside every row of the probe.

| family | `45-latin.conf` files it | `fc-match` | DOCX draws | ODF draws |
|---|---|---|---|---|
| `Candara`, `Corbel`, `Century Gothic`, `Tahoma`, `Verdana`, `Trebuchet MS`, `Britannic`, `Luxi Sans` | sans-serif | DejaVu Sans | **DejaVu Serif** | DejaVu Sans |
| `Consolas`, `Andale Mono`, `Inconsolata`, `Fixedsys`, `Terminal`, `Luxi Mono` | monospace | DejaVu Sans **Mono** | **DejaVu Serif** | DejaVu Sans **Mono** |
| `Constantia`, `Elephant`, `Garamond`, `Georgia`, `MS Serif`, `Luxi Serif`, `Palatino Linotype` | serif | DejaVu Serif | DejaVu Serif | DejaVu Serif |
| `Impact`, `Cooper Std` | fantasy | DejaVu Sans | **DejaVu Serif** | — |
| `Comic Sans MS`, `Zapfino` | cursive | DejaVu Sans | **DejaVu Serif** | — |
| `Segoe UI`, `Cantarell` | system-ui | DejaVu Sans | **DejaVu Serif** | — |
| `Aptos`, `Roboto`, `Lato`, `Montserrat`, `Myriad Pro`, `Futura`, `Optima`, `Univers`, 4 nonsense names | nothing | DejaVu Sans | **DejaVu Serif** | DejaVu Sans |

**Through the DOCX filter, 38 of 38 answer DejaVu Serif and fontconfig's filing is irrelevant.
Through the ODF filter the same names track `fc-match` exactly, `Consolas` included.** Same
binary, same fontconfig, same machine, in one probe run.

### The answer does not depend on the request

One family held fixed (`Aptos`) and one thing varied at a time: **bold** → `DejaVuSerif-Bold`,
**italic** → DejaVu Serif, **bold-italic** → `DejaVuSerif-Bold`, **8 pt** → DejaVu Serif, **40 pt**
→ DejaVu Serif, **`w:hint="eastAsia"`** → DejaVu Serif (plus WenQuanYi for the CJK run). A second
point on a filed name (`Candara` bold) answers `DejaVuSerif-Bold` too. **CJK text alone** goes to
WenQuanYi, which is glyph fallback and a different mechanism. So the family is fixed and only the
face within it moves — **the rule is the simpler one, and this says so explicitly** because the
brief asked for it to be said either way.

### The declared class, on four families × eight declarations

`swiss` is the only code that moves the DOCX answer. `roman`, `modern`, `script`, `decorative`,
`auto`, and `w:pitch="fixed"` on its own all leave it at DejaVu Serif; `swiss` and `swiss`+`fixed`
both give DejaVu Sans. `Consolas` declared `swiss` gives DejaVu **Sans**, not Mono — the
declaration beats fontconfig's monospace filing. `Garamond` declared `swiss` gives DejaVu Sans,
which is the known-answer control: `DocxLayoutSource.Face` already recorded that exact case
measured on 26.2.4.2, and it reproduces.

### And what survives the default

Twenty-one further names through the DOCX filter, chosen because each has an installed chain entry
or a special rule: `Times`, `Helvetica`, `Albany`, `Thorndale`, `Courier`, `Nimbus Roman`,
`Nimbus Sans`, `CG Times`, `Times-Roman`, `MS Gothic`, `MS Mincho`, `Century Schoolbook`,
`Book Antiqua`, `Bookman Old Style`, `Lucida Console`, `Palatino Linotype`, `SimSun`, `Wingdings`
— **all DejaVu Serif**. The only three that escape are `Times New Roman` → Liberation Serif,
`Arial` → Liberation Sans and `Symbol` → OpenSymbol: the two strong metric aliases and the pi
face, which is exactly the pair of exemptions `SystemFontResolver.DeclaredGenericFor` already
carries.

## The change

`dotnet/src/XRay.Office.WordProcessing/Layout/WordFallbackClass.cs`, called from two sites:
`DocxLayoutSource.Face` and `LayoutFonts.Lookup`. **Nothing under `XRay.Office.Text`, `Core`,
`Containers`, `Vector`, `Rendering`, `Markup` or `Ooxml` changed behaviour**, so the diff cannot
reach slides or sheets at all.

```csharp
public static FontFamilyClass ForDeclared(string? familyName, FontFamilyClass declared)
    => string.IsNullOrWhiteSpace(familyName) ? FontFamilyClass.Unknown
        : declared == FontFamilyClass.SansSerif ? FontFamilyClass.SansSerif
        : FontFamilyClass.Serif;
```

`OdtLayoutSource`, `SlideText`, `SheetFonts`, `MetafileTextEngine` and `SvgTextEngine` are
untouched, because the measurement says their filters are already right.

### The guard on the family name is the whole of the first attempt's failure

**The change shipped without it first, and the sweep caught it: 335 → 315 raw, and 18 verdicts
lost, 17 of them `.doc`.** The reason is an ordering the site itself documents: a declared class is
consulted in the **pre-match** step, *before* `GenericFallbacks` gets to separate "no font named"
from "a font nobody has". `GenericFallbacks` sends a blank family to `DefaultFallbacks` —
Liberation Serif — and handing it a declared class instead bypasses that rule entirely.

`.doc` is where it showed because the WW8 reader routinely produces a run with no family at all,
while a DOCX run inherits one from its style. The font-list census of that first attempt names the
damage precisely: **29 `.doc` documents moved from Liberation Serif to DejaVu Serif**, a
disagreement shape that did not exist at baseline.

That is a measurement doing its job, and it is worth recording as such: the failure was not in the
rule, which every probe still supports, but in the *scope* of the rule — and no authored probe
would have found it, because no authored probe had a run with no family in it. **The corpus sweep
was the only instrument that could see this**, which is the standing argument for sweeping the
whole track rather than the batches a change aims at.

## Refutations

### 1. The seat named by the audit is not the seat, and the recommended change would have broken two tracks

`TODO.24-2-7-audit.md` called `SystemFontResolver.GenericFallbacks` *"the largest single finding on
the list"*, said the fix was **one line in `XRay.Office.Text`**, and held it back only because such a
line owes a three-track sweep. Every measurement round 53 reported reproduces. **The verdict does
not.** `GenericFallbacks`'s `_ => SansFallbacks` is *correct* for every caller that reaches it
without a declared class, and those callers are ODF text, XLSX, PPTX and flat ODS.

Two independent instruments say so:

- authored PPTX, XLSX and flat ODS files naming `Aptos`, `Candara` and `Consolas` answer **DejaVu
  Sans, DejaVu Sans and DejaVu Sans Mono** — `fc-match`'s own column — where the same three names
  in a DOCX answer DejaVu Serif;
- over **302 slides and 307 sheets** renderings compared against the reference's own embedded font
  lists, **zero** documents show `ours = DejaVuSans, ref = DejaVuSerif`. Not one, on either track.

And the population that a `XRay.Office.Text` change would have reflowed is now counted rather than
imagined: **202 slides and 130 sheets renderings draw a DejaVu Sans face on our side**, and every
one of them currently agrees with the reference. Names are in `slides-at-risk.txt` and
`sheets-at-risk.txt` beside this file.

**The audit's diagnosis was right and its seat was wrong**, and the variable that separates them is
one round 53 held fixed without noticing it was a variable: the *format*. Ten families, all
`.docx`. That is a new shape for §7's list — not "a description matching a brief", but *a probe
whose every case shared a hidden constant*.

### 2. "73 of 337 renderings carry DejaVu Sans on our side" does not reproduce; 86 does, exactly

Re-derived rather than quoted, per the standing rule:

| reading | count |
|---|---:|
| renderings whose embedded font list disagrees with the reference's | **86** — reproduces exactly |
| …of those, our **full** list contains a DejaVu Sans face | **70** |
| …of those, the **difference** contains a DejaVu Sans face | **40** |
| …of those, the disagreement is *exactly* ours DejaVu Sans / reference DejaVu Serif | **32** |

**No reading gives 73.** The figure the audit and the merge note both carry is off by 3 against the
broadest reading and by more than 2× against the one the sentence actually describes. The number
that matters — the documents this change targets — is **32**, and it is a list, not a total.

### 3. My own `.doc` probe was confounded, and the corpus refuted it within one sweep

Binary `.doc` cannot be authored here, so the DOC filter was probed by converting the authored
DOCX pair to Word 97 with `soffice` and back to PDF. It answered DejaVu Serif undeclared and DejaVu
Sans for the swiss case — apparently confirming that DOC behaves exactly like DOCX.

**It cannot have measured that.** LibreOffice's own MS-Word-97 export writes the family code it
holds in memory, and its DOCX *import* had already applied the roman default — so the `.doc` I
converted declared `ff=roman`, and the probe measured "declared roman → Serif", which was never in
doubt. A round trip through the tool under test is not an independent fixture.

The corpus said so immediately: 29 `.doc` documents whose reference draws Liberation Serif. Whether
a genuinely undeclared `.doc` family answers Serif or reaches the chain is **still open**, and the
honest state of the DOC arm is that it now takes the roman default only for a *named* family, which
the corpus sweep supports and no authored probe does.

### 4. `FontRequest.DeclaredClass`'s own remarks ran two filters together

The comment said, as one measured sentence, *"`Garamond` declared `swiss` falls back to DejaVu Sans
where the same name undeclared falls back to DejaVu Serif, and `Futura` declared `roman` falls back
to DejaVu Serif where undeclared it falls back to DejaVu Sans."* Both halves are true and they are
**measurements through different filters** — `Futura` undeclared is DejaVu Sans through ODF and
DejaVu Serif through DOCX. Corrected at the site.

`ShapeOf`'s remarks carried the same conflation — *"the binary answers fontconfig's way on every
one"* of 296 families — which is a claim about the caller as much as about the name. Also corrected.

## Prediction against measurement

| | predicted | measured |
|---|---|---|
| renderings changed | 32–60 of 337 | **45 of 337** |
| documents whose font list stops disagreeing | the 32 named | **24 of them** |
| verdict movement | **0**, downside risk −1 to −3 | **−1** (`24-25_FAA_Holdover_Tables`, two rows for the one document) |
| gains | 0, and "+1 or more would mean the census under-reached" | **0** |
| cross-track verdicts | 0 slides, 0 sheets | **0 and 0** — no file they compile against changed |

**Words 318 → 317 of 337.** The prediction's shape was right and its downside fired at the top of
the stated range.

### Font-list agreement, which is what the change is actually for

| | baseline | after |
|---|---:|---:|
| renderings whose embedded font list disagrees with the reference | 86 | **66** |
| `ours = DejaVu Sans, ref = DejaVu Serif` | **32** | **8** |
| `ours = DejaVu Serif, ref = DejaVu Sans` — the new wrong direction | **0** | **6** |

**Twenty-four documents now draw the face the reference draws and six do not**, where before it was
nought and nought. Every one of those is a line-breaking difference as well as a visible one,
because the two faces have different advances — which is why 45 renderings moved for 32 targets.

### The regression, named and diagnosed

`24-25_FAA_Holdover_Tables.docx` (and its upper-case alias row, and its sibling
`FAA 2025-26 Holdover Tables.docx`, which does not carry a verdict of its own): **155/155 pages →
165/155**. It fails on page count, not on fonts.

Isolated to one family by cutting the document: replacing `Arial Bold` with `Arial` throughout
removes `DejaVuSerif` and `DejaVuSerif-Bold` from our output entirely, and removes
`DejaVuSans-Bold` from the **reference's** — so `Arial Bold` is what both sides fall back for, and
**the reference falls back to DejaVu Sans where we now fall back to DejaVu Serif.**

Five one-variable edits to the real package say the font table is not what decides it. Deleting
the `Arial Bold` entry outright, deleting its `w:family="auto"`, changing that to `roman`, to
`swiss`, deleting its `w:altName="Times New Roman"`, and deleting its `w:panose1` **all leave the
reference at DejaVu Sans and 155 pages**. Replacing the name with `Zzqqxx Nonesuch` or with
`Aptos` likewise: **in this document, any unrecognised name answers DejaVu Sans.**

And it is not the style chain either. An authored package whose paragraph style names `Arial`
(declared swiss) and whose run overrides with `Aptos` answers **DejaVu Serif**, and so does the
same package built on `Tahoma`/swiss, `Times New Roman`/roman, `Cambria`/roman and an undeclared
style family. Inheritance of the class through the style was the obvious hypothesis and it is
refuted.

**So there is a variable in a real document that flips this filter's default from roman to swiss,
and eight probes have not found it.** The eight documents still showing the *old* direction
(`ESPN-R - MCF - RA - Ed1`, `technical report template`, `Writing a technical report (SCE subject
guide)`, `How-to-Write-an-Architecture-Document…`, `Lessons-Learned-Bulletin-Dorset…`,
`FO.FCTOA_.000129…`, `Company-profile-2022-EN`, `SDL_FSDO_Part91_LOA_Checklist`) are the same
puzzle seen from the other side: `ESPN-R` declares `Verdana` and `Segoe UI` **swiss** and the
reference sets them in DejaVu **Serif**, where our declared-swiss rule keeps them in Sans.

This is stated rather than smoothed over, because it is the next round's whole job.

## Cross-track, measured rather than argued

The diff touches only `XRay.Office.WordProcessing`, so slides and sheets cannot be reached: no file
they compile against changed. That is checkable with `git diff --name-only` and needs no sweep.

**What did need measuring is the change the brief expected**, and it was measured twice:

1. **Authored files through each filter.** `Aptos`, `Candara` and `Consolas` in a PPTX, an XLSX and
   a flat ODS answer **DejaVu Sans, DejaVu Sans and DejaVu Sans Mono** — `fc-match`'s own column.
   Not one of them answers Serif.
2. **The whole corpus of both tracks**, swept at this branch's base and compared against the
   reference's own embedded font lists: **slides 199/302 and sheets 271/307**, both reproducing
   `MANIFEST.tsv` document for document, and **zero renderings on either track show
   `ours = DejaVuSans, ref = DejaVuSerif`.**

The population a `XRay.Office.Text` change would have moved is counted, not guessed: **202 slides and
130 sheets renderings draw a DejaVu Sans face on our side**, every one of them currently agreeing
with the reference. They are named in `slides-at-risk.txt` and `sheets-at-risk.txt`.

## The 24.2.7.2 audit

Both remaining `SystemFontResolver` sites are now recorded **at the site**, in both directions:

| site | was | now |
|---|---|---|
| `GenericFallbacks` | **WRONG** r53, not fixed | **VERIFIED** r54 — correct for every filter that reaches it undeclared; the DOCX answer is the reader's roman default |
| `DefaultFallbacks` | **UNDECIDED** r53 | **VERIFIED** r54 — Liberation Serif, by two fixtures that reach `DefaultFonts` |

`git grep -c '24\.2\.7-audit'` → 5 marker lines across 3 files. Two further comments were corrected
for conflating filters: `ShapeOf`'s "the binary answers fontconfig's way on every one" and
`FontRequest.DeclaredClass`'s `Futura` sentence.

## Tests

`XRay.Office.WordProcessing.Tests/WordFallbackClassTests.cs`, **27 tests**, all passing.

Run through `verify-test.sh`, tree clean before each and restored after:

| mutation | detected by |
|---|---|
| a declared `swiss` no longer escapes the default | `AnUnrecognisedFamilyInADocxFallsBackTheWayTheReferenceDoes` (2 cases), `OnlyADeclaredSansSerifEscapesTheRomanDefault` |
| no roman default at all — every named family stays `Unknown` | 11 cases across those two plus `TheDocAndRtfArmTakesTheSameDefault` |
| **the empty-family guard removed** — the exact defect the first sweep caught | `ARunNamingNoFamilyAtAllKeepsTheApplicationDefault` (all 3 cases) |

**Three of the six test methods are detectors and three are deliberate controls**, labelled as
such rather than counted as detectors: `AStrongMetricAliasStillBeatsTheDefault` and
`AnInstalledFamilyIsNotSubstitutedAtAll` say the default is a *fallback* and not a blanket, and
`ARequestThatDeclaresNothingStillGetsFontconfigsGeneric` is the reach control — it is the test that
would have caught the tempting version of this fix, and no mutation of the word-processing reader
can make it fail, which is exactly the point of it.

```
Core 337  Containers 109  Text 611  Vector 295  Rendering 150(1 skipped)  Markup 259
OpenDocument 125  WordProcessing 1123  Spreadsheets 905  Presentations 780   = 4694
0 failed, 1 skipped
```

**4667 → 4694, delta +27**, all in `WordProcessing`. `dotnet build -v q -nologo`: 0 warnings,
0 errors.

## Left open, in the order the next round should take it

1. **The variable that flips the DOCX filter's default from roman to swiss.** Fourteen corpus
   documents disagree with the reference on it, eight one way and six the other, and it is now the
   only thing standing between this change and a clean result — including the one verdict this
   round lost. Eight one-variable edits to `24-25_FAA_Holdover_Tables.docx` have ruled out the font
   table entry, its `w:family`, its `w:altName`, its `w:panose1`, the family name itself, and
   inheritance of the class through the paragraph style. What is left to try: `settings.xml`'s
   compatibility mode, the theme, `w:rPrDefault`, and the possibility that it is a property of the
   *first* font the document resolves rather than of the run.
2. **`w:altName` is read and never used.** `WordFontTable` parses it and `DocxLayoutSource` ignores
   it; LibreOffice's `FontTable.cxx` does not. It is not the cause of item 1 — that was tested —
   but it is a real gap with a known reference behaviour.
3. **`097` is still a 1.7 pt boundary case** on the standing line-height deficit (our empty
   paragraph 11.50 pt against 12.65), worth 1.15 pt on every empty paragraph in the corpus. Round
   53's first open item, untouched here.
4. `#_x0000_t15` VML (3 shapes, `090`), then `DrawingStyleMatrix` into `DocxFrames` (458 shapes,
   40 documents, arrow ends being 5 shapes).
5. `068`: 24 missing connector strokes and 6 surplus box outlines, fills exact.

## Files

- `prediction.md` — committed before the change, at `f8ca11486d0`.
- `font-fallback-rule.py` — 98 authored files, five known-answer controls, `fc-match` printed
  beside every row so the discriminator is stated rather than assumed.
- `cross-format-fallback.py` — the same question through RTF, XLSX, PPTX and flat ODS.
- `font-list-census.py` — embedded font list, ours against the reference, over a sweep directory.
- `cross-track-census.py` — the slides and sheets census, with its groups and its blind spots.
- `slides-at-risk.txt`, `sheets-at-risk.txt` — the 202 and 130 named.
