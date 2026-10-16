# An ODF document's own fonts, an empty paragraph's height, and the rule under a hyperlink

Round 79, on the converted corpus's `.odp` column. Three findings were briefed from five blind
readings of composed page pairs; all three are real, **two of the three had the cause the brief
gave them and one did not**, and closing them moved the column **289 → 295 of 302**.

## Environment

| | |
|---|---|
| base commit | `6659fa74f`, worktree `/home/user/wt-odpvis`, branch `agent/odpvis` |
| reference | `/opt/libreoffice26.2/program/soffice` — **LibreOffice 26.2.4.2** `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| corpus | `/home/user/corpus-odf` (26.2.4.2's own `--convert-to` of the 947-document corpus), `.odp` column 302 |
| original corpus | `/home/user/sample-files` |
| reference bank | `/home/user/gate-odf-r78/ref` — the whole converted corpus rendered 2026-09-07 at 20:36–20:57, **after** the eight DejaVu Condensed faces were moved aside |
| fonts | all five tarball confounds aside: `/opt/libreoffice26.2/share/fonts/truetype` holds 82 faces, with 38 in `.duplicates-aside/`, 8 in `.condensed-aside/` and 8 in `.noto-aside/` |
| workers | 2, as briefed. The main checkout's whole-corpus ODF gate had finished (1285 rows) before any sweep here started |

`/home/user/gate-odf-r78` is **this round's own base**: the main checkout is at `6659fa74f`
and clean, so its `ours/` half is the base binary's and its `ref/` half is 26.2.4.2's. Both
halves were reused rather than re-rendered.

## What moved

| track | before | after |
|---|---|---|
| converted `.odp` (302) | **289** match | **295** match |
| original `pptx` (251) + `ppt` (51) | 292 match | **292 match, 0 gate columns moved** |
| original `docx` (272) + `doc` (66) | 315 match | **315 match, 0 gate columns moved** |
| original `xlsx` (241) + `xls` (64) + `xlsm` (2) | 264 match | **264 match, 0 gate columns moved** |

The six `.odp` that moved, all `words` → `match`:

    slides/done-002/odp/medicines_bulletin_february_2023_final.odp
    slides/done-004/odp/0335fab9-79f0-4944-b92c-f223837ca2d8.odp
    slides/done-005/odp/manufacturing_process_simulation_working_group_overview_2023.odp
    slides/done-012/odp/Sean Monogue.odp
    slides/done-014/odp/redac-sas-201509-asisp-research.odp
    slides/metrics-001/odp/Ramp Up Campaign - French.odp

37 of the 302 renderings changed at all. The seven that still fail are the ones round 78 had
already classified: six raster-ceiling documents where **we draw more** than the reference
(`OnTrac_StarCertification` 7709 glyphs against 6233) and one CFF `unembedded` that is
glyph-exact (`vvsummit2022`, 8377/8377).

The original tracks were measured base-against-head with the *same* banked reference
(`/home/user/gate-2f47/ref`) so that only our side could move, all 947 documents, `KEEP_PDFS=0`.
**0 rows differ in any gate column.** The base binary is the main checkout's, built at the same
commit, so nothing had to be rebuilt to get it.

## Finding 1 — an ODF document's embedded fonts were never loaded. Confirmed, and closed.

`git grep font-face-uri -- dotnet/src` returned nothing, and it now returns
`OdfEmbeddedFonts.cs`.

**The rules, each read at the seat.**

- A `style:font-face` that carries a face has one `svg:font-face-src` holding one
  `svg:font-face-uri` per style, naming a package part through `xlink:href` or carrying the
  bytes as an `office:binary-data` child. `XMLFontStyleContextFontFaceUri::endFastElement`
  (`xmloff/source/style/XMLFontStylesContext.cxx`:250-277) decides the container from
  `svg:font-face-format` — absent, `opentype` and `truetype` are a bare sfnt,
  `embedded-opentype` is EOT, anything else warns and is assumed a bare sfnt — and hands the
  stream to `SvXMLImport::addEmbeddedFont` (`xmloff/source/core/xmlimp.cxx`:697-703).
- **The style is read out of the face, not off the declaration.** LibreOffice's own export
  writes `loext:font-style` and `loext:font-weight` on every `svg:font-face-uri`, and
  `XMLFontStyleContextFontFaceUri::SetAttribute` (`XMLFontStylesContext.cxx`:230-237) handles
  `xlink:href` and nothing else. Which of a family's four files is bold is therefore decided by
  that file's own `OS/2`.
- **The face is registered under its own typographic family**, falling back to the document's
  `svg:font-family` only when it has none:
  `EmbeddedFontsManager::addEmbeddedFont` (`vcl/source/gdi/embeddedfontsmanager.cxx`:355-362,
  tdf#172647) calls `font.getTypographicFamilyName()`, which is name 16 → name 1 → PostScript
  name (`vcl/source/font/TrueTypeFont.cxx`:140-145). `OpenTypeFace.FamilyName` already answers
  the first two of those three.
- **An external URL is not fetched.** `handleEmbeddedFont` warns *"External URL for font file
  not handled"* (`XMLFontStylesContext.cxx`:305) and does nothing, which is also this tree's
  standing rule for an `xlink:href` with a scheme.

**Reach, censused rather than assumed** (`census.py`): **6 of 302 `.odp`**, and **zero of 338
`.odt` and zero of 307 `.ods`** — the corpus's embedding all comes from `.pptx` sources carried
through LibreOffice's own ODF export. The six:

    Liturgical-Commission-2025-Convention-Presentation.odp   Play
    Tax factsheet 2022 (1).odp                               (Liberation/Carlito, all installed)
    Session-1-Presentation-Reporting-Forms-Form-12-final.odp  Montserrat
    Sean Monogue.odp                                          Verdana ×4
    servicedesk-plus-overviewfinal.odp                        Roboto ×5
    Ramp Up Campaign - French.odp                             Alegreya Sans ×3

**`pdffonts` over all six, ours against 26.2.4.2's: 6 of 6 face sets identical**, after being
2 of 6 before. Neither Verdana nor Alegreya Sans, Montserrat, Roboto nor Play exists on this
system (`fc-list` finds none; `fc-match` answers `DejaVuSans.ttf`).

### The half of it that was not in the ODF readers at all

Wiring the reader made `Sean Monogue` name Verdana and **not embed it**: the gate row went
`words` → `unembedded`, which is a different failure and a worse one. The subsetter was
returning null for all four `Font_Verdana_*.ttf`.

`FontSubsetter` named the tables it *dropped* — `GSUB`, `GPOS`, `GDEF`. LibreOffice does the
opposite: `PhysicalFontFace::CreateFontSubset` (`vcl/source/font/PhysicalFontFace.cxx`:546-562,
*"Keep only tables needed for PDF embedding, drop everything else"*) inverts
`HB_SUBSET_SETS_DROP_TABLE_TAG` and deletes fourteen tags from it. The difference is not weight:

> **A zero-length `hdmx` makes `hb_subset_or_fail` fail outright.**

Measured on `Font_Verdana_1.ttf`, whose directory carries `VDMX` at length 0 and `hdmx` at
length 0 — subsetting returns null as it stands, and rebuilding the same file without one table
at a time gives `hdmx` **3780 bytes**, `VDMX` null, `LTSH` null. All four Verdana faces subset
after the drop set is inverted, at 10 240–10 432 bytes. A failed subset is a face this writer
*names and does not embed*, so the defect's symptom is a PDF that draws the reader's own
substitute for a family the document carried with it — and only the gate's `unembedded` column
can see it.

The keep list is LibreOffice's, tag for tag, with `cmap` deliberately not on it:
`WithIdentityCharacterMap` writes the one a PDF simple font needs afterwards, and
`SfntTables.Replace` inserts a table that is absent.

## Finding 2 — the inter-paragraph excess. **The brief's cause is refuted; the measurement is not.**

The brief offered three candidates and asked which: *we add space the reference does not*, *the
reference collapses adjacent `fo:margin-bottom`/`fo:margin-top` where we sum them*, or *the
reference is shrinking to fit and we are not*. **None of the three.** It is one line of the
wrong size:

> **An empty paragraph's line is as tall as its own empty `text:span`, and we measured it
> against the shape's default instead.**

LibreOffice's export writes an empty line as `<text:p><text:span text:style-name="T15"/></text:p>`
— a span carrying the character formatting and no characters. EditEngine builds that line's
dummy portion from the character attributes at the paragraph's own position and gives it
`ImplCalculateFontIndependentLineSpacing(aTmpFont.GetFontHeight())`
(`editeng/source/editeng/impedit3.cxx`:1896-1902), so the span's size is what sets the height.
`OdfTextBody.Paragraph` fell back to `Run(file, cascade, 0, 0)` — the *paragraph's* cascade —
which on a presentation placeholder is the placeholder's default.

The excess is therefore `1.2 × (default − span)` per empty paragraph. On `0335fab9` page 6 that
is `1.2 × (32 − 16) = 19.2`, and the brief measured 19.2 on every gap. On `Sean Monogue` page 8
it is 16.8 against a measured 17.2.

**Measured against 26.2.4.2, four one-attribute variants of that slide** (`e*`/`f*`/`g*` in
`empty-paragraph-variants.py`), reading baselines out of the PDFs:

| the empty paragraph holds | 26.2.4.2 | ours before | ours after |
|---|---:|---:|---:|
| a 16 pt empty span | 23.19 | 42.41 | **23.19** |
| a 40 pt empty span | 51.98 | 42.41 | **51.98** |
| an 8 pt empty span | 13.58 | 42.41 | **13.58** |
| nothing at all (bare `<text:p/>`) | 42.41 | 42.41 | **42.41** |

The bare case is the control that hides the defect: there the paragraph's default *is* right and
both renderers already agreed, which is why comparing an empty paragraph against no paragraph
could never have found this.

**Which span, when there are several, is the last one entered** — not the largest and not the
outermost. 26.2.4.2 answers 8 pt for a 40 pt span followed by an 8 pt one, 40 pt for the reverse
order, and 8 pt for an 8 pt span nested inside a 40 pt one. The first cut of the fix recorded the
span after descending into it, which gets the nest wrong; it records it before descending now.

`0335fab9` page 6, baselines in points from the page top, after the fix against the reference:

    ours  80.674 123.449 170.617 189.808 208.998 228.189 274.564 297.751 344.126 …
    ref   80.674 123.449 170.617 189.808 205.795 221.783 268.157 291.345 337.720 …

Every pitch that does not cross a hyperlink now agrees to 0.001 pt (47.169 against 47.168,
19.191, 46.375, 23.187), and the page draws 15 lines where it drew 13. `Sean Monogue` page 8's
inter-paragraph gap goes **64.318 → 47.509 against the reference's 47.452**, and its fifth block
— which had been drawn as a column of two-character fragments — is drawn.

### The residual on that page, and it is a different rule

Two pitches still disagree: within a paragraph that contains a **hyperlink**, the reference's
line pitch is 15.99 and ours is 19.19. That is not a spacing rule at all —

> **A `text:a` in slide text is an EditEngine *field*, and a field portion is the one portion
> kind that is never given the fixed cell height.**

`ImpEditEngine::CreateLines`' `EE_FEATURE_FIELD` branch sets the portion's size straight from
`QuickGetTextSize` (`impedit3.cxx`:1100-1104) and, unlike the ordinary text branch twenty lines
below it (`:1256-1259`), never calls `ImplCalculateFontIndependentLineSpacing`. Established by
variant rather than by reading: stripping the `<text:a>` elements from `0335fab9` and keeping
their text makes 26.2.4.2 draw **every** pitch at 19.191, and the tree then agrees with it line
for line on the whole page except across the empty paragraphs. `spacing-variants.py` `v1-no-links`.

The same fact explains the brief's *"the reference breaks a long URL mid-token at any
character"*: an over-long field is broken at **cell boundaries** by
`xBreakIterator->nextCharacters(…, SKIPCELL, …)` into an `ExtraPortionInfo::lineBreaksList`
(`impedit3.cxx`:1131-1200), which is a break opportunity at every character. It is not a URL
rule and not a hyphenation rule; it is what a field does. **Reach: 485 `text:a` in 107 of the
302 `.odp`.** Left, with its seat named: it needs a portion kind in `SlideTextLayout`, which is
more than this round should change under a font fix.

### And a placement rule that a hand-built probe would otherwise get wrong

`style:font-independent-line-spacing` is honoured **only** on the shape's
`draw:text-style-name` paragraph style, not on the paragraphs' own `text:style-name`. The flag
is EditEngine-wide (`SetFixedCellHeight`), so it belongs to the shape's text. Measured: the same
four-slide fixture with the attribute on the paragraph style comes back
53.660 / 18.028 / 35.773 / 18.028 from 26.2.4.2, which is ascent + descent; moved to the frame's
text style it comes back 57.572 / 19.162 / 38.381 / 19.162, which is 1.2 em and is what this
tree draws. **This tree reads it from either place**, so it is presently more permissive than
26.2.4.2; nothing in the corpus distinguishes them, because LibreOffice's export writes the
attribute on both. Recorded rather than changed.

## Finding 3 — hyperlink underlines. Confirmed, and it was one missing argument.

`OdfTextFormat` has resolved `style:text-underline-style` and `style:text-line-through-style`
since it was written; `OdfTextBody.Run` passed on neither. No ODF slide had ever had a rule
drawn on it.

Counted in the PDFs' own marks — fills **and strokes** under 2.5 pt tall and over 20 pt wide,
because 26.2.4.2 strokes its rules and this tree fills them:

| page | before | after | 26.2.4.2 |
|---|---:|---:|---:|
| `0335fab9` p6 | 0 | **8** | 9 |
| `medicines_bulletin` p1 | 0 | **8** | 8 |

`0335fab9` is 8 against 9 because the reference's URL breaking gives that page one more link
line, which is the field rule above and not the decoration.

**There is no implicit rule to model.** ODF states the decoration explicitly on the link's own
text style — all four span styles wrapping a link on `medicines_bulletin` page 1 carry
`style:text-underline-style="solid"` — so DrawingML's *"a hyperlink is underlined unless
`a:rPr/@u` says otherwise"* has no ODF counterpart.

## What this brief got wrong

1. **Finding 2's cause.** All three candidates the readers left open are refuted. Nothing is
   added, no margins are summed that the reference collapses, and no shrink-to-fit is involved:
   `0335fab9`'s subtitle placeholder declares no `style:shrink-to-fit` at all and neither does
   its parent. The defect is one empty line measured at the wrong size.
2. **"The line wrapping matches the reference word for word."** Four readers reported this and
   it is false on `0335fab9` page 6, in both directions: the reference fits
   `…leadership. https://www.nhsggc.org.uk/working-with-us/` on one line where we end the line at
   `leadership.`, because it breaks a field at any character. The conclusion the readers drew
   from it — that text measurement is right and only the vertical arithmetic is wrong — happened
   to be correct anyway.
