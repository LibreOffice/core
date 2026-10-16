# r149 — O91: the height of a `<w:br/>` line in a `wps:wsp` text body, and what happens when the body overflows

Measurement-only round. **Nothing was built**; the parent session holds the build in this tree.
Every `[bin]` figure below is a render of **LibreOffice 26.2.4.2**
(`/opt/libreoffice26.2/program/soffice`, `0229ac93fcf0d7cbc6376066c6f35021cef002dc`) and every
`[src]` figure is a reading of the C++ checkout at `/home/user/libreoffice-core`, which declares
**27.2.0.0.alpha0+** and is *not* the reference binary's source.

## 0. Instrument state

| thing | value |
|---|---|
| reference | `/opt/libreoffice26.2/program/soffice` → `LibreOffice 26.2.4.2 0229ac93fcf0d7cbc6376066c6f35021cef002dc` |
| `/usr/bin/soffice` | 24.2.7.2 — **not used anywhere in this round** |
| tarball font confounds | all five aside: `.duplicates-aside/` 47 faces, `.noto-aside/` 8, no `Liberation*`/`DejaVu*`/`Carlito`/`Caladea`/`Noto{Sans,Serif}-*`/`*Condensed*` left in `share/fonts/truetype` (81 remaining, checked) |
| volatile-date confound | not applicable — the witness holds no `TODAY()`/`NOW()` |
| corpus | `/home/user/sample-files`, `MANIFEST.tsv` 947 rows |
| witness | `words/chartset-006/docx/086_Printable_Graph_Paper_Template_Gray_Theme_7300e5d7.docx` |

Renders are `timeout -k 30 900`, one at a time, each with its own `-env:UserInstallation=`, and
`render.sh` prints `FAIL` for any arm whose PDF does not exist. No arm without output is reported.

## 1. The seat's headline number still holds — and its decomposition does not

`mutate.py` builds 28 one-attribute variants of the witness; `render.sh` renders each through
26.2.4.2 into its own profile; `read.py` reads the drawn `Title:`/`Date:` spans out of the PDF.
All 28 produced output (`render.txt`), and every arm draws its text in **DejaVuSans at 11.00 pt**
— Century Gothic is not installed and falls back identically in every arm, so no font
substitution separates them.

**Total cost of the four breaks, 26.2.4.2 [bin] — confirmed, two independent ways:**

| measurement | arms | Σ of the four break lines |
|---|---|--:|
| top-anchored, baseline moves by the whole sum | `topk4` 51.801 − `topk0` 40.951 | **10.850 pt** |
| as authored (`anchor="ctr"`), baseline moves by half the sum | `k4` 52.701 − `k0` 47.301 = 5.400 | **10.800 pt** |

The register's **10.8 pt for all four** is reproduced today, to 0.05 pt. It is not a decayed
figure.

**But "2.7 pt each" is an average of four unequal lines, and the inequality is the fact.**
Top-anchored `k`-sweep, break runs at the authored `w:sz="4"` (2 pt):

| arm | `Title:` baseline | Δ from the previous k |
|---|--:|--:|
| `topk0` | 40.951 | — |
| `topk1` | 44.301 | **+3.350** |
| `topk2` | 46.801 | +2.500 |
| `topk3` | 49.301 | +2.500 |
| `topk4` | 51.801 | +2.500 |

So a 2 pt break line is **2.500 pt**, and the *first* one costs **0.850 pt more**. The centred
sweep halves every one of these (+1.650, +1.250, +1.250, +1.250), which is the same numbers and
an independent read of them.

**The paragraph mark is not consulted at all, and that half of the seat is confirmed by
refutation.** `msz4`, `msz22`, `msz40`, `msz80` put `<w:sz>` of 2, 11, 20 and 40 pt on the
paragraph's own `w:pPr/w:rPr` and change **nothing**: all four draw `Title:` at baseline
**44.301**, byte-identically to `bsz4`/`topk1`, which states no mark size at all. 26.2.4.2's
break-line height does not move with the paragraph mark by any amount, at any of four sizes.

**It is the break RUN's size that decides it.** One break kept, top-anchored, only `w:sz` on the
break run varied:

| `w:sz` | pt | `Title:` baseline | break-line height |
|--:|--:|--:|--:|
| 4 | 2 | 44.301 | 3.350 |
| 8 | 4 | 46.601 | 5.650 |
| 12 | 6 | 48.951 | 8.000 |
| 16 | 8 | 51.251 | 10.300 |
| 22 | 11 | 54.751 | 13.800 |
| 40 | 20 | 65.250 | 24.300 — the body now overflows and is clipped (§3) |

Least squares over all six points: **1.16434 pt of line per pt of font, plus a constant
1.0031 pt**, residual ≤ 0.018 pt. DejaVu Sans' own `(hhea.ascender + |descender|)/upem` is
2384/2048 = **1.164062**, so the slope is the face's own figure to **0.024 %** and the paragraph's
`w:line="259"` proportional spacing (108 %) is **not** in the slope. The constant is not a
mystery and §2.4 identifies it: it is the 108 % surplus of the *text* line — 276 − 256 twips =
exactly **1.00 pt** — which appears as soon as any break line exists and does not grow with the
break's size.

## 2. The closed form, from hand-built fixtures — 70 arms, all rendered

