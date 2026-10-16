# slides-sym-01 — prediction, committed before any measurement of the fix

Written after reading `dotnet/probes/slides-solog-01/results.md`, `oox/source/drawingml/textrun.cxx`
and the X-Ray.Office slides reader/layout, and after two things that are **results, not predictions**
and are recorded here so the scoring is honest about what was already known:

- **R0.** `XRay.Office.Fidelity.Tests` baseline on this worktree, measured before any edit:
  `Failed: 30, Passed: 520, Skipped: 0, Total: 550`. Matches the briefed baseline exactly.
- **R1.** The slides-track declaration census reproduces the previous round's figure exactly:
  **13 documents, 116 glyphs** carry a `U+F000`–`U+F0FF` character in an `a:t` on a `ppt/slides/slideN.xml`
  part, and all 116 of those runs carry an `a:sym` on their own `a:rPr`. 22 packages mention
  `<a:sym>` somewhere, which is the declaration count the previous round warned against.
- **R2.** Those 116 glyphs name four symbol faces: `Symbol` 51, `Wingdings 3` 25, `FontAwesome` 24,
  `Wingdings` 16.
- **R3.** 45 of the affected `a:t` values hold **both** private-use and ordinary characters, so a
  per-run face switch is not sufficient — the run has to be split at the character-class boundary,
  which is exactly what `textrun.cxx:99-105` does with its `bSymbol` run-length loop.

## Predictions

| # | claim | conf. |
|---|---|---:|
| 1.1 | The fix is expressible as a normalisation over `SlideParagraph` at the two `SlideTextLayout` entry points (`Place`, `Height`), with **no change to any offset downstream**, because the recode is one code point to one code point and the paragraph's `Text` keeps its length | 0.85 |
| 1.2 | The bullet path and the run path can share the recode decision — the same three-part guard (recodeable table, face's own file absent, slot in range) — so no second implementation is needed | 0.8 |
| 2.1 | **The resolved reach will disagree with 13/116, and the previous round's number is the one that is wrong** — it counted declarations of a symbol slot, not recodes. `FontAwesome` has no LibreOffice recode table, so its 24 glyphs on one document cannot recode | 0.8 |
| 2.2 | The recodeable subset is **12 documents, 92 glyphs** (116 − 24) | 0.7 |
| 2.3 | Renderings actually changed, measured by rendering the 163 twice and diffing, will be **11–13** — at most the 12 above, plus possibly `8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx`, whose slide layouts carry 4 further private-use glyphs the slide census does not see | 0.55 |
| 2.4 | No `.ppt` rendering moves: the binary path has no `a:sym` and its body runs are not symbolised at all (only its bullets are) | 0.85 |
| 3.1 | Zero verdict movement on the slides gate across all 163 documents. The gate counts letter-or-digit tokens and a symbol glyph is neither | 0.9 |
| 3.2 | `slides/batch-004` stays 9 of 10 and `slides/batch-00[1-6]` stays 57 of 58, with `solog` the single unchanged failure | 0.85 |
| 4.1 | After the fix `solog_orientation_august_2019.pptx` embeds `OpenSymbol`, moving the fonts column from 5/6 to 6/6 | 0.7 |
| 4.2 | The banked reference PDFs for the other 11 recodeable documents also embed `OpenSymbol`, so the same mechanism is visible corpus-wide and not a peculiarity of this deck | 0.75 |
| 5.1 | `XRay.Office.Fidelity.Tests` stays at exactly 30 failed, and the failing test *names* are unchanged | 0.85 |
| 5.2 | No other test project moves; build stays at 0 warnings | 0.9 |
| 6.1 | A blind reviewer given the `solog` page-9 pair will report the arrow as present in both halves after the fix, and will **not** rank it first among that page's differences — it is one 28 pt glyph on a text-heavy page | 0.6 |

## What would falsify the shape of the fix

If the resolved reach comes out well above 13 documents, the normalisation is firing on runs it
should not — most likely by switching a face for a *non*-private-use character, which
`textrun.cxx` never does. If it comes out at 1, the `a:sym` is not reaching the reader through the
`a:defRPr` chain and only `solog`'s run-level declaration is being seen.
