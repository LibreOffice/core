#!/usr/bin/env python3
"""Group the non-matching rows of the ODF parity TSV by *measured* cause.

    classify2.py <parity.tsv> <out.tsv>

Every screen below is a measurement on the two banked PDFs or on the source, not a lookup:

  ref-failed        the reference produced no PDF
  volatile          the reference's text carries a date in the current month that ours does
                    not, and the document holds TODAY()/NOW() — the reference re-evaluates on
                    open, we print the value cached in the file
  raster-ceiling    some page of the reference carries an image and under a fifth of the
                    alphanumeric characters we draw on the same page: LibreOffice rasterised
                    an embedded object, so ours is the better output and the gate scores it as
                    failure
  then by the shape of what is left:
  pages-short / pages-long / ink-missing / ink-extra
"""
import concurrent.futures, datetime, os, re, subprocess, sys, zipfile

CORPUS = "/home/user/corpus-odf"
VOLATILE = re.compile(r'\b(TODAY|NOW)\s*\(', re.I)
TODAY = datetime.date.today()
CURRENT = {
    f"{TODAY.month}/{TODAY.day}/{TODAY.year}", f"{TODAY.day}/{TODAY.month}/{TODAY.year}",
    TODAY.strftime("%Y-%m-%d"), TODAY.strftime("%d/%m/%Y"), TODAY.strftime("%m/%d/%Y"),
    TODAY.strftime("%d.%m.%Y"), TODAY.strftime("%d-%m-%Y"), TODAY.strftime("%m-%d-%Y"),
    TODAY.strftime("%d %b %Y"), TODAY.strftime("%d %B %Y"), TODAY.strftime("%b %d, %Y"),
    TODAY.strftime("%B %d, %Y"), TODAY.strftime("%d/%m/%y"), TODAY.strftime("%m/%d/%y"),
}


def text(pdf, first=None, last=None):
    cmd = ["pdftotext"]
    if first:
        cmd += ["-f", str(first), "-l", str(last)]
    cmd += [pdf, "-"]
    return subprocess.run(cmd, capture_output=True).stdout.decode("utf-8", "replace")


def alnum(s):
    return sum(1 for c in s if c.isalnum())


def has_volatile_source(path):
    try:
        with zipfile.ZipFile(path) as z:
            if "content.xml" not in z.namelist():
                return False
            with z.open("content.xml") as f:
                tail = b""
                while True:
                    chunk = f.read(1 << 20)
                    if not chunk:
                        return False
                    if VOLATILE.search((tail + chunk).decode("utf-8", "replace")):
                        return True
                    tail = chunk[-64:]
    except Exception:
        return False


def images_by_page(pdf):
    """Page number -> number of image XObjects drawn on it."""
    out = subprocess.run(["pdfimages", "-list", pdf], capture_output=True).stdout
    counts = {}
    for line in out.decode("utf-8", "replace").splitlines()[2:]:
        p = line.split()
        if len(p) > 2 and p[0].isdigit():
            counts[int(p[0])] = counts.get(int(p[0]), 0) + 1
    return counts


def ceiling(ours_pdf, ref_pdf, pages):
    """True when some page has an image in the reference and far less text than ours."""
    imgs = images_by_page(ref_pdf)
    if not imgs:
        return False
    for page in sorted(imgs):
        if page > pages:
            continue
        o = alnum(text(ours_pdf, page, page))
        r = alnum(text(ref_pdf, page, page))
        if o >= 40 and r < o * 0.2:
            return True
    return False


def volatile(ours_pdf, ref_pdf, src):
    if not has_volatile_source(src):
        return False
    ot, rt = text(ours_pdf), text(ref_pdf)
    return any(d in rt and d not in ot for d in CURRENT)


def main():
    parity, out = sys.argv[1], sys.argv[2]
    rows = []
    with open(parity, encoding="utf-8") as f:
        for line in f:
            if line.startswith("#") or line.startswith("path\t"):
                continue
            rows.append(line.rstrip("\n").split("\t"))

    def classify(r):
        path, ext, pages, words, fonts, unemb, verdict, raw, glyphs = r[:9]
        if verdict == "match":
            return None
        op, rp = pages.split("/")
        og, rg = glyphs.split("/")
        stem = os.path.basename(path).rsplit(".", 1)[0]
        key = f"{stem}__{ext}"
        ours_pdf = "/home/user/odsrow-work/bank/" + os.environ.get("OURS_TAG", "fixC") + "/" + key + ".pdf"
        ref_pdf = f"/home/user/odsgap-work/bank/ref/{key}.pdf"
        src = os.path.join(CORPUS, path)

        if verdict == "ref-failed":
            group = "ref-failed"
        elif verdict == "ours-failed":
            group = "ours-failed"
        elif volatile(ours_pdf, ref_pdf, src):
            group = "volatile"
        elif int(og) > int(rg) and ceiling(ours_pdf, ref_pdf, min(int(op), int(rp))):
            group = "raster-ceiling"
        elif op != rp:
            group = "pages-short" if int(op) < int(rp) else "pages-long"
        else:
            group = "ink-missing" if int(og) < int(rg) else "ink-extra"
        share = "-" if rg in ("-", "0") else f"{(int(og) - int(rg)) / int(rg) * 100:.1f}"
        return [path, ext, verdict, group, pages, glyphs, share]

    written = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        for got in pool.map(classify, rows):
            if got:
                written.append(got)

    written.sort(key=lambda w: (w[1], w[3], w[0]))
    with open(out, "w", encoding="utf-8") as f:
        f.write("# grouped from %s, %s\n" % (os.path.abspath(parity), TODAY.isoformat()))
        f.write("# ours   = XRay.Office.Cli at agent/odsgap with the print-range split fixed\n")
        f.write("# ref    = /opt/libreoffice26.2/program/soffice, LibreOffice 26.2.4.2\n")
        f.write("# fonts  = system fontconfig; all four tarball confounds aside, "
                "LiberationSansNarrow included (2026-09-07)\n")
        f.write("path\text\tverdict\tgroup\tpages\tglyphs\tglyph_delta_pct\n")
        for w in written:
            f.write("\t".join(w) + "\n")

    from collections import Counter
    for ext in ("ods", "odt"):
        c = Counter(w[3] for w in written if w[1] == ext)
        print(ext, sum(c.values()), dict(sorted(c.items(), key=lambda kv: -kv[1])))


main()
