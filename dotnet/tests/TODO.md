# Tests — TODO

## Projects

| Project | Purpose |
|---|---|
| `Paperless.TestKit` | Shared helpers: corpus discovery, the headless LibreOffice driver, comparison utilities |
| `Paperless.*.Tests` | Unit tests per library |
| `Paperless.Fidelity.Tests` | The comparison harness against LibreOffice |

All seven test projects build and run; each currently holds one placeholder test. Delete the
placeholders as real tests arrive.

## TestKit

- [ ] `LibreOfficeRunner` — implement the stubs. The four gotchas it documents are real and
      verified: output-name collisions, one user profile per concurrent process, PNG export
      giving only page 1, and font substitution deciding fidelity.
- [ ] `CheckFontEnvironment` — assert Calibri→Carlito and Cambria→Caladea up front. Without
      them every OOXML comparison is meaningless, and the failure looks like a Paperless
      layout bug.
- [ ] Corpus discovery, skipping cleanly when a corpus directory is absent rather than
      failing on a machine that never downloaded it
- [ ] `RasterComparer` — implement the metrics. Reference implementation already exists and
      is verified: `.claude/skills/render-comparison/scripts/compare-images.py`. Port it, and
      keep the metric names identical so numbers are comparable across the two.
- [ ] Text comparison mirroring `compare-text.py`'s normalisation
- [ ] PDF reference reading via PdfPig; rasterising via PDFtoImage so the harness does not
      require poppler-utils

## Unit tests

- [ ] `Core`: `Length` conversion round-trips and rounding at boundaries; geometry;
      `AffineTransform` composition order; format catalogue completeness
- [ ] `Containers`: CFB against handcrafted and deliberately malformed files; OPC content-type
      and relationship resolution; ODF mimetype/manifest mismatch handling; zip-bomb guards
- [ ] `Text`: font metrics against known fonts; line breaking against UAX #14 test data;
      shaping stability
- [ ] Per-format readers: one test per feature, with the smallest file that shows it

## Fidelity harness

- [ ] Data-driven over the corpus, one test case per document
- [ ] **Skip, not fail, when `soffice` is absent.** Unit tests must stay runnable on a machine
      without LibreOffice.
- [ ] Commit golden reference images with the LibreOffice version that produced them.
      Rasterised output is byte-deterministic, so checksums are meaningful and a change is a
      real signal.
- [ ] Report failures by signature — cascade / missing content / localised — not by raw
      difference percentage. A percentage does not say which, and the three need different
      fixes.
- [ ] Write diff images on failure and attach them to the test output. The most useful
      artefact the harness produces.
- [ ] Per-metric thresholds; **do not gate on exact pixel equality** — two correct rasterisers
      disagree on antialiasing.

## Fuzzing

- [ ] Fuzz every reader. These are pure parsing code over untrusted input, and malformed
      office documents are a classic attack surface.
- [ ] Corpus seeds from `tests/corpus/` plus LibreOffice's own `*/qa/` files
- [ ] The invariant: never hang, never crash, never allocate unboundedly. Throwing
      `MalformedDocumentException` is a correct outcome.
