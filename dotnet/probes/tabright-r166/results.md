# tabright-r166 — a right tab stop past the paragraph's right indent

**Reference: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`).** Ours is this tree's own CLI.

## Headline: the candidate fix is REFUTED, and nothing was landed

`probes/pages-r164` §2 diagnosed `02_mcar`'s 313-against-312 as a right tab's trailing stretch
being fitted against the paragraph's right indent where the reference fits it against the frame's
edge, named `TabRuler.WidthOf`:196 as the seat, and characterised the rule as

> `titleEnd ≤ tabStop − rightIndent − ε` **and** `titleEnd + numberWidth ≤ tabStop`, with ε ≈ 2.5 pt

**That characterisation does not survive a clean sweep, and the patch built from it makes this
fixture worse.** Implemented exactly as the named mechanism says — see §2 — the disagreement with
26.2.4.2 goes from **6 arm-steps to 9**. It was reverted, and the tree's rendering of this fixture
is byte-identical to its rendering before the change.

## 1. The fixture and the sweep

`build.py` writes 110 paragraphs of the shape every `TOC2` entry in `02_mcar` has: a text column
9360 twips wide, a **left** stop at 990 and a **right, dot-leader** stop declared at 9360 — exactly
the column's right edge — and `w:ind w:left="720" w:right=R w:hanging="720"`. Ten right indents ×
eleven title lengths. `word/settings.xml` is written, empty, so the importer takes the OOXML
compatibility defaults (`office-corpus/SKILL.md`).

`score.py` prints `#` where the page number stays on the entry's first line and `.` where it does
not, read from `pdftotext`'s own line grouping. **Two instrument corrections were needed and both
changed the answer:**

- A first cut keyed the number's baseline against a *global* set of baselines, so arms on the same
  line of different paragraphs paired with each other. It reported the reference fitting the number
  only at the **widest** indents, which is backwards.
- Matching the literal `2-123` reads an entry that *did* fit as one that did not, because where the
  number overprints the leader the extractor loses the hyphen — `....2123`. The pattern is `2-?123`.
  This one moved three cells of the reference's own table, including the `r0n54` cell the analysis
  below turns on.

`sweep.txt` holds both tables. The boundary — the largest title that keeps the number on the line:

| right indent (tw) | 0 | 180 | 360 | 540 | 720 | 850 | 994 | ≥1130 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 26.2.4.2 | 54 | 54 | 52 | 52 | 52 | 48 | 48 | never |
| this tree | 54 | 52 | 50 | 50 | 48 | 48 | 46 | never |

They agree at a zero right indent and part company as it grows — which is the direction §2's
diagnosis predicts. **Six arm-steps**, two characters each — and see §2 for why that is the wrong resolution to
reason about.

## 2. The mechanism is real, and it is not `TabRuler.WidthOf`'s return value

`[src]` `SwTextFormatInfo::GetLineWidth` (`sw/source/core/text/inftxt.cxx`:2132-2182) is the
named seat:

```cpp
SwTwips nLineWidth = Width() - X();
if (!bTabOverMargin && !bTabOverSpacing) return nLineWidth;
SwTabPortion* pLastTab = GetLastTab();
if (!pLastTab) return nLineWidth;
if (pLastTab->GetTabPos() <= Width()) return nLineWidth;
// "text is allowed to use the full text frame area to the right (RR above, but not LL)"
nLineWidth = nTextFrameWidth - X();
```

So where the last tab's stop lies beyond the paragraph's own right boundary, the text after it is
fitted against the **text frame's** edge — and for a right, centred or decimal stop under
`TabOverSpacing` (what every writerfilter document carries) the comment says outright that it
"can back-fill all the available space". The two boundaries differ by exactly the paragraph's end
indent, which is §2's reading.

**Implemented faithfully** — `TabbedSegment` carrying the stop it was placed by, and `WidthOf`
reporting `GapLeft + Width − EndIndent` when that stop is past the line's own edge — the sweep
comes back **indent-independent**: `#` to title 54 in every column from 0 to 994. That is the rule
doing exactly what the source says, and the reference does **not** do it:

| right indent (tw) | 0 | 180 | 360 | 540 | 720 | 850 | 994 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 26.2.4.2 | 54 | 54 | 52 | 52 | 52 | 48 | 48 |
| the patch | 54 | 54 | 54 | 54 | 54 | 54 | 54 |

Nine arm-steps against the six we started with, and wrong in the *other* direction on five of the
seven columns. So something else clamps the reference back, and this round did not find it.

**Three things the next round should not re-derive.**

- **ε is not a constant, and neither is anything else this fixture can measure — its resolution
  is too coarse to tell one rule from another.** The sweep steps the title by two characters,
  which is **16.28 pt** of the reference's own advance, while its right-indent steps are 6.5 to
  9 pt. A boundary located to ±16 pt in a variable that moves 9 pt at a time cannot discriminate,
  and two passes of arithmetic over this table reached two different conclusions for exactly that
  reason. `build-fine.py` is the instrument that can: it fixes the title and sweeps
  `w:ind w:right` in **20-twip (1 pt) steps**, so the boundary edge is located to a point.

  | title | 26.2.4.2 | this tree |
  |---|---|---|
  | 44 `A` | fails past edge **490.5** | fails past edge **490.5** — *exact* |
  | 50 `A` | fails past edge **497.5** | fails past edge **508.5** — **11.0 pt too strict** |
  | 56 `A` | fails at every indent | fails at every indent |

  So the residual is **one number**: at a title six characters longer, we give up 11 pt of line
  sooner than the reference does. And it is not an offset — at 44 characters the two agree to the
  point, so whatever the rule is, it is one both engines get right at one length and not at the
  other.

  **The rule itself is still underived**, and the arithmetic in an earlier draft of this file that
  claimed to refute a constant is withdrawn with the coarse table it was computed from: both
  engines' thresholds move far less with the title than the title's own width does (the
  reference's by 7.0 pt for 6 characters, ours by 18.0), so the binding term is not the title's
  end against the line's edge in either. `sweep-fine.txt` is the measurement to work from.
- **`GetTabStop`'s first-stop rule is why the stop is honoured out in the indent at all**, and it
  is already right in this tree. `SwLineInfo::GetTabStop` (`txttab.cxx`:43-62) rejects a stop past
  the paragraph's right boundary — *except the first in the ruler*, for which it instead **raises
  the boundary to the stop's own position**. Measured: at every right indent from 0 to 994 both
  engines put the number's right edge at x 540.0, the declared stop, which is up to 49.7 pt out in
  the indent. `ClampsTabsAtLineEdge` already models this.
- **At right ≥ 1130 both engines move the number to a second line, and where they then put it is a
  separate defect.** The reference right-aligns it at x **608.033** — 68 pt past the text column,
  out in the *page* margin — and we put it at 153.463, at the left. Worth a seat of its own; it is
  not what decides `02_mcar`.

## 3. What is still owed on `02_mcar`

The row is unchanged: 313 pages against 312, and the six ToC page numbers the reference keeps on
their entry's line and we orphan are still orphaned. The *direction* of the defect is confirmed by
this sweep — we break earlier than the reference at every non-zero right indent — and the seat is
still the fit width for a line ending in a deferred stop. What is missing is the clamp that keeps
the reference from taking the whole frame.
