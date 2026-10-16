# words/batch-008 round 1 — tracking must suppress the optional ligatures

Branch `wt-words-b008`. Prediction committed at `f8eacbf0167` before the fix compiled and before
any sweep; scored at the bottom of this file.

## The round in one line

`words/batch-008/docx/FAA-2017-0628-0002_attachment_1.docx` drew 28 words more than the
reference. Neither of the two "the gate is lying" shapes applied: it was a real defect in our
own text layer, it is fixed, and the document is now exact on every column.

```
before   pages 4/4   words 666/638   fonts ok   unembedded 0   verdict: words
after    pages 4/4   words 638/638   fonts ok   unembedded 0   verdict: match
```

Raw `pdftotext | wc -w` agrees as well: 671/641 before, 641/641 after.

## The two checks the brief demanded first, and what they said

**Determinism — negative.** Six `soffice 26.2.4.2` conversions of the same file, each with its own
profile: `638 638 638 638 638 638`. The banked reference is 638 too. So this is not
`fse_identification_form`'s shape; the reference is a fixed answer here. (Note the
`ref-baseline-all.tsv` row says `refwords 641`, which is the **raw** count, not the gate's
letter-or-digit one. That file's column is ambiguous; recompute from the banked PDF.)

**Whitespace-stripped character streams — identical.** 3750 non-space characters each side,
`difflib` zero diff blocks. Nothing is missing or surplus in either direction. The whole 28 is
tokenisation, and the token diff localises it to two runs of one line:

```
REF : ['PADM', '533:', 'Policy', 'Formation']   OURS: ['P','A','D','M','5','3','3',':', … ,'ti', …]
REF : ['Dr.', 'Marcia', 'Godwin']               OURS: ['D','r','.','M','a','r','c','i','a', …]
```

That is the mirror of `TODO.raster-ceiling.md`'s shapes 2 and 3 — but with *our* tokenisation
shattering rather than the reference's, which makes it ours to fix rather than a ceiling.

## Why it was not visible in the obvious places

**`pdf-ops.py` does not separate the two sides.** Both write that line as one show operator per
glyph inside a single `TJ` array — ours 45 glyphs in 45 shows, the reference 46 in 46 — with
adjustments of about `-300` on both. The geometry agrees too: our run spanned 338.64 pt against
the reference's 341.21.

**Ink does not separate them either**, and this is worth recording because ink was the wrong
instrument and looked like the right one. The document's `|ink|` is 1.16 before and 1.16 after: a
ligature is one glyph, and the defect is in the *text layer*, which no raster diff reads. The
instrument that does read it is the one-character-token count — 46 before, 12 after, and the
reference's own is **12**.

## The mechanism, measured

Byte-surgery on both PDFs — decompress the page's content stream, rewrite the `TJ` adjustments,
re-deflate, pad so that no file offset moves — sweeps poppler's intra-word gap tolerance:

| PDF | joins up to | splits from |
|---|---|---|
| the reference's | 0.350 em | **0.400 em** |
| a synthetic base-14 control | 0.395 em | **0.400 em** |
| **ours** | 0.100 em | **0.105 em** |

Same font, same size, same reader, a fourfold difference in threshold. Mutating our PDF one
property at a time:

| mutation | result |
|---|---|
| baseline | SPLIT |
| page stream reduced to that one `BT…ET` | SPLIT |
| `/Widths` integerised to match the reference's | SPLIT |
| `/StemV`, `/Descent`, `/FontBBox` set to the reference's | SPLIT |
| **`ToUnicode <15>` from `<00740069>` to `<0074>`** | **JOINED** |

So the trigger is a **multi-character `ToUnicode` entry**. We formed Carlito-Bold's `t`+`i`
ligature — confirmed in the font's own GSUB, `liga` lookup 37, `t ['i'] -> glyph02210` — which is
one glyph covering two characters, so its `ToUnicode` maps one code to two. Poppler's response to
that entry is to drop its tolerance below the 0.300 em the tracking itself puts between every
pair, and the whole line shatters.

The reference does not form it, and LibreOffice says why in one place:

```cpp
// vcl/source/outdev/text.cxx:996-998
if( maFont.IsFixKerning() || … PITCH_FIXED )
    nLayoutFlags |= SalLayoutFlags::DisableLigatures;
```

