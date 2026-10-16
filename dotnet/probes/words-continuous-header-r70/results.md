# `hdss-bulletin-issue-285-25-june-2025.docx` — we draw a running header the reference draws nowhere

Measured 2026-09-07 in `/home/user/wt-wordsgap`, base `f4b8c3825`, reference
`/opt/libreoffice26.2/program/soffice` **26.2.4.2** with its Latin duplicates, its Latin Noto and its
`LiberationSansNarrow` aside. **Diagnosed and left unfixed** — the cause is named and reproduced with
a one-attribute mutation, and closing it is a pagination round of its own.

---

## The symptom, and what survives from the earlier reading

Round 67 recorded per-page word counts wandering both ways —
`240/237, 323/342, 397/407, 393/472, 436/400, 404/465, 353/430, 496/443, 385/180, 167/162` — and named
four candidates: (a) our line height is short, (b) the banner or header takes less vertical room in
ours, (c) paragraph space-before/after is smaller, (d) a keep or break rule.

It is **(b), and with the sign the other way**: the furniture takes *more* room in ours, because we
draw a running head the reference does not draw at all.

```
pages carrying "HDSS Bulletin Issue 285"    ours 2 3 4 5 6 7 8 9 10      ref26  (none)
```

Page 1 is not affected and agrees with the reference **line for line**: its last twelve text records
match to 0.01–0.10 pt, including the 16 pt heading at 166.65 against 166.64 and the last body line at
105.00 against 104.99. Page 2 is where it starts:

| | ours | ref26 |
|---|---|---|
| running head | `HDSS Bulletin Issue 285 2` at (42.55, **799.45**) | *nothing* |
| first body line | (42.55, **759.25**) | (42.65, **802.79**) |

**43.54 pt**, which is about three of that document's 14 pt lines — and it is the same on every page
from 2 to 10, which is the shape of counts that wander rather than drift: page 1 unaffected, every
later page short by whatever its own content lets it lose, re-synchronised at each heading that
starts a page.

*The other half of round 67's reading was already withdrawn there:* `OFFICIAL` is drawn, at
(275.32, 31.40) against (275.50, 31.29).

## The cause: the section that names the header is a `continuous` break

The file has three sections. Section 1 names `header1.xml`, which holds **one empty paragraph**.
Section 2 is `<w:type w:val="continuous"/>`, names `header2.xml` — *HDSS Bulletin Issue 285* and a
`PAGE` field — and carries `w:titlePg`. Section 3 is continuous and names nothing.

Three mutations of the real document, one attribute each, rendered both ways (`mutate.py`):

| variant | ours draws the head on | ref26 draws it on |
|---|---|---|
| *(unchanged)* | 2–10 | **none** |
| `no-titlepg` — drop `<w:titlePg/>` from section 2 | 2–10 | **none** |
| `nextpage` — drop `<w:type w:val="continuous"/>` from section 2 | 3–11 | **3–11** |
| `sect3-header` — give section 3 its own reference to the same part | 2–10 | **10 only** |

`no-titlepg` refutes `w:titlePg` outright. `nextpage` is the discriminator: changing nothing but the
break type makes the reference agree with us page for page. So **a `continuous` section break does
not bring its own running head into force**, and a header named on one is drawn on no page — a page
has one page style, a continuous break starts no page, and a running head is a property of the page
style.

`sect3-header` says the residual rule is not simply *"ignore a continuous section's furniture"*: the
last section's own reference does reach page 10. Whatever the exact rule in
`SectionPropertyMap::CloseSectionGroup`'s continuous branch is, it is not the one this tree
implements, which is *every section's furniture is its own from its first page*.

## Reach

**16 of 272 corpus DOCX** carry a `continuous` section break that names a header or a footer
(`census.py`; 19 carry a continuous break at all):

```
Pre-Application_Letter_of_Intent_Example.docx        easa-form-1.docx
mde087077~283.docx                                   Press release_EUREKA labels ITEA 3 Cluster.docx
195584360.docx                                       b053-19.docx
b050-19.docx                                         hdss-bulletin-issue-285-25-june-2025.docx
231164_SystemDesignDocument.docx                     Regulations Governing the Status…docx
JEMIT_Template.docx                                  Allegiant_Company_Profile_with_History.docx
Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.docx
t_TEMPforInvProgs.docx                               ABCD-SDE-23-00 - Avionic System Description…docx
report-template.docx
```

Two of them are already in round 67's ink ranking — `t_TEMPforInvProgs.docx` at 11.11 (#6) and
`ABCD-SDE-23-00` at 2.31 — so this is not one document's curiosity.

## Why it is left

The rule that replaces *"a section's furniture is its own"* has to be read out of
`SectionPropertyMap::CloseSectionGroup`'s continuous branch rather than guessed: the obvious
simplification, *a continuous section keeps the furniture in force*, is contradicted by
`sect3-header`, where the last section's own reference does reach a page. And the same break type
carries a `w:pgMar` that we also apply and the reference may not — page 2's body sits 39.1 pt down an
A4 sheet in the reference, which is neither section 1's `w:top` of 22.7 pt nor section 2's 70.9 —
so the geometry half has to be settled with it. That is a pagination round with a 16-document reach,
and it should be measured before it is written.

## Reproducing

```sh
python3 mutate.py /abs/variants
CLI=<tree>/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli
for f in /abs/variants/*.docx; do
  SOURCE_DATE_EPOCH=1700000000 $CLI render --quiet --outdir /abs/out/ours "$f"
  /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir /abs/out/ref26 "$f"
done
# then, per page: pdftotext -f N -l N <pdf> - | grep -c 'HDSS Bulletin Issue 285'
python3 census.py
```
