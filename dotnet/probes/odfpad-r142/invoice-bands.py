#!/usr/bin/env python3
"""How far each span of one document sits from the reference's, as a histogram.

The mean |dx| is the wrong instrument where a document sits a whole margin out:
`084_Service_invoice`'s every span is 37.7 pt left of the reference's before and
after this round, for reasons no round has touched, and that constant swamps the
change. Banding the differences instead shows what actually moved -- and on this
document it shows the round collapsing EIGHT scattered offsets into THREE.
"""
import sys, pathlib, pymupdf, collections

def spans(path):
    doc = pymupdf.open(path)
    out = {}
    for page, sheet in enumerate(doc):
        for block in sheet.get_text('dict')['blocks']:
            for line in block.get('lines', []):
                for span in line['spans']:
                    text = span['text'].strip()
                    if text:
                        out.setdefault((page, round(span['bbox'][1], 1), text), []) \
                           .append(round(span['bbox'][0], 2))
    return out

def bands(ours, reference):
    counted = collections.Counter()
    for key in set(ours) & set(reference):
        for x, rx in zip(ours[key], reference[key]):
            counted[round(x - rx)] += 1
    return sorted(counted.items())

if __name__ == '__main__':
    before, after, reference = (spans(pathlib.Path(p)) for p in sys.argv[1:4])
    print('before', bands(before, reference))
    print('after ', bands(after, reference))
