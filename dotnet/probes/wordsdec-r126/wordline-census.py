#!/usr/bin/env python3
"""Census: does IsWordLineMode() (w:u val="words" / \\ulw / ODF skip-white-space /
sprmCKul word operands) occur in the corpus at all?

Three legs, because the four word-processing families state it four ways:
  DOCX/DOCM  w:u w:val="words"|"wavyHeavy"? no -- exactly "words", and w:em? no.
  RTF        \\ulw (and \\ulwave is a DIFFERENT control word -- prefix trap, r123 4.1)
  WW8 .doc   sprmCKul operand 4 (kulWord)
  ODF        style:text-underline-mode / -overline-mode / -line-through-mode
             = "skip-white-space"
Counted over the 947-document corpus and over the converted-ODF corpus.
"""
import re, sys, zipfile, csv
from pathlib import Path

CORPUS = Path('/home/user/sample-files')
ODF = Path('/home/user/corpus-odf')

def manifest():
    rows = []
    with (CORPUS / 'MANIFEST.tsv').open() as fh:
        r = csv.DictReader(fh, delimiter='\t')
        for row in r:
            rows.append(row)
    return rows

def zip_parts(path, pattern):
    try:
        with zipfile.ZipFile(path) as z:
            for name in z.namelist():
                if pattern.search(name):
                    try:
                        yield name, z.read(name)
                    except Exception:
                        pass
    except Exception:
        pass

WORD_PART = re.compile(r'^word/.*\.xml$')
U_WORDS = re.compile(rb'<w:u\b[^>]*w:val="words"')
U_ANY = re.compile(rb'<w:u\b[^>]*w:val="(?!none")')
ODF_MODE = re.compile(rb'style:text-(underline|overline|line-through)-mode="skip-white-space"')

def main():
    rows = manifest()
    words = [r for r in rows if r['family'] == 'words']
    hits = {'docx-words': [], 'doc-kulword': [], 'odf-skip': [], 'rtf-ulw': []}
    base = {'docx-u': []}
    for r in words:
        p = CORPUS / r['path']
        ext = r['ext'].lower()
        if ext in ('docx', 'docm', 'dotx', 'dotm', 'xlsm'):
            n = 0
            anyu = 0
            for name, data in zip_parts(p, WORD_PART):
                n += len(U_WORDS.findall(data))
                anyu += len(U_ANY.findall(data))
            if n:
                hits['docx-words'].append((r['path'], n))
            if anyu:
                base['docx-u'].append((r['path'], anyu))
        elif ext == 'doc':
            # sprmCKul is 0x2A3E in WW8; operand is one byte following.
            data = p.read_bytes()
            n = 0
            for m in re.finditer(rb'\x3e\x2a', data):
                op = data[m.end():m.end()+1]
                if op == b'\x02':
                    n += 1
            if n:
                hits['doc-kulword'].append((r['path'], n))
    # ODF converted corpus: odt + rtf columns
    if ODF.exists():
        for p in sorted(ODF.rglob('*.odt')):
            n = 0
            for name, data in zip_parts(p, re.compile(r'^(content|styles)\.xml$')):
                n += len(ODF_MODE.findall(data))
            if n:
                hits['odf-skip'].append((str(p.relative_to(ODF)), n))
        for p in sorted(ODF.rglob('*.rtf')):
            data = p.read_bytes()
            # \ulw not followed by a letter (so \ulwave does not match)
            n = len(re.findall(rb'\\ulw(?![a-zA-Z])', data))
            if n:
                hits['rtf-ulw'].append((str(p.relative_to(ODF)), n))
    for k, v in hits.items():
        print(f'{k}\tdocuments={len(v)}\tstatements={sum(n for _, n in v)}')
        for path, n in v[:10]:
            print(f'  {n}\t{path}')
    for k, v in base.items():
        print(f'BASE {k}\tdocuments={len(v)}\tstatements={sum(n for _, n in v)}')

if __name__ == '__main__':
    main()
