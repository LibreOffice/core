# Sheets round thirty-five — probe data

Base commit `5add4e1e7`, checked with `git log --oneline -1` in the worktree before anything was
measured. Reference renderer LibreOffice 24.2.7.2. Both whole-track sweeps ran against a
checksummed copy of the CLI whose assemblies were diffed against the tree's before the run
started, and the copy was proved to hold the change by re-running the probe whose row heights it
moves.

| file | what it is |
|---|---|
| `base-whole-track.tsv` | whole-track sweep at `5add4e1e7`, before any change |
| `solidus-whole-track.tsv` | the same on the final tree |
| `reach-*.tsv` | the byte-level reach of the change over all three tracks, clock pinned |
| `PREDICTION.md` | the reach predicted while the sweep was still running |
| `make-prefix-probe.py` | a workbook of prefixes of one string in one wrapping column |
| `make-strings-probe.py` | the same shape, one cell per line of a text file |
| `s137c21.txt` | the cell that costs `tk-syllabus-comparison-document-v5.xlsx` its three pages |
| `tokens.txt` | the twelve cases that separated the rule from every alternative |
| `rowdiff.py` | our resolved row heights against LibreOffice's own `style:row-height`, per sheet, per row |

## The baseline reproduces the brief exactly

```
153/171 matches   abs page error 77   exact page counts 159   abs word error 32630
001 10/10  002 10/10  003 10/10  004 10/10  005 10/10  006 10/10  007 10/10  008 10/10
009 9/9    010 8/10   011 9/10   012 9/10   013 8/10   014 9/10  015 6/9   016 6/9
017 6/10   018 3/4
```

All four figures are the brief's to the digit, which is the first time on this track that has
happened. 171 rows, no duplicate path, no `ref-failed`, no `ours-failed`.

## Cause (c) is not a rounding. It is LibreOffice's own line-break rule for paths

The brief's reading was a ±1 line difference in the 96 dpi measurement, erring in both directions
on one document, "which on this project has meant a rounding or a threshold three times running".
It reproduced exactly — 19 of `tk-syllabus`'s 6520 rows differ from LibreOffice's own
`style:row-height`, 17 of them −224 twips and 2 of them +224 — **and it is neither a rounding nor
a threshold.**

### How it was localised

The brief is right that the reference PDF cannot arbitrate a reserved height, so nothing was
measured against ink until the mechanism was already named. Instead:

1. **A prefix probe.** `make-prefix-probe.py` builds a workbook whose rows hold successively
   longer prefixes of one string, in one column of exactly the width the real document gives it
   (2301 twips, reproduced by `GridProbe` before the probe was used). LibreOffice's own
   `style:row-height` per row then states how many lines *it* breaks each prefix into, and
   `rowdiff.py` says where the two first disagree. Both sides break the string into 1, 2, 3, 4 and
   5 lines at the same four character counts, and part company at **107** characters: we go to six
   lines, LibreOffice stays at five until 118.
2. **Then, and only then, the ink.** With the divergence pinned to a fifteen-character window, a
   twelve-cell probe rendered to PDF says what LibreOffice actually does with those characters.
   That is safe where reading a whole document's ink is not, because the question has become "where
   does it put the break" rather than "how tall is the row".

### The rule

`BreakIterator_Unicode::getLineBreak`, `i18npool/source/breakiterator/breakiterator_unicode.cxx`
:541-560, i#17155:

```cpp
// Special case for Slash U+002F SOLIDUS in URI and path names.
// TR14 defines that as SY: Symbols Allowing Break After (A).
// This is unwanted in paths, see also i#17155
if (lbr.breakIndex > 0 && Text[lbr.breakIndex-1] == '/')
{
    const sal_Int32 nOverlyLong = 66;
    sal_Int32 nPos = lbr.breakIndex - 1;
    while (nPos > 0 && lbr.breakIndex - nPos < nOverlyLong)
        if (u_isWhitespace(Text.iterateCodePoints( &nPos, -1 ))) { lbr.breakIndex = nPos + 1; break; }
}
```

UAX #14 allows a break after a solidus. LibreOffice, having chosen one, looks **backwards** up to
66 characters for whitespace and moves the break to just after it — pulling the whole path onto
the next line instead of splitting it. And when that lands on the line's own start, EditEngine
throws the break iterator's answer away and cuts at the fitting limit —
`if (nBreakPos <= rLine.GetStart()) { nBreakPos = nMaxBreakPos; }`, "No separator in line =>
Chop!", `editeng/source/editeng/impedit3.cxx:2236-2247`.

