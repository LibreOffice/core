# Words rebase round 2 — prediction, committed BEFORE any measurement

Written before rendering anything, before opening the corrected reference's per-document
figures, and before running any gate. Timestamped by file mtime.

Environment: LibreOffice 26.2.4.2 620(Build:2); `fonts-dejavu-core` + `fonts-dejavu-mono`
installed; `fc-match "DejaVu Sans"` → DejaVuSans.ttf. CLI is HEAD's build (no `.cs` under
`dotnet/src` or `dotnet/tools` is newer than the binary — checked, zero files).

## P-1 — the thing the brief and my predecessor both got wrong (stated first, because it
determines everything else)

**Ours is not font-independent.** `XRay.Office.Text/Fonts/FontconfigPreferences.cs` reads
`/etc/fonts` and `GlyphFallback` emulates `FcFontMatch`. So installing DejaVu changes *our*
rendering too, not only the reference's. Consequences I commit to before measuring:

- The predecessor's **129** is a self-consistent measurement of a *different environment*
  (ours font-starved vs ref font-starved), not a wrong number — but it is not the answer to
  "how do we score here".
- Its **108** row (stored r47 ours × newly-measured font-starved ref) pairs a with-DejaVu
  ours against a without-DejaVu ref. I predict that row is **an incoherent mixture** and
  that its "the binary alone costs 50 verdicts" sentence is therefore wrong in a second way
  beyond the one the brief names.
- I must re-render ours from scratch. I will not reuse `words/gate/ours/*.pdf`.

## P0 — verdict movement this round

**Zero.** I plan no source change. If I touch source at all it will be in a worktree and
will not be part of the scoreboard.

## P1 — the corrected scoreboard

ours@HEAD vs ref@26.2.4.2+DejaVu. Point estimate **133**, band **120–145**.
Rationale: with the correct font set both engines resolve the same faces, and ours models
fontconfig's preference order deliberately, so the two should agree slightly *more* often
than they did when both were falling back to WenQuanYi Zen Hei — but only slightly, because
267/534 of the affected renders were fallbacks in the reference and ours' fallback chain is
an emulation, not the same code. If it lands outside 115–150 I will suspect the harness
before I believe the number.

## P2 — how much of ours moves with the font set

I predict **25–60** of our 200 page counts differ between the font-starved render and the
DejaVu render (the reference moved 42 by the brief's figure). I predict the direction is
predominantly the same as the reference's — **fewer** pages with DejaVu — on at least 70%
of our movers. I predict our extractable word counts move on **fewer than 40** documents,
far fewer than the reference's 174, because our word extraction does not depend on which
face is chosen (glyph→Unicode mapping is ours either way) whereas the reference's PUA
bullet inflation is an import/export behaviour.

## P3 — the decomposition

Legs I can compute, with predictions:

| leg | what it isolates | prediction |
|---|---|---|
| ours@r47 × ref@24.2.7.2 (stored) | the historical figure | 158 exactly, reproduces |
| ours@r47 × ref@26.2.4.2+fonts (computed) | **binary alone**, if the old container had DejaVu | **115–135**, point est. 124 |
| ours@r47 × ref@26.2.4.2−fonts (predecessor) | nothing coherent | 108 (reproduces) |
| ours@HEAD−fonts × ref−fonts | predecessor's environment | 129 (reproduces) |
| ours@HEAD+fonts × ref+fonts | **this environment** | see P1 |

I therefore predict the binary change alone costs **23–43** verdicts, point estimate **34**,
against the predecessor's claim of 50 — i.e. I predict roughly **a third of the claimed 50
is the font, not the binary**, and that the predecessor's headline decomposition is
overstated but not wildly so. If the computed binary leg comes out at or below 110 the
font contributed almost nothing at r47's code level and my P-1 reasoning is wrong.

## P4 — the three outliers, reference page count with DejaVu

- `A_320.doc`: font-starved ref was 118. I predict **118 ± 4** — it is a `.doc` whose faces
  are the usual Times/Arial family, so I expect little or no font effect.
- `AC-150-5370-10G-updated-201604.docx`: font-starved ref 724. The brief says it appears in
  *both* the version-effect and the font-effect lists, and the font effect is always
  downward. I predict **below 714**, i.e. it drops by more than 10.
- `150-5370-10H.docx`: font-starved ref 727. I predict it also drops, and by less than 10G
  does — the two revisions moving together again.

If 10G does *not* drop, the "font effect is all one direction" statement in the brief is
either wrong or does not include this document, and I will say which.

## P5 — the 720 dpi twip law

I predict it **holds unchanged, 170/170 at 720 dpi and no other resolution above 100**,
because the probes are authored flat-ODT naming Liberation Sans, which was installed in
both font environments. The re-check is a control against my own P-1 reasoning: if a probe
that names an installed font *does* move when an unrelated font is installed, then font
installation is perturbing something other than face selection and every other figure this
round is suspect.

## P6 — what this round's census CANNOT see, stated before the sweep

- **There is no LibreOffice 24.2.7.2 anywhere and no route to one** (the download hosts are
  firewalled). So the cell "ref@24.2.7.2 with a known font set" is unavailable, and the
  2×2 that would let me attribute the 158→X drop cleanly has **one of its four cells
  permanently missing**. Any full three-way decomposition I could write would be fabricated.
- **The old container's font set is inferred, not observed.** The argument that DejaVu was
  present rests on `SheetColumnDigitsTests` pinning DejaVu metrics against 24.2.7.2 output.
  That is strong for *DejaVu* and says nothing about the rest of that machine's fonts. So
  the "binary alone" leg assumes the old machine's font set equals this one's.
- **The stored r47 ours column cannot be re-rendered.** It also depended on that machine's
  HarfBuzz and Skia builds, not only its fonts. Every leg using it is computed, not measured.
- **No page-level correspondence.** `pdfinfo`/`pdftotext` say a document diverged, never where.
- **No `.doc` XML census.** 66 of the 200 are binary `.doc`; any zip-level classifier is a
  statement about the 134 DOCX only and is a ceiling, not an estimate.
- **A per-document attribution of lost verdicts is impossible where ours also moved.** Only
  documents whose ours column is identical across all three ours-renders can be attributed.

## P7 — controls I will run before believing any aggregate

1. **Known-answer control on the gate itself**: run the verdict harness with the corrected
   reference PDFs as *both* columns. It must return 200 match, 0 mismatch. Anything else
   means the id/join is broken — the failure mode that manufactured a "534 of 534 changed"
   result this session.
2. **Replay the verdict rule** over 1000 stored rows from five past rounds; require zero
   mismatches, re-run rather than inherited.
3. **Shape check on every aggregate**: no total may coincidentally equal a corpus-wide
   constant (page total, document count).
4. Every classifier runs over **all 200**, matching documents included.
