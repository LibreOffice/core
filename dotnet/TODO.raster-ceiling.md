# Pages where the reference rasterises and the word gate cannot be won

**Check this list before working any word-count failure.** Several agents have each spent part
of a round re-deriving that some page belongs to this class.

LibreOffice sometimes **rasterises** an embedded object instead of playing it, so its PDF holds a
picture where ours holds real, searchable glyph runs. The rendered pages look alike —
`pdf-image-diff.py` scores them near-identical — but `pdftotext` reads our text and finds nothing
in theirs. **Our output is the better one**, and the word gate scores it as a failure. Driving
those numbers down would mean drawing less text, which is the wrong direction.

Regenerate with:

```sh
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /workspace/sample-files out
.claude/skills/corpus-batches/scripts/raster-ceiling-pages.py /workspace/sample-files out --documents-only
```

Machine-readable copy: `dotnet/raster-ceiling-pages.tsv`.

## How a page earns its flag

Three conditions, on a document whose page count already agrees:

1. The reference draws a raster on that page.
2. **We do not draw that same raster.** Matched on dimensions.
3. We extract materially more words there than the reference does — at least 8 more and at
   least 25% more, about two-thirds of a line of prose.

**Condition 2 was missing until an agent disproved four of this file's own rows.** Without it,
the first condition is satisfied just as well by a logo *both* renderers draw: four pages of
`UG.CAO.00133` were flagged on a 162×109 JPEG of the EU flag in the footer, identical on both
sides, while the document's real surplus was a header block drawn on 13 of its 18 pages. The
signature misfires on any document that puts a small picture in its page furniture and has a
furniture defect elsewhere. Adding it removed **16 of 53 pages — nearly a third of the list.**

Matching on dimensions rather than on content is deliberate: a rasterised metafile and a logo
differ in size by orders of magnitude, and decoding every image to compare pixels would cost more
than the whole scan.

## The numbers

| | |
|---|---|
| pages flagged | **37** across 21 documents |
| by track | 28 slides, 8 words, 1 sheets |
| flagged pages whose document embeds a metafile | 21 |
| flagged pages whose document embeds **none** | 16 |
| excess words accounted for | **2706** |
| documents embedding a metafile at all | 100 of 534 |
| documents that cannot be judged yet | 83 |

An embedded metafile is the commonest cause and not the only one. `W3_Case_Study…ppt` holds none
and its page 10 is squarely this class — the reference draws there the same 845×572 object it
draws on `Thailand17.ppt`'s page 8. **The flag keys on the observable signature; the metafile
count rides along as an attribution.** An earlier version filtered the page scan down to metafile
carriers and hid nearly half the list that way.

The scan also could not originally see a metafile in a binary document at all: a `.ppt` keeps its
pictures zlib-compressed inside Escher blip records, so a raw signature search finds nothing in a
file that plainly contains one. Inflating every plausible stream took the carrier count from 76
to 100.

## Two boundaries worth stating

**A flagged page does not excuse its document, and the two can point opposite ways.** Re-measure
before subtracting. This file's own worked example inverted once already — `UG.CAO.00133` was
recorded as 225 words short overall and later measured +245 over — before turning out to be a
false positive entirely.

**Eighty-three documents cannot be judged.** A per-page comparison is meaningless while the page
counts disagree, so those are an honest **unknown** rather than a pass. Fix their pagination
first, then re-run.

## The flagged pages

