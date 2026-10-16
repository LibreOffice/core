# A floating table: where it goes across, and where the flow goes round it

*Measured 2026-09-03 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`, reference `soffice` the
distro's **24.2.7.2**. Every figure below is a fresh measurement in that environment.*

## Where this started

`087_Printable_Graph_Paper_Template_Green_Theme` was the worst document on the words track by
first-page ink — **55.25** — and its defect is one a picture makes obvious and a gate column cannot
see: the reference draws its grid from (35.5, 51.4) pt and we drew it from (70.6, 71.5), the same
size and the same 13.92 pt pitch, a whole cell and a half out of phase in both directions. Seven of
the eight `Printable_Graph_Paper_Template` documents were in the ink table's top twenty.

The grid is a **floating table** — `w:tblpPr` — and two of the three things that places one were
missing.

## The probes

`position.py` writes nine one-table documents, each a 2 × 2 table 200 pt wide followed by a
paragraph reading `AFTER`, on an A4 page with 72 pt margins. The observable is read from the PDF
text layer with `pdftotext -bbox`, so it is the cells' own text rather than ink.

| probe | `w:tblpPr` | 24.2.7.2 draws `A` at | and `AFTER` at |
|---|---|---|---|
| `T_flow` | *(none)* | 78.00, 73.03 | 72.10, 133.03 |
| `T_pageY` | `vertAnchor=page tblpY=1440` | 71.65, 73.03 | **266.25, 72.03** |
| `T_marginX` | `horzAnchor=margin tblpX=-594` | **41.95**, 73.08 | 236.55, 72.03 |
| `T_pageX` | `horzAnchor=page tblpX=1440` | 71.65, 73.08 | 266.25, 72.03 |
| `T_center` | `horzAnchor=margin tblpXSpec=center` | 203.15, 73.08 | 72.10, 72.03 |
| `T_textY` | `vertAnchor=text tblpY=720` | 71.65, 109.03 | 72.10, 72.03 |
| `T_wide` | as `T_pageY`, table the column's width | 71.65, 73.03 | **72.10, 133.03** |

Three things fall out of that table.

### 1. A floated table sits 6.35 pt left of where the same table sits in the flow

`T_flow` against `T_pageY`, which differ in nothing but the `w:tblpPr`: 78.00 against 71.65.

`DomainMapperTableHandler::endTableGetTableStyle` moves the frame left twice — by the first cell's
left margin when `compatibilityMode` is below 15
(`sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx`:543) and by half the first cell's
left border always (:612), both through `lcl_DecrementHoriOrientPosition`. A 108-twip cell margin
plus half a one-point border is 5.9 pt of the 6.35, the rest being the text-origin systematic this
reader already carries in the control row.

Unlike the `w:tblInd` rule beside it, the two are a **sum** rather than the larger of the two.

### 2. `w:tblpX` was not read at all

`T_marginX` moves `A` from 71.65 to 41.95 — exactly the 594 twips it states. This reader took
`w:tblInd`, which these files do not state, and left the table at the margin. On 087 that is the
whole 35 pt.

`w:horzAnchor="page"` measures from the sheet's own left edge, which nothing on the way to
`PageTable` carries; applying the offset against the text area instead would move `T_pageX` by a
whole margin, so a page-anchored table keeps the placement it had. Seven of the corpus's fifty
positioned tables say `page`.

### 3. The flow goes *beside* a fly where there is room and *under* it where there is not

`T_pageY` and `T_wide` differ in nothing but the table's width. `AFTER` lands at **x = 266.25**,
level with the table's first row and hard against its right edge, in the first; and at
**y = 133.03**, under the table's last row at the column's own left edge, in the second.

**Writer's test is whether the first word fits the strip**, and it is content-dependent. Sweeping
the fly's width against a fixed `AFTER`:

```
fly / column   0.50   0.70   0.80   0.90   0.95   0.96   0.98   1.00
AFTER          beside beside beside beside beside under  under  under
```

and replacing `AFTER` with a word five times as long moves the switch from between 0.95 and 0.96 to
somewhere below 0.80. The strip at 0.95 is 28.9 pt and `AFTER` at 10 pt is about 27; at 0.96 it is
22.6 pt and it is not.

That is line breaking, which this engine cannot do — so `Paginator.FillsTheColumn` uses the case
where *no* word could fit: the room left beside the fly is under a twentieth of the column. The
corpus makes the threshold safe rather than arbitrary. Censused over its **46 positioned tables
that state a grid**, the fly's width against its column is bimodal with nothing at all between
**0.911 and 0.960**:

```
  0.450 x3   0.531   0.709   0.886   0.911 | 0.960   0.971   0.994 ... 1.481
