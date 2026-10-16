#!/usr/bin/env python3
"""Probe: what does LibreOffice do when a section names only even/first headers?"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mkdocx import build, para, PGSZ

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'probes')
os.makedirs(OUT, exist_ok=True)

HA = para('HEADERALPHA')
HB = para('HEADERBETA')
HC = para('HEADERGAMMA')
HD = para('HEADERDELTA')


def sect(refs, first=False):
    """A sectPr; refs is list of (kind, type, name)."""
    x = ''.join(
        f'<w:{k}Reference w:type="{t}" r:id="rIdH{n}"/>' for k, t, n in refs)
    return x + PGSZ


def body_of(sections):
    """sections: list of (paras, sectPr-inner). Last one goes in body-level sectPr."""
    out = []
    for i, (ps, sp) in enumerate(sections):
        if i < len(sections) - 1:
            out.append(f'<w:p><w:pPr><w:sectPr>{sp}</w:sectPr></w:pPr></w:p>')
            out.insert(len(out) - 1, ''.join(ps))
        else:
            out.append(''.join(ps))
            out.append(f'<w:sectPr>{sp}</w:sectPr>')
    return ''.join(out)


def two_section(name, s2refs, settings='', s1refs=(('header', 'default', 'A'),)):
    body = (''.join(para(f'SECTIONONEBODY{i}') for i in range(3))
            + f'<w:p><w:pPr><w:sectPr>{sect(s1refs)}</w:sectPr></w:pPr></w:p>'
            + ''.join(para(f'SECTIONTWOBODY{i}') for i in range(3))
            + f'<w:sectPr>{sect(s2refs)}</w:sectPr>')
    build(os.path.join(OUT, name + '.docx'), body,
          {'A': HA, 'B': HB, 'C': HC, 'D': HD}, settings)


# 1. S2 names only even + first (the UG.CAO shape). No evenAndOddHeaders, no titlePg.
two_section('hdr-even-first-only',
            [('header', 'even', 'B'), ('header', 'first', 'C')])

# 2. S2 names only even.
two_section('hdr-even-only', [('header', 'even', 'B')])

# 3. S2 names only first.
two_section('hdr-first-only', [('header', 'first', 'C')])

# 4. S2 names nothing at all (pure link-to-previous control).
two_section('hdr-none', [])

# 5. S2 names default explicitly (control).
two_section('hdr-default', [('header', 'default', 'D')])

# 6. S2 names even+first, WITH evenAndOddHeaders on.
two_section('hdr-even-first-eao',
            [('header', 'even', 'B'), ('header', 'first', 'C')],
            settings='<w:evenAndOddHeaders/>')

print('built', sorted(os.listdir(OUT)))
