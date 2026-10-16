# words-r51-flies — prediction

Committed **before** anything was changed and before any post-change rendering.

Environment: LibreOffice **26.2.4.2 620(Build:2)**, `fc-match "DejaVu Sans"` → `DejaVuSans.ttf`,
`fc-match Calibri` → `Carlito-Regular.ttf`, corpus `/c/sandbox/workdir/sample-files` at `5fd4b17`,
worktree `wt-words-r50` on branch `wt-words-r51`, base `6798de946ce`,
`SOURCE_DATE_EPOCH=1700000000`, `TZ=UTC`.

## Baseline, reproduced before anything was believed

`batch-check.sh … 'words/*' … 8` → `TOTAL 355  MATCH 324  MISMATCH 31`.

**That total is wrong and the brief said it would be.** 355 rows for 337 documents: 18 of them are
the upper-case alias directory entries `look.py` has materialised on this case-insensitive mount over
previous rounds. Scored against `MANIFEST.tsv`'s path list instead:

- **309 of 337 match**, 28 open;
- **0 disagreements with the manifest's status column, document for document.**

Baseline reproduced.

## What the round's first item turned out to be — stated here because it changes the prediction

The brief's item 1 is "the wrap": *a body fly takes Writer's parallel surround, we place flies
correctly and never push text clear of them, and `AFS-050-004-F2_0i` page 3 is the witness — 364
words against the reference's 53, 318 extra tokens and 0 missing.*

The 318 extra tokens reproduce exactly. **They are not text drawn through a fly. They are the same
positioned table drawn twice.**

Measured before changing anything:

- Our pages against the reference's: `505 368 364 412 363 372 258 60` against
  `505 361 53 412 323 383 254 93`. A multiset diff of the whole document gives **318 tokens only in
  ours and 0 only in the reference**, and **not one of the 318 is a string the reference never
  draws** — every one is a repeat.
- Our page 2 against our page 3, token multisets: **5 tokens only on page 2**
  (`IASA Checklist Section Assignment`, and the page number), **1 only on page 3** (its page
  number). The two pages are the same table.
- Authored variant: the same document with all four `w:tblpPr` elements deleted and nothing else
  changed renders **8 pages and 2384 words** — the reference's own raw total to the token — with
  page 3 falling from 364 to 46.
- Traced through `Paginator.Fill`: block 23 is placed **twice**, once by the in-flow arm on page 2
  (`from=0 to=36`, `placed=True`) and once by `PlaceFloatedTable` on page 3. No
  `MoveTrailingGroupToNextPage` fires between them.

The mechanism follows from the trace. The table has 37 rows; the flow arm places rows 0–35 on
page 2, finds `lineIndex = 36 < 37`, and emits the page **leaving `paragraphIndex` on the table** —
which is how every split table continues. On the next page the table arm runs again, and
`PlaceFloatedTable` is asked before the continuation is placed. It never looks at `lineIndex`, so it
floats the table **from row 0**, entire. The first part is already on page 2.

So the round's first fix is not the wrap.

## The two changes

### A. A positioned table's continuation must not re-float the whole table

`PlaceFloatedTable` is offered every visit to the table arm. Guard it to the table's *first* part —
`lineIndex == 0 && rowDrawn == Length.Zero` — which is exactly the guard the `StartsNewPage` test
three lines above it already carries, and for the same reason: a continuation is by construction not
the table's start.

### B. A `v:group` inside a `v:group` is dropped, and everything inside it with it

`DocxVmlFrames.Group` walks a group's members and has

```csharp
if (member.Name.LocalName is "group") continue;   // nested groups: not measured, not guessed
```

so a nested group and its whole subtree are discarded. Recurse instead: a nested group's
`left/top/width/height` are bare numbers in the parent's `coordsize` space and resolve to a real
rectangle by exactly the arithmetic the flat case already does, and that rectangle is then the
child's own origin and extent.

This is the discriminator `068`'s round-50 blind reviewer proposed unprompted — *"if every rendered
item is at nesting depth ≤ 1 and every missing one is ≥ 2, it is a recursion limit rather than a
fill problem"* — and it is what the source says. A second blind reviewer this round, given only the
image, reported independently: *"the surviving items are exactly the top two levels of the tree, in
tree order, not a random scatter and not a partial-column truncation … there is no piling up and no
overlapping; the failure is omission."*

