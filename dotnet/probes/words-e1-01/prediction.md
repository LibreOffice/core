# words-e1-01 — prediction, written before any fix was measured

Round `words-e1-01`, 2026-08-15, worktree `wt-w-e1`, branch `wt-w-e1`. Reference LibreOffice
**26.2.4.2**, `check-env.sh` green (Carlito, Caladea, Liberation, DejaVu all resolving), references
reused from `/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words/`, `SOURCE_DATE_EPOCH=1700000000` on
every render that is diffed.

## Baseline, measured before this file was written

Whole track rendered once with the tree as merged (`886bcde7091`) and verdicted against the banked
references with `lineheight-01/verdict.py`:

```
words track          173 of 200 match
words/done-*         158 of 159   (airbus-pdf-information-package_v1-4.docx, 1272/1299, band 25.98)
words/extra-001        2 of 7
```

`extra-001`, per document:

| document | verdict | pages | words |
|---|---|---|---|
| `762.doc` | pages | 22/23 | 4120/4122 |
| `info-bulletin-601.doc` | pages | 5/6 | 1298/1302 |
| `ABCD-FE-01-00 Flight Envelope` | pages,words | 14/15 | 3844/3720 |
| `ABCD-SDE-23-00 Avionic System Description` | **match** | 29/29 | 8419/8402 |
| `ABCD-WB-08-00 Weight and Balance` | **match** | 12/12 | 2639/2612 |
| `UG.CAO.00006 …User Guide for Applicants` | pages,words | 30/29 | 8011/7399 |
| `UG.CAO.00133 …Language` | words | 18/18 | 3899/3667 |

So the brief's "5 failing" reproduces exactly, and `ABCD-WB-08-00` — which `words-extra-01` left at
+57 against a 55.2 band — has since crossed into `match` on a later merge.

## What I am going to do, and why in this order

Severity, not listing order:

1. **`info-bulletin-601.doc` renders as five blank pages.** Verified in the operators before any
   picture was looked at: on all five of our page streams the single `/Im … Do` sits *after* the
   last `BT`, and it is preceded by a full-page white `re f`; the reference's image sits at byte
   ~195 of the stream, before every `BT`. A blind reviewer who was given only the composed pair and
   forbidden the repo described our half as "a solid black band, then blank white all the way down".
   That outranks every word count in this group.
2. **We draw no fill and no outline on a DrawingML text box.** `words-extra-01` §"Two findings
   outside the brief" recorded it; a third blind reviewer, on a document neither of the first two
   saw (`ABCD-FE-01-00` page 13), independently reported the reference's grey header band, its dark
   grey "Document reference." bar and its filled footer boxes as absent from ours.
3. Everything else in the group is either the reference's own defect or measured against a
   reference that cannot draw what the document holds.

## Predictions

| # | claim | conf |
|---|---|---:|
| P1 | The `info-bulletin-601` image is behind-text by LibreOffice's own `bMoveToBackground` rule — `bDrawHell` (Escher `fPrint` bit 5, `fBehindDocument`) **or** an anchor in the header/footer story with `nwr == 3`. | 80% |
| P2 | Painting behind-text frames before the header and body makes `info-bulletin-601`'s five pages legible — the reviewer's "blank white" region fills with text. | 85% |
| P3 | It does **not** flip `info-bulletin-601` to `match`: the document is one page short (5 against 6) and paint order moves no line. | 80% |
| P4 | `762.doc` is **not** the same defect. Its only image-bearing page is page 1, and there our `/Im` ordering already agrees with the reference's (one image early, one late, on both sides). The brief asserts both documents share it; I expect that half of the brief to be wrong. | 85% |
| P5 | `UG.CAO.00006` is the same LibreOffice table-only-header import defect as `UG.CAO.00133`: its `word/header1.xml` has `['tbl']` as its only top-level child, and its per-page surplus is a flat ~+20 words on pages 2–13. Not ours, not fixed. | 85% |
| P6 | Reading `wps:spPr`'s `a:solidFill` and `a:ln` moves **no verdict at all** on the 200 — it is ink the gate cannot see. | 75% |
| P7 | Between them the two changes alter **10–40** of the 200 words renderings, and no more than 2 verdicts in either direction. | 50% |
| P8 | No `words/done-*` document loses its verdict; the track stays at 158 of 159. | 70% |
| P9 | Fidelity stays at **30 failed of 550**, checked by name before and after. | 75% |
| P10 | The paint-order fix needs a new `PageFrame` property, one branch in `PageDrawing.Draw`, and one rule each in `Ww8Frames` and `DocxFrames` — no change to the layouters, because z-order is not layout. | 80% |
| P11 | `ABCD-FE-01-00` does not flip. Its residual is a reference that draws nothing for 33 `m:oMathPara` (no `libreoffice-math` installed) plus a VML WordArt watermark (`_x0000_t136`, `PowerPlusWaterMarkObject`) neither renderer-side change here touches. | 85% |

## Refutation criteria, written now

- If more than **3** documents lose their verdict on the 200 under either change, the change is
  wrong in kind and comes out rather than being tuned.
- If `info-bulletin-601`'s page 3 is still blank after the paint-order change, then the frame is not
  reaching `page.Frames` at all and the whole diagnosis above is wrong.
- If the shape-fill change puts an opaque box over text on any document, the fill is being read from
  the wrong element — most likely an `a:solidFill` under `a:rPr` (a text colour) rather than under
  `wps:spPr`.

## Deliberately not attempted, named now rather than after the fact

- `762.doc`'s one-page shortfall. It is a flow difference spread over 23 pages (±2 to ±17 words a
  page, no single site), not the raster defect the brief attributes to it.
- `UG.CAO.00006` and `UG.CAO.00133`. Following the reference here means deleting
  `SectionInheritedHeaderTests`, which exists to say why not to. Two verdicts, knowingly left.
- `ABCD-FE-01-00`. Its reference is measuring a LibreOffice with no Math module; `MISSING_PACKAGES.md`
  says installing it is a re-banking decision and not one to take mid-measurement.
- The VML WordArt watermark. A preset text path with its own fill is a feature, not a fix.
