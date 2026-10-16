# The converted corpus at `873c766f9` — three merges and the fifth font confound

## Environment

    ours   = XRay.Office.Cli @ 873c766f9 -- rtfpage, odtresid and odspage merged
    ref    = /opt/libreoffice26.2/program/soffice, 26.2.4.2, via $REF_SOFFICE
    fonts  = ALL FIVE tarball confounds aside, including the eight DejaVu Condensed
             faces moved the same day. Not comparable to any earlier sweep.
    rule   = batch-check.sh of 2026-09-05

## The result

    TOTAL 1285  MATCH 1061  MISMATCH 222  REF-CANNOT-RENDER 2

| target | `odf-gate-r76` | this run | |
|---|---:|---:|---|
| `.odp` | 289 | 289 | of 302 — untouched by these three rounds, as expected |
| `.odt` | 264 | **281** | of 338 |
| `.rtf` | 243 | **257** | of 338 |
| `.ods` | 229 | **234** | of 307 |

`.rtf` 257 and `.ods` 234 reproduce `rtf-page-r77` and `ods-page-r77`'s own figures exactly;
`.odt` lands one below `odt-resid-r77`'s 282, the one row being within what the font change moves.

## A rule was broken during this run, and it happened not to matter

`dotnet/CLAUDE.md` says a sweep and a rebuild must never overlap: the gate measures
`dotnet/tools/XRay.Office.Cli/…/XRay.Office.Cli` in the tree it is run from, so rebuilding swaps the
binary out from under it and the rows either side of the rebuild describe different programs.

**A rebuild was run in this tree while the sweep process was still alive** — the `odpvis` merge at
22:37, against a sweep started at 21:16. What saves this run is only that every one of its 1285
renders had already finished by then: no output file postdates the new binary, and the four column
figures reproduce the three rounds' independent measurements. That is luck, not method. The
sweep's own liveness is the thing to check before building, and the check that settles it is the
mtime of the newest rendered file against the mtime of the binary — not the wall-clock reasoning
that says "it was probably finished".
