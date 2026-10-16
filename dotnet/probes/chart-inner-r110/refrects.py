#!/usr/bin/env python3
"""26.2.4.2's own chart page, plot-area and coordinate-region for every chart in a document."""
import pathlib, re, subprocess, sys, shutil
SOFFICE='/opt/libreoffice26.2/program/soffice'
PT=72.0/2.54
LEN=re.compile(r'svg:(x|y|width|height)="([-\d.]+)cm"')
FILT={'.docx':'fodt','.doc':'fodt','.odt':'fodt','.rtf':'fodt',
      '.xlsx':'fods','.xls':'fods','.ods':'fods','.xlsm':'fods',
      '.pptx':'fodp','.ppt':'fodp','.odp':'fodp','.pptm':'fodp'}
def rects(src, work):
    work=pathlib.Path(work); shutil.rmtree(work, ignore_errors=True); work.mkdir(parents=True)
    subprocess.run([SOFFICE,'--headless',f'-env:UserInstallation=file://{work}/prof',
                    '--convert-to',FILT[src.suffix.lower()],'--outdir',str(work),str(src)],
                   capture_output=True, check=False, timeout=300)
    out=[p for p in work.iterdir() if p.suffix.startswith('.fod')]
    if not out: return []
    s=out[0].read_text(encoding='utf-8', errors='replace')
    ch=re.findall(r'<chart:chart [^>]*>', s)
    pa=re.findall(r'<chart:plot-area[^>]*>', s)
    cr=re.findall(r'<chart:coordinate-region[^>]*>', s)
    cls=[re.search(r'chart:class="([^"]*)"', c) for c in ch]
    def cm(t): return {k: float(v)*PT for k,v in LEN.findall(t)}
    return [(m.group(1) if m else '?', cm(a), cm(b), cm(c)) for m,a,b,c in zip(cls,ch,pa,cr)]
for a in sys.argv[1:]:
    src=pathlib.Path(a)
    for i,(cls,page,pa,cr) in enumerate(rects(src, '/home/user/r110-work/rr')):
        sq_pa=min(pa.get('width',0),pa.get('height',0)); sq_cr=min(cr.get('width',0),cr.get('height',0))
        print(f'{src.name}\t{i}\t{cls}\tpage {page.get("width",0):.1f}x{page.get("height",0):.1f}'
              f'\tPA ({pa.get("x",0):.2f},{pa.get("y",0):.2f},{pa.get("width",0):.2f},{pa.get("height",0):.2f})'
              f'\tCR ({cr.get("x",0):.2f},{cr.get("y",0):.2f},{cr.get("width",0):.2f},{cr.get("height",0):.2f})'
              f'\tshrink {sq_cr/sq_pa if sq_pa else 0:.4f}')
