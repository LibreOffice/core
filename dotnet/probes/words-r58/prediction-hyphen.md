# words-r58 — second prediction, committed before the line-breaking change

The 24.2.7.2 audit, `XRay.Office.Text/Layout/LineBreaker.cs`:473, a **shared-layer** site.

## The site's claim, and what 26.2.4.2 actually does

The site says, of `MatchNumber`'s leading-sign clause:

> UAX #14 lets a hyphen open a number here as well, so that "-5" holds together. LibreOffice does
> not … measured against LibreOffice 24.2.7.2, `E-22`, `$-22`, `10-19` and a hyphen that begins its
> own token in `A -222` all break *after* the hyphen … Dropping HY here is therefore the whole of
> the rule.

`audit_hyphenbreak.py`, ten authored packages, each one token **longer than its line** in a column
about six characters wide — so the token has to break somewhere and *where* separates the two rules
with no width tuning at all. Two controls with known answers ran with it.

| token | reference (26.2.4.2) | ours | |
|---|---|---|---|
| `abcd-efghijklmnop` | `abcd-` \| … | `abcd-` \| … | **CONTROL** — an ordinary hyphenated word breaks after its hyphen on both sides, so the column width is not what is being measured |
| `(222222222222222` | `(22222` \| … | `(22222` \| … | **CONTROL** — the site's own negative case, and it holds |
| `E-222222222222` | **`E-2222`** | `E-` | site says "breaks after the hyphen" — **it does not** |
| `$-222222222222` | **`$-2222`** | `$-` | **wrong** |
| `A -222222222222` | **`A` \| `-22222`** | `A-` | breaks at the *space* and then holds the hyphen to the number — **wrong** |
| `abc-222222222222` | **`abc-22`** | `abc-` | **wrong** |
| `-2222222222222` | **`-22222`** | `-` | **wrong** |
| `10-1922222222222` | `10-` | `10-` | right |
| `5-2222222222222` | `5-` | `5-` | right |
| `222-abcdefghijkl` | `222-` | `222-` | right |

**Three of the site's five claims are false on 26.2.4.2, and the code implements the false version.**
One rule accounts for all ten: **a hyphen opens a number, and no break follows it — unless a digit
precedes the hyphen**, which is where LibreOffice's own i#83229 number-range customisation puts the
break back. The site had it exactly inverted: HY belongs in that clause, and `10-19` is the
exception rather than the rule.

## The change

`MatchNumber` admits `HY` beside `OP` in the optional-bracket position, guarded on the preceding
class not being `NU`. One clause, in `XRay.Office.Text`.

## What the census can and cannot see

`hyphcensus.py` counts, in **our own** current renderings, lines that end in a hyphen not preceded
by a digit whose next line begins with a digit — the exact break this change removes.

| track | documents | such line ends |
|---|---:|---:|
| words | 23 | 109 |
| slides | 14 | 28 |
| sheets | 31 | **2 071** |

**The sheets figure is not to be believed and I am saying so before the measurement, not after.**
`pdftotext -layout` reconstructs a spreadsheet row as one line, so "a line ending in a hyphen whose
next line starts with a digit" is satisfied by *the rightmost cell of one row ending in a hyphen and
the leftmost cell of the next beginning with a digit* — two unrelated cells. On flowing text the
proxy is sound; on a grid it is mostly noise. The words and slides numbers are the ones with
meaning, and even they over-count, because a break we remove is only visible if the token then
still fits somewhere else.

It also cannot see: a break this change *adds* (it cannot add any — the change only marks more
positions as unbreakable); a line that gets longer and pushes a word, and therefore a line, and
therefore a page; or a cell whose wrapped height changes and moves a row.

## Predicted movement

| quantity | baseline | predicted |
|---|---:|---|
| words renderings whose bytes change | — | **15 – 30** |
| slides renderings whose bytes change | — | **8 – 20** |
| sheets renderings whose bytes change | — | **3 – 40** |
| **words verdict** | 319 of 337 | **319**, downside risk −1 to −3 |
| **slides verdict** | 200 of 302 | **200**, downside risk −1 to −2 |
| **sheets verdict** | 276 of 307 | **276**, downside risk −1 to −3 |
| the ten probe cases agreeing with the reference | 7 of 10 | **10 of 10** |

**If any track loses a verdict, this commit is reverted and the site is marked `WRONG — reported,
not fixed`,** which is what rounds 55 and 57 did with `SheetPageDecoration.cs` and
`OdpSlideLayout.cs`. A shared-layer line-breaking change that costs a verdict is not worth a
correctly-transcribed rule.
