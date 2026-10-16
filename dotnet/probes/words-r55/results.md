# words-r55 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r55`, base
`1c7249ff8e9`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Two predictions, each committed before the
change it covers: `prediction.md` at `7376f9cdd84` before `f21285cf56d`, and
`prediction-doc-arm.md` at `8d8600e556a` before `a7337199ad1`.

## Baseline, reproduced

`batch-check.sh … 'words/*' … 8` → `TOTAL 355 MATCH 333 MISMATCH 22`. Scored against
`MANIFEST.tsv`'s own 337-path list rather than that total — the extra 18 rows are the
case-insensitive mount's alias entries — **317 match, 20 open, zero disagreements with the
manifest's status column, document for document.** The brief's figure exactly.

## The rule, and the two probes that establish it

**The DOCX family class is an inherited property, and the family name is not a property of it.**

`family-inheritance.py`, 28 authored packages of **one paragraph and one run each**, so the PDF's
font list has exactly one entry that can move. Three controls agree (`Arial` → Liberation Sans,
`Calibri` → Carlito, `Liberation Serif` → itself) and four restatements of round 54's rule agree.

| the class is donated by | the run's family is filed | 26.2.4.2 draws |
|---|---|---|
| `docDefaults` `Arial`(swiss) | `auto` | **DejaVu Sans** |
| `Normal` `Arial`(swiss), consumed by a style `basedOn` it | `auto` | **DejaVu Sans** |
| the same through **two** style levels | `auto` | **DejaVu Sans** |
| `Normal` `Arial`(swiss), consumed by **direct run formatting** | `auto` | **DejaVu Sans** |
| `docDefaults` `Times New Roman`(roman) | `auto` | DejaVu Serif |
| `docDefaults` `Arial`(swiss) | **its own `roman` entry** | DejaVu Serif |
| `docDefaults` `Times New Roman`(roman) | its own `swiss` entry | DejaVu Sans |
| `Normal` `Arial`(swiss), consumed through **`w:asciiTheme`** | — | **DejaVu Sans** |
| `Normal` `Times New Roman`(roman), through `w:asciiTheme` | **`swiss` in the table** | DejaVu **Serif** |
| nothing anywhere | `auto`, `modern`, absent, `pitch="fixed"` only | DejaVu Serif |
| `docDefaults` `Arial`(swiss) | `modern` / absent from the table / `pitch` only | **DejaVu Sans** |
| `docDefaults` `Arial`(swiss), run states only `w:hAnsi` | — | Liberation Sans — the *ancestor's* name |

So: the class is set **only** where `w:rFonts/@w:ascii` names a font `word/fontTable.xml` files
under `roman` or `swiss`. `auto`, `modern`, `script`, `decorative`, a pitch-only entry, an absent
entry and `w:asciiTheme` all leave whatever an ancestor put there; nothing anywhere stating one
leaves it roman; and only the `ascii` slot states one at all.

That is `DomainMapper::lcl_attribute` (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:436):
`LN_CT_Fonts_ascii` inserts `PROP_CHAR_FONT_NAME` unconditionally and `PROP_CHAR_FONT_FAMILY` only
when `FontTable::getFontEntryByName` answers something other than `DONTKNOW` — and
`FontTable::lcl_sprm` maps **only** `roman` and `swiss`, dropping `auto`, `modern`, `script`,
`decorative` and `w:pitch` on the floor. `LN_CT_Fonts_asciiTheme` inserts the name and never the
family; `LN_CT_Fonts_hAnsi` is `break; //unsupported`. The source supplied the hypothesis; the
probe decided it, because the tree here is 27.2.0.0.alpha0+ and the binary is 26.2.4.2.

### `24-25_FAA_Holdover_Tables.docx` is this shape exactly, and round 54's refutation was confounded

Its `Normal` names `Arial`, which its own table files `swiss`. `Heading2`, `Heading3` and `Caption`
are `basedOn Normal` and name `Arial Bold`, which it files `auto`. That is one style level and the
class survives it.

