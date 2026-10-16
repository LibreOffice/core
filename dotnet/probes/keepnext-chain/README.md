# The keep-with-next chain fires; it is simply not triggered at the break

`CRIF - Spécification technique - Socle applicatif.docx` renders 28 pages
against 29 with words at 6590/6618. Pages 5 to 19 match word for word. On page
20 the reference keeps **18 lines** and leaves a **487.4 pt blank band** to the
footer; we fill the page with 48.

Not a figure: `pdfimages` counts 2 images on both sides for every page from 17
to 23, and rasterising the reference's band at 60 dpi finds 9 dark pixels in it.
The band is empty, so the reference is breaking early on purpose.

The document is saturated with `w:keepNext` — **231 of its 611 paragraphs**,
with a longest consecutive run of **42**, which is far taller than a page. The
paragraphs either side of the reference's break both carry it. A previous round
already fixed the *don't split a keep-with-next paragraph* half on this same
document; `Paginator.cs`'s remark names it.

## The hypothesis, and its refutation

`MoveTrailingGroupToNextPage` walks the chain back transitively and then bails:

```csharp
if (first <= firstOnPage) return;   // the whole page cannot be moved
```

With a 42-paragraph chain that guard looked like the obvious culprit — a chain
spanning the page would reach the top and we would keep everything.

**It never fires.** A temporary `XRAY_OFFICE_TRACE_KEEP` instrument, since removed,
gives `trace.log` beside this file. Every call moves:

```
KEEP  firstOnPage=213 last=242 chainFirst=226 lines=44  -> MOVED 30 lines from 226
KEEP  firstOnPage=226 last=251 chainFirst=246 lines=41  -> MOVED  8 lines from 246
KEEP  firstOnPage=246 last=266 chainFirst=266 lines=39  -> MOVED  1 line  from 266
KEEP  firstOnPage=289 last=319 chainFirst=319 lines=41  -> MOVED  1 line  from 319
```

Three identical repetitions, because pagination runs three passes to resolve
fields.

## What that leaves

**Four moves in a document with 231 keep-with-next paragraphs, and none of them
near the reference's page-20 break.** The machinery works and is simply not
triggered there. The two triggers are `allowed <= 0` for a successor at line 0
whose predecessor keeps with it, and `!FirstLineFits(next, …)` after placing a
keep-with-next paragraph. Neither fires, so at that point the successor both
fits its first line and fits whole in the room we have — and the question is why
the reference disagrees about the room or about the successor's height.

## A trap that cost time here

The trace's paragraph numbers are **block indices**, in which a whole table is
one block. They are *not* the XML `<w:p>` ordinal. Mapping the reference's break
to XML paragraphs 345 and 347 and then comparing that against the trace's 319 is
meaningless, and it was done before being noticed. A next attempt should make the
paginator print the paragraph's first words rather than an index.

## Next

Print the successor's measured height and the room remaining at each candidate
break on page 20, on both sides, instead of reasoning about which trigger ought
to fire. Two hypotheses have now died on this document; a third guess is worth
less than one measurement.

Also untouched, and a different divergence on the same document: our page 3
starts with `SOMMAIRE` where the reference's starts with the running header, and
the reference's pages 3 and 4 hold 149 and 71 words against our 160 and 56.
