#!/usr/bin/env python3
"""Where our spans sit against the reference's, for a document holding a positioned table.

Pages and alphanumerics cannot see a wrap: a paragraph drawn *under* a table instead of beside
it costs no character and, unless the page overflows, no page.  What it does cost is position,
so this pairs each span with the reference's by page and text and reports the displacement.

The reference leg is the document as authored -- the positioned table wrapped -- so a document
whose prose we draw below the table reads here as a large mean |dy| with the sign one way.
"""
import collections, pathlib, re, subprocess, sys, os

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFFICE = "/opt/libreoffice26.2/program/soffice"


def spans(pdf):
    import fitz
    out = []
    with fitz.open(pdf) as doc:
        for number, page in enumerate(doc):
            for block in page.get_text("dict")["blocks"]:
                for line in block.get("lines", []):
                    for span in line["spans"]:
                        text = span["text"].strip()
                        if text:
                            out.append((number, text, span["bbox"][0], span["bbox"][1]))
    return out


def pair(ours, reference):
    """Pair spans sharing a page and a string, in draw order, one to one."""
    index = collections.defaultdict(collections.deque)
    for page, text, x, y in reference:
        index[(page, text)].append((x, y))
    pairs = []
    for page, text, x, y in ours:
        queue = index.get((page, text))
        if queue:
            rx, ry = queue.popleft()
            pairs.append((x - rx, y - ry))
    return pairs


def main():
    root = pathlib.Path(sys.argv[2])
    print("paired\tofOurs\tmean|dx|\tmean|dy|\tmax|dy|\tover10pt\tdocument")
    for line in open(sys.argv[1]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        work = root / re.sub(r"\W+", "-", path.stem)[:60]
        ourpdf = next((work / "ours").glob("*.pdf"), None)
        if ourpdf is None:
            (work / "ours").mkdir(parents=True, exist_ok=True)
            subprocess.run([CLI, "render", str(path), "--outdir", str(work / "ours")],
                           check=True, capture_output=True,
                           env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
            ourpdf = next((work / "ours").glob("*.pdf"))
        refpdf = next((work / "float").glob("*.pdf"), None)
        if refpdf is None:
            print(f"-\t-\t-\t-\t-\t-\t{path.name} (no reference)")
            continue
        ours, reference = spans(ourpdf), spans(refpdf)
        pairs = pair(ours, reference)
        if not pairs:
            print(f"0\t{len(ours)}\t-\t-\t-\t-\t{path.name}")
            continue
        dx = sum(abs(a) for a, _ in pairs) / len(pairs)
        dy = sum(abs(b) for _, b in pairs) / len(pairs)
        mx = max(abs(b) for _, b in pairs)
        big = sum(1 for _, b in pairs if abs(b) > 10)
        print(f"{len(pairs)}\t{len(ours)}\t{dx:.2f}\t{dy:.2f}\t{mx:.2f}\t{big}\t{path.name}",
              flush=True)


if __name__ == "__main__":
    main()
