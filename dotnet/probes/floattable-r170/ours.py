#!/usr/bin/env python3
"""Our own pages and alphanumerics for a list of documents, beside the two reference legs.

Reads `unfloat.py`'s two tables so each row says which of the reference's two answers -- the
positioned table wrapped, or the same table laid in the flow -- this tree reproduces.
"""
import pathlib, re, subprocess, sys

CLI = "/home/user/libreoffice-core/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/linux-x64/XRay.Office.Cli"


def render(document, outdir):
    outdir.mkdir(parents=True, exist_ok=True)
    try:
        subprocess.run([CLI, "render", str(document), "--outdir", str(outdir)],
                       check=True, capture_output=True, timeout=600,
                       env={**__import__("os").environ, "SOURCE_DATE_EPOCH": "0"})
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
        return None
    pdfs = sorted(outdir.glob("*.pdf"))
    return pdfs[0] if pdfs else None


def score(pdf):
    import fitz
    with fitz.open(pdf) as doc:
        text = "".join(page.get_text() for page in doc)
        return doc.page_count, sum(c.isalnum() for c in text)


def reference_table():
    table = {}
    for name in ("unfloat-movers.txt", "unfloat-rest.txt"):
        for line in open(name):
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 6 or parts[0] == "removed":
                continue
            _, fp, wp, fg, wg, doc = parts
            table[doc.removesuffix(" *")] = (int(fp), int(wp), int(fg), int(wg))
    return table


def main():
    reference = reference_table()
    root = pathlib.Path(sys.argv[2])
    print("ours\tfloat\tflow\toursG\tfloatG\tflowG\tagrees\tdocument")
    for line in open(sys.argv[1]):
        path = pathlib.Path(line.strip())
        if not path.name:
            continue
        pdf = render(path, root / re.sub(r"\W+", "-", path.stem)[:60] / "ours")
        if pdf is None:
            print(f"FAILED\t-\t-\t-\t-\t-\t-\t{path.name}")
            continue
        op, og = score(pdf)
        fp, wp, fg, wg = reference[path.name]
        agrees = ("float" if (op, og) == (fp, fg)
                  else "flow" if (op, og) == (wp, wg)
                  else "neither")
        print(f"{op}\t{fp}\t{wp}\t{og}\t{fg}\t{wg}\t{agrees}\t{path.name}", flush=True)


if __name__ == "__main__":
    main()
