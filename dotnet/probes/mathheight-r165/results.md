# mathheight-r165 — how tall a line holding an OMML formula is

**Reference throughout: `/opt/libreoffice26.2/program/soffice`, LibreOffice 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`).** `sweep.sh` prints the resolved path and version
on every run; `/usr/bin/soffice`, 24.2.7.2, was never used. **Nothing was rasterised** — the
whole measurement is `--convert-to fodt`, which prints the reference's own resolved view.

This is the half `probes/blankpage-r163` identified and did not implement: that round proved the
line's height *is* the StarMath object's `svg:height` and measured seven shapes; this one measures
**63** and turns them into a walk.

## Headline

`OfficeMathBox` is the vertical half of `SmNode::Arrange`, in StarMath's own units and with its own
integer divisions. Scored against 26.2.4.2's `svg:height` over the 53 arms whose construct it
implements: **worst |Δ| 0.369 pt, 51 of 53 within 0.25 pt, 18 exact.**

On the corpus it moves **3 renderings and nothing else**, because the code it adds sits inside a
namespace guard that was already there — a document stating no `m:oMath` cannot reach it. Measured
rather than argued: 24 words-track documents chosen at random render **byte-identically** at the
round's base and at its head.

| document | base | head | 26.2.4.2 |
|---|---|---|---|
| `ABCD-FE-01-00 Flight Envelope` | 14 p, 18 420 ch | **15 p, 18 500 ch** | 16 p |
| `ABCD-WB-08-00 Weight and Balance Report` | 12 p, 13 015 ch | 12 p, 13 015 ch | 12 p, 13 015 ch |
| `technical-report-template` | 7 p, 3 860 ch | 7 p, 3 860 ch | 7 p, 3 860 ch |

The two that were already page- and glyph-exact still **move in ink, and both improve**: runs
landing within 0.5 pt of the reference's own baseline go 43 → 47 of 114 on the first and its mean
|Δy| 47.95 → 44.88 pt, and the second's 1.450 → 0.990 pt. Neither worsens.

**`ABCD-FE`'s page 7 — the user's report — is closed.** Per-page alphanumeric counts now run
`325 1621 494 1411 2035 1787 79 1300 1225 1559` against the reference's
`325 1621 494 1411 2031 1783 79 1296 1225 1559`: pages 1 to 10 aligned, **page 7 the near-empty
page on both sides**, where before the fix our page 7 held the reference's page 8. The one
remaining divergence is the reference's 11/12/13 (`903 1020 448`) against our 11/12
(`1580 781`) — the gust-load table it keeps whole and we split, which `blankpage-r163` §6 already
filed as a separate defect, and the last three pages align again.

## 1. What the reference does, and the two laws `[bin]`

LibreOffice does not lay an OMML formula out as text. `oox/source/mathml` parses the subtree,
StarMath builds a document from it, and Writer embeds the result as an OLE object anchored
as-character — so the line takes the **object's** height, which `--convert-to fodt` prints as the
`draw:frame`'s `svg:height`.

1. **The line's height is exactly that attribute.** Over ten shapes, `ref pitch − svg:height` is a
   constant **37.45 ± 0.02 pt**, which is the two plain paragraphs either side of it
   (`blankpage-r163` §4).
2. **`w:sz` never reaches it.** StarMath lays out at *its own* base size — 12 pt,
   `SmFormat::SmFormat`, `starmath/source/format.cxx`:25. `x` at 8, 12 and 20 pt all come back
   **13.349 pt**, and `x` with a subscript **15.134 pt** at both 12 and 20.

This tree did the opposite in both directions at once: it reserved **11.90 pt for every shape** at
the default size — a nested fraction cost exactly what a bare `x` did, and the box was *shorter
than the ink drawn into it* — and it moved with `w:sz`.

## 2. The fixtures

`build.py` writes one DOCX per construct, copying `word/styles.xml`, `word/settings.xml`,
`word/fontTable.xml`, `word/numbering.xml` and `word/theme/theme1.xml` **verbatim from
`ABCD-FE-01-00 Flight Envelope`**, so each inherits the same OOXML compatibility defaults — the
trap `office-corpus/SKILL.md` names, where a hand-built DOCX with no settings part answers a
different question. 63 fixtures: nine plain rows, six script shapes, four radicals, nine
fractions, four n-ary, four delimiter, six matrix and stack, seven accent/limit/box, four size
controls and four sequences.

`sweep.sh` converts all 63 in one `soffice` call (about 40 s); `read-heights.py` prints the table
(`heights.txt`).

## 3. The model, and where each constant comes from

`model.py` is the Python prototype and
`dotnet/src/XRay.Office.Ooxml/OfficeMath/OfficeMathBox.cs` is the port; they agree arm for arm.

Everything is **hundredths of a millimetre, integer** — `SmO3tlLengthUnit` — because the divisions
are the reference's own. Writing it in floating point loses the ±1 unit that separates
`frac-nest-num` (61) from `frac-nest3` (62) on the *same* rule.

**Two measured constants, and only two:**

| | value | how |
|---|---:|---|
| a plain row's height at the 12 pt base | **471** | nine one-run fixtures — a letter, a capital, a digit, `abc`, `a+b`, two Greek letters with descenders, two runs — all 471 |
| its ascent | **376** | read off the `sup` arm, which pins it and nothing else does |

Liberation Serif's own `hhea` predicts 468 and 377 for those, so the height is three units out.
Taking the measured value keeps the whole ladder on the reference's number instead of three units
under it at every level.

**Everything else is `SmFormat`'s own default** (`format.cxx`:31-60) or a rule read out of
`node.cxx`/`rect.cxx`:

| construct | rule | source |
|---|---|---|
| script | 60 % size, hung from the base's `AlignT`/`AlignB`, offset `H×20/100`, clamped at the line 40 % of the way up | `SmSubSupNode::Arrange`, node.cxx:1143 |
| fraction | `num + rule + den`, rule = `H×5/100 + 2×(H/20)` | `SmBinVerNode::Arrange`, node.cxx:831; `SmRectangleNode::Arrange`, :1799 |
| radical | sign is `body − halfDescender`, plus a tenth; foot that half-descender above the body's | `SmRootNode::Arrange`, :725; `lcl_GetHeightVerOffset` |
| radical degree | foot 52 % down the sign | `lcl_GetExtraPos` |
| delimiter | bracket is `body + 2×(body×5/100)`, centred | `SmBraceNode::Arrange`, :1256 |
| stack (`eqArr`, a barless fraction) | rows `H×5/100` apart | `SmTableNode::Arrange`, :491 |
| matrix | rows `3H×3/100` apart | `SmMatrixNode::Arrange`, :1941 |
| n-ary, limits | limits at 60 %, no gap | `SmOperNode::Arrange`, :1515 |

**The `SmRect` algebra is modelled and that is not over-modelling.** A fraction has **no
baseline** — `ExtendBy(…, RectCopyMBL::None)` clears it, rect.cxx:457 — so a subscript on a
fraction is hung from `AlignB` rather than from a baseline, and `sub-of-frac` sits 4 pt wrong if
the baseline stands in for it. Four fences per rect is the smallest thing that gets that arm right.

## 4. Residuals (`residuals.txt`)

51 of 53 within 0.25 pt. The two that are not are both **nested radicals** — `rad-rad` 0.369 pt,
`rad-frac` 0.283 pt — where the sign's own glyph box is approximated by the height `AdaptToY` asks
for rather than measured from OpenSymbol. Closing them needs the font, and 0.37 pt is an order of
magnitude below the 8.25 pt this round is spending.

## 5. What is deliberately **not** asserted

`leaf-paren`, `func`, `mixed`, the four n-ary arms, `limlow`, `limupp`, `groupchr` and
`delim-nest` are measured in `heights.txt` and left out of the test ladder. Each holds a bracket
or an operator glyph **inside a text run**, which StarMath re-parses into a `SmBraceNode` or a
`SmMathSymbolNode` and this walk reads as ordinary text — `(x)` in one `m:t` comes back 14.515 pt
against a plain row's 13.349. Modelling that means parsing the run's text as StarMath source, and
the corpus does not need it.

**What the corpus does state**, counted over the six documents that hold any OMML at all:
`m:sSub` 156, `m:f` 55, `m:rad` 10, `m:d` 10, `m:sSup` 6, `m:func` 1 — and **no n-ary, matrix,
accent or limit anywhere**. The six arms that carry the corpus are all in the ladder.

## 6. Reach

Six documents in 947 hold OMML — `ABCD-FE-01-00` (87 `m:oMath`), `ABCD-WB-08-00` (70),
`WiGr_2021W_1_Angebot-Nachfrage-Elastizität` (67, a deck), `technical-report-template` (16),
`RPA P4 - Advanced Material` (3, a deck) and `Structural Testing` (1, a deck).

**The three decks are not closed by this round.** `OfficeMathBox` is in `XRay.Office.Ooxml` so that
`XRay.Office.Presentations` can reach it, but `PptxTextBody` does not call it yet and a slide's
autofit measures its own text; that is the owed half.
