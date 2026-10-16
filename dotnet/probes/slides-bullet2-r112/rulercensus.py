#!/usr/bin/env python3
"""Census of `.ppt` shapes whose own `TextRulerAtom` was never applied.

A shape whose text is an `OutlineTextRefAtom` keeps its ruler in its own client
textbox while the characters live in the document's slide list.  The reference
reads both -- `PPTTextObj`'s constructor remembers the ruler's file offset
before it patches the client-textbox header over to the referenced text
(`svdfppt.cxx`, the `nTextRulerAtomOfs` block) -- and this tree returned before
it reached the ruler on that branch.

Prints, per document: rulers in total, rulers in a textbox that refers out (the
ones that were lost), and how many of those state a level-0 text offset.

Usage: rulercensus.py <corpus-root> [glob]
"""
import struct, sys, pathlib, collections
import olefile

RULER = 4006            # RT_TextRulerAtom
OUTLINE_REF = 3998      # RT_OutlineTextRefAtom
CLIENT_TEXTBOX = 0xF00D


def records(data, off, end):
    while off + 8 <= end:
        ver, typ, ln = struct.unpack_from('<HHI', data, off)
        body, bend = off + 8, off + 8 + ln
        if bend > end:
            return
        yield typ, body, ln
        if (ver & 0xF) == 0xF:
            yield from records(data, body, bend)
        off = bend


def states_level0(data, body, length):
    """Whether the ruler's flags name a level-0 text offset (bit 3)."""
    if length < 4:
        return False
    flags, = struct.unpack_from('<I', data, body)
    return bool(flags & 0x08)


def scan(path):
    with olefile.OleFileIO(str(path)) as handle:
        if not handle.exists('PowerPoint Document'):
            return 0, 0, 0
        data = handle.openstream('PowerPoint Document').read()

    boxes = [(b, b + ln) for typ, b, ln in records(data, 0, len(data))
             if typ == CLIENT_TEXTBOX]
    inner = collections.defaultdict(list)
    for typ, b, ln in records(data, 0, len(data)):
        for start, end in boxes:
            if start <= b < end:
                inner[(start, end)].append((typ, b, ln))

    total = lost = level0 = 0
    for entries in inner.values():
        rulers = [(b, ln) for typ, b, ln in entries if typ == RULER]
        refers = any(typ == OUTLINE_REF for typ, b, ln in entries)
        total += len(rulers)
        if refers:
            lost += len(rulers)
            level0 += sum(1 for b, ln in rulers if states_level0(data, b, ln))
    return total, lost, level0


def main():
    root = pathlib.Path(sys.argv[1])
    pattern = sys.argv[2] if len(sys.argv) > 2 else '**/*.ppt'
    totals = collections.Counter()
    for path in sorted(root.glob(pattern)):
        try:
            total, lost, level0 = scan(path)
        except Exception as exc:
            print(f'{path.name}\tERROR\t{exc}')
            continue
        if total:
            print(f'{path.name}\trulers {total}\tlost {lost}\twith level-0 text offset {level0}')
            totals['documents'] += 1
            totals['rulers'] += total
            totals['lost'] += lost
            totals['level0'] += level0
            if lost:
                totals['documents-losing'] += 1
    print('TOTAL', dict(totals))


if __name__ == '__main__':
    main()