**Round 54 recorded "inheritance of the class through the paragraph style" refuted.** Its
counter-measurement was read off the **whole document's** embedded font list — and that document
draws DejaVu Sans for four other reasons: `Century Gothic`, `Tahoma`, `Charlotte Sans Book` and
`CWFZGM+Myriad-BoldItalic` are all declared `swiss` in the same table. The observable could not
move whatever the edit did, which is why eight one-variable edits all came back "still DejaVu Sans".
Every fixture here is one run, for that reason.

Its authored probe also put the unfiled name on a *run* rather than on a style and read DejaVu
Serif. That does not reproduce: `style-swiss/run-auto` answers **DejaVu Sans** on 26.2.4.2.

## The DOC arm, which round 54 left explicitly unmeasured

`doc-family-code.py`, nine fixtures. Round 54 refuted its own DOC probe in the same round — a
DOCX→DOC round trip bakes in the roman default, so it measured "declared roman". **A flat ODF file
defeats that**: the ODF filter has no roman default, a `style:font-face` with no
`style:font-family-generic` leaves the family at `FAMILY_DONTKNOW`, and `wwFont::Write`
(`wrtw8sty.cxx`:821) maps that onto `ff = 0`.

| fixture | draws | | fixture | draws |
|---|---|---|---|---|
| `Zqxwv Nonesuch`, no generic | **DejaVu Sans** | | `Aptos`, no generic | **DejaVu Sans** |
| …`roman` | DejaVu Serif | | `Garamond`, no generic | **DejaVu Serif** |
| …`swiss` | DejaVu Sans | | `Univers`, no generic | DejaVu Sans |
| …`modern` | DejaVu Sans | | `Helvetica`, no generic | DejaVu Sans |
| …`decorative` | DejaVu Sans | | | |

**Through the DOC filter only `ff = roman` gives Serif; every other code, and no code at all, gives
fontconfig's own generic.** `Garamond` is the control that says this really reaches
`SwWW8ImplReader::GetFontParams` (`ww8par6.cxx`:3767) rather than measuring the export: that
function carries a **fourteen-prefix name-override list with no counterpart in the DOCX filter**,
and `Garamond` comes back Serif where the otherwise identical `Aptos` comes back Sans.

### One mechanism, three filters — this simplifies the model rather than adding to it

In all three, `FAMILY_DONTKNOW` reaches fontconfig's own generic and `FAMILY_ROMAN` appends
`"serif"`. The **DOCX** filter never *sets* `DONTKNOW` — it leaves the inherited value, whose floor
is Writer's roman pool default. The **RTF** filter never sets the family at all, which is exactly
why `\fnil`, `\froman`, `\fswiss` and `\fmodern` are inert. The **DOC** filter sets it explicitly
per font, `ff = 0` included, and is the only one of the three that can reach `DONTKNOW`. Round 54's
three separate rules are one rule and a difference in who writes to the item.

## The changes

Both are confined to `XRay.Office.WordProcessing`. **No shared layer changes behaviour**: the only
edit outside it is comment-only in `XRay.Office.Text/Fonts/SystemFontResolver.cs`, verified by
`git diff` showing no non-`///` lines. Slides and sheets cannot be reached and no cross-track sweep
is owed.

1. `WordParagraphFormats.StatedClass` resolves the class from the layer stack
   `WordStyles.RunPropertyLayers` already builds; `WordTextStyle` carries it; `DocxLayoutSource.Face`
   reads it instead of asking the table about the run's own name. **The face cache key gains the
   class** — one family named under a `swiss` ancestor and under a `roman` one now resolves to two
   faces, and leaving it out would be a collision, not an omission.
2. `LayoutFonts` stops putting a DOC family through the roman default: when a font table is supplied
   — which only `DocReader` does — the `FFN`'s own code is the whole answer, `Unknown` included.
   `Ww8FontTable.ShapeOf` gains the fourteen-prefix override list. RTF, which supplies no table,
   keeps the roman default.

The empty-family guard round 54 lost 18 verdicts to is kept in both arms and is under test.

## Prediction against measurement

### The DOCX change