`Font::IsFixKerning()` is `mnSpacing != 0` (`vcl/source/font/font.cxx:232`), fed from
`RES_CHRATR_KERNING` (`sw/source/core/text/atrstck.cxx:619`) — which is exactly `w:spacing`. The
flag becomes `liga=0, clig=0` at `CommonSalLayout.cxx:453`. The run in question is a
`w:txbxContent` paragraph at `<w:spacing w:val="60"/>`, 3 pt of tracking per character.

**The tree is 27.2-alpha and the binary 26.2.4.2, so the tree is cited for the mechanism only.**
The behaviour is measured independently: the reference's PDF holds 46 glyphs on that line and no
ligature.

## The fix

The rule is stated once, on `ShapingOptions`, and read through a computed property on each of the
three types that carry a run's tracking beside its shaping:

| file | change |
|---|---|
| `XRay.Office.Text/Shaping/ITextShaper.cs` | `ShapingOptions.WithTracking(Length)` — the rule |
| `XRay.Office.Text/Layout/MeasuredParagraph.cs` | `FormattedRun.EffectiveShaping`; measurement uses it |
| `XRay.Office.WordProcessing/Layout/PageContent.cs` | `PageRun.EffectiveShaping`, `PageParagraph.EffectiveShaping` |
| `XRay.Office.WordProcessing/Layout/PageDrawing.cs` | drawing, tab-stretch width and tab leaders use it |
| `XRay.Office.WordProcessing/Layout/FlowLayouter.cs`, `Paginator.cs` | the uniform-paragraph layout path |
| `XRay.Office.Presentations/Layout/SlideTextLayout.cs` | slide drawing uses it |

`PageParagraph.EffectiveShaping` is not redundant: a paragraph set end to end in one tracked style
carries **no runs at all**, being uniform by every test the reader makes, and would have been the
one kind of paragraph to escape the rule.

Zero-tracking runs get the identical `ShapingOptions` value back, so the overwhelming majority of
runs reach HarfBuzz in the call they made before this existed. `rlig` is untouched, here as in
LibreOffice: tracked Arabic still joins.

## Reach, measured from what resolves

