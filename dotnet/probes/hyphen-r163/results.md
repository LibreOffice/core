# r163 — "a sentence at the end of page 126 falls to page 127 in LibreOffice"

**Document**: `/home/user/sample-files/words/pagination-001/docx/FAA 2025-26 Holdover Tables.docx`
**Reference**: `/opt/libreoffice26.2/program/soffice`, 26.2.4.2 / `0229ac93fcf0d7cbc6376066c6f35021cef002dc`
**Ours**: `dotnet/tools/XRay.Office.Cli/bin/Release/net10.0/linux-x64/XRay.Office.Cli` (prebuilt, **Sep 17**; see §7)
**Verdict**: **hyphenation is not involved.** The cause is a `REF` field whose cached result is
shorter than the bookmark it names.

> `/usr/bin/soffice` is 24.2.7.2 and `lo-convert.sh` takes `soffice` from `PATH`, so the first
> reference render here was the wrong version (154 pages). Everything below was re-run with
> `PATH=/opt/libreoffice26.2/program:$PATH`. **Prepend that path before calling the script.**

---

## 1. The observation, confirmed `[bin]`

Both sides print **167 pages**.

The moved sentence is the bulleted line under `CAUTIONS` on the `TABLE ADJ-19` page (printed
folio `Page A-38`):

> The cautions that apply to the holdover times in the table above can be found on page A-37.

| | page holding `TABLE ADJ-19` | where the sentence sits |
|---|---|---|
| 26.2.4.2 | 126 | **page 127**, alone with the header and footer |
| ours | 127 | **bottom of page 127**, at y = 537.08 |

(We print it as `… on page A-26`: that is a second, independent defect — see §6.)

**The offset does not persist.** Aligning the two page sequences by text similarity `[bin]`:

```
ours 1   -> ref 1     offset  0
ours 83  -> (a spurious near-blank page of ours; see §6)
ours 85  -> ref 84    offset -1      ← we are one page ahead from here
ours 128 -> ref 128   offset  0      ← resynchronised
```

The two page counts are equal **by cancellation**: we emit one spurious page at 83 and lose one
page at 127. The document is otherwise page-for-page identical, start to end.

## 2. The earliest diverging line

Read from the PDFs' own text-showing operators (`pdf-ops.py dump`) and from `pdftotext -bbox`
word boxes; both agree, and neither shows a hyphen anywhere (§3).

Our page 127 and the reference's page 126 are **identical line for line** — same words, same
breaks, same baselines to within 0.13 pt — from the running head down to and including note 10.
Two lines then differ:

**(a) the line that moves the sentence** — note 11, **four lines above** the symptom:

```
ref  y=490.65  11 Heavy snow, … and hail (Table Adj-51 provides adjusted allowance times for Type IV EG fluids and Table
ref  y=501.00     Adj-52: Adjusted Allowance Times for SAE Type IV Propylene Glycol (PG) Fluids1,2 provides adjusted allowance times for Type IV PG fluids in ice pellets and small
ref  y=511.36     hail).

ours y=490.53  521 Heavy snow, … and hail (Table Adj-51 provides adjusted allowance times for Type IV EG fluids and Table
ours y=500.88      Adj-52 provides adjusted allowance times for Type IV PG fluids in ice pellets and small hail).
```

Three lines against two. From there down everything of ours is exactly one line (10.35 pt) higher
— note 12 at 511.23 against 521.71, `CAUTIONS` at 525.27 against 535.75 — and the 10.35 pt the
reference does not have is exactly the room the sentence needs.

**(b) an earlier, harmless one** — the list markers of notes 6–12 read `516`…`522` in ours
against `6`…`12` in the reference, and their text therefore starts at x = 93.60 instead of
x = 71.40. First seen at y ≈ 418.1, eleven lines above the symptom. It is real ink, not a text-layer
artefact: the operator dump shows a 3-glyph show at x = 57.60 where notes 1–5 have a 1-glyph one.
**It does not move anything on this page** — every note it touches keeps its line count — so it is
not the cause. See §6.

