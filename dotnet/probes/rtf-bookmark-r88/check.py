#!/usr/bin/env python3
r"""Score `rotate.py`'s prediction against 26.2.4.2's own answer, probe by probe.

Reads the flat ODF the reference wrote for each probe (where the bookmarks landed) and the
PDF (what each REF drew), and prints one row per probe plus one per REF field.

Usage: check.py <dir with the .fodt/.pdf the reference wrote> [<probe dir>]
"""
import pathlib
import re
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from genprobes import PROBES            # noqa: E402
from rotate import Rotation             # noqa: E402

OUT = pathlib.Path(sys.argv[1])

NOT_FOUND = "Error: Reference source not found"


def predict(paras, names):
    """The bookmarks the importer emits, and what each REF expands to."""
    rot = Rotation()
    texts = []
    for index, tokens in enumerate(paras):
        text = ""
        for kind, value in tokens:
            if kind == 't':
                text += value
            else:
                rot.half(value, kind == 's', (index, len(text)))
        texts.append(text)

    marks = rot.names()

    expansions = {}
    for name in names:
        hit = next((m for m in marks if m[0] == name), None)
        if hit is None:
            expansions[name] = NOT_FOUND
            continue
        _, (start_para, start_off), (end_para, end_off) = hit
        text = texts[start_para]
        if (start_para, start_off) == (end_para, end_off):
            # A collapsed bookmark expands to nothing unless it is a cross-reference
            # bookmark, which expands to the whole node (reffld.cxx:1580-1586).
            end = len(text) if name.startswith(('__RefHeading__', '__RefNumPara__')) else start_off
        elif start_para == end_para:
            end = end_off
        else:
            end = len(text)                         # *pEnd = -1, read as nLen at :604-607
        expansions[name] = text[start_off:end]
    return marks, expansions


def observed_marks(path):
    """(name, start, end) per bookmark, positions as (paragraph index, offset)."""
    body = path.read_text(encoding='utf-8')
    body = body[body.index('<office:text'):]
    found, opened = [], {}
    for index, para in enumerate(re.findall(r'<text:p [^>]*>(.*?)</text:p>', body, re.S)):
        offset = 0
        for token in re.finditer(
                r'<text:bookmark-start text:name="([^"]*)"/>|<text:bookmark-end text:name="([^"]*)"/>'
                r'|<text:bookmark text:name="([^"]*)"/>|<[^>]+>|[^<]+', para):
            piece = token.group(0)
            if token.group(1) is not None:
                opened[token.group(1)] = (index, offset)
            elif token.group(2) is not None:
                found.append((token.group(2), opened.pop(token.group(2), None), (index, offset)))
            elif token.group(3) is not None:
                found.append((token.group(3), (index, offset), (index, offset)))
            elif not piece.startswith('<'):
                offset += len(piece.replace('&amp;', '&').replace('&lt;', '<').replace('&gt;', '>'))
    return found


def observed_refs(path, names):
    text = subprocess.run(['pdftotext', str(path), '-'], capture_output=True,
                          timeout=120).stdout.decode('utf-8', 'replace')
    drawn = {}
    for line in text.splitlines():
        match = re.match(r'\[([^\]]*)\]\s?(.*?)\s?;\s*$', line.strip())
        if match:
            drawn.setdefault(match.group(1), match.group(2))
    return {n: drawn.get(n, '<no line>') for n in names}


rows = 0
bad = 0
for stem, (paras, names) in PROBES.items():
    marks, expansions = predict(paras, names)
    got_marks = observed_marks(OUT / f'{stem}.fodt')
    got_refs = observed_refs(OUT / f'{stem}.pdf', names)

    want = sorted((m[0], m[1], m[2]) for m in marks)
    have = sorted(got_marks)
    ok = want == have
    rows += 1
    bad += 0 if ok else 1
    print(f"{stem:14s} marks   {'OK ' if ok else 'BAD'}  predicted {want}")
    if not ok:
        print(f"{'':14s}         reference {have}")
    for name in names:
        want_text = expansions[name]
        have_text = got_refs[name]
        ok = want_text.strip() == have_text.strip()
        rows += 1
        bad += 0 if ok else 1
        print(f"{stem:14s} REF {name[:12]:14s} {'OK ' if ok else 'BAD'} "
              f"predicted {want_text!r}" + ("" if ok else f"  reference {have_text!r}"))

print(f"\n{rows - bad} of {rows} predictions match 26.2.4.2")
sys.exit(1 if bad else 0)
