#!/usr/bin/env python3
"""Dump each top-level w:tbl's indent-relevant properties, in document order."""
import sys
from xml.etree import ElementTree as ET

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'


def a(el, name):
    return el.get(W + name) if el is not None else None


def child(el, name):
    return el.find(W + name) if el is not None else None


def main(path, stylespath):
    doc = ET.parse(path).getroot()
    body = doc.find(W + 'body')
    styles = ET.parse(stylespath).getroot()
    stymap = {}
    for st in styles.findall(W + 'style'):
        if st.get(W + 'type') == 'table':
            stymap[st.get(W + 'styleId')] = st

    n = 0
    for tbl in body.findall(W + 'tbl'):
        n += 1
        pr = child(tbl, 'tblPr')
        ind = child(pr, 'tblInd')
        style = a(child(pr, 'tblStyle'), 'val')
        cellmar = child(pr, 'tblCellMar')
        borders = child(pr, 'tblBorders')
        # style-level
        sty = stymap.get(style)
        styPr = child(sty, 'tblPr') if sty is not None else None
        styInd = child(styPr, 'tblInd')
        styCellMar = child(styPr, 'tblCellMar')
        styBorders = child(styPr, 'tblBorders')
        # first row/cell
        tr = tbl.find(W + 'tr')
        tc = tr.find(W + 'tc') if tr is not None else None
        tcPr = child(tc, 'tcPr')
        tcMar = child(tcPr, 'tcMar')
        tcBorders = child(tcPr, 'tcBorders')

        def marleft(m):
            if m is None:
                return None
            L = child(m, 'left') if child(m, 'left') is not None else child(m, 'start')
            return a(L, 'w') if L is not None else None

        def bl(b):
            if b is None:
                return None
            L = child(b, 'left') if child(b, 'left') is not None else child(b, 'start')
            if L is None:
                return None
            return f"{a(L,'val')}/sz={a(L,'sz')}"

        print(f"tbl{n:2d} style={style} tblInd={a(ind,'w')}({a(ind,'type')}) "
              f"styInd={a(styInd,'w')}({a(styInd,'type')}) "
              f"cellMarL={marleft(cellmar)} styCellMarL={marleft(styCellMar)} "
              f"tcMarL={marleft(tcMar)} "
              f"tblBdrL={bl(borders)} styBdrL={bl(styBorders)} tcBdrL={bl(tcBorders)}")


main(sys.argv[1], sys.argv[2])
