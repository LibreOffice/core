# words-r54 — prediction, committed before the change

Environment: LibreOffice **26.2.4.2 620(Build:2)**; `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`;
corpus `/c/sandbox/workdir/sample-files`; worktree `wt-words-r50` on branch `wt-words-r54`,
base `4aea606ac78`; `SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, reproduced before anything was touched

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 335  MISMATCH 20`; scored against
`MANIFEST.tsv`'s 337-path list rather than that total, **318 match, 19 open, and 0 disagreements
with the manifest's status column, document for document.** The briefed baseline reproduces
exactly, so round 53 did end byte-equal to baseline.

Both other tracks were swept at this same commit for the cross-track census below and both
reproduce too: **slides 199 of 302, sheets 271 of 307**, each with 0 disagreements against the
manifest.

## The rule, established before the change rather than assumed

`font-fallback-rule.py` (98 authored files) and `cross-format-fallback.py` (28 more), all through
the installed `soffice`, five known-answer controls agreeing in the first and the second's
families chosen so `fc-match` separates three generics. The rule is **not** the one the brief
described, and it is not a property of the font resolver at all:

| filter | an unrecognised family, nothing declared | with the document declaring a class |
|---|---|---|
| **DOCX** | **DejaVu Serif**, always | only `w:family="swiss"` moves it, to DejaVu Sans; `roman`, `modern`, `script`, `decorative`, `auto` and `w:pitch="fixed"` all leave it Serif |
| **DOC** | **DejaVu Serif** | `swiss` → DejaVu Sans, same as DOCX |
| **RTF** | **DejaVu Serif** | nothing moves it — `\fnil`, `\froman`, `\fswiss`, `\fmodern` all answer Serif |
| **ODF text** | fontconfig's own generic — Aptos → Sans, Consolas → **Mono**, Garamond → Serif | `style:font-family-generic` and `style:font-pitch` both honoured |
| **XLSX, PPTX, FODS** | fontconfig's own generic, exactly as `fc-match` | — |

So the answer does **not** depend on the request (bold, italic, 8 pt, 40 pt and an east-Asian
hint all answer the same family), and it does **not** depend on the shape of the name, and it does
**not** depend on what fontconfig files the name under. It depends on **which filter read the
file**. Twenty-one further names probed through DOCX — `Times`, `Helvetica`, `Albany`,
`Thorndale`, `Courier`, `Nimbus Roman`, `Nimbus Sans`, `CG Times`, `Times-Roman`, `MS Gothic`,
`MS Mincho`, `Century Schoolbook`, `Book Antiqua`, `Bookman Old Style`, `Lucida Console`,
`Palatino Linotype`, `SimSun`, `Wingdings` — **all answer DejaVu Serif**, and the only three that
escape are the strong metric aliases (`Times New Roman` → Liberation Serif, `Arial` → Liberation
Sans) and the pi face (`Symbol` → OpenSymbol), which is exactly what the resolver already exempts.

## The change

Two sites, **both in `XRay.Office.WordProcessing`**, and none in `XRay.Office.Text`:

- `DocxLayoutSource.Face` — the class handed to `FontRequest` becomes `SansSerif` when the font
  table declares `swiss` and **`Serif` otherwise**, which is the DOCX filter's roman default.
- `LayoutFonts.Lookup` — the same, which covers `.doc` (whose `FFN` supplies a class) and `.rtf`
  (which supplies none, so it takes the Serif default unconditionally, exactly as measured).

`OdtLayoutSource`, `SlideText`, `SheetFonts`, `MetafileTextEngine` and `SvgTextEngine` are
untouched, because the measurement says their filters are already right.

`SystemFontResolver.GenericFallbacks`'s `_ => SansFallbacks` is therefore **correct and must not
be changed**: it is the ODF/PPTX/XLSX answer. The `[24.2.7-audit: WRONG]` marker at that site is
right that the site's *stated reason* is false and right that ours disagrees with the reference on
DOCX; it is wrong about where the seat is.

## What I expect to change

**Renderings.** The 32 words documents whose baseline rendering already shows the disagreement —
31 of them exactly `ours = DejaVu Sans, ref = DejaVu Serif` and one (`fleetfastfacts16nov2023`)
that pair plus another difference:

```
003_Free_Genogram_Diagram_Template_Easy_Format          072_Storyboard_Template_Colored_Theme
004_Free_Genogram_Diagram_Template_Editable_Format      073_Storyboard_Template_Easy_to_Use
008_Free_Genogram_Diagram_Template_Green_and_Yellow     074_Storyboard_Template_Editable_Design
009_Free_Genogram_Diagram_Template_Handy_Format         076_Storyboard_Template_Gray_Theme
033_Venn_Diagram_Template_Colored_Theme                 077_Storyboard_Template_Pink_and_Blue_Theme
035_Venn_Diagram_Template_Editable_Format               078_Storyboard_Template_Pink_and_Gray_Theme
053_Organogram_Template_Creative_Theme                  1_tpr_template__from_fy14_
054_Organogram_Template_Grey_Vertical_Theme             AFS-050-004-F2_0i
056_Organogram_Template_Square_Theme                    Company-profile-2022-EN
063_Foot_Reflexology_Chart_Complete_Guide               ESPN-R - MCF - RA - Ed1
064_Foot_Reflexology_Chart_Customizable_Format          FO.FCTOA_.000129 Application … FSTD
068_Work_Breakdown_Structure_Template_Green_Theme       How-to-Write-an-Architecture-Document-…
Lessons-Learned-Bulletin-Dorset-version-to-IIRG-V5      SDL_FSDO_Part91_LOA_Checklist
Writing a technical report (SCE subject guide)          part-145-approval list (1)
part-145-approval list 2025                             technical report template
technical-memo-format                                   fleetfastfacts16nov2023
```

I expect **more than 32 renderings to move**, because a face change reflows and the census only
sees documents where the *embedded list* differs; a document that draws the wrong DejaVu **and**
the right one elsewhere already agrees on the list. **Predicted 32 to 60 of 337.**

**Verdicts: 0.** Not "probably zero" — the arithmetic says so. Font embedding is check 3 of the
gate and **no words document currently fails on it**: the 19 open ones fail 9 on words, 9 on
pages and 1 on both. So a font change can only move a verdict through reflow, and

- the only currently-**open** document in the target set is `008_Free_Genogram…`, which fails
  **66/70 words** — a text-extraction gap that a face change cannot close. No gain there.
- the other **31 currently pass**, so every one of them is a regression risk and none is a gain.

**Predicted verdict movement: 0, with a stated downside risk of −1 to −3.** The named risks are
the multi-page members of the target set, where a changed advance can move a page boundary:
`ESPN-R - MCF - RA - Ed1` (58 pages), `technical report template` (10), `AFS-050-004-F2_0i` (8),
`1_tpr_template` (8), `part-145-approval list (1)` (8), `part-145-approval list 2025` (7),
`FO.FCTOA_.000129` (6), `technical-memo-format` (5), `Company-profile-2022-EN` (5). The
single-page templates cannot move on page count and all sit at 0.00–1.16% word margin.

**A gain of +1 or more would mean the census under-reached**, and I would rather say that in
advance than discover it as a pleasant surprise.

## What this census cannot see

1. **Charts inside Word documents.** `FrameChart` builds `new FontRequest(family)` with no
   declared class and is not touched by either site, so chart labels keep the ODF/fontconfig
   answer. Whether LibreOffice's chart module agrees was not probed.
2. **EMF and SVG text.** `MetafileTextEngine` and `SvgTextEngine` resolve without a class too.
   Metafile text inside a DOCX may well take the same roman default in the reference; unprobed.
3. **A face that is wrong by file rather than by name.** `pdffonts` reports base font names.
4. **Documents that embed nothing.** A page with no text agrees trivially.
5. **The `.doc` arm's reach.** 17 of the 86 disagreements are `.doc` and none of them is a plain
   Sans-for-Serif pair, so the DOC census is *empty* while the DOC probe says the rule applies.
   Two `.doc` rows carry `DejaVuSans` on our side (`1228841571067_2009_TPPT_13…`,
   `150_5335_5a`) and could move. This is the one arm where the prediction is a floor.
6. **Glyph fallback.** A DejaVu Serif face that lacks a glyph falls back per character, and the
   fallback resolver is the same one; the census sees the resulting list, not the route.
7. **`w:family="swiss"` declared on a family that the reference nonetheless draws serif.** No such
   case was found in 98 probes, but the corpus is larger than the probe set.

## Cross-track: nothing is owed, and that is measured twice

`git diff --stat` will show no file under `XRay.Office.Text`, `Core`, `Containers`, `Vector`,
`Rendering`, `Markup` or `Ooxml`, so the change cannot reach slides or sheets at all.

Independently of that, the change the brief expected — flipping `GenericFallbacks` to serif in
`XRay.Office.Text` — **would have broken both other tracks**, and both instruments say so:

- authored PPTX, XLSX and FODS files, three families whose fontconfig generics differ, answer
  `DejaVu Sans`, `DejaVu Sans` and `DejaVu Sans Mono` — `fc-match`'s column exactly, not Serif;
- over all 302 slides and 307 sheets renderings compared against the reference's own font lists,
  **zero documents show `ours = DejaVuSans, ref = DejaVuSerif`.** Not one, on either track.

**Predicted cross-track verdict movement: 0 slides, 0 sheets. Predicted cross-track renderings
changed: 0, byte for byte.**
