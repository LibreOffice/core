# The slide title: LibreOffice measures it in one face and draws it in another,
# and the family class is why

Round 92. Seats on `probes/slide-title-track-r90/findings.md`, whose *corrected* reading —
*"the reference is laying the title out at DejaVu Sans Bold's advance table and we are laying it
out at Liberation Sans Bold's"* — is **confirmed, widened from 8 glyphs to 16, verified against
every face on the box rather than two, and given a named, cited mechanism.**

**Nothing in `dotnet/src` changed, and nothing should.** The reference's rendering is internally
inconsistent: it draws Liberation Sans Bold glyphs at DejaVu Sans Bold advances. Our rendering
agrees exactly with the face it *draws* and with that face's own advances. There is no answer to
copy here that is not a defect. §6 says what a future round would have to settle first.

---

## 1. The measurement, taken from the content stream rather than from an extractor

`probes/slide-title-track-r90` read advances out of PyMuPDF character boxes. That channel is
derived and was what produced the discarded letter-spacing framing. This round reads the
operators.

The reference's title (`/home/user/vis-r90/pdf/038_…pdf`, LibreOffice 26.2.4.2, rendered
2026-09-10) is **one text object**:

    BT 56.58 496.998 Td /F2 36 Tf
    [<01>-11<02>-76<03>-152<04>-105<05>-122<06>-144<07>-65<06>-144<07>-65<08>-94<05>-122
     <09>-70<0A>-51<0B>-105<08>-95<0C>-118<0D>-101<06>-144<0C>-118<0E>-105<05>-121<09>-71
     <01>-11<0C>-118<0F>-103<0B>-105<09>-70<10>56<09>]TJ ET

`/F2` is `CAAAAA+LiberationSans-Bold`. Every glyph carries a large **negative** `TJ` adjustment,
which in PDF *widens* the gap after it. The advance the producer actually intended is therefore
`(W − adj) × size / 1000`, where `W` is the resource's own `/Widths` entry. `implied-advances.py`
computes it; `implied-advances-line1.txt` is the full table. Sixteen distinct characters:

| ch | W (Liberation) | adj | drawn | **implied** |
|---|---:|---:|---:|---:|
| C | 722 | −11 | 25.992 | **26.388** |
| o | 610 | −76 | 21.960 | **24.696** |
| m | 889 | −152 | 32.004 | **37.476** |
| e | 556 | −122 | 20.016 | **24.408** |
| t | 333 | −144 | 11.988 | **17.172** |
| i | 277 | −65 | 9.972 | **12.312** |
| – | 556 | **+56** | 20.016 | **18.000** |

The en dash is the useful one: its adjustment is the only *positive* one, so whatever is being
imitated is **narrower** there and wider everywhere else. That is a font table, not a spacing
rule of any kind.

### The identification, widened

`face-scan.py` scores the 16 implied advances, in thousandths of an em, against **every scalable
face under `/usr/share/fonts` and `/opt/libreoffice26.2/share/fonts`** — 200-odd files, read
straight out of `head`/`hhea`/`hmtx`/`cmap`. `face-scan-line1.txt`:

| sum \|err\| (thousandths) | face |
|---:|---|
| **14.2** | **DejaVuSans-Bold** |
| 391.0 | DejaVuSerif-Bold |
| 940.7 | DejaVuSerif |
| 946.0 | DejaVuMathTeXGyre-Regular |

0.9 thousandths a glyph, and **every one of the sixteen residuals is positive** — which is the
`floor(hmtx × 1000 / upem)` truncation `dotnet/CLAUDE.md` already documents in LibreOffice's PDF
writer (mean deficit 0.48–0.65 thousandths). The second-placed face is **27× further away**. The
r90 caution that `fc-match` could not separate DejaVu Sans from DejaVu Sans Condensed or Noto Sans
is discharged: this test reads the files, not fontconfig, and neither of those faces is installed
here at all.

### And the outlines really are Liberation's

r90 asked for this directly. The embedded `FontFile2` for `/F2` was extracted and its **own**
`hmtx` read: `1536 1479 1251 1821 1251 1139 682 569 …` — Liberation Sans Bold's table to the unit,
for 20 of 20 subset glyphs. The `/Widths` array agrees. So the claim is exact and not an
inference from advances: **Liberation Sans Bold glyphs, DejaVu Sans Bold advances.**

