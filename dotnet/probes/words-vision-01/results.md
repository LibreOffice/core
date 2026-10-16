# words-vision-01 — results

**Scored against `prediction.md`, which was committed before any measurement.**

## First, the honest scoring: the round did not run the method the prediction predicts about

`prediction.md` is a set of predictions about a **visual review** of the words track — a
`look.py --worst` ranking, opened page by page. That review was never run. The session crashed
part-way and the work that survived on disk, and that this round finished, is a different
thing entirely: a defect in **font resolution**, found while setting up for the review and
followed instead because it is a cascade cause and the review is not.

So P1, P2, P3, P4 and P5 are **untested**, not refuted. They are not scored below as
"unconfirmed" — that would imply evidence was gathered and came up short. No ink ranking was
built and no page pair was opened.

| | claim | outcome |
|---|---|---|
| P1 | top-10 ink pages dominated by reflow, ≥6 of 10 | **not tested** |
| P2 | signed ink net negative but small | **not tested** |
| P3 | ≥5 of top 20 show the reference breaking earlier | **not tested** |
| P4 | ≥1 `.doc` with a picture present in the reference and blank in ours | **not tested** |
| P5 | ≥3 findings on documents that PASS the gate | **partially met, by another route** — see below |

P5 is the one the round bears on, and it bears on it in the prediction's own spirit rather than
by its method. The 26 word-processing renderings this round changed (below) are almost all
**gate-passing** documents: the gate is blind to which face a document is set in, so a document
can be page-exact and word-exact and still be drawn in the wrong family throughout. That is
exactly the shape P5 anticipated — findings live in the passing set — reached by measuring font
resolution rather than by looking at pages.

The prediction's own §"What my census cannot see" item 1 also stands vindicated and is worth
recording as a hit, because this round *did* hit it: 6 of the 26 movers are `.doc` files, whose
`FFN` records no XML census could have seen.

## What the round found instead

A DOCX `fontTable`, a DOC `FFN`, an ODF `style:font-face` and a SpreadsheetML `<font>` all state
a **generic family** beside the family name, and the resolver never saw it — it was handed a name
only. For a family that is installed, or whose substitution chain names something installed, the
declaration decides nothing. For a family nobody has, it decides everything.

### Measured on the installed 26.2.4.2, not read off the source

The source says an unknown family falls through LibreOffice's own generic lists. The running
binary demonstrably takes fontconfig's answer *unless the document declares a shape*. Both halves
were measured:

* `2017-04-27-Lease-Transition-Records-Checklist-FINAL-1.xlsx` sets its body in **Bell MT**,
  installed nowhere here, and declares it `family="1"` — roman. Rendered by 26.2.4.2 it comes out
  in **DejaVu Serif**. Delete just those five `<family val="1"/>` attributes from `xl/styles.xml`
  and the same binary renders the same workbook in **DejaVu Sans**, and its extractable words fall
  from 2545 to 2366. `fc-match "Bell MT"` answers DejaVu Sans, so the name alone cannot decide it.
* Re-rendered at `family="2"`, `"3"` and `"5"`: DejaVu Sans in all three — the undeclared answer.
* A one-cell flat ODS naming the same absent family: **DejaVu Serif** for
  `style:font-family-generic="roman"`, DejaVu Sans for `swiss`, `modern`, `decorative`, `script`
  and `system`.

Three filters, one rule: **only roman moves the answer off fontconfig's default**, and swiss moves
it onto the sans chain explicitly. The other four codes leave the name's own class standing.
Mapping `modern` onto a monospaced face is the tempting mistake and would invent a divergence.

## Reach, measured from what a request RESOLVES to

Not from a grep of what files declare. A previous round predicted 35–55 renderings from a grep and
measured 2. This is the whole of both tracks rendered **twice by one binary** — once with the
declared-shape branch live and once with it switched off at runtime — with `SOURCE_DATE_EPOCH`
fixed, and the PDFs diffed **byte for byte**.

| track | documents | renderings that change |
|---|---:|---:|
| words | 200 | **26** |
| sheets | 171 | **3** |

Words movers: 20 `.docx` and 6 `.doc`. Sheets movers: the two Lease-Transition workbooks
(`.xlsx`) and `data_20121018181533.xls` — the last of which is what validates the BIFF `FONT`
family byte against a real corpus document rather than only against a synthetic record.