## Documents expected to change

| document | before | expected after | confidence |
|---|---|---|---|
| `AFS-050-004-F2_0i.docx` (`done-014`) | `words` 8/8, 2503/2228 | match | high — the `tblpPr`-stripped variant already renders the reference's exact raw total |
| `068_Work_Breakdown_Structure_Template_Green_Theme` (`chartset-011`) | `words` 1/1, 19/86 | match | high — the words inside nested groups number **exactly 67**, and 19 + 67 = 86 |

**Predicted verdict movement: +2, to 311 of 337.**

Two further documents *may* move and are deliberately **not** counted, because I have no measurement
that says they will:

- `AW-104D-RVSM-Aircraft-Approval-Checklist.pdf.docx` — `words` 141/135, +6 against a band of 3, one
  `w:tblpPr`, two pages. A duplicated small table would explain it exactly.
- `ABCD-FE-01-00 Flight Envelope` — `pages,words` 14/15 and 3837/3720, +117, one `w:tblpPr`.

## Reach censuses, and what they cannot see

**Fix A.** 42 of the 337 words documents hold at least one `w:tblpPr`, across 271 zip-readable ones.
Of those 42, nine currently differ from the reference in word count while still matching the gate —
`ESPN-R − MCF − RA − Ed1` −39, `5709.16 ch.40_mgfinal` +35, `ABCD-WB-08-00` +27,
`FAA 2025-26 Holdover Tables` +17, `33004` +10, `461249` +2, `PES-Technical-Report-Template` +2,
`JEMIT_Template` +1, `draft-variation-notice-airbus` −1 — and those nine are the named regression
surface. The other 33 are at ±0 or are already failing.

**Fix B.** Exactly **one** words document holds a `v:group` whose direct child is another `v:group`
and which is *not* inside an `mc:Fallback`: `068`, with 5 such nestings, 35 shapes beneath them and
67 words. `065` and `069`, the other two Work Breakdown templates, hold **no** nested group at all —
the brief's grouping of the three is wrong on this axis. `056`, `057`, `025`, `030`, `008` and `071`
each hold 19–40 nested VML groups, but every one of them is inside an `mc:Fallback`, and an authored
variant settles that we do not read those: `056` with its whole `mc:Fallback` deleted renders **24
words, byte-identical in count to the unmodified document**, so we take the `mc:Choice` DrawingML.
**Fix B's regression surface inside words is empty.**

### What these censuses cannot see

- **Whether a positioned table splits is a layout fact, not a markup fact.** The 42-document
  `w:tblpPr` census bounds fix A from above and cannot say which of the 42 split across a page. Only
  the sweep can, and that is why the sweep is the whole track and not the two named documents.
- **Only the 271 zip-readable words documents were censused.** The 66 `.doc`/`.rtf` files have their
  own positioned-table paths (`Ww8Frames`, `RtfDocumentReader.Tables`) and their own drawing readers,
  and neither census reaches them. `absrc-pac-01-info-note-en.doc` is an open `pagination` failure.
- **Nor the other two families.** Both changed files are inside `XRay.Office.WordProcessing`, so no
  shared layer is touched — but if that stops being true the census must be redone.
- **A frame that is emitted is also an obstacle.** Fix B emits 35 new frames on `068`. Anchored
  frames narrow and push the lines of the paragraphs they overlap, so a document can move without
  its word count moving, and no gate column shows that.
- **Over-shoot is not excluded.** 19 + 67 = 86 assumes every nested box's text is drawn once and
  nothing already drawn is drawn twice. If a nested group's members are already reached by another
  route, `068` overshoots 86 rather than reaching it.
- **The band is `max(2%, 3)`, an AND.** `AW-104D` at 141/135 needs to lose all 6 of its surplus and
  not 3 of them.

## What is deliberately not being done

- **The wrap itself.** It has no witness left in this corpus: the only measurement offered for it was
  `AFS` page 3, and that is the duplication above. `RunsIntoTheFly` already refuses to float a table
  the following flow would run into, which produces the reference's page counts on all ten graph
  papers and five timelines. Implementing a real parallel surround would change every one of those
  fifteen with nothing measured asking for it.
- **A positioned table taller than its column is still stacked** rather than split in a fly.
- **The Carlito advance accumulation (`metrics-001`)**, per the brief.
