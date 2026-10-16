# L3-docx-reader — summary

Thirty-seven documents; **eight root causes**, five with a patch. The reflow/width
differences that most of the case notes also mention are the known advance-width and
table-sizing cascade recorded in `dotnet/CLAUDE.md`; nothing below is that.

## RC1 — A TOC entry takes the `Hyperlink` character style its runs name; LibreOffice drops it
- `Ooxml/DocxLayoutSource.cs` (`RunWalker`), `Ooxml/WordParagraphFormats.cs:ResolveRun`
- Reported on 6 documents (#006 #036 #048 #068 #089 #181); 16 of the 37 are structurally exposed
- `sw/source/writerfilter/dmapper/DomainMapper.cxx:3037-3047` refuses `PROP_CHAR_STYLE_NAME` inside a TOC
- Patch `patches/toc-entry-character-style.diff` · **confidence high**

## RC2 — List counters advance on paragraphs inside `w:vMerge` continuation cells
- `Ooxml/DocxLayoutSource.Tables.cs:Row`, `Ooxml/DocxLayoutSource.Lists.cs:ListFormatting`
- 5 documents (#029 #035 #092 #115 #133); the covered cells are dropped but their counting is not
- `sw/source/core/unocore/unotbl.cxx:978-990` clears counted-in-list for every node in a merged cell
- Patch `patches/covered-cell-list-counter.diff` · **confidence high** (reproduces 1–13 and 3.1–3.5 exactly)

## RC3 — `<w:color w:val="auto"/>` is read as "states nothing" instead of as automatic
- `Ooxml/WordThemeColour.cs:Read` returns null, so the run inherits its style's colour
- 4 documents (#043 #089 #145 #181), 111 runs; each sits in a style that sets a blue
- `DomainMapper.cxx:2676` inserts `COL_AUTO` into `PROP_CHAR_COLOR` like any other value
- Patch `patches/automatic-run-colour.diff` · **confidence high**

## RC4 — `w:ptab` (positional tab) is dropped, so a three-part running head runs together
- `Ooxml/DocxLayoutSource.cs` (`RunWalker`) has no case; `Ooxml/DocxContentReader.cs:527` skips it
- 3 documents carry one (#008 #013 #139); visible on #139 (`Page 5 of 12Version 1Last saved…`)
- `writerfilter/ooxml/model.xml:18204-18208` — `CT_PTab` → `action="tab"`
- Patch `patches/positional-tab.diff` · **confidence high**

## RC5 — A STYLEREF running head is answered from a document-final map
- `Ooxml/DocxLayoutSource.cs:_styleText` / `StyleReferenceText`; headers are read after the body walk
- 1 document (#073): we print "External Interfaces" on every page, the reference varies per page
- Patch `patches/styleref-running-head.diff` declines rather than answering wrongly; the real fix is
  a per-page substitution and is **cross-lane** (`XRay.Office.WordProcessing/Layout/`)
- **confidence high on the diagnosis, medium on the patch being the right trade**

## RC6 — A table border's *style* is not modelled at all
- `Layout/PageTable.cs:49` — `TableBorder(Length Width, Colour Colour)`; `w:val` is only tested for `none`
- 16 of the 37 carry `dotted`/`dashed`/`double`/`outset`/`thinThickSmallGap`; all drawn solid
- **No patch — cross-lane** (`Layout/PageTable.cs` plus the renderer) · **confidence high**

## RC7 — `w:pgBorders` is not read anywhere in `dotnet/`
- 3 documents (#010 green double rule, #043 thin rule, #068); we draw nothing
- Reader belongs in `Ooxml/DocxPageGeometry.cs` + `Model/PageGeometry.cs`; the painter does not
- **No patch — cross-lane** · **confidence high**

## RC8 — Refuted: the EASA footers are *not* a page-numbering offset, and ours is the better output
- #029 and #133 carry **no `w:pgNumType` at all**; their `PAGE`/`NUMPAGES` sit in a `wps` text box
  anchored in a table inside the footer, and the reference prints the producer's **cached** value on
  every page (`Page 3 of 6` on all six pages of #029)
- Writer's draw-layer outliner has no page field — `SwDoc::CalcFieldValueHdl`
  (`sw/source/core/doc/docdraw.cxx:562-621`) handles only date, URL, measure and time
- **No patch. Do not chase these two footers** · **confidence high**
