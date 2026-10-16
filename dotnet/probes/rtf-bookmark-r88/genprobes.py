#!/usr/bin/env python3
r"""Ten RTF files that put the bookmark rotation and the REF expansion under the reference.

Each file is one or two paragraphs of bookmarks followed by one paragraph per REF field, so
`--convert-to fodt` says where the bookmarks landed and `--convert-to pdf` says what each REF
drew.  The shapes separate the two halves of the mechanism:

  a_single      one bookmark, opened and closed -- the control the rotation must not move
  b_sequential  two bookmarks that do not overlap -- the second control
  c_nested      B inside A: A's entry takes B's name and A's range is emitted as B's
  d_overlap     A opens, B opens, A closes, B closes
  e_two         r87's shape: two starts then two ends, which comes out as one name twice
  f_five        the corpus's own shape: five starts, three collapsed ends, two more ends
  g_crosspara   a bookmark whose two ends are in different paragraphs
  h_point       a collapsed bookmark
  i_missing     a REF naming a bookmark the rotation leaves nobody holding
  j_crossref    a collapsed bookmark named the way Writer names a heading cross-reference

The paragraphs are token lists so that `check.py` can compute each half's offset without
parsing RTF: ('t', text), ('s', name) and ('e', name).
"""
import pathlib
import sys

# (paragraph token lists, the names the REF fields ask for)
PROBES = {
    'a_single': ([[('s', 'B1'), ('t', 'Alpha one'), ('e', 'B1')]], ['B1']),
    'b_sequential': ([[('s', 'B1'), ('t', 'Alpha one'), ('e', 'B1'), ('t', ' and '),
                       ('s', 'B2'), ('t', 'Beta two'), ('e', 'B2')]], ['B1', 'B2']),
    'c_nested': ([[('s', 'A'), ('t', 'outer '), ('s', 'B'), ('t', 'inner'), ('e', 'B'),
                   ('t', ' tail'), ('e', 'A')]], ['A', 'B']),
    'd_overlap': ([[('s', 'A'), ('t', 'first '), ('s', 'B'), ('t', 'middle'), ('e', 'A'),
                    ('t', ' last'), ('e', 'B')]], ['A', 'B']),
    'e_two': ([[('s', 'T3'), ('s', 'R1'), ('t', 'Table 48'), ('e', 'R1'),
                ('t', ': Caption Words'), ('e', 'T3')]], ['R1', 'T3']),
    'f_five': ([[('s', 'T1'), ('s', 'T2'), ('s', 'R2'), ('s', 'T3'), ('s', 'R1'),
                 ('e', 'T1'), ('e', 'T2'), ('e', 'R2'), ('t', 'Table 48'), ('e', 'R1'),
                 ('t', ': Caption Words Here'), ('e', 'T3')]],
               ['R1', 'T1', 'T2', 'T3', 'R2']),
    'g_crosspara': ([[('s', 'A'), ('t', 'first para')], [('t', 'second para'), ('e', 'A')]],
                    ['A']),
    'h_point': ([[('s', 'A'), ('e', 'A'), ('t', 'Alpha one')]], ['A']),
    'i_missing': ([[('s', 'A'), ('t', 'outer '), ('s', 'B'), ('t', 'inner'), ('e', 'B'),
                    ('t', ' tail'), ('e', 'A')]], ['A']),
    'j_crossref': ([[('s', '__RefHeading___Toc1'), ('e', '__RefHeading___Toc1'),
                     ('t', 'Heading text here')]], ['__RefHeading___Toc1']),
}

HDR = ("{\\rtf1\\ansi\\ansicpg1252\\deff0\n"
       "{\\fonttbl{\\f0\\froman\\fcharset0 Liberation Serif;}}\n"
       "\\paperw12240\\paperh15840\\margl1440\\margr1440\\margt1440\\margb1440\n")


def rtf_of(paras, names):
    body = ""
    for tokens in paras:
        body += "\\pard\\plain "
        for kind, value in tokens:
            if kind == 't':
                body += value
            elif kind == 's':
                body += "{\\*\\bkmkstart %s}" % value
            else:
                body += "{\\*\\bkmkend %s}" % value
        body += "\\par\n"
    for name in names:
        body += ("\\pard\\plain [%s] {\\field{\\*\\fldinst  REF %s \\\\h }"
                 "{\\fldrslt CACHED}} ;\\par\n" % (name, name))
    return HDR + body + "}\n"


if __name__ == '__main__':
    out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')
    out.mkdir(parents=True, exist_ok=True)
    for stem, (paras, names) in PROBES.items():
        (out / f'{stem}.rtf').write_text(rtf_of(paras, names), encoding='ascii')
    print("written", len(PROBES), "probes to", out)
