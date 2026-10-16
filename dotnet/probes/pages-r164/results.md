# pages-r164 — four page-count residuals, four separate mechanisms

**Reference throughout: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`)**, with `/opt/libreoffice26.2/program` prefixed onto
`PATH` on every call (`render-ref.sh`); `/usr/bin/soffice`, 24.2.7.2, was never invoked, and all
four reference PDFs carry `Producer: LibreOffice 26.2.4.2 (X86_64)`. The five tarball font
confounds were confirmed absent from `/opt/libreoffice26.2/share/fonts/truetype` before measuring.

**Our half is the frozen snapshot `/home/user/cli-frozen-r164/XRay.Office.Cli`** (mtime
2026-09-25 21:56). No C# was changed and nothing was built, so this round cannot have swapped the
binary under itself. Baselines were read from `Td`/`TJ` through
`.claude/skills/render-comparison/scripts/pdf-ops.py`; `pdftotext -bbox` was not used for any
vertical measurement, per this repository's own note that its `yMin` is a font-descriptor ink box.

`[bin]` The three arms:

| document | ref 26.2.4.2 | ours | ours + `XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` |
|---|---:|---:|---:|
| `02_mcar_part-2_and_IS_v2.10.docx` | 312 | **313** | 313 |
| `CRIF - Spécification technique - Socle applicatif.docx` | 29 | 28 | 28 |
| `absrc-pac-01-info-note-en.doc` | 7 | 6 | 6 |
| `150_5300_13_chg10.doc` | 78 | 77 | 77 |

## 0. The two standing excuses, ruled out for all four first

**The parity switch: no.** It changes all four renderings byte-wise and moves **no** page count.
The instrument was validated against this file's own control —
`24-25_FAA_Holdover_Tables.docx` renders 154 with the switch off and 155 with it on, exactly as
`TODO.word-parity.md` records — so the switch is honoured and none of these four is an accepted
divergence.

**The raster ceiling: no.** `raster-ceiling-pages.tsv` carries all four as `pagination-differs`
with **0** flagged pages, and `TODO.raster-ceiling.md`'s decision on `150_5300_13_chg10.doc` is
about the `words` column on pages 27 and 29–32, not the count.

## 1. `CRIF` — 28 against 29: a field result takes the cached run's `w:rPr`

Two events; only the second costs a page. A local one at ref p3/4 that nets out by p5, and the
page-losing one at ref p20/21, which persists to the end. The twelve-paragraph `w:keepNext` block
the reference moves is **identically tall on both sides** — 199.54 pt to 0.00 pt — so this is not
drift: it is whether the run fits.

`[bin]` Our body text area reaches **10.7 pt lower** than the reference's. Lowest body baseline
over pages 5–29: reference **96.94**, ours **86.25**. The cause is the footer's height:
`word/footer1.xml` holds two FILENAME fields in one cell, and the `w:fldSimple`'s cached-result
runs state `<w:sz w:val="16"/><w:color w:val="8D979B"/>` where style `Pieddepage` states 10 pt
black. **The reference discards the cached-result runs' `w:rPr` entirely and formats the
recomputed result from the paragraph style; we honour it.** At 10 pt black the first copy is ~30 %
wider, the cell overflows, the footer takes a second line, and the body bottom rises 11.5 pt.

`fx/mkfldsimple.py` → `fx/fldsimple-rpr2.docx` (with `word/settings.xml`), field result at 20 pt
red against a 10 pt black style:

| arm | ref 26.2.4.2 | ours |
|---|---|---|
| plain run with that `rPr` (control) | 20 pt | 20 pt |
| `w:fldSimple` cached result, `\* MERGEFORMAT` | **10 pt** | 20 pt |
| complex field result run, `\* MERGEFORMAT` | **10 pt** | 20 pt |
| `w:fldSimple`, **no** `\* MERGEFORMAT` | **10 pt** | 20 pt |
| `w:fldSimple` over `PAGE` | **10 pt** | 20 pt |
| complex field whose **`fldChar`/`instrText`** runs carry the `rPr` | **20 pt** | — |

So the rule is: **a field result takes the character properties of the field's own marker runs,
falling back to the paragraph style — never the cached result runs', and `\* MERGEFORMAT` makes no
difference.** The last arm is also why CRIF's `PAGE` field *does* draw at 8 pt in the reference:
its `fldChar`/`instrText` runs state `w:sz w:val="16"`.

**Confirmed by prediction.** Stripping the three cached-result `w:rPr` from a *copy* of the
document makes our footer two lines at y 75.85/64.35 and our lowest body baseline **96.95**
against the reference's 96.94, and ref pages 5–27 then have identical line counts to ours
(47/47, 49/49, 50/50, 16/16, 41/41, 43/43, 35/35, 56/56, 43/43, 48/48), the page-20 `keepNext`
break reproduced exactly.

**But it alone takes us to 32 against 29.** Three residual local events remain, masked by the
footer: ref p3/4 (resyncs), **ref p9/10** (+1, persists) and **ref p27/28** (+1, persists). Our 28
is a −4 error and a +3 error partly cancelling.

**It belongs behind the parity switch.** `\* MERGEFORMAT` means *preserve the formatting of the
previous result*, so **Word draws 8 pt grey and we already agree with Word.** Seat:
`DocxLayoutSource.cs` `case "fldSimple"` (~:2042), `DocxContentReader.cs`:400,
`DocxTocStyles.cs`:187.

*Secondary finding from the same fixture*: in the last arm we drew the **cached** `9` rather than
the recomputed value — a complex field whose `fldChar` runs carry a `w:rPr` is not recomputed at
all. Small, separate, worth a seat.

**Ruled out.** Filename encoding (worth 2 characters of 110, and the sweep shows the reference
wraps at every name length from 9 up). Drift (block height identical to 0.00 pt). Font metrics
(both sides draw Liberation Sans at the same advances). `w:keepNext` handling (both engines keep
the run together — the only question was whether it fits).

## 2. `02_mcar` — 313 against 312: a right tab's trailing stretch is charged to the right indent

**One event.** Ref pages 1–28 start on our same-numbered page; **ref page 29 starts on our page
30**, and the +1 persists to 312. We need one more table-of-contents page than the reference.

**It accumulates as whole lines, not as points.** Intra-page pitch agrees (ref p27: 44 body lines
over 559.4 pt = 13.00 pt/line; ours 49 over 629.4 = 13.11). What differs is the line *count*: over
ToC pages 8–29 the reference leaves **2** page numbers orphaned on a line of their own and we
leave **8** — six extra lines, ≈77 pt, against the ~85 pt of free space the reference still has at
the foot of its p27.

`[src]` Every affected entry is style `TOC2`, which states
`<w:ind w:left="720" w:right="994" w:hanging="720"/>` and a right tab with a dot leader at
`w:pos="9360"` — and 9360 twips is exactly the text column's right edge, i.e. **beyond the
paragraph's own right indent**. Witness, page 26, title ending at x 484.1 on both sides:

| | ref | ours |
|---|---|---|
| leader run | 5 glyphs from 484.10 | **13** glyphs from 484.15 |
| page number | x 504.95, same line | **next line**, x 108.00 |

The control on the same page is style `TOC3` (`w:right="720"`), where both sides are identical.

**Characterised in both directions by a 7 × 10 sweep** (`fx/mktocrighttab.py`; `#` = the number
stays on the entry's first line, `.` = it wraps; columns are `w:ind w:right` in twips):