### It is confined to the title

`run-census.py` reports drawn width against implied width for every text run on the page
(`run-census-ref-page1.txt`). Of 41 runs, only two are out: the two title lines, at ratio
**1.1727** and **1.1596**. Every 9 pt and 14 pt body run sits at 0.983–1.000 — ordinary kerning.
The 32 pt footer runs sit at 1.036–1.090 and are **not** this defect: the same character gets a
*different* implied advance at different places in the run (`t` at 369 and 382 thousandths, `n` at
563 and 571), which is a fixed `spc="150"` rounded into device units, whereas the title's implied
advances are constant per character to ±1 thousandth. A font table repeats; an accumulated
rounding does not.

---

## 2. The mechanism, named and cited

**Half of it is already in the record, and this round did not discover it.**
`dotnet/CLAUDE.md`'s *"The two references differ in a rule, not only in their fonts"* section, from
`probes/fonts-r64`, established that 26.2.4.2 appends a generic family derived from the declared
family class and that 24.2.7.2 does not, measured over 24 families. **What is new here is the other
half: on the draw layer that class survives into the *measurement* and is thrown away before the
*drawing*, so 26.2.4.2 contradicts itself within one text object.** That cannot happen in a Writer
paragraph, which is why four rounds of font work did not see it.

**`vcl/unx/generic/font/fontconfig.cxx`:1076-1087.** When VCL asks fontconfig to substitute a
family it appends a *second* `FC_FAMILY` entry derived from the request's `FontFamily` class:

```cpp
FontFamily eFamilyType = rPattern.GetFamilyType();
switch (eFamilyType)
{
    case FAMILY_ROMAN: FcPatternAddString(pPattern, FC_FAMILY, "serif"); break;
    case FAMILY_SWISS: FcPatternAddString(pPattern, FC_FAMILY, "sans");  break;
    default: break;
}
```

and `addtopattern` (`:928`) appends `monospace` when the pitch is `PITCH_FIXED`. On this box that
one extra token changes the answer:

```sh
fc-match "Helvetica:bold"          # LiberationSans-Bold.ttf
fc-match "Helvetica,sans:bold"     # DejaVuSans-Bold.ttf
fc-match "Helvetica,serif:bold"    # DejaVuSerif-Bold.ttf
```

**`include/drawinglayer/attribute/fontattribute.hxx`.** `FontAttribute` — the whole of what a
`TextSimplePortionPrimitive2D` carries about its font — holds family *name*, style name, weight,
symbol, vertical, italic, **monospaced**, outline, RTL, BiDiStrong. There is **no field for the
family class** and none for the charset.

**`drawinglayer/source/primitive2d/textlayoutdevice.cxx`:416-448.** `getVclFontFromFontAttribute`
rebuilds a `vcl::Font` from that attribute set. It calls `SetWeight`, `SetItalic`, `SetVertical`,
`SetOutline`, `SetCharSet(RTL_TEXTENCODING_UNICODE)` and
`SetPitch(monospaced ? PITCH_FIXED : PITCH_VARIABLE)` — and never `SetFamily`. The rebuilt font is
therefore `FAMILY_DONTKNOW`, whatever the document said.

**`drawinglayer/source/processor2d/vclprocessor2d.cxx`:485-491.** The processor then draws with
`mpOutputDevice->DrawTextArray(aStartPoint, rTextCandidate.getText(), aDXArray, …)`, where
`aDXArray` is the primitive's own DX array — measured earlier, by editeng, with the font that
*did* still carry the family class.

So the two halves of the same run resolve `Helvetica` differently:

| stage | font | fontconfig pattern | answer |
|---|---|---|---|
| editeng measures, fills the DX array | `SvxFont`, `FAMILY_SWISS` from OOXML `pitchFamily="34"` | `Helvetica`, `sans` | **DejaVu Sans Bold** |
| `VclProcessor2D` draws | rebuilt from `FontAttribute`, `FAMILY_DONTKNOW` | `Helvetica` | **Liberation Sans Bold** |

The OOXML side of it is `oox/source/drawingml/textfont.cxx`:104-108 —
`pitchFamily`'s low nibble is the pitch and its high nibble is the family
(0 DONTKNOW, 1 ROMAN, 2 SWISS, 3 MODERN, 4 SCRIPT, 5 DECORATIVE), so `34` = 0x22 = SWISS +
VARIABLE.

