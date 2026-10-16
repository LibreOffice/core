# odp-master-r70

Scripts kept beside `results.md`. All three record the environment they were run in, in the
first line of whatever they write — which 253 of this repository's 256 stored TSVs do not.

| Script | What it does |
|---|---|
| `sweep.py` | Renders **one half** of a corpus list — `--side ours` or `--side ref` — one output directory per document, and writes the gate's four measured columns per row. It does not compare; `score.py` does that. |
| `score.py` | Applies `batch-check.sh`'s verdict rule of 2026-09-05 to a pair of half-sweeps: page count, then alphanumeric characters within `max(2%, 15)`, then unembedded fonts. |
| `classify.py` | Predicts, from an `.odp` alone, how many alphanumeric characters its master pages contribute that the layout path never drew. Used to group the 182 non-matching rows before any code changed. |

Two traps `sweep.py` carries a comment about, because both cost time in this round:

- A worker directory keyed on a **slot index** is wrong for a thread pool, and one keyed on the
  **document path** is wrong for a corpus that holds spaces: `soffice` truncates
  `-env:UserInstallation=file://…` at the first space, scatters profile directories through the
  sweep root, and the document silently fails to convert. Key it on a hex digest.
- Backgrounding a sweep with `nohup … &` from a tool call does not survive the call. The first
  reference sweep died at 75 of 302 and reported nothing.
