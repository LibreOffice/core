# L1 — text metrics, shaping and line breaking

**The question this lane was given:** 107 of the 192 non-matching documents are tagged
`reflow`. How many of them are the known advance-width divergence, and how many are
something else?

**The answer, measured:** the advance divergence is the **only** measurable difference in
**17 of the 107 (16%)**, and it is quantitatively sufficient to explain the observed
line-break disagreement in **12 of those 17 (11%)**. In the other 90 the line breaks move —
or do not move at all — for reasons between 5 and 250 times larger than the advance
divergence, and every one of those reasons is measurable from the PDF pair. The largest
single group, **40 of 107 (37%), breaks every shared line at exactly the same word**: those
documents have no line-breaking fault of any kind and their divergence is vertical.

---

## 0. Which reference these numbers are against — read first

Every measurement below was taken from the PDFs in `/data/bench/lo/` and `/data/bench/pl/`.
The reference half carries `Producer: LibreOffice 24.2`, and `soffice --version` in this
container is **24.2.7.2**. The X-Ray.Office tree is developed against **26.2.4.2** —
`dotnet/CLAUDE.md`:546 says so, `dotnet/TODO.24-2-7-audit.md` is the list of 50 sites whose
calibration moved with it, and `dotnet/probes/fidelity-01/results.md`:38-39 records 40 of
550 fidelity cases changing verdict across the version alone.

**Two of the root causes below are that version gap and not defects**, and both are
identified with the file and the line that says which binary it was measured on (§4 and §5).
I have proposed no patch against either, and neither should be "fixed" to match this sweep's
reference. The advance divergence itself (§3) is version-relevant in its own right: the
per-face ratios in §3 are **ours against 24.2.7.2**, not against the 26.2.4.2 figures in
`dotnet/CLAUDE.md`, and they agree with those figures to within the measurement's own noise
— so the divergence survives the version move.

---

## 1. The instruments, and the two traps that bit

Everything below comes from four measurements over the PDF pair, on a three-page window
centred on each case's stated divergent page. The scripts are in `work/` (`measure.py`,
`pass3.py`, `pass4.py`, `pass5.py`, `census2.py`, `predict.py`); the per-case output is
`work/reflow-measured.json` and `work/census.txt`.

1. **Advance ratio.** The two pages' whole character streams are aligned with `difflib`, so
   reflow cannot break the alignment. For every word both stacks set, the width is taken
   from the pen origins *inside* the word — first origin to last origin plus the last
   glyph's advance. Summing inside words only means justification, which stretches blanks
   and not glyphs, cannot contaminate it. 296,847 glyphs aligned across the 107.
2. **Break agreement.** For every word the two share, whether it begins a line on each side:
   `both / max(ref_starts, our_starts)`. This is the discriminator the lane brief asked for.
   It separates "lines break in different places" from "lines break identically and
   something vertical moved".
3. **The measure, bracketed.** A wrapped line's right edge is a width the breaker
   *accepted*; that edge plus a space plus the first word of the continuation line is a
   width it *rejected*. Over a page the two bracket the breaker's own limit to within one
   word — tighter than any quantile of right edges, and indifferent to whether the text is
   justified. Only continuation pairs are used (same block, same left edge, one leading
   apart), and a "rejection" narrower than a demonstrated acceptance is dropped as a false
   pair.
4. **Rendered em size and resolved face**, per aligned word, from the PDF's own spans.

**Trap 1 — the trailing space.** LibreOffice keeps a line's trailing space in the PDF and we
do not, so the reference's last glyph box is up to a space wider. Comparing right edges
without dropping it read as a systematic **−3 pt of measure on 40 documents that have no
measure difference at all**. Dropped, those 40 go to zero.

**Trap 2 — a block translated is not a column resized.** Case #010's whole page sits
**37.2 pt** right of the reference's at exactly the same width; comparing absolute right
edges called that a 36 pt column-width fault. Every edge is now taken relative to the page's
own modal left edge, and #010's measure gap is 0.00 — as it must be, since all 113 of its
shared line starts agree.

