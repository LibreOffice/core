#!/usr/bin/env python3
"""Rewrite one attribute of a real .ods and render the result, which is the only way to read
a band's *own* text height off a document that was not authored for the purpose.

    variants.py <src.ods> <outdir> <sheet-master-layout-name> <attr> <values...>

Each variant differs from the original in exactly one attribute of one `style:page-layout`'s
`style:header-footer-properties`, so the difference between two first-row positions is the
difference between two bands and nothing else.
"""
import os, re, shutil, subprocess, sys, zipfile, hashlib

SOFFICE = "/opt/libreoffice26.2/program/soffice"


def rewrite(src, dst, layout, attr, value):
    shutil.copy(src, dst)
    with zipfile.ZipFile(src) as z:
        styles = z.read("styles.xml").decode("utf-8")
        names = z.namelist()
        blobs = {n: z.read(n) for n in names}
    m = re.search(r'<style:page-layout style:name="%s".*?</style:page-layout>' % layout,
                  styles, re.S)
    assert m, layout
    block = m.group(0)
    hs = re.search(r'<style:header-style>.*?</style:header-style>', block, re.S)
    assert hs, "no header-style"
    new = hs.group(0)
    if re.search(r'\b%s="[^"]*"' % re.escape(attr), new):
        new = re.sub(r'\b%s="[^"]*"' % re.escape(attr), f'{attr}="{value}"', new)
    else:
        new = new.replace("<style:header-footer-properties",
                          f'<style:header-footer-properties {attr}="{value}"', 1)
    styles = styles.replace(block, block.replace(hs.group(0), new))
    blobs["styles.xml"] = styles.encode("utf-8")
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as z:
        for n in names:
            z.writestr(n, blobs[n])


def render(src, out):
    h = hashlib.md5(src.encode()).hexdigest()
    d = os.path.join(out, h)
    os.makedirs(d, exist_ok=True)
    prof = "/tmp/lo-prof-" + h
    os.makedirs(prof, exist_ok=True)
    subprocess.run([SOFFICE, f"-env:UserInstallation=file://{prof}", "--headless",
                    "--convert-to", "pdf", "--outdir", d, src], capture_output=True,
                   timeout=900)
    pdf = os.path.join(d, os.path.basename(src).rsplit(".", 1)[0] + ".pdf")
    return pdf if os.path.exists(pdf) else None


def top(pdf, page=0):
    import pymupdf
    doc = pymupdf.open(pdf)
    p = doc[page]
    ys = []
    hdr = None
    for b in p.get_text("dict")["blocks"]:
        for line in b.get("lines", []):
            for s in line["spans"]:
                ys.append((s["bbox"][1], s["bbox"][3], s["text"].strip()))
    doc.close()
    ys.sort()
    return ys


def main():
    src, out, layout, attr = sys.argv[1:5]
    values = sys.argv[5:]
    os.makedirs(out, exist_ok=True)
    for v in values:
        dst = os.path.join(out, f"v-{attr.replace(':', '_')}-{v}.ods")
        rewrite(src, dst, layout, attr, v)
        pdf = render(dst, out)
        if not pdf:
            print(f"{v}\tRENDER FAILED")
            continue
        ys = top(pdf)
        head = ys[0] if ys else (0, 0, "")
        body = next((y for y in ys if y[0] > head[1] + 1), (0, 0, ""))
        print(f"{v}\thdr={head[0]:.3f}..{head[1]:.3f} {head[2]!r}"
              f"\tbody={body[0]:.3f} {body[2]!r}")


if __name__ == "__main__":
    main()
