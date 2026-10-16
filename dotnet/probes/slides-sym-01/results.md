# slides-sym-01 — `a:rPr/a:sym` recoded per run, and what measuring the reference changed about the rule

Subject: the finding left open as §6.2 of `dotnet/probes/slides-solog-01/results.md` — the slides
symbol recode is present and correct and is wired **for bullets only**, so a symbol character in
the middle of a sentence draws from whatever face the paragraph happens to be in.

Reference: `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides/` (banked, LibreOffice 26.2.4.2
620(Build:2)), reused throughout and never re-rendered. `fc-match "DejaVu Sans"` →
`DejaVuSans.ttf` and `fc-match Calibri` → `Carlito-Regular.ttf`, both verified at the start of the
round. Ours from
`/c/sandbox/workdir/wt-slides-sym/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli`
with `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every render, so the before/after diff is byte
for byte with nothing masked.

Prediction committed as `153f42c2b9d` before any measurement of the fix; scored in §8.

---

## 0. Headline

**Wired, and it works: the arrow is drawn as an arrow, and `solog`'s fonts column goes 5/6 → 6/6.**

**But the round's own headline is that the rule it was briefed to implement is not the rule the
reference follows, and the difference was only visible by measuring the reference.** A first,
citation-faithful implementation recoded every private-use slot behind an `a:sym` naming one of
LibreOffice's fourteen recodeable faces. Checking those glyphs against the banked references
found **two documents where the reference does not recode at all** — and the reason is that
`a:sym/@charset` decides not *whether* to recode but *whether fontconfig is consulted*, which then
decides where the face lands. §3 has the mechanism and the measurements.

| | resolved over the 163-document slides track |
|---|---:|
| renderings changed, byte for byte | **11 of 163** |
| documents that gained a recoded glyph | **10** |
| characters recoded | **89** |
| glyph placements drawn from OpenSymbol that were not before | **91** |
| …of those, landing inside the page box | **87** |
| gate verdicts moved, in either direction | **0** |

**The brief's figure of 13 documents / 116 glyphs is a declaration count and it is the wrong one.**
It is exactly reproducible — §2 reproduces it to the glyph — but 27 of those 116 glyphs cannot
recode and do not, and the reference agrees on every one. §2 and §3.

---

## 1. What was wired

Four files, one of them new.

**`src/XRay.Office.Presentations/Ooxml/PptxTextBody.cs`** reads `a:rPr/a:sym` into a new
`SlideSymbolFont(Typeface, IsMicrosoftEncoded)` on `SlideTextRun`. It is read through the same
`First(own, defaults, …)` the size and the weight use, because
`TextCharacterProperties::assignUsed` takes `maSymbolFont` from any source that states one
(`textcharacterproperties.cxx:55`) — so a level's `a:defRPr` can carry it for every run beneath
it. The typeface is theme-resolved for the same reason `a:latin` is: `getFontData` puts the name
through `Theme::resolveFont` before using it (`textfont.cxx:80-85`).

**`src/XRay.Office.Presentations/Layout/SlideSymbolRuns.cs`** (new, 200 lines with its remarks, ~60
of code) is the normalisation pass the previous round scoped. It runs at the top of
`SlideTextLayout.Place` and `SlideTextLayout.Height` — before anything is measured, because an
OpenSymbol arrow and a `.notdef` box are not the same width and a line break decided on the wrong
one is decided wrongly.

**It splits runs; it does not reassign them.** `oox/source/drawingml/textrun.cxx:96-135` walks a
run's text in maximal stretches under the predicate `(ch & 0xff00) == 0xf000`, applies `a:sym` over
each private-use stretch and **resets the four font properties after every one**. Measured over the
track: **45 of the affected `a:t` values hold both kinds of character**, so a whole-run reassignment
would have set 45 sentences in dingbats. `16 - UTM - (NASA).pptx` is the extreme case — its `a:t`
is one arrow followed by three spaces.

**Every character offset survives.** A recode is one code point for one code point
(`ConvertChar::RecodeChar`), so the paragraph's `Text` keeps its length and a run only ever splits
at a boundary inside its own range. Nothing downstream sees an index it did not see before — which
is what makes this a rewrite of two fields rather than a rebuild of the paragraph, and why
`Block.ColourAt`, `Block.DecorationAt`, `MarkerReach` and the tab ruler all needed no change.

**The bullet path and the run path share the decision, as asked.** `SlideTextLayout.Recoded` now
calls `SlideSymbolRuns.Recodes(typeface, reference)` instead of restating the three-part guard;
the run path calls an overload of the same method. They genuinely can share: the guard is a
question about a face and a resolution, and neither path has anything to add to it. What the run
path adds is the *second* path into that guard, §3 — and that is an extra argument to a shared
rule rather than a second copy of it.

---

## 2. The declaration census reproduces the brief exactly, and then stops being the answer

Counting runs on `ppt/slides/slideN.xml` parts whose `a:t` holds a `U+F000`–`U+F0FF` character:

```
packages mentioning <a:sym> anywhere                     22     (the number a grep gives)
documents with such a run on a slide part                13     116 glyphs
of those glyphs, the run's own a:rPr carries an a:sym    116     (all of them)
```

**13 and 116, to the glyph.** The previous round's figure is exactly right *as a census*.

The four faces those 116 glyphs name, and what each is:

| face | glyphs | documents | LibreOffice recode table? |
|---|---:|---:|---|
| `Symbol` | 51 | 4 | yes — `AdobeSymbolTable` |
| `Wingdings 3` | 25 | 2 | yes — `WingDings3Table` |
| **`FontAwesome`** | **24** | **1** | **no** |
| `Wingdings` | 16 | 6 | yes — `WingDingsTable` |

`FontAwesome` is not a legacy symbol encoding and LibreOffice holds no table for it, so its 24
glyphs on `_1___Opatrny_Ales_United_Kingdom_business_opportunities_final.pptx` cannot recode.
**Confirmed against the reference rather than assumed: that document's banked reference PDF embeds
no OpenSymbol at all.** So the declaration census overstates the reach of any fix by one document
and 24 glyphs before anything else is considered.

---

## 3. The correction the reference forced, which is the round

The first implementation recoded whenever the face had a table and its own file was absent — the
bullet path's guard, applied to runs. That gave **12 changed renderings and 94 added glyph
placements**, and every one of the 12 was in the recodeable census, so it looked clean.

**It was not.** Checking the added glyphs against the banked references, position by position,
found two documents where the reference draws something else entirely:

| document | `a:sym` | our first attempt | the reference, at the same coordinates |
|---|---|---|---|
| `Stakeholders-v08052017 - v5.pptx` p8 | `<a:sym typeface="Wingdings"/>` | OpenSymbol ×2 | **`DejaVuSans`** at (175.9, 94.3) and (189.2, 29.1) |
| `16 - UTM - (NASA).pptx` p11 | `<a:sym typeface="Wingdings"/>` | OpenSymbol ×1 | **`DejaVuSans-Bold`**; the document's reference embeds no OpenSymbol anywhere |
| `Structural Testing.pptx` pp3–6, 26 | `<a:sym typeface="Symbol" charset="0"/>` | OpenSymbol ×5 | **`OpenSymbol` ×5**, within 0.3 pt of ours on four of the five |

The first reading of that table is "the charset decides", and it is wrong: `Structural Testing`
states `charset="0"` and recodes anyway. The rule that fits all three — and all thirteen documents
— is this:

> **`a:sym/@charset` decides whether fontconfig is consulted. Where the face lands then decides
> whether the slot is recoded.**

`TextFont::implGetFontData` reports symbol encoding as `mnCharset == WINDOWS_CHARSET_SYMBOL`, the
value 2, and nothing else (`textfont.cxx:87-94`); the default when the attribute is absent is
`WINDOWS_CHARSET_DEFAULT`, which is 1. `FcPreMatchSubstitution::FindFontSubstitute` then returns
false immediately for a symbol-encoded pattern (`vcl/unx/generic/font/fontsubst.cxx:100-104`), so
such a request skips fontconfig entirely and falls to `VCL.xcu`'s chain — which names `opensymbol`
for every face there is a table for. A request that is *not* symbol-encoded is answered by
fontconfig first, and fontconfig does not know the name was meant as a symbol font.

And one family survives that, by name. `fonts-opensymbol` ships
`/etc/fonts/conf.d/30-opensymbol.conf`, whose entire first rule appends `OpenSymbol` to any pattern
whose family is `Symbol`. Measured over all fourteen recodeable families on this machine, it is the
only one that reaches OpenSymbol without a symbol-encoded request:

```
fc-match Symbol        -> opens___.ttf: "OpenSymbol"
fc-match Wingdings     -> DejaVuSans.ttf: "DejaVu Sans"      (and the other twelve, likewise)
```

That is `SymbolFontRecode.IsAliasedToSubstitute`, and it is stated in the presentation layer's
guard rather than taught to the resolver **because our resolver cannot express the distinction**:
`SystemFontResolver` applies `VCL.xcu`'s chain whatever the request, so asking it for `Wingdings`
returns OpenSymbol on either path. Teaching it fontconfig's pre-match ordering for symbol faces
would move every family on every track and is a font-layer change; this round states the fact where
its two callers are and records the divergence rather than hiding it.

**One consequence worth naming.** On the non-recoding path we now leave the run in its own face
rather than switching it to the `a:sym` family, which is narrower than `textrun.cxx`. It is the
nearer of the two available behaviours: switching would ask *our* resolver for `Wingdings`, get
OpenSymbol, and draw the private-use slot as `.notdef` out of a face that does not hold it, where
leaving it alone sends it to glyph fallback — which is where it went before this round and is what
the reference's DejaVu Sans most resembles. A genuinely installed `a:sym` face is still switched
to.

---

## 4. Reach, resolved by rendering the track twice and diffing

163 documents rendered before and after, `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every
render, compared with `cmp` — byte for byte, nothing masked.

