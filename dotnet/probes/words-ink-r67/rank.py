#!/usr/bin/env python3
"""Rank the words track by one-sided ink against LibreOffice 26.2.4.2.

Uses `verdict.py`'s own instruments and nothing hand-rolled: the comparability screen is its
`faces()` (a face *swap* voids a comparison, a one-sided extra face is a note), and the ink is
`pdf-image-diff.py`'s per-page column, which is `compare-images.py`'s `ink_delta` at 512 px on the
longest edge -- the same metric `track-ink-sweep.sh` ranks a track on.

    rank.py <ours-dir> <ref-dir> <out.tsv> [workers]
"""
import concurrent.futures as cf, re, subprocess, sys
from pathlib import Path

HERE = Path("/home/user/wt-words67/.claude/skills/render-comparison/scripts")
DIFF = HERE / "pdf-image-diff.py"
SUBSET = re.compile(r"^[A-Z]{6}\+")


def run(cmd, timeout=1200):
    return subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)


def faces(pdf):
    out = set()
    for line in run(["pdffonts", str(pdf)]).stdout.splitlines()[2:]:
        if line.strip():
            out.add(SUBSET.sub("", line.split()[0]))
    return out


def pages(pdf):
    r = run(["pdfinfo", str(pdf)])
    return next((int(l.split()[1]) for l in r.stdout.splitlines() if l.startswith("Pages:")), 0)


def words(pdf):
    r = run(["pdftotext", str(pdf), "-"])
    return sum(1 for w in r.stdout.split() if any(c.isalnum() for c in w))


def one(ours, ref):
    po, pr = pages(ours), pages(ref)
    fo, fr = faces(ours), faces(ref)
    only_ref, only_ours = sorted(fr - fo), sorted(fo - fr)
    note = ""
    if only_ref and only_ours:
        note = "unscoreable-font:" + ",".join(only_ours) + "|" + ",".join(only_ref)
    elif only_ref or only_ours:
        note = "font-note:" + ",".join(only_ours) + "|" + ",".join(only_ref)
    if po != pr:
        return dict(pages_ours=po, pages_ref=pr, abs_ink="-", signed_ink="-", major="-",
                    worst_page="-", worst_ink="-", note=(note + ";" if note else "") + "pages-differ",
                    dwords=words(ours) - words(ref))
    r = run(["python3", str(DIFF), str(ours), str(ref)])
    rows = []
    for line in r.stdout.splitlines():
        f = line.split("\t")
        if len(f) >= 4 and f[0].isdigit():
            try:
                rows.append((int(f[0]), float(f[2]), float(f[3])))
            except ValueError:
                pass
    if not rows:
        return dict(pages_ours=po, pages_ref=pr, abs_ink="-", signed_ink="-", major="-",
                    worst_page="-", worst_ink="-", note=(note + ";" if note else "") + "no-ink-rows",
                    dwords=words(ours) - words(ref))
    absink = sum(a for _, _, a in rows)
    signed = sum(s for _, s, _ in rows)
    major = 0
    m = re.search(r"pages, (\d+) with major differences", r.stdout)
    if m:
        major = int(m.group(1))
    wp, _, wi = max(rows, key=lambda t: t[2])
    return dict(pages_ours=po, pages_ref=pr, abs_ink=f"{absink:.2f}", signed_ink=f"{signed:.2f}",
                major=major, worst_page=wp, worst_ink=f"{wi:.2f}", note=note,
                dwords=words(ours) - words(ref))


def main():
    ours_dir, ref_dir, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    workers = int(sys.argv[4]) if len(sys.argv) > 4 else 4
    ids = sorted(p.stem for p in ours_dir.glob("*.pdf"))
    rows = {}

    def work(i):
        ref = ref_dir / f"{i}.pdf"
        if not ref.exists():
            return i, None
        try:
            return i, one(ours_dir / f"{i}.pdf", ref)
        except Exception as e:                                  # noqa: BLE001
            return i, dict(pages_ours="-", pages_ref="-", abs_ink="-", signed_ink="-", major="-",
                           worst_page="-", worst_ink="-", note=f"error:{type(e).__name__}", dwords="-")

    with cf.ThreadPoolExecutor(max_workers=workers) as pool:
        for i, r in pool.map(work, ids):
            if r is not None:
                rows[i] = r

    cols = ["pages_ours", "pages_ref", "abs_ink", "signed_ink", "major", "worst_page",
            "worst_ink", "dwords", "note"]
    with out.open("w") as fh:
        fh.write("# ours: XRay.Office.Cli from this worktree; ref: /opt/libreoffice26.2 26.2.4.2, "
                 "eight Latin duplicate faces aside; corpus /home/user/sample-files words track\n")
        fh.write("# abs_ink = sum of pdf-image-diff.py's per-page UNSIGNED |ink|%; rank on this\n")
        fh.write("id\t" + "\t".join(cols) + "\n")
        for i in sorted(rows, key=lambda k: -float(rows[k]["abs_ink"])
                        if rows[k]["abs_ink"] not in ("-",) else 1e9):
            fh.write(i + "\t" + "\t".join(str(rows[i][c]) for c in cols) + "\n")
    print("rows", len(rows), "->", out)


if __name__ == "__main__":
    main()
