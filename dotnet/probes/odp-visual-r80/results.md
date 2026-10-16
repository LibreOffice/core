# A `text:a` is a field, a bullet slot is a picture, and two of the five leads were not there

Round 80, on the converted corpus's `.odp` column. Five items were briefed from blind readings of
composed page pairs: two of them are real and are closed, one is real and had a different cause
from the one the previous round gave it, and **two are refuted in the PDF's own marks**. A sixth
defect — one no reader named — was found on the way and is the widest thing in this write-up.

## Environment

| | |
|---|---|
| base commit | `5c003d58c`, worktree `/home/user/wt-odpvisual`, branch `agent/odpvisual` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| corpus | `/home/user/corpus-odf` (26.2.4.2's own `--convert-to` of the 947-document corpus), `.odp` column 302 |
| original corpus | `/home/user/sample-files` |
| reference bank | `/home/user/gate-odf-r78/ref`, rendered 2026-09-07 **after** all five tarball font confounds were moved aside |
| base binary | `/home/user/libreoffice-core/…/XRay.Office.Cli`, the main checkout at the same commit `5c003d58c`, built 2026-09-07 22:37 and untouched since |
| fonts | all five confounds aside: `/opt/libreoffice26.2/share/fonts/truetype` holds 82 faces, 38 in `.duplicates-aside/`, 8 in `.condensed-aside/`, 8 in `.noto-aside/` |
| workers | 2, as briefed. A whole-corpus ODF gate was running in the main checkout throughout and two other agents' sweeps beside it |

The reference half was **reused, not re-rendered** — every change here is confined to `dotnet/src`,
which cannot reach `soffice`. No build in this worktree overlapped a sweep in it; checked with the
newest render's mtime against the binary's, as the rulebook requires, not with `pgrep`.

## Finding 1 — a `text:a` in slide text is a field. Confirmed, closed, and the previous round's cause for half of it is wrong.

The brief carried this from round 79 with its seat named, and the seat is right: in a *draw* shape's
text a hyperlink is not a character property but an EditEngine field. `txtparai.cxx`:1352-1370 asks
the cursor for a `HyperLinkURL` property and builds an `XMLImpHyperlinkContext_Impl` when it has one
and an `XMLUrlFieldImportContext` when it does not — Writer's text cursor has it, and Draw's,
Impress's and Calc's do not. The field's `Representation` is the element's own content
(`txtfldi.cxx`:2907-2918), flattened.

### Rule 1a: an over-long field is filled to the cell, not moved down and not broken at a separator

`ImpEditEngine::CreateLines`' `EE_FEATURE_FIELD` branch (`impedit3.cxx`:1101-1200) measures the
field, and if it is wider than the room left it walks the field's own text with
`xBreakIterator->nextCharacters(…, CharacterIteratorMode::SKIPCELL, …)`, pushing a break onto
`ExtraPortionInfo::lineBreaksList` wherever the next **cell** would overflow. A cell is a grapheme
cluster, so on Latin text that is a break opportunity at every character. The field is *not* moved
onto the next line — the only case that does is `bFieldStartNextLine`, when even the first character
does not fit and the line already has content (`:1174-1180`).

### Rule 1b: a field's spill lines are one *ascent* apart, and the previous round's cause for that is refuted

Round 79 recorded the 1.0 em pitch it measured as *"a field portion is the one portion kind that is
never given the fixed cell height"*, citing `impedit3.cxx`:1100-1104 against `:1256-1259`. **That is
not what produces it, and the true rule has two consequences the stated one does not.**

The field portion's own height never decides a line's height: `RecalcFormatterFontMetrics`
(`impedit3.cxx`:3119-3183) is run over every portion of the line whose kind is not `LINEBREAK` —
a `FIELD` portion included — and under `IsFixedCellHeight()` it sets the ascent to the font height
and the descent to `ImplCalculateFontIndependentLineSpacing(height) − ascent`, so the line holding a
field is 1.2 em tall like any other. What is 1.0 em is the *spill*, and it is a **paint-time** rule:

> `aTmpPos += MoveToNextLine(aStartPos, nMaxAscent, nColumn);`
> — `impedit3.cxx`:3793, whose own comment says *"only use GetMaxAscent(), pLine->GetHeight() will
> not proceed as needed (see PortionKind::TEXT above and nAdvanceY) what will lead to a compressed
> look with multiple lines"*.

`MoveToNextLine` takes `aStartPos` **by reference** (`:3273-3279`), so each spill line advances the
running pen by the line's `GetMaxAscent()` and the paragraphs after it move down with it. Two things
follow that "the portion misses the fixed cell height" does not predict, and both are measured
below:

- **The distance is the ascent, which equals the em only under fixed cell height.** With the flag
  off, 26.2.4.2 draws the spill 14.400 pt apart on 16 pt Liberation Sans where the ordinary pitch is
  17.773 — that is `ascent` against `ascent + descent`, not `1.0 em` against `1.2 em`.
- **The formatter never sees a spill line at all**, because the whole field is one portion of one
  `EditLine`. So the height that anchors the block, and that the shrink-to-fit search measures,
  counts the field's line **once**, and the spill hangs out of the bottom of a middle- or
  bottom-anchored box.

### Measured, on eleven one-attribute variants and a four-slide fixture

`field-variants.py` authors flat-ODF variants that differ in exactly one thing and reads the
baselines back out of 26.2.4.2's PDF. All figures in points from the page top, 16 pt Liberation
Sans in a 14 × 5 cm box:

| variant | what 26.2.4.2 draws |
|---|---|
| `v1` the URL as plain text | 42.747 60.521 78.294 96.067 113.840 — every pitch 17.773, breaks at the solidi |
| `v2` the same URL as a `text:a` | 42.747 60.521 **74.921 89.321** 107.094 — the two spill lines 14.400 apart |
| `v3` a link starting part way along a line | the field stays on the line it starts and spills at 14.400 |
| `v4` `v2` middle-anchored | first baseline 95.840 — two lines of 17.773 centred in the box, the spill below it |
| `v5` `v1` middle-anchored | first baseline 78.067 — four lines of 17.773 centred |
| `v6` `v2` bottom-anchored | first baseline 148.933, the spill past the box's bottom edge |
| `v7` a link short enough to fit | nothing changes at all |
| `v8` the link inside a `text:span` | identical to `v2` |
| `v9` text after the field | the tail is drawn on the last spill line's own baseline |
| `v10` a long run of text before the field | the field still starts on that line |
| `v11`/`v12` a centred and a right-aligned paragraph | see *what is left*, below |

`v4` against `v5` is the block-height half on its own: same box, same characters, and the first
baseline differs by 17.77 pt because one paragraph is two `EditLine`s and the other is four.

### What moved

`0335fab9-79f0-4944-b92c-f223837ca2d8.odp` page 6 is the witness the brief named. Before, our lines
were four short and every pitch was 19.191; after, **all fifteen lines agree with 26.2.4.2
character for character and every baseline to 0.001 pt**:

    ref   80.674 123.449 170.617 189.808 205.795 221.783 268.157 291.345 337.720 360.907 376.894 392.882 439.257 462.444 478.431
    ours  80.674 123.449 170.617 189.808 205.795 221.783 268.158 291.345 337.720 360.907 376.895 392.882 439.257 462.444 478.431

with the reference's own break positions — `…hr-connect/organisational-development/career-and-development-planning-framework/lead` / `ership/` — reproduced exactly.

**Reach: 665 `text:a` in 156 of the 302 `.odp`.** (The brief's *"485 in 107"* is `content.xml`
alone; `styles.xml` carries the rest.) **Not one of the 665 contains a child element of any kind** —
no nested `text:span`, no `text:s`, no `text:tab` — which matters twice. It is why flattening the
element's content is safe here: no character can be lost. And it is why the corpus cannot tell that
26.2.4.2 flattens too, which it does — `XMLTextFieldImportContext` overrides `characters` and
nothing else, so a nested span's text reaches no context at all and its formatting is dropped.

### Finding 2 — hyperlink underlines. Already closed; the residual was Finding 1.

Counted in the PDFs' own marks — fills and strokes under 2.5 pt tall and over 20 pt wide, the
measurement the brief named:

| page | round 78 | round 79 | **this round** | 26.2.4.2 |
|---|---:|---:|---:|---:|
| `0335fab9` p6 | 0 | 8 | **9** | 9 |
| `medicines_bulletin` p1 | 0 | 8 | **8** | 8 |

Round 79 closed the decoration itself and said the residual 8-against-9 was the field breaking
giving that page one more link line. It was: closing Finding 1 closes this column with no change to
any decoration code.

## Finding 3 — bullet glyphs. Confirmed in the marks, and it was two readers' defects in one.

Both halves are in the reference's own spans, so no image was needed to decide them.
`redac-sas-201509-asisp-research.odp` page 7, twelve markers, before:

| | face | size | colour | code point |
|---|---|---:|---|---|
| 26.2.4.2 | OpenSymbol | 12.61 / 11.91 | **#9bbb59 / #c0d597** | **U+F0FC / U+F0A7** |
| ours, before | OpenSymbol | 12.61 / 11.91 | #000000 | U+2022 |

So the face was already right and the *character* and the *colour* were both wrong. Two seats:

- **The character.** `OdfTextBody.Marker` took its label from `OdfListStyle.FormatLabel`, which is
  the **extraction** answer: it puts the bullet through `OutlineNumbers.NormaliseBullet`, which
  collapses any Private Use Area character to U+2022. That is right for an index — a Wingdings slot
  means nothing to a consumer — and wrong for a page, where `SlideTextLayout` needs the slot and the
  family together to reach the OpenSymbol glyph holding the same picture
  (`unotools/source/misc/fontcvt.cxx`:185 and :1325). The deck reader reached the same fork from the
  other side and took the other branch (`PptxTextBody.Marked`, *"a recodeable face keeps its Private
  Use Area slot"*); the ODF path now does too. **3598 bullet levels in 74 of the 302 `.odp` state a
  Private Use Area character**, every one in the F000 block, and every family they name has a recode
  table but one (`CommonBullets`, once).
- **The colour.** `fo:color` on the level's own `style:text-properties`, with
  `style:use-window-font-color="true"` meaning the item's own colour instead. Nothing read either.
  It is DrawingML's `a:buClr`, which the deck reader has read since it was written.
  **22 436 bullet levels in all 302 `.odp` state `fo:color`.**

A third, smaller seat came with them: `OdfTextBody.Family` trimmed a CSS-style family list only when
the name resolved to a `style:font-face` declaration, so a level writing
`fo:font-family="&apos;Wingdings 2&apos;"` asked for a family whose name is not `'Wingdings 2'`.
**721 of the corpus's bullet levels write it that way.**

After, on the same page: colours `#9bbb59` and `#c0d597` exactly, and the code points `U+E4C2` and
`U+E46F` where the reference states `U+F0FC` and `U+F0A7`. **The code point differs and the picture
does not** — the reference recodes when it draws and writes the original into its `ToUnicode`, this
tree recodes when it lays out. Checked rather than assumed: a 600 dpi crop of the check mark and of
the square sub-bullet is **byte-identical between the two renderings**, 0 of 75 651 and 0 of 75 150
bytes differing by more than 16. `Sean Monogue.odp` page 8's five markers go the same way —
`U+F06E` → `U+E439`, colour `#ffffcc` on both sides — which is the second reader's "gold squares
where we draw cream dots": the colour was already right and the *shape* was not.

## Finding 4 — the slide background's arc-work and its gradient. **Both halves refuted.**

The reader reported that a slide background's decorative arc-work is absent from ours and that the
same slide's gradient reaches near-black at the bottom where the reference stays blue. Neither is in
the marks.

**The gradient.** Sampled at ten fractional heights down each side at an x that avoids text — the
discriminator the reader named — `Sean Monogue.odp` page 8:

    height   0.0     0.1     0.2     0.3     0.4     0.5     0.6     0.7     0.8     0.9     1.0
    ref    #0066cc #0064c9 #005ab4 #0053a7 #004d9a #00478e #003f80 #003973 #003367 #002d5b #00274f
    ours   #0066cc #005fbf #0059b3 #0053a6 #004d9a #00468d #004081 #003a74 #003368 #002d5b #00274f

Both end at `#00274f`, a dark blue, and no sample differs by more than 5 of 255 in any channel.
Neither side goes near black.

**The arc-work.** A whole-page raster difference at 72 dpi, meaned over a 24 × 18 grid, gives a
worst cell of **26** and a page mean of **3.91** *at the round's base* — and the cells that carry it
are the ones with text on them. There is no region of gross difference anywhere on the page, so
nothing the size of a background decoration is missing.

**What made it look absent, and it is a trap worth writing down.** `page.get_drawings()` reports
**667 paths for the reference and 4 for us**. That is not 663 missing marks: 657 of the reference's
are the *bands* its own PDF writer decomposes a gradient into, and we emit one shading operator for
the same gradient. **A path-count census cannot compare two renderers that fill a gradient
differently**, and this one reads as a catastrophic absence.

## Finding 5 — the desaturated colour emoji. **Refuted, and there is no emoji.**

`medicines_bulletin_february_2023_final.odp` page 1 embeds **no emoji font on either side** —
`pdffonts` gives the same four faces for both (Carlito Bold and Regular, Liberation Sans and
Liberation Sans Bold) and no run on the page holds an emoji code point. What the reader saw is a
raster. Both sides carry five images at matching pixel sizes, and their mean colours agree:

| image | ref mean RGB | ours mean RGB |
|---|---|---|
| the warm banner | 223, 218, 51 | 225, 219, 48 |
| the other four | 220/210/202, 222/232/237, 32/118/184, 198/209/197 | 220/210/202, 223/233/239, 31/118/185, 199/210/198 |

Rasterising the page and meaning the banner's own rectangle gives **223, 218, 52 against 224, 219,
49** — warm yellow on both. The brief's own instruction to check which face each side used before
assuming a painting defect was the right one, and the answer it gives is that there is no defect
here at all.

**What is on that page and is a defect** is one image drawn in the wrong place: the 172 × 63
picture at the top runs 241.79 → 471.43 across and −0.68 → 73.73 down in the reference and
296.25 → 458.22 by 8.14 → 66.93 in ours. Different origin, different extent, and it bleeds off the
page top in the reference. Not investigated; recorded as a lead.

## Finding 6 — nobody's lead: `sub-view-size` is in the extension namespace, and 178 documents state one

Found while measuring Finding 4. A `draw:custom-shape` that LibreOffice imported from OOXML carries
`draw:type="ooxml-non-primitive"` and **`svg:viewBox="0 0 0 0"`** — its path coordinates are not in a
normalised box — and states its real coordinate space per subpath in `sub-view-size`.
`EnhancedCustomShape2d::SetPathSize` (`svx/source/customshapes/EnhancedCustomShape2d.cxx`:650-670)
takes that as `m_nCoordWidth`/`m_nCoordHeight` whenever both are non-zero and falls back to the
global view box otherwise.

`OdfEnhancedGeometry` read it — in the **`draw:`** namespace. LibreOffice's own exporter writes
`rExport.AddAttribute(XML_NAMESPACE_DRAW_EXT, XML_SUB_VIEW_SIZE, aStr)`
(`xmloff/source/draw/shapeexport.cxx`:5006) **under a comment that says "export
draw:sub-view-size"**, and `XML_NAMESPACE_DRAW_EXT` is `drawooo`. Censused over the whole converted
corpus:

| spelling | occurrences | documents |
|---|---:|---|
| `drawooo:sub-view-size` | **5673** | **178** — `.odp` 151 of 302, `.odt` 23, `.ods` 4 |
| `draw:sub-view-size` | **0** | 0 |

This is the **fifth** instance of the rule `dotnet/CLAUDE.md` records against `drawooo:display`,
`loext:shadow-blur`, `chartext:coordinate-region` and `text:line-break`, and it wears the same
disguise as the fourth: the attribute name looks right and the namespace is not. Read wrongly, every
such shape falls back to its own bounding box in hundredths of a millimetre and is drawn at a
fraction of its extent. On `Sean Monogue.odp`'s master the four solid-filled freeforms went from
20 × 24, 12 × 12, 17 × 17 and 8 × 7 points to **89.86 × 105.59, 50.97 × 51.73, 73.47 × 74.83 and
33.73 × 31.46** — which is their stated `svg:width`/`svg:height` to a twentieth of a point in all
eight numbers. Page 8's mean raster difference against 26.2.4.2 goes **3.91 → 3.38** with the bullet
fix beside it.

## What moved on the gate

**Nothing, in either direction, and that is the expected answer.** The `.odp` column is 295 of 302
before and after, with the same seven documents failing for the same reasons and the same glyph
counts on all seven:

| | base `5c003d58c` | this round |
|---|---|---|
| `match` | 295 | **295** |
| `words` | 6 | **6** |
| `unembedded` | 1 | **1** |

    slides/ceiling-002/odp/16 - UTM - (NASA).odp                      words  15121/13981
    slides/chartset-008/odp/038_Competitive_Advantage_Card…odp        words   1585/1449
    slides/ceiling-002/odp/8_P-Pavese_AIRBUS-ATB-journee-CRATB.odp    words  10496/9914
    slides/done-001/odp/Statement of Work presentation.odp            words   1286/1255
    slides/ceiling-001/odp/OnTrac_StarCertificationProgram-3Day.odp   words   7709/6233
    slides/ceiling-002/odp/Demick_JetBlue.odp                         words   3267/3179
    slides/done-014/odp/vvsummit2022-Research-Roadmap…odp        unembedded   8377/8377

Six are the raster ceiling — **we draw more** than the reference does — and the seventh is
glyph-exact and fails on a `CFF ` face the writer declines to embed. Both halves of that were
already classified by round 78 and neither is anything this round could reach.

The base column is the main checkout's own gate at this round's base commit
(`/home/user/gate-odf-r80`, rendering its own reference); this one is scored against the banked
`gate-odf-r78/ref`, which round 79 established reproduces a fresh reference 302 of 302 on pages and
glyphs. The two agree row for row on all seven failures.

**So the gate is the regression check and not the result**, exactly as the brief said it would be:
a break position, a bullet's picture, a marker's colour and a shape's extent add no glyphs and no
pages. Every figure this round is measured on is in the pages themselves — baselines, break
positions, drawn code points, drawn colours, thin-wide marks, and a raster mean.

### The original track cannot move, and it does not

Every reader this round touches is an ODF one, and `SlideTextRun.IsField` is set in exactly one
place — `OdfTextBody`. The two changes in shared code are inert without it: the new `cellBroken`
argument defaults to null and every branch that reads it is guarded, and `SlideTextLayout`'s stacking
rule fires only on a line marked `ContinuesField`, which nothing marks when a paragraph holds no
field.

Measured rather than argued, with `confine.sh`: **101 documents of the original corpus rendered with
the base binary and with this one, `SOURCE_DATE_EPOCH` fixed on both legs, and compared byte for
byte.** The base binary is the main checkout's, built at this round's own base commit and not
rebuilt since.

| column | sampled | renderings that differ |
|---|---:|---:|
| `.pptx` | 32 of 251 (every 8th) | **0** |
| `.ppt` | 13 of 51 (every 4th) | **0** |
| `.docx` | 23 of 272 (every 12th) | **0** |
| `.doc` | 6 of 66 | **0** |
| `.xlsx` | 21 of 241 | **0** |
| `.xls` | 6 of 64 | **0** |

A byte-identical rendering cannot have moved a gate column, so no verdict on those 101 can have
moved either. It is a sample and is stated as one; what makes it enough is that it is a sample of a
population the diff cannot reach at all.

### The converted `.odt` and `.ods` columns do not move either, and the reason is worth recording

`drawooo:sub-view-size` appears in 23 `.odt` and 4 `.ods` of the converted corpus, so those columns
looked like collateral. They are not: **`OdfEnhancedGeometry` has exactly two callers and both are
`OdpSlideLayout`** — no word-processing or spreadsheet path reads an ODF enhanced geometry at all.
All 26 of those documents that the base gate has a row for were rendered both ways and **0 of 22
`.odt` and 0 of 4 `.ods` differ by a byte.** The attribute's reach for this tree is the 151 `.odp`
and nothing else; that a `.odt` states one and draws no differently is a separate gap, and not this
round's.

### The suite

Every project run individually, at the round's head:

| project | base | this round |
|---|---:|---:|
| Containers | 109 | 109 |
| Core | 516 | 516 |
| Markup | 259 | 259 |
| OpenDocument | 139 | **143** |
| Presentations | 995 | **1005** |
| Rendering | 164 | 164 |
| Spreadsheets | 1188 | 1188 |
| Text | 723 | **727** |
| Vector | 302 | 302 |
| WordProcessing | 1757 | 1757 |
| **total** | **6152** | **6170** |

0 failed and 0 skipped everywhere; the eighteen new ones are the four classes this round adds.

`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed of 552**, the briefed baseline exactly:
`PageDrawingComparisonTests` ×4, `TabStopComparisonTests` ×4, `SheetDrawingComparisonTests` and
`JustificationShrinkComparisonTests`. **An eleventh failed on the first run and was the fixture's
fault, not the tree's**: `OdpShapePathComparisonTests` compares the *vertex count* of every shape
in `odp-shape-paths.fodp` against LibreOffice's own PDF, and the triangle added for Finding 6 was
written `M 0 0 L 1200 0 600 900 0 0 Z` — an explicit line back to the start *and* a close.
LibreOffice folds the redundant line into the `Z` and answers three vertices where this tree
answers four. Dropping the redundant pair from the fixture's path makes both answer three, and the
class is 3 of 3. Worth recording because it is a trap for any authored path: **a closing `Z` and a
line back to the first point are the same picture and not the same vertex list.**

## An instrument note, because it cost this round a whole sweep

**A sweep whose output directory already has a sweep in it reports the other one's documents as
`ours-failed`.** The first `.odp` run here was started with `nohup … &` from a tool call whose shell
then exited; it looked dead — it had stopped at 267 of 302 rows and did not appear in `ps` under the
pattern I grepped — so the directory was deleted and the sweep restarted into the same path. It was
not dead. Both
runs then wrote to one `rows.tsv` and each `rm -rf`'d the other's `t0`/`t1` between documents, so
**27 of 302 rows came back `ours-failed` and 4 documents were scored twice**. Every one of the 27
renders correctly on its own.

That reads exactly like a catastrophic regression, and it is the second shape of the rule
`dotnet/CLAUDE.md` already carries about a parallel sweep needing a directory per item: the first
shape is two *workers* of one run sharing a slot, this is two *runs* sharing a directory. The
check that settles it is not `pgrep` — the pattern I grepped did not match the shell wrapper the
harness had built — but the row file's own growth: a directory that gains rows after its sweep has
been declared finished has a second writer in it. Discarded and re-run into a fresh directory.

## What this brief got wrong

1. **Item 4 is not there.** Neither the arc-work nor the gradient. The page agrees with 26.2.4.2
   everywhere to within 26 of 765 on a coarse raster mean, and the gradient's own eleven samples
   agree to 5 of 255. The reading that produced it is the third single-reader observation in three
   rounds to survive into a brief as a fact; the discriminator it named — sample the background at
   ten fractional heights — is exactly the right one and it refutes the reading.
2. **Item 5 is not there either, and it is not an emoji.** No page-1 run on either side holds an
   emoji code point and no colour face is embedded in either PDF. The brief's hedge ("more likely
   font selection than painting") was still the wrong half of the tree: the object is a raster, and
   its colours match.
3. **Item 2 was already closed.** The measurement the brief asked for — thin wide marks — reads 8
   against the reference's 8 on `medicines_bulletin` and 8 against 9 on `0335fab9`, and the missing
   ninth is item 1, exactly as round 79 said it was. Nothing in the decoration path needed touching.
4. **The reach figure for item 1 is 665 in 156 of 302, not 485 in 107.** The smaller number counts
   `content.xml` only.
5. **Round 79's cause for the 1.0 em pitch is wrong** — see Finding 1b. It is not the field portion
   missing the fixed cell height; it is the painter advancing by `GetMaxAscent()` per spill line.
   The correction matters because the true rule also says the spill is invisible to the formatter,
   which is what decides where a middle-anchored box sits.
6. **A path-count census is not a way to compare two renderings.** 667 against 4 on `Sean Monogue`
   page 8 is one gradient drawn two ways.

## What is left, with its seat

- **A centred or right-aligned paragraph holding an over-long field.** 26.2.4.2 draws it broken —
  the `EditLine`'s width is the whole field's, so `ImpAdjustLine` starts the line left of the box
  and the spill runs off the edge (`v11` draws two fragments at x = −2.85 and −7.09 of a box
  starting at 28.35, and `v12` draws nothing at all). This tree aligns each spill line on its own
  and produces a readable paragraph. Deliberately not reproduced; recorded because it is a place
  where we do not match and should not.
- **`style:font-charset="x-symbol"` decides whether a symbol slot is recoded at all.** Without it
  26.2.4.2 sends the request to fontconfig, which answers DejaVu Sans, and draws the raw slot there;
  with it, VCL's own chain reaches OpenSymbol. It is the ODF spelling of `a:sym/@charset="2"`, which
  `SlideSymbolFont.IsMicrosoftEncoded` already models on the deck side. **13 of the 302 `.odp`
  declare `x-symbol` on a recodeable family and 4 declare one without it**; this tree recodes for
  both, which is right for the thirteen and draws a glyph where the reference draws a blank for the
  four. `OdfFontFace` would need a `Charset` and `SlideMarker` a way to carry it.
- **One image is placed wrongly on `medicines_bulletin` page 1** — 172 × 63, ours 162.0 × 58.8 at
  (296.25, 8.14) against the reference's 229.6 × 74.4 at (241.79, −0.68). Not diagnosed.
- **A master's `draw:line` and its gradient-filled shapes are not visible to a path census** on our
  side, so whether all 34 of `Sean Monogue`'s master shapes are drawn was settled by raster and not
  by operators. The raster says yes.
- **The `.pptx` side has the same field rule and is untouched.** `oox/source/drawingml/textrun.cxx`
  :144-170 builds a `com.sun.star.text.TextField.URL` for any run carrying an `a:hlinkClick`, with
  the run's text as its `Representation` — so a DrawingML hyperlink is a field exactly as a
  `text:a` is. Nothing here sets `SlideTextRun.IsField` from `PptxTextBody`, deliberately: the
  brief's regression check is that the original slides track does not move, and wiring it would
  move it. It is one line and a measurement, and the reach is larger than the ODF one —
  **2293 `a:hlinkClick` on the slides of 95 of the corpus's 251 `.pptx`-family decks.**

## The scripts

| | |
|---|---|
| `ref-render.sh` | one document through 26.2.4.2, with the hex-digest profile and `timeout -k` |
| `field-variants.py` | the thirteen one-attribute field variants, rendered and read back |
| `make-fixture.py` | writes `odp-hyperlink-field.fodp` |
| `make-bullet-fixture.py` | writes `odp-list-bullet-symbol.fodp` |
| `baselines.py` | the drawn baselines of one page and the pitch between them |
| `marks.py` | every drawn span with its face, size, colour and code points |
| `count-rules.py` | the thin wide marks on one page, fills and strokes |
| `background.py` | a page's background colour at ten fractional heights, and its path census |
| `sweep-ours.sh` | our half of one extension column, scored against a banked reference |
| `confine.sh` | one column rendered with two binaries, byte-compared |
| `inkgrid.py` | the mean and worst-cell raster difference between two renderings of one page |
