# Round 160 — a `TOC \t` switch voids the direct formatting of every paragraph in the styles it names

Reference binary: `/opt/libreoffice26.2/program/soffice`, **LibreOffice 26.2.4.2**
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). Corpus `/home/user/sample-files`, 271 DOCX.
Every rendering under `SOURCE_DATE_EPOCH=0`. This is **O110**, seated by round 159.

## What the seat said, and what it was

O110 was seated as *"a table splits a row late: on page 72 our rows sit a constant ~6 pt above the
reference's by the foot of the page"*. The 6 pt is real and the inference from it was wrong.
**Page 72 holds the same 1775 alphanumeric characters on both sides** and so does the page after
it, so no row moved: the reference simply emits a page that is *entirely empty* — its header, its
footer and nothing else — between `TABLE 50` and `TABLE 50 (CONT'D)`, and we do not.

Between those two tables the file states an otherwise-empty paragraph whose only content is
`<w:br w:type="page"/>`. The section's body bottom is y = 558 and the paragraph is one line of
13 pt Arial, about 14.9 pt tall. The reference's table ends at **545.80** — 12.20 pt of room, not
enough — so that paragraph goes to the next page and its break pushes the caption to the one after,
leaving page 73 blank. Ours ends at **539.75** — 18.25 pt — so it fits and no blank page appears.
The whole of O110 is therefore those 6.05 pt.

## Where the 6 pt is, measured line by line

Pairing the 32 horizontal rules of page 72: the offset is **−6.05 pt at the table's very first
rule and −6.05 at its last**, so the table's own row heights are exact and the whole of it starts
6.05 pt high. Above it, the caption's two lines agree to 0.02 pt on both sides and the paragraph
after the caption — `(see cautions and notes on pages 77 and 78)` — sits at **90.61** in the
reference and **84.61** here.

The caption is `<w:pStyle w:val="Heading3"/><w:spacing w:after="0"/>`; the `Heading3` style states
`<w:spacing w:after="120"/>`. The reference draws the **style's** 6 pt, not the paragraph's nought.

Two measurements establish that it is not a reading error. 26.2.4.2's own `--convert-to fodt` gives
that paragraph an automatic style holding `fo:break-before="page"` and **no margin at all**; and
patching the file's `w:after` from `0` to `480` and rendering the whole document through 26.2.4.2
changes **not one of its 155 pages**. The direct `w:spacing` on that paragraph is inert.

## The rule

Bisecting the body — keep the children from index *k* onwards, convert, read the caption's resolved
style — put the trigger at **child 23**, the paragraph that opens the document's
`TOC \h \z \t "Heading 2,1,Heading 3,2"` field. Adding that one field to a four-paragraph document
reproduces the defect exactly.

