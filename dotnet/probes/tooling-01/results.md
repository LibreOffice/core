# tooling-01 — `xray-office analyze`, and what it cost to trust it

The gate read every figure it has ever recorded out of three poppler binaries. This round replaces
them with a verb in our own CLI, pinned in the repository, and then spends most of its length
proving the replacement rather than asserting it.

**Branch `wt-tooling`.** Four commits. Build 0 warnings / 0 errors. Ten non-Fidelity projects
**3486 total, 0 failed** — the briefed 3458 plus this round's 28, with every other project's count
unchanged.

---

## 1. The headline

> **Page count agrees with poppler on 534 of 534.** So does the face count, the unembedded count and
> the subset count — 534 of 534 on each, which is more than was asked for and more than I expected.
>
> **Word counts differ, and every large difference is accounted for.** Under the gate's merged
> metric, 154 documents agree to the digit and **480 of 534 lie inside the gate's own 2%+3 band**.
> The net difference over 6.64 million words is **−0.17%**.
>
> **The claim is not that this is more accurate than poppler.** It is that the heuristic is now ours,
> pinned to a package version, and stops changing under us.

| | ours | poppler 26.01.0 | agree |
|---|---:|---:|---:|
| page count | — | — | **534 / 534** |
| page count vs banked `ref-baseline-all.tsv` | — | — | **534 / 534** |
| font faces | — | — | **534 / 534** |
| unembedded faces | — | — | **534 / 534** |
| subset faces | — | — | **534 / 534** |
| words, raw tokens | 6 945 129 | 6 937 100 | 129 exact, **471** in band |
| words, gate metric | 6 635 680 | 6 647 292 | 154 exact, **480** in band |
| documents that failed to read | **0** | 0 | — |

---

## 2. What was built

`xray-office analyze` reads a PDF in process with PdfPig and reports page count, per-page size,
extractable text and word counts, and every font face with its embedding and subset status. One
parse answers all three of the gate's questions; the shell version spawned five processes per
document and read the file five times.

Output is TSV with a header, one row per document, plus `--pages`, `--fonts`, `--text` and `--json`.
Numbers are formatted with `CultureInfo.InvariantCulture` explicitly — the solution sets
`InvariantGlobalization=false`, so a locale that writes `595,304` for a page width would turn the
TSV into something `awk` reads as two fields.

**Two things are named options rather than hidden constants**, because both were decisions and both
should be re-checkable:

| option | default | what it selects |
|---|---|---|
| `--words alnum \| raw` | `alnum` | which of the two reported totals the `words` column carries |
| `--grouping nearest \| simple` | `nearest` | how glyphs are grouped into words |

Both totals are always emitted, together with the three classes they differ by — `bullets`,
`symbols` (Private Use Area), `punct` — so a comparison can show what a difference is *made of*
rather than only how big it is. The classes partition the raw count exactly, and a test asserts it.

### The word definition is the gate's, reimplemented rather than re-decided

`words_of()` in `batch-check.sh` is `any(c.isalnum() for c in w)` over `text.split()`. Two details
matter and both were got wrong first:

- **`str.isalnum()` is true for every numeric category**, Nd, Nl *and* No — not only the decimal
  digits. `char.IsLetterOrDigit` stops at Nd and scores `½`, `Ⅻ` and a superscript `²` as
  not-words. The predicate is `Rune.IsLetter || Rune.IsNumber`.
- **Python iterates code points, .NET iterates UTF-16 units.** `char.IsLetter` is false for either
  half of a surrogate pair, so a CJK ideograph above the BMP would be counted as punctuation by a
  reader that believes it handles Unicode. The tokeniser enumerates runes.
- Python's `str.split()` also treats U+001C–U+001F as whitespace and `char.IsWhiteSpace` does not.
  Four code points, added rather than argued about.

Verified end to end: on `003__doc.pdf` our text gives 984 / 999 and poppler's text through the
gate's own one-liner gives 984 / 999.

### What was rejected

- **A stream/`byte[]` API.** The reader takes a path. The gate compares files on disk and the
  test-only cases that want otherwise can write a temp file; a second entry point is surface with no
  caller.
