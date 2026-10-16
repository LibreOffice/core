# Pages where the reference rasterises and the word gate cannot be won

**Check this list before working any word-count failure.** Several agents have each spent part
of a round re-deriving that some page belongs to this class.

LibreOffice sometimes **rasterises** an embedded object instead of playing it, so its PDF holds a
picture where ours holds real, searchable glyph runs. The rendered pages look alike —
`pdf-image-diff.py` scores them near-identical — but `pdftotext` reads our text and finds nothing
in theirs. **Our output is the better one**, and the word gate scores it as a failure. Driving
those numbers down would mean drawing less text, which is the wrong direction.

Regenerate with:

```sh
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /c/sandbox/workdir/sample-files out
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /c/sandbox/workdir/sample-files out --documents-only
```

Machine-readable copy: `dotnet/raster-ceiling-pages.tsv`.

## The threshold is a bar, and pages sit just under it

`8_P-Pavese`'s **page 6 carries the same 692×240 raster as its page 5** and is absent from the
table below only because +44 words on 180 is 24.4% against the 25% bar. That is a property of
my threshold, not of the document.

So the list under-counts, and by an unknown amount. Treat a document with one flagged page as
likely to have neighbours just under the bar, and re-measure rather than assuming the table is
exhaustive. Raising the bar is not the fix — it would start excusing real defects, which this
file has already done once.

### Why they sit under it, re-measured — and it is not the threshold

Round `slides-b008-01` took that example apart and **the sentence above has the wrong cause.**
The figure has moved (+32 on 169, 18.9%, not +44 on 180, 24.4%), and the reason page 6 misses
the bar is not that it is a near-miss. It is that **condition 3 is evaluated on the *net*
per-page delta, so a defect of ours on the same page subtracts from the ceiling's excess and
pushes the page under the bar.**

| `8_P-Pavese…pptx` | gross ceiling excess | ref raw words | gross % | our own defect there | net % | flagged |
|---|---:|---:|---:|---:|---:|---|
| page 5 | +44 | 70 | 62.9% | 0 | 62.9% | yes |
| page 6 | +44 | 169 | **26.0%** | −12 | **18.9%** | no |
| page 16 | +59 | 200 | **29.5%** | −25 | **17.0%** | no |

Both unflagged pages are **over** the bar on gross excess. So the two under-counts are not
independent — **the flag is suppressed exactly on the pages that are both a ceiling and a
defect**, which is not a rare combination: two of that document's three ceiling pages are it.

**If this is ever fixed, the fix is to compute condition 3 on the ours-only token count rather
than on the net delta**, which flags pages 5 and 6 correctly without touching the 25% bar. (Page
16 would still be missed, for an unrelated reason — see *the reference outlines its glyphs*
below.)

### Pages under the bar, measured

Recorded here rather than in the main table so the flag's own arithmetic stays reproducible.
Gross excess is the ours-only token count; net is what condition 3 currently sees.

| Document | Page | gross | net | ref raw | gross % | why it is under |
|---|---|---:|---:|---:|---:|---|
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 6 | +44 | +32 | 169 | 26.0% | our table row 12 pushed off the page cancels 12 |
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 16 | +59 | +34 | 200 | 29.5% | our unwrapped axis labels cancel 25 — **and it is outlining, not a raster** |

## How a page earns its flag

Three conditions, on a document whose page count already agrees:

1. The reference draws a raster on that page.
2. **We do not draw that same raster.** Matched on dimensions.
3. We extract materially more words there than the reference does — at least 8 more and at
   least 25% more, about two-thirds of a line of prose.

**Two known blind spots in that test, both measured, both on the same document.** Condition 1
asks `pdfimages`, so it cannot see the reference *outlining* glyphs into filled paths — ink with
no text layer and no image. And condition 3 is computed on the **net** per-page delta, so a
defect of ours on the same page subtracts from the ceiling's excess and pushes the page under
the bar. Both are worked out below on `8_P-Pavese…pptx`, which has three ceiling pages and one
row in the table.

**Condition 2 was missing until an agent disproved four of this file's own rows.** Without it,
the first condition is satisfied just as well by a logo *both* renderers draw: four pages of
`UG.CAO.00133` were flagged on a 162×109 JPEG of the EU flag in the footer, identical on both
sides, while the document's real surplus was a header block drawn on 13 of its 18 pages. (That
header block was **fixed on 2026-08-15** — the reference does not inherit a header holding no
paragraph of its own, and we did; the document is now `18/18 3674/3667`, a `match`. See
`SectionInheritedHeaderTests`.) The
signature misfires on any document that puts a small picture in its page furniture and has a
furniture defect elsewhere. Adding it removed **16 of 53 pages — nearly a third of the list.**

Matching on dimensions rather than on content is deliberate: a rasterised metafile and a logo
differ in size by orders of magnitude, and decoding every image to compare pixels would cost more
than the whole scan.

### The 25% threshold is excluding pages of this list's own class

Measured in round seventeen on `slides/batch-008/…/8_P-Pavese…pptx`. `pdfimages -list` shows the
reference drawing the **same 692x240 JPEG with a soft mask on pages 5 and 6**, and us drawing
neither; page 5 is on the list at +44 on 70 and page 6 is not, at +44 on **180 — 24.4%**. Page 16
of the same deck sits at +23% and is a different defect entirely (see below). So condition 3's
"at least 25% more" is not separating the class from anything here; it is dropping half of one
document's instances of it.

Either lower it, or say in this file that the list is a deliberate under-count and that a page
just under the bar should be checked with `pdfimages` before being treated as a defect. Note the
consequence for that document: it fails the word gate at 2240 against 2108, and **excusing only
its listed page leaves it at 2152, still outside the 2% band** — so a reader working from this
list alone would conclude the residue is ours when two-thirds of it is not.

> **Re-measured in round `slides-b008-01`, and three of the sentences above are now wrong.**
> The gate row is **2118 against 2010**, not 2240/2108 (that pair was the `wc -w` metric, before
> the 2026-08-13 change). Page 6's 24.4% is **18.9% net and 26.0% gross** — it is over the bar
> and pushed under it by a defect of ours on the same page, not a near-miss; see *Why they sit
> under it* above. And **page 16 is not "a different defect entirely"** — it is a ceiling too,
> of a fourth kind that `pdfimages` cannot see at all. Excusing all three ceiling pages leaves
> the document at 1973 against 2010, **inside** the band. The instinct in the last sentence was
> right and the arithmetic under it was not: the residue is not two-thirds ours, it is
> **negative** — we are 40 words *short*. Worked example at the end of this file.

## The numbers

| | |
|---|---|
| pages flagged | **36** across 20 documents |
| by track | 27 slides, 8 words, 1 sheets |
| flagged pages whose document embeds a metafile | 21 |
| flagged pages whose document embeds **none** | 16 |
| excess words accounted for | **2706** |
| documents embedding a metafile at all | 100 of 534 |
| documents that cannot be judged yet | 83 |

An embedded metafile is the commonest cause and not the only one. `W3_Case_Study…ppt` holds none
and its page 10 is squarely this class — the reference draws there the same 845×572 object it
draws on `Thailand17.ppt`'s page 8. **The flag keys on the observable signature; the metafile
count rides along as an attribution.** An earlier version filtered the page scan down to metafile
carriers and hid nearly half the list that way.

The scan also could not originally see a metafile in a binary document at all: a `.ppt` keeps its
pictures zlib-compressed inside Escher blip records, so a raw signature search finds nothing in a
file that plainly contains one. Inflating every plausible stream took the carrier count from 76
to 100.

## A row struck out: `approvals-and-standardisation-…` page 6 was never a ceiling

Recorded 2026-08-14 by round `words-table-01`, and worth reading as a caution rather than as one
correction. That row claimed **+38** ours-only words on page 6 and carried an em dash in the
metafile column — no metafile, which already put it among the sixteen flagged pages that have
none.

It was not a ceiling at all. Both sides draw the **same 47×90 JPEG** on that page, and neither
side rasterises anything else there. The whole +38 was **rotated table-cell text drawn upright,
one glyph per line** — `w:textDirection="btLr"` was unimplemented, so twelve labels that should
turn a quarter turn were set as vertical stacks of single characters, and `pdftotext` scored each
character as a word.

Implementing the property closed it exactly:

| page 6 | ours before | ours after | reference |
|---|---:|---:|---:|
| words | 157 | **121** | **121** |
| raw `wc -w` | 160 | 123 | 123 |

Zero excess, not a residue. The document as a whole moved from `words` to `match`.

**The general lesson, which the file's own preamble half-states and this makes concrete.** The
generator's condition is "our page holds tokens the reference's does not", and *any* defect that
manufactures tokens satisfies it. A raster ceiling is one cause of that condition; it is not the
only one, and the flag cannot tell them apart. Sixteen of the thirty-seven flagged pages have no
metafile, and this was one of them — so **an em dash in the metafile column is a reason to
re-derive the row, not a reason to trust it.** Confirm the ceiling in the two PDFs' own image
lists before excusing a page on the strength of this table.

## Two boundaries worth stating

**A flagged page does not excuse its document, and the two can point opposite ways.** Re-measure
before subtracting. This file's own worked example inverted once already — `UG.CAO.00133` was
recorded as 225 words short overall and later measured +245 over — before turning out to be a
false positive entirely.

**Eighty-three documents cannot be judged.** A per-page comparison is meaningless while the page
counts disagree, so those are an honest **unknown** rather than a pass. Fix their pagination
first, then re-run.

### One of those eighty-three has since been decided: `150_5300_13_chg10.doc`

`raster-ceiling-pages.tsv` carries it as `pagination-differs`. Round `words-tabs-01` closed that
pagination (78 pages against 77, now 77/77), which makes the per-page comparison legal, and it is
**ceiling**: `pdfimages -list` shows the reference drawing a full-page 300 dpi JPEG on pages 27,
29, 30, 31 and 32 — the taxiway-geometry figures — where we play the embedded metafile and emit
its labels as real text. Pages 1–28 measured together are **142 words short**; the whole document
is **494 over**, so every word of the excess and more is on those five pages. `words` is the only
column it still fails and it cannot be won.

### And its neighbour has now been decided too: `150_5300_13_chg8.doc`

This file recorded it as "not decidable yet — its pagination still differs, for an unrelated
reason (a tracked-changes duplication worth about 190 words)". **That reason is gone.** The WW8
round read `fRMView` off the `Dop` and stopped drawing tracked deletions in a document that hides
them, which took chg8 from 19 pages to 18 against the reference's 18, so the per-page comparison
is legal now.

Measured per page, the excess is **+172 on a band of 170.24** — it fails by 1.76 words — and it
is not spread:

