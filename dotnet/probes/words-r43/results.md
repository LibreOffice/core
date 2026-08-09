# Words round 43 — measured results

Predictions in `prediction.md`, committed at `a9ff7496b` before anything was measured post-change.

## Baseline

Reproduced exactly at `cce1cc314`, over `words/batch-*`, 200 rows, no duplicate paths:

| | brief | measured |
|---|---:|---:|
| documents matching | 154 | **154** |
| absolute page error | 78 | **78** |
| exactly-correct page counts | 164 | **164** |
| absolute word error | 6512 | **6512** |

`baseline.tsv`.

## The scoreboard after

**Unchanged, by construction.** No file under `dotnet/src` is touched this round. The corpus was
rendered once and classified twice.

| | baseline | after | predicted |
|---|---:|---:|---|
| documents matching | 154 | **154** | 154 |
| absolute page error | 78 | **78** | 78 |
| exactly-correct page counts | 164 | **164** | 164 |
| absolute word error | 6512 | **6512** | 6512 |
| renderings changed | — | **0** | 0 |

---

## Task one — why three documents over-draw their running head

### Refuted: two of the three do not over-draw a running head at all

The brief names `UG.CAO.00133 … Language.docx`, `A1. EASA Form 2.docx` and
`B11. TE.CAO.00129  Experience  logbook.docx` as one class. They are not one class, and the census
says so in one line:

| document | sections | header references |
|---|---:|---|
| `A1. EASA Form 2.docx` | **1** | one `default` |
| `B11. TE.CAO.00129  Experience  logbook.docx` | 3 | a `default` in **every** section |
| `UG.CAO.00133 … Language.docx` | 5 | a `default` in sections 0 and 3 only |

A document with one section and one default header has no inheritance question to get wrong, and a
document naming a default header in every section has none either. Their surplus is pagination:
ours runs **9 pages against 7** and **7 against 6**, and every extra page carries the same running
head and footer the reference's pages carry. Per-page extracted words confirm a reflow rather than
a furniture difference — ours `[503, 246, 180, 479, 198, 95, 207, 432, 97]` against the reference's
`[451, 199, 225, 312, 395, 314, 345]`.

So the residual over-draw round 42 recorded for those two is a **page-count defect being counted
in words**, and the running-head question is one document's, not three.

### The rule, established from both ends

The reference draws the head on pages **1, 14, 15, 16 and 17** of `UG.CAO.00133` and we draw it on
all eighteen. Pages 14–17 are the landscape section 3; page 1 is section 0. Those are exactly the
two sections naming a `default` header. Sections 1, 2 and 4 name an *even* and a *first* header,
both parts empty, and no default.

Round 42's refutation reproduces: `header-link-to-previous.py` authors six two-section shapes and
LibreOffice inherits in all six. **Our link-to-previous rule is right.** What round 42 then assumed
— that the section's header references are the variable — is wrong:

`header-slot-mutations.py` mutates the real document one slot at a time.

| variant | head on |
|---|---|
| as-is | 1, 14, 15, 16, 17 |
| every `w:headerReference` deleted from sections 1, 2, 4 | **1, 14, 15, 16, 17** |
| a `default` reference added to sections 1, 2, 4 | 1 … 18 |
| only `first` kept / only `even` kept | 1, 14, 15, 16, 17 |
| the empty even/first parts filled with text | 1, 14, 15, 16, 17, and the filled text on **no page** |

A section naming nothing at all is the shape §17.10.1 calls link-to-previous, and it still gets no
head. So the variable is elsewhere.

`header-inherit-bisect.py` cuts the document to its first two sections — three pages, the whole
effect — and replaces one part at a time:

| variant | head on page 2 |
|---|---|
| cut to two sections | no |
| `word/settings.xml` minimal | no |
| `word/styles.xml` minimal | no |
| the footer reference dropped | no |
| `w:cols` and `w:noEndnote` dropped | no |
| section 0's whole body replaced by one paragraph | no |
| **`word/header1.xml` replaced by one line of text** | **yes** |
| **the header's tables removed, its runs kept** | **yes** |
| **a bare `<w:p/>` added after the header's last table** | **yes** |
| **a bare `<w:p/>` added before its first** | **yes** |

