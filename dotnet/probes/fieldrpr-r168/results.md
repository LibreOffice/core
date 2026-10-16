# fieldrpr-r168 — where a recomputed field's value takes its character properties

**Reference: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`).** Ours is this tree's own CLI.

## Headline

**26.2.4.2 discards a field result's own `w:rPr` entirely.** The value it computes is drawn in
the character properties the field's **instruction run** carries, falling back to the paragraph
style — never the cached result runs'. `\* MERGEFORMAT` makes no difference to it: five of the
seven arms below carry the switch and answer the same as the one that does not.

**Word does honour the switch**, which is what the switch is for — *preserve the formatting of
the previous result*. So the default here follows Word (apply the rule to a field that does not
carry `\* MERGEFORMAT`) and `XRAY_OFFICE_LIBREOFFICE_QUIRKS` applies it to every field, which is
26.2.4.2's own reading. `TODO.word-parity.md` carries the entry.

## 1. The arms, in a footer

`build-footer.py` writes seven one-attribute arms of a `PAGE` field, each with a cached result
stating **20 pt red** against a paragraph style stating **10 pt black**, and puts them in a
**footer** — which is the correction this round had to make before anything could be measured.
`build.py` (`pages-r164`'s original) puts them in the body, and **this tree draws a body
`NUMPAGES` from its cache**, so every arm there differs by the value as well as by its
formatting and the formatting rule cannot be read out of it. In a footer both engines recompute
and the only thing left to differ is the formatting.

| arm | 26.2.4.2 | ours | ours + quirks |
|---|---:|---:|---:|
| `CTRL` — a plain run carrying the same `w:rPr` | 20.00 | 20.00 | 20.00 |
| `SIMPLE` — `w:fldSimple`, cached result carries it | **10.00** | 20.00 | **10.00** |
| `COMPLEX` — result run between `separate` and `end` | **10.00** | 20.00 | **10.00** |
| `SIMPLERPR` — `w:fldSimple` with its own `w:rPr` too | **10.00** | 20.00 | **10.00** |
| `NOMERGE` — no `\* MERGEFORMAT` | **10.00** | **10.00** | **10.00** |
| `FIELDRUNRPR` — the `fldChar`/`instrText` runs carry it, result run 10 pt | **20.00** | 10.00 | **20.00** |
| `NOSEPARATOR` — no cached result at all | **20.00** | **20.00** | **20.00** |

**`FIELDRUNRPR` is the arm that earns its keep.** Its cached result states 10 pt and its
instruction run 20 pt, and the reference draws 20 — the opposite sign from every other arm. So
the rule is *the instruction run's properties*, not "the smaller of the two" and not "the
paragraph style always". `SIMPLERPR` is the control that says a `w:fldSimple`'s own `w:rPr` child
does not count either: a compact field states no instruction run and falls all the way back to
the style.

`NOSEPARATOR` was a **separate defect and is now fixed too**: a complex field with no
`w:fldChar w:fldCharType="separate"` has no cached result, the reference computes and draws its
value, and this tree drew nothing at all — a page field's value is written by `PageFields` over a
span of the paragraph's text, so with no result there is no span. `DocxLayoutSource.PlaceHolder`
makes one: a single character in the field's own properties, which pagination replaces *before*
the flow is laid out so that its width never reaches the page. The arm matches the reference in
both modes, unconditionally — with no cached result there is nothing for `\* MERGEFORMAT` to
preserve.

**Its measured reach is nil, and why is the finding.** `census-noseparator.py` counts **10 such
`PAGE` fields in 8 corpus documents**, every one in a running head — and rendering all eight at
the round's base and at its head leaves **8 of 8 byte-identical**. Nine of the ten are Word's
"page number in a frame" template leftover: a paragraph whose whole content is the field, carrying
a `w:framePr` and no other text, which neither engine draws. The tenth reads `Page  of 10` and
sits in a `footer2.xml` that the document never reaches. So the corpus agrees with the reference
on all eight before and after, and the fixture is the only thing that says the rule is right.

## 2. The rule, and why only a recomputed field

`[src]` writerfilter builds a `com.sun.star.text.TextField` and deletes the cached result text
with it, so the value takes the character context in force at the instruction rather than
anything the result states. `[bin]` The table above is that rule, measured, with no free
parameter.

**It is applied only to a field whose value this walk actually writes** — `PageFieldOf` (page and
page-count fields, rewritten at paint time) or `ValueOf` (a constant this walk substitutes:
`FILENAME`, `TITLE`, `STYLEREF`, a `REF`'s bookmark). For anything else the cached text *is* what
gets drawn, and drawing it in anything but its own formatting would be a defect rather than a fix:
the reference recomputes far more fields than this tree does, and a rule about a value's
formatting cannot be borrowed for a value that was never recomputed.

## 3. Reach, and the census that overstates it by 59×

`census.py` counts field results whose `w:rPr` states something that **changes the drawn glyphs**
— a result stating only `w:noProof`, `w:webHidden` or `w:lang` is formatted identically either way
and is not reach. Counting the *element* instead gives 4298 and 215; counting the *property*:

| | fields | documents |
|---|---:|---:|
| without `\* MERGEFORMAT` (moves by default) | **470** | **59** |
| with it (moves only under quirks) | **106** | **29** |

**And the renderings that actually move are one and four.** Rendering all 59 at the round's base
and at its head, under `SOURCE_DATE_EPOCH`, one output directory per document:

- **1 of 59 moves**, the other 58 byte-identical, and **no gate verdict moves in either
  direction** (55 match / 4 pages, before and after). The mover is
  `AW-104D-RVSM-Aircraft-Approval-Checklist.pdf.docx`, whose footer `Page N of M` goes from
  Liberation Sans to Liberation Sans **Bold** — the weight its instruction run states, and the
  weight the reference draws the literal `Page` beside it in.
- Under quirks, isolating this round from everything already behind the switch by rendering the
  base binary with the variable set too: **4 of 29 move, 0 verdicts gained and 0 lost.**

So the census is a bound and not a measurement: in 58 of the 59 documents the cached result's
visible `w:rPr` happens to state what the instruction run or the style states anyway, and the two
rules agree. *Census what a rule paints, not how many elements state it* — the same correction
`probes/cond-format-r96` had to make.

## 4. What the quirks arm costs, on the row it was found on

`CRIF - Spécification technique - Socle applicatif.docx` is where `pages-r164` found this. Its
footer holds two `FILENAME` fields in one cell whose cached results state `w:sz 16` grey against
a `Pieddepage` style stating 10 pt black; drawn at 10 pt black the first is about 30 % wider, the
cell overflows, the footer takes a second line and the body bottom rises.

| | pages | 26.2.4.2 |
|---|---:|---:|
| default (Word) — byte-identical to the round's base | 28 | 29 |
| quirks, at the round's base | 28 | 29 |
| quirks, at its head | **32** | 29 |

So the mechanism works and **the row gets worse**, exactly as `pages-r164` predicted: the footer
was masking three further local events (at reference pages 3/4, 9/10 and 27/28), and correcting it
unmasks them. That is the reason it is behind the switch rather than the reason to leave it
unimplemented — Word agrees with us here, and the three residual events are the work that would
make the reference's rule safe to prefer.
