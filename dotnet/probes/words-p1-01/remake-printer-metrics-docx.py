#!/usr/bin/env python3
"""Rebuild `tests/corpus/features/printer-metrics{,-off}.docx` at a size that still separates.

    remake-printer-metrics-docx.py [features-dir] [--check /abs/scratch]

`research/probes/words-r13/make-printer-metrics-docx.py` built this pair at **12 pt Arial**,
chosen because that is where a 300 dpi printer grid showed largest: LibreOffice 24.2.7.2 set
it at 13.80 pt printer-independently and 13.95 pt with `w:usePrinterMetrics`.

The printer here is **600 dpi**, and 600 dpi sets 12 pt at exactly 100 device pixels — so the
flag changes nothing at all at that size and `PrinterMetricsTests` was asserting a difference
the binary no longer makes. A fixture that cannot separate is worse than no fixture: it fails
whichever way the code behaves.

**16 pt is the size to use on this container**, and it is chosen the same way the 12 pt was —
by sweeping and taking the largest separation, not by picking a round number:

```
pt     printer-independent   600 dpi printer   apart
 8.0        184 tw               185 tw          1
 9.5        218                  221             3
11.0        253                  252            -1
12.0        276                  276             0   <- the old fixture, now inert
16.0        369                  365            -4   <- the largest, and the one used
```

Four twips is 0.20 pt against a 0.03 pt tolerance. The old fixture's failure mode is the
instructive one and is why this script re-measures rather than asserting: **the size that
discriminates is a property of the device, not of the document**, so it moves whenever the
headless printer does.

`--check` renders both packages through `soffice` and prints the pitch each one actually
gets, so the numbers written into the test come from the binary rather than from this
script's arithmetic.
"""
import os
import re
import subprocess
import sys
import zipfile

W = 'xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"'

# Half-points, so 32 is 16 pt.
SIZE_HALF_POINTS = 32

CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="word/document.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
</Relationships>"""

DOC_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Target="styles.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
  <Relationship Id="rId2" Target="settings.xml" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
</Relationships>"""

STYLES = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles {W}>
<w:docDefaults><w:rPrDefault><w:rPr>
<w:rFonts w:ascii="Arial" w:hAnsi="Arial"/><w:sz w:val="{SIZE_HALF_POINTS}"/>
<w:lang w:val="en-US"/></w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>
<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/><w:qFormat/></w:style>
</w:styles>"""

LINE = ('The quick brown fox jumps over the lazy dog while the printer rounds every metric '
        'onto its own pixel grid and the line grows by a fraction of a point. ')

DOCUMENT = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document {W}><w:body>
<w:p><w:r><w:t xml:space="preserve">{LINE * 6}</w:t></w:r></w:p>
<w:sectPr><w:pgSz w:w="12240" w:h="15840"/>
<w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
</w:sectPr></w:body></w:document>"""


def settings(printer: bool) -> str:
    flag = '<w:usePrinterMetrics/>' if printer else ''
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings {W}>
  <w:defaultTabStop w:val="720"/>
  <w:compat>{flag}</w:compat>
</w:settings>"""


def build(path: str, printer: bool) -> None:
    parts = {
        '[Content_Types].xml': CONTENT_TYPES,
        '_rels/.rels': ROOT_RELS,
        'word/_rels/document.xml.rels': DOC_RELS,
        'word/document.xml': DOCUMENT,
        'word/styles.xml': STYLES,
        'word/settings.xml': settings(printer),
    }
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, data in parts.items():
            z.writestr(name, data)
    print('wrote', os.path.abspath(path))


def pitch(pdf: str) -> float:
    """The mean baseline-to-baseline distance down the first page's one paragraph."""
    out = subprocess.run(
        ['python3',
         '/c/sandbox/workdir/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py',
         'dump', pdf, '--page', '1', '--only', 'text'],
        capture_output=True, text=True).stdout
    ys = []
    for line in out.splitlines():
        m = re.match(r'text\s+p\d+\s+\(\s*[\d.-]+,\s*([\d.-]+)\)', line)
        if m:
            ys.append(float(m.group(1)))
    ys = sorted(set(ys), reverse=True)
    return (ys[0] - ys[-1]) / (len(ys) - 1)


def check(features: str, scratch: str) -> None:
    os.makedirs(scratch, exist_ok=True)
    for name in ('printer-metrics.docx', 'printer-metrics-off.docx'):
        src = os.path.join(features, name)
        subprocess.run(
            ['soffice', f'-env:UserInstallation=file://{scratch}/prof', '--headless',
             '--convert-to', 'pdf', '--outdir', scratch, src],
            capture_output=True, timeout=300)
        pdf = os.path.join(scratch, name.replace('.docx', '.pdf'))
        print(f"  {name:26s} pitch {pitch(pdf):.3f} pt")


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    here = os.path.dirname(os.path.abspath(__file__))
    features = args[0] if args else os.path.join(here, '..', '..', 'tests', 'corpus', 'features')
    build(os.path.join(features, 'printer-metrics.docx'), printer=True)
    build(os.path.join(features, 'printer-metrics-off.docx'), printer=False)
    if '--check' in sys.argv:
        print('what LibreOffice actually draws:')
        check(features, sys.argv[sys.argv.index('--check') + 1])
