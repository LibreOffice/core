#!/usr/bin/env python3
"""Row-height ranges of an ODF spreadsheet (packaged or flat), per table."""
import re, sys, zipfile

def load(path):
    if path.endswith('.fods'):
        return open(path, encoding='utf8').read()
    with zipfile.ZipFile(path) as z:
        return z.read('content.xml').decode('utf8')

def rows(path):
    s = load(path)
    sty = {}
    for m in re.finditer(r'<style:style style:name="(ro\d+)" style:family="table-row">\s*<style:table-row-properties ([^/]*?)/>', s):
        h = re.search(r'style:row-height="([\d.]+)(in|cm|mm|pt)"', m.group(2))
        opt = 'style:use-optimal-row-height="true"' in m.group(2)
        v = None
        if h:
            f = {'in':1440,'cm':1440/2.54,'mm':144/2.54,'pt':20}[h.group(2)]
            v = round(float(h.group(1))*f, 1)
        sty[m.group(1)] = (v, opt)
    out = []
    for tm in re.finditer(r'<table:table table:name="([^"]+)"(.*?)(?=<table:table table:name="|</office:spreadsheet>)', s, re.S):
        name = tm.group(1); body = tm.group(2); idx = 0; runs = []
        for m in re.finditer(r'<table:table-row table:style-name="(ro\d+)"([^>]*)>', body):
            rep = re.search(r'number-rows-repeated="(\d+)"', m.group(2))
            n = int(rep.group(1)) if rep else 1
            h = sty.get(m.group(1), (None, False))
            if runs and runs[-1][2] == h:
                runs[-1][1] += n
            else:
                runs.append([idx, n, h])
            idx += n
        out.append((name, runs))
    return out

if __name__ == '__main__':
    for name, runs in rows(sys.argv[1]):
        print('TABLE', name)
        for start, n, h in runs[:40]:
            print(f'  rows {start}-{start+n-1}\t{h[0]}\topt={h[1]}')