Scored against the reference's own embedded-font set, of the 26 words movers **21 move closer to
the reference**, 3 are score-neutral, and 2 move further. The two that move further —
`ABCD-FE-01-00 Flight Envelope` and `ABCD-WB-08-00 Weight and Balance Report` — are documents
where the reference embeds **no DejaVu at all** and we were already substituting DejaVu Sans; the
declaration changed which DejaVu, not whether we substitute. That is a pre-existing substitution
divergence with a different cause, and it is recorded here rather than smoothed over.

## Gate effect

Across the 29 changed renderings, one gate verdict is gained and none lost:

* **`150_5335_5a.doc`** — 63 pages against the reference's 64 before, **64 of 64** after, and
  into full parity. A font-fallback correction moving a page count is the cascade rule in its
  usual form.

Nothing else moved a page or a word count. On sheets, the three movers change only which face is
embedded: 0 verdicts gained, 0 lost.

## The Lease-Transition deficit did not close, and it is not a deficit

The brief named this as the thing to close: page-exact, 175 words short, short on every page.
**It did not close.** Our word count is 2323 before and 2323 after; the reference's is 2498.

What did change is the face: we now render it in DejaVu Serif and DejaVu Serif Bold, as the
reference does, where before we rendered it in DejaVu Sans. So the mechanism the brief identified
is real and is now fixed, and the word gap it was expected to close is a different thing.

The gap is a **poppler tokenisation artefact in the reference**, and this is established rather
than asserted. Strip all whitespace from both extractions and the two character streams are
**identical apart from one word** (`at`, which we place in a different cell) — 13 858 characters
on each side, two diff blocks in total. The reference carries **310 single-letter tokens** against
our 154: LibreOffice writes intra-word positioning adjustments after certain capitals, and
`pdftotext` reads each one as a word break — `L icense`, `M aintenance`, `AL S items`, `CM R`. The
reference's 2498 is 2323 real words plus ~175 splits of its own making.

This is `TODO.raster-ceiling.md`'s shape with the sign reversed. There the reference rasterises and
`wc -w` scores our better output as failure; here the reference over-tokenises and `wc -w` scores
our identical output as 7% short. **Both are gate ceilings, not defects**, and this document should
be added to the raster-ceiling list's neighbourhood rather than worked further.

One thing the comparison did surface and did not resolve: the reference embeds DejaVu Sans and
DejaVu Sans Bold *as well as* the two serif faces on every page, while ours embeds only the two
serif faces, on character-identical text. That is a **glyph-fallback** difference — LibreOffice
sends some characters DejaVu Serif does draw to DejaVu Sans anyway — and it is a separate lead,
named here so the next round does not re-derive it.

## What was wired and what was not

Wired: DOCX `w:family` (the branch), DOC `FFN` `ff` bits (the branch), SpreadsheetML
`<family val="N"/>` on both cell fonts and rich-text `rPr`, the workbook default font that column
widths are digits of, BIFF `FONT`'s family byte, XLSB `BrtFont`'s family byte, and ODF
`style:font-family-generic` on the spreadsheet path.

**Not wired: the ODF word-processing path.** `OdtLayoutSource` still resolves from the name alone,
and `OdfFontFace.GenericFamily` is already parsed and sitting there unused. It was left alone
because this round measured the ODF *spreadsheet* filter and not the Writer one, and the two
filters have already been shown to differ on the neighbouring question — LibreOffice's ODF filter
honours `style:font-pitch` where its DOCX filter does not. Wiring it on the strength of the Calc
measurement would be exactly the inference this project keeps having to undo.

**Not wired: the declared pitch, anywhere.** It is read on every path and passed on none. The one
measurement that exists says LibreOffice's Word filters do not act on it, and passing it put one
corpus document into DejaVu Sans Mono that the reference sets in DejaVu Sans.

## Regression

Per the project's ordering rule: `sheets/batch-006` first, then `sheets/batch-001`–`006` together,
then `words/batch-001`–`006` together. Each run twice, control and live, against the banked
26.2.4.2 references.

| range | control | live |
|---|---|---|
| sheets/batch-001–006 | 54 of 60 | **54 of 60**, the same six documents |
| words/batch-001–006 | 58 of 60 | **58 of 60**, the same two documents |

No page count and no word count moved anywhere in either range.
