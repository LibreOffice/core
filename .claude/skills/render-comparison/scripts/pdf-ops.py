#!/usr/bin/env python3
"""Dump a PDF's page content as typed, positioned records — and diff two of them.

    pdf-ops.py dump  <pdf> [--page N]
    pdf-ops.py diff  <ours.pdf> <ref.pdf> [--page N] [--only text|fill|stroke|image]

WHY THIS EXISTS
───────────────
The pixel diff says *a region differs*. The word gate says *a count differs*. Neither says
**which element**, and so every round so far has ended with an agent reading content streams by
hand. Seven consecutive rounds shipped a throwaway script that does part of this — `pdfops.py`,
`sl14-compare.py`, `compare-fit.py`, `read-autofit.py`, `sl14-pagediff.py`, `fold-check.py`,
`affcmp.py`. This is that script, kept.

It answers the question those all asked: *the reference draws a fill here that we do not*, or
*this show is 12.0 pt in one and 11.5 pt in the other*, or *this run is one show there and three
here*.

WHAT IT IS NOT
──────────────
It is not a fidelity metric and it does not produce a score. It is an **attribution** tool: it
turns "page 4 differs" into "the fill at (51, 252)-(226, 266) is #1F4E79 in the reference and
absent in ours". Use `pdf-image-diff.py` to find the pages worth looking at, then this to find
out what is on them.

TEXT DECODING, AND WHY GLYPH COUNTS RATHER THAN STRINGS
──────────────────────────────────────────────────────
A subset font's hex strings need its ToUnicode CMap to read, and reimplementing that is a large
surface to get quietly wrong. So a text record carries **position, effective size, font and glyph
count**, decoding the string only when it is a printable literal. That is enough for every
question these probes have actually asked, and it never lies. When you need the words, join
against `pdftotext -bbox`, which has poppler's decoder behind it.

EFFECTIVE SIZE
──────────────
`Tf` size times the text matrix's vertical scale times the CTM's — so a 1 pt font in a 12x matrix
reports 12.0, the number LibreOffice's own output would show for the same visual size. That is
what makes two renderings comparable when one scales the matrix and the other the size.
"""

import argparse
import math
import pathlib
import re
import subprocess
import sys
import xml.etree.ElementTree as ElementTree
import zlib

# How far apart two records may sit and still be the same drawing act.
#
# This has to be generous, and the first version got it wrong. Keying on position rounded to a
# tenth of a point matched nothing at all between two renderers: measured on one workbook, every
# show sits about 1 pt apart (51.39 against 52.38, 743.75 against 742.73) because the two put
# their page origin in slightly different places. Everything then reported as one-sided, which is
# the most useless answer a diff can give.
#
# So records are paired by *nearest neighbour* inside this window rather than by an exact key.
# Three points is wider than any origin offset seen here and far narrower than a line.
MATCH_WINDOW = 3.0
SIZE_TOLERANCE = 0.01

# A subset prefix is six capitals and a plus, assigned per file — `AAAAAA+Carlito` and
# `BAAAAA+Carlito` are the same face and comparing them raw reports a difference on every show.
# What is left after stripping it is the real name, and `Carlito` against `Carlito-Bold` is a
# finding rather than noise.
SUBSET_PREFIX = re.compile(r"^[A-Z]{6}\+")

TOKEN = re.compile(rb"""
      (?P<hex><[0-9A-Fa-f\s]*>)
    | (?P<name>/[^\s/\[\]<>(){}%]*)
    | (?P<num>[-+]?(?:\d*\.\d+|\d+\.?))
    | (?P<open>\[) | (?P<close>\])
    | (?P<dict_open><<) | (?P<dict_close>>>)
    | (?P<op>[A-Za-z'"*][A-Za-z0-9'"*]*)
""", re.VERBOSE)


def literal_string(data, start):
    """Read a `( ... )` string, honouring escapes and nesting. Returns (bytes, next index)."""
    out = bytearray()
    depth = 1
    i = start
    while i < len(data):
        c = data[i]
        if c == 0x5C:                                   # backslash
            i += 1
            if i >= len(data):
                break
            e = data[i]
            out += {0x6E: b"\n", 0x72: b"\r", 0x74: b"\t",
                    0x62: b"\b", 0x66: b"\f"}.get(e, bytes([e]))
            i += 1
            continue
        if c == 0x28:
            depth += 1
        elif c == 0x29:
            depth -= 1
            if depth == 0:
                return bytes(out), i + 1
        out.append(c)
        i += 1
    return bytes(out), i