**This resolves the contradiction r90 could not.** `main.xcd`'s `helvetica` node *is* consulted —
but only at `PhysicalFontCollection.cxx`:1179, *after* the fontconfig pre-match hook at `:1142`
and `:1175`. The hook answers first, and what it answers depends on a field the drawing side has
already thrown away. Nothing is reaching past the substitution table; the substitution table never
gets a turn on either path.

---

## 3. Fourteen one-attribute variants, and the mechanism predicts all fourteen

`make-variants.sh` rewrites **only** the `<a:latin>` of the title style, in every `ppt/**/*.xml`
that carries it (both `slideMaster1.xml` and `slideMaster2.xml` — the first cut of this script
edited one of the two, changed nothing, and produced fourteen identical results that looked like a
finding). Each variant is rendered by 26.2.4.2 through `render-ref.sh` and scored by
`run-census.py`. Full table in `variant-table.tsv`:

| variant | `a:latin` | class | pitch | drawn face | ratio | split? |
|---|---|---|---|---|---:|---|
| v0 control | `Helvetica` + panose + `pitchFamily="34"` + charset | SWISS | var | LiberationSans-Bold | **1.1727** | yes |
| v13 | `Helvetica` + `pitchFamily="34"` + charset, **no panose** | SWISS | var | LiberationSans-Bold | **1.1727** | yes |
| v9 | `Helvetica` + panose + `pitchFamily="34"`, **no charset** | SWISS | var | LiberationSans-Bold | **1.1727** | yes |
| v8 | `Helvetica` + panose + `pitchFamily="18"` | **ROMAN** | var | LiberationSans-Bold | **1.1576** | yes, *other face* |
| v2 | `Helvetica` alone | DONTKNOW | — | LiberationSans-Bold | 0.9979 | no |
| v6 | `Helvetica` + panose | DONTKNOW | — | LiberationSans-Bold | 0.9979 | no |
| v10 | `Helvetica` + panose + charset | DONTKNOW | — | LiberationSans-Bold | 0.9979 | no |
| v12 | `Helvetica` + panose + `pitchFamily="2"` | **DONTKNOW** | var | LiberationSans-Bold | 0.9979 | no |
| v11 | `Helvetica` + panose + `pitchFamily="49"` | MODERN | **FIXED** | **DejaVuSansMono-Bold** | **1.0000** | no |
| v1 | `Arial` + panose + `pitchFamily="34"` | SWISS | var | LiberationSans-Bold | 0.9979 | no |
| v3 | `Liberation Sans` + `pitchFamily="34"` | SWISS | var | LiberationSans-Bold | 0.9979 | no |
| v4 | `Zzyzx Nonexistent` + `pitchFamily="34"` | SWISS | var | **DejaVuSans-Bold** | 1.0000 | no |
| v5 | `Zzyzx Nonexistent` alone | DONTKNOW | — | DejaVuSans-Bold | 1.0000 | no |
| v7 | `Helvetica Neue` + `pitchFamily="34"` | SWISS | var | DejaVuSans-Bold | 1.0000 | no |

Six things fall out of that, and each is a prediction the mechanism made before the render:

1. **The family class is the whole trigger.** `panose` is irrelevant (v13 splits without it, v6
   does not split with it) and `charset` is irrelevant (v9 splits without it, v10 does not split
   with it). r90 flagged `panose` as an untested substitution input; it is now tested and it does
   nothing here.
2. **A stated class of DONTKNOW suppresses it** (v12) — the file itself can say what the drawing
   side assumes, and then the two agree.
3. **ROMAN picks a different face and a different ratio** (v8, 1.1576). Scored the same way,
   its implied advances are **DejaVuSerif-Bold at 16.6 thousandths, next best 394.9**
   (`face-scan-v8-roman.txt`) — exactly `fc-match "Helvetica,serif:bold"`.
4. **The one attribute that *does* survive the round trip produces no split.** v11 asks for
   MODERN + FIXED; `FontAttribute` carries `monospaced`, so both stages append `monospace`, both
   answer DejaVu Sans Mono Bold, and the ratio is **exactly 1.0000**.
