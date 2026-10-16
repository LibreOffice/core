# O15's third cause is a hyperlink that is not a hyperlink, and the arithmetic everyone has been hunting is exact

Round 103, slides track. Two seats. **O15 moves for the first time in three rounds and its
premise is refuted**: the autofit block height and the line breaking under it are reproduced
*exactly* — 84 of 84 block heights and 164 of 164 wrap thresholds against 26.2.4.2, measured to
the hundredth of a millimetre on decks built for it — and what was left is that this tree makes an
EditEngine **field** of a `.ppt` hyperlink where 26.2.4.2 makes none. A field's spill lines are
not counted in the height the shrink-to-fit search measures, so a block one line short of the
reference's fits a larger row of `constScaleLevels` and every character on the slide is drawn a
point too big. **17 → 14 pages, 13 → 11 documents, 66.06 → 62.06 pt, 3 fixed and 0 newly wrong**,
`|ink|%` **250.28 → 244.13** over the column with 7 renderings moving and all 7 improving.

**The mechanism is named, and it is an arithmetic defect in the reference's property reader.**
`Section::Read` clamps every OLE property's buffer to `nSecSize - nSecOfs` — a declared *length*
less an absolute *position* — so a `_PID_HLINKS` blob in a section that starts late in the stream
is handed over truncated, the hyperlink list stops at the last whole entry the buffer holds, and
every identifier past that resolves to nothing. On the register's own witness the rule predicts
**two** entries where the file declares ten, and a ten-variant series at 26.2.4.2 draws exactly
the first two as links.

**O13 is sized and stays open, with the brief's own reason for parking it corrected.** The census
holds — 2 shapes in 2 of the 51 `.ppt` — but the two are not one problem: the larger witness is a
**MAJOR page at 16.04 `|ink|%`** whose Escher fill is a plain solid black, so it needs half (a)
and nothing else, while the smaller is a *gradient* (Escher `fillType` 5, which 26.2.4.2's own ODF
export writes out as a stretched bitmap) on a page worth 0.01.

Every figure below was read out of a file in this directory or out of a run log beside it, and
each such file is named where the figure is quoted.

## Environment

| | |
|---|---|
| worktree / branch | `/home/user/wt-slidefw`, `agent/slidefw`, base **`e58567aea`** |
| reference | `/opt/libreoffice26.2/program/soffice` — **26.2.4.2**. `/usr/bin/soffice` (24.2.7.2) is used for nothing here |
| reference bank | `/home/user/gate-orig-r83/ref`, the 51 `.ppt` among its 948 PDFs |
| corpus | `/home/user/sample-files`; the slides binary column is **51 `.ppt`** (`ppt.list`) |
| working directory | `/home/user/r103-slidefw` — **not** `/home/user/r103-work`, which a second live session was already writing into when this round started |
| C++ tree | `/home/user/libreoffice-core`, read only, never built. It declares **`27.2.0.0.alpha0+`** and is not the reference binary's source; every arm below is measured at the binary as well as read here |

---

# 0. What O15 was, and the two instruments that closed it out

The seat's statistic is the per-page **dominant drawn text size** — the size carrying the most
alphanumeric characters — over all 1534 pages of the 51 `.ppt`. Round 100 left it at 17 pages in
13 documents, 66.06 pt of summed error, and a sharpened diagnosis: *the block is short by a fifth
of a point over thirteen lines*, from a dump showing `RESPA_-_Section_8_Webinar.ppt`'s tightest
autofitted shape winning its `constScaleLevels` row by 7 hundredths of a millimetre.

**That diagnosis is refuted, and refuting it took two measurements that neither the register nor
the dump could make.**

## 0.1 A renderer's measured block height can be read out of a PDF, exactly

`ImpEditEngine::ScaleContentToFitWindow` takes the **first** row of `constScaleLevels` whose
formatted block fits, and the comparison is against `box + 1`. So if `H` is a box height in
hundredths of a millimetre and the answer at `H` is row *k* while the answer at `H − 1` is row
*k+1*, then

