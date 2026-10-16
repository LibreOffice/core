# sheets-vision-02 — four failing pages, read blind by agents who had never seen them

Round subject: use vision on the sheets track's current failures, with the reading delegated
to fresh subagents rather than done by the agent that chose the pages.

**Environment.** LibreOffice 26.2.4.2; references are the banked
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/sheets/`. `fonts-dejavu-core` was **missing again**
at session start (`fc-match "DejaVu Sans"` → `wqy-zenhei.ttc`) despite being installed and
written up last session — reinstalled before any measurement, and recorded in
`MISSING_PACKAGES.md`. Every figure below is post-reinstall.

## Method, and why it is different from previous vision rounds

Previous rounds had the agent that picked the page also read it. That is not a control, and
the reason is measured rather than asserted: while calibrating the new `page-vision` skill I
read one page at 150 dpi, then 72, then 36. The 36 dpi read came out fluent and detailed and
was **worthless** — by then I was reciting the page. You cannot un-see a page.

So here: four failing documents, one page each, each composed into a single self-labelling
image by `page-vision/scripts/pair.sh`, each handed to a **separate subagent** that had never
seen the document, was forbidden to read any repo file or run any command, and — importantly
— **was not told the gate numbers**. Telling a reviewer "945 words against 431" buys a report
about word counts.

The scoreboard the pages were drawn from: joining `sd-sheets-d`'s `gate-after-all.tsv` against
`gate-ref-all.tsv` under the gate's own rule gives **146 of 171 sheets passing, 25 failing**.

## What came back

Four readings, four documents, and **two of the four independently named the same mechanism**
without any contact with each other.

### 1. An overflowing cell's text is painted again on the following page strip

`essd-16-3433-2024-t02.xlsx`, the track's most lopsided word failure (945 against 431).

The reviewer, given only the image, reported that our page 2 is a full page of prose whose
every line **begins mid-word** (`tadtm`, `horeline`, `ransect's`) and is clipped at both edges,
while the reference's page 2 is blank; and inferred that the run's origin cell lies on a page
to the left and that we let it bleed across the page break.

Measured afterwards, per page:

| page | ours | reference |
|---:|---:|---:|
| 1 | 439 | **439** |
| 2 | 315 | **0** |
| 3 | 153 | **0** |
| 4 | 49 | **0** |

**Page 1 agrees to the word.** The entire 514-word failure is text painted onto three pages
the reference leaves empty.

The important negative: **page counts already match, 4 against 4**. So `SheetTextOverflow`'s
print-area extension is **correct** and is not the defect — LibreOffice extends the area the
same way, producing the same four pages, and then simply does not paint the run again on the
strips after the one holding its anchor cell.

**The defect is in painting, not pagination:** the overflow run is drawn on every page its
extended width covers, instead of once on the anchor cell's page, clipped there.

Not yet fixed — it is a change to the sheet painter's clip, and wants its own round.

### 2. A rich-text run's underline and strikethrough were never read — **fixed**

`Infotabelle_WLAN im Flugzeug.xlsx`. The reviewer reported that the reference draws
`Innereuropäische Flüge:` **underlined** with a bold `kostenlos` under it and we draw the first
line un-underlined; and — this is the part that made it cheap to act on — noted that **bold
sub-runs render correctly everywhere else on the page** (`3 €`, `2,90 €`, `13,58 €`), so
"rich text is broken" was already ruled out before anyone opened a file.

The source cell is exactly that:

```xml
<r><rPr><u/><sz val="10"/>…</rPr><t>Innereuropäische Flüge</t></r>
<r><rPr><sz val="10"/>…</rPr><t xml:space="preserve">:      Chatten: </t></r>
<r><rPr><b/><sz val="10"/>…</rPr><t>kostenlos</t></r>
```

`XlsxRunFont` carried `Family, Points, Bold, Italic, Colour` and nothing else. An `rPr` is a
full `CT_Font` by schema, so `<u>` and `<strike>` were parsed, discarded, and drawn plain —
while the *cell* path read both (`XlsxCellFormats.UnderlineOf`) and `SheetTextLayout` already
drew both. Only the run path was missing, which is why nothing failed and nothing looked wrong.

