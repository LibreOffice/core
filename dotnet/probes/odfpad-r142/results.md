# Round 142 — O99: the two padding directives an ODF number format does not spell out

**Measured against LibreOffice 26.2.4.2, build `0229ac93fcf0d7cbc6376066c6f35021cef002dc`**, with
the tarball's duplicate, Noto Latin, Narrow and Condensed faces moved aside. Both sweep legs are
pinned with `SOURCE_DATE_EPOCH=0`; the before leg is round 141's after leg, rendered from the same
committed tree.

## 1. `_x` has no ODF spelling at all, and `*x` is an element

`_x` means *leave the width of `x` blank* and ODF states no directive for it. LibreOffice's
exporter writes the **spaces** into a `number:text` and records what they stood for in
`loext:blank-width-char`, a sequence of `<char>[<position>]` groups separated by `_` whose
positions are counted in the *unquoted* text. `*x` — *repeat `x` until the column is full* —
becomes a `number:fill-character` element holding the character.

`OdfNumberFormat` read neither. `fill-character` reached no branch of `Append` and fell through to
`default:`; the blank-width attribute was never looked at, so each blank became the single literal
space the element carries. An accounting format's currency symbol therefore ran into its digits
and its columns did not line up.

A third attribute in the same family was found while measuring and is fixed with them:
**`loext:max-blank-integer-digits`**, the count of integer digits written as `?` rather than `0`.
`xmlnumfi.cxx`:681-684 reads it, `:801-802` raises the minimum integer width to at least that
count, and `:1828-1844` replaces the first *N* zeros of the integer part with `?` — which is what
makes an accounting format's zero row `-??` rather than `-00`.

Three details decide the rewrite and all three are in the reference:

* **`lcl_InsertBlankWidthChars`** (`xmlnumfi.cxx`:879-926) is positional, and an approximation of
  it lands the directive in the wrong place. It is followed exactly here, down to leaving `i`
  un-advanced when a position runs to the end of the spec — a quirk that would re-read a
  two-digit position's second digit as the next group's character, and which no corpus value
  reaches.
* **A literal carrying a blank-width spec is always quoted.** `lcl_EnquoteIfNecessary`'s
  don't-quote branch is guarded on the spec being empty (`:539`, tdf#170670), and the insertion
  works on positions inside a quoted string.
* **How many characters a blank replaces is the character's own width**, from
  `SvNumberformat::InsertBlanks` and its `cCharWidths` table
  (`svl/source/numbers/zformat.cxx`:71-105). That matters: `€` is above ASCII and is **two**.

## 2. The instrument states the answer

`soffice --convert-to html` writes each cell's format as `sdnum="<language>;<system>;<code>"`, and
the third field is the string `SvNumberFormatter` holds. `reference-sdnum.tsv` banks it for the
fixture and for a corpus witness. After this round **every one of them is reproduced character for
character**:

| format | 26.2.4.2's own `sdnum`, and ours |
|---|---|
| ASCII accounting | `[>0]_(* #,##0_);[<0]_(* (#,##0);_(* "-"_);_(@_)` |
| euro accounting | `[>0]_-* #,##0.00" "_€_-;[<0]-* #,##0.00" "_€_-;_-* -??" "_€_-;_-@_-` |
| fill, no blanks | `* #,##0` |
| neither | `0.00` |

The euro arm is the one that separates a faithful implementation from a plausible one: two spaces
for `€`, and a multi-group spec with positions (`€1_-3`) over four spaces. A reader that removes
one character per group, or that ignores the positions, puts the directive in the wrong place and
the drawn characters do not show it.

## 3. The fixture

`features/sheet-odf-numfmt-padding.ods`, built by `make-fixture.py`, which writes an `.xlsx` and
converts it with 26.2.4.2 — the committed file is the exporter's, because the exporter is what is
under test. Four columns, four rows.

## 4. Reach and confinement

Censused over the 307 converted `.ods`, by the data style a cell actually names:
**122 549 cells in 55 documents state a `blank-width-char`** and **102 680 in 29 a
`fill-character`**.

Rendering the column again against round 141's after leg: **27 renderings move, 280
byte-identical, 0 failed either leg, and no page count moves.**

**The confinement control is exact**: every one of the 27 movers states at least one of the three
attributes, and no document that states none moved. (The reverse is not a control — LibreOffice
writes its built-in currency list into every file, so 280 *non*-movers state one somewhere too.
This is the same trap as round 141's `style:map` census, in a different attribute.)

Position, measured as the mean |Δx| of matched spans **about each document's own median**, because
several of these sit a whole margin out from the reference for reasons no round has touched and a
raw mean measures that constant instead of the change:

**16 better, 2 worse, 9 same; summed spread 120.84 → 108.79.** Five land essentially exactly —
`Global_Market_Forecast_2016-2035_Airbus_Data_Set` 4.877 → **0.019**, `airports_6` 1.573 →
**0.048**, `ecopy of 2016 statistical bulletin` 1.497 → **0.025**, `redac-sas-201503` 2.242 →
0.248, `ECA Sinters` 3.220 → 0.514. Alphanumeric distance is flat, 579 → 577, which is what a
padding change should do.

### The one real regression is a column in the wrong place, and the round makes that visible

`084_Service_invoice` goes 49.19 → 67.99 on that measure, and the histogram says why
(`invoice-bands.py`, which bands each span's distance from the reference instead of averaging it):

```
before  [(-213, 7), (-143, 4), (-136, 1), (-124, 12), (-38, 31), (0, 12), (32, 2), (39, 1)]
after   [(-213, 25),                                  (-38, 34), (0, 12)]
```

**Eight scattered offsets collapse into three.** −38 is the whole-page constant this document has
had throughout, 0 is exact, and −213 is one column that is in the wrong place. The formatting is
now right and what is left is a single geometry error rather than per-cell noise — the mean rises
only because the misplaced group grew from 7 spans to 25. Seated as **O100**.

## 5. Pinned by mutation

| mutation | of 36 |
|---|---|
| `fill-character` not read | **4 fail** |
| the blank-width spec ignored | **3 fail** |
| `max-blank-integer-digits` not read | **1 fail** |

Each was applied against the restored fix and rebuilt; restored with `cp` + `touch` and rebuilt,
36 of 36 pass and no `MUTATION` marker survives in the source.

## 6. Suite

| project | passed | failed | skipped |
|---|---:|---:|---:|
| Core | 591 | 0 | 0 |
| Containers | 109 | 0 | 0 |
| Text | 750 | 0 | 0 |
| Vector | 309 | 0 | 0 |
| Rendering | 164 | 0 | 0 |
| Markup | 259 | 0 | 0 |
| OpenDocument | 194 | 0 | 0 |
| WordProcessing | 2010 | 0 | 0 |
| Spreadsheets | 1416 | 0 | 0 |
| Presentations | 1205 | 0 | 0 |
| Fidelity | 542 | **10** | 0 |

The ten fidelity failures are the documented set, unchanged. **0 skipped on Fidelity.**

## 7. Files

| file | what it is |
|---|---|
| `make-fixture.py` | writes the `.xlsx` and converts it with 26.2.4.2; the `.ods` is the fixture |
| `reference-sdnum.tsv` | 26.2.4.2's own assembled code for the fixture and for the invoice |
| `movers.tsv` | the 27 movers: pages, alphanumerics, the constant offset and the spread about it |
| `invoice-bands.py` | bands one document's span distances instead of averaging them |
| `fingerprints-before.txt`, `fingerprints-after.txt` | md5 of all 307 renderings, each leg |
| `par-sweep.sh` | round 141's sweep, unchanged, kept so this round's legs are reproducible |
