#!/usr/bin/env python3
"""Census of Escher picture crops on the words and sheets tracks, by walking records.

    crop-census.py <corpus-root> [words|sheets|slides]

Deliberately NOT a regex. The previous round's regex census over XML was wrong by a
factor of sixteen because it paired alternating tag names with no backreference; the
same round's *binary* walker over the Escher property table was right to within one
deck. So this walks records:

  .doc  WordDocument FIB -> table stream -> fcDggInfo/lcbDggInfo (fibRgFcLcb97[50])
        -> the OfficeArt blob -> every OfficeArtSpContainer's OPT.
  .xls  Workbook/Book stream -> BIFF records -> MSODRAWING (0x00EC) payloads,
        concatenated per substream because a container straddles the record split
        -> the same Escher walk.

A shape counts when at least one of properties 256..259 (cropFromTop/Bottom/Left/Right)
is non-zero *read as a signed 32-bit value*, which is how EscherPicture.Fraction reads
them. `pib` (260) is reported alongside because a crop on a shape carrying no picture
cannot move a rendering.

The zipped members of each track (.docx/.xlsx) are counted separately, by their
`a:srcRect` element, purely to record what is NOT wired by this round: an element with
no children needs no backreference and cannot make the previous round's mistake.
"""
import re
import struct
import sys
import zipfile
from pathlib import Path

import olefile

CROP_IDS = {256: "top", 257: "bottom", 258: "left", 259: "right"}
PICTURE_ID = 260

SP_CONTAINER = 0xF004
SPGR_CONTAINER = 0xF003
OPT = 0xF00B
OPT2 = 0xF121
OPT3 = 0xF122
SP = 0xF00A


def signed(v):
    return v - 2**32 if v >= 2**31 else v


def children(buf, off, end):
    """Yield (version, instance, type, body-start, body-end) for records in [off,end)."""
    while off + 8 <= end:
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        body = off + 8
        if rl > end - body:
            stop = end
        else:
            stop = body + rl
        yield (vi & 0x0F, vi >> 4, rt, body, stop)
        off = body + rl
        if rl == 0 and rt == 0:
            break


def properties(buf, body, stop, count):
    props = {}
    p = body
    for _ in range(count):
        if p + 6 > stop:
            break
        pid, val = struct.unpack_from("<HI", buf, p)
        p += 6
        props[pid & 0x3FFF] = val
    return props


def shapes(buf, off, end, out, depth=0):
    if depth > 24:
        return
    for ver, inst, rt, b, s in children(buf, off, end):
        if rt == SP_CONTAINER:
            sp = {"props": {}, "type": None}
            for v2, i2, r2, b2, s2 in children(buf, b, s):
                if r2 == SP:
                    sp["type"] = i2
                elif r2 in (OPT, OPT2, OPT3):
                    sp["props"].update(properties(buf, b2, s2, i2))
            out.append(sp)
        elif rt == SPGR_CONTAINER or ver == 0x0F:
            shapes(buf, b, s, out, depth + 1)


def scan_shapes(buf, out):
    """Every OfficeArtSpContainer in a buffer, found by scanning rather than by walking.

    The recursive walk below is right whenever the container tree starts at offset 0 and
    every record is where its parent's length says it is. Two of the four hosts break that:

      * a .doc's OfficeArtContent is a DggContainer followed by OfficeArtWordDrawings, and
        each Word drawing has a one-byte `dgglbl` in front of its DgContainer, so a walk
        that trusts the previous record's length reads that byte as the next header and
        finds nothing at all beyond the first record;
      * an inline picture's container sits behind a PICF in the Data stream at an offset
        only the piece table knows.

    So the shapes are scanned for and validated three ways — the version nibble must be
    0x0F, the length must fit, and the first child must be an FSP (0xF00A) of exactly 8
    bytes. Validated against the walk on the .ppt track, where both are applicable: 51
    decks, same shape count, same 16 documents / 100 cropped shapes.
    """
    off = 0
    while off + 16 <= len(buf):
        vi, rt, rl = struct.unpack_from("<HHI", buf, off)
        if rt == SP_CONTAINER and (vi & 0x0F) == 0x0F and 8 <= rl <= len(buf) - off - 8:
            v2, r2, l2 = struct.unpack_from("<HHI", buf, off + 8)
            if r2 == SP and l2 == 8:
                sp = {"props": {}, "type": None}
                for _v, i3, r3, b3, s3 in children(buf, off + 8, off + 8 + rl):
                    if r3 == SP:
                        sp["type"] = i3
                    elif r3 in (OPT, OPT2, OPT3):
                        sp["props"].update(properties(buf, b3, s3, i3))
                out.append(sp)
                off += 8 + rl
                continue
        off += 1


