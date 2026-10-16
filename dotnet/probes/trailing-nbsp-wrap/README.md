# The two extra line pitches on `150-5370-10H.docx` are a NO-BREAK space, and the rule is not yet known

Measured 2026-08-16 against LibreOffice **26.2.4.2** with the full font set, on the two-block slice
`probes/trailing-space-wrap/` cuts out of
`sample-files/words/pagination-003/docx/150-5370-10H.docx` (blocks 3633/3634). Every row below is a
fresh build and a fresh `soffice` conversion.

## What was wrong with the previous account

Task #70 recorded this as *"a paragraph ending in a trailing space costs LibreOffice two lines and us
none"*, and warned that fixing it would move text on *"most of any real corpus"*.

**The trailing characters are not two spaces. They are `U+00A0` then `U+0020`** — a no-break space
followed by an ordinary blank. Two rounds read them off a terminal, where the two are
indistinguishable, and generalised from the wrong character.

That matters twice over. It explains the part the earlier round recorded as unexplained — *why the
**preceding** word is pushed down with the blank* — because a no-break space is precisely a thing
that forbids a break between itself and what precedes it. And it shrinks the blast radius from
"every paragraph ending in a space" to **7 documents and 24 paragraphs in the whole 140-document
OOXML words corpus**: `150-5370-10H` (10), `195584360` (5),
`2015-April-SWIM_Users_Forum-Q&A` (3), `docs-quality-MA.IMS.00001` (2),
`f445896eb008d14c1746fc37d412dc22` (2), `19-06 Assistive Technology TAB Final` (1), `33004` (1).

## What the tail does, on this paragraph

`NB` is `U+00A0`, `SP` is `U+0020`. The paragraph is 4 lines for us in every row.

| paragraph ends with | ref text lines | gap to next paragraph | extra pitches |
|---|---:|---:|---:|
| `lanes.` `NB` `SP` — **the real document** | 5 | 28.3 | **+2** |
| `lanes.` `NB` `SP` `SP` | 5 | 28.3 | **+2** |
| `lanes.` `SP` `NB` `SP` | 4 | **40.9** | **+2** |
| `lanes.` `NB` | 4 | 15.6 | 0 |
| `lanes.` `NB` `X` | 4 | 15.6 | 0 |
| `lanes.` `NB` `SP` `X` | 4 | 15.6 | 0 |
| `lanes.` `SP` `NB` | 4 | 15.6 | 0 |
| `lanes.` `SP` `SP` `SP` | 4 | 15.6 | 0 |
| `lanes.` | 4 | 15.6 | 0 |

Rows 1 and 3 together are what identify the no-break space as the binding agent: both cost two
pitches, but row 3 puts a break opportunity *before* the `NB`, so nothing visible is dragged and both
extra pitches show up as gap instead of as a fifth line.

Trailing **ordinary** blanks cost nothing, however many. Trimming 6 or 12 characters out of the body
moves the break but keeps the +2.

## The adjustment split, which names the C++ site

Same tail, varying the paragraph's `w:jc`:

| `w:jc` | text lines | gap |
|---|---:|---:|
| absent / `left` / `center` / `right` | 5 | 28.3 |
| **`both`** | 5 | **15.7** |

So the effect is two independent halves: **the word is pushed onto its own line under every
adjustment**, and **the further empty line appears under every adjustment except block**.

That split is exactly the gate in `SwTextGuess::maybeAdjustPositionsForBlockAdjust`
(`sw/source/core/text/guess.cxx:78-130`), whose own comment says it returns false *"to create a
trailing `SwHolePortion`"*: block adjustment takes one path and everything else needs
`MS_WORD_COMP_TRAILING_BLANKS`, which a DOCX sets. Beside it, `IsBlank` (`guess.cxx:47`) is
documented as *"UAX #14: spaces from SP and BA classes (elided in the end of a line)"* and covers
`CH_BLANK`, `CH_FULL_BLANK` and `CH_SIX_PER_EM` — **and not `U+00A0`**. Our `TrimTrailingSpaces`
(`TextMeasurer.cs:846`) already agrees on the no-break space and already elides only `U+0020`; it
does *not* elide `U+3000` or `U+2006`, which is a separate and unrelated narrowing.

## The rule is unconditional — and a false refutation of it is the trap worth carrying

Replacing the paragraph's whole text with synthetic words and sweeping its length — 6 to 32 words,
which is one, two, three and four lines — keeps the effect at **every** length. `NB SP` always costs
one more line than `SP` alone *and* a 28.3 pt gap against 15.6, and its last line always holds just
the final word (right edge 108.4 pt every time):

| synthetic words | `…wordNN.` `NB` `SP` | `…wordNN.` `SP` |
|---:|---|---|
| 6, 8, 10 | 2 lines, gap 28.3 | 1 line, gap 15.7 |
| 12 | 2 lines, gap 28.3 | 2 lines, gap 15.6 |
| 14 … 22 | 3 lines, gap 28.3 | 2 lines, gap 15.6 |
| 24 | 3 lines, gap 28.3 | 3 lines, gap 15.7 |
| 26 … 32 | 4 lines, gap 28.3 | 3 lines, gap 15.7 |

A one-line paragraph gains it too — `Hello world.` is 1 line and gap 15.7, and
`Hello world.` `NB` `SP` is **2 lines and gap 28.3**. So the cost is not width-dependent, does not
need the paragraph to wrap, and does not need anything from the real document's body.

**The rule, then:** a paragraph whose text ends with `U+00A0` followed by one or more ordinary
blanks and nothing else costs **two extra line pitches** — one under block adjustment. Anything
after the blanks removes it; a `U+00A0` with no blank after it removes it; blanks with no `U+00A0`
before them cost nothing however many there are.

### The false refutation, which is the part to carry forward

This section said the opposite for an hour. An ad-hoc version of the sweep showed `NB SP` and `SP`
identical at all fourteen lengths, which reads as a clean refutation of a rule derived from a single
paragraph — the most persuasive shape a wrong result can take, because it is the control failing.

It was measuring nothing. The `NB` constant in that script was typed into a shell heredoc and
arrived as an ordinary space, so both arms of the sweep built **the same document**. Confirmed by
dumping the bytes afterwards: the ad-hoc builds end `'.  '` and this script's end `'.\xa0 '`.

That is the same trap that produced the original wrong account of this bug two rounds ago, hit a
second time, in the script written to guard against it. Hence the rule at the top of this file, and
hence section 1 printing `encode('unicode_escape')` for every row it measures:

**a probe that cannot show you the bytes it wrote is not evidence about the bytes you meant.**

## Reproducing

```sh
export XRAY_OFFICE_CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 trailing-nbsp-wrap.py
```

**Dump the bytes of what a mutation actually wrote** — `text.encode('unicode_escape')` — before
believing any row. A case literal typed into a heredoc silently carried a `U+00A0`, and for one
round that looked exactly like the reference being nondeterministic. The reference is not: the
baseline rendered five times gives the same line count every time.
