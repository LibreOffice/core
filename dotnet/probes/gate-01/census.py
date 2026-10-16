"""Census of the word-count term, with the extractor held constant on both sides.

For every corpus document, both PDFs (ours and the canonical reference) are read by the SAME
pdftotext 26.01.0, so any per-document difference here is a difference between the two
*producers*.  The historical question -- did LibreOffice start emitting bullets, or did poppler
start surfacing them -- is not answerable in this container and is not what the gate consumes;
the gate consumes the difference between two columns read by one tool.

Emits one row per document:
    track path ext  rawO rawR  alnumO alnumR  nonO nonR
plus a control column checking that the raw count reproduces `wc -w` exactly, so the only
change between the old metric and the new one is the filter and not the tokenisation.
"""
import os, subprocess, sys, collections, json

CORPUS = "/c/sandbox/workdir/sample-files"
REF    = "/c/sandbox/workdir/refpdfs-26.2.4.2-fonts"
OURS   = "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/gate"

EXTS = {".doc", ".docx", ".rtf", ".odt", ".ott", ".xls", ".xlsx", ".ods", ".csv",
        ".ppt", ".pptx", ".odp", ".otp"}


def docs(track):
    out = []
    root = os.path.join(CORPUS, track)
    for d in sorted(os.listdir(root)):
        p = os.path.join(root, d)
        if not os.path.isdir(p):
            continue
        for f in sorted(os.listdir(p)):
            fp = os.path.join(p, f)
            if os.path.isdir(fp):
                for g in sorted(os.listdir(fp)):
                    if os.path.splitext(g)[1].lower() in EXTS:
                        out.append(os.path.join(fp, g))
            elif os.path.splitext(f)[1].lower() in EXTS:
                out.append(fp)
    return sorted(out)


def text(pdf):
    r = subprocess.run(["pdftotext", pdf, "-"], capture_output=True, timeout=600)
    return r.stdout.decode("utf-8", "replace")


def wcw(pdf):
    """`pdftotext … | wc -w`, the old metric, run through the shell exactly as the gate does."""
    r = subprocess.run(f'pdftotext "{pdf}" - 2>/dev/null | wc -w', shell=True, capture_output=True)
    return int(r.stdout.split()[0])


def isword(t):
    return any(c.isalnum() for c in t)


def main():
    hist_ref = collections.Counter()
    hist_our = collections.Counter()
    rows = []
    ctrl_bad = []
    for track in ("words", "slides", "sheets"):
        for f in docs(track):
            base = os.path.basename(f)
            stem, ext = base.rsplit(".", 1)
            idf = f"{stem}__{ext.lower()}.pdf"
            o = os.path.join(OURS, track, "ours", idf)
            r = os.path.join(REF, track, idf)
            if not (os.path.exists(o) and os.path.exists(r)):
                print("MISSING", track, base, file=sys.stderr)
                continue
            to, tr = text(o).split(), text(r).split()
            ao = sum(1 for t in to if isword(t))
            ar = sum(1 for t in tr if isword(t))
            for t in tr:
                if not isword(t):
                    hist_ref[t] += 1
            for t in to:
                if not isword(t):
                    hist_our[t] += 1
            if wcw(o) != len(to) or wcw(r) != len(tr):
                ctrl_bad.append(base)
            rows.append((track, os.path.relpath(f, CORPUS), ext.lower(),
                         len(to), len(tr), ao, ar, len(to) - ao, len(tr) - ar))

    with open(os.path.join(OURS, "census.tsv"), "w", encoding="utf-8") as fh:
        fh.write("track\tpath\text\trawO\trawR\talnumO\talnumR\tnonO\tnonR\n")
        for row in rows:
            fh.write("\t".join(str(x) for x in row) + "\n")
    with open(os.path.join(OURS, "hist.json"), "w", encoding="utf-8") as fh:
        json.dump({"ref": hist_ref.most_common(200), "ours": hist_our.most_common(200)},
                  fh, ensure_ascii=False, indent=1)
    print(f"{len(rows)} documents")
    print(f"TOKENISATION CONTROL: raw count != `wc -w` on {len(ctrl_bad)} of {2*len(rows)} PDFs",
          ctrl_bad[:5])


main()
