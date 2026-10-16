# slides/metrics-001 — prediction, written before the fix and before any sweep

Target: `Ramp Up Campaign - French.pptx`. Gate today (measured, unfixed binary, 26.2.4.2
banked reference): pages **6/6**, words **418 / 437** (raw 446 / 465), |Δ| = 19 against a
band of max(2% = 8.74, 3) → `words`. Fonts in our PDF: DejaVuSans, LiberationSans. Fonts
in the reference: AlegreyaSans-Medium, AlegreyaSans-Bold, AlegreyaSans-ExtraBold,
LiberationSans.

Everything below this line is a prediction, scored afterwards in `results.md`.

## 1. What the brief says about `.fntdata`, and what I predict instead

The brief states PowerPoint's `.fntdata` is "an obfuscated TTF: the first 32 bytes are XORed
with a key derived from the `p:embeddedFont`'s GUID".

**I predict the brief is wrong on the mechanism.** There is no GUID anywhere in
`p:embeddedFontLst` (that is DOCX's `w:embedRegular/@w:fontKey`, a different thing). I predict
`.fntdata` is **EOT** (Embedded OpenType): a header whose first ULONG is the whole part size,
second is the font-data size, third is version `0x00020002`, and a `0x504C` magic at offset 34;
the font data is the trailing `FontDataSize` bytes and no byte is XORed at all.

I further predict that LibreOffice reaches this through `oox::ppt::EmbeddedFontListContext` →
`EmbeddedFontsManager::addEmbeddedFont(..., key = empty vector, eot = true, ...)`, so the XOR
loop in that function runs zero iterations for PPTX and is only ever exercised by the Word path.

## 2. Does the words path already populate `EmbeddedFaceKey`?

**Predicted: no.** `FontRequest.EmbeddedFaceKey` is predicted to have exactly one producer in
the tree — a unit test — and zero producers in `src/`. `WordFontTable` is predicted to record
the embedded-font *relationship ids* and never open the parts. So this is a new mechanism, not
a wiring job.

## 3. Reach

Predicted, over all 163 documents in the slides track:

- decks carrying `p:embeddedFontLst`: **6**
- embedded font parts across them: **28**, of which **10** are uncompressed EOT and **18** are
  MicroType-Express-compressed (EOT flag `TTEMBED_TTCOMPRESSED`, 0x04) and therefore *not*
  usable without an MTX decompressor, which I do not intend to write
- decks where at least one usable face is named by a `typeface=` attribute the deck actually
  draws with: **3** — `metrics-001/Ramp Up Campaign - French.pptx`,
  `done-011/Session-1-Presentation-Reporting-Forms-Form-12-final.pptx` (Montserrat) and
  `done-014/servicedesk-plus-overviewfinal.pptx` (Roboto)

So I predict **exactly 3 of 163 slides renderings change** and the other 160 stay byte-identical
under a fixed `SOURCE_DATE_EPOCH`. This is a small-reach fix, and I predict it should be
prioritised as such: one gate verdict, two decks whose output changes without their verdict
being at stake.

## 4. Outcome on the target

Predicted after the fix: pages 6/6, words within the band (I predict our count lands at
**437 ± 3**, i.e. the clipped last paragraph returns in full), and `pdffonts` on our PDF names
AlegreyaSans-Medium, AlegreyaSans-Bold and AlegreyaSans-ExtraBold. Verdict `match`.

I predict the mechanism is exactly as the brief describes downstream of the font: DejaVu Sans
is wider than Alegreya Sans at the same size, every block gains a line, and the last paragraph
falls off the slide. So I predict the line count of the worst block drops by exactly one and
nothing else about the page moves.

## 5. Regression

Predicted `slides/metrics-001` → 1/1 match. Predicted `slides/done-*` (144 documents) → 144/144
match, i.e. **done-011 and done-014 change their rendering but keep their verdict**. This is the
prediction I am least confident in: both decks currently fall back to DejaVu for a face they
have no installed copy of, and switching them to the author's face reflows them. If either
loses its verdict I will say so rather than re-scope the fix.

## 6. Fidelity baseline

Predicted **30 failed of 550** in `XRay.Office.Fidelity.Tests` before the change, and predicted
unchanged after it — nothing in the fidelity corpus is a PPTX embedding an uncompressed EOT.

## 7. Tests

Predicted: new tests in `XRay.Office.Text.Tests` (EOT header parsing, compressed EOT declined,
registration of an embedded face and its resolution) and `XRay.Office.Presentations.Tests`
(the `p:embeddedFontLst` → face mapping on the real deck). Predicted all of them fail against
the unfixed tree.