def tokenise(data):
    """Yield (kind, value) over a content stream."""
    i = 0
    while i < len(data):
        c = data[i:i + 1]
        if c.isspace():
            i += 1
            continue
        if c == b"%":                                    # comment to end of line
            while i < len(data) and data[i:i + 1] not in (b"\n", b"\r"):
                i += 1
            continue
        if c == b"(":
            text, i = literal_string(data, i + 1)
            yield ("str", text)
            continue
        m = TOKEN.match(data, i)
        if not m:
            i += 1
            continue
        i = m.end()
        kind = m.lastgroup
        raw = m.group()
        if kind == "num":
            try:
                yield ("num", float(raw))
            except ValueError:
                pass
        elif kind == "hex":
            digits = re.sub(rb"\s", b"", raw[1:-1])
            if len(digits) % 2:
                digits += b"0"
            try:
                yield ("str", bytes.fromhex(digits.decode("ascii")))
            except Exception:
                yield ("str", b"")
        elif kind == "name":
            yield ("name", raw[1:].decode("latin1"))
        elif kind == "open":
            yield ("open", None)
        elif kind == "close":
            yield ("close", None)
        elif kind in ("dict_open", "dict_close"):
            yield (kind, None)
        else:
            yield ("op", raw.decode("latin1"))


def multiply(a, b):
    """a then b, both [x0 x1 y0 y1 dx dy] in PDF order."""
    return [
        a[0] * b[0] + a[1] * b[2],
        a[0] * b[1] + a[1] * b[3],
        a[2] * b[0] + a[3] * b[2],
        a[2] * b[1] + a[3] * b[3],
        a[4] * b[0] + a[5] * b[2] + b[4],
        a[4] * b[1] + a[5] * b[3] + b[5],
    ]


def apply(m, x, y):
    return (m[0] * x + m[2] * y + m[4], m[1] * x + m[3] * y + m[5])


def vertical_scale(m):
    return math.hypot(m[2], m[3]) or 1.0


def colour_hex(components):
    """Whatever colour space was set, reduced to RGB hex. Unknown spaces come back as None."""
    if len(components) == 1:
        g = max(0.0, min(1.0, components[0]))
        v = int(round(g * 255))
        return f"#{v:02X}{v:02X}{v:02X}"
    if len(components) == 3:
        r, g, b = (max(0.0, min(1.0, c)) for c in components)
        return f"#{int(round(r*255)):02X}{int(round(g*255)):02X}{int(round(b*255)):02X}"
    if len(components) == 4:
        c, m, y, k = (max(0.0, min(1.0, v)) for v in components)
        r, g, b = (1 - min(1.0, c + k), 1 - min(1.0, m + k), 1 - min(1.0, y + k))
        return f"#{int(round(r*255)):02X}{int(round(g*255)):02X}{int(round(b*255)):02X}"
    return None


class Objects:
    """Just enough of a PDF object store to reach page content and resources."""

    def __init__(self, blob):
        self.blob = blob
        self.offsets = {}
        for m in re.finditer(rb"(?m)^\s*(\d+)\s+(\d+)\s+obj\b", blob):
            self.offsets[int(m.group(1))] = m.end()

    def raw(self, number):
        start = self.offsets.get(number)
        if start is None:
            return None
        end = self.blob.find(b"endobj", start)
        return self.blob[start:end if end > 0 else len(self.blob)]

    def resolve(self, text):
        """Follow `N 0 R` once. `text` is a bytes fragment."""
        m = re.match(rb"\s*(\d+)\s+\d+\s+R\b", text)
        return self.raw(int(m.group(1))) if m else text

    def stream_of(self, body):
        m = re.search(rb"stream\r?\n", body)
        if not m:
            return b""
        data = body[m.end():]
        end = data.rfind(b"endstream")
        if end >= 0:
            data = data[:end]
        if b"FlateDecode" in body[:m.start()]:
            try:
                return zlib.decompressobj().decompress(data)
            except Exception:
                return b""
        return data

    def pages(self):
        """Page bodies in document order, best effort."""
        found = []
        for number, start in sorted(self.offsets.items()):
            body = self.raw(number)
            if body and re.search(rb"/Type\s*/Page\b", body) and b"/Pages" not in body[:200]:
                found.append(body)
        return found


