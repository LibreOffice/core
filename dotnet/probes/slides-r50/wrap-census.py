#!/usr/bin/env python3
"""Census a:bodyPr elements stating wrap="none" together with spAutoFit/normAutofit.

DECLARED, not resolved. Blind spots recorded in the prediction file: a bodyPr inheriting its
wrap from a layout or master while stating its own autofit (or the reverse) is invisible here,
in both directions, so this is neither a floor nor a ceiling -- it is the same-element case only.
"""
import zipfile,re,csv,sys
from pathlib import Path
CORPUS=Path("/c/sandbox/workdir/sample-files")
BODY=re.compile(r'<a:bodyPr\b[^>]*?(?:/>|>.*?</a:bodyPr>)',re.S)
rows=[r for r in csv.DictReader(open(CORPUS/"MANIFEST.tsv"),delimiter="\t") if r["family"]=="slides"]
print("path\tstatus\tkind\tbodies\twrapnone\twrapnone_with_autofit")
tot=hit=0
for r in rows:
    p=CORPUS/r["path"]
    if not p.is_file():
        c=[x for x in p.parent.glob(p.stem+".*") if x.suffix.lower()==p.suffix.lower()]
        if not c: continue
        p=c[0]
    try: z=zipfile.ZipFile(p)
    except Exception: continue
    nb=nw=na=0
    for n in z.namelist():
        if not (n.startswith("ppt/") and n.endswith(".xml")): continue
        try: d=z.read(n).decode("utf8","replace")
        except Exception: continue
        for m in BODY.finditer(d):
            b=m.group(0); nb+=1
            if 'wrap="none"' not in b: continue
            nw+=1
            if "spAutoFit" in b or "normAutofit" in b: na+=1
    print(f"{r['path']}\t{r['status']}\t{r['kind']}\t{nb}\t{nw}\t{na}")
    tot+=1
    if na: hit+=1
print(f"# documents scanned {tot}, with at least one wrap=none+autofit body: {hit}", file=sys.stderr)
