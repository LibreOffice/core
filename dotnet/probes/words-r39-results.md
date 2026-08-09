# Words round 39 — measured results against the predictions

Predictions in `words-r39-prediction.md`, committed at `6118a35c6` before anything was rendered
post-change.

## The scoreboard

| | baseline `9b1429040` | after | predicted |
|---|---:|---:|---|
| documents matching | 154 | **154** | 154 |
| absolute page error | 78 | **78** | 78 |
| exactly-correct page counts | 164 | **164** | 164 |
| absolute word error | 6427 | **6427** | 6427 |

Every prediction held. **Zero verdicts moved**, which was said in advance and is the headline.

One verdict *changed* without moving: `手机免提系统TSB.doc` went `words,unembedded` → `words`,
so the words track now has **no `unembedded` verdict at all**. It is still 36 words against 40 and
still a failure, exactly as predicted.

## Reach

Measured by rendering the track twice and byte-comparing, `/CreationDate` masked:
**2 of 200 renderings changed.**

| document | before | after | reference |
|---|---|---|---|
| `P200904290238_0238_51880.doc` | IPAGothic + WenQuanYi Zen Hei | WenQuanYi Zen Hei | WenQuanYi Zen Hei |
| `手机免提系统TSB.doc` | IPAGothic + Unifont + WenQuanYi | WenQuanYi Zen Hei | WenQuanYi Zen Hei |

`P200904290238_0238_51880.doc` matched before and still matches at 3/3 and 271/271, and its font
set is now **identical** to the reference's. `手机免提系统TSB.doc`'s font set now differs from the
reference's by one face — we draw `LiberationSerif-Bold` and it does not — where it differed by
three before.

The control held: `CRIF - Spécification technique - Socle applicatif.docx`, where the *reference*
draws Unifont and we do not, came back byte-identical.

## The prediction that was wrong, and the finding that replaced it

The prediction named **three** documents; two changed. The third,
`AWR OPS-AOC 044 … .docx`, draws IPAGothic for a different reason, found before the sweep was run
and written up in `TODO.batches.md`: its 218 runs name `w:ascii="MS Gothic"`, and
`msgothic`'s chain in LibreOffice's `VCL.xcu` names `ipagothic`, which is installed. That is
**family substitution, not glyph fallback**, so the tail rule cannot reach it — and the reference
resolves MS Gothic to DejaVu Sans, because it asks fontconfig before it reads that table.

Recording it as a wrong prediction rather than quietly restating the reach as 2 is the point of
having committed the prediction: the census that produced the 3 was correct about *which* documents
draw IPAGothic and wrong about *why* one of them does.
