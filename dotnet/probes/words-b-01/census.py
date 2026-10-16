#!/usr/bin/env python3
"""How many DOCX resolve to a label whose line box the item's does not cover?

    census.py /abs/corpus/words

Resolves rather than declares.  For every `w:p` it walks

    w:docDefaults -> w:style chain through w:basedOn -> w:pPr/w:rPr (the paragraph MARK,
    which is what a label inherits) -> w:lvl/w:rPr

for the size and the face on both sides, resolves the numbering through `w:numPr`, the
paragraph style's own `w:numPr`, `w:numStyleLink` and `w:lvlOverride`, and then turns each
side into a real ascent and descent by loading the font `fc-match` gives for the family —
the same substitution LibreOffice performs, and the reason a face-driven label is counted
here rather than left in a band that cannot be scored.

Four populations, because the round found four separate rules:

    label        the paragraph resolves to a drawn label at all
    taller-box   the label's whole box exceeds the item's      — our gate already fires
    deeper-only  the label's DESCENT exceeds the item's while its box and ascent do not
                 — the population our gate drops on the floor
    prop>100     proportional line spacing above 100%          — round 47's population
    prop<100     proportional line spacing below 100%
    atLeast-bind w:lineRule="atLeast" whose minimum exceeds the natural line height
"""
from __future__ import annotations

import re
import subprocess
import sys
import zipfile
from collections import defaultdict
from pathlib import Path
from xml.etree import ElementTree as ET

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
_metrics: dict[str, tuple[float, float]] = {}


def metrics(family: str) -> tuple[float, float]:
    """(ascent, descent) per point of em size, through fontconfig's own substitution."""
    key = (family or "").strip().lower()
    if key in _metrics:
        return _metrics[key]
    try:
        from fontTools.ttLib import TTFont
        path = subprocess.run(["fc-match", "-f", "%{file}", family or "serif"],
                              capture_output=True, text=True, timeout=30).stdout.strip()
        f = TTFont(path, fontNumber=0, lazy=True)
        upem = f["head"].unitsPerEm
        h = f["hhea"]
        got = ((h.ascender + h.lineGap) / upem, -h.descender / upem)
    except Exception:
        got = (1911 / 2048, 443 / 2048)
    _metrics[key] = got
    return got


def child(node, name):
    return None if node is None else node.find(W + name)


def val(node, name, attr="val"):
    got = child(node, name)
    return None if got is None else got.get(W + attr)


def number(node, name, attr="val"):
    got = val(node, name, attr)
    try:
        return int(got)
    except (TypeError, ValueError):
        return None


class Styles:
    def __init__(self, root):
        self.by_id: dict[str, ET.Element] = {}
        self.default_para: str | None = None
        self.doc_size = 20.0
        self.doc_face = ""
        self.doc_spacing = None
        if root is None:
            return
        defaults = child(root, "docDefaults")
        rpr = child(child(defaults, "rPrDefault"), "rPr")
        if (size := number(rpr, "sz")) is not None:
            self.doc_size = size
        fonts = child(rpr, "rFonts")
        if fonts is not None:
            self.doc_face = fonts.get(W + "ascii") or ""
        self.doc_spacing = child(child(child(defaults, "pPrDefault"), "pPr"), "spacing")
        for style in root.findall(W + "style"):
            sid = style.get(W + "styleId")
            if sid:
                self.by_id[sid] = style
            if style.get(W + "type") == "paragraph" and style.get(W + "default") == "1":
                self.default_para = sid

    def chain(self, style_id: str | None):
        seen, out = set(), []
        while style_id and style_id in self.by_id and style_id not in seen:
            seen.add(style_id)
            style = self.by_id[style_id]
            out.append(style)
            style_id = val(style, "basedOn")
        return out

    def lookup(self, style_id: str | None, run: bool, name: str):
        for style in self.chain(style_id) + self.chain(self.default_para):
            holder = child(style, "rPr" if run else "pPr")
            got = child(holder, name)
            if got is not None:
                return got
        return None


class Numbering:
    def __init__(self, root):
        self.abstract: dict[str, ET.Element] = {}
        self.num: dict[str, tuple[str, ET.Element]] = {}
        self.for_style: dict[str, str] = {}
        if root is None:
            return
        for node in root.findall(W + "abstractNum"):
            aid = node.get(W + "abstractNumId")
            if aid:
                self.abstract[aid] = node
        for node in root.findall(W + "num"):
            nid = node.get(W + "numId")
            aid = val(node, "abstractNumId")
            if nid and aid:
                self.num[nid] = (aid, node)
        for aid, node in self.abstract.items():
            link = val(node, "styleLink")
            if link:
                for nid, (other, _) in self.num.items():
                    if other == aid:
                        self.for_style[link] = nid

    def level(self, num_id: str, ilvl: int):
        entry = self.num.get(num_id)
        if entry is None:
            return None
        aid, num = entry
        for override in num.findall(W + "lvlOverride"):
            if override.get(W + "ilvl") == str(ilvl):
                own = child(override, "lvl")
                if own is not None:
                    return own
        abstract = self.abstract.get(aid)
        if abstract is None:
            return None
        link = val(abstract, "numStyleLink")
        if link and link in self.for_style and self.for_style[link] != num_id:
            return self.level(self.for_style[link], ilvl)
        for lvl in abstract.findall(W + "lvl"):
            if lvl.get(W + "ilvl") == str(ilvl):
                return lvl
        return None


