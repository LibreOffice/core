# blankpage-r163 — the blank page 7 of `ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx`

Document: `/home/user/sample-files/words/ceiling-001/docx/ABCD-FE-01-00 Flight Envelope - v1 08.03.16.docx`
Reference: `/opt/libreoffice26.2/program/soffice` — 26.2.4.2, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`.
Ours: `dotnet/tools/XRay.Office.Cli/.../XRay.Office.Cli` (prebuilt Release, used as-is; no C# was changed and
nothing was built).

Every claim below is marked `[src]` (read from a file) or `[bin]` (measured from a run).

---

## 1. The divergence, confirmed and corrected

| | reference 26.2.4.2 | XRay.Office |
|---|---|---|
| pages | **16** | **14** |

`[bin]` The user's report is right about page 7 and understates the total: we are **two** pages short,
not one, and the offset does **not** stay at one to the end.

Page-by-page alignment `[bin]` (first body line of each page):

| ref | ours | first line |
|---|---|---|
| 1–6 | 1–6 | identical starts, identical ends |
| **7** | — | **blank** (header + footer only, no body) |
| 8 | 7 | `Therefore aeroplane lift coefficient is estimated to …` |
| 9 | 8 | `4.4 Flaps maximum operating speed VF` |
| 10 | 9 | ours starts one line later (`(Note: In case the demonstrated VDF …`) |
| 11 | 10 | `6.1 Gust envelope` |
| 12 + 13 | 11 | the gust-load table + `Figure 1` — **ref spends two pages, we spend one** |
| 14 | 12 | `Compliance statements` |
| 15 | 13 | compliance table |
| 16 | 14 | compliance table, end |

So: **offset +1 from ref p7 through ref p11, then +2 from ref p14 to the end.** It never resynchronises.
The second page is lost in the gust-envelope region and is a **different** defect (§6).

---

## 2. Hypotheses ruled out

**There is exactly one `w:sectPr` in the whole document** `[src]` — the final body one, and it carries
**no `w:type`**:

```xml
<w:sectPr …><w:headerReference …/><w:footerReference …/>
  <w:pgSz w:w="11906" w:h="16838" w:code="9"/>
  <w:pgMar w:top="1440" w:right="850" w:bottom="1440" w:left="850" w:header="706" w:footer="706"/>
  <w:cols w:space="708"/><w:docGrid w:linePitch="360"/></w:sectPr>
```

The reference's own resolved view agrees `[bin]` — `soffice --convert-to fodt` gives **one**
`style:master-page` (`Standard`), **one** `style:page-layout` (`pm1`), **zero** `text:section`, and
**zero** `style:master-page-name` on any paragraph style, i.e. no page-style switch and no page-number
restart anywhere in the document.

| hypothesis | verdict |
|---|---|
| 1. odd/even page section start (`w:type="oddPage"`/`"evenPage"`) | **Ruled out.** No section break of any kind exists. (We *do* implement parity starts — `DocxPageGeometry.cs`:77-78 maps them to `SectionBreak.EvenPage`/`OddPage` and `Paginator.cs`:1303 inserts the filler page — it is simply not reached here.) `[src]` |
| 2. `w:type="nextPage"` against an empty section | **Ruled out.** Same reason: no section break. |
| 3. `w:br w:type="page"` next to a section break, coalesced by one side | **Ruled out as stated**, but it is the right neighbourhood. There are nine `w:br w:type="page"` `[src]` and no section break for any of them to sit next to. The construct that matters is a **page break alone in its own paragraph** — see §3. Neither side coalesces it, and both handle it identically (§5, fixture A). |
| 4. a table or frame whose height forces a spill | **Ruled out for page 7** — ref page 7 has no body content at all, not invisible content. It *is* the cause of the **second** lost page (§6). |
| `w:pageBreakBefore` | **Absent.** Zero occurrences in `word/document.xml` `[src]`. |

---

## 3. What actually produces the blank page

The XML at the page 6/8 boundary `[src]`:

```xml
<w:p …><w:r><w:t>…lowered by 15% with respect to the one of the profile.</w:t></w:r></w:p>
<w:p w:rsidR="00A479C9" …><w:r><w:br w:type="page"/></w:r></w:p>   <!-- nothing else in it -->
<w:p …><w:r><w:lastRenderedPageBreak/><w:t>Therefore </w:t></w:r>…</w:p>
```

The reference's resolved model of exactly that `[bin]`:

```xml
<text:p text:style-name="P33">Considering … lowered by 15% … .</text:p>
<text:p text:style-name="Standard"/>                      <!-- the break-only paragraph, now EMPTY -->
<text:p text:style-name="P62">Therefore aeroplane …</text:p>
```
with `P62` carrying `fo:break-before="page"` and `Standard` carrying
`fo:margin-bottom="0.139in"` (10.008 pt) and `fo:line-height="115%"` `[bin]`.

So LibreOffice's importer **moves the break off the paragraph onto the next one and leaves the now-empty
paragraph behind** — which is exactly what our own reader does too (`DocxLayoutSource.cs`:~2130,
`case "br"`: `w:type="page"` sets `_pageBreakAt` and emits no character, with the comment citing
`DomainMapper.cxx:4379`'s `BreakType_PAGE_BEFORE`) `[src]`.

That makes the blank page a pure **fit** question:

* if the empty paragraph **fits** at the foot of page 6 → it stays on page 6, `Therefore…` opens page 7,
  **no blank page** (us);
* if it **does not fit** → it moves to page 7, `Therefore…`'s `break-before` then opens page 8, and page 7
  holds nothing but an empty paragraph → **blank page** (the reference).

---

## 4. Root cause — a symptom of an earlier divergence, inside page 6

**Both engines apply the same fitting rule. What differs is how tall page 6's content is.**

`[bin]` Last body line on page 6, ink `yMax` (body bottom is 769.89 pt = 841.89 − 72):

| | ref | ours | ref − ours |
|---|---|---|---|
| page 6 last body line | 748.199 | 739.959 | **+8.240 pt** |

`[bin]` Where those 8.24 pt accumulate — line tops down page 6, ref minus ours:

| through line | Δ | what happens there |
|---|---|---|
| …52 (`maximum speed in …`) | 0.10 | agreement |
| 53 → 54 | **3.15** | the display formula `V_H = 130 kts` (`m:oMathPara`) |
| 55 → 58 | 3.22 | agreement |
| 59 | **4.88** | inline `m:oMath` in `The maximum lift coefficient …` |
| 62 (last) | **8.24** | further inline `m:oMath` in the same block |

Every point of the 8.24 pt is an OMML formula. Nothing else on page 6 differs.

### The decisive measurement: a slack sweep on the real document

`probes/blankpage-r163/build-slack.py` writes variants of the **real** document whose only change is an
explicit `w:spacing w:after` on the last paragraph of page 6, swept in 0.25 pt steps. The value at which
the empty paragraph is pushed off page 6 is each engine's remaining slack:

| | flips at | page count below / above |
|---|---|---|
| reference 26.2.4.2 | **8.50 pt** | 15 → 16 |
| XRay.Office | **16.75 pt** | 14 → 15 |

`[bin]` **Difference: 8.25 pt — the same 8.240 pt measured on the page.** The fitting rule is therefore
*identical*; only the content height differs. The real document supplies 10.008 pt of inherited
`margin-bottom` at that point, which is **above** the reference's 8.50 pt threshold (blank page) and
**below** our 16.75 pt (no blank page). Closing the 8.25 pt gap moves our threshold to the reference's and
the blank page appears.

### So: the blank page 7 is a downstream symptom. The defect is OMML line height.

`[src]` **We have no OMML handling at all.** `git grep -n 'oMath'` over `dotnet/src` returns nothing. The
paragraph walker `DocxLayoutSource.Append(XElement, int)` dispatches on `Name.LocalName` only, so:

* `m:oMathPara` / `m:oMath` fall into `default:` and are recursed into;
* `m:r` hits `case "r"` and its `<w:rPr>` (`Cambria Math`) becomes the ambient run properties;
* `m:t` hits `case "t" when !Suppressed: Emit(child.Value)`.

The formula therefore becomes **ordinary inline text**, and the line reserves the fallback face's text
height. `Cambria Math` is not installed; `fc-match "Cambria Math"` → **FreeSerif** `[bin]`, which is the
second face in our PDF (`pdffonts`: `Carlito-Regular`, `FreeSerif`) `[bin]`.

LibreOffice instead imports every `m:oMath` into a StarMath object anchored **as-char**, and the line takes
the object's height. In the fodt the object is a `draw:frame … text:anchor-type="as-char"` with an explicit
`svg:height`, `draw:style-name` parented to `Formula` `[bin]`.

### The reference's rule, measured exactly

`probes/blankpage-r163/build-mathsizes.py` writes one document per OMML shape, `before` / formula / `after`.
`svg:height` is read from 26.2.4.2's own fodt; the pitch is the `before`→`after` line-top distance in each
PDF `[bin]`:

| formula | LO `svg:height` | ref pitch | our pitch | ref − ours |
|---|---|---|---|---|
| `x` | 13.349 pt | 50.900 | 49.350 | +1.550 |
| `√x` | 14.400 pt | 51.850 | 49.350 | +2.500 |
| `x²` | 15.055 pt | 52.500 | 49.350 | +3.150 |
| `x₁` | 15.134 pt | 52.600 | 49.350 | **+3.250** |
| `x₁²` | 16.841 pt | 54.300 | 49.350 | +4.950 |
| `a/b` | 28.462 pt | 65.900 | 49.350 | **+16.550** |
| `(a/b)/c` | 43.538 pt | 81.000 | 49.350 | **+31.650** |
| `x` at `w:sz 8pt` | 13.349 pt | 50.900 | 45.550 | +5.350 |
| `x` at `w:sz 20pt` | 13.349 pt | 50.900 | 60.750 | **−9.850** |
| `x₁` at `w:sz 20pt` | 15.134 pt | 52.600 | 60.750 | −8.150 |

Two laws fall straight out `[bin]`:

1. **`ref pitch − svg:height` is constant at 37.45 ± 0.02 pt over all ten** (the two plain paragraphs'
   contribution). So the reference's math-paragraph height **is exactly the object's `svg:height`** —
   an exact law, not a correlation.
2. **`w:sz` is ignored by the reference and honoured by us.** `plain`, `plain-sz8` and `plain-sz20` all
   resolve to the same 0.1854 in; `sub` and `sub-sz20` both to 0.2102 in. LibreOffice lays the formula out
   at StarMath's own fixed base size (12 pt default). Our pitch, by contrast, is **49.350 pt for every shape
   at the default size** — a nested fraction costs us exactly what a bare `x` does — and moves with `w:sz`.

Our height for a bare `x` is 49.350 − 37.45 = **11.90 pt** against the reference's 13.349 pt. We also
reserve *less than our own ink*: in fixture `mathline-math` our line box is 12.12 pt while the ink we draw
into it is 13.22 pt tall `[bin]`.

---

## 5. Fixtures and their verdicts

All fixtures are built by copying `word/styles.xml`, `word/settings.xml`, `word/fontTable.xml`,
`word/numbering.xml` and `word/theme/theme1.xml` **verbatim from the real document**, so they inherit the
same OOXML compatibility defaults. `unzip -l` confirms `word/settings.xml` is present in each `[bin]` —
the trap named in `office-corpus/SKILL.md`. The `slack/` family is the *whole real document* with one
attribute changed, so it needs no such validation.

### Fixture A — `fixtures/breakpara-fillNN.docx` (the blank-page mechanism itself)

`NN` plain paragraphs, then a paragraph holding only `<w:br w:type="page"/>`, then `AFTER`.

| NN | reference | XRay.Office |
|---|---|---|
| 24–26 | 2 pages, no blank | 2 pages, no blank |
| **27** | **3 pages, page 2 blank** | **3 pages, page 2 blank** |
| 28–33 | 3 pages, page 2 blank | 3 pages, page 2 blank |

`[bin]` **Exact agreement, same flip point, same blank page.** Our handling of a break-only paragraph and
of the blank filler page it makes is correct. This is the control that proves §4's conclusion: the rule is
not the bug.

### Fixture B — `fixtures/mathline-{plain,math}.docx` (the actual defect)

Six paragraphs; in `math` the third is the real document's `V_H = 130 kts` `m:oMathPara`, copied verbatim.

| | ref | ours |
|---|---|---|
| `mathline-plain`, all six line tops | 72.029 … 199.279 | 72.028 … 199.278 — **identical to 0.001 pt** |
| `mathline-math`, last line top | 198.979 | 195.928 |
| math paragraph height, relative to a plain one | −0.30 pt | −3.35 pt |
| **per-formula deficit** | | **3.05 pt** |

`[bin]` **Cross-check against the real document:** the fixture's 3.05 pt per formula, over the three
formula-bearing paragraphs on page 6, predicts ≈ 8–9 pt; the page measures **8.240 pt** and the slack
sweep on the real document measures **8.25 pt**. Three independent instruments, same number.

### Fixture C — `mathsizes/math-*.docx` — the per-shape table in §4.

### Fixture D — `slack/slack-NNNpt.docx` — the real document, sweep in §4.

Recommended for the corpus: `fixtures/mathline-math.docx` + `fixtures/mathline-plain.docx` as
`tests/corpus/features/`, since the pair isolates the defect to one number with a control beside it.

---

## 6. The second lost page (ref 12+13 → ours 11) — a different defect

`[bin]` Reference page 11 stops at y = 402.9 with ~355 pt of empty page below it, and moves the whole
gust-load table to page 12, where it runs 91.3 → 564.7 (≈ 473 pt, taller than the 355 pt left). We split
that table across our pages 10 and 11 instead. So the reference kept a table whole that we broke. That is a
table page-break / keep-together difference, **not** the OMML line height, and it is out of scope here.

Also noted while aligning `[bin]`: the reference numbers the last two headings `7. V-n Envelope` and
`8. Compliance statements` where we number the latter `5.` — a heading-numbering defect, again separate.

---

## 7. Proposed patch — not applied

### 7a. The fix

**File** `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs`
**Method** the paragraph walker's `private void Append(XElement element, int depth)` — the
`switch (child.Name.LocalName)` at ~line 1918/2065-2250.

**Current behaviour** there is no case for `oMath` or `oMathPara`. They reach `default:` and recurse, so
`m:r` is handled by `case "r"` and `m:t` by `case "t"`, and the formula is laid out as plain text in the
run's own size in whatever face `Cambria Math` falls back to. The line reserves that text's height.

**Corrected behaviour** add

```csharp
case "oMath" or "oMathPara":
    _frames.Add(new FrameAnchor(_builder.Length, child));
    Append(child, depth + 1);      // keep emitting the text: ours stays searchable
    break;
```

placed before `default:`, and no anchor character (the formula's own text already holds the place).

**Supporting change** `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxFrames.cs`, `DocxFrames.Read`: accept
an `m:oMath` / `m:oMathPara` element and return a `PageFrame` with `Anchor = FrameAnchor.AsCharacter`,
`InlineExtent = new DocSize(Length.Zero, MathHeight(formula))` and nothing to draw. Zero width, so the
formula's text keeps its own advance and only the **height** is contributed; `IsTextPortion = true` so
`PageContent.ShapeLineTwip` adds no twip (it is not a draw object). `PageContent.InlineObjects`
(`Layout/PageContent.cs`:498) then folds it into the line maxima through machinery that already exists and
is already exercised by inline pictures.

**`MathHeight`** must return StarMath's layout height of the formula **at a fixed 12 pt base size,
ignoring `w:sz`** — law 2 of §4. The measured anchor points for a first implementation, all `[bin]`:

| structure | height |
|---|---|
| single row, no script | 13.349 pt |
| one subscript *or* superscript | 15.055–15.134 pt |
| sub **and** superscript | 16.841 pt |
| radical | 14.400 pt |
| fraction | 28.462 pt |
| fraction nested one deep | 43.538 pt |

A structural walk of `m:sSub` / `m:sSup` / `m:sSubSup` / `m:f` / `m:rad` / `m:nary` / `m:d` reproducing
those six numbers is the smallest thing that makes the rule right in kind. **Do not fit a scalar** —
`max(text height, 15.134)` would fix this document and get a fraction wrong by 13 pt.

### 7b. What must *not* change

`case "br"`'s deferral of `w:type="page"` and the resulting empty paragraph. Fixture A shows both engines
already agree on it exactly, including the blank filler page. Touching it to chase page 7 would break a
behaviour that is currently correct.

### 7c. Expected effect

`[bin]` Page 6 gains 8.25 pt, which moves our slack threshold from 16.75 pt to the reference's 8.50 pt,
below the document's 10.008 pt → **the empty paragraph moves to page 7, page 7 goes blank, and pages 7–13
shift to 8–14.** That is +1 page; the second (§6) needs its own fix. Expect movement across the whole
`ceiling-001` family and any OMML-bearing document — the corpus note in `dotnet/TODO.raster-ceiling.md`
already records this document as 14 pages against 15 and says the cause "has not been established". It is
established here, and it is **not** the extractable-text ceiling described alongside it: it is a height.

Note also that `dotnet/probes/words-extra-01/results.md` concludes the reference "draws nothing" for OMML
because `libreoffice-math` is not installed. **That is true of `/usr/lib/libreoffice` (24.2.7.2) and false
of `/opt/libreoffice26.2`** `[bin]` — 26.2.4.2 renders the formulas with a real text layer
(`pdftotext` on the reference gives `V H = 130 kts`, in Liberation Serif / Liberation Serif Italic). Any
conclusion about OMML banked against the 24.2.7.2 install should be re-measured.

---

## 8. Files in this probe

```
build-fixtures.py     fixtures A and B (parts copied verbatim from the real document)
build-slack.py        the slack sweep: the real document, one attribute changed
build-mathsizes.py    one document per OMML shape
fixtures/  fixref/  fixours/       fixtures A+B and both renderings
slack/     slackref2/ slackours2/  the 0.25 pt sweep and both renderings
mathsizes/ mathsizes/{fodt,ref,ours}
ref/ ours/ fodt/ unz/              the real document: both renderings, the resolved view, the unzipped parts
```

---

## 9. Answers to the coordinator's four asks

Raised mid-round from `dotnet/probes/omml-r163/`. Verified here independently; that directory was read,
not modified.

### (a) Is reference page 7 genuinely blank, or a page holding the tail of an overflowed object?

**Genuinely blank.** `[bin]` Rasterised at 150 dpi and scanned row by row, reference page 7 has **137 rows
carrying ink and exactly 0 of them inside the body band** (72 → 769.9 pt). Every dark row is header
(9.1 → <72 pt) or footer (>769.9 → 821.3 pt). Nothing of any object reaches the body.

The reference's own resolved view says the same `[bin]`: the body of that page is the single
`<text:p text:style-name="Standard"/>` of §3 — one empty paragraph, no frame, no object.

And the mechanism is a **fit boundary, not an overflow**: the 0.25 pt slack sweep of §4 flips the page
in/out of existence across a **0.25 pt** step (8.25 → 8.50 pt on the reference). An overflowed object does
not switch on a quarter point; a "does the next line fit" test does.

So this is not a parity filler page and not an overflow tail. It is the empty remains of the break-only
paragraph, pushed onto a page of its own.

### (b) The earliest page at which the two renderings diverge in content

`[bin]` Normalised page text, whitespace stripped:

| page | verdict |
|---|---|
| 1 | **identical**, 347 ch both |
| 2 | **identical**, 1676 ch both |
| 3 | **identical**, 3053 ch both |
| 4 | same content; 1592 ch both, differing only in how the formula text itself is spelled |
| 5 | same content; 2196 / 2202 ch |
| 6 | same content; 1940 / 1932 ch |
| 7 | **first divergence** — ref blank, ours `Therefore aeroplane lift coefficient …` |

**The earliest page at which the two sides carry different content is page 7.** Pages 1–6 hold the same
content on both sides; only the within-page geometry differs.

**Display formulas do appear before that point** — pages 4, 5 and 6 all carry OMML, and page 6 carries the
`m:oMathPara` display formula `V_H = 130 kts`. The within-page geometry on those pages already differs
`[bin]`: last body line ref − ours is +36.5 pt on p1 (a title page that ends far short of full, so it
cannot cascade), −14.0 on p3, +54.9 on p4, −3.1 on p5, **+8.24 on p6**. None of pages 1–5 cascades, because
each ends well short of its page bottom and its content boundary is unchanged. Page 6 is the first page
whose content reaches the bottom, so it is the first page where an accumulated height difference can decide
a page break — and it does.

### (c) Section parity — not abandoned, positively excluded

`[bin]`/`[src]` The document has **one** `w:sectPr`, the final body one, and it carries **no `w:type`**;
the reference's resolved view has one master page, one page layout, zero sections and no master-page-name
on any paragraph style (§2). There is no parity rule anywhere in this document for an upstream overflow to
compound with. The two causes cannot interact here because only one of them exists.

Our parity implementation was checked anyway and is present: `DocxPageGeometry.cs`:77-78 maps
`"evenPage"`/`"oddPage"` to `SectionBreak.EvenPage`/`OddPage`, and `Paginator.cs`:1303 inserts the filler
page citing `SwFrame::InsertPage` / `GetEmptyPageFormat()` `[src]`. It is simply never reached on this
document.

### (d) The conclusion, stated plainly

**This blank page is downstream of unimplemented OMML, and there is no separate pagination defect.**

Our break-only-paragraph handling and our page-fitting rule are both **correct and byte-for-byte in
agreement with 26.2.4.2** — fixture A flips at the same paragraph count on both sides (26 → 27) and
produces the same blank filler page, and the slack sweep on the real document shows the two engines'
thresholds separated by 8.25 pt, exactly the 8.240 pt of page-6 body height we fail to reserve for
formulas. Give page 6 its missing 8.25 pt and page 7 goes blank with no other change.

**The fix belongs to the formula work.** The two things this probe adds to it are:

1. **the exact law** the height fix has to satisfy — the reference's math-paragraph height *is* the imported
   object's `svg:height`, constant offset 37.45 ± 0.02 pt over ten shapes, and **`w:sz` is ignored** (§4,
   law 2); a per-shape table of the six anchor heights is in §7a;
2. **the control** that says the pagination machinery around it is sound and must not be touched
   (fixture A, §5).

The second lost page (§6) is a table keep-together difference and is **not** covered by that fix.

---

## 10. Build provenance — every `[bin]` on OUR side is a **Sep-17 build, not HEAD**

Raised by the coordinator mid-round. Verified here.

| | |
|---|---|
| binary used | `dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/` — `XRay.Office.Cli` and `XRay.Office.WordProcessing.dll` both **2026-09-17 06:33** |
| HEAD | `bccfeb64b`, **2026-09-20 22:06** |
| build base (last commit at or before the binary) | `dd9c2baf7`, 2026-09-17 05:11 |
| `WordParity` in the shipped assembly | **absent** — `strings XRay.Office.WordProcessing.dll \| grep -c WordParity` → **0** `[bin]` |

`dd9c2baf7..HEAD` touches **32 files / +2048 −187 lines** in `XRay.Office.WordProcessing` + `XRay.Office.Text`.

**So: every number in §1, §4, §5, §6 and §9(b) that describes *our* output is a Sep-17 measurement.**
Reference-side numbers are unaffected.

### 10a. What does NOT need re-measuring

* **Everything on the reference side** — the 16-page count, the blank-page-7 ink scan, the fodt resolved
  view, the one-`sectPr`/one-master-page facts, the `svg:height` table, the constant 37.45 pt offset, the
  reference's 8.50 pt slack threshold, fixture A's reference column. None of it touches our build.
* **The document's XML** (§2, §3).
* **The root cause as a source claim.** `git grep -c 'oMath' bccfeb64b -- dotnet/src` → **zero matches at
  HEAD** `[src]`. OMML is still unimplemented in the current tree. `git status --porcelain -- dotnet/src`
  is empty, so the source I read *is* HEAD, not a dirty tree.
* **The patch site.** `git diff dd9c2baf7..HEAD -- Ooxml/DocxLayoutSource.cs` adds and removes **no `case`
  and no `switch`** in the paragraph walk, and `Layout/PageContent.cs`'s `InlineObjects` and
  `ShapeLineTwip` are untouched. **§7a applies unchanged to HEAD.**
* **The two `WordParity` commits are inert by default and need no re-measure.** `WordParity` is *not* page
  parity — it is "which application this reader agrees with", read from `XRAY_OFFICE_LIBREOFFICE_QUIRKS`,
  and `ReproduceLibreOffice` is **false unless that variable is set** `[src]`. So `bccfeb64b` (REF-field
  expansion, `DocxReferenceFields.cs`:64) and `e35a69b16`/`acfa0e973` (TOC template styles,
  `DocxTocStyles.cs`:73) are switched **off** at HEAD and behave as the Sep-17 binary did. This document
  holds 32 `REF`, 19 `PAGEREF` and 1 `TOC` field `[src]`, so it would otherwise have been exposed to all
  three — with the switch off, net zero.

### 10b. What DOES need re-measuring, in priority order

1. **Our page count and the page-6 geometry.** `TableLayouter.cs` (+222) and `PageTable.cs` (+29) changed
   under `ffbdb45ac` (a cell a vertical merge covers is charged to the row's height), `f2207fde9` (the row
   below a table boundary pays the whole band), `a3159ce0d` (a split row ruled once at the cut) and
   `3a2ec4171` (a horizontal border hangs below the boundary). **The top half of page 6 is a large merged
   table.** Any of these can move the page-6 body by points, and points are exactly the currency of the
   fit test. A HEAD build could move the 8.240 pt deficit in either direction — conceivably far enough to
   produce the blank page on its own.
2. **`b5d46342a` — "A break-only line takes the break run's face, not the neighbour's" (O91).** This lands
   **directly on the construct at issue**: the empty break-only paragraph whose line height the fit test
   measures. It is the single commit most likely to move our 16.75 pt threshold.
3. **Line metrics** — `1ea79b834` (an escaped size truncated to a twip, not rounded) and `72db85d5d` (a
   character width stated by a style reaches the layout and was dropped three times). Both can change
   line heights and wrapping on page 6.
4. **`8af1cbb21`** (list counters keyed on the abstract definition), on by default, 19 `w:numId` in this
   document. Very likely closes the `5.` vs `8.` heading-number divergence of §6; label-width effect on
   pagination is small but not zero.

### 10c. What to re-run once a HEAD build exists

All scripted and idempotent, under this directory:

```bash
P=dotnet/probes/blankpage-r163
CLI=<HEAD build>/XRay.Office.Cli

# 1. page count + the offset table (§1)
"$CLI" render --format pdf --outdir $P/ours "<the document>"

# 2. the page-6 deficit (§4) — expect 8.240 pt to move
#    compare last body-line yMax on page 6 against the reference's 748.199

# 3. the slack sweep (§4) — ours should read 16.75 pt; the reference's 8.50 pt is fixed
python3 $P/build-slack.py
for f in $P/slack/*.docx; do "$CLI" render --format pdf --outdir $P/slackours2 "$f"; done

# 4. fixture A (§5) — our flip point must stay at 26 -> 27 to keep the control valid
# 5. fixture B (§5) — the 3.05 pt per-formula deficit
# 6. the "our pitch" column of the mathsizes table (§4)
python3 $P/build-fixtures.py && python3 $P/build-mathsizes.py
```

### 10d. Does the conclusion survive?

**Yes, and it does not rest on the stale numbers.** The chain is:

* the reference emits a blank page 7 — reference-side `[bin]`, unaffected;
* it does so because the empty break-only paragraph does not fit at the foot of page 6 — reference-side
  `[bin]` (fodt model + the reference's own 8.50 pt fit boundary), unaffected;
* the document carries no section break of any kind, so parity cannot be the cause — `[src]`/reference
  `[bin]`, unaffected;
* we lay out **no** OMML — `[src]` **at HEAD**, re-verified above;
* the reference reserves `svg:height` per formula and we reserve a text line — reference `[bin]` for the
  first half, `[src]` for the second.

What a HEAD build can change is **the size of the gap and therefore whether page 6 alone still decides it**
— not whether OMML is implemented, and not whether a parity rule exists. If a HEAD build already emits the
blank page, the correct reading is that something in the table work of `dd9c2baf7..HEAD` supplied the
missing height by accident; the OMML defect would still be real and still be the thing to fix, and §7a
would be unchanged. **That is the one outcome that would revise §9(d), and it is worth checking first.**
