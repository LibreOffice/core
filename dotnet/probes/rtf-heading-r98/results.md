# `rtf-heading-r98` — O30: the heading pool is an intermediate, not a constant, and the reach is one

Measured 2026-09-11 in `/home/user/wt-rtfhead` (branch `agent/rtfhead`, base `ac7745deb`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`**. Nothing below is read off a page by eye; there is no
`Task`/subagent tool in this container to delegate a blind reading to, and none of the arms here
needed one.

**The brief is right and the seat closes as *fixed in this tree*.** `RtfPoolParent.Heading` folded
`HeadingPool` in as a constant and stopped the walk; `COLL_HEADLINE_BASE`'s own pool parent *is*
`COLL_STANDARD`, the reference inherits seven character and paragraph properties from the
document's own `Normal` into `heading 1`…`heading 9`, `Title` and `Subtitle`, and this tree now
does too.

**And the reach is one document of 338, against a candidate set of twelve and a prediction of
nine.** That gap is the useful half of the round, it is explained document by document in §5, and
the prediction was written down and committed to the probe directory **before** either leg of the
sweep ran.

---

## 1. The instrument, checked before it was trusted

`rtf-style-r97/readfodt.py` was defective when that round inherited it and is fixed; the fix is what
this round's reading of headings rests on, so it was reproduced first rather than assumed.
Regenerating `genpool2.py`'s 60 probes and reading them back, **all twenty `own`-arm rows report
`12pt@direct`** — O9's rule, that a style's own `\fs` reaches no paragraph, 20 of 20. In its broken
form the same instrument reported 14 pt on all twenty and would have refuted O9 on O9's own
evidence.

`readfodt2.py` is that corrected reader with the columns this seat turns on — `fo:color`,
`fo:text-align`, `fo:margin-left`, `style:font-name`, `style:text-underline-style`,
`style:text-line-through-style`, `fo:text-transform` — and the same two-part resolution: direct
character formatting on the paragraph's first span beats the whole `style:parent-style-name` chain,
because that is where the reference writes the reset.

## 2. What 26.2.4.2 actually does — 75 probes, and it is not the structural claim

`genheading.py` writes 15 names × 5 arms. The names are the twelve heading-parented ones —
`heading 1`…`heading 9`, `Heading 1`, `Title`, `Subtitle` — plus three controls: `Body Text`
(*Standard*-parented, round 95's answer, must inherit), `Quote` (no Writer style at all, must
inherit nothing) and a bare `Heading` (*is* `COLL_HEADLINE_BASE`, so the import resets it — round
97's trap). Each name's entry carries a forward `\sbasedon1392` that cannot resolve, which is the
condition under which a pool parent decides anything and the same condition `FormattingOf`'s switch
requires. The arms vary only what the document's own `Normal` states:

| arm | `Normal` |
|---|---|
| `plain` | `\f0\fs20` |
| `chars` | `\f0\fs20\b\i\cf1` |
| `marks` | `\f0\fs20\ul\strike\caps` |
| `paras` | `\f0\fs20\qc\li720\sb400` |
| `size` | `\f0\fs36` |

**`\fs36` rather than `\fs28`, because `PT_14` *is* `\fs28`** and a probe that cannot tell the
intermediate's size from `Normal`'s measures nothing. `heading-resolved.txt` is the reading; every
one of the twelve resolves as `Heading_20_N>Heading>Standard` (`Title>Heading>Standard`,
`Subtitle>Heading>Standard`) and every one answers identically:

| property | `plain` | what `Normal` states | 26.2.4.2 answers |
|---|---|---|---|
| weight | `-` | `\b` | **`bold@Standard`** |
| font style | `-` | `\i` | **`italic@Standard`** |
| colour | `-` | `\cf1` | **`#ff0000@Standard`** |
| underline | `-` | `\ul` | **`solid@Standard`** |
| line-through | `-` | `\strike` | **`solid@Standard`** |
| text transform | `-` | `\caps` | **`uppercase@Standard`** |
| text align | `-` | `\qc` | **`center@Standard`** |
| left margin | `-` | `\li720` | **`0.5in@Standard`** |
| size | `14pt@Heading` | `\fs36` | `14pt@Heading` — **shadowed** |
| margin above | `0.1665in@Heading` | `\sb400` | `0.1665in@Heading` — **shadowed** |
| margin below | `0.0835in@Heading` | — | `0.0835in@Heading` — **shadowed** |
| keep with next | `always@Heading` | — | `always@Heading` — **shadowed** |

**The structural claim alone does not settle this, and saying so is the point of the table.**
*Heading* sits between the level style and *Standard*, and every property it states of its own
shadows `Normal` — so "the chain reaches Standard" is compatible with nothing at all getting
through. What decides it is which properties the intermediate leaves unstated, and that is measured
property by property rather than derived.

The controls behave: `Body Text` is `Text_20_body>Standard` and takes `Normal`'s size (10 pt under
`\fs20`, 18 pt under `\fs36`) as well as all seven inherited properties; a bare `Heading` is
`Heading>Standard` with no spacing and no keep, which is the reset; `Quote` has a chain of length
one and takes nothing.

