# A table that does not fit is moved whole, not split — and its caption goes with it

Measured 2026-08-15 against LibreOffice 26.2.4.2 with the full font set, on
`words/pagination-002/docx/AC-150-5370-10G-updated-201604.docx` (693 pages
against 696).

## Where the first lost page actually is

**Not page 195.** An earlier localisation put it there and was wrong twice over,
so both corrections are recorded rather than quietly dropped:

- The "**118.3 pt gap** on reference page 195" was measured from the last body
  line down to the **footer** at y=738.9, not to the bottom of the text area.
  Reference page 194 reaches y=639.8 and page 187 reaches y=703.1, so the text
  area runs to about 714 and the real gap there is ~19 pt — an ordinary "the next
  line did not fit", not an anomaly.
- Aligning by the printed folio proves nothing: every page prints its own number
  and both renderings put folio *n* on PDF page *n+6*, by construction. The
  instrument that works is content alignment with the running header and footer
  stripped, requiring an offset to hold for eight consecutive pages before it
  counts as a step.

Done that way the three losses are sharp:

| | offset 0 → 1 | 1 → 2 | 2 → 3 |
|---|---|---|---|
| reference page | **187** | 289 | 372 |

At the first one, both renderings agree **line for line down to y=583.0** on page
186. Then the reference stops, leaving about 120 pt, and opens page 187 with the
caption `Requirements for Gradation of Mixture` and the whole ten-row table under
it. We put the caption at y=601.8 and split the table after two data rows.

## The minimal reproducer

`reproduce.py` slices 13 top-level body blocks (`kids[2143:2155]` plus the
`sectPr`) out of the real document, keeping every other part of the package
byte-for-byte, and reproduces the divergence exactly:

```
        ref pages 2, p1 last body y = 583.5, no table row on p1
        our pages 2, p1 last body y = 693.7, two table rows on p1
```

Two traps cost a run each and are worth knowing. `ElementTree` reserialisation
invents `ns2:` prefixes unless **every** prefix from the original `<w:document>`
tag is passed to `register_namespace` first — and pasting the original root tag
back over the output instead produces a file LibreOffice rejects with only
"source file could not be loaded". And reading entries from a `ZipFile` while
writing derived copies in a loop raises `BadZipFile: Bad CRC-32`; read the whole
package into a dict up front.

## What it is

Sweeping how much room the table gets (`reproduce.py --sweep`, dropping leading
blocks so the table rises up the page) separates the two behaviours completely:

| leading blocks | ref pages | ref rows on p1 | our pages | our rows on p1 |
|---:|---:|---:|---:|---:|
| 12 | 2 | **0** | 2 | 2 |
| 11 | 2 | **0** | 2 | 6 |
| 10 and fewer | 1 | 8 (all) | 1 | 8 (all) |

**When the table fits we agree to 0.1 pt on every one of the nine cases. When it
does not fit, the reference never places a single row — it moves the table
whole — and we split it.** That is the defect, and it is not about the caption.

Writer's own gate is `sw/source/core/layout/tabfrm.cxx`:3061-3092:

```cpp
sal_uInt16 nMinNumOfLines = nRepeat;            // = GetRowsToRepeat(), the w:tblHeader count
if ( bTableRowKeep ) { ... while ( pTmpRow && pTmpRow->ShouldRowKeepWithNext() ) ++nMinNumOfLines; }
if ( !bTryToSplit ) ++nMinNumOfLines;
const SwTwips nBreakLine = ... + lcl_GetHeightOfRows( GetLower(), nMinNumOfLines );
if( aRectFnSet.YDiff(nDeadLine, nBreakLine) >= 0 || !pIndPrev || bEmulateTableKeepSplitAllowed )
    bSplit = true;
```

So a table is split **only if the first `nMinNumOfLines` rows fit**; otherwise it
is moved whole. We have no equivalent minimum at all — `TableLayouter.SliceRow`
takes any cut that fits. This table's first two rows are `<w:tblHeader/>`, so
`nRepeat` is 2; the document holds 176 `tblHeader` and 110 `cantSplit`.

## And a second, separate defect: keepNext onto a table

The caption is the built-in `Caption` style, whose `pPr` is
`<w:keepNext/><w:jc w:val="center"/>`, and its next sibling is `<w:tbl>`.
Ablating that one flag out of `styles.xml` moves the reference's last body line
from **583.5 to 602.2** — 602.2 being exactly where we put the caption. So the
reference *does* honour keep-with-next across a paragraph/table boundary, and we
do not: `Paginator.cs`:1357 requires `Laid(paragraphIndex).Paragraph is { } next`,
which is null for a table block, and `MoveTrailingGroupToNextPage` stops its
backward walk at a table on purpose.

Both must land together to match this page: the table has to move whole *and* the
caption has to follow it.

## What it is not — three refutations

- **Repeated headings are not the trigger.** Stripping `<w:tblHeader/>` from both
  header rows changes nothing: ref last y stays 583.5 and still no row on page 1.
  (`nRepeat` then being 0 does not make the reference split here, so the
  `nMinNumOfLines` arithmetic above is necessary but not sufficient — the
  keep-chain term is doing work this ablation did not isolate.)
- **Not the borders and not the width.** `no-borders` and `width-auto` both leave
  the reference at 583.5 with no row on page 1.
- **Not a floating table.** The `tblPr` holds only `tblW`, `jc`, `tblBorders`,
  `tblLook` and `tblDescription` — no `tblpPr`, so this is not task #65.

## An unresolved contradiction, deliberately left standing

`caption-table-keep.py` builds the same shape *synthetically* — filler, a caption
carrying keepNext, a ten-row table — and sweeps the room from 200 pt down to
34 pt. Across 36 cases the reference leaves the caption behind **every time**,
whether keepNext comes from a style named `caption`, from the paragraph's own
`pPr`, or not at all; and we agree with it in all 36.

That is the opposite of what the ablation on the real document shows. The
synthetic probe is not obviously broken — its paragraph-to-**paragraph** control
(`keepctl`, sweeping filler lines 44..59) finds the boundary cleanly at n=49,
where no-keep gives `(keeper 0, follower 1)` and keep gives `(1, 1)`, and **we
match the reference on all 32 of those**. So paragraph→paragraph keep is right in
both engines and the instrument works.

Something about the real document's shape enables the paragraph→table keep that
the synthetic one does not reproduce. **Do not treat "LibreOffice ignores
keepNext before a table" as established** — the real-document ablation is the
stronger evidence, and finding what the synthetic case is missing is the next
step.