- **Putting the reader in a `src/` library.** X-Ray.Office *writes* PDFs; nothing under `src/` reads
  one, so a reader there would add a dependency to redistributable libraries to serve a tool. It
  lives in `XRay.Office.Cli`, whose only package reference it is.
- **`pdfminer.six`, or any second extractor, as an arbiter.** Recorded because the previous round
  already paid for it: two extractors reading the *same current file* agree by construction and
  discriminate nothing.
- **Poppler's duplicate-glyph suppression.** Measured and deliberately not landed — §5.4.
- **Re-tuning PdfPig's grouping constants.** They are tuning constants over glyph geometry. Changing
  them puts the heuristic back in our hands and out of the pinned package, which is the whole point
  of pinning it.

### The dependency

`PdfPig` moves from `0.1.15` to **`0.1.16-alpha-20260811-39f52`** as asked, and out of the
`Test-only` group because the tool uses it too. It restored, built and ran with no incident; nothing
had to be pinned back. That version is upstream commit `39f52eb`. Apache-2.0, pure managed, no
native assets, so the host-RID pin in `Directory.Build.props` is untouched and unaffected.

---

## 3. The instrument had two defects. Both were found by measuring, not by reading

This is the part of the round that justifies the acceptance bar, so it is stated before the results
it produced.

### 3.1 The font walk did not terminate

Resource dictionaries are shared. A deck's ten slides point at one resources dictionary holding *n*
form XObjects, and each of those points back at a resources dictionary holding the same *n*.
Descending without remembering costs O(*n*^depth); **the depth cap does not save it, it only bounds
the exponent.**

It hung on **17 of the 534**. `1-secretariat__ppt.pdf` is 10 pages and 2402 glyphs, reads in 0.7 s
with the font walk removed, and **had not finished the walk after 570 s**. The first sweep looked
like "this container is slow" — 203 documents in 570 s — and a second pass over the same list added
*zero*, which is what finally made it legible.

With a document-wide visited set keyed on the indirect reference, the whole corpus reads in **107 s
wall** at 16 workers.

### 3.2 Glyphs drawn off the page were being counted

Poppler discards out-of-bounds characters in `TextPage::addChar`. This reader did not, and that was
**the single largest term in the entire difference**.

| | ours | poppler |
|---|---:|---:|
| `CIS_Debian_Linux_8_Benchmark_v1.0.0.xls`, all glyphs | 18 106 | 9290 |
| the same document, on-page glyphs only | **9016** | 9290 |

50.8% of that document's 125 086 glyphs sit off the page — a spreadsheet's cell contents overflowing
past the printable area. Corpus-wide Σ\|Δ\| on raw tokens fell from **120 122 to 74 088** and the
in-band count rose from 410 to 446.

The rule is the pen position against the **crop box**. Both alternatives were measured, not assumed:
an ink-box intersection test scores **442** in band against this rule's **446**, and the crop box
rather than the media box because the crop box is what a viewer shows and has already been clipped
to the media box.

---

## 4. The 534-document comparison

`/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/`, all 534 canonical reference renderings. Poppler
26.01.0, invoked with the expressions `batch-check.sh` used, plus the gate's own `words_of()` for
its alphanumeric column.

### 4.1 Page count — 534 / 534, three ways

| comparison | agreement |
|---|---:|
| our page count vs poppler's, now | **534 / 534** |
| our page count vs the banked `ref-baseline-all.tsv` | **534 / 534** |
| poppler's page count now vs the banked file | 534 / 534 |

The third row is the control that matters: it says the banked file and this container's poppler
still agree, so the first two rows are one fact confirmed twice rather than two independent facts.
44 documents carry more than one visible page size; none carries a `/Rotate` on its first page.

### 4.2 Fonts — 534 / 534 on all three columns, with nothing to explain

Face count, unembedded count and subset count each agree on every one of the 534. There is no
disagreement list because there are no disagreements.

