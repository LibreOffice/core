# No patches from L1, deliberately

Three candidate changes were considered and each is declined for a stated reason:

1. **The advance-width divergence** (17 documents) — closing it means reproducing FreeType's
   hinted advance at LibreOffice's ppem. Architectural, not a rounding patch; the kerning and
   quantisation-grid hypotheses are already refuted in `dotnet/probes/advance-divergence/`.
2. **`w:w` character scaling** (#106) — the `XRay.Office.Text` half is one field on
   `FormattedRun`, but it does nothing until `WordParagraphFormats.cs` sets it and the
   painter honours it, both outside this lane. Shipped alone it would be a property read by
   nobody. Recorded as a cross-lane dependency in `findings.md` §9.
3. **The `Helvetica`/declared-generic ordering in `SystemFontResolver.cs`** (#101, #031) —
   the rule is calibrated to LibreOffice 26.2.4.2 and this sweep's reference is 24.2.7.2.
   Re-ordering it would match these reference PDFs and regress the binary the tree targets.

See `findings.md` §3, §5 and §9.
