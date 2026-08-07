#!/usr/bin/env python3
"""Parse fontcvt.cxx's 224-entry recode tables and check them against OpenSymbol's cmap."""
import re, sys, struct

SRC = "/home/user/libreoffice-core/.claude/worktrees/agent-afd045030560da190/unotools/source/misc/fontcvt.cxx"
OPENSYMBOL = "/usr/share/fonts/truetype/libreoffice/opens___.ttf"

text = open(SRC, encoding="utf-8", errors="replace").read()

def parse_tables(text):
    out = {}
    for m in re.finditer(r'const sal_Unicode (a\w+Tab)\[224\]\s*=\s*\{(.*?)\n\};', text, re.S):
        name, body = m.group(1), m.group(2)
        body = re.sub(r'//[^\n]*', '', body)          # strip // F020 markers
        # Entries are comma-separated; a hole is written as a bare 0, NOT 0x0000.
        # Matching only /0x[0-9a-f]+/ silently drops those and shifts every later index.
        vals = []
        for tok in body.split(','):
            tok = tok.strip()
            if not tok:
                continue
            vals.append(int(tok, 16) if tok.lower().startswith('0x') else int(tok, 10))
        out[name] = vals
    return out

tabs = parse_tables(text)

def cmap(path):
    d = open(path, "rb").read()
    numt = struct.unpack(">H", d[4:6])[0]
    off = None
    for i in range(numt):
        rec = 12 + 16 * i
        if d[rec:rec+4] == b"cmap":
            off = struct.unpack(">I", d[rec+8:rec+12])[0]
    assert off is not None
    n = struct.unpack(">H", d[off+2:off+4])[0]
    best = None
    for i in range(n):
        pid, eid, so = struct.unpack(">HHI", d[off+4+8*i:off+12+8*i])
        if (pid, eid) in ((3, 1), (3, 10), (0, 3), (0, 4)):
            best = off + so
    assert best is not None
    fmt = struct.unpack(">H", d[best:best+2])[0]
    cps = set()
    if fmt == 4:
        segX2 = struct.unpack(">H", d[best+6:best+8])[0]
        seg = segX2 // 2
        ends = struct.unpack(">%dH" % seg, d[best+14:best+14+segX2])
        sp = best + 16 + segX2
        starts = struct.unpack(">%dH" % seg, d[sp:sp+segX2])
        dp = sp + segX2
        deltas = struct.unpack(">%dh" % seg, d[dp:dp+segX2])
        rp = dp + segX2
        ranges = struct.unpack(">%dH" % seg, d[rp:rp+segX2])
        for i in range(seg):
            for c in range(starts[i], min(ends[i], 0xFFFF) + 1):
                if c == 0xFFFF: continue
                if ranges[i] == 0:
                    g = (c + deltas[i]) & 0xFFFF
                else:
                    gi = rp + 2*i + ranges[i] + 2*(c - starts[i])
                    if gi + 2 > len(d): continue
                    g = struct.unpack(">H", d[gi:gi+2])[0]
                    if g: g = (g + deltas[i]) & 0xFFFF
                if g: cps.add(c)
    return cps

cps = cmap(OPENSYMBOL)

for name in sorted(tabs):
    t = tabs[name]
    print("%-28s entries=%d" % (name, len(t)))

print()
print("OpenSymbol cmap: %d code points" % len(cps))
print("  in F000-F0FF : %s" % sorted("%04X" % c for c in cps if 0xF000 <= c <= 0xF0FF))
print("  in E000-E9FF : %d" % len([c for c in cps if 0xE000 <= c <= 0xE9FF]))

# The specific claim: U+F0D8 through wingdings
def recode(tab, ch):
    idx = ch - 0xF000 if ch & 0xFF00 else ch
    if 0x20 <= idx <= 0xFF:
        v = tab[idx - 0x20]
        return v if v else None
    return None

for name, ch in (("aWingDingsTab", 0xF0D8), ("aWingDingsTab", 0xF0A7),
                 ("aWingDingsTab", 0xF06E), ("aAdobeSymbolTab", 0xF0B7),
                 ("aMonotypeSortsTab", 0xF0D8)):
    t = tabs.get(name)
    if not t: continue
    v = recode(t, ch)
    ok = (v in cps) if v else False
    print("%s U+%04X -> %s   inOpenSymbol=%s" %
          (name, ch, ("U+%04X" % v) if v else "(0, none)", ok))

# Coverage: how much of each table lands on a code point OpenSymbol actually has
print()
for name in sorted(tabs):
    t = tabs[name]
    nz = [v for v in t if v]
    hit = [v for v in nz if v in cps]
    print("%-28s non-zero=%3d  present-in-OpenSymbol=%3d  (%.0f%%)"
          % (name, len(nz), len(hit), 100.0*len(hit)/max(1,len(nz))))
