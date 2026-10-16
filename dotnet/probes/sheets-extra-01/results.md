# sheets/extra-001 — `_x000D_` drawn as seven glyphs

Branch `wt-sh-extra`. Reference binary LibreOffice **26.2.4.2**, `check-env.sh` green,
`fc-match "DejaVu Sans"` → DejaVu Sans. Banked references at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/` reused throughout; `SOURCE_DATE_EPOCH=1600000000`
exported for every byte-for-byte comparison.

The prediction written before any of this was measured is in `prediction.md`, scored at the end.

## The verdict that started it

```
sheets/extra-001/xlsx/FY2018_Q4_UAS_Sightings.xlsx   304/302   57225/55825   pages,words
```

4872 `_x000D_` in its shared string table. We drew all seven characters; the reference draws
none of them.

## What LibreOffice actually does — measured, not read

Authored workbooks, one string per row, converted with `soffice --convert-to pdf`, and the
answer taken from the PDF's **text-showing operators**, never from a raster. Nine probe
workbooks in all.

### The escape

| input in a shared string | 26.2.4.2 draws |
|---|---|
| `ALPHA_x000D_BRAVO` | `ALPHABRAVO` — one `Tj`, one baseline. CR **dropped**, not broken |
| `CHARLIE_x000A_DELTA` | two baselines. LF is a break |
| `ECHO_x005F_x000D_FOXTROT` | `ECHO_x000D_FOXTROT` — `_x005F_` un-escapes |
| `kk_x0020_ll` / `qq_x20AC_rr` | `kk ll` / `qq€rr` — non-controls decode and stay |
| `aa_x000d_bb` | `aabb` — **lower-case hex is accepted** |
| `mm_x00D_nn`, `oo_xZZZZ_pp` | literal. A malformed escape is not an escape |
| `aa_x007F_bb` | U+007F is **drawn**, not dropped |
| the same four strings in `w:t` (docx) and `a:t` (pptx) | **all literal.** No decoding at all |

### The context rule, which is the part that is easy to get wrong

The single-line answers look like a general rule and are not. **Whether the decoded string
contains a line feed changes what the other controls mean:**

| character | no LF in the string | at least one LF |
|---|---|---|
| U+000D | dropped — no glyph, no advance, no break | **a line break** |
| U+0009 | dropped | **kept; advances to a tab stop** |
| other U+0000–U+001F | dropped | dropped |

and in the line-feed case the breaks are ordinary line-ending normalisation, measured pair by
pair: `CR LF` is one break, `LF CR` is one break, `CR CR` is two, `LF LF` is two.

The rule is on the **character**, not on how it was spelled: a literal tab and `_x0009_`
behave identically in both contexts. That matters because the corpus reaches this code through
three different spellings of the same character — the `_x000D_` escape, a `&#x0D;` numeric
reference (which XML, unlike a literal CR, does *not* line-ending-normalise), and a literal tab.

## The seat

`dotnet/src/XRay.Office.Spreadsheets/Ooxml/XlsxCellText.cs`, new, called from the three places
that read a SpreadsheetML `CT_Rst`:

- `XlsxSharedStrings.ReadRichString` — shared strings, inline strings, comment text;
- `XlsxRichRuns.Read` — **in lockstep**, because it measures run offsets into the very string
  `ReadRichString` builds. Decoding one side only leaves every run after the first escape
  pointing seven characters too far right, and the fix comes back as mis-formatted text;
- `XlsxNoteCaptions.CaptionText` — the same `CT_Rst`, reached by a different path.

`XlsxDrawings` reads DrawingML `a:t` and is deliberately **not** called: `a:t` is `ST_String`.

**The other readers do not share the seat, and the class is deliberately not in
`XRay.Office.Ooxml`.** `w:t` and `a:t` are `ST_String`; only SpreadsheetML's `CT_Rst/t` is
`ST_Xstring`. Hoisting this decoder into the shared OOXML library is exactly the change that
would start eating `_x000D_` out of Writer and Impress text, where it is real content — and out
of the 78 corpus documents whose only `_xHHHH_` is `_x0000_` inside a VML `o:spid`.

## Measured reach — all three tracks, 534 documents, rendered twice and diffed

`reach.sh` in this directory: every document rendered with the pre-fix binary and the post-fix
binary and the two PDFs compared byte for byte. **13 documents change; every one is a
spreadsheet. 0 in `words`, 0 in `slides`, 521 byte-identical.**

