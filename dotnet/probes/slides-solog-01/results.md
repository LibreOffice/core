# slides-solog-01 — the one slides/001–006 mismatch, and what was actually wrong with the page

Subject: `slides/batch-004/pptx/solog_orientation_august_2019.pptx`, briefed as the single
`words` verdict in slides batches 001–006 (57 of 58 match), at `pages 15/15  words 670/685
fonts 5/6  unembedded 0`.

Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/` (banked, LibreOffice 26.2.4.2
620(Build:2), `fc-match "DejaVu Sans"` → `DejaVuSans.ttf` verified at the start of the round).
Ours from `dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli` in
`/c/sandbox/workdir/wt-slides-solog`, with `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every
render. Word counts are the corrected metric of `dotnet/probes/gate-01/results.md` — tokens
carrying at least one Unicode letter or digit.

Prediction committed as `6fe876ce02c` before the font measurements; scored in §8.

---

## 0. Headline

**The 15-word deficit is not a defect. Not one word is missing.** All fifteen tokens are
`pdftotext`'s reading of the *reference's* own output, and the two renderings' extractable
characters agree exactly. The gate row cannot be won and should be recorded as a ceiling.

**The document's real defects are elsewhere, and the round found them by looking at the page.**
Three blind reviewers, given one labelled image each and forbidden to read anything, all ranked
first a defect that appears nowhere in the brief: **hyperlink runs drew undecorated**. A second,
found the same way, was two of the three loudest differences on page 5 turning out to be **one**
bug in placeholder inheritance. Both are fixed here.

| | reach over the 163-document slides track |
|---|---:|
| hyperlink runs take the theme's `hlink` colour and a rule | **47 of 163 renderings** |
| a layout's untyped `p:ph` no longer becomes the master's footer-row placeholder | **6 of 163** |
| both, together | **50 of 163** |
| verdicts moved, in either direction | **0** |

---

## 1. Task 1 — the 15 missing words: they are the reference's tokenisation, all fifteen

Rendered both, extracted both with the same `pdftotext` 26.01.0, and compared **per page,
character by character, with whitespace removed**:

```
page   1 … 15      ref 4758 non-space characters      ours 4756
```

Every page's character multiset is identical except for **two `-`**, and both of those are
`pdftotext` eating a hyphen when it de-hyphenates one of *our* soft line breaks — the hyphen is
present in our PDF, at `xMax` of the first fragment, confirmed with `pdftotext -bbox`. So we draw
at least as much text as the reference does, everywhere.

The 15 tokens are the reference splitting its own words:

| where | reference tokens | ours | Δ |
|---|---|---|--:|
| `MIAMI` in the footer lock-up, pages 1, 2, 3, 15 | `M` `IAM` `I` | `MIAMI` | **8** |
| `dtpoole@miami.edu`, page 3 | `dtpoole@m` `iami.edu` | one | 1 |
| `http://bulletin.miami.edu/`, page 4 | `h` `ttp://…` | one | 1 |
| two wrapped URLs, page 5 | 2 + 3 | 1 + 1 | 3 |
| wrapped URL, page 8 | 2 | 1 | 1 |
| wrapped URL, page 9 | 2 | 1 | 1 |
| | | | **15** |

Two distinct mechanisms, and both are worth naming because neither is ours to fix.

**Eight of the fifteen are per-character positioning.** The reference draws
`UNIVERSITY OF MIAMI` as *"19 glyphs in 18 show(s)"* — LibreOffice writes one show operator per
character on printer-metric text — and the rounded advances leave a 1.26 pt gap after each `M`,
which is wide enough for `pdftotext` to insert a space. The two runs occupy 171.20–213.43 pt
against our 171.09–214.26: the ink is in the same place to under a point. We draw it as one show
and score one token.

**Seven of the fifteen are a line-breaking difference inside an over-long word.** Where a URL is
too long for the line, LibreOffice fills the current line and breaks *mid-token*; we move the
whole token to a new line and break it there. On page 8 the reference leaves
`(https://www.uresearch.miami.edu/ur` on the line that ends `Interest.` and continues
`esearch-services/…` at the margin; we put the whole `(` and URL on the next line. This is a real
fidelity difference — recorded in §6 as an open finding — but it moves no words, it only moves
where `pdftotext` decides one token ends.

