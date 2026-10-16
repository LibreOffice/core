# `slides/batch-008` round 1 — `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`

**Verdict: the document is at a ceiling, and the ceiling is *larger than the gap*. No code
changed.** The word gate reads +108; the ceiling accounts for **+145**, and the document only
looks like +108 because we are simultaneously **losing 40 words of real content** to three
defects of our own. Two of those defects sit on the very pages the ceiling is on, which is why
`raster-ceiling-pages.py` flags one page of this document instead of three.

Measured at `489be9b9eee` (the prediction commit; the tree is unchanged since).
Reference: banked `26.2.4.2` renderings with the correct font set, cross-checked against a
live `soffice` run — see "Instrument controls".

## The split

Gate row (`batch-check.sh` conventions, letter-or-digit words):
`pages 26/26  words 2118/2010  verdict: words`.

Only five of the twenty-six pages differ at all. The other twenty-one are exact, both metrics.

| page | net | of which ceiling | of which ours | mechanism |
|---|---:|---:|---:|---|
| 3 | **+3** | — | +3 | 14 glyphs of **zero-ink** text we emit and the reference does not — unexplained, see below |
| 5 | **+43** | +43 | — | *raster ceiling*: the reference draws a 692×240 JPEG + soft mask (object 207) where we draw the journal citation as real text |
| 6 | **+31** | +43 | −12 | the **same** object 207 raster; against it, a table row we push off the page |
| 8 | **−7** | — | −7 | `percentStacked` not honoured (axis reads `70000%`) and the `chartUserShapes` part not read |
| 16 | **+38** | +59 | −21 | ***outline ceiling*** — new, see below; against it, unwrapped category labels and half the value ticks |
| **total** | **+108** | **+145** | **−37** | |

```
ceiling  +145
ours     − 40   (−12 p6, −7 p8, −21 p16)
unknown  +  3   (p3, zero-ink)
         ------
net      +108   ✓ matches the gate exactly
```

**Both corrections point the same way and the gate cannot be won from either end.**

- Excuse the ceiling and the document reads 1973 against 2010, i.e. **−37 against a ±43.2
  band — inside it, a pass.** It passes only because two errors of opposite sign nearly cancel.
- Fix all three of our defects, changing nothing else, and it reads **2158 against 2010, +148**
  — *further* outside the band than it is now. Every real improvement available on this
  document makes its gate column worse.

That second line is the useful one. It is the raster ceiling's standing argument ("driving the
number down means drawing less text") in its sharpest form yet: here the number can only be
driven *up*, by drawing content we are currently dropping.

## The new thing: a ceiling `pdfimages` is structurally blind to

`TODO.raster-ceiling.md`'s three-condition page test asks `pdfimages` whether the reference
drew a raster we did not. **There is a second way for the reference to put ink on a page and
nothing in its text layer, and that test cannot see it: it outlines the glyphs into filled
paths.**

On page 16, in the band beneath the first chart's baseline (x 17–368, PDF-y 154.7–170.7):

| | reference | ours |
|---|---:|---:|
| text-showing operators in the band | **0** | 18 |
| glyphs in the band | **0** | 103 |
| glyph-sized filled paths (< 12×12 pt), one colour `#595959` | **120** | — |
| `pdfimages -list` entries on the page | **0** | 0 |
| month tokens from `pdftotext` (`-raw`, `-layout`, `-bbox`) | **none** | 59 fragments |

Twenty rotated date-axis labels (`Apr-19` … `Jun-22`), six characters each, is **120 glyphs**.
The reference's 120 grey fills are those glyphs, outlined. The page's whole-page fill count is
180 against our 58 — a difference of **122**, which is those 120 plus two bar rectangles.

**Two blind reviewers, sent the page and a 300 dpi crop with no numbers and no repository
access, both read the month labels as present and legible in the reference** — one calling them
"grey … rotated ~45° ascending", matching `#595959` and the observed bbox slopes. Their reading
and the operator census agree, and the text layer disagrees with both. That is the whole finding:
the ink is there, the text is not, and ours is the better output.

**The within-document control is what makes this a mechanism rather than an anecdote.** On the
same page, in the same PDF, the reference's *horizontal* axis labels — Figure 2's
`lundi…dimanche`, Figure 3's `[08 h ; 10 h[` intervals, every value tick — are ordinary text
operators that `pdftotext` reads without trouble. Only the rotated run is outlined.

