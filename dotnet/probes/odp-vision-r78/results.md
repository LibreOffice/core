# Reading the five unclassified `.odp` pages, and what they turned out to be

## Why vision, and how it was kept honest

The `.odp` column is at 289 of 302. Of the 13 failing rows, three are the known raster ceiling
(we replay a metafile as searchable text where 26.2.4.2 draws a JPEG, so we draw *more*), and one
is glyph-exact and fails on font embedding alone. **Five remain, all in the direction that means
content is genuinely missing: we draw fewer characters than the reference.** Two earlier rounds
classified these as "other, ODF reader" and left them; nothing programmatic had separated them.

Method, per `.claude/skills/page-vision`:

- Both sides re-rendered at `f0b1aec50` with **all five** font confounds aside. The banked
  `gate-odf-r76` pairs were *not* reused: their reference predates the DejaVu Condensed move, and
  reading a page whose reference used a font the current reference no longer has is exactly the
  stale-evidence error this repository keeps recording rules about.
- The **diverging page** of each document was found first, by per-page alphanumeric count, so a
  reviewer reads the page that differs rather than page 1.
- Each pair composed at the dpi that costs no downscaling. The first attempt threw away 14-42% of
  the pixels and `compose.py` said so; a reviewer reporting "illegible" would have been reporting
  the compositor, not the document.
- **Five independent reviewers, one page each, given the image and nothing else** -- no document
  name, no gate numbers, no repository access. Three of them, on three unrelated documents,
  independently reported the same class. That agreement is the finding; a single reader agreeing
  with itself across five pages would not have been.

## Finding 1: an ODF document's embedded fonts are never loaded

`Ramp Up Campaign - French.odp` holds `Fonts/Font_Alegreya_Sans_Bold_1.ttf` and seven more;
`Sean Monogue.odp` holds four `Fonts/Font_Verdana_*.ttf`. Both declare `EmbedFonts` true in
`settings.xml` and reference the faces by `svg:font-face-uri` in `content.xml`.

**Neither Verdana nor Alegreya Sans exists on this system or in the tarball** -- `fc-list` finds
none and `fc-match` answers `DejaVuSans.ttf` for both. The reference embeds them in its PDF
regardless, because it reads them out of the document. `pdffonts` on the two halves:

| | ours | reference |
|---|---|---|
| `Sean Monogue` | `DejaVuSans`, `LiberationSans` | **`Verdana`**, `Verdana-Bold`, `Verdana-Italic` |
| `Ramp Up Campaign` | `DejaVuSans`, `LiberationSans` | **`AlegreyaSans-Medium`**, `-Bold`, `-ExtraBold` |

`git grep font-face-uri -- dotnet/src` returns **nothing**: the reader never looks for it.

**This is not a confound.** The font is inside the document, so both renderers have equal access to
it and every divergence is ours. It is the opposite of the five tarball confounds, which are faces
the reference has and we cannot get.

Reach across the converted corpus: **6 of 302 `.odp`, 50 embedded faces**; zero `.odt` and zero
`.ods`, because the corpus's embedding all comes from `.pptx` sources carried through LibreOffice's
own ODF export.

The blind reading of `Ramp Up Campaign` p3 describes the consequence without knowing the cause:
*"the reference draws every heading and body run in a narrow, humanist face with true bold; ours
draws all of the same text in a wider, generic-looking sans with no bold at all"* -- then ten
blocks each running one line longer than the reference's, five text collisions where a block
overran into the next, and the last line sliced by the page edge.

## Finding 2: inter-paragraph spacing on a slide is too large by a constant

Reported independently by the readers of `Sean Monogue` p8 and `0335fab9` p6 and
`redac-sas` p7, none of whom saw another's report. All three describe the same thing: the gaps
*between* paragraphs are larger in ours, the error accumulates monotonically down the slide, and
the last block is pushed to or past the bottom edge.

Measured from the rendered PDFs, line centres in points from the page top:

    Sean Monogue p8   ours intra-paragraph 22.6   inter-paragraph 66.3
                      ref  intra-paragraph 22.9   inter-paragraph 49.1     excess 17.2, every gap
    0335fab9 p6       ours intra-paragraph 19.2   inter-paragraph 65.6
                      ref  intra-paragraph 16.0   inter-paragraph 46.4     excess 19.2, every gap

**Within a paragraph we are correct, and on `Sean Monogue` fractionally tighter than the
reference.** The excess is constant per document and appears only between paragraphs, which is
what rules out line height, font size and text measurement as causes.

**The control that separates this from Finding 1:** `0335fab9` renders in `LiberationSans` on
*both* sides -- identical font lists in `pdffonts` -- and still shows the 19.2 pt inter-paragraph
excess on every gap. So Finding 2 is not a consequence of Finding 1.

Both readers also independently noted that all three documents' **line wrapping matches the
reference word for word**, which is the strongest available evidence that the text measurement is
right and only the paragraph-level vertical arithmetic is wrong.

## What the readings could not decide, and what would separate it

The candidate the image cannot rule out, in the readers' own terms: whether we *add* space the
reference does not, or whether the reference *collapses* adjacent space-before and space-after
where we sum them, or whether the reference is shrinking to fit and we are not. The discriminator
is not visual: read the `fo:margin-top`/`fo:margin-bottom` the document declares for those
paragraph styles and compare the sum against the measured 17.2 and 19.2.

## Also observed, not yet worked

- Bullet glyphs: the reference draws green check marks and a green square sub-bullet where we draw
  undifferentiated dark dots (`redac-sas` p7), and gold squares where we draw cream dots
  (`Sean Monogue` p8).
- A slide background's decorative arc-work is absent from ours, and the same slide's gradient
  reaches near-black at the bottom where the reference stays blue (`Sean Monogue` p8).
- Hyperlink underlines are absent from ours (`0335fab9` p6), though the link colour is right.
- The reference breaks a long URL mid-token at any character; we break only at `-` and `/`, so our
  URL lines end short (`0335fab9` p6).

## Finding 3: hyperlink underlines are not drawn at all

Two readers, on unrelated documents, reported the link colour as correct and the underline as
absent. Confirmed **in the PDF's own marks rather than in a raster**, which is the rule this
repository already carries about believing an "it is absent" reading -- counting fills under
2.5 pt tall and over 20 pt wide:

| page | ours | reference |
|---|---:|---:|
| `0335fab9` p6 | **0** | 9 |
| `medicines_bulletin` p1 | **0** | 8 |

So the underline is genuinely not emitted; it is not a mark the compositor threw away.

## The count that makes this worth acting on

Four of the five blind readers reported Finding 2 independently, and two of five reported
Finding 3 independently, none having seen another's report, the document's name, or any figure
from the gate.