**Consequence for the scoreboard.** No change to X-Ray.Office can manufacture those 15 tokens: they
exist because the reference's PDF says something ours does not need to say. The row belongs with
`TODO.raster-ceiling.md`'s 37 as a documented ceiling rather than as a failure. Re-running the
gate after both fixes confirms it: `15/15 pages, 670/685 words, verdict words`, unchanged.

---

## 2. Task 3 — the blind readings, which is where the round actually turned

Three fresh subagents, one page each (15, the `--worst`; 3; 5), each given only the labelled pair
image, forbidden to read source, documentation or this brief, and told nothing about any number.
They were asked to describe each half alone before comparing.

**All three independently ranked the same thing first: the hyperlinks.**

> *"The bottom half renders both email addresses in blue with an underline; the top half renders
> both in the same grey as the surrounding body text, with no underline. … This is the single
> largest visual difference."* — page 3

> *"Hyperlinks not blue/underlined in the top half — two runs of blue underlined text vanishing
> into black is the single loudest change; a casual reader immediately notices 'the links aren't
> links.'"* — page 15

The page-5 reviewer went further and, without being able to grep anything, separated the two
defects on that page correctly:

> *"Whether the top's grey and its missing link colour are one defect or two. … The black colour
> of bullet 1's URL in the top is the only evidence, and it points to them being two independent
> problems."*

They are two, and the reviewer's reasoning was right. Its ranked #1 and #2 on that page — *"more
than half the content is washed out to a light grey"* and *"ragged-left bullet text with a big
gap after the dot looks plainly broken"* — turned out to be **one** bug with two symptoms (§4).

Things they saw that the round has not fixed are in §6. The one worth pulling forward: the page-15
reviewer reported the reference's footer tagline as *"clearly italic/slanted"* and ours as
*"upright"*. Checked afterwards against the content stream, the reference draws that run under
`1 0 0.3462535606 1 … Tm` — a **synthesised oblique**, because the face it resolved to has no
italic on this machine — and we draw it with no skew at all. A blind reading and an independent
measurement landing on the same mechanism is what makes that evidence.

---

## 3. Fix 1 — a linked run takes the theme's `hlink` colour and an underline

`src/XRay.Office.Presentations/Ooxml/PptxTextBody.cs`.

The target was already read and handed to extraction as `ContentRun.HyperlinkTarget`; nothing
decorated the run. The rule is `oox/source/drawingml/textrun.cxx:145-170`:

```cpp
if (!maTextCharacterProperties.maHyperlinkPropertyMap.hasProperty(PROP_CharColor))
    aTextCharacterProps.maFillProperties.maFillColor.setSchemeClr(XML_hlink);
aTextCharacterProps.maFillProperties.moFillType = XML_solidFill;
if (!maTextCharacterProperties.moUnderline.has_value())
    aTextCharacterProps.moUnderline = XML_sng;
```

Three details that each cost a measurement:

**The scheme slot is swapped; the transform chain over it survives.** `Color::setSchemeClr`
assigns only `meMode` and `mnC1` (`color.cxx:405-413`). This deck's `slideLayout1.xml` tints its
subtitle placeholder's `tx1` to 75%, drawing the body at `#8B8B8B` — and the reference draws page
3's three `mailto:` runs at **`#8B8BFF`**, not at the theme's flat `#0000FF`. Resolving `hlink`
on its own is right on the deck's other five linked pages and wrong on that one.

**A run's own literal colour loses.** Predicted the other way round in `prediction.md` and
refuted by the binary. `mnC1` is exactly where `setSrgbClr` put the literal, so reassigning the
slot destroys it. Measured rather than argued:
`slides/batch-003/pptx/ROK-PI Climate Bulletin - Edition 2017-06.pptx` states
`<a:srgbClr val="C00000"/>` on the linked run reading `clikp.sprep.org`, its theme's `hlink` is
`0563C1`, and sampling the reference's own page 1 at 150 dpi over that word's `pdftotext -bbox`
rectangle gives **239 pixels of `#0563C1` and none of `#C00000`**.

