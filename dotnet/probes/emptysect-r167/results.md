# emptysect-r167 — `150_5300_13_chg10.doc`'s blank page 55

**Reference: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`).** Ours is this tree's own CLI. Nothing in
`dotnet/src` was changed by this round.

## Headline: the proposed mechanism is REFUTED, and the boundary is narrowed

`probes/pages-r164` §4 read the reference's own flat-ODF view of the boundary correctly —
a one-column `text:section` holding a single empty paragraph, followed by a paragraph carrying
`style:master-page-name="Convert_20_37"` — and proposed that **a one-paragraph section between
two page-breaking section breaks gets no page from us**, seating it in `Ww8DocumentReader.Layout`
or in `Paginator`.

**Twelve one-attribute fixtures say otherwise: 12 of 12 agree with 26.2.4.2, in DOCX and again in
DOC.** The shape is reproduced directly — including the two-column body the real document has
before the empty section, which the first eight arms lacked — and this tree spends the page in
every one of them.

| arm | ref | ours |
|---|---:|---:|
| `empty-middle` — A, a section holding one empty paragraph, C on a new page setup | 3 | 3 |
| `empty-middle-pagechange` — the middle section's own break changes the page | 3 | 3 |
| `empty-middle-samepage` — no page-setup change anywhere | 3 | 3 |
| `two-empty-middle` — two empty paragraphs in the middle section | 3 | 3 |
| `nextpage-middle` — the middle break is `nextPage` rather than `continuous` | 3 | 3 |
| `worded-middle` — the middle section holds a word (control) | 3 | 3 |
| `no-middle` — no middle section (control) | 2 | 2 |
| `pagebreak-middle` — a `w:br w:type="page"` instead of the section (control) | 3 | 3 |
| `twocol-empty-middle` — the same, after a two-column body | 3 | 3 |
| `twocol-empty-middle-pagechange` | 3 | 3 |
| `twocol-no-middle` (control) | 2 | 2 |
| `twocol-worded-middle` (control) | 3 | 3 |

Converted to `.doc` by 26.2.4.2 itself and re-measured through the WW8 reader: **8 of 8 of the
first set agree as well.** So neither `Paginator`'s treatment of an empty section nor the WW8
reader's coalescing of one is the defect, and a round dispatched at either seat would find nothing.

## What the boundary actually is, narrowed

`[bin]` The page the reference draws and we do not is **a content page, not a parity filler**.
Reference page 55 carries the running head `DRAFT AC 150/5300-13 CHG 10 / Appendix 2` and
**nothing else — no page number**, where page 54 prints `102`. A Writer page inserted for an
even- or odd-page break uses `GetEmptyPageFormat()` and draws no furniture at all, and PDF export
skips it outright (`SwPrintUIOptions::IsPrintEmptyPages`, default true) — which is the rule
`Paginator`:1300-1322 already models and measured. So page 55 holds the one-paragraph section's
paragraph, and the question is why that paragraph starts a page.

`[bin]` Everything either side of it agrees. Our page 54 and the reference's hold the same content
and end on the same paragraph — `Note: This surface is provided for information only and does not
take effect until January 1, 2008.` — with room to spare below it on both. Aligned page by page
over 40 to 60, every page pairs at a similarity of 0.95 or better except the single insertion:
ours 54 → ref 54, ours 55 → ref **56**.

`[bin]` The enclosing section in the reference's own view is `Sect1`, `fo:column-count="2"`, and
the one-paragraph `Sect5` that follows states `fo:margin-right="0.2in"` and
`text:dont-balance-text-columns="true"` — so the section carries **its own indents** as well as a
column change, which none of the fixtures above do. That is the next thing to vary.

## The file's own section table

`sepx.py` reads the `.doc`'s `Plcfsed` ([MS-DOC] 2.8.26) and prints each section's character range,
its `bkc` ([MS-DOC] 2.9.4: 0 continuous, 1 new column, 2 new page, 3 even page, 4 odd page) and its
column count. `sections.txt` is the output: **66 sections**, against the 36 `text:section` the
reference's flat ODF holds — which is not a discrepancy, because a `text:section` is written only
for a section Writer models as a frame, and a section whose break changes the page becomes a page
descriptor instead. `probes/pages-r164`'s *"121 sections for our reading against the reference's
36, so the two models are not comparable"* should be read the same way.

Six of the 66 carry an even- or odd-page break and eighteen state two columns. **Which of the 66 is
this boundary is not established**: mapping a `Plcfsed` character position onto the text needs the
piece table, which this probe does not read.

## Two routes that were tried and are dead ends

- **Round-tripping the `.doc` through the reference's own DOCX export** to get an editable
  substrate for bisection: the round trip is **79 pages against our 75**, four apart rather than
  one, so it is a different document and not a proxy for this one.
- **Reading the boundary out of the flat ODF alone.** It tells you what the reference *did* and
  cannot tell you what the file *says* — the `text:section` it writes for `Sect5` is consistent
  with a continuous break, and the `.doc` may well declare something else there.
