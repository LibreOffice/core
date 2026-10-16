# Where a header or footer band is clipped

Round `clip`, 2026-09-05. Environment: this container, `/usr/bin/soffice` **24.2.7.2** and
`/opt/libreoffice26.2/program/soffice` **26.2.4.2** (TDF tarball, its 33 duplicate font files
moved aside), `fc-match "DejaVu Sans"` → DejaVu, Carlito and Caladea installed.

## What was asked

`ScPrintFunc::PrintHF` sets one clip region before it draws a band's three areas
(`sc/source/ui/view/printfun.cxx:1870`) and every area goes through it. `SheetPageDecoration`
already knew that rectangle existed — it used it to decide whether an area is drawn **at all** —
and read `ImpEditEngine::DrawText_ToPosition` as leaving a partly-overlapping area unclipped.
That reading was wrong. `editeng/source/editeng/impedit3.cxx:3367-3389` has three branches, not
two: no overlap draws nothing, wholly inside draws unwrapped, and **anything else is embedded in
a `MaskPrimitive2D` of the clip polygon** — which keeps every line in the primitive tree, and
cuts all of them geometrically when they are rendered.

So the question is where the rectangle is, and it is readable straight out of the reference's own
content stream as a `re W* n` rather than inferred from ink. `probe.py` prints it for both
references and for us.

## Result

`sheets/done-011/xlsx/FY2023-AIP-grants.xlsx`, page 3, in top-down page points. Its header is
three centred lines (`&C…\n…\n…`), its footer one; its print scale is 43 %; its page is
792 × 612; its margins are 0.25 in left and right, 0.75 in top and bottom, 0.3 in header and
footer.

| side | top | height | left | width |
|---|---:|---:|---:|---:|
| 24.2.7.2 header | 21.587 | 13.910 | 17.995 | 755.940 |
| 24.2.7.2 footer | 584.586 | 5.740 | 17.995 | 755.940 |
| 26.2.4.2 header | 21.607 | 13.889 | 17.996 | 755.897 |
| 26.2.4.2 footer | 584.585 | 5.741 | 17.996 | 755.940 |
| ours, after | 21.600 | 13.932 | 18.000 | 756.000 |