| document | why it changes | pages ours/ref | words ours/ref |
|---|---|---|---|
| `extra-001/…/FY2018_Q4_UAS_Sightings.xlsx` | 4872 `_x000D_` | 302/302 | 55795/55825 |
| `done-013/…/afn-…-fy25-jan25-mar25.xlsx` | 160 `_x000D_` | 270/270 | 72796/72843 |
| `done-016/…/STC_WebList.xlsx` | `&#x0D;` beside LF | 4372/4372 | 1286033/1284926 |
| `done-016/…/TK-Syllabus-Comparison-Document-v2.xlsx` | 6 `_x001E_` | 1235/1235 | 255741/255761 |
| `done-010/xls/Special-Procedures_2025-07-10.xls` | `_x000D_` (an xlsx mislabelled `.xls`) | 22/22 | 27923/27926 |
| `done-010/…/AFCforPtF-Digital-Certificate-…xlsx` | `&#x0A;&#x0D;` | 111/111 | 29739/29739 |
| `done-012/…/MinCh-Digital-Certificate-…xlsx` | `&#x0A;&#x0D;` | 179/179 | 112061/111969 |
| `done-014/…/MajCh-Digital-Certificate-…xlsx` | `&#x0A;&#x0D;` | 328/328 | 66897/66901 |
| `done-011/…/Application_for_authorisation…xlsx` | 1 `_x0002_` | 48/48 | 6223/6223 |
| `done-004/…/List-of-Members-August-2017.xlsx` | literal tab, single-line | 10/10 | 1674/1674 |
| `done-006/…/AFS-400_Contacts.xlsx` | literal tab, single-line | 48/48 | 6247/6246 |
| `done-007/…/hdss-bulletin-index-2019-2022.xlsx` | literal tab, single-line | 24/24 | 3611/3626 |
| `done-014/…/…ST Capability List Rev.16 - Web.xlsx` | literal tab, single-line | 217/217 | 88846/88842 |

All 13 pass the gate. **12 of the 13 pass it before the fix as well** — this is overwhelmingly a
fidelity change the scoreboard cannot see.

**Rendering understates the reach.** `done-015/…MAdB-Light-Prop-14-28112013.xlsx` holds one
`_x000B_`, in a shared string a cell does reference, and its PDF is byte-identical either way —
but its *extracted* text is not: two rows go from `0195/739_x000B_Propeller, Schalldämpfer…` to
`0195/739Propeller, Schalldämpfer…`. Extraction is the common path and pays for none of the
rendering, so a reach figure taken from renderings alone is a lower bound.

### The three documents the brief could not have predicted

`AFCforPtF`, `MinCh` and `MajCh` contain **no `_xHHHH_` at all**. They carry
`10073952&#x0A;&#x0D;(P-EASA.PTF.A.09.00931)` — a numeric character reference. XML normalises
*literal* line endings and leaves character references alone, so a real U+000D reaches the
reader. Confirmed in the operators, not in a raster:

```
BEFORE     p63 (54.82, 614.81)  "(P-EASA.PTF.A.09.00931)"
AFTER      p63 (54.82, 623.74)  "(P-EASA.PTF.A.09.00931)"
REFERENCE  p63 (54.82, 623.79)  "(P-EASA.PTF.A.09.00931)"
```

8.98 pt — one whole line — out of place before, 0.05 pt after, on documents whose page count
and word count never moved by one. Nothing in the gate could ever have found this.

## What the fix does to `sheets/extra-001`

```
before   304/302   57225/55825   pages,words
after    302/302   55795/55825   match
```

Pages land exactly on 302 as predicted. Per-page text agreement with the reference goes from
**4 of 302 pages to 200 of 302**. On page 245, where the before render spelled the escape 30
times: 54 text operators before, **37 after, and the reference draws 37**.

## Regression

`sheets/done-*`, 156 documents, the full track:

| | MATCH | MISMATCH |
|---|---|---|
| before | 156 | 0 |
| after | 156 | 0 |

The 156-document baseline was itself taken with a copy of the pre-fix binary (`cp -a`, never
`git stash` — the stash stack is repository-global and this clone has sixteen worktrees).

One process note that cost a re-run: piping `batch-check.sh` into `tail` kills a worker with
SIGPIPE part-way and the script still exits looking plausible. It wrote **155 of 156** rows and
the missing document was found only by diffing the row paths against the file list. Redirect to
a file; do not pipe.

## Tests

Every project run individually, discovered count checked against the run:

| project | result |
|---|---|
| XRay.Office.Containers.Tests | 109 passed / 109 |
| XRay.Office.Core.Tests | 332 passed / 332 |
| **XRay.Office.Fidelity.Tests** | **30 failed, 520 passed, 0 skipped, 550 total** |
| XRay.Office.Markup.Tests | 259 passed / 259 |
| XRay.Office.OpenDocument.Tests | 125 passed / 125 |
| XRay.Office.Presentations.Tests | 679 passed / 679 |
| XRay.Office.Rendering.Tests | 150 passed, 1 skipped / 151 |
| XRay.Office.Spreadsheets.Tests | 809 passed / 809 (of which 39 new) |
| XRay.Office.Text.Tests | 349 passed / 349 |
| XRay.Office.Vector.Tests | 295 passed / 295 |
| XRay.Office.WordProcessing.Tests | 827 passed / 827 |

4485 tests. Fidelity is exactly the pre-change baseline — 30 of 550, 0 skipped — so nothing
moved in a project this round had no reason to touch.

`XlsxCellTextTests` holds 39 tests. Against the unfixed tree — the decoder present so the suite
compiles, the three call sites reverted with `git checkout` — **4 fail**: the two that assert
the flattened text and the run offsets, the end-to-end one through `SpreadsheetReader`, and the
one that pins *which files may call the decoder*. The other 35 are the measurement table for a
class that does not exist in that tree, so they cannot fail there; they exist to keep the
LibreOffice behaviour written down beside the code that imitates it.

