#!/usr/bin/env python3
"""splitnil.docx -- the ONLY arm that reaches InsertFollowTopBorder / InsertMasterBottomBorder.

Every interior horizontal edge is `nil` on BOTH sides, so no cell supplies a border at the cut;
the first row states a 3 pt top and the last row a 3 pt bottom, which are what those two
functions copy.  3 pt is chosen so `RefMode::Begin` and `RefMode::Centered` differ by 1.5 pt.
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
spec = importlib.util.spec_from_file_location('mp', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'make-probes.py'))
# make-probes.py runs on import; give it a scratch dir it may write into
sys.argv = [sys.argv[0], os.path.join(os.path.dirname(os.path.abspath(__file__)), 'fixtures')]
mp = importlib.util.module_from_spec(spec); spec.loader.exec_module(mp)

N = 70
rows = []
for i in range(N):
    top = mp.X if i == 0 else mp.NIL
    bottom = mp.X if i == N - 1 else mp.NIL
    rows.append([(top, bottom, mp.OUT, mp.OUT)] * 2)
t = mp.tbl('SN', rows, mp.COL2)
mp.write(os.path.join(sys.argv[1], 'splitnil.docx'), mp.para('ARM SN') + t)
