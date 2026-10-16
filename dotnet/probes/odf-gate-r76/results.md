# The converted ODF and RTF corpus at `c6e730d90`, against the calibration target

## Environment

    ours   = XRay.Office.Cli @ c6e730d90 -- five rounds merged: rtfshape, odpchart, odtframe,
             chartresid, odshdr
    ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2, passed through
             batch-check.sh's new $REF_SOFFICE rather than PATH
    fonts  = system fontconfig; all four tarball confounds moved aside
    corpus = /home/user/corpus-odf, 1285 files -- 26.2.4.2's own export of all 947 corpus
             documents into the ODF spelling of their family, and the words track additionally
             into RTF. Both renderers read identical bytes, so every divergence is ours.
    rule   = batch-check.sh of 2026-09-05: page count, then alphanumeric characters within
             max(2%, 15), then unembedded fonts

## The result

    TOTAL 1285  MATCH 1025  MISMATCH 258  REF-CANNOT-RENDER 2

| target | match | of | | at `odf-gate-01`, the first measurement |
|---|---:|---:|---|---|
| `.odp` | 289 | 302 | **95.7%** | 120 of 302, 39.7% |
| `.odt` | 264 | 338 | **78.1%** | 146 of 329, 44.4% |
| `.ods` | 229 | 307 | **74.6%** | 179 of 307, 58.3% |
| `.rtf` | 243 | 338 | **71.9%** | 198 of 328, 60.4% |

## Why this run exists: the one before it was scored against the wrong binary

The immediately preceding sweep, `gate-odf-r74`, was launched without `PATH` pointed at the
tarball, so `batch-check.sh` took `soffice` from `PATH` and measured against **24.2.7.2**. It
produced a table that looked entirely ordinary -- `.odp` 285, `.ods` 188, `.odt` 260, `.rtf` 238 --
and was discarded. `dotnet/CLAUDE.md` already warned that the two binaries are both present and
that the tree is calibrated to the tarball; reading that warning was not enough. The script now
resolves `$REF_SOFFICE` and **prints the reference's path and version** beside the CLI it measures,
so a run says which of the two it used. The header of this one:

    measuring .../XRay.Office.Cli
    reference /opt/libreoffice26.2/program/soffice -- LibreOffice 26.2.4.2 0229ac93...

## What this confirms

Each of the five rounds measured its own column in its own worktree. This sweep is the
independent check, and it reproduces four of them and betters the fifth:

| round | claimed | this sweep |
|---|---|---|
| `odpchart` | `.odp` 289 of 302 | **289** |
| `odtframe` | `.odt` 264 of 338 | **264** |
| `odshdr` | `.ods` 228 of 307 | **229** |
| `rtfshape` | `.rtf` 239 of 328 | **243** of 338 |

`.ods` is one above its round's figure and `.rtf` above its round's on a larger denominator,
because each round measured its own branch and this measures all five merged together.

## The two `REF-CANNOT-RENDER` rows

Not ours. Both are reference-side failures under the same 240 s bound our own renders take.
