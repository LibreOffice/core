# words-r55 — prediction, committed before the change

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
worktree `wt-words-r50` on branch `wt-words-r55`, base `1c7249ff8e9`; `SOURCE_DATE_EPOCH=1700000000`,
`TZ=UTC`.

**Baseline reproduced before anything was touched.** `batch-check.sh … 'words/*' … 8` →
`TOTAL 355 MATCH 333 MISMATCH 22`; scored against `MANIFEST.tsv`'s own 337-path list,
**317 of 337, with zero disagreements against the manifest's status column, document for document.**

## What the probe established

`family-inheritance.py`, 28 authored packages, **one paragraph and one run each** so the PDF's font
list has exactly one entry that can move, three controls agreeing (`Arial` → Liberation Sans,
`Calibri` → Carlito, `Liberation Serif` → itself) and four restatements of round 54's rule agreeing:

**The DOCX family class is an inherited property.** It is set only where `w:rFonts/@w:ascii` names a
font `word/fontTable.xml` files under `roman` or `swiss`. It is *not* cleared by a `w:ascii` naming
a font the table files `auto`, `modern`, a pitch only, or does not mention; it is *not* set by
`w:asciiTheme`, which supplies the name and never the class; and it survives docDefaults → style
chain (any depth) → direct run formatting. Nothing anywhere naming one leaves it roman.

| case | 26.2.4.2 draws |
|---|---|
| `docDefaults` `Arial`(swiss), run names an `auto` family | **DejaVu Sans** |
| `Normal` `Arial`(swiss), a style `basedOn` it names an `auto` family | **DejaVu Sans** |
| the same through two style levels | **DejaVu Sans** |
| `Normal` `Arial`(swiss), *direct run formatting* names an `auto` family | **DejaVu Sans** |
| the consumer's own entry says `roman` over a swiss ancestor | DejaVu Serif |
| the consumer names its font through `w:asciiTheme`, swiss ancestor | **DejaVu Sans** |
| …the theme font is itself declared `swiss` in the table, roman ancestor | DejaVu **Serif** |
| a font absent from the table entirely, swiss ancestor | **DejaVu Sans** |
| `w:pitch="fixed"` and no class, swiss ancestor | **DejaVu Sans** |

`24-25_FAA_Holdover_Tables.docx` fits exactly: its `Normal` names `Arial`, which its table files
`swiss`; `Heading2`, `Heading3` and `Caption` are `basedOn Normal` and name `Arial Bold`, which it
files `auto`. Round 54's refutation of "inheritance through the style" was read off the **whole
document's** embedded font list, and that document draws DejaVu Sans for four other reasons —
`Century Gothic`, `Tahoma`, `Charlotte Sans Book` and `CWFZGM+Myriad-BoldItalic` are all declared
`swiss` in the same table — so the observable was over-determined.

## The change

`WordFallbackClass.ForDeclared`'s second argument changes meaning from *the class of this name* to
*the class inherited at this run*; the `.doc` and `.rtf` arms keep passing the per-name class,
because the WW8 `FFN` carries a family per font and there is no inheritance there to model.
`WordTextStyle` gains the resolved class, `WordParagraphFormats.ResolveRun` computes it, and
`DocxLayoutSource.Face` reads it instead of asking the table about the run's own name. The face
cache key gains the class, because two runs naming one family under different ancestors now resolve
to two different faces.

**No shared layer.** Nothing under `XRay.Office.Text`, `Core`, `Containers`, `Vector`, `Rendering`,
`Markup` or `Ooxml` changes, so slides and sheets cannot be reached and no cross-track sweep is owed.

## The census, and what it cannot see

`class-inheritance-census.py` walks every `.docx` in the words corpus, rebuilds the layer stack
(direct `w:rPr` → character style chain → paragraph style chain → `docDefaults`), and counts runs
whose resolved family takes a different class under the two rules, ignoring families that never
reach a fallback. **16 distinct documents, 4765 runs:**

