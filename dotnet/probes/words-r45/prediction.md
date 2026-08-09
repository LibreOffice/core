# Words round 45 — prediction, committed before anything is rendered post-change

Baseline reproduced exactly at `45fea26c2` (`baseline.tsv`, 200 rows, no duplicate paths):
155/200 match, absolute page error 77, 165 exactly-correct page counts, absolute word error 6605.

## The change

Proportional line spacing adds `(prop − 100)%` of the line's **text** height to the line height,
rather than scaling the line. It differs from what we did only where an as-character object — an
inline picture, an inline shape — has made a line taller than its text, and only at or above 100%
(below that Writer scales the whole line and we already agreed).

## What the census counted, and over what

Two censuses, and between them they cover **all 200 documents of the track**, which is unusual
here and worth saying plainly:

| | reads | documents | carriers | paragraphs |
|---|---|---:|---:|---:|
| `inline-object-spacing-census.py` | `word/*.xml` in the zip | 134 DOCX | **1** | 1 |
| `inline-object-spacing-census-doc.py` | LibreOffice's own flat-ODF export | 66 `.doc` | **3** | 10 |

**Both are ceilings, not reaches.** Each counts a paragraph that states proportional spacing above
100% *and* holds an as-character object. Neither tests the third condition, which is the one that
actually makes the two heights differ: the object has to be **taller than the paragraph's text**. A
logo smaller than the line changes nothing at all, and a census that resolved that would have to
compare each picture's declared extent against its run's resolved font.

The four named documents and their current verdicts:

| document | now | why it is in the list |
|---|---|---|
| `1257259179492_2007_TPPT_102_Supporting_Doc_2.doc` | `pages` 10/9 | 4 paragraphs; a 214.5 pt picture at 150% |
| `1228841571067_2009_TPPT_13__2007_TPPT_102__R.doc` | `pages` 13/12 | 5 paragraphs, same house style |
| `Reid.doc` | **`match`** 4/4 | 1 paragraph — a document at risk, not a document to gain |
| `t_TEMPforInvProgs.docx` | **`match`** 26/26 | 1 paragraph — likewise |

## Predictions

| | baseline | predicted after |
|---|---:|---|
| renderings changed | — | **2–4** of 200 |
| documents matching | 155 | **156–157** |
| absolute page error | 77 | **75–77** |
| exactly-correct page counts | 165 | **166–167** |
| absolute word error | 6605 | **6590–6610** |

The reasoning behind each, so a miss can be attributed rather than excused:

- **Two renderings are certain** — the two TPPT `.doc`, both measured by hand before this was
  written. `1257259179492` renders 9 pages against the reference's 9 with the change in, from 10,
  and its `Introduction` heading lands at 476.76 against the reference's 476.91.
- **Two are conditional** on the picture being taller than the text, which the census cannot see.
  Both currently match, so they are the downside risk rather than the upside.
- **Word error should move a little and downwards.** Losing a page removes that page's running head
  and footer from `wc -w` and removes no body text: `1257259179492` is 2159 against 2162 now and
  should land near 2158; `1228841571067` is 2811 against 2803 and should fall towards it. Nothing in
  the change can move body text.
- **At most two verdicts.** `1228841571067` is the second, and it is less certain than the first: it
  is a page long for the same reason as far as the census can tell, but it was not rendered
  post-change before this file was written.

## What would make this wrong

- `Reid.doc` or `t_TEMPforInvProgs.docx` regressing would show up as matching 154–155 and is the
  outcome that says the rule is right in shape and wrong somewhere in its edges.
- A reach above 4 would mean a census over both formats missed a carrier, which after the OOXML and
  flat-ODF passes would be a real surprise rather than the usual grep shortfall — it is the one
  number here that is a genuine ceiling rather than an estimate.