**And the reference's own resolved *Heading* can be read out whole, which is stronger than a probe
per property.** From `p_h1_chars.fodt`:

```xml
<style:style style:name="Heading" style:family="paragraph" style:parent-style-name="Standard"
             style:next-style-name="Text_20_body" style:class="chapter">
 <style:paragraph-properties fo:margin-top="0.1665in" fo:margin-bottom="0.0835in"
                             style:contextual-spacing="false" fo:keep-with-next="always"/>
 <style:text-properties style:font-name="Liberation Sans" … fo:font-size="14pt" …/>
</style:style>
<style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph"
             style:parent-style-name="Heading" style:next-style-name="Standard"
             style:class="chapter"/>
```

`Heading_20_1` is **empty** — `SetPropertiesToDefault` wiped it — and `Heading` states a font, a
size, an upper/lower space and keep, and nothing else. That is `HeadingPool`'s four members exactly,
with the face as the one deliberate omission: `ContributionOf` emits no font face at all, because no
style's face reaches a run (round 80), so modelling the intermediate's `LATIN_HEADING` font would
change nothing and there is no RTF `\f` index to express it as.

## 3. The source leg, and the one citation this seat was seated on

**`poolfmt.cxx`:279-289 is exact**, which is worth stating because round 97 found five of its own
inherited citations off by enough to matter. Transcribed from this tree:

```c++
279        case COLL_DOC_BITS:
280            switch (nId)
281            {
282                case SwPoolFormatId::COLL_HEADLINE_BASE:
283                    nRet = SwPoolFormatId::COLL_STANDARD;
284                    break;
285                default:
286                    nRet = SwPoolFormatId::COLL_HEADLINE_BASE;
287                    break;
288            }
289            break;
```

`COLL_DOC_BITS` is the group holding `COLL_DOC_TITLE`, `COLL_DOC_SUBTITLE`, `COLL_DOC_APPENDIX` and
`COLL_HEADLINE_BASE` through `COLL_HEADLINE10` (`sw/inc/poolfmt.hxx`:382-399), so the `default:` arm
is every heading level, `Title` and `Subtitle`, and the case above it is the intermediate's own
parent.

What the intermediate carries is `DocumentStylePoolManager.cxx`:769-820 — a `SvxFontItem` per script
from `DefaultFontType::LATIN_HEADING`/`CJK_HEADING`/`CTL_HEADING` at `:795-807`,
`SvxFontHeightItem aFntSize(PT_14, …)` at `:809`, `SvxULSpaceItem aUL(PT_12, PT_6, …)` at `:810` and
`SvxFormatKeepItem(true, RES_KEEP)` at `:813`. **Two line numbers carried in `RtfStyles.cs` were one
out and are corrected**: the block is `:769-820` not `:768-819`, the keep item is `:813` not `:814`,
and `aHeadlineSizes` is `:107-114` not `:107-115`.

**This tree is not the reference binary's source.** `configure.ac`:21 declares
`27.2.0.0.alpha0+`; the binary is 26.2.4.2 and its build hash is not an object here. So the source
leg above is a different version's explanation, and §2's flat-ODF reading — which is a measurement
of the actual reference — is what the change stands on. The two agree property for property, which
is the most that can be said.

## 4. The prediction, written before the sweep

`headingreach.py` applies `ContributionOf`'s own-beats-inherited rule by hand to the entry's
control words, over the properties §2 shows survive the intermediate. It is a reimplementation of
the model and is therefore evidence about the model rather than about the reference; it is here to
be wrong in public. `headingreach.txt` is its output and was committed to this directory before
either leg of the sweep ran:

| | |
|---|---:|
| converted `.rtf` scanned | **338** |
| documents applying a heading-parented name with no resolvable `\sbasedon` | **12** |
| (document, style) pairs | **28** |
| pairs predicted to gain a property | **18** |
| **documents predicted to change** | **9** |