`build.py` and `build4.py` write minimal one-shape DOCX by hand: a `wps:wsp` 324 × 288 pt,
`anchor="t"`, `lIns=rIns=tIns=bIns=0`, whose body is *N* runs holding nothing but `<w:br/>` at
`w:sz=BSZ`, then one run `Hxy` at 11 pt; a `RULER` run sits in the page body and every position
below is **relative to the RULER's own baseline**, so no absolute page origin is ever needed.
**Every fixture carries `word/settings.xml`** (`compatibilityMode` 15) — `dotnet/CLAUDE.md`
records five clean, mutually corroborating and entirely wrong answers got for want of it.
51 + 19 arms, `render3.txt`/`render4.txt`, **70 of 70 produced output**.

### 2.1 The height comes from the break RUN's own size

One 2 pt break line, `w:line="240" w:lineRule="auto"` (single):
`n0..n6` = 0, 2.350, 4.700, 7.050, 9.400, 11.750, 14.100 — **exactly 2.350 pt per break line,
additive, no first-line anomaly**. The two-break family `d02..d80` is 2× the one-break family
`b02..b80` at every one of seven sizes, to 0.000.

Break-run size swept, one break, single spacing (`b*`):

| pt | 1 | 2 | 4 | 6 | 8 | 10 | 11 | 12 | 16 | 20 | 24 | 30 | 40 |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| drawn | 1.150 | 2.350 | 4.650 | 7.000 | 9.300 | 11.650 | 12.800 | 14.000 | 18.650 | 23.300 | 27.950 | 34.950 | 46.550 |
| `round_twip((asc+desc)·s)` | 1.15 | 2.35 | 4.65 | 7.00 | 9.30 | 11.65 | 12.80 | 13.95 | 18.65 | 23.30 | 27.95 | 34.90 | 46.55 |

DejaVu Sans `hhea` is 1901/−483/0 on 2048 = **1.1640625 em**. Rounding the ascent and the descent **separately** — `round(1901·s·20/2048) + round(483·s·20/2048)`
twips — is exact at **twelve of the thirteen**, including the 12 pt and 30 pt sizes the combined
rounding misses. The residual is 1 pt, where it predicts 24 twips and 23 are drawn; at 0.05 pt
that is the twip itself and is not pursued.

### 2.2 It is the break run's own FACE, and LibreOffice's ordinary metric precedence

One break at 20 pt, only the break run's `w:rFonts` varied (`f-*`), against the face's own tables:

| face | `hhea` asc+desc+gap | `OS/2` win | `OS/2` typo+gap | drawn | `USE_TYPO_METRICS` |
|---|--:|--:|--:|--:|---|
| DejaVu Sans | 23.281 | 23.281 | 24.004 | **23.300** | no |
| Liberation Sans | 22.998 | 22.344 | 21.768 | **23.000** | no |
| Liberation Mono | 22.656 | 22.656 | 16.025 | **22.650** | no |
| FreeSerif | 26.040 | 24.040 | **22.000** | **22.000** | **yes** |

