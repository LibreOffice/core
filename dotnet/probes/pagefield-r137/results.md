# Round 137 — the reference does not freeze a page-number field, it never imports the field at all

Round 137, `/home/user/wt-pagefield`, branch `agent/pagefield`, base `9a90bd663`, 2026-09-15.
One seat: **O88**, the two words-track documents whose footer page number the reference prints
as one constant value on every page. **The seat number may be renumbered at merge** — three
other rounds are live and seat collisions have happened repeatedly; if O88 is taken, this is
the *field-in-a-grouped-text-box-in-a-table-cell* entry and the probe directory name is the
stable identifier.

| | |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| second reference | `/usr/bin/soffice` — **24.2.7.2**, used deliberately in §6 to ask whether this is a 26.2 regression. It is not |
| reference renderings | the **banked** half of gate r133, `/home/user/gate-r133/ref` (947 PDFs), plus eleven documents rendered fresh in this round — eight mutants and three synthetics |
| our renderings | `dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli`, built 2026-09-14 22:52, unmodified. **No dotnet source was touched in this round**, so no rebuild happened and nothing here contended with a build |
| C++ tree | `/home/user/wt-pagefield`, read only. **C8 applies in full**: it declares `27.2.0.0.alpha0+` and is not the reference binary's source. Every claim below is labelled *measured* or *candidate*; nothing that rests only on a source reading is stated as established |
| population | 272 `.docx`, 947 corpus documents, 97 header/footer page-field documents, 338 converted `.odt` — each denominator named where it is used |
| sweeps | no `soffice` sweep. The corpus-wide census reads **banked** PDFs with `pdftotext` and invokes no renderer at all |

**Reading contamination.** No page was opened by eye. Every figure below is extracted text
(`pdftotext -layout`), an XML structure count, or a flat-ODF element count.

---

## 0. The headline

- **The brief's table is correct, cell for cell.** Both documents, both sides, all thirteen pages.
- **It is not a frozen field. There is no field.** 26.2.4.2's own `--convert-to fodt` of both
  documents shows the footer text box holding a literal `<text:span>Page 6 of 7</text:span>` and
  **zero** `<text:page-number>` elements, where the nine documents that work hold a real
  `<text:page-number text:select-page="current">`. The reference is not failing to update a
  field; its DOCX import drops the field and keeps the cached result as ordinary text.
- **The brief's discriminator is half right and the corrected one is an interaction of two
  factors**: the text box must be inside a **`wpg:wgp` group** *and* the drawing must be
  anchored **in a table cell**. Either alone is harmless. Shown in both directions on four real
  documents and on a from-scratch 2×2 factorial.
- **Neither the header/footer nor the `PAGE` field is part of it.** The same two factors drop a
  `FILENAME` field, and drop the field in the document **body** as well. The freeze on these two
  documents is the visible corner of a general field-drop.
- **Verdict: LibreOffice defect, not ours.** Nothing in either file pins the field — no
  `w:fldLock`, no `w:dirty`, no document protection — and the same binary keeps the field in
  three of four synthetic arms that differ only in the drawing's container. §5.
- **No behaviour was changed.** The deliverable is the seat, this write-up and a gate note.
- **Reach on the gate is zero.** Both documents are `match` rows in gate r133 and the census
  finds **2 of 947** banked reference renderings affected.

---

## 1. The brief's table, verified

Rendered with the CLI above into a scratch directory; the reference half is the banked gate r133
PDF, not a fresh render. Page counts agree on both documents, so the comparison is page for page.

| document | pages | ours | reference 26.2.4.2 |
|---|--:|---|---|
| `words/table-001/docx/A1. EASA Form 2.docx` | 7/7 | `Page 1 of 7` … `Page 7 of 7` | `Page 6 of 7` on **all 7** |
| `words/table-001/docx/B11. TE.CAO.00129  Experience  logbook.docx` | 6/6 | `Page 1 of 6` … `Page 6 of 6` | `Page 3 of 6` on **all 6** |

