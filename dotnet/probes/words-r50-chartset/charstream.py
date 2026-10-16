#!/usr/bin/env python3
"""Whitespace-stripped charstream comparison, per COMMON.md §3.

Same characters + failing word count  = tokenisation ceiling.
Different characters                  = real content/layout defect.
"""
import subprocess, sys, os, unicodedata, difflib

def chars(pdf):
    t = subprocess.run(['pdftotext','-enc','UTF-8','-nopgbrk',pdf,'-'],
                       capture_output=True, text=True).stdout
    return ''.join(c for c in t if not c.isspace()), t

rows = []
for line in open('base/rows.tsv'):
    f = line.rstrip('\n').split('\t')
    if len(f) < 7 or 'words' not in f[6]:
        continue
    stem = os.path.basename(f[0]).rsplit('.',1)[0] + '__' + f[1] + '.pdf'
    o, ot = chars('base/ours/'+stem)
    r, rt = chars('base/ref/'+stem)
    same = o == r
    # normalised comparison: NFKC + drop the soft-hyphen/zero-width family
    def norm(s):
        s = unicodedata.normalize('NFKC', s)
        return ''.join(c for c in s if c not in '­​‌‍﻿')
    samen = norm(o) == norm(r)
    rows.append((os.path.basename(f[0])[:46], f[3], len(o), len(r), same, samen))
    if not samen:
        # what is in ref and not ours
        sm = difflib.SequenceMatcher(None, norm(o), norm(r), autojunk=False)
        miss = ''.join(norm(r)[j1:j2] for tag,i1,i2,j1,j2 in sm.get_opcodes() if tag in ('delete','replace','insert') and tag!='delete')
        extra= ''.join(norm(o)[i1:i2] for tag,i1,i2,j1,j2 in sm.get_opcodes() if tag in ('delete','replace'))
        rows[-1] = rows[-1] + (miss[:200], extra[:200])

print(f"{'document':46} {'words':11} {'ourChars':>8} {'refChars':>8}  same  sameNFKC")
for r in rows:
    print(f"{r[0]:46} {r[1]:11} {r[2]:8} {r[3]:8}  {str(r[4]):5} {str(r[5])}")
print()
for r in rows:
    if len(r) > 6:
        print(f"--- {r[0]}  words={r[1]}")
        print(f"    ONLY IN REF   : {r[6]!r}")
        print(f"    ONLY IN OURS  : {r[7]!r}")
