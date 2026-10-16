#!/usr/bin/env python3
r"""Census `style:text-scale` (any namespace prefix) over a directory of ODF packages.

    census-odf.py <dir-of-*.odt|*.ods|*.odp> [ext]

Every ODF attribute is grepped BY LOCAL NAME and bucketed by the prefix that carried it,
because `dotnet/CLAUDE.md` records six attributes LibreOffice's own exporter writes in a
namespace the specification does not put them in.  Both `content.xml` and `styles.xml` are
read: a character width stated by a NAMED style lives in the second and is exactly the shape
round 143's witness had in DOCX.

Base rates per document: `<style:text-properties>` (every place a run property can be stated),
`<text:span>` (character runs), `<text:p>` + `<text:h>` (paragraphs).
"""
import collections, pathlib, re, sys, zipfile

SCALE = re.compile(rb'(?:([A-Za-z0-9_-]+):)?text-scale\s*=\s*"([^"]*)"')
PROPS = re.compile(rb'<style:text-properties')
SPAN = re.compile(rb'<text:span[\s>]')
PARA = re.compile(rb'<text:(?:p|h)[\s/>]')
MEMBERS = ('content.xml', 'styles.xml')

root = pathlib.Path(sys.argv[1])
ext = sys.argv[2] if len(sys.argv) > 2 else root.name

print('document\tmember\tprefix\tvalue\tcount')
tot = collections.Counter()
values = collections.Counter()
prefixes = collections.Counter()
docs_any = set()
docs_nonunit = set()
base = collections.Counter()
nfiles = 0
for f in sorted(root.glob('*.' + ext)):
    nfiles += 1
    try:
        z = zipfile.ZipFile(f)
    except Exception as exc:
        print('%s\tUNREADABLE\t%s\t\t0' % (f.name, exc))
        continue
    with z:
        for member in MEMBERS:
            try:
                blob = z.read(member)
            except KeyError:
                continue
            base['props'] += len(PROPS.findall(blob))
            base['span'] += len(SPAN.findall(blob))
            base['para'] += len(PARA.findall(blob))
            hits = collections.Counter()
            for prefix, value in SCALE.finditer(blob):
                # `text-scale-minimum`/`-maximum` cannot match: the regex requires `=` next.
                p = (prefix or b'').decode()
                v = value.decode()
                hits[(p, v)] += 1
                prefixes[p] += 1
                values[v] += 1
                tot['occurrences'] += 1
                docs_any.add(f.name)
                if v.strip().rstrip('%') not in ('100', '100.0'):
                    tot['non_unit_occurrences'] += 1
                    docs_nonunit.add(f.name)
            for (p, v), n in sorted(hits.items()):
                print('%s\t%s\t%s\t%s\t%d' % (f.name, member, p or '(none)', v, n))

print('\n# files\t%d' % nfiles)
print('# occurrences\t%d' % tot['occurrences'])
print('# occurrences != 100%%\t%d' % tot['non_unit_occurrences'])
print('# documents stating it\t%d' % len(docs_any))
print('# documents stating != 100%%\t%d' % len(docs_nonunit))
print('# base style:text-properties\t%d' % base['props'])
print('# base text:span\t%d' % base['span'])
print('# base text:p + text:h\t%d' % base['para'])
print('# prefixes\t%s' % dict((k or '(none)', v) for k, v in prefixes.items()))
print('# values\t%s' % dict(values.most_common()))
