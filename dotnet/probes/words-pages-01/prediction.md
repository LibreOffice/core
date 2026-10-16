# words-pages-01 — prediction

Written **before** either fix was implemented and before any corpus sweep. Diagnosis was already
complete when this was written and the diagnosis half is therefore not a prediction; what is
predicted is **reach** — how many of the 200 words documents each rule moves, and in which
direction. Scored in `results.md`.

## What was measured first (not predicted)

Two documents, both `.doc`, both exactly one page short, both with word counts exact:

* `words/batch-004/doc/1447.doc` — 3 pages against 4.
* `words/batch-006/doc/003.doc` — 4 pages against 5.

They turned out to have **nothing in common**. The brief's framing — "a vertical-budget error
(line height, text-area height, or the line-fits rule)" — is right for neither.

### 1447.doc is a font substitution the gate's font column cannot see

`pdffonts` counts 5 faces in each rendering, so the gate reports `fonts 5/5` and the verdict says
`pages`. The five are not the same five:

| ours | reference |
|---|---|
| LiberationSerif, -Bold, -Italic, -BoldItalic | LiberationSerif, -Bold |
| LiberationSans-BoldItalic | LiberationSans-BoldItalic |
| | **DejaVuSerif, DejaVuSerif-Bold** |

The body of the document names the family **`Times`**. We resolve it to Liberation Serif; the
reference resolves it to DejaVu Serif. Both wrap to the same 432.0 pt measure and the reference
fits about 11% less text on each line, so its first paragraph takes 9 lines where ours takes 7.
The line advance names the faces outright: ours is **13.80 pt**, which is Liberation Serif's
`(1824 + 443 + 87) / 2048 x 12 pt`; the reference's is **14.00 pt**, which is DejaVu Serif's.

Measured against the installed 26.2.4.2 binary with ten authored one-paragraph documents, the
rule is not about `Times` at all:

| declared | family class declared | LibreOffice draws | `fc-match <name>` |
|---|---|---|---|
| Times | *(none)* | Liberation Serif | Liberation Serif |
| Times | roman | **DejaVu Serif** | Liberation Serif |
| Times | swiss | **DejaVu Sans** | Liberation Serif |
| Times | modern / script / decorative | Liberation Serif | Liberation Serif |
| Helvetica | *(none)* / swiss | Liberation Sans / **DejaVu Sans** | Liberation Sans |
| Albany | *(none)* / swiss | Liberation Sans / **DejaVu Sans** | Liberation Sans |
| Thorndale | *(none)* / roman | Liberation Serif / **DejaVu Serif** | Liberation Serif |
| Times New Roman | roman | Liberation Serif | Liberation Serif |
| Arial | swiss | Liberation Sans | Liberation Sans |
| Calibri / Cambria / Courier New | swiss / roman / modern | Carlito / Caladea / Liberation Mono | same |

`fc-match "Times,serif"` answers DejaVu Serif. `FontConfigManager::Substitute`
(`vcl/unx/generic/font/fontconfig.cxx`:1076-1086) adds the requested name as `FC_FAMILY` and then
**appends a second `FC_FAMILY`** — `"serif"` for `FAMILY_ROMAN`, `"sans"` for `FAMILY_SWISS`, and
nothing for any other family type. It runs as the *pre-match* substitution, i.e. **before**
LibreOffice consults its own `VCL.xcu` chain, which is the ordering our resolver has backwards:
`VCL.xcu`'s chain for `times` names `liberationserif` and we take it.

The names that survive the generic are exactly those an installed face declares itself
metric-compatible *with* (Arial, Times New Roman, Courier New, Calibri, Cambria) — fontconfig's
`30-metric-aliases.conf` binds those strongly. A second-hand alias (Albany, Helvetica, Thorndale,
all of which our table routes to Arial or Times New Roman) does not survive it.

### 003.doc is an empty paragraph in the WW8 reader, and the deficit is exactly accounted for

Page 2 onwards is *identical* in pitch — 13.80, 27.60, 47.40, 19.80 pt line for line — and differs
only in where the page starts. The whole error is on page 1, and it is 32.20 pt, which is three
empty paragraphs measured at the wrong font size:

| paragraph | reference | ours | deficit |
|---|---|---|---|
| empty, after "Tisková a informační služba" | 16.10 pt (14 pt) | 13.80 pt (12 pt) | 2.30 |
| empty, after "INFOSERVIS" | 41.40 pt (36 pt) | 13.80 pt (12 pt) | 27.60 |
| empty, after the empty Heading 2 | 16.10 pt (14 pt) | 13.80 pt (12 pt) | 2.30 |

32.20 pt is two 16 pt lines, which is what lets two extra empty paragraphs fit at the foot of our
page 1; every page after it then carries two lines more than the reference's, and the seven
trailing empty paragraphs that give the reference a fifth (blank) page fit on our fourth.

Reading the file itself (`chpx.py`) settles where the sizes come from. Each of those three
paragraph marks has **no CHPX exception at all** and `istd` 0 (Normal), so the paragraph style
gives 12 pt — which is what we use. Each of them is the *first* paragraph after a CHPX run that
did carry a size:

```
run 3: fc 1094..1150  CHps=1c00 (14pt)   <- ends at cp 63, the empty paragraph's own mark
run 4: fc 1150..1170  (no exception)     <- cp 63..72, ten empty paragraphs
run 5: fc 1170..1192  CHps=4800 (36pt)   <- ends at cp 84
run 6: fc 1192..1194  (no exception)     <- cp 84
run 7: fc 1194..1196  CHps=1c00 (14pt)   <- cp 85
run 8: fc 1196......  (no exception)     <- cp 86 onwards
```

The empty paragraphs at cp 63, 84 and 86 take 14, 36 and 14 pt in LibreOffice; the ones at cp 64-72
and cp 87 take 12 pt. The discriminator is whether a CHPX exception **ends at the mark**: the
reader closes such an attribute at offset 0 of the node the mark has already opened, and a
zero-length hint on an empty node covers the whole node. Same fix as a rule: *an empty paragraph
whose own mark carries no CHPX exception takes the one in effect at the position before it.* All
seven points on this document agree, including the two that must not inherit.

Our own ODF path lays the same document out correctly (36.00 / 14.00 / 14.00), so the layout engine
is not implicated — only `Ww8DocumentReader.Describe`.

## Predictions

| # | Claim | Confidence |
|---|---|---|
| P1 | The WW8 empty-paragraph rule makes `003.doc` 5 pages and a `match`. | 0.85 |
| P2 | The family-class rule makes `1447.doc` 4 pages and a `match`. | 0.75 |
| P3 | The WW8 rule changes **3 to 12** of the 200 words renderings, and no more than 2 verdicts go backwards. | 0.55 |
| P4 | The family-class rule changes **10 to 45** of the 200 words renderings — it is a font change and fonts cascade. | 0.60 |
| P5 | Net verdicts across the 200 improve by **at least 2** and the total absolute page error falls. | 0.60 |
| P6 | Neither rule touches `batch-001`..`batch-003`, which are already 10/10 — those documents name Times New Roman, Arial, Calibri or nothing. | 0.50 |
| P7 | Some document currently matching will break, because both rules move line breaks. The count is 1 to 4. | 0.65 |

**The way this round is most likely to be wrong** is P4 in the other direction: the family-class
rule is measured on *authored* documents and on the corpus it may reach almost nothing, because
the corpus's `.doc` files may all name families that either are installed or have a strong metric
alias. An understated reach that comes true reads as a good prediction (words-r45's warning), so
the reach figure is to be taken from a whole-track sweep and not from a census of declared names.