def font_map(objects, page_body):
    """`/F1` → base font name, for whatever the page's resources reach."""
    names = {}
    m = re.search(rb"/Resources\s*(<<.*?>>|\d+\s+\d+\s+R)", page_body, re.S)
    if not m:
        return names
    resources = objects.resolve(m.group(1))
    if resources is None:
        return names
    f = re.search(rb"/Font\s*(<<.*?>>|\d+\s+\d+\s+R)", resources, re.S)
    if not f:
        return names
    fonts = objects.resolve(f.group(1))
    if fonts is None:
        return names
    for entry in re.finditer(rb"/([A-Za-z0-9.+-]+)\s+(\d+)\s+\d+\s+R", fonts):
        body = objects.raw(int(entry.group(2)))
        if not body:
            continue
        base = re.search(rb"/BaseFont\s*/([^\s/>\]]+)", body)
        names[entry.group(1).decode("latin1")] = (
            base.group(1).decode("latin1") if base else "?")
    return names


def interpret(stream, fonts, page_number):
    """Walk one content stream, emitting typed records."""
    records = []
    stack = []
    ctm = [1, 0, 0, 1, 0, 0]
    fill = stroke = None
    font = ""
    size = 0.0
    tm = tlm = [1, 0, 0, 1, 0, 0]
    leading = 0.0
    path = []
    operands = []
    array_depth = 0
    in_dict = 0

    def path_box():
        if not path:
            return None
        xs = [p[0] for p in path]
        ys = [p[1] for p in path]
        return (min(xs), min(ys), max(xs), max(ys))

    def numbers(count):
        vals = [v for k, v in operands if k == "num"]
        return vals[-count:] if len(vals) >= count else None

    for kind, value in tokenise(stream):
        if kind == "dict_open":
            in_dict += 1
            continue
        if kind == "dict_close":
            in_dict = max(0, in_dict - 1)
            continue
        if in_dict:
            continue
        if kind == "open":
            array_depth += 1
            operands.append((kind, value))
            continue
        if kind == "close":
            array_depth = max(0, array_depth - 1)
            operands.append((kind, value))
            continue
        if kind != "op":
            operands.append((kind, value))
            continue

        op = value
        if op == "q":
            stack.append((list(ctm), fill, stroke, font, size))
        elif op == "Q":
            if stack:
                ctm, fill, stroke, font, size = stack.pop()
                ctm = list(ctm)
        elif op == "cm":
            n = numbers(6)
            if n:
                ctm = multiply(n, ctm)
        elif op == "BT":
            tm = tlm = [1, 0, 0, 1, 0, 0]
        elif op == "Tf":
            names = [v for k, v in operands if k == "name"]
            n = numbers(1)
            if names:
                font = fonts.get(names[-1], names[-1])
            if n:
                size = n[0]
        elif op == "TL":
            n = numbers(1)
            if n:
                leading = n[0]
        elif op in ("Td", "TD"):
            n = numbers(2)
            if n:
                if op == "TD":
                    leading = -n[1]
                tlm = multiply([1, 0, 0, 1, n[0], n[1]], tlm)
                tm = list(tlm)
        elif op == "Tm":
            n = numbers(6)
            if n:
                tm = tlm = list(n)
        elif op == "T*":
            tlm = multiply([1, 0, 0, 1, 0, -leading], tlm)
            tm = list(tlm)
        elif op in ("Tj", "TJ", "'", '"'):
            if op in ("'", '"'):
                tlm = multiply([1, 0, 0, 1, 0, -leading], tlm)
                tm = list(tlm)
            glyphs = 0
            literal = []
            for k, v in operands:
                if k == "str":
                    glyphs += len(v)
                    literal.append(v)
            joined = b"".join(literal)
            try:
                text = joined.decode("ascii")
                if not text.isprintable():
                    text = ""
            except Exception:
                text = ""
            full = multiply(tm, ctm)
            x, y = apply(full, 0, 0)
            records.append({
                "kind": "text", "page": page_number, "x": x, "y": y,
                "size": size * vertical_scale(full),
                "font": font, "glyphs": glyphs, "text": text,
                "shows": 1 if op != "TJ" else sum(1 for k, _ in operands if k == "str"),
            })
        elif op in ("g", "rg", "k"):
            n = numbers({"g": 1, "rg": 3, "k": 4}[op])
            if n:
                fill = colour_hex(n)
        elif op in ("G", "RG", "K"):
            n = numbers({"G": 1, "RG": 3, "K": 4}[op])
            if n:
                stroke = colour_hex(n)
        elif op in ("sc", "scn"):
            n = [v for k, v in operands if k == "num"]
            if n:
                fill = colour_hex(n) or fill
        elif op in ("SC", "SCN"):
            n = [v for k, v in operands if k == "num"]
            if n:
                stroke = colour_hex(n) or stroke
        elif op in ("m", "l"):
            n = numbers(2)
            if n:
                path.append(apply(ctm, n[0], n[1]))
        elif op in ("c", "v", "y"):
            n = numbers(6 if op == "c" else 4)
            if n:
                for i in range(0, len(n) - 1, 2):
                    path.append(apply(ctm, n[i], n[i + 1]))
        elif op == "re":
            n = numbers(4)
            if n:
                x, y, w, h = n
                for cx, cy in ((x, y), (x + w, y), (x + w, y + h), (x, y + h)):
                    path.append(apply(ctm, cx, cy))
        elif op in ("f", "F", "f*", "B", "B*", "b", "b*", "S", "s"):
            box = path_box()
            if box:
                painted = "fill" if op[0] in "fFBb" else "stroke"
                records.append({
                    "kind": painted, "page": page_number,
                    "x0": box[0], "y0": box[1], "x1": box[2], "y1": box[3],
                    "colour": (fill if painted == "fill" else stroke) or "?",
                    "op": op,
                })
            path = []
        elif op == "n":
            path = []
        elif op == "Do":
            names = [v for k, v in operands if k == "name"]
            corners = [apply(ctm, x, y) for x, y in ((0, 0), (1, 0), (1, 1), (0, 1))]
            xs = [p[0] for p in corners]
            ys = [p[1] for p in corners]
            records.append({
                "kind": "image", "page": page_number,
                "x0": min(xs), "y0": min(ys), "x1": max(xs), "y1": max(ys),
                "name": names[-1] if names else "?",
            })
        operands = []
    return records