Per page in `verified-table.tsv`. **Every cell of the brief's table reproduces.**

The frozen values are the cached `w:t` results. `word/footer1.xml` of the first states
`<w:instrText>page</w:instrText>` … `<w:t>6</w:t>` and `<w:instrText>numpages</w:instrText>` …
`<w:t>7</w:t>`; the second states `3` and `6`. In both documents the cached `NUMPAGES` happens to
equal the real page count, so *these two documents alone* cannot distinguish "prints the cache"
from "evaluates `NUMPAGES` and freezes `PAGE`". §2 settles it by another route.

**One correction the brief did not ask for.** `A1. EASA Form 2.docx` also has a genuine content
divergence on its last three pages, unrelated to the field: the "I acknowledge…" line appears on
reference pages 6 *and* 7 and on our page 7 only, and per-page word counts differ 432/382,
289/300, 325/365 on pages 5–7. That is a separate defect, is not attributed here, and is **not**
part of O88.

---

## 2. The mechanism, measured in the reference's own output

`soffice --convert-to fodt` is a view of what the importer actually built, and it is a
measurement of the reference binary, not of the C++ tree.

| document | `<text:page-number>` in the footer master page | what the text box holds |
|---|--:|---|
| `A1. EASA Form 2.docx` | **0** | `<text:span>Page 6 of 7</text:span>` |
| `B11. … logbook.docx` | **0** | `<text:span>Page 3 of 6</text:span>` (three master pages, all three literal) |
| `TE.CAO.00125 … OJT Logbook.docx` (works) | **3** | `Page <text:page-number text:select-page="current">3</text:page-number> of <text:page-count>5</text:page-count>` |

The two structures are otherwise the same shape — a `draw:custom-shape` named "Text Box …"
inside a `draw:g`, in the footer of a `style:master-page`. So the divergence is decided at
**import**, before any layout or field-update walk runs. That answers the brief's framing
question ("why does the per-page field update skip this frame?") with "it does not skip it;
there is nothing there to update".

*(`A1. EASA Form 2.fodt` does contain one `<text:page-number>` — at offset 1 040 196, inside the
second `style:master-page` of the file, which belongs to an **embedded** object the document
carries under `word/embeddings`. It is not the footer under test.)*

---

## 3. The discriminator, established in both directions on real documents

The brief proposed *"the drawing is anchored inside a `w:tbl` in the footer"*, a 2/2, 0/7 split
on nine found documents. Re-derived here the population is **eleven**, not nine (§7), and the
proposal is **necessary but not sufficient**. Four single-variable mutations of real corpus
documents settle it; each rewrites exactly one `word/footer*.xml` inside the original zip,
leaving styles, `settings.xml`, the drawing XML and every other part byte-identical.

| mutation | one variable changed | reference 26.2.4.2 |
|---|---|---|
| `A1. EASA Form 2` — drawing paragraph moved out of the footer table to a direct child of `w:ftr` | table removed | **1..7**, unfrozen |
| `B11. … logbook` — same move | table removed | **1..6**, unfrozen |
| `UG.CAO.00133 … Language` — drawing paragraph wrapped in a new one-cell table (works unmodified, grouped, 18 pages) | table added | **frozen at 1 on all 18 pages** |
| `TE.CAO.00125 … OJT Logbook` — all three footers wrapped likewise (works unmodified, grouped) | table added | **frozen at each footer's own cache** |
| `system_design__technical_architecture_template` — wrapped likewise, but its text box is a **bare `wps`, not grouped** | table added | **2..17, unchanged** |
| `ABCD-SDE-23-00 …` — wrapped likewise, also **not grouped** | table added | **1..29, unchanged** |

So the table is necessary — removing it unfreezes both real documents — and it is **not
sufficient**: adding it to an ungrouped text box changes nothing, twice. The factor the brief
refuted, grouping, is the second half of the conjunction: it is harmless on its own (three
grouped documents work, `TE.CAO.00125`, `UG.CAO.00006`, `UG.CAO.00133`) and decisive together
with the table.

