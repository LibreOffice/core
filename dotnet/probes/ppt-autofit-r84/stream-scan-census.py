#!/usr/bin/env python3
"""Census of `.ppt` text-range hyperlinks by a real record walk.

The previous round's figure -- "17 of 51, 91 atoms" -- was a byte-pattern scan for a
record header of type 4063 and was stated as an upper bound.  This walks the PowerPoint
Document stream's record tree instead, and separates the three things
`svdfppt.cxx`:6900-6941 actually requires of a text-range hyperlink:

  * an `InteractiveInfo` (4082) holding an `InteractiveInfoAtom` (4083),
  * the *next sibling* record being a `TxInteractiveInfoAtom` (4063) with a non-zero end,
  * the atom's `exHyperlinkId` matching an entry of the document's hyperlink list, which
    `pptin.cxx`:531-537 fills from the `ExObjList`'s `ExHyperlinkAtom` (4051) values.
"""
import sys, struct, olefile, pathlib, csv

EXOBJLIST, EXHYPERLINKATOM, EXHYPERLINK = 1033, 4051, 4055
TXINTERACTIVE, INTERACTIVEINFO, INTERACTIVEINFOATOM = 4063, 4082, 4083
CLIENTTEXTBOX = 0xF00D

def records(buf, start, end):
    p = start
    while p + 8 <= end:
        ver_inst, rtype, rlen = struct.unpack_from('<HHI', buf, p)
        body = p + 8
        stop = body + rlen
        if stop > end or rlen > len(buf):
            return
        yield (ver_inst & 0x0F, rtype, body, stop)
        p = stop

def walk(buf, start, end, depth=0):
    """Every record in the tree, with its parent chain type list."""
    for ver, rtype, body, stop in records(buf, start, end):
        yield (rtype, body, stop, depth)
        if ver == 0x0F:
            yield from walk(buf, body, stop, depth + 1)

def hyperlink_ids(buf):
    ids = []
    for rtype, body, stop, _ in walk(buf, 0, len(buf)):
        if rtype == EXOBJLIST:
            for t2, b2, s2, _ in walk(buf, body, stop):
                if t2 == EXHYPERLINKATOM and s2 - b2 >= 4:
                    ids.append(struct.unpack_from('<I', buf, b2)[0])
    return ids

def text_ranges(buf):
    """(exHyperlinkId, start, end) for every InteractiveInfo followed by a Tx atom."""
    out = []
    def scan(start, end):
        seq = list(records(buf, start, end))
        for i, (ver, rtype, body, stop) in enumerate(seq):
            if rtype == INTERACTIVEINFO:
                hid = None
                for t2, b2, s2, _ in walk(buf, body, stop):
                    if t2 == INTERACTIVEINFOATOM and s2 - b2 >= 8:
                        hid = struct.unpack_from('<I', buf, b2 + 4)[0]
                        break
                nxt = seq[i + 1] if i + 1 < len(seq) else None
                if hid is not None and nxt and nxt[1] == TXINTERACTIVE and nxt[3] - nxt[2] >= 8:
                    s, e = struct.unpack_from('<II', buf, nxt[2])
                    out.append((hid, s, e))
            elif ver == 0x0F:
                scan(body, stop)
    scan(0, len(buf))
    return out

def main(paths, out):
    w = csv.writer(out, delimiter='\t')
    w.writerow(['document', 'exhyperlink_atoms', 'tx_atoms', 'tx_nonzero_end', 'resolvable', 'ids'])
    tot = [0, 0, 0, 0, 0]
    for p in paths:
        try:
            ole = olefile.OleFileIO(str(p))
            stream = 'PowerPoint Document'
            if not ole.exists(stream):
                continue
            buf = ole.openstream(stream).read()
            ole.close()
        except Exception as exc:
            w.writerow([p.name, 'ERR', str(exc), '', '', ''])
            continue
        ids = hyperlink_ids(buf)
        ranges = text_ranges(buf)
        nonzero = [r for r in ranges if r[2]]
        resolvable = [r for r in nonzero if r[0] in ids]
        w.writerow([p.name, len(ids), len(ranges), len(nonzero), len(resolvable),
                    ','.join(str(r[0]) for r in resolvable[:12])])
        tot[0] += 1
        tot[1] += 1 if ranges else 0
        tot[2] += len(ranges)
        tot[3] += len(nonzero)
        tot[4] += len(resolvable)
    print(f'documents {tot[0]}  with a Tx atom {tot[1]}  atoms {tot[2]}  '
          f'non-zero end {tot[3]}  resolvable {tot[4]}', file=sys.stderr)

if __name__ == '__main__':
    root = pathlib.Path(sys.argv[1])
    paths = sorted(q for q in root.rglob('*') if q.suffix.lower() == '.ppt' and q.is_file())
    main(paths, sys.stdout)
