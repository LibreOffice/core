#!/usr/bin/env python3
"""Which legacy PPT-family corpus documents STATE a family class whose fc-match splits.

A `.ppt` states the class in the top nibble of `lfPitchAndFamily`, the last byte of every
68-byte FontEntityAtom.  `FontConfigManager::Substitute` appends a second FC_FAMILY --
"serif" for FF_ROMAN, "sans" for FF_SWISS -- so the document is exposed to the seventh
confound whenever `fc-match "<name>"` and `fc-match "<name>,<generic>"` disagree.

This counts documents that STATE such a pair, not documents where it reaches drawn text;
`sweep2.sh` measures the latter against the reference's own output.
"""
import olefile, glob, os, struct, subprocess

GEN = {0x10: 'serif', 0x20: 'sans'}
_cache = {}


def fcm(query):
    if query not in _cache:
        out = subprocess.run(['fc-match', query], capture_output=True, text=True).stdout
        _cache[query] = os.path.basename(out.split(':')[0])
    return _cache[query]


def name_of(body):
    """The FontEntityAtom's name: 64 bytes of UTF-16LE, NUL-terminated, garbage after."""
    out = []
    for k in range(0, 64, 2):
        if body[k] == 0 and body[k + 1] == 0:
            break
        out.append(body[k:k + 2])
    return b''.join(out).decode('utf-16-le', errors='replace')


def main():
    files = sorted(sum((glob.glob(f'/home/user/sample-files/slides/**/*.{e}', recursive=True)
                        for e in ('ppt', 'pps', 'pot')), []))
    total = affected = 0
    pairs = {}
    for path in files:
        ole = olefile.OleFileIO(path)
        streams = [s for s in ole.listdir() if s[-1] == 'PowerPoint Document']
        if not streams:
            print('NOSTREAM', path)
            ole.close()
            continue
        data = ole.openstream(streams[0]).read()
        ole.close()
        total += 1
        seen, split, faces = set(), [], 0
        i = 0
        while True:
            j = data.find(b'\xb7\x0f', i)
            if j < 0:
                break
            i = j + 1
            if j + 6 + 68 > len(data):
                continue
            if struct.unpack('<I', data[j + 2:j + 6])[0] != 68:
                continue
            body = data[j + 6:j + 6 + 68]
            name = name_of(body)
            if not name or '�' in name:
                continue
            key = (name, body[67] & 0xF0)
            if key in seen:
                continue
            seen.add(key)
            faces += 1
            generic = GEN.get(key[1])
            if not generic:
                continue
            for weight in ('', ':bold'):
                bare, classful = fcm(name + weight), fcm(f'{name},{generic}{weight}')
                if bare != classful:
                    row = (name, hex(key[1]), bare, classful)
                    split.append(row)
                    pairs[row] = pairs.get(row, 0) + 1
                    break
        if split:
            affected += 1
        print(f"{'SPLIT' if split else '-    '} {os.path.basename(path):58s} faces={faces:2d} "
              + '; '.join(f'{n}[{c}] {a}->{b}' for n, c, a, b in split))
    print()
    print(f'legacy PPT-family corpus files: {total}')
    print(f'stating at least one class-ful family whose fc-match splits: {affected}')
    for row, count in sorted(pairs.items(), key=lambda x: -x[1]):
        print('  ', count, row)


if __name__ == '__main__':
    main()