```
title |     0   180   360   540   720   850   994  1130  1440  1800
   40 r|     #    #    #    #    #    #    #    #    #    .
      o|     #    #    #    #    #    #    #    .    .    .
   42 r|     #    #    #    #    #    #    #    #    .    .
      o|     #    #    #    #    #    .    .    .    .    .
   44 r|     #    #    #    #    #    #    #    .    .    .
      o|     #    #    #    .    .    .    .    .    .    .
   45 r|     #    #    #    #    #    #    .    .    .    .
      o|     #    #    .    .    .    .    .    .    .    .
   46 r|     #    #    #    #    #    .    .    .    .    .
      o|     #    .    .    .    .    .    .    .    .    .
   47 r|     .    #    #    #    .    .    .    .    .    .
   48 r|     .    #    #    .    .    .    .    .    .    .
```

**Ours is exactly `titleEnd + numberWidth ≤ tabStop − rightIndent`** — it predicts all five of our
rows to the twip. **The reference's is `titleEnd ≤ tabStop − rightIndent − ε` *and*
`titleEnd + numberWidth ≤ tabStop`**, ε ≈ 2.5 pt — it predicts all seven of its rows. In words:
*the reference measures only the text before the tab against the right indent and lets the page
number sit out in the indent, right-aligned at the stop; we charge the number against the indent
too.* Both single-boundary rules alone fail cells of the sweep; only the conjunction reproduces it.

