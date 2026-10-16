#!/usr/bin/env python3
"""Score the sweep: page and alphanumeric counts against 26.2.4.2, at base and at head."""
import hashlib, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))


def key(path):
    return hashlib.md5(path.encode()).hexdigest()[:12]


def only_pdf(directory):
    if not os.path.isdir(directory):
        return None
    for name in sorted(os.listdir(directory)):
        if name.endswith(".pdf"):
            return os.path.join(directory, name)
    return None


def counts(pdf):
    if pdf is None:
        return None
    out = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    m = re.search(r"Pages:\s+(\d+)", out)
    if not m:
        return None
    text = subprocess.run(["pdftotext", pdf, "-"], capture_output=True, text=True).stdout
    return int(m.group(1)), sum(c.isalnum() for c in text)


def verdict(ours, ref):
    """`batch-check.sh`'s own rule: equal pages, and alphanumerics within max(2 %, 15)."""
    if ours is None or ref is None:
        return "failed"
    if ours[0] != ref[0]:
        return "pages"
    d = abs(ours[1] - ref[1])
    return "match" if not (d > ref[1] * 0.02 and d > 15) else "glyphs"


def main():
    rows = []
    for path in open(os.path.join(HERE, "movers.txt")).read().split("\n"):
        if not path:
            continue
        k = key(path)
        r = counts(only_pdf(f"{HERE}/sweep/ref/{k}"))
        b = counts(only_pdf(f"{HERE}/sweep/base/{k}"))
        h = counts(only_pdf(f"{HERE}/sweep/head/{k}"))
        rows.append((os.path.basename(path), r, b, h))

    moved = [x for x in rows if x[2] != x[3]]
    print(f"{len(rows)} documents, {len(moved)} whose counts moved\n")
    print(f"{'document':<52} {'ref':>12} {'base':>12} {'head':>12}  {'base':>7} {'head':>7}")
    gained, lost = [], []
    for name, r, b, h in rows:
        vb, vh = verdict(b, r), verdict(h, r)
        if vb != vh:
            (gained if vh == "match" else lost).append((name, vb, vh))
        flag = "" if vb == vh else "  <--"
        if b != h or vb != vh:
            fmt = lambda t: f"{t[0]:>4}p{t[1]:>7}" if t else "   failed   "
            print(f"{name[:51]:<52} {fmt(r)} {fmt(b)} {fmt(h)}  {vb:>7} {vh:>7}{flag}")
    tally = lambda rs, i: {v: sum(1 for x in rs if verdict(x[i], x[1]) == v)
                           for v in ("match", "pages", "glyphs", "failed")}
    print("\nbase", tally(rows, 2))
    print("head", tally(rows, 3))
    for label, items in (("gained", gained), ("lost", lost)):
        for name, vb, vh in items:
            print(f"  {label}: {name}  {vb} -> {vh}")


if __name__ == "__main__":
    main()