def cropped_shapes(buf):
    """(cropped shapes, total shapes, shapes carrying a picture) in an OfficeArt blob.

    The last two are the instrument's own control: a walker that reached no shapes at
    all, or no pictures, would report zero crops for a reason that has nothing to do
    with the corpus.
    """
    out = []
    scan_shapes(buf, out)
    found = []
    pictures = 0
    for sp in out:
        if sp["props"].get(PICTURE_ID, 0):
            pictures += 1
        crops = {
            CROP_IDS[k]: signed(v)
            for k, v in sp["props"].items()
            if k in CROP_IDS and signed(v) != 0
        }
        if crops:
            found.append((crops, sp["props"].get(PICTURE_ID, 0), sp["type"]))
    return found, len(out), pictures


# --------------------------------------------------------------------------- .doc


def doc_office_art(ole):
    """The OfficeArt blob a .doc's FIB names, or b''."""
    fib = ole.openstream("WordDocument").read()
    if len(fib) < 0x100:
        return b""
    flags = struct.unpack_from("<H", fib, 0x0A)[0]
    table_name = "1Table" if (flags & 0x0200) else "0Table"
    if not ole.exists(table_name):
        return b""

    # FIB: base(32) csw rgW97 cslw rgLw97 cbRgFcLcb fibRgFcLcb97
    p = 32
    csw = struct.unpack_from("<H", fib, p)[0]
    p += 2 + csw * 2
    cslw = struct.unpack_from("<H", fib, p)[0]
    p += 2 + cslw * 4
    cb = struct.unpack_from("<H", fib, p)[0]
    p += 2
    if cb <= 50:
        return b""
    fc, lcb = struct.unpack_from("<II", fib, p + 50 * 8)
    if lcb == 0:
        return b""

    table = ole.openstream(table_name).read()
    return table[fc:fc + lcb]


def doc_inline_containers(ole):
    """Every inline picture's OfficeArtSpContainer in the Data stream.

    An inline picture in a .doc is NOT in the fcDggInfo blob — it hangs off a
    sprmCPicLocation offset into the Data stream, as a PICF followed by an
    OfficeArtInlineSpContainer (this is what Ww8DocumentReader.InlinePicture and
    EscherBlips.Inline read). Reaching the piece table and the character sprms from
    here would be a second Word reader, so the stream is scanned for the container
    header instead, and validated: the version nibble must be 0x0F, the length must
    fit, and the first child must be an FSP (0xF00A) of exactly 8 bytes. That triple
    is what stops a length field inside a JPEG from being read as a shape.
    """
    if not ole.exists("Data"):
        return []
    data = ole.openstream("Data").read()

    out = []
    off = 0
    while off + 16 <= len(data):
        vi, rt, rl = struct.unpack_from("<HHI", data, off)
        if rt == SP_CONTAINER and (vi & 0x0F) == 0x0F and 8 <= rl <= len(data) - off - 8:
            v2, r2, l2 = struct.unpack_from("<HHI", data, off + 8)
            if r2 == SP and l2 == 8:
                out.append(data[off:off + 8 + rl])
                off += 8 + rl
                continue
        off += 1
    return out


# --------------------------------------------------------------------------- .xls

MSODRAWINGGROUP = 0x00EB
MSODRAWING = 0x00EC
MSODRAWINGSELECTION = 0x00ED
CONTINUE = 0x003C
BOF = 0x0809
EOF_ = 0x000A


def xls_office_art(ole):
    """One OfficeArt blob per BIFF substream, concatenated as the reader does."""
    name = "Workbook" if ole.exists("Workbook") else ("Book" if ole.exists("Book") else None)
    if name is None:
        return []
    data = ole.openstream(name).read()

    blobs, current, joining = [], bytearray(), False
    p = 0
    while p + 4 <= len(data):
        rt, rl = struct.unpack_from("<HH", data, p)
        body = p + 4
        if body + rl > len(data):
            break
        payload = data[body:body + rl]
        if rt == BOF:
            if current:
                blobs.append(bytes(current))
            current, joining = bytearray(), False
        elif rt in (MSODRAWING, MSODRAWINGGROUP):
            current += payload
            joining = True
        elif rt == CONTINUE and joining:
            current += payload
        elif rt != MSODRAWINGSELECTION:
            joining = False
        p = body + rl

    if current:
        blobs.append(bytes(current))
    return blobs


