#!/usr/bin/env python3
r"""Every paragraph-style *name* an `.rtf` applies without a resolvable `\sbasedon`.

`probes/rtf-bookmark-r88/poolcensus.py` asks the same question of a fixed list of five names.
This asks it of all of them, because the open issue is not "does `Body Text` appear" but *how
many names reach the `COLL_STANDARD` branch of `GetPoolParent` at all* — a style whose
`ConvertStyleName` (`StyleSheetTable.cxx`:1620-1660) name is one Writer already has keeps that
style's pool parent, and where the parent is `COLL_STANDARD` what the paragraph inherits is the
document's own `Normal` entry rather than a constant.

The selection rule is r88's, unchanged and for its reason: a style counts as *used* only when a
`\sN` outside the `{\stylesheet}` group names it, because each entry's own `\sN` would otherwise
make every declared style look used.

  standard-census.py <corpus root>

Prints one row per name, most-used first, with the documents that apply it.
"""
import collections
import pathlib
import re
import sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')
USE = re.compile(r'\\s(\d+)(?![0-9])')


def entries(text):
    r"""Each `{...}` directly inside `{\stylesheet ...}`, and where that group ends."""
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


def main():
    documents = collections.Counter()
    entry_count = collections.Counter()
    where = collections.defaultdict(set)
    files = 0

    for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
        files += 1
        text = path.read_bytes().decode('cp1252', 'replace')
        sheet, end = entries(text)
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
            here.add(name)
            entry_count[name] += 1
            where[name].add(path.name)
        for name in here:
            documents[name] += 1

    print(f'{files} .rtf scanned')
    print(f'{len(documents)} distinct names applied without a resolvable \\sbasedon')
    print(f'{"documents":>10}{"entries":>9}  name')
    for name, count in sorted(documents.items(), key=lambda kv: (-kv[1], kv[0])):
        print(f'{count:>10}{entry_count[name]:>9}  {name!r}')
    print()
    for name, count in sorted(documents.items(), key=lambda kv: (-kv[1], kv[0])):
        if count <= 4:
            print(f'  {name!r}: {", ".join(sorted(where[name]))}')


main()
