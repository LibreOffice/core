"""Two reach censuses for WordArt, both of which decide whether a body of work is worth writing.

1. Binary Escher WordArt across the corpus's `.doc`/`.ppt`/`.xls`, by scanning for an `msofbtSp`
   record (fbt 0xF00A, length 8) whose instance -- the shape type -- is in the WordArt range
   136..175 (`include/svx/msdffdef.hxx:412-451`). Then the sibling `msofbtOPT` (0xF00B) is read for
   the `gtext*` properties, and in particular for `DFF_Prop_gtextFStrikethrough` (255), whose bits
   carry text-path-on (0x4000), fitpath (0x100), fitshape (0x400), ScaleX (0x40) and
   SameLetterHeights (0x80) -- `filter/source/msfilter/msdffimp.cxx:2516-2600`.

2. Which faces the corpus's WordArt actually resolves to, and whether any of them is CFF rather
   than `glyf`. `XRay.Office.Text.Fonts.GlyphOutlines` reads `glyf` only.

Usage:  python3 census.py [corpus-root]
"""
import collections, os, struct, subprocess, sys

CORPUS = sys.argv[1] if len(sys.argv) > 1 else "/home/user/sample-files"
BINARY = ('.doc', '.dot', '.ppt', '.pot', '.pps', '.xls', '.xlt', '.docm', '.rtf')

FSP = {struct.pack('<HHI', (t << 4) | 2, 0xF00A, 8): t for t in range(136, 176)}
GTEXT = {192: 'gtextUNICODE', 194: 'gtextAlign', 195: 'gtextSize', 196: 'gtextSpacing',
         197: 'gtextFont', 245: 'gtextFStretch', 250: 'gtextFBold', 251: 'gtextFItalic',
         255: 'gtextFStrikethrough'}


def escher():
    found = collections.Counter()
    scanned = 0
    for dp, _, fn in os.walk(CORPUS):
        for f in fn:
            if not f.lower().endswith(BINARY):
                continue
            scanned += 1
            data = open(os.path.join(dp, f), 'rb').read()
            for pat, t in FSP.items():
                at = 0
                while True:
                    i = data.find(pat, at)
                    if i < 0:
                        break
                    at = i + 1
                    found[f] += 1
                    print(f"  {f}  shape type {t}")
                    j = data.find(struct.pack('<H', 0xF00B), i, i + 4096)
                    if j < 0:
                        print("      no msofbtOPT within 4 KiB")
                        continue
                    count = struct.unpack('<H', data[j - 2:j])[0] >> 4
                    props = data[j + 6:j + 6 + 6 * count]
                    seen = {}
                    for k in range(count):
                        pid, value = struct.unpack('<HI', props[6 * k:6 * k + 6])
                        base = pid & 0x3FFF
                        if base in GTEXT:
                            seen[base] = value
                    flags = seen.get(255, 0)
                    print(f"      {{{', '.join(f'{GTEXT[k]}=0x{v:x}' for k, v in sorted(seen.items()))}}}")
                    print(f"      textpath={bool(flags & 0x4000)} fitpath={bool(flags & 0x100)} "
                          f"fitshape={bool(flags & 0x400)} ScaleX={bool(flags & 0x40)} "
                          f"SameLetterHeights={bool(flags & 0x80)} "
                          f"gtextFStretch-hard={245 in seen}")
    print(f"scanned {scanned} binary files; {sum(found.values())} WordArt shapes "
          f"in {len(found)} documents")


def faces(families):
    for family in families:
        path = subprocess.run(['fc-match', '-f', '%{file}', family],
                              capture_output=True, text=True).stdout.strip()
        data = open(path, 'rb').read()
        off = struct.unpack('>I', data[12:16])[0] if data[:4] == b'ttcf' else 0
        n = struct.unpack('>H', data[off + 4:off + 6])[0]
        tables = {data[off + 12 + 16 * i:off + 16 + 16 * i].decode('latin1') for i in range(n)}
        kind = 'glyf' if 'glyf' in tables else ('CFF' if {'CFF ', 'CFF2'} & tables else 'other')
        print(f"  {family:26s} -> {os.path.basename(path):34s} {kind}")


if __name__ == "__main__":
    print("=== binary Escher WordArt ===")
    escher()
    print("=== the families the corpus's WordArt names ===")
    faces(["Arial", "Perpetua Titling MT", "Kristen ITC", "Times New Roman", "Calibri",
           "Arial Black", "Corpid E1s SCd Regular", "Papyrus", "Informal Roman"])