**Both guards read the run's *own* `a:rPr`, not the merged chain.** `textrun.cxx` tests
`maTextCharacterProperties`, which is the run's properties before `assignUsed` merges the
defaults. So an inherited `u="none"` — which is what LibreOffice's own PPTX export writes on
every run — does *not* refuse the rule, and only the run's own `@u` can. Getting this backwards
would have been silent and would have lost every link rule on every LibreOffice-round-tripped
deck.

The `a:extLst` under an `a:hlinkClick` is PowerPoint's "use the text's own colour" extension;
`hyperlinkcontext.cxx:166-169` turns it into the `PROP_CharColor` the guard tests, so it refuses
the colour and keeps the rule.

**Result on the subject document:** the set of text fill colours now agrees with the reference's
on **all fifteen pages**, `#8B8BFF` on page 3 included. Before, we emitted no link colour on any
page.

---

## 4. Fix 2 — a layout's untyped `p:ph` must not take the master's type for that index

`src/XRay.Office.Presentations/Ooxml/PptxTextStyles.cs`, one argument.

`PptxPlaceholder.Read` gives an untyped `p:ph` the type of whatever placeholder shares its `idx`
one level up. **That rule belongs to a slide's placeholder alone.** LibreOffice reaches it
through `mpSlidePersistPtr->getMasterPersist()` (`pptshapecontext.cxx:68,82-90`), and a layout has
no master persist to get: the layout fragment is imported *into the master's own* `SlidePersist`
— `LayoutFragmentHandler(rFilter, aLayoutFragmentPath, pMasterPersistPtr)`
(`presentationfragmenthandler.cxx:287`), the constructor taking that argument straight into its
`mpSlidePersistPtr`. Only slides and notes ever get a `setMasterPersist`, at `:614` and `:643`.
So the branch is never entered while a layout's shapes are read, and a layout's bare
`<p:ph idx="4"/>` keeps the default `obj`.

We passed the master, so it did not. On this deck:

| | |
|---|---|
| slide 5's right-hand content box | `<p:ph sz="quarter" idx="4"/>` |
| `slideLayout5.xml` at index 4 | untyped |
| `slideMaster1.xml` at index 4 | **`<p:ph type="sldNum" …/>`**, `sz="1200" algn="r"`, `tx1` at 75% tint |

Five bulleted paragraphs therefore drew **flush right in `#8B8B8B`** where the reference sets
them flush left in black — the blind reviewer's first and second findings on that page, one bug.

The collision needs three coincidences (slide untyped at *N*, layout untyped at *N*, master's *N*
being `dt`/`ftr`/`sldNum`), which is why it is rare and why it is severe when it fires: those are
exactly the three types no content box should ever inherit from.

**Independent confirmation on a document this round did not diagnose.**
`diapo_6_fees_vous_etes_poctdoct_exterieur_a_luniv_v2_0_1.pptx`, one page:

```
reference   black ×15, red ×1
before      black ×2,  #8B8B8B ×24, red ×3
after       black ×26,             red ×3
```

---

## 5. Reach, measured by rendering the track three times and diffing

163 documents, `SOURCE_DATE_EPOCH=1700000000` on every render, `/CreationDate` masked (with the
epoch set it is equal anyway; the mask is belt and braces).

| | renderings changed |
|---|---:|
| hyperlink decoration alone | **47 of 163** |
| layout placeholder type alone | **6 of 163** |
| both | **50 of 163** (3 documents changed by both) |

**Attribution is exact, not inferred.** Two censuses were built *before* the diff — decks with an
`a:hlinkClick` on a run that draws text on a slide (49), and decks where a slide's untyped `p:ph`
index is untyped on its layout *and* is a `dt`/`ftr`/`sldNum` on its master (8). Every one of the
50 changed renderings is in the union; **no changed rendering is unexplained**, and three
predicted documents did not change because the properties happened to agree anyway. The
placeholder census is a resolved one — it follows the slide's own relationship to its layout and
the layout's to its master — which is why it came out at 8 and measured 6, rather than at the
fortyfold overshoot a grep would have produced.

