# Tests — TODO

## Projects

| Project | Purpose |
|---|---|
| `Paperless.TestKit` | Shared helpers: corpus discovery, the headless LibreOffice driver, comparison utilities |
| `Paperless.*.Tests` | Unit tests per library |
| `Paperless.Fidelity.Tests` | The comparison harness against LibreOffice |

All seven test projects build and run. 128 tests currently pass: 106 in
`Paperless.Containers.Tests` and 17 in `Paperless.Core.Tests`, plus five remaining
placeholders. Delete each placeholder as real tests arrive in that project.

## TestKit

- [ ] `LibreOfficeRunner` — implement the stubs. The four gotchas it documents are real and
      verified: output-name collisions, one user profile per concurrent process, PNG export
      giving only page 1, and font substitution deciding fidelity.
- [ ] `CheckFontEnvironment` — assert Calibri→Carlito and Cambria→Caladea up front. Without
      them every OOXML comparison is meaningless, and the failure looks like a Paperless
      layout bug.
- [x] `Corpus` helper: locates `tests/corpus/` by walking up from the test assembly, so the
      files live in one place instead of being copied into every project's output.
      `Require` throws with a message naming the generator script, since a missing corpus
      file means the test cannot run at all and silently skipping would let coverage
      evaporate.
- [x] The 17 format samples committed under `tests/corpus/minimal/`, so unit tests need no
      LibreOffice install
- [ ] Optional-corpus handling for `external/`, which is not committed: skip cleanly rather
      than failing on a machine that never downloaded it
- [ ] `RasterComparer` — implement the metrics. Reference implementation already exists and
      is verified: `.claude/skills/render-comparison/scripts/compare-images.py`. Port it, and
      keep the metric names identical so numbers are comparable across the two.
- [ ] Text comparison mirroring `compare-text.py`'s normalisation
- [ ] PDF reference reading via PdfPig; rasterising via PDFtoImage so the harness does not
      require poppler-utils

## Unit tests

- [x] `Core`: format catalogue completeness, extension ambiguity, family partitioning,
      template and macro flags
- [ ] `Core`: `Length` conversion round-trips and rounding at boundaries; geometry;
      `AffineTransform` composition order
- [x] `Containers`: CFB on real and handcrafted damaged files (cyclic chains, missing
      sectors, truncation); OPC content-type and relationship resolution in both directions;
      ODF mimetype/manifest mismatch; zip-bomb guards on both the catch and false-positive
      sides; XXE refusal
- [ ] `Containers`: decryption, once it exists
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