| page | ours | ref | delta | what |
|---:|---:|---:|---:|---|
| 8 | 162 | 30 | **+132** | `pdfimages -list` shows the reference drawing **2 images** and only the caption, "Figure 3-5. Obstacle Free Zone (OFZ) for Runways…", where we play the embedded metafile and emit its labels — `INNER-APPROACH OFZ`, `DISPLACED THRESHOLD`, `RUNWAY OFZ` — as real searchable text |
| 15 | 75 | 6 | +69 | **not a ceiling**: neither side draws an image there. The reference leaves page 15 nearly blank (`AC 150/5300-13 Appendix 2 292 9/29/89`) and we fill it, compensated by −59 and −60 on pages 13 and 14 — a local pagination shift inside the appendices that nets out |
| others | | | −29 | noise |

**Page 8 alone decides it.** Subtracting its +132 leaves the document at **+40 against a 170.24
band**, comfortably inside — so `words` is the only column it fails, the failure is the ceiling,
and it cannot be won. Ours is the better output on that page: the reference's figure is a picture
and ours is text a reader can search.

The page-15 shift is a real difference and is *not* excused by this. It costs no gate column today
(the pages either side cancel it) and should be looked at on its own terms rather than folded into
the ceiling.

## A third: EMF metafiles on slides

`slides/chart-001/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`, 2122 words against 2010 and
26 of 26 slides. Decided 2026-08-16, and it is the ordinary raster ceiling on the slides track
rather than a new mechanism — recorded because the *evidence* runs the opposite way from the word
documents: there `pdfimages` finds the reference's picture on a page we fill with text, and here it
finds pictures where our page has **none**.

Three slides embed an EMF — `image7.emf` on slide 3, `image8.emf` on 5 and `image9.emf` on 6 —
and LibreOffice rasterises each one on export while we play the metafile and emit its labels as
real searchable text:

| slide | ours | ref | delta | ref images | our images | ref ink | our ink |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 3 | 63 | 60 | +3 | 2 | 1 | 9.07 % | 8.75 % |
| 5 | 100 | 57 | **+43** | 2 | 0 | 5.50 % | 5.01 % |
| 6 | 196 | 165 | **+31** | 2 | 0 | 11.36 % | 11.17 % |

The ink agrees to half a percent on every one, so both renderings draw the figure; only ours is
text. Slides 5 and 6 carry the same journal-article extract because their two EMFs are
byte-identical in size, and our extra tokens on both read `study of of telephone calls to a a
hotline` — the doubling is the metafile drawing overlapping runs, not a duplication defect.

**The ceiling decides the document.** The fourth differing slide is 16 at +35, which is the chart
category-axis tokenisation of task #73 and not a ceiling; +3 +43 +31 +35 is the whole +112.
Subtracting only the three EMF slides leaves **+35 against a 40.2 band**, inside it — so `words` is
the only column it fails and it cannot be won.

## A second kind of ceiling: OMML formulas

Everything above is about **rasters** — LibreOffice drawing a picture where we draw text.
`ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx` is the same trade made by a different
mechanism, and it is worth naming separately because no image is involved and `pdfimages -list`
reports nothing on either side.

The document holds **87 `m:oMath` and 33 `m:oMathPara`** formulas. LibreOffice imports OMML into
its Math component and draws the result with **no extractable text at all**; we draw real
searchable text. Measured on page 4, where a symbol table gives the cleanest view:

- Both renderings put `=` at x≈244.6 and `[1/rad]` at x≈519.5 — **the formula's slot is identical**,
  so this is not a width or layout difference.
- The reference leaves the 269.5 pt between them **empty of text**; we write `dcLdα` into it.
- Ink agrees to a tenth of a percent — **7.251 % against 7.359 %** — so both genuinely *draw* the
  formula. Only ours is text.
- `pdffonts` shows the reference embedding five faces and ours those five plus DejaVu Serif, the
  face our formula text resolves to.

Across the whole document **156 tokens are ours alone against 39 the reference's alone**, which is
**0.97 per formula site** over the 120 sites — the entire +109 excess, against a 74.40 band.

**`words` cannot be won on this document**, and ours is the better output: a reader can search our
formulas and cannot search LibreOffice's.

**It also fails `pages`, and that is NOT excused by this.** A ceiling explains an
extractable-text difference, not a pagination one.

***Established, and half of it is closed.*** *Where the pagination diverges is the OMML line
height, and the page-4 row pitch that said otherwise was measured on the one page whose formulas
are all single rows — the rule is that the reference reserves the StarMath **object's** height and
we reserved our own text's, which agree for a bare `x` and are 15 pt apart for a fraction.
`OfficeMathBox` is that height; the document goes **14 pages to 15** against the reference's 16,
pages 1 to 10 now align, and the near-empty page 7 the reference draws is drawn. See
`probes/mathheight-r165/results.md` and `probes/blankpage-r163/results.md` §4. **The remaining page
is a different defect** — the reference keeps the gust-load table whole on its page 12 and we split
it across ours 11 and 12 (`blankpage-r163` §6) — and the `words` column is still a ceiling and
still cannot be won.*

One real defect was found and fixed while measuring this and is *not* part of the ceiling: the
document's heading list carries no `w:start`, whose default is zero rather than one, so we numbered
every section one higher. That is worth −8 words and no pages.

## The flagged pages

| Document | Page | ours | ref | excess | metafile |
|---|---|---|---|---|---|
| `words/batch-016/…/AFS-050-004-F2_0i.docx` | 3 | 419 | 53 | +366 | — |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 5 | 429 | 162 | +267 | 2/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 10 | 281 | 30 | +251 | 2/0 |
| `slides/batch-014/…/N2_E_Maestroni_Swarm_COP.pptx` | 7 | 307 | 102 | +205 | — |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 18 | 418 | 224 | +194 | 6/0 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 43 | 396 | 229 | +167 | 6/0 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 29 | 109 | 1 | +108 | 2/0 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 7 | 261 | 158 | +103 | 2/0 |
| `slides/batch-010/…/W3_Case_Study_of_a_Tsunami_Warning_Simulation_` | 10 | 102 | 9 | +93 | — |
| `slides/batch-014/…/Thailand17.ppt` | 8 | 102 | 9 | +93 | 6/0 |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 2 | 254 | 187 | +67 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 21 | 78 | 23 | +55 | 0/1 |
| `slides/batch-010/…/Fundamentals_Module_1_basics.ppt` | 6 | 70 | 20 | +50 | 1/0 |
| `words/batch-011/…/f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 | 181 | +49 | 0/3 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 28 | 53 | 5 | +48 | 0/1 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 76 | 97 | 51 | +46 | 6/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 9 | 96 | 50 | +46 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 45 | 50 | 5 | +45 | 0/1 |
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 5 | 114 | 70 | +44 | 3/2 |ᵃ
| `slides/batch-017/…/Demick_JetBlue.pptx` | 5 | 133 | 54 | +79 | — |ᵇ
| ~~`words/batch-015/…/approvals-and-standardisation-organisation-app`~~ | ~~6~~ | ~~161~~ | ~~123~~ | ~~+38~~ | — | **false positive — see below** |
| `sheets/batch-010/…/TOGAF9-Tool-ConfReqts-CSQ.xls` | 21 | 69 | 31 | +38 | — |
| `slides/batch-014/…/Structural Testing.pptx` | 19 | 37 | 5 | +32 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 44 | 34 | 4 | +30 | 0/1 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 46 | 35 | 5 | +30 | 0/1 |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 7 | 127 | 64 | +63 | — |ᵇ
| `slides/batch-017/…/Demick_JetBlue.pptx` | 4 | 163 | 79 | +84 | — |ᵇ
| ~~`slides/batch-016/…/FAAAIandtheArtandScienceofV&Vfinal.pptx`~~ | ~~14~~ | ~~119~~ | ~~91~~ | ~~+28~~ | ~~1/1~~ | **false positive — WordArt, see below** |
| `slides/batch-014/…/Intersil_Italy_CAN_Bus_Transceiver_Presentatio` | 30 | 130 | 103 | +27 | 6/0 |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 38 | 84 | 63 | +21 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 2 | 38 | 27 | +11 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 13 | 41 | 31 | +10 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 14 | 46 | 36 | +10 | — |
| `slides/batch-004/…/ws_prod-g-doc-Events-industrymeeting18112004-E` | 9 | 38 | 29 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 26 | 30 | 21 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 39 | 38 | 30 | +8 | — |

ᵃ This document has **three** ceiling pages, not one — 5, 6 and 16. See *Why they sit under it*
above and *the reference outlines its glyphs* below. Its full accounting is in
`probes/slides-b008-01/results.md`.

ᵇ Re-measured 2026-08-15 (`slides-verify-01`), and **the mechanism on these three rows is
outlining, not rasterisation** — the em dash in the metafile column was the tell and this file
already says to treat it as one. See the audit section above. The `ours` figures rose because
our own output did; the reference's 54 / 64 / 79 are unchanged.

**The other thirteen slides rows in this table reproduce exactly.** Every listed page of the
fourteen `slides/ceiling-*` documents was re-measured with the generator's own raw metric on
2026-08-15: thirteen are identical on both columns and the three above moved on ours only. So
this table's slides half has aged far better than its preamble's shelf-life warning implies —
what has aged is the *attribution*, not the counts.

## Audited 2026-08-14 against 26.2.4.2 — one row was inverted, and it was not a near-miss

**Every one of the 37 rows was re-measured**, per page, with the script's own metric
(`pdftotext -f N -l N | split()`, 1-based pages) against the banked
`refpdfs-26.2.4.2-fonts/` references. Round `slides-missing-01`; the sweep is in
`probes/slides-missing-01/results.md`.

**`NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` page 5 was in this table with its sign the
wrong way round, and has been removed.** The row read *ours 15 / ref 5* — the classic
over-drawing signature. Measured on the unfixed tree it was **ours 5 / ref 72**: we were not
over-drawing there, we were drawing an entirely empty subtitle placeholder on three of the
deck's slides. A real missing-content defect had been filed in the list of pages that cannot be
won, which is the most expensive possible place to put one. With the fix
(`SlideAutofit`'s `FitFloor`) the page reads **ours 84 / ref 72** and the document matches the
gate outright at 644/638 words, so it is no longer a ceiling page under any reading.

The most likely account of how the row got its numbers is that it was measured against
**24.2.7.2**, whose autofit was the same unbounded search we had ported — so the *reference*
drew almost nothing there too, and 15/5 was true of a pair of renderings neither of which
exists any more. That cannot be verified here: the 24.2 binary is not installable in this
container.

**No other row has flipped sign.** All 36 survivors still have ours ≥ ref. Four, however, no
longer clear the flagging threshold (≥8 extra words *and* ≥25% of the reference's count) and
should be re-checked before anyone leans on them:

| document | page | as filed | measured 2026-08-14 |
|---|---:|---|---|
| `f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 / 181 | 197 / 181 |
| `EHEST-SMS-Safety-Management-Manual-V2.docx` | 43 | 396 / 229 | 231 / 229 |
| `introduction_to_bea_tuxedo.ppt` | 26 | 30 / 21 | 23 / 21 |
| `introduction_to_bea_tuxedo.ppt` | 38 | 84 / 63 | 72 / 63 |

