# odp-running-r71

Scripts and data for the round that closed the master's running objects,
`style:shrink-to-fit` and `loext:shadow-blur`. `results.md` is the write-up.

| File | What it is |
|---|---|
| `census.py`, `census.tsv` | First census of a master's running objects. Superseded — it required `presentation:display-*` to state `true` and resolved no declarations, so it missed `introduction_to_bea_tuxedo.odp` entirely. Kept because the difference between it and `census2` is the measurement that found the declaration half of the rule. |
| `census2.py`, `census2.tsv` | The census the round worked from: per document, how many alphanumeric characters a master's running objects contribute, with LibreOffice's own defaults and with the three declaration fields and `text:page-number` resolved. |
| `census3.py` | What a master's `presentation:class` frames actually hold — which field elements the four running kinds carry, and whether every other class is an empty placeholder. |
| `pagediff.py` | Per-page alphanumeric and image-count diff between our rendering and 26.2.4.2's for one document. Asserts each side produced a PDF before comparing. |
| `score-bank.py` | Scores a fresh half-sweep of the original corpus against `/home/user/gate-2f47/rows.tsv`, computing *both* the baseline and the fresh verdict with the 2026-09-05 rule so the two sides are the same instrument. |
| `ours-before.tsv`, `ours-after-{1,2,3}.tsv` | Our half of the `.odp` column at the base commit and after each of the three commits. |
| `parity-*.tsv` | The same four, scored against `probes/odp-master-r70/ref.tsv`. |
| `slides-ours-after.tsv`, `slides-ours-head.tsv` | Our half of the *original* slides track, for the no-movement check. |
| `remainder.tsv` | The 17 rows still failing, with a cause and a note each. |
| `remainder-pagediff.txt` | `pagediff.py` over all 17, which is what the causes were read from. |

The sweep itself is `probes/odp-master-r70/sweep.py` and the scorer `odp-master-r70/score.py`;
neither needed changing, and the reference half was reused rather than re-rendered because
nothing in this round's diff can reach `soffice`.
