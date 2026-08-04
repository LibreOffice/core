# The row-height instrument

What is here, and how to run it again. Everything reads LibreOffice's *own* recomputed row
heights out of a flat-ODS export, so the ground truth is the binary rather than an inference.

## Putting the probe back

`RowHeightProbe.cs.txt` is a test class that no-ops unless an environment variable names a
workbook. It lives here rather than in `dotnet/tests/Paperless.Spreadsheets.Tests/` because a
test that always passes is worse than no test. To use it:

```sh
cp RowHeightProbe.cs.txt ../../dotnet/tests/Paperless.Spreadsheets.Tests/RowHeightProbe.cs
cd ../../dotnet && dotnet build tests/Paperless.Spreadsheets.Tests/Paperless.Spreadsheets.Tests.csproj
```

Remember to delete it again before committing — it adds three vacuous tests to the count.

## The three instruments

**Row heights, ours against LibreOffice's.** The sharpest one, and the one that turned "rows
are 10.4% too tall" into "763 rows are a whole number of lines short".

```sh
bash tofods.sh /workspace/sample-files/sheets/batch-017/xlsx/TK-....xlsx   # LibreOffice's answer
python3 rowheights.py rt/TK-....fods > lo.tsv
bash probe.sh /workspace/.../TK-....xlsx "$PWD/ours.tsv"                   # ours
python3 compare-rows.py lo.tsv ours.tsv | head -40
```

`compare-rows.py` reports exact / taller / shorter counts, the worst rows, and a per-sheet
total. `colwidths.py` does the same for `style:column-width`, which is how the column widths
were ruled out as a cause.

**Break points, by bisection.** `make-widths.py` authors a flat-ODS holding the same strings at
fifty-odd column widths, one sheet each; `run-widths.sh` round-trips it and `lines.py` turns the
recomputed heights back into line counts. The width at which a string drops a line pins the
paper LibreOffice measured it against, to within fifty twips.

```sh
bash run-widths.sh probe-ns 11 Calibri strings2.txt widths1.txt
```

`strings2.txt` holds unbreakable words on purpose: character-level breaking makes the line count
a clean `ceil(width / paper)`, where word breaking blurs it by up to a word.

**What LibreOffice draws, as opposed to what it measures.** `draw.sh` puts the same strings in a
column wide enough that nothing wraps, renders to PDF and reads the word boxes back with
`pdftotext -bbox`. That is what showed the 6% is the measuring device and not the face:
LibreOffice draws Carlito at Paperless's advances to within a quarter point over four hundred.

**Checking the model.** `check-probe.sh <name>` opens a probe `.fods` with Paperless, dumps its
row heights and scores them against LibreOffice's round trip of the same file. All five probes
here report 416 or 450 of 416 or 450.

## Sweeps

`summary.py <rows.tsv> [n]` gives match count, page error and page-exact count from a
`batch-check.sh` run, plus the n worst documents. `diff.py <a> <b>` lists what moved between two
runs, split into improved and worse. `ink.sh <run>` runs the image comparison over every
document in a run that already passes the word gate.