## 3. Hyphenation: not involved

- **No line on either side ends in a hyphen.** 57 lines on our page 127, 57 on the reference's
  page 126, 3 on the reference's page 127: zero line-final `-`, `U+00AD`, `U+2010` or `U+2011` in
  the ink or in the text layer `[bin]`.
- **`word/settings.xml` does not state `w:autoHyphenation`** — absent, so off. No
  `w:consecutiveHyphenLimit`, no `w:doNotHyphenateCaps`; only `w:hyphenationZone w:val="425"`,
  which is inert while auto-hyphenation is off. **Zero** `w:suppressAutoHyphens` in
  `word/document.xml` or `word/styles.xml` `[src]`.
- **The reference resolved it off.** In the 7.4 MB flat ODF there is exactly **one**
  `fo:hyphenate` attribute in the whole file, on the default paragraph style, and it is
  `fo:hyphenate="false"`; `fo:hyphenation-ladder-count="no-limit"` `[bin]`.
- **Language is fine and would not matter**: `fo:language="en"` on 327 styles, `fo:country="CA"`
  176 / `"US"` 160; `<w:themeFontLang w:val="en-US"/>`.
- **Our tree has no Writer hyphenation to get wrong.** `HyphenationPatterns` / `Hyphenators` exist
  in `XRay.Office.Core.Globalization` but the only caller is `Charts/ChartAxisLabels.cs` `[src]`.

## 4. Measurement, kerning and fonts: ruled out

Both sides use the **same substituted faces**, all embedded in the output, none embedded in the
DOCX: `LiberationSans`, `-Bold`, `-Italic`, `-BoldItalic`, `LiberationSerif`, `LiberationMono`,
`DejaVuSans`, `DejaVuSans-Bold`, `OpenSymbol` `[bin]`.

Widths of lines that are word-for-word identical and start at the same x `[bin]`:

| line | ours | ref | Δ |
|---|---:|---:|---:|
| note 2 | 559.0 | 558.7 | 0.3 pt (0.05 %) |
| note 3 | 606.1 | 605.8 | 0.3 pt (0.05 %) |
| note 4 | 410.9 | 410.7 | 0.2 pt (0.05 %) |

0.05 % over 600 pt lines is not a measurement defect and has never moved a break here.

## 5. Root cause

`[src]` `word/document.xml`, the note-11 paragraph (offset 10 822 089) holds

```xml
<w:fldChar w:fldCharType="begin"/>
<w:instrText xml:space="preserve"> REF _Ref199241802 \h </w:instrText>
<w:fldChar w:fldCharType="separate"/>
<w:t>Table Adj-</w:t><w:t>52</w:t>          <!-- the cached result -->
<w:fldChar w:fldCharType="end"/>
```

`[src]` and the bookmark it names (offset 14 628 172) spans the **whole caption**:

```xml
<w:bookmarkStart w:id="1154" w:name="_Ref199241802"/>
  Table Adj- {SEQ Table_Adj- → 52} : Adjusted Allowance Times for SAE Type IV <w:br/>
  Propylene Glycol (PG) Fluids <sup>1</sup><sup>,2</sup>
<w:bookmarkEnd w:id="1154"/>
```

`[bin]` LibreOffice recomputes the field on load — the flat ODF says so literally:

```xml
<text:bookmark-ref text:reference-format="text" text:ref-name="_Ref199241802"
  >Table Adj-52: Adjusted Allowance Times for SAE Type IV Propylene Glycol (PG) Fluids1,2</text:bookmark-ref>
```

so the reference draws ~303 pt more text than the cache does, note 11 wraps to a third line, and
the bulleted sentence no longer fits. We draw the cached `Table Adj-52`.

The sibling `REF _Ref75338163` in the same sentence is unaffected: its bookmark covers only
`Table Adj-51`, so cache and recomputation agree, and both sides draw the same thing.

