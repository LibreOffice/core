# slides-r50 — results

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
base `ac147b7e5bb`, `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`. Read `prediction.md` beside this
file first; it was committed before anything was rendered post-change.

## Baseline, and it reproduced exactly

Whole-track sweep, all 35 slides batches: reconciled document by document against `MANIFEST.tsv`,
**302 of 302 agree, 0 disagreements — 198 of 302 passing**, the briefed figure.

## Measured against the prediction

| | predicted | measured |
|---|---|---|
| verdict movement | **0** | **+1** (198 → **199**) |
| renderings moved | 30 near-certain, up to ~60 | **59** documents' `\|ink\|%` moved; 56 word counts moved |
| regressions among the 8 named passing documents | possible | **0** — all eight held `match` |
| page counts moved | not predicted | **0 of 302** |

The prediction was **wrong in the safe direction**, and the reason is worth recording: I argued
no document sat near the max(2%, 3) band *from below*. One did.
`7-Zulkefli_Part147n66_IKMAS__pptx` (kind `ceiling`) was 941 words to the reference's 973 — a gap
of 32 against a band of 19.46 — and recovering **28** words it had been drawing off the page took
it to 969/973, inside the band. It is the single verdict, `words` → `match`.

That is also the answer to the brief's warning about the `ceiling` label: this document was filed
as an unwinnable measurement ceiling and it was not one. It had 10.6 pt of text hanging off the
page edge.

### Ink over the whole track — the sweep round 41 never ran

| | before | after |
|---|---:|---:|
| `\|ink\|%` unsigned | 1419.70 | **1409.36** |
| `ink%` signed | 1041.64 | 1040.62 |
| major pages | 500 | 498 |

Invariant `\|signed\| ≤ \|ink\|` holds on both. 59 documents moved, **57 improved and 2 worsened**.

**A naming trap worth fixing before the next round quotes it.** `track-ink-sweep.sh`'s own
`INK` summary line sums the **signed** column (1045.96 at baseline), while `ink-ranking.py`'s
headline is the **unsigned** one (1419.70). They are the two measurements the brief warns
circulate under one name, and here they are produced by two scripts in the same skill.

### Character identity — the measurement that can actually see this change

Charstream (all whitespace stripped from both `pdftotext` extractions) over all 302, before → after:

| kind | n | character-identical before | after | delta |
|---|---:|---:|---:|---:|
| ceiling | 75 | 58 | 58 | +0 |
| **text** | 28 | **5** | **22** | **+17** |
| chart | 1 | 0 | 0 | +0 |
| passing | 198 | 60 | 60 | +0 |
| **total** | 302 | 123 | **140** | **+17** |

Our word count rose on 55 documents and fell on 1.

## The change

`a:bodyPr/@wrap="none"` is honoured **only while the body's autofit leaves the shape alone**.
Measured on 26.2.4.2 with nine authored one-shape decks, both axes varied independently, a 236 pt
box holding a 64-character line at 22 pt:

| `wrap` | `(absent)` | `noAutofit` | `spAutoFit` | `normAutofit` |
|---|---:|---:|---:|---:|
| `none` | 1 line | 1 line | **4 lines** | **4 lines** |
| `square` | 4 | 4 | 4 | 4 |

We read the attribute on its own and laid every `wrap="none"` body out unbounded. The cost is not
cosmetic: an unbounded line runs off the **page**, not merely off the shape, and everything past
the media box is gone from the text layer. At baseline **30 of our renderings drew text outside
the page against the reference's 9**.

`PptxTextBody`'s chain walk already located the nearest stated autofit; it simply did not record
which of the three it was.

## Refutations

### 1. The brief's mechanism for the 75 `ceiling` documents is wrong

The brief attributes the token-splitting to `spc="150"` letter spacing. On
`049_Five-Block_Hub_Spoke` the `spc="150"` in `slideMaster1.xml` is at offset 3210 and
`p:titleStyle` spans [97608, 98070] — **it is not in the title's inheritance chain at all** and
does not apply to the text that splits.

Re-zipping the real deck one attribute at a time isolates the cause to **the family nibble of
`pitchFamily` on `<a:latin>`**:

| variant | title span | `pdftotext` tokens |
|---|---:|---:|
| as-is (`Helvetica` + panose + `pitchFamily="34"` + charset) | 622.40 pt | **11** |
| `panose` only | 541.55 | 6 |
| `charset` only | 541.55 | 6 |
| **`pitchFamily` only** | **622.40** | **11** |
| `panose` + `charset` | 541.55 | 6 |
| `kern` removed | 622.40 | 11 |

