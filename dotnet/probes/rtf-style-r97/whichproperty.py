#!/usr/bin/env python3
r"""For each candidate (document, style) pair, which property the pool parent newly supplies.

`reach-sweep.py` is the measurement -- this only *explains* it, by applying `ContributionOf`'s
own-beats-inherited rule by hand to the entry's control words. A property changes when the entry
states nothing for it and the document's `Normal` states something, and nothing changes otherwise;
that is why twelve documents apply a name this round adds and four renderings move.

The parent is `Normal` for a `RtfPoolParent.Standard` name and the document's own `caption` entry
laid over `Normal` for a `RtfPoolParent.Caption` one -- which is not a detail: `bulletin`'s `Text`
entry gains **nothing** from `Normal` and gains the bold that makes it a mover from the `caption`
entry, so a version of this script that compares against `Normal` alone explains three of the four
movers and misses the fourth.

It is a reimplementation of the model and therefore evidence about the model, not about the
reference. Read it beside the sweep, never instead of it.

  whichproperty.py <corpus root>
"""
import pathlib
import re
import sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')
USE = re.compile(r'\\s(\d+)(?![0-9])')

# The members `RtfStyleFormatting` carries that a pool parent can supply, and the control words
# that state them. `\sa` is absent on purpose: no RTF `\sa` is ever read into the type.
PROPS = {
    'size': re.compile(r'\\fs\d'),
    'bold': re.compile(r'\\b\d?(?![a-zA-Z0-9])'),
    'italic': re.compile(r'\\i\d?(?![a-zA-Z0-9])'),
    'underline': re.compile(r'\\ul(?![a-zA-Z])|\\ulnone'),
    'colour': re.compile(r'\\cf\d'),
    'align': re.compile(r'\\q[lcrj](?![a-zA-Z])'),
    'spacebefore': re.compile(r'\\sb\d'),
}
CAPTION = {'Figure', 'Illustration', 'Table', 'Drawing', 'Text'}
NAMES = {'header', 'Header', 'footer', 'Footer', 'toc 1', 'toc 2', 'toc 3', 'TOC 1',
         'Heading', 'Comment', 'Signature', 'Index 1', 'Index 2', 'Index 3'} | CAPTION


def entries(text):
    start = text.find('{\\stylesheet')
    if start < 0:
        return [], -1
    depth, opened, out = 0, None, []
    for index in range(start, len(text)):
        char = text[index]
        if char == '{':
            depth += 1
            if depth == 2:
                opened = index
        elif char == '}':
            if depth == 2 and opened is not None:
                out.append(text[opened:index + 1])
                opened = None
            depth -= 1
            if depth == 0:
                return out, index
    return out, len(text)


def name_of(entry):
    body = re.sub(r'\{[^{}]*\}', '', entry[1:-1])
    last = None
    for match in CW.finditer(body):
        last = match
    return (body[last.end():] if last else body).strip().rstrip(';').strip()


def stated(entry):
    return {key for key, pattern in PROPS.items() if pattern.search(entry)}


for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    text = path.read_bytes().decode('cp1252', 'replace')
    sheet, end = entries(text)
    body = text if end < 0 else text[:text.find('{\\stylesheet')] + text[end + 1:]
    used = {int(m.group(1)) for m in USE.finditer(body)}
    seen, normal, caption, rows = set(), set(), None, []
    for entry in sheet:
        match = re.match(r'\{\\(\*\\)?(s|cs|ts)(\d+)?\b', entry)
        kind = match.group(2) if match else 's'
        sid = int(match.group(3)) if match and match.group(3) else 0
        based = re.search(r'\\sbasedon(\d+)\b', entry)
        resolves = bool(based) and int(based.group(1)) != sid and int(based.group(1)) in seen
        seen.add(sid)
        if kind != 's':
            continue
        if sid == 0:
            normal = stated(entry)
        if name_of(entry) in ('caption', 'Caption'):
            caption = stated(entry)          # the last such entry wins, as ApplyStyleSheets does
        if resolves or sid not in used or name_of(entry) not in NAMES:
            continue
        rows.append((sid, name_of(entry), stated(entry)))
    for sid, name, own in rows:
        parent = normal | caption if name in CAPTION and caption is not None else normal
        gained = sorted(parent - own)
        print(f'{path.name}\ts{sid}\t{name}\t'
              + (','.join(gained) if gained else '(nothing)'))