They are left in place rather than removed: below-threshold is not the same claim as
not-a-ceiling, and none of the four was validated end to end in that round. **The general
lesson is that this table has a shelf life.** Its inputs are our renderer *and* the reference
binary, both of which move, and a stale row here does not merely go unused — it actively tells
the next reader not to look.

## Audited 2026-08-15 — a second row was a defect of ours, and condition 1 is nearly inert

**`FAAAIandtheArtandScienceofV&Vfinal.pptx` page 14 was never a ceiling.** Its +28 was our own
surplus: the deck's gauge carries five labels in `a:prstTxWarp` text boxes, which LibreOffice
puts into Fontwork text-path mode and draws as filled outlines carrying no glyphs, and which we
drew as ordinary text. Round `slides-extra-01`; the fix is a single gate in `PptxSlideLayout`
and the document now matches the word gate outright at 1135/1133. Re-derived with the
detector's own functions, both pages now read +1 and neither flags.

So this is the *fake a ceiling* direction of the condition-3 blind spot, and it is the more
dangerous one: a page can be listed here purely because of a defect of ours, and the row then
tells the next reader not to fix it. It cost this one nothing only because the round was
briefed to disbelieve it.

**Page 13 of the same document carries the identical defect and was never listed, and the
reason is not the net-delta blind spot documented above.** Nothing cancels anything there —
gross and net are both +28. It is the **25% ratio bar**: the reference has 120 words on page 13
and 89 on page 14, so the floor is 30.0 on one page and 22.2 on the other, and the same +28
clears one and not the other. A defect of a fixed size is therefore flagged or not according to
how much *other* text the page happens to carry.

**Condition 1 barely discriminates at all.** Measured over the whole slides track — all 163
documents, 4199 pages, our renderings against the banked `refpdfs-26.2.4.2-fonts/`:

| | pages | share |
|---|---:|---|
| pages compared | 4199 | |
| condition 1 fires — the reference draws a raster there that we do not | **1482** | 35.3% of pages |
| …of those, where our word count **equals** the reference's exactly | **1091** | 73.6% of them |
| …of those, that also clear condition 3 and would flag | 26 | 1.8% of them |

Condition 1 fires on a third of every slide page in the corpus, and on three-quarters of those
there is no word difference whatever — the dimension match is answering "the two renderers
scaled or tiled some picture differently", not "the reference rasterised an object". On the
FAAAI document alone it fires on **23 of 30 pages**, 21 of them with a zero word delta.

The consequence is that **the flag is condition 3 almost by itself**, and condition 3 is a
statement about our own surplus rather than about the reference. Any defect that adds words to
a page has a better-than-one-in-three chance of finding a condition-1 page to sit on and being
recorded here as unwinnable. That is a stronger reason to compute condition 3 on the ours-only
token count — as the note above already recommends — and to add a condition that says something
about the reference's *ink*: a page where the reference draws a raster **and has materially less
text than the shape it covers would imply** is a ceiling; a page where the reference merely
draws a picture is every other page.

## Audited 2026-08-15 — all fourteen unverified `slides/ceiling-*` documents, re-measured

Round `slides-verify-01`. Every figure below is from **this** container: LibreOffice 26.2.4.2
with `fonts-dejavu-core` present, our tree at `e1630c6f1a6`, the gate's own metric. The
provenance warning further up this file said the *classification* was expected to survive and
every *number* had to be re-measured. Both halves of that turned out to be right, and the second
half turned out to be cheaper than feared: **all fourteen gate rows reproduce their 2026-08-09
figures to the word.** The stale-data risk on this list was real and did not fire.

**The reference is deterministic on all fourteen.** Three independent samples each — the banked
`refpdfs-26.2.4.2-fonts/` render, a fresh `batch-check.sh` conversion, and a third standalone
`soffice --convert-to pdf` — agree on page count and on both word metrics for every document.
The instability class is ruled out for this whole set, so nothing here should be filed
`unstable`.

### The verdicts

`ceilΔ` is the sum of the per-page deltas on the pages the mechanism accounts for; `residue` is
what is left of the gate's gap once those are excused, against the gate's own max(2%, 3) band.

| document | gate | Δ | band | mechanism | ceiling pages | ceilΔ | residue |
|---|---|---:|---:|---|---|---:|---:|
| `W3_Case_Study_…Ed.ppt` | 900/812 | +88 | 16.2 | 1 raster | 10 | +88 | **0** |
| `Thailand17.ppt` | 2736/2625 | +111 | 52.5 | 1 raster | 8 | +88 | +23 |
| `Fundamentals_Module_1_basics.ppt` | 1135/1089 | +46 | 21.8 | 1 raster | 6 | +49 | **−3** |
| `Demick_JetBlue.pptx` | 812/608 | +204 | 12.2 | **2 outlining** | 4, 5, 7 | +204 | **0** |
| `N2_E_Maestroni_Swarm_COP.pptx` | 5285/5126 | +159 | 102.5 | 1 raster **+ 2** | 7 | +160 | **−1** |
| `OnTrac_StarCertificationProgram-3Day.pptx` | 1326/1030 | +296 | 20.6 | 1 raster | 9, 10 | +296 | **0** |
| `16 - UTM - (NASA).pptx` | 2454/2260 | +194 | 45.2 | 1 raster | 7, 29 | +193 | +1 |
| `WiGr_2021W_1_….pptx` | 2154/1958 | +196 | 39.2 | 1 raster | 21, 28, 44, 45, 46, **47** | +201 | **−5** |
| `Sylva%20introduction%20session.pptx` | 1064/1467 | **−403** | 29.3 | 3 ref splits | 13 of 15 | −412 | +9 |
| `pres_ioc_phuket.ppt` | 992/1518 | **−526** | 30.4 | 3 ref splits | 5, 6, 23, 24 | −524 | −2 |
| `architecture6.ppt` | 1926/2544 | **−618** | 50.9 | 3 ref splits | 10, 14, 21, 24, 27 | −618 | **0** |
| `RRM-training-syllabus-Chapter-3-….ppt` | 2475/2576 | **−101** | 51.5 | 3 ref splits | 22 of 27 | −90 | −11 |
| `solog_orientation_august_2019.pptx` | 670/685 | **−15** | 13.7 | 3 ref splits | 8 | −15 | **0** |
| `7-Zulkefli_Part147n66_IKMAS.pptx` | 941/973 | **−32** | 19.5 | 3 ref splits | 3, 4 | −40 | +8 |

**Fourteen of fourteen are confirmed ceilings, and every one of them passes the gate outright
once its mechanism is excused.** Not one is a mislabelled defect. Three carry a real defect
*inside* the ceiling, listed below; none of the three is large enough to change the verdict.

### The direction is not uniform, and the deficits are a different mechanism, not a wrong label

Eight surpluses and six deficits. The brief that commissioned this round was right to single the
deficits out and the suspicion behind it was wrong: **all six deficits are mechanism 3, the
reference splitting its own words, and mechanism 3 is a ceiling.** What was wrong on those rows
was the *mechanism* the file implied — `pdfimages` has nothing to do with any of them — not the
`ceiling` label.

The test is the character stream and it is decisive:

| document | Δ words | ours chars | ref chars | pages whose character multisets are identical |
|---|---:|---:|---:|---|
| `Sylva` | −403 | 5939 | 5939 | **15 of 15** |
| `pres_ioc_phuket` | −526 | 6333 | 6339 | 25 of 26 |
| `architecture6` | −618 | 11048 | 11035 | 29 of 31 |
| `RRM` | −101 | 14493 | 14546 | 25 of 27 |
| `solog` | −15 | 4756 | 4758 | 13 of 15 |
| `7-Zulkefli` | −32 | 5808 | 5772 | 17 of 18 |

`Sylva` is the cleanest instance on the project: **every one of its fifteen pages carries the
identical character multiset on both sides, to the character**, and the gate reads −403.

The operator-level cause, whole-document, from `pdf-ops.py dump` — glyphs per show operator:

| document | ours | reference |
|---|---:|---:|
| `RRM` | 16.28 | **1.34** |
| `pres_ioc_phuket` | 13.32 | **2.26** |
| `solog` | 11.90 | **2.00** |
| `architecture6` | 9.40 | **1.64** |
| `Sylva` | 4.44 | **1.39** |
| `7-Zulkefli` | 4.46 | **1.77** |

On `Sylva` pages 2, 3, 10 and 11 the reference's figure is exactly 1.00 — **482 glyphs in 482
shows, 406 in 406, 655 in 655, 436 in 436**, one show operator per glyph — against our 42, 77,
80 and 71 shows for the same text. `pdftotext` reads each positioned glyph as a token. Closing
any of these would mean shattering our own text layer to match poppler's misreading of theirs.

A blind reviewer given `pres_ioc_phuket` page 24 — the single largest deficit page on the track,
−275 words — with no numbers and no repository access reported the mechanism without being
pointed at it: the reference's text is *"drawn with much wider inter-character spacing"* and
carries *"conspicuously wide letter-spacing"*, while every one of the twelve event lines is
present in both halves. A second reviewer, on `Sylva` page 10: *"nothing is missing from either
half … every word of body copy is present and identical in both"*.

### `Demick_JetBlue.pptx` is mechanism 2, not mechanism 1 — the second instance this file asked for

This file's rows for `Demick_JetBlue` pages 4, 5 and 7 sit in the rasterisation table with an em
dash in the metafile column. **The mechanism is wrong.** Neither side draws any raster those
pages do not share: the 3006×340 and 1563×200 images are the deck's page furniture and appear on
all ten pages, including the six where the word delta is zero — precisely the false-positive
shape the condition-2 note above warns about.

What the reference actually does there is **outline the rotated category-axis labels into filled
paths**:

| page | our surplus | our ours-only characters | reference glyph-sized fills, all `#000000` |
|---|---:|---:|---:|
| 4 | +63 | 126 | **126** |
| 5 | +78 | 157 | **156** |
| 7 | +63 | 126 | **126** |

Twenty-one quarter labels of six characters (`2006-2` … `2012-3`) is 126; twenty-six is 156. Our
side draws zero such fills on those pages and the reference draws none of them on the six pages
where the deltas are zero. The ours-only character multiset is nothing but `0123456789` and `-`.

A blind reviewer given page 5 with no numbers reported the ink as present and legible on both
sides — *"small edge text on the chart is present and legible in both halves and reads
identically"* — while `pdftotext` extracts not one of those characters from the reference. Ink
with no text layer, which is the definition of this shape.