Every one of the 50 is a `.pptx`; no `.ppt` moved, which is the expected shape since both fixes
are on the OOXML path.

**Verdicts.** Scoring all 163 of our renderings against the banked references on the gate's own
three columns:

```
before  144 / 163 match
after   144 / 163 match
verdicts moved: 0
```

144/163 is the figure `gate-01/results.md` records for this metric, so the scorer agrees with the
canonical scoreboard.

**One rendering shrank by 15 245 bytes** — `REDAC Briefing_SSIT_CARA_08132014.ppt.pptx` — which
looks like lost content and is not: same 14 pages, same 462 words, and it now embeds four faces
instead of five. The one it dropped is `LiberationSans-BoldItalic`, which the reference does not
embed either. It moved toward the reference.

---

## 6. Task 2 — the two font findings, and what is left open

### 6.1 `DejaVuSans-Bold` versus ours — **diagnosed, already a recorded defect, not fixed**

The brief's table said "we embed `LiberationSans-Bold` where the reference embeds
`DejaVuSans-Bold`", and treated the two as one row. They are unrelated.

**The reference's `DejaVuSans-Bold` is `Century Schoolbook`.** It carries exactly sixteen text
records — the footer lock-up (`UNIVERSITY OF MIAMI`, `COLLEGE of`, `ENGINEERING`) and the tagline
— on pages 1, 2, 3 and 15, and every one of those runs is
`<a:latin typeface="Century Schoolbook" …/>` with `b="1"`. **We resolve the same family to
`DejaVuSerif-Bold`**, not to Liberation Sans Bold.

Measured, not reasoned: `fc-match "Century Schoolbook"` → `DejaVuSans.ttf`,
`fc-match "Century Schoolbook:bold"` → `DejaVuSans-Bold.ttf`. LibreOffice's `VCL.xcu` chain for
`centuryschoolbook` names twenty-one faces and not one of them is installed here, so
`FcPreMatchSubstitution` answers first with fontconfig's default — which is DejaVu **Sans**, not
a serif. Our `FontSubstitutions.ClassOf` files the family as roman from `VCL.xcu`'s own
`FontType` and lands on DejaVu Serif.

This is **already recorded**: `dotnet/TODO.batches.md:2361` lists `Century Schoolbook` by name
among the families where "we give DejaVu Serif / Liberation Serif, LibreOffice gives DejaVu
Sans", with the fix stated as *"needs `ClassOf` replaced by fontconfig's own classification —
`45-latin.conf` files these as sans where `VCL.xcu`'s `FontType` says roman"*. Deliberately not
attempted here: it reaches the words track as well as slides and is a larger change than either
fix in this round. What this round adds is the confirmation that it is the seat of this
document's `fonts 5/6`, and the visible consequence — the reference's footer lock-up wraps
`COLLEGE of` / `ENGINEERING` onto two lines because DejaVu Sans is wider, and ours does not.

**The `LiberationSans-Bold` in our output is not this at all.** It is three glyphs: the `-` bullet
of three level-3 paragraphs on page 8. The reference draws those same three in
`LiberationSans` *regular*. So it is a separate, smaller finding — **we draw a symbol bullet at
the first run's weight and the reference does not** (the paragraphs are `b="1"`). Left open.

### 6.2 `OpenSymbol` — **diagnosed and localised, not fixed**

The reference's `OpenSymbol` is **one glyph**, at 28.01 pt at (412.33, 252.65) on page 9. Its
source is one run on slide 9:

```xml
<a:r><a:rPr lang="en-US" dirty="0" smtClean="0">
       <a:sym typeface="Wingdings" pitchFamily="2" charset="2"/>
     </a:rPr>
     <a:t>&#xF0E0;</a:t></a:r>
```

A Wingdings arrow in the Private Use Area, mid-sentence. LibreOffice switches the run's face to
the `a:sym` typeface with `SYMBOL_CHARSET` (`textrun.cxx:96-120`), Wingdings is not installed, VCL
substitutes OpenSymbol and recodes the slot. We draw the raw `U+F0E0` in the paragraph's own face.