def part(zf, name):
    try:
        return ET.fromstring(zf.read(name))
    except (KeyError, ET.ParseError):
        return None


def census(path: Path) -> dict[str, int]:
    counts: dict[str, int] = defaultdict(int)
    with zipfile.ZipFile(path) as zf:
        doc = part(zf, "word/document.xml")
        if doc is None:
            return counts
        styles = Styles(part(zf, "word/styles.xml"))
        numbering = Numbering(part(zf, "word/numbering.xml"))

    for para in doc.iter(W + "p"):
        ppr = child(para, "pPr")
        style_id = val(ppr, "pStyle")
        mark = child(ppr, "rPr")

        size = number(mark, "sz")
        if size is None:
            got = styles.lookup(style_id, True, "sz")
            raw = got.get(W + "val") if got is not None else None
            if raw and raw.isdigit():
                size = int(raw)
        if size is None:
            size = styles.doc_size
        points = size / 2.0

        fonts = child(mark, "rFonts") or styles.lookup(style_id, True, "rFonts")
        face = (fonts.get(W + "ascii") if fonts is not None else None) or styles.doc_face

        spacing = child(ppr, "spacing") or styles.lookup(style_id, False, "spacing") \
            or styles.doc_spacing
        rule = spacing.get(W + "lineRule") if spacing is not None else None
        line = spacing.get(W + "line") if spacing is not None else None
        line = int(line) if line and line.lstrip("-").isdigit() else None

        item_asc, item_desc = (m * points for m in metrics(face))

        if rule == "auto" and line:
            percent = line * 100 // 240
            if percent > 100:
                counts["prop>100"] += 1
            elif percent < 100:
                counts["prop<100"] += 1
        elif rule == "atLeast" and line and line / 20.0 > item_asc + item_desc:
            counts["atLeast-bind"] += 1

        numpr = child(ppr, "numPr")
        num_id = val(numpr, "numId")
        ilvl = number(numpr, "ilvl") or 0
        if num_id is None and style_id:
            from_style = None
            for style in styles.chain(style_id):
                from_style = child(child(style, "pPr"), "numPr")
                if from_style is not None:
                    break
            if from_style is not None:
                num_id = val(from_style, "numId")
                ilvl = number(from_style, "ilvl") or 0
            elif style_id in numbering.for_style:
                num_id = numbering.for_style[style_id]
        if num_id in (None, "0"):
            continue
        lvl = numbering.level(num_id, max(0, min(8, ilvl)))
        if lvl is None:
            continue
        if val(lvl, "numFmt") == "none":
            continue
        counts["label"] += 1

        lvl_rpr = child(lvl, "rPr")
        lvl_size = number(lvl_rpr, "sz")
        lvl_points = lvl_size / 2.0 if lvl_size else points
        lvl_fonts = child(lvl_rpr, "rFonts")
        lvl_face = (lvl_fonts.get(W + "ascii") if lvl_fonts is not None else None) or face

        asc, desc = (m * lvl_points for m in metrics(lvl_face))
        if asc + desc > item_asc + item_desc + 1e-6 or asc > item_asc + 1e-6:
            counts["taller-box"] += 1
            if rule == "auto" and line and line * 100 // 240 > 100:
                counts["taller-box+prop"] += 1
        elif desc > item_desc + 1e-6:
            counts["deeper-only"] += 1
            if lvl_face.strip().lower() != (face or "").strip().lower():
                counts["deeper-only-by-face"] += 1
    return counts


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    root = Path(sys.argv[1])
    files = sorted(p for p in root.rglob("*") if p.suffix.lower() == ".docx")
    keys = ["label", "taller-box", "taller-box+prop", "deeper-only", "deeper-only-by-face",
            "prop>100", "prop<100", "atLeast-bind"]
    docs = defaultdict(int)
    paras = defaultdict(int)
    rows = []
    for path in files:
        try:
            got = census(path)
        except Exception as error:  # noqa: BLE001
            print(f"SKIP {path.name}: {error}", file=sys.stderr)
            continue
        for key in keys:
            if got.get(key):
                docs[key] += 1
                paras[key] += got[key]
        rows.append((path.name, got))

    print(f"{len(files)} DOCX read\n")
    print(f"{'population':>22} {'documents':>10} {'paragraphs':>11}")
    for key in keys:
        print(f"{key:>22} {docs[key]:>10} {paras[key]:>11}")

    print("\ndocuments in deeper-only (the population our gate drops):")
    for name, got in rows:
        if got.get("deeper-only"):
            print(f"  {got['deeper-only']:>5}  {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
