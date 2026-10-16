# fidelity-01 — prediction, committed before measuring

Written after reading the brief, `MISSING_PACKAGES.md`, the Fidelity project layout and
`XRay.Office.TestKit/LibreOffice/LibreOfficeRunner.cs`, but **before running the suite even once**.

## What I already know (measured, not predicted)

- `check-env.sh` on this container: LibreOffice **26.2.4.2** 620(Build:2); pdftoppm/pdftotext
  **26.01.0**; DejaVu Sans resolves to DejaVu Sans (font present). Environment reports "good".
- `df -h /` → 13 GB free, 34% used. **The mass-failure-means-full-disk signature does not apply.**
- The Fidelity harness runs `soffice` **live, at test time** (`LibreOfficeRunner`), and compares
  our output to freshly produced LibreOffice output. It is *not* purely a bank of stored numbers.

That last point matters and I want it on the record before I measure, because it cuts against the
brief's framing. If the reference side is computed live, then a version bump does **not** make an
expectation stale in the usual sense — both sides move together on a pure round-trip comparison.
A version bump only bites where a test hard-codes a figure, or where it asserts a *tolerance* that
was calibrated against 24.2.7.2's particular output.

## Prediction

**P1 — the split.** 550 total is unchanged; only the pass/fail split moved. 510 + 40 = 550 exactly,
and the arithmetic is too clean for tests to have been added or removed. I predict the handover's
550 and today's 550 are the same 550 tests. *Confidence: high.*

**P2 — 21 names / 40 cases.** The 21:40 ratio confirms parameterised cases (xUnit `[Theory]`).
I predict the 19 extra failures are additional data rows on a minority of the 21 names, not a
second mechanism. *Confidence: high.*

**P3 — the dominant class.** I predict the plurality classification is **environment (LibreOffice
version)** — roughly 20-25 of the 40 — concentrated in tests that hard-code a figure read out of
24.2.7.2 rather than tests that compare live-to-live. *Confidence: medium.*

**P4 — the font class is real but small.** Because DejaVu moved 53 of 534 reference page counts,
I predict **2-6** failures trace to the font set, and that they are page-count / pagination
assertions rather than text-content assertions. *Confidence: medium-low.*

**P5 — poppler.** Extraction-family tests (`ExtractionComparisonTests`,
`XlsxExtractionComparisonTests`, anything counting words) are the exposed surface. I predict
**3-8** failures here. *Confidence: low.* This is the weakest of my predictions because the brief's
poppler evidence (169 of 200 documents' word counts moved) is about a corpus sweep, not about
these tests, and I have not yet checked whether Fidelity's extraction tests use poppler at all.

**P6 — the one I most expect to be wrong.** The brief says every predecessor claim reproduces to
the digit while the sentence attached to it is wrong. Applying that to the brief itself: I predict
**at least one genuine defect** is in the 40, hiding behind the environment story, and that it is
most likely in the family with the *largest* number of failing cases — because a big parameterised
block failing wholesale is as easily one broken code path as one stale number. I predict
**1-8 genuine defects**. *Confidence: deliberately wide; this is the finding worth having.*

**P7 — unexplained.** I predict a non-zero unexplained residue, **1-5**. Forcing all 40 into a
clean cause would itself be the predecessor error this project keeps repeating.

## What would falsify each

- P1: a total that is not 550 today, or a `git log` showing tests added since the handover.
- P2: 21 names where the extra 19 failures come from a distinct second family.
- P3: the hard-coded-figure tests turn out to be a small minority of the 40.
- P4/P5: re-running the specific tests with the variable held constant does not move them.
- P6: all 40 reproduce as environment with a demonstrable pre-existing green commit.

## Method I intend to follow

1. Build and run Fidelity once, capture the full failure list with messages. Measure before theory.
2. Group the 40 by test class and by assertion shape.
3. For each group, decide the cheapest discriminator:
   - hard-coded figure → read the constant and its comment; does the comment name 24.2.7.2?
   - live-vs-live → the version cannot be the cause; look for a real defect.
   - `git log` the test file and the code it exercises; if the test predates any relevant code
     change and the code is untouched, environment is decisive.
   - `git merge-base --is-ancestor <fix> HEAD` before asserting any defect is still open.
4. Classify. Leave "unexplained" where the evidence does not reach.
