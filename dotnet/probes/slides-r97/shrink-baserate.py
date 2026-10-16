#!/usr/bin/env python3
"""The base rate behind `shrink-census.txt`, which is the number that census is worth nothing without.

`shrink-census.txt` says 16 of the 19 dominant-size pages carry a `style:shrink-to-fit` shape in
26.2.4.2's own flat-ODP view, and all 13 of the pages whose alphanumeric counts agree do. That
reads as evidence only against how often a page that does NOT differ carries one. This measures
that, over the same 51 documents and the same flat-ODP exports, resolving `style:parent-style-name`
and looking at `draw:style-name`, `presentation:style-name` and `draw:text-style-name` alike --
which is what reproduces `shrink-census.txt` exactly.

    shrink-baserate.py <fodp-dir> <sizes-ref.tsv> <sizes-head.tsv>
"""
import re, sys, glob, os

def resolver(x):
    own, parent = {}, {}
    for m in re.finditer(r'<style:style\b([^>]*)(/>|>((?:(?!</style:style>).)*)</style:style>)', x, re.S):
        a, body = m.group(1), m.group(3) or ''
        n = re.search(r'style:name="([^"]+)"', a)
        if not n: continue
        own[n.group(1)] = 'style:shrink-to-fit="true"' in body
        p = re.search(r'style:parent-style-name="([^"]+)"', a)
        if p: parent[n.group(1)] = p.group(1)
    def res(name):
        seen = set()
        while name and name not in seen:
            seen.add(name)
            if own.get(name): return True
            name = parent.get(name)
        return False
    return res

def pages(fodpdir):
    out = {}
    for path in sorted(glob.glob(os.path.join(fodpdir, '*.fodp'))):
        x = open(path, encoding='utf-8', errors='replace').read()
        res = resolver(x)
        doc = os.path.basename(path)[:-5] + '__ppt'
        for i, p in enumerate(re.findall(r'<draw:page\b.*?</draw:page>', x, re.S), 1):
            used = (set(re.findall(r'draw:style-name="([^"]+)"', p))
                    | set(re.findall(r'presentation:style-name="([^"]+)"', p))
                    | set(re.findall(r'draw:text-style-name="([^"]+)"', p)))
            out[(doc, i)] = any(res(u) for u in used)
    return out

def sizes(path):
    d = {}
    for line in open(path):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 4: d[(f[0], int(f[1]))] = (float(f[2]), int(f[3]))
    return d

def main(fodpdir, refp, headp):
    sf = pages(fodpdir); ref = sizes(refp); head = sizes(headp)
    common = [k for k in ref if k in head and k in sf]
    diff = {k for k in common if abs(head[k][0] - ref[k][0]) > 0.15}
    same = [k for k in diff if head[k][1] == ref[k][1]]
    agree = [k for k in common if k not in diff]
    texty = [k for k in agree if ref[k][1] >= 30]
    def rate(ks, label):
        h = sum(1 for k in ks if sf[k])
        print(f'{label:44s} {h:5d} / {len(ks):5d} = {100*h/len(ks):5.1f}%')
    print(f'{len(common)} scored pages present in both the census and the flat-ODP export\n')
    rate(common, 'every scored page')
    rate(agree, 'pages whose dominant size AGREES')
    rate(texty, '  ... of those, with >= 30 alnum chars')
    rate(sorted(diff), 'the 19 that differ')
    rate(same, 'the 13 whose alnum counts also agree')
    p = 1.0
    n = sum(1 for k in texty if sf[k]) / len(texty)
    for _ in same: p *= n
    print(f'\n13 of 13 under the agreeing-page rate of {100*n:.1f}%: p = {p:.3f}')
    print('So "16 of 19" is AT the base rate and carries no information, and "13 of 13" is')
    print('suggestive at best. The census does not establish the autofit reading; it is')
    print('consistent with it. It is also page-level: it cannot say that the shape carrying')
    print('the dominant text is the one stating shrink-to-fit.')

if __name__ == '__main__':
    main(*sys.argv[1:4])
