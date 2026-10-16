#!/usr/bin/env python3
"""One legend, eleven font sizes a point apart — the probe that finds the step.

    make-legend-step-probe.py <outdir>
    soffice --headless --convert-to pdf --outdir <outdir>/ref <outdir>/*.pptx
    ../slides-r23/read-legend.py <outdir>/ref/*.pdf

Round twenty-three measured a legend's row height at three font sizes, found it was not a
constant multiple of the font, wrote "a quantisation somewhere in the reference device is the
obvious suspect", and named this sweep as the probe that would separate the candidates. It
does: see `legend-step.tsv` beside this file.

Reuses `slides-r23/make-legend-probe.py`'s rewriting — same source deck, same two five-character
entry names, only `c:legend`'s own `sz` moves — so the two rounds' numbers are directly
comparable, and they agree: 7.55, 11.28 and 16.50 pt at 7, 10 and 14 points, to two places.
"""
import importlib.util
import os
import re
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
SPEC = importlib.util.spec_from_file_location(
    'legendprobe', os.path.join(HERE, '..', 'slides-r23', 'make-legend-probe.py'))
LEGEND = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(LEGEND)


def main():
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2

    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)

    for hundredths in range(600, 1601, 100):
        target = os.path.join(out, f'legend-n05-s{hundredths}.pptx')
        with zipfile.ZipFile(LEGEND.SOURCE) as src, \
                zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
            for item in src.infolist():
                data = src.read(item.filename)
                if re.match(r'ppt/charts/chart\d+\.xml$', item.filename):
                    data = LEGEND.rechart(data, 5, hundredths)
                dst.writestr(item, data)
        print(target)

    return 0


if __name__ == '__main__':
    sys.exit(main())