That is a stronger result than "very closely" and it deserves a word of suspicion rather than
celebration, so: the two are **not** the same code path. Poppler walks `GfxFont` objects built by
its own parser; this walks the raw token tree, de-duplicating on the indirect reference of the font
dictionary. They agree because the question — "which font dictionaries are reachable from a page's
resources, and does each one's descriptor carry a font program" — has one right answer that both
happen to compute. The corpus exercises it: 2119 faces in all, including Type 0 fonts whose
descriptor lives on the descendant CIDFont, and faces reachable only through form XObjects.

### 4.3 Words

Raw tokenisation against poppler's raw tokenisation first, because agreement on the *corrected*
metric proves nothing until the raw comparison is on the table.

| | exact | in 2%+3 band | within 1% | net | Σ\|Δ\| |
|---|---:|---:|---:|---:|---:|
| **raw vs poppler raw** | 129 | 471 | 429 | +8 029 | 86 313 |
| **gate metric vs poppler's** | 154 | **480** | 456 | −11 612 | 60 704 |

Per track, gate metric:

| track | n | exact | in band | net | Σ\|Δ\| |
|---|---:|---:|---:|---:|---:|
| words | 200 | 57 | **193** | +4 874 | 5 324 |
| slides | 163 | 39 | **132** | +3 661 | 8 205 |
| sheets | 171 | 58 | **155** | −20 147 | 47 175 |

**The gate's control was 1068/1068 on raw against a stored `rawwords` column, and mine is 129/534.
That gap is the finding, and it is expected**: the gate compared poppler's tokenisation with
poppler's own, which must agree; this compares two different extractors' inferences about where
words begin. A PDF stores positioned glyphs, not words.

The excluded classes, corpus-wide: **7347 bullets, 8372 private-use symbols, 293 730 other
punctuation-only tokens**, totalling 309 449 against poppler's 289 808 excluded. The classes
partition the raw count on 534 of 534.

---

## 5. The word difference, by cause

Each of these was established by comparing the two token streams document by document, not inferred.

### 5.1 Off-page glyphs — the largest, and now removed

§3.2. Was worth 46 018 tokens of Σ\|Δ\| on its own. Concentrated in spreadsheets, where a cell
overflowing a narrow column is drawn in full and clipped.

### 5.2 Rotated text — the second largest, and now removed

`DefaultWordExtractor` has **no concept of text orientation**. Every glyph of a 90°-rotated run sits
at a different height, so its "same line" test starts a new word at every character — and it sorts
by height, which interleaves glyphs from *different* rotated labels. A chart's category axis is
where this lives.

| document | simple grouping | nearest-neighbour | poppler |
|---|---:|---:|---:|
| `Keywords_Mapping_Graphs_and_Charts.xlsx` | 8225 | **4431** | 4519 |
| `NAS-Infrastructure-Roadmaps-v16.0.pptx` | 22 771 | **18 919** | 18 850 |
| `2014BSA_Sunday_Killion.pptx` | 5208 | **3566** | 3559 |

**A correction to the brief, and it changed the round's default.** `NearestNeighbourWordExtractor`
first measured at *twice* poppler's count on ordinary documents — 1951 against 999 on `003__doc` —
which read as it being unusable. It is not: it returns **space glyphs as singleton `Word` objects**,
roughly one per real word. Counting `Word` objects doubles the total; tokenising the text, which is
what `wc -w` does and what this reader does, does not. On the same document it gives 999.

Chosen on the gate's own criterion: **480 documents in band against 460**, 37 moving in and 17 out.
It is also the structurally right answer rather than merely the better average — the alternative is
not slightly worse on rotated text, it returns one word per glyph.

### 5.3 Accounting-format cells — real, and invisible to the merged metric

`fy2011-aip-grants.xls`, +4511 raw tokens. The whole difference is `$` and `-`: an accounting zero
puts the currency symbol at the left edge of the cell and the dash at the right, and the two
extractors draw the word boundary differently across that gap. Ours emits 6067 `$` and 3965 `-`;
poppler emits 4157 `$`, 804 `-` and 294 joined `$-`.

**Under the gate's merged metric this document is inside the band**, because none of those tokens
carries a letter or a digit. An independent confirmation, from a direction the gate round did not
look from, that the merged definition removes a real term rather than a cosmetic one.

