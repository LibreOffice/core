# L6-sheets — summary

24 documents, 11 root causes. Four patches, one of which **must not be applied**.

**Read this first.** Three of the 24 are not defects. The tree is calibrated against
LibreOffice **26.2.4.2**; this sweep's reference is **24.2.7.2** (`soffice --version` here,
producer "LibreOffice 24.2" in every `/data/bench/lo/*/out.pdf`). `dotnet/CLAUDE.md` names
`sectors-defense-and-aerospace.xlsx` (227 → 449), `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls`
(109 → 88) and `grants-2005.xls` (220 → 201) as documents whose *reference* page count moved
across that version change. Our page counts for those three are **449, 88 and 201** — the
26.2.4.2 answers, exactly. #163, #183 and #184 are version divergences.

---

**RC-1 · Digit width per em is calibrated to 26.2.4.2 (version divergence).** `#163`.
`SheetFonts.cs:237` `DigitWidthCarry = 0.57`; its own remarks record it was `0.67` for
24.2.7.2 and that the two windows do not overlap. Probed here on the installed 24.2.7.2:
Calibri 12 pt column of width 40 is **4840 twips in the reference and 4880 in ours**
(d = 121 vs 122); Calibri 11 pt agrees at 111 either way. Two columns per page against one,
227 pages against 449. Patch supplied as `DO-NOT-APPLY-…`. **Confidence: certain.**

**RC-2 · The print range of a sheet that declares none.** `#001` (97 pages against 96);
suspected on `#066`. `SheetLayout.PrintedRange` widens `UsedRange` by cell fills, borders,
drawings and text overflow; it does not reach a column whose only claim is a `<col>` entry.
Probed on the document itself: deleting `<col min="6" …>` moves the **reference** from 96
pages to 97 and its zoom from 46 to 52 — our exact output; widening that column to 30
characters moves the reference to zoom 42, as the fit-to-width arithmetic predicts. So the
reference's print range reaches column **F** and ours reaches **D**. No patch — the exact
Calc predicate is not isolated. **Confidence: high on the mechanism, none on the predicate.**

**RC-3 · Printed row and column headings are drawn at a fixed 10 pt.** `#136`.
`SheetPageDecoration.cs:863` `Box` uses `SheetBandText.DefaultSize`; the strip around it is
already `* _scale`. Calc scales the text with everything else (`aOffsetMode` carries the zoom,
`printfun.cxx:2642, 2350-2357`). Probe on 24.2.7.2 at `scale="30"`: reference heading `A` is
3.35 pt, ours 11.17 pt; at `scale="100"` both 11.17 pt. Patch `heading-label-scale.diff`.
**Confidence: certain.**

**RC-4 · Header/footer field codes are read case-sensitively.** `#012`. `&p` falls through
`SheetHeaderFooter.ParseCodes`' switch and is swallowed, so the footer prints `Page` where
the reference prints `Page 2`. Calc upper-cases the token first ("ignore case of token codes",
`pagesettings.cxx:565`). Probe: `Page &p of &n` renders "Page 1 of 1" in the reference and
"Page of" in ours. Patch `header-footer-code-case.diff`. **Confidence: certain.**

**RC-5 · BIFF custom-view blocks are not skipped.** `#109`, plus two passing `.xls`.
`USERSVIEWBEGIN`/`USERSVIEWEND` bracket a saved view's own `HEADER`, `FOOTER`, `SETUP` and
margins; LibreOffice ignores the whole block (`read.cxx:952-966`, `#i39464#`) and we let the
last one win. `programs contact list…xls` holds six `HEADER` records in one substream.
Patch `biff-custom-views.diff`. **Confidence: certain on the mechanism; the two other
workbooks that carry custom views currently pass and could move either way.**

**RC-6 · Cell text spilling past a horizontal page break is not drawn on the next page.**
`#174` (we render the page blank). Already diagnosed in
`XRay.Office.Spreadsheets/TODO.md` with the fix named. No patch. **Confidence: certain.**

**RC-7 · Volatile date formulas print their cached value.** `#146`, `#187`. Both hold
`TODAY()`-derived chains with stale `<v>`; the reference recalculates on load. Needs a formula
evaluator. No patch. **Confidence: certain on cause, out of scope for a patch.**

**RC-8 · `AAA`/`AAAA` day-of-week format codes are not recognised.** `#146` prints the literal
`aaaa` from `mm/dd/yy\ aaaa`. The fix is in **`XRay.Office.Core/Numbers`** — cross-lane.
**Confidence: certain.**

**RC-9 · A pivot table's output range gets none of Calc's pivot cell styles.** `#098` (ink
×0.34): every rule of a three-column DataPilot grid and the frame down the left margin are
missing. `dpoutput.cxx:1245` applies `STR_PIVOT_STYLENAME_*` over the output. No patch.
**Confidence: high.**

**RC-10 · A sheet's `<picture>` background is not drawn.** `#136` (the tiled DRAFT
watermark). LibreOffice imports it as `PROP_BackGraphic`, `GraphicLocation_TILED`
(`pagesettings.cxx:993`). Two corpus workbooks use it. No patch. **Confidence: certain.**

**RC-11 · Unresolved marginal drift.** `#082 #105 #127 #131 #134 #135 #144 #153 #170 #177
#180 #189`. Column widths a fraction out, line spacing looser, a lost bold on a `.xls`, a
watermark picture undrawn, a header band clipped 2 pt lower. Individually small; none
isolated to a mechanism. `SheetGrid.IsOptimalSize` is **not** the "read but never used"
property the brief asked about — `SheetOptimalRowHeights.cs:257,261,300` consumes it.
**Confidence: low; these are a residue, not a cause.**
