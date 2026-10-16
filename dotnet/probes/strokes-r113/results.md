# Round 113 — stroke weight across the corpus, and a first census that was wrong

## Why

The blind reading of `Thailand17` page 11 (`probes/vision-r112`) found that this tree strokes a
`.ppt` table's rules 2.4–2.5× heavier than 26.2.4.2. That was seated as **O55** with its reach
explicitly unmeasured. Both gate legs are banked for all 947 documents, so the reach could be
measured without rendering anything.

## The first census was confounded, and its numbers must not be quoted

The obvious measure is *stroke ink* — Σ(width × item count) per document — ours against the
reference. Over 818 documents that produced an arresting table: **49** documents where we stroke
≥1.5× heavier, three of them at 167× and 190×, and **317** where we stroke *lighter*, many at
exactly zero.

It measures the wrong thing. A renderer may draw a rule as a thin **fill** rather than a stroke,
and this project already knows 26.2.4.2 writes an underline as a zero-height non-fill rectangle.
Zero stroke ink against a reference full of it means the two sides chose different primitives, not
that one drew nothing. `Thailand17` was only a sound reading because both sides used strokes *and
the same item counts*, 13 and 4.

## The like-for-like census

Restricted to documents where **both sides emit ≥10 stroked items and their counts agree within
25 %** — that is, both chose strokes for the same population — comparing the **mean stroke width**:

**363 of 947 documents are comparable. Median ratio 1.000.**

| | documents |
|---|---:|
| ours lighter by >10 % | 19 |
| within 10 % | **287** |
| ours heavier by 10–50 % | 28 |
| **ours heavier by ≥1.5×** | **29** |

`like-for-like.tsv`.

**The class is spreadsheets, not `.ppt` tables**, and the base rates say so:

| format | ≥1.5× heavier | comparable |
|---|---:|---:|
| `.xlsx` | **18** | 122 |
| `.xls` | **9** | 41 |
| `.ppt` | 2 | 10 |
| `.pptx` | 0 | 61 |
| `.docx` | 0 | 111 |
| `.doc` | 0 | 18 |

27 of the 29 are spreadsheets. The words track is **clean, 0 of 129**, and so are OOXML slides.

**And our width is a constant.** 16 of the 29 have a mean of exactly **0.75 pt** — which is one
device pixel at 96 dpi — against a reference median of 0.38. Verified at width level on the worst,
`079_Org_charts_visual`: ours draws **all 157 strokes at 0.75 pt**; the reference draws **127 at
0.15, 28 at 0.173 and 4 at 0.295**. That is a flat default standing in for a resolved set, not a
conversion error, and it is a different defect from the one O55 describes.

## Limits of this measurement, stated

- **Mean width per document is coarse.** A document mixing heavy and hairline borders can average
  to agreement while both are wrong. The 287 "within 10 %" are evidence about the bulk, not a
  guarantee per rule.
- **The item-count guard excludes the interesting middle.** 584 of 947 documents are *not*
  comparable, most because one side strokes where the other fills. Those may hold the same defect
  in a form this instrument cannot see.
- Eight pages per document are sampled, not all.
- Ratios are of means, so a document with few strokes moves easily; the guard's ≥10 floor limits
  but does not remove this.

## What this changes

**O55 is one witness of a wider class.** Seated as **O56**: a spreadsheet's cell borders are
stroked at a flat 0.75 pt where 26.2.4.2 resolves the stated width, on **27 of 163 comparable
spreadsheets**, with the words track clean. `Thailand17` is in the comparable set at 1.93× over
the whole document (11 items each side) — the 2.37–2.50 in `probes/vision-r112` is page 11 alone.