**The brief's refutation of grouping was a correct observation drawn one step too far.** All
three grouped-and-working documents are grouped *outside* a table, so they test one factor of a
two-factor interaction and cannot see it. Recording it that way rather than as "grouping:
REFUTED" is the difference.

There is also a natural control in the corpus that nothing had to be built for.
`words/table-001/docx/approvals-and-standardisation-…-B11.-TE.CAO.00129-…-logbook.docx` is the
same logbook, same six pages, same footer table — but its `PAGE`/`NUMPAGES` fields are plain runs
in the cell rather than runs inside a grouped text box. 26.2.4.2 prints **1..6** on it.

---

## 4. The synthetic factorial

Built from scratch, not derived, so it is independent of the corpus. It carries
`word/settings.xml` (with `compatibilityMode` 15), `word/styles.xml` with `docDefaults`,
`fontTable.xml` and both `docProps` parts, per the `office-corpus` skill's warning that a
DOCX without a settings part answers a different question. Four pages, four arms, one variable
per axis, generated by `syn/gen.py`. It has **no `mc:AlternateContent` and no VML fallback at
all**, so nothing below can be an artefact of the fallback branch.

| | text box in a plain footer paragraph | text box in a footer table cell |
|---|---|---|
| **bare `wps` text box** | 1, 2, 3, 4 | 1, 2, 3, 4 |
| **inside a `wpg:wgp` group** | 1, 2, 3, 4 | **2, 2, 2, 2** — the cached value |

One cell of four. The fodt of the three working arms holds a real `text:page-number`; the fodt
of the failing arm holds `<text:span>Page 2 of 4</text:span>` and zero fields — the same
signature as the two real documents, which is the sanity check the skill asks for.

**Ours renders all four arms 1, 2, 3, 4.**

Three further arms extend it:

| arm | result |
|---|---|
| grouped text box in a **body** table cell | field **dropped** (fodt: 0 `text:page-number`) |
| grouped text box in a plain **body** paragraph | field survives |
| grouped text box in a footer table cell, field changed `PAGE` → `FILENAME` | field **dropped** (fodt: 0 `text:file-name`) |

So the header/footer is not part of the mechanism, and neither is the page-number field. The
general statement is: **26.2.4.2's DOCX import discards any field inside a text box inside a
`wpg:wgp` group anchored in a table cell, and leaves the field's cached result behind as plain
text.** The page-number case is simply the one that is visible, because a cached page number is
wrong on every page but one.

---

## 5. The verdict, and where its argument runs out

**This is a LibreOffice defect and our 1..N is right.** Four arguments, in descending strength:

1. **The reference is internally inconsistent.** In the 2×2 factorial it keeps the field in three
   arms and drops it in the fourth, and the four differ only in whether the shape is grouped and
   whether the containing paragraph sits in a table cell. Neither of those is a property of the
   field. A consumer that had *decided* to honour a cached field result would honour it in all
   four. This argument needs no external authority and no reading of Word.
2. **Nothing in either file pins the field.** Grepped both: **no** `w:fldLock`, **no**
   `w:dirty`, **no** `w:documentProtection`. Under ECMA-376 the run between `separate` and `end`
   is the *last computed* result, and `w:fldLock` is the only thing that forbids recomputing it.
   Absent a lock, printing the cache for `PAGE` is a choice the file does not ask for.
3. **The corpus answers it against itself.** The sibling document in the same directory, same
   content, same six pages, same footer table, field not in a grouped text box, gets **1..6**
   from the same binary on the same day.
4. **It is a data-loss defect, not only a rendering one.** 26.2.4.2's own `--convert-to odt` of
   these two documents writes the frozen string and no field, so the field is gone from the
   converted file permanently. §7 measures that at 2 of 97.

**The case that we are wrong, looked for and not found.** Three readings were considered:

- *"The cache is the author's intent."* Refuted by (1): the intent would not depend on `wpg:wgp`.
- *"The field is locked."* Refuted by (2): checked, not present.
- *"A floating shape's page number is its anchor's, not the page it is drawn on."* Does not apply:
  the anchor is the footer of every page, and both renderers draw the box on every page. It would
  also predict the anchor's page number, not a constant unrelated to it — the frozen values 6 and
  3 are the *cached* numbers, not any page's number.

**Where the argument runs out.** I cannot run Word, so nothing here says what Word does; the
argument above deliberately does not need it. And I cannot rule out that Word also mishandles
this structure — it wrote these files, so it evidently renders the field, but that is an
inference from the file, not an observation of the program. The strength of the verdict rests
entirely on (1) and (3), which are measurements of the reference itself.

---

## 6. Not a 26.2 regression

`/usr/bin/soffice`, **24.2.7.2**, renders `syn-group-table.docx` as `Page 2 of 4` on all four
pages and `syn-group-para.docx` as 1, 2, 3, 4 — identical to 26.2.4.2 on both. So the defect is
long-standing rather than new, which removes "regression" from the argument and adds stability:
two independently built binaries two major versions apart agree, so this is not an artefact of
one build.

---

## 7. Census, with denominators

### 7.1 The structural population (`sig.py`, `fields.py`)

| | count |
|---|--:|
| `.docx` in `/home/user/sample-files/words` (the corpus holds `.docx` nowhere else) | **272** |
| …stating a `PAGE`/`NUMPAGES`/`SECTIONPAGES` field in a `word/header*.xml` or `word/footer*.xml` | **97** |
| …with that field inside a `w:txbxContent` | **11** |
| …with a `wpg:wgp` group anywhere in a header/footer | **9** |
| …**with the field inside a text box inside a `wpg:wgp` group anchored in a header/footer table** | **2** |
| …the same, for any other field type (`DATE`, `DOCPROPERTY`, `STYLEREF`, …) | **0** |

**The brief's 96 and 9 do not reproduce and the corrected figures are 97 and 11.** Two
definitions were tried: matching `PAGE`/`NUMPAGES` case-insensitively gives 97 and 11, matching
uppercase only gives 89 and 6. Neither gives 96 and 9. The two documents the brief missed are
`SWDD-template.docx` (a VML-only `w:pict` text box, no DrawingML at all) and
`PI-doc.-no.-2E-Technical-Review-Report.docx` (the field is in `header3.xml`, not a footer).
Both work. `textbox-field-contexts.txt` lists all eleven with their contexts.

The brief's instrument also flagged `PI-doc` as *"anchored in a table"*. It is not: its `w:tbl`
is **inside** the `w:txbxContent`, not around the `w:drawing`. An ancestor test that does not
record whether the table was entered before or after the drawing cannot tell those apart, and
that is the one place where the brief's 2/2, 0/7 split was measuring the wrong thing.

### 7.2 The behavioural population — all 947 banked reference renderings

`frozen-census.py` reads every PDF in `/home/user/gate-r133/ref` with `pdftotext -layout`, splits
on the form feed, and looks for a page-number-like token on each page. A document is **FROZEN**
when the same value appears on every page of a document of three pages or more.

| verdict | documents |
|---|--:|
| scanned | **947** |
| fewer than 3 pages, skipped | 291 |
| no page-number token on enough pages | 591 |
| sequential — the value equals the page index | 54 |
| mixed values (restarts, several counters) | 8 |
| **FROZEN** | **3**, of which **1 is a false positive** |

The false positive is `form_1123_application_form_rvsm_spa.docx`: the regex matched *"issue 1 of
20/06/2019"*; its real field prints `page 1/3`, `page 2/3`, `page 3/3`, correctly. A second pass
(`frozen2.py`) widened the patterns to `N/M` and bare `page N` and added one more candidate,
`4400-91_Proposal_To_Lease_Space_10-2024.docx`, also a false positive — `10/24` is a form
revision date.