def words_by_page(pdf):
    """Poppler's decoded words with boxes, per page — the text a subset font hides from us.

    Reading a subset's hex strings needs its ToUnicode CMap, and reimplementing that is a large
    surface to get quietly wrong. pdftotext already has poppler's decoder behind it, so the
    honest move is to ask it and join on position rather than decode badly here.
    """
    try:
        out = subprocess.run(["pdftotext", "-bbox", str(pdf), "-"],
                             capture_output=True, timeout=240).stdout
        root = ElementTree.fromstring(out)
    except Exception:
        return {}
    pages = {}
    namespace = "{http://www.w3.org/1999/xhtml}"
    for number, page in enumerate(root.iter(f"{namespace}page"), start=1):
        height = float(page.get("height", 0) or 0)
        found = []
        for word in page.iter(f"{namespace}word"):
            try:
                # pdftotext measures from the top; the content stream measures from the bottom.
                found.append((float(word.get("xMin")), height - float(word.get("yMax")),
                              (word.text or "")))
            except Exception:
                continue
        pages[number] = found
    return pages


def attach_text(records, pdf):
    """Give each text record the words whose boxes start nearest it."""
    pages = words_by_page(pdf)
    for record in records:
        if record["kind"] != "text" or record.get("text"):
            continue
        near = []
        for x, y, word in pages.get(record["page"], ()):
            if abs(y - record["y"]) <= MATCH_WINDOW and x >= record["x"] - MATCH_WINDOW:
                near.append((x, word))
        near.sort()
        joined = " ".join(w for _, w in near[:12])
        record["text"] = joined[:60]
    return records


def read(pdf):
    blob = pathlib.Path(pdf).read_bytes()
    objects = Objects(blob)
    out = []
    for number, body in enumerate(objects.pages(), start=1):
        fonts = font_map(objects, body)
        m = re.search(rb"/Contents\s*(\[[^\]]*\]|\d+\s+\d+\s+R)", body)
        if not m:
            continue
        streams = b""
        target = m.group(1)
        for ref in re.finditer(rb"(\d+)\s+\d+\s+R", target):
            part = objects.raw(int(ref.group(1)))
            if part:
                streams += objects.stream_of(part) + b"\n"
        out.extend(interpret(streams, fonts, number))
    return attach_text(out, pdf)


def show(record):
    if record["kind"] == "text":
        body = f' "{record["text"][:40]}"' if record["text"] else ""
        return (f'text  p{record["page"]:<3} ({record["x"]:8.2f},{record["y"]:8.2f}) '
                f'{record["size"]:6.2f}pt {record["font"][:28]:<28} '
                f'{record["glyphs"]:4d} glyphs in {record["shows"]} show(s){body}')
    if record["kind"] in ("fill", "stroke"):
        return (f'{record["kind"]:<5} p{record["page"]:<3} '
                f'({record["x0"]:8.2f},{record["y0"]:8.2f})-({record["x1"]:8.2f},{record["y1"]:8.2f}) '
                f'{record["colour"]} [{record["op"]}]')
    return (f'image p{record["page"]:<3} '
            f'({record["x0"]:8.2f},{record["y0"]:8.2f})-({record["x1"]:8.2f},{record["y1"]:8.2f}) '
            f'{record["name"]}')


