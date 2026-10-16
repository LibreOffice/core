#!/usr/bin/env python3
"""O33's census: how many BIFF `TXO` formatting runs state a colour or an italic that
`SheetShapeRun` cannot carry — and, of those, how many are on an object whose text goes
through the shape path at all.

A `TXO` (0x01B6) is followed by a `CONTINUE` holding the characters and a second holding
the eight-byte run array (character index, `FONT` index, four reserved).  The `OBJ`
(0x005D) before it opens with an `ftCmo` subrecord whose object type separates a text box
or button from a cell comment, whose text Calc turns into a `ScPostIt` and never draws as
a shape (`XclImpNoteObj::SetInsertSdrObj(false)`).

A `FONT` (0x0031) is dyHeight, grbit, icv, bls, sss, uls, family, charset, reserved, cch —
`grbit` bit 1 is italic and `icv` is a palette index, 0x7FFF meaning automatic.  BIFF's
font index 4 does not exist: an index of four or more names the record one earlier.
"""
import glob, os, struct, sys, collections
import olefile

CORPUS = "/home/user/sample-files"
TXO, CONTINUE, BOF, OBJ, FONT, PALETTE = 0x01B6, 0x003C, 0x0809, 0x005D, 0x0031, 0x0092

NOTE_TYPES = {25}
SHAPE_TYPES = {6, 7}      # text box, button — the two that reach the shape path


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


def main():
    print("document\tobjtype\truns\tcolour_runs\titalic_runs\tfont_indices")
    tally = collections.Counter()
    docs = collections.defaultdict(collections.Counter)

    for path in sorted(glob.glob(os.path.join(CORPUS, "**", "*.[xX][lL][sS]"), recursive=True)):
        data = workbook(path)
        if data is None:
            continue
        name = os.path.basename(path)

        fonts = []          # globals-stream FONT records, in order
        pending = None
        objtype = None
        for rid, body in records(data):
            if rid == FONT and len(body) >= 14:
                grbit, icv = struct.unpack_from("<HH", body, 2)
                fonts.append((bool(grbit & 0x0002), icv))
                continue

            if rid == OBJ and len(body) >= 10:
                ft, cb = struct.unpack_from("<HH", body, 0)
                objtype = struct.unpack_from("<H", body, 4)[0] if ft == 0x15 else None
                continue

            if rid == TXO and len(body) >= 14:
                chars, size = struct.unpack_from("<HH", body, 10)
                pending = [chars, size, 0, objtype]
                continue

            if rid == CONTINUE and pending is not None:
                if pending[2] == 0 and pending[0] > 0:
                    pending[2] = 1
                    continue
                count = pending[1] // 8
                indices = [struct.unpack_from("<H", body, at * 8 + 2)[0]
                           for at in range(count) if at * 8 + 8 <= len(body)]
                # The last entry is the terminator: it names a character index past the
                # string and applies to nothing.
                applied = indices[:-1] if len(indices) > 1 else indices
                colour = italic = 0
                for ix in applied:
                    if ix == 0xFFFF: continue
                    rec = ix - 1 if ix >= 4 else ix
                    if rec < 0 or rec >= len(fonts): continue
                    it, icv = fonts[rec]
                    if it: italic += 1
                    if icv != 0x7FFF and icv != 0x0008: colour += 1
                kind = ('note' if pending[3] in NOTE_TYPES
                        else 'shape' if pending[3] in SHAPE_TYPES
                        else 'other:%s' % pending[3])
                if pending[0] > 0:
                    tally[kind + '/boxes'] += 1
                    tally[kind + '/runs'] += len(applied)
                    tally[kind + '/colour'] += colour
                    tally[kind + '/italic'] += italic
                    docs[name][kind + '/colour'] += colour
                    docs[name][kind + '/italic'] += italic
                    docs[name][kind + '/boxes'] += 1
                    print("%s\t%s\t%d\t%d\t%d\t%s" % (
                        name, kind, len(applied), colour, italic,
                        ','.join(str(i) for i in indices)))
                pending = None
                continue
            if rid != CONTINUE:
                pending = None

    print("\n# totals", file=sys.stderr)
    for k in sorted(tally): print("#   %-18s %d" % (k, tally[k]), file=sys.stderr)
    print("# documents with a shape-path run stating colour or italic:", file=sys.stderr)
    n = 0
    for d, c in sorted(docs.items()):
        if c['shape/colour'] or c['shape/italic']:
            n += 1
            print("#   %-60s colour %-4d italic %-4d boxes %d"
                  % (d, c['shape/colour'], c['shape/italic'], c['shape/boxes']), file=sys.stderr)
    print("#   -> %d documents" % n, file=sys.stderr)


if __name__ == "__main__":
    main()
