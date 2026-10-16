#!/usr/bin/env python3
"""The one-attribute variants that established what puts a Calc row on the measured branch.

Each variant removes exactly one thing from a real corpus `.ods`, hands the patched file to
26.2.4.2 and reads the row heights out of its own `--convert-to fods` — the number Calc
computed, rather than a pitch inferred from a rendered page.

    python3 variants.py /home/user/corpus-odf/sheets /tmp/out

Measured 2026-09-10 against /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2:

    Special-Procedures_2025-07-10.ods   as it stands            rows 6-200   298 twips
                                        conditional formats removed         276
    hdss-bulletin-index-2019-2022.ods   as it stands            rows 1-200   298
                                        every <text:a> unwrapped            276
                                        (row 0, its header, is 276 either way)

276 is `lcl_GetAttribHeight`'s arithmetic for Calibri 11 — `trunc(220 x 1.18) + 40 - 23` — and
298 is one measured EditEngine line, `(14 + 4 + 1 + 1) px / 0.067`. Both documents are 11 pt
Calibri throughout, so the two numbers are the two branches and nothing else.
"""
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

SOFFICE = '/opt/libreoffice26.2/program/soffice'

VARIANTS = {
    'Special-Procedures_2025-07-10.ods': [
        ('as-is', None),
        ('no-condfmt',
         lambda c: re.sub(r'<calcext:conditional-formats>.*?</calcext:conditional-formats>',
                          '', c, flags=re.S)),
    ],
    'hdss-bulletin-index-2019-2022.ods': [
        ('as-is', None),
        ('no-hyperlink',
         lambda c: re.sub(r'<text:a [^>]*>(.*?)</text:a>', r'\1', c, flags=re.S)),
    ],
}


def repack(source: Path, target: Path, patch):
    with tempfile.TemporaryDirectory() as work:
        work = Path(work)
        with zipfile.ZipFile(source) as archive:
            archive.extractall(work)
        if patch is not None:
            content = (work / 'content.xml').read_text(encoding='utf8')
            (work / 'content.xml').write_text(patch(content), encoding='utf8')
        with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as out:
            out.write(work / 'mimetype', 'mimetype', zipfile.ZIP_STORED)
            for path in sorted(work.rglob('*')):
                if path.is_file() and path.name != 'mimetype':
                    out.write(path, str(path.relative_to(work)))


def heights(path: Path, out: Path):
    subprocess.run(
        ['timeout', '-k', '30', '900', SOFFICE, '--headless', '--norestore',
         '-env:UserInstallation=file://' + str(out / 'profile'),
         '--convert-to', 'fods', '--outdir', str(out), str(path)],
        check=False, capture_output=True)
    flat = out / (path.stem + '.fods')
    if not flat.exists():
        return []
    text = flat.read_text(encoding='utf8')
    styles = {}
    for match in re.finditer(
            r'<style:style style:name="(ro\d+)" style:family="table-row">\s*'
            r'<style:table-row-properties ([^/]*?)/>', text):
        stated = re.search(r'style:row-height="([\d.]+)in"', match.group(2))
        styles[match.group(1)] = round(float(stated.group(1)) * 1440, 1) if stated else None
    runs, index = [], 0
    for match in re.finditer(r'<table:table-row table:style-name="(ro\d+)"([^>]*)>', text):
        repeat = re.search(r'number-rows-repeated="(\d+)"', match.group(2))
        count = int(repeat.group(1)) if repeat else 1
        value = styles.get(match.group(1))
        if runs and runs[-1][2] == value:
            runs[-1][1] += count
        else:
            runs.append([index, count, value])
        index += count
    return runs


def main(corpus: str, outdir: str):
    corpus, outdir = Path(corpus), Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    for name, variants in VARIANTS.items():
        source = next(corpus.rglob(name))
        print('####', name)
        for label, patch in variants:
            work = outdir / f'{source.stem}-{label}'
            if work.exists():
                shutil.rmtree(work)
            work.mkdir(parents=True)
            patched = work / f'{label}.ods'
            repack(source, patched, patch)
            for start, count, value in heights(patched, work)[:6]:
                print(f'  {label:14s} rows {start}-{start + count - 1}\t{value}')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else tempfile.mkdtemp())