## A real result the word column hides

`afn-…-fy25-jan25-mar25.xlsx` moves from 72830 to 72796 against the reference's 72843 — 13
short before, 47 short after, and it *looks* like a small regression. It is the opposite.
Token-for-token against the reference:

| | tokens missing | tokens surplus | of which spelled `_x000D_` |
|---|---:|---:|---:|
| before | 384 | 371 | 153 |
| after | 266 | 219 | 0 |

Total disagreement falls from 755 to 485. The 153 spurious escape tokens had been **cancelling**
a pre-existing shortfall, and removing them exposed it. The residual is word-splitting
(`day`/`days`, `corresponde`, stray single letters) — a different, older defect. This is the
same shape as the `###` and `$`/`-` cancellations recorded in `batch-check.sh`'s own notes, and
it is the reason a net word count is a poor instrument.

## Blind visual review

Two fresh subagents, one page each, given only an image path — no numbers, no document name,
forbidden to read any file or run any command. Pairs built from our render and the **banked**
reference at 120 dpi (the compositor's first attempt at 150 dpi reported `shown at 80% of
composed`, so the dpi was reduced until it reported 100% and no pixels were discarded).
`compose.py` raised no size-mismatch warning on either pair, so the two halves agree in page
geometry.

- Page 245: *"I could not identify a single difference I would defend."*
- Page 182: *"No substantive rendering difference … a reader shown these two halves would say
  they are the same page produced twice."*

Both independently flagged the same limitation — the right-hand end of long lines is clipped on
*both* halves, so a divergence out there would be invisible to them, and the text layer should
be diffed instead. It was: 200 of 302 pages byte-identical in text, and the page-245 operator
counts above.

## Prediction, scored

| | claim | outcome |
|---|---|---|
| P1 | seat is `ReadRichString` + `XlsxRichRuns` in lockstep, nothing in words/slides | **right**, and `XlsxNoteCaptions` was a third call site I had not listed |
| P2 | `extra-001` reaches exactly 302/302 and passes | **right**, exactly |
| P3 | reach is 6 documents, plus 8 tab documents moving 0 words, plus 0 in words/slides | **half right.** 0 in words and slides is right and is the important half. The count was wrong in both directions: the answer is 13, I missed the three `&#x0D;` documents entirely, four of my eight tab documents did not move, and `MAdB-Light-Prop` moved only in extraction. The "0 words on the tab documents" part held — every one of them is 0/0 |
| P4 | `done-*` holds its MATCH count | **right**, 156 → 156 |
| P5 | fidelity stays 30/520/0/550 | **right** |

**What I got wrong, and it is the interesting part.** P3 assumed the defect was spelled
`_xHHHH_`. Two of the three spellings that actually reach this code are not: a `&#x0D;`
character reference and a literal tab. I found them only because the reach sweep rendered all
534 documents rather than the six the brief named, and then because a determinism control
(same binary twice — byte-identical) refused to let me write the surprises off as noise.

I also shipped a wrong version first. Stripping *all* C0 controls unconditionally is what the
single-line probes supported, and it cost `afn` 76 words by gluing `-` to `Division`: in a
multi-line string the tab is not dropped, it advances. Two more probe workbooks — the same
controls in a cell that already holds a newline — produced the context rule. **The first set of
probes was not wrong, it was incomplete, and it read as complete.**

## Contradicting the brief

1. **"LibreOffice decodes it to a line break" — no.** With no line feed in the string it decodes
   to a carriage return and draws *nothing at all*. The brief named this as one of three
   possibilities to check; the answer is the third one, and it is context-dependent besides.
2. **"Six documents carry the literal escape … five of those six currently PASS."** Four of the
   six are `oddHeader`/`oddFooter` only, and **the reference draws the seven glyphs there too**.
   Our page-1 footer for `Published_Issuances_2024.xlsx` is byte-identical to the banked
   reference's. Those four are already correct and are untouched.
3. **"A blind reviewer independently read `_x000D_ Classification: GENERAL` in
   `Published_Issuances_2024`'s footer."** They were reading a *correct* rendering. LibreOffice
   prints exactly that string. The reading was accurate; the inference from it was not.
4. The real reach is 15 documents, not 6, and the overlap with the brief's list is 2.

## Left open

- **Tab stops inside a multi-line cell.** 104 tabs across three corpus workbooks now survive
  into layout rather than being drawn as whatever we drew before. All three documents match, so
  nothing is visibly wrong — but *where* Calc puts its tab stops in an `EditEngine` cell has not
  been measured, only that the tab is not discarded.
- **U+0085 and U+2028/U+2029.** Measured — U+0085 becomes a space, U+2028 and U+2029 are dropped
  — but no corpus document contains one, so none of it is implemented and none of it is tested
  against a document.
- **`xl/tables/*.xml` `name`/`uniqueName`** are `ST_Xstring` and hold `_x005f_x0020_` in two
  corpus workbooks. They are not drawn, so they are left alone.