`header-inherit-content-shape.py` authors the same finding from the other side, holding the section
markup at round 42's inheriting shape and varying only what the header holds: text, a table, a
nested table, an inline image, a table with the image in it, a paragraph then a table, and a table
with **no** paragraph beside it. Eight variants, seven inherit, and the one that does not is the
corpus shape exactly.

> **LibreOffice copies a section's header into a following section only when the source header
> holds at least one top-level `w:p`. A header whose content is nothing but tables arrives empty.**

### Why this is the reference's defect and not a rule to copy

The document supplies its own control. `word/header1.xml`'s only top-level child is a `w:tbl`;
`word/footer1.xml`'s only top-level child is a `w:p`. Section 1 names neither a header nor a footer
of its own — and the reference prints the **footer** on all eighteen pages while printing the header
on five. One document, one inheritance mechanism, opposite outcomes, and the only difference is the
kind of the top-level node.

Three more readings point the same way:

- LibreOffice's flat-ODF export of the document gives `Converted1`, `Converted2` and `Converted4` a
  `<style:header>` that is **present and empty** — header on, no content. A decision not to link
  would leave the header off.
- The identical header content, *named* by section 3's own `w:headerReference`, draws perfectly on
  all four landscape pages. Named it draws; copied it vanishes.
- LibreOffice's own source says it means to link: *"Any headers and footers which were not defined
  in this docx section should be 'linked' with the corresponding header or footer from the previous
  section. LO does not support linking … so we just copy the content"* —
  `sw/source/writerfilter/dmapper/PropertyMap.cxx:1118-1146`. The copy is
  `removeXTextContent` followed by `copyText` inside a `try`/`catch` that logs and returns, which is
  what an empty target looks like from outside. (Cited as a mechanism, not as evidence: the
  reference binary is 24.2.7.2 and the tree is a development branch.)

CLAUDE.md's rule for exactly this case is to record it as a known deviation rather than contort
Paperless into reproducing it, and reproducing it would mean deleting a running head Word draws.
**So it is not implemented, and the decision is pinned by `SectionInheritedHeaderTests` rather than
left in a report** — a later round that wants the verdict has to delete a test that says why not to.

### What it costs, stated plainly

`table-only-header-census.py`: **3 of the 134 DOCX** in the track have a section inheriting a
table-only header. The census cannot see the **66 `.doc`**, whose header stories live in the WW8
text stream.

| document | pages | words | if the deviation were reproduced |
|---|---|---|---|
| `UG.CAO.00133 … Language.docx` | 18/18 | 3944/3700 | the 244-word surplus is exactly the head on the 13 bare pages — it would **match** |
| `UG.CAO.00006 … .docx` | 30/29 | 8124/7458 | word error falls; the page count still differs, so no verdict |
| `docs-quality-MA.IMS.00001 … .docx` | 43/44 | 12174/12397 | word error **rises**; it already fails on pages |

**One verdict, and it is the only one.** Recorded so the choice can be revisited with the number
rather than re-derived.

---

## Task two — the instrument round 42 diagnosed

### What changed in `pdf-ops.py`

Two additive changes, both in the diff path only; `dump` is untouched.

- **`pairable()`** refuses to pair two *linear* records that run in different directions. A record
  is linear when its major side is at least `LINEAR_RATIO = 3` times its minor one, so a panel or a
  shading is exempt and keeps the old behaviour — a 100x99 fill against a 100x101 one must not be
  split into two one-sided records over two points.
- **The tie-break.** Two rules meeting at a table corner have *identical* anchors, so the
  nearest-neighbour distance is exactly equal and whichever was written first won. The tie is now
  broken by the closer major extent, which can only fire on an exact tie.
- **`hairline WxH vs WxH`** is a new note name for a box note whose longer side is under 20 pt —
  a segment of a flattened curve rather than a rule. `first-divergence.py` gains a matching
  `hairline` class, placed last so that a tie in the dominant count favours a real class.

### The anatomy, over the 43 documents carrying a box note on their first divergent page

| | notes | cross | hairline | rule |
|---|---:|---:|---:|---:|
| published matcher | 486 | **142** | **146** | 198 |
| cleaned matcher | 315 | **9** | 94 | 212 |