```
block(k) = H + 1
```

in that renderer's own units. `make-fitedge-probe.py` builds one deck whose every slide is the
same autofitted body in a box of a stated height; `fitedge.py` bisects, holding every open
boundary of every case in one deck per round, so the whole sweep costs ten conversions rather than
one per sample. Both renderers see the identical file, and nothing in it reads a bounding box:
the answer comes from the drawn `/Tf` size and the baselines `tfz.py` reads out of the content
stream's own operators.

**Seven constructs, twelve rows each, 1200 → 16000 hundredths of a millimetre**
(`fitedge-blocks-pptx.tsv`, `fitedge-samples-pptx.tsv`): a plain seven-paragraph body, the same
with 6 pt of space above and below, a replica of RESPA page 18's own seven paragraphs with and
without that space, a 90 % `a:lnSpc`, a bulleted body, and an 18 pt body with 4 pt of space.

| | |
|---|---:|
| thresholds resolved to the unit, both legs | **84** |
| where the two renderers' block heights differ | **0** |

The RESPA replica's rows include 12- and 13-line wrapped blocks and paragraph spacing at three
scale levels. **So the block arithmetic — the em quantisation, the whole-point rounding, the
device round trip, the `1.2 em`, the proportional and `Off` line-spacing branches, and the
truncating paragraph-space scale — is exact.**

## 0.2 And so is the line breaking

The same driver with `--vary w` sweeps the box **width** on a non-autofitting box, so the only
threshold in the sweep is the width at which a paragraph stops fitting on one line — which is that
renderer's advance for the string, again with no glyph box anywhere in it. Ten constructs: RESPA's
own paragraphs at 19 and 20 pt in DejaVu Sans, and one paragraph at 12, 20 and 24 pt in Liberation
Sans, DejaVu Sans and Carlito (`fitedge-blocks-wrap.tsv`).

| | |
|---:|---:|
| wrap thresholds resolved to the unit | **164** |
| where the two renderers differ | **0** |

**Two nulls with 248 cells between them.** That is what re-aimed this round: whatever O15 is, it
is not the fit's arithmetic and not the break iterator, and no probe deck was going to show it —
because the difference is not in how a body is measured but in **what the body is**.

## 0.3 The instrument that found it: read the same document twice

`soffice --convert-to fodp` on `RESPA_-_Section_8_Webinar.ppt` and then rendering **that** through
26.2.4.2 puts the reference on both sides of its own import. Page 18 comes back at **20.013 pt** —
this tree's answer, with the same ten lines and every baseline within 0.03 pt — where the same
binary reading the `.ppt` draws **18.992** (`witness-p18.txt`). Over the 32-slide deck the two
routes disagree on **13 pages**, the flat-ODP leg one `constScaleLevels` row larger every time,
and on 12 of those 13 **this tree agrees with the reference's `.ppt`**.

So the disagreement is in what the PPT filter puts in the model, not in what the layout does with
it — and it is one thing the ODF round trip does not carry.

---

# 1. O15 — a `.ppt` hyperlink is a field only while the truncated `_PID_HLINKS` reaches it

**Fixed in this tree.**

## 1.1 The witness, measured rather than reasoned

`RESPA_-_Section_8_Webinar.ppt` page 18: an outline placeholder of seven 24 pt paragraphs with
6 pt above and below, text area 9963 units, so the search fits against 9964. A temporary dump in
`SlideAutofit.Solve` printed this tree's block at every row:

| row | our block | lines |
|---|---:|---:|
| unscaled | 12905 | 12 |
| 1.000 / 0.900 | 12647 | 12 |
| 0.925 / 0.900 | 11841 | 12 |
| **0.850 / 0.900** | **9432** | 10 |
| 0.850 / 0.800 | 9185 | 10 |
| 0.775 / 0.800 | 8800 | 10 |

We took 0.850 / 0.900 with 532 units to spare — **not** the 7 the register's summary implies,
which belongs to a different shape of that document. The reference took 0.775 / 0.800.

