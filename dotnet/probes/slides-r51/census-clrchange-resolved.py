#!/usr/bin/env python3
"""Resolve every a:clrChange to the image it applies to, and say whether it changes a pixel.

Declaring a clrChange is not the same as it doing anything.  Three ways it can be inert:
  * clrTo is opaque and equal to clrFrom;
  * the image contains no pixel matching clrFrom (within the tolerance the renderer uses);
  * it sits in a layout/master part that no slide actually instantiates (not checked here --
    recorded as the part, so the reach figure can be read with that caveat attached).
"""
import io, os, re, sys, zipfile, collections
from PIL import Image

CORPUS = "/c/sandbox/workdir/sample-files"
BLIP = re.compile(rb"<a:blip\b[^>]*?r:embed=\"([^\"]+)\"[^>]*?(/>|>(.*?)</a:blip>)", re.S)
CLRCHANGE = re.compile(rb"<a:clrChange\b([^>]*)>(.*?)</a:clrChange>", re.S)
SRGB = re.compile(rb"<a:srgbClr val=\"([0-9A-Fa-f]{6})\"\s*(/>|>(.*?)</a:srgbClr>)", re.S)
ALPHA = re.compile(rb"<a:alpha val=\"(\d+)\"")

def rels_for(z, part):
    d, n = part.rsplit("/", 1)
    rp = f"{d}/_rels/{n}.rels"
    try:
        data = z.read(rp)
    except KeyError:
        return {}
    return {m.group(1).decode(): m.group(2).decode()
            for m in re.finditer(rb'Id="([^"]+)"[^>]*Target="([^"]+)"', data)}

def norm(base, target):
    if target.startswith("/"):
        return target.lstrip("/")
    d = base.rsplit("/", 1)[0]
    parts = (d + "/" + target).split("/")
    out = []
    for p in parts:
        if p == "..":
            out.pop()
        elif p not in (".", ""):
            out.append(p)
    return "/".join(out)

rows = []
seen = set()
for root, dirs, files in os.walk(CORPUS):
    if ".git" in root.split(os.sep): continue
    for name in files:
        if os.path.splitext(name)[1].lower() not in (".pptx",".ppsx",".potx",".docx",".xlsx"): continue
        p = os.path.join(root, name); rel = os.path.relpath(p, CORPUS)
        if rel.casefold() in seen: continue
        seen.add(rel.casefold())
        if not zipfile.is_zipfile(p): continue
        with zipfile.ZipFile(p) as z:
            names = set(z.namelist())
            for part in list(names):
                if not part.endswith(".xml"): continue
                try: data = z.read(part)
                except Exception: continue
                if b"<a:clrChange" not in data: continue
                rels = rels_for(z, part)
                for bm in BLIP.finditer(data):
                    inner = bm.group(3) or b""
                    if b"<a:clrChange" not in inner: continue
                    rid = bm.group(1).decode()
                    tgt = rels.get(rid)
                    media = norm(part, tgt) if tgt else None
                    for cm in CLRCHANGE.finditer(inner):
                        attrs, body = cm.group(1), cm.group(2)
                        usea = b'useA="0"' not in attrs
                        fr = re.search(rb"<a:clrFrom>(.*?)</a:clrFrom>", body, re.S)
                        to = re.search(rb"<a:clrTo>(.*?)</a:clrTo>", body, re.S)
                        def col(seg):
                            if not seg: return (None, 100000)
                            m = SRGB.search(seg.group(1))
                            if not m: return (None, 100000)
                            a = ALPHA.search(m.group(3) or b"")
                            return (m.group(1).decode().upper(), int(a.group(1)) if a else 100000)
                        cf, _ = col(fr); ct, ta = col(to)
                        pct = None; size = None; mode = None
                        if media and media in names:
                            try:
                                im = Image.open(io.BytesIO(z.read(media)))
                                mode = im.mode; size = im.size
                                im2 = im.convert("RGB")
                                if cf:
                                    want = tuple(int(cf[i:i+2],16) for i in (0,2,4))
                                    cnt = sum(n for n,c in im2.getcolors(maxcolors=1<<24) or [] if c==want)
                                    pct = 100.0*cnt/(im2.size[0]*im2.size[1])
                            except Exception as exc:
                                mode = f"ERR:{exc}"
                        rows.append((rel, part.split("/")[1] if "/" in part else part,
                                     media, cf, ct, ta, usea, pct, size, mode))

print(f"clrChange blip instances resolved: {len(rows)}\n")
hdr = f"{'doc':58} {'part':14} {'from':7} {'to':7} {'a%':>5} {'useA':>4} {'%px match':>9} {'mode':>6}"
print(hdr); print("-"*len(hdr))
eff = collections.Counter()
for rel, part, media, cf, ct, ta, usea, pct, size, mode in sorted(rows):
    a = "-" if ta==100000 else str(ta//1000)
    transparent = (ta != 100000 and ta == 0)
    changes = (pct is not None and pct > 0.0) and (transparent or (cf != ct))
    eff[bool(changes)] += 1
    print(f"{os.path.basename(rel)[:57]:58} {part[:14]:14} {cf or '-':7} {ct or '-':7} {a:>5} "
          f"{'y' if usea else 'n':>4} {('%.1f'%pct) if pct is not None else '-':>9} {str(mode)[:6]:>6}"
          f"{'   <-- CHANGES PIXELS' if changes else ''}")
print(f"\ninstances that actually change pixels: {eff[True]} of {len(rows)}")
docs = {r[0] for r in rows}
docs_eff = {r[0] for r in rows if (r[7] is not None and r[7] > 0.0) and ((r[5]==0) or (r[3]!=r[4]))}
print(f"documents declaring: {len(docs)}   documents where at least one changes pixels: {len(docs_eff)}")
for d in sorted(docs_eff): print("   ", d)
