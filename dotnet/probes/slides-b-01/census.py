#!/usr/bin/env python3
"""Corpus reach for the slides-b findings, over all 163 slide decks.

Every count is of a construct in a part a *drawn shape can actually resolve
against* — slide, layout or master — never a theme, because the round that
predicted 35-55 and measured 2 counted theme declarations.
"""
import os, re, sys, zipfile, struct, collections
import olefile

ROOT = "/c/sandbox/workdir/sample-files/slides"

# ---------------------------------------------------------------- pptx probes


def pptx_parts(z):
    for n in z.namelist():
        if not n.endswith(".xml"):
            continue
        if n.startswith("ppt/slides/") or n.startswith("ppt/slideLayouts/") \
           or n.startswith("ppt/slideMasters/"):
            yield n


def probe_pptx(path):
    out = collections.Counter()
    slides = set()
    try:
        z = zipfile.ZipFile(path)
    except Exception:
        return None
    for n in pptx_parts(z):
        try:
            d = z.read(n).decode("utf-8", "replace")
        except Exception:
            continue
        is_slide = n.startswith("ppt/slides/")

        # C: a path gradient stated on a slide/layout/master background or shape
        for m in re.finditer(r'<a:gradFill\b.*?</a:gradFill>', d, re.S):
            g = m.group(0)
            pm = re.search(r'<a:path\s+path="([a-z]+)"', g)
            if pm:
                out["gradpath_" + pm.group(1)] += 1
                if is_slide:
                    out["gradpath_slidepart_" + pm.group(1)] += 1

        # C2: background specifically
        for m in re.finditer(r'<p:bg>.*?</p:bg>', d, re.S):
            g = m.group(0)
            pm = re.search(r'<a:path\s+path="([a-z]+)"', g)
            if pm:
                out["bg_gradpath_" + pm.group(1)] += 1
                if is_slide:
                    slides.add(n)

        # B: a:ln with a width on a p:pic
        for m in re.finditer(r'<p:pic>.*?</p:pic>', d, re.S):
            g = m.group(0)
            lm = re.search(r'<a:ln\b[^>]*\bw="(\d+)"', g)
            if lm and int(lm.group(1)) > 0:
                out["pic_ln_w"] += 1
                if int(lm.group(1)) >= 63500:      # >= 5 pt, visible
                    out["pic_ln_w_5pt"] += 1

        # E: a camera roll, which rotates the shape
        for m in re.finditer(r'<a:camera\b.*?</a:camera>|<a:camera\b[^>]*/>', d, re.S):
            g = m.group(0)
            rm = re.search(r'<a:rot\b[^>]*\brev="(-?\d+)"', g)
            if rm and int(rm.group(1)) % 21600000 != 0:
                out["camera_rev"] += 1

        # D: a text run / list level whose colour is a gradFill
        for m in re.finditer(r'<a:(?:rPr|defRPr|endParaRPr)\b.*?</a:(?:rPr|defRPr|endParaRPr)>',
                             d, re.S):
            if "<a:gradFill" in m.group(0):
                out["run_gradfill"] += 1
        # D2: any a:alpha inside a run property
        for m in re.finditer(r'<a:(?:rPr|defRPr)\b.*?</a:(?:rPr|defRPr)>', d, re.S):
            if "<a:alpha " in m.group(0):
                out["run_alpha"] += 1
    return out


# ----------------------------------------------------------------- ppt probes

CROP_IDS = {256, 257, 258, 259}
PIB = 260


def ppt_stream(path):
    ole = olefile.OleFileIO(path)
    for e in ole.listdir():
        if e[-1].lower() == "powerpoint document":
            return ole.openstream(e).read()
    return None


def walk_records(buf, off, end, hit):
    while off + 8 <= end:
        if off + 8 > len(buf):
            return
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        stop = min(body + rl, end)
        if rt == 0xF00B:
            hit(vi >> 4, body, stop)
        elif (vi & 0x0F) == 0x0F:
            walk_records(buf, body, stop, hit)
        off = body + rl
        if rl > len(buf):
            return


def probe_ppt(path):
    out = collections.Counter()
    try:
        buf = ppt_stream(path)
    except Exception:
        return None
    if buf is None:
        return None

    def hit(n, body, stop):
        props = {}
        p = body
        for _ in range(n):
            if p + 6 > stop:
                break
            pid, val = struct.unpack_from("<HI", buf, p)
            p += 6
            props[pid & 0x3FFF] = val
        crops = {k: v for k, v in props.items() if k in CROP_IDS and v}
        if crops:
            out["crop_shapes"] += 1
            if PIB in props:
                out["crop_with_pib"] += 1
            biggest = max(v if v < 2**31 else v - 2**32 for v in crops.values())
            if abs(biggest) / 65536.0 >= 0.02:
                out["crop_2pc"] += 1
            if abs(biggest) / 65536.0 >= 0.10:
                out["crop_10pc"] += 1

    try:
        walk_records(buf, 0, len(buf), hit)
    except Exception:
        pass
    return out


def main():
    rows = []
    for dirpath, _, files in os.walk(ROOT):
        for f in sorted(files):
            p = os.path.join(dirpath, f)
            low = f.lower()
            if low.endswith((".pptx", ".pptm", ".ppsx", ".potx")):
                r = probe_pptx(p)
                kind = "pptx"
            elif low.endswith((".ppt", ".pps", ".pot")):
                r = probe_ppt(p)
                kind = "ppt"
            else:
                r = probe_pptx(p) or probe_ppt(p)
                kind = "?"
            if r is None:
                r = collections.Counter()
                kind += "!"
            rows.append((f, kind, r))

    print("decks:", len(rows),
          "ppt:", sum(1 for _, k, _ in rows if k.startswith("ppt") and k != "pptx"),
          "pptx:", sum(1 for _, k, _ in rows if k == "pptx"))
    keys = ["crop_shapes", "crop_with_pib", "crop_2pc", "crop_10pc",
            "pic_ln_w", "pic_ln_w_5pt", "camera_rev", "run_gradfill", "run_alpha",
            "bg_gradpath_circle", "bg_gradpath_rect", "bg_gradpath_shape",
            "gradpath_circle", "gradpath_rect", "gradpath_shape",
            "gradpath_slidepart_circle"]
    print("%-30s %8s %8s" % ("construct", "decks", "instances"))
    for k in keys:
        decks = sum(1 for _, _, r in rows if r.get(k))
        inst = sum(r.get(k, 0) for _, _, r in rows)
        print("%-30s %8d %8d" % (k, decks, inst))

    print("\n-- decks by construct --")
    for k in keys:
        names = [f for f, _, r in rows if r.get(k)]
        if 0 < len(names) <= 14:
            print("%s: %s" % (k, ", ".join(sorted(names))))


if __name__ == "__main__":
    main()