Beside it, a refinement that the census does not make and that was written down at the same time:
of the nine, only the four whose `Normal` states `\qj` should change *ink* — `\ql` is left alignment,
which is already the default; `\cf0` is colour-table index 0, whose resolved value is the black the
runs already had (round 97 measured exactly that on `19-06`); and `\lang` changes nothing drawn.

**Measured: one.** The prediction is wrong by a factor of nine on documents and four on ink, and §5
is why.

## 5. The sweep, and the per-document table

`reach-sweep.py` renders every one of the **338 converted `.rtf`** in `/home/user/corpus-odf/words`
with `XRay.Office.Cli render --format pdf` under a pinned `SOURCE_DATE_EPOCH`, hashes, and deletes the
PDF immediately. Determinism was checked before the sweep rather than assumed: two renders of
`bulletin.rtf` by the same binary are byte-identical. Both legs rendered **338 of 338** with no
failures. `reach-base.tsv` is `ac7745deb` with `RtfStyles.cs` at HEAD-of-base; `reach-head.tsv` is
this round's code.

**The corpus is 338 `.rtf` and 336 of them have a banked 26.2.4.2 reference**
(`/home/user/gate-odf-r80/ref`). This sweep is the 338, because it compares our two legs against
each other; §6 scores the one mover against the banked 336.

| | |
|---|---:|
| `.rtf` rendered on each leg | **338** |
| renderings that differ by a byte | **1** |
| renderings byte-identical | **337** |
| documents applying a heading-parented name | 12 |
| of those twelve, documents whose rendering does not move | **11** |

`candidate-table.txt`, generated from the corpus after the sweep and joined to its mover list:

| document | pairs | `Normal` align | what the pool parent newly supplies | rendering |
|---|---:|---|---|---|
| `PES-Technical-Report-Template_Jan_2019` | 2 | `\qj` | align, colour, lang | **moves** |
| `24-25_FAA_Holdover_Tables` | 2 | `\qj` | align, colour | does not move |
| `FAA 2025-26 Holdover Tables` | 2 | `\qj` | align, colour | does not move |
| `EHEST-SMS-Safety-Management-Manual-V2` | 1 | `\qj` | align, colour, lang | does not move |
| `PAT-047 - Architecture and Detailed Design Assessment` | 1 | `\ql` | align, colour, lang | does not move |
| `final-technical-report-template` | 1 | `\ql` | align, colour, lang | does not move |
| `03_Technical_Report_(progress)_template` | 2 | `\ql` | align, colour, lang | does not move |
| `Sample_SQMS_Program` | 4 | `\ql` | align, colour, lang | does not move |
| `template---tpr-technical-progress-report-with-guidance` | 3 | `\ql` | align, colour, lang | does not move |
| `hdss-bulletin-issue-285-25-june-2025` | 4 | `\ql` | **nothing** | does not move |
| `231164_SystemDesignDocument` | 5 | `\ql` | **nothing** | does not move |
| `CRIF - Spécification technique - Socle applicatif` | 1 | `\ql` | **nothing** | does not move |

Three classes, and each of the eleven non-movers is in one of them rather than shrugged at:

* **Three gain nothing at all.** Their heading entries state every property `Normal` states —
  `hdss-bulletin`'s `heading 1` is `\ql\cf17\lang3081…`, `231164`'s is `\qr\cf0\lang1033\b` — which
  is O24's finding arriving again: *a style name appearing in a style table is not reach, and
  neither is a name that is applied.*
* **Five gain only pixel-neutral properties.** `\ql` is `TextAlignment.Start`, which every paragraph
  already had; `\cf0` resolves to the black the run was already drawn in; `\lang` reaches nothing
  drawn. Setting a value to the value it already held changes no byte, and the sweep says so:
  byte-identical, not merely pixel-identical.
* **Three gain justification and still do not move, because of where their heading styles are
  applied.** `24-25_FAA` and `FAA 2025-26` apply `heading 1` to a single one-line paragraph
  (`\pard\plain \s1\lang1033{FAA\line HOLDOVER TIME GUIDELINES}`) and `heading 4` to 214 and 238
  paragraphs that are each one line of a table caption; a justified line that is also the
  paragraph's last line is not stretched. `EHEST`'s twenty `heading 2` paragraphs are one-line
  chapter titles. **This is the class the prediction could not see**, because `whichproperty`-shaped
  arithmetic knows what a style contributes and not whether the paragraphs it reaches have a second
  line.