def face(name):
    """A font name with its per-file subset prefix removed."""
    return SUBSET_PREFIX.sub("", name or "")


def anchor(record):
    """The point a record is matched on."""
    if record["kind"] == "text":
        return record["x"], record["y"]
    return record["x0"], record["y1"]


def compare(a, b):
    """What differs between two records that are the same drawing act, as notes."""
    notes = []
    if a["kind"] == "text":
        if abs(a["size"] - b["size"]) > SIZE_TOLERANCE:
            notes.append(f'size {a["size"]:.2f} vs {b["size"]:.2f}')
        if face(a["font"]) != face(b["font"]):
            notes.append(f'face {face(a["font"])} vs {face(b["font"])}')
        if a["glyphs"] != b["glyphs"]:
            notes.append(f'glyphs {a["glyphs"]} vs {b["glyphs"]}')
        if a["shows"] != b["shows"]:
            notes.append(f'shows {a["shows"]} vs {b["shows"]}')
    else:
        if a.get("colour") != b.get("colour"):
            notes.append(f'colour {a.get("colour")} vs {b.get("colour")}')
        if a["kind"] != "image":
            aw, ah = a["x1"] - a["x0"], a["y1"] - a["y0"]
            bw, bh = b["x1"] - b["x0"], b["y1"] - b["y0"]
            if abs(aw - bw) > MATCH_WINDOW or abs(ah - bh) > MATCH_WINDOW:
                notes.append(f"size {aw:.1f}x{ah:.1f} vs {bw:.1f}x{bh:.1f}")
    return notes


def differences(mine, theirs):
    """Records only one side draws, and records both draw differently.

    Greedy nearest-neighbour within `MATCH_WINDOW`, per page and per kind. Greedy is enough
    because the alternative — two records of one kind within three points of each other on one
    page — is a page where every answer is arguable anyway.
    """
    only_ours, only_ref, changed = [], [], []
    pages = {r["page"] for r in mine} | {r["page"] for r in theirs}
    for page in sorted(pages):
        for kind in ("text", "fill", "stroke", "image"):
            ours = [r for r in mine if r["page"] == page and r["kind"] == kind]
            ref = [r for r in theirs if r["page"] == page and r["kind"] == kind]
            taken = set()
            for a in ours:
                ax, ay = anchor(a)
                best, best_distance = None, None
                for i, b in enumerate(ref):
                    if i in taken:
                        continue
                    bx, by = anchor(b)
                    if abs(ax - bx) > MATCH_WINDOW or abs(ay - by) > MATCH_WINDOW:
                        continue
                    distance = math.hypot(ax - bx, ay - by)
                    if best_distance is None or distance < best_distance:
                        best, best_distance = i, distance
                if best is None:
                    only_ours.append(a)
                    continue
                taken.add(best)
                notes = compare(a, ref[best])
                if notes:
                    changed.append((a, "; ".join(notes)))
            only_ref.extend(b for i, b in enumerate(ref) if i not in taken)
    return only_ours, only_ref, changed


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("mode", choices=("dump", "diff"))
    parser.add_argument("pdf")
    parser.add_argument("reference", nargs="?")
    parser.add_argument("--page", type=int)
    parser.add_argument("--only", choices=("text", "fill", "stroke", "image"))
    args = parser.parse_args()

    def keep(records):
        out = records
        if args.page:
            out = [r for r in out if r["page"] == args.page]
        if args.only:
            out = [r for r in out if r["kind"] == args.only]
        return out

    if args.mode == "dump":
        for r in keep(read(args.pdf)):
            print(show(r))
        return 0

    if not args.reference:
        print("diff needs two PDFs", file=sys.stderr)
        return 2
    only_ours, only_ref, changed = differences(keep(read(args.pdf)), keep(read(args.reference)))

    print(f"=== only in ours ({len(only_ours)})")
    for r in sorted(only_ours, key=lambda r: (r["page"], -r.get("y", r.get("y1", 0)))):
        print("  " + show(r))
    print(f"=== only in the reference ({len(only_ref)})")
    for r in sorted(only_ref, key=lambda r: (r["page"], -r.get("y", r.get("y1", 0)))):
        print("  " + show(r))
    print(f"=== drawn by both, differently ({len(changed)})")
    for r, note in sorted(changed, key=lambda p: (p[0]["page"], -p[0].get("y", p[0].get("y1", 0)))):
        print("  " + show(r))
        print(f"      {note}")
    return 1 if (only_ours or only_ref or changed) else 0


if __name__ == "__main__":
    sys.exit(main())