5. **A family that resolves needs no substitution and cannot split** (v1, v3).
6. **A family the generic answer already covers cannot split either** (v4, v5, v7): plain
   `fc-match "Zzyzx Nonexistent"` is already DejaVu Sans, so adding `sans` changes nothing. The
   split needs a name whose *own* fontconfig answer differs from its generic one.

**And r90's other lead is refuted.** The overflow — two 36 pt lines at 90 % in a 58.2 pt box, with
the slide's `<a:noAutofit/>` — is *identical* in v12, which does not split. Autofit, fit-to-frame
and `SvxCharScaleWidthItem` have nothing to do with it.

---

## 4. Where we stand relative to the reference

Our rendering of the deck is 26.2.4.2's rendering of the same deck **with the family class
removed**, to a tenth of a point:

| | line 1 | implied width | line 2 | implied width |
|---|---|---:|---|---:|
| ours, deck as authored | `Competitive Advantage Card – Slide` | 616.816 | `Template` | 155.373 |
| 26.2.4.2 on v12 (class DONTKNOW) | `Competitive Advantage Card – Slide ` | 626.220 | `Template` | 155.268 |
| 26.2.4.2 on the deck as authored | `Competitive Advantage Card – ` | 623.484 | `Slide Template` | 294.300 |

Line 1 differs only by the trailing space the reference includes and we do not
(616.816 + 9.404 = 626.220); line 2 differs by **0.105 pt on 155**, or 0.07 %, which is the
truncation floor. So we agree with the face the reference *draws*, with that face's own advances,
and with 26.2.4.2 itself on every branch that does not lose the class.

---

## 5. Reach

`reach-census.py` walks every zip-based corpus document, collects each
`(typeface, pitchFamily)` pair stated on an `<a:latin>`/`<a:cs>`/`<a:ea>`, and keeps the pair when
the class is ROMAN or SWISS, the pitch is not FIXED, and `fc-match "<name>"` differs from
`fc-match "<name>,<generic>"` at either weight. `reach-census.txt`, `reach-documents.tsv`:

| track | documents |
|---|---:|
| slides | **99** |
| sheets | 2 |
| words | 0 |
| **total** | **101 of 803** |

**91 of the 101 are `Helvetica`** (`LiberationSans → DejaVuSans`). The rest are serif names —
`Century Schoolbook` ×2, `Times`, `Bodoni MT`, `Book Antiqua`, `Castellar`, `Batang`,
`Adobe Myungjo Std M`, `Amasis MT Pro Black`, `Webdings` — all `DejaVuSans → DejaVuSerif` or
`LiberationSerif → DejaVuSerif`.

Two limits on that figure, both deliberate. It counts documents that *state* such a pair, not
documents where such a run carries text. And it is the **draw-layer** population only: Writer body
text is painted by `SwTextPainter` and never becomes a drawinglayer primitive, so a `w:rFonts`
family class cannot split the same way — which is consistent with the words column being zero
here, but is reasoning rather than measurement. ODF states the same thing as
`style:font-family-generic` and should behave identically; untested.

---

## 6. The two candidate answers, scored against the reference

Neither candidate can match the reference, because the reference does not match itself. Both can be
scored without building anything, because 26.2.4.2 renders each of them for us:

- **leave it** — Liberation Sans Bold measured and drawn. That is `v12` (class stated as DONTKNOW),
  and §4 shows our own output *is* `v12` to 0.1 pt.
- **adopt the class** — DejaVu Sans Bold measured and drawn. That is `v4`/`v7`, where the name is
  one fontconfig already answers DejaVu for on both paths, so the two stages agree.

Both scored against `v0`, the deck as authored, with the repository's own
`render-comparison/scripts/pdf-image-diff.py`:

| candidate | verdict, default settings | page 1 diff% | page 1 \|ink\|% |
|---|---|---:|---:|
| leave it (`v12`) | **1 page MAJOR** — 3 regions on page 2, *ink we draw that the reference does not* and *marks displaced* | 3.16 | **0.30** |
| adopt the class (`v7` = `v4`) | **0 pages major** | **2.10** | 0.64 |

(`candidate-scores.txt` carries both runs, at the default settings and at
`--threshold 8 --min-area 0.0005`; the finer run moves every number a little and moves no ordering.)

