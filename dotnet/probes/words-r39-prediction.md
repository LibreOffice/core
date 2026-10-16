# Words round 39 — predictions, written before the post-change sweep

Base `9b1429040`. Baseline reproduced exactly: 154/200, |page error| 78, 164 exact pages,
|word error| 6427.

## Change: the last-resort glyph fallback is ordered by the machine's fontconfig preferences

Only the tail of `SystemFontResolver.FallbackFor` moves — the branch reached when *nothing* on
LibreOffice's `ImplInitGenericGlyphFallback` list is installed and covers the character. Everything
above it (the requested family, the substitution table, the generic list) is untouched, and the
generic list is still consulted **before** the platform, which is the reverse of LibreOffice's own
order and is deliberate: putting fontconfig first would take symbol bullets away from OpenSymbol and
undo round 37.

### Reach

The census that bounds it is a font-set comparison over the baseline sweep's own 200 rendered PDFs,
which is a rendering measurement rather than a grep. Three documents draw a face that can only have
come from this tail:

| document | ours | reference |
|---|---|---|
| `P200904290238_0238_51880.doc` | IPAGothic + WenQuanYi Zen Hei | WenQuanYi Zen Hei |
| `手机免提系统TSB.doc` | IPAGothic + Unifont + WenQuanYi Zen Hei | WenQuanYi Zen Hei |
| `AWR OPS-AOC 044 … .docx` | IPAGothic | — (no CJK face at all) |

**Predicted reach: 3 of 200 renderings change.** A fourth document,
`CRIF - Spécification technique - Socle applicatif.docx`, has the difference in the other direction
— the *reference* draws Unifont and we do not — and this change cannot reach it, so it must come
back byte-identical. That is the control.

### Verdict movement

**Predicted: zero documents move to `match`.**

- `手机免提系统TSB.doc` is `words,unembedded` at 2/2 pages and 36/40 words. Dropping Unifont should
  clear `unembedded`, because Unifont is the only CFF face in the file and a CFF face is named and
  not embedded by design. 36 against 40 is 10% out, far outside the 2% band, so the document stays
  a failure on words. Predicted new verdict: `words`. That is a verdict *changing* and not a verdict
  *moving*, and the distinction is the whole point of writing this down.
- `P200904290238_0238_51880.doc` already matches at 3/3 and 271/271. The risk runs the other way
  here: a changed face changes advances and could change pagination. Predicted: still `match`.
- `AWR OPS-AOC 044 … .docx` is `pages,words` at 12/15 pages. The reference has no CJK face, so
  whatever we route through the tail there is drawn by the reference from one of its own faces or
  not at all. Predicted: no verdict change.

**Predicted |word error|: unchanged at 6427** — every character involved is non-ASCII and `wc -w`
in the POSIX locale cannot see it. **Predicted |page error| and exact pages: unchanged** at 78 and
164.

So the honest headline is set in advance: a real defect, the only total-content-loss defect on the
track, with a measured reach of three documents and no expected movement on the scoreboard at all.