| | predicted | measured |
|---|---|---|
| renderings whose bytes change | 16–24 | **17** |
| documents that stop disagreeing on fonts | 9–12 of 14 | **13**, plus 2 improved in shape |
| font-list disagreements (66 at baseline) | 54–60 | **53** |
| `ours=DejaVuSans, ref=DejaVuSerif` (8) | 1–2 | **2** |
| `ours=DejaVuSerif, ref=DejaVuSans` (6) | 3 | **3** |
| **verdict movement** | **+1** | **+1** |
| downside risk | −1 to −3 | **0** |
| cross-track | 0 and 0 | no file they compile against changed |

**Words 317 → 318 of 337**, one gain and **zero regressions**. `24-25_FAA_Holdover_Tables.docx`
**165/155 → 155/155 pages, with an embedded font list identical to the reference's.**

**The census under-reached, and it said in advance that it would.** It named 16 documents and 15 of
them moved; 2 more moved that it could not see — `012_Project_Timeline_Template…` and `33004`, both
of which *stopped* disagreeing. The prediction file names table styles and numbering levels as the
layers it does not model, and that is where they are. The named risk — the four documents where our
`ascii`→`hAnsi`/`cs`/`eastAsia` slot fallback invents a family LibreOffice never draws — cost
nothing on the gate; one of them, `AFS-050-004-F2_0i`, is slightly worse on fonts and is discussed
below.

### The DOC change

| | predicted | measured |
|---|---|---|
| renderings whose bytes change | 2–15, all `.doc` | **3** |
| font-list disagreements (53) | 50–53 | **52** |
| `ours=DejaVuSerif, ref=DejaVuSans` (3) | **1** | **1** |
| **verdict movement** | **0** | **0** |
| downside risk | −1 to −4, within the 13 correct `.doc` | **0** — none of the 13 moved |

`congregationalhistories_ky_2023.doc` stops disagreeing outright; `手机免提系统TSB.doc` loses its
DejaVu disagreement entirely and is left with a Liberation-bold difference that predates this round;
`150_5300_13_chg10.doc` changed bytes and not its font list. **Zero documents newly disagree.**

### Font-list agreement over the whole round

| | baseline | after DOCX | after DOC |
|---|---:|---:|---:|
| renderings disagreeing with the reference's embedded font list | **66** | 53 | **52** |
| `ours = DejaVu Sans, ref = DejaVu Serif` | 8 | 2 | **2** |
| `ours = DejaVu Serif, ref = DejaVu Sans` | 6 | 3 | **1** |
| documents newly disagreeing | — | **0** | **0** |

Round 54 took this from 86 to 66 at a cost of one verdict. This round takes it to **52** and gives
the verdict back. **Fourteen of the sixteen documents in the two wrong directions are now right.**

## What is left, and it is named rather than rounded off

Two documents still disagree the old way and one the new way, and **none of the three is explained
by this rule**:

- `FO.FCTOA_.000129 …` — ours DejaVu Sans, reference DejaVu Serif. Every ordinary run in it names
  its font through `w:asciiTheme="minorHAnsi"` → `Calibri` → Carlito, so no fallback is involved
  there at all. A blind reader found what is actually wrong with the page, below.
- `AFS-050-004-F2_0i` — the one document whose font row got *worse*: `DejaVuSans-Bold` on our side
  against `DejaVuSerif-Bold` on the reference's, from `Helvetica-Narrow`, which is reached only
  because our `Family()` falls through the `ascii` slot to `hAnsi`/`cs`/`eastAsia`. LibreOffice's
  `DomainMapper` treats `w:hAnsi` as **unsupported** and never takes a western family from `w:cs` or
  `w:eastAsia` — measured here, case `dd-swiss/run-hAnsi-only`, which draws the *ancestor's* family.
  **That slot fallback is a real and separately-measured divergence** and is the next font item.
- `template---tpr-technical-progress-report-with-guidance` — a `.docx` whose font table declares
  **no `w:family` on any entry at all**, so nothing can state a class and everything takes the roman
  default. The reference draws a DejaVu Sans we do not, and the only unresolvable name in it is
  `Noto Sans Symbols`. Unexplained.