**So the outlining mechanism now has its second instance and can be promoted.** The section
below records it on one page of one document with the caveat *"do not promote it without a
second instance"*. It has one: three pages of a second document, a different deck, a different
chart library path, with the same signature and the same exact arithmetic. `N2_E_Maestroni…`
page 7 is a third, mixed with a raster — the reference draws 173 black glyph-sized fills there
that we do not.

### One page joins the list and none leaves it

`WiGr_2021W_1_….pptx` **page 47** is a ceiling page and was never listed: the reference draws a
530×472 raster and two 427×64 rasters that we do not draw, and we extract 16 words there to its
9. It misses the flag on condition 3's **absolute** bar — +7 where eight are required — while
clearing its ratio bar at 78%. A small page can therefore carry an unmistakable ceiling and
never be listed, which is the other end of the same arithmetic as the 25% bar's known
behaviour.

### Real defects found inside these documents — do NOT mistake them for ceiling

Every one of these was found while auditing, and none of them changes a verdict. They are
recorded here because a row in this file that says *unwinnable* is exactly where a real defect
goes to be ignored.

1. **`16 - UTM - (NASA).pptx` — an all-caps run property we do not apply.** On nine pages (1, 5,
   12, 13, 14, 15, 16, 20, 35) the two character multisets differ **only in case**, ours lower
   and the reference's upper, 369 characters in all: page 1 is
   `aaaabcdeeeefgiiiiilllnnnnoopprrsstttuwy` against
   `AAAABCDEEEEFGIIIIILLLNNNNOOPPRRSSTTTUWY`. Word-neutral in both metrics, and invisible to
   every column of the gate.
2. **`RRM-training-syllabus-Chapter-3-….ppt` — 54 characters of body text we drop.** Page 8's
   last block ends `…and the tug lost` in ours against the reference's `…and the tug lost
   control. The tug was traveling at about 7-9 kmph.”`, and page 20 loses `careful speed`. 11
   words, the only genuine content difference in the document's 101-word deficit.
3. **`pres_ioc_phuket.ppt` — page 16 loses `response mechanisms.`**, 21 characters. Same shape:
   the tail of an overflowing block.
4. **`Fundamentals_Module_1_basics.ppt` page 25 — an auto-numbered list drawn as bullets.** The
   reference sets `1.` … `7.`; we set `•` seven times. Worth **−7 gate words**, and fixing it
   moves this document's column *away* from a pass, which is this file's standing lesson again.
5. **`OnTrac_StarCertificationProgram-3Day.pptx` page 10** — a blind reviewer found two, unled:
   we omit the background's *"faint concentric arc bands"* entirely, and our page number `10`
   sits ~90 px right and ~60 px up of the reference's, far enough that the `1` is clipped by the
   slide edge. Neither moves a word.
6. **`pres_ioc_phuket.ppt` page 24 — we draw no arrowheads** on the two timeline rules the
   reference terminates with filled triangles.
7. **`7-Zulkefli_Part147n66_IKMAS.pptx` page 1** — both sides clip an overflowing text box and
   they clip it at different points, ours 45 characters further in. A real autofit divergence,
   +8 words to us.
8. **`Thailand17.ppt` pages 29 and 35** — identical character multisets, and here it is **our**
   tokenisation that shatters: 14 one-character tokens against the reference's 3 on page 29, 18
   against 7 on page 35. +20 words, and the only item in this audit that points at something
   winnable in our own text layer.

### What this settles about the track

Sixteen slides failures remain, and with `chart-001`'s two re-verified separately, **all sixteen
are now confirmed ceilings on current measurements** — every one page-exact, words-only, and
every one passing the gate once its mechanism is excused. **The slides track has no winnable
gate failure left in it.** What it has left is the eight defects above, none of which the gate
can see, which is the argument for retiring `|words|` as the instrument on this track rather
than for closing it.

## What is known about the mechanism, and what is not

**Established.** The rasterisation happens **upstream of PDF export**. The raster is not in the
file — two `.ppt`s were scanned through every inflated zlib stream, not just their raw bytes —
and it is not the PDF writer, since `implWriteBitmapEx` downsamples only under
`ReduceImageResolution` and the 300 dpi `FLOATTRANSPARENT` branch cannot yield the observed
66–265 dpi. `8_P-Pavese…pptx` slide 5 is a bare `p:pic` over an EMF with 791 `EXTTEXTOUTW`
records, no EMF+, no alpha, no raster-op and no bitmap, and the reference draws a 692×240 raster
with a soft mask.

**Not established.** Which LibreOffice path does it, and whether the metafile-carrying and
metafile-free cases share one. EMF+ is ruled out as the trigger by counter-example — `2014BSA`
slide 5's EMF *does* carry EMF+ and renders as text. `SELECTCLIPPATH` is the standout structural
difference between the two, but that is a correlation on two documents and is **unverified**.

Naming that path would let the flag become a rule rather than a list. Until then this is the
record.

## A second ceiling, with a different mechanism and a named cause

Rasterisation is not the only way the reference draws less than we do. The slides track's
largest single ink figure turned out to be half this, and unlike the rasterisation class the
mechanism is **named and verified** rather than open.

`slides/batch-012/pptx/NAS-Infrastructure-Roadmaps-v16.0.pptx` puts each of its data tables in
a `p:graphicFrame` wrapped in `mc:AlternateContent`:

```xml
<mc:Choice xmlns:v="urn:schemas-microsoft-com:vml" Requires="v">
  <p:oleObj r:id="rId3" progId="Excel.Sheet.12"><p:link/></p:oleObj>
</mc:Choice>
<mc:Fallback>
  <p:oleObj …><p:link/><p:pic>…<a:blip r:embed="rId4"/>…</p:pic></p:oleObj>
</mc:Fallback>
```

`rId3` is an *external* relationship to a SharePoint workbook. `rId4` is `image14.emf`, sitting
in the package, and it is a picture of the table's data.

`oox/source/core/contexthandler2.cxx:238-249` lists the namespaces LibreOffice will take a
`mc:Choice` for, and **`v` is on it** — so LibreOffice takes the Choice, gets a linked OLE
object with no local replacement picture, cannot reach the link, and draws nothing. We do not
claim VML, take the Fallback, and draw the EMF. Ours is the better output by any reading, and
the spec's rule — take the first Choice whose namespaces you understand — is on our side, since
we have no VML reader at all.

Measured, splitting the document's per-page ink by whether the page carries one:

| | pages | ink | major |
|---|---|---|---|
| carrying a `Requires="v"` `p:oleObj` | 24 | **152.12** | 24 |
| everything else | 113 | 73.21 | 42 |

The 152.12 did not move under either of this round's fixes — it is the same figure before and
after, which is what says it is a property of those pages rather than noise.

**Re-derived independently in round fourteen and it reproduces to the digit**: 152.12 on the 24
pages carrying one and 73.28 on the other 113, against the 152.12 and 73.21 recorded here. That
makes this one of the few claims on this track to survive an independent check with its *sentence*
intact as well as its number — the usual result is the reverse.

Round fourteen also took the other half apart, and it is **not** a second discrete defect waiting
to be found. Its worst pages carry none of `p:graphicFrame`, `a:tbl`, `a:blipFill`,
`a:pattFill`, `a:gradFill`, `dgm:relIds`, `a:prstTxWarp` or `a:outerShdw` in any
concentration, and the diff
report calls 40-50% of each one *"marks displaced or reshaped"*: a reflow spread thin over 113
pages at about 0.65 each, worst page 4.27. So the splitting method paid here once and has now been
run to the end on this document; the next instrument for what is left is the extraction
comparison, not more pixels.

Corpus-wide the pattern is small: ten decks have a slide with a `Requires="v"` choice around a
`p:oleObj`, and only NAS has it on more than four slides. So this is one document's ceiling
rather than a class to build a tool around — but it is 10% of the track's ink and it had been
recorded twice as "linked Excel OLE, known" without the number being split, which is what let
its other 216.29 sit unexamined for two rounds. **Split a big document's ink before believing
its attribution.**

## Sheets is nearly untouched by this

One flagged page on the whole track. That track's image problem is the opposite one, and note
that its headline example was also wrong: `apron-area.xls` was recorded as drawing 0 images
against the reference's 1670, and the census that produced it was counting placements of EMFs
that draw as vector content. The document was a full match all along, page-1 ink 1.09%. Treat the
rest of that census as suspect.

## The slides track's ink, ranked with both ceilings subtracted

**Read this before ranking anything by `|ink|%`.** Measured at `2ced17655` over the whole
163-document slides track, from the sweep's own kept comparison reports and its two sets of
rendered PDFs — no re-rendering, so it can be reproduced from any sweep's output:

> **Provenance: this table is from the pre-container era and its absolute numbers no longer
> reproduce.** `2ced17655` and `dotnet/probes/slides-ink-ranking.tsv` (2026-08-09) both predate
> the 2026-08-13 move to LibreOffice 26.2.4.2 with `fonts-dejavu-core` present. The version
> change alone moved 160 of 163 slides' word counts, and ink moves with them, so **every
> figure in the table below and every row of the ranking was measured against a reference bank
> this container cannot reproduce.**
>
> What survives the change is the part that matters: the *mechanisms* — the reference
> rasterises, and the reference takes an `mc:Choice` it cannot resolve — are properties of the
> documents and of LibreOffice's importer, not of a particular build, and a sweep on
> 2026-08-15 still finds all 16 open slides failures to be words-only on exact slide counts,
> which is the signature those mechanisms predict. **Trust the classification; re-measure
> before quoting a number.** `dotnet/probes/PROVENANCE.tsv` records the era of every stored
> figure on the project.

```sh
python3 dotnet/probes/slides-r22/alternate-content-oleobj-census.py /c/sandbox/workdir/sample-files/slides > altcontent.tsv
python3 dotnet/probes/slides-r22/raster-pages-from-renderings.py <sweepdir> > raster-pages.tsv
python3 dotnet/probes/slides-r22/slides-ink-ranking.py            # reads both, beside sweep-base/
```

| | ink | pages | major pages |
|---|---:|---:|---:|
| the track, as swept | **1233.03** | 4199 | 415 |
| on a page at one of the two ceilings | 201.27 | 63 | 38 |
| **residual — what a fix can still win** | **1031.76** | 4136 | 377 |

The full ranking is `dotnet/probes/slides-ink-ranking.tsv`, sorted by the residual column.

**The two ceilings are different mechanisms and only one of them is visible to `pdfimages`.**

1. *The reference rasterises.* The signature this file's table already uses, applied per page:
   the reference draws a raster we do not draw, and we extract at least 8 and at least 25% more
   words there. **27 pages, 40.74 of ink.**
2. *The reference draws nothing at all,* because it takes an `mc:Choice` it claims and finds an
   unreachable external link, where we take the `mc:Fallback` and draw the replacement picture —
   the mechanism named in the section above. Censused from the packages rather than inferred:
   **38 slides across 10 documents, 165.46 of ink, 28 major pages.**

The two overlap on two pages of `16 - UTM - (NASA).pptx`, which is why the totals are 63 rather
than 65.

### What the subtraction changes, which is the point of doing it

`NAS-Infrastructure-Roadmaps-v16.0.pptx` has been quoted at **224.77, 18% of the track**. That
over-attributes it by more than two to one: **152.14 of the 224.77 is on the 24 slides carrying a
`Requires="v"` `p:oleObj`** and **72.63 is not** — which reproduces the 152.12/73.21 recorded in
the section above, independently and from a different instrument. NAS stays first on the residual
ranking and is worth a fifth of what its headline says.

Two documents rise past it in share of what is left, and **neither has ever been taken apart**:

| document | ink | residual | ceiling pages |
|---|---:|---:|---:|
| `Wildlife for REDAC September 11.pptx` | 54.89 | **54.78** | 1 |
| `Reporting_responsibilities_matrix.pptx` (268 pages) | 54.27 | **54.27** | 0 |
| `Thailand17.ppt` | 48.80 | 47.96 | 1 |
| `ITE106-Chapter 4.ppt` | 27.98 | 27.98 | 0 |

`Demick_JetBlue.pptx` moves the other way — 36.40 to **16.19**, three of its ten pages being the
rasterisation ceiling — so the automatic-series-colour work it motivates is worth less than its
headline suggested.

**Regenerate this after any round that moves the track.** The table above is `2ced17655`; the
round that produced it then took the track to `|ink|%` 1185.07 over ten documents, so the residual
is about 984 and the ranking's top rows have moved. The three scripts read a finished sweep's own
output and cost nothing but the reading.

**The ceiling subtraction is a floor on the ceiling, not a measurement of it.** Mechanism 1's
threshold is this file's own, and the section above records that it under-counts; mechanism 2 is
exact for `pptx` and blind to `.ppt`, which has no `mc:AlternateContent` to census. So 1031.76 is
an upper bound on what is winnable, and the ranking is the part to trust rather than the total.

---

# The same ceiling with the sign reversed: the reference's count is inflated

Everything above is about pages where **we** extract more words than the reference and are right
to. The mirror case exists, was measured on 2026-08-14, and belongs in the same file because a
round working a word-count failure has to rule out both.

**`2017-04-27-Lease-Transition-Records-Checklist-FINAL-1.xlsx` and its
`2020-01-29` twin** — page-exact at 5/5, **2323 words against the reference's 2498**, on every
page. It looks exactly like 175 words of lost content. It is not:

- Strip all whitespace from both extractions and the two character streams are **identical apart
  from one transposed word** — 13 858 characters each side, two diff blocks.
- The reference carries **310 single-letter tokens against our 154**.

LibreOffice writes intra-word positioning adjustments, and `pdftotext` reads each reposition as a
word boundary: `L icense`, `M aintenance`, `CM R`. The words are all there and in the right place
on both sides; only the reference's tokenisation is shattered.

This is the `Tj`-granularity artefact the render-comparison skill records, seen from the other
end. There it was *ours* fragmenting (`http://www.` counted 48 times); here it is the
reference's, and the gate reads it as our deficit.

