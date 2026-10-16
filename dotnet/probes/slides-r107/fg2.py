#!/usr/bin/env python3
"""Independent second census of the fGtext bit, with a different failure mode from
slides-r107/fgtext.py.

fgtext.py walks the record tree from offset 0 and can silently desync on one bad length.
This one does not walk: it scans every byte offset for a plausible msofbtOPT header
(ver=3, recType=0xF00B, len == 6*instance) and reads the property array there.  A desync
cannot hide a record from it; the cost is that it can in principle find a false OPT inside
a picture blob, so it also reports how many candidate OPT records it saw in total.
"""
import struct, sys
import olefile

OPT = 0xF00B


def main(paths):
    tot_opt = tot_hit = 0
    for path in paths:
        try:
            ole = olefile.OleFileIO(path)
            s = ole.openstream('PowerPoint Document').read()
            ole.close()
        except Exception as e:
            print(f'{path}\tERROR {e}')
            continue
        n_opt = 0
        hits = []
        for p in range(0, len(s) - 8):
            vi, rt, rl = struct.unpack_from('<HHI', s, p)
            if rt != OPT or (vi & 0x0F) != 3:
                continue
            inst = vi >> 4
            if inst == 0 or rl < inst * 6 or p + 8 + rl > len(s):
                continue
            n_opt += 1
            for i in range(inst):
                pid, val = struct.unpack_from('<HI', s, p + 8 + i * 6)
                if (pid & 0x3FFF) == 255 and not (pid & 0x8000) and (val & 0x4000):
                    hits.append((p, val))
                    break
        tot_opt += n_opt
        tot_hit += len(hits)
        for p, val in hits:
            print(f'{path.rsplit("/",1)[-1]}\topt@{p}\tprop255=0x{val:08x}')
        print(f'# {path.rsplit("/",1)[-1]}\tcandidate OPT records {n_opt}\tfGtext {len(hits)}')
    print(f'## files {len(paths)}   candidate OPT {tot_opt}   fGtext shapes {tot_hit}')


main(sys.argv[1:])
