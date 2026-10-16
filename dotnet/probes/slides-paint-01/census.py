#!/usr/bin/env python3
"""Census across the 163-deck slides track by walking records.

  .ppt   -> olefile for the CFB, then a recursive walk of the PowerPoint Document
            stream's record tree (recVer==0xF means container).
  .pptx  -> zipfile + ElementTree.

No regex anywhere. A regex census on this track was wrong by a factor of sixteen.
"""
import os, sys, struct, zipfile, json
import olefile
import xml.etree.ElementTree as ET

A = "http://schemas.openxmlformats.org/drawingml/2006/main"
R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
P = "http://schemas.openxmlformats.org/presentationml/2006/main"

# --- Escher / PPT record ids -------------------------------------------------
FOPT       = 0xF00B   # OfficeArtFOPT
FOPT2      = 0xF121   # OfficeArtSecondaryFOPT
FOPT3      = 0xF122   # OfficeArtTertiaryFOPT
STYLETEXT  = 0x0FA1   # StyleTextPropAtom
TXMASTER   = 0x0FA3   # TxMasterStyleAtom

PROP_PICTURE_TRANSPARENT = 263
PROP_PIB                 = 260


def props(buf, inst):
    """The (id, fComplex, value) triples of one FOPT body."""
    out = []
    n = min(inst, len(buf) // 6)
    for i in range(n):
        opid, val = struct.unpack_from("<HI", buf, i * 6)
        out.append((opid & 0x3FFF, bool(opid & 0x8000), val))
    return out


def cf_exception(buf, off):
    """Parse one TextCFException. Returns (newoff, masks, style) or None."""
    if off + 4 > len(buf):
        return None
    masks = struct.unpack_from("<I", buf, off)[0]
    off += 4
    style = 0
    # fontStyle exists iff any of bold/italic/underline/shadow/fehint/kumi/emboss
    # or fHasStyle is set -- i.e. any of the low 16 bits (the rest are unused).
    if masks & 0x0000FFFF:
        if off + 2 > len(buf):
            return None
        style = struct.unpack_from("<H", buf, off)[0]
        off += 2
    for bit, size in ((16, 2), (21, 2), (22, 2), (23, 2), (24, 2), (25, 2),
                      (17, 2), (18, 4), (19, 2)):
        if masks & (1 << bit):
            off += size
    return (off, masks, style) if off <= len(buf) else None


def style_text_shadow(buf):
    """Runs in a StyleTextPropAtom with the shadow bit stated and on.

    The paragraph half has to be skipped first and its layout depends on its own
    mask, so instead of parsing it we scan for the character half from every
    plausible start and take the parse that consumes the atom exactly.  A run
    count is a char count, which we do not know here, so 'consumes exactly' is
    the only anchor available.
    """
    best = None
    for start in range(0, min(len(buf), 512)):
        off, runs, shadow = start, 0, 0
        ok = True
        while off < len(buf):
            if off + 4 > len(buf):
                ok = False
                break
            off += 4                       # count (chars covered by this run)
            r = cf_exception(buf, off)
            if r is None:
                ok = False
                break
            off, masks, style = r
            runs += 1
            if (masks & 0x10) and (style & 0x10):
                shadow += 1
            if runs > 4096:
                ok = False
                break
        if ok and off == len(buf) and runs:
            if best is None or shadow > best[1]:
                best = (runs, shadow)
    return best or (0, 0)


def walk_ppt(path):
    hit = {"prop263": 0, "pib": 0, "cf_shadow_runs": 0, "cf_runs": 0,
           "master_atoms": 0, "err": None}
    try:
        ole = olefile.OleFileIO(path)
    except Exception as e:
        hit["err"] = "ole:" + type(e).__name__
        return hit
    stream = None
    for entry in ole.listdir():
        if entry[-1] == "PowerPoint Document":
            stream = entry
            break
    if stream is None:
        hit["err"] = "no-ppt-stream"
        ole.close()
        return hit
    data = ole.openstream(stream).read()
    ole.close()

    def rec(lo, hi, depth):
        off = lo
        while off + 8 <= hi:
            v_i, typ, ln = struct.unpack_from("<HHI", data, off)
            ver = v_i & 0x0F
            inst = v_i >> 4
            body = off + 8
            end = body + ln
            if ln < 0 or end > hi:
                return
            if ver == 0x0F and depth < 40:
                rec(body, end, depth + 1)
            elif typ in (FOPT, FOPT2, FOPT3):
                for pid, _cx, val in props(data[body:end], inst):
                    if pid == PROP_PICTURE_TRANSPARENT:
                        hit["prop263"] += 1
                    elif pid == PROP_PIB and val:
                        hit["pib"] += 1
            elif typ == STYLETEXT:
                runs, shadow = style_text_shadow(data[body:end])
                hit["cf_runs"] += runs
                hit["cf_shadow_runs"] += shadow
            elif typ == TXMASTER:
                hit["master_atoms"] += 1
            off = end

    rec(0, len(data), 0)
    return hit


def walk_pptx(path):
    hit = {"clrChange": 0, "blip": 0, "u_runs": 0, "u_defrpr": 0,
           "hlink": 0, "outerShdw_txt": 0, "err": None}
    try:
        z = zipfile.ZipFile(path)
    except Exception as e:
        hit["err"] = "zip:" + type(e).__name__
        return hit
    for name in z.namelist():
        if not name.endswith(".xml"):
            continue
        if not (name.startswith("ppt/slides/") or name.startswith("ppt/slideLayouts/")
                or name.startswith("ppt/slideMasters/") or name.startswith("ppt/notesSlides/")
                or name.startswith("ppt/theme/") or name.startswith("ppt/drawings/")):
            continue
        try:
            root = ET.fromstring(z.read(name))
        except Exception:
            continue
        for el in root.iter():
            t = el.tag
            if t == "{%s}clrChange" % A:
                hit["clrChange"] += 1
            elif t == "{%s}blip" % A:
                hit["blip"] += 1
            elif t == "{%s}hlinkClick" % A and el.get("{%s}id" % R):
                hit["hlink"] += 1
            elif t in ("{%s}rPr" % A, "{%s}defRPr" % A, "{%s}endParaRPr" % A):
                u = el.get("u")
                if u is not None and u != "none":
                    if t == "{%s}rPr" % A:
                        hit["u_runs"] += 1
                    else:
                        hit["u_defrpr"] += 1
                if el.find("{%s}effectLst/{%s}outerShdw" % (A, A)) is not None:
                    hit["outerShdw_txt"] += 1
    z.close()
    return hit


def main(root):
    rows = []
    for dirpath, _dirs, files in os.walk(root):
        for f in sorted(files):
            p = os.path.join(dirpath, f)
            ext = os.path.splitext(f)[1].lower()
            if ext in (".ppt", ".pot", ".pps"):
                rows.append((f, "ppt", walk_ppt(p)))
            elif ext in (".pptx", ".pptm", ".potx", ".ppsx"):
                rows.append((f, "pptx", walk_pptx(p)))
    json.dump(rows, open(sys.argv[2], "w"), indent=1)
    print("documents:", len(rows))


if __name__ == "__main__":
    main(sys.argv[1])