`[src]` The seat is one line — `XRay.Office.Text/Layout/TabRuler.cs`, `WidthOf`:196:

```csharp
return countsDeferredStretch || !last.Deferred ? last.Right : last.GapLeft + last.Width;
```

`TextMeasurer.Measure` calls this with `countsDeferredStretch: false` during line filling, so a
line ending in a right, centre or decimal stop is fitted at `GapLeft + Width` against the *line's*
limit, which already has the right indent taken out. `TabRuler.Place` and `TabRuler.BreakAt` are
both correct — they clamp against the *frame* edge and skip `Deferred` segments — and the
doc-comment on `countsDeferredStretch` states the intended rule correctly; the returned value does
not implement it. Fixing it needs `WidthOf` to report the two widths separately, and
`TextMeasurer.Measure`/`TabEnd` (~:885-900 and ~:926-950) to pass the frame edge through.

**Ruled out.** Line pitch (13.00 against 13.11, worth nothing over one page). Font metrics (title
end agrees to 0.05 pt). Tab-stop resolution (identical in the `TOC3` control and in every `R0` arm
of the sweep). `w:keepNext`. The `TOC2`/`TOC3` style lookup.

## 3. `absrc-pac-01-info-note-en.doc` — 6 against 7: an index wrapped beside an obstacle

**One event, at the foot of page 1**, persisting to the end: our page 1 holds ref pages 1 *and* 2
(47 lines against 24).

`[bin]` Everything else on that page agrees to 0.05 pt — the three header lines, the five
quick-link baselines, `INFORMATION HIGHLIGHTS` at x 311 on both, the six images to 0.15 pt — and
only the table of contents differs: we place it **179.9 pt to the right and 340.9 pt higher**,
beside the quick-links table instead of below it. The shift is a constant on the entry number, the
title and the right tab stop alike, so the whole paragraph box is displaced.

`[bin]` The reference's own `--convert-to fodt` puts them in the same document order we read:
`QUICK LINKS` → `Table2` (1 column, 2.1417 in, `table:align="left"`) → eight empty paragraphs →
`INFORMATION HIGHLIGHTS` → `<text:table-of-content>`. **Both engines already wrap
`INFORMATION HIGHLIGHTS` beside the table**, so both treat it as an obstacle; **only the index is
treated differently.**

**Leading hypothesis, NOT settled:** a Writer index/section frame (`SwSectionFrame`) is not
wrapped beside a fly — it takes the full text width and descends past the obstacle — where we hand
the index's paragraphs to the ordinary `FrameObstacles` path. `FrameLayout.cs`:1401-1436 ports
`SwTextFly::GetSurroundForTextWrap` (TEXT_MIN 2 cm, FRAME_MAX 1.5 cm), and on this obstacle
(154 pt wide, 319 pt of room to its right) that rule says *wrap on the right*, which is right for
a body paragraph and wrong for the index.

