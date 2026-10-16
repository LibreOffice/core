# words-spautofit-r182 — what `a:spAutoFit` does to a shape's height

Twelve authored boxes, one `wps:wsp` each, `wrap="square"`, insets 91440/45720 EMU, filled
`00B0F0` so the shape's own rectangle is measurable from the PDF's fill operator rather than
from its text. Three text lengths (one, three and six lines of 16 pt Liberation Sans) crossed
with two stated `a:ext/@cy` values (200 pt and 40 pt) crossed with `a:noAutofit` and
`a:spAutoFit`.

Rendered with `/opt/libreoffice26.2/program/soffice --convert-to pdf`, 26.2.4.2
`0229ac93fcf0d7cbc6376066c6f35021cef002dc`. The figure is the filled rectangle's height in
points, read out of the page's drawings.

| lines | stated `cy` | `a:noAutofit` | `a:spAutoFit` |
|---:|---:|---:|---:|
| 1 | 200 pt | 200.00 | **25.55** |
| 1 | 40 pt | 40.00 | **25.55** |
| 3 | 200 pt | 200.00 | **62.45** |
| 3 | 40 pt | 40.00 | **62.45** |
| 6 | 200 pt | 200.00 | **117.80** |
| 6 | 40 pt | 40.00 | **117.80** |

Two rules, and the second is the one that mattered:

* **`a:noAutofit` keeps the stated height exactly**, whatever the text does — six 16 pt lines
  in a 40 pt box still fill 40.00 pt, and the lines that do not fit are not formatted at all
  (`PageFrame.HasFixedHeight`, measured separately in `probes/words-extra-01`).
* **`a:spAutoFit` discards the stated height in both directions.** The same three text lengths
  give the same three heights whether the file says 200 pt or 40 pt, so `cy` is read as
  neither a floor nor a ceiling. The step is 18.45 pt a line — 36.90 from one line to three and
  55.35 from three to six — and one line comes to 18.35 plus the 7.2 pt of top and bottom
  inset.

This is *not* ODF's rule, which is why `PageFrame.HeightFloor` exists: `fo:min-height` is a
floor with `svg:height` as the starting height, so an ODF frame with less text than it states
keeps its height. A DrawingML autofitting shape does not.

The document that paid for it is `words/done-016/docx/HC-Bulletin-template.docx`, whose page 5
holds one such box stating `cy="1403985"` — 110.55 pt — around three 16 pt paragraphs. Read out
of the two PDFs' fill operators: both draw the rectangle at the same x, the same top edge and
the same 508.50 pt width, and then **ours is 110.55 pt tall against the reference's 65.75**,
which is the 0.9134 in the reference's own `--convert-to fodt` writes for it.
