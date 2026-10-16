#!/usr/bin/env python3
"""Filled and stroked path counts for a set of documents, three ways.

    marks-table.py <after-dir> <base-dir> <ref-dir> <id> [<id> ...]

`<id>` is `batch-check.sh`'s identity, `<stem>__<ext>`. `after-dir` holds PDFs rendered by the
tree under test (named `<stem>.pdf`); `base-dir` and `ref-dir` are banks named `<id>.pdf`.

A missing file is reported rather than counted as zero, because a zero here reads exactly like
the defect under test — the guard `dotnet/CLAUDE.md` asks for after a round produced a clean,
confident, entirely fabricated match from a glob that had picked up the wrong PDF.
"""
import os
import sys
import pymupdf


def counts(path):
    if not os.path.exists(path):
        return None
    doc = pymupdf.open(path)
    f = s = 0
    for page in doc:
        for d in page.get_drawings():
            if d['type'] in ('f', 'fs'):
                f += 1
            if d['type'] in ('s', 'fs'):
                s += 1
    return doc.page_count, f, s


def main(after, base, ref, idents):
    print(f'{"document":58s} {"pages b/a/ref":>14s} {"fills b/a/ref":>18s} {"strokes b/a/ref":>20s}')
    for ident in idents:
        stem = ident.rsplit('__', 1)[0]
        a = counts(os.path.join(after, stem + '.pdf'))
        b = counts(os.path.join(base, ident + '.pdf'))
        r = counts(os.path.join(ref, ident + '.pdf'))
        if a is None or r is None:
            print(f'{ident[:58]:58s}  MISSING '
                  f'{"after" if a is None else ""} {"ref" if r is None else ""}')
            continue
        z = b or ('-', '-', '-')
        print(f'{ident[:58]:58s} {z[0]:>4}/{a[0]:<4}/{r[0]:<4} '
              f'{z[1]:>6}/{a[1]:<6}/{r[1]:<6} {z[2]:>6}/{a[2]:<6}/{r[2]:<6}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:])
