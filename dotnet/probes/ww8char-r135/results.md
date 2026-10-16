# Round 135 — a WW8 toggle is relative to one style, and an attribute still open where a contents field begins is thrown away

Both seats are in the WW8 character layer, both are on the same two documents, and both are
**fixed**. The second one's mechanism was a candidate when this round opened; it is now established
by a six-arm series at the reference binary, and the rule it establishes is narrower and more
general at once than the seat described — it is not about underlines and it is not about fields.

| | |
|---|---|
| worktree | `/home/user/wt-ww8char`, branch `agent/ww8char`, base `9a90bd663`, 2026-09-15 |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**, `0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| reference renderings | the **banked** leg of gate r133, `/home/user/gate-r133/ref`, 947 PDFs |
| our base leg | built in this worktree at `9a90bd663`, saved as `/home/user/r135-base-cli` |
| C++ tree | `/home/user/libreoffice-core`, read only. **C8**: it declares `27.2.0.0.alpha0+` and is a bulk import, so it is not 26.2's source — every arm below is confirmed a second time against 26.2.4.2's own output, and §2's arms rest on that leg alone |
| C11 | no arm rests on a per-document reference delta of a few characters. §1's witness is 523 glyphs of one face against 0, §2's is a 92 pt rule present or absent; neither is inside the reference's run-to-run instability |

**Headline.**

| | before | after | 26.2.4.2 |
|---|--:|--:|--:|
| O86 — `150_5335_5a.doc` page 3, glyphs in `LiberationSerif-Bold` | **0** | **521** | **523** |
| the same page in `LiberationSerif` | 4408 | 3860 | 3863 |
| O86 — the whole document, bold glyphs | 924 | **4224** | 4244 |
| O87 — `361400CSLegislation1RF01PUBLIC1.doc`, rule cover on its contents page | **238 pt** | **146 pt** | **146 pt** |
| Σ\|ours − reference\| of rule cover, the 10 contents pages of the 5 `.doc` that have one | **677 pt** | **585 pt** | — |
| summed \|ink\|% over every `.doc` rendering that moves | **24.14**, 8 MAJOR | **21.84**, 7 MAJOR | — |
| `150_5335_5a.doc` alone | 19.52, 6 MAJOR | **17.26**, 5 MAJOR | — |
| `.doc` renderings that move | — | **7 of 66** | — |
| `.docx` renderings that move | — | **0 of 272** | — |
| page counts and alphanumeric counts that move, either track | — | **0 of 338** | — |

---

## 0. The two rules, in one paragraph each

**O86.** A WW8 toggle sprm's `0x80` and `0x81` operands — *as the style* and *against the style* —
are relative to **one style's own resolved value**, and that style is the one the same CHPX names.
Never the accumulated value the run is being layered onto, and never the paragraph style when a
character style is named. `150_5335_5a.doc`'s contents entries name `Hyperlink`, which states no
weight, so `sprmCFBold 0x81` over it is **bold**; this tree measured the same operand against the
`TOC 3` paragraph style, which *is* bold, and drew them regular.

**O87.** A character attribute still open where a `TOC` or `INDEX` field begins is not drawn.
`Read_F_Tox` moves the insertion point backwards into the index section it has just inserted while
every attribute of that CHPX is still on the control stack, so each closes in a node it did not
open in and the range is never set. It is **not** specific to the underline the seat named — a
colour sprm in the same run is lost too — and it is **not** a property of fields in general: only
the two types that reach `Read_F_Tox` do it.

---

## 1. O86 — the rule, and the confound ruled out before anything else

### The rule, read out of the sprm handler

`SwWW8ImplReader::Read_BoldUsw` (`sw/source/filter/ww8/ww8par6.cxx`:3117-3156, the tree named
above) is the handler for `sprmCFBold`, `sprmCFItalic`, `sprmCFStrike`, `sprmCFOutline`,
`sprmCFShadow`, `sprmCFSmallCaps`, `sprmCFCaps`, `sprmCFVanish` and the out-of-sequence
`sprmCFDStrike`. It opens at the paragraph style and then **replaces it outright**:

```cpp
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(m_nCurrentColl);
    if (m_xPlcxMan && eVersion > ww::eWW2)
    {
        SprmResult aCharIstd = m_xPlcxMan->GetChpPLCF()->HasSprm(... NS_sprm::CIstd::val);
        if (aCharIstd.pSprm && aCharIstd.nRemainingData >= 2)
            pSI = GetStyle(SVBT16ToUInt16(aCharIstd.pSprm));
    }
    ...
        if( *pData & 0x80 )
        {
            if (pSI && pSI->m_n81Flags & nMask) bOn = !bOn;
```

Four things in that hunk decide the shape of the fix and each was worth reading before writing
anything:

- **`m_n81Flags` is that style's chain walked from nothing.** It is seeded from the base style —
  `rSI.m_n81Flags = pj->m_n81Flags` (`ww8par2.cxx`:3825), and only when the two styles are the
  same kind — and then set or cleared by each toggle sprm in the style's own CHPX, in the
  `m_pCurrentColl` branch of the same function. So a style that states **no** weight answers *not
  bold*, not "whatever the paragraph resolved to". That is the whole of the defect.
- **The test is whether the sprm is present, not whether its value is non-zero.** A CHPX naming
  istd 0 hands `GetStyle(0)` — *Normal* — to the toggle, where our style *layer* correctly skips
  zero because `Read_CColl` ignores every paragraph style (`ww8par6.cxx`:4145-4148). The two
  questions are separate and this tree had one function answering both.
- **`Read_BoldUsw` does not ask whether the style was applied.** It has no `m_bLoadingTOXCache`
  test, so the toggle is resolved against `Hyperlink` on exactly the runs where round 130's O81
  fix declines to apply `Hyperlink`. O86 is uncovered by O81 rather than caused by it.
- **The style's own toggles are settled when the style is read**, against its base chain alone, so
  layering a character style's chain over the paragraph's resolved value is the same defect one
  level up. §1.4 measures that half separately; its corpus reach is nil.

### Confirmed a second time, against 26.2.4.2's own output — and L1/O57 ruled out

A **bold** difference is where a face-resolution confound can masquerade as an attribute one, so
this was settled before the rendering was looked at. `--convert-to fodt` of `150_5335_5a.doc`
states the weight as an *attribute*, with no font lookup anywhere in the answer:

| | 26.2.4.2's own view |
|---|---|
| `Contents_20_3`, the paragraph style it rebuilds the entries in | **no `fo:font-weight` at all** |
| `T4`, the span on `a.` and on `Definition of ACN` | **`fo:font-weight="bold"`** |
| `T7`, the span on the tab between them | **`fo:font-weight="normal"`** |
| `T8`, the span on the tab before the page number | **`fo:font-weight="normal"`** |

So the bold is not a face the reference fell back to, and it is not inherited from the paragraph
style — the reference's own model states `bold` on those runs and `normal` on the two beside them.
L1 and O57 are about a family class surviving into a measurement and being dropped before the
drawing; nothing here reads a family at all.

**The document is its own control, in one paragraph.** `TocProbe` on the file gives the runs of the
`Definition of ACN` entry (paragraph style 30, `TOC 3`, whose CHPX is `350881 …` over a `Normal`
that is not bold, so `TOC 3` resolves to bold):

| cp | CHPX | names a style? | 26.2.4.2 draws | this tree drew |
|---|---|---|---|---|
| 1744 `a.` | `304A1A00 350881` | **istd 26, `Hyperlink`** | **bold** (`T4`) | regular |
| 1746 tab | `350881 504A0000` | no | normal (`T7`) | regular |
| 1747 `Definition of ACN` | `304A1A00 350881` | **istd 26** | **bold** (`T4`) | regular |
| 1764 tab | `110881 350881` | no | normal (`T8`) | regular |

Same paragraph, same paragraph style, the same `sprmCFBold 0x81` on all four — and the reference's
answer alternates with the presence of `sprmCIstd`. Under the old reading all four resolve against
`TOC 3`'s bold and come out regular, which is what the rendering showed. `Hyperlink` is style 26,
kind 2, base 10 (`Default Paragraph Font`, empty CHPX), and its own CHPX is `3E2A01 422A02
70680000FF00` — an underline and a blue, **no weight** — so its `m_n81Flags` bold bit is clear.

### The fix

Three pieces, all in `XRay.Office.WordProcessing/Ww8`:

- `ApplyLayoutSprms` takes an optional `toggleBase`. Where one is given, the seven
  `Read_BoldUsw` attributes resolve their `0x80`/`0x81` operands against **it** rather than against
  the value being layered onto; where none is, the accumulated value stands in, which is exactly
  what a style chain's own sprms resolve against. The base is captured at the grpprl's entry, so a
  CHPX stating one toggle twice resolves both against the style, as `Read_BoldUsw` does.
- `StyleToggleFormat(istd)` is `m_n81Flags`: the character chain of one style applied to nothing,
  memoised per style index because the walk asks once per character.
- `ApplyCharacterException` chooses the base — `CharacterStyleNamedIn(exception)` if the CHPX
  carries a `sprmCIstd` at all, the paragraph style otherwise — and passes it to the direct sprms
  **whether or not the style layer was applied**, which is what keeps O81 and O86 from interfering.
  `WithStatedToggles` then replaces the toggles the style chain decided with the chain's own answer,
  because those were settled at definition time too.

The same rule is on the content path (`ResolveCharacterFormat`), whose fields are not nullable, so
"does this chain decide the toggle" is answered by resolving it twice — from all-clear and from
all-set — and taking the chain's answer only where the two agree.

### Reach, with its base rate

`CharProbe --census --toggles` over all **66** corpus `.doc`, from the file rather than from a
rendering, one row per document, counting distinct CHPX entries over **every story** — body,
footnotes, headers, annotations, endnotes and both text-box stories (`toggle-census.tsv`):

| | documents | CHPX | characters |
|---|--:|--:|--:|
| CHPX carrying a toggle stated relative to a style **and** a `sprmCIstd` | **15 of 66** | **580** | **6822** |
| …of which the two readings **disagree** | **2** | **472** | **3665** |
| **base rate**: the same toggles with **no** `sprmCIstd`, where the two readings must agree | **65 of 66** | **12 804** | **176 109** |
| CHPX naming a style whose own chain states a relative toggle | 7 | 41 | — |
| …of which the chain's two readings disagree | **0** | **0** | — |

So the population on which the change can fire at all is **4.3 %** of the toggles, and the part of
it where the two readings actually differ is **3.6 %** — 472 CHPX, of which `150_5335_5a.doc` holds
468 and `361400CSLegislation1RF01PUBLIC1.doc` the other 4. The base rate is the rest of the very
same sprm in 65 of the 66 documents, and it does not move.

**Round 130's figure was 14 documents, 583 CHPX and 6765 characters, and this is not a
contradiction.** That census counted `0x0854`, `0x0858`, `0x085A`, `0x085C` and `0x085D` — the
BiDi and complex-script toggles — and not `sprmCFDStrike` (`0x2A53`), and it counted `istd != 0`
rather than "the sprm is present". Both differences are named in §5.

---

## 2. O87 — established at the reference, in six arms, and it is not what the seat said

Round 130 left this as *"the mechanism is a candidate and is NOT confirmed"*, with a one-attribute
variant series on a hand-built `.doc` as the thing that would settle it, and authoring a `.doc` as
the part it could not afford. **It does not need one.** A corpus `.doc` can be changed one field at
a time in place: the `PlcFld` names a field's type in one byte, a CHPX FKP names a run's end in one
`rgfc` entry, and a grpprl's sprm can be swapped for another of the same length. Each variant is
read back through 26.2.4.2's own `--convert-to fodt`, which states the answer as an attribute
rather than as ink.

`361400CSLegislation1RF01PUBLIC1.doc`'s `Table of Contents` heading is the CHPX at cp 141-158,
fc 2189-2207, `1668E3156C00 3E2A01` — `sprmCRsidText` and `sprmCKul` 1. Its last character is the
paragraph mark; the next character, cp 159 at fc 2207, is the `TOC` field's `U+0013`, whose
`PlcFld` entry is at table offset 14301 and says 13. The paragraph is `Block Text`, style 21.

| arm | one field changed | 26.2.4.2's `P14` |
|---|---|---|
| **as authored** | — | **no underline** |
| A | `flt` 13 → **9**, a type `aWW8FieldTab` has no handler for | `style:text-underline-style="solid"` |
| B | the CHPX FKP's `rgfc` entry **2207 → 2206**, so the run closes at the paragraph mark | `style:text-underline-style="solid"` |
| C | `3E2A01` → `422A06`, `sprmCKul` 1 replaced by `sprmCIco` 6 (red) | **no colour** |
| D | arm C's colour **and** arm A's field type | `fo:color="#ff0000"` |
| E | `flt` 13 → **8** (`INDEX`, the other `Read_F_Tox` slot) | **no underline** |
| F | `flt` 13 → **3** (`REF`) | `style:text-underline-style="solid"` |
| G | `flt` 13 → **88** (`HYPERLINK`) | `style:text-underline-style="solid"` |

Four things follow, and only the first was in the seat:

- **The `TOC` field is the cause** (A), and **the boundary is the field's begin marker** (B) — one
  byte earlier and the attribute survives with the field still in place.
- **It is the CHPX that goes, not the underline** (C and D). So the rule is about the control
  stack, not about one attribute, and D is the control that says the patched bytes are read at all.
- **It is the two `Read_F_Tox` types and not fields in general** (E against F and G). That is
  exactly `IsIndexField`'s pair, which this reader already had for O81.
- **The paragraph style's own character half survives**: `Block_20_Text` keeps its
  `fo:font-weight`, because a style's items are on the node's format and not on the stack.

The candidate mechanism is therefore **corroborated and not isolated**: arm B says the loss turns
on where the attribute closes relative to the field's begin, which is what
`m_oPosAfterTOC.emplace(*m_pPaM, m_pPaM); (*m_pPaM).Move(fnMoveBackward);`
(`ww8par5.cxx`:3531-3533) would cause, and arms E/F/G say only `Read_F_Tox` does it. Nothing here
discriminates between that line and another statement of the same function, and this round did not
try to — the behaviour and its exact trigger are what a reader needs.

### The fix, and why it is reproduced rather than declined

`Ww8DocumentReader.IndexFieldStarts` collects every `TOC`/`INDEX` begin in the story being walked,
`StrippedExceptionRanges` turns each into the file-offset range of the CHPX run that ends exactly
there, and the three places that resolve a run treat such a run's exception as empty. It is a
defect in the reference and it is reproduced deliberately, exactly as `Read_CColl`'s `Hyperlink`
suppression is: the project's target is 26.2.4.2's output, and the alternative is 92 pt of rule
this tree draws and the reference does not, on the page the previous round left as O81's entire
residue.

### Reach, with its base rate

`CharProbe --fields` over the same 66 `.doc` (`field-census.tsv`):

| | |
|---|--:|
| `.doc` carrying a `TOC` or `INDEX` field | **4 of 66** |
| of those, with a CHPX run ending **exactly** at the field's begin | **3** |
| of those three, with a drawable sprm in that run | **3** (18, 1 and 1 characters) |
| **base rate**: CHPX runs ending exactly at **any other** field's begin, which the rule must not touch | **1526 runs in 38 of 66 documents**, 1294 of them drawable |
| `.doc` with no index field, which cannot move | 62 of 66 |

`150_5335_5a.doc` is the fourth TOC-bearing document and is **not** reachable: its run does not end
at the field's begin, which is why O81's residue there was O86 and not this. Of the three that are
reachable, **two move a rendering** and `CP-ETSO template_v2020.DOC` does not — its one affected
character draws the same either way.

The base rate is the measurement that matters here, because arms F and G say the reference keeps
every one of those 1526: a rule keyed on "a field" rather than on the two `Read_F_Tox` types would
strip 1294 drawable runs in 38 documents instead of three in three.

---

## 3. Before and after, on the named documents

### Contents-page rule cover, over the whole `.doc` track

`toc-cover.py` is round 130's, unchanged, so these are comparable with `ww8toc-r130/results.md`
column for column (`toc-cover-base.tsv`, `toc-cover-final.tsv`):

| document | contents pages | ours, base | ours, after | 26.2.4.2 |
|---|--:|--:|--:|--:|
| `361400CSLegislation1RF01PUBLIC1.doc` | 1 | **238 pt** | **146 pt** | **146 pt** |
| `150_5335_5a.doc` | 3 | 0 | 0 | 0 |
| *control* `A320SimNotes.doc` | 2 | 1698 | 1698 | 1698 |
| *control* `absrc-pac-01-info-note-en.doc` | 1 | 1911 | 1911 | 2458 |
| *control* `150_5300_13_chg10.doc` | 3 | 0 | 0 | 38 |

Σ|ours − reference| over those ten pages: **677 → 585 pt**, and the 92 round 130 left is nil. The
585 is `absrc-pac-01`'s 547 and `chg10`'s 38, neither of which is either of these seats.

### Faces drawn, the witness

`faces.py`, glyph counts per face over the whole document (`faces-o86.tsv`):

| document | face | base | O86 | 26.2.4.2 |
|---|---|--:|--:|--:|
| `150_5335_5a.doc` | `LiberationSerif` | 118 747 | 115 389 | 116 519 |
| `150_5335_5a.doc` | `LiberationSerif-Bold` | **924** | **4224** | **4244** |
| `361400CSLegislation1RF01PUBLIC1.doc` | `LiberationSerif` | 77 863 | 77 859 | 78 694 |
| `361400CSLegislation1RF01PUBLIC1.doc` | `LiberationSerif-Bold` | 3421 | 3425 | 3427 |

On page 3 alone, where round 130 measured the gap: ours goes from 4408 regular and **0** bold to
3860 and **521**, against the reference's 3863 and 523. Two glyphs of the 523 are still drawn
regular by this tree and five of the page's characters are not drawn at all; neither was chased.

### `|ink|%` against 26.2.4.2, over every `.doc` rendering that moves

`ink.py` over `pdf-image-diff.py` at 512 px, against the banked r133 reference
(`ink-doc-base.tsv`, `ink-doc-o86.tsv`, `ink-doc-final.tsv`):

| document | base | O86 only | O86 + O87 |
|---|--:|--:|--:|
| `150_5335_5a.doc` | 19.52, 6 MAJOR | **17.26, 5 MAJOR** | 17.26, 5 MAJOR |
| `361400CSLegislation1RF01PUBLIC1.doc` | 1.12, 1 MAJOR | 1.11 | **1.08, 1 MAJOR** |
| `644730BRI0mna000BOX361539B00public0.doc` | 3.45, 1 MAJOR | 3.45 | 3.45 |
| `1447.doc` | 0.05 | 0.05 | 0.05 |
| `150_5300_13_chg10.doc` | *not scoreable — page counts differ* | | |
| `150_5300_13_chg8.doc` | *not scoreable — 18 pages ours against 17* | | |
| `absrc-pac-01-info-note-en.doc` | *not scoreable — page counts differ* | | |
| **total** | **24.14, 8 MAJOR** | 21.87, 7 MAJOR | **21.84, 7 MAJOR** |

O87's 92 pt of rule is worth **0.04** of `|ink|%` on a 35-page document, which is the honest size of
it: a 91.33 pt line 0.6 pt thick is a large fraction of a contents page's *rule cover* and a small
fraction of its ink. The rule-cover column in §3.1 is the instrument that sees it.

The three documents that are not scoreable on ink are so because `pdf-image-diff.py` refuses a pair
whose page counts differ; two of them were already in that state at the base, and this round did
not change any page count anywhere.

---

## 4. Confinement — what moves and what does not

This is the measurement the round turns on, because a toggle-resolution change touches how *every*
bold, italic, caps, small-caps, strike and hidden sprm in every `.doc` resolves. Our half of the
words track rendered twice, at `9a90bd663` and at the head, one output directory per **document**,
`SOURCE_DATE_EPOCH` pinned on both legs (`sweep-doc.py`, `confine.py`; 66 of 66 and 272 of 272 on
each leg, 0 failures):

| | `.doc` | `.docx` | words track |
|---|--:|--:|--:|
| renderings that move | **7 of 66** | **0 of 272** | **7 of 338** |
| byte-identical | 59 | **272** | 331 |
| page counts that move | **0** | 0 | **0** |
| alphanumeric counts that move | **0** | 0 | **0** |

**The `.docx` column is 0 of 272 and that is the confinement statement**, not an absence of
evidence: the two files changed are both in `XRay.Office.WordProcessing/Ww8`, and `git grep` says the
only type any other reader borrows from that folder is `Ww8DateTime`. Slides and sheets cannot move
for the same structural reason and were not swept.

**The converted ODF columns were not swept and do not need to be.** Round 130 had to sweep them
because its third change was in a shared layer; this round's is not. `.odt` and `.rtf` read their
own readers, which cannot reach `Ww8DocumentReader`.

### Every mover attributed

| document | what moved | direction against 26.2.4.2 |
|---|---|---|
| `150_5335_5a.doc` | 3300 glyphs change weight | `|ink|%` **19.52 → 17.26**, MAJOR 6 → 5 |
| `361400CSLegislation1RF01PUBLIC1.doc` | 4 glyphs change weight (O86); one 91.33 pt underline goes (O87) | rule cover **238 → 146** against 146; `|ink|%` 1.12 → 1.08 |
| `absrc-pac-01-info-note-en.doc` | O87 strips a one-character run before its `TOC`, and the contents list moves down 1.10 pt | mean \|Δy\| over matched lines **167.96 → 167.50 pt** |
| `1447.doc` | an empty paragraph's height, 0.10 pt | mean \|Δy\| **0.5376 → 0.5095 pt** |
| `644730BRI0…public0.doc` | the same, 0.10 pt | mean \|Δy\| **53.7254 → 53.7199 pt** |
| `150_5300_13_chg8.doc` | the same | mean \|Δy\| 62.1942 → 62.1950 pt — level |
| `150_5300_13_chg10.doc` | the same | not separately scored |

**Nothing is materially worse.** The four small movers are the *empty-paragraph carry-over*'s
toggle base and are attributed exactly rather than guessed: a variant binary differing only in
whether that one call passes a base renders all four **byte-identical to the base leg**
(`§5`, refutation 3). Text, faces and drawings are identical on all four; what changes is one
empty paragraph's height by 0.10 pt, and the shift carries down the page.

---

## 5. What is left of `150_5335_5a.doc`, and it is not a character question

Round 130 said O86 was *"the largest identified part"* of that document's 19.52 and **had not shown
it was all of it**. It is not: O86 is **2.26 of the 19.52**, and the remaining **17.26 over 64
pages with 5 MAJOR pages** is a table.

Measured rather than described. Pairing our lines against the reference's by text, page by page
(64 pages on both sides, 3439 lines ours, 264 of them unmatched):

- **58 of the 64 pages are vertically exact** — median per-page Δy is **0.00 pt**, and only six
  pages exceed 1 pt.
- Those six are **pages 44 to 47 and two neighbours**, where our content sits **124 to 131 pt
  higher** than the reference's.
- Pages 43-45 carry table rules on both sides, and **we fit more rows per page than the reference
  does**: 123 text lines on page 44 against 112, 61 on page 46 against 50. Our rows are *shorter*.
- Every region `pdf-image-diff.py` names on the five MAJOR pages is `marks displaced or reshaped` —
  a placement difference, not ink we miss or invent. The largest single page is 1.47, so nothing is
  concentrated: 5.31 of the 17.26 is on the five MAJOR pages and the other 11.95 is spread over 60
  pages at a mean of 0.20.

**A row that is short by half a border, accumulating down a table until a page holds one row too
many, is O84**, whose own measurement is *eleven rows short by 5.25 pt in total* on a different
document; **O85's own witness is this document's page 39**; and **O83** is the same family. So the
residue of `150_5335_5a.doc` is already seated three times over on the table side, and no new seat
is taken for it. What this round adds is that it is *not* a character-attribute question and not a
contents-list question: after O86 the contents pages are 0 pt of rule against the reference's 0,
and the document's bold count is 4224 against 4244.

**No blind reading was asked for**, and the reason is that the residue stopped being unexplained:
the ask in the brief was conditional on there being a large unattributed difference with no
hypothesis, and a 124 pt row-placement drift inside a table with three open seats on it is neither.

---

## 6. Refutations and corrections of this round's own findings

1. **The first form of the toggle census over-counted the disagreement by a factor of four, and the
   error was in the model of the *old* reading, not of the new one.** It compared the paragraph
   style's own flags against the named style's, and reported *4 documents, 480 CHPX, 3753
   characters*. But this tree never used the paragraph style's flags as the base: it layered the
   character style's chain over the paragraph's resolved value and took *that*. Where the named
   style states the toggle at all the two are the same, so the first census counted every such CHPX
   as a disagreement. `absrc-pac-01-info-note-en.doc`'s `Strong` (`sprmCFBold 0x81` over an empty
   `Default Paragraph Font`) is the case that exposed it — two CHPX counted as disagreeing, and the
   document's rendering byte-identical. Corrected, the disagreement is **2 documents, 472 CHPX,
   3665 characters**, and every disagreeing document moves a rendering. `f111.doc`'s five, also in
   the first figure, are all inside a `HYPERLINK` field's instruction or on a field marker and draw
   nothing either way.
2. **A body-only census misses the other seven stories**, and this one did at first: 13 documents
   and 578 CHPX over `fib.TextLength` against **15 and 580** over every story the piece table
   addresses. It did not change the disagreement count here, but a WW8 census keyed on `ccpText`
   alone is a census of the body and should say so.
3. **The empty-paragraph carry-over's toggle base cannot be decided by this corpus, and the round
   does not claim to have decided it.** `EmptyParagraphCharacterLayout` models a WW8 attribute
   still open across a paragraph mark; under the new rule the toggle base for that inherited CHPX
   has to be a *style*, and two readings are defensible — the empty paragraph's own paragraph style,
   or the previous paragraph's, which is what `m_nCurrentColl` held when the reference read the
   sprm. Both were built and rendered: **byte-identical on all four affected documents.** The
   simpler one ships. What is *not* defensible is the old answer — the accumulated value including
   the previous run's own sprms — and dropping the base entirely restores the base leg's bytes on
   all four, which is how the four were attributed at all.
4. **Round 130's O86 reach figure is superseded rather than wrong.** Its 14 documents / 583 CHPX /
   6765 characters counted the BiDi and complex-script toggles (`0x0858`, `0x085A` and the three
   beside them) and not `sprmCFDStrike`, and treated a CHPX naming istd 0 as naming none. This
   round's 15 / 580 / 6822 counts the nine sprms `Read_BoldUsw` actually dispatches, over every
   story, with presence as the test.
5. **`Read_BoldBiDiUsw` follows the identical rule through `m_n81BiDiFlags`, is not implemented,
   and has NIL REACH.** `sprmCFBoldBi` and `sprmCFItalicBi` (`ww8par6.cxx`:3254-3320) read the same
   `sprmCIstd` and invert against the base style's BiDi flags in the same way, and neither sprm is
   in `ApplyLayoutSprms` at all — so the defect there is that the attribute is unread rather than
   mis-resolved. Censused (`bidi-census.tsv`, the same walk over every story of all 66 `.doc`):
   **0 CHPX state either, with or without a style, in 0 of 66 documents**, against a base rate of
   **12 804 western toggles in 65 of the 66**. It is left, and the census is banked so it is not
   rediscovered.
6. **The strike and double-strike toggles share one flag in this model and are two bits in
   `m_n81Flags`.** `Ww8LayoutFormat.IsStruckThrough` is set by both `sprmCFStrike` (bit 2) and
   `sprmCFDStrike` (bit 8), so a style stating only the doubled form answers the base for the
   single one too. No corpus `.doc` states a relative `sprmCFDStrike` at all, so it costs nothing
   here; it is recorded because it is the one place the model is knowingly coarser than the rule.
7. **O87's content-path twin is not implemented.** The strip is on the layout path only, so
   `xray-office extract` still honours a `sprmCFVanish` in a run the reference throws away. No corpus
   `.doc` has one — the three affected runs carry an underline, a colour and a font — so the reach
   is nil today, and the asymmetry between the two paths is recorded rather than closed.

---

## 7. Tests

| file | cases | what it pins |
|---|--:|---|
| `Ww8ToggleBaseTests` (new) | 6 | O86: a toggle's base is the style it is given, in both operands, for every sprm in one grpprl; and with no base the accumulated value still stands in |
| `Ww8IndexFieldBoundaryTests` (new) | 5 | O87: which field types are boundaries, that it is the begin marker, nesting, and a story's own field base |

Both are unit files and say so in their own remarks: the corpus holds the only `.doc` that exercise
either rule and the repository holds no fixture with a styled toggle or a decorated run before a
contents field. Each is pinned by **mutation** instead of by failing at the base, and both mutations
fail exactly the tests they should and nothing else:

| mutation | result |
|---|---|
| `ApplyLayoutSprms`'s `toggleBase ?? format` → `format` | **4 failed** of 2007, all `Ww8ToggleBaseTests` |
| `IndexFieldStarts`' `IsIndexField(...)` test removed | **1 failed** of 2007, `OnlyTheTwoFieldTypesThatReachReadFToxAreIndexFieldStarts` |

Whole suite, discovered against passed per project (`test-run.txt`):

| project | discovered | passed | failed |
|---|--:|--:|--:|
| `XRay.Office.Containers.Tests` | 109 | 109 | 0 |
| `XRay.Office.Core.Tests` | 591 | 591 | 0 |
| `XRay.Office.Markup.Tests` | 249 | 259 | 0 |
| `XRay.Office.OpenDocument.Tests` | 169 | 169 | 0 |
| `XRay.Office.Presentations.Tests` | 1205 | 1205 | 0 |
| `XRay.Office.Rendering.Tests` | 164 | 164 | 0 |
| `XRay.Office.Spreadsheets.Tests` | 1387 | 1387 | 0 |
| `XRay.Office.Text.Tests` | 750 | 750 | 0 |
| `XRay.Office.Vector.Tests` | 309 | 309 | 0 |
| `XRay.Office.WordProcessing.Tests` | 2007 | 2007 | 0 |
| `XRay.Office.Fidelity.Tests` | 552 | 542 | **10** |

**0 skipped everywhere**, so `XRay.Office.Fidelity.Tests` really did reach LibreOffice. The ten
failures are the same ten by name that round 130 recorded — the four `paginated.*`
`PageDrawingComparisonTests`, the four `list-label-overrun.*` `TabStopComparisonTests`,
`justify-shrink-2013.docx` and `sheet-rich-text.xlsx` — the family `CLAUDE.md` leaves failing on
purpose. Nothing new fails and nothing previously failing passes.

`Markup` discovers 249 and runs 259 because `--list-tests` counts a theory once; the same pair is in
round 130's record. `Text` and `WordProcessing` are 750 and 2007 against round 130's 744 and 1988 —
six and nineteen cases added by the rounds merged since.

---

## 8. The scripts and the banked data

| file | what it is |
|---|---|
| `CharProbe/` | the two censuses, reading the file rather than a rendering. `--toggles` is §1.4, `--fields` is §2.3; `CHARPROBE_WHY=1` describes each disagreeing CHPX on stderr |
| `toggle-census.tsv` | one row per `.doc`: CHPX with a toggle and a style, how many disagree, and the base rate |
| `field-census.tsv` | one row per `.doc`: index fields, runs ending at one, and the same for every other field type |
| `bidi-census.tsv` | the same walk asking for `sprmCFBoldBi`/`sprmCFItalicBi` instead — 0 of 66, §6.5 |
| `sweep-doc.py` | round 130's, unchanged — one output directory per document, `SOURCE_DATE_EPOCH` pinned |
| `confine.py` | what moved between two legs, and whether any page or alphanumeric count did |
| `doc-o86.tsv`, `doc-final.tsv`, `docx-final.tsv` | its output for the three comparisons |
| `ink.py` | round 130's, unchanged |
| `ink-doc-base.tsv`, `ink-doc-o86.tsv`, `ink-doc-final.tsv` | summed \|ink\|% per mover at the three legs |
| `faces.py`, `faces-o86.tsv` | glyphs per font face, ours twice against the reference — the instrument for a weight change, which no gate column can see |
| `toc-cover-base.tsv`, `toc-cover-final.tsv` | round 130's `toc-cover.py` over the whole `.doc` track |
| `variants.md` | the six one-field variants of §2, with the byte offsets each was made at |
| `test-run.txt` | discovered against passed, per project |

The variant `.doc` themselves are **not** committed: they are patched copies of a corpus document
and `variants.md` states the offset and the value for each, which is smaller and reproducible.