**Its own drawn geometry says why.** Converting the reference's ten baselines on that page into
hundredths of a millimetre from the text area's top edge: the first sits at 670 (one em, the
fixed-cell-height ascent), the in-paragraph pitch is 804, the paragraph-to-paragraph distance is
1142 — all three exactly this tree's model at that row — and the block runs to **9605**, which is
`8800 + 804`. One line more than we measured.

The line is the last paragraph's second. That paragraph is a URL, this tree read it as an
EditEngine **field**, and a field's spill lines are deliberately excluded from the height the
shrink-to-fit search measures (`SlideTextLayout`, from round 82's `.odp` measurement of
`MoveToNextLine`). Add it back and the arithmetic answers the reference on every row: `9432 + 847
> 9964` rejects 0.850, `9185 + 847 > 9964` rejects it again, `8800 + 804 = 9604 ≤ 9964` takes
0.775.

**Three independent signs that 26.2.4.2 makes no field there**, all on the same page:

| | 26.2.4.2 | this tree, base |
|---|---|---|
| the URL's colour | `#333e48`, the body's | `#1e6d37`, the scheme's hyperlink slot |
| a rule under it | none | two |
| where it breaks | at a hyphen, `…/lighthouse-` | mid-word, `…lightho` / `use-title/` — the per-character cell breaks a field portion gets |
| the spill line's pitch | 804, the line height | 706, the ascent — which is what a spill takes |

## 1.2 Why: `Section::Read` clamps a property to a length less a position

`ImplSdPPTImport::Import` builds one hyperlink entry per link of the `_PID_HLINKS` blob in the
user-defined property section — six OLE properties each — and **breaks at the first one whose type
is not `VT_I4` or whose string will not read** (`sd/source/filter/ppt/pptin.cxx`:392-518). Only
the entries it built are then given an index from the `ExObjList`'s `ExHyperlink` records, in
stream order (`:530-549`); when it built none, and only then, it falls back to one entry per
record (`:551-575`). A text range whose `InteractiveInfoAtom.exHyperlinkId` matches no entry's
index is stepped over and its text drawn as it stands
(`filter/source/msfilter/svdfppt.cxx`:6907-6941).

And the blob reaches that loop cut short:

```cpp
if( nPropSize > nSecSize - nSecOfs )
    nPropSize = nSecSize - nSecOfs;
```

`sd/source/filter/ppt/propread.cxx`:443-447. `nSecSize` is the section's declared **length** and
`nSecOfs` its absolute **offset** in the stream. A section that begins late in a
`\005DocumentSummaryInformation` therefore has its largest property truncated, and `_PID_HLINKS`
is the largest property such a stream carries. (Where the offset exceeds the size the unsigned
subtraction wraps and nothing is clamped, which is the ordinary case.)

On RESPA the user-defined section is at **offset 1540** and declares **2244**, so a 2180-byte blob
arrives as **704** bytes. The entries end at 280, 592, 820, …, so exactly **two** are built where
the blob declares ten (`pidhlinks.py`, `hlinks.txt`).

## 1.3 Measured at the binary, with no free parameter

`hyperid.py` patches the four bytes of one text range's `exHyperlinkId` — checked to be unique in
the file first — to each of the ten `ExHyperlinkAtom` values the deck declares, renders all twelve
variants through 26.2.4.2 and reads the answer off the drawn text (`hyperid.txt`):

| patched id | rule under the URL | its colour |
|---|---:|---|
| unpatched (30) | 0 | `333e48` |
| **13** | **1** | **`1e6d37`** |
| **15** | **1** | **`1e6d37`** |
| 17, 20, 22, 24, 27, 30, 32, 35 | 0 | `333e48` |
| 999 | 0 | `333e48` |

**Exactly the first two, which is what the clamp predicts and what nothing else does.** This
reproduces the observation `dotnet/CLAUDE.md` records as *"26.2.4.2 declines to make a field on
five decks whose ids do resolve, for a reason nobody has named"* — and names it.

