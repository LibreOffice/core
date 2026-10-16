#!/usr/bin/env python3
"""Multiset difference of two PDFs' `pdftotext` tokens.

The gate counts alphanumeric characters out of `pdftotext`, so this is the same
channel the verdict is decided on — unlike PyMuPDF's `get_text`, which clips to the
page and therefore cannot see text drawn outside it.
"""
import collections, re, subprocess, sys

TOK = re.compile(r'\w+', re.UNICODE)
ALNUM = re.compile(r'[^\W_]', re.UNICODE)


def text(p):
    return subprocess.run(['pdftotext', p, '-'], capture_output=True, text=True).stdout


def main(ours, ref, limit=30):
    a, b = text(ours), text(ref)
    ca, cb = collections.Counter(TOK.findall(a)), collections.Counter(TOK.findall(b))
    print(f'alnum ours {len(ALNUM.findall(a))} ref {len(ALNUM.findall(b))}')
    for name, c in (('ONLY OURS', ca - cb), ('ONLY REF', cb - ca)):
        items = sorted(c.items())
        chars = sum(len(ALNUM.findall(t)) * n for t, n in items)
        print(f'--- {name}  {sum(c.values())} tokens, {chars} alnum')
        for t, n in items[:limit]:
            print(f'   x{n} {t!r}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 30)
