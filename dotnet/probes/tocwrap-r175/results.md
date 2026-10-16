# r175 — a right stop past the line's end gives the paragraph's end indent back

`wordsink-r174` ranked the words track by ink and the head of it was
`02_mcar_part-2_and_IS_v2.10.docx` at **881.67 summed unsigned ink, 214 of its 312 pages MAJOR**,
nearly three times the next document — while its alphanumerics were **414 531 against 414 481**
and its page count **313 against 312**.

## The 214 MAJOR pages were one page, counted 214 times

Aligning the two renderings page by page shows ours one PDF page behind the reference's from page
29 on: our page 30 holds the reference's page 29 to the character, and so on to the end. Comparing
page *i* against page *i* after that compares different pages, so the ink figure is one defect
reported two hundred times. **`wordsink-r174`'s write-up said "something is drawn differently on
two thirds of the pages without changing what is drawn" — that reading is withdrawn.**

Four blind page readings (`page-vision`, one page each, no access to the repository) independently
reported the same thing and named it: *"ours is running roughly one full page behind reference at
this point"*, *"the item sequence is unbroken between the two"*. They agreed with the arithmetic
and with each other, which is what makes the class trustworthy.

## The extra page is one page of table of contents

Our TOC runs to a roman folio of xxvii where the reference's ends at xxvi. Diffing the two TOCs
line by line: **1906 lines against 1894 — twelve more**, and each of the twelve is an entry whose
page number we wrapped onto a second line:

```
ours   2.6.5 Instructors for Aviation Maintenance Technician Licences … 
ours   2-112
ref    2.6.5 Instructors for Aviation Maintenance Technician Licences.2-112
```

The reference keeps the number on the line by **shrinking the leader to one dot or none** —
`Licences.2-112`, `Examiner. .2-80`, `Check. 2-99`, `Licence 2-129`, `State. IS 2-8`.

## The rule, and one attribute settles it in both directions

`TOC1`–`TOC9` declare a dot-leadered **right stop at 9360 twips — the frame's own right edge** —
over `w:ind w:right="720"`, so the stop sits 720 twips past the paragraph's own line end.

`SwTextFormatInfo::GetLineWidth` (`sw/source/core/text/inftxt.cxx`:2132-2182) answers
`Width() - X()` until there is a pending tab whose stop is past `Width()`, and then answers
`nTextFrameWidth - X()` instead — the frame's width less the paragraph's **left** margin alone,
under the comment *"text is allowed to use the full text frame area to the right (RR above, but
not LL)"*. The two differ by exactly the end indent. So the text after such a stop is fitted
against the frame's edge, not the line's.

`variants.py` changes one attribute of those styles and nothing else:

| arm | ours before | ours after | 26.2.4.2 |
|---|---:|---:|---:|
| as authored | **313** | **312** | 312 |
| `w:ind w:right` deleted | 312 | 312 | 312 |
| right stop moved 9360 → 8640 (the line's end) | 313 | 313 | **313** |
| `w:leader="dot"` deleted | 313 | 312 | 312 |

**Both directions.** Taking the indent away makes *this tree* agree; moving the stop inside the
line makes *the reference* wrap exactly as this tree did. The leader is irrelevant, which rules
out the obvious alternative — that the reference simply measures leader dots differently.

## What it moves

`TabRuler.WidthOf`'s fitted answer now gives the end indent back when the line's last stretch
follows an aligned stop declared past the line's end. `TabbedSegment` carries the stop's declared
position for it, because that is not recoverable from where the stretch was placed.

Rendering all 337 words documents at the round's base and again after:

| | |
|---|---|
| renderings moved | **4 of 337**; 333 byte-identical |
| gate | **328 → 329 match** of 337 |
| `02_mcar_part-2_and_IS_v2.10` | 881.67 → **25.57** ink, **214 → 0** MAJOR pages, 313 → 312 pages, `pages` → `match` |
| `SPA-02_mcar_part-2_and_IS_v2.9` | 35.14 → 34.90 |
| the two FAA Holdover Tables | 321.20 → 321.27 and 92.95 → 93.02 |

The two that worsen do so by 0.07 each and both already fail on pages for the reason
`TODO.word-parity.md` records.

**The other two tracks cannot move**, and that is static rather than sampled: the rule is gated on
`ParagraphFormat.TabsOverSpacing`, whose only two setters in the tree are `WordParagraphFormats`
and `RtfDocumentReader` — the ODF, WW8, slide and sheet readers never set it.

## Correction (r179): the give-back is the trailing stretch's alone

The first cut subtracted the end indent from the **whole line's** fitted width, which also let a
long *title* borrow it and stay on a line the reference wraps. Writer reaches the wider limit only
once the right tab is the pending one — `GetLastTab()`, `inftxt.cxx`:2142-2144 — so everything in
front of that tab was already fitted against `rInf.Width()`.

`WidthOf` now floors the answer at `GapLeft`, where the tab began, so the title still has to fit
the line and only the stretch after the tab may reach into the indent.

Found by following the corrected ink ranking (`probes/inkmetric-r178`) to its head:
`SPA-02_mcar_part-2_and_IS_v2.9.docx` is `02_mcar`'s Spanish sibling with the same TOC geometry,
and there this tree fitted an entry's last word one line too high and then pushed the bare page
number onto a line of its own with no leader at all. A blind reading of its page 21 caught exactly
that — *"the left rendering loses the dot leader entirely when the trailing word is pushed to its
own line, while the right keeps the word, leaders and number glued together"*.

**4 of 337 renderings move, all four better and none worse**, 333 byte-identical, no page count
changed and so no gate verdict either:

| document | pixelD before | after |
|---|---:|---:|
| `SPA-02_mcar_part-2_and_IS_v2.9.docx` | 1128.68 | **1050.26** |
| `02_mcar_part-2_and_IS_v2.10.docx` | 223.28 | 222.79 |
| `24-25_FAA_Holdover_Tables.docx` | 690.96 | 690.79 |
| `FAA 2025-26 Holdover Tables.docx` | 397.69 | 397.51 |

`SPA-02`'s **page 21 is now identical** — 53 lines on both sides, the same first and last body
baselines, the same footer. What is left on that document is two other things and neither is this
rule: a nested list that runs one item later than the reference's from page 140 on, and a table
whose rows drift about 0.02 pt each so that page 3's lines sit 0.6 pt low by the foot.

`TheTextBeforeTheTabDoesNotGetTheIndent` pins the floor.

## Deliberately not modelled: the `.doc` arm

`GetLineWidth` needs `TAB_OVER_MARGIN` **or** `TAB_OVER_SPACING`, and a binary `.doc` carries the
first. Its arm is not the frame's edge at all but a flat **558 mm** — `tdf#158658`, *"Put content
after tab into margin like Word"*. That is a far wider give-back; no corpus `.doc` is known to
need it and reproducing it would move every `.doc` with a trailing right tab at once.
`WithoutTabOverSpacingNothingIsGivenBack` pins the boundary.
