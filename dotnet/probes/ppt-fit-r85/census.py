#!/usr/bin/env python3
"""Text-range hyperlinks in the corpus's 51 `.ppt`, over the *live* record tree.

Walks only the objects the persist directory names -- the current Document container and
every persisted page -- because a `.ppt` stream keeps every superseded version of every
object and a top-to-bottom scan counts orphans as if they were live.

Three columns, and the differences between them are the finding:

  tx       an `InteractiveInfo` (4082) immediately followed by a `TxInteractiveInfoAtom`
           (4063) whose end is non-zero (`svdfppt.cxx`:6913-6934).
  links    the `ExHyperlinkAtom` (4051) values of the live `ExObjList` (1033), which is
           what `pptin.cxx`:531-537 assigns to `m_aHyperList`'s entries in order.
  fields   `tx` restricted to those whose `exHyperlinkId` matches one of those values
           (`svdfppt.cxx`:6907-6909).  A document with no live `ExObjList` has an empty
           list and makes no field at all, whatever its Tx atoms say.
"""
import struct, sys, pathlib, csv
import olefile
from persist import directory, current_user_edit, header

EXOBJLIST, EXHYPERLINKATOM = 1033, 4055 - 4, 
EXHYPERLINKATOM = 4051
TXINTERACTIVE, INTERACTIVEINFO, INTERACTIVEINFOATOM = 4063, 4082, 4083

def records(buf, s, e):
    p = s
    while p + 8 <= e:
        vi, rt, rl = struct.unpack_from('<HHI', buf, p)
        body, stop = p + 8, p + 8 + rl
        if stop > e or rl > len(buf): return
        yield (vi & 0x0F, rt, body, stop)
        p = stop

def walk(buf, s, e):
    for ver, rt, b, st in records(buf, s, e):
        yield (rt, b, st)
        if ver == 0x0F:
            yield from walk(buf, b, st)

def live_roots(buf, offsets, doc):
    """The record extents the persist directory names, the Document first."""
    seen, roots = set(), []
    order = [doc] + [k for k in sorted(offsets) if k != doc]
    for pid in order:
        off = offsets.get(pid)
        if off is None or off in seen: continue
        seen.add(off)
        h = header(buf, off)
        if h: roots.append((h[1], off, h[3]))
    return roots

def link_ids(buf, roots):
    ids = []
    for rt, off, end in roots:
        for t, b, st in walk(buf, off + 8, end):
            if t == EXOBJLIST:
                for t2, b2, s2 in walk(buf, b, st):
                    if t2 == EXHYPERLINKATOM and s2 - b2 >= 4:
                        ids.append(struct.unpack_from('<I', buf, b2)[0])
    return ids

def tx_pairs(buf, roots):
    out = []
    def scan(s, e):
        seq = list(records(buf, s, e))
        for i, (ver, rt, b, st) in enumerate(seq):
            if rt == INTERACTIVEINFO:
                hid = None
                for v2, t2, b2, s2 in records(buf, b, st):
                    if t2 == INTERACTIVEINFOATOM and s2 - b2 >= 8:
                        hid = struct.unpack_from('<I', buf, b2 + 4)[0]
                        break
                nxt = seq[i + 1] if i + 1 < len(seq) else None
                if hid is not None and nxt and nxt[1] == TXINTERACTIVE and nxt[3] - nxt[2] >= 8:
                    a, z = struct.unpack_from('<II', buf, nxt[2])
                    if z: out.append((hid, a, z))
            elif ver == 0x0F:
                scan(b, st)
    for rt, off, end in roots:
        scan(off + 8, end)
    return out

def read(path):
    ole = olefile.OleFileIO(str(path))
    if not ole.exists('PowerPoint Document'):
        ole.close(); return None
    buf = ole.openstream('PowerPoint Document').read()
    cur = current_user_edit(ole)
    ole.close()
    offsets, doc = directory(buf, cur)
    roots = live_roots(buf, offsets, doc)
    ids = link_ids(buf, roots)
    pairs = tx_pairs(buf, roots)
    fields = [t for t in pairs if t[0] in set(ids)]
    return buf, ids, pairs, fields

def main(root, out):
    w = csv.writer(out, delimiter='\t', lineterminator='\n')
    w.writerow(['document', 'links', 'tx', 'fields'])
    n = dict(docs=0, tx_docs=0, tx=0, field_docs=0, fields=0)
    for p in sorted(q for q in pathlib.Path(root).rglob('*')
                    if q.suffix.lower() == '.ppt' and q.is_file()):
        got = read(p)
        if got is None: continue
        _, ids, pairs, fields = got
        w.writerow([p.name, len(ids), len(pairs), len(fields)])
        n['docs'] += 1
        n['tx_docs'] += 1 if pairs else 0
        n['tx'] += len(pairs)
        n['field_docs'] += 1 if fields else 0
        n['fields'] += len(fields)
    print(f"{n['docs']} .ppt; {n['tx_docs']} state a live text range ({n['tx']} atoms); "
          f"{n['field_docs']} make a field ({n['fields']} atoms)", file=sys.stderr)

if __name__ == '__main__':
    main(sys.argv[1], sys.stdout)
