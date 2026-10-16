# Round 145 — re-censusing O97, and why "150 `v:line` in 36 documents" counts markup nobody reads

Reference binary for anything measured: `/opt/libreoffice26.2/program/soffice` →
**LibreOffice 26.2.4.2**. **[src]** marks a reading of this tree (27.2.0.0.alpha0+, *not* that
binary's source); **[bin]** a measurement against 26.2.4.2's own output.

**This is a partial result and says so.** It was begun by a dispatched agent that the container
restart killed part-way. Its two censuses had been written to disk and are analysed here; its
authored probes (`make-probes.py`, `make-group-probes.py`, `render.sh`, `readprobes.py`) were
never run, so **the coordinate rule for `from`/`to` is still unestablished** and no reference
rendering of the surviving documents has been taken.

## The correction

O97 was seated as *"all 150 top-level `v:line` in a `w:pict` are `position:absolute` and state no
`style` width or height, so every one returns null and no rule is drawn — 36 documents"*, taken
as a by-product of round 140's VML census.

`vline-occurrences.tsv` is one row per `v:line` in the corpus's DOCX, with the element's
ancestry as well as its attributes:

| | occurrences |
| --- | ---: |
| total `v:line` | **865** |
| …inside an `mc:Fallback` | **859** |
| …not inside one | **6**, in **2 documents** |
| by depth | top-level 150, inside a `v:group` 715 |

**Every one of the 859 is the VML half of an `mc:AlternateContent` whose `mc:Choice` requires
`wps` or `wpg`** — `vline-branch.tsv` lists the requirement and the elements each choice holds,
and they are `wsp`/`wgp` shapes with `prstGeom` of `line`, `rect`, `ellipse`,
`straightConnector1` and so on. LibreOffice supports both namespaces, so it takes the **Choice**
branch and the VML line is never read.

**[src]** And so does this tree: `OoxmlXml.ResolveAlternateContent` (`OoxmlXml.cs`:73-85)
resolves every `mc:AlternateContent` when the part is loaded, on the *resolved URI* of each
`Requires` prefix rather than on the prefix text, and `OoxmlNamespaces` records the supported
set with the reach figure — 2324 choices — beside it.

So round 140's 150 is a count of **markup neither renderer looks at**. The live population is
six rules in two documents:

```
JEMIT_Template.docx  word/footer3.xml   from -.7pt,-3.5pt  to 179.3pt,-3.5pt  0.25pt windowText
33004.docx           word/header1..4    from 0,15.85pt     to 468pt,15.85pt   1pt    #006
33004.docx           word/footer18.xml  from 0,748.1pt     to 468pt,748.1pt   1pt    #006
```

All six are horizontal hairlines in a header or footer, all state `from`/`to` and no `style`
width or height, none states `stroked="f"`, and all six parse.

## What this does and does not settle

- **It resizes the seat by two orders of magnitude**, from 150 rules in 36 documents to **6 in 2**.
  A seat that reads "this tree draws none of the corpus's 150" is a claim about how much ink is
  missing, and 859 of those 150+715 are ink the reference does not draw either.
- **It does not refute the defect.** Six real rules in two documents are still six rules this tree
  does not draw, and they are page furniture — a rule under a header is exactly the kind of mark a
  reader notices.
- **The coordinate rule is still unknown.** Whether `from`/`to` are relative to the shape's
  `style:left`/`top` or absolute in its own space, and what an absent `style` position means, has
  to come from authored one-attribute probes rendered through 26.2.4.2 and read out of the PDF's
  path operators. `make-probes.py` is written and unrun.
## The ink, measured

The agent had rendered both witnesses through 26.2.4.2 before it died (`ref/`), so this much
*is* measured. `rules.txt` counts long thin horizontal rules per page — a stroked path or a thin
filled rectangle at least 100 pt wide and under 3 pt tall, because LibreOffice draws a hairline
either way and counting only strokes undercounts one side — and matches them with a 2 pt
tolerance, since an exact match reads every rule as missing from both sides.

| | pages | ours | 26.2.4.2 |
| --- | ---: | ---: | ---: |
| `JEMIT_Template.docx` | 4 | 21 | 22 |
| `33004.docx` | 47 | 335 | 409 |

**And the missing rule is identifiable by its width**, which is what makes this a measurement of
*this* defect rather than of the documents' other differences:

- `JEMIT_Template` page 1: the reference draws a **180 pt** rule at y 786 that we do not, and
  the live `v:line` is `from -.7pt,-3.5pt to 179.3pt,-3.5pt` — 180 pt wide, in `footer3.xml`.
- `33004` page 1: the reference draws a **468 pt** rule at y 63 that we do not, and the live
  `v:line` is `from 0,15.85pt to 468pt,15.85pt` — 468 pt wide, in `header1.xml`. It is missing
  from **all 47 pages**.

The other entries in each document's difference list are **not** this defect and must not be
counted towards it: `JEMIT_Template`'s table rules sit at y 283 on the reference and y 296 in
ours, which is a pre-existing vertical layout difference moving rules that both sides draw.
Reading the count difference alone — 22 against 21 — would have been right by luck on one
document and wrong on the other.

So the reach is small in documents and **not** small in pages: a header rule absent from every
page of a 47-page document is the kind of furniture a reader notices immediately.

## The coordinate rule, measured — 18 one-attribute probes

The agent had also authored and rendered its probes before it died. Each holds, in the first
body paragraph, a green 1 pt `v:rect` marker at `left:0;top:0` — so the line is read against
*where the anchor's own origin landed* rather than against an assumption — and one red `v:line`
carrying the attribute under test. Every probe but `nosettings` includes a
`word/settings.xml`, because a hand-built DOCX without one takes different OOXML compatibility
defaults and answers a different question. `probes-ref.txt` is the reading, taken from the PDF's
own path operators.

The page is A4 with 1134 twip margins, so the text area starts at **56.7 pt** and the marker
lands there.

| probe | states | 26.2.4.2 draws | what it settles |
| --- | --- | --- | --- |
| `base` | `from="0,0" to="144pt,0"` | (56.70, 56.70) → (200.70, 56.70) | **`from`/`to` are measured from the anchor's origin**, and a bare `0` is nought |
| `lefttop` | + `left:100pt;top:50pt` | *identical to `base`* | **`style:left`/`top` are ignored** when `from`/`to` are present |
| `wh` | + `width:200pt;height:100pt` | *identical to `base`* | **`style` width/height are ignored** too — which is why the seat's "states no width or height" is beside the point |
| `both` | all four | *identical to `base`* | the two above, together |
| `diag` | `to="144pt,72pt"` | → (200.70, 128.70) | both axes |
| `px` | `to="96,48"` | → (128.70, 92.70) | **a bare number is a pixel at 96 dpi**: 96 → 72 pt, 48 → 36 pt |
| `inch` | `from="1in,0.5in" to="3in,0.5in"` | (128.70, 92.70) → (272.70, 92.70) | unit suffixes are honoured |
| `relpage` | + `mso-position-*-relative:page` | (0, 0) → (144, 0) | the origin moves to the **page** corner |
| `relmargin` | + `…-relative:margin` | *identical to `base`* | the margin origin is the text area's here |
| `thirdpara` | two paragraphs before the anchor | marker 85.30, line 84.30 | the origin follows the **anchor paragraph** down the page |
| `nosettings` | no `word/settings.xml` | *identical to `base`* | a negative control: the compatibility defaults do not reach this |
| `strokedf` | + `stroked="f"` | **no ink** | as expected |

**And one asymmetry that no specification suggests.** A line whose **x decreases** is not drawn
at all, while a line whose **y** decreases is:

| `revh` | `from="144pt,0" to="0,0"` | **no ink** |
| `revboth` | `from="144pt,72pt" to="0,0"` | **no ink** |
| `revv` | `from="0,72pt" to="144pt,0"` | (56.70, **128.70**) → (200.70, 56.70) |

So the horizontal direction is load-bearing and the vertical one is not. Anyone implementing
this must reproduce that rather than normalising the two points into a rectangle — normalising
would draw three lines where the reference draws two.

**Inside a `v:group` the numbers are the group's own coordinate space.** The group is 200 × 100 pt
with `coordsize="1000,1000"`:

| `group-full` | `from="0,0" to="1000,1000"` | (56.75, 56.70) → (256.75, 156.70) | 1000 units = the group's full 200 × 100 pt |
| `group-half` | `from="250,250" to="750,250"` | (106.75, 81.70) → (206.75, 81.70) | 250 units = 50 pt across and 25 pt down, exactly the ratio |
| `group-pt` | `from="0,0" to="100pt,50pt"` | (56.75, 56.70) → (76.75, 61.70) | `100pt` is **100 units**, i.e. 20 pt — a bare number is *not* a pixel here |

`group-full` separates the two readings of a bare number: 1000 read as pixels would be 750 pt and
scale to 150 pt drawn, and the reference draws 200. **What `group-pt` cannot separate** is whether
the suffix is parsed to points and the point count used as the unit count, or simply dropped —
both give 100. A probe stating `1in` inside a group would settle it and was not authored.

## What is left

Everything needed to write the fix is above except that one detail. `DocxVmlFrames.Floating`
requires a `style` width and height and must instead take the box from `from`/`to` when they are
present; `Flatten` has the group-space arithmetic already and needs the same treatment.

## The method note

**A census of an element is not a census of what is read, and `mc:AlternateContent` is where the
two part company by a factor of 144.** Round 140 counted `v:line` elements correctly and drew a
reach figure from them; the elements were in the branch a conforming consumer discards. The
discriminator is one column — the element's ancestry — and it costs nothing to record while
walking the tree.

The same trap has a second form worth naming: **counting an element in the raw part rather than
after markup-compatibility resolution**. Any census script that walks `word/document.xml` with a
regular expression, as a reach census naturally does, sees both branches of every
`mc:AlternateContent` in the file.
