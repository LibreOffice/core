# The row-height divergence on dense grids is the gate binary, not a defect

*Measured 2026-09-04 in the container described at the top of `dotnet/CLAUDE.md`: repository at
`/home/user/libreoffice-core`, corpus at `/home/user/sample-files`. **Two** references, and which
one is which is the whole point: the distro's **24.2.7.2** at `/usr/bin/soffice`, which is what
`batch-check.sh` measures against, and the TDF tarball's **26.2.4.2** at `/opt/libreoffice26.2`,
which is what this tree is calibrated to.*

## Where this started

After the border round, **seven of the sixteen worst documents on the words track by first-page ink
were the same family** — `Printable_Graph_Paper_Template` 080, 081, 084, 085, 088, 089 and 087, at
32 to 51 ink. Read side by side they are structurally identical to the reference: same grid extent,
same heavy axis rules, same footer. What differs is the row *pitch*, by a fraction of a point,
forty-eight rows deep, which walks the whole grid a cell out of phase.

Every one of those rows states `w:trHeight` with no `w:hRule`, over a cell holding one empty
paragraph. So the question is what a `w:trHeight` floor actually costs a row.

## The probe

`pitch.py` writes six one-cell rows all stating the same `w:trHeight w:val="480"`, and sweeps four
things independently: the grid's `w:sz` (0, 4, 8, 24), the `w:hRule` (absent, `atLeast`, `exact`,
`auto`), the cell's top and bottom `w:tcMar` (0, 100 twips) and whether the cell holds text. The
observable is the median gap between the tops of consecutive horizontal rules at 600 dpi, with the
first and last dropped so the table's own outer half-borders cannot reach it.

## The answer, in twips over the 480-twip floor

| `w:hRule` | `w:sz` | `w:tcMar` | 24.2.7.2 | 26.2.4.2 | ours |
|---|---:|---:|---:|---:|---:|
| `atLeast` | 4 | 0 | 480.0 | **489.6** | **489.6** |
| `atLeast` | 4 | 100 | 480.0 | **690.0** | **690.0** |
| `atLeast` | 8 | 0 | 480.0 | **500.4** | **500.4** |
| `atLeast` | 8 | 100 | 489.6 | **699.6** | **699.6** |
| `atLeast` | 24 | 0 | 480.0 | **540.0** | **540.0** |
| `atLeast` | 24 | 100 | 529.2 | **740.4** | **740.4** |
| `exact` | 4 | 100 | 480.0 | **579.6** | **579.6** |
| `exact` | 8 | 100 | 480.0 | **579.6** | **579.6** |
| `exact` | 24 | 100 | 480.0 | **579.6** | **579.6** |
| `auto` | 4 | 0 | 480.0 | 489.6 | **241.2** |
| `auto` | 8 | 100 | 489.6 | 699.6 | **451.2** |
| `auto` | 24 | 100 | 529.2 | 740.4 | **490.8** |

An absent `w:hRule` gives the same figures as `atLeast` in all three columns, and text in the cell
changes nothing anywhere, so both are left out of the table.

**We match 26.2.4.2 to the twip on every `atLeast` and every `exact` case.** The tree is right for
the version it targets, and the 24.2.7.2 column — which is what the gate and every ink figure in
this repository's recent rounds are measured against — is a *superseded binary that does not have
the rule at all*.

That is checkable without rendering anything:

```sh
strings /usr/lib/libreoffice/program/libswlo.so   | grep -c MinRowHeightInclBorder   # 0
strings /opt/libreoffice26.2/program/libswlo.so   | grep -c MinRowHeightInclBorder   # 1
```

`MinRowHeightInclBorder` is set unconditionally by the DOCX filter
(`sw/source/writerfilter/dmapper/DomainMapper.cxx`:156, *"calculate table row height with 'atLeast'
including horizontal border width"*) and read by `lcl_CalcMinRowHeight` and `lcl_GetFixedRowHeight`
(`sw/source/core/layout/tabfrm.cxx`:5058-5100). It does not exist in 24.2.7.2.

## And on the document that motivated the work

`080_Printable_Graph_Paper_Template_Black_Theme`, first page, median pitch between the grid's 51
horizontal rules:

| | rules | median pitch | span |
|---|---:|---:|---:|
| 24.2.7.2, the gate | 51 | 273.6 twips | 713.64 pt |
| 26.2.4.2, the target | 51 | **283.2** | 749.16 pt |
| ours | 51 | **283.2** | 747.24 pt |

So **there is no row-height defect to fix on this family.** Its 32-to-51 ink is the version gap, and
so is the constant offset left on `PAT-047` after the border round. A round that "fixed" it would be
retuning the tree to a binary the project does not target — which is the standing trap
`dotnet/CLAUDE.md` describes, arriving from the other direction than usual: not a stored figure gone
stale, but a *live* measurement made against the wrong binary.

**The existing figures were right.** `TablePaginationRulesTests` and
`probes/words-pagination-01/row-min-height-border.py` read 24.00 / 24.50 / 25.00 / 26.00 / 27.00 pt
for `w:sz` 0 / 4 / 8 / 16 / 24, and say in their own prose that they were measured against 26.2.4.2.
This probe reproduces them there and contradicts them at 24.2.7.2, which is agreement, not conflict.

## The one real defect, and it has no reach

`w:hRule="auto"`. Both reference versions treat it as the floor their own `atLeast` gives; we alone
dropped the height entirely and let the empty paragraph decide, which is 241.2 twips against 480.
Writer never looks at the word: `MeasureHandler` opens at `SizeType::MIN` and its
`LN_CT_Height_hRule` case tests only for `exact`
(`sw/source/writerfilter/dmapper/MeasureHandler.cxx`:35, 70-76).

Fixed, with a test that asserts `auto` and `atLeast` are the same rather than pinning a figure, so
the two cannot drift apart. **No corpus document does it**: 11 230 `w:trHeight` elements across every
DOCX in `sample-files`, of which 10 825 state no rule at all, 380 `exact`, 25 `atLeast` and **none**
`auto`. The words sweep confirms it — not one row and not one hundredth of a point moved.
