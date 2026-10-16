# slides/metrics-001 — a deck's own fonts, and what the gate was actually seeing

`Ramp Up Campaign - French.pptx`, page-exact 6/6 and failing on words. The brief's causal chain
was right and its mechanism was wrong, which is the more useful half of this round.

## The seat

`src/XRay.Office.Presentations/Layout/SlideText.cs`, `SlideFonts.Resolve` — the one place a slide's
run becomes a `FontRequest`. It built the request from family, weight, slant and declared pitch and
left `EmbeddedFaceKey` at its default.

## Did the words path already do this? No — and nothing else did either

`FontRequest.EmbeddedFaceKey` had, before this change, **one producer in the whole tree and it was a
unit test** (`FontResolutionTests.AnEmbeddedFaceWinsOverAnythingInstalled`). Zero producers in
`src/`. The word processor reads `word/fontTable.xml` into `WordFontTable` and records the
`w:embedRegular`/`w:embedBold` relationship ids, and says so in its own remarks — *"this is read and
reported, and nothing consumes it yet. Loading the font bytes is a separate job and a larger one."*
So this was a new mechanism, not a wiring job.

What *was* already correct is the resolver half. `SystemFontResolver.Resolve`'s embedded branch, and
`LoadFace` behind it, needed no change at all: hand them a key they can open and they do the right
thing and record no substitution. The defect was entirely upstream of them.

## The mechanism: `.fntdata` is EOT, not an obfuscated TTF

> The brief: *"PowerPoint's `.fntdata` is an obfuscated TTF: the first 32 bytes are XORed with a key
> derived from the `p:embeddedFont`'s GUID."*

**This is wrong, and it is WordprocessingML's mechanism wearing PresentationML's name.** There is no
GUID anywhere in `p:embeddedFontLst` — the element carries `typeface`, `panose`, `pitchFamily`,
`charset` and up to four `r:id`s, and nothing else. The GUID is DOCX's `w:embedRegular/@w:fontKey`.

Measured on the seven parts of this deck, before writing any code:

| part | first 16 bytes decode as | file size vs `EOTSize` | version | flags |
|---|---|---|---|---|
| font1–3 | EOT header | equal | `0x00020002` | `0` |
| font4–7 | EOT header | equal | `0x00020002` | `0x05` |

Field for field: ULONG `EOTSize` = the part length exactly, ULONG `FontDataSize`, ULONG `Version`,
ULONG `Flags`, 10-byte PANOSE — which for `font4` is `02 0F 05 02 02 02 04 03 02 04`, byte for byte
the `panose="020F0502020204030204"` the XML declares for Calibri — then charset, italic, ULONG
weight, `fsType`, and `MagicNumber = 0x504C` at offset 34. Walking the variable-length half in full
lands on `EOTSize - FontDataSize` on **28 of 28 parts across the corpus**, so the header walk and
the trailing slice agree everywhere they can be checked. Not one byte is XORed.

LibreOffice agrees and the confusion has a precise home. `oox::ppt::EmbeddedFontListContext`
(`oox/source/ppt/EmbeddedFontListContext.cxx`) calls

```cpp
maEmbeddedFontManager.addEmbeddedFont(xInputStream, moCurrentFont->aTypeface, u"",
                                      std::vector<unsigned char>(), true, false);
//                                    ^ key: EMPTY                  ^ eot: TRUE
```

and `EmbeddedFontsManager::addEmbeddedFont` (`vcl/source/gdi/embeddedfontsmanager.cxx:289`) opens
with the XOR loop the brief describes — `bufferRange[pos] ^= key[keyPos++]` — bounded by
`keyPos < key.size()`. For a deck the key is empty, so **that loop runs zero iterations**; it exists
for the Word path, which passes the GUID and `eot = false`. The two mechanisms meet in one function
and only one of them is ours. Established against the installed 26.2.4.2 and its own sources, not
from memory.

**What the three uncompressed parts actually hold**, once unwrapped:

| part | `p:font/@typeface` | name 1 | name 16 | PostScript name |
|---|---|---|---|---|
| font1 | Alegreya Sans Bold | Alegreya Sans Bold | *(absent)* | AlegreyaSans-Bold |
| font2 | Alegreya Sans Bold Bold | Alegreya Sans Bold | Alegreya Sans | AlegreyaSans-ExtraBold |
| font3 | Alegreya Sans Regular Bold | Alegreya Sans Regular | Alegreya Sans | AlegreyaSans-Medium |

Which is exactly the three faces `pdffonts` reports on the reference. The declared `typeface` and
the face's own family disagree on all three, and the *run* names the declared one — so the lookup is
keyed on that. LibreOffice reaches the same place from the other end: since tdf#172647 it registers
the face under `getTypographicFamilyName()` and converts the legacy full name on the run into that
family. Same face on the same runs, without our owning a legacy-name table.

## Compressed EOT is declined, deliberately

