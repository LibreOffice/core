# colour-r65 — a colour bitmap glyph is a Type 3 font, and the fallback search has a floor under it

The defect handed on by `fonts-r64`: *"Colour bitmap faces do not paint. `U+2714` now names Noto
Color Emoji at the right advance and draws nothing (CBDT/CBLC)."*

Everything below is measured against **LibreOffice 26.2.4.2** from the TDF tarball with the three
font confounds moved aside. `/usr/bin/soffice` (24.2.7.2) is not used anywhere in this round.

---

## 1. The census, first, because it decided the split

### What is installed

`census.tsv` beside this, from the table directory of every font file under `/usr/share/fonts`,
`/usr/local/share/fonts`, `~/.fonts`, `~/.local/share/fonts` and
`/opt/libreoffice26.2/share/fonts`.

| glyph source | faces |
|---|---:|
| `glyf` outlines | 120 |
| `CFF ` outlines | 29 |
| `CBDT`/`CBLC` colour bitmaps | **1** |
| `COLR`/`CPAL` layered colour | **0** |
| `sbix`, `SVG ` | **0** |
| total scanned | 150 |

The one colour face is `/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf`, and it carries
**neither `glyf` nor `CFF `** — the strikes are the whole of it. **Nothing installed carries
`COLR`/`CPAL` at all**, which is what decides how much of this round is worth writing.

### What the corpus reaches

`probes/fonts-r64/faces-after.tsv` is a whole-corpus sweep of the faces our own PDFs embed, and
`faces-before.tsv` the same sweep at the previous commit.

| | documents of 947 |
|---|---:|
| draw a character that lands on **Noto Color Emoji** | **2** |
| draw a character that lands on a **`COLR`/`CPAL`** face | **0** |
| draw a character that lands on a **`CFF `** face | **1** |

The two colour documents are both sheets:

| document | colour glyphs the reference draws | ours, before |
|---|---:|---|
| `sheets/chartset-013/…/019_Free_Blood_Sugar_Chart…xlsx` | 6 distinct (⌛ ⏰ 🍕 🏃 🚫 🛏) | blank |
| `sheets/done-007/xlsx/jobs-bulletin-51-22-december-2025.xlsx` | 1 (⭐) | blank |

**Both were already resolving to Noto Color Emoji before `fonts-r64`** — they are in
`faces-before.tsv` as well as `faces-after.tsv`, the same two rows. So the round that named this
regression created the *probe*'s blank and not the corpus's: the corpus blank is older than the
round that reported it, and the fallback reordering neither caused nor widened it.

### The CFF question, which the brief asked to be checked and which is not the same bug

One corpus document reaches a `CFF ` face:
`slides/done-014/…/vvsummit2022-Research-Roadmap…pptx`, through the Unifont last-resort fallback.
It is **not** a blank and it is **not** this defect:

- the **rasteriser draws it** — Skia reads Type 2 charstrings — so only the PDF writer is affected;
- the PDF writer's behaviour there is already deliberate and already documented in
  `PdfFontCatalogue.IsCompactFontFormat`: the face is named, its widths are kept and the program is
  not embedded, so a reader substitutes and draws **tofu, not nothing**;
- and on that document it is entangled with a *resolution* difference rather than a painting one —
  26.2.4.2 draws **NotoSansArmenian-Regular** there, a face this machine does not have. That half
  belongs to whoever owns font resolution.

Its cost, measured: the deck is the one `MISMATCH` in `slides/done-014`, on `unembedded 1`, at
33/33 pages and 1281/1281 words. Unchanged by this round, before and after.

---

## 2. What the reference does with a colour glyph, exactly

`gen-colour.py` builds one DOCX per (declared class, character) for `U+2714`, `U+2611`, `U+263A`
and `U+1F600` — the same generator as `fonts-r64`, narrowed. Through 26.2.4.2, `roman__2714.pdf`:

```
pdffonts:  BAAAAA+NotoColorEmoji   Type 3   Custom   emb yes   sub yes   uni yes
pdftotext: U+2714
```

and inside it:

```
9 0 obj  <</Type/Font/Subtype/Type3/Name/NotoColorEmoji
          /FontBBox[0 -244 1245 928] /FontMatrix[0.001 0 0 0.001 0 0]
          /CharProcs<< /gid152 10 0 R >>
          /Encoding<</Type/Encoding/Differences[1 /gid152]>>
          /FirstChar 0 /LastChar 1 /Widths[0 1245.1171875 ]
          /FontDescriptor 7 0 R /Resources 11 0 R /ToUnicode 8 0 R >>

10 0 obj  1245.1171875 0 d0
          q 1247.55859375 0 0 1174.31640625 0 -247.55859375 cm /Im12 Do Q

12 0 obj  <</Subtype/Image/Width 136/Height 128/ColorSpace/DeviceRGB
            /BitsPerComponent 8/Filter/FlateDecode/SMask 14 0 R>>
```

So: **a Type 3 font whose char procs draw decoded images, with a `/ToUnicode` and no font
program**, and the descriptor — not the font dictionary — carries the subset-tagged name.

### The placement arithmetic falls straight out of those three constants

Noto Color Emoji is **2048 units per em** with one strike at **109 ppem**, whose glyphs are
**136 × 128 pixels** at `bearingX 0, bearingY 101` (`CBLC` index format 1, `CBDT` image format 17,
small metrics then a whole PNG). Under `/FontMatrix 0.001` the reference's three numbers are

| | design units | × 1000/2048 |
|---|---:|---:|
| width | `round(136 × 2048/109)` = **2555** | 1247.55859375 |
| height | `round(128 × 2048/109)` = **2405** | 1174.31640625 |
| bottom | `round(101 × 2048/109) − 2405` = **−507** | −247.55859375 |

Every digit of the reference's own `cm`. **The pair settles rounding against truncation**: the
width is 2555.30 units and the height 2404.99, so one rounds down and the other up, and truncation
would have written 2404. That is `ColourBitmap.PlacementIn`, and `ColourBitmapTests` asserts those
three integers rather than a re-derivation of them.

The advance is `hmtx`, not the strike's: 2550 units, which is the `1245.1171875` of both the `d0`
and the `/Widths`. **Our `/Widths` already said `1245.1172` before this round** — `fonts-r64`'s
"the right advance" is confirmed, and the advance did not move.

---

## 3. What was implemented, and what was not

### Implemented: `CBDT`/`CBLC`, both backends

- **`XRay.Office.Text/Fonts/ColourBitmaps.cs`** reads `CBLC` index subtable formats 1–5 and `CBDT`
  image formats 17, 18 and 19 — the three that wrap a whole image file. The bit-aligned monochrome
  formats are deliberately not read: a face using them is not a colour face and draws from its
  outlines. Nothing is decoded here; the PNG bytes go to whichever backend has a codec, exactly as
  `RasterImage.Encoded` already carries an undecoded picture.
- **`PdfFontCatalogue.Type3`** writes the shape above. The page's content stream is *untouched* —
  the run is still `<01> Tj` against a font resource — so pen positions, word breaks and
  `/ToUnicode` are the same objects the ordinary path writes.
- **`SkiaDrawingSink.DrawColourBitmaps`** draws the same strike from the same `PlacementIn` call, so
  the two backends cannot disagree about where the glyph goes.

### Deferred: `COLR`/`CPAL`, v0 as well as v1

**No face installed on this machine carries the tables and no corpus document reaches one**, so
there is nothing here to measure an implementation against, and the project's standing rule is that
measurement decides. Writing the layer composition would have been perhaps 150 lines of code that
this container cannot render a single page with.

**What falls back to what in its absence**, which is the part that had to be built anyway: a
`COLR`/`CPAL` face is reported *unpaintable* by `GlyphPainting.CanPaint`, so the fallback search
passes it over and answers with the next candidate that covers the character — a monochrome face
drawing a monochrome glyph. `sbix` and `SVG ` faces (Apple's colour emoji is `sbix`; neither is
installed) behave the same way. That is a visible wrong glyph rather than a blank, which is the
trade the brief asks for. Implementing `COLR` v0 later is a strict improvement over it and needs
nothing here undone: `CanPaint` gains a clause and the two sinks gain a composition path.

### The floor: `SystemFontResolver.Covers`

All three fallback stages — the emoji preference list, the request's own generic list, and
LibreOffice's fixed list — ask their candidates through one method, so the test sits there and an
unpaintable candidate falls through everywhere at once. **It changes no preference**: candidates are
offered in exactly the order they were and one is only ever *skipped*, never promoted. The advance
therefore follows whichever face actually draws, because the face this returns is the face that is
measured, shaped and painted.

