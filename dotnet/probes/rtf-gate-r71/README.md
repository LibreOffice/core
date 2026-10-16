# `rtf-gate-r71` — the `.rtf` column of the ODF gate

The sample corpus holds no RTF at all, so all 947 words-track documents were converted
through 26.2 and gated as a column of their own. **Every file in it is LibreOffice's own
export, so both renderers read identical bytes and every divergence is ours.**

| script | what it does |
|---|---|
| `sweep-ours.sh` | Renders our half of a corpus's `.rtf` and measures the gate columns into `ours.tsv`. |
| `score.py` | Joins `ours.tsv` onto the reference columns banked in `gate-odf-rows.tsv` and applies the gate rule. Does not re-render the reference — sound because a diff confined to `dotnet/src` cannot change what `soffice` made. |
| `ref-ten.sh` | Renders the reference for the ten documents the `odf-gate-01` sweep died before reaching. |
| `measure-ref.sh` | Measures a directory of reference PDFs into bank-shaped rows. |
| `census.py` | Counts the positioned-table and cell-text-flow control words, and separates the ones LibreOffice's tokeniser *recognises* from the ones its dispatchers actually *handle* — which is not the same set, and the difference is load-bearing. |

`results.md` records what the round established.

**The reference is `/opt/libreoffice26.2/program/soffice` (26.2.4.2), not `/usr/bin/soffice`
(24.2.7.2).** `batch-check.sh` takes `soffice` from `PATH` and cannot reproduce the banked ODF
rows; put `/opt/libreoffice26.2/program` first, or re-render our half only and reuse the
bank's reference columns, which is what `score.py` does.