The two metrics disagree, and the disagreement is the finding rather than a nuisance: **adopting the
class buys the line break and costs the letterforms.** `diff%` is dominated by the wrap — a whole
word moving between lines — and falls; `|ink|%` is dominated by the letterforms and rises, because
we would then be drawing DejaVu Sans Bold where the reference draws Liberation Sans Bold. At the
tool's default settings, which is the verdict the corpus scripts use, *adopt the class* is the
closer of the two on this document.

## 7. Why no code changed

**The slides path leaves the family class unread on purpose, and the stated reason for that is now
discharged.** `src/XRay.Office.Presentations/Layout/SlideText.cs`:678-682:

> *The pitch and not the family class, deliberately. The family bits are in the same byte and the
> word processor's equivalent leaves them alone for the same reason — declaring a family class
> changes the answer for every name in the deck and **has never been measured on a slide**, where a
> declared pitch has now been measured twice.*

It has now been measured on a slide, and the machinery to honour it is already in the tree:
`FontItem.DeclaredClass`, `FontconfigPreferences` and `FontFamilyClass` are wired for the sheets
path (`SheetDeclaredFonts.FromWindowsCode`, `FromOdfGeneric`) and the words path
(`WordFallbackClass`). The slides change is `SlideFonts` reading the high nibble of the same
`pitchFamily` byte it already reads the low two bits of, and passing it down. It is small.

**It was still not made, for three reasons that are about the evidence rather than the size.**

1. **The reference has two answers and we already match one of them exactly** (§4, to 0.1 pt on
   155). Changing to the other is not correcting an error, it is choosing the other horn. §6 says
   the horns cost different things and no single number picks between them.
2. **The reach is 101 documents and no gate column can see any of it.** Both sides draw the same
   characters, so page counts and alphanumeric counts do not move; the only instrument is ink, and
   ink has to be scored against the reference over all 99 slide documents in both directions before
   the §6 table generalises from one deck. That is a second full 26.2.4.2 sweep, and
   `dotnet/CLAUDE.md` is explicit that a sweep taken while other rounds are building undercounts on
   the reference side. Three other rounds are live.
3. **The whole effect is contingent on this container's fontconfig graph — the seventh confound.**
   It exists only because `fc-match "Helvetica"` and `fc-match "Helvetica,sans"` disagree here, and
   they disagree because Liberation is installed (so `Helvetica` reaches Liberation Sans through
   fontconfig's metric aliases) *and* DejaVu is installed and is this system's generic `sans`.
   Install `urw-base35` and `Helvetica` answers Nimbus Sans on both paths and the effect vanishes;
   remove Liberation and both answer DejaVu and it vanishes again. **Run the two `fc-match` lines in
   §2 before believing any figure in this file was reproduced**; if they agree, this document
   describes a machine you are not on.

What a round that wants to take it on should carry:

- the §6 table over all 99 slide documents rather than one, scored both ways against 26.2.4.2 with
  `RENDER_TIMEOUT` raised and the sweep and the rebuild kept apart by a file;
- whether LibreOffice upstream treats the missing `SetFamily` in `getVclFontFromFontAttribute` as a
  bug — if it is fixed there the reference's answer becomes DejaVu on both paths and the choice
  makes itself;
- whether ODF's `style:font-family-generic` reaches the same drawinglayer path, which decides
  whether the converted corpus doubles the reach or leaves it alone. `SheetDeclaredFonts` already
  reads that attribute for a sheet, so the reader half exists.

## Files

| file | what |
|---|---|
| `implied-advances.py` | per-character intended advance from a PDF's `/Widths` + `TJ`, not from an extractor |
| `run-census.py` | drawn vs implied width for every run on a page; the ratio is the tell |
| `face-scan.py` | scores a set of target advances against every face on the box, reading `hmtx` directly |
| `make-variants.sh` | one-attribute rewrites of the deck's title `<a:latin>` |
| `render-ref.sh` | 26.2.4.2, private profile, `timeout -k 30` |
| `reach-census.py` | the corpus census in §5 |
| `candidate-scores.txt` | the §6 scoring of the two candidates against the deck as authored |
| `dump-stream.py` | page font resources and the raw content stream around a marker |
| `variant-table.tsv`, `face-scan-*.txt`, `implied-advances-line1.txt`, `run-census-*.txt`, `reach-census.txt`, `reach-documents.tsv` | the banked outputs |