**Confirmed on a second document, on a boundary the first cannot show.**
`joint_user_outcomes_michael_fullerton_29.06.12.ppt` declares 12 links and yields **7**; its six
text ranges name ids 31 to 36, so the rule predicts that **one** of them — id 31, the seventh
entry — is a field and five are not. Page 20 of the reference draws exactly one URL in `f49100`
and five in `000000`; this tree drew four coloured at base and draws **one** at head, five black,
matching span for span.

## 1.4 The census, with the base rate beside it

`pidhlinks.py` over all 51 `.ppt` (`hlinks.txt`). **28 documents state a hyperlink construct at
all**; of those, **9** have a `_PID_HLINKS` the clamp cuts:

| document | blob declares | blob yields | ranges | resolvable before | after |
|---|---:|---:|---:|---:|---:|
| `gillikin_online_user_mtg_2010` | 40 | 26 | 20 | 20 | 6 |
| `iep-amount-frequency-for-webinar` | 28 | 11 | 14 | 14 | 0 |
| `0335fab9-79f0-4944-b92c-f223837ca2d8` | 22 | 18 | 11 | 11 | 7 |
| `joint_user_outcomes_michael_fullerton` | 12 | 7 | 6 | 6 | 1 |
| `RESPA_-_Section_8_Webinar` | 10 | 2 | 5 | 5 | 0 |
| `ws_prod-…-NATO-activities` | 9 | 6 | 12 | 12 | 6 |
| `1-secretariat` | 6 | **0** | 3 | 3 | 3 (the fallback) |
| `introduction_to_bea_tuxedo` | 5 | 1 | 5 | 5 | 1 |
| `gfopportunitiesforlinkagespres_2010_en` | 32 | 18 | 16 | 16 | 2 |

The other 19 are unaffected: **19 of 28 blobs are not cut, so a document stating hyperlinks is
more likely than not to be unchanged by this** — which is the base rate the fix has to beat, and
does, because the nine it changes are exactly the nine whose renderings move.

**Counted a third way, off the drawn page.** `underlines.py` counts the rules under text in each
rendering — 26.2.4.2 writes an underline as a **zero-height** rectangle that is not a fill, which
is why the first two cuts of that counter reported every reference PDF as carrying none
(`underlines.txt`):

| document | base | head | 26.2.4.2 |
|---|---:|---:|---:|
| `gfopportunitiesforlinkagespres_2010_en` | 18 | **5** | 5 |
| `RESPA_-_Section_8_Webinar` | 19 | **12** | 12 |
| `iep-amount-frequency-for-webinar` | 79 | **74** | 74 |
| `gillikin_online_user_mtg_2010` | 54 | **50** | 50 |
| `joint_user_outcomes_michael_fullerton` | 31 | 21 | 22 |
| `introduction_to_bea_tuxedo` | 98 | 95 | 96 |
| `1-secretariat` | 6 | 6 | 7 |
| `0335fab9-79f0-4944-b92c-f223837ca2d8` | 21 | 14 | 19 |
| `ws_prod-…-NATO-activities` | 35 | 35 | 196 |

Four land on the reference's count **exactly** and three come within one; summed unsigned distance
over the eight comparable documents goes **43 → 8**. `0335fab9` now under-creates by five, so the
cap is slightly too tight there and something else is still wrong on the NATO deck, which draws
196 rules against our 35 before and after — a separate defect this does not touch.

## 1.5 The change

`PptHyperlinkBlob` reads the `\005DocumentSummaryInformation` stream, finds the user-defined
section through its own dictionary, applies `Section::Read`'s clamp and walks the blob with
`PropItem::Read`'s semantics — a count, the characters, a dword alignment, and **false**, which
ends the list, whenever the last character it can reach is not a terminator. `PptHyperlinks.Read`
takes that number as a cap on the `ExHyperlink` records it may hand identifiers to, counted in
records rather than in distinct identifiers because a deck may state one identifier on several.
Zero and absent both mean *no cap*, which is the reference's own fallback.