**So: 2 of 947, and they are the two documents already in hand.** Across `.doc`, `.xls`, `.ppt`,
`.pptx`, `.xlsx` and `.xlsm` the census finds nothing, which is consistent with the mechanism —
a `wpg:wgp` group is a DrawingML-in-Writer construct and `.docx` is the only format in this
corpus that can state one. The `slides` and `sheets` tracks are 302 and 307 documents and
contribute **0**.

### 7.3 The ODF and RTF converted tracks

`/home/user/corpus-odf` is 26.2.4.2's own conversion of the corpus, so its `.odt` and `.rtf` were
written **from the already-broken import**. The two documents therefore show **no divergence on
those tracks, and that is the defect propagating rather than the defect being absent**:

| file | `text:page-number` in `styles.xml` | ours | reference |
|---|--:|---|---|
| `odt/A1. EASA Form 2.odt` | **0** | `Page 6 of 7` on all 7 | `Page 6 of 7` on all 7 |
| `odt/B11. … logbook.odt` | **0** | `Page 3 of 6` on all 6 | `Page 3 of 6` on all 6 |
| `odt/approvals-…-logbook.odt` (the sibling) | **3** | — | — |
| `rtf/` both documents | no field | neither side draws the footer text box at all | same |

On the RTF track neither renderer puts the footer text box's text on the page, so there is
nothing to compare; the two RTF renderings also differ in page count (8 reference against 7 and 6
ours), which is a separate divergence and is not O88.

**Export-side loss, over the whole words track** (`odt-loss.py`): of the **97** `.docx` that state
a header/footer page field, **4** have no `text:page-number` in the `styles.xml` of their
26.2.4.2 `.odt`. Two are O88's. The other two —
`words/done-003/docx/mde087077~283.docx` and `words/done-007/docx/template-technical-report.docx`
— are a different and benign thing: the footers carrying their fields are never used on a
rendered page, and **neither renderer prints a page number anywhere in either document**
(checked, four and ten pages). So the export-side reach of O88 is **2 of 97**.

---

## 8. The source, as a candidate only — C8

Everything above is measured. This section is not, and **nothing in the verdict depends on it.**
The tree read is `/home/user/wt-pagefield`, `27.2.0.0.alpha0+`; the binary is 26.2.4.2; the
release build compiles out `SAL_WARN`, so the branch actually taken cannot be observed and none
of this could be confirmed the second way the register requires.

What a reading does establish is that **the group is a special case in the import, which is what
a two-factor interaction with grouping needs**:

- `OOXMLFastContextHandlerWrapper::lcl_startFastElement`
  (`sw/source/writerfilter/ooxml/OOXMLFastContextHandler.cxx`:2244-2250) emits
  `startTextBoxContent()` on `wps:txbx` **only** when `mxShapeHandler->isDMLGroupShape()` — or,
  on the VML side, when `mbIsWriterFrameDetected` and the element is `vml:textbox`. A bare
  `wps:txbx` outside a group never takes this path at all.
- `DomainMapper::lcl_startTextBoxContent` (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:4221)
  forwards it to `DomainMapper_Impl::PushTextBoxContent`
  (`DomainMapper_Impl.cxx`:6255-6282), which creates a real Writer `SwXTextFrame` for the box's
  content — the thing a `SwPageNumberField` can live in — and pushes it on the text-append stack.
- That function **abandons the routing silently** when the top of the stack is not an `SwXText`:
  `rtl::Reference<SwXText> xText = dynamic_cast<SwXText*>(m_aTextAppendStack.top().xTextAppend.get()); if (!xText) return;` (`DomainMapper_Impl.cxx`:6264-6267). No frame is pushed and
  `bIsInTextBox` is never set.
- A group shape is exactly what puts a **null** `xTextAppend` on that stack:
  `PushShapeContext`'s GroupShape branch pushes `TextAppendContext(uno::Reference<text::XTextAppend>(xShape, uno::UNO_QUERY), …)` under the comment *"A GroupShape doesn't
  implement text::XTextRange"* (`DomainMapper_Impl.cxx`:5088-5092).
