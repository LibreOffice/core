#!/usr/bin/env python3
"""Score how much of a renderer a PDF actually exercises.

Given PDFs produced by LibreOffice from a corpus of office documents, this reports
what each one demands of a renderer, so a corpus can be worked through in order of
difficulty rather than alphabetically.

The score is deliberately *not* a quality judgement and not a page count. It counts
the distinct capabilities a document forces a renderer to have, because that is what
predicts where a port breaks: a fifty-page memo in one face is easier than a single
slide carrying a gradient, a clipped picture and a rotated table.

Emits TSV on stdout: path, ext, pages, fonts, images, shadings, groups, paths,
textops, clips, score, band.
"""
import re
import sys
import zlib
from pathlib import Path

# What each capability costs. Weights are ratios of "how often does getting this
# wrong produce a visibly wrong page", not of implementation effort — a document
# with one gradient is a harder *test* than one with a thousand more glyphs.
WEIGHTS = {
    "pages": 0.5,
    "fonts": 6.0,      # a second face means fallback, embedding and metrics
    "images": 8.0,     # decode, colour space, transparency mask
    "shadings": 14.0,  # gradients: the axial/radial/mesh machinery
    "groups": 10.0,    # transparency groups
    "clips": 3.0,
    "paths": 0.02,     # vector density; individually cheap, collectively telling
    "textops": 0.01,
}

# Where the bands fall. Chosen so that a plain prose document lands in `low` and
# anything with a picture or a gradient cannot.
BANDS = [(25.0, "01-low"), (70.0, "02-moderate"), (160.0, "03-complex")]
TOP = "04-demanding"


def streams(data: bytes):
    """Every content stream in the file, inflated where it is deflated."""
    for m in re.finditer(rb"stream\r?\n(.*?)endstream", data, re.S):
        raw = m.group(1)
        try:
            yield zlib.decompress(raw)
        except zlib.error:
            yield raw  # already plain, or a filter we do not need to read


def score(path: Path) -> dict | None:
    try:
        data = path.read_bytes()
    except OSError:
        return None
    if not data.startswith(b"%PDF"):
        return None

    ops = {"paths": 0, "textops": 0, "clips": 0}
    for s in streams(data):
        t = s.decode("latin-1", "replace")
        ops["paths"] += len(re.findall(r"\b(?:re|m|c|v|y)\b", t))
        ops["textops"] += len(re.findall(r"\b(?:Tj|TJ)\b", t))
        ops["clips"] += len(re.findall(r"\bW\*?\b", t))

    counts = {
        "pages": len(re.findall(rb"/Type\s*/Page\b(?!s)", data)) or 1,
        # A face is counted once per descriptor, so a subset split across objects
        # does not inflate the number the way /Font references would.
        "fonts": len(re.findall(rb"/Type\s*/FontDescriptor", data)),
        "images": len(re.findall(rb"/Subtype\s*/Image", data)),
        "shadings": len(re.findall(rb"/ShadingType", data)),
        "groups": len(re.findall(rb"/S\s*/Transparency", data)),
        **ops,
    }

    total = sum(WEIGHTS[k] * v for k, v in counts.items())
    band = TOP
    for limit, name in BANDS:
        if total < limit:
            band = name
            break
    return {**counts, "score": round(total, 1), "band": band}


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__, file=sys.stderr)
        return 2

    cols = ["pages", "fonts", "images", "shadings", "groups", "paths", "textops", "clips", "score", "band"]
    print("\t".join(["path", "ext", *cols]))

    for root in argv[1:]:
        for pdf in sorted(Path(root).rglob("*.pdf")):
            got = score(pdf)
            if got is None:
                continue
            # The stem carries the source document's extension when the caller
            # rendered with per-format names, e.g. `report__docx.pdf`.
            stem = pdf.stem
            ext = stem.rsplit("__", 1)[1] if "__" in stem else ""
            print("\t".join([str(pdf), ext, *(str(got[c]) for c in cols)]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