## What the vision round found, which is not what it was pointed at

Three pages, each chosen for a stated reason rather than by `--worst`, each handed to a fresh
reviewer forbidden from reading anything else, each asked to describe both halves separately before
comparing. Two of the three found defects **larger than the one this round was about**:

1. **`24-25_FAA_Holdover_Tables` page 3** (chosen because it is the document the round is for): the
   reviewer reports sans-serif everywhere on **both** halves, identical line breaking with nothing
   wrapping on either side, identical vertical rhythm, identical rules and furniture. The only
   differences named are a 2–3 % wider setting on the reference and correspondingly shorter dot
   leaders — which is the standing advance divergence in `CLAUDE.md`, not this defect. **The fix is
   confirmed on the page and not only on the gate.**

2. **`AFS-050-004-F2_0i` page 2 — five black banner rows whose white reversed-out text we do not
   draw at all.** The reference sets `0.000 General Information…`, `CE-1 Primary Aviation
   Legislation`, `CE-2 …`, `CE-3 …`, `CE-4 …` in white serif on black; ours draws five solid black
   bands with nothing in them, so those rows are shorter and one extra table row fits on the page.
   The reviewer also reports a page-border rectangle the reference draws and we do not, and a
   header row we set in blue and the reference sets in black. This is a much bigger defect than the
   font row that drew me to the document.

3. **`FO.FCTOA_.000129` page 3 — every checkbox is missing.** The reference draws a small empty
   square before each option; we draw nothing, leaving the space blank. **These are legacy
   `FORMCHECKBOX` form fields**, not `w:sym` and not literal characters: `<w:checkBox>` inside
   `w:ffData`. Census over the words corpus: **675 of them in 12 documents** —
   `FO.FCTOA.00010` 249, `Form-SM-76A-…` 152, `FO.FCTOA_.000129` 60, `A1. EASA Form 2` 52,
   `te.iors.00048-002` 48, `SPA-06_mcar_part-6` 41, and six more. Two of those documents are in the
   *current* font-list disagreement list with a reference-only face we never load, which is
   consistent with the reference drawing the box out of a face our layout never asks for.
   The same reviewer reports our first-column numbering running **3.1, 3.3, 3.4, 3.6, 3.12** where
   the reference runs **3.1 … 3.5** — we advance a list counter for paragraphs the reference does
   not. **`w:numId w:val="0"` is not the cause**: `DocxLayoutSource.Lists` line 176 already returns
   null for it, checked at the site, so the next round should not spend a probe on that hypothesis.

## Refutations

1. **Round 54's "inheritance of the class through the paragraph style is refuted" is itself
   refuted**, by two independent measurements: an authored package whose `Normal` names a `swiss`
   family and whose derived style names an `auto` one draws **DejaVu Sans**, and the corpus document
   the claim was made on now renders 155 pages with the reference's own font list. The original
   observable was over-determined — four other families in the same table are declared `swiss`.
2. **Round 54's "DOCX, DOC and RTF all take the roman default" is wrong for DOC.** Nine flat-ODF
   fixtures exported to Word 97 and back say only `ff = roman` draws Serif. Round 54 had already
   flagged its own DOC probe as confounded; this is the measurement that replaces it.
3. **`Ww8FontTable.ShapeOf`'s "the other family codes leave LibreOffice's answer unchanged" is
   measurably false**, and it came from the same confounded round trip. Corrected at the site.
4. **`SystemFontResolver.GenericFallbacks`'s newly-written marker carried an over-general sentence.**
   Round 54 verified the site from four filters and wrote that the word-processing filters never
   reach it undeclared. The DOC filter does. The verdict was right and the reason was too broad —
   **re-verifying an already-`VERIFIED` site found a real correction**, which is the argument for
   re-marking with a date and round rather than treating `VERIFIED` as terminal.

## The 24.2.7.2 audit

| site | outcome |
|---|---|
| `XRay.Office.Text/Fonts/SystemFontResolver.cs` `GenericFallbacks` | **VERIFIED again, round 55, from a fifth caller** — the DOC filter reaches it undeclared and it answers correctly there too, `Consolas` → DejaVu Sans **Mono** included. The sentence saying no word-processing filter reaches it is corrected at the site |