**So the prediction held on the arithmetic and failed on the geometry.** Every pair it said would
gain a property does gain it; what it could not say is that gaining `justify` is worth nothing to a
paragraph that never wraps. The correction that would have made it right is one more column — *does
any paragraph in this style exceed one line* — and it needs a layout, not a regex.

## 6. The one mover, and which way it went

`PES-Technical-Report-Template_Jan_2019` is 15 pages here and **17 at 26.2.4.2**, so `mover-ink.py`
— which scores page *N* against page *N* — cannot answer whether this change moved it towards the
reference. It reports 12.0220 → 12.0241, and **that number should not be read as "slightly worse"**:
past the pagination divergence it is comparing different pages. Our own two legs differ on 4 of 15
pages (mean grey 6.80, 1.80, 2.88, 0.92 at 150 dpi), so the change is pixels and not only bytes.

`flushlines.py` is the instrument that does answer it, and it does not depend on the two sides
paginating alike. A justified line ends flush at the text area's right edge and a ragged one does
not; so it takes every text line with its right edge, takes each rendering's **own** right margin as
the most common right edge (ours 522.00 pt, the reference's 525.10 — the first cut of the script
used the *largest* right edge, which a running head or a table rule reaches past, and that put the
margin at 588.66 and found one flush line of 340), and pairs a line with the reference's by its
text. `flush-pes.txt`:

| leg | lines | paired | flush | reference flush over the paired | agree |
|---|---:|---:|---:|---:|---:|
| base | 340 | 294 | 115 | 108 | **256 / 294** |
| head | 340 | 294 | 142 | 108 | **274 / 294** |

**18 paired lines change flushness between the two legs, and 18 of 18 end up agreeing with the
reference. None moves away.** The witness, one `heading 9` paragraph, read out of all three PDFs:

```
ref  (page 13)  x0=162.10  x1=525.15  'Use words rather than symbols or abbreviations when writing…'
                x0=162.10  x1=525.10  'labels to avoid confusing the reader. As an example, write…'
                x0=162.10  x1=426.51  '“Magnetization,” or “Magnetization, M,” not just “M.”'
base (page 11)  x0=144.00  x1=511.94  ragged
head (page 11)  x0=144.00  x1=522.04  flush at our own 522.00 margin
```

The reference justifies that paragraph; at base we drew it ragged and now we justify it. **The
residual 3.1 pt between the two right margins is not this change** — it is a pre-existing difference
in where the two put the text area's right edge on that document, and it is why a pairing instrument
has to use each side's own margin.

Why the document is the one mover is visible in its stylesheet: its `heading 9` is applied 27 times
to multi-line body paragraphs that state no alignment of their own, while its `heading 8` is applied
15 times to paragraphs that each state `\qc` and are therefore unaffected. The reference's own view
of the file agrees with the model exactly — `--convert-to fodt` gives `Standard`
`fo:text-align="justify"`, `Heading` no `fo:text-align` at all, and `Heading_20_8`/`Heading_20_9`
parented on `Heading` — and the same reading on `24-25_FAA` (Standard justify) and
`Sample_SQMS_Program` (Standard `fo:text-align="left"`) is what confirms two of the non-movers are
non-movers for the stated reason rather than because the code did not fire.

## 7. What changed in the tree

Two statements, both of which make `Heading` behave exactly as `Caption` already did:

* `FormattingOf`'s switch adds `HeadingPool` to the chain **and continues the walk into style 0**
  rather than stopping, so the merge is `entry.Over(HeadingPool.Over(Normal))`;
* `PoolInheritance` answers `HeadingPool.Over(NormalInheritance(id))` rather than the bare constant.

`HeadingPool`'s four members are unchanged and are what shadows `Normal`'s size, spacing and keep.
No new name is added to `PoolParentOf`, and `IsNumbered`'s exact-ordinal comparison is untouched.
The `visited` set already guards the one degenerate case the new `current = DefaultStyleId` creates
— a document whose style 0 is itself named `Title` — by refusing to re-enter a style already walked.

Eleven test cases, in two theories:

* `AHeadingParentedStyleTakesTheDocumentsOwnNormal` over seven names, asserting weight 700, italic
  and `TextAlignment.Centre` from a `Normal` stating `\b\i\qc`. **All seven fail at the base**, with
  `RtfStyles.cs` reverted and the test project rebuilt.
