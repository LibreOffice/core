# What a border's line style draws, and how much room it takes

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`, reference `soffice` the
distro's **24.2.7.2**. Every figure below is a fresh measurement in that environment.*

## Where this started

`07-04.doc` draws a **3 pt double rule** across its head — `fo:border-top="3pt double"` in
LibreOffice's own reading of it — and we drew a single hairline. It is one of eighteen documents
catalogued under *table rules and shading*, and the census that followed showed the class is not
rare: **82 OOXML documents in the corpus state a border style that is not a plain single rule**,
`double` alone 2059 times across 56 of them, `dotted` 663 across 17, `outset` 417 across 3.

Nothing in the tree modelled a line style at all. `ParagraphBorder` and `TableBorder` each carried
a width and a colour, and every border was one solid stroke of the stated width.

## The probes

`rules.py` writes one document per (style, `w:sz`) pair — a paragraph reading `ABOVE`, one carrying
the border under test, one reading `BELOW` — and reads two observables off page one:

* every horizontal run of dark pixels across the border's band in a **300 dpi** raster, as
  (top, thickness) in points, which is what the style draws;
* the bordered paragraph's own y from the PDF text layer, which is the room the border took.

`dashlen.py` (in the scratch, reproduced below) does the same along the rule at **600 dpi**, which
is what a dash pattern needs.

## What 24.2.7.2 draws, at `w:sz="24"` — 3 pt

| style | strokes | room |
|---|---|---:|
| `single` | one of 3.12 | 3 pt |
| `double`, `triple` | 3.12, gap 2.88, 3.12 | **9 pt** |
| `thick` | one of 6.0 | **6 pt** |
| `thinThickSmallGap` | 3.12 then 0.72 | 4.5 pt |
| `thickThinSmallGap` | 0.72 then 2.88 | 4.5 pt |
| `thinThickThinMediumGap` | 1.68 then 2.88 | 6 pt |
| `outset` | 0.72 then 2.64 | 6.05 pt |
| `inset` | 2.64 then 0.96 | 6.05 pt |
| `dotted`, `dashed`, `dotDash`, `wave`, `dashSmallGap` | as `single` | 3 pt |

So **the stated width is the width of *a* line, not of the border**, and three of these cost the
page two to three times what we were charging for them.

## The dash lengths do not scale with the rule

The thing a reader guesses wrong, and the reason `DashPresets` is the wrong source — DrawingML's
presets *are* multiples of the pen. Measured at 600 dpi at `w:sz` 8 and 24, identical at both:

| style | ink | gap |
|---|---:|---:|
| `dotted` | 0.48 | 1.00 |
| `dashed` | 8.04 | 2.52 |
| `dashSmallGap` | 3.00 | 1.00 |
| `dotDash` | 8.04, 2.52, 2.50 | 2.52 |

Which is `svtools::GetDashing`'s table — 1/2, 16/5, 6/2, 16/5/5/5 — scaled by **10 twips**:
`fPatScFact` (`svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx`:600) in the draw layer's
units, which are twips for Writer.

## What we do now

`BorderRules` ports `editeng`'s own arithmetic rather than fitting these numbers:
`ConvertBorderWidthFromWord` turns the stated width into the total, and `BorderWidthImpl` divides
the total into an outer rule, a gap and an inner rule — each of the three either a constant in
twips or a share of the total, with a scaling component having the *other two* constants taken off
it. That is what makes `thinThickSmallGap`'s scaling rule come out as the stated width exactly, and
it reproduces every row of both tables above, dash for dash.

`ParagraphBorder` gained the line and now carries the *drawn* width, so `Allowance` charges the
page correctly with no further change.

## Result

Words gate 337 documents, **310 match, verdicts identical row for row**; mean first-page ink
7.858 → 7.834. Four documents moved:

| document | ink | |
|---|---|---|
| `195584360` | 12.35 → **4.09** | |
| `system_design__technical_architecture_template` | 4.80 → **1.58** | |
| `Form-SM-76A-…-Compliance-Statement-…-11` | 41.98 → 40.63 | |
| `Technical_Issue_Report_Form` | 23.82 → **28.44** | worse by the metric, exactly right on the page |

The last one is worth the space, because it is the second compensating error this round has turned
up. Its running head carries `thickThinSmallGap w:sz="24"`, and we drew 3 pt of it where the
reference draws 4.5. Stroke for stroke on page one at 300 dpi:

| | head rule | first table rules |
|---|---|---|
| reference | (55.44, 2.88), (59.04, 0.72) | 125.52, 153.84 |
| before | (55.44, 2.88) | 123.84, 152.16 |
| after | (55.44, 2.88), (59.04, 0.96) | **125.52, 153.84** |

and the body's first line moves from 70.290 to **71.790** against the reference's 71.790764. The
missing 1.5 pt had been cancelling an accumulating row-height excess further down the page; with
the head right, the excess is no longer hidden. The ink is worse and the page is correct.

---

## The same question for a table's own borders

`tables.py` is `rules.py`'s companion: one two-row table whose `w:tblBorders` state the style on
every side, `ABOVE` before it and `BELOW` after. The observables are the horizontal rules on page
one and `BELOW`'s y, which is the table's whole height and therefore what each of the three
horizontal borders cost a row.

Every style's *increment* over `single` now matches the reference exactly. At `w:sz="24"`, with
three horizontal borders in the table:

| style | reference `BELOW` | ours | increment |
|---|---:|---:|---:|
| `single` | 121.38 | 116.09 | — |
| `double` | 139.38 | 134.09 | **+18** both |
| `thick` | 130.38 | 125.09 | **+9** both |
| `thinThickSmallGap` | 125.88 | 120.59 | **+4.5** both |
| `outset` | 130.53 | 125.24 | **+9.15** both |

The constant 5.29 pt between the columns is the standing row-height divergence and predates this.

**`outset` is the style that pays for reading `editeng` rather than fitting the probe.** Its total
from `ConvertBorderWidthFromWord` is `2w + 0.75`, but the two components it divides that into are
each `(2w + 0.75)/2 − 0.75`, so three quarters of a point of the total goes nowhere and the border
covers `2w` — 6.05 pt for a 3 pt rule, not 6.75. The reference charges the parts, which the
increment above shows to a hundredth of a point, and `PAT-047` states **388 `outset` borders**, so
storing the figure instead of the sum would have over-charged that one document by nearly 300 pt.
`BorderRules.FromWord` therefore hands back the sum, and `Bands` adds the slack back when it
divides — a fixed point, since 121 twips becomes 136 and divides into 15, 53 and 53.

### Result

Words gate 337 documents, **310 match**, one verdict-preserving word moved on
`AFS-050-004-F2_0i`; mean first-page ink 7.834 → 7.848. Four documents improve by more than a
point, two get visibly worse by the metric, and both of those are the same compensating error the
paragraph half turned up — read the *structure*, not the ink:

| | rules on page 1 | first pair |
|---|---:|---|
| `PAT-047` reference | 37 | 273.8, 275.0 |
| before | 25 | 279.4 |
| after | **35** | **279.4, 280.6** |
| `JEMIT_Template` reference | 14 | 285.8, 286.8 |
| before | 11 | 296.2 |
| after | **14** | **296.2, 297.1** |

Both drew half their rules before. The row *pitch* also comes right — `PAT-047`'s reference steps
38.0 pt and 32.1 pt where we stepped 37.2 and 31.4 before and step 38.1 and 31.9 now — and what is
left is a constant 5.6 pt inherited from the page's first thirteen rows, which are text-driven and
belong to the line-height divergence rather than to this.

---

## And the DOC reader, which is where this started

`07-04.doc` is a binary Word file, so none of the above reached it until its `BRC`s went through the
same map. The chain is two steps rather than one, and both were already half-ported:
`WW8_BRCVer9::DetermineBorderProperties` (`sw/source/filter/ww8/ww8scan.cxx`) gives the thickness
Word reserves — with its own adjustments for a triple line and the two waves — and `GetLineIndex`
(`ww8par6.cxx`:1444-1478) then hands that and the `brcType` to the *same* pair of editeng functions
the DOCX reader uses. `Ww8Border.Width` was the first step; `FromWord` is the second.

One substitution belongs to the filter rather than to editeng and is reproduced with it: **Word 9's
`outset` and `inset` become a thick-thin and a thin-thick large gap, drawn in silver**, with the
comment *"LO cannot handle outset/inset (new in WW9 BRC) so fall back same as WW8"*. A DOCX stating
the same two is not substituted. That is Writer's inconsistency, not ours.

Words gate 337 documents, 310 match, **not one row changed**; mean first-page ink 7.848 → 7.754.
Seven documents improved and none got worse:

| document | ink |
|---|---|
| `2013_11.doc` | 20.10 → **9.37** |
| `SFSP_2013-02_Bulletin.doc` | 18.24 → **8.25** |
| `FlightLaws.doc` | 12.66 → **5.07** |
| `07-04.doc` | 22.05 → **19.14** |
| the three `150_5300_13_chg*.doc` | −0.17 to −0.27 each |

`07-04`'s own rule, the one that started this, at 300 dpi: the reference draws (196.32, 0.72) and
(197.76, 1.44), we drew a single (210.0, 1.44), and we now draw (210.0, 0.72) and (211.44, 1.68).
The 13.7 pt of vertical offset is a separate defect — the document has a `January 1, 2008` line in
its head that LibreOffice's DOC importer drops and we do not.

## The ODF reader is deliberately *not* changed, and here is why

`OdtLayoutSource.Border` parses `fo:border`'s shorthand and throws the style word away, so the
obvious fourth step is to keep it. It was written, measured and reverted, for two reasons that
belong together:

1. **The corpus has no such document.** Censused over every `.odt .ods .odp .ott .fodt .sxw` in
   `sample-files`, counting `double dotted dashed groove ridge inset outset` in every `fo:border*`
   of `content.xml` and `styles.xml`: **zero documents, zero occurrences.** The words sweep with the
   change in agrees — 337 documents, not one row and not one hundredth of a point of ink moved.
2. **The obvious reading would be wrong for the commonest real shape.** ODF's shorthand states the
   *whole* border's width, so a `double` would have to be split into its three bands here rather
   than scaled — and LibreOffice does not split it evenly. Converting `07-04.doc` to `.fodt` gives
   `fo:border-top="3pt double"` beside `style:border-line-width-top="0.0209in 0.0102in 0.0102in"`,
   which is **1.505 pt, 0.734, 0.734** — not 1/3, 1/3, 1/3. Honouring that needs `TableBorder` to
   carry explicit bands rather than derive them, which is a model change and not this round's.

So the reader stays as it is until a document asks for it. The census script is the thing to re-run
first if one appears.
