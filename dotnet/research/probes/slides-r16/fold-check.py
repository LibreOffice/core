#!/usr/bin/env python3
"""Does the reference's *single* rounding of the two spacing factors explain the fixture?

`slide-autofit-grid.pptx` states `a:lnSpc/a:spcPct val="80000"` and `a:normAutofit`, so a
line height passes through two factors: the paragraph's stated 0.8 and whatever spacing scale
the fit search settles on. EditEngine multiplies both into one product and rounds it once —
`nHeight = fround(pLine->GetHeight() * fProportionalScale * fSpacingFactor)`
(editeng/source/editeng/impedit3.cxx:1568, 24.2.7). We round twice.

This checks the arithmetic against the six pitches read out of the reference's own PDF, using
only the chosen em size the test already asserts. No rendering involved.
"""
MM100_PER_PT = 2540.0 / 72.0

# box height pt, chosen em in mm100 (asserted exactly by the test), reference pitch in pt
CASES = [
    (90, 459, 10.006),
    (110, 494, 12.103),
    (135, 600, 14.683),
    (150, 600, 16.327),
    (175, 670, 14.598),
    (200, 670, 16.412),
]

STATED = 0.8                       # a:lnSpc/a:spcPct val="80000"
FIT_CANDIDATES = [1.0, 0.9, 0.8]   # the search's spacing ladder


def fround(x):
    return int(x + 0.5)


print(f'{"box":>5} {"em":>5} {"natural":>8} {"ref pt":>8} {"fit":>5} '
      f'{"one round":>10} {"two rounds":>11} {"twips-ish":>10}')

exact_folded = exact_split = 0
for box, em_mm100, ref_pt in CASES:
    natural = int(em_mm100 * 1.2 + 0.5)        # EditEngine's font-independent line height

    # Which spacing scale the fit chose is recoverable: it is the ladder rung whose single
    # rounded product lands on the reference's own pitch.
    best = None
    for fit in FIT_CANDIDATES:
        folded = fround(natural * STATED * fit)
        if best is None or abs(folded / MM100_PER_PT - ref_pt) < abs(best[1] / MM100_PER_PT - ref_pt):
            best = (fit, folded)
    fit, folded = best

    # What we do instead: round the stated proportion, then scale by the fit without
    # quantising to the draw layer's unit at all (Spaced works in EMU).
    split = fround(natural * STATED) * fit

    print(f'{box:5d} {em_mm100:5d} {natural:8d} {ref_pt:8.3f} {fit:5.2f} '
          f'{folded / MM100_PER_PT:10.4f} {split / MM100_PER_PT:11.4f}')

    if abs(folded / MM100_PER_PT - ref_pt) < 0.001:
        exact_folded += 1
    if abs(split / MM100_PER_PT - ref_pt) < 0.001:
        exact_split += 1

print(f'\nexact to a thousandth of a point: one rounding {exact_folded}/6, '
      f'two roundings {exact_split}/6')