Both traps produced confident, wrong numbers before they were found. Both are guarded in the
scripts with the case that found them named in the comment.

---

## 2. The census

107 reflow-tagged documents, each attributed to the **largest measured** difference between
the two renderings. Full table: `work/census.txt`.

| class | n | % | what it means |
|---|---:|---:|---|
| **vertical** | 40 | 37% | every shared line breaks at the same word; no line-breaking fault exists |
| **font-size** | 26 | 24% | the same text is rendered at a different em size |
| **advance-band** | **17** | **16%** | same face, same size, same measure — only the advances differ, by under 0.4% |
| **face** | 8 | 7% | the two stacks resolved different faces |
| **measure** | 5 | 5% | the column width itself is provably different |
| **no-text** | 5 | 5% | graph-paper grids; under 150 aligned glyphs, nothing to measure |
| **leading** | 4 | 4% | line pitch differs |
| **advance-large** | 2 | 2% | advances differ by 0.67%, six times the documented band |

The ranking is by size of effect, and the sizes are not close. On a 460 pt measure the
advance divergence displaces a line end by **0.2–0.5 pt**; the measure faults displace it by
**9–65 pt**, the face faults by **2–27%** of the line, and the vertical group by nothing.

---

## 3. Root cause A — the advance-width divergence · 17 of 107 (16%)

### What the pages show

`#032 #036 #037 #043 #044 #045 #064 #073 #078 #081 #093 #107 #126 #155 #170 #183 #185`
(one in this lane, six in L2, three in L3, five in L4, one in L5, two in L6). In every one
the two stacks resolve the same faces at the same sizes, the bracketed measure agrees to
under 3 pt, the line pitch is identical, and between 0.6% and 12% of line starts disagree.

### What it measures, on this corpus, against 24.2.7.2

Word widths over every case where both stacks set the same face at the same size —
**296,847 glyphs**, ours ÷ reference:

| face (both sides) | cases | glyphs | ours/ref | pt per glyph |
|---|---:|---:|---:|---:|
| Carlito-Bold | 29 | 7,145 | **1.00125** | +0.0066 |
| Carlito-Regular | 36 | 63,086 | **1.00108** | +0.0053 |
| Carlito-Italic | 5 | 1,812 | 1.00074 | +0.0036 |
| DejaVuSans | 40 | 35,359 | **1.00069** | +0.0039 |
| DejaVuSerif | 9 | 4,578 | 1.00061 | +0.0059 |
| LiberationSans | 58 | 76,587 | 1.00009 | +0.0005 |
| DejaVuSans-Bold | 28 | 3,836 | 1.00004 | +0.0004 |
| LiberationSans-Italic | 9 | 4,257 | 0.99994 | −0.0003 |
| LiberationSerif-Bold | 16 | 4,581 | 0.99970 | −0.0015 |
| LiberationSerif | 32 | 74,375 | 0.99958 | −0.0019 |
| WenQuanYiZenHei | 3 | 1,261 | 1.00003 | +0.0003 |
| LiberationSans-Bold | 56 | 16,993 | 0.99894 | −0.0065 |
| LiberationSerif-Italic | 10 | 2,977 | 0.99730 | −0.0123 |
| **all** | | **296,847** | **1.00019** | **+0.0010** |

This **confirms `dotnet/CLAUDE.md`'s seat and sharpens it**. Its figures were Carlito
+0.115% and Liberation Serif +0.011%, measured on authored probes; on real corpus documents
against 24.2.7.2 the same two faces come out at **+0.108%** and **−0.042%**. The face
dependence is real and it is an order of magnitude, exactly as recorded. Two faces the notes
do not cover behave like the two poles: **DejaVu Sans accumulates (+0.069%)** and
**Liberation Sans does not (+0.009%)**.

The corpus-weighted figure is the number worth carrying forward: **+0.019%, one thousandth
of a point per glyph** — about **0.09 pt per line** on the commonest measure in the corpus.

