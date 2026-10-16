# O57: the extra width is INVENTED, not stated — 26.2.4.2 draws a `.ppt`'s Helvetica in Liberation Sans at DejaVu Sans' advances

Round 115. The seat asked one question and everything else waited on it:

> **Is the extra width stated in the `.ppt`, or invented by the reference?**

**Invented.** It is not spacing at all. On `architecture6.ppt` page 10, 26.2.4.2 lays the table's
14 pt text out at **DejaVu Sans / DejaVu Sans Bold**'s advance widths while drawing — and
embedding — **Liberation Sans / Liberation Sans Bold**'s glyphs. That is the **seventh confound**
(`L1`, `probes/title-font-r92`), which was established on a `.pptx` and is here shown to reach the
binary format through `lfPitchAndFamily`. **Ours is the self-consistent output and no code
changed.**

O57 is therefore terminal as an established 26.2 defect, in the same family as round 111's
`RRM 16`. One residual remains and is *not* this: a ~3.3 pt difference in the table cell's own
measure, seated below as **O59**.

Everything below is measured on this container unless it says *inferred*. No blind reader was
available (a subagent has no subagent tool here, established four times); every reading in this
file is arithmetic, not vision.

---

## 1. Leg one, from the record: a `.ppt` cannot state character spacing

`PPTStyleTextPropReader`'s character branch — the **reference's own reader** —
`filter/source/msfilter/svdfppt.cxx`:5096-5185, reads the whole of a `TextCFException`:

    nMask                          the flag word: bold, italic, underline, shadow, strikeout, emboss
    0x010000 cfTypeface            mnFont
    0x200000 cfFEOldTypeface       mnAsianOrComplexFont
    0x400000 cfANSITypeface        mnANSITypeface
    0x800000 cfSymbolTypeface      mnSymbolFont
    0x020000 cfSize                mnFontHeight
    0x040000 cfColor               mnColor
    0x080000 cfPosition            mnEscapement

and stops. There is **no tracking field in the record and no `PPT_CharAttr_*` constant for one** —
`include/filter/msfilter/svdfppt.hxx`:1416-1428 defines thirteen and the highest is
`PPT_CharAttr_Symbol`. `git grep -n "Kerning\|SvxKerningItem" filter/source/msfilter/svdfppt.cxx`
is empty.

So whatever 26.2.4.2 drew, it did not come from a spacing record: its own reader has nowhere to put
one. The flat ODP's zero `letter-spacing` occurrences, which round 113 correctly declined to treat
as conclusive, is a second symptom of the same fact rather than the evidence.

*(Read in the `27.2.0.0.alpha0+` bulk-import tree, which is not the reference binary's source. Leg
two is a measurement of 26.2.4.2 itself and does not depend on it.)*

## 2. Leg two, from the reference's own output: it is a different face's advance table

`advances.tsv`. Per-glyph drawn advance for `Description`, from the glyph `origin` values in each
PDF, in ems of the drawn 14.003 pt:

| glyph | ours | **26.2.4.2** | Liberation Sans Bold `hmtx` | DejaVu Sans Bold `hmtx` |
|---|---:|---:|---:|---:|
| D | 0.72200 | **0.83100** | 0.72217 | 0.83008 |
| e | 0.55600 | **0.67900** | 0.55615 | 0.67822 |
| s | 0.55600 | **0.59400** | 0.55615 | 0.59521 |
| c | 0.55600 | **0.59400** | 0.55615 | 0.59277 |
| r | 0.38900 | **0.49300** | 0.38916 | 0.49316 |
| i | 0.27800 | **0.34300** | 0.27783 | 0.34277 |
| p | 0.61100 | **0.71800** | 0.61084 | 0.71582 |
| t | 0.33300 | **0.47700** | 0.33301 | 0.47803 |
| o | 0.61100 | **0.68700** | 0.61084 | 0.68701 |

Not a constant per glyph and not a constant ratio: it is a **font table**, and it is DejaVu's.
`verify.py` scores every adjacent pair at 14 pt on the page, spaces excluded because justification
lives in the space:

