#!/usr/bin/env python3
"""Round 99's `TXO` census with its CONTINUE assumption corrected.

r99's `txo-colour-census.py` skipped exactly ONE `CONTINUE` as the character data and read
the next one as the run array.  A `TXO` whose characters need several continuations —
`EHEST-Pre-departure-checklist`'s instruction boxes do — therefore had character bytes
parsed as eight-byte run entries, which is where its "43 colour runs against the
reference's 119 spans" came from.

The fix: concatenate every `CONTINUE` that follows the `TXO` and take the run array as the
LAST `formatSize` bytes of it, which is where it sits — the array follows the last
character and nothing follows the array.  That needs no knowledge of how the string was
split, and it is checked here by asserting the array's first entry is character 0 and its
entries are non-decreasing, which random character bytes are not.
"""
import glob, os, struct, sys, collections
import olefile

CORPUS = "/home/user/sample-files"
TXO, CONTINUE, OBJ, FONT = 0x01B6, 0x003C, 0x005D, 0x0031
NOTE_TYPES, SHAPE_TYPES = {25}, {6, 7}


def workbook(path):
    try:
        ole = olefile.OleFileIO(path)
    except Exception:
        return None
    for name in ("Workbook", "Book"):
        if ole.exists(name):
            return ole.openstream(name).read()
    return None


def records(data):
    at = 0
    while at + 4 <= len(data):
        rid, size = struct.unpack_from("<HH", data, at)
        body = at + 4
        if body + size > len(data):
            return
        yield rid, data[body:body + size]
        at = body + size


def plausible(entries):
    """Whether a parsed run array looks like one: opens at character 0 and never goes back."""
    if not entries:
        return False
    if entries[0][0] != 0:
        return False
    return all(entries[i][0] <= entries[i + 1][0] for i in range(len(entries) - 1))


def main():
    print("document\tobjtype\tchars\truns\tapplied\tcolour\titalic\tplausible")
    tally = collections.Counter()
    docs = collections.defaultdict(collections.Counter)

    for path in sorted(glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)):
        data = workbook(path)
        if data is None:
            continue
        name = os.path.basename(path)

        fonts = []
        objtype = None
        pending = None      # [chars, formatSize, objtype, [continue bodies]]
        for rid, body in records(data):
            if rid == FONT and len(body) >= 14:
                grbit, icv = struct.unpack_from("<HH", body, 2)
                fonts.append((bool(grbit & 0x0002), icv))
                continue

            if rid == OBJ and len(body) >= 10:
                ft = struct.unpack_from("<H", body, 0)[0]
                objtype = struct.unpack_from("<H", body, 4)[0] if ft == 0x15 else None
                continue

            if rid == TXO and len(body) >= 14:
                if pending is not None:
                    emit(pending, fonts, tally, docs, name)
                chars, size = struct.unpack_from("<HH", body, 10)
                pending = [chars, size, objtype, b""]
                continue

            if rid == CONTINUE and pending is not None:
                pending[3] += body
                continue

            if pending is not None:
                emit(pending, fonts, tally, docs, name)
                pending = None

        if pending is not None:
            emit(pending, fonts, tally, docs, name)

    print("\n# totals", file=sys.stderr)
    for k in sorted(tally):
        print("#   %-20s %d" % (k, tally[k]), file=sys.stderr)
    print("# documents with a shape-path run stating a colour:", file=sys.stderr)
    for d, c in sorted(docs.items()):
        if c['shape/colour']:
            print("#   %-58s colour %-4d boxes %d" % (d, c['shape/colour'], c['shape/boxes']),
                  file=sys.stderr)


def emit(pending, fonts, tally, docs, name):
    chars, size, objtype, blob = pending
    if chars <= 0:
        return
    count = size // 8
    entries = []
    if count > 0 and len(blob) >= size:
        tail = blob[len(blob) - size:]
        entries = [struct.unpack_from("<HH", tail, at * 8) for at in range(count)]
    ok = plausible(entries)
    applied = entries[:-1] if len(entries) > 1 else entries
    colour = italic = 0
    for _, ix in applied:
        if ix == 0xFFFF:
            continue
        rec = ix - 1 if ix >= 4 else ix
        if rec < 0 or rec >= len(fonts):
            continue
        it, icv = fonts[rec]
        if it:
            italic += 1
        if icv not in (0x7FFF, 0x0008):
            colour += 1
    kind = ('note' if objtype in NOTE_TYPES
            else 'shape' if objtype in SHAPE_TYPES else 'other:%s' % objtype)
    tally[kind + '/boxes'] += 1
    tally[kind + '/runs'] += len(applied)
    tally[kind + '/colour'] += colour
    tally[kind + '/italic'] += italic
    tally[kind + '/plausible'] += 1 if ok else 0
    docs[name][kind + '/colour'] += colour
    docs[name][kind + '/boxes'] += 1
    print("%s\t%s\t%d\t%d\t%d\t%d\t%d\t%s"
          % (name, kind, chars, len(entries), len(applied), colour, italic, ok))


if __name__ == "__main__":
    main()
