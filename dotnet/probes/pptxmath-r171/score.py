#!/usr/bin/env python3
"""Pages, alphanumerics and span positions for the three OMML-bearing decks.

`OfficeMathBox` measures an OMML formula's height for the word-processing layout; `PptxTextBody`
does not call it, so a deck's formula is owed from that round.  A slide's page count is its slide
count, so the only columns that can show it are the characters drawn and where they sit.
"""
import os, pathlib, re, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"
SOFFICE = "/opt/libreoffice26.2/program/soffice"


def render_ours(document, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    subprocess.run([CLI, "render", str(document), "--outdir", str(outdir)], check=True,
                   capture_output=True, timeout=900,
                   env={**os.environ, "SOURCE_DATE_EPOCH": "0"})
    return next(outdir.glob("*.pdf"))


def render_reference(document, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    subprocess.run([SOFFICE, f"-env:UserInstallation=file://{outdir / 'profile'}", "--headless",
                    "--norestore", "--convert-to", "pdf", "--outdir", str(outdir), str(document)],
                   check=True, capture_output=True, timeout=900)
    return next(outdir.glob("*.pdf"))


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


def main():
    root = pathlib.Path(sys.argv[2])
    print("pages\trefPages\tglyphs\trefGlyphs\tspans\trefSpans\tpaired\tmean|dx|\tmean|dy|\tdeck")
    for line in open(sys.argv[1]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        work = root / re.sub(r"\W+", "-", path.stem)[:60]
        ours = render_ours(path, work / "ours")
        reference = render_reference(path, work / "ref")
        import fitz, collections
        def counts(pdf):
            with fitz.open(pdf) as d:
                t = "".join(p.get_text() for p in d)
                return d.page_count, sum(c.isalnum() for c in t)
        op, og = counts(ours)
        rp, rg = counts(reference)
        a, b = spans(ours), spans(reference)
        index = collections.defaultdict(collections.deque)
        for page, text, x, y in b:
            index[(page, text)].append((x, y))
        pairs = []
        for page, text, x, y in a:
            q = index.get((page, text))
            if q:
                rx, ry = q.popleft()
                pairs.append((x - rx, y - ry))
        dx = sum(abs(p) for p, _ in pairs) / len(pairs) if pairs else float("nan")
        dy = sum(abs(q) for _, q in pairs) / len(pairs) if pairs else float("nan")
        print(f"{op}\t{rp}\t{og}\t{rg}\t{len(a)}\t{len(b)}\t{len(pairs)}\t{dx:.2f}\t{dy:.2f}\t"
              f"{path.name}", flush=True)


if __name__ == "__main__":
    main()
