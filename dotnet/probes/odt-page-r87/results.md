# odt-page-r87 — an ODF tab stop is measured from the text edge, not from the indent

Measured 2026-09-08 in `/home/user/wt-odtpage` (branch `agent/odtpage`, base `3466245b1`).
Reference **`/opt/libreoffice26.2/program/soffice` — LibreOffice 26.2.4.2 0229ac93**, taken from
the banked reference half of `/home/user/gate-odf-r80` (rendered 2026-09-08, after the five font
confounds were moved aside). Our half was re-rendered at HEAD by
`sweep-odt-ours.sh`, which is `probes/overflow-r69/sweep-ours.sh` with `find` in place of
`git ls-files`, because `/home/user/corpus-odf` is not a git checkout.

## 0. The banked table in the brief was stale in both directions

`probes/odf-gate-r80/rows.tsv` records the `.odt` track at **281 of 338**. At HEAD, before any
change of this round, it is **290 of 338** (`rows-before.tsv`). Three of the four rows the brief
named as the largest pagination gaps had already moved:

| brief's row | r80 | at HEAD |
|---|---|---|
| `ESPN-R - MCF - RA - Ed1.odt` | 41/58 | **match** |
| `A_320.odt` | 134/118 | 134/118 |
| `150_5300_13_chg10.odt` | 85/76 | 85/76 |
| `WordArt_Shapes_Arrows_Catalog1.odt` | 45/52 | 45/52 |

`Case-Study-Heathrow-Airport.odt` (1/3) and `33004.odt` (48/47) had also closed, and one row not in
the r80 list had opened (`slcc-architecture-uu-architecture.odt`, 3/4).

**And the brief's `A_320` framing was wrong, as the coordinator corrected mid-round.** There is no
`.doc` defect and no "opposite directions" pair: against 26.2.4.2 the `.doc` is **118/118 with
88958 glyphs on both sides**, and only the `.odt` diverges. That makes it the strongest shape a row
in this track can have — one document, two readers, one of which already passes — and it is the
control every number below is checked against.

## 1. The cause: `TabsRelativeToIndent`

`A_320.odt` is 117 tables and 1561 rows of an aircraft MMEL. Aligning our pages against the
reference's (`anchor.py`) shows no structural insertion — the drift accrues at about one page in
seven from page 17 onward, which is a capacity difference. The line pitch is identical (15.65 pt on
both) and the text band starts at the same y, so it is not a body-height or line-height question.

Reading one page side by side (`rows.py`, then the spans) locates it exactly. On page 16, in the
same table cell:

```
ours   52 at x = 75.00,  Air Cooling System at 96.60, "(cont'd)" wrapped to a second line
26.2   52 at x = 53.50,  Air Cooling System (cont'd) at 75.10, one line
```

21.6 pt — 0.3 inch — on both spans. The paragraph is

```xml
<text:p text:style-name="P64"><text:tab/>52<text:tab/>Air Cooling System</text:p>
```

and `P64` states `fo:margin-left="0.6in" fo:text-indent="-0.6in"` with stops at 0.3, 0.6 and 0.9
inch. So the two renderings disagree about **what the stop positions are measured from**, and the
document says which:

```xml
<config:config-item config:name="TabsRelativeToIndent" config:type="boolean">false</config:config-item>
```

### The seat in LibreOffice

`SwTextFormatter::NewTabPortion` (`sw/source/core/text/txttab.cxx`:94-98), under the comment
*"#i24363# tab stops relative to indent"*:

```cpp
const SwTwips nTabLeft = bRTL
    ? m_pFrame->getFrameArea().Right() - ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
    : m_pFrame->getFrameArea().Left()  + ( bTabsRelativeToIndent ? GetTabLeft() : 0 );
```

`GetTabLeft()` is `mnTabLeft` (`itrcrsr.cxx`:387), which is
`SwTextNode::GetLeftMarginForTabCalculation` — the paragraph's own text-left margin, or its list
level's `GetIndentAt()` (`ndtxt.cxx`:3573-3593). The same flag also moves `nLeftMarginTabPos`, the
position the hanging-indent overrule falls back to: it is `Left() - frameArea.Left()` when the flag
is off and **zero** when it is on (`txttab.cxx`:222-285, `#i115705#`).

That second half is what makes the observed numbers exact, and it is why the defect is worth 21.6 pt
rather than the 43.2 a naive reading of the first half predicts:

* **flag off (Word).** The origin is the cell's text edge, the stops are at 21.6 / 43.2 / 64.8, the
  first line begins at the origin, and the overrule does not fire because the nearest stop (21.6) is
  not beyond the 43.2 pt left margin. The tab reaches **21.6**.