**A `TOC` field whose `\t` switch names a built-in `heading N` style makes every paragraph in that
style, anywhere in the document, lose its direct paragraph formatting.** The `\t` branch of
`DomainMapper_Impl::handleToc` (`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:7663-7707)
is where the difference is introduced — it fills the index's `LevelParagraphStyles` and sets
`CreateFromLevelParagraphStyles` — but **the seat that does the discarding is not located**, and
this write-up does not pretend otherwise.

### Twelve arms and three controls, all measured before a line was written

`tests/corpus/features/words-toc-template-styles.docx` (`make-probe.py`), read through
`--convert-to fodt` so the answer is the importer's rather than a rasteriser's
(`reference-arms.txt`):

| arm | the paragraph | 26.2.4.2 |
|---|---|---|
| A | built-in `heading 3`, named, `w:spacing after=0` | **discarded** — the style's 6 pt is drawn |
| B | the same with `w:jc left` over a centring style | **discarded** — drawn centred |
| C | the same with `w:ind left=720` | **discarded** |
| D | the same with `w:pageBreakBefore` | **kept**, and it is the only thing kept |
| E | the same as A but written *before* the field | **discarded** |
| G | `heading 4` while the switch names `heading 3` | kept |
| I | a `w:customStyle` whose **name** is `heading 5`, named | kept |
| J | a `w:customStyle` with an outline level, named | kept |
| K | the built-in `Title`, named | kept |

And three controls that **cannot** share a file with those arms, because the registration is
document-wide and A–E would void them — one document each, `make-controls.py`,
`reference-controls.txt`:

| control | 26.2.4.2 |
|---|---|
| F — a `\t` naming only `Heading 2` | keeps its direct spacing |
| H — a `\o "1-3"` switch and no `\t` | keeps it |
| L — no `TOC` field at all | keeps it |

So: it is the **built-in** heading and not the name (I), not the outline level (J), not any
built-in style (K), not the TOC as such (H), and not the paragraph's position (E). The match is
case-insensitive, which the witness needs — it writes `\t "… Heading 3,2"` where its style sheet
says `<w:name w:val="heading 3"/>`.

Also spot-checked on a second real document: `SPA-11_mcar_part-11_v2.9.docx`'s
`Parte 11. Trabajos aéreos` heading states `<w:numPr><w:numId w:val="0"/></w:numPr><w:ind
w:left="576" w:hanging="576"/>` and 26.2.4.2's resolved view of it carries **no indent of its own**.

## What changed

`DocxTocStyles.Voided(body, styles)` collects the ids, and `DocxTocStyles.Prune` replaces such a
paragraph's `w:pPr` with a copy holding only `w:pStyle`, `w:rPr` (the paragraph mark's), `w:sectPr`
and `w:pageBreakBefore`. `DocxLayoutSource.Read` computes the set before the walk — a `TOC` may be
written after the paragraphs it decides — and `Paragraph` prunes at the single point where a
`w:pPr` is read. `WordStyle.IsCustom` is new and is what arm I turns on.

## Reach and confinement

`census.py`: **21 of 271 corpus DOCX state a `TOC \t` switch; 7 of them hold a paragraph it voids**,
129 paragraphs in all — `SPA-11` 87, `OM template` 18, `hdss-bulletin-issue-285` 9, the two FAA
documents 6 each, `SPA-06` 2, `report-template` 1.

Rendering all 271 against round 159's bank: **265 byte-identical, 6 moved, 0 failures**
(`confine-rows.tsv`, `movers.txt`):

| document | reference | before | after |
|---|---|---|---|
| `24-25_FAA_Holdover_Tables` | 155p 299 583 | **154**p 299 532 | **155**p 299 605 |
| `FAA 2025-26 Holdover Tables` | 167p 335 603 | 167p 335 623 | 167p 335 623 |
| `hdss-bulletin-issue-285` | 10p 19 647 | 10p 19 699 | 10p 19 699 |
| `SPA-06_mcar_part-6_and_IS_v2.9` | 85p 142 938 | 85p 142 938 | 85p 142 942 |
| `SPA-11_mcar_part-11_v2.9` | 49p 70 763 | 49p 70 763 | 49p 70 781 |
| `OM template for non-complex NCC operators` | 165p 270 009 | 166p 270 063 | 166p 270 086 |

**One gate verdict gained and none lost**: the witness goes `pages` → `match` at 155 of 155, which
closes O110. Two documents drift away from the reference by a handful of characters — `SPA-11` by
18 on 70 763 and `SPA-06` by 4 on 142 938, both from an exact match and both still well inside the
band — and that is the honest cost of the change: the rule moves indents and spacing on 129
paragraphs, and where a line then wraps differently the character count follows. `OM template`
stays one page long, which is a different defect.

## The decision was reversed: the rule ships switched OFF

**The user's call, and it changes the conclusion of this round rather than its measurements: this
project wants Word parity.** Word honours the paragraph's own formatting; 26.2.4.2 throws it away.
So `DocxTocStyles` is gated on `XRAY_OFFICE_LIBREOFFICE_TOC_STYLES`, which is **off by default**, and
everything below this line describes what happens when it is switched on.

What that costs is written down in `dotnet/TODO.word-parity.md` and is one gate row:
`24-25_FAA_Holdover_Tables.docx` prints **154 pages against the reference's 155**, deliberately,
while sitting within 51 characters of it over those 155 pages. It also *gains* something — with the
rule off, `SPA-11` and `SPA-06` are back on the reference's character count **exactly**, where the
rule put them 18 and 4 characters away.

Re-rendering all 271 corpus DOCX with the rule off reproduces round 159's output **271 of 271,
byte for byte** (`confine-rows-off.tsv`), so the shipped behaviour of this round is exactly the
behaviour before it. What the round leaves is the measurement, the switch and the note.

So read the table of movers above as *what the reference does*, not as what this tree draws, and
read the verdict claim in it — `pages` → `match` on the witness — as conditional on the switch.

## A note on reproducing it at all

This is not behaviour any reader would choose, and Word does not do it. The round shipped it on the
same footing as the RTF bookmark rotation of round 88 and the doubled `Tables Table 55` of round
158 — the gate scores this tree against 26.2.4.2's own output and the difference decides pages —
and that was overruled the same day: **the project wants Word parity**, and those two precedents
are worth re-examining on the same grounds by whoever next has cause to. The difference between
this case and theirs is that theirs change what a reader *reads* to match the reference's own text,
while this one throws away formatting the author asked for; but that is a distinction to argue
rather than one the code currently makes.