| rendering | pairs | nearer Liberation | nearer DejaVu | mean \|Δ\| vs Liberation | mean \|Δ\| vs DejaVu |
|---|---:|---:|---:|---:|---:|
| **26.2.4.2 as authored** | 767 | **0** | **767** | 0.07278 em | **0.00107 em** |
| 26.2.4.2, class cleared | 772 | **768** | 4 | **0.00072 em** | 0.07230 em |
| 26.2.4.2, name → Arial | 772 | **768** | 4 | **0.00072 em** | 0.07230 em |
| ours | 772 | **768** | 4 | **0.00029 em** | 0.07300 em |

And the glyphs really are Liberation's on both sides, read rather than inferred. Both PDFs embed
`LiberationSans-Bold`; the per-character **ink bbox** widths agree to **0.014 pt** (`D`
10.110/10.110, `e` 7.786/7.786, `r` 5.447/5.447, `p` 8.556/8.542); and the reference's own
`/Widths` array for that subset is

    /Widths [ 0 722 610 556 610 333 556 389 277 556 556 722 556 277 610 277 722 … ]

which is `floor(hmtx × 1000 / 2048)` of **Liberation** Sans Bold to the unit. The whole difference
is carried in per-glyph **negative `TJ`** adjustments — the `Description` run is
`[<10>-109<06>-123<11>-38<0C>-38<07>-104<0D>-66<04>-108<05>-144<0D>-66<15>-77<13>]`, so `D` is
declared 722 and adjusted −109 to reach 831 — which is `drawHorizontalGlyphs` reconciling a layout
that used one table with a subset that declares another.

## 3. The single-variable confirmation, on the deck itself

The deck states `Helvetica` with `lfPitchAndFamily = 0x22` — **`FF_SWISS` | `VARIABLE_PITCH`** — in
both of its `FontEntityAtom`s (offsets 277396 and 279354; `variants.py` prints them). `svdfppt.cxx`
:2183-2186 puts that class on the `vcl::Font` with `aFont.SetFamily(aFontAtom.eFamily)`, having mapped `lfPitchAndFamily & 0xf0` to a `FontFamily` at `:410-435`, and
`FontConfigManager::Substitute` (`vcl/unx/generic/font/fontconfig.cxx`:1076-1087) appends `sans`
for `FAMILY_SWISS`. On this container:

```sh
fc-match "Helvetica:bold"        # LiberationSans-Bold.ttf   <- what 26.2.4.2 DRAWS
fc-match "Helvetica,sans:bold"   # DejaVuSans-Bold.ttf       <- what 26.2.4.2 MEASURES
fc-match "Arial:bold"            # LiberationSans-Bold.ttf
fc-match "Arial,sans:bold"       # LiberationSans-Bold.ttf   <- both, so no split
```

Two byte-length-preserving variants of the real file, each changing exactly one thing, rendered by
26.2.4.2 (`variants.py`; `var-ctrl.ppt`, which rewrites the name to itself, compares equal to the
original). Ink-bbox widths of the five header strings on page 10:

| string | 26.2.4.2 as authored | **class byte 0x22 → 0x02** | **name Helvetica → Arial** | ours |
|---|---:|---:|---:|---:|
| `Name` | 43.577 | 38.172 | 38.172 | 38.130 |
| `Description` | 89.185 | 77.073 | 77.073 | 77.031 |
| `Example` | 65.310 | 57.636 | 57.636 | 57.595 |
| `Advantages` | 92.966 | 79.439 | 79.439 | 79.370 |
| `Disadvantages` | 116.337 | 98.903 | 98.903 | 98.834 |

Either change collapses the reference onto this tree to **0.042–0.069 pt**, which is the
`floor(hmtx × 1000 / upem)` truncation floor `dotnet/CLAUDE.md` already documents (11 glyphs ×
~0.5/1000 em × 14 pt ≈ 0.077 pt). The blind reader's `Nam e`, `Exam ple` disappear with it: MuPDF
inserts a synthetic space when a gap exceeds its threshold, and `Name` comes back as 4 characters
instead of 5.