```

Six narrow flies, forty that fill their column — and forty that *overflow* it, which is what a
graph-paper grid pulled into both margins does.

### And only for a fly the file has moved off the flow

`w:vertAnchor="text"` with a small `w:tblpY` — which is 26 of the corpus's 50 — puts the fly where
the flow already is. Leaving such a table in the flow reproduces it exactly; floating it drops the
table's own space above and below, which the flow still owes.

Measured: `slcc-architecture-uu-architecture.docx` states `vertAnchor="text" tblpY="1"` on a fly
that fills its column, matches the reference at **4 pages** with the table in the flow, and comes to
**3 pages and 24 words short** with it floated. Two more — `461249.docx` and
`AW-104D-RVSM-Aircraft-Approval-Checklist.pdf.docx` — move the same way and by less. So the
displacement is applied only to a `page`- or `margin`-anchored fly, which costs
`5709.16 ch.40_mgfinal` 0.74 of ink and is worth it.

## What the two together are worth

First-page ink against the reference, over the 42 corpus documents holding a `w:tblpPr`:

| | before | after |
|---|---:|---:|
| `087_Printable_Graph_Paper_Template_Green_Theme` | 55.249 | **19.184** |
| `084_Printable_Graph_Paper_Template_Editable_Layout` | 48.304 | **32.372** |
| `081_Printable_Graph_Paper_Template_Blue_Theme` | 40.925 | 37.250 |
| `HC-Bulletin-template` | 40.690 | 38.411 |
| `system_design__technical_architecture_template` | 4.681 | 4.804 |
| **42 documents, mean** | **18.672** | **17.295** |

The one that gets worse is the one whose fly sits closest to the threshold, at 0.960 — the honest
cost of a fixed proxy for a rule that is really about the width of one word.

## What is left, and why 087 still fails its page count

087 is still two pages against the reference's one. Its grid is now placed exactly —
**35.52 .. 557.76 pt across in both renderings** — and what is left is vertical: 670.08 pt of grid
in the reference against 672.48 here, over 48 rows, which is **one twip per row**. That is enough
to push its trailing `Title: ___ Date: ___` off the page the reference keeps it on.

One twip a row is row-height rounding and nothing more. It is emphatically *not* the
`w:trHeight`-and-borders divergence below, and the arithmetic says so: 087's rows are
content-driven rather than resting on their floors — they state 12217 twips between them and are
drawn 670 pt tall by both engines — so the floor rule cannot reach them, and its 0.25 pt border
would be 12 pt over the table rather than 2.4.

## Not a defect: `w:trHeight` and the row's borders, on 24.2.7.2

Chasing 087's residual height turned this up. It does not explain 087 — see above — but it is a
real divergence on other documents in the family, and it is worth writing down because it looks
exactly like a defect and is not.

`probes/words-pagination-01/row-min-height-border.py` exists to answer whether a row's `w:trHeight`
floor sits under its borders or includes them. Re-run here against **24.2.7.2**:

```
  w:sz  border pt      rule  LibreOffice     ours    diff
     0       0.00   atLeast        24.00    24.00   +0.00
     4       0.50   atLeast        24.00    24.50   -0.50
     8       1.00   atLeast        24.00    25.00   -1.00
    16       2.00   atLeast        24.00    26.00   -2.00
    24       3.00   atLeast        24.00    27.00   -3.00
    16       2.00     exact        24.00    24.00   +0.00
```

The same probe against **26.2.4.2** — which is what `TableLayouter`'s behaviour was built from, and
whose figures its own remarks quote — read **24.00 / 24.50 / 25.00 / 26.00 / 27.00**, the floor plus
one border. Independently, six rows of `w:trHeight="274"` holding an empty 2 pt paragraph come to
82.20 pt with no borders in both renderers — exact — and to 83.70 in this container's reference
against 92.70 here once a 1.5 pt grid is added, which is one border against seven.

**The C++ in this checkout settles which of the two is the target.** `lcl_CalcMinRowHeight`
(`sw/source/core/layout/tabfrm.cxx`:5085-5097) adds `lcl_GetTopSpace` to the floor under
`MIN_ROW_HEIGHT_INCL_BORDER`, and `lcl_GetTopSpace` is `SvxBoxItem::CalcLineSpace(TOP, true)` — the
border's **line width and its distance**, not the distance alone. So the tree's own reference source
adds the border, 26.2.4.2 does, and 24.2.7.2 — which predates it — does not.

So this reader is right and the container's reference is old, and it must not be "fixed": doing so
would calibrate the tree to a LibreOffice older than the one it targets. `080_..._Black_Theme` is
where it shows on this family — its rows come out 0.85 pt tall against the reference's stated
floors, under a `w:sz="12"` grid — and it is the same thing `renderer-parity-sweep-01`'s L2 lane
proposed as `row-height-floor.diff` and that round held back for the same reason. If a later round
sees a bordered table running long against this container, this is why.

## Reproducing

```sh
python3 position.py <dir>
for f in <dir>/*.docx; do
  "$XRAY_OFFICE_CLI" render "$f" --outdir ours
  soffice --headless --convert-to pdf --outdir ref "$f"
done
pdftotext -bbox -f 1 -l 2 <pdf> - | grep '<word'
python3 ../words-pagination-01/row-min-height-border.py /abs/scratch "$XRAY_OFFICE_CLI"
```