**It fires nowhere on this machine**, and that is the honest reach: the one colour face is now
painted, every other installed face has outlines. It is the guard `fonts-r64` did not have, and
`GlyphFallbackPaintabilityTests` exercises it on faces made by stripping `glyf`/`loca` out of
installed ones — with the ordering control in the same test, so the assertion is about the guard
and not about the order.

**`CFF ` counts as paintable, deliberately.** Its non-embedding is a limitation of one *backend*
and not of the face: the rasteriser draws such a face correctly and only the PDF writer declines the
program. Rejecting it here would move a line break to work around a writer.

---

## 4. Reach, before and after

### The probe, at 100 dpi

| | ink pixels | bounding box |
|---|---:|---|
| 26.2.4.2 | 510 | (80, 80)–(118, 117) |
| ours, before | **0** | — |
| ours, after (PDF) | 510 | (80, 79)–(118, 116) |
| ours, after (PNG backend) | 465 | (81, 80)–(117, 116) |

The mean colour of the ink in our rasterised PDF is `(99.6, 121.6, 133.0)` against the reference's
`(99.6, 121.6, 133.0)`. The one-pixel vertical offset is the *baseline*, not the strike: our
`Td` is `56.7 762.95` against the reference's `56.8 762.339`, a pre-existing 0.61 pt, and our char
proc is `1245.1172 0 d0 / q 1247.5586 0 0 1174.3164 0 -247.5586 cm` — the reference's numbers to
four decimals. The deflated colour plane is **1351 bytes on both sides**.

### The corpus, page ink at 100 dpi

| document | page | reference | ours before | ours after |
|---|---:|---:|---:|---:|
| `019_Free_Blood_Sugar_Chart…xlsx` | 1 | 96 432 | 108 172 | 109 280 |
| `019_Free_Blood_Sugar_Chart…xlsx` | 2 | 51 013 | 57 901 | 58 744 |
| `jobs-bulletin-51-22-december-2025.xlsx` | 1 | 37 056 | 36 735 | **37 016** |

(The blood-sugar sheet's pages carry more ink than the reference's for reasons that predate this
round; what moved is the emoji, +1108 and +843 pixels. The jobs bulletin's page 1 goes from 321
pixels short of the reference to 40.)

Both documents' PDF font sets now match the reference's shape exactly — the colour face is a
Type 3 on both sides and every other face a subsetted TrueType.

### The gate, screened against 26.2.4.2

| batch | total | match | mismatch | note |
|---|---:|---:|---:|---|
| `sheets/chartset-013` | 10 | 8 | 2 | `019` (colour) **match**; the two are `001_Contextures` on pages and `033_Event_planning` on words, neither a colour face |
| `sheets/done-007` | 10 | **10** | 0 | includes `jobs-bulletin` |
| `slides/done-014` | 10 | 9 | 1 | the Unifont `unembedded`, unchanged |

---

## 5. Verification

- `dotnet build XRay.Office.slnx -v q -nologo` — 0 warnings, 0 errors.
- Ten non-fidelity projects, run individually: 109 + 427 + 259 + 128 + 916 + 162 + 1106 + 674 + 302
  + 1593 = **5676 passed, 0 failed, 0 skipped**, against a baseline of 5656 and the 20 tests added
  here (9 + 3 + 6 + 2).
- `XRay.Office.Fidelity.Tests`: **542 passed, 10 failed, 0 skipped** of 552 — the baseline exactly,
  and the same ten names (four `TabStopComparisonTests`, one `JustificationShrink`, four
  `PageDrawingComparisonTests`, one `SheetDrawingComparisonTests`).

## 6. What this contradicts

**`GlyphOutlines` was never in the path, and the brief's premise about it is wrong in a way worth
recording.** `XRay.Office.Text/Fonts/GlyphOutlines.cs` is `glyf`-only and that is true, but it is
reached *only* by Fontwork (`Ooxml/DrawingML/FontworkFitting.cs`) and never by text. The blank had
two seats and neither was that reader: `PdfFontCatalogue` embedded a `glyf`-less TrueType program
and announced it as one, and `SkiaDrawingSink.DrawOutlines` asked *Skia's* `SKFont.GetGlyphPath`,
which answers empty. So widening `GlyphOutlines` would have fixed nothing at all.

**The corpus blank predates the round that reported it.** See the census: the same two sheets are in
`faces-before.tsv` and `faces-after.tsv`.
