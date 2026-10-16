# Round 150 — `style:table-centering`: the ODF reader asked for a word ODF does not use

**Reference** `/opt/libreoffice26.2/program/soffice`, LibreOffice **26.2.4.2** `0229ac93…`.
Claims are marked **[src]** (the C++ checkout, 27.2.0.0.alpha0+, *not* the reference binary's
source) or **[bin]** (26.2.4.2's own output). Seat **O100**, diagnosed by the measurement-only
round banked at `probes/invcol-r149/`; this is the fix, its pin and its confinement.

---

## 1. The seat was wrong in both halves, and the correction is the finding

O100 read: *"one column of `084_Service_invoice` is drawn 213 pt left of where 26.2.4.2 draws it"*,
with a second, separate band of 37.7 pt called *"a whole-page constant this document has carried
throughout"*. Neither is a column and neither is a constant. They are **the two printed pages of one
sheet**, each displaced by exactly what horizontal print centring would move it, and the third page
is the `About` sheet, whose page layout is the only one of the three that states no centring — and
which was exact all along.

Predicted from the file with no free parameter (`invcol-r149/centring-arith.py`):

| pdf page | columns | block width | predicted | measured median | n |
|---|---|---:|---:|---:|---:|
| 1 | B C D E | 464.710 pt | **−37.645** | −37.672 | 34 |
| 2 | F | 114.660 pt | **−212.670** | −212.678 | 25 |
| 3 | *states no centring* | — | **0** | +0.000 | 12 |

## 2. The seat in one word

```csharp
string? centring = Get(page, OdfNamespaces.Style, "table-centring");   // ODF: "table-centering"
```

`OdsPrintSetup.cs`:106. This project's convention is British wherever it names something of its own —
`centring`, `CentresHorizontally`, `SheetHorizontalAlignment.Centre` — and the convention leaked one
word too far, onto an attribute name, which is the specification's to spell. `Get` therefore returned
null for **every `.ods` ever read**, so `CentresHorizontally` and `CentresVertically` were never once
true and `SpreadsheetPages.BodyOrigin`, which has implemented the rule correctly throughout
(including round 69's unclamped halving), was never asked.

**[src]**, verified here rather than taken from the brief: `xmloff/source/core/xmltoken.cxx`:1992
interns `TOKEN( "table-centering", XML_TABLE_CENTERING )`, and `PageMasterStyleMap.cxx`:97-98 maps
the **one** attribute onto **both** `PROP_CenterHorizontally` and `PROP_CenterVertically` with
`MID_FLAG_MERGE_ATTRIBUTE`. `ScPrintFunc::PrintPage` (`sc/source/ui/view/printfun.cxx`:2144-2191)
then adds half the slack to the left and top space, **unclamped**.

**[bin]** Censused over the 307 converted `.ods`: **81 documents state `style:table-centering`, 175
occurrences, and `style:table-centring` appears not once.**

## 3. Four one-attribute arms at the reference, and a fixture built from the same source

`make-probe.py` writes four flat-ODF files differing in that one attribute and nothing else: one
4 cm column on a 21 cm page with 2 cm margins, so the printable width is 17 cm, the slack is 13 cm,
and half of it is **6.5 cm = 184.252 pt** — predicted before anything was rendered.

| arm | 26.2.4.2 x | Δx | ours | 26.2.4.2 baseline | Δy | ours |
|---|---:|---:|---:|---:|---:|---:|
| `none` | 57.685 | — | **0.000** | 66.332 | — | **−0.001** |
| `horizontal` | 241.937 | **+184.252** | **0.000** | 66.332 | 0 | −0.001 |
| `vertical` | 57.685 | 0 | **0.000** | 424.177 | **+357.845** | **+0.042** |
| `both` | 241.937 | +184.252 | **0.000** | 424.177 | +357.845 | +0.042 |

The horizontal shift is the predicted 6.5 cm to the thousandth. `reference.txt` and `ours.txt` hold
the four arms; the fixture `tests/corpus/features/sheet-print-centring.fods` is the same factorial as
four sheets with four master pages, so a test gets the whole of it in one rendering and the arms
share every other input by construction. `fixture-reference.txt` is 26.2.4.2's own answer for it.

**The witness settles it the other way round**: re-measured after the fix, the invoice's three bands
`[(−213, 25), (−38, 34), (0, 12)]` collapse to **one band at 0.000** — 71 of 71 spans pair, 61 of
them exactly at 0.0 and the rest within 0.1 pt.

## 4. Two instrument notes, both of which cost time here

**A double hyphen is illegal inside an XML comment, and `soffice` says only "source file could not be
loaded".** The first cut of the probe documented itself with ` -- ` in the leading comment and all
four arms failed to load with no other diagnostic. Use an em dash.

**`pymupdf`'s span bbox top is the INK top and sits the face's ascent above the baseline** — 9.056 pt
here. A test comparing its own drawn origin, which is a baseline, against that number is out by
exactly that much; `readbaseline.py` reads the reference's own `Td` instead and turns it the right
way up. `dotnet/CLAUDE.md` records a round nearly filing a font-metric bug on the same confusion,
and this round reproduced it within the hour.

## 5. Mutation pin

`mutate.sh`, restoring with `cp` + `touch` and never `mv`:

| arm | reverted | tests red |
|---|---|---:|
| M1 | the reader asks for the British spelling again | **4 of 5** |
| M2 | the horizontal arm does not accept the value `both` | 2 |
| M3 | the vertical arm does not accept the value `both` | 2 |

M1 leaves only the `none` control green, which is the shape of the defect: under it every sheet in
the corpus looked like the control, because the reader could not tell the other three from it.

## 6. Confinement: 81 of 1590, and they are exactly the 81 documents that state it

Our half of the whole corpus and of both converted ODF columns rendered twice — once at the round's
base and once with the fix — under `SOURCE_DATE_EPOCH=0`, one output directory per document keyed on
the whole path. **1590 renderings a leg, 0 failures on either**, and every render of each leg
postdates that leg's own build (head 00:52:33–01:17:48 against a 00:52:29 build; base 01:31:51–01:52:08
against 01:31:42).

| family | ext | moved | of |
|---|---|---:|---:|
| **ods column** | ods | **81** | 307 |
| odt column | odt | 0 | 337 |
| sheets | xlsx / xls / xlsm | **0** | 307 |
| slides | pptx / ppt | **0** | 302 |
| words | docx / doc | **0** | 337 |

**81 movers, 1509 byte-identical — and the mover set is the census set exactly**: 0 movers outside
the 81 documents stating `style:table-centering`, and 0 of those 81 failed to move.

## 7. Do they get closer to 26.2.4.2? 71 of 81, and the median falls by a factor of 47

The 81 movers rendered through 26.2.4.2 as well (`ref-movers.sh`), and both legs scored against it by
the median absolute x distance over `difflib`-paired spans (`score.py`, `score.txt`):

| | base | head |
|---|---:|---:|
| median \|dx\| over the 81 | **4.359 pt** | **0.092 pt** |
| documents within 0.1 pt of the reference | 6 | **48** |
| closer / further / level | — | **71 / 4 / 6** |

The largest are outright: `064_Small_business_cash_flow` **242.268 → 0.073**, `057_Simple_balance_sheet`
73.739 → 0.043, `030_Basic_balance_sheet` 65.216 → 0.100, `SLSA_Directory_031423` 59.862 → 0.069 over
1527 spans.

**The four that worsen, named rather than averaged away.** `048_Expense_trends_budget` 0.019 → 0.108 —
this is round 69's own witness for the *unclamped* halving, whose block is wider than the paper so the
correction goes the other way, and 0.108 pt is a tenth of a point; `066_Agile_Gantt_chart` 4.400 → 8.187;
`071_Four-week_project_timeline` 16.207 → 17.119, which was already 16 pt out for some other reason;
and `058_Social_media_engagement_data` 1.659 → 3.813 on nine paired spans, which is too few to weigh.
None of the four is explained here and none is claimed as noise.

## 8. Three instrument failures in this round, all mine

Worth more than the fix, because each produced a confident wrong answer rather than an error.

**`grep -c 'table-centring'` matched the comment that documents the defect.** It was used three times
as the check that the tree was in its base state and returned 1 every time for a tree carrying the
*fix*, because the fix's own remark explains that the British spelling matches nothing. Two "base"
builds were therefore no-ops on an already-fixed tree and 122 renders labelled base were the fix.
**Grep the code line, not the file**: `Get(page, OdfNamespaces.Style, "table-cent`.

**A `git checkout` without a `touch` leaves the source older than the assembly, and MSBuild skips the
project** while reporting `0 Error(s)` in four seconds. `dotnet/CLAUDE.md` records this for `mv`; it is
the same trap and `git checkout` is the likelier way to meet it. The tell is the elapsed time and the
output's own mtime, and both are one `stat` away.

**A shell `until [ -z "$(pgrep -f 'sweep.sh head')" ]` loop matches the other waiters' command lines.**
Nine of them blocked one another for twenty minutes while the leg they were all waiting for had long
finished and the next one never started. Wait on a *marker file* or on the render process itself, never
on a pattern that the waiter's own command line contains.

And one that was the script's: **`set -e` tore `ref-movers.sh` down between building its key table and
rendering anything**, because the table-building loop's last statement was a failing `[ … ] && echo`.
It exited silently, having written a complete-looking table and no PDFs.