**The brief's worry that the symbol recode "may have been done on the words path only" is
refuted.** `XRay.Office.Presentations` has it: `SlideTextLayout.Recoded` and `PptxTextBody.Marked`
implement exactly this rule, including the subtlety that the trigger is the face being *absent*
rather than the request resolving to OpenSymbol. It is wired **for bullets only**.
`a:rPr/a:sym` is read nowhere in `XRay.Office.Presentations` — the only `"sym"` readers in the tree
are two in `XRay.Office.WordProcessing`.

**Reach, resolved rather than declared.** 22 of 163 decks mention `<a:sym>` somewhere; that is the
declaration count and it is not the answer. Counting only runs on *slide* parts whose `a:t`
actually holds a `U+F000`–`U+F0FF` character: **13 documents, 116 glyphs** — 59 on `passiv.pptx`,
24 on `_1___Opatrny_Ales_…pptx`, one here. Every such run carries an `a:sym`, so the signal is
clean.

**Why it is not fixed here, stated plainly.** The bullet path escapes the problem because a
`SlideMarker` carries its own text, so `SlideTextLayout` can substitute both face and code point
after font resolution. A run does not: `EmitStretch` takes its characters from
`block.Measured.Text[run.Start..run.End]`, the paragraph's shared string, so a per-run character
substitution is not expressible without either (a) recoding in the reader, which cannot know
whether the symbol face is installed — the exact distinction `Recoded`'s remarks say was got
wrong once already — or (b) a normalisation pass over `SlideParagraph` at the point where fonts
are available. (b) is the right shape and is a self-contained ~40-line change; it was not
attempted because this round already ships two fixes to the same file pair and the project's
cascade rule makes a third unvalidated change the wrong trade.

### 6.3 Also open, from the blind readings

- **A synthesised oblique is not synthesised.** The reference draws the tagline under
  `1 0 0.34625 1 … Tm`; we draw it upright. The run is `i="1"`, and neither renderer has an
  italic face for what it resolved to, so LibreOffice skews the upright one and we do not.
- **An over-long word breaks at the margin, not on the next line.** §1's second mechanism.
  LibreOffice fills the line and breaks mid-token; we move the whole token down first.
- **A symbol bullet takes the first run's weight.** §6.1's tail.

---

## 7. Validation

**Order, which is the project's rule and was followed:**

```
slides/batch-004          TOTAL 10  MATCH  9  MISMATCH 1   (solog, as briefed)
slides/batch-00[1-6]      TOTAL 58  MATCH 57  MISMATCH 1
```

Identical to the briefed baseline — same count, same single row, nothing else moved in either
direction.

**Tests, run project by project and counted rather than read for colour:**

| project | passed | failed | skipped |
|---|---:|---:|---:|
| `XRay.Office.Core.Tests` | 313 | 0 | 0 |
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 |
| `XRay.Office.Text.Tests` | 289 | 0 | 0 |
| `XRay.Office.Vector.Tests` | 295 | 0 | 0 |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 |
| `XRay.Office.Rendering.Tests` | 148 | 0 | 1 |
| `XRay.Office.OpenDocument.Tests` | 125 | 0 | 0 |
| `XRay.Office.Presentations.Tests` | **646** | 0 | 0 |
| `XRay.Office.Spreadsheets.Tests` | 695 | 0 | 0 |
| `XRay.Office.WordProcessing.Tests` | 792 | 0 | 0 |
| `XRay.Office.Fidelity.Tests` | 519 | **31** | 0 |
| **total** | **4190** | **31** | **1** |

`--list-tests` discovers 646, 695, 792 and 550 for the four largest, and each ran its full
number, so none of these was a truncated run. The 31 fidelity failures are the pre-existing set:
the failing test *names* were captured with the fix in and again with both files stashed out, and
the two lists are **identical**, so none was added and none was accidentally fixed. Build is
0 warnings, 0 errors.

