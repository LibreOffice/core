"""Run the predictor against probe4 (the witness with empty/text drivers)."""
import sys
sys.path.insert(0, '/home/user/libreoffice-core/dotnet/probes/cfsem-r144')
import predict as P
from readfods import cellvals
from witness_read import read as read_pdf
P.V = cellvals('out/probe4.fods')
cols, rows, obs = read_pdf('out/probe4.pdf')
print('drivers:', {r: [P.cell('%s%d' % (c, r)) for c in 'CDEFG'] for r in (12, 14, 20, 22, 24, 25)})
bad = 0
for p in sorted(cols):
    for r in sorted(rows):
        want, name = P.predict(7 + p, r)
        got = obs[(p, r)]
        if (want or '').upper() != (got or '').upper():
            bad += 1
            print('MISMATCH %s%d predicted %s (%s) observed %s' % (P.colname(7 + p), r, want, name, got))
print('compared %d mismatches %d' % (len(cols) * len(rows), bad))