**Reach**, over the 109 corpus workbooks that have a shared string table: **13 state `<u>` on a
run, 6 state `<strike>`.** An upper bound, per the standing rule about declaration counts — a
shared string no cell references is still in the table. Two of the 13 are in the current
failing list.

**It moves no words, so no gate column can see it.** That is the class this round existed to
find.

**Correction, and it is about this round's own instrument.** Two follow-up commits reported
that the underline still did not appear on the page, on the strength of two independent
reviewers who each measured the composed image and found no rule in our half. Both reviewers
read their image correctly. **The image was wrong.**

`compose.py` reduced with nearest-neighbour, on a comment claiming that preserved a one-pixel
hairline. It does not — nearest samples one source pixel per destination pixel, so a rule
thinner than the sampling step falls between samples and vanishes. At 80% it deleted the
underline.

Measured afterwards, at the same commit the reviewers were shown:

- the PDF holds the rule as a fill, `(50.39, 456.62)-(168.07, 457.06)`, 117.7 pt by 0.44 pt;
- the 150 dpi render holds a **245 px solid black run at y=802**, the only long dark run on
  the page;
- recomposed with a darkest-of-block reducer, both halves show `y=672, run=197 px` — same
  position, same length, ours and the reference.

So the fix worked on the first commit and two further rounds of doubt were spent on an
artefact. Three lessons, all of which this project had already written down somewhere else:

- **An instrument can manufacture a defect out of nothing** — `render-comparison` says exactly
  this, and it applies to an instrument you wrote yourself an hour ago as much as to one you
  inherited.
- **Confirm an absence in the PDF's operators, not in a downscaled raster.** A missing mark and
  a mark the pipeline threw away are indistinguishable to the reader.
- **Check a new instrument against a known answer before trusting its first result.** The
  known-answer check here costs one `pdf-ops.py dump` and would have caught it immediately.

Fixed: `Underline` and `StruckThrough` added to `XlsxRunFont`, read in `XlsxRichRuns.ReadFont`,
applied in `XlsxCellFormats.Apply`. Absent stays **null rather than `None`**, because absent
means "keep what the run inherits" and an explicit `val="none"` turns an inherited line off;
collapsing the two would have left every underlined run plain, which is the original bug
wearing a different hat. Eight tests in `XlsxRunDecorationTests`; Spreadsheets 684/684, 0
skipped; solution builds 0/0.

### 3. A token too wide for its cell is not broken; LibreOffice breaks it mid-token

`Published_Issuances_2024.xlsx`, 457 words against 479, one page, page-exact.

The reviewer measured this off the image rather than eyeballing it. Every row's `LINK` cell
holds a URL. The reference wraps each onto **two lines, breaking mid-token**:

```
https://www.bsp.gov.ph/Regulations/Published%2
0Issuances/Images/M-2024-039.pdf
```

We draw **one line, clipped at the cell border**, with *zero ink* on the second line — in all
22 rows, including rows with ~35 px of unused vertical space. So the filename tail of every URL
is missing, which is the word deficit.

What the same reviewer measured as **identical**, which is what makes this specific: all six
column widths to the pixel, all 22 row heights, row count and content, body font metrics to the
pixel — and **the DESCRIPTION column wraps line-for-line identically in both**, including a
4-line cell. So this is not a wrap-width, measurement, autofit or font difference. It is
specifically **the absence of a character-level break fallback for a token containing no
break opportunity**.

Not fixed. Candidate causes the image cannot separate, and the probe that would: put a long
space-free token and a long spaced token into wrap-enabled cells, one plain and one a
hyperlink. If only the space-free hyperlink fails, the run is being treated as atomic; if every
space-free token fails, the character-break fallback is simply missing.

### 4. Borders: we draw rules the reference does not, and draw them doubled

Same document, same reviewer, measured in pixels:

- a **vertical rule between `ISSUANCE` and `DATE ISSUED`** that the reference does not draw
  anywhere — ours is dark on 587 of 605 body scanlines, the reference's on 36 (incidental glyph
  pixels);
- **two horizontal rules** the reference omits, at exactly the `1194/1193` and `1189/1188` row
  boundaries — ours 100% dark across the table width, reference 0%. The *adjacent* `1188/1187`
  boundary is drawn by both, so it is not a whole-region effect and the source almost certainly
  distinguishes those two pairs;