**So when a page-exact document's word count is short, compare the whitespace-stripped character
streams before believing content is missing.** If they match, the defect is in tokenisation and
the gate cannot be won on that document — chasing it would mean making our text layer *worse* to
match poppler's misreading of theirs.

Note these two documents were worked anyway and the work was not wasted: they also had a genuine
font defect (a declared `family="1"` we ignored, so we set Bell MT in DejaVu Sans where the
reference sets DejaVu Serif). That is now fixed and the faces match. **The visible page improved;
the gate column did not move, and never will.**

## A third shape: the reference splits its own words

`slides/batch-004/pptx/solog_orientation_august_2019.pptx` — page-exact at 15/15, **670 words
against the reference's 685**, which reads as fifteen words lost.

Nothing is lost. Extracted with the same `pdftotext` and compared per page, character by
character with whitespace removed: **4758 non-space characters in the reference against our
4756**, every page's character multiset identical apart from two hyphens — and those are
`pdftotext` de-hyphenating *our* soft line breaks, with the hyphens confirmed present in our PDF
by `-bbox`.

All fifteen tokens are the reference splitting words it drew whole:

- **8 of 15** — LibreOffice writes **one show operator per character** on the footer lock-up
  ("19 glyphs in 18 shows"), and rounded advances leave a 1.26 pt gap after each `M`, so
  `pdftotext` reads `MIAMI` as `M` `IAM` `I` on pages 1, 2, 3 and 15. Ink spans 171.20–213.43 pt
  against our 171.09–214.26 — the same width, drawn in the same place.
- **7 of 15** — LibreOffice fills a line and breaks an over-long URL **mid-token**; we move the
  whole token to the next line first. A real fidelity difference, worth fixing on its own merits,
  but it moves zero words in either direction.

### The same shape at forty times the size: `architecture6.ppt`

`slides/batch-007/ppt/architecture6.ppt` — page-exact at 31/31, **1926 words against the
reference's 2544**. A 618-word deficit, a quarter of the document, and the largest word gap left
on the slides track. It is the same shape as the fifteen words above, and nothing is missing.

Whitespace-stripped character streams: **11048 characters ours against the reference's 11038**.
We draw *ten more* than the reference. Every opcode of the difference is one of four things, and
none of them is content:

- the bullet PUA code point — `U+E47A`/`U+E46F` ours against `U+F0B2`/`U+F0A7` — because
  LibreOffice keeps the symbol slot at `0xF000 | code` in its `ToUnicode` while we map it to the
  OpenSymbol glyph we actually draw. Both are unreadable Private Use Area noise in the text layer
  and neither counts as a word: the character is glued to the word after it in both;
- reading order on the five table pages, where `pdftotext` visits the label column and the footer
  at different points;
- three hyphens `pdftotext` de-hyphenates out of the **reference's** own line breaks
  (`lowest-level`, `multi-level`, `batch-`), which we do not break there;
- **two words the reference loses.** Its page 13 table overruns the page: the last row's
  `each layer.` is drawn past the bottom edge and its body text runs straight through the footer.
  We fit the row and draw the words. The deficit is 618 in the reference's favour *despite* this.

All 618 are on five pages — 10, 14, 21, 24 and 27, the pattern-table slides — and all of them are
the reference positioning table text glyph by glyph. Page 10's description cell is **"65 glyphs in
64 show(s)"** in the reference against **"74 glyphs in 11 show(s)"** in ours.

**Why LibreOffice does it here is worth recording, because it looks like a metric bug of ours and
is not.** The reference's `TJ` arrays carry per-glyph corrections of −12 to −164 thousandths of an
em, all negative, all widening. Solving for the advances they imply gives, on `Description` at
14 pt: 831, 679, 594, 594, 493, 344, 719, 477, 344, 688 — which is **DejaVu Sans Bold**
(830, 678, 595, 595, 493, 343, 716, 478, 343, 687), while the glyphs drawn and the `/Widths`
written are **Liberation Sans Bold** (722, 556, 556, 556, 389, 278, 611, 333, 278, 611).
LibreOffice measured that text with one face and drew it with another, which inflates the line by
15% and is what overruns the page. Our own pen is not implicated: the 24 pt title on the same page
is **157.39 pt ours against 157.28 pt reference**, 0.07% apart.

So the document is unwinnable twice over — the tokenisation is the reference's, and closing the
gap would mean adopting per-glyph positioning we have no reason to want. A blind reviewer sent the
page pair with no numbers reported the mechanism independently: *"the bottom half's text is WIDER
than the top's for identical strings … a wide, splayed m"*, and *"the table overruns the bottom of
the page … the footer text is overlapped by the table's body text"*.

**What the round found instead is in `probes/slides-arch-01/results.md`:** every bullet on every
binary `.ppt` was drawn hard black, on 26 of the corpus's `.ppt` decks and 935 glyphs. The word
column cannot see it and did not move.

### The same shape a fourth time, and here it is *our* tokenisation that shatters

`sheets/batch-010/xls/Template Pilot Logbook JAR-FCL V3.0.xls` — page-exact at 38/38, **1587 words
against the reference's 1531**, and it reached that number by being *fixed*. Before
`probes/sheets-dateaxis-01`, the same document read 1327 against 1531 and the 204 missing words
were real: its three chart pages draw a date axis whose ~72 rotated tick labels we were not
drawing at all. Drawing them takes the whitespace-stripped character streams from 128/91/19
against 352/383/55 to 344/363/49, which is the content arriving — and takes the word column
straight past the band and out the other side.

The cause is the third shape above with the sign reversed. **LibreOffice emits one `Tj` per glyph
inside a `Tm` for rotated text and we emit one `Tj` for the whole label inside a `cm`**, and
`pdftotext` fragments the two differently: about **3 gate-words per label for the reference and 4
for us**. We draw 29/36/3 labels against its 30/38/4 — *fewer* — and still extract more words.
Drawing the two labels we are still missing would take us to about 1603.

So the gate cannot be won here either, and the direction it would have to be won in is drawing
*fewer* labels than the reference. The document is now a ceiling; before this round it was a
defect, and the word column could not tell the two apart.

So this document joins the list from a third direction. The three shapes now recorded here:

| shape | who over-counts | cause |
|---|---|---|
| the reference rasterises an embedded object | ours | we draw real text where it draws a picture |
| the reference's tokenisation shatters | the reference | intra-word positioning read as word breaks |
| the reference splits its own words | the reference | one show per character, plus mid-token URL breaks |
| **the reference outlines its glyphs** | **ours** | **rotated text drawn as filled paths — ink with no text layer** |

**The common test is the same in all three: compare the whitespace-stripped character streams
before believing a word count.** If they match, the gate cannot be won on that document and the
number is about `pdftotext`, not about the renderer.

**Two further shapes were added after that sentence was written and each qualifies it.** The
outlining shape is immediately below and is a fourth way the gate cannot be won. The one after
the character-stream section — *we shatter our own line* — is the only shape so far that is ours
and **winnable**, and is the reason the sentence above is not a general rule. The ordinals in
the headings below grew by accretion and are not a sequence; read the shape names.

### The rotated-text shape again on a slides deck: `southern-classic-…-final.pptx`

Measured 2026-08-15, round `slides-chart-01`. `slides/chart-001/pptx/southern-classic-kennesaw-state-university-final.pptx`
— page-exact at 23/23, **2217 gate-words against the reference's 2270**, a 53-word deficit against
a ±45.4 band. It reads as lost content and none is lost.

