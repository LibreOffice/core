#!/usr/bin/env python3
"""Count the positioned-table and cell-text-flow control words in a tree of RTF.

    census.py [corpus-root]

Counts occurrences and the documents holding them, and — for the row-position family — how
many `\\trowd` definitions state at least one word LibreOffice actually dispatches into
`w:tblpPr`, against how many state only a word its tokeniser recognises and then drops.
"""
import collections
import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/corpus-odf')

# The words `RTFDocumentImpl::dispatchFloatingTableFlag` and `dispatchValue` write into
# `LN_CT_TblPrBase_tblpPr` — the whole of what makes a row's table positioned.
POSITIONING = ['tpvpara', 'tpvmrg', 'tpvpg', 'tphcol', 'tphmrg', 'tphpg',
               'tposyc', 'tposyb', 'tposxc', 'tposxr', 'tposx', 'tposy',
               'tdfrmtxtLeft', 'tdfrmtxtRight', 'tdfrmtxtTop', 'tdfrmtxtBottom']

# Tokenised by `rtftokenizer.cxx` and then dispatched nowhere at all: they reach no `tblpPr`,
# so a row stating only one of these is an ordinary table in the flow.
DROPPED = ['tposxl', 'tposxi', 'tposxo', 'tposyt', 'tposyil', 'tposyin', 'tposyout',
           'tposnegx', 'tposnegy']

FLOW = ['cltxlrtb', 'cltxtbrl', 'cltxbtlr', 'cltxlrtbv', 'cltxtbrlv']

WORD = re.compile(rb'\\([A-Za-z]+)')


def main() -> None:
    files = sorted(ROOT.rglob('*.rtf'))
    occurrences: collections.Counter[str] = collections.Counter()
    documents: collections.Counter[str] = collections.Counter()
    rows = positioned = dropped_only = 0

    for path in files:
        data = path.read_bytes()
        seen = {m.group(1).decode('ascii') for m in WORD.finditer(data)}
        for name in POSITIONING + DROPPED + FLOW:
            n = len(re.findall(rb'\\' + name.encode() + rb'(?![A-Za-z])', data))
            if n:
                occurrences[name] += n
                documents[name] += 1
        del seen

        # Row definitions: `\trowd` opens one and `\row` closes it.
        for chunk in re.split(rb'\\trowd(?![A-Za-z])', data)[1:]:
            body = re.split(rb'\\row(?![A-Za-z])', chunk)[0]
            names = {m.group(1).decode('ascii') for m in WORD.finditer(body)}
            rows += 1
            if names & set(POSITIONING):
                positioned += 1
            elif names & set(DROPPED):
                dropped_only += 1

    print(f'{len(files)} documents, {rows} row definitions')
    print(f'{positioned} row definitions state a word that reaches w:tblpPr')
    print(f'{dropped_only} state only a word LibreOffice drops')
    print()
    print(f'{"word":16} {"occurrences":>12} {"documents":>10}')
    for name in POSITIONING + DROPPED + FLOW:
        if occurrences[name]:
            print(f'{name:16} {occurrences[name]:12} {documents[name]:10}')


if __name__ == '__main__':
    main()
