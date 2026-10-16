#!/usr/bin/env python3
"""How many corpus DOCX use two `w:num` over one `w:abstractNumId`?

Those are the documents where the counters part company: Writer runs them as one list and a
reader keyed on the `w:numId` runs them as two.  A `w:num` nobody uses cannot matter, so the
count is of instances actually named -- by a paragraph's `w:numPr` or by a paragraph style's.
"""
import re, sys, zipfile, pathlib

NUM = re.compile(r'<w:num w:numId="(\d+)"[^>]*>\s*<w:abstractNumId w:val="(\d+)"/>')
USED = re.compile(r'<w:numId w:val="(\d+)"\s*/>')


def main(root):
    total = withshare = 0
    rows = []
    for p in sorted(pathlib.Path(root).rglob('*')):
        if p.suffix.lower() not in ('.docx', '.docm'):
            continue
        total += 1
        try:
            with zipfile.ZipFile(p) as z:
                names = set(z.namelist())
                if 'word/numbering.xml' not in names:
                    continue
                numbering = z.read('word/numbering.xml').decode('utf-8', 'replace')
                used = set()
                for part in ('word/document.xml', 'word/styles.xml'):
                    if part in names:
                        used |= set(USED.findall(z.read(part).decode('utf-8', 'replace')))
        except Exception as exc:
            print(f'SKIP\t{p.name}\t{exc}', file=sys.stderr)
            continue

        groups = {}
        for numId, abstractId in NUM.findall(numbering):
            if numId in used:
                groups.setdefault(abstractId, []).append(numId)

        shared = {a: ids for a, ids in groups.items() if len(ids) > 1}
        if shared:
            withshare += 1
            rows.append((max(len(ids) for ids in shared.values()), len(shared), p.name))

    rows.sort(reverse=True)
    for widest, count, name in rows:
        print(f'{widest}\t{count}\t{name}')
    print(f'# {total} documents, {withshare} where a used abstract carries more than one used '
          f'instance')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