**We draw 19 characters *more* than the reference.** Whitespace-stripped, over all 23 pages:
**12261 ours against 12242**. Only five pages differ at all, and every one of them carries a chart
with a rotated category axis:

| page | word delta | ours single-glyph text records | ref single-glyph text records |
|---|---:|---:|---:|
| 2 | **−37** | 9 | **125** |
| 9 | −8 | 0 | **33** |
| 10 | −11 | 1 | **45** |
| 11 | +1 | — | — |
| 12 | +2 | — | — |
| | **−53** | **10** | **203** |

**Page 9 is the control that makes this a mechanism rather than a correlation.** Its character
multiset is **exactly identical** on both sides — zero characters either way — and its glyph
count is **392 against 392**. The only thing that differs is granularity: **80 show operators
ours against the reference's 240**. Same glyphs, same characters, same positions, −8 words.

At the operator level, in page 2's rotated date-axis band (`Jan-16` … `Jan-17`, 20 labels):

| page 2, axis band | ours | reference |
|---|---:|---:|
| text records | 20 | **111** |
| glyphs | 132 | 128 |
| show operators | 39 | **122** |

The reference writes **one text record with one show per glyph, each in its own `Tm`**; we write
one record per label. `pdftotext` reads each of the reference's positioned glyphs as a token.
This is exactly the `Template Pilot Logbook JAR-FCL V3.0.xls` mechanism above, with the sign
reversed — there our tokenisation shattered *more* than the reference's, here the reference's
shatters more than ours, and both are unwinnable for the same reason.

**Where the two sides genuinely differ in content, we draw more, not less.** The per-page
character-multiset difference is: page 2 ours-only `Optimistic` — a fourth chart legend entry the
*reference* drops, its legend box being too small once it wraps `Closing Price` and splits
`Pessimisti`/`c`; pages 11 and 12 ours-only `Domestic` and `Revenue`/`Total`, the same class; and
three `U+E46F` against three `U+F0A7`, the bullet-PUA difference already recorded under
`architecture6.ppt`. A blind reviewer given page 2 with no numbers reported the missing legend
entry independently: *"the top half has 4 entries, the bottom has 3 … there is no 'Optimistic'
entry."*

**The reference is deterministic here** — three independent `soffice 26.2.4.2` conversions gave
2270 gate-words each, matching the banked PDF, so this is not the instability class.

Closing this gap would mean emitting one show per glyph for rotated text, i.e. deliberately
shattering our own text layer to match poppler's reading of theirs. **The gate cannot be won on
this document.**

### Two real defects on it that the word column cannot see, so they are not mistaken for ceiling

Both were found by blind page review and confirmed against the package. Neither moves a word.

1. **Page 10, value-axis increment.** `chart10.xml` fixes `c:min val="320"` and leaves the maximum
   and the interval automatic over data spanning 343–468. The reference draws `$320 $360 $400
   $440 $480`; we draw `$320 $370 $420 $470 $520`. Ours is a coarser increment over a taller axis,
   so our data line uses less of the plot box. LibreOffice's normalised increments are only
   {1, 2, 5}×10ⁿ (`chart2/source/view/axes/ScaleAutomatism.cxx`,
   `calculateExplicitIncrementAndScaleForLinear`, STEP 3), so its 40 is an increment of **20 drawn
   at label rhythm 2**, not an increment of 40 — which means the divergence is in
   `nMaxMainIncrementCount`, not in the nice-number set.
2. **Page 20, bubble-chart data labels — and here the reference is wrong.** `chart21.xml` is a
   `c:bubbleChart` whose `c:dLbls` carry `showVal=1` and `showBubbleSize=0`, with `c:yVal`
   47.27/52.89/62.70 and a constant `c:bubbleSize` of 3. We label 47.27/52.89/62.70; the reference
   labels **`$3.00` three times** — the bubble size, put through the value axis's `"$"0.00`.
   Ours is the correct output. Our bubbles are also drawn far too small, which *is* ours: the
   reviewer described "a small pale-blue blob with a tiny dark dot" against the reference's
   "solid disc surrounded by a wide pale halo".

## The reference outlines its glyphs, and `pdfimages` cannot see it

This file's page test asks `pdfimages` whether the reference drew a raster we did not. **There
is a second way for the reference to put ink on a page and nothing in its text layer, and that
test is structurally blind to it: it converts the glyphs to filled paths.** No image is
reported, so the page reads as an ordinary word-count defect of ours.

Measured in round `slides-b008-01` on
`slides/batch-008/pptx/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` **page 16**, in the band
beneath its first chart's baseline (x 17–368, PDF-y 154.7–170.7):

| | reference | ours |
|---|---:|---:|
| text-showing operators in the band | **0** | 18 |
| glyphs in the band | **0** | 103 |
| glyph-sized filled paths (< 12×12 pt), all one colour `#595959` | **120** | — |
| `pdfimages -list` entries anywhere on the page | **0** | 0 |
| month tokens from `pdftotext` (`-raw`, `-layout`, `-bbox` alike) | **none** | 59 fragments |

Twenty rotated date-axis labels (`Apr-19` … `Jun-22`) of six characters each is **120 glyphs**.
Whole-page fill counts are 180 against our 58 — a difference of 122, being those 120 plus two
bars.

**Two blind reviewers, given the page and a 300 dpi crop with no numbers and no repository
access, both read the labels as present and legible in the reference**, one describing them as
"grey … rotated ~45° ascending" — matching `#595959` and the measured bbox slopes. The ink is
there; the text layer is empty; ours is the better output and the gate scores it +59.

**The within-document control is what makes this a mechanism rather than an anecdote.** On the
same page of the same PDF, the reference's *horizontal* labels — Figure 2's `lundi…dimanche`,
Figure 3's `[08 h ; 10 h[` intervals, every value tick — are ordinary text operators that
`pdftotext` reads fine. Only the rotated run is outlined.

**Established:** the table above. **Not established:** that LibreOffice outlines *all* rotated
chart tick labels on PDF export. One document, one chart, one good internal control. Do not
promote it without a second instance.

> **The second instance arrived on 2026-08-15 and the third with it.** `Demick_JetBlue.pptx`
> pages 4, 5 and 7 carry it — 126, 156 and 126 glyph-sized `#000000` fills against our 126, 157
> and 126 ours-only characters, all of them the digits and hyphens of a rotated quarter axis —
> and `N2_E_Maestroni_Swarm_COP.pptx` page 7 carries 173 more alongside a raster. Both are
> `pptx` decks unrelated to `8_P-Pavese`, and in both the reference's *horizontal* labels on the
> same page stay ordinary text, which is the same internal control. Worked in the
> `slides-verify-01` audit above. The mechanism is no longer a single anecdote; what is still
> unestablished is the *rule* — which rotated text LibreOffice outlines and which it emits as
> per-glyph shows, since both behaviours are now attested on rotated chart tick labels.

**It is not the `Template Pilot Logbook JAR-FCL V3.0.xls` shape**, recorded further up, where
LibreOffice emits one `Tj` per glyph for rotated text. There the rotated text is still text and
`pdftotext` over-tokenises it. Here it is not text at all and `pdftotext` reads nothing.

### How common — censused, not guessed

`dotnet/probes/slides-b008-01/outline-ceiling-census.py` reads a finished sweep's own output and
looks for: page-exact document, ≥ 8 more raw words on the page than the reference, ≥ 20
glyph-sized one-colour fills on the reference page, ≥ 20 more text glyphs from us.

```sh
python3 dotnet/probes/slides-b008-01/outline-ceiling-census.py <sweepdir> \
        /c/sandbox/workdir/refpdfs-26.2.4.2-fonts/slides
```

Over **78 documents and ~2400 pages** (`slides/batch-001` … `008`) it returns **one row** — this
page. Rare in what has been swept, and worth keeping the script for, because it costs nothing on
a sweep that already exists and no other instrument the project points at a word-count failure
can see this class at all.

### The rule this section asked for, established 2026-09-07: it is a SHEAR, not a rotation

The paragraph above leaves *"which rotated text LibreOffice outlines and which it emits as
per-glyph shows"* unestablished. It is now established, on a thirteen-variant probe of one corpus
document, and it is **not the angle**.

**A text run is outlined when it is both turned off a right angle *and* inside a chart that had
to be fitted anisotropically. Both halves are necessary; neither is sufficient.**

`dotnet/probes/odp-chart-r72/variants.py` builds one-attribute variants of
`038_Competitive_Advantage_Card…odp`'s embedded chart and renders each through 26.2.4.2. The
`textlines` column counts spans PyMuPDF finds in the label band and always includes the two
legend keys, so **2 means no label was drawn as text at all**; `fills` counts glyph-sized filled
paths there; `legend` is the *unrotated* legend key's width and font size, which is the anisotropy
witness.

| variant | textlines | fills | legend | |
|---|---:|---:|---|---|
| `base` — five long names, auto 45° | 2 | **66** | 59.37, **13.94** | outlined |
| `oc15` — one name shortened, `style:rotation-angle="15"` | 5 | 2 | 59.35, 14.00 | turned, still text |
| `oc30` — the same at 30° | 7 | 2 | 59.35, 14.00 | turned, still text |
| `oc45` — the same at 45° | 2 | **66** | 59.37, **13.94** | outlined |
| `oc60` — the same at 60° | 2 | **61** | 59.38, **13.96** | outlined |
| `oc90`, `oc270` | 2 | 2 | 59.25, **13.87** | text, drawn elsewhere |
| `short45` — names `A`…`E`, `style:rotation-angle="45"` | 7 | 3 | 59.35, 14.00 | turned, still text |

`short45` against `oc45` is the discriminator: same angle, same file, and the short labels stay
text. What separates them is whether the labels overflow the chart's own page — because **an
embedded chart is fitted to its own drawn extent by two factors, one per axis**
(`ViewContactOfSdrOle2Obj::createPrimitive2DSequenceWithParameters`,
`svx/source/sdr/contact/viewcontactofsdrole2obj.cxx`:88-116), so overflowing labels squeeze the
whole chart. The legend column measures it without any rotation in the way: **14.00 pt in every
variant whose labels fit and 13.94 to 13.87 in every variant whose labels overflow**, at an
unchanged width of 59.35 to 59.38 — 1.00034 across against 0.99571 down on `base`, a 0.46%
anisotropy.

An anisotropic scale composed with a rotation that is not a right angle decomposes to a **shear**,
and that is what the renderer refuses
(`VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D`,
`drawinglayer/source/processor2d/vclprocessor2d.cxx`:126-141):

