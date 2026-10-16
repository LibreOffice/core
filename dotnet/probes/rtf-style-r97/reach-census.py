#!/usr/bin/env python3
r"""How many of the 338 converted `.rtf` apply a name whose pool parent this round models.

`hasbyname-census.py` answers *which names reach a Writer style*; this answers the other half the
rulebook asks for — *how many documents apply one*, which is the only figure that can be called a
reach. It reuses that script's readers rather than restating them.

  reach-census.py <writer-names.tsv> <StyleSheetTable.cxx> <corpus root>
"""
import collections
import pathlib
import re
import sys

HERE = pathlib.Path(__file__).parent
source = (HERE / 'hasbyname-census.py').read_text(encoding='utf-8')
exec(source.split('def main()')[0].split('"""', 2)[2])  # noqa: S102 - the shared readers

# `RtfStyles.PoolParentOf`, transcribed. The heading family was already modelled before this
# round; everything else here is what it adds.
BEFORE = re.compile(r'[hH]eading [1-9]|Title|Subtitle')
ADDED_STANDARD = {'header', 'Header', 'footer', 'Footer', 'Heading', 'Comment', 'Signature'}
ADDED_CAPTION = {'Figure', 'Illustration', 'Table', 'Drawing', 'Text'}
ADDED_NUMBERED = re.compile(r'[tT][oO][cC] [1-9]|[iI]ndex [1-3]')


def classify(name):
    name = name.strip()
    if name == 'Normal':
        return None
    if name in ADDED_CAPTION:
        return 'Caption'
    if name in ADDED_STANDARD or ADDED_NUMBERED.fullmatch(name):
        return 'Standard'
    if name == 'Body Text' or BEFORE.fullmatch(name):
        return 'before'
    return None


def main():
    per = collections.Counter()
    where = collections.defaultdict(set)
    added, already, files = set(), set(), 0
    for path in sorted(pathlib.Path(sys.argv[3]).rglob('*.rtf')):
        files += 1
        text = path.read_bytes().decode('cp1252', 'replace')
        sheet, end = entries(text)
        body = text if end < 0 else text[:text.find('{\\stylesheet')] + text[end + 1:]
        used = {int(m.group(1)) for m in USE.finditer(body)}
        seen = set()
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
            kind = classify(name)
            if kind is None:
                continue
            per[name] += 1
            where[name].add(path.name)
            (already if kind == 'before' else added).add(path.name)

    print(f'{files} .rtf scanned')
    print(f'{len(added)} apply a name this round adds; {len(already)} apply one already modelled; '
          f'{len(added | already)} either')
    print()
    for name, count in sorted(per.items(), key=lambda kv: (-kv[1], kv[0])):
        print(f'{count:>5}  {classify(name):<9} {name!r}')
    print()
    for name in sorted(where):
        if classify(name) != 'before':
            print(f'  {name!r}: {", ".join(sorted(where[name]))}')


main()
