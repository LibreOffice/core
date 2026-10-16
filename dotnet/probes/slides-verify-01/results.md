# `slides-verify-01` — the fourteen unverified `slides/ceiling-*` documents, re-audited

**Measured 2026-08-15.** Environment: LibreOffice **26.2.4.2** 620(Build:2), `fonts-dejavu-core`
present (`fc-match "DejaVu Sans"` → `DejaVuSans.ttf`), corpus `/c/sandbox/workdir/sample-files`,
our tree at `e1630c6f1a6` on branch `wt-p-verify`, gate metric = `batch-check.sh`'s
letter-or-digit token count.

Why the round existed: fourteen of the sixteen open slides failures were classified `ceiling` on
2026-08-09, **before** the 2026-08-13 container move, and `dotnet/probes/PROVENANCE.tsv` flags
their figures as pre-move. The task was to re-verify the classification, not to fix anything.

## Result in one line

**All fourteen are confirmed ceilings, none is a mislabelled defect, and every one passes the
gate outright once its mechanism is excused.** The write-up lives in `dotnet/TODO.raster-ceiling.md`
under *Audited 2026-08-15*; this directory is the data behind it.

## What is here

| file | what it is |
|---|---|
| `parity.tsv` | the `batch-check.sh` sweep of `slides/ceiling-00*`, 15 rows (Sylva is present twice, `.PPTX` and `.pptx`, byte-identical) |
| `pagewords/*.tsv` | per-page word and whitespace-stripped character accounting for each pair |
| `glyphs-per-show.tsv` | whole-document glyphs and show operators, both sides, from `pdf-ops.py dump` |
| `third-conversion.tsv` | a third independent `soffice --convert-to pdf` of every document, for the determinism control |
| `pagewords.py` `pagemech.py` `docops.py` | the three instruments, kept |

`pagemech.py` takes a document id and page numbers and prints, per side, the `pdfimages -list`
dimensions aggregated by size and the content-stream counts of text records, glyphs, show
operators, fills, **glyph-sized fills with their colours**, images and strokes. That last column
is the one that separates the outlining ceiling from everything else, and no other instrument on
the project reports it.

## Three controls, and each one earned its cost

1. **The gate figures reproduce.** All fourteen rows in `parity.tsv` equal their 2026-08-09
   values to the word. The stale-evidence risk this round was commissioned to check was real and
   did not fire.
2. **The reference is deterministic on all fourteen.** Three independent samples each — the
   banked `refpdfs-26.2.4.2-fonts/` render, the sweep's own conversion, and
   `third-conversion.tsv` — agree on page count and on both word metrics for every document.
   Nothing here belongs in `unstable`.
3. **Five pages went to blind reviewers**, one page each, no numbers, no repository access:
   `Sylva` p10, `pres_ioc_phuket` p24, `Demick_JetBlue` p5, `16 - UTM - (NASA)` p29,
   `OnTrac_StarCertificationProgram-3Day` p10. All five reported that **neither half is missing
   content the other has**, which is the whole claim a ceiling makes. Three of them named the
   mechanism unled: *"much wider inter-character spacing"* (mechanism 3), *"small edge text on
   the chart is present and legible in both halves"* (mechanism 2 — ink with no text layer), and
   *"the whole slide body looks softly blurred/rasterized"* (mechanism 1).

## The one methodological trap this round hit

The first cut of the per-page mechanism script used `awk`'s three-argument `match()`, which is
**gawk only** — `mawk` is the default here and it fails with a syntax error per line while the
`pdfimages` half of the same command still prints. The output looked like a partial answer
rather than like a broken tool. Rewritten in Python. The generalisation is the project's own:
assert your instrument produced what you think it produced before comparing anything.

## Reproducing

```sh
cd /c/sandbox/workdir/wt-p-verify
export XRAY_OFFICE_CLI=$PWD/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
.claude/skills/corpus-batches/scripts/batch-check.sh \
    /c/sandbox/workdir/sample-files 'slides/ceiling-00*' /abs/out 3 > sweep.log 2>&1
grep '^TOTAL' sweep.log
python3 dotnet/probes/slides-verify-01/pagewords.py /abs/out/ours/<id>.pdf /abs/out/ref/<id>.pdf --chars
python3 dotnet/probes/slides-verify-01/pagemech.py '<id>' <page> [<page> …]
```

`pagemech.py` and `docops.py` hardcode `/c/sandbox/workdir/ver-out/sweep` as the sweep root;
change the `S` constant if yours is elsewhere.
