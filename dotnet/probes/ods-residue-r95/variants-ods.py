#!/usr/bin/env python3
"""One-attribute variants of a packaged `.ods`, rendered by 26.2.4.2.

Rewrites `content.xml` with a caller-named edit, repackages, and reports what the
reference prints for it — page count, and per page the characters and drawn paths.

Usage: variants-ods.py <document.ods> <outdir> [variant ...]
"""
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

SOFFICE = '/opt/libreoffice26.2/program/soffice'
PROFILE = 'file:///tmp/xray-office-lo-r95v'


def repackage(source, target, edit):
    with zipfile.ZipFile(source) as zin:
        content = zin.read('content.xml').decode('utf8')
        edited = edit(content)
        if edited == content:
            raise SystemExit(f'{target.name}: the edit changed nothing')
        with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = edited.encode('utf8') if item.filename == 'content.xml' \
                    else zin.read(item.filename)
                zout.writestr(item, data)


def render(path, outdir):
    subprocess.run(
        ['timeout', '-k', '30', '300', SOFFICE, '--headless',
         f'-env:UserInstallation={PROFILE}', '--convert-to', 'pdf',
         '--outdir', str(outdir), str(path)],
        check=False, capture_output=True)
    pdf = outdir / (path.stem + '.pdf')
    return pdf if pdf.exists() else None


def report(tag, pdf):
    if pdf is None:
        print(f'{tag}\tFAILED')
        return
    import pymupdf
    doc = pymupdf.open(pdf)
    pages = [(len(page.get_text().strip()), len(page.get_drawings())) for page in doc]
    doc.close()
    detail = ' '.join(f'p{i + 1}:{c}c/{d}d' for i, (c, d) in enumerate(pages))
    print(f'{tag}\t{len(pages)} pages\t{detail}')


def drop_element(name):
    """Delete every `<name .../>` or `<name ...>…</name>` from the content."""
    def edit(text):
        text = re.sub(rf'<{name}\b[^>]*/>', '', text)
        return re.sub(rf'<{name}\b.*?</{name}>', '', text, flags=re.S)
    return edit


VARIANTS = {
    'no-connector': drop_element('draw:connector'),
    'no-line': drop_element('draw:line'),
    'no-custom-shape': drop_element('draw:custom-shape'),
    'no-frame': drop_element('draw:frame'),
}


def main():
    source = Path(sys.argv[1])
    outdir = Path(sys.argv[2])
    wanted = sys.argv[3:] or list(VARIANTS)
    outdir.mkdir(parents=True, exist_ok=True)

    base = outdir / source.name
    shutil.copy(source, base)
    report('as it stands', render(base, outdir))

    for tag in wanted:
        target = outdir / f'{source.stem}--{tag}.ods'
        repackage(source, target, VARIANTS[tag])
        report(tag, render(target, outdir))


if __name__ == '__main__':
    main()
