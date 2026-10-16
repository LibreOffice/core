# words-r55 — second prediction, the DOC arm, committed before the change

Committed after the DOCX change was measured (**words 317 → 318 of 337, one gain, zero
regressions**) and before anything on the `.doc` path is touched or re-rendered.

## What the probe established

`doc-family-code.py`, nine fixtures. Round 54 probed this arm by converting an authored DOCX to
Word 97 with `soffice` and back, and refuted its own probe in the same round: the DOCX *import*
applies the roman default before the export runs, so the `.doc` it wrote declared `ff=roman` and the
probe measured "declared roman". **A flat ODF file defeats that**, because the ODF filter has no
roman default: a `style:font-face` with no `style:font-family-generic` leaves `SvxFontItem`'s family
at `FAMILY_DONTKNOW`, and `wwFont::Write` (`sw/source/filter/ww8/wrtw8sty.cxx`:821) maps that onto
`ff = 0`. So `.fodt` → `.doc` → `.pdf` reaches the WW8 *import* with a genuinely undeclared `FFN`.

| fixture | 26.2.4.2 draws |
|---|---|
| `Zqxwv Nonesuch`, no generic | **DejaVu Sans** |
| `Zqxwv Nonesuch`, `roman` | DejaVu Serif |
| `Zqxwv Nonesuch`, `swiss` | DejaVu Sans |
| `Zqxwv Nonesuch`, `modern` | DejaVu Sans |
| `Zqxwv Nonesuch`, `decorative` | DejaVu Sans |
| `Aptos`, no generic | **DejaVu Sans** |
| `Garamond`, no generic | **DejaVu Serif** |
| `Univers`, no generic | DejaVu Sans |
| `Helvetica`, no generic | DejaVu Sans |

**Through the DOC filter only `ff = roman` gives Serif; every other code, and no code at all, gives
fontconfig's own generic.** That is the opposite of what we implement, which turns an unclassified
`FFN` into the roman default.

`Garamond` is the control that says this is really reaching `SwWW8ImplReader::GetFontParams`
(`sw/source/filter/ww8/ww8par6.cxx`:3767) rather than measuring the export: that function carries a
**name-override list with no counterpart in the DOCX filter** — seven prefixes forced to
`FAMILY_ROMAN` (`Tms Rmn`, `Timmons`, `CG Times`, `MS Serif`, `Garamond`, `Times Roman`,
`Times New Roman`) and seven to `FAMILY_SWISS` (`Helv`, `Arial`, `Univers`, `LinePrinter`,
`Lucida Sans`, `Small Fonts`, `MS Sans Serif`) — and `Garamond` comes back Serif where the
otherwise identical `Aptos` comes back Sans.

**And this unifies all three word-processing filters rather than adding a third rule.** In every
one of them `FAMILY_DONTKNOW` reaches fontconfig's own generic and `FAMILY_ROMAN` appends
`"serif"`. The DOCX filter never *sets* `DONTKNOW` — it leaves the inherited value, whose floor is
Writer's roman pool default; the RTF filter never sets the family at all, which is why `\fnil`,
`\fswiss` and `\fmodern` are inert; and the DOC filter sets it explicitly per font, `ff = 0`
included, which is the only one of the three that can reach `DONTKNOW`.

## The change

`Ww8FontTable.ShapeOf` gains the fourteen-prefix override list, and `LayoutFonts` stops handing a
DOC family through the roman default: when a font table is supplied — which only `DocReader` does —
the `FFN`'s own code is the whole answer, `Unknown` included. Nothing else moves; RTF, which
supplies no table, keeps the roman default, and the DOCX path does not use `LayoutFonts` at all.

## The census, and what it cannot see

`.doc` renderings in the corpus: **66**. Of those, **15 draw a DejaVu Serif face on our side** and
are the only ones a Serif→Sans change can reach:

| | count |
|---|---:|
| our `.doc` renderings drawing DejaVu Serif | 15 |
| …where the reference draws DejaVu Serif too — **currently correct, and at risk** | **13** |
| …where the reference draws DejaVu Sans instead — the target | **2** (`congregationalhistories_ky_2023`, `手机免提系统TSB`) |

What this cannot see: **which `FFN` code each of those 15 actually carries.** The census is over the
*rendered* faces, not the font tables, so it bounds the blast radius at 15 documents and cannot say
which way any of them will move. Thirteen of them are correct today. It also cannot see a document
the override list moves in the *other* direction — a `.doc` naming `Helvetica`, `Univers` or
`Lucida Sans` and currently drawn Serif would become Sans, and one naming `Garamond` or `CG Times`
and currently drawn Sans would become Serif; those are inside the 15 only if they already draw a
DejaVu face.

## The numbers predicted

| | predicted |
|---|---|
| renderings whose bytes change | **2–15**, all `.doc` |
| font-list disagreements, 53 after the DOCX change | **50–53** |
| `ours=DejaVuSerif, ref=DejaVuSans` (3 now) | **1** — `template---tpr-…` is a `.docx` and this cannot reach it |
| **verdict movement** | **0.** Both target documents already pass the gate; this buys font-list agreement, not a verdict |
| downside risk | **−1 to −4**, entirely within the 13 `.doc` documents that are correct today |

**Zero upside on verdicts and a real downside, stated plainly.** The case for making the change is
that it is a measured correctness fix with a blast radius of at most 15 documents and a full-track
sweep to check it. **If the sweep shows a net loss it will be reverted and the round will report the
refutation**, and the probe stands either way.