Reproduced in isolation on authored decks for `Helvetica`, `Times`, `Albany` and `Thorndale`
(`pitchFamily="2"`, whose family nibble is zero, behaves as absent), and **inert on all five
installed families tested** — `Arial`, `Calibri`, `Liberation Sans`, `Carlito`, `DejaVu Sans` all
render identically with and without it. So it fires only where LibreOffice must substitute.

**This is unimplemented and deliberately so.** `SlideText.DeclaredPitches` records that the family
bits are read by nothing because the behaviour "has never been measured on a slide". It has now
been measured, and the measurement says implementing it would make our text ~15% wider and split
its own words — matching LibreOffice at the cost of matching PowerPoint. The right next step is a
decision, not a patch.

### 2. Font substitution is not the cause of the wider reference title

Both sides embed the **same subset, `DAAAAA+LiberationSans-Bold`**, at the same `36 Tf`. We emit
one `Tj`; the reference emits a `TJ` array with a negative adjustment after every glyph, worth
+2.84 pt/gap on line 1 and +3.32 pt/gap on line 2 — **not uniform, so not letter spacing either**.
An authored `Helvetica` probe resolves to `LiberationSans-Bold` at every `pitchFamily` value, so
the face never changes; only the positioning does. My own first reading of this — that the
reference had fallen back to DejaVu Sans Bold — was wrong, and the `-bbox` height caught it:
both sides report 40.2 pt, which is Liberation's `usWinAscent + usWinDescent` at 36 pt and not
DejaVu's.

### 3. The 28 `text` documents were not 28 defects, and mostly not a ceiling either

9 of the 28 had an *identical* character multiset before the change. 15 more differed from the
reference by **exactly two characters**, always the same two: the `es` of `Google Slides`, lost
off the page edge. Only **two** members — `038_Competitive_Advantage_Card` (48 characters) and
`035_Chemistry_Column_PowerPoint_Chart` (20) — differ in chart labels and are genuinely separate.

## Ceiling-sample verification — all 75, not a sample of 10

The brief asked for ~10 of the 75 `ceiling` documents. The sweep's own PDFs made the charstream
test cheap enough to run on all 302, so **all 75 were checked**.

**58 of 75 correctly filed** — characters identical once whitespace is stripped, word gate failing
purely on tokenisation. **17 have genuinely differing characters and are misfiled**, and they are
not all trivial:

| jaccard | document | note |
|---:|---|---|
| 0.787 | `OnTrac_StarCertificationProgram-3Day` | known-open |
| 0.886 | `Demick_JetBlue` | known-open (missing subgrid) |
| 0.899 | `W3_Case_Study_of_a_Tsunami…` | — |
| 0.904 | `16 - UTM - (NASA)` | known-open |
| 0.913 | `WiGr_2021W…` | on the "we render better" list |
| 0.947 | `8_P-Pavese_AIRBUS…` | known-open (table fills) |
| 0.965–0.999 | 11 others incl. `7-Zulkefli…` | `7-Zulkefli` **was fixed this round** |

So the label was wrong on 17 of 75, and on at least one of them (`7-Zulkefli`) it was hiding a
verdict that a small fix could take. The brief's instruction to sample was the right one.

**And the control that matters:** the same test over the 198 documents that already **pass** finds
**138 of 198 with differing characters**. Character difference on its own therefore does not
separate a failing document from a passing one, and must not be used as a classifier without the
word-count context beside it.

## The one regression, stated plainly

`iris07.12.12__pptx` (passing, and still passing) went from 881/881 exact to **871/881**, and its
`|ink|` from 0.40 to 0.59. Its character *multiset* is unchanged — 5010 both sides — so no text
was lost; the emission **order** changed on rotated labels, where the reference writes `coverage`
as `agerevoc`. Ten tokens merged differently as a result. It remains inside the band (10 against
17.62) and I have left it, but it is the honest cost of the change and the next round should not
rediscover it as a mystery.

`WiGr_2021W…` also moved +0.16 `|ink|`; it is on the do-not-work list.

## An environment fact that invalidates a claim in my own prediction file

`prediction.md` says three corpus files exist twice on disk, `.pptx` and `.PPTX`, byte-identical.
**That is wrong and the correction is the more useful fact.** `/c/sandbox/workdir` is a
**virtiofs mount and case-insensitive**: `049_…pptx` and `049_….PPTX` are the *same inode*
(2251799814379512, one md5). There is one file.