## 4. What that is worth over the whole deck

`pagecmp.txt`. Every text run on all 31 pages, keyed by its string, compared by first-glyph origin:

| ours against | pages with any difference | run-set symmetric difference | worst common-run origin shift |
|---|---:|---:|---:|
| 26.2.4.2 as authored | 20 | **268** | **322.07 pt** |
| 26.2.4.2, family class cleared | 19 | **52** | **1.60 pt** |

Run **counts** are identical on all 31 pages against the de-confounded reference. Of the residual
52, 40 are private-use bullet code points (`U+E46F`/`U+E47A` against `U+F0A7`/`U+F0B2` — the same
marks through different PUA) and 12 are one wrapped word on page 14, which is §6.

The five pages that move wholesale as authored — **10, 14, 21, 24, 27** — are exactly the five the
O57 row named.

## 5. Reach, measured rather than censused

Two figures, because they answer different questions. `L1`'s census covered the **zip** corpus only
(101 of 803, slides 99), which is the same trap `dotnet/CLAUDE.md` records for charts: a legacy
binary has no zip part to walk, so the whole `.ppt`/`.doc`/`.xls` track was invisible to it.

**Documents that STATE such a pair** (`census.py`, `declared-census.txt`): **20 of the 51 legacy
PPT-family corpus documents**, against a base rate of 51 scanned and 51 readable. By family:
`Wingdings 2` ×9 (declared `FF_ROMAN`), `Times` ×4, `Book Antiqua` ×3, **`Helvetica` ×2**,
`UBSHeadline`, `Perpetua`.

**Documents where it reaches drawn text** (`sweep2.sh`, `class-reach.tsv`): each of the 51 rendered
by 26.2.4.2 twice — as authored, and with **every** `FontEntityAtom`'s family nibble cleared and
nothing else changed — scoring the fraction of glyph slots carrying a `|TJ| ≥ 20/1000 em`
adjustment. **3 of 51 move:**

| document | as authored | class cleared |
|---|---:|---:|
| `architecture6.ppt` | 0.4389 | **0.0441** |
| `RRM-training-syllabus-…-Dec-2009.ppt` | 0.2150 | **0.0000** |
| `pres_ioc_phuket.ppt` | 0.1593 | **0.0005** |

The middle one is the document `O15` already files under the seventh confound, which the
instrument rediscovered without being told — the control this measurement most needed.
`pres_ioc_phuket.ppt` is **new**: it states `Times` at `FF_ROMAN`, so its split is Liberation Serif
drawn at DejaVu Serif's advances, the same shape as `RRM 16`.

The other 48 do not move by a thousandth. That includes three documents with a *high* `TJ`
fraction that is **not** this — `FAA_Form_337.ppt` 0.4446 → 0.4427, `010605Vul.ppt` 0.3078 → 0.3078,
`hofman.ppt` 0.0790 → 0.0790 — which is justification and kerning, and is why the discriminator is
run as a **difference** rather than as a level.

**No gate column can see any of it.** Both sides draw the same characters, so page counts and
alphanumeric counts do not move; `O57`'s row already recorded *5 pages, 1 document, no page count
and no alphanumeric count moves*, and nothing here changes that. No sweep was run because nothing
was changed.

## 6. What is left, and it is not O57 — seating it as O59

With the confound removed, the reference's table cell still measures **~3.3 pt wider** than ours.
`measure3.py` takes the pen position at the end of each justified line in page 10's body cell,
adding the last glyph's own design advance to its origin, so it is the paragraph measure and not an
ink edge:

| | left | justified line ends |
|---|---:|---|
| ours | 199.05 | 678.54 – 678.58 → measure **479.51** |
| 26.2.4.2, class cleared | 199.05 | 681.63 – 682.33 → measure **~482.85** |
| 26.2.4.2 as authored | 199.05 | 681.70 – 682.45 → measure **~482.9** |

The class byte does not move it (482.85 against 482.9), so it is a separate question about a
`.ppt` table cell's inner width — 0.69 % — and it is what still costs page 14 one word. The short,
unjustified lines in the same cell agree to **0.21–0.45 pt**, so the text itself is right and only
the measure is not. Seated as **O59**; the next free number was 59 and a parallel round is live.

