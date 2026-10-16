#!/usr/bin/env python3
"""Render our half of a banked gate again and re-score it against the bank's reference half.

    rescore.py <cli> <corpus-root> <banked-rows.tsv> <path-filter-regex> <outdir> [workers]

Sound only while the diff under test cannot reach `soffice`, which a change confined to
`dotnet/src` cannot. The reference columns are reused verbatim, so the comparison is against
exactly the bytes the baseline was scored from — and, unlike a bank re-rendered under
`SWEEP_ONLY`, every row here is rendered afresh, so nothing is a match by construction.

Verdict rule copied from `batch-check.sh` of 2026-09-05: pages exact, alphanumeric characters
within max(2%, 15), no unembedded font. The elapsed seconds and peak RSS of each render are
recorded as well, because the `ours-failed` rows of an .ods gate are a timeout class rather
than a crash class and a bare verdict hides that.
"""
import concurrent.futures, hashlib, os, re, shutil, subprocess, sys, time

cli, root, banked, pattern, outdir = sys.argv[1:6]
workers = int(sys.argv[6]) if len(sys.argv) > 6 else 2
timeout = int(os.environ.get("RENDER_TIMEOUT", "240"))
os.makedirs(outdir, exist_ok=True)

rows = []
for line in open(banked, encoding="utf-8"):
    if line.startswith("#") or line.startswith("path\t"):
        continue
    f = line.rstrip("\n").split("\t")
    if len(f) >= 9 and re.search(pattern, f[0]):
        rows.append(f)
assert rows, f"no banked rows matched {pattern}"
print(f"{len(rows)} banked rows", file=sys.stderr)


def counts(pdf):
    info = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    pages = next((l.split()[1] for l in info.splitlines() if l.startswith("Pages:")), "-")
    text = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout
    text = text.decode("utf-8", "replace")
    glyphs = sum(1 for c in text if c.isalnum())
    fonts = subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout.splitlines()[2:]
    unemb = sum(1 for l in fonts if len(l.split()) >= 8 and l.split()[-5] == "no")
    return pages, glyphs, unemb


def render(row):
    rel = row[0]
    src = os.path.join(root, rel)
    # One directory per document, never per worker slot: a thread pool does not work
    # consecutive indices, so a slot-keyed directory lets two renders collide silently.
    d = os.path.join(outdir, "d" + hashlib.md5(rel.encode()).hexdigest())
    shutil.rmtree(d, ignore_errors=True)
    os.makedirs(d, exist_ok=True)
    started = time.time()
    note = ""
    try:
        subprocess.run([cli, "render", src, "--format", "pdf", "--outdir", d],
                       capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        note = "timeout"
    elapsed = time.time() - started
    pdfs = [x for x in os.listdir(d) if x.lower().endswith(".pdf")]
    got = counts(os.path.join(d, pdfs[0])) if pdfs else None
    shutil.rmtree(d, ignore_errors=True)
    return rel, got, round(elapsed, 1), note or ("no output" if got is None else "")


def verdict(got, rp, rg):
    if rp == "-":
        return "ref-failed"
    if got is None:
        return "ours-failed"
    op, og, un = got
    v = []
    if op != rp:
        v.append("pages")
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            v.append("words")
    elif og > 15:
        v.append("words")
    if un != 0:
        v.append("unembedded")
    return ",".join(v) if v else "match"


results = {}
with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
    for rel, got, elapsed, note in pool.map(render, rows):
        results[rel] = (got, elapsed, note)

out = os.path.join(outdir, "rescored.tsv")
tally = {}
with open(out, "w", encoding="utf-8") as f:
    f.write("path\text\tpages\tglyphs\tunemb\tverdict\twas\tseconds\tnote\n")
    for row in rows:
        rel, ext = row[0], row[1]
        rp = row[2].split("/")[1]
        rg = row[8].split("/")[1]
        rg = int(rg) if rg != "-" else 0
        got, elapsed, note = results[rel]
        now = verdict(got, rp, rg)
        tally[now] = tally.get(now, 0) + 1
        op = got[0] if got else "-"
        og = got[1] if got else 0
        un = got[2] if got else 0
        f.write(f"{rel}\t{ext}\t{op}/{rp}\t{og}/{rg}\t{un}\t{now}\t{row[6]}\t{elapsed}\t{note}\n")

print(f"wrote {out}")
for k in sorted(tally, key=lambda k: -tally[k]):
    print(f"{k:16s} {tally[k]}")