**Stated as a hypothesis, not as established:** LibreOffice outlines rotated chart tick labels
on PDF export. One document, one chart, with a good internal control. What is *established* is
the observation in the table above. Note this is **not** the same shape as
`Template Pilot Logbook JAR-FCL V3.0.xls`, already in the ceiling file, where LibreOffice emits
one `Tj` per glyph for rotated text — there the rotated text is still text. Here it is not text
at all.

### How common is it

`outline-ceiling-census.py` (in this directory) looks for the signature over a finished sweep:
page-exact document, ≥ 8 more raw words on the page than the reference, ≥ 20 glyph-sized
one-colour fills on the reference page, ≥ 20 more text glyphs from us. Over **78 documents and
~2400 pages** (`slides/batch-001` … `008`) it returns **exactly one row** — this page.

So it is rare in what has been swept. It is worth keeping the script because it costs nothing
on a sweep that already exists, and because the class is invisible to every instrument the
project currently points at a word-count failure.

## Why the flag table under-counts this document, precisely

`TODO.raster-ceiling.md` already says the list is a deliberate under-count and offers page 6 of
this document as the example, at "+44 on 180 — 24.4%" against the 25% bar. The number has moved
and, more importantly, **the reason is not the one recorded.**

| page | gross ceiling excess | ref raw words | gross % | our defect on the same page | net % | flagged? |
|---|---:|---:|---:|---:|---:|---|
| 5 | +44 | 70 | **62.9%** | 0 | 62.9% | yes |
| 6 | +44 | 169 | **26.0%** | −12 | **18.9%** | no |
| 16 | +59 | 200 | **29.5%** | −25 | **17.0%** | no (and invisible to `pdfimages` anyway) |

Pages 6 and 16 are both **over** the 25% bar on gross excess. They fall under it only because a
defect of ours on the same page subtracts from the delta. Condition 3 is evaluated on the *net*
per-page difference, so **the flag is suppressed exactly on the pages that also carry a defect**
— the two under-counts are not independent, they compound. A page that is both a ceiling and a
defect is the case the signature handles worst, and it is not a rare combination: two of this
document's three ceiling pages are that case.

This is a sharper statement than "it is a property of my threshold", and it suggests the fix if
one is ever wanted: compute condition 3 on the **ours-only token count** rather than on the net
delta. On this document that alone would flag pages 5 and 6 correctly without moving the bar.

## The three defects of ours, all real, all recorded rather than fixed

None was fixed. Each is chart- or table-layout work of its own size, and each moves the gate
column the wrong way. Recording them precisely is the deliverable.

### Page 8 — `percentStacked` ignored, and the `chartUserShapes` part not read (−7)

`ppt/charts/chart1.xml` is `<c:grouping val="percentStacked"/>` with `<c:overlap val="100"/>`,
raw data `548, 317` (*Suivi*) over `73, 122` (*Non suivi*) — so 548/621 and 317/439 — and a
value axis carrying `<c:numFmt formatCode="0%" sourceLinked="1"/>`.

We do not normalise the stack. We auto-scale to the raw total (≈621 → 700) and *then* apply the
`0%` format, so the axis reads **`0%, 10000% … 70000%`** in eight ticks where the reference reads
**`0% … 100%`** in eleven. Three of the seven missing tokens are those three ticks.

The other four are `88%`, `72%`, `(548/621)`, `(317/439)`. They are not in `chart1.xml` and not
in `slide8.xml`; `chart1.xml.rels` carries
`Type=…/chartUserShapes Target="../drawings/drawing1.xml"`, and that part's entire text content
is exactly `['88%', '72%', '(317/439)', '(548/621)']`. **We do not read `chartUserShapes` at
all.**

`chart1.xml.rels` also carries a `themeOverride` → `ppt/theme/themeOverride1.xml`, whose
`minorFont` latin is **Palatino Linotype**. The blind reviewer, with no access to any of this,
reported that the reference's chart title, axis labels and category labels are a **serif** face
while ours are **sans**, and named "a theme major/minor latin font not being applied" as its
first candidate cause. Blind reading and source agree. The reviewer also reported the
reference's black chart-area and charcoal plot-area fills as absent from ours, and its data
labels as invisible in ours — **the fills I did not trace to a cause and do not claim the
override explains them.**

Of the six charts in this deck, `chart1` is the only one with either relationship.

### Page 6 — a table row pushed off the page (−12)

The reference's table has twelve body rows; ours has eleven. The twelfth,
`Cardiovascular infection (excluding material) 133 (5) 14 (3) 5 (1) 152 (4)`, is exactly the
twelve tokens the reference has and we do not. It is in `slide6.xml` (and so is a thirteenth,
`Non infectious pathology`, which neither side draws).