Marker lines: **11 → 12**; open `24.2.7` hits unchanged at 42. No new site was opened.

## Tests

```
Core 337   Containers 109   Text 611   Vector 295   Rendering 150(1 skipped)   Markup 259
OpenDocument 125   WordProcessing 1155   Spreadsheets 925   Presentations 788      = 4754
0 failed, 1 skipped
```

**4722 → 4754, delta +32**, all in `WordProcessing`: 23 in a new
`WordInheritedFamilyClassTests.cs` and 9 added to `WordFallbackClassTests.cs`.
`dotnet build -v q -nologo`: **0 warnings, 0 errors**.

Run through `verify-test.sh`, tree clean before each and restored after — **four mutations, four
detected**:

| mutation | detected by |
|---|---|
| the class stops inheriting past the innermost layer (`layers.Take(1)`) | 10 of the 23 new tests, across all five inheritance methods |
| the DOC arm takes the roman default again (`ForWw8Font` → `ForDeclared`) | `TheRtfArmTakesTheRomanDefaultAndTheDocArmDoesNot` |
| the WW8 name-override list is removed | `TheWw8ReaderOverridesTheFfnCodeByName`, 7 of 9 cases |
| the empty-family guard is removed from the WW8 arm — round 54's 18-verdict defect | `TheRtfArmTakesTheRomanDefaultAndTheDocArmDoesNot` |

**Three of the new tests are deliberate controls and are labelled as such rather than counted as
detectors**: `NothingStatingAClassIsStillTheRomanDefault` (round 54's whole result, so a change to
the inheritance cannot quietly take the default with it), `AnInstalledFamilyIgnoresTheInheritedClass`,
and `NoFontTableStatesNothing`.

## What the next round does first

1. **The `ascii` slot fallback.** Our `Family()` falls through `ascii` → `hAnsi` → `cs` →
   `eastAsia`; LibreOffice's `DomainMapper` treats `w:hAnsi` as unsupported and never takes a western
   family from `w:cs` or `w:eastAsia` — measured here, `dd-swiss/run-hAnsi-only` draws the
   *ancestor's* family. Four corpus documents reach a family through that fallback
   (`AWR OPS-AOC 044`, `form_1123_application_form_rvsm_spa`, `AFS-050-004-F2_0i`,
   `AW-104D-RVSM-…`), and it is the one thing standing between this thread and the last two
   font-list rows.
2. **Legacy `FORMCHECKBOX` form fields are not drawn: 675 of them in 12 documents**, found by a
   blind reader on a page chosen for a different reason. `<w:checkBox>` inside `w:ffData`; the
   reference draws an empty square, we draw nothing.
3. **`AFS-050-004-F2_0i`'s five banner rows**: white reversed-out text on black we do not draw at
   all, which shortens the rows and lets an extra row onto the page.
4. **`097`'s 1.7 pt boundary case** — the standing line-height deficit, our empty paragraph 11.50 pt
   against 12.65, worth 1.15 pt on every empty paragraph in the corpus. Round 53's first open item,
   untouched for three rounds.
5. `w:altName` is still parsed and never used — and note that this checkout's
   `writerfilter/dmapper/FontTable.cxx` ignores it too (`case NS_ooxml::LN_CT_Font_altName: break;`),
   so the round-54 note that "LibreOffice's `FontTable.cxx` does not" needs re-checking against the
   binary before anyone acts on it.
6. `#_x0000_t15` VML (3 shapes, `090`), then `DrawingStyleMatrix` into `DocxFrames` (458 shapes,
   40 documents).

## Files

- `prediction.md`, `prediction-doc-arm.md` — each committed before the change it covers.
- `family-inheritance.py` — 28 authored one-run packages, three controls, four restatements of
  round 54's rule.
- `doc-family-code.py` — nine flat-ODF fixtures exported to Word 97 and back, with the
  name-override list as its control.
- `class-inheritance-census.py` — the corpus census, with its blind spots written into the docstring.
