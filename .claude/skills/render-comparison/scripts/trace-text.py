#!/usr/bin/env python3
"""Rewrite a document's text with unique traceable tokens, and resolve them back.

The problem this solves: `pdf-ops.py` will tell you that a text record moved, changed
size or lost its face, and it will quote the text it holds. What it cannot tell you is
*which part of the source produced it* — and on a real document the same word appears
thirty times, so the quoted text does not identify anything. Chasing "the text on page 3
is 4pt low" then means reading the markup and guessing which paragraph that was.

So: replace every ASCII word in the document with a token that appears exactly once in
the whole file, keeping the character count identical wherever the counter fits. Render
the rewritten document through both renderers, diff the operators, and every differing
record names a token. `resolve` turns that token back into part, element, ordinal and the
original surrounding sentence.

    trace-text.py rewrite in.docx out.docx --map map.tsv
    trace-text.py locate map.tsv 3fX 41X
    pdf-ops.py diff ours.pdf ref.pdf | trace-text.py resolve map.tsv

Two properties matter and are worth stating because they are what make the output
comparable to the original document's:

* **The document's word count is preserved exactly.** Only maximal runs of
  ``[A-Za-z0-9]`` are replaced; whitespace, punctuation and every non-ASCII script are
  left alone — verified with ``paperless extract``, which reads the model rather than the
  page, at 1142 words against 1142 on a deck and 604 against 604 on a workbook. A
  ``pdftotext`` count can still drift about a percent, because extraction from a PDF
  re-infers word boundaries from geometry and a token's advances are not the replaced
  word's. That drift applies equally to both renderers of the same rewritten file, so it
  leaves the comparison you are actually running intact; it only rules out comparing a
  rewritten file's word count against the original's.
* **Character count is preserved wherever it can be.** A token is base-36 of its index
  padded to the original length with ``X``, which is outside the base-36 alphabet and so
  cannot collide with an encoded digit. Words too short to hold the counter get a longer
  token; the run tallies how many, because that is exactly the population whose line
  breaking the rewrite may have changed.

Equal character count is *not* equal width — every glyph is a different advance — so a
rewritten document does not lay out identically to its original and must never be used as
a fidelity reference. It is an instrument for attribution: run it on a document you
already know differs, to find out which source run the difference belongs to.

Binary formats (.doc/.xls/.ppt) are not supported. Converting one first would change the
layout under study, which defeats the purpose.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path
from xml.etree import ElementTree as ET

# ---------------------------------------------------------------------------
# Token alphabet
# ---------------------------------------------------------------------------

DIGITS = "0123456789abcdefghijklmnopqrstuvwxyz"
PAD = "X"  # outside the base-36 alphabet, so stripping it recovers the index unambiguously

WORD = re.compile(r"[A-Za-z0-9]+")
TOKEN = re.compile(r"\b([0-9a-z]+X*)\b")
TOKEN_PADDED = re.compile(r"\b([0-9a-z]*X+)\b")
QUOTED = re.compile(r'"([^"]*)"')

CONTEXT_CHARS = 60


def encode(index: int) -> str:
    if index == 0:
        return DIGITS[0]
    out = []
    while index:
        index, rem = divmod(index, 36)
        out.append(DIGITS[rem])
    return "".join(reversed(out))


def decode(token: str) -> int | None:
    body = token.rstrip(PAD)
    if not body or any(c not in DIGITS for c in body):
        return None
    value = 0
    for c in body:
        value = value * 36 + DIGITS.index(c)
    return value


# ---------------------------------------------------------------------------
# Which parts and which elements hold prose
# ---------------------------------------------------------------------------

W = "{http://schemas.openxmlformats.org/wordprocessingml/2006/main}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"
S = "{http://schemas.openxmlformats.org/spreadsheetml/2006/main}"
TEXT = "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}"
OFFICE = "{urn:oasis:names:tc:opendocument:xmlns:office:1.0}"

# Field codes, formulas and the like look like prose and are not. Rewriting one produces
# a document that still opens and renders something entirely different, which is the
# worst possible failure mode for a diagnostic tool.
SKIP_ELEMENTS = {
    W + "instrText",  # field code: PAGE, REF, HYPERLINK …
    W + "delText",  # tracked deletion, not drawn
    S + "f",  # formula
    S + "v",  # cached value / shared-string index
    TEXT + "page-number",
    TEXT + "page-count",
    TEXT + "date",
    TEXT + "time",
    TEXT + "sequence",
    TEXT + "placeholder",
}

# Ancestors whose whole subtree is out of bounds.
SKIP_SUBTREES = {
    W + "fldSimple",
    TEXT + "tracked-changes",
}


@dataclass
class Family:
    name: str
    parts: re.Pattern[str]
    elements: frozenset[str]


FAMILIES = (
    Family(
        "wordprocessingml",
        re.compile(r"^word/(document|header\d*|footer\d*|footnotes|endnotes|comments)\.xml$"),
        frozenset({W + "t", A + "t"}),
    ),
    Family(
        "presentationml",
        re.compile(r"^ppt/(slides|notesSlides|slideLayouts|slideMasters)/[^/]+\.xml$"),
        frozenset({A + "t"}),
    ),
    Family(
        "spreadsheetml",
        re.compile(r"^xl/(sharedStrings\.xml|worksheets/[^/]+\.xml|drawings/[^/]+\.xml)$"),
        frozenset({S + "t", A + "t"}),
    ),
    Family(
        "opendocument",
        re.compile(r"^(content|styles)\.xml$"),
        frozenset(),  # decided structurally — see rewrite_odf
    ),
)


def family_for(archive: zipfile.ZipFile) -> Family | None:
    names = set(archive.namelist())
    if "word/document.xml" in names:
        return FAMILIES[0]
    if any(n.startswith("ppt/slides/slide") for n in names):
        return FAMILIES[1]
    if "xl/workbook.xml" in names:
        return FAMILIES[2]
    if "content.xml" in names:
        return FAMILIES[3]
    return None


# ---------------------------------------------------------------------------
# The rewrite
# ---------------------------------------------------------------------------


@dataclass
class Entry:
    token: str
    original: str
    part: str
    element: str
    node: int
    word: int
    context: str


class Rewriter:
    def __init__(self) -> None:
        self.index = 0
        self.entries: list[Entry] = []
        self.grew = 0

    def string(self, text: str, part: str, element: str, node: int) -> str:
        out: list[str] = []
        last = 0
        word_ordinal = 0
        for match in WORD.finditer(text):
            out.append(text[last : match.start()])
            original = match.group()
            token = self.mint(len(original))
            out.append(token)
            self.entries.append(
                Entry(
                    token=token,
                    original=original,
                    part=part,
                    element=element.rpartition("}")[2],
                    node=node,
                    word=word_ordinal,
                    context=context_of(text, match.start(), match.end()),
                )
            )
            word_ordinal += 1
            last = match.end()
        if not out:
            return text
        out.append(text[last:])
        return "".join(out)

    def mint(self, length: int) -> str:
        body = encode(self.index)
        self.index += 1
        if len(body) > length:
            self.grew += 1
            return body
        return body + PAD * (length - len(body))


def context_of(text: str, start: int, end: int) -> str:
    half = CONTEXT_CHARS // 2
    left = text[max(0, start - half) : start]
    right = text[end : end + half]
    joined = f"{left}‹{text[start:end]}›{right}"
    return " ".join(joined.split())


def strip_namespace(tag: str) -> str:
    return tag.rpartition("}")[2]


def rewrite_ooxml(root: ET.Element, family: Family, part: str, rw: Rewriter) -> None:
    node = 0

    def walk(element: ET.Element) -> None:
        nonlocal node
        for child in element:
            if child.tag in SKIP_SUBTREES:
                continue
            if child.tag in family.elements and child.tag not in SKIP_ELEMENTS:
                if child.text:
                    child.text = rw.string(child.text, part, child.tag, node)
                node += 1
            walk(child)

    walk(root)


def rewrite_odf(root: ET.Element, part: str, rw: Rewriter) -> None:
    """ODF puts prose in the text of `text:*` elements and, just as often, in the *tail*
    of an inline child — a `text:span` splits its parent's run in two and the remainder
    arrives as a tail. Handling only `.text` silently leaves half of a formatted
    paragraph unrewritten, which reads as the tool having missed those words."""
    body = root.find(OFFICE + "body")
    if body is None:
        return
    node = 0

    def walk(element: ET.Element) -> None:
        nonlocal node
        for child in element:
            if child.tag in SKIP_SUBTREES or child.tag in SKIP_ELEMENTS:
                continue
            if child.text and child.tag.startswith(TEXT):
                child.text = rw.string(child.text, part, child.tag, node)
                node += 1
            walk(child)
            if child.tail and element.tag.startswith(TEXT):
                child.tail = rw.string(child.tail, part, element.tag, node)
                node += 1

    walk(body)


def register_namespaces(data: bytes) -> None:
    """ElementTree renames every namespace to ns0, ns1 … on write unless the prefixes are
    registered first. Some consumers cope; LibreOffice's OOXML import does not reliably,
    and a document that imports differently is useless as a probe."""
    for prefix, uri in re.findall(rb'xmlns:([A-Za-z0-9_.-]+)\s*=\s*"([^"]+)"', data[:8192]):
        ET.register_namespace(prefix.decode(), uri.decode())
    default = re.search(rb'xmlns\s*=\s*"([^"]+)"', data[:8192])
    if default:
        ET.register_namespace("", default.group(1).decode())


def rewrite(source: Path, target: Path, map_path: Path) -> int:
    if not zipfile.is_zipfile(source):
        print(
            f"{source.name}: not a zip container. Binary .doc/.xls/.ppt cannot be "
            f"rewritten in place, and converting first would change the layout under "
            f"study.",
            file=sys.stderr,
        )
        return 2

    rw = Rewriter()
    with zipfile.ZipFile(source) as archive:
        family = family_for(archive)
        if family is None:
            print(f"{source.name}: unrecognised package layout", file=sys.stderr)
            return 2
        infos = archive.infolist()
        payload = {info.filename: archive.read(info.filename) for info in infos}

    touched = []
    for name, data in payload.items():
        if not family.parts.match(name):
            continue
        try:
            register_namespaces(data)
            root = ET.fromstring(data)
        except ET.ParseError as exc:
            print(f"{name}: unparsable ({exc}) — left as found", file=sys.stderr)
            continue
        before = rw.index
        if family.name == "opendocument":
            rewrite_odf(root, name, rw)
        else:
            rewrite_ooxml(root, family, name, rw)
        if rw.index == before:
            continue
        payload[name] = ET.tostring(root, encoding="UTF-8", xml_declaration=True)
        touched.append((name, rw.index - before))

    target.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(target, "w", zipfile.ZIP_DEFLATED) as out:
        for info in infos:
            data = payload[info.filename]
            # ODF requires `mimetype` first and stored uncompressed; a repack that
            # ignores that produces a file some readers refuse outright.
            method = zipfile.ZIP_STORED if info.filename == "mimetype" else zipfile.ZIP_DEFLATED
            new = zipfile.ZipInfo(info.filename, date_time=info.date_time)
            new.compress_type = method
            new.external_attr = info.external_attr
            out.writestr(new, data)

    with map_path.open("w", encoding="utf-8") as fh:
        fh.write("token\toriginal\tpart\telement\tnode\tword\tcontext\n")
        for e in rw.entries:
            fh.write(
                f"{e.token}\t{e.original}\t{e.part}\t{e.element}\t{e.node}\t{e.word}\t{e.context}\n"
            )

    print(f"{family.name}: {rw.index} tokens across {len(touched)} parts -> {target}")
    for name, count in touched:
        print(f"  {count:6d}  {name}")
    if rw.grew:
        print(
            f"  {rw.grew} token(s) longer than the word replaced — those runs may break "
            f"differently from the original"
        )
    print(f"map: {map_path}")
    return 0


# ---------------------------------------------------------------------------
# Resolving tokens back to source
# ---------------------------------------------------------------------------


def load_map(path: Path) -> dict[str, Entry]:
    table: dict[str, Entry] = {}
    with path.open(encoding="utf-8") as fh:
        header = fh.readline()
        if not header.startswith("token\t"):
            print(f"{path}: not a trace-text map", file=sys.stderr)
            return table
        for line in fh:
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 7:
                continue
            table[parts[0]] = Entry(
                token=parts[0],
                original=parts[1],
                part=parts[2],
                element=parts[3],
                node=int(parts[4]),
                word=int(parts[5]),
                context=parts[6],
            )
    return table


def describe(e: Entry) -> str:
    return f"{e.part}  {e.element}[{e.node}].{e.word}  {e.original!r}  {e.context}"


def locate(map_path: Path, tokens: list[str]) -> int:
    table = load_map(map_path)
    if not table:
        return 2
    missing = 0
    for token in tokens:
        entry = table.get(token)
        if entry is None:
            print(f"{token}\t— not in map")
            missing += 1
        else:
            print(f"{token}\t{describe(entry)}")
    return 1 if missing else 0


def resolve(map_path: Path, stream) -> int:
    """Read anything on stdin — a pdf-ops diff, a pdftotext dump — and append the source
    location of every token it mentions. Lines with no token pass through unchanged, so
    piping a whole report through this is lossless.

    A short word mints a short token, and a short token is indistinguishable from an
    ordinary number: run over a `pdf-ops.py` diff line naively and `p1`, `17 glyphs` and
    the x-coordinate `342.78` all resolve to whichever words happen to hold indices 1, 17
    and 34. Two rules keep that out. When the line carries quoted text — as every pdf-ops
    record does — only the quoted part is searched, and every token in it counts. When it
    does not, the whole line is searched but only *padded* tokens count, because a padded
    token contains an `X` and so cannot be a number. The cost is that a bare `pdftotext`
    dump loses the handful of words short enough to mint an unpadded token; the
    alternative was reporting a false source for every integer in the report.
    """
    table = load_map(map_path)
    if not table:
        return 2
    for line in stream:
        line = line.rstrip("\n")
        quoted = QUOTED.findall(line)
        haystacks, pattern = (quoted, TOKEN) if quoted else ([line], TOKEN_PADDED)
        seen: list[str] = []
        for haystack in haystacks:
            for match in pattern.finditer(haystack):
                entry = table.get(match.group(1))
                if entry is not None and match.group(1) not in seen:
                    seen.append(match.group(1))
        print(line)
        for token in seen:
            print(f"    ↳ {token}  {describe(table[token])}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="mode", required=True)

    r = sub.add_parser("rewrite", help="replace every ASCII word with a unique token")
    r.add_argument("source", type=Path)
    r.add_argument("target", type=Path)
    r.add_argument("--map", type=Path, required=True, help="where to write the token map")

    l = sub.add_parser("locate", help="print the source location of named tokens")
    l.add_argument("map", type=Path)
    l.add_argument("tokens", nargs="+")

    v = sub.add_parser("resolve", help="annotate stdin with the source of any token in it")
    v.add_argument("map", type=Path)

    args = parser.parse_args()
    if args.mode == "rewrite":
        return rewrite(args.source, args.target, args.map)
    if args.mode == "locate":
        return locate(args.map, args.tokens)
    return resolve(args.map, sys.stdin)


if __name__ == "__main__":
    sys.exit(main())