* `TheHeadingPoolStillShadowsSizeSpacingAndKeep` over four names, asserting 14 pt, 12 pt above, 6 pt
  below and keep against a `Normal` stating `\fs36\sb400`. **All four pass at the base**, which is
  what a control must do — they are there so that the walk cannot be widened into a regression
  without a test noticing.

`RtfStyleFormattingTests` is **52 of 52 at the base and 63 of 63 at HEAD**, and the 24 cases round
97 added still pass.

## 8. What is not settled

* **`COLL_TEXT` as an intermediate** is still `N17` and still nil on this corpus; nothing here
  touches it, and `AnIntermediateBelowTextBodyIsNotModelledYet` still pins it.
* **The face.** `HeadingPool` states no font and `ContributionOf` emits none, so this tree cannot
  express *Heading*'s `LATIN_HEADING` family. It costs nothing today — round 80 measured that no
  style's face reaches a run at 26.2.4.2, four probes — but it is the one property of the
  intermediate that is modelled by omission rather than by value, and a round that ever makes a
  style's face reach a run has to revisit it.
* **`COLL_DOC_APPENDIX`** is the third `COLL_DOC_BITS` name and is not in `PoolParentOf`. It was not
  measured; 0 of the 338 apply a style called `Appendix`.
* **Space *after*** was not probed against a `Normal` that states `\sa`, because no RTF `\sa` is
  ever read into `RtfStyleFormatting` (round 80's rule) — so the arm could not have discriminated.
  `HeadingPool` states 120 twips and shadows whatever is underneath it either way.
* **PES's remaining 12.02 of ink** is a pagination gap (15 pages against 17) and a 3.1 pt difference
  in the text area's right edge. Neither is this seat, and the second is the cheaper-looking of the
  two.

## 9. Validation

* `dotnet build XRay.Office.slnx` — **0 warnings, 0 errors**.
* The ten non-fidelity projects, run one at a time, all `Passed!` with 0 failed and 0 skipped:
  Containers 109, Core 521, Markup 259, OpenDocument 146, Presentations 1045, Rendering 164,
  Spreadsheets 1271, Text 728, Vector 309, **WordProcessing 1924** (1913 before, plus this round's
  eleven).
* **`XRay.Office.Fidelity.Tests`: `Failed: 10, Passed: 542, Skipped: 0, Total: 552`** — the briefed
  baseline, and the ten are exactly the known names, read out of this run's own log rather than
  asserted: `PageDrawingComparisonTests.EveryLineIsDrawnWhereLibreOfficeDrawsIt` ×4
  (`paginated.doc/.fodt/.rtf/.docx`),
  `TabStopComparisonTests.AListLabelsTabAdvancesToLibreOfficesStop` ×4
  (`list-label-overrun.fodt/.docx/.odt/.doc`),
  `SheetDrawingComparisonTests.APictureIsDrawnWhereLibreOfficeDrawsIt(sheet-rich-text.xlsx)` and
  `JustificationShrinkComparisonTests.TheParagraphBreaksWhereLibreOfficeBreaksIt(justify-shrink-2013.docx)`.
  No eleventh.
* **The tests fail at the base**: `RtfStyles.cs` reverted to `ac7745deb` and the test project
  rebuilt, `RtfStyleFormattingTests` is 7 failed / 56 passed of 63; at HEAD 63 of 63.
* Corpus reach: **338 rendered on each leg, 1 differs, 337 byte-identical**, and the one mover's
  18 changed lines all move towards 26.2.4.2 (§6). `RtfStyles.cs` is reachable only from an RTF
  document, and the 338 are every `.rtf` there is — the corpus proper holds none — so the 337
  unchanged renderings are the confinement measured rather than assumed.
* Every figure quoted above comes from a file in this directory that exists and is non-empty;
  `heading-resolved.txt` is 75 probe rows, `headingreach.txt` 28 pairs plus its three totals,
  `reach-base.tsv`/`reach-head.tsv` 338 rows each, `flush-pes.txt` the two legs and the eighteen.

**One thing this round nearly published and did not.** The first cut of `flushlines.py` took each
rendering's right margin as its *largest* right edge, and reported `flush 1 of 340` with the two
legs in perfect agreement — a clean, confident null that would have said the one mover's change was
invisible. A running head reaches past the body's measure. **An instrument that reports agreement is
as much in need of a control as one that reports a difference**, and the control here was the
histogram of right edges, which had 522.00 at 115 occurrences sitting plainly in it.