The blind reviewer found this unled, and found the mechanism with it: *"the whole table body
sits LOWER in OURS by very close to exactly one row height … row-to-row pitch looks identical in
both halves, so this is a single fixed downward offset introduced above the body, not an
accumulating per-row growth"*, locating it in the gap between the multi-line column headers and
the `N=2576` row. It also noticed the corroborating detail: the four cell highlight rectangles
sit at the same absolute page positions in both renderings but land on rows 1/4/8/11 in ours and
2/5/9/12 in the reference — the fills are page-anchored and the text slid under them.

So: a one-row over-measurement of the header block, most likely the 3-line
`Grenoble university-affiliated hospital` cell.

### Page 16 — category labels not wrapped, value axis under-ticked (−21)

Figure 3's nine interval labels (`[08 h ; 10 h[` …) are wrapped onto two lines by the reference
and drawn in full. We measure each as one unbreakable run, and drop every second label — five of
nine. Its value axis we tick every 200 to 800; the reference ticks every 100 to 900.

## Page 3: we write text that makes no ink, and it is in no known source

Fourteen one-glyph text operators at PDF-y 357.98, x 84.18 → 253.59, Carlito at 27.29 pt with a
29.64 horizontal width — decoding through WinAnsi as **`Merci à vous !`**. The reference draws
nothing there.

Two things about it are established and one is not:

- **It puts no ink on the page.** A 300 dpi grey crop of x 80–260 pt, y 155–195 pt returns
  **0 pixels darker than 254** out of 124 500, on both sides. The same probe over our page-3
  title returns 55 448 dark pixels of 239 250, so the probe works. A blind reviewer sent that
  crop reported it blank, correctly.
- **The string is in no part of the package.** Every zip entry scanned for `Merci` and `vous` in
  UTF-8 and UTF-16LE: `Merci` occurs once, in `slide22.xml`, as `Merci P Lesprit`; `vous` occurs
  once, inside `nervous` in `slide6.xml`. It is not in `slide3.xml`, `slideLayout4.xml`,
  `slideMaster1.xml`, any notes slide, or `docProps`.
- **Unexplained.** Candidates not separated: a placeholder body we synthesise; a run we draw in
  white or in text-render-mode 3; or a wrong `ToUnicode`/encoding making other glyphs decode as
  this phrase. It is +3 words, so it was not chased further — but *"we emit glyphs that make no
  ink"* is worth someone's attention on its own, independent of its size, because it inflates
  the word gate for free wherever it happens.

## Instrument controls

Three, because two of this round's conclusions would have been wrong without them.

1. **Banked reference against a live one.** `batch-check.sh`, re-rendering the reference through
   `soffice`, returns this document at `26/26  2118/2010  words`, raw `2306/2199`. The banked
   PDF returns the same figures to the digit. The ceiling file's warning that at least four
   documents render non-deterministically does not bite here.
2. **The text layer cannot answer a question about paint.** `pdftotext` says the reference draws
   no month labels on page 16. It draws 120 outlined glyphs of them. Had this round stopped at
   the extraction, page 16 would have been filed as +38 of our own surplus — the exact inversion
   the brief warned about.
3. **A downscaled raster cannot answer a question about the text layer.** The first page-16
   reviewer reported the month labels present in *both* halves, which is true of the ink and
   false of the text layer, and its report also asserted several small differences it could not
   actually resolve. Two reviewers reading page 6 and the crop were reliable on presence and
   direction and unreliable on everything below glyph scale, exactly as `page-vision` says.

**The brief's "5 flagged pages" is a misreading of the table and should not be carried forward.**
`TODO.raster-ceiling.md` has one row for this document; the `5` in it is the *page number*
column. `dotnet/raster-ceiling-pages.tsv` confirms one row. The under-count is therefore larger
than the brief assumed: one flagged page against three ceiling pages.

## Sweeps

