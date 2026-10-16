#!/usr/bin/env python3
"""Rank the words track by ink, which is the one measure the gate is blind to.

The gate is page count, alphanumerics within max(2 %, 15) and unembedded fonts, and a whole track
can be page-exact while its pages are visibly wrong -- a border, a fill, a bullet, a missing
arrowhead and a displaced block all add no character and no page.  So this scores every document
on summed unsigned per-page ink against 26.2.4.2 and prints the gate verdict beside it, because
the interesting rows are the ones that PASS and are still far out.

A MAJOR page is one whose |ink| exceeds one per cent of the page; a percentage summed over pages
is not comparable between a 5-page document and a 200-page one, so the page count and the per-page
mean are printed with it.

**Two columns of difference, because the cheap one is blind and biased.** `meanD` is
`|mean(ours) - mean(ref)|` per page, which is what this scorer first used; it cannot see ink that
merely *moved*, since a block drawn in the wrong place leaves the page's mean untouched, and it is
biased by colour quantisation (below). `pixelD` compares the two rasters pixel for pixel with a
one-level dead band, which sees displacement and ignores quantisation. Rank on `pixelD`.

**The dead band is not fastidiousness.** MuPDF resolves a colour literal one grey level lower
whenever the literal is at or below `v/255` exactly: measured, `0.502` (= 128.01/255) rasterises to
128 while `0.5019607843` and even the exact `0.50196078431372549` both rasterise to **127**, and
`0.851` gives 217 against `0.8509803922`'s 216. This tree writes colour components to four decimals
and 26.2.4.2 writes ten, so **every shaded area the two render is one grey level apart although
both name the same colour**. On `Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification` that
alone was 59.56 of `meanD` over 148 pages — the worst *passing* document on the track, for a
difference of one part in 255 that no reader can see and neither renderer is wrong about.

**And the `aligned` column is not optional reading.** Ink is compared page against page, so a
document that loses or gains one page early reports every page after it as different -- one defect
counted two hundred times. That has now misread three documents in this project: `02_mcar` at
881.67 (one extra page of table of contents, `probes/tocwrap-r175`) and both FAA Holdover Tables at
321.27 and 93.02 (one word-parity page each, `probes/holdover-r177`). `aligned` is the fraction of
our pages whose text matches the reference's at the *same* index; well below 1.00 means the ink
figure is measuring an offset and the rank is meaningless until that is explained.
"""
import difflib, hashlib, pathlib, sys


def key(path):
    return hashlib.md5(str(path).encode()).hexdigest()[:16]


DEAD_BAND = 1
"""Grey levels of per-pixel difference to ignore, which is the colour-quantisation bias."""


def load(pdf):
    import fitz
    with fitz.open(pdf) as doc:
        text = "".join(page.get_text() for page in doc)
        ink, signatures, rasters = [], [], []
        for page in doc:
            pixmap = page.get_pixmap(dpi=36, colorspace=fitz.csGRAY)
            data = pixmap.samples
            ink.append(sum(255 - b for b in data) / (255.0 * len(data)))
            signatures.append("".join(c for c in page.get_text() if c.isalnum())[:400])
            rasters.append(data)
        return doc.page_count, sum(c.isalnum() for c in text), ink, signatures, rasters


def pixelwise(ours, reference):
    """Per-page mean |difference| past the dead band, as a percentage, one entry per reference page.

    Unlike a difference of two page means this sees ink that *moved*: a block drawn in the wrong
    place cancels out of a mean and does not cancel out of this.
    """
    out = []
    for index in range(len(reference)):
        theirs = reference[index]
        mine = ours[index] if index < len(ours) else None
        if mine is None or len(mine) != len(theirs):
            out.append(100.0 if mine is None else 0.0)
            continue
        total = 0
        for a, b in zip(mine, theirs):
            gap = a - b if a > b else b - a
            if gap > DEAD_BAND:
                total += gap - DEAD_BAND
        out.append(total / (255.0 * len(theirs)) * 100)
    return out


def aligned(ours, reference):
    """The fraction of our pages whose text matches the reference's at the same index.

    `quick_ratio` rather than `ratio` because this runs over every page of a 727-page document and
    the question is only whether the two are the same page, not how nearly.
    """
    pages = min(len(ours), len(reference))
    if pages == 0:
        return 0.0
    same = sum(
        1 for index in range(pages)
        if difflib.SequenceMatcher(None, ours[index], reference[index]).quick_ratio() > 0.9)
    return same / max(len(reference), 1)


def verdict(ours, reference):
    if ours[0] != reference[0]:
        return "pages"
    d = abs(ours[1] - reference[1])
    return "match" if not (d > reference[1] * 0.02 and d > 15) else "glyphs"


def main():
    root = pathlib.Path(sys.argv[1])
    rows = []
    for line in open(sys.argv[2]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        a = next((root / "ours" / key(path)).glob("*.pdf"), None)
        b = next((root / "ref" / key(path)).glob("*.pdf"), None)
        if a is None or b is None:
            print(f"# missing {'ours' if a is None else 'ref'}\t{path.name}", file=sys.stderr)
            continue
        ours, reference = load(a), load(b)
        pages = reference[0]
        pairs = list(zip(ours[2] + [0.0] * pages, reference[2]))[:pages]
        mean = [abs(x - y) * 100 for x, y in pairs]
        pixel = pixelwise(ours[4], reference[4])
        rows.append((sum(pixel), sum(mean), sum(1 for v in pixel if v > 1.0), pages,
                     verdict(ours, reference), ours[1], reference[1],
                     aligned(ours[3], reference[3]), path))
    rows.sort(reverse=True)
    print("pixelD\tperPage\tmeanD\tMAJOR\tpages\taligned\tverdict\toursG\trefG\tdocument")
    for total, mean, major, pages, v, og, rg, align, path in rows:
        print(f"{total:.2f}\t{total / max(pages, 1):.3f}\t{mean:.2f}\t{major}\t{pages}\t"
              f"{align:.2f}\t{v}\t{og}\t{rg}\t{path.name}")
    passing = [r for r in rows if r[4] == "match"]
    print(f"\n{len(rows)} scored, {len(passing)} pass the gate", file=sys.stderr)
    print("the ten worst PASSING documents by pixelD:", file=sys.stderr)
    for total, mean, major, pages, v, og, rg, align, path in passing[:10]:
        print(f"  {total:8.2f}  {total / max(pages,1):6.3f}/page  meanD {mean:7.2f}  {major:>3} MAJOR"
              f"  {pages:>4}p  aligned {align:.2f}  {path.name[:48]}", file=sys.stderr)
    drifted = [r for r in rows if r[7] < 0.9 and r[0] > 1.0]
    print(f"\n{len(drifted)} documents whose ink is measuring a page offset rather than a defect:",
          file=sys.stderr)
    for total, mean, major, pages, v, og, rg, align, path in drifted[:12]:
        print(f"  {total:8.2f}  aligned {align:.2f}  {path.name[:62]}", file=sys.stderr)


if __name__ == "__main__":
    main()
