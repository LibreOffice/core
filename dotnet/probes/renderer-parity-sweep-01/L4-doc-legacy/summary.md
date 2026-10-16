# L4-doc-legacy — summary

Four root causes with patches, three case-note mechanisms refuted, four left unresolved.
All four patches are inside `dotnet/src/XRay.Office.WordProcessing/Ww8/**` and apply cleanly to
`582c8c671` (`git apply --check`, all four). Nothing was built or tested — read-only checkout.

## R1 — a nested field's separator ends the *enclosing* field's instruction
The walk tracks "am I in an instruction" as one counter/flag instead of one entry per open field, so
an inner `SEQ`'s `U+0014`/`U+0015` cancels the outer `TC`'s suppression. The rest of the `TC` code —
`. RUNWAY OBJECT FREE AREA"`, quotation mark included — is then drawn on every numbered heading, and
the inner `SEQ`'s own result is dropped. `Ww8DocumentReader.Layout.cs:630,679,757,767,795` and
`Ww8DocumentReader.Content.cs:131,139,155,713`. **3 documents** (#004, #005, #007), 402/854/278
leaked characters. `patches/field-instruction-nesting.diff`. **Confidence: high** — byte stream,
both readers' own output and LibreOffice's flat-ODF export all agree.

## R2 — a shape whose FSPA rectangle has no area is discarded, and a rule has none
`Ww8Frames.cs:91` rejects `Width <= 0 || Height <= 0`. Word writes a vertical rule — a revision
change-bar — as an `mso_sptLine` whose two x-edges coincide, and `PageDrawing` already strokes a
line frame as its box's *diagonal*, so a coincident pair is exactly the rule wanted. **6 documents**;
degenerate anchors 108/123 (#003), 25/25 (#004), 58/72 (#005), 18/23 (#007), 2/2 (#033), 2/23 (#042)
— every one of them Escher shape type 20. `patches/degenerate-line-shapes.diff`. **Confidence: high**
for the mechanism, medium for how much of each page it recovers.

## R3 — HTML auto space-after is never handed back when a list ends at a cell mark
`Ww8DocumentReader.AutoSpacing.cs` runs the list rule per cell, so a run that reaches the end of a
cell never meets the unnumbered paragraph that restores its margin, and every such gap closes up.
LibreOffice's memory is reader-global and its cell rule (`SetPamInCell`) has already zeroed the
cell's *last* paragraph, so the margin lands on the one before it. **1 document in this lane**
(#080, 9 such runs; no other lane document has one). `patches/cell-auto-spacing.diff`.
**Confidence: medium-high** — the placement was measured on 24.2.7.2's own export only.

## R4 — a `FORMCHECKBOX`'s square lives inside the field's instruction and is dropped
Word writes `U+0013 " FORMCHECKBOX " U+0001 U+0015` with no separator, so the placeholder that *is*
the box is the last character of the field's code; the instruction guard eats it. 37 fields on #054
and 58 on #060, none with a `U+0014`. The DOCX reader already implements the identical portion
(`DocxLayoutSource.CheckBoxFrame`, pinned on 26.2.4.2); the WW8 side had nothing.
`patches/form-checkbox.diff`. **Confidence: high** for the diagnosis, medium for the ticked state.

## Refuted — three case-note mechanisms the files do not contain
* **#080 "empty paragraphs dropped from table cells"** — the piece table holds a single `U+000D`
  between the two bullets the gap sits between. The gap is a 280-twip auto margin (R3).
* **#033 "the ideographic comma after each list number dropped"** — both PDFs read `1、` and `A、`.
  The comma is drawn.
* **#054 "checkbox glyphs drawn in some sections and not others"** — the ones drawn are literal
  `U+25A1` characters (7 of them); all 37 *fields* are missing. No inconsistency to explain.

## Not established — recorded, not patched
#059's underlined date line (a Word-frame position/wrap question, not text placement);
#005/#007's footer chapter marker (derivative of the R1/R2 pagination offset); #025's diamond
bullets and #159's spurious bullet (neither corroborated by the list tables). The rest of the lane —
23 of 34 — is the reflow/advance-divergence class `dotnet/CLAUDE.md` already owns; nothing in the
WW8 reader accounts for it.

## Reference-version check (24.2.7.2 here vs 26.2.4.2 the tree is developed against)
R1 and R4 are version-independent: no Word or LibreOffice draws a `TC` field's code, and the
checkbox geometry these reuse was itself pinned on 26.2.4.2 (`probes/words-r56/prediction-checkbox.md`).
R2 draws a line the file states and is guarded to line shapes only. **R3 is the one measured against
24.2.7.2 alone** and its discriminating probe is stated in `findings.md`; today's output gives the
margin to *no* item, so the patch moves toward either answer rather than away from one.

## Cross-lane dependencies
**None.** All four patches are inside the two directories this lane owns.

**Apply order:** R1 before R4 (both rewrite the same instruction guard in
`Ww8DocumentReader.Layout.cs`; `form-checkbox.diff` is generated on top of
`field-instruction-nesting.diff`). R2 and R3 are independent and apply alone. All four applied in the
order R1, R2, R3, R4 give exactly the intended tree — verified with `patch --dry-run` at each step.