15 tests added, both files in `tests/XRay.Office.Presentations.Tests`:
`SlideHyperlinkDecorationTests` (11) and `PptxLayoutPlaceholderTypeTests` (4).

---

## 8. The prediction, scored

`prediction.md`, committed as `6fe876ce02c`. Item 0 was recorded as a result rather than a
prediction and is not scored.

| # | claim | conf. | outcome |
|---|---|---:|---|
| 1.1 | the `DejaVuSans-Bold` split is a real substitution, not a `/BaseFont` naming artefact | 0.6 | **right** — two genuinely different families, confirmed by `fc-match` |
| 1.2 | the requested family is an uninstalled MS face and both sides substitute differently | 0.55 | **right in shape, wrong in every particular.** Predicted Arial/Tahoma/Trebuchet/Verdana/Times; it is **Century Schoolbook**. And the sub-claim "if it is Arial we should be right" was answering a question the document does not ask |
| 1.3 | the slides path already has the symbol recode; the brief's worry is refuted | 0.9 | **right**, and the refutation is sharper than predicted — it is wired for bullets and `a:sym` is unread |
| 1.4 | the bullet reaching OpenSymbol is declared in a layout or master | 0.4 | **refuted.** It is not a bullet at all. It is an `a:sym` on an ordinary run on the slide itself |
| 2.1 | reach is 2–20 renderings | 0.55 | **refuted, and badly — 50.** The band came from the round's own prior that reach estimates overshoot; here the estimate undershot by 2.5×, because I sized it against the fixes I expected to make rather than the ones I made |
| 2.2 | zero verdicts move on the slides track | 0.7 | **right**, 144/163 before and after |
| 2.3 | solog stays a `words` failure whatever is fixed | 0.85 | **right**, and §1 says why it must |
| 3.1 | batches 001–006 stay at 57 of 58 | 0.75 | **right** |
| 4.1 | the reviewer names a bullet or glyph difference before a bold face | 0.5 | **refuted** — all three named hyperlinks first, and no reviewer mentioned a bullet class at all |
| 4.2 | the reviewer names a defect that is nowhere in this brief | 0.65 | **right**, and it is the round |

Eight scored, four refuted. Two are worth carrying forward.

**2.1 is the instructive one.** The project's standing lesson is that reach estimates come out
*high* because a grep counts declarations. Mine came out **low**, and for a different reason
entirely: I sized the band against the defect I had been briefed on — a face substitution and a
symbol glyph, both narrow — and the fixes that actually shipped came from looking at the page.
The lesson generalises as: *a reach estimate is only about the fix you predict you will make.*
Neither shipped fix existed when the number was written.

**4.1 is the argument for the method.** I predicted the reviewers would notice the things the
brief was about. All three noticed something the brief had never mentioned, and ranked it above
everything the brief did mention. Had this round chased only its two font findings it would have
shipped nothing visible and left a defect on 47 of 163 renderings in place.

---

## 9. What contradicts the brief

Recorded plainly, because four of the brief's statements did not survive measurement.

1. **"15 words short."** No words are short. Ours and the reference extract the same characters
   on all 15 pages, ±2 characters that `pdftotext` itself removes from *our* output.
2. **"we embed LiberationSans-Bold"** as the counterpart of the reference's `DejaVuSans-Bold`.
   Those are two unrelated things. Our `LiberationSans-Bold` is three bullet hyphens on page 8;
   the Century Schoolbook runs resolve to `DejaVuSerif-Bold` in our output, and the brief's
   framing of finding 1 as "a bold run resolves to Liberation Sans Bold" is not what happens.
3. **"the reference embeds OpenSymbol and we embed nothing like it … the symbol-font bullet
   class."** It is one glyph and it is not a bullet: it is an `a:sym` Wingdings arrow in the
   middle of a sentence. The bullet recode the brief suspected was missing is present and
   working.
4. **"the missing OpenSymbol is still a real fidelity defect"** — this one holds, and the brief
   was also right to insist the two findings be kept apart. They are not one defect; they are
   two, plus a third the brief did not have.

And one thing the brief was right about that decided the round: *"the gate is blind to most real
defects."* Every fix here is invisible to all three gate columns.
