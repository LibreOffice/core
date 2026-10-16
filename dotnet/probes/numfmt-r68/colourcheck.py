"""Before / after / reference, on the drawn colour of every text span.

before = the banked gate's own `ours` PDF at 2f4709c08 (XlsxCellFormats, XlsxPalette and
XlsxTint are byte-identical between that commit and this round's base, so it is a valid
before for the colour question and no rebuild is needed).
"""
import sys, collections
import pymupdf

def counts(path):
    out = collections.Counter()
    for page in pymupdf.open(path):
        for block in page.get_text('dict')['blocks']:
            if block['type'] != 0:
                continue
            for line in block['lines']:
                for span in line['spans']:
                    text = span['text'].strip()
                    if text:
                        out[f"#{span['color']:06x}"] += len(text)
    return out

def agreement(a, b):
    shared = sum((a & b).values())
    total = sum(a.values())
    return shared, total

print(f"{'document':58s} {'before':>13s} {'after':>13s}")
for line in sys.stdin:
    stem, before, after, ref = line.split()
    b, a, r = counts(before), counts(after), counts(ref)
    sb, tb = agreement(b, r)
    sa, ta = agreement(a, r)
    print(f'{stem[:58]:58s} {sb:6d}/{tb:6d} {sa:6d}/{ta:6d}'
          + ('   IMPROVED' if sa > sb else ('   WORSE' if sa < sb else '')))
