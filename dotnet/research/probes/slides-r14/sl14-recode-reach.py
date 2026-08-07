#!/usr/bin/env python3
"""What do the slides track's *actual* symbol bullets recode to, and does OpenSymbol have it?

The census counted documents that state a recodeable face. This counts the concrete
(face, char) pairs those documents ask for, pushes each through fontcvt.cxx's table,
and checks the destination against OpenSymbol's cmap. That is the number that decides
whether porting the table draws glyphs or tofu.
"""
import os, re, sys, zipfile, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util

spec = importlib.util.spec_from_file_location(
    "tabs", os.path.join(os.path.dirname(os.path.abspath(__file__)), "sl14-tabs.py"))

# Re-implement the two helpers rather than exec the probe (it prints).
SRC = "/home/user/libreoffice-core/.claude/worktrees/agent-afd045030560da190/unotools/source/misc/fontcvt.cxx"
OPENSYMBOL = "/usr/share/fonts/truetype/libreoffice/opens___.ttf"
import struct

text = open(SRC, encoding="utf-8", errors="replace").read()
TABS = {}
for m in re.finditer(r'const sal_Unicode (a\w+Tab)\[224\]\s*=\s*\{(.*?)\n\};', text, re.S):
    body = re.sub(r'//[^\n]*', '', m.group(2))
    vals = []
    for tok in body.split(','):
        tok = tok.strip()
        if tok:
            vals.append(int(tok, 16) if tok.lower().startswith('0x') else int(tok, 10))
    TABS[m.group(1)] = vals

FACE_TO_TAB = {
    "starbats": "aStarBatsTab", "starmath": "aStarMathTab",
    "symbol": "aAdobeSymbolTab", "standardsymbols": "aAdobeSymbolTab",
    "standardsymbolsl": "aAdobeSymbolTab",
    "monotypesorts": "aMonotypeSortsTab", "zapfdingbats": "aMonotypeSortsTab",
    "itczapfdingbats": "aMonotypeSortsTab", "dingbats": "aMonotypeSortsTab",
    "webdings": "aWebDingsTab", "wingdings": "aWingDingsTab",
    "wingdings2": "aWingDings2Tab", "wingdings3": "aWingDings3Tab",
    "mtextra": "aMTExtraTab",
}

def cmap(path):
    d = open(path, "rb").read()
    numt = struct.unpack(">H", d[4:6])[0]
    off = None
    for i in range(numt):
        rec = 12 + 16 * i
        if d[rec:rec+4] == b"cmap":
            off = struct.unpack(">I", d[rec+8:rec+12])[0]
    n = struct.unpack(">H", d[off+2:off+4])[0]
    best = None
    for i in range(n):
        pid, eid, so = struct.unpack(">HHI", d[off+4+8*i:off+12+8*i])
        if (pid, eid) in ((3, 1), (3, 10), (0, 3), (0, 4)):
            best = off + so
    cps = set()
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

CPS = cmap(OPENSYMBOL)

def norm(n): return re.sub(r"[^a-z0-9]", "", (n or "").lower())

def recode(face, ch):
    tab = TABS.get(FACE_TO_TAB.get(norm(face), ""))
    if tab is None: return None
    idx = ch - 0xF000 if (ch & 0xFF00) else ch
    if not (0x20 <= idx <= 0xFF): return None
    v = tab[idx - 0x20]
    return v if v else 0          # 0 == a hole in the table

pairs = collections.Counter()
docs = collections.defaultdict(set)
ROOT = "/workspace/sample-files/slides"
for dirpath, _, files in os.walk(ROOT):
    for fn in sorted(files):
        if not fn.lower().endswith((".pptx", ".pptm", ".potx", ".ppsx")):
            continue
        p = os.path.join(dirpath, fn)
        try: z = zipfile.ZipFile(p)
        except Exception: continue
        for n in z.namelist():
            if not n.endswith(".xml"): continue
            try: b = z.read(n).decode("utf-8", "replace")
            except Exception: continue
            # a:buChar and a:buFont are siblings inside one a:pPr. Slice from each
            # opening tag to its matching close; a self-closing a:pPr carries no
            # children and so no bullet, and is skipped by the buChar test below.
            for pm in re.finditer(r'<a:pPr\b', b):
                end = b.find('</a:pPr>', pm.start())
                nxt = b.find('<a:pPr', pm.start() + 6)
                if end < 0: continue
                if nxt >= 0 and nxt < end: continue      # self-closed; no children
                blk = b[pm.start():end]
                f = re.search(r'<a:buFont[^>]*typeface="([^"]*)"', blk)
                c = re.search(r'<a:buChar[^>]*char="([^"]*)"', blk)
                if not (f and c) or not c.group(1): continue
                face = f.group(1)
                if norm(face) not in FACE_TO_TAB: continue
                ch = ord(c.group(1)[0])
                pairs[(face, ch)] += 1
                docs[(face, ch)].add(fn)
        z.close()

print("%-16s %-8s %-10s %-8s %s" % ("face", "char", "recoded", "inOS", "uses/docs"))
tot = hit = hole = miss = 0
for (face, ch), n in sorted(pairs.items(), key=lambda kv: -kv[1]):
    v = recode(face, ch)
    tot += n
    if v is None:
        state = "OUTSIDE"          # char outside 0x20-0xFF: not a symbol slot at all
        dest = "-"
    elif v == 0:
        state, hole = "HOLE", hole + n
        dest = "-"
    elif v in CPS:
        state, hit = "yes", hit + n
        dest = "U+%04X" % v
    else:
        state, miss = "NO", miss + n
        dest = "U+%04X" % v
    print("%-16s U+%04X   %-10s %-8s %d / %d" % (face, ch, dest, state, n, len(docs[(face, ch)])))

print()
print("bullet uses: %d   recode lands on a glyph OpenSymbol has: %d (%.0f%%)   table hole: %d   absent from OpenSymbol: %d"
      % (tot, hit, 100.0*hit/max(1,tot), hole, miss))
