# r177 — both FAA Holdover Tables are the word-parity page, and nothing else

`wordsink-r174`'s ink ranking put `24-25_FAA_Holdover_Tables.docx` at the head with **321.27
summed unsigned ink and 48 of 155 pages MAJOR**, and `FAA 2025-26 Holdover Tables.docx` second at
93.02. Both fail the gate on pages — 154 against 155 and 166 against 167 — with their
alphanumerics within 51 and 147 of the reference's.

`TODO.word-parity.md`'s first entry is a `TOC \t` switch voiding a built-in heading's direct
paragraph formatting: 26.2.4.2 discards it, Word honours it, this tree follows Word, and
`XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` switches the reference's reading back on. Rendering both
documents with it:

| document | pages | with quirks | reference | ink | with quirks |
|---|---:|---:|---:|---:|---:|
| `24-25_FAA_Holdover_Tables.docx` | 154 | **155** | 155 | 321.27 | **33.35** |
| `FAA 2025-26 Holdover Tables.docx` | 166 | **167** | 167 | 93.02 | **34.73** |

**Both become page-exact**, their alphanumerics land within 22 and 20 of the reference's, and
**neither has a single MAJOR page left**. So the whole of 321.27 and 93.02 is one deliberately
divergent page each, amplified by every page after it being compared against the wrong one. There
is no second defect underneath; the residual is 0.21 per page, thin and even.

## Two corrections

**`TODO.word-parity.md`'s cost table said the rule costs one page on one document.** Measured at
HEAD it costs one page on **both** Holdover Tables. `FAA 2025-26 Holdover Tables` sat in that
table's *"the other four — unmoved on pages and characters"* row and is not unmoved. Corrected
there.

**And an ink ranking cannot be read without a page-alignment column.** Ink is compared page
against page, so a document that loses or gains one page early reports every later page as
different. That has now misread three documents in this project — `02_mcar` at 881.67
(`probes/tocwrap-r175`) and these two — and in all three cases the figure was one page. `rank.py`
now prints `aligned`, the fraction of our pages whose text matches the reference's at the same
index, and lists the documents whose ink is measuring an offset rather than a defect. **37 of the
337 words documents are in that list**, which is why the column is not optional reading.

It is a screen rather than a verdict: a page of dense numerals can score below the threshold while
being the right page, so a low `aligned` says *explain this before ranking it*, not *this is
offset*.
