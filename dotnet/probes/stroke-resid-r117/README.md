# Round 117 — `probes/stroke-resid-r117`

Read `results.md`. The files:

| file | what |
|---|---|
| `results.md` | the round, measured separated from inferred |
| `dash-census.py`, `dash-census.tsv` | which corpus spreadsheets *state* a patterned cell border — `xl/styles.xml` for the zip formats, a BIFF `XF` walk for the OLE ones |
| `dash-census-ods.py`, `dash-census-ods.tsv` | the same question through 26.2.4.2's own `--convert-to ods` of the sheets track, one code path for all three containers |
| `chart-census.py`, `chart-census.tsv` | which corpus charts state no width on an axis line or a gridline, per track, with the theme's subtle-line width beside it |
| `make-chart-fixtures.py` | builds the three chart fixtures the tests read |
| `census2.py` | round 113's ink census plus `hair_ours`, `hair_ref` and `ratio_ex_hair` — see C14 |
| `sweep.py` | our half of one track, one directory per document, `SOURCE_DATE_EPOCH` pinned |
| `reach2.py` | base-vs-after confinement: bytes, page counts, alphanumeric characters |
| `like-for-like-base.tsv`, `like-for-like-after.tsv` | the two censuses |
| `moved-*.tsv` | the renderings that changed, per track |
| `words-*.tsv`, `slides-*.tsv` | the same census over the other two tracks |

Reference is **26.2.4.2** (`/opt/libreoffice26.2/program/soffice`); corpus `/home/user/sample-files`;
reference bank `/home/user/gate-r114/ref`; converted ODF corpus `/home/user/corpus-odf`; 2026-09-13.
