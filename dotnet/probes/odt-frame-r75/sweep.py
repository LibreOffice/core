#!/usr/bin/env python3
"""Score one track of the converted ODF corpus, reusing a banked reference.

  sweep.py ref   <glob>  <bank>            render the reference half once
  sweep.py ours  <glob>  <bank> <cli> <tag>  render our half with one binary
  sweep.py score <bank> <tag> [<tag2>]     apply the gate's verdict rule

The reference half is banked because the diff under test is confined to dotnet/src,
which cannot reach soffice; the verdict rule is batch-check.sh's, character for
character — max(2%, 15) on alphanumeric characters, plus pages and unembedded fonts.

Every worker directory is keyed on a hex digest of the document's own path, never on a
worker slot: `soffice` truncates -env:UserInstallation at the first space, and a
thread pool does not hand consecutive indices to consecutive slots.
"""
import hashlib, os, re, shutil, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

CORPUS = "/home/user/corpus-odf"
SOFFICE = "/opt/libreoffice26.2/program/soffice"
WORKERS = 2


def docs(glob):
    import glob as g
    out = []
    for p in sorted(g.glob(os.path.join(CORPUS, glob))):
        if os.path.isfile(p):
            out.append(p)
    return out


def ident(path):
    base = os.path.basename(path)
    stem, ext = os.path.splitext(base)
    return f"{stem}__{ext[1:].lower()}"


def digest(path):
    return hashlib.md5(path.encode("utf-8")).hexdigest()[:16]


def counts(pdf):
    if not os.path.exists(pdf):
        return None
    pages = subprocess.run(["pdfinfo", pdf], capture_output=True, text=True).stdout
    m = re.search(r"^Pages:\s+(\d+)", pages, re.M)
    npages = int(m.group(1)) if m else 0
    text = subprocess.run(["pdftotext", pdf, "-"], capture_output=True).stdout.decode("utf-8", "replace")
    glyphs = sum(1 for c in text if c.isalnum())
    fonts = subprocess.run(["pdffonts", pdf], capture_output=True, text=True).stdout.splitlines()[2:]
    unemb = 0
    for line in fonts:
        f = line.split()
        if len(f) >= 8 and f[-5] == "no":
            unemb += 1
    return npages, glyphs, len(fonts), unemb


def render_ref(path, bank):
    out = os.path.join(bank, "ref")
    os.makedirs(out, exist_ok=True)
    dest = os.path.join(out, ident(path) + ".pdf")
    if os.path.exists(dest):
        return
    d = digest(path)
    tmp = os.path.join(bank, "t-" + d)
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp, exist_ok=True)
    subprocess.run(
        [SOFFICE, f"-env:UserInstallation=file:///tmp/xray-office-lo-{d}",
         "--headless", "--norestore", "--convert-to", "pdf", "--outdir", tmp, path],
        capture_output=True, timeout=300)
    stem = os.path.splitext(os.path.basename(path))[0]
    made = os.path.join(tmp, stem + ".pdf")
    if os.path.exists(made):
        shutil.move(made, dest)
    shutil.rmtree(tmp, ignore_errors=True)
    shutil.rmtree(f"/tmp/xray-office-lo-{d}", ignore_errors=True)


def render_ours(path, bank, cli, tag):
    out = os.path.join(bank, tag)
    os.makedirs(out, exist_ok=True)
    dest = os.path.join(out, ident(path) + ".pdf")
    if os.path.exists(dest):
        return
    d = digest(path)
    tmp = os.path.join(bank, f"o-{tag}-{d}")
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp, exist_ok=True)
    subprocess.run([cli, "render", path, "--format", "pdf", "--outdir", tmp],
                   capture_output=True, timeout=300)
    stem = os.path.splitext(os.path.basename(path))[0]
    made = os.path.join(tmp, stem + ".pdf")
    if os.path.exists(made):
        shutil.move(made, dest)
    shutil.rmtree(tmp, ignore_errors=True)


def verdict(ours, ref):
    """batch-check.sh's rule, unchanged."""
    if ours is None and ref is None:
        return "both-failed"
    if ref is None:
        return "ref-failed"
    if ours is None:
        return "ours-failed"
    op, og, _of, un = ours
    rp, rg, _rf, _ru = ref
    v = []
    if op != rp:
        v.append("pages")
    if rg > 0:
        if abs(og - rg) > rg * 0.02 and abs(og - rg) > 15:
            v.append("words")
    elif og > 15:
        v.append("words")
    if un:
        v.append("unembedded")
    return ",".join(v) if v else "match"


def main():
    mode = sys.argv[1]
    if mode == "ref":
        glob, bank = sys.argv[2], sys.argv[3]
        files = docs(glob)
        with ThreadPoolExecutor(WORKERS) as ex:
            list(ex.map(lambda p: render_ref(p, bank), files))
        print(f"ref: {len(os.listdir(os.path.join(bank, 'ref')))} of {len(files)}")
    elif mode == "ours":
        glob, bank, cli, tag = sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]
        files = docs(glob)
        with ThreadPoolExecutor(WORKERS) as ex:
            list(ex.map(lambda p: render_ours(p, bank, cli, tag), files))
        print(f"{tag}: {len(os.listdir(os.path.join(bank, tag)))} of {len(files)}")
    elif mode == "score":
        bank, tag = sys.argv[2], sys.argv[3]
        glob = sys.argv[4]
        files = docs(glob)
        rows = []
        for p in files:
            i = ident(p)
            r = counts(os.path.join(bank, "ref", i + ".pdf"))
            o = counts(os.path.join(bank, tag, i + ".pdf"))
            rows.append((os.path.relpath(p, CORPUS), i, o, r, verdict(o, r)))
        with open(os.path.join(bank, f"rows-{tag}.tsv"), "w") as f:
            for rel, i, o, r, v in rows:
                f.write("\t".join([
                    rel,
                    f"{o[0] if o else '-'}/{r[0] if r else '-'}",
                    f"{o[1] if o else '-'}/{r[1] if r else '-'}",
                    f"{o[3] if o else '-'}",
                    v]) + "\n")
        import collections
        c = collections.Counter(v for *_x, v in rows)
        print(tag, len(rows), dict(c))
    else:
        raise SystemExit("mode?")


main()