### Does it actually move the breaks?

A line breaks somewhere else when the width our glyphs add over the line exceeds the slack
the reference left at the end of it. The slack is roughly uniform over the width of the next
word, so the chance a given line moves is `(ratio − 1) × measure / mean word width`. One
moved break restarts every later line of its paragraph, so the share of *line-start words*
that disagree is a small multiple of that. Per case (`work/predict.py`):

- median predicted per-line move probability **1.23%**
- median observed line-start disagreement **2.22%**
- median observed ÷ predicted **1.6×** — the paragraph-restart multiplier, and the right
  order.

**12 of the 17 land between 0.3× and 3.6×**, as close as this model can resolve. The advance
divergence is a sufficient explanation for those twelve and for nothing else in the sweep.

**Five do not** — `#045` 23×, `#081` 18×, `#107` 13×, `#126` 29×, `#155` 5.6×. They disagree
far more than their advance ratio can produce, so each has a second cause these four
instruments do not see. I did not establish what it is. They are the honest residue.

### The proposed change

**None, and deliberately.** `dotnet/CLAUDE.md` already establishes that ours is exactly
`hmtx × size / upem` on every glyph tested while the reference grid-fits the outline per
glyph, and that no quantisation grid from 16 to 4000 units reproduces it. My measurement
adds two faces to that picture and does not disturb it. Closing it means reproducing
FreeType's hinted advance at LibreOffice's ppem inside a stack that reads its own OpenType
tables and shapes with HarfBuzz scaled to upem
(`dotnet/src/XRay.Office.Text/Shaping/HarfBuzzShaper.cs`; `XRay.Office.Text/TODO.md` — "advances
come back on the design grid unrounded"). That is an architectural change to the metrics
source, not a rounding patch, and a patch pretending otherwise would be a factor tuned to
one sample — which `AGENTS.md`'s OOXML policy forbids and which the refuted grid hypothesis
already shows is unavailable.

The useful conclusion is the size of the prize: closing it perfectly would address 17 of 192
non-matching documents, of which 12 would be expected to resolve.

### The probe that would refute me

Re-break the 17 with the reference PDF's own per-glyph advances as the metric source. If the
line starts then agree at better than 99.5% for all 17 the attribution is complete; if the
five outliers still disagree, their second cause is confirmed as something else. Cheaper
falsifier for the group: any one of the 17 whose disagreeing breaks sit on lines where the
next word is more than 2 pt from the margin — the advance cannot move those, and one such
line refutes the attribution for that document.

**Confidence: high** for the ratios (296,847 glyphs, two independent traps found and
closed); **medium-high** for the 12/17 attribution (the slack model is first-order and the
amplification factor is estimated, not measured).

---

## 4. Root cause B — the rendered em size differs · 26 of 107 (24%) · mostly a version divergence

### What the pages show

`#001 #015 #030 #047 #049 #066 #071 #076 #083 #086 #096 #099 #103 #104 #106 #110 #119 #122
#125 #128 #132 #149 #168 #171 #177 #181`. Between 8% and 100% of aligned glyphs are drawn at
a different em size from the reference's — almost always by exactly **one point** (17→18,
15→16, 26→24, 23→22). Once the comparison is restricted to words both stacks set at the
*same* size, the advance ratio in this group is **1.0000–1.0018** — §3's band. The whole
2–12% width difference is the size, not the advances.

**17 of the 26 are slide decks**, and every `.pptx` among them carries
`<a:normAutofit fontScale="…">` on the affected placeholders (unzipped from
`ppt/slides/slideN.xml`; values 25000, 62500, 70000, 77500, 85000, 90000, 92500).

### Where it lives, and why it is not a defect here

`dotnet/src/XRay.Office.Presentations/Layout/SlideAutofit.cs`:31-38 —