Four faces, four different answers, each its own `hhea` figure — **external leading included**
(Liberation Sans' 67-unit `lineGap` is the whole of its 22.344 → 22.998) — except FreeSerif,
which sets `fsSelection` bit 7 and is measured on `typo`. That is LibreOffice's ordinary Writer
font-metric precedence and nothing special to a break.

### 2.3 Three things the break line does NOT take

- **Not the paragraph mark.** `m002 … m120` put `w:pPr/w:rPr/w:sz` of 1, 2, 11, 20, 40 and 60 pt
  on the paragraph and every one of the six draws `Hxy` at **2.350** — identical to the arm that
  states no mark size at all. 26.2.4.2 does not consult it, for the break line or for the last
  line, at any size.
- **Not the next run.** `t04/t40/t80` put the *following* run at 2, 20 and 40 pt: the break line
  stays 2.350 and only the text line's own ascent moves. Predicted from
  `round_twip(1901·s/2048)` = 1.85 / 18.55 / 37.15 pt, drawn −6.000 / 10.700 / 29.300 against a
  predicted −6.000 / 10.700 / 29.300 — **exact at all three**.
- **Not "a break is a portion with no font".** `c04/c40/c80` put a real character `A` *in the
  break run*, so the line carries a text portion of the same font too: 2.350 / 23.300 / 46.550,
  byte-identical to `b04/b40/b80`. The break portion alone already carries the run's font.

### 2.4 The paragraph's line-spacing rule applies on top, and skips the frame's first line

`l-*`, one 2 pt break, only `w:spacing` varied:

| rule | drawn | predicted |
|---|--:|--:|
| `auto 240` (100 %) | 2.350 | 2.35 |
| `auto 259` (107.9 %) | **3.350** | 2.35 + 1.00 |
| `auto 480` (200 %) | **15.150** | 2.35 + 12.80 |
| `exact 240` | 12.000 | 12.00 — the break run's 2 pt is overridden outright |
| `atLeast 480` | 24.000 | 24.00 |

and the full sweeps `pr259-n0..n4`, `pr360-n0..n4`, `pr480-n0..n4`:

| proportion | n=0 | n=1 | n=2 | n=3 | n=4 | Δ per later break |
|---|--:|--:|--:|--:|--:|--:|
| 259/240 | 0 | 3.350 | 5.850 | 8.350 | **10.850** | 2.500 = `trunc(47 tw × 259/240)` |
| 360/240 | 0 | 8.750 | 12.250 | 15.750 | 19.250 | 3.500 = `trunc(47 × 1.5)` |
| 480/240 | 0 | 15.150 | 19.850 | 24.550 | 29.250 | 4.700 = 47 × 2 |

**`pr259-n4` = 10.850 pt is the witness's own figure, rebuilt from nothing.** The first break
costs more than the later ones because the paragraph's *first* line is formatted at its raw
height and the proportional surplus of the *text* line then appears for the first time: 2.35 +
(276 − 256) tw = 3.35 at 259, 2.35 + (512 − 256) tw = 15.15 at 480. Both are predicted with no
free parameter.

### 2.5 The rule is not special to a shape

`pb259-n0..n3` put the identical paragraph in the **page body** instead of the shape:
13.800, 17.150, 19.650, 22.150 — the same +3.350 then +2.500, +2.500. Whatever is wrong in this
tree's shape path is wrong in the same rule, not in a shape-only one.

**Closed form.** For a line of a Writer paragraph, in a shape body or in the page body, whose
only content is a `<w:br/>`:

```
raw   = ascent(F, s) + descent(F, s) + externalLeading(F, s)     # LibreOffice's own precedence,
                                                                 # each term rounded to a twip
height = raw                                    if the line is the text frame's first line
       = raw + trunc((prop - 100) * raw / 100)  for `auto` otherwise, prop = round(w:line/240*100)
       = w:line twips                           for `exact`
       = max(raw, w:line twips)                 for `atLeast`
```

where **`F` and `s` are the font and size of the run that carries the `<w:br/>`** — not the
paragraph mark's, not the next run's, and not the body's default.

### 2.6 An instrument warning that cost this round a whole section

**PyMuPDF's text extraction honours the page's clip paths, and 26.2.4.2 clips an overflowing
shape body rather than dropping it.** Read through `page.get_text('dict')`, the witness's `bsz40`
arm comes back as *no text on the page*, and `s22`–`s32` come back as `Title:` with its
underscores gone. Both readings are false. `readraw.py` parses the content stream directly —
every `Td … Tf [ … ] TJ` origin and glyph count and every `re W* n` rectangle — and shows those
arms emitting the **full 44 glyphs** at their ordinary positions with a clip rectangle added.
The first cut of §3 was written from the clip-honouring read and said the reference discards its
overflow; it does not. Every figure below is from `readraw.py`. The positions in §1 and §2 are
unaffected — those arms carry no clip — and were re-derived with `readraw.py` to 0.001 pt.

The clip-blind reader also closes the model to **absolute** positions. The clip rectangle on an
overflowing arm *is* the shape's inner rectangle, x 52.05, **y 30.75, h 25.45**, which gives the
body's own top for free. With it, every arm's baseline is predicted with no free parameter:

| arm | predicted | drawn |
|---|--:|--:|
| `topk0` | 30.75 + 10.20 | **40.95** |
| `bsz22` | 30.75 + 12.80 + 1.00 + 10.20 | **54.75** |
| `bsz40` | 30.75 + 23.30 + 1.00 + 10.20 | **65.25** |
| `asis` (centred, body 23.65 in 25.45) | 30.75 + 0.90 + 9.85 + 1.00 + 10.20 | **52.70** |
| `q08` (4 breaks at 4 pt) | 30.75 + 4.65 + 3×5.00 + 1.00 + 10.20 | **61.60** |
| `q10` (4 breaks at 5 pt) | 30.75 + 5.85 + 3×6.30 + 1.00 + 10.20 | **66.70** |

Six arms, six exact hits.

## 3. The two halves, separated — and O91b as written is refuted at the reference

### 3.1 The witness's body FITS in 26.2.4.2. It never overflows.

The shape's inner rectangle is 25.45 pt tall (32.65 − 3.6 − 3.6), and the clip rectangle the
reference emits on an overflowing arm confirms it to the twip. From §2's closed form the
authored body is

```
line 1 (break, 2 pt, paragraph's first line, no proportional)   2.35
lines 2-4 (break, 2 pt, + 8 % of 47 tw)              3 x 2.50 = 7.50
line 5 (text, 11 pt, + 8 % of 256 tw)                          13.80
                                                       total  23.65   <=  25.45
```

with **1.80 pt to spare**, and the centred baseline this predicts, 52.70, is the one drawn. So
the reference is not overflowing and then rescuing the text; there is nothing to rescue. This
tree's 14.8 pt per break line makes the same body 4 × 14.8 + 13.8 ≈ **73 pt in a 25.45 pt box** —
**O91a is what creates the overflow that O91b then reacts to.**

**The threshold is measured and it is exactly where the closed form puts it.** Sweeping only the
break run's `w:sz` on the witness with one break kept, the reference must overflow above
`raw + 13.80 > 25.45`, i.e. above `raw = 11.65 pt`, i.e. above **exactly 10 pt**:

| break pt | 8 | 9 | 9.5 | **10** | **10.5** | 11 | 12 | 14 | 16 | 20 |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| predicted body | 22.10 | 24.30 | 24.85 | **25.45** | **26.00** | 26.60 | 27.80 | 30.10 | 32.45 | 37.10 |
| shape clip emitted? | no | no | no | **no** | **yes** | yes | yes | yes | yes | yes |
| `anchor` t/ctr/b distinct? | yes | yes | yes | (collapsing) | **no** | no | no | no | no | no |
| glyphs emitted | 44 | 44 | 44 | 44 | 44 | 44 | 44 | 44 | 44 | 44 |

The clip appears, and the three anchors collapse onto one position, between 10 and 10.5 pt — the
predicted boundary — and **all 44 glyphs are emitted at every size**.

The same boundary from the other direction, sweeping the size of *all four* breaks (`q*`):
the text line's own top is `raw + 3·prop(raw)`, and it is emitted while that is below 25.45.

| break pt | 2 | 4 | 5 | **6** | 7 | 8 |
|---|--:|--:|--:|--:|--:|--:|
| text line's top | 9.85 | 19.65 | 24.45 | **29.65** | 34.60 | 39.65 |
| text emitted? | yes | yes | yes | **no** | no | no |

### 3.2 26.2.4.2 does not discard an overflowing body — it lays it out normally and clips it

`build5.py` puts a 200 × 40 pt shape with zero insets in front of the reference; 31 of 31 arms
rendered, read with `readraw.py`.

- **Nothing moves.** On every overflowing arm (`of030` … `of060`) the lines that *are* emitted
  are at **byte-identical positions to the `-tall` twin**. The overflow changes no layout at all.
- **A clip rectangle equal to the shape appears**, `(56.7, 56.7, 200, 40)`, and only on arms
  whose body overflows. `of004` and `of020` fit and carry no clip.
- **A line is emitted iff its TOP is above the shape's inner bottom.** `multi` (eight one-word
  paragraphs, 12.80 pt each, 40 pt shape) emits **four** — tops 0, 12.8, 25.6, 38.4 — and the
  fifth, top 51.2, is absent; its clip is `(56.7, 95.1, 200, 1.6)`, the 1.6 pt sliver the
  straddling fourth line has left. `of050`/`of060` lose their third line (tops 41.90 and 47.70)
  and keep their second (29.10, 34.90). `of080`/`of120` emit nothing but the ruler, because the
  break line alone is 46.55 / 69.85 pt and the text line starts below the box — and they carry
  **no clip at all**, because nothing was painted to clip.
