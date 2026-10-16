#!/usr/bin/env python3
"""Read a PDF's text-showing operators back as glyph pens, in points.

LibreOffice's PDF writer states one pen per text object (`Td`/`Tm`) and encodes every
inter-glyph gap as an integer thousandth of an em inside the `TJ` array:

    nAdjustment = trunc(nNativeWidth - gap*1000/fontheight + 0.5)

(`vcl/source/pdf/pdfwriter_impl.cxx`, `drawHorizontalGlyphs`), where `nNativeWidth` is the
same integer the `/Widths` array declares. So a run's gaps are recoverable exactly as

    gap = (declared_width - adjustment) * fontsize / 1000

and the only thing lost is the sub-unit part of the adjustment's own truncation, +-0.5/1000
of an em. That is the resolution of this instrument and every reading here is quoted with it.

Not `pdftotext`: poppler reconstructs a word box from the same declared widths and then
reports its *ink*, so a word's right edge is the last glyph's ink rather than its pen.
"""

import re
import sys
import zlib


def _objects(data: bytes) -> dict[int, bytes]:
    found = {}
    for m in re.finditer(rb'(?<![0-9])(\d+)\s+0\s+obj\b', data):
        end = data.find(b'endobj', m.end())
        found[int(m.group(1))] = data[m.end():end if end > 0 else len(data)]
    return found


def _stream(body: bytes) -> bytes:
    start = body.find(b'stream')
    if start < 0:
        return b''
    start = body.find(b'\n', start) + 1
    end = body.find(b'endstream', start)
    raw = body[start:end]
    try:
        return zlib.decompress(raw)
    except zlib.error:
        return raw


class Pdf:
    """The fonts and the text runs of a LibreOffice-written PDF."""

    def __init__(self, path: str):
        self.path = path
        data = open(path, 'rb').read()
        self.objects = _objects(data)
        self.fonts = self._fonts()
        self.content = b'\n'.join(
            _stream(body) for body in self.objects.values()
            if b'/Length' in body and (b'Tf' in _stream(body)))

    def _fonts(self) -> dict[str, dict]:
        """`/Fn` -> {base, first, widths, tounicode}, read per font object."""
        fonts = {}
        names = {}
        for body in self.objects.values():
            for m in re.finditer(rb'/(F\d+)\s+(\d+)\s+0\s+R', body):
                names[int(m.group(2))] = m.group(1).decode()
        for number, body in self.objects.items():
            if b'/Type/Font' not in body.replace(b'/Type /Font', b'/Type/Font'):
                continue
            base = re.search(rb'/BaseFont\s*/(?:[A-Z]{6}\+)?([A-Za-z0-9\-]+)', body)
            first = re.search(rb'/FirstChar\s+(\d+)', body)
            widths = re.search(rb'/Widths\s*\[([^\]]*)\]', body)
            unicode_ref = re.search(rb'/ToUnicode\s+(\d+)\s+0\s+R', body)
            if not (base and first and widths):
                continue
            mapping = {}
            if unicode_ref:
                cmap = _stream(self.objects.get(int(unicode_ref.group(1)), b''))
                mapping = {int(e.group(1), 16): chr(int(e.group(2), 16))
                           for e in re.finditer(rb'<([0-9A-Fa-f]{2})>\s*<([0-9A-Fa-f]{4})>', cmap)}
            fonts[names.get(number, f'?{number}')] = {
                'base': base.group(1).decode(),
                'first': int(first.group(1)),
                'widths': [int(float(v)) for v in widths.group(1).split()],
                'tounicode': mapping,
            }
        return fonts

    def runs(self) -> list[dict]:
        """Every text-showing operator, as a run with its pen, size, text and gaps.

        A run is `{'x','y','size','font','text','adjust','gaps','widths'}`; `gaps` are the
        inter-glyph advances in points, `adjust` the raw `TJ` integers.
        """
        text = self.content.decode('latin1')
        found = []
        state = {'x': 0.0, 'y': 0.0, 'font': None, 'size': 0.0, 'angle': 0.0}
        pattern = re.compile(
            r'(?P<tm>(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s+Tm)'
            r'|(?P<td>(-?[\d.]+)\s+(-?[\d.]+)\s+Td)'
            r'|(?P<tf>/(F\d+)\s+([\d.]+)\s+Tf)'
            r'|(?P<show>\[(.*?)\]\s*TJ|<([0-9A-Fa-f]*)>\s*Tj)')
        for m in pattern.finditer(text):
            if m.group('tm'):
                state['x'], state['y'] = float(m.group(6)), float(m.group(7))
                state['angle'] = (float(m.group(2)), float(m.group(3)))
            elif m.group('td'):
                state['x'], state['y'] = float(m.group(9)), float(m.group(10))
            elif m.group('tf'):
                state['font'], state['size'] = m.group(12), float(m.group(13))
            elif m.group('show'):
                font = self.fonts.get(state['font'], {})
                codes: list[int] = []
                adjust: list[int] = []
                pending = 0

                def push(hexed: str) -> None:
                    for i in range(0, len(hexed) - 1, 2):
                        nonlocal_codes = int(hexed[i:i + 2], 16)
                        if codes:
                            adjust.append(pending[0])
                            pending[0] = 0
                        codes.append(nonlocal_codes)

                pending = [0]
                if m.group(16) is not None:
                    push(m.group(16))
                else:
                    for piece in re.finditer(r'<([0-9A-Fa-f]*)>|(-?[\d.]+)', m.group(15)):
                        if piece.group(1) is not None:
                            push(piece.group(1))
                        else:
                            pending[0] += int(round(float(piece.group(2))))
                widths = [font.get('widths', [0])[c - font.get('first', 0)]
                          if font and 0 <= c - font['first'] < len(font['widths']) else 0
                          for c in codes]
                gaps = [(widths[i] - adjust[i]) * state['size'] / 1000.0
                        for i in range(len(adjust))]
                found.append({
                    'x': state['x'], 'y': state['y'], 'size': state['size'],
                    'font': font.get('base', state['font']), 'angle': state['angle'],
                    'text': ''.join(font.get('tounicode', {}).get(c, '?') for c in codes),
                    'codes': codes, 'adjust': adjust, 'gaps': gaps, 'widths': widths,
                })
        return found


def main() -> int:
    for path in sys.argv[1:]:
        print(f'## {path}')
        pdf = Pdf(path)
        for run in pdf.runs():
            print(f'{run["x"]:9.3f} {run["y"]:9.3f}  {run["size"]:7.3f}  {run["font"]:20s}'
                  f'  {run["text"]!r:24s} adj={run["adjust"]} '
                  f'gaps={[round(g, 4) for g in run["gaps"]]}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
