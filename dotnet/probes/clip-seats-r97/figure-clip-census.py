#!/usr/bin/env python3
"""Census: pages where a *shape* (a /Figure marked-content element) carries a clip
smaller than 90% of the page and still shows text lying wholly outside that clip.

The 90% filter is the one clip-textlayer-r93 found it had omitted: LibreOffice emits a
full-page clip first, so a census that does not exclude it is measuring nothing.  The
area distribution of what survives the filter is printed with the result.

Usage: figure-clip-census.py <pdf|dir> [...]
"""
import sys, os, re, math, statistics
import pymupdf

NUMBER = re.compile(rb"[-+]?(?:\d+\.?\d*|\.\d+)")

def tokenize(data):
    """Yield (kind, value) with kind in {'num','name','str','dict','array','op'}."""
    i, n = 0, len(data)
    while i < n:
        c = data[i:i+1]
        if c in b" \t\r\n\f\x00":
            i += 1; continue
        if c == b"%":
            j = data.find(b"\n", i)
            i = n if j < 0 else j + 1
            continue
        if c == b"/":
            j = i + 1
            while j < n and data[j:j+1] not in b" \t\r\n\f\x00/[]<>(){}":
                j += 1
            yield ("name", data[i+1:j].decode("latin-1")); i = j; continue
        if c == b"(":
            depth, j, buf = 1, i+1, bytearray()
            while j < n and depth:
                ch = data[j:j+1]
                if ch == b"\\":
                    buf += data[j:j+2]; j += 2; continue
                if ch == b"(": depth += 1
                elif ch == b")":
                    depth -= 1
                    if depth == 0: j += 1; break
                buf += ch; j += 1
            yield ("str", bytes(buf)); i = j; continue
        if data[i:i+2] == b"<<":
            depth, j = 1, i+2
            while j < n and depth:
                if data[j:j+2] == b"<<": depth += 1; j += 2; continue
                if data[j:j+2] == b">>": depth -= 1; j += 2; continue
                j += 1
            yield ("dict", data[i:j]); i = j; continue
        if c == b"<":
            j = data.find(b">", i)
            j = n if j < 0 else j + 1
            yield ("str", data[i:j]); i = j; continue
        if c in b"[]":
            yield ("op", c.decode()); i += 1; continue
        if c in b"{}":
            i += 1; continue
        m = NUMBER.match(data, i)
        if m and (c in b"+-.0123456789"):
            yield ("num", float(m.group())); i = m.end(); continue
        j = i
        while j < n and data[j:j+1] not in b" \t\r\n\f\x00/[]<>(){}%":
            j += 1
        if j == i: j = i + 1
        yield ("op", data[i:j].decode("latin-1")); i = j

def mat_mul(a, b):
    return (a[0]*b[0]+a[1]*b[2], a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2], a[2]*b[1]+a[3]*b[3],
            a[4]*b[0]+a[5]*b[2]+b[4], a[4]*b[1]+a[5]*b[3]+b[5])

def apply(m, x, y):
    return (m[0]*x + m[2]*y + m[4], m[1]*x + m[3]*y + m[5])

def box_of(m, pts):
    xs, ys = [], []
    for x, y in pts:
        u, v = apply(m, x, y); xs.append(u); ys.append(v)
    return (min(xs), min(ys), max(xs), max(ys))

def intersect(a, b):
    if a is None: return b
    if b is None: return a
    r = (max(a[0], b[0]), max(a[1], b[1]), min(a[2], b[2]), min(a[3], b[3]))
    return r

def area(r):
    if r is None: return None
    return max(0.0, r[2]-r[0]) * max(0.0, r[3]-r[1])

def disjoint(a, b):
    return a[2] <= b[0] or b[2] <= a[0] or a[3] <= b[1] or b[3] <= a[1]

