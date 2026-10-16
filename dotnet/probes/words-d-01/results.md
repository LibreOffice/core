# Words-D round 1 — the page cluster, and the geometry that turned out not to explain it

Baseline `d82acd45832`. Worktree `/c/sandbox/workdir/wt-words-d`, branch `wt-words-d`,
`XRAY_OFFICE_CLI` set explicitly to this tree's binary on every sweep. Reference
`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, 200 PDFs from LibreOffice 26.2.4.2
620(Build:2) with `fonts-dejavu-core`. `SOURCE_DATE_EPOCH=1700000000` and `TZ=UTC` on every
render, ours and the probes' alike.

**Metric caveat.** Every figure below uses the gate metric as it stands on this branch
(`pdftotext | wc -w`, 2%+3 band). A corrected word check is in flight on another branch — a
word will need a letter or a digit — and a `xray-office analyze` verb is to replace `pdftotext`
outright. **Check 1 is the subject here and is unaffected by both.** No check-2-only failure
was touched.

---

## 1. The headline

> **Words is 154 / 200 before and 154 / 200 after.** Absolute page error 117 → 117, exact page
> counts 163 → 163, absolute word error 7023 → 7024, render failures 0 → 0. **No document
> changed verdict, and no page count moved on any of the 200.**
>
> What the round buys instead is a **law, a fix and an exclusion**. The law: LibreOffice fits
> every stated Word page dimension to the nearest standard paper dimension within 0.44 mm. The
> fix reaches all three Word-family readers and makes **33 of 200 renderings** agree with the
> reference's sheet where they did not; the media-box census goes from 31 documents disagreeing
> to **0**. The exclusion is the finding: **31 documents had the wrong sheet and correcting
> every one of them moved not a single page count**, which removes page geometry from the list
> of candidate causes for the 37-document page cluster.

Verdict split, unchanged at both ends: 29 `pages`, 8 `pages,words`, 9 `words`, 0 `unembedded`,
0 render failures. 100 of 134 `.docx` and 54 of 66 `.doc` match.

---

## 2. The prediction, scored

`prediction.md` in this directory, committed as `810e35671b4`'s parent content before the
first render of the round and before any per-document figure of this baseline was read.

| # | prediction | outcome |
|---|---|---|
| P1 | the baseline reproduces to the digit: 154, 117, 163, 7023, 0 failed | **held** — all five |
| P2 | the 37 check-1 failures are exactly `words-rebase-02/gate.tsv`'s, same page pairs | **held** — `diff` on the page and verdict columns is empty over all 200 |
| P3 | `glyphs` dominant on more matching than failing; 40–70 matching, 18–33 failing | **held** — 54 matching, 21 page-failing (27 counting all 46 failures) |
| P4 | `face` dominant on ≥3 matching and ≤2 failing | **refuted** on the second half — 9 matching and **4** failing. The round-47-era reading that `face` is a pass signature does not survive this baseline |
| P5 | 70–115 of the 154 matching documents have a divergent page at all | **held** — **85** |
| P6 | the ±1 cluster is not one cause: ≥10 distinct first-divergence pages and ≥3 kinds | **split.** Six distinct kinds — refutation stands. But only **five** distinct page numbers, because divergence starts at page 1 on 6 of the 14 `−1` and 2 of the 9 `+1`. The numeric criterion was wrong about *why* |
| P7 | the largest shared-cause sub-cluster is page geometry, seen as `page size differs` on 4–12 of the 37 | **refuted as stated — and right underneath.** `first-divergence.py` reports `page size` on **1** document, because `pdf-image-diff.py` rasterises to 512 px on the long edge and a 0.66 pt sheet difference is invisible there. A direct media-box census found **31**, of which **8** are page failures |
| P8 | a cause pinned to a file and line, and shipped | **held** — `DomainMapper.cxx`:827/836, `ww8par6.cxx`:521/1083, `paper.cxx`:169/208 |
| P9 | verdicts moved: point estimate 0, band 0–4 | **held at the point estimate** — 0 |
| P10 | 3–12 of the 37 have no divergent page in the common prefix | **refuted** — **0**. Every one of the 37 diverges inside the prefix, 17 of them on page 1 |
| P11 | ≥4 of the P10 set are `.doc` | **void** — P10's set is empty |

**Four refuted, one void, one split.** P7 is the one worth reading twice: the prediction named
the right cause and the wrong instrument, and an agent who had run only `first-divergence.py`
would have concluded page geometry was a one-document curiosity.

---

## 3. The law, measured on the installed binary

The C++ tree in this checkout is 27.2.0.0.alpha0+ and is **not** the binary that made the
references. Everything in this section was measured first, on authored minimal DOCX rendered
by the installed `soffice`, and attributed to source afterwards.

`papersnap.py` builds a one-paragraph, one-section DOCX with a swept `w:pgSz` and reads the
emitted media box back with `pdfinfo`. Nothing else varies.

### 3.1 The window, to the twip

Stated width fixed at 11906 twips; stated height swept one twip at a time.

| stated height (twips) | emitted height |
|---|---|
| … 16811, 16812, **16813** | itself, to the hundredth |
| **16814** … **16862** | **841.890 pt**, all 49 of them |
| **16863**, 16864, … | itself again |

So the window is 16814–16862 twips inclusive and its edges are sharp on both sides. Converted
the way LibreOffice converts (`twips × 127 / 72`, rounded), those are 29658 and 29743
hundredths of a millimetre against A4's 29700 — **strictly inside ±44**, with 16813 landing on
29656 (exactly 44 away, excluded) and 16863 on 29745 (45 away).

That is `MAXSLOPPY = PT2MM100(1.25) = 44` and a strict `<`, which is what
`i18nutil/source/utility/paper.cxx`:169 and :208 say. The 27.2-alpha source describes 26.2.4.2
here **exactly**, edge for edge — recorded because the same tree misdescribed 26.2.4.2's line
spacing badly enough to cost two predictions last round, and "the dev tree is wrong" is not a
rule either.

### 3.2 One dimension at a time, against every dimension in the table

A stated **20638 × 25000** twips comes back **1031.81 × 1249.99 pt**. 1031.81 pt is 364 mm,
which is B4(JIS)'s **height** and no paper's width; 20500 twips at the same height comes back
unchanged. So the fit does not match whole paper formats — it fits each dimension on its own
against every width *and* height in `aDinTab`, and a page can leave it as one paper's width and
another's height. Also measured: 12240 × **15845** → 612 × **792** (Letter's height snapped),
**12245** × 15840 → **612** × 792 (Letter's width snapped), 11900 × 16830 → both.

### 3.3 The law, as it now stands measured

> Each of a Word document's two stated page dimensions is replaced, independently, by the first
> dimension in LibreOffice's standard-paper table — counting every entry's width and its height
> alike — that is strictly less than 0.44 mm away from it, and left alone when none is. The
> comparison happens in hundredths of a millimetre, so the input's own conversion rounding is
> part of the rule.

**Where it lives.** `DomainMapper.cxx`:827 and :836 apply
`PaperInfo::sloppyFitPageDimension` to `w:pgSz`'s two attributes — and to nothing else in the
section — and `rtfdispatchvalue.cxx`:1274-1289 routes `\paperw`, `\paperh`, `\pgwsxn` and
`\pghsxn` through those same two cases, so RTF is not a parallel implementation but the same
one. `ww8par6.cxx`:521 and :1083 do it for DOC through
`SvxPaperInfo::GetSloppyPaperDimension`, which is the same fit with a twip round trip either
side. **ODF is deliberately absent from that list** and is left alone here: an ODF document
states its page in the same hundredths of a millimetre the model uses, so there is no rounding
scar to erase.

### 3.4 A second, smaller effect, measured and *not* acted on

Even where nothing snaps, LibreOffice's page dimension passes through hundredths of a
millimetre: 16780 twips comes back as 838.998 pt, not 839.000. The residual is at most 0.014 pt
per edge. It moves no line and no page, modelling it would mean putting the whole page
geometry chain through mm100, and it is recorded here rather than done.

---

## 4. Reach, measured over all 200 — and why the census is a media-box census

The rule reaches DOCX, DOC and RTF, so a census that reads `word/document.xml` would see at
most 134 of 200 and **the 66 `.doc` would be invisible to it** — the same blind spot
`words-b-01` named and then measured 8 movers inside. So nothing is parsed at all here:
`mediabox.py` compares **the set of page sizes ours emits against the set the reference emits**,
document by document, over all 200. The PDF's media box is each renderer's answer, and it is
available for both halves of the corpus equally.

| | before | after |
|---|---:|---:|
| documents emitting a sheet the reference does not (0.05 pt tolerance) | **31** | **0** |
| …`.docx` / `.doc` | 22 / 9 | — |
| renderings changed byte for byte | — | **33** (22 `.docx`, 11 `.doc`) |
| of those, page count moved | — | **0** |
| of those, verdict moved | — | **0** |

**The census under-counted by two and I can say exactly how much.** Its 0.05 pt tolerance
scored `20110608_psp.docx` and `手机免提系统TSB.doc` as agreeing at 595.25 pt against the
reference's 595.304 — 0.054 pt apart, on the wrong side of a boundary I chose. Byte-comparing
the renderings is the instrument that does not have a threshold, and it says 33. Both numbers
are reported rather than the flattering one.

The 33 by verdict: **24 `match`, 4 `pages`, 4 `pages,words`, 1 `words`**. Every one of the 24
matching documents was drawing on a sheet that was the wrong size and passing the gate anyway,
which is the whole argument for running the census over the documents that already match.

Word error moved by one, on one document: `JEMIT_Template.docx`, 1705 → 1706 against a
reference 1697. That is the entire measurable effect on check 2.

**Not established:** the RTF reach, because `words/` holds no RTF at all. The arm is reasoned
from `rtfdispatchvalue.cxx` and pinned by the one test that can reach it, and its corpus reach
is unmeasured rather than zero.

---

## 5. The classification over all 200, with the matching-document control

`divsweep.py` runs `first-divergence.py`'s own `analyse()` — imported, not reimplemented — over
pre-rendered pairs, so the classification is the skill's and only the plumbing is mine. Ours
comes from this round's baseline sweep, the reference from the stored PDFs; that avoids a
second whole-corpus `soffice` run and changes nothing, since the stored PDFs are that run.

**All 200 documents, matching included.** "Dominant" is the majority note kind on the first
materially divergent page; `(none)` means no page in the common prefix passed the 0.35 % ink
floor.

| dominant kind | matching (154) | `pages` failures (37) | `words`-only failures (9) |
|---|---:|---:|---:|
| **(none)** | **69** | **0** | **0** |
| glyphs | 54 | 21 | 6 |
| one-sided | 11 | 6 | 0 |
| face | 9 | 3 | 1 |
| box | 6 | 1 | 1 |
| size | 3 | 4 | 1 |
| colour | 1 | 0 | 0 |
| width | 1 | 0 | 0 |
| hairline | 0 | 1 | 0 |
| page size | 0 | 1 | 0 |

**The finding reproduces, and it is the same one: what separates a failure from a pass is
whether a divergent page exists at all, not what kind it is.** 69 of 154 matching documents
have none; **0 of 46 failures** have none. Given that a divergent page exists, the probability
of failing is 46/131 = 35 %, and every kind's column is close to that split — `glyphs` is
dominant on **54 matching against 21 page-failing**, which is the majority kind on *both* sides
and discriminates nothing.

The control also kills a reading I would otherwise have taken from the failing half alone.
Where the divergence *starts*:

| first divergent page | matching | `pages` failures |
|---|---:|---:|
| none | 69 | 0 |
| 1 | 43 | 17 |
| 2–4 | 29 | 14 |
| 5–20 | 10 | 5 |
| 21+ | 3 | 1 |

Seventeen of thirty-seven page failures diverge **on page one**, which read on its own says the
fault is at the top of the document. It says nothing: 43 of the 85 matching documents that
diverge at all also diverge on page one. The distributions are the same shape. **Nothing in
this classification, in either column, separates a page failure from a pass except the
existence of a divergence.**

One more cut, since two readers meet in this corpus: the `.doc` half is *more* likely to have
no divergence at all (29 of 54 matching `.doc`, against 40 of 100 matching `.docx`), and 12 of
its 66 fail check 1 against 34 of 134 `.docx`.

### What this classification cannot see, stated rather than discovered later

1. **Only the common prefix is compared.** `A_320.doc`'s 23 surplus pages and every `+N`
   document's reference tail are never looked at.
2. **The 0.35 % ink floor and the 512 px raster.** This is what refuted P7 the wrong way: a
   0.66 pt sheet difference — the very cause this round pinned — is *below the resolution of
   the instrument that was supposed to find it*, and `page size differs` fired on exactly one
   document, the one whose pages differ by an orientation. A geometry census had to be built
   separately, and did the work in one pass.
3. **The dominant kind is a majority vote on one page**, which the script's own header records
   losing on `150-5370-10H.docx`. No conclusion here is drawn from a kind alone.
4. **There is no kind for "this page is extra".** A page break in the wrong place between two
   same-sized pages presents as `glyphs` noise on everything after it.

---

## 6. The ±1 cluster: refuted again, and by a different route than last time

Twenty-three of the 37 are exactly one page out — 14 short, 9 long.

| Δ | ext | first divergence | dominant | document |
|---:|---|---|---|---|
| −1 | doc | p1 of 2 | size | `手机免提系统TSB.doc` |
| −1 | doc | p1 of 22 | one-sided | `762.doc` |
| −1 | doc | p1 of 3 | face | `1447.doc` |
| −1 | doc | p1 of 4 | one-sided | `003.doc` |
| −1 | doc | p1 of 5 | glyphs | `info-bulletin-601.doc` |
| −1 | doc | p1 of 6 | one-sided | `absrc-pac-01-info-note-en.doc` |
| −1 | doc | p4 of 63 | face | `150_5335_5a.doc` |
| −1 | docx | p1 of 14 | glyphs | `ABCD-FE-01-00 Flight Envelope…docx` |
| −1 | docx | p1 of 15 | glyphs | `FO.FCTOA.00010…docx` |
| −1 | docx | p1 of 31 | box | `5709.16 ch.40_mgfinal.docx` |
| −1 | docx | p2 of 164 | glyphs | `OM template…2016.docx` |
| −1 | docx | p2 of 43 | glyphs | `docs-quality-MA.IMS.00001…docx` |
| −1 | docx | p4 of 19 | glyphs | `report-template.docx` |
| −1 | docx | p4 of 34 | one-sided | `ESPN-R - MCF - Manual - Ed1.0…docx` |
| +1 | doc | p1 of 31 | glyphs | `150_5300_13_chg12.doc` |
| +1 | docx | p1 of 47 | glyphs | `33004.docx` |
| +1 | docx | p14 of 76 | glyphs | `FRE-03_mcar_part-3_and_IS_v2.9.docx` |
| +1 | docx | p16 of 266 | glyphs | `SPA-02_mcar_part-2_and_IS_v2.9.docx` |
| +1 | docx | p2 of 29 | glyphs | `UG.CAO.00006…docx` |
| +1 | docx | p2 of 7 | glyphs | `template---tpr…docx` |
| +1 | docx | p3 of 8 | page size | `1_tpr_template__from_fy14_.docx` |
| +1 | docx | p4 of 58 | glyphs | `ESPN-R - MCF - RA - Ed1.docx` |
| +1 | docx | p4 of 6 | glyphs | `B11. TE.CAO.00129 Experience logbook.docx` |

**Refuted.** Six distinct dominant kinds, both file formats, both signs, and — the point the
control makes — a distribution indistinguishable from that of the 85 matching documents that
also diverge. Splitting by the sign of the error does not rescue it either: `glyphs` dominates
both halves.

The one honest correction to my own prediction: I expected the divergence pages to be *spread*,
as they were the last time this was refuted (page 1 to page 91). They are not — they are
bunched at page 1. **The cluster is refuted for a different reason than I predicted, and a
prediction that had scored only "refuted / not refuted" would have hidden that.**

**Six of the 37 are not our movement at all.** `words-rebase-02` §5 measured, with our code
held identical, six documents whose *reference* page count moved with the 24.2.7.2 → 26.2.4.2
binary change: `1447.doc`, `003.doc`, `template---tpr…docx`, `150_5335_5a.doc`,
`ESPN-R - MCF - RA - Ed1.docx`, `FAA 2025-26 Holdover Tables.docx`. All six are in the 37. Any
future round hunting a shared cause across this set is hunting across at least six documents
where our own number never changed.

### The one sub-shape that *is* real, and is three documents

Page-shape sequences, run-length coded, are the one place a page-count failure shows structure.
Three documents have ours holding **exactly one extra portrait page immediately before the
first landscape run**, with every later run identical:

| document | ours | reference |
|---|---|---|
| `template---tpr…docx` | A**6**BA | A**5**BA |
| `1_tpr_template__from_fy14_.docx` | A**3**BA4B | A**2**BA4B |
| `33004.docx` | A**40**B3A5 | A**39**B3A5 |

It is a shape, not yet a cause: the extra page sits at the end of a portrait run, so it is as
consistent with the run overflowing by one page as with a spurious break at the section
boundary, and the first two documents are two revisions of the same TPR template. It is
recorded with its evidence so the next round does not have to find it again. Note also that
`template---tpr…docx` is one of the six above — it was 8/8 at 24.2.7.2 and is 8/7 now because
the *reference* lost a page.

---

## 7. The exclusion, which is the round's real result

Thirty-one documents were being laid out on a sheet the reference does not use — up to 0.66 pt
taller and 0.3 pt wider, which is real text area on every page of them. Eight of those are
check-1 failures. Correcting all thirty-one, so that **0 of 200 documents now emit a sheet the
reference does not**, moved:

- page counts on **0 of 200**,
- verdicts on **0 of 200**,
- and, on the eight page failures among them, **nothing at all** — same page count, same gap.

**So page geometry is excluded as a cause of the 37-document page cluster.** That is worth more
than the zero in the verdict column: a whole candidate class is closed, by a measurement rather
than an argument, and it is closed *because* the fix was made and swept rather than reasoned
about. The cluster's causes are inside the flow, not in the sheet it flows onto.

---

## 8. The implementation, with file and line

`dotnet/src/XRay.Office.WordProcessing/Model/PaperSizes.cs`, new. `aDinTab` transcribed in its
own order — verified entry for entry against `i18nutil/source/utility/paper.cxx` by a script
that re-evaluates `MM2MM100`/`IN2MM100`/`PT2MM100` and compares all 86 rows — with the
`PAPER_USER` row, which is 0 × 0 and would swallow every small dimension, absent.

**The order is load-bearing and is commented as such.** The fit returns the *first* entry within
the window, and eleven pairs of distinct dimensions in the table sit closer together than two
window widths — Quarto's 21519 and Letter's 21590 are 0.71 mm apart — so a page between two of
them gets whichever comes first. A "nearest" implementation would be a different function.

Three call sites, one line each:

| file | what changed |
|---|---|
| `Ooxml/DocxPageGeometry.cs`:158 `Dimension` | `Length.FromTwips(twips)` → `PaperSizes.SloppyFit(Length.FromTwips(twips))`. Used by `w:pgSz`'s two attributes and nothing else |
| `Ww8/Ww8SectionTable.cs` `Dimension` | the same, for `sprmSXaPage`/`sprmSYaPage` and nothing else |
| `Rtf/RtfPageGeometry.cs` `ToSection` | the same, on the width and height that `\paperw`/`\pgwsxn` and `\paperh`/`\pghsxn` produce |

The fitted value is rebuilt **from twips**, not from hundredths of a millimetre: the DOC reader
converts straight back (`GetSloppyPaperDimension`), and the DOCX one hands hundredths to a page
style whose frame size Writer then holds in twips. Both land on 11906 twips for A4's 21000,
which is what the reference's 595.304 pt media box reads back as; building the `Length` from
21000 hundredths instead would sit 0.028 pt off.

Margins are **not** fitted, because LibreOffice does not fit them — `LN_CT_PageMar_*` a dozen
lines below in the same `switch` passes straight through.

### Tests — six, all verified by reintroduction

`verify-test.sh`, on a clean tree, one mutation at a time. **None of the six is a drift guard.**

| test | mutation it detects |
|---|---|
| `APageSizeJustOffA4IsFittedToA4` | removing the fit from `DocxPageGeometry` |
| `ThePageDimensionOneTwipInsideTheWindowIsStillFitted` | removing the fit from `DocxPageGeometry` |
| `ADimensionIsFittedAgainstEveryDimensionAndNotAgainstWholeFormats` | removing the fit from `DocxPageGeometry` |
| `ThePageDimensionOneTwipOutsideTheWindowIsLeftAlone` | widening the window from 44 to **60** hundredths of a millimetre — 0.16 mm |
| `AnExactAndAnUnusualPageSizeBothPassThrough` | widening the window to 400 (4 mm) |
| `AnRtfPaperSizeIsFittedTheSameWay` | removing the fit from `RtfPageGeometry` — the only evidence that arm has, since the corpus holds no RTF |

Two of this file's own assertions asserted the wrong answer before they were first run, both in
the same way, and the mistake is left recorded in the test's own comment: the table holds 96
distinct dimensions between 26 mm and 1414 mm, so a round number is *not* an unusual page size.
10000 twips is 176.39 mm, which is 0.39 mm from ISOB5's width and inside the window. The
"unusual" size in the passing test had to be searched for; it is 8561 × 13850 twips.

**The WW8 arm has no test.** Authoring a synthetic `.doc` is not something this test project can
do, and no fixture was copied from the corpus. Its evidence is the corpus sweep — 11 `.doc`
renderings changed and 9 of them were measured, before the change, as emitting a sheet the
reference does not. That is a real gap and it is stated here rather than papered over with a
test of `PaperSizes` in isolation, which would have proved the table and not the wiring.

### Build and suite

`dotnet build XRay.Office.slnx -v q -nologo`: **0 warnings, 0 errors.** Ten non-Fidelity projects,
one at a time in the foreground: Core 284, Containers 109, Text 289, Vector 295, Rendering 121
(1 skipped), Markup 259, OpenDocument 125, **WordProcessing 769**, Spreadsheets 621,
Presentations 592 — **3464 total, 0 failed**, against 3458 before. Fidelity not run, per
instruction.

**No cross-track sweep is owed and none was taken.** The change is confined to
`XRay.Office.WordProcessing`; `XRay.Office.Spreadsheets` and `XRay.Office.Presentations` do not
reference it (checked in their `.csproj` files), so no slide or sheet can reach it. This is a
structural argument, not a measurement, and is labelled as one — `words-b-01` owed a real
measurement because it touched the shared `XRay.Office.Text`, and this does not.

---

## 9. Measured, inferred, not established

**Measured**, each from a render:

- The baseline 154 / 117 / 163 / 7023 / 0, reproducing the briefed figures to the digit, and
  the after figures 154 / 117 / 163 / 7024 / 0.
- The snap window, 16814–16862 twips, both edges sharp, on the installed 26.2.4.2 (§3.1).
- Per-dimension fitting against every table dimension, on 364 mm and on Letter both ways (§3.2).
- The mm100 quantisation of *unsnapped* dimensions (§3.4).
- 31 documents disagreeing with the reference's sheet before and 0 after; 33 renderings changed,
  0 page counts, 0 verdicts (§4).
- The divergence classification over all 200 including the 154 that match (§5).
- The page-shape sequences behind §6's three-document sub-shape.

**Inferred, and marked as such:**

- That `PaperInfo::sloppyFitPageDimension` is the *mechanism*. The behaviour is measured on the
  installed binary and matches the 27.2-alpha source edge for edge; the attribution is still a
  reading of a tree that made none of the references.
- That the RTF arm behaves as the DOCX one does. `rtfdispatchvalue.cxx` routes the tokens into
  the same `DomainMapper` case, and the corpus cannot check it.
- That the eleven changed `.doc` are the same rule through `Ww8DocumentReader`. Nine of them
  were measured as emitting a wrong sheet beforehand; the other two were inside my census's
  tolerance and are inferred from the diff containing nothing else that could move them.

**Not established:**

- **Why the 37 fail.** Page geometry is now excluded (§7) and nothing has replaced it. The
  classification says only that a divergent page exists.
- **The three-document sub-shape's cause** (§6) — overflow versus a spurious break at the
  section boundary is untested, and two of the three are revisions of one template.
- **The RTF corpus reach**, zero on this track and unknown elsewhere.
- **Whether any of the 24 currently-matching documents whose sheet changed is now closer to the
  reference in ways the gate cannot see.** The three checks say nothing; only 33 renderings
  changing says anything at all.
- `A_320.doc`'s 141/118, still the largest single page error at 23, still unexplained, and still
  measured in `words-rebase-02` as having no font component whatever.

---

## Artefacts

In this directory: `prediction.md` (committed first), `papersnap.py` (the authored page-size
sweeps), `gate.py` (the whole-track render and score), `divsweep.py` (the divergence sweep
driver), `mediabox.py` (the geometry census), `pagesizes.py` (the page-shape sequences),
`gate-base.tsv` / `gate-after.tsv` (200 rows each), `div.tsv` (the 200-row classification),
`mediabox-base.tsv` / `mediabox-after.tsv`. Every fixture is newly authored and minimal; nothing
was copied or excerpted from the corpus, and the CV is not mentioned in any of it.