Reference half read from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/`, `SOURCE_DATE_EPOCH`
set, via `sweep-banked.sh` in this directory (`batch-check.sh`'s three checks and verdict rule
copied verbatim; only the source of the reference PDF differs).

| range | total | match | mismatch |
|---|---:|---:|---:|
| `slides/batch-001` … `007` | 68 | **66** | 2 — `solog_orientation_august_2019`, `architecture6` |
| `slides/batch-008` | 10 | **9** | 1 — this document |
| together | 78 | **75** | 3 |

Both prior failures are documented ceilings and neither is this round's. Nothing moved, which is
what a round with no code change should show.

## Tests

No code changed, so these are a baseline rather than a check. Counts, not colours:

| project | failed | passed | skipped | total |
|---|---:|---:|---:|---:|
| `XRay.Office.Containers.Tests` | 0 | 109 | 0 | 109 |
| `XRay.Office.Core.Tests` | 0 | 332 | 0 | 332 |
| `XRay.Office.Fidelity.Tests` | **30** | 520 | **0** | **550** |
| `XRay.Office.Markup.Tests` | 0 | 259 | 0 | 259 |
| `XRay.Office.OpenDocument.Tests` | 0 | 125 | 0 | 125 |
| `XRay.Office.Presentations.Tests` | 0 | 679 | 0 | 679 |
| `XRay.Office.Rendering.Tests` | 0 | 150 | 1 | 151 |
| `XRay.Office.Spreadsheets.Tests` | 0 | 762 | 0 | 762 |
| `XRay.Office.Text.Tests` | 0 | 349 | 0 | 349 |
| `XRay.Office.Vector.Tests` | 0 | 295 | 0 | 295 |
| `XRay.Office.WordProcessing.Tests` | 0 | 819 | 0 | 819 |
| **total** | **30** | **4400** | **1** | **4430** |

`XRay.Office.Fidelity.Tests` reproduces the briefed baseline exactly — 30 failed of 550, 0 skipped.
Run per project and totalled by hand, per `CLAUDE.md`; the one skip is in
`XRay.Office.Rendering.Tests` and is not a `soffice` skip (Fidelity reports 0 skipped, which is
the number that matters for whether the reference was actually exercised).

## Prediction, scored

`prediction.md`, committed at `489be9b9eee` before our side was rendered.

| # | claim | outcome |
|---|---|---|
| 1 | pages 5 and 6 both raster ceiling, combined raw 80–95, alnum 70–85 | **hit** — raw +88 gross, alnum +86 gross; net alnum +74 |
| 2 | pages 8 and 9 also this class, +20 to +60 | **miss, and the useful kind.** We draw the 859×234 object on both pages ourselves, so condition 2 fails and they are not the class. Page 9 is exact; page 8 goes the *other* way at −7 and is three real defects of ours. The stated alternative — "we draw the same raster and these pages are neutral" — is what happened, plus a defect underneath it |
| 3 | page 16 is not raster; residue +20 to +50 and a genuine candidate defect | **half hit.** "Not raster" is right and the size is right (+38). "A genuine defect of ours" is **wrong**: +59 of it is a ceiling of a kind not previously recorded, and only −21 is ours |
| 4 | split 75–95 ceiling, 15–35 residue | **miss.** Ceiling is 145, i.e. *more than the whole gap*; our residue is −40, negative. The prediction assumed the residue had the same sign as the gap. It does not, and that assumption is what made items 3 and 4 wrong together |
| 5 | not the `words/batch-008` ligature shape | **hit** — no ligature or tracking involvement anywhere; the mechanism is unrelated |
| 6 | no code change | **hit** |
| 7 | excusing the ceiling puts it inside the band, contradicting the file's worked example | **hit** — 1973 against 2010, −37 against ±43.2. The file's sentence was written when only page 5 was excused and is correct on its own terms |
| baselines | Fidelity 30/550; 001–007 at 66/68 with those two names; batch-008 4 of 5 | **hit, hit, and wrong in a harmless way** — batch-008 holds ten documents, not five; 9 of 10 match |

**Six of nine, and the two real misses share one root**: I assumed the 110-word gap was made of
ceiling plus a same-signed residue, so I predicted a residue smaller than the gap and looked for
it in the pages the gap was on. The gap is a *difference of two larger numbers of opposite sign*.
The generalisable form: **a word-gate delta is a net, and netting hides both terms.** Split every
page's delta into ours-only and reference-only tokens before attributing any of it — the same
discipline `TODO.raster-ceiling.md` already demands for a document's ink, applied to its words.

## What was added to `TODO.raster-ceiling.md`

- pages 6 and 16 of this document, in a new under-the-bar table, with gross and net figures;
- the compounding under-count: condition 3 is netted, so the flag is suppressed on pages that
  are both a ceiling and a defect, with the proposed `ours-only` fix;
- a fifth shape, *the reference outlines its glyphs*, with the page-16 measurement, the
  within-document control, the census result over 78 documents, and the census script;
- a correction to the page-6 worked example, whose 24.4% is now 18.9% net / 26.0% gross;
- this document's three real defects, so the next reader does not file them as ceiling too.
