#!/usr/bin/env python3
"""Which documents two sweeps of the same corpus list rendered differently.

Both legs render one document per directory under `SOURCE_DATE_EPOCH=0`, so a byte difference is
the change under test and nothing else. Prints the movers, and refuses to print a total unless
every document on the list produced a rendering in both legs — a sweep that lost a worker reports
fewer rows rather than an error.
"""
import hashlib
import pathlib
import sys

docs = [line.strip() for line in open(sys.argv[1]) if line.strip()]
before, after = pathlib.Path(sys.argv[2]), pathlib.Path(sys.argv[3])

moved, missing = [], []
for path in docs:
    key = hashlib.md5(path.encode()).hexdigest()[:12]
    a = sorted((after / key).glob('*.pdf'))
    b = sorted((before / key).glob('*.pdf'))
    if not a or not b:
        missing.append(path)
        continue
    if a[0].read_bytes() != b[0].read_bytes():
        moved.append(path)

if missing:
    print(f'REFUSING A TOTAL: {len(missing)} of {len(docs)} rendered in only one leg')
    for path in missing[:10]:
        print('   missing', path)
    raise SystemExit(1)

print(f'{len(moved)} of {len(docs)} moved')
for path in moved:
    print('  ', path)
