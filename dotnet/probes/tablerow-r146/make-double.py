#!/usr/bin/env python3
"""double.docx -- how 26.2.4.2 scales `w:val="double" w:sz=n`, which census-docx.py assumes."""
import sys, os, importlib.util
here = os.path.dirname(os.path.abspath(__file__))
sys.argv = [sys.argv[0], os.path.join(here, 'fixtures')]
spec = importlib.util.spec_from_file_location('mp', os.path.join(here, 'make-probes.py'))
mp = importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)

arms = []
for sz in (2, 4, 6, 8, 12, 18, 24):
    arms.append(mp.page(f'D{sz}', mp.tbl(f'D{sz}', [
        [(mp.OUT, ('double', sz), mp.OUT, mp.OUT)] * 2,
        [(mp.NIL, mp.OUT, mp.OUT, mp.OUT)] * 2,
    ], mp.COL2)))
mp.write(os.path.join(sys.argv[1], 'double.docx'), ''.join(arms))
