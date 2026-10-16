# The converted corpus at `995e83aa3` — every round of this session merged

    ours = XRay.Office.Cli @ 995e83aa3      ref = 26.2.4.2 via $REF_SOFFICE
    fonts = all five tarball confounds aside          rule = batch-check.sh of 2026-09-05

    TOTAL 1285  MATCH 1058  MISMATCH 214  REF-CANNOT-RENDER 13

## Read this table like-for-like, not on the totals

This run scored `.ods` **225** where `odf-gate-r78` scored 234, and nothing regressed. Eight of the
nine lost rows are **`ref-failed`** — the *reference* timed out — and the ninth is `ours-failed` on
`Global_Market_Forecast…ods`, which `ods-page-r77` already measured at 210 s alone against the
240 s bound. Three rounds were building and sweeping in their own worktrees throughout. The tell is
`REF-CANNOT-RENDER`, which went **2 → 13**.

Excluding the 15 rows that failed on either side in either run:

| column | r78 `873c766f9` | r80 `995e83aa3` | of |
|---|---:|---:|---:|
| `.odp` | 289 | **295** | 302 |
| `.odt` | 281 | 281 | 338 |
| `.ods` | 225 | 225 | 294 |
| `.rtf` | 257 | 257 | 336 |

**No row that matched at r78 is a genuine mismatch at r80.** The `.odp` +6 is `odp-embed-r79`'s,
reproduced independently here.

## The session's arc on this corpus

| column | first measurement (`odf-gate-01`) | now |
|---|---|---|
| `.odp` | 120 of 302, 39.7% | **295 of 302, 97.7%** |
| `.odt` | 146 of 329, 44.4% | **281 of 338, 83.1%** |
| `.ods` | 179 of 307, 58.3% | **225 of 294 scoreable, 76.5%** |
| `.rtf` | 198 of 328, 60.4% | **257 of 336 scoreable, 76.5%** |
