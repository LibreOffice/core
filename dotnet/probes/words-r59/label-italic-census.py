#!/usr/bin/env python3
"""How many corpus documents can the list-label slant reach, and by which of its three arms.

    label-italic-census.py <manifest>

Counted per *arm*, not summed per document -- round 58's census summed three faces per document
and turned a face-selection divergence into an apparent lean defect.  The three arms here decide
different code paths and a document can be in more than one, so they are printed separately and
the union is printed as a union.

  A. a `w:lvl/w:rPr/w:i` on a level: the arm that reaches both bullets and numbers.
  B. a `w:pPr/w:rPr/w:i` *directly* on a list paragraph: the bullet-only arm (a paragraph style's
     `w:i` does not reach a bullet -- #i53199 resets the base font's posture -- so the style
     chain is deliberately NOT walked here).
  C. `w:i w:val="0"` on a level over an italic paragraph: the arm that takes a lean *away*.

What it cannot see, stated because an under-reaching census hides itself:
  * `.doc`: a WW8 level's `sprmCFItalic` is in a binary grpprl this script does not parse, so the
    66 `.doc` documents are counted as "not examined" rather than as zero.
  * whether a level that states `w:i` is ever *used* by a paragraph -- Word writes nine levels
    per abstractNum and a document may reference one.  So A and C are upper bounds.
  * inheritance of `w:i` through `w:pPrChange`, `w:docDefaults` or a numbering style, none of
    which reach a bullet anyway.
"""
import csv, os, re, sys, zipfile

man = sys.argv[1]
root = os.path.dirname(os.path.abspath(man))
I = re.compile(r'<w:i(?:\s+w:val="(?P<v>[^"]*)")?\s*/>')
OFF = {'0', 'false', 'off'}


def stated(fragment):
    m = I.search(fragment)
    if not m:
        return None
    return (m.group('v') or 'true').lower() not in OFF


rows = []
notexamined = []
with open(man, newline='', encoding='utf-8') as f:
    for r in csv.DictReader(f, delimiter='\t'):
        if r['family'] != 'words':
            continue
        path = os.path.join(root, r['path'])
        if r['ext'] != 'docx':
            notexamined.append(r['path'])
            continue
        try:
            z = zipfile.ZipFile(path)
            num = z.read('word/numbering.xml').decode('utf-8', 'replace')
        except Exception:
            num = ''
        try:
            doc = z.read('word/document.xml').decode('utf-8', 'replace')
        except Exception:
            doc = ''
        levels = re.findall(r'<w:lvl\b.*?</w:lvl>', num, re.S)
        on = sum(1 for l in levels
                 if (m := re.search(r'<w:rPr>.*?</w:rPr>', l, re.S)) and stated(m.group(0)) is True)
        off = sum(1 for l in levels
                  if (m := re.search(r'<w:rPr>.*?</w:rPr>', l, re.S)) and stated(m.group(0)) is False)
        marks = 0
        for para in re.findall(r'<w:p\b[^>]*>.*?</w:p>', doc, re.S):
            if 'numPr' not in para:
                continue
            ppr = re.search(r'<w:pPr>.*?</w:pPr>', para, re.S)
            if not ppr:
                continue
            rpr = re.search(r'<w:rPr>.*?</w:rPr>', ppr.group(0), re.S)
            if rpr and stated(rpr.group(0)) is True:
                marks += 1
        if on or off or marks:
            rows.append((r['path'], on, off, marks))

a = [r for r in rows if r[1]]
b = [r for r in rows if r[3]]
c = [r for r in rows if r[2] and r[3]]
print('docx documents examined: %d   .doc not examined: %d'
      % (sum(1 for _ in rows) or 0, len(notexamined)))
print('  A  level states w:i on          : %3d documents, %d levels'
      % (len(a), sum(r[1] for r in a)))
print('  B  list paragraph mark states it: %3d documents, %d paragraphs'
      % (len(b), sum(r[3] for r in b)))
print('  C  level states w:i off AND a mark states it on: %3d documents' % len(c))
print('  union of A and B                : %3d documents' % len({r[0] for r in a + b}))
print()
for p, on, off, marks in sorted(rows, key=lambda t: -(t[1] + t[3])):
    print('  %-78s levels-on %2d  levels-off %2d  marks %3d' % (p[-78:], on, off, marks))
