#!/usr/bin/env python3
"""From a flat-ODT round trip of an RTF, report what SwGetRefField expands each
REF-to-a-bookmark to.

reffld.cxx:604-607   ReferencesSubtype::Bookmark: nStart = nNumStart;
                     nEnd = nNumEnd < 0 ? nLen : nNumEnd
reffld.cxx:585-589   FindAnchor sets *pEnd = -1 when the bookmark's two ends are
                     in different nodes, so the expansion runs to the end of the
                     paragraph the start is in.
"""
import sys, xml.parsers.expat

TEXT = 'urn:oasis:names:tc:opendocument:xmlns:text:1.0'

class Walk:
    def __init__(self):
        self.para = 0            # id of the paragraph currently open
        self.stack = []          # nested paragraphs (a frame inside a paragraph)
        self.buf = []            # (id, chunks)
        self.text = {}           # id -> text
        self.starts = {}
        self.ends = {}

    def cur(self):
        return self.buf[-1] if self.buf else None

    def add(self, s):
        c = self.cur()
        if c is not None and s:
            c[1].append(s)

    def off(self):
        c = self.cur()
        return sum(len(x) for x in c[1]) if c is not None else 0

    def start(self, name, attrs):
        ns, _, local = name.rpartition(' ')
        if ns != TEXT:
            return
        if local in ('p', 'h'):
            self.para += 1
            self.buf.append([self.para, []])
        elif local == 'bookmark-start':
            if self.cur(): self.starts[attrs.get(TEXT + ' name')] = (self.cur()[0], self.off())
        elif local == 'bookmark-end':
            if self.cur(): self.ends[attrs.get(TEXT + ' name')] = (self.cur()[0], self.off())
        elif local == 'bookmark':
            if self.cur():
                n = attrs.get(TEXT + ' name')
                self.starts[n] = (self.cur()[0], self.off()); self.ends[n] = (self.cur()[0], self.off())
        elif local == 's':
            self.add(' ' * int(attrs.get(TEXT + ' c', 1)))
        elif local == 'tab':
            self.add('\t')
        elif local == 'line-break':
            self.add('\n')

    def end(self, name):
        ns, _, local = name.rpartition(' ')
        if ns == TEXT and local in ('p', 'h'):
            c = self.buf.pop()
            self.text[c[0]] = ''.join(c[1])

    def chars(self, data):
        self.add(data)

w = Walk()
p = xml.parsers.expat.ParserCreate(namespace_separator=' ')
p.StartElementHandler = w.start
p.EndElementHandler = w.end
p.CharacterDataHandler = w.chars
p.buffer_text = True
with open(sys.argv[1], 'rb') as fh:
    p.ParseFile(fh)

names = sys.argv[2:]
for n in names or sorted(w.starts):
    if n not in w.starts:
        print(f"{n}\t<no start>"); continue
    pid, s = w.starts[n]
    body = w.text.get(pid, '')
    if n in w.ends and w.ends[n][0] == pid:
        e = w.ends[n][1]
    else:
        e = len(body)
    print(f"{n}\t{body[s:e]}")