Both halves rendered with `SOURCE_DATE_EPOCH=1700000000` against the **banked** references at
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts` — reused, never re-rendered. `banked-check.sh` beside
this file runs `batch-check.sh`'s three checks, its word definition and its 2%+3 band verbatim
against them. It reproduces the briefed state exactly on its first use: words/batch-001..008 came
out **78/80** before the fix, the two failures being `1447.doc` and the target.

| track | before | after |
|---|---|---|
| words | 157/200 | **158/200** |
| slides | 144/163 | 144/163 |
| sheets | 155/171 | 155/171 |
| **corpus** | **456/534** | **457/534** |

**Verdict changes: one, and it is the target. Regressions: none, on any track.**

Renderings whose bytes changed at all — the honest measure of blast radius, since two runs with
`SOURCE_DATE_EPOCH` set are byte-identical given identical input:

| track | changed | of |
|---|---:|---:|
| words | 9 | 200 |
| slides | 9 | 163 |
| sheets | 0 | 171 |
| **total** | **18** | **534** |

So **8 words documents and 9 decks got typographically closer to the reference without any gate
column moving** — which is the standing point that the gate is blind to most real differences,
seen from the productive side for once. Five of the nine words documents shed multi-character
`ToUnicode` entries outright:

| document | multi-char `ToUnicode` before → after |
|---|---|
| `FAA-2017-0628-0002_attachment_1.docx` | 4 → 2 |
| `Form-SM-76A-…-Compliance-Statement-_-11.docx` | 18 → 6 |
| `AWR OPS-AOC 044 …RVSM… .docx` | 38 → 34 |
| `FO.FCTOA_.000129 …FSTD.docx` | 32 → 30 |
| `A1. EASA Form 2.docx` | 24 → 22 |

61 of the 200 words documents carry a non-zero run `w:spacing`. That is a **grep prior and not a
reach figure** — it is quoted only to say why the blast radius was expected to be wider than it
turned out to be. What resolves is 18 renderings and one verdict.

### Regression suite

`words/batch-008` alone: **10/10**. `words/batch-001..008` together: **79/80**, the single failure
being `words/batch-004/doc/1447.doc` at 3/4 pages — the line-height law, deliberately not chased,
and unchanged by this round.

## Tests

Eleven added. All eleven were confirmed to **fail** with the rule neutralised (`WithTracking`
returning `this` unchanged): 8 of the 10 new `XRay.Office.Text.Tests` fail, and the two that do not
are the deliberate preconditions — that untracked options are returned unchanged, and that the
face ligates `ti` at all.

| project | before | after |
|---|---:|---:|
| `XRay.Office.Containers.Tests` | 109 | 109 |
| `XRay.Office.Core.Tests` | 313 | 313 |
| `XRay.Office.Markup.Tests` | 259 | 259 |
| `XRay.Office.OpenDocument.Tests` | 125 | 125 |
| `XRay.Office.Presentations.Tests` | 646 | 646 |
| `XRay.Office.Rendering.Tests` | 150 (+1 skipped) | 150 (+1 skipped) |
| `XRay.Office.Spreadsheets.Tests` | 758 | 758 |
| `XRay.Office.Text.Tests` | 310 | **320** |
| `XRay.Office.Vector.Tests` | 295 | 295 |
| `XRay.Office.WordProcessing.Tests` | 818 | **819** |
| `XRay.Office.Fidelity.Tests` | 520 passed, **30 failed**, 550 total | 520 passed, **30 failed**, 550 total |

The Fidelity baseline reproduced the briefed **30 of 550** exactly, with **0 skipped**, before
anything was touched. Afterwards the failing set is not merely the same size — `diff` of the
failing test names before and after is **empty**. Build stayed at 0 warnings, 0 errors.

## Scoring the predictions

| # | prediction | outcome |
|---|---|---|
| 1 | the document reaches `638/638` and `match`, **high** | **right**, on both the gate metric and the raw one |
| 2 | our footer widens ~2.6 pt to within ~0.5 pt of the reference, **medium-high** | **right** — 338.64 → 341.71 against 341.21, 0.50 pt out |
| 3 | 1–3 of 200 words documents change verdict, **medium** | **right at the bottom of the range**: exactly 1 |
| 4 | at least one currently-passing words document regresses, **40%** | **wrong, and I am glad** — zero regressions across all 534 |
| 5 | Fidelity moves by at most 2 from 30/550, **medium** | **right, and stronger than predicted**: it moved by 0, same 30 tests |
| 6 | `words/batch-001..008` ends at 79/80 with `1447.doc` the failure, **medium** | **right** |

Five of six right. The one I got wrong I had hedged at 40%, and the reason it did not fire is
worth keeping: suppressing a ligature moves a line by the difference between the ligature's
advance and its components' — 14 units of 2048 for Carlito's `ti`, about 0.07 pt at 10 pt. That
is far too small to move a line break unless a line was already within a hair of one. **The
cascade risk of this change was real but tiny, and the reason is arithmetic that could have been
done in advance.** I did not do it, and predicted from the shape of the change instead.

Prediction 3's method deserves less credit than its result. I put 1–3 on a grep prior of 61
documents and got 1; the mechanism that decides it — a tracked run *and* a `liga` pair *and*
within the band — was never counted, so the interval was a guess that happened to contain the
answer.

## What this round did not fix, and deliberately

**We ignore a Word text box's internal insets.** A blind page-vision reviewer, given the page
image and none of the numbers, reported that the reference breaks the cover title over three
lines where we break it over two, sets everything below it lower, and draws two full-measure
rules around the footer that we omit. Independently: our footer starts at x=33.85 and the
reference's at 41.15, a difference of **7.30 pt** against the shape's `wps:bodyPr/@lIns` of
`91440` EMU = **7.2 pt**. `lIns` appears nowhere in `XRay.Office.WordProcessing`. Ignoring it both
displaces the text and widens the measure by `lIns + rIns` = 14.4 pt, which is exactly the kind of
difference that makes a title wrap one line later — the reviewer's own first candidate, arrived at
without knowing any of this.

It moves no word and no page on this document, it has a different blast radius, and folding it in
would have made prediction 4 unscoreable. It is recorded for a round of its own.

**A second instance of the shatter is still out there.** `words/batch-013/docx/A1. EASA Form 2.docx`
extracts 188 one-character tokens against the reference's 79 and only came down from 24 to 22
multi-character `ToUnicode` entries. It fails on pages (9/7), so the word gate cannot be judged on
it until that is fixed, but the residue says this class is not exhausted.

## Reproducing

```sh
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
export SOURCE_DATE_EPOCH=1700000000
dotnet/probes/words-b008-01/banked-check.sh /c/sandbox/workdir/sample-files 'words/batch-0*' /tmp/out 4
```
