#!/usr/bin/env python3
"""Does 26.2.4.2 honour `a:normAutofit/@fontScale`, or does it search for the fit itself?

`ppt-fit-r85` read the answer out of the source -- textbodypropertiescontext.cxx:242-243 to
unoshape.cxx:2343-2367 to svdotext.cxx:1238-1247 -- and recorded that it had never been
measured against a rendering.  This measures it: one corpus deck, one attribute changed at a
time, the drawn em read out of 26.2.4.2's own PDF.

    fontscale-variants.py <deck.pptx> <slide index> <outdir>

The discriminator is what happens when the stated scale is made absurd.  If the reference
honours the stated pair, the drawn size follows it wherever it is put; if it searches, the
drawn size is whatever fits and the attribute moves nothing.
"""
import re, shutil, subprocess, sys, pathlib, zipfile, collections

SOFFICE = '/opt/libreoffice26.2/program/soffice'

def rewrite(src, dst, slide, replacement, target=None):
    zin = zipfile.ZipFile(src)
    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == f'ppt/slides/slide{slide}.xml':
                text = data.decode('utf-8')
                pattern = re.escape(target) if target else r'<a:normAutofit fontScale="\d+"\s*/>'
                text, n = re.subn(pattern, replacement, text, count=1)
                if n != 1:
                    raise SystemExit(f'the element to patch was not found in slide{slide}.xml')
                data = text.encode('utf-8')
            zout.writestr(item, data)

def render(path, outdir, profile):
    outdir.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        ['timeout', '-k', '30', '900', SOFFICE, '--headless', '--norestore',
         f'-env:UserInstallation=file://{profile}',
         '--convert-to', 'pdf', '--outdir', str(outdir), str(path)],
        capture_output=True)
    pdfs = list(outdir.glob('*.pdf'))
    return pdfs[0] if pdfs else None

def sizes(pdf, page):
    """The Tf sizes drawn on one page, as a histogram."""
    sys.path.insert(0, '/home/user/libreoffice-core/.claude/skills/render-comparison/scripts')
    import importlib.util
    spec = importlib.util.spec_from_file_location(
        'po', '/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py')
    m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
    blob = pathlib.Path(pdf).read_bytes()
    objects = m.Objects(blob)
    for number, body in enumerate(objects.pages(), start=1):
        if number != page: continue
        found = re.search(rb"/Contents\s*(\[[^\]]*\]|\d+\s+\d+\s+R)", body)
        stream = b''
        for ref in re.finditer(rb"(\d+)\s+\d+\s+R", found.group(1)):
            part = objects.raw(int(ref.group(1)))
            if part: stream += objects.stream_of(part) + b'\n'
        return collections.Counter(
            float(v) for v in re.findall(rb'/F\d+\s+([\d.]+)\s+Tf', stream))
    return collections.Counter()

if __name__ == '__main__':
    deck = pathlib.Path(sys.argv[1]); slide = int(sys.argv[2]); out = pathlib.Path(sys.argv[3])
    target = sys.argv[4] if len(sys.argv) > 4 else None
    out.mkdir(parents=True, exist_ok=True)
    variants = {
        'stated-90000':  '<a:normAutofit fontScale="90000"/>',
        'absent':        '<a:normAutofit/>',
        'stated-50000':  '<a:normAutofit fontScale="50000"/>',
        'stated-25000':  '<a:normAutofit fontScale="25000"/>',
        'no-normAutofit': '',
        # The other arm.  `mnSpacingScale` defaults to 100000 when `lnSpcReduction` is absent,
        # and the importer stores `1.0 - mnSpacingScale/100000` -- so an absent attribute makes
        # the spacing scale exactly ZERO, and `setupAutoFitText`'s guard
        # `fFontScale > 0.0 && fSpacingScale > 0.0` then falls to `resetScalingParameters()`.
        # State one and the stored pair is supposed to reach the outliner.
        'fs90000-lnSpc10000': '<a:normAutofit fontScale="90000" lnSpcReduction="10000"/>',
        'fs50000-lnSpc10000': '<a:normAutofit fontScale="50000" lnSpcReduction="10000"/>',
        'fs25000-lnSpc10000': '<a:normAutofit fontScale="25000" lnSpcReduction="10000"/>',
        'fs100000-lnSpc10000': '<a:normAutofit fontScale="100000" lnSpcReduction="10000"/>',
        'fs25000-lnSpc50000': '<a:normAutofit fontScale="25000" lnSpcReduction="50000"/>',
    }
    print('variant\tdrawn Tf sizes on page %d (size:count)' % slide)
    for name, replacement in variants.items():
        work = out / name
        if work.exists(): shutil.rmtree(work)
        work.mkdir(parents=True)
        patched = work / deck.name
        rewrite(deck, patched, slide, replacement, target)
        pdf = render(patched, work / 'pdf', work / 'profile')
        if pdf is None:
            print(f'{name}\tRENDER FAILED'); continue
        hist = sizes(pdf, slide)
        print(name + '\t' + ' '.join(f'{k:g}:{v}' for k, v in sorted(hist.items())))