**The rectangle is as tall as the band at the print scale and as wide as the band at full size.**
The header band the margins imply is `0.75 − 0.3 = 0.45 in = 32.4 pt`, and `32.4 × 0.4293 = 13.91`;
its width is `774 − 18 = 756` with no scale at all. That asymmetry is the one
`SheetPrintSetup.PrintableAreaAt` already carries and cites: `aPageRect` is in *document* twips,
so `nLineWidth` comes off it and arrives unscaled while `nHeight − nDistance` is added whole and
arrives at `nHeight × zoom` (`ScPrintFunc::GetDocPageSize`, `printfun.cxx:3002`, over the map
mode's zoom fraction at `:2645`). The footer confirms the same arithmetic from the other end: its
band is dynamic, so `nHeight − nDistance` is its own one-line text height of 13.4 pt, and
`13.4 × 0.4293 = 5.75`.

We emit no footer clip on this document because our footer's ink is wholly inside its rectangle;
the reference emits one and draws the same thing.

## Consequence on the page

The header's third line, `As of 10/20/2023`, does not fit a band that holds two. Both engines
draw it. The reference cuts it at 35.50 pt — `pdftotext` still reads all three lines off its page
— and before this round we drew the whole line over the sheet's own header row.

## Reach

Measured by rendering the 171 sheets documents that carry a header or footer, or that are not a
zip (so `.xls` and `.ods` are all included), through the binary either side of the change, and
comparing the PDFs byte for byte: **22 of 171 moved**. Scored against both references over up to
six pages each, mean ink at 60 dpi grayscale:

| | 24.2 before | 24.2 after | 26.2 before | 26.2 after |
|---|---:|---:|---:|---:|
| `020_Free_Blood_Pressure_Chart…` | 0.769 | **0.612** | 2.704 | **2.516** |
| `CSA_CCM_v1.2.xls` | 2.848 | **2.701** | 2.161 | **2.014** |
| `TICAPCapability_Final.xls` | 9.420 | **9.361** | 9.421 | **9.363** |
| `Hazard Analysis Template.xls` | 7.107 | **7.056** | 7.564 | **7.512** |
| `RMP 2011-2014 and Inventory.xls` | 3.675 | **3.655** | 3.471 | **3.451** |
| `orbus_togaf_tool_csq.xls` | 4.845 | **4.827** | 4.372 | **4.353** |
| `FY2023-AIP-grants.xlsx` | 13.484 | **13.477** | 13.191 | **13.184** |
| 12 others | | ≤0.01 better | | ≤0.01 better |
| `066_Agile_Gantt_chart` | 4.092 | 4.093 | 3.911 | 3.912 |
| `027_Simple_personal_cash_flow_statement` | 3.535 | 3.537 | 6.372 | 6.373 |

Nineteen improved, one unchanged, and two moved the wrong way by 0.001–0.002 — a thousandth of a
grey level, which is a rounding of the clip edge and not a visible change. Nothing in the set
lost a word: the clip is `ClipPathKeepingText`, so the glyphs stay in the PDF's text layer
exactly as the reference's do.

The change cannot reach a document with no header or footer, and at an unscaled sheet the new
rectangle is the same figure as the old one, so only a *scaled* sheet whose band overflows can
move at all.

---

## Correction, same round: the rejection branch had the rectangle in the wrong space

The reach census above scored **ink only**, and it missed a regression that ink cannot see. On
`sheets/done-014/xls/TICAPCapability_Final.xls` the ink *improved* — 9.420 to 9.361 against
24.2.7.2 — while six of its seventeen pages silently lost their whole header and their whole
footer, seventeen words each, on pages that had been word-exact against the reference. Three more
documents lost 8, 18 and 195 words the same way. **Ink at 60 dpi over six pages cannot see two
lines of 4 pt text disappear; a word count can.** Every census in this directory now scores words.

### What was wrong

`DrawBand` holds two figures in two different spaces. `bandText` is the text at the print scale —
`SizeOf` multiplies every em size by the zoom — while `height` is the band as the file states it,
unscaled. Calc has no such split: `PrintHF` does its whole arithmetic in logical twips and lets
the map mode apply the zoom to all of it at once (`printfun.cxx:1867`, `:2645`), so
`nDif = paperHeight − textHeight` reaches the paper as `(paperHeight − textHeight) × zoom / 2`.
Comparing a scaled text against an unscaled band overstates `nDif` by `height × (1 − zoom)`.

That is exactly nothing on an unscaled sheet, which is why it had never been visible. TICAP's
sheets print at 57 % and 35 %. At 35 % its band is 30.16 pt stated and its text 7.82 pt drawn, so
the pen went 6.17 pt below the band's top and the ink began at 47.9 against a rectangle ending at
46.6 — **outside the window it was then tested against**. The first port of the clip derived that
window a second time from `top` and `height` instead of taking it from where the text had
actually been put, so the two drifted apart and `if (!overlaps) return;` threw the area away.
`aStart` is one variable in Calc: `PrintHF` passes it to `SetClipRegion` and to each
`DrawText_ToPosition` alike (`printfun.cxx:1870-1912`). The clip's origin is now `bandTop`, and
the band's own arithmetic is done at the print scale throughout.

### The control that says the arithmetic is right rather than merely harmless

`tests/corpus/features/sheet-band-scale-pinned.fods` — letter portrait at `style:scale-to="35%"`,
a 0.42 in band with no gap, one short line in each band, so both have room to spare. ODF is the
format that can state a pinned band; `XlsxPrintSetup` flags every SpreadsheetML band dynamic.
Ink-box tops in page points:

| | header | body | footer |
|---|---:|---:|---:|
| 24.2.7.2 | 39.340 | 46.834 | 748.702 |
| 26.2.4.2 | 39.340 | 46.834 | 748.702 |
| ours, before the correction | *not drawn* | 46.822 | *not drawn* |
| ours, after | **39.372** | 46.822 | **748.788** |

Both references centre that header 3.34 pt below the band's top — `nDif/2` at the print scale —
and so do we now, to 0.09 pt. Taking `nDif` unscaled put the pen 9.5 pt lower and off the end of
the rectangle.

### Reach of the correction

The same 171 documents, rendered through three builds — before the band clip, with it, and with
this correction — and compared. **Words first:** only 5 of 171 differ between the three builds at
all, and all four regressions are restored exactly.

| | ref 24.2 | ref 26.2 | before | with the clip | corrected |
|---|---:|---:|---:|---:|---:|
| `TICAPCapability_Final.xls` | 4936 | 4918 | 4946 | 4844 | **4946** |
| `disclosures_ecm.xls` | 3534 | 3534 | 3534 | 3526 | **3534** |
| `TOGAF9-Tool-ConfReqts-CSQ.xls` | 24097 | 24092 | 24205 | 24010 | **24205** |
| `environment-edb-…-databank.xls` | 65605 | 65422 | 65426 | 65408 | **65426** |
| `PBN Matrix NAAs (V01).xlsx` | 5567 | 5575 | 5585 | 5583 | 5583 |

TICAP page by page, extractable words, reference / before / with the clip / corrected: pages 4–9
run `290 290 273 290`, `564 564 547 564`, `417 417 400 417`, `212 212 195 212`,
`570 570 553 570`, `84 84 67 84`. No page of any document now holds fewer words than it did
before the band clip landed.

**Then ink.** The correction changes 25 of the 171 — every scaled sheet with a band — and against
24.2.7.2 it improves 16, worsens 3 and leaves 6 unchanged. The largest gains are
`Hazard Analysis Template.xls` 7.056 → **6.864**, `CSA_CCM_v1.2.xls` 2.701 → **2.568**,
`Application_Compliance_Checklist` 15.615 → **15.538**, `cy01_state.xls` 6.100 → **6.006** and
`NPIAS_App_A.xls` 1.933 → **1.852**; 26.2.4.2 agrees on every one of them.

### What it leaves worse, and why

- **`NPA_21_21_Sentenced_Comments.xls`, +0.050 against 24.2 and +0.054 against 26.2**, no word
  moved. Its header's ink box is at 16.685 in the reference, 16.456 before and **15.465** after —
  the correction moves it 0.99 pt *away*. The arithmetic is not the variable: the `.fods` control
  above lands on the reference to 0.03 pt with a band height read straight from the file. What
  differs is the **band height the BIFF reader computes** — too short here, and too tall on TICAP,
  whose header ends up 1.40 pt below the reference's for the same reason. Scaling `nDif` correctly
  exposes that as a ±1 pt residue instead of burying it inside a much larger one. It is a real
  open defect in `SheetBandHeight` on the `.xls` path and it is not addressed here.
- `disclosures_ecm.xls` 0.218 → 0.222, which is the cost of drawing the eight words it had been
  dropping; against the pre-merge build it is 0.223 → 0.222.
- `026_Monthly_cash_flow_statement` 4.658 → 4.659, a thousandth of a grey level.