- Downstream, a field the importer never created makes `IsFieldResultAsString()` return false
  (`DomainMapper_Impl.cxx`:8953-8968, `bRet = pContext->GetTextField().is() || …`), which sends
  the cached result runs to `appendTextPortion` as ordinary text
  (`DomainMapper.cxx`:4953-4978 — the `IsFieldResultAsString` branch carries the comment
  *"depending on the success of the field insert operation this result will be set at the field or
  directly inserted into the text"* at :4962, and the `else` beside it calls `appendTextPortion`
  at :4974), and
  `PopFieldContext` then inserts no field (`DomainMapper_Impl.cxx`:9228-9232, guarded on
  `xTextAppend.is()`). **That chain is exactly the observed output**: the cached text present,
  the field absent — which is why it is worth recording as the candidate.
- One further candidate for why the *table* matters, also unconfirmed:
  `DomainMapper_Impl::ClearPreviousParagraph`, called from `TableManager::closeCell()`, pops any
  still-open field context whose recorded table depth equals the current one, with the comment
  *"if there are any broken fields (opened in a cell, but not closed), then close them now"*
  (`DomainMapper_Impl.cxx`:5475-5480). It discards the `FieldContext` without inserting anything.

**What the reading does not explain, and I did not resolve:** why the group path fails *only*
inside a table cell. The chain above, read literally, would drop the field for every grouped text
box, and the measurement says it does not — three grouped documents and one synthetic arm keep
their field outside a table. Either the child `wps` inside the group pushes its own non-null
text-append before `startTextBoxContent()` fires and the table changes that ordering, or the
`ClearPreviousParagraph` path is the operative one and the group merely changes when the field
context is still open at cell close. Distinguishing them needs a build with logging, and the
absolute rule forbids one. **Treat §8 as a map of the neighbourhood, not as the mechanism.**

---

## 9. Reach, and why no gate ever saw this

Both documents are `match` rows in gate r133 — `7/7` and `6/6` pages, glyph counts 11547/11527
and 6685/6645, inside the `max(2%, 15)` band. A page number frozen at 6 has the same glyph count
as one that counts 1..7, so **the gate's verdict column is structurally blind to this defect**
and always will be. The census in §7.2 is the instrument that can see it, and it is the one
banked here.

**Nothing was changed.** Our output is 1..N on all four synthetic arms and on both real
documents, which is what the field asks for; matching the reference here would mean reproducing
an import bug.

## 10. Artefacts

| file | what |
|---|---|
| `verified-table.tsv` | §1, per page, both sides |
| `arms.tsv` | every experimental arm in §3, §4 and §6 with its one changed variable and its result |
| `fields.py` | per-header/footer field reporter: instruction, cached result, and whether the field is in a text box, in a group, and whether the drawing is inside a table |
| `textbox-field-contexts.txt` | its output for all eleven text-box documents plus the sibling control |
| `sig.py` | §7.1, the structural census over the 272 `.docx` |
| `frozen-census.py`, `frozen-census.tsv` | §7.2, the behavioural census over all 947 banked reference PDFs |
| `frozen2.py`, `frozen2.tsv` | the widened second pass |
| `odt-loss.py` | §7.3, the export-side census over the 97 |
| `mutate2.py`, `wrap.py` | §3, the two single-variable mutation instruments; they rewrite one `word/footer*.xml` inside a copy of the zip and touch nothing else |
| `pages.sh` | per-page text extractor used throughout |
| `syn/gen.py`, `syn/gen-body.py` | §4, the synthetic generators |
| `syn/syn-{plain,group}-{para,table}.docx` | the 2×2 factorial itself, 5 KB each, authored here |

The mutants of corpus documents are **not** banked — they are derived from third-party corpus
files and `mutate2.py`/`wrap.py` rebuild any of them in a second.