### 5.4 Coincident and near-coincident double strikes — measured, not landed

Some documents draw their text twice, offset by a fraction of the glyph size: a fake-bold or a drop
shadow. Poppler suppresses the second copy (`TextPage::addChar`, within `dupMaxPriDelta`=0.1 and
`dupMaxSecDelta`=0.2 of the font size); this reader counts it. `Framing Europe.ppt` extracts as
"Framing Europe Framing Europe …" against poppler's single copy.

Implemented as an experiment and measured over all 534: it finds 16 560 duplicate glyphs (0.04% of
46.7 million) across 87 documents and would move the in-band count from **480 to 486**, slides from
132 to 138.

**Not landed, deliberately.** It is a fourth hand-tuned rule carrying two magic constants copied out
of poppler — which is precisely the thing pinning a package was meant to stop — it costs about 40%
more time, and it does not close the case it was aimed at: `Framing Europe` goes 4480 → 4158 against
poppler's 2237, so its doubling is at a larger offset than the rule catches and remains unexplained.
Recorded with its number so the next round can take it as a decision rather than a discovery.

### 5.5 Nearest-neighbour merging across spreadsheet columns — the cost of §5.2

The grouping that fixes rotated text bridges the gap between two adjacent cells on a wide sheet and
merges their contents. It is the reason the sheets track is the one place the simpler grouping wins
(155 in band against 159), and it is concentrated: three documents carry 34 365 of the sheets
track's 47 175 Σ\|Δ\| — `Laser Report 2024 FOIA (Oct).xlsx` alone **−22 308**.

### 5.6 Poppler fragmenting words — where our reading is the better one

Three `.ppt` decks where we report *fewer* words: `architecture6` −616, `pres_ioc_phuket` −520,
`Sylva introduction session` −372. The token diff is unambiguous — poppler's extra tokens are
`e`, `t`, `n`, `va`, `ion`, `p`: **fragments of words we return whole**. LibreOffice writes these
decks' text with per-glyph positioning and poppler's gap rule breaks inside words.

### 5.7 What remains unexplained

`STC_WebList.xlsx`, +9354 on 1 294 280 (0.7%, comfortably in band), and the residue of `Framing
Europe` after §5.4. Also the 37 pages in `TODO.raster-ceiling.md` where LibreOffice rasterises an
embedded object and its PDF carries a picture where ours carries text — a term that belongs to
neither extractor and is already documented.

---

## 6. Controls

1. **Known answer, built by construction.** An authored flat ODT — 250 plain words, ten lines of a
   literal bullet plus two words, five punctuation-only tokens — rendered through our own pipeline.
   Expected 285 raw, 270 words, 10 bullets, 5 punctuation. Reported **285 / 270 / 10 / 5**, and
   poppler independently reports 285 / 270 on the same file. **The error is zero, not near zero.**
2. **Determinism, whole corpus.** Two independent full sweeps of all 534: identical on every one of
   the 16 columns of all 534 rows. Extracted text byte-identical over 12 documents by hash.
   (Not a truism — the word grouper fans out over orientation buckets and merges under a lock at its
   defaults, which makes the *order* of its output depend on scheduling. It is pinned to one thread.)
3. **Metric parity with the gate's own code.** Our C# and the gate's Python one-liner agree on the
   same text.
4. **The banked-baseline cross-check**, §4.1 — three-way, so that agreement is confirmed rather than
   assumed.
5. **Shape check.** 534, 480, 471, 154, 129, 44 — none coincides with a corpus constant. The failure
   this guards against is real on this project: a mis-aligned `join` once reported "534 of 534
   documents changed", the tell being that the total equalled the corpus page count.
6. **Zero read failures on 534**, so no figure above is an average over a subset.

---

## 7. Speed

Whole corpus, 534 documents, 16 workers:

| | wall | serial total | median | p95 |
|---|---:|---:|---:|---:|
| `xray-office analyze` (1 process per document) | **107 s** | 1171 s | 1405 ms | 5226 ms |
| poppler (5 processes per document) | **25 s** | 450 s | 424 ms | 2310 ms |