def run_page(page):
    """Return list of (tag, textbox, clipbox) and list of (tag, clipbox)."""
    data = page.read_contents()
    ctm = (1, 0, 0, 1, 0, 0)
    clip = None
    stack = []
    mc = []                      # marked-content tag stack
    operands = []
    path_pts = []
    pending_clip = None
    tm = tlm = (1, 0, 0, 1, 0, 0)
    fsize = 0.0
    leading = 0.0
    texts = []
    clips = []
    cur = (0.0, 0.0)             # current point for path building

    for kind, val in tokenize(data):
        if kind != "op" or val in ("[", "]"):
            if kind == "op":
                operands.append(val)
            else:
                operands.append(val)
            continue
        op = val
        try:
            if op == "q":
                stack.append((ctm, clip))
            elif op == "Q":
                # The marked-content stack is NOT part of the graphics state: BDC/EMC
                # nesting crosses q/Q freely in LibreOffice's output, and saving it here
                # was silently mislabelling every clip a /P block opens after a Q.
                if stack: ctm, clip = stack.pop()
            elif op == "cm":
                nums = [o for o in operands if isinstance(o, float)][-6:]
                if len(nums) == 6: ctm = mat_mul(tuple(nums), ctm)
            elif op == "re":
                nums = [o for o in operands if isinstance(o, float)][-4:]
                if len(nums) == 4:
                    x, y, w, h = nums
                    path_pts += [(x, y), (x+w, y), (x+w, y+h), (x, y+h)]
                    cur = (x, y)
            elif op in ("m", "l"):
                nums = [o for o in operands if isinstance(o, float)][-2:]
                if len(nums) == 2:
                    path_pts.append(tuple(nums)); cur = tuple(nums)
            elif op in ("c",):
                nums = [o for o in operands if isinstance(o, float)][-6:]
                if len(nums) == 6:
                    path_pts += [(nums[0], nums[1]), (nums[2], nums[3]), (nums[4], nums[5])]
                    cur = (nums[4], nums[5])
            elif op in ("v", "y"):
                nums = [o for o in operands if isinstance(o, float)][-4:]
                if len(nums) == 4:
                    path_pts += [(nums[0], nums[1]), (nums[2], nums[3])]
                    cur = (nums[2], nums[3])
            elif op in ("W", "W*"):
                pending_clip = True
            elif op in ("n", "f", "F", "f*", "B", "B*", "b", "b*", "S", "s"):
                if pending_clip and path_pts:
                    box = box_of(ctm, path_pts)
                    tag = None
                    for t in reversed(mc):
                        if t: tag = t; break
                    clip = (clip or []) + [(box, tag)]
                    clips.append((tag, box))
                pending_clip = None
                path_pts = []
            elif op == "h":
                pass
            elif op in ("BDC", "BMC"):
                tag = None
                for o in operands:
                    if isinstance(o, str) and not isinstance(o, bytes):
                        tag = o; break
                mc.append(tag)
            elif op == "EMC":
                if mc: mc.pop()
            elif op == "BT":
                tm = tlm = (1, 0, 0, 1, 0, 0)
            elif op == "Tf":
                nums = [o for o in operands if isinstance(o, float)]
                if nums: fsize = nums[-1]
            elif op == "TL":
                nums = [o for o in operands if isinstance(o, float)]
                if nums: leading = nums[-1]
            elif op == "Tm":
                nums = [o for o in operands if isinstance(o, float)][-6:]
                if len(nums) == 6: tm = tlm = tuple(nums)
            elif op in ("Td", "TD"):
                nums = [o for o in operands if isinstance(o, float)][-2:]
                if len(nums) == 2:
                    if op == "TD": leading = -nums[1]
                    tlm = mat_mul((1, 0, 0, 1, nums[0], nums[1]), tlm)
                    tm = tlm
            elif op == "T*":
                tlm = mat_mul((1, 0, 0, 1, 0, -leading), tlm); tm = tlm
            elif op in ("Tj", "TJ", "'", '"'):
                if op in ("'", '"'):
                    tlm = mat_mul((1, 0, 0, 1, 0, -leading), tlm); tm = tlm
                nglyph = 0
                for o in operands:
                    if isinstance(o, bytes):
                        s = o
                        if s.startswith(b"<"):
                            nglyph += max(1, (len(s) - 2) // 4)
                        else:
                            nglyph += len(s)
                w = nglyph * fsize * 0.55
                full = mat_mul(tm, ctm)
                box = box_of(full, [(0, -fsize*0.30), (w, -fsize*0.30),
                                    (w, fsize*1.0), (0, fsize*1.0)])
                tag = None
                for t in reversed(mc):
                    if t: tag = t; break
                texts.append((tag, box, list(clip) if clip else [], nglyph))
                tm = mat_mul((1, 0, 0, 1, w, 0), tm)
        except Exception:
            pass
        operands = []
    return texts, clips

# 26.2.4.2 does not tag every drawing object the same way: a chart's clip is opened
# inside a /Figure element and a plain shape's inside a /Div one (measured on
# `make-shape-text-probe.py`'s output and on `044_Cash_flow_forecast` page 4).  Both are
# the drawing layer; /P is the cell text the clip never governed.
DRAW_TAGS = ("Figure", "Div")

def figure_clip(clips, tags=DRAW_TAGS):
    """The intersection of the clip rectangles a drawing-layer element established."""
    boxes = [b for b, t in clips if t in tags]
    if not boxes: return None
    r = boxes[0]
    for b in boxes[1:]: r = intersect(r, b)
    return r

def effective_clip(clips):
    if not clips: return None
    r = clips[0][0]
    for b, _ in clips[1:]: r = intersect(r, b)
    return r

def main(paths):
    files = []
    for p in paths:
        if os.path.isdir(p):
            for f in sorted(os.listdir(p)):
                if f.lower().endswith(".pdf"): files.append(os.path.join(p, f))
        else:
            files.append(p)

    fig_clip_fracs = []
    all_clip_fracs = []
    witness_rows = []
    pages_with_fig_subclip = 0
    pages_scanned = 0
    pages_with_fig_text = 0
    docs_with_fig_subclip = set()
    docs_witness = set()
    tag_counts = {}
    total_out_glyphs = 0

    for path in files:
        try:
            doc = pymupdf.open(path)
        except Exception as e:
            print(f"# open failed {path}: {e}", file=sys.stderr); continue
        name = os.path.basename(path)[:-4]
        for pno in range(doc.page_count):
            page = doc[pno]
            parea = page.rect.width * page.rect.height
            if parea <= 0: continue
            pages_scanned += 1
            try:
                texts, clips = run_page(page)
            except Exception as e:
                print(f"# parse failed {name} p{pno+1}: {e}", file=sys.stderr); continue
            for tag, box in clips:
                f = area(box) / parea
                all_clip_fracs.append(f)
                if f < 0.9: tag_counts[tag] = tag_counts.get(tag, 0) + 1
                if tag in DRAW_TAGS: fig_clip_fracs.append(f)
            subclipped_figure = any(tag in DRAW_TAGS and area(box) < 0.9 * parea
                                    for tag, box in clips)
            if subclipped_figure:
                pages_with_fig_subclip += 1
                docs_with_fig_subclip.add(name)
            # Only text that lands on the paper counts.  A chart straddling a page
            # break puts most of its axis labels at a negative x, and no extractor reads
            # a glyph outside the MediaBox: counting those would inflate the answer with
            # text no reader could ever see.
            page_box = (page.rect.x0, page.rect.y0, page.rect.x1, page.rect.y1)
            out = 0; outglyphs = 0; under = 0
            for tag, tbox, tclip, ng in texts:
                if disjoint(tbox, page_box): continue
                fc = figure_clip(tclip)
                if fc is None: continue
                if area(fc) >= 0.9 * parea: continue
                under += 1
                if disjoint(tbox, fc):
                    out += 1; outglyphs += ng
            if under: pages_with_fig_text += 1
            if out:
                witness_rows.append((name, pno + 1, out, outglyphs, under))
                docs_witness.add(name)
                total_out_glyphs += outglyphs
        doc.close()

    def dist(v, label):
        if not v:
            print(f"{label}: none"); return
        v = sorted(v)
        print(f"{label}: n={len(v)} min={v[0]:.3f} p10={v[len(v)//10]:.3f} "
              f"median={statistics.median(v):.3f} max={v[-1]:.3f} "
              f"under0.9={sum(1 for x in v if x < 0.9)}")

    print(f"pages scanned: {pages_scanned}  files: {len(files)}")
    dist(all_clip_fracs, "all clip rects, area / page area")
    dist(fig_clip_fracs, "drawing-layer clip rects, area / page area")
    print("sub-page clips by the marked-content element that opened them: "
          + ", ".join(f"{k}={v}" for k, v in sorted(tag_counts.items(), key=lambda kv: -kv[1])))
    print(f"pages with a /Figure clip under 90% of the page: {pages_with_fig_subclip} "
          f"in {len(docs_with_fig_subclip)} documents")
    print(f"pages drawing any text under such a clip: {pages_with_fig_text}")
    print(f"pages where that text lies wholly outside the clip: {len(witness_rows)} "
          f"in {len(docs_witness)} documents, {total_out_glyphs} glyphs")
    for r in sorted(witness_rows, key=lambda r: -r[3]):
        print(f"  {r[0]}\tp{r[1]}\truns_out={r[2]}/{r[4]}\tglyphs_out={r[3]}")

if __name__ == "__main__":
    main(sys.argv[1:])
