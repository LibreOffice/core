#!/usr/bin/env python3
"""C11: re-render the reference twice and compare the quantity each finding rests on.

    c11-control.py <run-a-dir> <run-b-dir> <ref-bank>

C11 records that the reference is not reproducible run to run on a handful of documents, that
the roster is not fixed, and that part of the variation is length-preserving -- so a character
count is a lower bound and what must be compared is the quantity the finding actually uses.
Here that is, per document: the image placements and their boxes, and the extracted text
itself rather than its length.
"""
import pathlib
import sys

import pymupdf

a, b, bank = sys.argv[1:4]
CASES = {
    '091_Volunteer_Sign_Up_Sheet_Template_Colored_Background_2cb58d01':
        '091_Volunteer_Sign_Up_Sheet_Template_Colored_Background_2cb58d01__xlsx',
    'A1. EASA Form 2': 'A1. EASA Form 2__docx',
    '086_Printable_Graph_Paper_Template_Gray_Theme_7300e5d7':
        '086_Printable_Graph_Paper_Template_Gray_Theme_7300e5d7__docx',
}


def quantity(path):
    d = pymupdf.open(path)
    boxes, text = [], []
    for p in d:
        for i in p.get_image_info():
            boxes.append(tuple(round(v, 2) for v in i['bbox']))
        text.append(p.get_text('text'))
    d.close()
    return boxes, ''.join(text)


print('document\tplacements a/b/bank\tboxes identical\ttext identical')
for stem, banked in CASES.items():
    qa = quantity(pathlib.Path(a) / f'{stem}.pdf')
    qb = quantity(pathlib.Path(b) / f'{stem}.pdf')
    qk = quantity(pathlib.Path(bank) / f'{banked}.pdf')
    print(f'{stem[:44]}\t{len(qa[0])}/{len(qb[0])}/{len(qk[0])}\t'
          f'{qa[0] == qb[0] == qk[0]}\t{qa[1] == qb[1] == qk[1]}')