`font4`–`font7` set `TTEMBED_TTCOMPRESSED` (0x04): MicroType Express, which LibreOffice delegates to
`libeot` and which has no C# prior art. They are reported as read-but-undecodable rather than as
unreadable, so a caller can tell that case from "not a font part" and fall back on purpose. It costs
this document nothing — the four are Calibri, the deck draws with it twice, and the reference embeds
no Calibri or Carlito at all.

## Reach

Measured over the corpus by parsing every package, resolving every `p:embeddedFont` relationship and
opening every part — **not by grepping**, and reported by what resolves:

| | count |
|---|---:|
| documents walked (whole corpus, all three tracks) | 742 files |
| decks carrying `p:embeddedFontLst` | **6**, all in `slides` |
| embedded font parts across them | **28** |
| … uncompressed and holding a readable sfnt | **10** |
| … MicroType Express compressed | **18** |
| … unreadable for any other reason | **0** |
| decks with ≥1 usable face whose `typeface` the deck actually names | **3** |

The three: `metrics-001/Ramp Up Campaign - French.pptx`,
`done-011/Session-1-Presentation-Reporting-Forms-Form-12-final.pptx` (Montserrat) and
`done-014/servicedesk-plus-overviewfinal.pptx` (Roboto). No `p:embeddedFontLst` exists anywhere
outside the slides track.

**Confirmed against what actually renders**, which is the measurement that counts: all 163 slides
documents rendered twice with `SOURCE_DATE_EPOCH` set, before and after, and diffed byte for byte —
**160 identical, 3 changed, and they are those three.** A small-reach fix, and worth prioritising as
one.

## Result on the target

| | pages | words | raw words | fonts | unembedded |
|---|---|---|---|---|---|
| before | 6/6 | 418 / 437 | 446 / 465 | 2 / 4 | 0 |
| after | **6/6** | **437 / 437** | **465 / 465** | **4 / 4** | **0** |

`batch-check.sh 'slides/metrics-001'` → `TOTAL 1 MATCH 1 MISMATCH 0`. Every column is now exact,
including the raw count the filtered one is derived from. `pdffonts` on our PDF names
AlegreyaSans-ExtraBold, AlegreyaSans-Medium, AlegreyaSans-Bold and LiberationSans — the reference's
list, face for face.

**The clipped paragraph, confirmed in the operators rather than in a raster.** On page 5, before:
15 text-showing operators over 1660 bytes of glyph operands; after: 14 operators over **1973** bytes.
One fewer line and 313 more glyphs shown — the text that was falling off the slide is being drawn.
`pdftotext -f 5 -l 5`, which reads the content stream through the font's `ToUnicode` and rasterises
nothing, finds "…en utilisant les ressources réunies dans le Portail Bien-Être" in the reference and
in ours after the fix, and only "…en utilisant les ressources" before it.

**A fresh subagent read the worst page blind**, given the composed pair and no numbers, forbidden to
read the repository. The first compose warned that each half had been downscaled to 58 %, so it was
recomposed at 87 dpi to land at 98 % and only then handed over. Its report, unprompted:

> *"No difference in line-break points. Every single paragraph wraps at exactly the same word in both
> halves — I checked each one. … No missing or extra text. … No overlaps or collisions."*

with a block-by-block line count — twelve blocks, identical in both halves. What it did find is
worth recording as *separate* open defects, since it had no way to know they were not this one: a
cumulative sub-line vertical drift growing toward the bottom of the slide, the first bullet's text
sitting lower against its icon than the reference's, and slightly tighter inter-paragraph gaps in the
right column.

## Regression

`batch-check.sh 'slides/done-*'` → **`TOTAL 144 MATCH 144 MISMATCH 0`**. All 144 rows written.

The two `done-*` decks whose rendering changed did better than hold their verdict:

| | before | after | reference |
|---|---:|---:|---:|
| `servicedesk-plus-overviewfinal` words | 1820 | **1805** | 1805 |
| `Session-1-…-Form-12-final` words | 1131 | 1131 | 1124 |

`servicedesk-plus` was *passing the gate* at 1820 against 1805 and is now exact, and its `pdffonts`
list matches the reference's face for face and in order — Roboto-Regular, Roboto-MediumItalic,
Roboto-ThinItalic, LiberationSans, Roboto-Light, Roboto-Bold, DejaVuSans, DejaVuSans-Bold.
`Session-1` gains both Montserrat-Bold faces; its one remaining font difference is that we draw
DejaVuSans where the reference draws ArialNarrow, which is an unrelated substitution.

That is the "look at the documents that PASS" rule arriving from the other direction: two of the
three documents this fix touches were already green and were already wrong.

## Tests

14 new tests. Verified against the unfixed tree by copying the changed files aside and reverting —
never `git stash`, which is repository-global here.