- roughly half our interior rules are **2 px where the reference's are 1 px**, and ours
  consistently occupies the reference's pixel *plus the one to its left*.

This corroborates, from an independent direction and on a different document, the standing
unassigned finding that **cell borders are not coalesced** (19 drawn strokes against
LibreOffice's 103 on the sampled page, with doubled hairlines at joins). Worth treating as one
item when it is picked up.

### 5. Chart defects on a page nobody had looked at

`Template Pilot Logbook JAR-FCL V3.0.xls`, page 17 — one embedded chart on an otherwise empty
page. Five differences, of which the cheapest to act on:

- **Y axis tick labels are `1200` where the reference draws `1200.0`** — every tick, one decimal
  place. This is precisely the failure mode `Core/Numbers` was moved down into Core to prevent
  ("every tick was written in its shortest round-trip form"). The move happened; the axis is
  still not applying the source's number format.
- **We draw a legend the reference does not** (and ours is clipped off the page edge).
- **The reference sets the title and axis titles bold; we set them regular**, and its axis title
  is larger.
- **We draw 2 X-axis labels where the reference draws ~37**, and our two label *values* match
  none of the reference's — so this may be date interpretation rather than label thinning.
- The series mark is in a different place and a different shape.

Ruled out by the same reading: plot-area geometry, position on the page, axis scale and
gridline positions, and the grey/magenta colours are all identical, so chart framing, axis
auto-scaling and theme colour resolution are **not** implicated.

## What this round says about method

- **Two of four reviewers independently named overflow clipping**, on two unrelated documents
  (§1 and the right-aligned `kein WLAN` spill in `Infotabelle`). Convergence across independent
  blind readings is worth much more than the same reader finding a class three times.
- **The "what looks identical" section carried most of the value.** In §3 it is what reduced a
  vague "text is missing" to "character-level break fallback", by ruling out six other causes
  before anyone read a line of code.
- **Not briefing the reviewers with the gate numbers worked.** None of the four reports is
  about word counts, and three of the five findings move no words at all.

## Not done

- §1 (overflow repainted on later strips), §3 (character break fallback) and §4 (border
  coalescing) are diagnosed and unfixed.
- §5's chart items are diagnosed from one page only; reach across the corpus is unmeasured.
- Only 4 of the 25 current sheets failures were read. The other 21 are untouched.
- The `essd` finding was checked for breadth on three other documents and the "reference leaves
  the page blank" signature appeared on only one of them (`NorwegianXPension…`, 1 blank page, 6
  words). So §1 as stated is a **narrow** variant; the broader "we do not clip at a boundary"
  mechanism needs a detector of its own rather than the blank-page proxy.

---

# Fan-out: four more failing pages, four more independent blind readers

Same method, four more of the 25 sheets failures, each read by a separate subagent that built
its own pair, had never seen the document, was forbidden the repo and was not told the gate
numbers.

## The headline: overflow clipping is now named by FIVE independent readers

