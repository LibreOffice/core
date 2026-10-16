#!/usr/bin/env python3
"""Substitute each REF field's result with what 26.2.4.2's RTF import expands it
to, computed by expand.py from the flat-ODT round trip. A test of reach, not a
fix: if the page count moves to the reference's, the expansion is the gap."""
import re, sys, subprocess

src, fodt, out = sys.argv[1], sys.argv[2], sys.argv[3]
d = open(src, 'rb').read().decode('cp1252', 'replace')

names = sorted(set(re.findall(r'\\fldinst\s+REF\s+(\S+)', d)))
rows = subprocess.run([sys.executable, 'expand.py', fodt, *names],
                      capture_output=True, text=True).stdout.splitlines()
exp = dict(r.split('\t', 1) for r in rows if '\t' in r)

def esc(s):
    o = []
    for ch in s:
        if ch == '\n': o.append('\\line ')
        elif ch in '\\{}': o.append('\\' + ch)
        elif ord(ch) < 128: o.append(ch)
        else: o.append('\\u%d\\\'3f' % ord(ch))
    return ''.join(o)

n = 0
def sub(m):
    global n
    name = m.group(1)
    if name not in exp: return m.group(0)
    n += 1
    return '{\\field{\\*\\fldinst  REF %s \\\\h }{\\fldrslt %s}}' % (name, esc(exp[name]))

d2 = re.sub(r'\{\\field\{\\\*\\fldinst\s+REF\s+(\S+)\s*\\\\h\s*\}\{\\fldrslt[^{}]*\}\}', sub, d)
open(out, 'wb').write(d2.encode('cp1252', 'replace'))
print(f"{n} REF results replaced, {len(exp)} bookmarks")
