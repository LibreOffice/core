# Regression corpus

One file per fixed bug, named for what it pins. Same rules as the rest of the corpus: small,
freely licensed, and reduced to the minimum that still shows the behaviour — see
`../README.md` and the `office-corpus` skill.

Every file here is **authored by a script in `dotnet/probes/`**, not collected, so its licence
is this repository's and every byte in it can be traced to one line of the generator.

| File | Written by | Pins |
|---|---|---|
| `pivot-default-style.xlsx` | `probes/pivot-fmt-r110/make-default-fixture.py` | What an emptied pivot cell falls back to. States three different fonts — Liberation Sans 11 black in the `Normal` `cellStyleXf`, Liberation Serif 18 red in `cellXfs[0]`, Liberation Mono 8 bold green on yellow in the cell format its pivot's own cells carry — because every workbook in the sample corpus gives the first two the same content and so cannot separate them. 26.2.4.2 resolves all twelve cells of the cleared rectangle to the `Normal` style's font, and the control sheet's `s="0"` block to `cellXfs[0]`'s. Its `Plain` sheet's third block — the same cells with **no `s` at all** — is the *sheet-wide* half of the same question, re-measured in r111 and answered the same way. `SheetPivotDefaultStyleTests`, and `probes/sheet-default-r111`. |
| `pivot-format-records.xlsx` | `probes/pivot-fmt-r110/make-format-fixture.py` | What a pivot's own `<format>` records put back over the cleared range. The file above with a `<formats>` element added and nothing else changed, so the two are a single-variable experiment: five records, one per arm of `FormatOutput`, including the three where the C++ tree read in this checkout predicts something 26.2.4.2 does not do. Its `dxf` fill is green where the cells' own hard fill is yellow, so the two candidate sources are told apart rather than agreeing by accident. `SheetPivotFormatRecordsTests`. |
