# Bundled metric-compatible faces

The four families whose absence silently invalidates every OOXML comparison, shipped with
the library so that a rendering does not depend on what happens to be installed.

`dotnet/CLAUDE.md` records the failure this prevents twice over: a container that lost
`fonts-dejavu-core` moved **53 of 534 page counts and 426 pages** on its own, holding
LibreOffice constant — the same order as a whole reference-version change — and nothing in
the harness declared the font set, so it survived a full pass unnoticed. `fc-match` cannot
warn about it either: it never fails, it always returns *something*.

| family | licence |
|---|---|
| Carlito (metric-compatible with Calibri) | SIL Open Font License 1.1 |
| Caladea (metric-compatible with Cambria) | Apache License 2.0 |
| Liberation Sans / Serif / Mono (Arial, Times New Roman, Courier New) | SIL Open Font License 1.1 |
| DejaVu Sans / Serif / Sans Mono | Bitstream Vera License; DejaVu changes public domain |

All four are redistributable, and the licence of every file was read out of its own `name`
table (id 13) rather than assumed. Two faces that ship beside them in the same LibreOffice
directory are deliberately **not** here: `LiberationSansNarrow`, which is under the older
Liberation Fonts licence rather than the OFL, and `DejaVuMathTeXGyre`, which declares none.

Taken from the LibreOffice 26.2.4.2 Linux x86-64 release, so they are the files that build
of LibreOffice itself reads. That is the point: `SystemFontIndex` prefers these over the
machine's copies, and the machine's copies of the same families are routinely *different
builds* — Caladea-Regular is 58 964 bytes here against Ubuntu 24.04's 81 600, and Carlito
635 996 against 628 032.
