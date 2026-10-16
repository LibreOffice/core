#!/usr/bin/env python3
"""Align our PDF text layer against the reference's, line by line, tolerantly.

Our glyph positions differ from the reference's by ~0.1-0.2pt, so keying on rounded
coordinates splits lines that are really the same line.  Lines are therefore grouped by
baseline and matched across sides within a tolerance, then their character sequences are
diffed.  For every glyph WE draw and the reference does not, the useful facts are:

  - is the whole line absent from the reference (a line clipped away entirely), or
  - is only part of a line absent (a line the clip edge cuts through: the reference keeps
    the glyphs whose ink rises above the edge and drops the rest), or
  - does the reference draw nothing near there at all (invented / duplicated text).

`inkTop` is the topmost ink y of the glyph; on a straddled line the kept glyphs have a
smaller inkTop than the dropped ones, which is the clip's fingerprint.
"""
import collections, difflib, pathlib
import pymupdf
import census as C

TOL = 1.2   # pt: how far two baselines may differ and still be the same line


def lines(pdf):
    doc = pymupdf.open(pdf)
    out = []
    try:
        for pno, p in enumerate(doc):
            for b in p.get_text('rawdict')['blocks']:
                if b['type'] != 0:
                    continue
                for ln in b['lines']:
                    gs = []
                    for sp in ln['spans']:
                        for c in sp['chars']:
                            if c['c'].isalnum():
                                gs.append((c['c'], c['bbox'][0], c['bbox'][1], sp['origin'][1]))
                    if gs:
                        out.append((pno, round(gs[0][3], 1), gs))
    finally:
        doc.close()
    # merge fragments that share a page and baseline (the reference splits a straddled
    # line into one span per surviving glyph run)
    merged = collections.defaultdict(list)
    for pno, base, gs in out:
        hit = None
        for (p2, b2) in merged:
            if p2 == pno and abs(b2 - base) <= TOL:
                hit = (p2, b2); break
        merged[hit or (pno, base)].extend(gs)
    return {k: sorted(v, key=lambda g: g[1]) for k, v in merged.items()}


def match(ourkeys, refkeys):
    """our line key -> ref line key (or None)."""
    m, used = {}, set()
    for k in ourkeys:
        best, bd = None, 1e9
        for r in refkeys:
            if r[0] != k[0] or r in used:
                continue
            d = abs(r[1] - k[1])
            if d <= TOL and d < bd:
                best, bd = r, d
        if best:
            used.add(best)
        m[k] = best
    return m


def main():
    here = pathlib.Path(__file__).parent
    rows = [l.split('\t') for l in (here / 'docs.tsv').read_text().split('\n') if l.strip()]
    for stem, _ in rows:
        ours = lines(C.only_pdf(str(here / 'ours' / stem)))
        ref = lines(C.only_pdf(str(here / 'ref' / stem)))
        m = match(sorted(ours), sorted(ref))
        gone, part, extra_tot = [], [], 0
        for k in sorted(ours):
            ot = ''.join(g[0] for g in ours[k])
            r = m[k]
            if r is None:
                gone.append((k, ot)); extra_tot += len(ot); continue
            rt = ''.join(g[0] for g in ref[r])
            if rt == ot:
                continue
            sm = difflib.SequenceMatcher(None, ot, rt, autojunk=False)
            dropped = ''.join(ot[i1:i2] for tag, i1, i2, _, _ in sm.get_opcodes()
                              if tag in ('delete', 'replace'))
            keptidx = [i for tag, i1, i2, _, _ in sm.get_opcodes() if tag == 'equal'
                       for i in range(i1, i2)]
            dropidx = [i for tag, i1, i2, _, _ in sm.get_opcodes()
                       if tag in ('delete', 'replace') for i in range(i1, i2)]
            ktop = min((ours[k][i][2] for i in keptidx), default=None)
            dtop = min((ours[k][i][2] for i in dropidx), default=None)
            if dropped:
                part.append((k, ot, dropped, ktop, dtop)); extra_tot += len(dropped)
        print('### %-52s surplus=%d  lines-absent=%d  lines-cut=%d'
              % (stem[:52], extra_tot, len(gone), len(part)))
        for k, t in gone[:8]:
            print('    ABSENT  base %7.1f  %r' % (k[1], t))
        for k, t, d, kt, dt in part[:8]:
            print('    CUT     base %7.1f  we=%r drop=%r  keptInkTop=%s dropInkTop=%s'
                  % (k[1], t, d, '%.1f' % kt if kt else '-', '%.1f' % dt if dt else '-'))
        print()


main()
