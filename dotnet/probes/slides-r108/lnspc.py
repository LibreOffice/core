#!/usr/bin/env python3
"""Which pages state a proportional line height below 100 %, per 26.2.4.2's own flat ODP.

This is the base rate beside slides-r108's observation that the three pages where this tree and
the reference disagree *on the reference's own ODF model* all state one.  A page counts as
stating it when any paragraph style used on the slide itself -- notes excluded -- carries
`fo:line-height` with a percentage under 100.

Documents whose flat ODP has a different number of `draw:page` elements than the reference PDF
has pages are dropped rather than compared: on those the page indices do not line up and every
row would be a comparison of two different slides.

    lnspc.py <sizes-ref.tsv> <fodp>...
"""
import re, sys, os, collections

WITNESSES = [
    ("2015-Civil-Rights-Website-training__ppt", 22),
    ("Fundamentals_Module_1_basics__ppt", 6),
    ("JesuitAssocOfStudentPersonnel__ppt", 24),
    ("RRM-training-syllabus-Chapter-3-Teamwork-TC-Towing-Incident-Analysis-Dec-2009__ppt", 16),
    ("Thailand17__ppt", 8),
    ("Thailand17__ppt", 11),
    ("W3_Case_Study_of_a_Tsunami_Warning_Simulation_Exercise_Ed__ppt", 10),
    ("gfopportunitiesforlinkagespres_2010_en__ppt", 27),
    ("ws_prod-g-doc-Events-2007-september-M.017-(French)-France__ppt", 14),
    ("ws_prod-g-doc-Events-2007-september-M.017-(French)-France__ppt", 16),
    ("ws_prod-g-doc-Events-Part-M-presentation__ppt", 21),
]


def pages_of(path):
    x = open(path, encoding='utf-8').read()
    styles = {}
    for m in re.finditer(r'<style:style style:name="(P\d+)"[ >](.*?)</style:style>', x, re.S):
        a = re.search(r'fo:line-height="(\d+)%"', m.group(2))
        styles[m.group(1)] = int(a.group(1)) if a else None
    out = []
    for p in re.findall(r'<draw:page\b.*?</draw:page>', x, re.S):
        p = re.sub(r'<presentation:notes.*?</presentation:notes>', '', p, flags=re.S)
        vals = [styles.get(n) for n in set(re.findall(r'text:style-name="(P\d+)"', p))]
        out.append(any(v is not None and v < 100 for v in vals))
    return out


def main(reftsv, paths):
    rendered = collections.Counter()
    for line in open(reftsv):
        rendered[line.split('\t')[0]] += 1

    flags = {}
    counts = {}
    for path in paths:
        stem = os.path.basename(path)[:-5] + '__ppt'
        ps = pages_of(path)
        counts[stem] = len(ps)
        for i, f in enumerate(ps, 1):
            flags[(stem, i)] = f

    bad = sorted(d for d in counts if counts[d] != rendered.get(d))
    print(f"# documents dropped, flat-ODP page count != rendered page count: {len(bad)}")
    for d in bad:
        print(f"#   {d[:60]:60} {counts[d]:4d} vs {rendered.get(d)}")
    good = {k: v for k, v in flags.items() if k[0] not in bad}
    n = sum(1 for v in good.values() if v)
    print(f"# pages on the {len(counts) - len(bad)} aligned documents: {len(good)}")
    print(f"# stating a sub-100 % fo:line-height: {n}   ({100.0 * n / len(good):.1f} %)  <-- the base rate")
    print()
    print(f"{'document':60} {'pg':>4}  sub-100%  aligned")
    for d, p in WITNESSES:
        print(f"{d[:60]:60} {p:4d}  {str(flags.get((d, p))):8}  {d not in bad}")


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2:])
