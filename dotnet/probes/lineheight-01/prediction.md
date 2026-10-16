# lineheight-01 — predictions, written before any measurement of my own

Round `lineheight-01`, 2026-08-15, worktree `wt-lineheight`. Reference LibreOffice **26.2.4.2**.

Everything below was written from **reading the C++ tree plus arithmetic on the six (face, size)
points already published** in `dotnet/probes/words-pages-01/results.md` §4 and
`dotnet/probes/words-regress-01/results.md` §1B. No new probe had been run when this was committed.

## The rule I claim

LibreOffice does not scale a face's `hhea` metrics to the point size. It quantises them onto the
**reference device's pixel grid**, twice, and the two prior rounds searched the wrong grid.

Writer's reference device, when the document does not ask for printer metrics, is a
`VirtualDevice` created by `DocumentDeviceManager::CreateVirtualDevice_()`
(`sw/source/core/doc/DocumentDeviceManager.cxx:259`) with
`SetReferenceDevice(VirtualDevice::RefDevMode::MSO1)` and `MapUnit::MapTwip`. And
`MSO1` is **`6*1440` = 8640 dpi** (`vcl/source/gdi/virdev.cxx:407`) — *not* 600, and not
anything in the 72–6000 range the last round swept. One twip is exactly six device pixels.

Then, with `H` the font height in device pixels and `u` the units per em:

| step | where | arithmetic |
|---|---|---|
| 1 | `OutputDevice::ImplNewFont` → `CoordinateMapper::LogicToViewDistanceY` | `H = llround(size_twips * 6)` — exact, no loss |
| 2 | `FontMetricData::ImplCalcLineSpacing` (`vcl/source/font/fontmetric.cxx:538-540`) | `a = round(hheaAsc*H/u)`, `d = round(-hheaDesc*H/u)`, `g = round(lineGap*H/u)`, **each to a whole device pixel, separately** |
| 3 | `LogicalFontInstance::mnLineHeight` (`vcl/source/outdev/font.cxx:910`) | `a + d`, still in pixels |
| 4 | `OutputDevice::GetTextHeight` (`vcl/source/outdev/text.cxx:640-651`) | `textHeight = llround((a+d)/6)` — the **sum** converted once |
| 5 | `OutputDevice::GetFontMetric` (`vcl/source/outdev/font.cxx:355`) | `extLeading = llround(g/6)` — converted **separately** |
| 6 | `SwFntObj::GetFontHeight` (`sw/source/core/txtnode/fntcache.cxx:336-376`) | line height = `textHeight + extLeading` |

`round`/`llround` are round-half-**away from zero**, which is what C++ gives and what .NET's
default `Math.Round` does **not**.

This explains the two facts that refuted every previous candidate at once:

- **Why the sum does not decide.** Ascent and descent are rounded to pixels *separately* at step 2
  before being added at step 3, so two faces with the same `hhea` total can land a pixel apart.
- **Why per-component rounding also failed.** The line gap is converted back to twips on its own
  (step 5) while ascent and descent share one conversion (step 4). It is neither "round three" nor
  "round one" — it is **two** roundings of a three-term sum, grouped 2+1, on both grids.

Checked by hand against all six published points before this was written:

| face | pt | exact twips | ours today | LibreOffice | this rule |
|---|---:|---:|---:|---:|---:|
| Liberation Serif | 10 | 229.980 | 230 | **231** | 231 |
| Liberation Sans | 10 | 229.980 | 230 | 230 | 230 |
| Liberation Sans | 13 | 298.975 | 299 | **300** | 300 |
| Liberation Sans | 16 | 367.969 | 368 | **369** | 369 |
| Carlito | 18 | 439.453 | 439 | **440** | 440 |
| DejaVu Sans | 12 | 279.375 | 279 | **280** | 280 |

Six of six, including both faces of the Liberation 2355-unit refutation.

## Predictions

| | claim | conf |
|---|---|---:|
| P1 | The rule above is right in mechanism: an 8640 dpi twip grid, per-component pixel rounding, ascent+descent converted together and the gap separately | 85% |
| P2 | It reproduces **195 of 195** measured (face, size) line heights | 80% |
| P3 | It also reproduces the measured **ascents**, as `llround(a/6) + llround(g/6)` (Writer charges external leading to the ascent) | 70% |
| P4 | .NET's banker's `Math.Round` in `MetricGrid` is a second, independent defect: at least one of the 195 needs `MidpointRounding.AwayFromZero` to come out right | 90% |
| P5 | The existing `MetricGrid` type needs **no structural change** — `TextHeightOn` already groups 2+1 correctly; only the grid it is handed and the midpoint mode change | 70% |
| P6 | `words/done-015/docx/Sample_SQMS_Program.docx` flips to 61 pages and `match` | 70% |
| P7 | More than 100 of the 200 words renderings change bytes | 60% |
| P8 | The words track `match` count does not fall below its current 163 | 55% |
| P9 | `words/ceiling-001/doc/1447.doc` improves its page count | 35% |
| P10 | `XRay.Office.Fidelity.Tests` does not get worse than the 30-of-550 baseline | 50% |
| P11 | Our `hhea`/`OS/2` precedence is **not** the defect — the brief's untested assumption survives | 85% |
| P12 | Scoping the grid to the Writer path leaves slides and sheets **byte-identical** | 80% |
| P13 | Calc's reference device is also MSO1/8640 but in 1/100 mm, and Impress's is 600 dpi in 1/100 mm, so both have their *own* grids that this round does not implement | 75% |

## Falsification tests, fixed in advance

- If the rule misses **any** of the 195 pairs, it is wrong as stated and I will say so rather than
  add a term to make it fit. A rule that needs a per-face constant is the fudge this round exists
  to avoid.
- If more than **three** `words/done-*` documents lose their verdict, the grid is wrong for some
  path I have not identified and the change does not land, whatever `Sample_SQMS_Program` does.
- If slides or sheets move a single byte while the grid is scoped to Writer, my model of where the
  metric is consumed is wrong.

## Baseline to establish first

`XRay.Office.Fidelity.Tests` at **30 failed of 550**, on the unmodified tree, before anything is
changed.
