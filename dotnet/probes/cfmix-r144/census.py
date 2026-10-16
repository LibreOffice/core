"""Two censuses this round needs and one it does not: how many `patternFill` in the corpus
state a hatch, and — the figure that matters — how many of those a *cell* can actually take.

A `dxf` is counted whenever it exists, because a `cfRule` naming it is what paints it. A
`fills` entry is not: Excel writes `<patternFill patternType="gray125"/>` into index 1 of
every workbook it saves, so counting the element finds one per file and says nothing. The
question is whether any `cellXfs` entry names it, which is what `XlsxCellDecoration` reads."""
import collections
import os
import re
import zipfile

CORPUS = '/home/user/sample-files'
FILL = re.compile(r'<fill>.*?</fill>|<fill/>', re.S)
TYPE = re.compile(r'patternType="([^"]+)"')
XF = re.compile(r'<xf\b[^>]*>')
FILLID = re.compile(r'fillId="(\d+)"')
DXF = re.compile(r'<dxf>.*?</dxf>', re.S)


def section(styles, name):
    """The text between `<name` and `</name>`, or '' — the parts are flat enough for this."""
    start = styles.find('<' + name)
    end = styles.find('</' + name + '>')
    return styles[start:end] if start >= 0 and end > start else ''


def main():
    dxf_hatch = collections.Counter()
    cell_hatch = collections.Counter()
    used_hatch = collections.Counter()
    dxf_docs, cell_docs, used_docs = set(), set(), set()
    workbooks = 0

    for root, _, names in os.walk(CORPUS):
        for name in names:
            if not name.lower().endswith(('.xlsx', '.xlsm', '.xls')):
                continue
            path = os.path.join(root, name)
            try:
                archive = zipfile.ZipFile(path)
            except Exception:
                continue                       # a real BIFF file, read by another reader
            if 'xl/styles.xml' not in archive.namelist():
                continue
            workbooks += 1
            styles = archive.read('xl/styles.xml').decode('utf8', 'replace')

            for kind in TYPE.findall(section(styles, 'dxfs')):
                if kind not in ('none', 'solid'):
                    dxf_hatch[kind] += 1
                    dxf_docs.add(path)

            fills = FILL.findall(section(styles, 'fills'))
            hatched = {}
            for index, fill in enumerate(fills):
                for kind in TYPE.findall(fill):
                    if kind not in ('none', 'solid'):
                        hatched[index] = kind
                        cell_hatch[kind] += 1
                        cell_docs.add(path)

            if not hatched:
                continue
            for xf in XF.findall(section(styles, 'cellXfs')):
                stated = FILLID.search(xf)
                if stated and int(stated.group(1)) in hatched:
                    used_hatch[hatched[int(stated.group(1))]] += 1
                    used_docs.add(path)

    print('OPC workbooks scanned: %d' % workbooks)
    print('dxf hatches: %d in %d documents %s'
          % (sum(dxf_hatch.values()), len(dxf_docs), dict(dxf_hatch)))
    print('fills hatches: %d in %d documents %s'
          % (sum(cell_hatch.values()), len(cell_hatch and cell_docs), dict(cell_hatch)))
    print('cellXfs naming one: %d in %d documents %s'
          % (sum(used_hatch.values()), len(used_docs), dict(used_hatch)))
    for path in sorted(used_docs):
        print('   ', path[len(CORPUS) + 1:])


if __name__ == '__main__':
    main()