> *"LibreOffice 25.2 replaced the bisection with a walk down a fixed [ladder]
> (`constScaleLevels`) … [24.2.7-audit: FIXED 2026-08-20, round slides-r52 — was WRONG: 25.2
> replaced the bisection with constScaleLevels; worth −155.40 abs_ink, −11.1% of the slides
> track.] What stood here for thirty rounds was the bisection of 24.2.7.2"*

and :87 *"Measured against the installed 26.2.4.2, not read out of the tree."*

So X-Ray.Office deliberately implements **26.2.4.2's** autofit ladder and this sweep's
reference is the **24.2.7.2** bisection it replaced. Two different search procedures landing
one ladder step apart is exactly the ±1 pt signature measured. **Ours is right for the
project's reference binary; the sweep's reference PDFs are from the superseded one.** Do not
re-tune this to match them — round 52 already paid −155.40 `abs_ink` to move it the other
way. This is L5's and L8's lane; I am reporting the measurement, not proposing the change.

### The nine that are not slides

`#001 #015 #066 #106 #177 #181` (plus `#049 #076` on the L8 boundary) are word-processing and
spreadsheet documents where a rendered size still differs, and autofit does not apply.
**`#106` is mine and is a genuine defect — §8.** The others belong to their lanes.

### The probe that would refute the version story

Render any one of the 17 autofit decks through **26.2.4.2** and read the placeholder's em
size out of the PDF. If it matches ours the divergence is the binary; if it matches the
24.2.7.2 reference, `SlideAutofit.cs` is wrong on both and round 52's finding needs
revisiting. I cannot run it — 26.2.4.2 is not installed in this container.

**Confidence: high** that the size differs and that it, not the advances, drives the width
(direct measurement of both halves). **Medium-high** that the autofit ladder is the
mechanism: the file states its own calibration, every affected deck carries `normAutofit`,
and the gap is one ladder step — but I could not run the two binaries side by side.

---

## 5. Root cause C — a different face is resolved · 8 of 107 (7%) · part version divergence, part defect

### What the pages show

| case | reference resolves | we resolve | width |
|---|---|---|---:|
| #031 `1447.doc` | LiberationSerif | DejaVuSerif | +27.4% |
| #101 `May 25 bulletin…docx` | LiberationSans | DejaVuSans | +14.2% |
| #019 `2024-12_Comlux…docx` | LiberationSans | DejaVuSans | +13.1% |
| #135 `…Lease-Transition…xlsx` | DejaVuSans | DejaVuSerif | +2.5% |
| #140 `Company-profile-2022-EN.docx` | DejaVuSans | DejaVuSerif | +1.8% |
| #016 `644730BRI…doc` | DejaVuSans | DejaVuSerif | +1.6% |
| #097 `How-to-Write-an-Architecture…docx` | DejaVuSans | DejaVuSerif | +1.3% |
| #003 `AAC-AD-No-2021-01…doc` | Carlito | LiberationSerif | shape differs, width does not |

Restricted to words both stacks set in the same face, the advance ratio in this group is
1.0004–1.0010 — §3's band again. The reflow is entirely the substitution.

### What the documents contain

`#101` (mine): `word/document.xml` names **Helvetica** on 99 runs and `word/fontTable.xml`
declares it `<w:family w:val="swiss"/>`. `#097`: no `w:ascii` at all — the theme's
`<a:latin typeface="Aptos"/>`, which nothing on this machine has. `#140`: **Segoe UI**,
declared `swiss`. `#019`: **Arial** on 39 runs plus the theme's **Aptos**.

### Where it lives

