# How tall an empty paragraph is, and which default decides

*Measured 2026-09-06 in the container described at the top of `dotnet/CLAUDE.md`. **Both** installed
references were used and they agree on every figure below to the hundredth: the distro's **24.2.7.2**
at `/usr/bin/soffice` and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2` with its bundled
Latin faces moved aside. X-Ray.Office at `agent/words-draw2`, base `e152bc0b2`.*

## Why this exists

`probes/words-margin-print-area/results.md` §4 filed a residual and left it: *"our empty header
paragraph is shorter than Writer's by [1.90 pt] … it reaches any document with an empty paragraph in
its running head."* The round that opened this one was briefed that the same 1.90 pt was behind
`WordArt_Shapes_Arrows_Catalog1.docx`'s remaining vertical divergence, since 4 px at 150 dpi is
1.92 pt.

**It is not, and that is settled in `probes/words-inline-shape-ink/results.md`** — the catalogue's
only blank header is its `w:type="even"` one and the document declares no `w:evenAndOddHeaders`, so
neither renderer draws it. This probe is the other half: what the 1.90 pt actually is, and how far it
reaches.

## The measurement

`makeprobe.py` puts the same empty paragraph in two places. In the **body**, between a 12 pt
`TOPLINE` and a 12 pt `BOTLINE`, so its height is the gap between them less one line. In a **header**,
where it shows as the body's own top. It varies the five spellings of "an empty paragraph" a real
document uses, the stated size and face, and — the variable that turns out to decide everything —
what the package's `word/styles.xml` says.

Gap between `TOPLINE` and `BOTLINE`, and `BODYLINE`'s own top, in PDF points:

| fixture | what it is | 24.2 / 26.2 | ours, before | ours, after |
|---|---|---:|---:|---:|
| `body-none` | two lines, nothing between | 13.80 | 13.80 | 13.80 |
| `body-full` | a 12 pt line between | 27.60 | 27.60 | 27.60 |
| `body-run-empty-t` | a run with an empty `w:t` | **27.25** | 25.35 | **27.25** |
| `body-run-no-t` | a run with no `w:t` at all | **27.25** | 25.35 | **27.25** |
| `body-bare` | `<w:p/>` | **27.25** | 25.35 | **27.25** |
| `body-mark-only` | the size on the paragraph *mark* | 27.60 | 27.60 | 27.60 |
| `body-mark-and-run` | on both | 27.60 | 27.60 | 27.60 |
| `body-sz16/20/24/40` | the run's size, four values | **27.25** each | 25.35 | **27.25** |
| `body-carlito`, `body-libsans` | the run's face | **27.25** | 25.35 | **27.25** |
| `hdr-run-empty-t`, `hdr-run-no-t`, `hdr-bare` | the same, in a header | **49.36** | 47.46 | **49.36** |
| `hdr-full`, `hdr-mark-only`, `hdr-mark-and-run` | | 49.71 | 49.71 | 49.71 |

Two things the top half settles, before the cause:

1. **The paragraph mark decides, and a run cannot stand in for it.** Four sizes and two faces stated
   on the *run* of an empty paragraph all give 27.25, the same as a paragraph with no run at all;
   stated on the *mark* they give the paragraph's own height. That is the rule `DocxLayoutSource`
   already models and it was already right.
2. **So the shortfall is in the default the mark falls back to**, not in the mark's resolution.

## The cause, and it is the presence of an element rather than its content

Adding a `word/styles.xml` to the same fixture moves the reference, and only one of the four arms:

| fixture | `w:docDefaults` | 24.2 / 26.2 | ours, before | ours, after |
|---|---|---:|---:|---:|
| `body-run-empty-t` | *no styles part at all* | **27.25** | 25.35 | **27.25** |
| `body-dd-bare` | `<w:rPrDefault><w:rPr/></w:rPrDefault>` | **25.35** | 25.35 | 25.35 |
| `body-dd-carlito` | the same, naming Carlito | **26.00** | 26.00 | 26.00 |
| `body-dd-sz28` | the same, `w:sz w:val="28"` | **29.90** | 29.90 | 29.90 |
| `body-dd-both` | both | **30.90** | 30.90 | 30.90 |

An *empty* `w:rPrDefault` changes the answer by 1.90 pt. That is not a value being read; it is the
element being present:

- `DomainMapper::DomainMapper` (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:182-193, tdf#108350)
  sets the document text defaults to **Calibri 11 pt** for every OOXML import, commented *"In Word
  since version 2007, the default document font is Calibri 11 pt. If a DOCX document doesn't contain
  font information, we should assume the intended font to provide best layout match."*
- `StyleSheetTable::applyDefaults(false)`, reached only from the `LN_CT_DocDefaults_rPrDefault` arm of
  `StyleSheetTable::sprm` (`StyleSheetTable.cxx`:672-681), then resets them to **Times New Roman
  10 pt** (`:2161-2167` for the family, `:341-350` for the 10 pt, *"set font height default to 10pt"*)
  and lays the file's own `rPrDefault` values over the top.

11 pt in Carlito is 13.45 pt of line (Carlito's line box is 2500/2048 of the em) and 10 pt in
Liberation Serif is 11.55 (2355/2048). The difference is the 1.90.

We had the 10 pt unconditionally — `WordParagraphFormats.DefaultSize`, whose comment named the right
`w:docDefaults` fallback and not the condition on it — and no Calibri branch at all.

## The reach, which is the part worth carrying

**No document in the words corpus reaches it.** Every one of the **272** DOCX-family files under
`sample-files/words` declares a `w:docDefaults/w:rPrDefault`; 61 of them declare one that states no
`w:sz`, and those take 10 pt in both renderers, which is what the `body-dd-bare` row above measures.
Word always writes `docDefaults`, so the branch is reached by hand-built packages — this project's
probe fixtures and several of its test fixtures — and not by files from the wild.

So `words-margin-print-area` §4's *"it reaches any document with an empty paragraph in its running
head"* is too wide by a long way: the empty paragraph is not the condition, the missing
`w:rPrDefault` is, and an empty paragraph is merely where a wrong default first becomes visible.

## After

Every one of the 24 fixtures agrees with both references at **0.00**. On
`words-margin-print-area`'s own fixtures, `hdr-empty`'s body top goes 47.46 → **49.36** against the
reference's 49.36 and its band centre 409.50 against 409.38 (was 0.88 out, now 0.12, which is the
288 dpi raster quantum); `inhdr-3line` goes to **430.00** exactly, from 429.25 against 430.00. Those
were the two rows that file records as a different defect, and they are the ones that closed.

One test elsewhere had to change and it is worth naming.
`CharacterSpacingTests.TrackingChangesTheTextsWidthByOneUnitPerGap` asserts an *exact* width
difference on a package with no styles part, and the fixture's face was therefore the fallback. In
Calibri the sentence kerns by 0.392 pt where in Liberation Serif it does not, and the tracked run
suppresses that kerning, so the difference read 190.392 against 190. The fixture now names Liberation
Serif at 10 pt — which is what it was resolving to — exactly as its sibling `Glyphs` helper already
names Calibri. The test measures the same thing and no longer depends on a default it never stated.

## Reproducing

```sh
export XRAY_OFFICE_CLI=.../tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
python3 makeprobe.py /abs/scratch/epfx
python3 measure.py   /abs/scratch/epfx /abs/scratch/epout
```

`measure.py` needs `pdftotext` and renders every fixture through `/usr/bin/soffice`,
`/opt/libreoffice26.2/program/soffice` and `$XRAY_OFFICE_CLI`, so the version question is re-checked on
every run rather than taken from this file.

**A caution about the first run of this probe, because it produced a confident wrong answer.** The
`dd-*` fixtures' `word/styles.xml` was not being written into the package — the generator's edit had
not landed — and every one of them read 27.25, identical to the no-styles control. That is exactly
what "LibreOffice ignores `w:docDefaults` for an empty paragraph" would look like, and it was
believed for about ten minutes. What refuted it was reading the round trip: `soffice --convert-to
fodt` reported the default paragraph style as `Calibri 11pt` on a fixture whose `rPrDefault` said
Carlito 14, which is impossible if the file had been read at all. `unzip -l` then showed no
`word/styles.xml`. **Assert your fixture contains what you think it does before believing what it
measures.**
