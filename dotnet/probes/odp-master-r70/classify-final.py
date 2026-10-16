#!/usr/bin/env python3
"""Classify the 182 non-matching .odp rows of the first ODF gate by CAUSE.

Every column is measured, not asserted:
  master_alnum   what the row's master pages contribute that the layout path never drew
                 (classify.py's model: non-placeholder master shapes that land on the page)
  display_none   occurrences of drawooo:display="none" in the file
  running_alnum  what the master's running objects would contribute on the pages that
                 switch them on, from presentation:*-decl and presentation:use-*-name
  ooxml_verdict  the same deck's verdict in its original .pptx/.ppt spelling, from
                 /home/user/gate-2f47 -- 'match' means a divergence is the ODF reader's
  before/after   this round's own glyph columns, ours/ref
"""
import csv, os, sys, zipfile, collections
import xml.etree.ElementTree as ET

OFF='{urn:oasis:names:tc:opendocument:xmlns:office:1.0}'
D='{urn:oasis:names:tc:opendocument:xmlns:drawing:1.0}'
S='{urn:oasis:names:tc:opendocument:xmlns:style:1.0}'
P='{urn:oasis:names:tc:opendocument:xmlns:presentation:1.0}'
T='{urn:oasis:names:tc:opendocument:xmlns:text:1.0}'
SVG='{urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0}'
FO='{urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0}'
DOO='{http://openoffice.org/2010/draw}'
KIND={'header':'display-header','footer':'display-footer',
      'date-time':'display-date-time','page-number':'display-page-number'}

def alnum(s): return sum(1 for c in s if c.isalnum())
def paratext(el): return '\n'.join(''.join(p.itertext()) for p in el.iter(T+'p'))

def cm(v):
    if not v: return None
    for u,f in (('cm',1.0),('mm',.1),('in',2.54),('pt',2.54/72),('pc',2.54/6),('px',2.54/96)):
        if v.endswith(u):
            try: return float(v[:-len(u)])*f
            except ValueError: return None
    try: return float(v)
    except ValueError: return None

def leaves(el):
    for c in el:
        if not c.tag.startswith(D): continue
        tag=c.tag[len(D):]
        if tag in ('enhanced-geometry','equation','handle','image','glue-point','text-box','page-thumbnail'):
            continue
        if tag=='g': yield from leaves(c)
        else: yield c

def analyse(path):
    with zipfile.ZipFile(path) as z:
        content=ET.fromstring(z.read('content.xml'))
        styles=ET.fromstring(z.read('styles.xml'))
        raw=z.read('styles.xml')+z.read('content.xml')
    masters={m.get(S+'name'):m for m in styles.iter(S+'master-page')}
    layouts={l.get(S+'name'):l for l in styles.iter(S+'page-layout')}
    dp={}
    for s in list(content.iter(S+'style'))+list(styles.iter(S+'style')):
        if s.get(S+'family')!='drawing-page': continue
        pr=s.find(S+'drawing-page-properties')
        if pr is not None: dp[s.get(S+'name')]={k:pr.get(P+v) for k,v in KIND.items()}
    pres=content.find('.//'+OFF+'presentation')
    decls={}
    if pres is not None:
        for tag in ('header-decl','footer-decl','date-time-decl'):
            for d0 in pres.findall(P+tag): decls[d0.get(P+'name')]=''.join(d0.itertext())

    def size(m):
        l=layouts.get(m.get(S+'page-layout-name'))
        if l is None: return None
        pr=l.find(S+'page-layout-properties')
        if pr is None: return None
        return cm(pr.get(FO+'page-width')), cm(pr.get(FO+'page-height'))

    def bg(m):
        sz=size(m); tot=0
        for sh in leaves(m):
            if sh.get(P+'class') is not None: continue
            if sz and sz[0]:
                x,y=cm(sh.get(SVG+'x')),cm(sh.get(SVG+'y'))
                w,h=cm(sh.get(SVG+'width')) or 0, cm(sh.get(SVG+'height')) or 0
                if x is not None and y is not None and (x>=sz[0] or y>=sz[1] or x+w<=0 or y+h<=0):
                    continue
            tot+=alnum(paratext(sh))
        return tot

    cache={}; master_alnum=0; running=0; pages=0
    for pg in (pres.findall(D+'page') if pres is not None else []):
        pages+=1
        mn=pg.get(D+'master-page-name'); m=masters.get(mn)
        if m is None: continue
        if mn not in cache: cache[mn]=bg(m)
        master_alnum+=cache[mn]
        props=dp.get(pg.get(D+'style-name'), {})
        for fr in m:
            k=fr.get(P+'class')
            if k not in KIND or props.get(k)!='true': continue
            if k=='page-number': running+=2; continue
            running+=alnum(decls.get(pg.get(P+'use-'+k+'-name'),''))
    return pages, master_alnum, raw.count(b'display="none"'), running

def main():
    root='/home/user/corpus-odf/'
    def load(p):
        d={}
        with open(p) as fh:
            fh.readline()
            for r in csv.DictReader(fh, delimiter='\t'): d[r['path']]=r
        return d
    before, after = load(sys.argv[1]), load(sys.argv[2])
    ooxml={}
    for r in csv.reader(open('/home/user/gate-2f47/rows.tsv'), delimiter='\t'):
        if r[0].startswith('slides/'):
            ooxml[os.path.splitext(os.path.basename(r[0]))[0]]=(r[6], r[8])

    w=csv.writer(sys.stdout, delimiter='\t')
    w.writerow(['path','pages','before_glyphs','after_glyphs','before_verdict','after_verdict',
                'master_alnum','display_none','running_alnum','ooxml_verdict','ooxml_glyphs','cause'])
    tally=collections.Counter()
    for p in sorted(before):
        if before[p]['verdict']=='match' and after[p]['verdict']=='match': continue
        pages, ma, dn, run = analyse(root+p)
        stem=os.path.splitext(os.path.basename(p))[0]
        ov, og = ooxml.get(stem, ('', ''))
        if before[p]['verdict']!='match' and after[p]['verdict']=='match':
            cause='master-background-objects (closed)'
        elif before[p]['verdict']=='match':
            cause='regressed: another ODF defect crossed the band'
        elif ov and ov!='match':
            cause='shared with the OOXML reader'
        elif run:
            cause='master running objects (open)'
        elif ma:
            cause='master background partly closed, residual elsewhere'
        else:
            cause='other, ODF reader'
        tally[cause]+=1
        w.writerow([p, pages, before[p]['glyphs'], after[p]['glyphs'],
                    before[p]['verdict'], after[p]['verdict'], ma, dn, run, ov, og, cause])
    for k,n in tally.most_common(): print('# %-52s %d' % (k,n), file=sys.stderr)

main()
