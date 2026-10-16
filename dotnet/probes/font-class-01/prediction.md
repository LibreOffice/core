# font-class-01 — prediction, written before any effect was measured

Committed before the change was written and before any corpus sweep was run. What is *already*
measured at this point is the defect, not its fix: the 296-family gold table re-taken on the
installed 26.2.4.2, `fc-match` over the same 296, and our own resolver's answers over the same 296.

## What I intend to change

1. `FontconfigPreferences` learns fontconfig's own **classification** of a family — the
   `<alias><family>X</family><default><family>serif|sans-serif|monospace|…</family></default></alias>`
   rules in the machine's configuration, resolved transitively (30-metric-aliases.conf's
   `<default>` names a *concrete* family, so `Century Schoolbook → New Century Schoolbook → ∅`),
   defaulting to **sans-serif** for a family it names no rule for, which is `49-sansserif.conf`.
2. `SystemFontResolver` takes its shape from that instead of from `VCL.xcu`'s `FontType`
   (`FontSubstitutions.ClassOf`). The table's classification survives only for the `Symbol` test —
   fontconfig has no symbol generic — and as the whole answer on a machine with no fontconfig.
3. `FontSubstitutions.FontconfigOverridesTheChain`, today a hardcoded `{helv, sansserif}`, becomes
   derived: **the `VCL.xcu` chain is consulted only when fontconfig names the family at all**, or
   when the family is symbol-encoded, or when the machine has no fontconfig. That is the pre-match
   ordering `words-pages-01` established, applied to the name rather than to a declared class.

## Predictions

| # | claim | conf |
|---|---|---:|
| P1 | The 296-family dump after the change agrees with the 26.2.4.2 gold table on **≥ 288** of 296, against 274 now. | 0.70 |
| P2 | `Wingdings`, `Wingdings 2`, `Wingdings 3` and `Webdings` still answer OpenSymbol — the pi-face exemption holds and the gold table stays the wrong instrument for them. | 0.90 |
| P3 | `MS Gothic`/`MS PGothic` move IPAGothic → DejaVu Sans and no corpus verdict is lost by it. Latin *and* Japanese were measured on 26.2.4.2 and neither answers IPAGothic. | 0.60 |
| P4 | Renderings changed: **words 20–50, slides 15–45, sheets 8–25**. ~70 corpus documents name an affected family. | 0.50 |
| P5 | Face-set distance to the reference: net closer on every track; on words **≥ 8 closer and ≤ 3 further**, on slides **≥ 10 closer**. | 0.55 |
| P6 | `slides/batch-004/pptx/solog_orientation_august_2019.pptx` — the document the defect was found on — stops drawing a face the reference does not, i.e. its face-set distance falls. | 0.60 |
| P7 | `ABCD-FE-01-00 Flight Envelope` and `ABCD-WB-08-00`, the two `words-pages-01` moved *away* from the reference, lose the extra DejaVu Serif they draw from their `Times-Roman` entry. | 0.70 |
| P8 | `XRay.Office.Fidelity.Tests` stays at **30 failed of 550**. | 0.60 |
| P9 | No batch verdict is lost: words stays 59/60, slides ≥ 57/58, sheets ≥ 57/60. | 0.50 |
| P10 | The declared-family logic (`DeclaredGenericFor`, `ClaimsEquivalenceWith`, the pi-face exemption) is **not** made redundant: the declared class still overrides fontconfig's classification of the name, which is the only way `Times` declared swiss reaches DejaVu Sans while fontconfig files `Times` as serif. | 0.85 |

## The way this could be the wrong shape of fix

Reading the machine's configuration makes the answer a property of the machine. That is already
true of `FontconfigPreferences` and is defensible for the same reason — the reference renderer asks
the same configuration — but it means a stored figure is only reproducible on a box with the same
`/etc/fonts`. The alternative, baking fontconfig's tables into the source the way `VCL.xcu` is
baked, is what produced this defect in the first place.
