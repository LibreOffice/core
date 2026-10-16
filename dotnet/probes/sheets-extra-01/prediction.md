# sheets/extra-001 — prediction, written before the fix was measured

Written 2026-08-14, branch `wt-sh-extra`, against LibreOffice 26.2.4.2 with the full font set
(`check-env.sh` green, `fc-match "DejaVu Sans"` → DejaVu Sans).

Everything below is a claim about what the *fix* will do. The reference-side measurements it
rests on were taken first, with `soffice`, and are recorded as facts rather than predictions in
the "already established" section.

## Already established by measurement (not predictions)

Authored `.xlsx`/`.docx`/`.pptx` probes, converted with the installed 26.2.4.2, read out of the
PDF's text-showing operators rather than off a raster:

| input, in a shared string | LibreOffice 26.2.4.2 draws |
|---|---|
| `ALPHA_x000D_BRAVO` | `ALPHABRAVO` — one `Tj`, one baseline. CR is **dropped, not broken** |
| `CHARLIE_x000A_DELTA` | two baselines. LF **is** a line break |
| `vv_x000D__x000A_ww` | two baselines — CR dropped, LF breaks |
| `ECHO_x005F_x000D_FOXTROT` | `ECHO_x000D_FOXTROT` — `_x005F_` un-escapes |
| `kk_x0020_ll` | `kk ll` — space decoded and kept |
| `qq_x20AC_rr` | `qq€rr` — non-ASCII BMP decoded |
| `aa_x000d_bb` | `aabb` — **lower-case hex is accepted** |
| `ee_x001E_ff`, `gg_x000B_hh`, `ii_x0002_jj`, `ss_x0000_tt` | glued, no glyph — other C0 dropped |
| `mm_x00D_nn`, `oo_xZZZZ_pp` | literal — a malformed escape is not an escape |
| `aa<TAB>bb` (literal tab) | `aabb` — same as `_x0009_`; the rule is on the character, not the spelling |
| same strings in `w:t` (docx) and `a:t` (pptx) | **all four literal.** No decoding at all |

Two consequences the brief does not have:

1. **The words and slides readers do not share the gap.** `w:t` and `a:t` are `ST_String`;
   only SpreadsheetML's `CT_Rst/t` is `ST_Xstring`. Measured, not read off the schema.
2. **Header/footer is not in scope, and our footer is already right.** `oddFooter` holding
   `&L_x000D_&1#…` renders the seven glyphs *in the reference too*. Our page-1 footer for
   `Published_Issuances_2024.xlsx` is byte-identical to the banked reference's. The blind
   reviewer who read `_x000D_ Classification: GENERAL` was reading a correct rendering.

So the brief's "six documents, five of them in `done-*`" is wrong in both directions: four of
its six are header/footer-only and must not change, and the corpus sweep finds four more with
escapes in shared strings that the brief does not list.

## Predictions

**P1 — the seat.** One decoder for `ST_Xstring`, applied where SpreadsheetML rich text is
flattened: `XlsxSharedStrings.ReadRichString` and, in lockstep, `XlsxRichRuns.Read`, which
computes run offsets into that same flattened string. If the two are not changed together the
runs will point at the wrong characters. Nothing in `XRay.Office.WordProcessing`,
`XRay.Office.Presentations` or the header/footer path changes.

**P2 — `sheets/extra-001` passes.** `FY2018_Q4_UAS_Sightings.xlsx` goes from `304/302`,
`57225/55825` to `302/302` and a word count inside the 2% band. I expect pages to land exactly
on 302 rather than merely closer, because the surplus is one systematic term.

**P3 — reach is 6 documents, not 5.** Renderings that change across all three tracks:

| document | why |
|---|---|
| `sheets/extra-001/…/FY2018_Q4_UAS_Sightings.xlsx` | 4872 `_x000D_` in `sharedStrings` |
| `sheets/done-013/…/afn-…-fy25-jan25-mar25.xlsx` | 160 `_x000D_` in `sharedStrings` |
| `sheets/done-016/…/TK-Syllabus-Comparison-Document-v2.xlsx` | 6 `_x001E_` |
| `sheets/done-010/xls/Special-Procedures_2025-07-10.xls` | 4 `_x000D_` (an xlsx mislabelled `.xls`) |
| `sheets/done-015/…/…MAdB-Light-Prop-14-28112013.xlsx` | 1 `_x000B_` |
| `sheets/done-011/…/Application_for_authorisation…xlsx` | 1 `_x0002_` |

plus, from the literal-control half of the same rule, up to 8 more sheets documents that carry a
literal TAB or U+007F in a `<t>`. I predict **0 words** and **0 pages** move on those 8: every
one I inspected has the tab leading or trailing, or between a bullet and a word.

I predict **0 documents in `words/` and `slides/` change at all** — the 78 documents there
carrying `_xHHHH_` text all carry `_x0000_` inside VML `o:spid`/`id` attributes, which is not
text and is not decoded by anybody.

**P4 — regression.** `sheets/done-*` (156 documents) holds the same MATCH count after the fix as
before it, and the six documents above stay `match`. This is the run that matters; the group is
one document.

**P5 — tests.** New tests fail against the unfixed tree and pass after. Fidelity stays at
**30 failed / 520 passed / 0 skipped / 550 total** — the escape appears in no fidelity fixture,
so a change there would mean I had moved something I did not intend to.

## What would falsify the reading

If `FY2018` lands on 303 or 301 pages, the decode is right and something else on that document
is also wrong. If any `done-*` document that carries **no** `_xHHHH_` and no literal control
moves at all, the change has leaked outside its seat.