`dotnet/src/XRay.Office.Text/Fonts/SystemFontResolver.cs`:488-527 (this lane's file). The
declared generic is consulted **before** LibreOffice's `SubstFonts` chain, and the comment at
:490-499 states why and against what:

> *"`FontConfigManager::Substitute` … is the pre-match substitution: it runs before
> LibreOffice consults `VCL.xcu` at all … **Measured against 26.2.4.2** with authored
> one-paragraph documents on the four names that can tell the two orderings apart, because
> each has a chain entry that *is* installed: **`Times`, `Helvetica`, `Albany` and
> `Thorndale` all answer DejaVu once a class is declared, where the chain answers
> Liberation.**"*

That is precisely the pair `#101` and `#031` exhibit, named in the comment, in the direction
observed. `FontSubstitutions.Tables.cs`:138 carries the chain
(`helvetica → albanyamt, albany, liberationsans, arial, …`), and the chain is what 24.2.7.2
followed. **So #101 and #031 are the same version divergence as §4, in this lane's own file,
and I am not patching it.** Re-ordering those two branches would answer this sweep's
24.2.7.2 reference and regress the 26.2.4.2 the tree targets.

`#019` is **not** covered by that story and looks like a real defect: Arial is a *strong*
metric alias and :506-517 exists to let it beat the generic
(`ClaimsEquivalenceWith(candidate, request.FamilyName)`), so Arial should resolve to
Liberation Sans on both binaries. It resolves to DejaVu Sans. The likely path is that the
affected runs are theme runs naming **Aptos** rather than the 39 Arial runs — which would
make `#019` the same case as `#097`/`#140`: an unknown family with a declared class landing
on a different generic from the reference's. I did not separate the two; `#019` is L3's
document.

### The proposed change

**None from this lane.** Two of the eight are the documented version divergence; the other
six turn on what LibreOffice does with an *unknown* family (`Aptos`, `Segoe UI`), and I
could not establish that against 24.2.7.2 without rendering probes, which belong with
whoever owns the affected corpus documents.

### The probe that would refute me

One authored `.docx` per name — `Helvetica`, `Aptos`, `Segoe UI`, each declared `swiss` in
`fontTable.xml`, one paragraph of known text — rendered through both 24.2.7.2 and 26.2.4.2,
with the face read out of each PDF by `pdffonts`. That separates "the version moved" from
"we are wrong on both" in four renderings. `fc-match` is **not** a substitute:
`dotnet/CLAUDE.md`:580-590 records it disagreeing with the installed LibreOffice on 8 of 296
corpus families, all of them punctuated names.

**Confidence: high** on the measurement; **high** that #101/#031 are the version divergence
(the file names the exact fonts and the direction); **low** on the mechanism for the four
`DejaVuSans → DejaVuSerif` cases.

---

## 6. Root cause D — the column width is provably different · 5 of 107 (5%)

`#046 #091 #108 #117 #129`. The bracket instrument proves the two breakers had different
limits — our accepted widths exceed every width the reference refused, or the reverse:

| case | proven gap | direction |
|---|---:|---|
| #091 `b053-19.docx` | **−65.1 pt** | our column is narrower |
| #117 `03_Technical_Report…docx` | −58.5 pt | ours narrower |
| #129 `iep-amount-frequency…ppt` | −28.9 pt | ours narrower |
| #046 `mde087077~283.docx` | +20.9 pt | ours wider |
| #108 `10795.doc` | −8.9 pt | ours narrower |

These are 20–250 times the advance divergence's 0.2–0.5 pt per line, so on these documents
the advance is not a contributing cause at all. Only `#091` is mine; §8 has its markup and
its arithmetic. The other four belong to L3, L2, L5 and L4 and I have not investigated them
beyond the number.

**Confidence: high.** The bracket is a proof, not an estimate: ours filled a line to a width
the reference demonstrably refused on the same page.

---

## 7. Root cause E — the line breaks are identical · 40 of 107 (37%)

`#011 #012 #017 #021 #023 #025 #028 #034 #038 #039 #041 #058 #061 #070 #075 #077 #082 #085
#087 #089 #102 #105 #109 #115 #116 #121 #123 #127 #134 #143 #144 #147 #151 #159 #160 #178
#179 #182` and two more. Every word that starts a line in the reference starts a line in
ours, over **6,469 line starts** in total — 196 of them on `#093`, 235 on `#155`, 241 on
`#038`.

**These documents have no text-metrics fault.** Their pages diverge vertically: where the
page breaks, how tall a row is, how much space a paragraph gets. Several of their case notes
attribute the divergence to "the text measure is wider"; the measurement says the measure
agrees to under 3 pt and the breaks agree exactly. That reading should not be acted on.
`#017` is mine and is the clearest instance — §8.

**The probe that would refute me:** break agreement is computed over the words the two pages
*share*, so a page where we dropped half the content could show 1.000 on the half that
remains. The guard is the line-start count in `work/census.txt`; the three cases with under
25 shared line starts (`#058` 4, `#178` 16, `#160` 20) are where this bucket is weak
evidence, and I would not defend those three individually.

**Confidence: high** for the 37 with more than 25 shared line starts; **low** for the other
three.

---

## 8. This lane's six documents

### #064 · `09.docx` — the advance divergence, and the only one of the six that is

Same faces (Liberation Sans 8/12/22 pt, Carlito 11 pt), same sizes; measure bracket
`[526.6, 539.5]` against ours `[527.4, 540.3]` — agreement to 0.8 pt; line pitch identical;
line-start offset a constant −0.100 pt. Advance ratio **1.00068** over 2,669 glyphs. 38 of
41 shared line starts agree; the three that move are in the justified Spanish paragraph, and
0.30 pt accumulated over a 442 pt measure is enough to move them.
**Verdict: root cause A. No patch.** The case note's "the text block is set wider and
shifted right" is not measurable: the left edges differ by 0.1 pt and the measures by 0.8 pt.
(Our infographic is drawn slightly smaller than the reference's — a separate, untagged
defect, and not mine.)

### #017 · `healthcare-reform-nsa-provider-bulletin.docx` — no text-metrics fault at all

- Break agreement **112 of 112**. Every line breaks at the same word.
- Line pitch identical: 13.8 pt within a paragraph, 27.6 pt between, on both sides.
- Measure brackets identical to 0.8 pt; measure gap 0.00.
- Advance ratio 1.00035.
- And yet: **page 1 holds 37 lines on both; page 2 holds 36 against our 38; page 3 holds 40
  against our 42** — with the same first-line top (54.4 pt) and last-line top (731.1 pt) on
  every page of both.

Same lines, same pitch, same band, two more lines per page from page 2 on. The difference is
therefore **the empty paragraphs**: the reference spends two 27.6 pt gaps that we do not,
which is what a page-boundary rule about a blank paragraph's space looks like.
**Verdict: vertical page filling — L2's `XRay.Office.WordProcessing/Layout` paginator, not
text metrics.** The case note's "the text measure is wider … and the space between list
items larger" is half right: the spacing differs, the measure does not.

### #027 · `Form-SM-76A…docx` — table row height

Carlito on both sides at the same sizes; face-matched advance ratio **1.00155**, inside §3's
band. Break agreement 0.992 over 124 line starts. Measure gap 0.00. Line pitch **+0.5 pt** on
our side. The rendered pair shows our table columns marginally wider and every row a shade
taller, which is what pushes `OPS 2A.422` onto page 2.
**Verdict: table row height / cell padding — L2.** The residual 5% of glyphs much wider than
the reference's are the checkbox and crest glyphs, a substitution on a symbol run: small, and
not what moved the row.

### #091 · `b053-19.docx` — a section-margin fault of 65 pt

The proof, from page 1 of each: the reference fills body lines out to **577.5 pt** and
refuses anything past 580.0; we accept nothing past **499.4** and refuse from **508.2**. The
brackets are disjoint by **65.1 pt** — 130 times the 0.5 pt the advance ratio (1.00112,
DejaVu Sans) can produce. Left edges agree at 36.0 / 36.1 pt, so the block is not shifted;
the column is narrower.

What the document contains: `word/document.xml` has **two** `w:sectPr`. The first sits in the
body's opening (empty) paragraph and declares
`<w:pgMar w:top="288" w:right="720" w:bottom="720" w:left="720" …/>` with `w:titlePg`. The
final, body-level one declares `<w:type w:val="continuous"/>` with
`<w:pgMar … w:right="1440" … w:left="1440" …/>`. `w:pgSz w:w="12240"` = 612 pt.

The reference's page 1 is section 1 throughout: left 36 pt = 720 twips, right limit
612 − 36 = 576 (it reaches 577.5). Page 2 is section 2 and both stacks agree there —
brackets `[536.1, 539.7]` against `[536.7, 540.3]`, 0.6 pt apart.

Our page-1 limit lands in `(499.4, 508.2]`, and **612 − 36 (section 1's left) − 72
(section 2's right) = 504.0** falls inside it. That is the arithmetic of laying the text out
at the *second* section's width from the *first* section's left origin.
**Verdict: section margin resolution across a `continuous` section break. Cross-lane —
L2/L3 (`XRay.Office.WordProcessing`).** Nothing in `XRay.Office.Text` can cause or fix it: the
breaker was handed the wrong width and filled to it correctly.

### #101 · `May 25 bulletin focus on carers in the workplace.docx` — a version divergence in my own file

The document asks for **Helvetica** on 99 runs, declared `<w:family w:val="swiss"/>` in
`word/fontTable.xml`. The 24.2.7.2 reference draws Liberation Sans; we draw DejaVu Sans,
**+14.2% wider** — the whole of the reflow, and of the wrapped call-to-action label.
`SystemFontResolver.cs`:490-499 names this exact case ("`Times`, `Helvetica`, `Albany` and
`Thorndale` all answer DejaVu once a class is declared, where the chain answers Liberation")
and states it was **measured against 26.2.4.2**.
**Verdict: reference-version divergence; ours is correct for 26.2.4.2. No patch.** See §5.
(The Campaigns banner being drawn wider is a separate graphics defect, not mine.)

### #106 · `Regulations Governing the Status…docx` — `w:w` character scaling is read by nobody

Two faults, and the first is document-wide.

**(a) `w:w` is ignored.** `word/styles.xml`'s default paragraph style carries
`<w:rPr><w:spacing w:val="4"/><w:w w:val="103"/><w:kern w:val="14"/>…`, so every glyph in
the document is set **103% wide** and tracked +4 twips. The reference honours it: MuPDF reads
the reference's spans at **10.2 / 12.2 / 14.2 pt** where ours are at **10.0 / 12.0 / 14.0**
— the reference scales the text matrix horizontally and we do not. Our text comes out
**2.6% narrow** (0.9746 over 4,301 glyphs), which is 1/1.03 plus the tracking we *do* apply
(`w:spacing` is read and consumed:
`XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs`:490-493 → `FormattedRun.Tracking`
→ `XRay.Office.Text/Layout/MeasuredParagraph.cs`:347-350). Only 3 of its 74 shared line starts
agree.

`<w:w>` appears **nowhere** in `dotnet/src`: `grep -rn '"w"' --include=*.cs` returns only
`w:tblW`/`w:tcW`/`gridCol` width *attributes*, `PptxDiagram*` map keys, and EMF+ tracking.
`WordParagraphFormats.ResolveRunProperty` reads `sz b i lang color vertAlign caps smallCaps
highlight u strike dstrike kern spacing` — and not `w`. `FormattedRun`
(`XRay.Office.Text/Layout/MeasuredParagraph.cs`:63-70) carries `Start Length Face EmSize
Shaping MetricEmSize Tracking` and no horizontal scale. The WW8 reader has no
`sprmCCharScale` (0x4852) either.

Corpus reach, from unzipping all 271 docx-family files: **11 carry a non-100 `<w:w>`**, and
only `#106` carries one document-wide via `Normal`. `#046` (90%), `#068` (99/101/108 on 48
runs) and `#093` (99 on 4 runs of 2,714) are the other reflow-tagged ones — `#093` is AWR,
and four scaled runs cannot be the cause of its divergence, so `dotnet/CLAUDE.md`'s account
of AWR is untouched by this.

**(b) a second, smaller measure fault.** On pages 4-6 the reference's justified lines are
flush at **488.0 pt** (43 of 47 on one page) and ours at **505.5** — 17.3 pt wider. The
reference's edge is exactly `612 − 1195/20 (pgMar right) − 1267/20 (SingleTxt's
w:ind right)` = 488.9, so the reference is right and our right indent is under-applied on
*some* paragraphs — on others (the page-3 opening paragraph) we are flush at 488.73, exact.
I did not isolate which paragraphs differ. **L2/L3.**

**Verdict: `w:w` character scaling read by nobody, plus an indent fault.** The change is
described in §9 as a cross-lane dependency and is **not** offered as a diff, for the reason
given there.

---

## 9. Cross-lane dependencies

### `w:w` character scaling — needs three files, one of them mine

ECMA-376 §17.3.2.43 `w:w`: the percentage each glyph's *width* is scaled by, independent of
its height. LibreOffice models it as `SvxCharScaleWidthItem`. To honour it:

1. **`dotnet/src/XRay.Office.WordProcessing/Ooxml/WordParagraphFormats.cs`** (L2/L3) — resolve
   `"w"` alongside `"spacing"` at :490-493, carry it on `WordTextStyle`, hand it to
   `PageRun` (`Layout/PageContent.cs`:877-891).
2. **`dotnet/src/XRay.Office.Text/Layout/MeasuredParagraph.cs`** (**mine**) — a `Scale` on
   `FormattedRun` beside `Tracking`, multiplying each glyph's advance in the prefix table.
   It scales the advance and not the em size, so it must not touch line height: the same
   separation `MetricEmSize` already makes for small capitals.
3. **The painter** — whatever draws a `PageRun` must apply the same horizontal scale, or
   measurement and drawing disagree and the line is laid out at one width and painted at
   another. `FormattedRun.EffectiveShaping`'s doc comment records what that cost when it was
   missing for tracking.

**I have not written the `XRay.Office.Text` half as a patch**, and that is the recommendation
rather than an omission: a `Scale` field that no reader sets and no painter honours is
exactly the "property read but never consumed" pattern the brief says has been the cause
four times here, and applied alone it would change nothing while looking like a fix. It
should land as one change across the three files, sequenced by the coordinator. Reach: one
document decisively (`#106`), two or three marginally, out of 946.

### `XRay.Office.WordProcessing` — section margins across a `continuous` break (`#091`)

The arithmetic in §8 (`612 − section1.left − section2.right = 504.0`, inside our measured
bracket) points at the page frame keeping the first section's origin while the text width
follows the second. 65 pt on page 1 of `#091`; worth checking the corpus's other
`continuous`-section documents against it.

---

## 10. What I did not establish

- **Why five of the 17 advance-band cases disagree 5–29× more than their ratio predicts.**
  They have a second cause and I did not find it.
- **The mechanism behind the four `DejaVuSans → DejaVuSerif` substitutions** (`#016 #097
  #135 #140`). The families are `Aptos` and `Segoe UI`, installed nowhere; I traced the
  resolver to `GenericFallbacks`, which sends an unclassified family to `SansFallbacks`, and
  could not reconcile that with the serif we emit. It needs the authored probes in §5.
- **Which paragraphs of `#106` lose their right indent** — only that some do and some do not.
- **Whether the slide autofit story is the version or a defect on both binaries.** 26.2.4.2
  is not installed here.
- **Anything about the 85 non-`reflow` documents.** Out of this lane's scope.

## 11. One thing that is *not* a defect, so nobody re-derives it

Every case's line starts sit **exactly 0.100 pt left** of the reference's — 58 of 107 at
−0.100 with zero spread, 13 more at 0.000. It is a constant, it is the text origin and not a
drift, and the exact margin is on *our* side (a 720-twip margin lands at 36.00 pt for us and
36.10 pt for the reference). 0.1 pt cannot move a line break. `dotnet/CLAUDE.md` already
records that the declared-margin probe refuted "our pen is off"; this is the corpus-wide
confirmation.