## 7. Two corrections to round 113's numbers, both from the same instrument

1. **The 24 pt heading is not part of this.** `453.10 → 468.00` compared 39 characters against 40:
   26.2.4.2 draws a trailing space as its own `Tj` at x = 511.2 and this tree does not draw it at
   all. Its `TJ` adjustments are ±1/1000 em — the heading resolves to a face with no split and
   agrees. The row's *"the heading, whole, +14.90"* should not be carried forward.
2. **The measures were quoted the wrong way round.** *"the reference's justified lines fill
   478.76–479.51 against our 471.70–472.51"* is origin-to-origin with a different final glyph on
   each side. Like for like, ours is 479.51 and the reference's is ~482.85 — the reference's box is
   **3.3 pt** wider, not 7.1.

## 8. Why nothing was implemented

`SlideFonts.Resolve` builds its `FontRequest` with the declared **pitch** and never with a declared
**class** (`src/XRay.Office.Presentations/Layout/SlideText.cs`), so a `.ppt`'s `FF_SWISS` cannot reach
the resolver. That was a deliberate abstention whose stated reason was *"has never been measured on
a slide"*. It has now been measured on a slide twice — round 92 on a `.pptx`, this round on a
`.ppt` — and the measurement **endorses the abstention**:

- reading the class would put us on **DejaVu Sans**, measured *and* drawn. That is 26.2.4.2's other
  branch and it is equally not what the reference draws;
- not reading it puts us where we are: **Liberation Sans, measured and drawn** — the face 26.2.4.2
  itself draws, at that face's own advances, agreeing with 26.2.4.2's own class-cleared render to
  0.042–0.069 pt over five strings and to 1.60 pt over 31 pages.

The words and sheets paths *do* pass a declared class, and that is right for them and not
inconsistent: Writer body text is painted by `SwTextPainter` and never becomes a drawinglayer
primitive, so there the reference measures and draws in the same face and honouring the declaration
is a fix (`FontResolutionTests.ADeclaredShapeBeatsAWeakAliasTheChainWouldHaveTaken`, worth 11 % of a
line on `1447.doc`). The split is a property of the **draw layer** alone.

**And the whole effect is contingent on this container's fontconfig graph.** Install `urw-base35`
and `Helvetica` answers Nimbus Sans on both of the reference's paths; remove Liberation and both
answer DejaVu. Run the four `fc-match` lines in §3 before believing any number in this file was
reproduced.

## 9. Regression

`tests/XRay.Office.Presentations.Tests/PptFontFamilyClassTests.cs`, 10 assertions:

- `SlideFonts.PitchIn` masks `0x03`, so `0x22` and `0x02` are the same request — the seat itself;
- a `FontRequest` built the way the slides path builds one carries `FontFamilyClass.Unknown`;
- **the measurement is pinned**: Liberation Sans Bold's and DejaVu Sans Bold's own advance sums for
  the five strings at 14 pt — `Description` **77.0137** against **90.5488**, ratio 1.1758 — so the
  next round does not re-derive it, and an implementation that adopts the reference's width fails
  here rather than in a corpus sweep. Both sides are sums of `hmtx` entries; neither is fitted.

## What a reader would still be worth, and on which page

No blind reading was available this round and none of the above depends on one. Two pages would
repay an independent reader when one exists:

- **`architecture6.ppt` page 14, ours against 26.2.4.2 rendered from `var-class00.ppt`** — the
  de-confounded pair. Everything on it agrees except one wrapped word and the 3.3 pt measure of
  §6/O59, and a reader given that pair and nothing else is the check on whether the measure is the
  whole of what is left or only the part arithmetic can see.
- **`pres_ioc_phuket.ppt`** — newly named under the seventh confound by §5 and never looked at.
  Its split is `Times` at `FF_ROMAN`, so Liberation Serif drawn at DejaVu Serif's advances; nobody
  has confirmed what that does to the page.

## Test totals, run project by project and totalled by hand

