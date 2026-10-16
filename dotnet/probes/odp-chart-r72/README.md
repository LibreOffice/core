# odp-chart-r72

Scripts and data for the round that closed the ODF chart reader's axis resolution,
`draw:text-rotate-angle` and `text:line-break`. `results.md` is the write-up.

| File | What it is |
|---|---|
| `variants.py` | Builds one-attribute variants of a corpus `.odp`'s embedded chart and renders each through 26.2.4.2. Its docstring carries the thirteen-row table that establishes **why** 26.2.4.2 draws some rotated axis labels with no text layer at all — and it is not the angle. |
| `classify.py` | Per-page classifier for a rendered pair: alphanumeric counts, page images, glyph-sized filled paths on each side, and how many of our text lines are turned. The three columns tell the raster ceiling, the outlining ceiling and a defect of ours apart. |
| `ours-base.tsv` | Our half of the `.odp` column at the round's base commit `97628b7fe`. |
| `ours-charts-and-rotate.tsv` | The same after the first two commits — the axis resolution and `draw:text-rotate-angle`. |
| `ours-after.tsv` | The same after all three commits. |
| `parity-base.tsv`, `parity-after.tsv` | Those two scored against `probes/odp-master-r70/ref.tsv` with `odp-master-r70/score.py`. |
| `slides-ours-base.tsv`, `slides-ours-after.tsv` | Our half of the **original** slides track, at the base commit and after, for the no-movement check. Rendered from `/home/user/sample-files` against `/home/user/gate-2f47`'s row list. |
| `remainder.tsv` | The 13 rows still failing, with a cause and a note each. Three are reclassified from `odp-running-r71/remainder.tsv`. |
| `remainder-classify.txt` | `classify.py` over all 13, which is what the causes were read from. |

The sweep itself is `probes/odp-master-r70/sweep.py` and the scorer `odp-master-r70/score.py`;
neither needed changing, and the reference half was reused rather than re-rendered because
nothing in this round's diff can reach `soffice`. The original slides track was rendered at the
base commit as well as at the head, rather than scored against the bank, because the bank's
commit predates two chart rounds that move one of its rows.