* **flag on (Writer, what we did).** The origin moves to the indent, the first line begins 43.2 pt
  *behind* it, every stop is beyond the left margin (now zero), the overrule fires, and the tab is
  sent to the indent — **43.2**.

`53.50 − 31.90 = 21.6` in the reference and `75.00 − 31.80 = 43.2` in ours. Both arms reproduce with
no free parameter; `OdtTabSettingsTests` pins them.

### Absent means *true*, and only the ODF reader had to read it

`mbTabRelativeToIndent(true)` (`DocumentSettingManager.cxx`:80) is Writer's own answer for a document
it created. Every Word-family importer turns it off — `ww8par.cxx`:1951 for a `.doc`,
`DomainMapper`'s constructor for DOCX *and* RTF (`sw/source/writerfilter/dmapper/DomainMapper.cxx`
:128-132) — and LibreOffice's ODF export writes the resulting flag into `settings.xml`.

This tree's `DocxReader`, `RtfDocumentReader` and `Ww8DocumentReader` each set
`ParagraphFormat.TabsRelativeToIndent = false` by hand, which is why the `.doc` control passes.
**The ODF reader never set it at all** and took the `true` default, so every `.odt` converted from a
Word file was laid out by the rule its own settings deny.

### Reach

**All 338 converted `.odt` state the flag false** — which is what their provenance predicts and is
not itself a reach figure. The two rules differ only for a paragraph that holds both a tab and a
non-zero left indent, and that is **38 of the 338 documents, 3728 paragraphs**
(`census-tabs.py`, which resolves `style:parent-style-name` chains rather than reading the
paragraph's own style alone). The census is a lower bound: it does not model list-level indents or
the default tab interval, and 69 renderings actually move. `.ods` and `.odp` state the item nowhere
and could not reach this code in any case.

## 2. What moved

`OdtLayoutSource.TabsRelativeToIndent(settings)` reads the item, the value is resolved once per
document beside `_shrinksJustifiedBlanks` and `_breaksWrappedTables`, and
`OdfParagraphFormats.Resolve` puts it on the `ParagraphFormat`.

| | `.odt` track |
|---|---|
| before | **290** of 338 match |
| after | **291** of 338 match |
| gained | `A_320.odt` — 134/118 → **118/118**, glyphs 89754 → **88938** against 88958 |
| lost | **none** |

`rows-before.tsv` and `rows-after.tsv`; both halves scored against the same banked reference bytes,
so only our side moved and no row failed on either side in either run.

**One verdict is not the size of the change, and this is the `w:pgBorders` shape again**: a tab
position adds no glyphs and no pages, so no gate column can see it. **69 of the 338 renderings
change** with the conversion date masked out (`movers.py`, `movers.txt`), and the quantity the defect
actually moves is where a line starts:

| matched lines | mean \|Δx\| from 26.2.4.2 | lines within 0.1 pt |
|---:|---:|---:|
| 194 483 | **2.061 pt → 1.952 pt** | **79 600 → 81 145** |

Per document, of the 69 movers: **42 better, 25 level, 2 worse** (`startx.txt`). `A_320` goes 2.549
→ 0.177 pt mean and 1207 → 2130 lines exact. The two that worsen are `150_5300_13_chg10`, whose
mean is 38.7 pt either way and whose matching is unreliable at 85 pages against 76, and
`hdss-bulletin-issue-285`, whose defect is the `continuous`-section header height already recorded
in `dotnet/CLAUDE.md`.

### Confinement, measured rather than argued

The 69 movers' **`.docx`/`.doc` originals** from `/home/user/sample-files`, their **`.rtf` twins**
from the converted corpus, and a sample of **10 `.ods` and 10 `.odp`** — 158 documents — were
rendered by a binary built with the patch reverted and again with it restored, in the same tree,
with `obj`/`bin` cleared on both legs:

```
158 renderings compared with the date masked, 0 differ
```

`confinement.txt`. So the same content through the WW8, DOCX and RTF readers does not move by a
byte, and neither do the two other ODF columns.

### Tests

`XRay.Office.WordProcessing.Tests` **1827 → 1834 passed, 0 failed** (seven new in
`OdtTabSettingsTests`). The other nine non-fidelity projects run individually: 519, 109, 259, 143,
1044, 164, 1226, 728, 302 passed, 0 failed and 0 skipped throughout.
`XRay.Office.Fidelity.Tests` is **542 passed / 10 failed**, and the ten are exactly the expected
names — `PageDrawingComparisonTests.EveryLineIsDrawn` ×4,
`TabStopComparisonTests.AListLabelsTabAdvance` ×4, `SheetDrawingComparisonTests.APictureIsDrawn`,
`JustificationShrinkComparisonTests`. Two of the four `AListLabelsTabAdvance` failures are the
`.fodt` and `.odt` spellings of `list-label-overrun`, which go through the changed path and neither
gained nor lost.

## 3. The residual, with its seat where one is established

47 rows still fail. Ranked by how far our line starts sit from the reference's over matched lines
(`residual-startx.txt`), they split at about 5 pt into two groups that want different work:

**Nine rows are a horizontal-placement defect and their pagination follows from it.** Ranked by mean
\|Δx\| over matched lines, counting only documents where enough lines match for the mean to mean
anything — under about 150 the figure is noise, which is why `644730BRI0mna000BOX361539B00public0`
(19 matched lines at 90 pt) and the two Venn templates (4 each at 109) are not in this table:

| mean \|Δx\| | matched | pages ours/ref | document |
|---:|---:|---|---|
| 89.29 | 220 | 6/4 | `JEMIT_Template` |
| 39.97 | 3040 | 85/76 | `150_5300_13_chg10` |
| 30.69 | 602 | 12/12 | `ABCD-WB-08-00 Weight and Balance Report` |
| 27.89 | 1343 | 35/32 | `150_5300_13_chg12` |
| 19.18 | 2594 | 63/64 | `150_5335_5a` |
| 17.01 | 878 | 21/18 | `150_5300_13_chg8` |
| 14.91 | 796 | 15/16 | `ABCD-FE-01-00 Flight Envelope` |
| 8.73 | 1601 | 46/44 | `docs-quality-MA.IMS.00001` |
| 6.57 | 1587 | 30/29 | `ABCD-SDE-23-00 Avionic System Description` |

The three `150_5300_13` documents are one family and the largest coherent group left; the three
`ABCD-*` are a second, and `dotnet/CLAUDE.md` already records three `ABCD-*` documents sharing one
bug on the original track. Note that two of these nine are **page-exact** — `ABCD-WB-08-00` at 12/12
and `150_5335_5a` one page short — so the group is defined by where the text sits rather than by the
pagination, which is the point of ranking this way.

**Of the other 38, four have a large mean over too few matched lines to trust** —
`011_Project_Timeline_Template_Beautiful_Theme` (341.96 pt over 100 lines), the two Venn templates
(108.7 over 4) and `644730BRI0mna000BOX361539B00public0` (90.42 over 19) — and the remaining **34
are within 4.5 pt of the reference on every line they draw**, so whatever ends their pages is not
where the text starts. Eight of the 38 are the `chartset` templates whose "excess" is 26.2.4.2
outlining its own glyphs, measured in `probes/odt-split-r82/overdraw.py`, and three of the four
untrustworthy means are among them: outlined text is absent from the reference's text layer
altogether, so only a handful of lines match and the mean is noise. **Read a \|Δx\| ranking with the
matched-line count beside it** — that is the instrument note this table exists to carry.

### One reading refuted on the way

The page-alignment output for `150_5300_13_chg10` shows several of our pages holding 100-600
characters where the reference's neighbours hold thousands, which reads as *we insert nearly-empty
pages*. Counting them says otherwise: **ours holds 27 pages under 400 characters and the reference
29** on that document, 2 against 3 on `chg8`, 11 against 9 on `chg12`. The thin pages are the
document's own, on both sides. What that family really shows is the 17-40 pt line-start divergence
in the table above.

## Files

| file | what it is |
|---|---|
| `sweep-odt-ours.sh` | re-renders our half of the `.odt` track against a banked reference |
| `render-list.sh` | renders an arbitrary list of documents, keyed by stem and extension |
| `census-tabs.py` | documents holding a tabbed paragraph with a non-zero left indent |
| `anchor.py` | places each page of one PDF inside another by a slice of its own text |
| `pagediff.py` | the first cut of that, kept because it is the instrument that failed (see below) |
| `rows.py` | a page's baseline band and pitch |
| `movers.py` | which renderings differ between two sweeps, with the conversion date masked |
| `startx.py` | mean start-x divergence of matched lines against a reference |
| `agree.py` | exact (page, x, y, text) line agreement — too brittle to use, kept as the record |
| `rows-before.tsv`, `rows-after.tsv` | the two sweeps |
| `movers.txt`, `movers.list` | the 69 renderings that changed |
| `startx.txt`, `residual-startx.txt`, `residual.list` | the before/after and residual rankings |
| `confinement.txt` | the 158-document no-reach check |

**Two instrument notes.** `pagediff.py` matches pages on their first three lines, and every page of
`A_320` carries the same three-line running head, so it reported an offset that stepped by one at
every page and meant nothing. `agree.py` counts lines agreeing to a tenth of a point in *both* axes,
and a global baseline difference of a fraction of a point makes almost every line disagree — it
scored 2517 agreeing lines out of 193 590 before the change, which is a number with no information
in it. `startx.py` measures one axis on lines matched by their text and is the one to reuse.