`tests/XRay.Office.Presentations.Tests/PptxEmbeddedFontTests.cs` (6) is written end to end — a deck
synthesised in memory, laid out, drawn into a `RecordingDrawingSink`, and the face identified by
what it *is* rather than by where it was written. It uses no type this change introduced, so it
compiles against the unfixed tree, and there **4 of 6 fail**. The other two pass by design and are
named as controls: `ADeckThatEmbedsNothingForTheFamilyStillSubstitutes` is the path 160 of 163
documents take, and `ACompressedEntryFallsBackToSubstitution…` guards a fallback both trees share.
A suite where all six failed would be one with no control in it.

`tests/XRay.Office.Text.Tests/EmbeddedOpenTypeTests.cs` (8) covers the container: the header walk, the
names, a compressed container reported rather than decoded, the XOR-masked variant, truncation, the
store's content addressing, and the resolver round trip. Against the unfixed tree it does not
compile, the types not existing — which is the honest form of "fails" for a new reader, and weaker
evidence than the six above. Its load-bearing negative is
`BytesThatAreNotAContainerAreDeclined`: a plain sfnt, and a sfnt with the Word obfuscation applied
over it, must both be refused. A reader that accepted either would be the brief's hypothesis wearing
this type's name.

**Every project, run individually.** Fidelity baseline established *before* the change and
reproduced after it.

| project | passed | failed | skipped | total |
|---|---:|---:|---:|---:|
| Core | 337 | 0 | 0 | 337 |
| Containers | 109 | 0 | 0 | 109 |
| Text | 357 | 0 | 0 | 357 |
| Vector | 295 | 0 | 0 | 295 |
| Markup | 259 | 0 | 0 | 259 |
| OpenDocument | 125 | 0 | 0 | 125 |
| Rendering | 150 | 0 | 1 | 151 |
| Presentations | 700 | 0 | 0 | 700 |
| Spreadsheets | 832 | 0 | 0 | 832 |
| WordProcessing | 850 | 0 | 0 | 850 |
| **Fidelity** | **520** | **30** | **0** | **550** |
| total | 4534 | 30 | 1 | 4565 |

Fidelity is **30 of 550 before and 30 of 550 after**, 0 skipped both times — the briefed baseline,
reproduced. The one Rendering skip is pre-existing and environmental
(`ACffFlavouredFaceIsNotClaimedToBeTrueType`, `Assert.SkipUnless(TestCffFace.IsAvailable)` — no
CFF-flavoured face on this machine). `XRay.Office.Vector.Tests` reported 295/295 on every run; the
intermittent phantom failures recorded in `CLAUDE.md` did not appear.

## Scoring the prediction

| # | prediction | outcome |
|---|---|---|
| 1 | `.fntdata` is EOT, not a GUID-XORed TTF; LibreOffice's XOR loop runs zero iterations for a deck | **right**, verified on the bytes and in `vcl`'s source |
| 2 | nothing populates `EmbeddedFaceKey`; the words path does not | **right**, one producer and it was a test |
| 3 | 6 decks, 28 parts, 10 uncompressed, 18 compressed, 3 usable-and-named; 3 of 163 renderings change | **right on all six figures**, and the byte-diff picked out the same three decks |
| 4a | our words land at 437 ± 3, font set = the reference's three Alegreya faces | **right**, landed exactly on 437/437 and 465/465 raw |
| 4b | *"the line count of the worst block drops by exactly one and nothing else about the page moves"* | **wrong.** 17 lines went across the document, not one; and the worst page by ink (page 3) had *correct* line counts before the fix — its divergence was sub-line drift, a different defect. I predicted the shape of the change from the brief's narrative rather than from a measurement, which is the mistake the standing instruction is about. |
| 5 | `done-*` 144/144, the two changed decks keep their verdicts | **right**, and understated — one of them went from passing-but-wrong to exact |
| 6 | fidelity 30/550 before and after | **right** |
| 7 | *"all of them fail against the unfixed tree"* | **partly wrong.** Two of the six Presentations tests are controls and pass on both trees by design, and the eight Text tests fail to compile rather than fail. I should have predicted which tests can discriminate and which cannot. |

Five right, one wrong, one partly wrong.

## What this does not do

- **MicroType Express is not implemented**, so 18 of the corpus's 28 embedded parts stay unusable.
  No document in the corpus is currently failing the gate for that reason, which is why it was not
  written; the reach figure above is what would justify writing it.
- **DOCX and ODP embedded fonts are untouched.** The Word path's obfuscation is genuinely the
  GUID-XOR mechanism and needs its own reader; ODF's `Fonts/` parts need a third. `WordFontTable`
  still records the relationship ids and consumes nothing.
- `EmbeddingRights` (`fsType`) is parsed and **not acted on**. LibreOffice defers a restricted face
  rather than dropping it, and re-admits it when the family turns out to be installed anyway — a
  policy nothing in the corpus exercises, since every face actually drawn with declares `0`. Acting
  on it would be an unmeasured guess.