`PptReader` reads the stream and `PptSlideLayout` passes it on; nothing else changed.

## 1.6 O15's statistic

`sizes.py`, `sizescore.py`, `size-summary.txt`. **The scorer is validated before it is used**: it
reproduces `probes/slides-r97/sizes-ref.tsv` **1534 of 1534 rows** off the same bank, and its base
column reproduces round 100's head figures exactly.

| | base (= round 100 head) | head |
|---|---:|---:|
| pages differing by more than 0.15 pt | **17** | **14** |
| documents holding one | 13 | **11** |
| total \|size error\| over 1534 pages | **66.06** | **62.06** |
| of the differing pages, same alphanumeric count | 12 | 9 |
| … of those, we draw the larger size | 11 | 8 |
| **fixed / newly wrong** | — | **3 / 0** |

The three: `RESPA` page 18 (20.01 → 18.99, the reference's), `gfopportunitiesforlinkagespres_2010_en`
page 25 (29.0 → 27.01) and `joint_user_outcomes` page 20 (11.99 → 11.0).

## 1.7 What it costs the column

`sweep.sh`, `base.tsv`, `head.tsv`, `gate-summary.txt`:

| | base | head |
|---|---:|---:|
| documents | 51 | 51 |
| `match` (pages equal, alphanumerics within max(2 %, 15)) | **49** | **49** |
| page counts differing | 0 | 0 |
| sum \|glyph distance\| | 1307 | **1307** |
| sum \|ink\|% | 250.28 | **244.13** |
| MAJOR pages | 65 | **63** |
| renderings whose bytes changed | — | **7** |

**All seven movers improve** — RESPA −1.59, `joint_user_outcomes` −1.02, `gillikin` −0.94,
`0335fab9` −0.81, `iep` −0.77, `introduction_to_bea_tuxedo` −0.71,
`gfopportunitiesforlinkagespres` −0.31 — and no glyph or page count moves anywhere, which is
expected: a field changes a colour, an underline and a line break, and the gate can see none of
those.

## 1.8 The test

`PptHyperlinkBlobTests`, seven cases: the blob read whole, the clamp cutting after two whole
entries, a clamp landing inside an entry's own string, an absent stream, the cap applied to an
`ExObjList` of five, the uncut control, and a blob cut before its first entry falling back to one
entry per record. Measured rather than asserted: with the cap line replaced by `int cap = 0` and
both test files left in place, **1 of the 7 fails and 6 pass** — the six that do not exercise the
cap are the class's own arithmetic, and they are the half that would silently keep passing if the
blob walk were wrong, which is why the corpus measurements above and not these are what the seat
is scored on.

## 1.9 What is left of O15

**Fourteen pages in eleven documents, and they are not one thing.** Four of them
(`Thailand17` 8 and 11, `W3_Case_Study` 10, `Fundamentals_Module_1_basics` 6) differ in
alphanumeric count as well as in size, so a dominant-size row there is comparing two different
sets of characters and the seat is elsewhere. Nine are same-alphanumeric and one
`constScaleLevels` row apart, in eight documents of which **none** states a hyperlink the clamp
cuts — so whatever is left is a third cause again, and this round's two nulls say it is not the
block arithmetic and not the wrap.

---

# 2. O13 — sized, and the reason it was parked applies to the smaller half only

**Not closed. Left seated, with a corrected sizing.**

## 2.1 The census holds

`wordart.py` walks the record tree of all 51 `.ppt` and reports every `msofbtSp` whose *instance*
— the shape type — is in the text-path range 136…175, which is what
`EnhancedCustomShapeTypeNames::Get` maps onto the `fontwork-*` names
(`svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx`). `wordart.txt`:

**2 shapes in 2 documents of 51**, and both carry a `gtextUNICODE`:

| document | type | text | fill properties |
|---|---:|---|---|
| `pres_ioc_phuket.ppt` | **136** (`fontwork-plain-text`) | `INTERNATIONAL TSUNAMI HAZARD MITIGATION` | `fillType`(384) = **5**, `fillColor` = 0x00FFFF, `fillBackColor` = 0x3399FF, plus 395-400 |
| `8.16_AOD_FINAL…ppt` | **144** | `Do you know what these are?` | **no `fillType`**, `fillColor`(385) = **0** |

## 2.2 What each is worth, and it is not what the brief assumed

The phuket shape is on **page 1** and the AOD shape on **page 59** (its containing `Slide` record
is the 59th in the stream, and page 59 is the only page of that deck where the reference draws
materially more paths than we do — 39 against 6).

| | 26.2.4.2 draws | we draw | that page's `\|ink\|%` |
|---|---:|---:|---:|
| `pres_ioc_phuket` page 1 | 92 filled paths | 2 | **0.01** (2.08 % of pixels differ) |
| `8.16_AOD…` page 59 | 39 paths | 6 | **16.04 — a MAJOR page** |

**So the two witnesses are not one seat.** The brief's reason for parking it — *landing the Escher
reader alone draws a solid yellow bar, which is worse than what is there* — is a statement about
the phuket shape, whose Escher `fillType` of 5 is a shade-centre **gradient** (which is why
26.2.4.2's own flat ODP writes that shape as `draw:fill="bitmap"` with `style:repeat="stretch"`:
it realises such a fill as a generated bitmap). It is worth **0.01** of ink.

The AOD shape states **no fill type at all and `fillColor` 0**, so it is a plain solid black
Fontwork — exactly what `SlideFontwork.Read`'s `Paint.Solid` already draws — and it is a **MAJOR
page at 16.04**. Half (a) alone is *correct* for it.

## 2.3 What (a) is, and why it is not a wiring job

The Fontwork machinery is shared and complete: `Fontwork.Outline`, `FontworkRequest`,
`SlideTextBody.WarpFontworkType`, the adjustment plumbing, and two callers already
(`PptxSlideLayout`, `OdpSlideLayout`). What the `.ppt` path lacks is everything upstream of it:

- **the shape type → `fontwork-*` map**, forty entries of
  `EnhancedCustomShapeTypeNames.cxx` — *not* `msdffimp.cxx`:2516-2600, which is the `TextPath`
  property set and was an earlier round's miscitation; this round read the table and confirms it
  is the `fontwork-*` name list;
- **the text**, which for an Escher WordArt is not in a text body at all but in `gtextUNICODE`
  (property 192, complex), with its face in property 197 and its size in 195/196;
- **the adjustments**, Escher's `adjustValue` properties, in WordArt units;
- **`FromWordArt`**, which `SlideFontwork` currently passes false and which the arch family needs
  true for a shape that came from a binary WordArt object.

That is a feature rather than a hook, on a two-shape reach — which is why it is left rather than
started at the end of a round.

## 2.4 And (b) is narrower than "no format path paints a non-solid Fontwork"

True as written: `SlideFontwork.Read` ends `Paint.Solid(stated.Colour)`. But the paint the phuket
shape needs is one this tree already builds for ordinary shapes — `PptFills.Resolve` reads
`fillType` 4…8 and returns a `GradientPaint`, centred kinds included
(`PptFills.cs`:123-290) — so (b) is *"let a Fontwork take the shape's own resolved fill"* rather
than a new paint. Whether 26.2.4.2's bitmap realisation of a shade-centre gradient and this tree's
gradient paint agree closely enough to be worth 0.01 of ink is the question a round taking it
should answer **before** writing either half.

---

# 3. The suite

Read out of this run's own output (`tests.log`), not from the briefed number.
`dotnet build XRay.Office.slnx -c Release` at **0 warnings, 0 errors**, then
`dotnet test XRay.Office.slnx -c Release --no-build`.

| project | |
|---|---|
| Containers | 109 / 109 |
| Core | 528 / 528 |
| Markup | 259 / 259 |
| OpenDocument | 146 / 146 |
| Rendering | 164 / 164 |
| Spreadsheets | 1295 / 1295 |
| **Text** | **728 / 728** — re-run alone, see below |
| Vector | 309 / 309 |
| **WordProcessing** | **1938 / 1938** — re-run alone, see below |
| **Presentations** | **1061 / 1061** |
| **Fidelity** | **542 passed / 10 failed of 552, 0 skipped** |

The ten are exactly the briefed baseline and nothing else — `PageDrawing` ×4, `TabStop` ×4,
`SheetDrawing` ×1, `JustificationShrink` ×1. **No eleventh.** Presentations is **1061** against
round 100's 1054: the seven `PptHyperlinkBlobTests`.

**One note on which binary was measured.** The sweeps in §1.6 and §1.7 and the suite above were
run on a build that still carried a temporary diagnostic in `SlideAutofit.Solve`, gated on
`XRAY_OFFICE_FIT_DUMP` and inert without it; the committed tree has it removed. Checked rather than
asserted: the committed binary draws `RESPA_-_Section_8_Webinar.ppt` show for show identically to
the swept one over all 35 pages, and the 75 autofit and hyperlink tests pass on it.

**The whole-solution run lost two projects and said so in a way that is easy to miss.** Its log
carries `Test Run Aborted.` for `XRay.Office.Text.Tests` and
`Catastrophic failure: Test process crashed with exit code 137` for
`XRay.Office.WordProcessing.Tests`, which reported **1063** of its 1938 — the container was running
a second session's sweeps and builds throughout. Re-run alone (`tests-text.log`, `tests-wp.log`)
both are green at their full counts, which are round 100's exactly. **The failure count alone
would have read as a clean run**: neither project reported a failure, one of them simply
disappeared from the summary and the other reported a smaller total.

---

# 4. The files

| | |
|---|---|
| `make-fitedge-probe.py`, `fitedge.py` | the block-height and wrap-width instruments: bisect the box height (or width) at which each renderer changes its answer, and read its measured block — or its advance — out of that threshold |
| `fitedge-blocks-pptx.tsv`, `fitedge-samples-pptx.tsv` | 84 block-height thresholds over seven constructs and twelve rows; **0 disagree** |
| `fitedge-blocks-wrap.tsv` | 164 wrap thresholds over ten constructs and four faces; **0 disagree** |
| `witness-p18.txt` | RESPA page 18 at four legs, including 26.2.4.2 reading back its own flat ODP of the same file, which is what separated the import from the layout |
| `pidhlinks.py`, `hlinks.txt` | how many hyperlinks the reference's truncated `_PID_HLINKS` yields, per document, with the `ExHyperlinkAtom` ids and the ranges that name them |
| `hyperid.py`, `hyperid.txt` | the ten-variant series at 26.2.4.2: which `exHyperlinkId` makes a text range a link |
| `underlines.py`, `underlines.txt` | rules under text, ours against the reference's, before and after — and the zero-height-rectangle trap that makes the obvious counter report none |
| `sizes.py`, `sizescore.py`, `sizesweep.sh`, `sizes-{ref,base,head}.tsv`, `size-summary.txt` | O15's own statistic, with the scorer's 1534-of-1534 validation against `slides-r97/sizes-ref.tsv` |
| `sweep.sh`, `base.tsv`, `head.tsv`, `gate-summary.txt` | the 51 `.ppt` at both legs: pages, alphanumerics, summed \|ink\|%, MAJOR, and the md5 that says which renderings moved |
| `wordart.py`, `wordart.txt` | every Escher text-path shape in the column, with its type and its fill properties |
| `tfz.py`, `_parts.py` | round 100's instrument and probe scaffolding, unchanged |
| `ppt.list` | the 51 documents |
| `tests.log`, `tests-text.log`, `tests-wp.log` | the suite, and the two projects the whole-solution run lost to an out-of-memory kill, re-run alone |