The consequence is a live measurement trap. `look.py` resolves a document by
`CORPUS.rglob(stem.ext)` **plus** `CORPUS.rglob(stem.EXT)`, and touching the upper-case spelling
materialises it in the directory cache — so `find` then returns it as a second file *forever after,
for every subsequent sweep*. My baseline sweep counted 305 files; after running `pair.sh` on six
decks the post-change sweep counted **311**, with no change to the corpus. Both are 302 documents.

**Any sweep total on this corpus is unstable, and `batch-check.sh`'s `TOTAL` line can rise between
two runs of the same command with nothing changed.** Reconcile on a case-folded identity, which is
what every number in this file does. The three files flagged at baseline were the ones some earlier
session had already touched the same way.

## Tests

`SlideWrapAutofitTests`, 11 tests: the 9-cell measured grid plus two inheritance cases.

Verified by **reintroduction** (`verify-test.sh XRay.Office.Presentations '<revert the rule>'
SlideWrapAutofit`): the mutation was **DETECTED**, failing 3 of 11 —
`WrapNoneIsHonouredOnlyWhileTheAutofitLeavesTheShapeAlone(wrap: none, autofit: spAutoFit)` and
`(normAutofit)`, and `AnInheritedFittingAutofitBeatsTheBodysOwnWrapNone`. The other **8 are drift
guards**: they pin the cases that were already correct, which is what stops a future fix from
buying these two rows by breaking `wrap="square"`.

The mutation had to be written as `|| (resizes && false)` rather than by deleting the term,
because `TreatWarningsAsErrors` promotes the unused local to a build failure and `verify-test.sh`
correctly refuses to score a build failure as a detection.

Ten non-Fidelity projects, all green: Containers 109, Core 337, Markup 259, OpenDocument 125,
Presentations 747, Rendering 150 (+1 skipped, `PdfFontTests.ACffFlavouredFaceIsNotClaimedToBeTrueType`,
skipped at baseline too), Spreadsheets 882, Text 596, Vector 295, WordProcessing 1052 —
**4552 passed, 0 failed**.

## Shared layers

The diff touches `XRay.Office.Presentations` only — `Ooxml/PptxTextBody.cs` and
`Layout/SlideText.cs` (comment). Nothing in `Core`, `Containers`, `Text`, `Vector`, `Rendering` or
`Markup`, so no cross-track measurement is owed. The words and sheets sweeps are unaffected by
construction, and the Spreadsheets and WordProcessing suites were run and are green.

## Left open, in the order the next round should take them

1. **`social-media-app-bulletin-january__pptx` p3 — we paint an opaque black rectangle behind a
   transparent picture**, occluding the words `Social Media` in the title and clipping the first
   body line. Found by a blind reviewer on a **passing** document, previously unrecorded. The
   image cannot separate: alpha discarded and composited onto black, a colour-key transparency we
   ignore, or a shape fill we paint and the reference suppresses. Cheap to settle by sampling the
   pixel right of the wordmark and checking the picture for an alpha channel.
2. **Autofit on the `.ppt` path, in both directions.** Two independent blind readings of passing
   documents found the same class with opposite signs: `2015-Civil-Rights-Website-training__ppt`
   p42 sets the body ~10% **larger** than the reference (14 lines to 12);
   `ITE106-Chapter 4__ppt` p7 sets it ~10–15% **smaller** (9 lines to 10) with visibly larger
   inter-bullet gaps. The second reviewer named the mechanism unprompted — a font scale applied
   without the matching spacing reduction. This is HANDOVER §8's largest named front
   ("text sizes are different", 17 of the user's 30 observations) reproduced blind on two
   documents no one had flagged.
3. **Decide `pitchFamily`'s family nibble.** The measurement the code asked for exists now (§1
   above). It is a decision about whether to match LibreOffice's substituted-font metrics at the
   cost of drawing text PowerPoint would not draw, and it governs the largest block of remaining
   slides word-gate failures. Do not implement it without that decision.
4. **Re-file the 17 misfiled `ceiling` documents** listed above, and re-run the charstream test
   over the track after any change — it is the only instrument here that can see a fix the word
   gate cannot.
5. `038_Competitive_Advantage_Card` and `035_Chemistry_Column_PowerPoint_Chart` — the only two
   genuine content differences left in the old `text` pool, both in chart labels.