`essd` (§1), `Infotabelle` (§2's neighbour), and now three more, on unrelated documents, none of
them in contact with one another:

- **`fse_identification_form.xlsx` p1** — the reference hard-clips description text mid-glyph at
  exactly x=1483 on four separate lines; we paint on to x=1547. Decisively *not* line breaking:
  the reviewer measured every wrap point as identical in both, so both renderers agree on the
  wrap width and only the paint clip differs.
- **`SIL_TDB605.xls` p6** — the reference's page ends at x=904, which is exactly the right border
  of the page's table and boxes, i.e. a column boundary; ours keeps drawing at least 93 px past
  it. The extra strip holds *only* glyph continuation — no gridlines, no new cell content —
  which argues against "we assigned one more column to this strip".
- **`RCO_VOR_Master_List_082824.xlsx` p73** — we draw 40 rows of red note text; the reference's
  page 73 is pure white, zero non-white pixels. **Caveat the reviewer raised themselves:** this
  may instead be a page-index difference, and they named the measurement that settles it. Not to
  be counted in the class until that is done.

A class named five times from five blind readings is worth more than one named five times by one
reader. Dispatched as `wt-sheet-overflow`.

## New findings not previously recorded

**Bullet glyphs are a dot where the reference draws a filled square**
(`Company_Seniority_Date_Calculator.xlsx` p8). Measured on all five bulleted lines: ours 2-4 ink
px at x 77-78, the reference 18 ink px at x 77-80, five rows tall — a ~1.3x0.6 pt dot against a
~2.5x3.1 pt square. The bullet *origin* is identical, so the indent is right and only the mark is
wrong; the following text then starts 2-3 px further left in ours on every line, which also says
the run is positioned by advance rather than to a tab stop. This is the sheets counterpart of the
symbol-bullet recode that closed 81 of 96 documents on the words track.

**A footnote marker is not superscripted** (`SIL_TDB605.xls` p6): `Date and Time1` against the
reference's `Date and Time¹`, full size and on the baseline. Same omission as §2's underline and
in the same record — `XlsxRunFont` still has no escapement field, so `<vertAlign>` is parsed and
discarded exactly as `<u>` was. Routed to the `wt-rich-portion` agent, with the warning that
`render-comparison` already records escapement as a case where there was nowhere to *store* the
magnitude, so it may not be a wiring change.

**Text is shrunk where LibreOffice does not shrink it** (`SIL_TDB605.xls` p6). In two lower text
blocks: cap height ours 11 px against 12, x-height 8 against 9, line pitch **18.75 against 21.9**.
Local to those blocks — the heading block above matches to the pixel with zero drift across 720 px
of cross-correlation. The reference plainly does not shrink; it lets the text overflow and clips
it, which ties this to the clipping class above. Suspect a shrink-to-fit we apply and it does not.

**Two heavy borders are simply absent from ours** (same page): the reference draws a 4 px
rectangle around each of the two lower text blocks and we draw no top, bottom or left edge for
either. Not a "we cannot draw thick borders" problem — the `Notes` box on the same page, same 4 px
weight, we draw correctly. Folds into the border item.

**We draw a sentence the reference does not** (`fse_identification_form.xlsx` p1): the Serial
number row's description cell is empty in the reference and carries a full sentence in ours. Note
the *direction* — this is the mirror of §2's dropped word, and the two must not be assumed to
share a cause.

## A fix confirmed from two independent directions

The `fse` reviewer measured our three blue bands as `#2351A3` against the reference's `#2F5597`
and observed, unprompted and with no repo access, that `#2F5597` is what an HSL luminance
modulation of the Office accent `#4472C4` produces while a naive sRGB channel scale gives
`#33568F`. That is the exact claim `wt-sheets-v` had made from the LibreOffice source and left
unmerged when the session crashed. Merged. The same reading rules out colour management: the
page's other three fills are byte-identical between the two renderings.

## Method note

Seven readings in, the pattern holds: **the "what is identical" section carries most of the
value.** It is what reduced `fse`'s clipping finding from "text differs" to "the wrap points are
the same and only the paint clip differs", and what confined `SIL_TDB605`'s shrink to two blocks
rather than the page. None of the seven reports is about word counts, and most of the findings
move no words at all.


---

## Correction: a text-layer measurement was used as evidence about paint

This write-up originally argued that page 1's overflow painting was already right, on the
grounds that "both renderers push text past the right page edge by almost exactly the same
amount — rightmost text `xMax` 617.0 pt ours, 617.7 pt reference, on a 612 pt page".

**That figure is from `pdftotext -bbox`, and a clip never touches the text layer.** Glyphs a
clip excludes are still in the content stream and still carry their positions, so measuring
`xMax` cannot see a clipping difference at all. Measured as *ink*, the same page is 973.9 pt
against 933.6 — a 40 pt overhang, which is the defect the figure was being used to rule out.

A later round found the same confusion in its own §6(b) and stated the rule this cost:

> **When the question is paint, measure ink.**

The two instruments answer different questions and both are correct at their own. `pdftotext`
and `pdf-ops.py`'s text records answer "what does the document say and where does it claim to
say it". A raster, or a fill/stroke census, answers "what is on the page". A clip, a
background, a merge and an overdraw are all invisible to the first and obvious to the second.

Worth pairing with the compositor correction above: that one was an instrument *losing* ink it
should have kept, this one is the wrong instrument for the question. Both produced a confident
figure that pointed the opposite way from the truth.