**Poppler is faster and this does not claim otherwise** — roughly 4× on wall, 2.6× serially. Almost
all of our time is PdfPig's content-stream parsing, which is the price of reading the text at all;
the font walk and the word grouping together are under a fifth of it. 107 s for the whole corpus is
a small fraction of a gate run that also renders 534 documents through `soffice` and through our own
CLI, so the exchange is a real cost bought for a real property.

---

## 8. Tests

**28 test cases in 17 methods**, in `tests/XRay.Office.Rendering.Tests/PdfAnalysisTests.cs` — the
project that produces the files the reader reads. Six of them are built from hand-assembled PDFs
with computed offsets and a real cross-reference table, because "this glyph is off the page" and
"this face has no font program" have to be *inputs*, not hopes.

### Verified by reintroduction — 12 methods

Each was shown to fail when the behaviour it claims was removed, using `verify-test.sh` (exit 0).

| method | mutation it caught |
|---|---|
| `GlyphsDrawnOffThePageAreNotCounted` | on-page filter replaced by `page.Letters` |
| `TheWordDefinitionFollowsTheGateAcrossEveryNumericCategory` | `IsLetter\|\|IsNumber` → `IsLetterOrDigit` |
| `ASharedResourceTreeIsWalkedOnceRatherThanOncePerPath` | visited-set guard defeated; also XObject recursion removed |
| `AFaceReachableOnlyThroughAFormXObjectIsFound` | XObject recursion removed |
| `APageInheritsItsResourcesFromThePageTree` | `/Parent` walk replaced by `null` |
| `AFaceWithAFontProgramIsReportedEmbedded` | `/FontFile3` read deleted |
| `ASubsetPrefixIsSixUpperCaseLettersAndAPlus` | `name[6] == '+'` → `name.Contains('+')` |
| `AnAuthoredDocumentsWordsAreCountedExactly` | bullet class disabled |
| `APrivateUseSymbolIsCountedApartFromWordsAndFromBullets` | private-use class disabled |
| `RotatedTextIsGroupedIntoWordsUnderTheDefaultGrouping` | default grouping flipped to `Simple` |
| `PageCountAndPageSizeAreReadFromTheFile` | page count off by one |
| `AFileThatIsNotAPdfIsReportedRatherThanThrown` | exceptions rethrown instead of reported |

### Drift guards — 5 methods

Kept and labelled, not weakened.

- `AFaceWithNoFontProgramIsReportedUnembedded` — the negative of the embedded case; no mutation was
  run that only it catches.
- `EveryFaceOurOwnWriterEmitsIsReportedEmbedded` — a cross-check of `PdfFontEmbeddingTests`'
  claim through a different reader, which is its value; it guards our writer, not this reader.
- `TheTokenClassesPartitionTheRawCount` — an invariant across four rendered documents.
- `ReadingTheSameFileTwiceGivesTheSameTextAndTheSameCounts`
- `TheWordCountPolicySelectsBetweenTheTwoReportedTotals`

### Reintroduction found three real gaps, which is the point of running it

All three were reported as **undetected** first, and each was a coverage hole rather than an
equivalent formulation:

1. **`/FontFile3` was untested.** Every embedded font any test built used `/FontFile2`. That read is
   where every CFF face lands; the gap would have surfaced as "LibreOffice's Type 1C fonts are all
   unembedded".
2. **The subset theory could not tell position from presence.** Its only negative case with a plus
   in it, `Foo+Bar`, is seven characters and fails the length guard before the position is read.
3. **Nothing tested inherited `/Resources`.** A producer that shares one dictionary across all pages
   puts it on the `/Pages` node; a reader that misses that reports *zero fonts for the whole
   document* — a failure with no partial form, which reads as a corpus-wide result rather than a bug.

Two further mutations were undetected and were closed by adding the tests that now cover them
(rotated-text grouping, private-use class). After the additions, **every mutation run in this round
is detected**.

---

## 9. Follow-up: the TestKit readers, with a correction to the reach

