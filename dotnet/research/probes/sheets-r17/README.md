# What LibreOffice actually resolves a font family to

`lo-faces.tsv` is a gold table: for each of the 296 font families the whole 534-document sample
corpus names, the face **LibreOffice 24.2.7.2 on this machine actually used**, measured rather
than inferred. `our-faces.tsv` is the same question asked of `SystemFontResolver` at
`e49c70453`. They agree on 270 of the 293 the probe could name.

It exists because the obvious source — `officecfg/registry/data/org/openoffice/VCL.xcu`, which
`FontSubstitutions.Tables.cs` is generated from — **is not what the running binary follows**.
`PhysicalFontCollection::FindFontFamily` asks the fontconfig pre-match hook at
`vcl/source/font/PhysicalFontCollection.cxx:1142` and returns its answer at `:1151`;
`ImplFontSubstitute`, which is the `SubstFonts` chain, is only reached at `:1180`. On Linux that
hook is `FcPreMatchSubstitution::FindFontSubstitute` (`vcl/unx/generic/font/fontsubst.cxx:98`),
which asks fontconfig about every request that is not symbol-encoded — and fontconfig always
answers, with its own default family for a name it has no rule for. So the chain is unreachable
for an uninstalled, non-symbol family.

## How to rebuild it

```sh
python3 corpusfams.py families.txt                 # every family the corpus names
python3 mkprobe2.py families.txt corpus.fods       # one row per family
soffice --headless --convert-to pdf corpus.fods
python3 facewidth2.py families.txt corpus.pdf > lo-faces.tsv
```

Each row draws `Hamburgefonstiv` and `0123456789` in its family, with an `Rnnnn` index cell in a
fixed face beside them. The two drawn widths name the face outright — the eight installed faces
are more than a point apart on both at 10 pt — and the index cell is what makes the row
identifiable, since three cells of one row sit at different vertical positions when the faces
differ and counting rows in order silently shifts the whole table when one goes missing.

## What it cannot tell you

**Symbol faces.** ODF states no charset, so `Wingdings`, `Wingdings 2`, `Wingdings 3` and
`Webdings` were not symbol-encoded requests here and fontconfig answered them. A DOCX or XLSX
font carrying `charset="2"` makes the hook bail at `fontsubst.cxx:101` and the chain *does* run,
which is why this table says DejaVu Sans for those four and our OpenSymbol answer is probably the
right one.

**CJK.** fontconfig's reply depends on the characters asked for, and this asks in Latin. `MS
Gothic` and `MS PGothic` come back DejaVu Sans here and would not for CJK text.

## The other scripts

- `whonames.py` — which corpus documents name a given family. The OOXML side parses; the binary
  side is a byte scan and gives false positives, which is the safe direction for deciding what to
  re-render.
- `renderours.sh` — renders our side only for a list of corpus documents and reports pages and
  extractable words. The reference does not move for a Paperless-only change, so this compares
  two of our own binaries in a fraction of a sweep.
- `affcmp.py` — joins two `renderours.sh` runs to the reference column of a `batch-check.sh`
  sweep and says, per document, whether it moved closer.