| Document | Page | ours | ref | excess | metafile |
|---|---|---|---|---|---|
| `words/batch-016/…/AFS-050-004-F2_0i.docx` | 3 | 419 | 53 | +366 | — |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 5 | 429 | 162 | +267 | 2/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 10 | 281 | 30 | +251 | 2/0 |
| `slides/batch-014/…/N2_E_Maestroni_Swarm_COP.pptx` | 7 | 307 | 102 | +205 | — |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 18 | 418 | 224 | +194 | 6/0 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 43 | 396 | 229 | +167 | 6/0 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 29 | 109 | 1 | +108 | 2/0 |
| `slides/batch-016/…/16 - UTM - (NASA).pptx` | 7 | 261 | 158 | +103 | 2/0 |
| `slides/batch-010/…/W3_Case_Study_of_a_Tsunami_Warning_Simulation_` | 10 | 102 | 9 | +93 | — |
| `slides/batch-014/…/Thailand17.ppt` | 8 | 102 | 9 | +93 | 6/0 |
| `words/batch-013/…/FO.FCTOA_.000129 Application for activities re` | 2 | 254 | 187 | +67 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 21 | 78 | 23 | +55 | 0/1 |
| `slides/batch-010/…/Fundamentals_Module_1_basics.ppt` | 6 | 70 | 20 | +50 | 1/0 |
| `words/batch-011/…/f2_registro_de_aprovacao_com_pbcs_EN.docx` | 1 | 230 | 181 | +49 | 0/3 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 28 | 53 | 5 | +48 | 0/1 |
| `words/batch-020/…/EHEST-SMS-Safety-Management-Manual-V2.docx` | 76 | 97 | 51 | +46 | 6/0 |
| `slides/batch-012/…/OnTrac_StarCertificationProgram-3Day.pptx` | 9 | 96 | 50 | +46 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 45 | 50 | 5 | +45 | 0/1 |
| `slides/batch-008/…/8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx` | 5 | 114 | 70 | +44 | 3/2 |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 5 | 93 | 54 | +39 | — |
| `words/batch-015/…/approvals-and-standardisation-organisation-app` | 6 | 161 | 123 | +38 | — |
| `sheets/batch-010/…/TOGAF9-Tool-ConfReqts-CSQ.xls` | 21 | 69 | 31 | +38 | — |
| `slides/batch-014/…/Structural Testing.pptx` | 19 | 37 | 5 | +32 | 2/0 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 44 | 34 | 4 | +30 | 0/1 |
| `slides/batch-014/…/WiGr_2021W_1_Angebot-Nachfrage-Elastizität-211` | 46 | 35 | 5 | +30 | 0/1 |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 7 | 94 | 64 | +30 | — |
| `slides/batch-017/…/Demick_JetBlue.pptx` | 4 | 108 | 79 | +29 | — |
| `slides/batch-016/…/FAAAIandtheArtandScienceofV&Vfinal.pptx` | 14 | 119 | 91 | +28 | 1/1 |
| `slides/batch-014/…/Intersil_Italy_CAN_Bus_Transceiver_Presentatio` | 30 | 130 | 103 | +27 | 6/0 |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 38 | 84 | 63 | +21 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 2 | 38 | 27 | +11 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 13 | 41 | 31 | +10 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 14 | 46 | 36 | +10 | — |
| `slides/batch-009/…/NWD-GLA-Community-Outreach-Day-Oct-2025.pptx` | 5 | 15 | 5 | +10 | — |
| `slides/batch-004/…/ws_prod-g-doc-Events-industrymeeting18112004-E` | 9 | 38 | 29 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 26 | 30 | 21 | +9 | — |
| `slides/batch-007/…/introduction_to_bea_tuxedo.ppt` | 39 | 38 | 30 | +8 | — |

## What is known about the mechanism, and what is not

**Established.** The rasterisation happens **upstream of PDF export**. The raster is not in the
file — two `.ppt`s were scanned through every inflated zlib stream, not just their raw bytes —
and it is not the PDF writer, since `implWriteBitmapEx` downsamples only under
`ReduceImageResolution` and the 300 dpi `FLOATTRANSPARENT` branch cannot yield the observed
66–265 dpi. `8_P-Pavese…pptx` slide 5 is a bare `p:pic` over an EMF with 791 `EXTTEXTOUTW`
records, no EMF+, no alpha, no raster-op and no bitmap, and the reference draws a 692×240 raster
with a soft mask.