**This is a known, deliberate divergence.** `dotnet/TODO.word-parity.md` §"A `REF` field is
recomputed on load" names this document, this rule and this page, and settles it in Word's favour:
Word does not update a `REF` on open or print, so the cache is what the author saw. The recomputation
is implemented in `DocxReferenceFields` behind `WordParity.ReproduceLibreOffice`
(`XRAY_OFFICE_LIBREOFFICE_QUIRKS=1`). `[src]`

## 6. Three other defects found on the way, none of them the cause

1. **List markers `516`…`522` for notes 6–12.** `numId 54` (on the paragraphs, with
   `<w:startOverride w:val="1"/>`) and `numId 118` (from the `ListNotes` style, no override) share
   `w:abstractNumId 21`, as do ~130 other instances. Writer counts them as **one** list, so `118`
   continues at 6; we counted per `w:numId` and `118` had accumulated 515 items document-wide.
   `[src]` **Already fixed at HEAD** by `8af1cbb21` — `WordNumbering.CountsIn` now keys counters on
   the abstract definition — which the Sep-17 binary predates.
2. **A spurious near-blank page at our 83**, between two `TABLE 58 (CONT'D)` pages. Cause not
   investigated; it is what makes the two page counts equal.
3. **`PAGEREF TypeIVCautionsAdj \h` is drawn from its cache**: we print `page A-26`, the reference
   prints `page A-37`, which is where the target actually lands. `[bin]` `PAGEREF` is recognised
   (`FieldInstructions` → `WritingFieldKind.PageReference`) but never evaluated `[src]`. Unlike
   `REF`, a `PAGEREF`'s cache is stale *by construction* — pagination decides it, exactly as for
   `PAGE`, which `TODO.word-parity.md` already lists as "still LibreOffice's reading". This one
   looks misclassified.

## 7. The binary is older than the tree

`[bin]` `XRay.Office.WordProcessing.dll` is dated **17 Sep**; HEAD is `bccfeb64b`, **20 Sep**. The
assembly contains `DocxLayoutSource` but **not** `DocxReferenceFields`, `DocxTocStyles`,
`WordParity` or `ReferenceFieldText`, and `XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` changes **nothing** in
its output (byte-identical text on this document and on
`tests/corpus/features/words-reference-field.docx`). So the measurements above are of the Sep-17
tree. At HEAD the default behaviour on this page is the same — the cache — by design.

## 8. Fixture

`fixture/words-reference-field-wraps-a-page.docx`, 2 021 bytes, built by `fixture/build.py`.
Carries a `word/settings.xml` (with `compatibilityMode 15`) on purpose. One bookmark spanning a
caption, one `REF` whose cached result is just `Table 2`, filler, and a trailing sentence on a
3.6 in page.

```
26.2.4.2  2 pages — note wraps to 2 lines, the sentence is alone on page 2
ours      1 page  — note is 1 line, the sentence stays on page 1
```

Sweeping the filler count: 13, 14 → 1/1; **15 → 2/1**; 16 → 2/2. The fixture sits on the
threshold, which is the same threshold the real document sits on.

## 9. Refuted / not settled

- **Refuted: hyphenation** (§3), in every form — no hyphen in the ink, off in the file, off in the
  reference's resolved view, and unimplemented for Writer text on our side.
- **Refuted: text measurement, kerning, tracking** (§4) — 0.05 % over 600 pt.
- **Refuted: font substitution** (§4) — identical faces on both sides, nothing embedded in the DOCX.
- **Refuted: the `516` markers** (§2b) — real, fixed at HEAD, and it moves nothing on this page.
- **Not settled**: the spurious page at our 83; whether HEAD still emits it.
- **Not settled**: whether `PAGEREF` should join `PAGE` on the recomputed list (§6.3).

## 10. Proposed patch (not applied)

### 10.1 The symptom the user reported — a policy flip, one line

**File** `dotnet/src/XRay.Office.WordProcessing/WordParity.cs`
**Member** `public static bool ReproduceLibreOffice`

