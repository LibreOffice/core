# N14 is refuted: 26.2.4.2 paints a BIFF8 conditional format, and the measurement that said otherwise was vacuous

## What round 95 concluded

`probes/ods-residue-r95/results.md` §1 closed O4 as nil reach, on two legs:

1. a BIFF8 conditional format cannot move a **row height**, because `ImportExcel8::Read` holds
   its `AdjustRowHeight()` inside an `#if 0` (`sc/source/filter/excel/read.cxx`:1284-1288);
2. *"Reading the record anyway moves nothing, and that is measured"* — the reader was written,
   checked against the reference's `--convert-to ods` range list, and all **64 of 64** `.xls`
   came back byte-identical.

Leg 1 is correct and I have re-read the `#if 0` myself. **Leg 2 is vacuous**, and the entry it
produced — N14 — has to come out of the nil-reach section.

## Why leg 2 measured nothing

The reader that was written parses `CONDFMT`. `CONDFMT` is a count and a `SqRef` — *which
ranges* a conditional format covers. The rules are in the `CF` records that follow it, and the
formatting each rule applies is in the style that rule carries. A reader that reads ranges and
never reads a rule **cannot move a pixel by construction**, so 64 of 64 byte-identical is a
property of the instrument and not a fact about the corpus.

This is the same failure as `probes/clip-textlayer-r93`, where "197 pages, 0 with text outside
the clip" turned out to have enumerated 197 *page-level* clips of median area 1.000; filtering
to real clips found 834, and 733 of them had text outside. Both measurements were true
sentences about the wrong set.

## Correction, round 98: this census was short by a whole document

The table below says **4 of 64** `.xls`, 29 formats, 43 conditions. That is wrong, and it is
wrong for the reason this file exists: **I took round 95's witness list instead of walking the
substreams myself.** Round 98 walked them and found
`EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls` carrying **120 `CONDFMT` and 144 `CF`** —
more than the other four together. I have re-run `--convert-to fods` on it at 26.2.4.2 and
confirmed: **120 conditional formats, 144 conditions.** The corpus total is **149 formats and
187 conditions in 5 of 64, 175 of them inked.**

So this probe caught round 95 reusing an instrument without re-deriving it, and then reused
round 95's census without re-deriving it. The figures below are correct for the four documents
they name and are not the corpus. `probes/biff-reader-r98` supersedes them.

One thing the larger census did **not** change: N11 still holds. Round 98 read all 144 of
EHEST's rules and **not one byte of its rendering moved** — 126 are `cellIs equal 2` over a
blank checklist. A rule count is not a reach figure, and 144 new rules moving nothing is a
sharper demonstration of that than 1215 moving three renderings.

## What the reference actually does

Asked instead of argued. `census.py` converts each of round 95's four witnesses with
26.2.4.2's own `--convert-to fods` and counts what the reference resolves (`census.tsv`):

| document | `calcext:conditional-format` | conditions | named styles | of those, carrying ink |
|---|---:|---:|---:|---:|
| `Background_Declaration_Template.xls` | 23 | 26 | 26 | **26** |
| `NPA_21_21_Sentenced_Comments.xls` | 3 | 9 | 9 | **9** |
| `Hazard Analysis Template.xls` | 1 | 2 | 2 | **2** |
| `TICAPCapability_Final.xls` | 2 | 6 | 6 | **6** |
| | **29** | **43** | **43** | **43** |

"Carrying ink" means the style the condition names declares a `fo:background-color`, an
`fo:color`, or bold. **43 of 43.** There is no witness here whose conditional format resolves
to nothing.

The rules are ordinary ones, well inside what round 96 landed for the OOXML spelling — the
first four of `Background_Declaration_Template`:

```
Excel_CondFormat_1_1_1  ="Yes"                    base Project.C10
Excel_CondFormat_1_1_2  ="No"                     base Project.C10
Excel_CondFormat_1_2_1  formula-is("")            base Project.C38
Excel_CondFormat_1_2_2  formula-is(">0")          base Project.C39
```

`cellIs` and `expression`, both of which `XlsxConditionalStyles` has evaluated since round 94.
So the missing part is the **BIFF reader**, not the predicate evaluation.

## One trap in measuring this, which cost an hour

A condition names its style as `Excel_CondFormat_1_1_1`, and the style that carries the
formatting is declared as

```xml
<style:style style:name="Excel_5f_CondFormat_5f_1_5f_1_5f_1"
             style:display-name="Excel_CondFormat_1_1_1" ...>
  <style:table-cell-properties fo:background-color="#00f..."/>
```

Every `_` in the display name is `_5f_` in the style name. A census that matches on the
unescaped name finds zero styles and reports, cleanly and wrongly, that no conditional style
carries any formatting. `census.py` does the escaping; the first pass of this probe did not,
and printed `styles defined: 0`.

## What this leaves

`Background_Declaration_Template.xls` is O18 in the register at **136.07** mean page ink, and it
holds 23 of the 29 formats. That is a candidate cause for its residual, **not** a demonstrated
one: nobody has yet shown how much of the 136.07 those 1972 cells account for. The honest
statement is that the seat is live again and has a plausible mechanism, and the next round
should measure the ink the fills would move before implementing anything.

The remaining scope is the `CF` record itself — `XclImpCondFormat::ReadCF`
(`sc/source/filter/excel/xicontent.cxx`), the BIFF `dxf`-equivalent inline format it carries,
and how a BIFF operator maps onto the `ScConditionMode` values round 96 already implements.
