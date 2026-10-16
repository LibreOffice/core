# L5-slides — summary

35 documents, six root causes, five patches. Read root cause **A** first: it is a *decision*, not
a bug, and it accounts for 24 of the 35.

| # | Root cause | Seat | Docs | Confidence |
|---|---|---|---|---|
| **A** | **Autofit is targeted at LibreOffice 26.2.4.2; this sweep's reference is 24.2.7.2.** The twelve `constScaleLevels` rows and the 0.250 floor are a correct port of 26.2. 24.2.7.2 answers a **2.5 % font grid with no floor and no full-size leading reduction**: measured over 36 probe decks it draws scales 0.550/0.625/0.725/0.800/0.825/0.850/0.875/0.925/1.000, four of which the table cannot produce at any rounding. `Lepore.ppt` p2 is the proof both ways — this reference draws 21.005/21.005 pt where we draw **20.013/20.409**, the pair `SlideAutofit.cs` itself records as measured on 26.2.4.2. | `Layout/SlideAutofit.cs:118-131` (`FitLevels`), `:155` (`FitFloor`) | **24** | **High** on the divergence and its size; the patch is a retarget and must not be applied without the target decision. |
| **B** | **`PPT_PST_TextRulerAtom` (4006) is parsed by nobody.** The constant is declared in `PptRecordTypes.cs:84` and has no consumer anywhere in the tree, so a shape that overrides its master's per-level indents is laid out from the master. Exact three-number match on `Aerospace…ppt` p5 (ruler says 152/419/304 master units; the reference draws 18.99/52.38/38.01 pt; the master says 228/495/304). | `MsBinary/PptTextReader.cs` (no reader), `MsBinary/PptTextBody.cs:178-182` | 2 measured, latent in **all 14** `.ppt` in the lane | **High** |
| **C** | **`PPT_PST_ExtendedParagraphAtom` (4012) is parsed by nobody.** PowerPoint's automatic numbering and picture bullets live in the shape's private `___PPT9` data, not in its paragraph properties — the paragraph goes on stating the master's round dot. So `(a) (b) (c) (d)` and `I. II. III.` come out as round bullets and arrow picture bullets come out as nothing. | `MsBinary/PptTextReader.cs` (no reader), `MsBinary/PptTextBody.cs:252-301` | 3 | **High** on the mechanism; the picture-bullet half is parsed but not drawn. |
| **D** | **A marker never asks for glyph fallback.** Runs do (`SlideTextLayout.cs:760`); a marker resolves on its own path and shapes `.notdef`, which draws nothing. `FAA_Form_337.ppt` p4: five Monotype Sorts slots recode to U+2776–U+277A, **OpenSymbol has a glyph for none of them**, the reference falls back to DejaVu Sans and draws the circled numerals. | `Layout/SlideTextLayout.cs:555-586` | 1 | **High** |
| **E** | **The PPT bullet's face ignores `PPT_ParaAttr_BuHardFont`.** The flag is parsed, documented in `PptParagraphRun.BulletFlags`' own remarks, and only its `BuHardColor` sibling is consumed. With the flag clear the face word means nothing and the bullet takes the first run's face. | `MsBinary/PptTextBody.cs:265` | 1 | **High** |
| **F** | **`lo-broken` — confirmed, not chased.** #162 the reference drops all four tables; #084 the reference overlaps its own title and headings; #130 the reference clips its last bullet under the footer. | — | 3 | **High** (two read directly) |

**Four refutations worth as much as the causes.** (1) *"A tab stop is not honoured"* on #113 — the
file uses **thirteen spaces**, not a tab; it is root cause A. (2) *"Bullet glyphs sit high"* on four
decks is **not** a bullet bug: `EmitMarker`'s centring is exact, and the 3.104 pt against the
reference's 0.964 on `G-Invoicing…pptx` is entirely `FitLevels` row 0's `{1.000, 0.900}` — a row
the reference is measured never to answer. (3) *"The body placeholder is laid out wider"* — no
measure defect found: on #030 and #129 both sides' text-area left edges and marker pens agree to a
thousandth of a point. (4) The largest cluster, *"list markers dropped or substituted"*, is
**three unrelated faults** (C, D, E), not one.

Counts overlap: #086 and #156 each carry a size difference (A) *and* a marker fault (C, D), and
#126 carries B and E on the same page.

**Unattributed: 4 of 35** — #096, #120, #147, #172 draw the same sizes at the same pens on both
sides and differ only in where lines break. That is the known advance-width divergence
(`dotnet/CLAUDE.md` rule 3), which is not in this lane.

**Apply order** (all `git apply --check` clean against HEAD):
`ppt-text-ruler` → `ppt-bullet-hard-font` → `marker-glyph-fallback` apply cumulatively.
`ppt-extended-paragraph-numbering` applies alone but collides with `ppt-text-ruler` in two
places — both add a trailing optional parameter to `PptTextRun` and to `PptTextReader.Read`; keep
both. `autofit-version-divergence` is independent and is **gated on a decision, not on a build**.