# --------------------------------------------------------------------------- zipped

SRCRECT = re.compile(r"<a:srcRect\b[^>]*>")
NONZERO = re.compile(r'\b[ltrb]="(-?\d+)"')
FOCLIP = re.compile(r'fo:clip="([^"]*)"')


def zipped_srcrect(path):
    n = 0
    try:
        with zipfile.ZipFile(path) as z:
            for info in z.infolist():
                if not info.filename.lower().endswith((".xml", ".rels")):
                    continue
                try:
                    text = z.read(info).decode("utf-8", "replace")
                except Exception:
                    continue
                for m in SRCRECT.finditer(text):
                    if any(int(v) != 0 for v in NONZERO.findall(m.group(0))):
                        n += 1
                for m in FOCLIP.finditer(text):
                    if "0cm" not in m.group(1) or "rect" not in m.group(1):
                        n += 1
    except Exception:
        return 0
    return n


# --------------------------------------------------------------------------- main


def ppt_office_art(ole):
    """The whole PowerPoint Document stream: the Escher walk finds its PPDrawings."""
    for entry in ole.listdir():
        if entry[-1].lower() == "powerpoint document":
            return [ole.openstream(entry).read()]
    return []


def probe(path):
    """(kind, cropped shapes, total shapes, shapes with a picture)."""
    with open(path, "rb") as fh:
        head = fh.read(8)

    if head[:8] == b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1":
        try:
            ole = olefile.OleFileIO(str(path))
        except Exception:
            return ("ole-unreadable", [], 0, 0)
        try:
            if ole.exists("WordDocument"):
                floating = [doc_office_art(ole)]
                inline = doc_inline_containers(ole)
                blobs = floating + inline
                kind = "doc"
            elif ole.exists("Workbook") or ole.exists("Book"):
                blobs, kind = xls_office_art(ole), "xls"
            else:
                blobs, kind = ppt_office_art(ole), "ppt"
                if not blobs:
                    return ("ole-other", [], 0, 0)
        except Exception:
            return ("ole-error", [], 0, 0)

        found, total, pictures = [], 0, 0
        for blob in blobs:
            if not blob:
                continue
            try:
                f, t, p = cropped_shapes(blob)
            except Exception:
                continue
            found += f
            total += t
            pictures += p
        return (kind, found, total, pictures)

    if head[:2] == b"PK":
        n = zipped_srcrect(path)
        return ("zip", [("srcRect", n, None)] if n else [], 0, 0)

    return ("other", [], 0, 0)


def main():
    root = Path(sys.argv[1])
    tracks = sys.argv[2:] or ["words", "sheets", "slides"]

    for track in tracks:
        files = sorted(
            p for p in (root / track).rglob("*") if p.is_file() and not p.name.startswith("."))
        crop_docs = crop_shapes = pib_docs = pib_shapes = 0
        zip_docs = zip_shapes = 0
        n_binary = all_shapes = all_pictures = shape_docs = 0

        for path in files:
            kind, found, total, pictures = probe(path)
            if kind in ("doc", "xls", "ppt"):
                n_binary += 1
                all_shapes += total
                all_pictures += pictures
                if total:
                    shape_docs += 1
                with_pib = [f for f in found if f[1]]
                if found:
                    crop_docs += 1
                    crop_shapes += len(found)
                    print(f"  {track}/{path.name}\t{kind}\t{len(found)} cropped, "
                          f"{len(with_pib)} with a picture")
                    for crops, pib, sptype in found:
                        frac = " ".join(f"{k}={v / 65536.0:.4f}" for k, v in crops.items())
                        print(f"      pib={pib} type={sptype} {frac}")
                if with_pib:
                    pib_docs += 1
                    pib_shapes += len(with_pib)
            elif kind == "zip" and found:
                zip_docs += 1
                zip_shapes += found[0][1]

        print(f"{track}: {len(files)} files, {n_binary} OLE2")
        print(f"{track}: CONTROL {shape_docs} documents reached, {all_shapes} Escher shapes, "
              f"{all_pictures} of them carrying a pib")
        print(f"{track}: BINARY  {crop_docs} documents / {crop_shapes} shapes carry a crop; "
              f"{pib_docs} documents / {pib_shapes} shapes carry a crop AND a picture")
        print(f"{track}: ZIPPED  {zip_docs} documents / {zip_shapes} a:srcRect|fo:clip "
              f"(NOT wired by this round)")
        print()


if __name__ == "__main__":
    main()