**That is why one rule errs in both directions.** Glued, a line ends earlier than UAX #14 would
end it and the cell grows a line. Chopped, it ends *later* — mid-number, past the solidus — and
the cell loses one.

### The twelve cases that separate it from every alternative

Read out of LibreOffice's own PDF, one cell per row, one column 2301 twips wide, Calibri 10 pt.
The first column is the cell's text; the second is where LibreOffice put the first break.

| cell | first line |
|---|---|
| `CAT.IDE.A.170/CAT.IDE.H.170` | `CAT.IDE.A.170/` — nothing behind it, so the solidus stands |
| `CAT.IDE.A.170/CAT.IDE.H.170;` | `CAT.IDE.A.170/` |
| `CAT.IDE.A.170/CAT.IDE.H.170; CA` | `CAT.IDE.A.170/` |
| `AMC1 CAT.IDE.A.170/CAT.IDE.H.170; CA` | `AMC1 ` — then 25 characters ending `…CAT.IDE.H.1` |
| `zz CAT.IDE.A.170/CAT.IDE.H.170; CA` | `zz ` — then the same 25-character cut |
| `abcd/efghijklmnop/qrstuvwxyz/ABCDEFGH` | `abcd/efghijklmnop/` — no blank anywhere, solidus stands |
| `abcd.efghijklmnop.qrstuvwxyz.ABCDEFGH` | 25 characters — a full stop between letters is no opportunity at all |

The fourth and fifth rows are the finding in one line: **the text after the blank is character for
character the same as the first row's, and the breaks are not.** No rounding can do that, and
neither can a threshold on a width.

### It is not a break-opportunity rule and cannot be written as one

The glue jumps *past* any other opportunity standing between the blank and the solidus, so
deleting the solidus from the opportunity set would break at an intervening hyphen instead. It
therefore lives in `TextMeasurer`'s fill loop rather than in `LineBreaker`, beside the existing
three LibreOffice-specific departures from UAX #14 rather than among them.

### What it fixes, against the only oracle that can judge it

Rows disagreeing with LibreOffice's own `style:row-height`:

| | before | after |
|---|---:|---:|
| the 276-row prefix probe | 137 of 276 | **0** |
| the twelve-cell token probe | — | **0 of 12** |
| `tk-syllabus-comparison-document-v5.xlsx` | 19 of 6520 | **1** |

### One narrower leg is deliberately not reproduced

When the solidus break falls **exactly** on the last character that fits, LibreOffice refuses it
outright and takes the previous opportunity (`breakiterator_unicode.cxx:494-496`: the line-boundary
branch is skipped when `Text[nStartPos - 1] == '/'`). Deciding that needs the fitting limit
measured to the character, where the two renderers agree only to within a rounding — so
reproducing it would convert a sub-twip disagreement into a whole line, in either direction. It
is stated in the code's remarks rather than implemented.

## Reach

`PREDICTION.md` was committed while the sweep was still running. It predicted **10–25 of 171
sheets documents changing and 1–3 verdicts moving**, and 3–15 documents on each of words and
slides.

See `reach-*.tsv` for the measured answer and the round's report for the comparison. The
prediction's *shape* — that the rule needs a narrow measure, so sheets is where it bites — is
worth keeping separate from its magnitude, which is what a census cannot give: nearly every
workbook in the corpus holds a solidus, and the ones whose rendering changes are the ones where a
line break happens to land on one.

## The residue this leaves, named

- **`seihon_zassi_kikou_20221215.xlsx` is a CJK substitution, not a break rule.** With rows 0–4000
  already exact, extending `rowdiff.py` to the sheet's full 5159 finds **121 rows differing, all of
  them −268.8 twips** — one line of 18 device pixels. The cells name `游ゴシック` (Yu Gothic), which
  is not installed; `fc-match` answers `DejaVu Sans`, which has no CJK coverage at all, so a
  29-character Japanese title that LibreOffice breaks onto two lines stays on one for us. That is a
  font-fallback question and a different round's.
- **`tk-syllabus`'s last row** is the same signature: `Reader Instructions` row 5, −269.2 twips.
- **`sectors-defense-and-aerospace.xlsx`** matches on pages, 227/227, and the human review's
  "some empty cells missing their shading" is **2 of 227 pages** by the image diff, and on both of
  them the band the reference shades and we do not sits directly above a band we shade and it does
  not. That is a row displaced within the page, not a fill that was never drawn.
