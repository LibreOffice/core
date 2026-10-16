#!/usr/bin/env python3
"""Census chart:class in the reference's own ODF export (corpus-odf).
Embedded charts live as Object N/content.xml inside the package."""
import re, sys, zipfile, os, csv
ROOT="/home/user/corpus-odf"
cls_re = re.compile(rb'<chart:chart[^>]*chart:class="([^"]*)"')
_ser_re = re.compile(rb'<chart:series[^>]*chart:class="([^"]*)"')
sub_re = {
  "vertical_true": re.compile(rb'chart:vertical="true"'),
  "stacked_true":  re.compile(rb'chart:stacked="true"'),
  "percentage_true":re.compile(rb'chart:percentage="true"'),
  "three_d_true":  re.compile(rb'chart:three-dimensional="true"'),
  "lines_true":    re.compile(rb'chart:lines="true"'),
  "symbol_type":   re.compile(rb'chart:symbol-type="[^"]*"'),
  "interpolation": re.compile(rb'chart:interpolation="[^"]*"'),
  "japanese_true": re.compile(rb'chart:japanese-candle-stick="true"'),
  "stock_open":    re.compile(rb'chart:stock-with-volume="true"'),
  "deep_true":     re.compile(rb'chart:deep="true"'),
  "solid_type":    re.compile(rb'chart:solid-type="[^"]*"'),
}
out=csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
out.writerow(["path","ext","part","feature"])
for dirpath,_,files in os.walk(ROOT):
    for fn in sorted(files):
        ext=fn.rsplit(".",1)[-1].lower()
        if ext not in ("odt","ods","odp"): continue
        p=os.path.join(dirpath,fn)
        rel=os.path.relpath(p,ROOT)
        try: z=zipfile.ZipFile(p)
        except Exception as e:
            sys.stderr.write("BADZIP %s %s\n"%(rel,e)); continue
        for n in z.namelist():
            if not n.endswith("content.xml"): continue
            try: d=z.read(n)
            except Exception as e:
                sys.stderr.write("BADPART %s %s\n"%(rel,n)); continue
            cs=set(cls_re.findall(d))
            if not cs: continue
            feats=["class:"+c.decode() for c in cs]
            feats+=["series-class:"+c.decode() for c in set(_ser_re.findall(d))]
            for k,rx in sub_re.items():
                if rx.search(d): feats.append("@"+k)
            for ft in feats: out.writerow([rel,ext,n,ft])
        z.close()
