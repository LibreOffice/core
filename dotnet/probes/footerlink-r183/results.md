# footerlink-r183 — the reference adds an empty paragraph to an inherited footer

`words/pagination-002/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx`
headed round 182's `drift`-free ink ranking at 10.22. Two blind readings of its pages 25 and 30,
given no numbers, both reported the same thing: *"ours places the footer about 20 px closer to the
bottom edge than the reference"*. That is real, it is on 37 of the document's 44 pages, and it is
**the reference's artefact rather than ours** — so it is recorded here and in
`TODO.word-parity.md` instead of being fixed.

## The measurement

Footer text bottom edge, in points, read from the two PDFs' own text blocks. Page height 841.89,
`w:footer="567"` = 28.35 pt, `w:bottom="453"` = 22.65 pt.

| pages | what the section says | ours | 26.2.4.2 |
|---|---|---:|---:|
| 1–7 | the section states its own `w:footerReference` | 813.55 | **813.55** |
| 8–44 | the section states a `w:headerReference` and **no** `w:footerReference` | 813.55 | **802.05** |

The delta is **11.50 pt, constant, on every one of pages 8 to 44** and nought on pages 1 to 7 —
the same document, the same footer part, and the split falls exactly at the first section that
inherits its footer.

`813.55` is `841.89 − 28.35`: the footer's bottom sits at `w:footer` from the page's bottom edge.
Perturbing the real file confirms which operand it is — doubling `w:bottom` to 906 twips moves the
footer **not at all**, and doubling `w:footer` to 1134 moves it by exactly 28.35 pt.

## What occupies the 11.50 pt

An empty paragraph. Appending `<w:p><w:r><w:t>XTRAIL</w:t></w:r></w:p>` after the footer table in
`word/footer5.xml` and re-rendering with the reference draws `XTRAIL` at **y 802.40–813.56** —
precisely the gap — and **does not move the copyright block at all**. A paragraph supplied by the
file fills a space the reference had already reserved.

Appending a bare `<w:p/>` to every footer part likewise moves nothing. The reference's own
`--convert-to fodt` says why: of its five master pages, `Converted3` and `Converted4` — the two
built for the sections that inherit — write

```xml
</table:table> <text:p text:style-name="Footer"/>
```

while `Standard`, `Converted1` and `Converted2`, whose sections each name a footer of their own,
end at `</table:table>`. The footer parts those masters were built from all end
`</w:tbl></w:ftr>`, with no paragraph of any kind.

## The seat

LibreOffice cannot link header or footer *content* across page styles, so it copies it, and the
comment above the copy says so (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1118-1124,
*"LO does not support linking of header/footer content across page styles so we just copy the
content from the previous section"*). The copy is `copyHeaderFooterTextProperty` (`:935-960`):
`removeXTextContent` then `copyText`. `removeXTextContent` (`:518-526`) empties the target with
`setString("")` and then appends and disposes one paragraph — a text body cannot be left with
none, so the target still holds one empty paragraph when `copyText` puts the source's table in
beside it. That spare paragraph is the 11.50 pt.

## Reproduced from scratch

`inherit-footer.docx` and `inherit-footer-hdr.docx` are two sections, the second stating no
`w:footerReference`, whose footer part holds one table and nothing else. The reference's fodt for
the second gives the inheriting master page's footer as **`<text:p text:style-name="Footer"/>` and
nothing else** — the spare paragraph is there and the table did not come across at all, which is
the stronger form of the same bug and is already modelled in `DocxReader.FurnitureCarry`
(a header or footer holding no top-level paragraph is not passed down). The real document's copy
keeps its table; the synthetic's does not. Both end with the spare paragraph.

`ftr-tbl-only.docx` and `ftr-tbl-plus-p.docx` are the control for the arithmetic, on a footer the
section names itself: the table alone puts its text at 805.40–813.55 and the same table followed by
`<w:p/>` at 793.85–802.00, 11.55 pt higher. So a trailing paragraph is worth a line of the
document's default face, and nothing here inserts one that the file did not write.

## What it costs, which is almost nothing

Supplying the paragraph in `word/footer5.xml` and re-rendering **ours** against the unmodified
reference takes the document from `10.22` unsigned ink to `9.93`, and from 2 major pages to 1. So
the footer offset is **0.29 of the 10.22** — it moves three short lines by 11.5 pt on 37 pages and
that is all it is worth. It closes page 25, opens a two-page drift at page 34, and leaves page 30
at 2.55.

**The 10.22 was length, not a defect.** The document's mean per-page `|ink|%` is **0.23** over 44
pages, which is the raster floor for text this dense; its worst single page is 2.77, which ranks it
seventh rather than first. That is what sent this round after it, and it is why
`track-ink-sweep.sh` now writes `worst` and `mean` beside the sum and says to rank on `worst`.

## What is left on page 30

2.55, and it is not the footer. A blind reading: ours draws the figure caption's second line
*"End to end process mapping"* under *"Figure 9"* and the reference draws *"Figure 9"* alone,
because the reference's embedded process-map drawing is about 0.5 % taller — its inner grid, its
`SUPPORT PROCESSES` band and each of its five `CORE PROCESSES` chevrons all end a few points lower
while the top edge is identical — and that is enough to push one caption line onto page 31. Every
one of the drawing's ~60 labelled shapes matches in wording, colour, position and line breaking,
including a clipping artefact on *"Safety recommendations"* that **both** sides reproduce. Not
taken.