3. **"Findings 1 and 3 move few or no gate rows."** Finding 1 moved four rows on its own and
   Finding 3 none; but Finding 1 also moved a row from `words` to `unembedded` until the
   subsetter was fixed, which is a gate column that had never been exercised by an embedded ODF
   face.
4. **The reach figure "6 of 302 `.odp`, 50 embedded faces"** is right about the documents; the
   50 is the count of `font-face-uri` *occurrences* in `Sean Monogue` alone across `content.xml`
   and `styles.xml`, and the document embeds nine files.

## What is left, with its seat

- **A `text:a` is a field, and a field's portion takes neither the fixed cell height nor a
  word-boundary break.** `impedit3.cxx`:1100-1200. Reach 485 in 107 of 302. It needs a portion
  kind in `SlideTextLayout`.
- **Bullet glyphs**: the reference draws green check marks and a green square sub-bullet on
  `redac-sas` p7 and gold squares on `Sean Monogue` p8 where we draw undifferentiated dots. Not
  investigated this round.
- **A slide background's decorative arc-work and its gradient** on `Sean Monogue` p8. Not
  investigated.
- **A colour emoji rendering grey-brown.** Not investigated; the tree implements CBDT/CBLC, so
  the brief's own guess that it is font *selection* rather than painting is the place to start.
- **`style:font-independent-line-spacing` is honoured from a paragraph's own style here and not
  by 26.2.4.2.** No corpus document distinguishes them.

## The scripts

| | |
|---|---|
| `ref-render.sh` | one document through 26.2.4.2, with the hex-digest profile and `timeout -k` |
| `spacing-variants.py` | five one-attribute variants of `0335fab9`, rendered and read back |
| `empty-paragraph-variants.py` | the ten empty-paragraph probes, both renderers, `10/10` exact after the fix |
| `baselines.py` | the drawn baselines of one page and the pitch between them |
| `count-rules.py` | the thin wide marks on one page, fills and strokes |
| `census.py` | `svg:font-face-uri` and `text:a` reach over the converted corpus |
| `sweep-ours.sh` | our half of one extension column, scored against a banked reference |