`dotnet build XRay.Office.slnx` — **0 warnings, 0 errors**. Each project run on its own with
`--no-build` and its count compared against round 113's:

| project | passed | failed | skipped | total |
|---|---:|---:|---:|---:|
| Core | 588 | 0 | 0 | 588 |
| Containers | 109 | 0 | 0 | 109 |
| Text | 728 | 0 | 0 | 728 |
| Vector | 309 | 0 | 0 | 309 |
| Markup | 259 | 0 | 0 | 259 |
| Rendering | 164 | 0 | 0 | 164 |
| OpenDocument | 160 | 0 | 0 | 160 |
| Spreadsheets | 1352 | 0 | 0 | 1352 |
| Presentations | **1203** | 0 | 0 | 1203 |
| WordProcessing | 1938 | 0 | 0 | 1938 |
| **ten unit projects** | **6810** | **0** | **0** | **6810** |
| Fidelity | **542** | **10** | **0** | **552** |

Presentations is 1203 against 1193, the ten added here. Fidelity is **542 / 10 / 552, 0 skipped** —
byte for byte round 113's ten and no others: `TabStopComparisonTests` ×4 and
`PageDrawingComparisonTests` ×4 (the PDF truncation channel this file's `CLAUDE.md` section
documents as deliberately left failing), `SheetDrawingComparisonTests` and
`JustificationShrinkComparisonTests`. **0 skipped** everywhere, so nothing covered nothing.

No corpus sweep was run, because nothing in `dotnet/src` changed that can move a rendering: the
only source edit is a documentation comment.

## An instrument warning: do not run `provenance-index.py` in this worktree

It regenerates `PROVENANCE.tsv` by default — `--help` is not a recognised flag, so *asking for the
usage writes the file*. Run here it goes **1724 rows → 1225: 816 lost against 317 gained**. Only 69
of the losses are the `dotnet/research/probes` root, which a `dotnet`-only sparse checkout does not
have; the rest are ordinary tracked, present probe directories (`rtf-htmautsp-r85` 46 rows,
`odt-split-r82` 27, `sheet-wrap-r99` 25, `slides-r100` 25 …). It was run here by accident, the
result was reverted with `git checkout --`, and this round's fifteen rows were **appended by hand**
in the file's own format instead. Whatever is wrong with the generator, running it costs the index
half its content and looks like a routine refresh.

## A second, smaller instrument note

`OPEN-ISSUES.md` is a markdown table and **ten of its rows contained an unescaped `|` inside a code
span**, which splits the row into five or seven cells and drops everything after the first pipe from
every renderer. Nine are `|ink|%` and `|glyph distance|` in other seats' rows and are left alone —
a parallel round is editing this file — but the O57 row written here escapes its `\|TJ\|`, and so
should the next one. `python3 -c "import re;[print(i+1) for i,l in enumerate(open('dotnet/probes/OPEN-ISSUES.md')) if re.match(r'^\| [A-Z]',l) and len(re.split(r'(?<!\\)\|',l))-2!=3]"`
lists them.

## Files

| file | what |
|---|---|
| `variants.py` | the two one-attribute variants of the deck, and the control |
| `advances.tsv` | per-glyph drawn advance, both renderings, against both faces' `hmtx` |
| `verify.py` | scores every adjacent pair on a page against Liberation and DejaVu |
| `ttf.py` | minimal `head`/`hhea`/`hmtx`/`cmap` reader, so the faces are read rather than matched |
| `tj.py` | fraction of glyph slots carrying a `\|TJ\| ≥ 20/1000 em` adjustment |
| `declass.py` | clears every `FontEntityAtom`'s family nibble in place |
| `sweep2.sh`, `class-reach.tsv` | the 51 × 2 reference renders and their scores |
| `census.py`, `declared-census.txt` | which documents *state* a splitting class-ful family |
| `pagecmp.py`, `pagecmp.txt` | run-by-run comparison over all 31 pages |
| `measure3.py` | the justified paragraph measure, from pen positions |
| `tj-census.tsv` | the as-authored `TJ` fractions alone, kept for the base rate |