```
renderings changed:  11 of 163
```

| document | glyphs added | note |
|---|---:|---|
| `passiv.pptx` | 61 | 59 characters; two are drawn twice, being shadowed |
| `WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211017-171222.pptx` | 7 | |
| `Structural Testing.pptx` | 5 | `Symbol`, `charset="0"` — recodes via the fontconfig alias |
| `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 4 | its four further slots are on *notes* parts, which do not draw |
| `redac-nasops-201503-RIRP-portfolio-update.pptx` | 4 | `Symbol`, no charset |
| `Sylva%20introduction%20session.pptx` | 3 | |
| `redac-sas-201509-asisp-research.pptx` | 3 | |
| `PAL Block Intro 2023.pptx` | 2 | |
| `Snowbirds_High_Show.pptx` | 1 | |
| `solog_orientation_august_2019.pptx` | 1 | the subject |
| `16 - UTM - (NASA).pptx` | **0** | changed only in how its run is segmented — see below |
| | **91** | |

**Attribution is exact and nothing is unexplained.** Every one of the 11 changed renderings is in
the recodeable census; the two census documents that did *not* change are
`_1___Opatrny_Ales_…pptx` (FontAwesome, no table) and `Stakeholders-v08052017 - v5.pptx`
(Wingdings, not symbol-encoded, and its runs are pure private-use so not even the segmentation
moves). No `.ppt` moved, which is the expected shape: the binary path has no `a:sym` and its body
runs are not symbolised at all.

**`16 - UTM - (NASA).pptx` is worth its own line**, because it is the one document whose *rendering*
changed while its *ink* did not. Its `a:t` is an arrow followed by three spaces, so the split at
the private-use boundary emits two shows where there was one. Same glyphs, same pen, same faces —
a real byte difference and a zero-glyph one. A reach measured only by "renderings changed" counts
it; a reach measured by glyphs does not. Both are reported above rather than one being chosen.

**Four of the 91 do not land on the page.** `passiv.pptx` page 9 draws two of its `Wingdings 3`
slots at y ≈ −74, in a shape that sits off the bottom edge, and each is drawn twice because the run
is shadowed. So 87 of the 91 new placements are inside the page box. That distinction only exists
because the count was taken from the rendered PDFs; no census would have produced it.

### Against the brief's 13 / 116

| | documents | glyphs |
|---|---:|---:|
| brief, declaration census | 13 | 116 |
| …less `FontAwesome`, which has no table and which the reference does not recode | 12 | 92 |
| …less `Wingdings` without `charset="2"`, which the reference does not recode | **10** | **89** |
| renderings that change at all (adds `16 - UTM`, segmentation only) | 11 | — |

**The disagreement is real and the brief's figure is the one that is wrong**, in the sense that
matters: it counts declarations of a symbol slot, not recodes. Both of the deductions were checked
against the reference rather than argued — the FontAwesome document's reference embeds no
OpenSymbol, and the two Wingdings documents' references draw DejaVu Sans at the exact coordinates
concerned. This is the same lesson the previous round recorded from the other end, where a
declaration census would have claimed 22.

---

## 5. The subject document

Scored against the banked reference on the gate's own columns:

```
before   15/15 pages   670/685 words   fonts 5/6   unembedded 0   verdict words
after    15/15 pages   670/685 words   fonts 6/6   unembedded 0   verdict words
```

**The reference embeds OpenSymbol and we now do too.** The `words` verdict is unchanged and must
be: `slides-solog-01` §1 established that all fifteen tokens are the reference's own tokenisation
of text we draw identically, and the row is a documented ceiling.

**The arrow is drawn as an arrow, measured as ink and not as text.** Page 9, from the PDFs' own
operators:

```
reference   (412.33, 252.65)  28.01 pt  GAAAAA+OpenSymbol   1 glyph   next run resumes at 440.28
before      (412.88, 263.78)  28.01 pt  BAAAAA+DejaVuSerif  1 glyph   next run resumes at 429.69
after       (412.88, 263.78)  28.01 pt  FAAAAA+OpenSymbol   1 glyph   next run resumes at 440.89
```

The advance is **28.01 pt against the reference's 27.95** — 0.06 pt — where before it was 16.81.
Cropping both at 400 dpi over the glyph's own box gives 6660 dark pixels against the reference's
6525, a 2% difference consistent with antialiasing and the 0.55 pt horizontal offset. The 11.13 pt
vertical offset between the two is a pre-existing line-break difference higher up the page — the
over-long-URL finding of `slides-solog-01` §6.3 — and is present in the before rendering
identically.

---

## 6. The gate, over all 163

Scored against the banked references with `probes/slides-sym-01/score-against-banked.py`, which
applies `batch-check.sh`'s three checks in its order and with its rules.

```
before      144 / 163 match     19 words
after       144 / 163 match     19 words
verdicts moved: 0
```

**Verdicts are identical document for document**, not merely equal in total — `diff` over the
name-and-verdict columns is empty. 144/163 is the figure `gate-01/results.md` records for this
metric and the one `slides-solog-01` measured, so the scorer agrees with the canonical scoreboard.

**Zero verdict movement was expected and is what happened, and it is worth saying plainly rather
than framing as a null result.** The gate counts pages, letter-or-digit tokens and unembedded
fonts. A symbol glyph carries no letter and no digit; a recode changes which face a glyph comes out
of and never how many pages there are; and both faces were embedded either way. There is no column
in which 89 recoded glyphs could show up. The value here is fidelity the gate cannot see, and the
only instruments that saw it were the reference's own content stream and a blind reader.

## 7. Validation

**Order, which is the project's rule and was followed:**

```
slides/batch-004          TOTAL 10  MATCH  9  MISMATCH 1   (solog, as briefed)
slides/batch-00[1-6]      TOTAL 58  MATCH 57  MISMATCH 1
```

Identical to the briefed baseline — same count, same single row, and `solog`'s fonts column now
reads 6/6 inside an otherwise unchanged row.

**Tests, run project by project, counted rather than read for colour, and checked against
`--list-tests` for the four largest:**

| project | passed | failed | skipped | discovered |
|---|---:|---:|---:|---:|
| `XRay.Office.Core.Tests` | 313 | 0 | 0 | |
| `XRay.Office.Containers.Tests` | 109 | 0 | 0 | |
| `XRay.Office.Text.Tests` | 310 | 0 | 0 | |
| `XRay.Office.Vector.Tests` | 295 | 0 | 0 | |
| `XRay.Office.Markup.Tests` | 259 | 0 | 0 | |
| `XRay.Office.Rendering.Tests` | 150 | 0 | 1 | |
| `XRay.Office.OpenDocument.Tests` | 125 | 0 | 0 | |
| `XRay.Office.Presentations.Tests` | **667** | 0 | 0 | 667 |
| `XRay.Office.Spreadsheets.Tests` | 758 | 0 | 0 | 758 |
| `XRay.Office.WordProcessing.Tests` | 818 | 0 | 0 | 818 |
| `XRay.Office.Fidelity.Tests` | 520 | **30** | 0 | 550 |
| **total** | **4324** | **30** | **1** | |

**The Fidelity baseline was established before any edit and is 30 of 550**, exactly as briefed —
`Failed: 30, Passed: 520, Skipped: 0, Total: 550`. It was then established a second time *after*
the fix by copying the five changed files aside, reverting them, rebuilding and re-running (never
`git stash`, which is repository-global and has already crossed two agents here today): 30 again,
and the failing test **names** are identical, 15 distinct methods covering 30 theory cases,
`diff` empty. So none was added and none was accidentally fixed. The files were then restored and
the restored binary re-renders the subject document byte-identically to the one every figure above
was measured on.

Build is 0 warnings, 0 errors.

**21 tests added**, `tests/XRay.Office.Presentations.Tests/SlideSymbolRunTests.cs`. Four assert the
reader, including that an `a:sym` on a level default reaches the run and that only `charset="2"` is
symbol-encoded; the rest assert the layout — the recode, the split, that ordinary characters in the
same run keep the paragraph's face, that a face with no table is untouched, that a Wingdings slot
that is not symbol-encoded is *not* recoded while an Adobe Symbol one is, and that a split run
keeps its underline on every piece.

---

## 8. The blind reading

One fresh subagent, given two labelled images and nothing else — the whole of page 9 stacked, and a
400 dpi crop of the line the arrow is on — forbidden to read source, documentation or this brief,
and told nothing about what had changed or what to look for. It was asked to describe each half
alone, then rank the differences by how obvious they would be to a casual reader.

**It described the arrow, in both halves, as an arrow, and ranked it last.**

> *"a right-pointing arrow — one straight horizontal shaft of even, fairly thin weight, terminating
> at the right in an open arrowhead made of two straight diagonal strokes meeting at a point … the
> same construction … the same horizontal position within the line, the same vertical alignment
> against the letters, and, as far as I can judge by eye, the same length and the same head size."*

and, correctly disciplined about what an image can establish:

> *"I am reporting this as 'no visible difference', not as 'identical' — the crop cannot certify
> that the two halves reached that mark by the same route (same font, same codepoint, same
> fallback), only that they landed in the same place looking the same."*

Its ranked #1 was something else entirely and is a known open finding: the reference starts the
long URL inline and breaks it mid-token at the margin, where we move the whole token to the next
line — `slides-solog-01` §6.3's second item, independently found.

**One of its observations is refuted by measurement, and recording that is the point of the
method.** It reported the URL run as *"set distinctly smaller"* in ours. It is not: both draw that
run at **14.00 pt**, from the PDFs' own operators. The skill's own list says relative size is
reliable from an image and exact values are not — this is a case where a run that starts in a
different place, surrounded by different neighbours, read as a different size. The observation was
right about *where* and wrong about *how big*, which is exactly the split the skill predicts.

Before handing the images over, the arrow's presence was confirmed in the PDF's own operators
(`pdf-ops.py dump`, §5) rather than in a raster, per the compositor warning. The full-page
composite came back *"shown at 86% of composed"*, which is precisely the downscale that once ate a
one-pixel rule — so the glyph claim never rested on it.

---

## 9. The prediction, scored

`prediction.md`, committed as `153f42c2b9d`. R0–R3 were recorded there as results rather than
predictions and are not scored.

| # | claim | conf. | outcome |
|---|---|---:|---|
| 1.1 | expressible as a normalisation at the two `SlideTextLayout` entry points, no offset downstream changes | 0.85 | **right** — and the reason is sharper than predicted: the recode being one code point for one makes the run *split* free as well |
| 1.2 | the bullet path and the run path can share the recode decision | 0.8 | **right**, and they do — but the run path needed a second entry into the shared guard, which the prediction did not foresee |
| 2.1 | the resolved reach will disagree with 13/116 and the brief's number is the wrong one | 0.8 | **right**, and by more than predicted: 10 documents and 89 glyphs, not 12 and 92 |
| 2.2 | the recodeable subset is 12 documents, 92 glyphs | 0.7 | **right as arithmetic, and not the answer.** It was the reach of the *wrong rule*; two of those documents are ones the reference does not recode |
| 2.3 | 11–13 renderings change | 0.55 | **right**, 11 — and right for a reason the band did not contain: it lands there because `16 - UTM` changes without gaining a glyph while `Stakeholders` gains nothing and does not change |
| 2.4 | no `.ppt` rendering moves | 0.85 | **right**, none did |
| 3.1 | zero verdict movement on the slides gate | 0.9 | **right**, 144/163 before and after, identical document for document |
| 3.2 | batch-004 stays 9/10 and batches 001–006 stay 57/58 | 0.85 | **right** |
| 4.1 | `solog` embeds OpenSymbol, fonts 5/6 → 6/6 | 0.7 | **right** |
| 4.2 | the other 11 recodeable documents' references also embed OpenSymbol | 0.75 | **refuted, and it is the round.** `16 - UTM - (NASA).pptx`'s reference embeds none, which is what exposed §3 |
| 5.1 | Fidelity stays at exactly 30 failed with unchanged names | 0.85 | **right** |
| 5.2 | no other test project moves; 0 warnings | 0.9 | **right** |
| 6.1 | a blind reviewer reports the arrow present in both halves and does not rank it first | 0.6 | **right on both halves**, and it ranked a known open finding first |

Eleven scored right, one refuted, one right-but-beside-the-point. Two are worth carrying forward.

**4.2 is the one that earned its place.** It was written as a corroboration — *"the same mechanism
is visible corpus-wide"* — and its whole value turned out to be the single document that broke it.
Had the round only checked `solog`, which was the brief's request, the wrong rule would have
shipped with a passing gate, a passing regression sweep, 30 unchanged Fidelity failures, and three
glyphs quietly wrong on two documents nothing would have looked at again. **A corroborating
prediction is worth committing precisely because the interesting outcome is the one that refutes
it.**

**2.2 is a subtler version of the standing lesson about reach.** The number was right; it was a
correct count of the wrong set, because it was computed from the fix I intended rather than the fix
the reference required. `slides-solog-01` §8 recorded that *"a reach estimate is only about the fix
you predict you will make"*. This round adds: it is also only about the *rule* you predict you will
implement, and a census cannot tell you the rule is wrong — only the reference can.

---

## 10. What contradicts the brief

1. **"Resolved reach: 13 documents, 116 glyphs."** Reproduced exactly as a census (§2) and it is
   not the reach. Resolved by rendering and diffing: **10 documents gain glyphs, 89 characters
   recode, 91 placements appear, 11 renderings change.** The gap is one FontAwesome document
   (24 glyphs, no recode table) and two Wingdings documents (3 glyphs, not symbol-encoded), and the
   reference was checked on all three.

2. **"Reusing the existing bullet recode rather than a second implementation of it."** Done, and
   the brief was right that they can share — but sharing was not sufficient. The bullet path's
   guard is a *subset* of the run rule, not the whole of it, and applying it unchanged is what
   produced the three wrong glyphs of §3. Two callers of one method, one of them passing an extra
   fact.

3. **"Roughly 40 lines."** The mechanism is about that; the file is 200 lines because most of it is
   the record of §3, which was not in the previous round's scope and could not have been.

4. **"Expect zero verdict movement."** Right, and stated plainly in §6.

5. Not a contradiction but worth recording against the previous round's §6.2: its claim that a
   per-run substitution *"is not expressible without a normalisation pass over `SlideParagraph`"*
   is exactly correct, and the pass it described is what shipped.

---

## 11. Left open

- **`a:sym` on the binary PowerPoint path.** `.ppt` has no `a:sym`, but its character runs can name
  a symbol-charset face from the font table and its *body* runs are not symbolised at all — only
  its bullets are (`PptTextReader.Symbolised` is called from the marker path and nowhere else).
  `SlideTextRun.SymbolFont` is format-agnostic, so the reader is where the work would be. No corpus
  measurement of how much it is worth was taken.
- **`a:sym` in an ODP.** Not looked at.
- **Our resolver cannot express fontconfig's pre-match ordering for symbol faces**, §3. The
  presentation layer now states the one fact it needs; the general defect is untouched and reaches
  the words track.
- **The four off-page glyphs on `passiv.pptx` page 9.** A shape whose text sits at y ≈ −74. Whether
  the shape is misplaced or genuinely off-slide was not investigated; the reference was not
  compared there.
- The three findings `slides-solog-01` §6.3 left open are all still open, and the blind reader
  independently ranked one of them first.