Round 42's two headline counts reproduce **to the digit** on a different job list and a different
build — 142 cross and 146 hairline — which is the corroboration that its anatomy was measuring what
it said. Its third figure, 151 rule, does not: my list is 43 documents at first-divergent pages that
moved when round 42's own fix landed, and the published rule count over it is 198.

Three movements, and the middle one is the finding:

- **Cross 142 → 9, and only 2 of the 9 are rule-scale.** Those two are fills of aspect ratio 1.5 and
  1.4 — a shaded panel and a page-sized white rectangle — where "orientation" is not a property of
  the thing. The other seven are segments under 15 pt inside one logo, already labelled `hairline`
  in the note. **No pair of rule-scale linear records is now matched across orientations.**
- **Rule 198 → 212.** A cross pair is not noise standing in for nothing; it is frequently a *real*
  same-orientation difference whose true partner was stolen. On `UG.CAO.00133` page 1 the published
  matcher reports `v 36.6 vs h 486.9`, `v 36.6 vs h 0.6` and `h 487.1 vs v 26.4` — three sentences
  about nothing — and the cleaned matcher reports three vertical rules, ours 36.6 pt where the
  reference's are 26.4, 25.5 and 26.4. **The header table's cell edges are about 10 pt taller in our
  rendering**, which is a coherent measurement the instrument was hiding.
- **Hairline 146 → 94.** Fifty-two of them were pairings that, once each segment could find its own
  orientation, agree within the window and produce no note at all.

### The dominant-class table, over all 200

Same `first_page` for every row — only the classification is re-run, by
`probes/words-r43/reclassify.py`, so nothing here can be an artefact of a second image diff.

| dominant | published match | published fail | cleaned match | cleaned fail |
|---|---:|---:|---:|---:|
| `box` | 5 | 9 | **4** | **8** |
| `hairline` | — | — | 0 | 1 |
| `glyphs` | 54 | 28 | 55 | 28 |
| `one-sided` | 13 | 5 | 13 | 5 |
| `size` | 4 | 3 | 4 | 3 |
| `face` | 5 | 0 | 5 | 0 |
| no divergent page | 71 | 0 | 71 | 0 |

**The prediction was wrong and it is worth reading rather than rounding off.** `prediction.md` said
`box` would land near 5 failing and 3 matching, which is where round 42's post-filtered table put
it. Measured: **8 failing and 4 matching**. Post-filtering and repairing are not the same operation:
dropping a cross note assumes there is nothing under it, and the paragraph above shows there
usually is. Only **two of 200 documents** change dominant class at all — one matching document
`box → glyphs`, and `150-5370-10H.docx` `box → hairline`, which is precisely the document round 42
identified as 236 notes of one 12 pt graphic.

### The control: run it over the documents that already match

| | documents | notes | cross | hairline | rule |
|---|---:|---:|---:|---:|---:|
| matching, published | 21 | 100 | 28 | 0 | 72 |
| matching, cleaned | 21 | **74** | **1** | 0 | **73** |
| failing, published | 22 | 386 | 114 | 146 | 126 |
| failing, cleaned | 22 | 241 | 8 | 94 | 139 |

**73 same-direction rule-scale notes fire on 21 documents that match the reference exactly.** The
instrument is repaired and the class it produces is still not a discriminator: `box` is dominant on
4 matching documents against 8 failing, and its underlying notes are as common on correct output as
on wrong. Read a `size WxH vs WxH` note as a lead about one rule, never as evidence a document is
broken — a table rule half a point out is invisible at every raster scale this project measures at,
and both renderers draw thousands of them.

### What a stored measurement from before this change means afterwards

A `box` count recorded by rounds 34, 39 or 42 is the sum of three populations: same-orientation
rule differences, hairline segments of flattened curves, and cross-orientation pairs that the
matcher no longer makes. Only the first is still spelled `size WxH vs WxH`. Round 42's *published*
column reproduces exactly against the current `pdf-ops.py` in the main checkout, so any number in
its report can still be re-derived — with `PDF_OPS` pointing there, which
`box-note-anatomy.py` now takes as an environment variable.
