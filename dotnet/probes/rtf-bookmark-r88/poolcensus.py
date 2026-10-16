#!/usr/bin/env python3
r"""How many .rtf hold a paragraph style whose name is one Writer answers, whose
\sbasedon does not resolve, and which a paragraph actually uses.

Round 87's census of the same thing is `probes/rtf-holdover-r87/poolcensus.py`, and it
takes its "used" set from the whole file -- which counts each stylesheet entry's own
`\sN` as a paragraph using it, so every declared style looks used. Excluding the
stylesheet is what separates a style a document styles a paragraph with from one it
merely declares: `Technical_Issue_Report_Form.rtf` declares `Title` at `\s667` and no
paragraph names it, and both of that round's Title/Subtitle candidates are that case.

  poolcensus.py <corpus root> [name ...]
"""
import collections
import pathlib
import re
import sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')
USE = re.compile(r'\\s(\d+)(?![0-9])')

DEFAULT_NAMES = ['Title', 'Subtitle', 'Body Text', 'caption', 'Caption'] + \
                [f'{h}eading {n}' for h in 'hH' for n in range(1, 10)]


def entries(text):
    """Each `{...}` directly inside `{\\stylesheet ...}`, and where that group ends."""
    start = text.find('{\\stylesheet')
    if start < 0:
        return [], -1
    depth = 0
    opened = None
    out = []
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


names = sys.argv[2:] or DEFAULT_NAMES
documents = collections.Counter()
entry_count = collections.Counter()
hits = {}
files = 0

for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    files += 1
    text = path.read_bytes().decode('cp1252', 'replace')
    sheet, end = entries(text)

    # A style is *used* when a `\sN` names it outside the stylesheet group.
    body = text if end < 0 else text[:text.find('{\\stylesheet')] + text[end + 1:]
    used = {int(m.group(1)) for m in USE.finditer(body)}

    seen = set()
    here = set()
    for entry in sheet:
        match = re.match(r'\{\\(\*\\)?(s|cs|ts)(\d+)?\b', entry)
        kind = match.group(2) if match else 's'
        sid = int(match.group(3)) if match and match.group(3) else 0
        based = re.search(r'\\sbasedon(\d+)\b', entry)
        resolves = bool(based) and int(based.group(1)) != sid and int(based.group(1)) in seen
        seen.add(sid)
        if kind != 's' or resolves or sid not in used:
            continue
        name = name_of(entry)
        if name in names:
            here.add(name)
            entry_count[name] += 1
            hits.setdefault(path.name, set()).add(name)
    for name in here:
        documents[name] += 1

print(f"{files} files; {len(hits)} hold one of these styles, unresolved and used")
for name, count in sorted(documents.items()):
    print(f"  {name}: {count} documents, {entry_count[name]} entries")
print()
for document, found in sorted(hits.items()):
    print(f"  {document}: {', '.join(sorted(found))}")
