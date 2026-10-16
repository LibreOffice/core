#!/usr/bin/env python3
"""Score a batch-check rows.tsv the way the words track's scoreboard does."""
import sys

rows = [l.rstrip("\n").split("\t") for l in open(sys.argv[1]) if l.strip()]
match = pe = exact = we = 0
for r in rows:
    ours_p, ref_p = (int(x) for x in r[2].split("/"))
    ours_w, ref_w = (int(x) for x in r[3].split("/"))
    if r[6] == "match":
        match += 1
    pe += abs(ours_p - ref_p)
    if ours_p == ref_p:
        exact += 1
    we += abs(ours_w - ref_w)
print(f"rows {len(rows)}  match {match}  |page err| {pe}  exact pages {exact}  |word err| {we}")
