#!/usr/bin/env python3
"""Census the drawing shapes anchored in the cells of the converted `.ods` column.

    census.py <corpus-root> [shapes|ink|direct]

  shapes   per element kind: how many sit under a `table:table-cell`, how many of those carry
           a `text:p` at all, how many carry one with ink in it, and in how many documents.
           The walk descends through `draw:g` and `draw:a`, which are transparent wrappers
           with no rectangle of their own.
  ink      one line per document holding an inked shape, with the kinds and paragraph count.
  direct   the same counts over a cell's *direct* children only, which is what round 77's
           census did — kept so the two are comparable.

The dedup is by inode because the mount is case-insensitive and reports one file twice.
"""
import os
import sys
import zipfile
from xml.etree import ElementTree as ET

DRAW = '{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
TABLE = '{urn:oasis:names:tc:opendocument:xmlns:table:1.0}'
TEXT = '{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'


def documents(root):
    seen, found = set(), []
    for directory, _, names in os.walk(root):
        for name in names:
            if not name.lower().endswith('.ods'):
                continue
            path = os.path.join(directory, name)
            key = os.stat(path).st_dev, os.stat(path).st_ino
            if key in seen:
                continue
            seen.add(key)
            found.append(path)
    return sorted(found)


def content(path):
    with zipfile.ZipFile(path) as package:
        return ET.fromstring(package.read('content.xml'))


def cells(tree):
    for element in tree.iter():
        if element.tag in (TABLE + 'table-cell', TABLE + 'covered-table-cell'):
            yield element


def shapes(container, out, wrappers=True):
    for child in container:
        if not child.tag.startswith(DRAW):
            continue
        local = child.tag[len(DRAW):]
        if wrappers and local in ('g', 'a'):
            shapes(child, out)
        else:
            out.append((local, child))


def inked(element):
    paragraphs = element.findall('.//' + TEXT + 'p')
    text = ''.join(''.join(p.itertext()) for p in paragraphs)
    return len(paragraphs), text.strip()


def main():
    root = sys.argv[1]
    mode = sys.argv[2] if len(sys.argv) > 2 else 'shapes'
    total, withtext, withink, chars, docs = {}, {}, {}, {}, {}
    perdoc = {}

    for path in documents(root):
        try:
            tree = content(path)
        except Exception as error:                                    # noqa: BLE001
            print('SKIP', path, error, file=sys.stderr)
            continue

        for cell in cells(tree):
            found = []
            shapes(cell, found, wrappers=(mode != 'direct'))
            for kind, element in found:
                total[kind] = total.get(kind, 0) + 1
                docs.setdefault(kind, set()).add(path)
                paragraphs, text = inked(element)
                if paragraphs:
                    withtext[kind] = withtext.get(kind, 0) + 1
                if text:
                    withink[kind] = withink.get(kind, 0) + 1
                    chars[kind] = chars.get(kind, 0) + len(text)
                    perdoc.setdefault(path, []).append((kind, paragraphs))

    if mode == 'ink':
        print('%d documents hold an inked shape in a cell' % len(perdoc))
        for path in sorted(perdoc, key=lambda p: -len(perdoc[p])):
            entries = perdoc[path]
            print('%4d shapes %5d paras  %-16s %s'
                  % (len(entries), sum(n for _, n in entries),
                     ','.join(sorted({k for k, _ in entries})),
                     os.path.relpath(path, root)))
        return

    print('%-16s %7s %9s %8s %9s %6s'
          % ('kind', 'count', 'with p', 'with ink', 'chars', 'docs'))
    for kind in sorted(total, key=lambda k: -total[k]):
        print('%-16s %7d %9d %8d %9d %6d'
              % (kind, total[kind], withtext.get(kind, 0), withink.get(kind, 0),
                 chars.get(kind, 0), len(docs[kind])))
    print('# %d documents scanned' % len(documents(root)), file=sys.stderr)


main()
