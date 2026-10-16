# The ODF and RTF readers, measured for the first time

## Why there was nothing to measure

`MANIFEST.tsv`'s `ext` column holds **docx 272, pptx 251, xlsx 241, doc 66, xls 64, ppt 51,
xlsm 2 — and nothing else.** There is no ODF, no RTF and no template in the sample corpus at
all, so `XRay.Office.OpenDocument` and the RTF reader, both named in `dotnet/CLAUDE.md`'s
Scope, had never been measured against a real document. Every fidelity figure this project
has ever quoted for them comes from hand-built fixtures.

## What was done

`.claude/skills/office-corpus/scripts/convert-corpus.sh` fanned all 947 corpus documents
out into the ODF spelling of their own family, and the words track additionally into RTF,
through **26.2.4.2**, mirroring the corpus layout so `batch-check.sh` globs it unchanged:
**1285 files, zero conversion failures.**

**What this measures, and what it does not.** Each file is *LibreOffice's own export*, so
both renderers read the identical file and a divergence is genuinely ours. It is **not**
evidence about reading ODF written by anything else, and it cannot be: the corpus holds no
third-party ODF to test that with.

Flat ODF (`.fodt`/`.fods`/`.fodp`) is deliberately absent — a flat export of a 700-page
document runs to tens of megabytes and this container's writable allowance is finite.

## Environment

    ours   = XRay.Office.Cli @ ddb05e4e5
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2 -- the calibration target,
             with PATH pointed at its program directory so batch-check.sh picked it up
    fonts  = system fontconfig; three of the tarball's four confounds moved aside.
             The fourth, LiberationSansNarrow, was NOT moved -- see CLAUDE.md. 49 corpus
             documents name a Narrow family and their rows here are suspect.
    rule   = batch-check.sh of 2026-09-05: page count, then max(2%, 15) alphanumeric characters
    rows   = 1266 of 1285; the sweep died before writing its summary, so score from rows.tsv

## The result

| target | match | of | | against the same documents in their original spelling |
|---|---:|---:|---|---|
| `.rtf` | 198 | 328 | **60.4%** | |
| `.ods` | 179 | 307 | **58.3%** | |
| `.odt` | 146 | 329 | **44.4%** | |
| `.odp` | 120 | 302 | **39.7%** | |
| | | | | OOXML + legacy binary: **864 of 947 = 91.2%** |

Verdicts overall: `match` 643, `words` 369, `pages` 129, `pages,words` 100,
**`ours-failed` 22**, `ref-failed` 2, `words,unembedded` 1.

**The readers this project has been measuring are between 30 and 50 points ahead of the ones
it has not.** That gap is not a surprise in hindsight — a reader improves where it is
measured — but nothing in the tree stated it, and several rounds have quoted whole-corpus
figures as though they described the project rather than three of its seven formats.

## The 22 hard failures

Every one is `.ods`. Reduced before dispatch:

    00514292.ods : rc=134  176s  "Out of memory."   34 735 bytes
    00514292.xlsx: rc=0      1s   9 page(s)         33 930 bytes -- the file the ODS came from

Its `content.xml` is 245 KB and declares `number-columns-repeated="16381"` and
`number-rows-repeated="1048567"`, with **156 repeat counts over 1000**. 16381 x 1048567 is
seventeen billion cells, and we materialise them. `sc` does not.

**Not all 22 are necessarily genuine.** `001_Contextures_chart_sample_b089bc34.ods` failed in
the sweep and rendered fine in isolation at 13 pages; that sweep ran under four other agents'
load against a 240 s per-document timeout. The OOM above is genuine and was reproduced twice.
Each of the 22 needs re-running alone before it is counted.

## What this does not yet say

Only that the gap exists and how large it is. The 623 non-`match` rows are unclassified: no
screening for the version gap (the reference here is already 26.2, so that class should be
absent, which is itself worth confirming), no raster-ceiling check, no grouping by cause. The
`.odp` column being the worst by a clear margin is the most interesting single number and has
no explanation yet.
