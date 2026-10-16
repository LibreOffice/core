# The twelve band probes

Authored single-sheet XLSX workbooks, one variable apart, used to establish what LibreOffice
**26.2.4.2** does with a header or footer band that its margins leave too small for its text. All
of them state a band of **3.6 pt** (`bottom`/`top` 0.05 in above the `footer`/`header` margin)
unless the table says otherwise, and set their text at **9 pt**. Letter portrait, so the page is
792 pt tall, the footer band's top edge is 770.4 pt and the header band's is at the header margin.

Reproduce with:

```sh
export SOURCE_DATE_EPOCH=1700000000 TZ=UTC
.claude/skills/libreoffice-reference/scripts/lo-convert.sh --pdf --outdir /abs/ref hA.xlsx
"$XRAY_OFFICE_CLI" render --outdir /abs/ours hA.xlsx
```

| probe | band | content | reference draws | ours (at `3e4f4f50344`) |
|---|---|---|---|---|
| `hA` | header 3.6 pt | 1 text line | 1 | 1 |
| `hD` | header 3.6 pt | 2 text lines | 2 | 2 |
| `hH` | header 3.6 pt | 3 text lines | 3 | 3 |
| `hG` | header 3.6 pt | 9 text lines | **9** | 9 |
| `hB2` | header 3.6 pt | 8 empty lines, then 1 text line | **0** | **1** ✗ |
| `hJ` | header **64.8 pt** | 8 empty lines, then 1 text line | **0** | **1** ✗ |
| `hK` | header **64.8 pt** | 1 empty line, then 1 text line | 1 | 1 |
| `hL` | header **64.8 pt** | 2 text lines | 2 | 2 |
| `hE` | footer 3.6 pt | 2 text lines | 2 | 2 |
| `hI` | footer 3.6 pt | 3 text lines | **2** | 2 |
| `hF` | footer 3.6 pt | 9 text lines | **2** | 2 |
| `hC2` | footer 3.6 pt | 8 empty lines, then 1 text line | 0 | 0 |

Three things this table settles, and one it does not.

**A header is never cut off by the paper or by its band.** Nine 9 pt lines out of a 3.6 pt band
are all nine in the reference's PDF.

**A footer stops at the paper, not at the band.** Two lines fit between 770.4 pt and the 792 pt
page edge; a third would start at 792.8. `hI` and `hF` both keep exactly two however many are
declared.

**Clipping a band line to the band's own bottom edge is wrong** and was tried and reverted in this
round. It drops the second line of any two-line footer and cost
`fm-provider-service-measures.xlsx` thirty words the reference draws. Clipping to the *paper*
instead is correct but inert: rendering all twelve with that clip in and with it out is identical,
because the PDF writer already drops a run whose baseline is off the media box.

**Unexplained: `hB2` and `hJ`.** Eight empty lines followed by a text line draws nothing in the
reference at either band size, while nine *text* lines out of the smaller band draw all nine and
one empty line followed by text draws it. It is not `&R` followed by a line break — `hK` has that
shape and is drawn. `FAA-2019-0995-0002_attachment_2.xlsx` is the corpus instance, whose header is
`&R` then eight `\r\n` then `&9PAGE \r\n&P OF &N`, and it is worth twenty words.