| document | family, old → new | runs |
|---|---|---:|
| `FAA 2025-26 Holdover Tables` | `Arial Bold` roman → swiss | 2164 |
| `24-25_FAA_Holdover_Tables` | `Arial Bold` roman → swiss | 2007 |
| `Company-profile-2022-EN` | `Calibri Light` swiss → roman | 220 |
| `ESPN-R - MCF - Manual - Ed1.0` | `Calibri Light` swiss → roman | 105 |
| `ESPN-R - MCF - RA - Ed1` | `Calibri Light` swiss → roman | 64 |
| `AWR OPS-AOC 044 …` | `MS Gothic` roman → swiss | 42 |
| `technical report template` | `Calibri Light` swiss → roman | 39 |
| `How-to-Write-an-Architecture-Document…` | `Aptos`, `Aptos Display` swiss → roman | 34 |
| `OM template for non-complex NCC operators` | `ArialMT` roman → swiss | 30 |
| `form_1123_application_form_rvsm_spa` | `MS Mincho` roman → swiss | 15 |
| `f2_registro_de_aprovacao_com_pbcs_EN` | `Gill Sans MT` swiss → roman | 13 |
| `Writing a technical report (SCE subject guide)` | `Calibri Light` swiss → roman | 11 |
| `Lessons-Learned-Bulletin-Dorset…` | `Aptos` swiss → roman | 8 |
| `AFS-050-004-F2_0i` | `Helvetica-Narrow` roman → swiss | 6 |
| `SDL_FSDO_Part91_LOA_Checklist` | `Calibri Light` swiss → roman | 6 |
| `AW-104D-RVSM-Aircraft-Approval-Checklist.pdf` | `MS Gothic` roman → swiss | 1 |

**What the census cannot see**, written down before the sweep rather than after it:

* **`w:tblStylePr` run properties and a numbering level's own `w:rPr`** are property layers and this
  models neither, so 16 is a **floor**, not a ceiling. Textbox and drawing text is likewise not
  walked.
* **`.doc` and `.rtf`.** Two of the six documents in the corpus's current wrong-direction list are
  `.doc` (`congregationalhistories_ky_2023`, `手机免提系统TSB`) and this rule cannot reach them.
  Whether a genuinely undeclared WW8 `FFN` family answers Serif or reaches fontconfig's generic is
  **still open** — round 54's `.doc` probe was confounded by a round trip and nothing has replaced it.
* **Whether the change is an improvement per document.** The census counts disagreements between two
  rules; the authored probe says which rule 26.2.4.2 follows; only the sweep says what it costs.
* **The four East-Asian and narrow cases are a different defect wearing this one's clothes.**
  `MS Gothic`, `MS Mincho` and `Helvetica-Narrow` are reached only because our `Family()` falls
  through the `ascii` slot to `hAnsi`/`cs`/`eastAsia` when no layer states an ascii name.
  LibreOffice's `DomainMapper` treats `w:hAnsi` as **unsupported** and never takes a western family
  from `w:cs` or `w:eastAsia`, so it is drawing a *different family* there, not the same family in a
  different class. Applying the true class rule to a name our slot fallback invented may make those
  four worse. They are `AWR OPS-AOC 044`, `form_1123_application_form_rvsm_spa`,
  `AFS-050-004-F2_0i` and `AW-104D-RVSM-Aircraft-Approval-Checklist.pdf`, and **three of the four
  pass the gate today.**

## The numbers predicted

| | predicted |
|---|---|
| renderings whose bytes change | **16–24** (16 named, plus the layers the census cannot see) |
| documents whose font list stops disagreeing with the reference | **9–12** of the 14 named below |
| font-list disagreements, 66 at baseline | **54–60** |
| `ours=DejaVuSans, ref=DejaVuSerif` (8 now) | **1–2** — `FO.FCTOA_.000129` is *not* explained by this rule and is expected to stay |
| `ours=DejaVuSerif, ref=DejaVuSans` (6 now) | **3** — the two `.doc` and `template---tpr-…`, none of which this rule reaches |
| **verdict movement** | **+1**, `24-25_FAA_Holdover_Tables` 165/155 → 155/155 |
| downside risk | **−1 to −3**, concentrated in the four slot-fallback documents above, three of which pass today |
| cross-track verdicts | **0 slides, 0 sheets** — no file they compile against changes |

**+1 is the whole point of the round and it is also the number most likely to be wrong**, because
the FAA document fails on *page count*: getting the face right is necessary for 155 pages and this
round cannot show it is sufficient. A result of "the font list agrees and the page count does not"
is a live outcome and would be reported as a partial success, not smoothed over.

A gain of more than +1, or a change on any document outside the 16, means the census under-reached —
most likely through the table-style and numbering layers it does not model.
