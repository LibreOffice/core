#!/usr/bin/env python3
"""Perturb one `sprm` in one section of a .doc and write the result beside it.

    patch-sepx.py <doc> <section-index> <sprm-hex> <value> <out.doc>

The corpus file this round used is `words/done-014/doc/PK_FlugzeugeStricken.doc`, whose
`PlcfSed` holds two sections at CP [0,738) and [738,12823).  Both state `sprmSBkc` = 0, so
both are continuous; the second states `sprmSDyaTop` (0x9023) = 1701 twips against the
first's 1417.

Patching is done by locating a unique 32-byte window around the value in the whole file and
rewriting the two bytes at its centre, which avoids having to rebuild the compound file.
The script refuses to write when the window is not unique.
"""
import olefile, struct, sys


def sepx_offsets(fib, table):
    """(cp_start, cp_end, fcSepx) per section, from the FIB's PlcfSed."""
    fc, lcb = struct.unpack_from("<II", fib, 0xCA)
    n = (lcb - 4) // 16                      # 4 bytes a CP plus a 12-byte SED
    cps = struct.unpack_from("<%dI" % (n + 1), table, fc)
    base = fc + 4 * (n + 1)
    out = []
    for k in range(n):
        _, fcSepx, _, _ = struct.unpack_from("<hihi", table, base + 12 * k)
        out.append((cps[k], cps[k + 1], fcSepx))
    return out


def value_offset(fib, fcSepx, wanted):
    """Where in the WordDocument stream that sprm's operand starts."""
    cb = struct.unpack_from("<H", fib, fcSepx)[0]
    blob = fib[fcSepx + 2:fcSepx + 2 + cb]
    i = 0
    while i + 2 <= len(blob):
        sprm = struct.unpack_from("<H", blob, i)[0]
        i += 2
        spra = (sprm >> 13) & 7
        if spra == 6:                        # variable length, one-byte count
            i += 1 + blob[i]
            continue
        if sprm == wanted:
            return fcSepx + 2 + i
        i += {0: 1, 1: 1, 2: 2, 3: 4, 4: 2, 5: 2, 7: 3}[spra]
    return None


def main(path, section, sprm, value, out, window=16):
    ole = olefile.OleFileIO(path)
    fib = ole.openstream("WordDocument").read()
    flags = struct.unpack_from("<H", fib, 0x0A)[0]
    table = ole.openstream("1Table" if (flags >> 9) & 1 else "0Table").read()

    _, _, fcSepx = sepx_offsets(fib, table)[section]
    at = value_offset(fib, fcSepx, sprm)
    if at is None:
        raise SystemExit(f"section {section} states no sprm {sprm:#06x}")

    data = open(path, "rb").read()
    pattern = fib[at - window:at + window]
    if data.count(pattern) != 1:
        raise SystemExit(f"the {2 * window}-byte window is not unique; widen it")

    where = data.find(pattern) + window
    open(out, "wb").write(data[:where] + struct.pack("<H", value) + data[where + 2:])
    print(f"{out}: section {section} sprm {sprm:#06x} -> {value}")


if __name__ == "__main__":
    main(sys.argv[1], int(sys.argv[2]), int(sys.argv[3], 16), int(sys.argv[4]), sys.argv[5])