- **`vertOverflow` is inert.** `ov-overflow`, `ov-clip`, `ov-ellipsis`, `ov-absent` and
  `oa-t`/`oa-ctr`/`oa-b` are identical to one another in text, positions and clip rectangle; so
  are the witness's own `ovf-*` (fitting) and `ovf40-*` (overflowing) quartets. The attribute has
  **no effect of any kind**, and neither has `anchor` once the body overflows: all three collapse
  onto the top.

So the register's **O91b — "draw an overflowing shape body rather than discarding it" — is the
wrong rule, and so is its opposite.** The correct statement is:

> **O91b′** — an overflowing shape body is laid out at exactly the positions it would have had in
> a shape tall enough, painted under a **clip rectangle equal to the shape's inner rectangle**,
> and the only lines omitted are those whose top is already at or below the shape's inner bottom.
> `anchor` degenerates to `t`; `vertOverflow` is never consulted.

### 3.3 Which half costs more ink on the witness

**O91a costs all of it. O91b costs none of it, on this document.**

- With (a) corrected, the body is 23.65 pt against 25.45, the overflow path is never entered, and
  all 95 characters are drawn at the reference's own positions — the predicted centred baseline
  52.70 is the one 26.2.4.2 draws. **(a) alone closes the witness completely.**
- With (b) "corrected" to the register's wording — draw the overflow — and (a) left alone, the
  body is ~73 pt in a 25.45 pt box and the text lands roughly 24 pt below the shape. That is ink
  in the wrong place and, against 26.2.4.2, worse than drawing nothing.
- With (b) corrected to **O91b′** and (a) left alone: break line 1 occupies 0–14.8, line 2
  14.8–29.6 (top 14.8 < 25.45, emitted but empty), line 3 top 29.6 > 25.45, and the text line's
  top is about 59 pt — far below the box, so **not emitted**. Still 0 of the 95 characters.

**(b) is therefore not the larger half on the witness; it is not any of it.** It is a real and
separate divergence, and O91b′ is a *narrower* change than the register implies — a clip, not an
overflow — but its reach has to be argued somewhere other than `086`.

### 3.4 What could not be measured, and why

**This round did not render a single page through XRay.Office.** The parent session's build was
live in this tree throughout — MSBuild nodes resident and
`src/XRay.Office.Presentations/bin/Debug/net10.0/*.dll` written within the twenty minutes before
this was written, against a `XRay.Office.Cli` stamped 21:41 — so any figure from that binary would
describe a program that no longer exists, which is exactly the contamination `dotnet/CLAUDE.md`'s
*"before you rebuild, check the sweep is finished"* section records. The "14.8 pt each" and "this
tree draws none of it" are therefore the **register's** figures carried forward, not re-measured;
§3.3's three cases are arithmetic on those plus this round's measured reference behaviour. The
measurement that would settle them is one render of `fixtures/`, `fixtures2/`, `fixtures3/` and
`fixtures5/` through a *quiescent* `XRay.Office.Cli`, read with `readraw.py` — which is also the
only way to find out whether this tree already gets the page-body case (§2.5) right and is wrong
only in a shape.

## 4. The seat in the C++ tree [src] — and the brief's pointer is at the wrong engine

Read out of `/home/user/libreoffice-core`, which declares **27.2.0.0.alpha0+** and is *not*
26.2.4.2's source. Every claim here is confirmed independently by §2's measurements, which are.

### 4.1 A `.docx` shape body is a Writer fly, not an EditEngine text

`oox/source/shape/WpsContext.cxx`:895, :917 sets `mpShapePtr->setTextBox(true)`, and
`DomainMapper_Impl::PushTextBoxContent`
(`sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx`:6255-6272) answers it with
`m_xTextDocument->createTextFrame()` — a **`SwXTextFrame`**, appended into the body's text and
later bound to the shape by `AttachTextBoxContentToShape` (:6303). So the body of a `wps:wsp` is
ordinary Writer text in a fly frame, laid out by `sw/source/core/text/`.

