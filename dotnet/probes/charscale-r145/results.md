# Round 145 — sizing O101, the character width the other three word-processing readers do not read

Reference binary for anything measured here: `/opt/libreoffice26.2/program/soffice` →
**LibreOffice 26.2.4.2**. **[src]** marks a reading of this tree (27.2.0.0.alpha0+, *not* the
reference binary's source); **[bin]** a measurement against 26.2.4.2's own output.

**This is a partial result and says so.** It was begun by a dispatched agent that the container
restart killed part-way; its WW8 walk had finished and is kept, the ODF census was re-run here,
and the RTF one **could not be run at all**. Nothing below is a reference measurement yet — the
`[bin]` leg that shows the attribute changes the drawn advance in each format is still to do.

## The censuses

`census-odf.txt` holds the numbers; `census-ww8.py` and `ww8-census.tsv` are the agent's walk.

| reader | states it | states something other than 100 % | base rate |
| --- | ---: | ---: | ---: |
| ODF (`style:text-scale`) | 350 in 22 of 337 `.odt` | **111 in 18** | 210 828 `text:span` |
| WW8 (`sprmCCharScale`, 0x4852) | 8 in **1** of 66 `.doc` | **8 in 1**, all 99 % | 61 366 CHPX |
| RTF (`\charscalex`) | **not censused — see below** | | |

**A run stating 100 % costs nothing**, which is why the middle column is the one to read: two
thirds of the ODF occurrences are the identity.

## Two instrument findings, either of which would have produced a fabricated number

**The converted RTF column does not exist in this container.** `dotnet/CLAUDE.md` describes
`/home/user/corpus-odf` as 1285 files — `odt` 338, `rtf` 338, `ods` 307, `odp` 302. It holds
**`odt` 337 and `ods` 307 and nothing else**: the round that re-created it after an earlier
container rebuild re-created two columns of four. A glob for `\charscalex` over the absent
directory returns **0 occurrences**, which reads exactly like a nil-reach finding and is not one.

*The tell is the base rate.* The same pass counted **0** `\fN` run tokens, and a corpus of 338
rich-text files cannot contain no font switches. **Census a base rate beside every reach figure
and refuse the figure when the base rate is impossible** — that check is one line and it is the
difference between "RTF states this nowhere" and "there is no RTF here".

**And a byte-pattern scan of a WW8 stream is not a census.** Scanning for the two bytes `52 48`
anywhere in the document streams finds **185** occurrences where the live CHPX walk finds **8** —
a factor of 23. The same shape of error has bitten this project on `.ppt` hyperlink records, where
a byte scan reported 91 atoms in 17 documents against a record walk's 171 ranges in 23.

## Where it would have to be read, and what each reader would need

**[src]**, all three quoted from this tree:

- `XRay.Office.WordProcessing/Layout/PageContent.cs` — `WidthPerCent` and `IsHorizontallyScaled`,
  added by round 143, are what a paragraph carries.
- `XRay.Office.WordProcessing/Ooxml/DocxLayoutSource.cs` sets `WidthPerCent = text.WidthPerCent`;
  no other layout source does.
- The three measurement fallbacks round 143 had to reach — `PageParagraph.Measure`,
  `PageDrawing.RunsIn`, and the uniform-paragraph shortcut in `Paginator` and `FlowLayouter` —
  are format-independent and already handle it, so each remaining reader needs only to *set*
  the property.

## Recommendation

- **ODF: worth a round.** 18 documents and 111 non-identity occurrences, and the values are
  Word's condense/expand micro-adjustments (99 %, 98 %, 105 %) that move a line break rather
  than a glyph — which is the half round 143 showed decides pagination. One of the 18 is
  `Regulations Governing the Status…`, round 143's own DOCX witness, so the two halves can be
  compared directly on one document.
- **WW8: near-nil.** One document, eight runs, all at 99 %. Worth reading because it is three
  lines beside the ODF change, not worth a round of its own.
- **RTF: unknown, and the corpus column has to be rebuilt before anyone can say.** Do not
  quote a zero for it.