The brief lists four TestKit readers as shelling out to `pdftotext`. **Measured, it is two**, and
only one of them launches a process:

| file | how it reaches poppler | call sites | Fidelity test files |
|---|---|---:|---:|
| `tests/XRay.Office.TestKit/LibreOffice/PdfWords.cs` | `ProcessStartInfo("pdftotext")`, `-bbox` — **the only launch site in the tree** | 30 | 18 |
| `tests/XRay.Office.TestKit/LibreOffice/PdfPageSizes.cs` | calls `PdfWords.RunBoundingBoxes` and re-parses the same output | 14 | 7 |
| `tests/XRay.Office.TestKit/LibreOffice/PdfTextRuns.cs` | **does not** — inflates content streams itself (`ZLibStream` + regex) | 67 | 27 |
| `tests/XRay.Office.TestKit/LibreOffice/PdfFills.cs` | **does not** — same, its own content-stream reader | 21 | 13 |

So the conversion is **2 files and 1 subprocess launch, reaching 44 call sites across 25 Fidelity
test files** — 43 of the Fidelity project's files touch at least one of the four, out of 550 tests.
`PdfTextRuns` and `PdfFills` only *mention* `pdftotext` in their doc comments.

Three things the next round should know before starting:

- **`PdfWords` reports box geometry, not just text.** Its `Left` is the pen position rather than the
  first glyph's ink, which is what makes it comparable with a layout engine's arithmetic to within a
  fraction of a point. PdfPig's `Letter.StartBaseLine` is the same quantity, so the substitution is
  available — but `Word.BoundingBox` is not the same thing and swapping to it would move every
  justification assertion.
- **`PdfWords`' doc comment records that poppler groups words by vertical position**, so a 22 pt word
  on an 11 pt line is reported apart from its neighbours. The nearest-neighbour grouping does not
  behave that way. Any test that depends on the quirk will move.
- **The type would have to be reachable from both.** `PdfAnalysis` is `public` in `XRay.Office.Cli`
  and `XRay.Office.Rendering.Tests` already references that project, so the route exists; making
  `XRay.Office.TestKit` reference the CLI would put the CLI's dependencies under every test project,
  which is a decision rather than a detail.

The currently-passing Fidelity tests are the only check on that change, and it is a second change
with its own risk. **Not attempted this round, by instruction and on the merits.**

---

## 10. Two container notes

- **`git status` reports untracked files twice under different casings.** The mount is
  case-insensitive, so `dotnet/tests/XRay.Office.Rendering.Tests` and
  `dotnet/tests/xray.office.rendering.tests` are the same directory — same inode — listed twice, and a
  new file appears under both spellings. Stage the correctly-cased path only; a `grep -r` over the
  tree likewise returns every file twice, which inflates any count taken from one. This is a second
  instance of the same family as the symlink trap, and it is the same remedy: stage explicit paths.
- **Background processes do not survive between tool calls here.** Two full corpus sweeps were lost
  to it. Every sweep in this round is written per-document into a directory and skips work already
  present, so an interrupted run resumes instead of restarting.

---

## 11. Measured vs inferred

**Measured here:** all of §4 (page, font and word comparisons over all 534, against poppler 26.01.0
and against the banked baseline); the off-page and grouping rules chosen by re-running the whole
corpus under each alternative; the non-termination defect and its fix; the duplicate-suppression
experiment; every timing figure; the determinism and known-answer controls; the TestKit reach.

**Inferred, and labelled:** that poppler's out-of-bounds discard is the mechanism behind §3.2 — the
*effect* is measured to the document, the attribution to `TextPage::addChar` is from knowledge of
poppler's implementation and no poppler source is present in this image. Same for the duplicate
constants in §5.4.

**Not established:** the residue in §5.7; whether the 480-in-band figure would move if the gate's
band were re-derived against this extractor rather than inherited from poppler's; and anything about
how these figures interact with the Fidelity harness, which was not run.

**Not claimed:** that any of this recovers a verdict. The gate round measured our glyph output at
291 830 against the reference's 289 808 with the term cancelling document by document, and zero
verdicts moved. This round changes the instrument, not the score.