**Not established.** Which LibreOffice path does it, and whether the metafile-carrying and
metafile-free cases share one. EMF+ is ruled out as the trigger by counter-example — `2014BSA`
slide 5's EMF *does* carry EMF+ and renders as text. `SELECTCLIPPATH` is the standout structural
difference between the two, but that is a correlation on two documents and is **unverified**.

Naming that path would let the flag become a rule rather than a list. Until then this is the
record.

## A second ceiling, with a different mechanism and a named cause

Rasterisation is not the only way the reference draws less than we do. The slides track's
largest single ink figure turned out to be half this, and unlike the rasterisation class the
mechanism is **named and verified** rather than open.

`slides/batch-012/pptx/NAS-Infrastructure-Roadmaps-v16.0.pptx` puts each of its data tables in
a `p:graphicFrame` wrapped in `mc:AlternateContent`:

```xml
<mc:Choice xmlns:v="urn:schemas-microsoft-com:vml" Requires="v">
  <p:oleObj r:id="rId3" progId="Excel.Sheet.12"><p:link/></p:oleObj>
</mc:Choice>
<mc:Fallback>
  <p:oleObj …><p:link/><p:pic>…<a:blip r:embed="rId4"/>…</p:pic></p:oleObj>
</mc:Fallback>
```

`rId3` is an *external* relationship to a SharePoint workbook. `rId4` is `image14.emf`, sitting
in the package, and it is a picture of the table's data.

`oox/source/core/contexthandler2.cxx:238-249` lists the namespaces LibreOffice will take a
`mc:Choice` for, and **`v` is on it** — so LibreOffice takes the Choice, gets a linked OLE
object with no local replacement picture, cannot reach the link, and draws nothing. We do not
claim VML, take the Fallback, and draw the EMF. Ours is the better output by any reading, and
the spec's rule — take the first Choice whose namespaces you understand — is on our side, since
we have no VML reader at all.

Measured, splitting the document's per-page ink by whether the page carries one:

| | pages | ink | major |
|---|---|---|---|
| carrying a `Requires="v"` `p:oleObj` | 24 | **152.12** | 24 |
| everything else | 113 | 73.21 | 42 |

The 152.12 did not move under either of this round's fixes — it is the same figure before and
after, which is what says it is a property of those pages rather than noise.

**Re-derived independently in round fourteen and it reproduces to the digit**: 152.12 on the 24
pages carrying one and 73.28 on the other 113, against the 152.12 and 73.21 recorded here. That
makes this one of the few claims on this track to survive an independent check with its *sentence*
intact as well as its number — the usual result is the reverse.

Round fourteen also took the other half apart, and it is **not** a second discrete defect waiting
to be found. Its worst pages carry none of `p:graphicFrame`, `a:tbl`, `a:blipFill`,
`a:pattFill`, `a:gradFill`, `dgm:relIds`, `a:prstTxWarp` or `a:outerShdw` in any
concentration, and the diff
report calls 40-50% of each one *"marks displaced or reshaped"*: a reflow spread thin over 113
pages at about 0.65 each, worst page 4.27. So the splitting method paid here once and has now been
run to the end on this document; the next instrument for what is left is the extraction
comparison, not more pixels.

Corpus-wide the pattern is small: ten decks have a slide with a `Requires="v"` choice around a
`p:oleObj`, and only NAS has it on more than four slides. So this is one document's ceiling
rather than a class to build a tool around — but it is 10% of the track's ink and it had been
recorded twice as "linked Excel OLE, known" without the number being split, which is what let
its other 216.29 sit unexamined for two rounds. **Split a big document's ink before believing
its attribution.**

## Sheets is nearly untouched by this

One flagged page on the whole track. That track's image problem is the opposite one, and note
that its headline example was also wrong: `apron-area.xls` was recorded as drawing 0 images
against the reference's 1670, and the census that produced it was counting placements of EMFs
that draw as vector content. The document was a full match all along, page-1 ink 1.09%. Treat the
rest of that census as suspect.