**The brief's pointer at `editeng/source/editeng/impedit3.cxx` and
`RecalcFormatterFontMetrics` is therefore the wrong engine for this seat**, and
`dotnet/CLAUDE.md` says as much in its own `.docx`-hyperlink paragraph (*"in a paragraph or
inside a `wps` text box, whose content is a Writer fly's text and not EditEngine's"*). Three
measurements say the same thing independently: external leading is **in** the line height
(§2.2 — EditEngine's `IsAddExtLeading()` is false), the paragraph's `SvxLineSpacingItem`
proportional rule is honoured with a first-line exception (§2.4), and the overflow is a Writer
fly's clip (§3.2). The EditEngine rule is nonetheless the *same shape of rule*, which is why the
brief's instinct was right even though its file was not.

### 4.2 The four hunks

1. **`SwLineLayout::CalcLine`, `sw/source/core/text/porlay.cxx`:288-297.**

   ```cpp
   if( mpNextPortion )
   {
       SetContent( false );
       if( mpNextPortion->IsBreakPortion() )
       {
           SetLen( mpNextPortion->GetLen() );
           ...
       }
       else
       {
           const SwTwips nLineHeight = Height();
           Init( GetNextPortion() );
           ...                      // the whole max-over-portions loop
       }
   }
   ```

   **A line whose first portion is a break portion never enters the portion loop**, so its height
   is not recomputed from its portions. It keeps whatever the line already had. This is the exact
   Writer twin of the EditEngine rule `dotnet/CLAUDE.md` already records — *"`RecalcFormatterFontMetrics`
   runs over every portion of the line whose kind is not `LINEBREAK`"*.

2. **What the line already had: `SwTextFormatter::BuildPortions`, `itrform2.cxx`:398-399.**

   ```cpp
   if( !m_pCurr->GetAscent() && !m_pCurr->Height() )
       CalcAscent( rInf, m_pCurr );
   ```

   The `SwLineLayout` is seeded before any portion is built.

3. **`SwTextFormatter::CalcAscent`, `itrform2.cxx`:848, the general `else` branch at :888-935.**
   `bFirstPor` is `rInf.GetLineStart() == rInf.GetIdx()`, true here, so it calls `SeekAndChg(rInf)`
   — **seek the character attributes at the line's own start index** — and then

   ```cpp
   pPor->SetAscent( rInf.GetAscent() );
   pPor->Height( rInf.GetTextHeight() );
   ```

   `SwTextSizeInfo::GetTextHeight()` (`inftxt.hxx`:786) is `GetFont()->GetHeight(m_pVsh, *GetOut())`,
   which reaches `SwFntObj::GetFontHeight` (`sw/source/core/txtnode/fntcache.cxx`:336) — ascent +
   descent **+ `GetFontLeading`** (:379-434), the font's external leading whenever
   `DocumentSettingId::ADD_EXT_LEADING` is on. **That is the whole of the rule: the line takes the
   font in force at the line's first character, and for a `<w:br/>`-only line that character is
   the break, whose attributes are the break run's.**

4. **`SwBreakPortion::Format`, `sw/source/core/text/porrst.cxx`:204-214** then copies the line's
   own figures onto the portion —

   ```cpp
   Width( 0 );
   Height( pRoot->Height() );
   ...
   SetAscent( pRoot->GetAscent() );
   ```

   — which is why there is no independent "break portion height" to look for: the portion takes
   the line, and the line took the break run's font.

5. **The proportional exception, `SwTextFormatter::CalcRealHeight`, `itrform2.cxx`:2424-2437.**

   ```cpp
   // Note: for the _first_ line the line spacing of the previous
   // paragraph is applied in SwFlowFrame::CalcUpperSpace()
   if( !IsParaLine() )
       switch( pSpace->GetInterLineSpaceRule() ) {
           case SvxInterLineSpaceRule::Prop: {
               tools::Long nTmp = pSpace->GetPropLineSpace();
               ...
               nTmp -= 100;
               nTmp *= pTextHeightLine->GetLineSpacingBaseHeight();
               nTmp /= 100;
               nTmp += nLineHeight;
   ```

   Integer arithmetic, the surplus taken as `(prop − 100) %` of the base height and **skipped
   entirely on the paragraph's first line**. This predicts §2.4's whole table exactly: at
   `prop = 108`, `8×47/100 = 3` twips on a 2 pt break line (47 → 50 → 2.500 pt) and `8×256/100 = 20`
   on the 11 pt text line (256 → 276 → 13.800 pt), and `prop` must be **108** rather than 107 for
   the 276 to come out — which also fixes writerfilter's own `round(259/240 × 100)`.

**Nothing in this path consults the paragraph mark**, which is why §2.3's six mark sizes move
nothing.

## 5. Reach, counted by what the rule paints, with its base rate

`census.py` and `census-body.py`. Two things they do that a `grep` cannot: they **skip the
`mc:Fallback` VML twin** of every `mc:AlternateContent`, which carries a second copy of the same
`w:txbxContent` and doubles every figure (the witness holds 16 `<w:br/>` in the file and **8**
that LibreOffice reads), and they **resolve each size** through `rPr → rStyle → pStyle →
docDefaults` rather than reading the literal attribute. `w:br w:type="page"/"column"` are
excluded: they are not this rule.

### 5.1 Inside a shape body — `.docx` family, 271 documents

| | count |
|---|--:|
| **base rate** — live `w:txbxContent` bodies | **1642** |
| **base rate** — paragraphs inside them | **2944** |
| `<w:br/>` inside them | 319, in 13 documents |
| **the population: break run's size ≠ the paragraph mark's** | **265, in 7 documents** |
| shape bodies holding at least one such break | **112 of 1642** |
| ... whose extra height *alone* exceeds the box's inner height | **12** |
| ... characters inside those 12 bodies | **259** |
| characters inside all 112 differing bodies | 1137 |

| document | breaks | mark/run | extra | bodies | forced over |
|---|--:|---|--:|--:|--:|
| `065_Work_Breakdown_Structure_…_Blue_Theme` | 96 | 28/4 | +1340.9 pt | 19 | 2 |
| `066_Work_Breakdown_Structure_…_Colored_Background` | 56 | 32/4 | +912.6 | 28 | 0 |
| `067_Work_Breakdown_Structure_…_Gray_Theme` | 62 | 32/4, 44/4 | +1024.3 | 26 | 5 |
| `068_Work_Breakdown_Structure_…_Green_Theme` | 33 | 18/16, 32/16 | +46.6 | 33 | 0 |
| `069_Work_Breakdown_Structure_…_Professional_Format` | 4 | 28/22, 48/4 | +80.3 | 2 | 1 |
| `084_Printable_Graph_Paper_…_Editable_Layout` | 6 | 22/4 | +62.9 | 2 | 2 |
| `086_Printable_Graph_Paper_…_Gray_Theme` | 8 | 22/4 | +83.8 | 2 | 2 |

**259 characters in 12 bodies is the static upper bound and it is not the figure to quote.**
`probes/coverage-r136` *measured* the text actually lost at **86 characters in 2 documents**
(`086` 10 against 95, `084` 10 against 21). This round did not re-measure that — §3.4 — so the
measured figure stands and the census's 259 is the ceiling it sits under. The other five
documents' 253 differing breaks move text without losing it.

### 5.2 Outside a shape — the same rule, a different population

§2.5 measures the rule as **identical** in a page-body paragraph, so a fix does not touch only
`w:txbxContent`. Over the same 271 documents:

| | count |
|---|--:|
| **base rate** — page-body paragraphs | **163 342** |
| `<w:br/>` in them | 4770 |
| **break run's size ≠ the paragraph mark's** | **132, in 28 documents** |

Four times as many documents as the shape population, and the cost per occurrence is smaller
(nothing overflows out of a page). **Whether this tree is wrong there too could not be measured**
(§3.4), and it is the first thing to check, because it decides whether the fix is in the shape
path or in the shared line-height path.

### 5.3 The converted ODF column

| column | files | base rate | `text:line-break` | of those, in a drawing shape |
|---|--:|--:|--:|--:|
| `/home/user/corpus-odf/odt` | 337 | 210 828 `text:span` | 5130 in 100 documents | **177 in 13 documents** |
| `/home/user/corpus-odf/ods` | 307 | 37 195 `text:span` | 40 in 4 documents | 30 in 2 documents |
| `/home/user/corpus-odf/odp` | **0** | 0 | 0 | 0 — **absent, and 0 base-rate tokens is the tell** |

The shape has to be searched for as **any `draw:*` element**, not `draw:frame`: LibreOffice's own
converter writes these boxes as `draw:custom-shape`, and a census keyed on `draw:frame` answers
**0 of 337** with a healthy base rate, which looks like a real negative and is not. The witness's
own ODT twin is the shape — `<draw:custom-shape …><text:p><text:span text:style-name="T3">`
with four `<text:line-break/>` inside the 2 pt span and the text in a second span. **The ODF form
carries the size on the span the break is in**, so the rule maps directly, and it is the same
family as `dotnet/CLAUDE.md`'s already-recorded *an empty ODF paragraph is as tall as its own
empty `text:span`*.

*Correction to the brief*: `/home/user/corpus-odf/rtf` **does** exist, with 337 files. It is not
an ODF package, so the reader above reads none of them; its analogue is `\line` inside a
`\shptxt`, on a different importer, and it is out of this round's scope.

## 6. The C# seat — and O91b is a phantom

### 6.1 O91b does not exist. The rule is already implemented, and it is already right.

`FlowLayouter.Truncated` (`dotnet/src/XRay.Office.WordProcessing/Layout/FlowLayouter.cs`:430),
called from `FrameLayout.Content` (`FrameLayout.cs`:1250-1266) for every
`PageFrame.HasFixedHeight`, is:

```csharp
foreach (PlacedLine line in flow.Lines)
{
    if (line.Top < height) lines.Add(line);
}
…
// The first thing always survives.
if (lines.Count == 0 && tables.Count == 0) { … lines.Add(flow.Lines[0]); … }
```

**That is exactly O91b′ as §3.2 measured it at 26.2.4.2** — keep a line whose top is strictly
less than the content height, drop the rest, and always keep the first. Its own remarks cite the
sixty authored boxes of `probes/words-extra-01/` and say so.

It also **predicts the register's k-sweep exactly**. With 14.8 pt break lines and a 25.45 pt
inner height, the line tops are 0, 14.8, 29.6, 44.4 and the text line's is 59.2:

| k | line tops | last top < 25.45 | text drawn? | register observed |
|--:|---|---|---|---|
| 0 | text at 0 | yes | yes | **yes** |
| 1 | 0, text at 14.8 | yes | yes | **yes** |
| 2 | 0, 14.8, text at 29.6 | no | no | **no** |
| 3 | 0, 14.8, 29.6, text at 44.4 | no | no | **no** |
| 4 | 0, 14.8, 29.6, 44.4, text at 59.2 | no | no | **no** |

So *"when the body then exceeds the shape this tree draws none of it"* is not a second rule. It
is the correct rule, applied to a body that O91a made three times too tall. **O91b should be
struck from the register and O91a left as the whole of the row.** The one thing O91b′ adds that
this tree does not do is the *clip* — 26.2.4.2 also emits a clip rectangle equal to the shape's
inner rectangle, so a line it keeps that straddles the bottom edge is drawn cut in half — and
that is cosmetic beside the truncation, which both sides already agree on.

### 6.2 The change O91a needs, and the honest state of its seat

The rule to hold, with §2's measured constants:

```
a line whose only content is a manual break takes
    height = ascent(F, s) + descent(F, s) + externalLeading(F, s)
where F, s are the FONT AND SIZE OF THE RUN CARRYING THE BREAK,
each of the three terms rounded to a twip,
and then the paragraph's line-spacing rule applies to it exactly as to any other line:
    unchanged                                      on the paragraph's first line
    height + trunc((prop - 100) * height / 100)    for `auto`, prop = round(w:line/240*100)
    w:line twips                                   for `exact`
    max(height, w:line twips)                      for `atLeast`
```

On the witness that is **2.350 pt for the first break line and 2.500 for each of the other
three**, 10.850 pt for the four, against the 14.8 pt each the register records.

**The seat is `MeasuredParagraph.MeasureLine`
(`dotnet/src/XRay.Office.Text/Layout/MeasuredParagraph.cs`:778) and its `Fold`
(`:~960`), reached through `ParagraphLayouter.BandHeight`
(`dotnet/src/XRay.Office.Text/Layout/ParagraphLayouter.cs`:412) —** and this round could not
confirm that it is where the defect is, which is the round's largest gap. Read statically, that
path already answers the break run:

- `DocxLayoutSource.Emit` (`Ooxml/DocxLayoutSource.cs`:2215-2237) appends the U+2028 **under
  `_runProperties`**, so the break gets a `StyledRange` at `w:sz="4"`;
- `size != paragraph.Size` sets `varies` (`:1245`), so the uniform-paragraph shortcut does not
  discard it and a `PageRun` at 2 pt reaches `PageContent.Measure` (`Layout/PageContent.cs`:688);
- `HoldsNothingButAnchors` tests U+0001 only (`PageContent.cs`:769) and `IsAllBlanks` tests tab
  and five space characters only (`MeasuredParagraph.cs`), so neither rewrites the break run to
  the paragraph's size nor makes it transparent;
- and `Fold`'s `contains = start == end && run.Run.Covers(start)` is written for exactly this
  case — a break-only line whose `VisibleEnd` equals its `Start` — and `Covers` is
  `index >= Start && index < End`, which the break run satisfies.

**So the three places that could take the paragraph mark's size all appear to take the run's, and
the register's diagnosis is not reproduced by reading the code.** Something between the reader
and the line box is still answering 14.8 pt, and 14.8 is suggestive — 11 pt DejaVu Sans is 12.80,
the witness's 108 % makes it 13.80, and 14.80 is one further 1.00 pt surplus on top of that,
which is the *same* surplus applied twice. That is a hypothesis, not a finding.

**The measurement that settles it, and it is one render:** with the build quiescent, render
`fixtures3/n0.docx … n6.docx`, `b02 … b80`, `m002 … m120` and `pr259-n0 … n4` through
`XRay.Office.Cli` and read them with `readraw.py`. Those arms are one attribute apart and their
26.2.4.2 answers are in §2, so the shape of the disagreement names the seat outright:

- `n0..n6` flat at 2.350 and `b*` on the 1.164-em slope → the rule is right and the defect is in
  the witness's own path, not the shared one;
- `m002 … m120` **moving** → the paragraph mark is being consulted, and the seat is whichever of
  the four bullets above is not doing what it reads as doing;
- `n*` right but `pr259-n*` wrong → the seat is the proportional surplus in
  `LineSpacing.Apply` / `ParagraphLayouter.HeightOfLine`, not the break at all — which is what
  the 14.80 = 13.80 + 1.00 arithmetic points at;
- `p0/p1/p2` (the page-body twin) wrong as well → the seat is shared and §5.2's 132 breaks in 28
  documents are in scope too.

No change is made here.

## 7. What this round could NOT establish

1. **Nothing was measured on this tree's own output.** The parent's build was live throughout
   (§3.4). Every "this tree" figure above is the register's, carried forward. In particular
   **the register's central claim — that the break line takes the paragraph mark's size — is
   confirmed as 26.2.4.2's *opposite* but is not confirmed as this tree's behaviour**, and §6.2
   reads the C# as already taking the run's size. The one render that settles it is specified
   there, arm by arm.
2. **Where the 14.8 pt comes from.** Two candidates are live and this round cannot separate them:
   (a) the break line is measured at the paragraph's size somewhere between
   `DocxLayoutSource.Emit` and `MeasuredParagraph.Fold`; (b) the break line is measured
   correctly and the *proportional surplus* is applied twice, which would make 12.80 → 13.80 →
   14.80 and would also be wrong on every other line of every 108 % paragraph. The
   discriminator is `m002 … m120` against `pr259-n0 … n4`: (a) moves the first family, (b) moves
   the second, and neither moves the other.
3. **Whether the page-body population (§5.2, 132 breaks in 28 documents) is affected.** The
   reference treats it identically (§2.5); this tree was not measured. `p0/p1/p2` answer it.
4. **The reach of the *clip* half of O91b′.** This tree truncates by line, which is the rule, but
   does not emit 26.2.4.2's clip rectangle, so a kept line straddling the shape's bottom is drawn
   whole where the reference draws it cut. A static census cannot find those — it needs a body
   height — and no rendering was available to find them dynamically.
5. **One twip at 1 pt.** §2.1's model predicts 24 twips for a 1 pt break line and 23 are drawn.
   Separate rounding of ascent and descent fixes the 12 pt and 30 pt residuals and not this one.
   0.05 pt; not pursued.
6. **The `.doc`, `.rtf` and `.ppt` twins.** The corpus holds 66 `.doc` and 51 `.ppt`, and
   `/home/user/corpus-odf/rtf` holds 337 files. None was censused: a WW8 text box's break is a
   `Special.LineBreak` on a different reader
   (`Ww8DocumentReader.Layout.cs`:797) and an RTF's is `\line` inside a `\shptxt`, and whether
   either shares the seat is unknown.

## 8. Recommended edits to the register (not made here)

- **Strike O91b.** "*When the body then exceeds the shape this tree draws none of it*" is not a
  defect: `FlowLayouter.Truncated` already implements 26.2.4.2's own line-by-line rule, and the
  witness's k-sweep is predicted by it exactly (§6.1). Replace it, if anything, with a much
  narrower **O91b′ — the clip** (§3.2, §7.4), whose reach is uncensused.
- **Correct O91a's arithmetic.** "*26.2.4.2 spends 10.8 pt on all four, 2.7 pt each, which is 2 pt
  at the paragraph's 1.35 spacing*" — the 10.8 is right (10.850 measured today) and the rest is
  not: the four lines are **3.350 + 2.500 + 2.500 + 2.500**, the spacing is the document's stated
  `w:line="259"` = 108 %, and the factor on the break run's own 2 pt is DejaVu Sans'
  **1.1640625 em**, with the 108 % surplus falling on the *text* line and skipping the
  paragraph's first line. `1.35` is 2.7/2 and is an artefact of averaging four unequal lines.
- **Add the base rate to the reach.** 265 differing breaks in 7 documents sit against 1642 live
  shape bodies and 2944 paragraphs in them; the file count is **8** breaks in the witness, not
  16, once the `mc:Fallback` twin is excluded (§5.1).
- **Record the instrument warning** (§2.6): PyMuPDF's text extraction honours clip paths, and
  26.2.4.2 clips an overflowing shape body. Reading an overflow arm with `get_text` reports text
  the reference did in fact draw as missing.
- **Correct `dotnet/CLAUDE.md`'s pointer for this family**: a `.docx` `wps:wsp` body's line
  heights are `sw/source/core/text/` (`SwLineLayout::CalcLine`, `SwTextFormatter::CalcAscent`,
  `SwBreakPortion::Format`, `SwTextFormatter::CalcRealHeight`), not
  `editeng/source/editeng/impedit3.cxx`. The file already says so in its `.docx`-hyperlink
  paragraph; O91's brief sent this round to EditEngine anyway.

## 9. Files

| file | what it is |
|---|---|
| `mutate.py` → `fixtures/`, `mutations.txt` | 28 one-attribute variants of the witness |
| `mutate2.py` → `fixtures2/`, `mutations2.txt` | 49 arms sweeping the break size across the overflow threshold, at three anchors, with a tall control for each |
| `build.py` → `fixtures3/` | 51 hand-built minimal DOCX, each with its own `word/settings.xml` |
| `build4.py` → `fixtures4/` | 19 arms for the proportional-line-spacing rule |
| `build5.py` → `fixtures5/` | 31 arms for what an overflowing body does |
| `render.sh` | one arm at a time through 26.2.4.2, `timeout -k 30 900`, its own `-env:UserInstallation=`, `FAIL` printed for any arm with no output |
| `render.txt` … `render5.txt` | 178 arms, **178 `ok`, 0 `FAIL`** |
| `read.py`, `read3.py` | span-level reads (clip-honouring — see §2.6) |
| **`readraw.py`** | the clip-blind content-stream reader every §3 figure is taken with |
| `measured*.txt`, `measured*-raw.txt` | the readings |
| `fontmetrics.py`, `fontmetrics.txt` | the four faces' `hhea`/`OS/2` tables against the drawn heights |
| `census.py`, `census.txt` | the shape-body reach with its base rate |
| `census-body.py`, `census-body.txt` | the page-body population and the converted ODF columns |

All 178 renders are of `/opt/libreoffice26.2/program/soffice`, 26.2.4.2,
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. Batches 4 and 5 were rendered twice, on the second
pass to record `render4.txt`/`render5.txt`, and every figure came back identical.

---

## 9. Note on what is banked here

**The 178 rendered PDFs are not committed.** They were 6.4 MB — the bulk of this directory — and
every one is reproducible from the fixtures beside them with `render.sh`, which refuses to report an
arm whose output does not exist. What *is* banked is the evidence a later round would actually read:
the fixtures each arm was built from (`fixtures`…`fixtures5`, built by `build*.py`), the readers
(`read.py`, `read3.py`, and `readraw.py`, the clip-blind content-stream parser §7 explains you must
use), the measurements they produced (`measured*.txt`, `measured*-raw.txt`), the render logs
(`render*.txt`, every line `ok`), the censuses and this write-up.