**What would settle it:** two fixtures with the same left-anchored obstacle, one followed by a
plain paragraph and one by a `text:table-of-content` / `TOC` field section, through 26.2.4.2. They
were not built because the natural carrier (`.fodt`) exercises the ODF path rather than the WW8
path this document takes, and a WW8 fixture is not cheap to author. **This is the one root cause
of the four that is not established.**

**Ruled out.** Drift. Font metrics and glyph fallback. Table geometry. Document order. `TEXT_MIN`
/ `FRAME_MAX` being wrong (they are the reference's own constants and give the right answer for
`INFORMATION HIGHLIGHTS` on both sides). The `PAGEREF` and `DATE` differences on page 1 are
consequences, not causes — same line counts.

## 4. `150_5300_13_chg10.doc` — 77 against 78: a one-paragraph section gets no page

**Several events; one survives.** Offsets over 78 pages: 0 through p25, −1 at p26 (a figure page),
0 again at p27, +1 at p40 (another figure page), 0 at p41, **−1 at p56 and thereafter**. The two
cancelling local events are in the figure regions the raster ceiling already describes — the
reference draws only the caption over a rasterised figure while we play the embedded metafile and
emit its labels as real text.

`[bin]` **Reference page 55 carries nothing but its running head.** Its own resolved view of that
boundary:

```xml
   </text:section>
   <text:section text:style-name="Sect5" text:name="Section21">
    <text:p text:style-name="P353"><text:soft-page-break/></text:p>
   </text:section>
   <text:p text:style-name="P354">Table A2-1. …</text:p>
```

`Sect5` is a one-column section holding a **single empty paragraph**, and `P354` carries
`style:master-page-name="Convert_20_37"` — a page-style change, which forces a page break before
it. So the reference spends a whole page on that section. In WW8 terms: two consecutive section
breaks with nothing but an empty paragraph between them, the second changing the page setup.

**Not settled on our side.** `extract --format json` reports **121** sections for our reading
against the reference's 36 `text:section`, so the two models of this document's sectioning are not
comparable, and the round did not separate the two candidate seats:
`Ww8DocumentReader.Layout.cs` coalescing the empty section, or `Paginator.cs` (~:1303, beside the
`EvenPage`/`OddPage` filler) declining to open a page for a section whose only content is an empty
paragraph. Dumping our section list at the offset after
`does not take effect until January 1, 2008.` decides it.

**Ruled out.** The raster ceiling as the cause of the *count* (the two figure events cancel; the
surviving event is on a page with no image on either side). Drift. A `w:pageBreakBefore`. The tail
(ref 76/77/78 against our 75/76/77 are three index pages on both sides; the `−2` a first-match
heuristic prints there is the metric, not the document).

## Across the four: no shared cause

| document | mechanism | layer |
|---|---|---|
| CRIF | a field result takes the cached-result runs' `w:rPr` instead of the field's own | DOCX reader |
| `02_mcar` | a right tab's trailing stretch is fitted against the right indent, not the frame edge | text line-breaker |
| `absrc` | an index section is wrapped beside a positioned obstacle instead of descending past it | frame/obstacle layout |
| `150_5300_13_chg10` | a one-paragraph section between two page-breaking breaks gets no page | WW8 sectioning / paginator |

Each was checked against the other three. CRIF's field rule cannot reach `02_mcar` (its `PAGEREF`
result runs state only `w:webHidden`) or the two `.doc`. `02_mcar`'s right-tab rule cannot reach
CRIF or `absrc` (both ToCs state a zero right indent, and the sweep's whole `R0` column is
identical on both sides) nor `chg10`'s index (page-for-page identical). `absrc`'s obstacle rule has
no counterpart in the other three. That three of the four are one page *short* is a coincidence of
direction, not a shared cause.

**The most valuable single finding is `02_mcar`'s**, because the rule is characterised in both
directions and lands on one line. **The one to be careful with is CRIF's**: the mechanism is
certain, landing it alone takes the row from 28/29 to 32/29, and Word agrees with us rather than
with 26.2.4.2.