```cpp
// Acceptance is restricted to no shearing and positive scaling in X and Y (no font mirroring
// for VCL)
aLocalTransform.decompose(aFontScaling, aTranslate, fRotate, fShearX);
// tdf#95581: Assume tiny shears are rounding artefacts or whatever and can be ignored,
// especially if the effect is less than a pixel.
if (std::abs(aFontScaling.getY() * fShearX) < 1)
```

Everything that guard rejects falls through to the primitive's own decomposition, which for text
is filled polygons. That accounts for all thirteen rows: no anisotropy at 15°, 30° and on the
short labels, so no shear; anisotropy but a right angle at 90° and 270°, so still no shear;
anisotropy *and* 45° or 60°, so a shear of about 1.8 against the guard's 1.

**Two consequences worth carrying.**

1. **The condition is a property of the chart's fit, not of the label.** A document can outline
   one chart's turned labels and draw another chart's turned labels as text on the same page,
   which is exactly what `8_P-Pavese…` page 16 does — and the "internal control" the section
   above rests on (the reference's *horizontal* labels stay text) is therefore weaker evidence
   than it looks. The real control is a turned label that stays text.
2. **The class is bigger than the four documents recorded above**, and it is now attested in ODF
   as well as OOXML. Two more `.odp` rows are it — see `probes/odp-chart-r72/remainder.tsv` —
   and one of them, `038_Competitive_Advantage_Card…odp`, was carried by the previous round as an
   ODF *chart reader* defect on the grounds that "the reference draws none". It draws all of them,
   as 75 filled paths per page, and our arrangement agrees with its to **1.6 pt over 246**.

### Two `.odp` rows join the list, and one of them is a `.pptx` row's twin

| Document | Pages | evidence |
|---|---|---|
| `slides/chartset-008/odp/038_Competitive_Advantage_Card_for_PowerPoint_and_Google_Slides_373720f6.odp` | 1, 2 | 75 glyph-sized `#595959` fills on the reference against our five 45° text lines, +68 each. Both pages hold the same chart twice. |
| `slides/ceiling-002/odp/Demick_JetBlue.odp` | 4, 5, 7 | 170, 210 and 156 fills against our 23, 27 and 23 turned lines — **the same three pages this file already records for `Demick_JetBlue.pptx`**, so the ODF conversion reproduces it. Its pages 6 and 8 are a −90 deficit of ours and the two cancel, which is the net-delta blind spot again. |

`slides/ceiling-002/odp/8_P-Pavese_AIRBUS-ATB-journee-CRATB.odp` page 16 is the third instance and
is on a row already listed for its raster ceiling: 122 fills against our 36 turned lines.
`dotnet/probes/odp-chart-r72/classify.py` is the per-page instrument; unlike the census script
above it needs no banked reference bytes, only a rendered pair.

### And the ordinary raster ceiling reaches the converted `.odp` column too

Three of the thirteen `.odp` rows still failing the gate are the plain first mechanism — the
reference rasterises an embedded object and we replay its metafile as real text — confirmed page
by page with `classify.py` and recorded with their per-page evidence in
`dotnet/probes/odp-chart-r72/remainder.tsv`. They are **not** in the flagged-pages table above,
which is keyed on the original corpus' own paths and produced by a generator that never sees
`/home/user/corpus-odf`:

| Document | Pages | ours − ref |
|---|---|---:|
| `slides/ceiling-001/odp/OnTrac_StarCertificationProgram-3Day.odp` | 9, 10 | +293, +1183 |
| `slides/ceiling-002/odp/16 - UTM - (NASA).odp` | 7, 29 | +592, +548 |
| `slides/ceiling-002/odp/8_P-Pavese_AIRBUS-ATB-journee-CRATB.odp` | 5, 6 | +234, +269 |

All three are the `.odp` twins of documents this file already carries, which is the useful part:
**a ceiling survives LibreOffice's own conversion**, so the ODF column inherits the corpus'
ceilings rather than having its own. Check this file before working an `.odp` word-count failure
as well as a `.docx` one.

## `8_P-Pavese…pptx` in full — a worked example where the ceiling exceeds the gap

Kept here because the shape of it is the lesson, and because the next reader must not file its
remaining pages as ceiling too. Full workings in `probes/slides-b008-01/results.md`.

Gate row `26/26  2118/2010  words`, i.e. **+108**. Only five of twenty-six pages differ at all:

| page | net | ceiling | ours | mechanism |
|---|---:|---:|---:|---|
| 3 | +3 | — | +3 | 14 glyphs of **zero-ink** text we emit; the string is in no part of the package. Unexplained |
| 5 | +43 | +43 | — | raster, 692×240 JPEG + smask, object 207 |
| 6 | +31 | +43 | −12 | the same object 207; against it, a table row we push off the page |
| 8 | −7 | — | −7 | `percentStacked` ignored, `chartUserShapes` not read |
| 16 | +38 | +59 | −21 | outlined glyphs, above; against it, unwrapped category labels |
| | **+108** | **+145** | **−37** | |

**The ceiling is larger than the gap.** The document only reads +108 because we are
simultaneously losing 40 words of real content. Both corrections point the same way:

- excuse the ceiling and it reads 1973 against 2010, **−37 against a ±43.2 band — a pass**;
- fix all three of our own defects and it reads **2158, +148 — further outside than it is now.**

**Every real improvement available on this document makes its gate column worse.** That is this
file's standing argument in its sharpest form: here the number cannot be driven down at all, only
up, by drawing content we currently drop.

### Re-measured 2026-08-15, and the prediction above has since come true

Round `slides-chart-01` re-derived the whole accounting independently. The gate row is now
**26/26 2122/2010, +112** — four *worse* than the +108 recorded above, and the reason is that
one of the three defects was fixed:

| page | net, as recorded | net, 2026-08-15 | ours-only | ref-only | what changed |
|---|---:|---:|---:|---:|---|
| 3 | +3 | +3 | 3 | 0 | — |
| 5 | +43 | +43 | 43 | 0 | — |
| 6 | +31 | +31 | 43 | 12 | — |
| 8 | −7 | **0** | 1 | 1 | `percentStacked` / `chartUserShapes` **fixed** — and the gate went up 7 |
| 16 | +38 | **+35** | 56 | 21 | — |
| | **+108** | **+112** | **142** | **34** | |

So the file's sharpest sentence has been demonstrated rather than argued: a real defect was
closed, the page became correct, and the document's gate column moved further from a pass.

The ceiling re-confirmed at the operator level, on the current tree against the banked reference:

- **pages 5 and 6** — `pdfimages -list` reports the reference drawing object 207, a 692×240 JPEG
  with a soft mask, on both; we draw **no image on either page**.
- **page 16** — **neither side draws any image at all**. The reference draws **120 glyph-sized
  fills, every one `#595959`** (114 of them inside the axis band) and **zero text operators**
  there; we draw **20 text records of 6 glyphs each — exactly 120 glyphs — and one fill.**
  Whole-page fills are 180 reference against our 58, a difference of 122, being those 120 plus
  two bars.

Excusing the ceiling — 2122 − 142 — gives **1980 against 2010, −30 against a ±40.2 band: a pass.**
Fixing all three of our own defects instead gives 2152, +142.

**The "arc-warped gauge labels drawn twice" suspicion recorded against this document in the task
list is refuted.** There is no doubling anywhere on it; that suspicion belongs to
`FAAAIandtheArtandScienceofV&Vfinal.pptx`, whose `a:prstTxWarp` gauge labels were the round
`slides-extra-01` fix recorded further up this file.

### Its three defects, so they are not mistaken for ceiling

1. **Page 8, `percentStacked` ignored.** `ppt/charts/chart1.xml` is
   `<c:grouping val="percentStacked"/>` over raw counts 548/317 and 73/122, with a value axis
   carrying `formatCode="0%"`. We do not normalise the stack: we auto-scale to the raw total
   (≈621 → 700) and *then* apply the percent format, so the axis reads
   **`0%, 10000% … 70000%`** in eight ticks where the reference reads `0% … 100%` in eleven.
2. **Page 8, `chartUserShapes` not read.** `chart1.xml.rels` carries
   `Type=…/chartUserShapes Target="../drawings/drawing1.xml"`, and that part's entire text
   content is `['88%', '72%', '(317/439)', '(548/621)']` — exactly the four tokens missing from
   our page. The same rels file carries a `themeOverride` whose `minorFont` latin is **Palatino
   Linotype**, which is why the reference's chart text is serif and ours is sans; a blind
   reviewer named "a theme major/minor latin font not being applied" as its first candidate
   cause with no access to any of this. Of the deck's six charts only `chart1` has either
   relationship.
3. **Page 6, a table row pushed off the page.** The reference draws twelve body rows and we draw
   eleven; the twelfth is the twelve tokens of the difference. A blind reviewer found it unled
   and located the mechanism: the body sits lower in ours by very nearly exactly one row, with
   identical row pitch, so it is a single fixed offset above the body — and the cell-highlight
   rectangles sit at the same absolute page positions in both while landing on rows 1/4/8/11 in
   ours against 2/5/9/12 in the reference.
4. **Page 16, category labels not wrapped.** The reference wraps Figure 3's nine interval labels
   onto two lines and draws them all; we measure each as one unbreakable run and drop every
   second one. Its value axis we tick every 200 to 800 against the reference's 100 to 900.

## The character-stream test passing does NOT mean the gate cannot be won

**Read this before filing the next identical-streams document as a ceiling.** The sentence above
was true of the three documents it was written from and is false as a general rule, and the
counter-example was worked in round `words-b008-01`.

`words/batch-008/docx/FAA-2017-0628-0002_attachment_1.docx` — page-exact at 4/4, **666 words
against the reference's 638**, whitespace-stripped character streams **byte-identical**, 3750
characters each side, zero diff blocks. Every symptom of shape 2. It was a real defect in our own
output and it is now fixed; the document is exact on every column.

**A fourth shape, and the only one so far that is ours and winnable:**

| shape | who over-counts | cause |
|---|---|---|
| **we shatter our own line** | **ours** | **a ligature's multi-character `ToUnicode` collapses poppler's gap tolerance** |

We formed Carlito's `t`+`i` ligature on a run carrying `w:spacing="60"`. A ligature is one glyph
covering two characters, so its `ToUnicode` maps one code to two — and **poppler answers a
multi-character entry by dropping its intra-word gap tolerance from 0.400 em to 0.100 em**,
measured by byte-surgery on both PDFs. The tracking's own 0.300 em sits between the two, so all
45 glyphs on the line extracted as 45 separate words. LibreOffice never forms it: non-zero
character spacing disables `liga`/`clig` (`vcl/source/outdev/text.cxx:996-998`). That rule is now
implemented — see `ShapingOptions.WithTracking`.

So the test to add when the character streams match is: **count the one-character tokens on each
side, and count the multi-character `ToUnicode` entries in each PDF.** Here they were 46 against
the reference's 12, and 4 against 1. Those two numbers separate "the reference's tokenisation is
shattered, and we cannot help that" from "ours is, and we can". `|ink|%` does **not** separate
them — it was 1.16 before the fix and 1.16 after, because the defect lives entirely in the text
layer and a raster diff cannot read it.

---

# A fourth shape, and a different kind: **the reference is not deterministic**

Everything above assumes the reference is a fixed answer we are trying to match. On at least one
document it is not.

**`sheets/batch-005/xlsx/fse_identification_form.xlsx`** — page-exact at 3/3, and its gate row
has been 440 words against 427 all day. Converted five times by the same `soffice` 26.2.4.2, the
same file, the same session:

```
run 1: 430   run 2: 430   run 3: 430   run 4: 430   run 5: 443
```

(raw `pdftotext` counts; the gate's letter-or-digit counts are the same swing, 427 against 440)

The 13-word difference is one sentence, and it is always the same one:

> *The serial number of the FSE assigned by the Original Equipment Manufacturer (OEM).*

**LibreOffice draws that cell in about one run in five and omits it in the others. We draw it
every time.** So the direction of the "defect" is the opposite of how the gate reads it: the text
is in the document, we render it, and the banked reference happens to be one of the runs that
dropped it.

A blind reviewer had already reported "we draw a sentence the reference leaves blank" and, quite
properly, listed *"the reference genuinely drops it — least likely given it is the declared ground
truth"* as its fourth candidate cause. That candidate was the right one. It is worth remembering
how reasonable it was to rank it last.

## What this costs, and what to do about it

- **This document cannot be scored against a freshly rendered reference.** Its verdict is decided
  by which run you happen to take. Anyone working it must use the banked PDF and know that the
  banked PDF is *one sample*, not the answer.
- **`.claude/skills/README.md`'s "the same input converted twice gives identical output" is
  qualified by this** and has been annotated. That claim was verified — on the documents it was
  verified on. It does not hold universally.
- **Before believing any single-document reference figure, render the reference more than once.**
  It costs one extra conversion. Two rounds have now spent effort on this document's 13 words:
  one attributing them to a paint clip, one to a dropped cell.

## It is not one document — at least four sheets are unstable

The sweep asked for above has now partly been run, as a control inside the tagged-PDF audit
(`dotnet/probes/sheets-tagged-01/results.md`). Rendering a document twice **in the same mode** and
comparing it against itself found:

| document | cross-mode difference | **same-mode difference** |
|---|---|---|
| `ans_mappings_of_eccairs_terms.xlsx` | 55 pages | **24 pages** |
| `PBN Matrix NAAs (V01).xlsx` | 8 pages | **7 pages** |
| `fse_identification_form.xlsx` | 1 page | **1 page** |
| `SIL_TDB648.xlsx` p56 / p85 | 442 / 271 px | **549 / 634 px** |

`SIL_TDB648` is the instructive one: **its same-mode difference is larger than its cross-mode
one.** It sets a header cell's `AirbusA350`/`A380AESU` on two lines in one run and four in
another. It had exactly the signature of a second paint-affecting tagged-PDF site and would have
been reported as one; the same-mode control is the only reason it was not.

**So the rule generalises past `fse`: before attributing a per-document difference to anything —
a code change, a flag, a version — render the reference twice in the SAME configuration and check
the document against itself.** A difference that survives that is real. One that does not is the
document, and any story told about it will be fiction with evidence attached.

Three of these four are in the failing lists above, which means an unknown share of what those
rows report is instability rather than defect. The rate over the words and slides tracks is still
unmeasured.

## A fourth ceiling: LibreOffice writes rotated axis labels one glyph at a time

`southern-classic-kennesaw-state-university-final.pptx` (slides `chart-001`) is 2217 words against
2270, 23 slides of 23, and **the whole of its deficit is how the two PDFs encode the same rotated
text.** Nothing is missing from our rendering.

Three of its charts crowd a horizontal axis badly enough that LibreOffice's escalation ladder turns
the labels 45° — `AxisLabelProperties::autoRotate45` (`VAxisProperties.cxx:403-408`), reached from
`VCartesianAxis.cxx:936-947` once a label's box overlaps its neighbour's. **We turn them too, at
the same angle, in the same places, and we draw the same labels.** Counted off the two content
streams:

| slide | labels drawn, both sides | label | chars | reference rotated `Tm` |
|---|---:|---|---:|---:|
| 2 | 19 | `Jan-16` | 6 | **114** = 19 × 6 |
| 9 | 8 | `2002` | 4 | **32** = 8 × 4 |
| 10 | 11 | `1995` | 4 | **44** = 11 × 4 |

The reference emits **one positioned glyph per character** for rotated text; we emit one positioned
string per label (19, 8 and 11 rotated `cm` operators against its 114, 32 and 44 rotated `Tm`).
`pdftotext` splits at every one of the reference's, so its axis reads as a hundred-odd "words" where
ours reads as a dozen. Bucketed by *y*, every one of the three deficits lands in the rotated band and
nowhere else — slide 2 −39 across y 340-400, slide 9 −8 across y 380-420, slide 10 −11 across
y 200-220 — summing to −58 against a document total of −55.

**This is the mechanism `ChartAxisLabels`' own class documentation already records** for
`bnc889755.pptx`, which "extracts 89 words for 16 month names" for exactly this reason: *"Neither
number is reachable by drawing the labels correctly."* It is written down here because that note
lives in a source file nobody reads while chasing a word count, and this document is the list that
gets checked first.

Two readings were made and refuted on the way, both worth not repeating:

- **"The reference rotates and we do not."** Scanning the PDFs for rotated *text matrices* finds 4
  streams in the reference and **0** in ours — a clean, confident, wrong answer. Our backend rotates
  with a `cm` on the graphics state and leaves `Tm` axis-aligned; the reference does the opposite.
  **Scan for both, or the same page reads as unrotated in whichever stack you did not check.**
- **"It is just tighter spacing, so `pdftotext` joins our labels."** Also wrong, and in the other
  direction: the labels are identical and it is the reference that is fragmented, not us that are
  merged.

Our output is the better one — real searchable strings rather than 114 loose glyphs — and `wc -w`
scores it as the failure. **Ceiling.**

### What is genuinely wrong on this document, and it is not the word count

Slide 10's value axis is auto-scaled from `<c:min val="320"/>` alone over data 343…468. The
reference draws `$320 $360 $400 $440 $480`; we draw `$320 $370 $420 $470 $520`. **A step of 40 is
not on the 1-2-5 ladder** that both `ScaleAutomatism.cxx:884-893` and our faithful port of it
(`ChartScale.cs:283-295`) are limited to, so the reference's increment is really **20 with every
second label suppressed** — the vertical arm of the same escalation ladder, where
`canAutoAdjustLabelPlacement` refuses rotation for a horizontal-text vertical axis
(`VCartesianAxis.cxx:552-556`) and the loop falls through to `m_nRhythm++`.

Two things follow, and both are needed together:

1. The `maximumIntervals` `ChartLayout` derives for this axis is **≤ 7** where LibreOffice's is
   **≥ 8**. At 10 the raw interval is 148/10 = 14.8 → 2 × 10 = **20**; at 4 it is 37 → 5 × 10 =
   **50**, which is what we draw. The boundary is sharp: ceilings 8, 9 and 10 all give 20, and 4
   through 7 all give 50.
2. We do not thin a **value** axis' labels by rhythm at all — `ChartAxisLabels.Resolve` is reached
   only from `ArrangeCategories`. Fixing (1) alone would draw nine labels at `$320 $340 $360 …`,
   which is further from the reference than what we draw now.

The word gate cannot see any of this: both sides draw exactly five labels, so it costs nothing and
is invisible in every column. It is a real rendering defect and it needs its own round, over a full
sweep — the interval ceiling reaches every auto-scaled value axis in the corpus.

## A fifth ceiling, and this one we created on purpose: an EMF we draw as text

`UG.CAO.00006 Foreign Part 145 approvals…docx` matched the gate at 7390 words against 7399 while we
drew **no figure at all**. Giving `w:pict`/`w:object` a layout path (`DocxVmlFrames`) draws its
519.55 × 224.65 pt inline OLE object, and the gate now fails it at 7565.

The extra words are the object's own, and the two renderings differ in kind rather than in content.
On page 13:

| | words | images |
|---|---:|---:|
| reference | 103 | **3** |
| ours | 279 | **1** |

LibreOffice converts the EMF preview to a bitmap and draws two pictures; we draw the metafile, so its
text stays text. `pdftotext` reads ours and cannot read the reference's, and the 176-word difference
is exactly that. Page counts agree at 29 either way.

**This is the same mechanism as the raster ceilings above with the sign reversed** — there the
reference's rasterising *inflates* its count against ours, here it *deflates* it — and the same
verdict follows: ours is the better output and `wc -w` scores it as the failure.

**It was taken knowingly, and the trade is worth stating plainly.** The change costs this one
document on the gate (words track 201 → 200 of 210) and buys a figure on every document that has
one. `EHEST-SMS`'s Figure 1 is the case it was built for: the gap between "…An example is provided
below:" and the caption was 60 pt against the reference's 236.6 and is now **236.00**. Its page count
does not move — the rest of its 80/82 is elsewhere, in the Appendix forms — so the gate shows this
as a pure loss and the rendering shows it as a pure gain. Reverting it would put the figure back to
nothing.

## The same ceiling on the `.rtf` column, and it has the sign of the paragraph above

Four rows of the converted corpus's `.rtf` column
(`/home/user/corpus-odf/words/chartset-001/rtf/`) fail the gate on text alone with the page
counts agreeing at 1/1, and they are this class rather than a defect:

| document | ref glyphs | ours | ref images | ours |
|---|---:|---:|---:|---:|
| `clustered-column-result.rtf` | 253 | 389 | 1 | 1 |
| `clustered-column-template.rtf` | 121 | 200 | 1 | 1 |
| `pie-chart-result.rtf` | 157 | 234 | 1 | 3 |
| `pie-chart-template.rtf` | 57 | 73 | 1 | 3 |

Each carries **two `{\pict}` and no `{\shp}`**: LibreOffice's own RTF export writes the chart as
`{\*\shppict{\pict\wmetafile8 …}}{\nonshppict{\pict\pngblip …}}`, two representations of one
picture. It reads the metafile and **rasterises it**, so its PDF holds a picture with the chart's
own labels inside it and nothing in the text layer; we play the metafile and its text records stay
real text.

The excess is exactly the chart's furniture and nothing else — on
`clustered-column-result` it is `Production in 2015`, `… 2016`, `… 2017` and the six category
names, and on `pie-chart-result` the six names plus `30% 16% 8% 6% 5% 35%`. **No word the
reference draws is missing from ours** on any of the four.

Ours is the better output. Do not work these rows.
