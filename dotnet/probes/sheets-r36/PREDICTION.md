# Round 36 — prediction, committed before the sweep

Written at commit `2d2445577`, with the whole-track sweep not yet started. The change is
glyph fallback in the spreadsheet cell text path.

## What I predict

**Byte reach on the sheets track: 8 to 30 of 171 renderings changed.**

Reasoning, and why it is a guess rather than a measurement. A census of the reference PDFs
against ours found **4** documents where LibreOffice embeds a face we do not — two
`WenQuanYiZenHei`, one `Unifont`, one `NotoColorEmoji`. That is a *lower* bound rather than a
ceiling, and unusually so: the census can only see a fallback whose chosen face is one we do not
already embed for some other reason, and it is blind to a document where both sides fall back to
a face already in the file. It also cannot see a fallback that changes a *width* without changing
the set of faces — which is most of the point of the change.

Last round's prediction was low by two to five times on exactly this kind of reasoning, so the
band above is widened deliberately and I expect to land in its upper half.

**Verdicts: 1 to 3.** `seihon_zassi_kikou_20221215.xlsx` is already measured as
83/84 → 84/84 with 0 of 5159 rows disagreeing with LibreOffice's own `style:row-height`, so one
is close to certain. `aircraft_analysis_2016-04-27.xls` is the reference's `Unifont` document and
still reads 44/46 with the change in, so it is *not* one of them. The other two are speculative.

**Words and slides: 0 documents changed.** Nothing outside `Paperless.Spreadsheets` passes the
new `ParagraphLayouter` shaper parameter and `MeasuredParagraph` is untouched, so the two tracks
take the identical code path. This is a prediction of zero and will be measured rather than
asserted, because "additive by construction" is exactly the claim this project has been wrong
about before.

## What would refute the change rather than the prediction

A document whose page count moves *away* from the reference. The fallback face we pick is not
always the one LibreOffice picks — we take LibreOffice's own hard-coded generic list and then
anything installed, deterministically by name, where LibreOffice asks fontconfig with the missing
character as a charset. On `seihon` that difference costs nothing (we choose `IPAGothic` and
`WenQuanYiZenHei` where the reference chooses `WenQuanYiZenHei` alone, and every row height comes
out exact), but a face whose ideograph advance is not one em would show up as a wrong break.
