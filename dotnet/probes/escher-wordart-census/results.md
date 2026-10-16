# Escher WordArt in the binary formats — the reach, measured before the work

Round `agent/draw-shapes`, base `6bf527227`. Corpus `/home/user/sample-files`, 947 documents, of
which **181 are OLE2 binary** (`.doc .dot .ppt .pot .pps .xls .xlt`). Gate figures are from this
round's own before-sweep, `soffice` = 24.2.7.2.

## The census

`census.py` walks every `msofbtOPT` record in each binary document and counts the shapes carrying
`gtextUNICODE` (property 192, complex), which is the property
`SvxMSDffManager::ApplyAttributes` keys a WordArt shape on
(`filter/source/msfilter/msdffimp.cxx`). It is a byte scan rather than a stream walk because
Escher containers sit in three different places across DOC, PPT and XLS; a header is accepted only
when its version nibble is 3, its instance is a plausible property count and its stated length
covers that many six-byte entries, which is what keeps the scan from reporting noise.

```
181 binary documents scanned
4 carry Escher WordArt, 5 shapes in all
  2  words/done-014/doc/644730BRI0mna000BOX361539B00public0.doc
  1  slides/ceiling-002/ppt/pres_ioc_phuket.ppt
  1  slides/done-008/ppt/8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt
  1  words/done-011/doc/135.doc
```

This reproduces the figure an earlier round recorded — 5 shapes over 4 documents — independently.

## What those four documents do today

| document | verdict | pages | characters |
|---|---|---|---|
| `words/done-014/…/644730BRI0mna000BOX361539B00public0.doc` | **match** | 5/5 | 12838/12838 |
| `words/done-011/doc/135.doc` | **match** | 14/14 | 26659/26620 |
| `slides/done-008/…/8.16_AOD_FINAL_Provider_Training_Presentation_9_2009.ppt` | **match** | 94/94 | 22481/22481 |
| `slides/ceiling-002/ppt/pres_ioc_phuket.ppt` | **match** | 26/26 | 6090/6090 |

**All four already pass the gate**, and one of them is in `ceiling-002` — a batch whose word gate
cannot be won for an unrelated reason. So reading `msofbtTextPath` and the `gtext*` properties can
move **no gate verdict at all**, on any of the three tracks, and its whole visible effect is five
shapes in four documents out of 947.

## Recommendation, and what would change it

**Not worth a round at this reach**, which is the outcome the brief named as acceptable. The two
things that would change the arithmetic:

- **A corpus that holds more of them.** The population is a property of the sample, not of the
  format: WordArt is common in real DOC and PPT and this corpus happens to hold five instances.
  A census on a different corpus is one script and should be re-run rather than assumed.
- **A visible defect on one of the four.** Each currently draws *nothing* where the reference
  draws a warped string; the four documents are each out for unrelated reasons as well, so the
  shape's absence is not what any of their scores is measuring. If a reading of one of those four
  pages puts the missing WordArt at the top of what is wrong with it, that is a reason the gate
  cannot supply.

The engine the work would need is already here and that is not the obstacle: the preset geometry
and the fitting are in `XRay.Office.Ooxml/DrawingML/Fontwork*.cs`, keyed by LibreOffice's own type
names, and both the DrawingML and the VML readers already drive them
(`probes/words-vml-fontwork/results.md`). What is missing is only the Escher reader — the
`gtextUNICODE`, `gtextFont`, `gtextSize` and `gtextAlign` properties, and the shape type to
Fontwork-name mapping in `msdffimp.cxx:2516-2600`.