*Current* — `XRAY_OFFICE_LIBREOFFICE_QUIRKS` must be `1`/`true`/`yes`; unset means Word, so
`DocxReferenceFields.Expansions` returns `null`, `DocxLayoutSource.ValueOf` finds no expansion
for `_Ref199241802`, and the cached `Table Adj-52` is drawn.

*Proposed* — default to the reference and let a caller opt out, e.g.

```csharp
public const string Variable = "XRAY_OFFICE_WORD_PARITY";

public static bool ReproduceLibreOffice =>
    Environment.GetEnvironmentVariable(Variable) is not ("1" or "true" or "yes");
```

**This is a decision, not a fix, and it reverses a decision `TODO.word-parity.md` already took
deliberately.** What it buys and costs, from that file's own measurements:

| | with the flip | today |
|---|---|---|
| `FAA 2025-26 Holdover Tables.docx` | 167 pages = reference; this page correct | 166 |
| `24-25_FAA_Holdover_Tables.docx` | 155 = reference | 154 |
| `SPA-11_mcar_part-11_v2.9.docx` | loses its exact character match | exact |
| `SPA-06_mcar_part-6_and_IS_v2.9.docx` | loses its exact character match | exact |
| page 7 of this document | draws `(Tables Table 55, Table 56, Table 57 and Table 58)` | `(Tables 55, 56, 57 and 58)` |

That last row is the argument against, and it is a good one: the doubled `Table` is what the file
genuinely means and *not* what any reader of the document has ever seen. **Recommendation: do not
flip the global default.** If the renderer is being scored against 26.2.4.2, set
`XRAY_OFFICE_LIBREOFFICE_QUIRKS=1` for that run; if it is producing output for a person, the present
default is right and this page is the documented cost. Either way the user's report should be
answered as "known and deliberate", not as an open defect.

### 10.2 A genuine bug in the same sentence — evaluate `PAGEREF`

**File** `dotnet/src/XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs`
**Method** `private string? ValueOf(string instruction)` (≈ line 1806), reached from `Substitute`

*Current* — `PAGEREF` matches no branch: `StyleReferenceName` and `ReferenceBookmark` both decline
it and `ConstantFieldOf` has no case for it, so `ValueOf` returns `null`, `Substitute` returns
false, and the cached result is drawn. On this page that is `page A-26` where the reference and
the document both say `A-37`.

*Proposed* — treat `PAGEREF <bookmark>` the way `PAGE` and `NUMPAGES` are already treated, not the
way `REF` is:

1. `FieldInstructions.PageFieldOf` (≈ line 106) gains a `"PAGEREF"` case returning a new
   `Layout.PageFieldKind.BookmarkPage` together with the bookmark name;
2. `Layout.PageFieldSpan` (`Layout/PageFields.cs`:54) carries that name;
3. `Paginator` (`Layout/Paginator.cs`:734) already runs a second pass for `NUMPAGES`; in that pass
   it resolves each `BookmarkPage` span from the page that holds the named bookmark's anchor,
   formatted by the section's page-number format — which is what produces `A-37` rather than `37`.

**This is not a `WordParity` rule and must not go behind the switch.** A `PAGEREF`'s cache is stale
*by construction* — pagination decides it, exactly as for `PAGE` — so it belongs in
`TODO.word-parity.md`'s "Still LibreOffice's reading" list beside `PAGE`, `NUMPAGES` and
`SECTIONPAGES`, where it is currently missing. It changes text, not pagination, so it does **not**
move the sentence.

### 10.3 Rebuild the CLI

`dotnet/tools/XRay.Office.Cli/bin/Release/.../XRay.Office.WordProcessing.dll` is three days older than
HEAD and lacks `DocxReferenceFields`, `DocxTocStyles`, `WordParity` and the shared-abstract list
counter of `8af1cbb21`. Any further measurement on this document made with it will re-find the
`516` markers, which are already fixed.
