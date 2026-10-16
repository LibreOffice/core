# Deliberate divergences from 26.2.4.2, and what they cost the gate

The gate scores this tree against LibreOffice 26.2.4.2's own rendering, so every difference from it
normally counts against us. **These ones do not, because they are places where 26.2.4.2 is wrong
about the document and Word is right, and this project wants Word parity.** Each is measured, each
has a switch, and each costs a gate row that no later round should spend time on.

**One switch turns all of them back on**: `XRAY_OFFICE_LIBREOFFICE_QUIRKS=1`
(`XRay.Office.WordProcessing.WordParity`). It is one switch rather than one per rule because it is a
statement about which application the output should resemble, and half of each answer is neither.

**Check this file before working a words-track row that will not close.** It is the sibling of
`TODO.raster-ceiling.md`, which lists the rows the gate cannot win for a different reason: there
the reference rasterises text and our better output scores as the failure; here our output is
better because we read the file the way the application that wrote it does.

---

## A `TOC \t` switch voids a built-in heading's direct paragraph formatting

**26.2.4.2**: a `TOC` field whose `\t` template switch names a built-in `heading N` style makes
every paragraph in that style — anywhere in the document, before the field as well as after it —
lose its direct paragraph formatting. `w:spacing`, `w:jc`, `w:ind`, `w:shd`, `w:numPr` and
`w:keepNext` are all discarded; only `w:pageBreakBefore` and the runs' own `w:rPr` survive. The
`\t` branch of `DomainMapper_Impl::handleToc`
(`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:7663-7707) is where the difference is
introduced; the seat that does the discarding is not located.

**Word**: honours the paragraph.

**This tree**: honours the paragraph. `DocxTocStyles` implements the reference's rule and the
switch above turns it on for a run that wants to agree with LibreOffice instead. Nine arms and
three controls are in
`probes/tocstyle-r160/results.md`, and both halves are pinned by
`DocxTocTemplateStyleTests`.

**Reach**: 21 of the 271 corpus DOCX state a `\t` switch and **7** hold a paragraph it voids, 129
paragraphs in all — `SPA-11_mcar_part-11_v2.9` 87, `OM template for non-complex NCC operators` 18,
`hdss-bulletin-issue-285-25-june-2025` 9, the two FAA Holdover Tables 6 each,
`SPA-06_mcar_part-6_and_IS_v2.9` 2, `report-template` 1.

**What it costs**, measured by rendering all 271 with the rule on and with it off:

| document | 26.2.4.2 | this tree | why |
|---|---|---|---|
| **`words/pagination-001/docx/24-25_FAA_Holdover_Tables.docx`** | **155 pages** | **154** | a gate row this costs |
| **`words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx`** | **167 pages** | **166** | the second, measured in round 177 |
| `SPA-11_mcar_part-11_v2.9.docx` | 49p, 70 763 chars | 49p, 70 763 — *exact* | ours is closer with the rule **off** |
| `SPA-06_mcar_part-6_and_IS_v2.9.docx` | 85p, 142 938 | 85p, 142 938 — *exact* | likewise |
| the other four | — | unmoved on pages and characters | — |

So the rule is worth one page on each of the two Holdover Tables, and switching it off puts two
other documents back on the reference's character count exactly.

**The second Holdover Table was recorded here as unmoved and is not.** Round 177 rendered both with
the switch on: `24-25` goes 154 → **155** pages against the reference's 155 and `FAA 2025-26` 166 →
**167** against 167, their alphanumerics landing within 22 and 20. It had been carried in the *"the
other four"* row above, and that row is wrong for it.

**What that one page is worth on an ink ranking is 321.27 and 93.02**, because every page after the
lost one is then compared against the wrong page. With the switch on, the two documents' summed
unsigned ink is **33.35 and 34.73 with not one MAJOR page between them** — so there is no second
defect underneath either of them, and a round that takes them off the top of an ink table is
chasing this entry. `probes/holdover-r177`.

### Why `24-25_FAA_Holdover_Tables.docx` is 154 against 155, in one paragraph

Its `TABLE 50` caption states `<w:pStyle w:val="Heading3"/><w:spacing w:after="0"/>` over a style
stating `w:after="120"`. We draw the nought the author asked for and 26.2.4.2 draws the style's
6 pt. On that page the table therefore ends at **539.75** here and **545.80** there, against a body
bottom of 558 — and the otherwise-empty `<w:br w:type="page"/>` paragraph that follows the table is
about 14.9 pt of 13 pt Arial. It fits for us and not for the reference, so the reference puts it on
a page of its own, blank, and we do not. Page 72 carries the same 1775 alphanumeric characters on
both sides and no row moves; the whole difference is that one empty page.

**Do not chase this row.** The document is otherwise within **51 characters of the reference over
155 pages** (299 532 against 299 583), and it is page-exact with
the switch set.


---

## A `REF` field is recomputed on load

**26.2.4.2**: recomputes every `REF` naming a bookmark when the document is opened, from the
bookmark's own text — a `SwGetRefField` with `ReferenceFieldSource::BOOKMARK`
(`dmapper/DomainMapper_Impl.cxx`:8541-8635) whose `UpdateField` reads `FindAnchor`'s range
(`reffld.cxx`:1559-1591) through `FilterText` (`:461-489`).

**Word**: draws the cached result. A `REF` is not updated when a document is opened or printed;
only F9, or *update fields before printing*, changes what the reader sees. So the two do **not**
disagree about what a `REF` evaluates to — they disagree about *when*.

**This tree**: draws the cache. `DocxReferenceFields` implements the recomputation behind the
switch; `DocxReferenceFieldTests` pins Word's answer and the reference's side by side, over the
twelve arms of `tests/corpus/features/words-reference-field.docx`. `probes/docxref-r158/results.md`
has the measurements.

**Reach**: 7 of the 271 corpus DOCX hold a resolvable `REF`; **2** state one whose bookmark says
something other than the cache.

**What it costs**:

| document | 26.2.4.2 | this tree | why |
|---|---|---|---|
| **`words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx`** | **167 pages** | **166** | the gate row this costs |
| `Agile_Arc_SysDes.docx` | 20p, 33 905 chars | 20p, 33 897 | unchanged either way |

The reference's own reading of that document's page 7 is the argument for following Word rather
than against it:

```
26.2.4.2   The list of fluids (Tables Table 55, Table 56, Table 57 and Table 58) has been updated …
Word, us   The list of fluids (Tables 55, 56, 57 and 58) has been updated …
```

The four bookmarks cover the word `Table` as well as the number, so recomputing produces that
doubling — and it is what any application would produce on F9, including Word. The difference is
that Word does not press F9 on the reader's behalf. Elsewhere in the same document one note's
`REF` expands to a whole table caption, and the extra wrapped line is the page this costs.

---

## A recomputed field's value takes the field's formatting, not its cached result's

**26.2.4.2**: a field it recomputes is drawn in the character properties the field's own
**instruction run** carries, falling back to the paragraph style. The cached result's `w:rPr` is
discarded entirely — writerfilter builds a `com.sun.star.text.TextField` and deletes the cached
result text with it — and **`\* MERGEFORMAT` makes no difference to it**.

**Word**: honours `\* MERGEFORMAT`, which is what the switch is for — *preserve the formatting of
the previous result*, and the previous result's formatting is exactly the cached runs' `w:rPr`.
Without the switch Word reformats from the field's own, agreeing with the reference.

**This tree**: applies the rule to a field that does **not** carry the switch, which is Word's
answer and the reference's at once, and the switch above applies it to every field. Only to a
field this tree actually recomputes — for anything else the cached text *is* what gets drawn, so
its own formatting is the right formatting. `DocxLayoutSource.RewritesTheResultOf`;
`FieldResultFormatTests` pins the default and `probes/fieldrpr-r168/results.md` holds the seven
arms, of which the reference reproduces six under the switch.

**Reach**: censused on the properties that change the drawn glyphs rather than on the presence of
a `w:rPr` — a result stating only `w:noProof`, `w:webHidden` or `w:lang` is formatted identically
either way — **470 field results in 59 documents** without the switch and **106 in 29** with it.
**The renderings that move are one and four**: in 58 of the 59 the cached result's `w:rPr` happens
to state what the instruction run or the style states anyway.

**What it costs.** Nothing by default: 1 of 59 renderings moves, no gate verdict either way, and
the mover (`AW-104D-RVSM-Aircraft-Approval-Checklist.pdf.docx`) draws its footer page number in
the weight its instruction run states, which is the weight the reference draws the literal `Page`
beside it in. Under the switch, isolated from everything already behind it by rendering the base
binary with the variable set too: 4 of 29 move and no verdict moves.

| document | 26.2.4.2 | default | under the switch |
|---|---|---|---|
| **`words/missing-002/docx/CRIF - Spécification technique - Socle applicatif.docx`** | **29 pages** | **28** | **32** |

CRIF is the row this was found on and the reason the switch is the right home for it. Its footer
holds two `FILENAME` fields in one cell whose cached results state 8 pt grey against a
`Pieddepage` style stating 10 pt black; drawn at 10 pt black the first is about 30 % wider, the
cell overflows, the footer takes a second line and the body bottom rises 11.5 pt — which is the
real defect, and correcting it makes the *line counts* match the reference on pages 5 to 27
exactly. It also unmasks three further local events (at reference pages 3/4, 9/10 and 27/28) that
the footer was hiding, so the page total goes from one out to three out. Those three are the work
that would make the reference's rule safe to prefer here; until then Word agrees with us.

**A separate defect found with it, and fixed unconditionally rather than switched**: a complex
field with no `w:fldChar w:fldCharType="separate"` has no cached result, so there was no span for
pagination to write its value over and the field drew nothing where 26.2.4.2 draws it. With no
cached result there is nothing for `\* MERGEFORMAT` to preserve, so Word and the reference agree
and this is not a parity question. `DocxLayoutSource.PlaceHolder`; reach **0 of the 8 corpus
documents that state one**, because nine of the ten are Word's "page number in a frame" template
leftover that neither engine draws — `probes/fieldrpr-r168` §1.

---

## A VML shape inside a `v:group` loses its paragraph formatting, its numbering and its italic

**26.2.4.2**: a `v:shape` at the top level of a `w:pict` becomes a Writer text frame and its
`w:txbxContent` goes through the whole paragraph machinery. The same shape inside a `v:group`
cannot be a text frame, so it becomes a drawing object whose text is EditEngine text — and almost
none of the paragraph formatting survives that. Measured on thirteen one-property fixtures against
a `w:docDefaults` stating `w:spacing w:after="160" w:line="259" w:lineRule="auto"`:

| property | top level | inside a `v:group` |
|---|---|---|
| `w:docDefaults` spacing | pitch 22.90 | pitch **13.80** — the bare face metric |
| `w:ind w:left="1440"` / `w:firstLine="720"` | 79.3 / 43.3 | **7.2 / 7.2** — dropped |
| `w:numPr` | `1. Alpha` | **`Alpha`** — the number is not drawn |
| `w:jc w:val="center"` | that paragraph | **and the paragraph after it** |
| `w:i` | italic | **upright** |
| `w:sz`, `w:b`, `w:rFonts` | applied | applied |

`oox::vml::TextBoxContext` reads two things out of a `w:pPr` — `w:jc` and `w:pStyle`
(`oox/source/vml/vmltextboxcontext.cxx`:265-275) — and `TextBox::convert`
(`vmltextbox.cxx`:78-160) appends the portions with `ParaAdjust` and six character properties. A
paragraph break is itself a portion appended as `"\n"` carrying that paragraph's model, which is
why the adjust lands on the paragraph the break *opens* and leaks forward exactly one paragraph.
The character half of that reading does not describe Writer — `w:sz`, `w:b` and `w:rFonts` arrive
by some other route while `w:i` falls between the two — so the arms are the authority and the
file:line is the explanation of the paragraph half only.

**Word**: honours all of it — the spacing, the indents, the numbering and the italic.

**This tree**: honours all of it, by running the same machinery whether or not the shape is
grouped. Thirteen arms in `probes/vmlboxtext-r172/results.md`; every top-level arm is reproduced
exactly.

**Reach**: **113 of the corpus's 166 reachable VML text boxes are inside a `v:group`, and all 113
are in the five `06x_Work_Breakdown_Structure_Template` documents.** (The 1638 a markup census
gives are 1472 `mc:Fallback` halves that neither renderer reads, plus 53 top-level boxes where the
two engines agree — `probes/vmlinset-r171/results.md`.)

**What it costs**: nothing the gate can see. All five are one page and all five are `match` after
round 171, and their summed unsigned ink against 26.2.4.2 is 0.02 to 0.07. The residual it
accounts for, each span paired against the reference's within its own box
(`probes/vmlgroup-r172/inbox.py`):

| document | spans | mean \|dx\| | mean \|dy\| | max \|dy\| |
|---|---:|---:|---:|---:|
| `068` | 26 | 0.10 | 0.60 | 0.80 |
| `066` | 24 | 0.04 | 1.34 | 1.60 |
| `067` | 17 | 0.04 | 1.48 | 1.60 |
| `065` | 10 | 1.62 | 4.20 | 15.20 |
| `069` | 66 | 24.93 | 27.14 | 63.70 |

`069` is the pitch alone: 23.10 against 14.00, which is `1.079 × 13.97 + 8` against the bare 13.97.

**Not switched, and this one is deliberate rather than pending.** The other entries in this file
each have a switch because reproducing them is a *reading* of the file that can be turned on and
off at one seat. This one is not a reading: it would mean routing a grouped shape's text through a
different layout engine, so that an author's numbering, indents, spacing and italic are discarded
on purpose — for five one-page templates that already pass, and 0.05 of ink. The measurement is
the deliverable; if a document ever turns up where the pitch costs a page inside a fixed-height
box, this is the entry that explains it.

**And one thing in `069` is not this rule.** Its `SUBTASK` is drawn 59.80 pt wide by the reference
and 55.67 by us in the same face at the same size. That is `CLAUDE.md`'s **seventh confound** —
measuring on the draw layer in the class-ful family and drawing in the class-less one — reaching a
Writer document, which that section says body text cannot show *because `SwTextPainter` never
becomes a drawinglayer primitive*. A grouped VML shape does.

---

## An inherited header or footer gains an empty paragraph

**26.2.4.2**: a section that states no `w:footerReference` — OOXML's *"link to previous"*, §17.10.1
— gets the previous section's footer as a **copy carrying one extra empty paragraph**, which makes
the footer a line taller and pushes its content a line up the page. The same applies to a header.
LibreOffice cannot link header or footer *content* between page styles, and the comment above the
copy says so (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1118-1124, *"LO does not support
linking of header/footer content across page styles so we just copy the content from the previous
section"*). The seat is `copyHeaderFooterTextProperty` (`:935-960`), which calls
`removeXTextContent` (`:518-526`) and then `copyText`: the removal empties the target with
`setString("")` and appends and disposes one paragraph, and since a text body cannot be left with
none, the target still holds an empty paragraph when the source's content arrives beside it.

**Word**: shows the linked footer, unchanged.

**This tree**: shows the linked footer, unchanged. `DocxReader.FurnitureCarry` carries the slot
across the sections and nothing is appended. **Not switched** — see below.

**Measured on `words/pagination-002/docx/docs-quality-MA.IMS.00001-Integrated-Management-System-manual.docx`**,
whose sections 4 and 5 state a `w:headerReference` and no `w:footerReference`. Footer text bottom
edge, in points, on an 841.89 pt page with `w:footer="567"`:

| pages | the section's own `w:footerReference` | this tree | 26.2.4.2 |
|---|---|---:|---:|
| 1–7 | stated | 813.55 | 813.55 |
| 8–44 | absent, so inherited | 813.55 | **802.05** |

11.50 pt, constant, and the split falls exactly at the first inheriting section — the same document
and the same footer part on both sides of it. Appending `<w:p><w:r><w:t>XTRAIL</w:t></w:r></w:p>`
after that footer's table draws `XTRAIL` at y 802.40–813.56, filling the gap without moving the
copyright block, so the space is one paragraph's and the reference had already reserved it. The
reference's own `--convert-to fodt` writes `<text:p text:style-name="Footer"/>` after the table in
the two master pages built for the inheriting sections and not in the three built for sections that
name their own. Both directions are in `probes/footerlink-r183/`, with a two-section synthetic that
reproduces the spare paragraph from scratch.

**Reach**: not censused. It needs a document with more than one section, at least one of which
links its footer or header to the previous, and a running foot tall enough for a line to matter.

**What it costs: 0.29 of ink and no gate row.** Supplying the paragraph in the file and re-rendering
this tree against the unmodified reference takes the document from 10.22 unsigned ink to 9.93 and
from two major pages to one — while opening a two-page drift at page 34. That is why it is **not
switched**: the rule is a defect in the reference's importer rather than a reading of the file, the
whole of what it buys is three short lines moving 11.5 pt on 37 pages, and reproducing it would
cost a pagination it currently gets right. The measurement is the deliverable.

---

## A continuous section's own furniture arrives a page late

**26.2.4.2**: a continuous section that states a running head of its own does not get it on the
page where the section begins. Writer cannot give a page style to part of a page, so the importer
builds the descriptor and then hunts forward for the first *hard page break* inside the section to
hang it on — and the section's paper, margins, running head and numbering all arrive there
together, a page or more after the section did. The WW8 importer says so in terms
(`sw/source/filter/ww8/ww8par.cxx`:4518-4560, *"In this nightmare scenario the continuous section
has its own headers and footers so we will try and find a hard page break between here and the end
of the section and put the headers and footers there"*), and `SectionPropertyMap::CloseSectionGroup`
does the same for DOCX and RTF (`dmapper/PropertyMap.cxx`:1746-1801).

**Word**: shows a section's own header on the first page that is wholly that section's.

**This tree**: the same as Word. `ContinuousPageDescriptors` models the *drop* — a descriptor that
can be hung nowhere is lost, which is Word's behaviour too, since a section that never gets a page
of its own never gets its page setup either — but not the *deferral*, and the two are different
claims. **Not switched**, for the same reason the entry below this one is not: deferring a
descriptor to a later page is an importer's workaround for a model limitation this tree does not
have, and reproducing it means drawing the wrong running head on a page on purpose.

**Measured on `words/done-014/doc/PK_FlugzeugeStricken.doc`**, whose second section is continuous,
begins at CP 738 — page 2 — and states its own running head. The reference draws section 0's
full-page letterhead image on pages 1 **and** 2 and the `SIGL / SCHAFFLER / STOLLBERGER / KASTNER`
running head from page 3, the first hard page break inside the section (CP 2936); we draw the
letterhead on page 1 and the running head from page 2. Two blind readings of the composed pairs,
given no numbers, reported the split independently. `probes/docsection-r184/`.

**Reach**: not censused as a class. In this corpus it is one document's one page.

**What it costs: 6.42 of unsigned ink on one page**, which as of round 184 is the worst single page
in the words track — and the reason the track's head is worth re-reading rather than worked. The
same document's *other* six pages come to 0.71 between them once the margin half of this rule was
fixed, which was ours and is in `probes/docsection-r184/` as well.

---

## Still LibreOffice's reading, and why

These are places where this tree recomputes a field that Word would leave at its cache, and they
are **not** switched: each computes something the file cannot state correctly, so the cache is
stale by construction rather than merely unupdated.

- **`PAGE`, `NUMPAGES`, `SECTIONPAGES`.** Pagination decides them and Word maintains them too.
- **`FILENAME`, `TITLE`.** The cache is a statement about a file that no longer exists; the
  measurement that put them in is in `ConstantFields`' own remarks (351 of one document's 363-word
  gap). Word would show the cache here, so this is the closest of the three to being switched —
  and it is left because the value it draws is right about the document in hand where the cache is
  right about a previous one.
- **`STYLEREF`.** Page-dependent by nature, like `PAGE`: it quotes the heading in force, which
  Word recomputes as the layout moves. `DocxLayoutSource.StyleReferenceText` already declines
  inside a running head, where this tree cannot answer per page.

**Not yet decided: the RTF bookmark rotation (round 88).** `RtfBookmarkRotation` reproduces a
*writerfilter bug* — RTF sends a bookmark half's name before its id and the mapper is written for
the opposite order, so every name after the first lands one bookmark early. Word has no such bug.
Under this file's rule it should be off; what stops that being a one-line change is that the
corpus holds no `.rtf` at all, so the only RTF evidence is `/home/user/corpus-odf`'s 338 files —
**written by LibreOffice itself**, where reproducing LibreOffice's own read of its own output is
not obviously wrong. Measure the reach on real Word-written RTF before switching it.
