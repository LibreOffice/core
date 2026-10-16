# Round 159 — several `w:num` over one `w:abstractNumId` are one list

Reference binary: `/opt/libreoffice26.2/program/soffice`, **LibreOffice 26.2.4.2**
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`). Corpus `/home/user/sample-files`, 271 DOCX.
Every rendering under `SOURCE_DATE_EPOCH=0`. This is **O109**, seated by round 158 from the same
page that produced O108.

## The rule, read twice

**[src] The list a paragraph counts in belongs to the `w:abstractNumId`, not to the `w:numId`.**
`AbstractListDef::MapListId` (`sw/source/writerfilter/dmapper/NumberingManager.cxx`:405-412) is
three lines — it records the first id it is given and returns that same one ever after — and it is
a member of the **abstract** definition. `DomainMapper_Impl::finishParagraph`
(`DomainMapper_Impl.cxx`:2962-2976) calls it with each numbered paragraph's own Writer list id and
writes the answer back onto the paragraph, so every instance over one abstract ends up on one
list. A reader keyed on the `w:numId` runs them as separate lists.

**[src] A `w:startOverride` is a restart of that shared list, and it fires once per instance.**
The same function (`:2986-2997`) reads `pListLevel->GetStartOverride()` for the paragraph's own
level and, while the id is absent from `m_aListOverrideApplied`, sets `ParaIsNumberingRestart` and
`NumberingStartValue` on the paragraph and inserts the id. The set is keyed on the `w:numId` alone
— its own comment says *"we can do this only once on first occurrence of list with override"* — so
an instance that has restarted at one level cannot restart at another.

**[bin] Seven arms, 32 labels, measured before a line was written.**
`make-probe.py` builds `tests/corpus/features/words-list-instance-share.docx`, one arm per group of
paragraphs, each paragraph carrying its own label in its text so the number beside it can be
attributed rather than guessed. `reference-arms.txt` is 26.2.4.2's own rendering.

| arm | shape | 26.2.4.2 | this tree, after |
|---|---|---|---|
| A | two instances over one abstract, neither overriding | 1 2 **3 4** | same |
| B | the first overrides, the second inherits — *the witness's shape* | 1 2 **3 4** | same |
| C | the second overrides | 1 2 **1 2** | same |
| D | the second overrides at 5 | 1 2 **5 6** | same |
| E | the overriding instance used twice | 1 2 1 2 **3 4 5 6** | same |
| F | **control**: two instances over two abstracts | 1 2 **1 2** | same |
| G | the style route — paragraphs with no `w:numPr` | 1 2 **3 4** | same |

Arm F is what stops the rule degenerating into one counter per document; arm E is
`m_aListOverrideApplied`; arm D separates a restart from a level's own `w:start`; arm C is the half
that a "share the counter" change gets wrong on its own, because the counter already exists by the
time the overriding instance is first used.

## What changed

`WordNumbering`'s counters are keyed on `ListOf(numId)` — the instance's `w:abstractNumId`, or the
instance itself when the file declares none — rather than on the instance. `Advance` consults
`TakeStartOverride`, which yields an instance's `w:startOverride` **once** and then never again, in
place of the old `StartOf`, which consulted it only where no counter existed and so could not
restart a running list. `ResetCounters` clears the applied set with the counters, because resetting
one without the other would leave a restarting instance taking its level's plain `w:start` the
first time a new flow reached it.

The `w:numStyleLink` indirection is deliberately **not** followed when keying: the id Writer keys on
belongs to the abstract definition the instance names, and `FollowStyleLink` answers a different
question (where a level's *definition* comes from).

## Reach and confinement

`census.py`: **14 of 271 corpus DOCX** have an abstract definition carrying more than one *used*
instance — `SPA-02_mcar_part-2_and_IS_v2.9.docx` and its revision lead with **326** instances over
one abstract, the two FAA documents have 106 and 94.

Rendering all 271 with the binary under test against round 158's bank: **262 byte-identical, 9
moved, 0 failures** (`confine-rows.tsv`). Scored against the 26.2.4.2 bank (`movers.txt`):

| document | reference | before | after |
|---|---|---|---|
| `SPA-11_mcar_part-11_v2.9` | 49p 70 763 | 49p 70 937 | 49p **70 763** |
| `Sample_SQMS_Program` | 61p 86 680 | 61p 86 734 | 61p **86 680** |
| `SPA-06_mcar_part-6_and_IS_v2.9` | 85p 142 938 | 85p 143 116 | 85p **142 938** |
| `SPA-02_mcar_part-2_and_IS_v2.9` | 266p 479 154 | **268**p 485 953 | **266**p 479 214 |
| `FRE-03_mcar_part-3_and_IS_v2.9` | 76p 124 706 | 76p 125 011 | 76p 124 737 |
| `02_mcar_part-2_and_IS_v2.10` | 312p 414 481 | 314p 421 243 | 313p 414 531 |
| `OM template for non-complex NCC operators` | 165p 270 009 | 166p 270 077 | 166p 270 063 |
| `FAA 2025-26 Holdover Tables` | 167p 335 603 | 167p 336 847 | 167p **335 623** |
| `24-25_FAA_Holdover_Tables` | 155p 299 583 | 155p 300 664 | **154**p 299 532 |

**Three land on the reference's character count exactly**, and every one of the nine is closer than
it was. `SPA-02` gains its gate verdict — `pages` → `match`, 268 pages to 266 and 479 214 against
479 154. The witness's distance goes **0.37 % → 0.01 %**.

### The one verdict lost is a reveal, and the arithmetic says so

`24-25_FAA_Holdover_Tables` goes `match` → `pages`, at 154 against 155 — while its character
distance goes from **1081 to 51**. It was passing on two errors that cancelled. Take the pages the
reference renders near-empty and compare the three lists:

```
reference   16  73  95 98 100 102 104 106 108 110 112      147 150 152 154
before      16      94 97  99 101 103 105 107 109 111  118 147 150 152 154
after       16      94 97  99 101 103 105 107 109 111      146 149 151 153
```

The reference's near-empty page **73** — its header, its footer and nothing else, between
`TABLE 50` and `TABLE 50 (CONT'D)` — is missing from both legs, which is why both are one page
behind from there on. *Before*, a **spurious** near-empty page at 118 put the count back: the
reference has no near-empty page anywhere between 112 and 147. *After*, that spurious page is gone
and only the genuine gap remains, so the offset runs to the end and the total is one short.

That is the same shape this project has recorded once before on the sibling document, and it is
strictly better: one real defect instead of two compensating ones. The remaining one is **not** this
round's rule and not O108's — on page 72 our rows sit a constant ~6 pt above the reference's by the
foot of the page, so the table splits a row later and the reference's empty continuation page never
appears. Seated as **O110**.

## What the numbering change does not touch

`WordNumbering` has four consumers and all four are the DOCX reader (`DocxContentReader`,
`DocxFile`, `DocxLayoutSource` and its `.Lists` half), so the ODF, RTF and WW8 readers are
untouched by construction — and the sweep confirms it, with the whole `.doc` half of the words
track byte-identical.
