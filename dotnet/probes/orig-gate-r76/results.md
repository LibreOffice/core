# The original corpus at `f9e755b72` — a regression check across five merges

## Environment

    ours   = XRay.Office.Cli @ f9e755b72
    ref    = /usr/bin/soffice, LibreOffice 24.2.7.2, taken deliberately and not by accident
    corpus = /home/user/sample-files, 947 documents in their original spellings
    rule   = batch-check.sh of 2026-09-05

**Why 24.2.7.2 here and 26.2.4.2 next door.** `odf-gate-r76` measures fidelity, so it uses the
binary the tree is calibrated to. This run answers a different question -- *did five merges break
anything that used to work* -- and the answer is only meaningful if the reference is held fixed
against the banked baseline, which was taken against 24.2.7.2. The reference is now announced in
the run's own header, so which one a sweep used is a matter of record rather than of memory.

## The result

    TOTAL 947  MATCH 871  MISMATCH 76  REF-CANNOT-RENDER 0

Identical in every figure to the baseline banked at `97628b7fe`, before any of
`rtfshape`, `odpchart`, `odtframe`, `chartresid` or `odshdr` landed.

| track | match | of |
|---|---:|---:|
| `docx` | 257 | 272 |
| `pptx` | 243 | 251 |
| `xlsx` | 206 | 241 |
| `doc` | 58 | 66 |
| `xls` | 57 | 64 |
| `ppt` | 49 | 51 |
| `xlsm` | 1 | 2 |

## An equal total is not the same as an unchanged set

871 = 871 would also be produced by losing one row and gaining another, so the counts were
checked against the *set*. The banked row list at `97628b7fe` was not on disk after a container
restart; `/home/user/gate-2f47` was, an older commit scoring **860**. That makes the available
comparison span longer than the five merges rather than shorter:

    matched at 2f47 and not now: 0
    matched now and not at 2f47: 11

**The 860 are a strict subset of the 871.** No document that rendered correctly at the older
commit renders incorrectly now, over a span that contains every round since. That is a stronger
statement than the one this check set out to make, and it is the one worth recording.
